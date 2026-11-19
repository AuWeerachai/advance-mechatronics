#include <stdint.h>
#include "stm32f446xx_timer_mort.h"
#include "stm32f446xx_usart3_mort.h"
#include "led.h"
#include "sensors.h"
#include "calibration.h"
#include "fan.h"
#include "motor.h"

#define GAME_DURATION_US ((uint32_t)45000000)   // Checkpoint 2: 45-second game.
#define SENSOR_PRINT_INTERVAL_US ((uint32_t)200000) // 5 serial updates per second.
#define PUMPS_PER_AIR_LED ((uint32_t)2)    // Complete low-high-low cycles per LED. Change this to 5 if wanted.
#define AIR_PER_LED ((uint32_t)10)            // Each of the 10 LEDs represents 10% air.
#define AIR_DRAIN_PER_SECOND ((uint32_t)3)      // Initial review setting; tune during play testing.
#define FAN_TEST_DUTY ((uint32_t)300)           // Chosen test command: 30% PWM HIGH time.
#define FAN_TEST_DURATION_US ((uint32_t)2000000) // End the independent fan test after 2 seconds.
#define MOTOR_TEST_POWER ((int32_t)200)         // Chosen test command: 20% drive.
#define MOTOR_TEST_DURATION_US ((uint32_t)250000) // Short 0.25-second direction test.
#define USART3_RX_READY ((uint16_t)0x20)        // USART SR bit 5: received data is ready.

enum GameState
{
    WELCOME,
    PLAYING
};

enum GameEvent
{
    NO_EVENT,
    HAND_PRESENT,
    PUMP_STROKE,  // One complete low-high-low movement.
    TIME_UP
};

// These steps track one pump movement; the game still has WELCOME and PLAYING.
enum PumpStep
{
    PUMP_WAIT_FOR_RESET,
    PUMP_WAIT_FOR_HIGH,
    PUMP_WAIT_FOR_RETURN
};

static enum GameState state;
static uint32_t airLevel;
static uint32_t gameStartTime;
static uint32_t previousAirTime;
static uint32_t previousPrintTime;
static enum PumpStep pumpStep;
static uint32_t pumpCount;
static uint32_t fanTestRunning;
static uint32_t fanTestStartTime;
static uint32_t motorTestRunning;
static uint32_t motorTestStartTime;

void stopActuatorTests(void)
{
    // Apply zero drive commands and cancel both independent test timers.
    writeFanPower(0);
    writeMotorPower(0);
    fanTestRunning = 0;
    motorTestRunning = 0;
}

void dispatchGameEvent(enum GameEvent event)
{
    // Handle an event according to the current state. Only two game states exist.
    switch(state)
    {
        case WELCOME:
            if(event == HAND_PRESENT)
            {
                stopActuatorTests();
                airLevel = 100;
                pumpStep = PUMP_WAIT_FOR_RESET;
                pumpCount = 0;
                gameStartTime = readTimer2();
                previousAirTime = gameStartTime;
                state = PLAYING;
                transmitLineUSART3("WELCOME -> PLAYING: 45-second countdown started");
            }
            break;

        case PLAYING:
            if(event == TIME_UP)
            {
                stopActuatorTests();
                airLevel = 0;
                pumpStep = PUMP_WAIT_FOR_RESET;
                pumpCount = 0;
                state = WELCOME;
                transmitLineUSART3("PLAYING -> WELCOME: game ended");
            }
            else if(event == PUMP_STROKE)
            {
                pumpCount = pumpCount + 1;
                printfUSART3("Pump cycles: %u/%u\r\n", pumpCount, PUMPS_PER_AIR_LED);
                if(pumpCount == PUMPS_PER_AIR_LED)
                {
                    pumpCount = 0;
                    airLevel = airLevel + AIR_PER_LED;
                    if(airLevel > 100)
                    {
                        airLevel = 100;
                    }
                    printfUSART3("%u complete cycles: add 1 air LED; air = %u percent, air LEDs = %u/10\r\n",
                                 PUMPS_PER_AIR_LED, airLevel, airLevel / AIR_PER_LED);
                }
            }
            break;
    }
}

void checkPumpInput(void)
{
    uint32_t lever;

    if(state != PLAYING)
    {
        return;
    }
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
            // Wait for the upper threshold. Partial movements do not count.
            if(lever >= PUMP_HIGH_ADC)
            {
                pumpStep = PUMP_WAIT_FOR_RETURN;
                printfUSART3("Pump reached top: raw ADC = %u; return below %u to count\r\n", lever, PUMP_LOW_ADC);
            }
            break;

        case PUMP_WAIT_FOR_RETURN:
            // Returning to minimum completes one cycle and prepares the next one.
            if(lever < PUMP_LOW_ADC)
            {
                pumpStep = PUMP_WAIT_FOR_HIGH;
                printfUSART3("Pump cycle complete: raw ADC = %u\r\n", lever);
                dispatchGameEvent(PUMP_STROKE);
            }
            break;
    }
}

void updateGame(void)
{
    uint32_t currentTime;
    uint32_t elapsedSeconds;
    uint32_t airUsed;

    // Timeout has priority over other events; keep the air update nonblocking.
    if(state != PLAYING)
    {
        return;
    }
    currentTime = readTimer2();
    if(currentTime - gameStartTime >= GAME_DURATION_US)
    {
        dispatchGameEvent(TIME_UP);
        return;
    }

    elapsedSeconds = (currentTime - previousAirTime) / 1000000;
    if(elapsedSeconds != 0)
    {
        previousAirTime = previousAirTime + elapsedSeconds * 1000000;
        airUsed = elapsedSeconds * AIR_DRAIN_PER_SECOND;
        if(airUsed >= airLevel)
        {
            airLevel = 0;
        }
        else
        {
            airLevel = airLevel - airUsed;
        }
    }
}

