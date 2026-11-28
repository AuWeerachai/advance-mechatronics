#include "state_machine.h"
#include "framework.h"
#include "services.h"
#include "targets.h"
#include "game_config.h"

static enum GameState gameState;
static enum PumpState pumpState;
static enum RoundState roundState;

void resetPumpStateMachine(void)
{
    pumpState = PUMP_WAIT_FOR_MINIMUM;
    resetLeverEventChecker();
}

void initStateMachines(void)
{
    gameState = WELCOME;
    roundState = ROUND_INACTIVE;
    resetPumpStateMachine();
}

enum GameState readGameState(void)
{
    return gameState;
}
enum PumpState readPumpState(void)
{
    return pumpState;
}
enum RoundState readRoundState(void)
{
    return roundState;
}

static void finishGame(enum GameResult result)
{
    clearEventQueue();
    resetPumpStateMachine();
    roundState = ROUND_INACTIVE;
    stopGameService(result);
    gameState = WELCOME;
    resetStartButtonEvents();
}

void runGameStateMachine(struct Event event)
{
    switch(gameState)
    {
        case WELCOME:
            switch(event.type)
            {
                case START_BUTTON_PRESSED:
                    if(targetSettingsReady() == 0)
                    {
                        targetSetupRequiredService();
                    }
                    else if(readGameStatus()->clockAtEnd == 0)
                    {
                        clearEventQueue();
                        resetPumpStateMachine();
                        startGameService();
                        gameState = PLAYING;
                        roundState = REPOSITIONING;
                    }
                    break;
                case STOP_REQUESTED:
                    finishGame(STOPPED);
                    break;
                case QUEUE_OVERFLOW:
                    finishGame(EVENT_QUEUE_ERROR);
                    break;
                default:
                    break; // Ignore play events while waiting for a button press.
            }
            break;

        case PLAYING:
            switch(event.type)
            {
                case GAME_TIMEOUT:
                    finishGame(FAILED_TIME);
                    break;
                case STOP_REQUESTED:
                    finishGame(STOPPED);
                    break;
                case QUEUE_OVERFLOW:
                    finishGame(EVENT_QUEUE_ERROR);
                    break;
                case CLOCK_END_CHANGED:
                    if(event.value != 0)
                    {
                        finishGame(FAILED_TIME);
                    }
                    break;
                case ROUND_FAILED:
                    finishGame((enum GameResult)event.value);
                    break;
                case ROUND_PASSED:
                    recordTargetPass();
                    if(readTargetStatus()->roundsPassed >= TARGET_ROUND_COUNT)
                    {
                        finishGame(WON);
                    }
                    else
                    {
                        nextTargetService();
                        roundState = REPOSITIONING;
                    }
                    break;
                case PUMP_COMPLETE:
                    pumpService();
                    break;
                case HAND_PRESENT:
                case HAND_CHANGED:
                    handService(event.value);
                    break;
                case HAND_REMOVED:
                    handService(0);
                    break;
                case AIR_DECAY_TIMEOUT:
                    airDecayService(event.value);
                    break;
                default:
                    break; // Other events belong to their services or the round machine.
            }
            break;
        default:
            finishGame(EVENT_QUEUE_ERROR);
            break;
    }
}

void runRoundStateMachine(struct Event event)
{
    if(gameState != PLAYING)
    {
        return;
    }
    switch(roundState)
    {
        case REPOSITIONING:
            // Entering the range early does not shorten the reposition period.
            if(event.type == REPOSITION_TIMEOUT)
            {
                if(ballIsAtTarget(event.value) != 0)
                {
                    beginHoldingService(event.time);
                    roundState = HOLDING;
                }
                else
                {
                    roundState = ROUND_INACTIVE;
                    postEvent(ROUND_FAILED, FAILED_REPOSITION);
                }
            }
            break;
        case HOLDING:
            if(event.type == BALL_SAMPLE_READY)
            {
                recordTargetSample(event.value, event.time);
            }
            else if(event.type == HOLDING_TIMEOUT)
            {
                recordTargetSample(event.value, event.time);
                roundState = ROUND_INACTIVE;
                if(targetHoldingPassed() != 0)
                {
                    postEvent(ROUND_PASSED, 0);
                }
                else
                {
                    postEvent(ROUND_FAILED, FAILED_HOLD);
                }
            }
            break;
        case ROUND_INACTIVE:
        default:
            break; // Await the game machine's next-round or end-game action.
    }
}

void runPumpStateMachine(struct Event event)
{
    if(gameState != PLAYING)
    {
        return;
    }
    switch(pumpState)
    {
        case PUMP_WAIT_FOR_MINIMUM:
            if(event.type == LEVER_REACHED_MINIMUM)
            {
                pumpState = PUMP_WAIT_FOR_MAXIMUM;
            }
            break;
        case PUMP_WAIT_FOR_MAXIMUM:
            if(event.type == LEVER_REACHED_MAXIMUM)
            {
                pumpState = PUMP_WAIT_FOR_RETURN;
            }
            break;
        case PUMP_WAIT_FOR_RETURN:
            if(event.type == LEVER_REACHED_MINIMUM)
            {
                postEvent(PUMP_COMPLETE, 0);
                pumpState = PUMP_WAIT_FOR_MAXIMUM;
            }
            break;
        default:
            resetPumpStateMachine();
            break;
    }
    // Events not accepted by the current pump state are ignored.
}
