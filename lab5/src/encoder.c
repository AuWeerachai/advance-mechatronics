#include "encoder.h"
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_interruptcontroller_mort.h"

static volatile int32_t encoderCount;
static uint32_t previousPins;
static int32_t encoderPosition;
static int32_t previousPosition;
static float encoderVelocity;

//PART 3
void initEncoder(void)
{
    initGpioC6C7AsInput();
    encoderCount = 0;
    encoderPosition = 0;
    previousPosition = 0;
    encoderVelocity = 0.0f;
    previousPins = readGpioC6C7();
    initEncoderInterrupts();
}

//PART 3
void updateEncoderCount(uint32_t pins)
{
    if((previousPins == 0 && pins == 1) ||
       (previousPins == 1 && pins == 3) ||
       (previousPins == 3 && pins == 2) ||
       (previousPins == 2 && pins == 0))
    {
        encoderCount = encoderCount + 1;
    }
    else if((previousPins == 0 && pins == 2) ||
            (previousPins == 2 && pins == 3) ||
            (previousPins == 3 && pins == 1) ||
            (previousPins == 1 && pins == 0))
    {
        encoderCount = encoderCount - 1;
    }
    previousPins = pins;
}

//PART 3
void updateEncoderVelocity(uint32_t elapsedMicroseconds)
{
    int32_t changeInPosition;

    encoderPosition = encoderCount;
    changeInPosition = encoderPosition - previousPosition;
    encoderVelocity = (float)changeInPosition * 1000000.0f / (float)elapsedMicroseconds;
    previousPosition = encoderPosition;
}

//PART 3
int32_t readEncoderPosition(void)
{
    return encoderPosition;
}

//PART 3
float readEncoderVelocity(void)
{
    return encoderVelocity;
}
