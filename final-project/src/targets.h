#ifndef TARGETS_H
#define TARGETS_H

#include <stdint.h>

enum TargetLED
{
    targetLED_OFF,
    targetLED_1,
    targetLED_2,
    targetLED_3
};

struct TargetStatus
{
    uint32_t roundNumber;
    uint32_t roundsPassed;
    enum TargetLED activeLED;
    uint32_t distance;
    uint32_t repositionStartTime;
    uint32_t holdingStartTime;
    uint32_t timeInRange;
};

void initTargets(void);
uint32_t targetSettingsReady(void);
void startTargetGame(uint32_t now);
void selectNextTarget(uint32_t now);
void startTargetHolding(uint32_t sampleTime);
void recordTargetSample(uint32_t distance, uint32_t sampleTime);
uint32_t ballIsAtTarget(uint32_t distance);
uint32_t targetHoldingPassed(void);
void recordTargetPass(void);
void turnTargetOff(void);
const struct TargetStatus * readTargetStatus(void);

#endif
