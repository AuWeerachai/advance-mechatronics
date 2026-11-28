#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "events.h"

enum GameState
{
    WELCOME,
    PLAYING
};

// Pump progress is separate from the two main game states.
enum PumpState
{
    PUMP_WAIT_FOR_MINIMUM,
    PUMP_WAIT_FOR_MAXIMUM,
    PUMP_WAIT_FOR_RETURN
};

enum RoundState
{
    ROUND_INACTIVE,
    REPOSITIONING,
    HOLDING
};

void initStateMachines(void);
void resetPumpStateMachine(void);
void runGameStateMachine(struct Event event);
void runPumpStateMachine(struct Event event);
void runRoundStateMachine(struct Event event);
enum GameState readGameState(void);
enum PumpState readPumpState(void);
enum RoundState readRoundState(void);

#endif
