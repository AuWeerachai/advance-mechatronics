#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include "targets.h"
#include "game_config.h"

static uint32_t handReading, leverReading;
uint32_t readHandSensor(void) { return handReading; }
uint32_t readLeverSensor(void) { return leverReading; }

static void prepare(uint32_t start)
{
    initTargets();
    startTargetGame(start);
    startTargetHolding(start + TARGET_REPOSITION_US);
}

// Spread in-range time across two separate visits, using timed sensor samples.
static void holdFor(uint32_t goodTime)
{
    uint32_t start = readTargetStatus()->holdingStartTime;
    uint32_t distance = readTargetStatus()->distance;
    uint32_t firstGood = goodTime / 2;
    uint32_t firstBad = (TARGET_HOLD_US - goodTime) / 2;
    for(uint32_t elapsed = 0; elapsed < TARGET_HOLD_US;)
    {
        uint32_t step = SENSOR_CHECK_INTERVAL_US;
        uint32_t reading = elapsed < firstGood ||
            (elapsed >= firstGood + firstBad && elapsed < goodTime + firstBad) ? distance : 0;
        // Stop the interval precisely at every transition and at the window end.
        uint32_t limits[] = {firstGood, firstGood + firstBad, goodTime + firstBad, TARGET_HOLD_US};
        for(unsigned int i = 0; i < 4; i++)
            if(limits[i] > elapsed && limits[i] - elapsed < step) step = limits[i] - elapsed;
        recordTargetSample(reading, start + elapsed);
        elapsed += step;
        recordTargetSample(reading, start + elapsed);
    }
}

int main(void)
{
    prepare(0);
    assert(targetSettingsReady());
    uint32_t d = readTargetStatus()->distance;
    assert(ballIsAtTarget(d) && ballIsAtTarget(d - 3) && ballIsAtTarget(d + 3));
    assert(!ballIsAtTarget(d - 4) && !ballIsAtTarget(d + 4) && !ballIsAtTarget(0));
    holdFor(TARGET_REQUIRED_TIME_US);
    assert(readTargetStatus()->timeInRange == TARGET_REQUIRED_TIME_US);
    assert(targetHoldingPassed());
    prepare(UINT32_MAX - TARGET_REPOSITION_US - 50000U);
    holdFor(TARGET_REQUIRED_TIME_US - 1);
    assert(readTargetStatus()->timeInRange == TARGET_REQUIRED_TIME_US - 1);
    assert(!targetHoldingPassed());
    puts("PASS: inclusive +/-3 cm, cumulative exact 80% passes, one microsecond below fails, counter wrap.");

    prepare(0);
    recordTargetSample(readTargetStatus()->distance, TARGET_REPOSITION_US + TARGET_HOLD_US);
    assert(readTargetStatus()->timeInRange == 0 && !targetHoldingPassed());
    prepare(0);
    holdFor(TARGET_HOLD_US);
    assert(readTargetStatus()->timeInRange == TARGET_HOLD_US);
    recordTargetSample(readTargetStatus()->distance, TARGET_REPOSITION_US + TARGET_HOLD_US + 10000);
    assert(readTargetStatus()->timeInRange == TARGET_HOLD_US);
    turnTargetOff(); assert(!ballIsAtTarget(d));
    puts("PASS: missing samples earn no hold time, credited time stops at the deadline, inactive targets cannot match.");
    const uint32_t distances[] = {0, TARGET_LED_1_DISTANCE_CM,
                                  TARGET_LED_2_DISTANCE_CM, TARGET_LED_3_DISTANCE_CM};
    uint32_t firstSeen[4] = {0};
    uint32_t nextSeen[4][4] = {{0}};
    handReading = 0; leverReading = 0;
    for(uint32_t firstTime = 0; firstTime < 3; firstTime++)
    {
        // Every new game can select any LED, even after another game was active.
        startTargetGame(firstTime);
        enum TargetLED first = readTargetStatus()->activeLED;
        assert(first >= targetLED_1 && first <= targetLED_3);
        firstSeen[first] = 1;
        assert(readTargetStatus()->roundNumber == 1 && readTargetStatus()->roundsPassed == 0);
        assert(readTargetStatus()->distance == distances[first]);

        // Try different input/timer values from each possible current LED.
        for(uint32_t value = 0; value < 16; value++)
        {
            handReading = 0; leverReading = 0;
            startTargetGame(firstTime);
            handReading = value;
            selectNextTarget(UINT32_MAX - value / 2);
            enum TargetLED next = readTargetStatus()->activeLED;
            assert(next >= targetLED_1 && next <= targetLED_3 && next != first);
            assert(readTargetStatus()->distance == distances[next]);
            nextSeen[first][next] = 1;
        }
        handReading = 0; leverReading = 0;
    }
    for(uint32_t current = 1; current <= 3; current++)
    {
        assert(firstSeen[current]);
        for(uint32_t next = 1; next <= 3; next++)
            assert(nextSeen[current][next] == (current != next));
    }

    // Unchanged inputs must still give a different target without a retry loop.
    for(uint32_t i = 0; i < 100; i++)
    {
        enum TargetLED previous = readTargetStatus()->activeLED;
        selectNextTarget(0);
        assert(readTargetStatus()->activeLED != previous);
    }
    puts("PASS: any first target, both alternatives from every LED, no consecutive repeats and correct target distance.");
}
