#include "services.h"
#include "state_machine.h"
#include "game_config.h"
#include "motor.h"
#include "motor_controller.h"
#include "serial.h"
#include "targets.h"
#include "fan_config.h"
#include "sensors.h"
#include "stm32f446xx_timer_mort.h"

static struct GameStatus game;

const struct GameStatus * readGameStatus(void)
{
    return &game;
}

void initServices(void)
{
    game.handDistance = 0;
    game.ballDistance = 0;
    game.pumpCount = 0;
    game.airLevel = 0;
    game.elapsedSeconds = 0;
    game.fanDuty = 0;
    game.clockAtStart = 0;
    game.clockAtEnd = 1; // Wait for the first clock sensor event before accepting start.
    game.queueErrors = 0;
    game.fanRPM = 0;
    game.result = NOT_STARTED;
    writeMotorPower(0);
}

void startGameService(void)
{
    game.handDistance = calculateSharpDistanceCm(readHandSensor());
    game.pumpCount = 0;
    game.airLevel = AIR_AT_START;
    game.elapsedSeconds = 0;
    game.result = IN_PROGRESS;
    startTargetGame(readTimer2());
    game.ballDistance = calculateSharpDistanceCm(readBallSensor());
    startRoundTimer(REPOSITION_TIMEOUT, readTargetStatus()->repositionStartTime, TARGET_REPOSITION_US);
    startGameTimers();
    refreshSensorEvents();
    game.fanDuty = runMotorController(game.handDistance, game.airLevel);
}

void stopGameService(enum GameResult result)
{
    stopGameTimers();
    refreshSensorEvents();
    game.pumpCount = 0;
    game.airLevel = 0;
    game.result = result;
    turnTargetOff();
    game.fanDuty = 0;
    writeMotorPower(0);
}

void targetSetupRequiredService(void)
{
    game.result = TARGET_SETUP_REQUIRED;
}

void nextTargetService(void)
{
    selectNextTarget(readTimer2());
    startRoundTimer(REPOSITION_TIMEOUT, readTargetStatus()->repositionStartTime, TARGET_REPOSITION_US);
}

void beginHoldingService(uint32_t sampleTime)
{
    startTargetHolding(sampleTime);
    startRoundTimer(HOLDING_TIMEOUT, readTargetStatus()->holdingStartTime, TARGET_HOLD_US);
}

void pumpService(void)
{
    game.pumpCount = game.pumpCount + 1;
    if(game.pumpCount >= PUMPS_PER_AIR_LED)
    {
        game.pumpCount = 0;
        game.airLevel = game.airLevel + AIR_PER_LED;
        if(game.airLevel > AIR_MAXIMUM)
        {
            game.airLevel = AIR_MAXIMUM;
        }
        game.fanDuty = runMotorController(game.handDistance, game.airLevel);
    }
}

void handService(uint32_t handDistance)
{
    game.handDistance = handDistance;
    game.fanDuty = runMotorController(game.handDistance, game.airLevel);
}

void airDecayService(uint32_t seconds)
{
    uint32_t airUsed;

    game.elapsedSeconds = game.elapsedSeconds + seconds;
    airUsed = seconds * AIR_DRAIN_PER_SECOND;
    if(airUsed >= game.airLevel)
    {
        game.airLevel = 0;
    }
    else
    {
        game.airLevel = game.airLevel - airUsed;
    }
    game.fanDuty = runMotorController(game.handDistance, game.airLevel);
}

void printGameCommands(void)
{
    if(beginSerialMessage() == 0)
    {
        return;
    }
    writeSerialText("Air Levitation Tower\r\n");
    writeSerialText("Serial events: h=simulate start button, p=complete pump, t=timeout, x=stop, ?=help\r\n");
    writeSerialNumber(PUMPS_PER_AIR_LED);
    writeSerialText(" full pumps = +");
    writeSerialNumber(AIR_PER_LED);
    writeSerialText("% air. Rounds: ");
    writeSerialNumber(TARGET_ROUND_COUNT);
    writeSerialText(". Release and press START for a new game.\r\n");
    if(targetSettingsReady() == 0)
    {
        writeSerialText("Set all 3 target distances in game_config.h before playing.\r\n");
    }
}

