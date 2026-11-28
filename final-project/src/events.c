#include "events.h"
#include "framework.h"
#include "game_config.h"
#include "calibration.h"
#include "sensors.h"
#include "serial.h"
#include "tachometer.h"
#include "state_machine.h"
#include "stm32f446xx_timer_mort.h"

enum LeverRegion
{
    LEVER_UNKNOWN,
    LEVER_MINIMUM,
    LEVER_MIDDLE,
    LEVER_MAXIMUM
};

static enum LeverRegion previousLeverRegion;
static uint32_t previousHandDistance;
static uint32_t buttonCandidate;
static uint32_t buttonCandidateTime;
static uint32_t buttonArmed;
static uint32_t previousClockStart;
static uint32_t previousClockEnd;
static uint32_t previousSensorTime;
static uint32_t previousStatusTime;
static uint32_t gameTimerRunning;
static uint32_t previousAirTime;
static uint32_t roundTimerRunning;
static uint32_t roundTimerStart;
static uint32_t roundTimerDuration;
static enum EventType roundTimeoutEvent;

void resetStartButtonEvents(void)
{
    buttonCandidate = 2; // Neither pressed nor released: wait for a fresh input.
    buttonCandidateTime = readTimer2();
    buttonArmed = 0; // Require a stable release before accepting a press.
}

void resetLeverEventChecker(void)
{
    // Send the present end position once when a new round starts.
    previousLeverRegion = LEVER_UNKNOWN;
}

void refreshSensorEvents(void)
{
    // Resend clock levels after clearing the queue.
    previousClockStart = 2;
    previousClockEnd = 2;
    resetLeverEventChecker();
}

void initEvents(void)
{
    uint32_t now;

    now = readTimer2();
    previousHandDistance = 0;
    resetStartButtonEvents();
    previousClockStart = 2; // Neither 0 nor 1: report initial sensor levels once.
    previousClockEnd = 2;
    previousSensorTime = now;
    previousStatusTime = now;
    gameTimerRunning = 0;
    roundTimerRunning = 0;
    resetLeverEventChecker();
}

void startGameTimers(void)
{
    previousAirTime = readTimer2();
    gameTimerRunning = 1;
}

void stopGameTimers(void)
{
    gameTimerRunning = 0;
    roundTimerRunning = 0;
}

void startRoundTimer(enum EventType event, uint32_t startTime, uint32_t duration)
{
    roundTimeoutEvent = event;
    roundTimerStart = startTime;
    roundTimerDuration = duration;
    roundTimerRunning = 1;
}

static void checkTimerEvents(uint32_t now)
{
    uint32_t seconds;
    uint32_t ballDistance;

    if(roundTimerRunning != 0 && now - roundTimerStart >= roundTimerDuration)
    {
        ballDistance = calculateSharpDistanceCm(readBallSensor());
        if(roundTimeoutEvent == REPOSITION_TIMEOUT
           && now - roundTimerStart - roundTimerDuration > BALL_SAMPLE_MAX_GAP_US)
        {
            // A delayed check cannot establish the ball's position at the deadline.
            ballDistance = 0;
        }
        if(postEvent(roundTimeoutEvent, ballDistance) != 0)
        {
            roundTimerRunning = 0;
        }
    }
    if(gameTimerRunning != 0)
    {
        seconds = (now - previousAirTime) / 1000000;
        if(seconds != 0 && postEvent(AIR_DECAY_TIMEOUT, seconds) != 0)
        {
            previousAirTime = previousAirTime + seconds * 1000000;
        }
    }

    if(now - previousStatusTime >= STATUS_PRINT_INTERVAL_US)
    {
        if(postEvent(STATUS_TIMEOUT, 0) != 0)
        {
            previousStatusTime = now;
        }
    }
}

