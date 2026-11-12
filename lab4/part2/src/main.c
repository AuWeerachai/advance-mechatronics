// Part 2: scan red, green, and blue, then hold the detected color.
#include "nucleo_led.h"
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_timer_mort.h"
#include "stm32f446xx_usart3_mort.h"

enum DetectorState
{
    RED_ILLUMINATION,
    RED_READ,
    GREEN_ILLUMINATION,
    GREEN_READ,
    BLUE_ILLUMINATION,
    BLUE_READ,
    DONE
};
static enum DetectorState state;

static void displayResult(void);
static uint32_t redReading;
static uint32_t greenReading;
static uint32_t blueReading;


// Initialize the LEDs, comparator input, and timer; clear the stored readings.
void initGummyBearDetector(void)
{
    initLEDs();
    initGpioC6AsInput();
    initTimer3();

    redReading = 0;
    greenReading = 0;
    blueReading = 0;
    state = RED_ILLUMINATION;
}

// Illumination states turn the LED ON and start the timer.
// Read states check for overflow before reading and turning the LED OFF.
void runGummyBearDetector(void)
{
    switch(state)
    {
        case RED_ILLUMINATION:
            writeRedIllumination(1);
            restartTimer3();
            state = RED_READ;
            break;

        case RED_READ:
            if(checkTimer3Overflow() != 0)
            {
                redReading = checkGPIOC6();
                writeRedIllumination(0);
                state = GREEN_ILLUMINATION;
            }
            break;

        case GREEN_ILLUMINATION:
            writeGreenIllumination(1);
            restartTimer3();
            state = GREEN_READ;
            break;

        case GREEN_READ:
            if(checkTimer3Overflow() != 0)
            {
                greenReading = checkGPIOC6();
                writeGreenIllumination(0);
                state = BLUE_ILLUMINATION;
            }
            break;

        case BLUE_ILLUMINATION:
            writeBlueIllumination(1);
            restartTimer3();
            state = BLUE_READ;
            break;

        case BLUE_READ:
            if(checkTimer3Overflow() != 0)
            {
                blueReading = checkGPIOC6();
                writeBlueIllumination(0);
                state = DONE;
                displayResult();
            }
            break;

        case DONE:
            break;
    }
}


// Decode the raw comparator readings using the Lab 3 report data.
// RGB: 000 = 0 =white, 001 = 1 =orange, 011 = 3 =red, 101 = 5 = green.
static void displayResult(void)
{
    uint32_t result;
    result = 4 * redReading + 2 * greenReading + 1 * blueReading;

    // Print once after all three comparator readings are collected.
    printfUSART3("Red: %u, Green: %u, Blue: %u\r\n", redReading, greenReading, blueReading);

    if(result == 0)
    {
        writeWhiteDetection(1);
        transmitLineUSART3("Detected: WHITE");
    }
    else if(result == 1)
    {
        writeOrangeDetection(1);
        transmitLineUSART3("Detected: ORANGE");
    }
    else if(result == 3)
    {
        writeRedDetection(1);
        transmitLineUSART3("Detected: RED");
    }
    else if(result == 5)
    {
        writeGreenDetection(1);
        transmitLineUSART3("Detected: GREEN");
    }
    else
    {
        transmitLineUSART3("Detected: UNKNOWN");
    }
}




int main(void)
{
    initUSART3(); // Configure PD8 and USART3 TX for the serial monitor.
    initGummyBearDetector();
    while(1)
    {
        runGummyBearDetector();
    }
}
