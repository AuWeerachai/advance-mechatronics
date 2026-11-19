#include "led.h"
#include "stm32f446xx_mort.h"
#include "stm32f446xx_rcc_mort.h"

// Register offsets used with each air LED's port base address.
#define GPIO_MODER_REGISTER_OFFSET  ((uint32_t)0x00)
#define GPIO_OTYPER_REGISTER_OFFSET ((uint32_t)0x04)
#define GPIO_PUPDR_REGISTER_OFFSET  ((uint32_t)0x0C)
#define GPIO_ODR_REGISTER_OFFSET    ((uint32_t)0x14)

struct AirLedPin
{
    uint32_t portBase;
    uint32_t clock;
    uint32_t pinMask;    // One bit for ODR and OTYPER.
    uint32_t modeMask;   // Two bits for MODER and PUPDR.
    uint32_t modeOutput; // MODER value 01 in this pin's two-bit field.
};

// Array order = LED order. HIGH turns an LED on; LOW turns it off.
// D43-D50 are on CN8; D51-D52 continue on CN9. Each LED needs its series resistor.
static const struct AirLedPin airLedPins[AIR_LED_COUNT] =
{
    // Port base,       clock,                  pinMask, modeMask,   modeOutput
    {GPIOC_BASE_MORT, RCC_AHB1Periph_GPIOC, 0x0100, 0x00030000, 0x00010000}, // LED 1:  D43 / PC8,  CN8 pin 2.
    {GPIOC_BASE_MORT, RCC_AHB1Periph_GPIOC, 0x0200, 0x000C0000, 0x00040000}, // LED 2:  D44 / PC9,  CN8 pin 4.
    {GPIOC_BASE_MORT, RCC_AHB1Periph_GPIOC, 0x0400, 0x00300000, 0x00100000}, // LED 3:  D45 / PC10, CN8 pin 6.
    {GPIOC_BASE_MORT, RCC_AHB1Periph_GPIOC, 0x0800, 0x00C00000, 0x00400000}, // LED 4:  D46 / PC11, CN8 pin 8.
    {GPIOC_BASE_MORT, RCC_AHB1Periph_GPIOC, 0x1000, 0x03000000, 0x01000000}, // LED 5:  D47 / PC12, CN8 pin 10.
    {GPIOD_BASE_MORT, RCC_AHB1Periph_GPIOD, 0x0004, 0x00000030, 0x00000010}, // LED 6:  D48 / PD2,  CN8 pin 12.
    {GPIOG_BASE_MORT, RCC_AHB1Periph_GPIOG, 0x0004, 0x00000030, 0x00000010}, // LED 7:  D49 / PG2,  CN8 pin 14.
    {GPIOG_BASE_MORT, RCC_AHB1Periph_GPIOG, 0x0008, 0x000000C0, 0x00000040}, // LED 8:  D50 / PG3,  CN8 pin 16.
    {GPIOD_BASE_MORT, RCC_AHB1Periph_GPIOD, 0x0080, 0x0000C000, 0x00004000}, // LED 9:  D51 / PD7,  CN9 pin 2.
    {GPIOD_BASE_MORT, RCC_AHB1Periph_GPIOD, 0x0040, 0x00003000, 0x00001000}  // LED 10: D52 / PD6,  CN9 pin 4.
};

void initAirLEDs(void)
{
    uint32_t i;
    uint32_t pinMask;
    volatile uint32_t * reg_pointer;

    for(i = 0; i < AIR_LED_COUNT; i++)
    {
        RCC_AHB1PeriphClockCmd(airLedPins[i].clock, ENABLE);
        pinMask = airLedPins[i].pinMask;

        // Set LOW before enabling output mode, so the LED starts off.
        reg_pointer = (volatile uint32_t *)(airLedPins[i].portBase + GPIO_ODR_REGISTER_OFFSET);
        *reg_pointer = *reg_pointer & ~pinMask;

        // Push-pull output: OTYPER bit = 0.
        reg_pointer = (volatile uint32_t *)(airLedPins[i].portBase + GPIO_OTYPER_REGISTER_OFFSET);
        *reg_pointer = *reg_pointer & ~pinMask;

        // No internal pull-up or pull-down: PUPDR bits = 00.
        reg_pointer = (volatile uint32_t *)(airLedPins[i].portBase + GPIO_PUPDR_REGISTER_OFFSET);
        *reg_pointer = *reg_pointer & ~airLedPins[i].modeMask;

        // Output mode: set this pin's MODER bits to 01.
        reg_pointer = (volatile uint32_t *)(airLedPins[i].portBase + GPIO_MODER_REGISTER_OFFSET);
        *reg_pointer = *reg_pointer & ~airLedPins[i].modeMask;
        *reg_pointer = *reg_pointer | airLedPins[i].modeOutput;
    }
}

void writeAirLEDs(uint32_t ledCount)
{
    uint32_t i;
    uint32_t pinMask;
    volatile uint32_t * reg_pointer;

    // One pass assigns every LED from the requested count, regardless of its previous state.
    for(i = 0; i < AIR_LED_COUNT; i++)
    {
        pinMask = airLedPins[i].pinMask;
        reg_pointer = (volatile uint32_t *)(airLedPins[i].portBase + GPIO_ODR_REGISTER_OFFSET);
        if(i < ledCount)
        {
            *reg_pointer = *reg_pointer | pinMask;  // First ledCount LEDs HIGH.
        }
        else
        {
            *reg_pointer = *reg_pointer & ~pinMask; // Remaining LEDs LOW.
        }
    }
}
