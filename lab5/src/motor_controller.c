#include "motor_controller.h"
#include "motor.h"
#include "encoder.h"
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_timer_mort.h"

//PART 3
#define CONTROL_INTERVAL_US ((uint32_t)10000)  // 10,000 us = 10 ms; position control runs about 100 times/second
#define POSITION_KP 20.0f                      // Each count of position error adds 20 to the signed duty command
#define POSITION_KD 0.4f                       // Subtract 0.4 * speed in counts/second to oppose motion
#define POSITION_TOLERANCE_COUNTS 1            // Within +/-1 encoder count of the target counts as close enough
#define SETTLED_SAMPLES 20                     // 20 checks * 10 ms = about 0.2 s near the target with zero measured speed

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

    button = readGpioC13();  // Read the onboard button
    if(button != previousButton)
    {
        previousButton = button;
        buttonSamples = 1;   // Start checking the new button state
    }
    else if(buttonSamples < 3)
    {
        buttonSamples = buttonSamples + 1;
    }

    if(buttonSamples == 3 && button != stableButton)  // Accept 3 matching 10 ms checks; reject bouncing and repeated events while held
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
    quarterTurnCounts = (int32_t)(ENCODER_COUNTS_PER_OUTPUT_REVOLUTION / 4.0f + 0.5f);  // 48 * 4.4 / 4 = 52.8; +0.5 then integer conversion rounds a quarter-turn to 53 counts
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
    elapsedTime = currentTime - previousTime;  // Find elapsed microseconds, including timer wraparound
    if(elapsedTime < CONTROL_INTERVAL_US)      // If less than 10 ms elapsed, return now; main calls again without blocking
    {
        return;
    }
    previousTime = currentTime;

    updateEncoderVelocity(elapsedTime);  // Update position and speed using the actual elapsed time
    position = readEncoderPosition();
    velocity = readEncoderVelocity();
    buttonPressed = checkButtonPress();
    error = targetPosition - position;   // Find how far the motor is from the target

    if(error >= -POSITION_TOLERANCE_COUNTS &&
       error <= POSITION_TOLERANCE_COUNTS && velocity == 0.0f)
    {
        if(settledSamples < SETTLED_SAMPLES)
        {
            settledSamples = settledSamples + 1;  // Count checks with no movement and small position error
        }
    }
    else
    {
        settledSamples = 0;
    }

    switch(state)
    {
        case REST:  // Hold target 0; a press selects the quarter-turn target
            if(buttonPressed == 1)
            {
                targetPosition = quarterTurnCounts;
                settledSamples = 0;
                state = MOVE_CLOCKWISE;
            }
            break;

        case MOVE_CLOCKWISE:  // Wait for the positive target to settle; motor/encoder wiring must agree on direction
            if(settledSamples == SETTLED_SAMPLES)
            {
                state = HOLD_POSITION;
            }
            break;

        case HOLD_POSITION:  // Keep applying feedback at the quarter-turn target; a press selects 0
            if(buttonPressed == 1)
            {
                targetPosition = 0;
                settledSamples = 0;
                state = MOVE_COUNTERCLOCKWISE;
            }
            break;

        case MOVE_COUNTERCLOCKWISE:  // Drive back to 0 and wait for it to settle before returning to REST
            if(settledSamples == SETTLED_SAMPLES)
            {
                state = REST;
            }
            break;
    }

    error = targetPosition - position;       // Find how far the motor is from the target
    power = POSITION_KP * (float)error;      // Drive toward the target based on position error
    power = power - POSITION_KD * velocity;  // Subtract signed speed to oppose motion; positive speed reduces the positive command

    if(power > 1000.0f)
    {
        power = 1000.0f;
    }
    else if(power < -1000.0f)
    {
        power = -1000.0f;
    }

    writeMotorPower((int32_t)power);  // Apply the direction and duty command
}
