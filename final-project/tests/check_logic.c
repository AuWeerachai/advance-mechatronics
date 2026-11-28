#define _GNU_SOURCE
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include "game_config.h"
#include "calibration.h"

#define main firmware_main
#include "../src/main.c"
#undef main

#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define UART_SR REG(0x40004800)
#define UART_DR REG(0x40004804)

static uint32_t testTime, handReading, ballReading, leverReading;
static uint32_t startReading, endReading, motorCommand, initialization;
static uint32_t roundStart, txReady, buttonReading, buttonReadCount;
static char output[65536];
static size_t outputLength;

void initTimer2(void) { assert(initialization == 0); initialization = 1; }
void initUSART3(void) { assert(initialization == 1); initialization = 2; }
void initMotor(void) { assert(initialization == 2); initialization = 3; motorCommand = 0; }
void initTachometer(void) {}
void checkTachometerEvent(void) {}
void initSensors(void) { assert(initialization == 3); initialization = 4; }
uint32_t readTimer2(void) { return testTime; }
uint32_t readStartButton(void)
{
    assert(readGameState() == WELCOME);
    buttonReadCount++;
    return buttonReading;
}
uint32_t readHandSensor(void) { return handReading; }
uint32_t readBallSensor(void) { return ballReading; }
uint32_t readLeverSensor(void) { return leverReading; }
uint32_t readClockStartSensor(void) { return startReading; }
uint32_t readClockEndSensor(void) { return endReading; }
// Test game behavior in cm; check_registers.c checks the actual ADC conversion.
uint32_t calculateSharpDistanceCm(uint32_t raw) { return raw >= 20 && raw <= 70 ? raw : 0; }
void writeMotorPower(uint32_t duty) { assert(duty <= 1000); motorCommand = duty; }

static void iteration(void)
{
    checkEvents();
    UART_SR = txReady ? 0x80 : 0; // Emulate RXNE clearing after a received byte is read.
    serviceEventQueue();
    UART_DR = 0xFFFFFFFF;
    serviceSerialOutput();
    if(UART_DR != 0xFFFFFFFF)
    {
        assert(txReady && outputLength + 1 < sizeof(output));
        output[outputLength++] = (char)UART_DR;
        output[outputLength] = '\0';
    }
}

static void settle(void)
{
    for(unsigned int i = 0; i < 512; i++) iteration();
}

static void advance(uint32_t microseconds)
{
    testTime += microseconds;
    settle();
}

static void reset(uint32_t time)
{
    testTime = time;
    handReading = 0;
    buttonReading = 0;
    buttonReadCount = 0;
    ballReading = 40;
    leverReading = 0;
    startReading = 1;
    endReading = 0;
    motorCommand = 123;
    initialization = 0;
    outputLength = 0;
    output[0] = '\0';
    UART_SR = 0x80;
    txReady = 1;
    initProject();
    assert(initialization == 4);
    settle();
    assert(strstr(output, "Serial events:"));
    assert(readGameState() == WELCOME && motorCommand == 0);
    assert(readGameStatus()->airLevel == 0);
    assert(readGameStatus()->clockAtStart == 1 && readGameStatus()->clockAtEnd == 0);
    outputLength = 0;
    output[0] = '\0';
}

static void startGame(uint32_t distance)
{
    handReading = distance;
    buttonReading = 0;
    settle();
    advance(START_BUTTON_DEBOUNCE_US);
    assert(readGameState() == WELCOME); // A hand does not start the game.
    buttonReading = 1;
    settle();
    advance(START_BUTTON_DEBOUNCE_US - 1);
    assert(readGameState() == WELCOME);
    advance(1);
    assert(readGameState() == PLAYING);
    assert(readGameStatus()->airLevel == AIR_AT_START);
    roundStart = testTime;
}

static void moveLever(uint32_t raw)
{
    leverReading = raw;
    advance(SENSOR_CHECK_INTERVAL_US);
}

static void pump(void)
{
    moveLever(0);
    moveLever(PUMP_HIGH_ADC);
    moveLever(0);
}

static void command(char c)
{
    UART_DR = c;
    UART_SR = 0x20 | (txReady ? 0x80 : 0);
    iteration();
    settle();
}

static void runFor(uint32_t duration)
{
    while(duration != 0)
    {
        uint32_t step = duration < SENSOR_CHECK_INTERVAL_US ? duration : SENSOR_CHECK_INTERVAL_US;
        advance(step);
        duration -= step;
    }
}

static void enterHolding(void)
{
    assert(readRoundState() == REPOSITIONING);
    ballReading = readTargetStatus()->distance;
    uint32_t elapsed = testTime - readTargetStatus()->repositionStartTime;
    runFor(TARGET_REPOSITION_US - elapsed - 1);
    assert(readRoundState() == REPOSITIONING);
    advance(1);
    assert(readRoundState() == HOLDING);
    assert(readTargetStatus()->holdingStartTime == readTargetStatus()->repositionStartTime + TARGET_REPOSITION_US);
}

