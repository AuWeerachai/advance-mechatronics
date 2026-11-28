#define _GNU_SOURCE
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include "game_config.h"
#include "calibration.h"
#include "stm32f446xx_dma_controller_mort.h"

#define main firmware_main
#include "../src/main.c"
#undef main

#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
static uint32_t now, motorDuty, clockStart, clockEnd;
static volatile uint16_t * adc;

// Simulate peripheral inputs/time; use the real sensors, calibration and game code.
void initTimer2(void) {}
uint32_t readTimer2(void) { return now; }
void initUSART3(void) {}
void initMotor(void) { motorDuty = 0; }
void writeMotorPower(uint32_t duty) { assert(duty <= 1000); motorDuty = duty; }
void initTachometer(void) {}
void checkTachometerEvent(void) {}
void initGpioA3AsAnalog(void) {}
void initGpioC0AsAnalog(void) {}
void initGpioC3AsAnalog(void) {}
void initGpioF15AsInput(void) {}
void initGpioE13AsInput(void) {}
uint32_t readGpioF15(void) { return clockStart; }
uint32_t readGpioE13(void) { return clockEnd; }
void initDMA2Stream0(volatile uint16_t * destination, uint32_t count)
{
    assert(count == 3); adc = destination;
}
void initADC1(void) {}
void startADC1(void) {}

static void settle(void)
{
    for(uint32_t i = 0; i < 64; i++)
    {
        REG(0x40004800) = 0x80;
        checkEvents();
        serviceEventQueue();
        serviceSerialOutput();
    }
}

static void advance(uint32_t duration)
{
    while(duration > 0)
    {
        uint32_t step = duration < 1000 ? duration : 1000;
        now += step;
        duration -= step;
        settle();
    }
}

static uint16_t rawAt(uint32_t cm)
{
    // Pick a raw value the actual calibration maps to this whole-centimetre value.
    for(uint32_t raw = SHARP_ADC_MIN; raw <= SHARP_ADC_MAX; raw++)
        if(calculateSharpDistanceCm(raw) == cm) return raw;
    assert(0); return 0;
}

static void resetGameAt(uint32_t startTime)
{
    now = startTime; clockStart = 1; clockEnd = 0;
    REG(0x40004800) = 0x80;
    initProject();
    adc[0] = SHARP_ADC_30_CM;
    adc[1] = SHARP_ADC_45_CM;
    adc[2] = 0;
    settle();
    advance(100000);
    postEvent(START_BUTTON_PRESSED, 0); settle();
    assert(readGameState() == PLAYING);
    adc[0] = rawAt(readTargetStatus()->distance);
}

static void resetGame(void)
{
    resetGameAt(0);
}

static void enterHolding(void)
{
    uint32_t elapsed = now - readTargetStatus()->repositionStartTime;
    advance(TARGET_REPOSITION_US - elapsed);
    assert(readRoundState() == HOLDING);
}