static void printResult(void)
{
    switch(game.result)
    {
        case WON:
            writeSerialText("WON");
            break;
        case FAILED_REPOSITION:
            writeSerialText("FAILED: reposition");
            break;
        case FAILED_HOLD:
            writeSerialText("FAILED: hold");
            break;
        case FAILED_TIME:
            writeSerialText("FAILED: clock/timeout");
            break;
        case STOPPED:
            writeSerialText("STOPPED");
            break;
        case TARGET_SETUP_REQUIRED:
            writeSerialText("SET TARGET DISTANCES");
            break;
        case EVENT_QUEUE_ERROR:
            writeSerialText("QUEUE ERROR");
            break;
        default:
            writeSerialText("WELCOME");
            break;
    }
}

void printGameStatus(void)
{
    const struct TargetStatus * target;

    if(beginSerialMessage() == 0)
    {
        return; // Keep servicing the game if a previous message is still pending.
    }
    target = readTargetStatus();
    if(readGameState() == PLAYING)
    {
        writeSerialText("Round: ");
        writeSerialNumber(target->roundNumber);
        writeSerialText("/");
        writeSerialNumber(TARGET_ROUND_COUNT);
        if(readRoundState() == REPOSITIONING)
        {
            writeSerialText(" REPOSITION");
        }
        else
        {
            writeSerialText(" HOLD");
        }
        writeSerialText(" | targetLED_");
        writeSerialNumber(target->activeLED);
        writeSerialText(": ");
        writeSerialNumber(target->distance);
        writeSerialText(" cm | Ball: ");
        writeSerialNumber(game.ballDistance);
        writeSerialText(" cm | In range: ");
        writeSerialNumber(target->timeInRange / 1000);
        writeSerialText("/");
        writeSerialNumber(TARGET_REQUIRED_TIME_US / 1000);
        writeSerialText(" ms");
    }
    else
    {
        printResult();
    }
    writeSerialText(" | Hand: ");
    if(game.handDistance == 0)
    {
        writeSerialText("out of range");
    }
    else
    {
        writeSerialNumber(game.handDistance);
        writeSerialText(" cm");
    }
    writeSerialText(" | Pump: ");
    writeSerialNumber(game.pumpCount);
    writeSerialText("/");
    writeSerialNumber(PUMPS_PER_AIR_LED);
    writeSerialText(" | Time: ");
    writeSerialNumber(game.elapsedSeconds);
    writeSerialText(" s | Air: ");
    writeSerialNumber(game.airLevel);
    writeSerialText("% | LEDs: ");
    writeSerialNumber(game.airLevel / AIR_PER_LED);
    writeSerialText("/");
    writeSerialNumber(AIR_LED_COUNT);
    writeSerialText(" | RPM: ");
    if(FAN_TACH_CONNECTED != 0)
    {
        writeSerialNumber(game.fanRPM);
    }
    else
    {
        writeSerialText("not connected");
    }
    writeSerialText("\r\n");
}

void runService(struct Event event)
{
    switch(event.type)
    {
        case LEVER_REACHED_MINIMUM:
        case LEVER_REACHED_MAXIMUM:
            runPumpStateMachine(event);
            break;

        case HAND_PRESENT:
        case HAND_CHANGED:
        case HAND_REMOVED:
            game.handDistance = event.value;
            runGameStateMachine(event);
            break;

        case BALL_SAMPLE_READY:
        case REPOSITION_TIMEOUT:
        case HOLDING_TIMEOUT:
            game.ballDistance = event.value;
            runRoundStateMachine(event);
            break;

        case FAN_RPM_READY:
            game.fanRPM = event.value;
            break;

        case CLOCK_START_CHANGED:
            game.clockAtStart = event.value;
            break;

        case CLOCK_END_CHANGED:
            game.clockAtEnd = event.value;
            runGameStateMachine(event);
            break;

        case START_BUTTON_PRESSED:
        case ROUND_PASSED:
        case ROUND_FAILED:
        case PUMP_COMPLETE:
        case AIR_DECAY_TIMEOUT:
        case GAME_TIMEOUT:
        case STOP_REQUESTED:
            runGameStateMachine(event);
            break;

        case STATUS_TIMEOUT:
            printGameStatus();
            break;

        case HELP_REQUESTED:
            printGameCommands();
            break;

        case QUEUE_OVERFLOW:
            game.queueErrors = game.queueErrors + 1;
            runGameStateMachine(event);
            break;

        case NO_EVENT:
        default:
            break;
    }
}
