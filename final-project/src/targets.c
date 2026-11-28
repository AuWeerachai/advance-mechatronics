#include "targets.h"
#include "game_config.h"
#include "calibration.h"
#include "sensors.h"

static const uint32_t targetDistances[TARGET_LED_COUNT] =
{
    TARGET_LED_1_DISTANCE_CM,
    TARGET_LED_2_DISTANCE_CM,
    TARGET_LED_3_DISTANCE_CM
};

static struct TargetStatus target;
static uint32_t previousSampleTime;
static uint32_t previousSampleInRange;

void initTargets(void)
{
    target.roundNumber = 0;
    target.roundsPassed = 0;
    target.activeLED = targetLED_OFF;
    target.distance = 0;
    target.repositionStartTime = 0;
    target.holdingStartTime = 0;
    target.timeInRange = 0;
    previousSampleTime = 0;
    previousSampleInRange = 0;
}

uint32_t targetSettingsReady(void)
{
    uint32_t i;

    for(i = 0; i < TARGET_LED_COUNT; i++)
    {
        if(targetDistances[i] < SHARP_MIN_DISTANCE_CM || targetDistances[i] > SHARP_MAX_DISTANCE_CM)
        {
            return 0;
        }
    }
    return 1;
}

const struct TargetStatus * readTargetStatus(void)
{
    return &target;
}

void selectNextTarget(uint32_t now)
{
    uint32_t randomValue;
    uint32_t index;

    randomValue = now + readHandSensor() + readLeverSensor();
    if(target.activeLED == targetLED_OFF)
    {
        // First round: choose from all three LEDs.
        index = randomValue % TARGET_LED_COUNT;
    }
    else
    {
        // Choose between the two remaining LEDs, skipping the current LED's index.
        index = randomValue % (TARGET_LED_COUNT - 1);
        if(index >= (uint32_t)target.activeLED - 1)
        {
            index = index + 1;
        }
    }
    target.activeLED = (enum TargetLED)(index + 1);
    target.distance = targetDistances[index];
    target.roundNumber = target.roundNumber + 1;
    target.repositionStartTime = now;
    target.timeInRange = 0;
    previousSampleInRange = 0;
}

void startTargetGame(uint32_t now)
{
    target.roundNumber = 0;
    target.roundsPassed = 0;
    target.activeLED = targetLED_OFF;
    selectNextTarget(now);
}

uint32_t ballIsAtTarget(uint32_t distance)
{
    uint32_t error;

    if(distance == 0 || target.activeLED == targetLED_OFF)
    {
        return 0; // Invalid sensor readings cannot satisfy a target.
    }
    if(distance >= target.distance)
    {
        error = distance - target.distance;
    }
    else
    {
        error = target.distance - distance;
    }
    return error <= TARGET_TOLERANCE_CM;
}

void startTargetHolding(uint32_t sampleTime)
{
    target.holdingStartTime = target.repositionStartTime + TARGET_REPOSITION_US;
    target.timeInRange = 0;
    previousSampleTime = sampleTime;
    previousSampleInRange = 1; // The reposition deadline has just passed its range check.
}

void recordTargetSample(uint32_t distance, uint32_t sampleTime)
{
    uint32_t previousElapsed;
    uint32_t currentElapsed;
    uint32_t sampleGap;

    previousElapsed = previousSampleTime - target.holdingStartTime;
    currentElapsed = sampleTime - target.holdingStartTime;
    sampleGap = sampleTime - previousSampleTime;
    if(currentElapsed > TARGET_HOLD_US)
    {
        currentElapsed = TARGET_HOLD_US;
    }

    // The previous sample describes the interval until this sample.
    // Do not award unobserved time if sampling was delayed or stopped.
    if(previousSampleInRange != 0 && sampleGap <= BALL_SAMPLE_MAX_GAP_US
       && previousElapsed < TARGET_HOLD_US && currentElapsed >= previousElapsed)
    {
        target.timeInRange = target.timeInRange + currentElapsed - previousElapsed;
    }
    previousSampleTime = sampleTime;
    previousSampleInRange = ballIsAtTarget(distance);
}

uint32_t targetHoldingPassed(void)
{
    return target.timeInRange >= TARGET_REQUIRED_TIME_US;
}

void recordTargetPass(void)
{
    target.roundsPassed = target.roundsPassed + 1;
}

void turnTargetOff(void)
{
    target.activeLED = targetLED_OFF;
}