int main(void)
{
    assert(mmap((void *)0x40004000, 0x2000, PROT_READ | PROT_WRITE,
                MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, -1, 0) != MAP_FAILED);
    resetGame();
    uint32_t goodDuty = motorDuty;
    adc[1] = 0; advance(10000);
    uint32_t badDuty = motorDuty;
    adc[1] = SHARP_ADC_45_CM; advance(10000);
    printf("HAND_DROPOUT: normal=%u, one bad sample=%u, recovered=%u (duty /1000)\n", goodDuty, badDuty, motorDuty);
    assert(goodDuty == 500 && badDuty == 0 && motorDuty == 500);

    resetGame();
    advance(TARGET_REPOSITION_US - 10000);
    adc[0] = 4095; advance(10000);
    assert(readGameStatus()->result == FAILED_REPOSITION);
    puts("BALL_DEADLINE_DROPOUT: one invalid sample at the reposition deadline fails the round.");

    resetGame(); enterHolding();
    uint16_t goodBall = adc[0];
    uint32_t holdStart = readTargetStatus()->holdingStartTime;
    for(uint32_t i = 0; i < TARGET_HOLD_US / 10000; i++)
    {
        adc[0] = i % 10 == 0 ? 4095 : goodBall;
        advance(10000);
    }
    assert(readTargetStatus()->roundsPassed == 1);
    printf("HOLD_10_PERCENT_DROPOUT: passed after %u ms; invalid time is not credited.\n", (now - holdStart) / 1000);

    resetGame();
    adc[1] = SHARP_ADC_MIN; advance(10000);
    uint32_t edgeDuty = motorDuty;
    adc[1] = SHARP_ADC_MIN - 1; advance(10000);
    printf("HAND_RANGE_EDGE: ADC %u -> %u duty; ADC %u -> %u duty.\n", SHARP_ADC_MIN, edgeDuty, SHARP_ADC_MIN - 1, motorDuty);
    assert(edgeDuty == 1000 && motorDuty == 0);

    // Exact percentage checks using real raw ADC values through the whole game.
    const uint32_t dropoutPercent[] = {0, 10, 20, 21, 30, 100};
    for(uint32_t test = 0; test < 6; test++)
    {
        resetGame(); enterHolding();
        goodBall = adc[0];
        uint32_t badPercent = dropoutPercent[test];
        for(uint32_t i = 0; i < TARGET_HOLD_US / 10000; i++)
        {
            adc[0] = i % 100 < badPercent ? 0 : goodBall;
            advance(10000);
        }
        uint32_t passed = readTargetStatus()->roundsPassed != 0;
        assert(passed == (badPercent <= 20));
        printf("HOLD_DROPOUT_%u_PERCENT: %s\n", badPercent, passed ? "PASS ROUND" : "FAIL ROUND");
    }

    // A raw value can be wrong while still lying inside the accepted ADC range.
    resetGame();
    adc[1] = SHARP_ADC_70_CM; advance(10000);
    printf("HAND_VALID_RANGE_SPIKE: one sample at 70 cm commands %u duty.\n", motorDuty);
    assert(motorDuty == 1000);

    // Noise near the raw acceptance boundary can repeatedly switch the command.
    resetGame();
    uint32_t changes = 0, lastDuty = motorDuty;
    for(uint32_t i = 0; i < 20; i++)
    {
        adc[1] = i % 2 == 0 ? SHARP_ADC_MIN : SHARP_ADC_MIN - 1;
        advance(10000);
        if(motorDuty != lastDuty) changes++;
        lastDuty = motorDuty;
    }
    printf("HAND_RANGE_EDGE_CHATTER: %u PWM changes in 200 ms.\n", changes);

    // Each target at its calibration point, with +/-15 raw counts of noise.
    resetGame();
    const int32_t noise[] = {0, 15, -15, 7, -7, 2, -2};
    const uint16_t targetRaw[] = {0, SHARP_ADC_30_CM, SHARP_ADC_45_CM, SHARP_ADC_60_CM};
    uint32_t noiseIndex = 0;
    while(readGameState() == PLAYING)
    {
        uint32_t currentLED = readTargetStatus()->activeLED;
        adc[0] = (uint16_t)(targetRaw[currentLED] + noise[noiseIndex % 7]);
        adc[1] = (uint16_t)(SHARP_ADC_45_CM + noise[noiseIndex % 7]);
        noiseIndex++;
        advance(10000);
        assert(now <= 41000000);
    }
    assert(readGameStatus()->result == WON);
    puts("SMALL_RAW_NOISE: all 5 rounds pass with +/-15 raw ADC counts around the configured calibration points.");

    // Software cannot distinguish a stuck valid value from a stationary ball.
    resetGame(); enterHolding(); advance(TARGET_HOLD_US);
    assert(readTargetStatus()->roundsPassed == 1);
    puts("STUCK_VALID_VALUE: still accepted as stationary; no freshness diagnostic exists for the sensor/DMA path.");

    // Simulate a scheduling gap spanning the deadline, then a late arrival.
    resetGame(); adc[0] = 0; advance(TARGET_REPOSITION_US - 20000);
    now += 100000; // No event checks during the simulated stall.
    adc[0] = rawAt(readTargetStatus()->distance); settle();
    printf("LATE_DEADLINE_CHECK: checked 80 ms late; holding_started=%u.\n", readRoundState() == HOLDING);
    assert(readGameStatus()->result == FAILED_REPOSITION);

    const uint32_t lateness[] = {0, BALL_SAMPLE_MAX_GAP_US, BALL_SAMPLE_MAX_GAP_US + 1, 80000};
    for(uint32_t wrap = 0; wrap < 2; wrap++)
    {
        for(uint32_t i = 0; i < 4; i++)
        {
            resetGameAt(wrap ? UINT32_MAX - 2000000U : 0);
            advance(TARGET_REPOSITION_US - 10000);
            now += 10000 + lateness[i]; settle();
            assert((readRoundState() == HOLDING) == (lateness[i] <= BALL_SAMPLE_MAX_GAP_US));
            if(lateness[i] > BALL_SAMPLE_MAX_GAP_US)
                assert(readGameStatus()->result == FAILED_REPOSITION);
        }
    }
    puts("DEADLINE_DELAY_LIMIT: 20 ms allowed; greater delays fail, including across timer wrap.");

    const uint32_t calibrationRaw[] = {SHARP_ADC_20_CM, SHARP_ADC_30_CM, SHARP_ADC_40_CM,
        SHARP_ADC_45_CM, SHARP_ADC_50_CM, SHARP_ADC_55_CM, SHARP_ADC_60_CM, SHARP_ADC_65_CM, SHARP_ADC_70_CM};
    const uint32_t calibrationCm[] = {20, 30, 40, 45, 50, 55, 60, 65, 70};
    puts("CALIBRATION_NOISE_SWEEP: +/-15 ADC counts, all integer readings equally represented.");
    for(uint32_t i = 0; i < 9; i++)
    {
        uint32_t minimum = 100, maximum = 0, invalid = 0;
        for(int32_t error = -15; error <= 15; error++)
        {
            uint32_t distance = calculateSharpDistanceCm((uint32_t)((int32_t)calibrationRaw[i] + error));
            if(distance == 0) invalid++;
            else
            {
                if(distance < minimum) minimum = distance;
                if(distance > maximum) maximum = distance;
            }
        }
        printf("  %u cm: valid output %u..%u cm; %u/31 invalid values.\n", calibrationCm[i], minimum, maximum, invalid);
    }
    printf("TOO_CLOSE_AMBIGUITY: the previously observed near-sensor raw value 1398 maps to %u cm.\n", calculateSharpDistanceCm(1398));

    resetGame();
    adc[2] = 0; advance(10000);
    adc[2] = 4095; advance(10000);
    adc[2] = 0; advance(10000);
    assert(readGameStatus()->pumpCount == 1);
    puts("LEVER_SPIKE: one full-scale spike followed by low counts as a stroke.");

    resetGame();
    clockEnd = 1; settle(); clockEnd = 0; settle();
    assert(readGameStatus()->result == FAILED_TIME);
    puts("CLOCK_END_GLITCH: one observed HIGH ends the game.");
}
