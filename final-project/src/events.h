#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>

enum EventType
{
    NO_EVENT,
    START_BUTTON_PRESSED,
    HAND_PRESENT,
    HAND_CHANGED,
    HAND_REMOVED,
    BALL_SAMPLE_READY,
    LEVER_REACHED_MINIMUM,
    LEVER_REACHED_MAXIMUM,
    PUMP_COMPLETE,
    CLOCK_START_CHANGED,
    CLOCK_END_CHANGED,
    AIR_DECAY_TIMEOUT,
    GAME_TIMEOUT,
    REPOSITION_TIMEOUT,
    HOLDING_TIMEOUT,
    ROUND_PASSED,
    ROUND_FAILED,
    FAN_RPM_READY,
    STATUS_TIMEOUT,
    STOP_REQUESTED,
    HELP_REQUESTED,
    QUEUE_OVERFLOW
};

struct Event
{
    enum EventType type;
    uint32_t value; // Distance in cm, sensor level, or elapsed seconds, depending on type.
    uint32_t time;  // Timer 2 microseconds when the event was posted.
};

void initEvents(void);
void resetStartButtonEvents(void);
void checkEvents(void);
void startGameTimers(void);
void stopGameTimers(void);
void resetLeverEventChecker(void);
void refreshSensorEvents(void);
void startRoundTimer(enum EventType event, uint32_t startTime, uint32_t duration);

#endif