int main(void)
{
    assert(mmap((void *)0x40004000, 0x2000, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0) != MAP_FAILED);
    reset(0);
    if(targetSettingsReady() == 0)
    {
        command('h');
        assert(readGameState() == WELCOME && motorCommand == 0);
        assert(readGameStatus()->result == TARGET_SETUP_REQUIRED);
        advance(STATUS_PRINT_INTERVAL_US);
        assert(strstr(output, "SET TARGET DISTANCES"));
        puts("PASS: production defaults refuse to play with unmeasured target heights.");
        return 0;
    }

    // Stable hand input never starts a game. Button bounce restarts its timer.
    handReading = 45; advance(200000);
    assert(readGameState() == WELCOME);
    buttonReading = 1; settle(); advance(START_BUTTON_DEBOUNCE_US / 2);
    buttonReading = 0; settle(); advance(START_BUTTON_DEBOUNCE_US / 2);
    buttonReading = 1; settle(); advance(START_BUTTON_DEBOUNCE_US - 1);
    assert(readGameState() == WELCOME);
    advance(1);
    assert(readGameState() == PLAYING);
    uint32_t readsAtStart = buttonReadCount;
    buttonReading = 0; advance(10000);
    buttonReading = 1; advance(10000);
    assert(buttonReadCount == readsAtStart); // No input reads, not just ignored events.
    postEvent(START_BUTTON_PRESSED, 0); settle();
    assert(readGameState() == PLAYING && readTargetStatus()->roundNumber == 1);
    command('x');
    advance(START_BUTTON_DEBOUNCE_US * 2);
    assert(readGameState() == WELCOME); // Button held across game-over cannot restart.
    buttonReading = 0; settle(); advance(START_BUTTON_DEBOUNCE_US / 2);
    buttonReading = 1; settle(); advance(START_BUTTON_DEBOUNCE_US * 2);
    assert(readGameState() == WELCOME); // Short release bounce does not rearm.
    buttonReading = 0; settle(); advance(START_BUTTON_DEBOUNCE_US);
    handReading = 0;
    buttonReading = 1; settle(); advance(START_BUTTON_DEBOUNCE_US);
    assert(readGameState() == PLAYING && motorCommand == 0); // Start needs no hand.
    puts("PASS: button debounce, no hand auto-start, no reads during play, held-button and release-bounce rejection.");

    // Held at power-up: require release and a fresh press, also across time wrap.
    reset(UINT32_MAX - 1000U);
    buttonReading = 1; settle(); advance(START_BUTTON_DEBOUNCE_US * 2);
    assert(readGameState() == WELCOME);
    startGame(70);
    assert(readGameState() == PLAYING);
    puts("PASS: button held at startup, Timer 2 wrap and explicit release before starting.");

    // Repositioning still uses the full period: an early visit cannot pass.
    reset(0);
    handReading = 45; advance(10000);
    handReading = 0; advance(START_BUTTON_DEBOUNCE_US);
    assert(readGameState() == WELCOME);
    startGame(70);
    ballReading = readTargetStatus()->distance;
    runFor(TARGET_REPOSITION_US - SENSOR_CHECK_INTERVAL_US);
    assert(readRoundState() == REPOSITIONING);
    ballReading = 0;
    advance(SENSOR_CHECK_INTERVAL_US);
    assert(readGameState() == WELCOME && motorCommand == 0);
    assert(readGameStatus()->result == FAILED_REPOSITION);
    runFor(200000); assert(readGameState() == WELCOME); // Held button cannot restart.
    puts("PASS: button start, full reposition period and immediate failure outside range at its deadline.");

    reset(0); startGame(45); enterHolding();
    ballReading = 0; advance(SENSOR_CHECK_INTERVAL_US);
    runFor(TARGET_HOLD_US - SENSOR_CHECK_INTERVAL_US - 1);
    assert(readRoundState() == HOLDING);
    advance(1);
    assert(readGameState() == WELCOME && readGameStatus()->result == FAILED_HOLD && motorCommand == 0);
    puts("PASS: holding uses its entire period and insufficient in-range time fails.");

    // Exactly 80% in two visits must pass through the real event/framework path.
    reset(0); startGame(45); enterHolding();
    uint32_t goodPart = TARGET_REQUIRED_TIME_US / 2;
    uint32_t badPart = (TARGET_HOLD_US - TARGET_REQUIRED_TIME_US) / 2;
    runFor(goodPart);
    ballReading = 0; postEvent(BALL_SAMPLE_READY, 0); settle();
    runFor(badPart);
    ballReading = readTargetStatus()->distance; postEvent(BALL_SAMPLE_READY, ballReading); settle();
    runFor(TARGET_REQUIRED_TIME_US - goodPart);
    ballReading = 0; postEvent(BALL_SAMPLE_READY, 0); settle();
    runFor(TARGET_HOLD_US - TARGET_REQUIRED_TIME_US - badPart);
    assert(readTargetStatus()->roundsPassed == 1 && readGameState() == PLAYING);
    assert(readRoundState() == REPOSITIONING);
    puts("PASS: two separate in-range visits totaling exactly 80% advance to the next round.");

    // Complete all configured rounds with continuous fresh samples, including timer wrap.
    reset(UINT32_MAX - 200000U); startGame(70);
    enum TargetLED previousTarget = targetLED_OFF;
    for(uint32_t round = 1; round <= TARGET_ROUND_COUNT; round++)
    {
        assert(readTargetStatus()->roundNumber == round);
        assert(readTargetStatus()->activeLED >= targetLED_1 && readTargetStatus()->activeLED <= targetLED_3);
        assert(readTargetStatus()->activeLED != previousTarget);
        previousTarget = readTargetStatus()->activeLED;
        enterHolding();
        runFor(TARGET_HOLD_US - 1);
        assert(readRoundState() == HOLDING);
        advance(1);
        assert(readTargetStatus()->roundsPassed == round);
        if(round != TARGET_ROUND_COUNT) assert(readRoundState() == REPOSITIONING);
    }
    assert(readGameState() == WELCOME && readGameStatus()->result == WON);
    assert(readTargetStatus()->activeLED == targetLED_OFF && motorCommand == 0);
    puts("PASS: all configured rounds, no consecutive target repeats, full holding periods, win and Timer 2 wraparound.");

    // Pump edges and configurable reward: mid-stroke or a held maximum cannot count.
    reset(0); startGame(70);
    postEvent(AIR_DECAY_TIMEOUT, 10); settle();
    assert(readGameStatus()->airLevel == 70 && motorCommand == 700);
    moveLever(15); moveLever(169); moveLever(14);
    assert(readGameStatus()->pumpCount == 0);
    moveLever(170); moveLever(170); moveLever(80); moveLever(170); moveLever(15);
    assert(readGameStatus()->pumpCount == 0);
    moveLever(14);
    assert(readGameStatus()->pumpCount == 1);
    for(uint32_t i = 0; i < 5; i++) moveLever(0);
    assert(readGameStatus()->pumpCount == 1);
    for(uint32_t i = 1; i < PUMPS_PER_AIR_LED; i++) pump();
    assert(readGameStatus()->pumpCount == 0 && readGameStatus()->airLevel == 80 && motorCommand == 800);
    for(uint32_t i = 0; i < 4 * PUMPS_PER_AIR_LED; i++) postEvent(PUMP_COMPLETE, 0);
    settle();
    assert(readGameStatus()->airLevel == 100);
    puts("PASS: min-max-min pumping, no duplicate/partial strokes, configurable pump reward and air cap.");

    reset(0); startGame(70);
    runFor(999999); assert(readGameStatus()->airLevel == 100);
    runFor(1); assert(readGameStatus()->airLevel == 97 && motorCommand == 970);
    postEvent(AIR_DECAY_TIMEOUT, 34); settle();
    assert(readGameStatus()->airLevel == 0 && motorCommand == 0);
    for(uint32_t i = 0; i < PUMPS_PER_AIR_LED; i++) pump();
    assert(readGameStatus()->airLevel == 10 && motorCommand == 100);
    handReading = 0; advance(10000); assert(motorCommand == 0);
    handReading = 70; advance(10000); advance(START_BUTTON_DEBOUNCE_US);
    assert(readGameState() == PLAYING && motorCommand == 100);
    assert(readTargetStatus()->repositionStartTime == roundStart); // Hand return does not restart round.
    puts("PASS: time-based air depletion, empty-air recovery and hand/air PWM limits during play.");

    reset(0); startGame(45);
    endReading = 1; settle();
    assert(readGameState() == WELCOME && motorCommand == 0 && readGameStatus()->result == FAILED_TIME);
    command('h'); assert(readGameState() == WELCOME);
    endReading = 0; settle();
    command('h'); assert(readGameState() == PLAYING);
    command('p'); assert(readGameStatus()->pumpCount == 1);
    command('x'); assert(readGameState() == WELCOME && readGameStatus()->result == STOPPED);
    command('h'); command('t'); assert(readGameStatus()->result == FAILED_TIME);
    puts("PASS: physical END interlock and serial start, pump, stop and timeout events.");

    reset(0); startGame(45); txReady = 0; UART_SR = 0;
    ballReading = 0; runFor(TARGET_REPOSITION_US);
    assert(readGameState() == WELCOME && motorCommand == 0);
    txReady = 1; settle();
    reset(0); startGame(45);
    for(uint32_t i = 0; i < EVENT_QUEUE_SIZE; i++) assert(postEvent(PUMP_COMPLETE, 0));
    assert(postEvent(PUMP_COMPLETE, 0) == 0); serviceEventQueue();
    assert(readGameState() == WELCOME && motorCommand == 0 && readGameStatus()->queueErrors == 1);
    puts("PASS: UART backpressure cannot block deadlines; event queue overflow stops play.");
}
