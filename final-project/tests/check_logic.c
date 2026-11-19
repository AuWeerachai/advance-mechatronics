#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>

#define main firmware_main
#ifndef TEST_MAIN_SOURCE
#define TEST_MAIN_SOURCE "../src/main.c"
#endif
#include TEST_MAIN_SOURCE
#undef main

static uint32_t testTime;
static uint32_t fanDuty;
static int32_t motorPower;
static uint32_t shownAirLEDs;
static uint32_t airLedWrites;
static char incoming;
static uint32_t incomingReady;
static uint32_t leverReading;
static uint32_t ballReading;
static uint32_t handReading;
static char output[16384];
static size_t outputLength;

void initTimer2(void) {}
uint32_t readTimer2(void) { return testTime; }
void initUSART3(void) {}
void initSensors(void) {}
void initFan(void) { fanDuty = 0; }
void initMotor(void) { motorPower = 0; }
void initAirLEDs(void) { shownAirLEDs = 0; airLedWrites = 0; }
void writeAirLEDs(uint32_t count) { shownAirLEDs = count; airLedWrites++; }
void writeFanPower(uint32_t duty) { fanDuty = duty; }
void writeMotorPower(int32_t power) { motorPower = power; }
uint32_t readBallSensor(void) { return ballReading; }
uint32_t readHandSensor(void) { return handReading; }
uint32_t calculateSharpDistanceCm(uint32_t reading) { return reading == 2048 ? 36 : 0; }
uint32_t readLeverSensor(void) { return leverReading; }
uint32_t readClockStartSensor(void) { return 1; }
uint32_t readClockEndSensor(void) { return 0; }
uint16_t returnStatusRegisterUART3(void) { return incomingReady ? 0x20 : 0; }
char receiveCharUSART3(void) { assert(incomingReady); incomingReady = 0; return incoming; }
void printfUSART3(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(output + outputLength, sizeof(output) - outputLength, fmt, args);
    va_end(args);
    assert(n >= 0 && (size_t)n < sizeof(output) - outputLength);
    outputLength += (size_t)n;
}
void transmitLineUSART3(const char *text) { printfUSART3("%s\n", text); }
static void command(char value)
{
    incoming = value;
    incomingReady = 1;
    runProject();
}
static void reset(uint32_t time)
{
    testTime = time;
    outputLength = 0;
    output[0] = 0;
    incomingReady = 0;
    leverReading = 0;
    ballReading = 4095;
    handReading = 2048;
    initProject();
    assert(state == WELCOME && airLevel == 0 && fanDuty == 0 && motorPower == 0);
    assert(pumpCount == 0 && pumpStep == PUMP_WAIT_FOR_RESET);
    assert(shownAirLEDs == 0);
}
static void moveLever(uint32_t value)
{
    leverReading = value;
    runProject();
}
int main(void)
{
    char expected[180];
    reset(0);
    runProject();  // No serial byte: must not call the blocking receiver.
    command('p'); assert(state == WELCOME && airLevel == 0);
    command('h'); assert(state == PLAYING && airLevel == 100 && shownAirLEDs == 10);
    testTime = 5000000; runProject(); assert(airLevel == 85);
    for(uint32_t cycle = 1; cycle <= PUMPS_PER_AIR_LED; cycle++)
    {
        command('p');
        assert(pumpCount == cycle % PUMPS_PER_AIR_LED);
        assert(airLevel == (cycle == PUMPS_PER_AIR_LED ? 95u : 85u));
    }
    for(uint32_t cycle = 1; cycle <= PUMPS_PER_AIR_LED; cycle++)
    {
        command('p');
        assert(airLevel == (cycle == PUMPS_PER_AIR_LED ? 100u : 95u));
    }
    command('h'); assert(gameStartTime == 0);  // Repeated start must not extend the game.
    command('f'); command('m'); command('r'); assert(fanDuty == 0 && motorPower == 0);
    testTime = 34000000; runProject(); assert(airLevel == 13);
    testTime = 44000000; runProject(); assert(airLevel == 0 && state == PLAYING);
    for(uint32_t cycle = 1; cycle <= PUMPS_PER_AIR_LED; cycle++)
    {
        command('p');
        assert(airLevel == (cycle == PUMPS_PER_AIR_LED ? 10u : 0u));
    }
    testTime = 45000000; runProject(); assert(state == WELCOME && airLevel == 0 && shownAirLEDs == 0);
    command('h'); assert(state == PLAYING && airLevel == 100);
    command('t'); assert(state == WELCOME);

    // The whole bar is assigned from the current air level on every main-loop pass.
    reset(0);
    command('h'); assert(shownAirLEDs == 10);
    uint32_t writes = airLedWrites;
    runProject(); assert(shownAirLEDs == 10 && airLedWrites == writes + 1);
    testTime = 23000000; runProject(); assert(airLevel == 31 && shownAirLEDs == 3);
    testTime = 24000000; runProject(); assert(airLevel == 28 && shownAirLEDs == 2);
    for(uint32_t cycle = 0; cycle < PUMPS_PER_AIR_LED; cycle++)
    {
        command('p');
    }
    assert(airLevel == 38 && shownAirLEDs == 3);
    command('x'); assert(state == WELCOME && shownAirLEDs == 0);
    puts("PASS: air LED initialization, absolute updates, drain, pumping, timeout and stop.");

    // Starting high cannot count until a new low-high-low cycle.
    reset(0);
    moveLever(170); assert(pumpCount == 0 && pumpStep == PUMP_WAIT_FOR_RESET);
    command('h'); assert(pumpStep == PUMP_WAIT_FOR_RESET);
    testTime = 5000000;
    moveLever(170); assert(pumpCount == 0 && airLevel == 85);
    moveLever(15); assert(pumpStep == PUMP_WAIT_FOR_RESET);
    moveLever(14); assert(pumpStep == PUMP_WAIT_FOR_HIGH);

    // Partial movements never reach the upper threshold and cannot count.
    for(uint32_t attempt = 0; attempt < 8; attempt++)
    {
        moveLever(16); moveLever(120); moveLever(169); moveLever(0);
        assert(pumpCount == 0 && pumpStep == PUMP_WAIT_FOR_HIGH);
    }

    // Reaching the top does not count. Pauses and reversals are allowed.
    moveLever(170); assert(pumpCount == 0 && pumpStep == PUMP_WAIT_FOR_RETURN);
    moveLever(170); runProject(); assert(pumpCount == 0);
    moveLever(169); moveLever(170); moveLever(75); moveLever(170);
    assert(pumpCount == 0 && pumpStep == PUMP_WAIT_FOR_RETURN);
    moveLever(15); assert(pumpCount == 0); // Exactly 15 is not below the low threshold.
    moveLever(14); assert(pumpCount == 1 && airLevel == 85);
    runProject(); moveLever(0); assert(pumpCount == 1); // Holding low cannot repeat the count.
    snprintf(expected, sizeof(expected), "Pump cycles: 1/%u", PUMPS_PER_AIR_LED);
    assert(strstr(output, expected));

    // Second upward movement cannot award an LED; it still needs the return.
    moveLever(170); assert(pumpCount == 1 && airLevel == 85);
    runProject(); assert(pumpCount == 1 && airLevel == 85);
    moveLever(14);
    assert(pumpCount == 2 % PUMPS_PER_AIR_LED);
    assert(airLevel == 85 + 10 * (2 / PUMPS_PER_AIR_LED));
    for(uint32_t cycle = 3; cycle <= PUMPS_PER_AIR_LED; cycle++)
    {
        moveLever(170); assert(pumpCount == cycle - 1 && airLevel == 85);
        moveLever(0);
        assert(pumpCount == cycle % PUMPS_PER_AIR_LED);
        assert(airLevel == (cycle == PUMPS_PER_AIR_LED ? 95u : 85u));
    }
    assert(pumpCount == 0 && airLevel == 95);
    snprintf(expected, sizeof(expected), "%u complete cycles: add 1 air LED; air = 95 percent", PUMPS_PER_AIR_LED);
    assert(strstr(output, expected));

    // A new group starts from zero; partial movements cannot erase completed cycles.
    moveLever(170); moveLever(120); moveLever(170); moveLever(0);
    assert(pumpCount == 1 && pumpStep == PUMP_WAIT_FOR_HIGH);
    moveLever(169); moveLever(0); assert(pumpCount == 1 && airLevel == 95);
    for(uint32_t cycle = 2; cycle <= PUMPS_PER_AIR_LED; cycle++)
    {
        moveLever(170); moveLever(0);
    }
    assert(pumpCount == 0 && airLevel == 100);
    assert(strstr(output, "Pump cancelled") == NULL);

    // New games discard incomplete movements and unredeemed cycles.
    moveLever(170); moveLever(0); assert(pumpCount == 1);
    moveLever(170); assert(pumpStep == PUMP_WAIT_FOR_RETURN);
    command('t'); assert(pumpCount == 0 && pumpStep == PUMP_WAIT_FOR_RESET);
    command('h'); assert(pumpCount == 0 && pumpStep == PUMP_WAIT_FOR_RESET);
    moveLever(0); assert(pumpCount == 0);
    moveLever(170);
    testTime += GAME_DURATION_US;
    moveLever(0); assert(state == WELCOME && pumpCount == 0);

    reset(0);
    command('f'); assert(fanDuty == 300 && motorPower == 0);
    testTime = 1999999; runProject(); assert(fanDuty == 300);
    testTime = 2000000; runProject(); assert(fanDuty == 0 && fanTestRunning == 0);
    command('m'); assert(motorPower == 200);
    testTime += 249999; runProject(); assert(motorPower == 200);
    testTime += 1; runProject(); assert(motorPower == 0);
    command('r'); assert(motorPower == -200);
    command('x'); assert(motorPower == 0 && fanDuty == 0 && state == WELCOME);
    command('f'); command('m'); assert(fanDuty == 0 && motorPower == 200);
    command('h'); assert(state == PLAYING && motorPower == 0 && !motorTestRunning);
    command('\r'); command('\n'); assert(state == PLAYING);
    command('x'); assert(state == WELCOME);

    reset(UINT32_MAX - 20000000u);
    command('h');
    testTime += 44000000u; runProject(); assert(state == PLAYING);
    testTime += 1000000u; runProject(); assert(state == WELCOME);
    reset(UINT32_MAX - 100000u);
    command('m'); testTime += 250000u; runProject(); assert(motorPower == 0);
    command('f'); testTime += 2000000u; runProject(); assert(fanDuty == 0);

    reset(0);
    testTime = 200000; printSensorReadings();
    assert(strstr(output,"Ball A0: 4095 (3300 mV), Hand A1: 2048 (1650 mV), Lever A2: 0 (0 mV)"));
    assert(strstr(output,"Clock START D2: 1, Clock END D3: 0 (raw input levels)"));
    assert(strstr(output, "Hand distance: about 36 cm"));
    assert(strstr(output, "Ball reading outside calibrated range"));
    outputLength = 0;
    output[0] = 0;
    ballReading = 2048;
    handReading = 0;
    testTime += 200000; printSensorReadings();
    assert(strstr(output, "Ball distance: about 36 cm"));
    assert(strstr(output, "Hand reading outside calibrated range"));
    assert(!strstr(output, "Hand distance: about"));
    assert(!strstr(output, "Ball reading outside calibrated range"));
    assert(state == WELCOME && fanDuty == 0 && motorPower == 0);
    command('h'); testTime += GAME_DURATION_US + 1; printSensorReadings();
    assert(strstr(output,"0 seconds left"));  // Printing across the boundary must not underflow.
    printf("PASS: %u complete cycles per LED, min-max-min thresholds, pauses/reversals allowed, return-only counting, game/serial/timer regressions.\n", PUMPS_PER_AIR_LED);
}
