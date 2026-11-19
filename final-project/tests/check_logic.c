#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define main firmware_main
#include "../src/main.c"
#undef main

static uint32_t testTime;
static uint32_t ballReading;
static uint32_t handReading;
static uint32_t leverReading;
static uint32_t startReading;
static uint32_t endReading;
static uint32_t initialization;
static uint32_t motorCommand;
static char output[16384];
static size_t outputLength;

void initTimer2(void) { assert(initialization == 0); initialization = 1; }
void initUSART3(void) { assert(initialization == 1); initialization = 2; }
void initMotor(void) { assert(initialization == 2); initialization = 3; motorCommand = 0; }
void initSensors(void) { assert(initialization == 3); initialization = 4; }
void writeMotorPower(uint32_t duty) { assert(duty <= 1000); motorCommand = duty; }
uint32_t readTimer2(void) { return testTime; }
uint32_t readBallSensor(void) { return ballReading; }
uint32_t readHandSensor(void) { return handReading; }
uint32_t readLeverSensor(void) { return leverReading; }
uint32_t readClockStartSensor(void) { return startReading; }
uint32_t readClockEndSensor(void) { return endReading; }
uint32_t calculateSharpDistanceCm(uint32_t reading) { return reading == 2048 ? 36 : 0; }
void printfUSART3(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    int length = vsnprintf(output + outputLength, sizeof(output) - outputLength, format, args);
    va_end(args);
    assert(length >= 0 && (size_t)length < sizeof(output) - outputLength);
    outputLength += (size_t)length;
}
void transmitLineUSART3(const char *text) { printfUSART3("%s\r\n", text); }

static void clearOutput(void)
{
    outputLength = 0;
    output[0] = 0;
}
static void reset(uint32_t time)
{
    clearOutput();
    testTime = time;
    ballReading = 4095;
    handReading = 2048;
    leverReading = 170;
    startReading = 1;
    endReading = 0;
    initialization = 0;
    initSensorTest();
    assert(initialization == 4);
    assert(motorCommand == 0 && motorDuty == 0);
    assert(pumpCount == 0 && pumpStep == PUMP_WAIT_FOR_RESET);
    assert(strstr(output, "Sensor test:"));
    clearOutput();
}
static void moveLever(uint32_t reading)
{
    leverReading = reading;
    runSensorTest();
}

int main(void)
{
    reset(0);
    runSensorTest(); assert(outputLength == 0);
    assert(motorCommand == 320); // 36 cm maps to 32%, even before the first serial print.
    handReading = 0;
    runSensorTest(); assert(motorCommand == 0 && outputLength == 0);
    handReading = 2048;
    testTime = 199999; runSensorTest(); assert(outputLength == 0);
    testTime = 200000; runSensorTest();
    assert(strstr(output, "Ball A0: 4095 (3300 mV), Hand A1: 2048 (1650 mV), Lever A2: 170 (136 mV)"));
    assert(strstr(output, "Ball reading outside calibrated range"));
    assert(strstr(output, "Hand distance: about 36 cm"));
    assert(strstr(output, "Fan PWM duty: 32%"));
    assert(strstr(output, "Clock START D2: 1, Clock END D3: 0 (raw input levels)"));
    assert(strstr(output, "Completed pump cycles: 0"));
    clearOutput();
    runSensorTest(); assert(outputLength == 0);
    ballReading = 2048; handReading = 0;
    startReading = 0; endReading = 1;
    testTime += 200000; runSensorTest();
    assert(strstr(output, "Ball distance: about 36 cm"));
    assert(strstr(output, "Hand reading outside calibrated range"));
    assert(motorCommand == 0 && strstr(output, "Fan PWM duty: 0%"));
    assert(strstr(output, "Clock START D2: 0, Clock END D3: 1 (raw input levels)"));
    assert(!strstr(output, "Hand distance: about"));
    assert(!strstr(output, "WELCOME") && !strstr(output, "PLAYING"));

    // Display timing must work when the 32-bit microsecond timer wraps.
    reset(UINT32_MAX - 100000u);
    testTime += 199999u; runSensorTest(); assert(outputLength == 0);
    testTime += 1u; runSensorTest(); assert(strstr(output, "Ball A0:"));

    // Count complete lever cycles immediately, without a serial start command.
    reset(0);
    moveLever(170); moveLever(15); assert(pumpCount == 0 && pumpStep == PUMP_WAIT_FOR_RESET);
    moveLever(14); assert(pumpStep == PUMP_WAIT_FOR_HIGH);
    for(uint32_t attempt = 0; attempt < 4; attempt++)
    {
        moveLever(16); moveLever(120); moveLever(169); moveLever(0);
        assert(pumpCount == 0 && pumpStep == PUMP_WAIT_FOR_HIGH);
    }
    moveLever(170); assert(pumpCount == 0 && pumpStep == PUMP_WAIT_FOR_RETURN);
    moveLever(170); moveLever(75); moveLever(170); moveLever(15);
    assert(pumpCount == 0);
    moveLever(14); assert(pumpCount == 1);
    moveLever(0); runSensorTest(); assert(pumpCount == 1);
    for(uint32_t cycle = 2; cycle <= 5; cycle++)
    {
        moveLever(170); assert(pumpCount == cycle - 1);
        moveLever(0); assert(pumpCount == cycle);
    }
    testTime = 200000; runSensorTest();
    assert(strstr(output, "Completed pump cycles: 5"));
    reset(0); assert(pumpCount == 0);
    puts("PASS: fan command starts at zero, hand control independent of print timing, serial readings, timer wraparound and lever counting.");
}
