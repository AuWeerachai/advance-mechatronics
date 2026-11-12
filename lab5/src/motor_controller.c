#include "motor_controller.h"
#include "motor.h"
#include "encoder.h"
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_timer_mort.h"

//PART 3
#define CONTROL_INTERVAL_US ((uint32_t)10000)
#define POSITION_KP 20.0f
#define POSITION_KD 0.4f
#define POSITION_TOLERANCE_COUNTS 1
#define SETTLED_SAMPLES 20

enum MotorState
{
    REST,
    MOVE_CLOCKWISE,
    HOLD_POSITION,
    MOVE_COUNTERCLOCKWISE
};

static enum MotorState state;

static uint32_t previousTime;
static uint32_t previousButton;
static uint32_t stableButton;
static uint32_t buttonSamples;
static uint32_t settledSamples;
static int32_t quarterTurnCounts;
static int32_t targetPosition;

//PART 3
static uint32_t checkButtonPress(void)
{
    uint32_t button;

    button = readGpioC13();
    if(button != previousButton)
    {
        previousButton = button;
        buttonSamples = 1;
    }
    else if(buttonSamples < 3)
    {
        buttonSamples = buttonSamples + 1;
    }

    if(buttonSamples == 3 && button != stableButton)
    {
        stableButton = button;
        if(stableButton == 1)
        {
            return 1;
        }
    }
    return 0;
}

//PART 3
void initMotorController(void)
{
    initMotor();
    initEncoder();
    initGpioC13AsInput();

    state = REST;
    targetPosition = 0;
    quarterTurnCounts = (int32_t)(ENCODER_COUNTS_PER_OUTPUT_REVOLUTION / 4.0f + 0.5f);
    previousButton = readGpioC13();
    stableButton = previousButton;
    buttonSamples = 0;
    settledSamples = 0;
    previousTime = readTimer2();
}

//PART 3
void runMotorController(void)
{
    uint32_t currentTime;
    uint32_t elapsedTime;
    uint32_t buttonPressed;
    int32_t position;
    int32_t error;
    float velocity;
    float power;

    currentTime = readTimer2();
    elapsedTime = currentTime - previousTime;
    if(elapsedTime < CONTROL_INTERVAL_US)
    {
        return;
    }
    previousTime = currentTime;

    updateEncoderVelocity(elapsedTime);
    position = readEncoderPosition();
    velocity = readEncoderVelocity();
    buttonPressed = checkButtonPress();
    error = targetPosition - position;

    if(error >= -POSITION_TOLERANCE_COUNTS &&
       error <= POSITION_TOLERANCE_COUNTS && velocity == 0.0f)
    {
        if(settledSamples < SETTLED_SAMPLES)
        {
            settledSamples = settledSamples + 1;
        }
    }
    else
    {
        settledSamples = 0;
    }

    switch(state)
    {
        case REST:
            if(buttonPressed == 1)
            {
                targetPosition = quarterTurnCounts;
                settledSamples = 0;
                state = MOVE_CLOCKWISE;
            }
            break;

        case MOVE_CLOCKWISE:
            if(settledSamples == SETTLED_SAMPLES)
            {
                state = HOLD_POSITION;
            }
            break;

        case HOLD_POSITION:
            if(buttonPressed == 1)
            {
                targetPosition = 0;
                settledSamples = 0;
                state = MOVE_COUNTERCLOCKWISE;
            }
            break;

        case MOVE_COUNTERCLOCKWISE:
            if(settledSamples == SETTLED_SAMPLES)
            {
                state = REST;
            }
            break;
    }

    error = targetPosition - position;
    power = POSITION_KP * (float)error;
    power = power - POSITION_KD * velocity;

    if(power > 1000.0f)
    {
        power = 1000.0f;
    }
    else if(power < -1000.0f)
    {
        power = -1000.0f;
    }

    writeMotorPower((int32_t)power);
}
