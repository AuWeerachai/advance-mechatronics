#include "stm32f446xx_timer_mort.h"
#include "stm32f446xx_rcc_mort.h"

//PART 1.1
#define TIMER2_BASE_ADDRESS ((uint32_t)0x40000000)
#define TIMER2_CR1_REGISTER (TIMER2_BASE_ADDRESS + 0x0)
#define TIMER2_SR_REGISTER (TIMER2_BASE_ADDRESS + 0x10)
#define TIMER2_EGR_REGISTER (TIMER2_BASE_ADDRESS + 0x14)
#define TIMER2_CNT_REGISTER (TIMER2_BASE_ADDRESS + 0x24)
#define TIMER2_PSC_REGISTER (TIMER2_BASE_ADDRESS + 0x28)
#define TIMER2_ARR_REGISTER (TIMER2_BASE_ADDRESS + 0x2C)

//PART 2
#define TIMER3_BASE_ADDRESS ((uint32_t)0x40000400)
#define TIMER3_CR1_REGISTER (TIMER3_BASE_ADDRESS + 0x0)
#define TIMER3_DIER_REGISTER (TIMER3_BASE_ADDRESS + 0xC)
#define TIMER3_SR_REGISTER (TIMER3_BASE_ADDRESS + 0x10)
#define TIMER3_EGR_REGISTER (TIMER3_BASE_ADDRESS + 0x14)
#define TIMER3_CCMR2_REGISTER (TIMER3_BASE_ADDRESS + 0x1C)
#define TIMER3_CCER_REGISTER (TIMER3_BASE_ADDRESS + 0x20)
#define TIMER3_PSC_REGISTER (TIMER3_BASE_ADDRESS + 0x28)
#define TIMER3_ARR_REGISTER (TIMER3_BASE_ADDRESS + 0x2C)
#define TIMER3_CCR3_REGISTER (TIMER3_BASE_ADDRESS + 0x3C)

//PART 1.1
#define TIMER_COUNTER_ENABLE ((uint32_t)1)   // 1 sets CR1 bit 0 (CEN), allowing the counter to run
#define TIMER_UPDATE_GENERATE ((uint32_t)1)  // 1 sets EGR bit 0 (UG), loading the prescaler and buffered settings

//PART 2
#define TIMER3_CH3_PWM1 ((uint32_t)0x60)     // 0x60 = 6 * 16; CCMR2 bits 6-4 = 110 selects PWM mode 1 on channel 3
#define TIMER3_CH3_PRELOAD ((uint32_t)0x8)   // 0x8 = 2^3; CCMR2 bit 3 buffers CCR3 writes until the next update
#define TIMER3_CH3_ENABLE ((uint32_t)0x100)  // 0x100 = 2^8; CCER bit 8 enables channel 3, routed to PB0 by GPIO AF2

//PART 1.1
void initTimer2(void)
{
    volatile uint32_t * reg_pointer;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);  // Turn on the clock for TIM2

    reg_pointer = (volatile uint32_t *)TIMER2_CR1_REGISTER;  // Select the control register 1
    *reg_pointer = 0;                                        // Stop Timer2 and clear its settings

    reg_pointer = (volatile uint32_t *)TIMER2_PSC_REGISTER;  // Select the clock divider register
    *reg_pointer = 89;                                       // system_mort.c sets APB1 = 45 MHz and timer clock = 90 MHz; PSC = 90 - 1 gives 1 tick/us

    reg_pointer = (volatile uint32_t *)TIMER2_ARR_REGISTER;  // Select the counter limit register
    *reg_pointer = (uint32_t)0xFFFFFFFF;                     // Maximum 32-bit count; wraps to 0 after 2^32 microseconds (about 71.6 minutes)

    reg_pointer = (volatile uint32_t *)TIMER2_EGR_REGISTER;  // Select the update command register
    *reg_pointer = TIMER_UPDATE_GENERATE;                    // Load the new timer settings

    reg_pointer = (volatile uint32_t *)TIMER2_SR_REGISTER;  // Select the status register
    *reg_pointer = 0;                                       // Clear old timer flags

    reg_pointer = (volatile uint32_t *)TIMER2_CR1_REGISTER;  // Select the control register 1
    *reg_pointer = *reg_pointer | TIMER_COUNTER_ENABLE;      // Start the timer counting
}