static void checkClockEvents(void)
{
    uint32_t start;
    uint32_t end;

    start = readClockStartSensor() == CLOCK_SENSOR_BLOCKED;
    end = readClockEndSensor() == CLOCK_SENSOR_BLOCKED;
    if(start != previousClockStart && postEvent(CLOCK_START_CHANGED, start) != 0)
    {
        previousClockStart = start;
    }
    if(end != previousClockEnd && postEvent(CLOCK_END_CHANGED, end) != 0)
    {
        previousClockEnd = end;
    }
}

static void checkStartButtonEvents(uint32_t now)
{
    uint32_t pressed;

    pressed = readStartButton(); // 1 = pressed, 0 = released.
    if(pressed != buttonCandidate)
    {
        buttonCandidate = pressed;
        buttonCandidateTime = now; // Every bounce restarts the stable-time check.
        return;
    }
    if(now - buttonCandidateTime < START_BUTTON_DEBOUNCE_US)
    {
        return; // Main keeps processing other events; no delay or waiting loop.
    }

    if(pressed == 0)
    {
        buttonArmed = 1;
    }
    else if(buttonArmed != 0)
    {
        if(postEvent(START_BUTTON_PRESSED, 0) != 0)
        {
            buttonArmed = 0; // Holding the button cannot post a second press.
        }
    }
}

static void checkHandEvents(void)
{
    uint32_t distance;
    enum EventType event;

    distance = calculateSharpDistanceCm(readHandSensor());
    if(distance == previousHandDistance)
    {
        return;
    }
    event = HAND_CHANGED;
    if(distance == 0)
    {
        event = HAND_REMOVED;
    }
    else if(previousHandDistance == 0)
    {
        event = HAND_PRESENT;
    }
    if(postEvent(event, distance) != 0)
    {
        previousHandDistance = distance;
    }
}

static void checkBallEvents(void)
{
    // A new timed sample is an event even if the measured distance is unchanged.
    postEvent(BALL_SAMPLE_READY, calculateSharpDistanceCm(readBallSensor()));
}

static void checkLeverEvents(void)
{
    uint32_t reading;
    enum LeverRegion region;

    reading = readLeverSensor();
    region = LEVER_MIDDLE;
    if(reading < PUMP_LOW_ADC)
    {
        region = LEVER_MINIMUM;
    }
    else if(reading >= PUMP_HIGH_ADC)
    {
        region = LEVER_MAXIMUM;
    }
    if(region == previousLeverRegion)
    {
        return;
    }

    if(region == LEVER_MINIMUM)
    {
        if(postEvent(LEVER_REACHED_MINIMUM, 0) == 0)
        {
            return;
        }
    }
    else if(region == LEVER_MAXIMUM)
    {
        if(postEvent(LEVER_REACHED_MAXIMUM, 0) == 0)
        {
            return;
        }
    }
    previousLeverRegion = region;
}

static void checkSerialEvents(void)
{
    char command;

    if(readSerialCharacter(&command) == 0)
    {
        return;
    }
    switch(command)
    {
        case 'h':
            if(readGameState() == WELCOME)
            {
                postEvent(START_BUTTON_PRESSED, 0); // Simulate an accepted button press.
            }
            break;
        case 'p':
            postEvent(PUMP_COMPLETE, 0);
            break;
        case 't':
            postEvent(GAME_TIMEOUT, 0);
            break;
        case 'x':
            postEvent(STOP_REQUESTED, 0);
            break;
        case '?':
            postEvent(HELP_REQUESTED, 0);
            break;
        default:
            break; // Ignore line endings and unrecognized characters.
    }
}

void checkEvents(void)
{
    uint32_t now;

    now = readTimer2();
    checkClockEvents();
    if(readGameState() == WELCOME)
    {
        checkStartButtonEvents(now); // Do not read or debounce the button during play.
    }
    if(now - previousSensorTime >= SENSOR_CHECK_INTERVAL_US)
    {
        previousSensorTime = now;
        checkHandEvents();
        checkBallEvents();
        checkLeverEvents();
    }
    checkTimerEvents(now);
    checkTachometerEvent();
    checkSerialEvents();
}
