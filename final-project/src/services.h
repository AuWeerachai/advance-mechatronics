#ifndef SERVICES_H
#define SERVICES_H

#include "events.h"

enum GameResult
{
    NOT_STARTED,
    IN_PROGRESS,
    WON,
    FAILED_REPOSITION,
    FAILED_HOLD,
    FAILED_TIME,
    STOPPED,
    TARGET_SETUP_REQUIRED,
    EVENT_QUEUE_ERROR
};

struct GameStatus
{
    uint32_t handDistance;
    uint32_t ballDistance;
    uint32_t pumpCount;
    uint32_t airLevel;
    uint32_t elapsedSeconds;
    uint32_t fanDuty;
    uint32_t clockAtStart;
    uint32_t clockAtEnd;
    uint32_t queueErrors;
    uint32_t fanRPM;
    enum GameResult result;
};

void initServices(void);
void runService(struct Event event);
void startGameService(void);
void stopGameService(enum GameResult result);
void targetSetupRequiredService(void);
void nextTargetService(void);
void beginHoldingService(uint32_t sampleTime);
void pumpService(void);
void handService(uint32_t handDistance);
void airDecayService(uint32_t seconds);
void printGameStatus(void);
void printGameCommands(void);
const struct GameStatus * readGameStatus(void);

#endif