//PART 1.1
uint32_t readTimer2(void)
{
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)TIMER2_CNT_REGISTER;  // Select the counter value register
    return *reg_pointer;
}

//PART 1.1
void delayMicroseconds(uint32_t duration)
{
    uint32_t startTime;

    startTime = readTimer2();
    while((uint32_t)(readTimer2() - startTime) < duration)  // Unsigned subtraction gives elapsed ticks even across wrap; 1 tick = 1 microsecond
    {
    }
}

//PART 2
void initTimer3PWM(void)
{
    volatile uint32_t * reg_pointer;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);  // Turn on the clock for TIM3

    reg_pointer = (volatile uint32_t *)TIMER3_CR1_REGISTER;  // Select the control register 1
    *reg_pointer = 0;                                        // Stop Timer3 and clear its settings

    reg_pointer = (volatile uint32_t *)TIMER3_DIER_REGISTER;  // Select the interrupt enable register
    *reg_pointer = 0;                                         // Disable timer interrupts

    reg_pointer = (volatile uint32_t *)TIMER3_CCER_REGISTER;  // Select the output enable register
    *reg_pointer = 0;                                         // Disable timer outputs during setup

    reg_pointer = (volatile uint32_t *)TIMER3_PSC_REGISTER;  // Select the clock divider register
    *reg_pointer = 0;                                        // Use the 90 MHz timer clock without division

    reg_pointer = (volatile uint32_t *)TIMER3_ARR_REGISTER;  // Select the counter limit register
    *reg_pointer = 2999;                                     // Lab 5 Part 2 requests about 30 kHz: 90 MHz / 30 kHz = 3000 ticks; ARR = 3000 - 1

    reg_pointer = (volatile uint32_t *)TIMER3_CCR3_REGISTER;  // Select the PWM duty register
    *reg_pointer = 0;                                         // Start with zero duty

    reg_pointer = (volatile uint32_t *)TIMER3_CCMR2_REGISTER;  // Select the channel mode register
    *reg_pointer = 0;                                          // Clear channel mode settings
    *reg_pointer = *reg_pointer | TIMER3_CH3_PWM1;             // PWM1: PB0 high while count < CCR3, low for the rest of the 3000-tick period
    *reg_pointer = *reg_pointer | TIMER3_CH3_PRELOAD;          // Apply duty changes at the next PWM period

    reg_pointer = (volatile uint32_t *)TIMER3_EGR_REGISTER;  // Select the update command register
    *reg_pointer = TIMER_UPDATE_GENERATE;                    // Load the new timer settings

    reg_pointer = (volatile uint32_t *)TIMER3_SR_REGISTER;  // Select the status register
    *reg_pointer = 0;                                       // Clear old timer flags

    reg_pointer = (volatile uint32_t *)TIMER3_CCER_REGISTER;  // Select the output enable register
    *reg_pointer = *reg_pointer | TIMER3_CH3_ENABLE;          // Enable the channel 3 output

    reg_pointer = (volatile uint32_t *)TIMER3_CR1_REGISTER;  // Select the control register 1
    *reg_pointer = *reg_pointer | TIMER_COUNTER_ENABLE;      // Start the timer counting
}

//PART 2
void writeTimer3Duty(uint32_t duty)
{
    volatile uint32_t * reg_pointer;

    if(duty > 1000)  // Software duty scale: 1000 means 100%; limit larger commands to full duty
    {
        duty = 1000;
    }

    reg_pointer = (volatile uint32_t *)TIMER3_CCR3_REGISTER;  // Select the PWM duty register
    *reg_pointer = duty * 3;                                  // CCR3 = duty * 3000 / 1000; 0 = always low, 1500 = 50%, 3000 = always high
}