void updateActuatorTests(void)
{
    uint32_t currentTime;

    // Return each independent actuator test to zero drive when its time expires.
    currentTime = readTimer2();
    if(fanTestRunning == 1 && currentTime - fanTestStartTime >= FAN_TEST_DURATION_US)
    {
        writeFanPower(0);
        fanTestRunning = 0;
        transmitLineUSART3("Fan test ended: zero PWM command");
    }
    if(motorTestRunning == 1 && currentTime - motorTestStartTime >= MOTOR_TEST_DURATION_US)
    {
        writeMotorPower(0);
        motorTestRunning = 0;
        transmitLineUSART3("Motor test ended: zero drive");
    }
}

void printCommands(void)
{
    // Single-character commands work with any serial-monitor line-ending option.
    transmitLineUSART3("h = simulate hand present; p = simulate one complete pump cycle; t = simulate timeout");
    printfUSART3("In PLAYING: below %u -> at least %u -> below %u completes 1 cycle; %u cycles = 1 air LED\r\n",
                 PUMP_LOW_ADC, PUMP_HIGH_ADC, PUMP_LOW_ADC, PUMPS_PER_AIR_LED);
    transmitLineUSART3("f = fan test; m = motor forward test; r = motor reverse test (WELCOME only)");
    transmitLineUSART3("x = stop outputs and return to WELCOME; ? = show commands");
}

void checkSerialInput(void)
{
    char command;

    // Read only when a byte is ready, so waiting for serial input never blocks the game.
    if((returnStatusRegisterUART3() & USART3_RX_READY) == 0)
    {
        return;
    }
    command = receiveCharUSART3();
    switch(command)
    {
        case 'h':
            dispatchGameEvent(HAND_PRESENT);
            break;
        case 'p':
            transmitLineUSART3("Serial command: simulated complete pump cycle");
            dispatchGameEvent(PUMP_STROKE);
            break;
        case 't':
            dispatchGameEvent(TIME_UP);
            break;
        case 'f':
            if(state == WELCOME)
            {
                stopActuatorTests();
                writeFanPower(FAN_TEST_DUTY);
                fanTestStartTime = readTimer2();
                fanTestRunning = 1;
                transmitLineUSART3("Fan test: 30 percent for 2 seconds");
            }
            break;
        case 'm':
        case 'r':
            if(state == WELCOME)
            {
                stopActuatorTests();
                if(command == 'm')
                {
                    writeMotorPower(MOTOR_TEST_POWER);
                }
                else
                {
                    writeMotorPower(-MOTOR_TEST_POWER);
                }
                motorTestStartTime = readTimer2();
                motorTestRunning = 1;
                transmitLineUSART3("Motor test: 20 percent for 0.25 seconds");
            }
            break;
        case 'x':
            stopActuatorTests();
            dispatchGameEvent(TIME_UP);
            transmitLineUSART3("Zero drive commanded; WELCOME");
            break;
        case '?':
            printCommands();
            break;
        case '\r':
        case '\n':
            break;
        default:
            transmitLineUSART3("Unknown command; send ? for commands");
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
    uint32_t remainingSeconds;

    // Print raw readings, approximate millivolts and estimated ball/hand distances.
    currentTime = readTimer2();
    if(currentTime - previousPrintTime < SENSOR_PRINT_INTERVAL_US)
    {
        return;
    }
    previousPrintTime = currentTime;
    ball = readBallSensor();
    hand = readHandSensor();
    ballDistance = calculateSharpDistanceCm(ball);
    handDistance = calculateSharpDistanceCm(hand);
    lever = readLeverSensor();
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
    printfUSART3("Clock START D2: %u, Clock END D3: %u (raw input levels)\r\n",
                 readClockStartSensor(), readClockEndSensor());
    if(state == PLAYING)
    {
        remainingSeconds = 0;
        if(currentTime - gameStartTime < GAME_DURATION_US)
        {
            remainingSeconds = (GAME_DURATION_US - (currentTime - gameStartTime) + 999999) / 1000000;
        }
        printfUSART3("PLAYING: %u seconds left, air = %u percent, air LEDs = %u/10, pump cycles = %u/%u\r\n",
                     remainingSeconds, airLevel, airLevel / AIR_PER_LED, pumpCount, PUMPS_PER_AIR_LED);
    }
    else
    {
        transmitLineUSART3("WELCOME");
    }
}

void initProject(void)
{
    // Start timing, serial, sensing and zero-drive outputs before accepting events.
    initTimer2();
    initUSART3();
    initSensors();
    initFan();
    initMotor();
    initAirLEDs();
    stopActuatorTests();
    state = WELCOME;
    airLevel = 0;
    pumpStep = PUMP_WAIT_FOR_RESET;
    pumpCount = 0;
    previousPrintTime = readTimer2();
    transmitLineUSART3("Air Levitation Tower: Checkpoint 2 review");
    printCommands();
}

void runProject(void)
{
    // Check timers, accept serial and pump events, then display the current readings.
    updateGame();
    updateActuatorTests();
    checkSerialInput();
    checkPumpInput();
    writeAirLEDs(airLevel / AIR_PER_LED); // Assign the whole bar from the current air level.
    printSensorReadings();
}

int main(void)
{
    initProject();
    while(1)
    {
        runProject();
    }
}
