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
    initGpioC6C7AsInput();          // Set up both encoder input pins
    encoderCount = 0;
    encoderPosition = 0;
    previousPosition = 0;
    encoderVelocity = 0.0f;
    previousPins = readGpioC6C7();  // Save the initial encoder pin state
    initEncoderInterrupts();        // Count changes on either encoder input
}

//PART 3
void updateEncoderCount(uint32_t pins)
{
    if((previousPins == 0 && pins == 1) ||       // States use PC7:PC6; 00 -> 01 -> 11 -> 10 -> 00 is counted positive
       (previousPins == 1 && pins == 3) ||
       (previousPins == 3 && pins == 2) ||
       (previousPins == 2 && pins == 0))
    {
        encoderCount = encoderCount + 1;         // One positive edge; physical direction depends on encoder A/B wiring
    }
    else if((previousPins == 0 && pins == 2) ||  // Reverse sequence: 00 -> 10 -> 11 -> 01 -> 00 is counted negative
            (previousPins == 2 && pins == 3) ||
            (previousPins == 3 && pins == 1) ||
            (previousPins == 1 && pins == 0))
    {
        encoderCount = encoderCount - 1;         // One negative edge; 4 valid edges make one encoder signal cycle
    }
    previousPins = pins;                         // Remember the state; unchanged states and two-bit jumps add no count
}

//PART 3
void updateEncoderVelocity(uint32_t elapsedMicroseconds)
{
    int32_t changeInPosition;

    encoderPosition = encoderCount;                                                       // Take a copy of the count updated by interrupts
    changeInPosition = encoderPosition - previousPosition;                                // Find the movement since the last check
    encoderVelocity = (float)changeInPosition * 1000000.0f / (float)elapsedMicroseconds;  // counts / microseconds * 1,000,000 = counts/second; sign gives direction
    previousPosition = encoderPosition;
}

//PART 3
int32_t readEncoderPosition(void)
{
    return encoderPosition;  // Return the last saved position in counts
}

//PART 3
float readEncoderVelocity(void)
{
    return encoderVelocity;  // Return the last calculated speed in counts per second
}
