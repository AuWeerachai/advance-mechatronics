#include <stdint.h>
#include "stm32f446xx_timer_mort.h"
#include "stm32f446xx_usart3_mort.h"
#include "sensors.h"
#include "calibration.h"
#include "motor.h"
#include "motor_controller.h"

#define SENSOR_PRINT_INTERVAL_US ((uint32_t)200000) // Print every 0.2 seconds.

enum PumpStep
{
    PUMP_WAIT_FOR_RESET,
    PUMP_WAIT_FOR_HIGH,
    PUMP_WAIT_FOR_RETURN
};

static enum PumpStep pumpStep;
static uint32_t pumpCount;
static uint32_t previousPrintTime;
static uint32_t motorDuty;

void checkPumpInput(void)
{
    uint32_t lever;

    lever = readLeverSensor();
    switch(pumpStep)
    {
        case PUMP_WAIT_FOR_RESET:
            // Start at minimum: below 15.
            if(lever < PUMP_LOW_ADC)
            {
                pumpStep = PUMP_WAIT_FOR_HIGH;
                printfUSART3("Pump ready: raw ADC = %u\r\n", lever);
            }
            break;

        case PUMP_WAIT_FOR_HIGH:
            // Reaching the upper threshold does not complete a cycle yet.
            if(lever >= PUMP_HIGH_ADC)
            {
                pumpStep = PUMP_WAIT_FOR_RETURN;
                printfUSART3("Pump reached top: raw ADC = %u; return below %u to count\r\n",
                             lever, PUMP_LOW_ADC);
            }
            break;

        case PUMP_WAIT_FOR_RETURN:
            // Count once on the return to minimum, then wait for the next maximum.
            if(lever < PUMP_LOW_ADC)
            {
                pumpCount = pumpCount + 1;
                pumpStep = PUMP_WAIT_FOR_HIGH;
                printfUSART3("Pump cycle complete: %u\r\n", pumpCount);
            }
            break;
    }
}

void printSensorReadings(void)
{
    uint32_t currentTime;
    uint32_t ball;
    uint32_t hand;
    uint32_t ballDistance;
    uint32_t handDistance;
    uint32_t lever;

    currentTime = readTimer2();
    if(currentTime - previousPrintTime < SENSOR_PRINT_INTERVAL_US)
    {
        return;
    }
    previousPrintTime = currentTime;
    ball = readBallSensor();
    hand = readHandSensor();
    lever = readLeverSensor();
    ballDistance = calculateSharpDistanceCm(ball);
    handDistance = calculateSharpDistanceCm(hand);

    printfUSART3("Ball A0: %u (%u mV), Hand A1: %u (%u mV), Lever A2: %u (%u mV)\r\n",
                 ball, ball * 3300 / 4095, hand, hand * 3300 / 4095, lever, lever * 3300 / 4095);
    if(ballDistance != 0)
    {
        printfUSART3("Ball distance: about %u cm\r\n", ballDistance);
    }
    else
    {
        transmitLineUSART3("Ball reading outside calibrated range");
    }
    if(handDistance != 0)
    {
        printfUSART3("Hand distance: about %u cm\r\n", handDistance);
    }
    else
    {
        transmitLineUSART3("Hand reading outside calibrated range");
    }
    printfUSART3("Fan PWM duty: %u%%\r\n", motorDuty / 10);
    printfUSART3("Clock START D2: %u, Clock END D3: %u (raw input levels)\r\n",
                 readClockStartSensor(), readClockEndSensor());
    printfUSART3("Completed pump cycles: %u\r\n", pumpCount);
}

void initSensorTest(void)
{
    initTimer2();
    initUSART3();
    initMotor();  // Start the fan PWM control at 0% duty before reading sensors.
    initSensors();
    pumpStep = PUMP_WAIT_FOR_RESET;
    pumpCount = 0;
    motorDuty = 0;
    previousPrintTime = readTimer2();
    transmitLineUSART3("Sensor test: ball A0, hand A1, lever A2, START D2, END D3");
    transmitLineUSART3("Hand A1 controls fan PWM: D6/PE9 (25 kHz, open-drain)");
    printfUSART3("Lever cycle: below %u -> at least %u -> below %u. RESET clears the count.\r\n",
                 PUMP_LOW_ADC, PUMP_HIGH_ADC, PUMP_LOW_ADC);
}

void runSensorTest(void)
{
    motorDuty = runMotorController();
    checkPumpInput();
    printSensorReadings();
}

int main(void)
{
    initSensorTest();
    while(1)
    {
        runSensorTest();
    }
}
