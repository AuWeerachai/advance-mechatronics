#include "stm32f446xx_timer_mort.h"
#include "stm32f446xx_rcc_mort.h"
#include "fan_config.h"

// Timer 2: one count per microsecond.
#define TIMER2_BASE_ADDRESS ((uint32_t)0x40000000)
#define TIMER2_CR1_REGISTER (TIMER2_BASE_ADDRESS + 0x0)
#define TIMER2_SR_REGISTER (TIMER2_BASE_ADDRESS + 0x10)
#define TIMER2_EGR_REGISTER (TIMER2_BASE_ADDRESS + 0x14)
#define TIMER2_CNT_REGISTER (TIMER2_BASE_ADDRESS + 0x24)
#define TIMER2_PSC_REGISTER (TIMER2_BASE_ADDRESS + 0x28)
#define TIMER2_ARR_REGISTER (TIMER2_BASE_ADDRESS + 0x2C)

// Timer 1 channel 1: fan PWM at 25 kHz.
#define TIMER1_BASE_ADDRESS ((uint32_t)0x40010000)
#define TIMER1_CR1_REGISTER (TIMER1_BASE_ADDRESS + 0x00)
#define TIMER1_DIER_REGISTER (TIMER1_BASE_ADDRESS + 0x0C)
#define TIMER1_SR_REGISTER (TIMER1_BASE_ADDRESS + 0x10)
#define TIMER1_EGR_REGISTER (TIMER1_BASE_ADDRESS + 0x14)
#define TIMER1_CCMR1_REGISTER (TIMER1_BASE_ADDRESS + 0x18)
#define TIMER1_CCER_REGISTER (TIMER1_BASE_ADDRESS + 0x20)
#define TIMER1_PSC_REGISTER (TIMER1_BASE_ADDRESS + 0x28)
#define TIMER1_ARR_REGISTER (TIMER1_BASE_ADDRESS + 0x2C)
#define TIMER1_RCR_REGISTER (TIMER1_BASE_ADDRESS + 0x30)
#define TIMER1_CCR1_REGISTER (TIMER1_BASE_ADDRESS + 0x34)
#define TIMER1_BDTR_REGISTER (TIMER1_BASE_ADDRESS + 0x44)

// Flags.
#define TIMER_COUNTER_ENABLE ((uint32_t)1)   // 1 sets CR1 bit 0 (CEN), allowing the counter to run
#define TIMER_UPDATE_GENERATE ((uint32_t)1)  // 1 sets EGR bit 0 (UG), loading the prescaler and buffered settings

#define TIMER1_CCMR1_CH1_PWM1 ((uint32_t)0x60)
#define TIMER1_CCMR1_CH1_PRELOAD ((uint32_t)0x8)
#define TIMER1_CCER_CH1_ENABLE ((uint32_t)0x1)
#define TIMER1_BDTR_MAIN_OUTPUT_ENABLE ((uint32_t)0x8000)

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

uint32_t readTimer2(void)
{
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)TIMER2_CNT_REGISTER;  // Select the counter value register
    return *reg_pointer;
}

void delayMicroseconds(uint32_t duration)
{
    uint32_t startTime;

    startTime = readTimer2();
    while((readTimer2() - startTime) < duration)  
    {
    }
}

void initTimer1FanPWM(void)
{
    volatile uint32_t * reg_pointer;

    // Set up an initially LOW fan command before connecting it to PE9.
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    reg_pointer = (volatile uint32_t *)TIMER1_CR1_REGISTER;
    *reg_pointer = 0;
    reg_pointer = (volatile uint32_t *)TIMER1_DIER_REGISTER;
    *reg_pointer = 0;
    reg_pointer = (volatile uint32_t *)TIMER1_CCER_REGISTER;
    *reg_pointer = 0;
    reg_pointer = (volatile uint32_t *)TIMER1_BDTR_REGISTER;
    *reg_pointer = 0;
    reg_pointer = (volatile uint32_t *)TIMER1_PSC_REGISTER;
    *reg_pointer = 0;
    reg_pointer = (volatile uint32_t *)TIMER1_ARR_REGISTER;
    *reg_pointer = FAN_PWM_PERIOD_COUNTS - 1;  // Timer 1 clock = 180 MHz; 180,000,000 / 7200 = 25,000 Hz.
    reg_pointer = (volatile uint32_t *)TIMER1_RCR_REGISTER;
    *reg_pointer = 0;     // Update every PWM period.
    reg_pointer = (volatile uint32_t *)TIMER1_CCR1_REGISTER;
    *reg_pointer = 0;     // Start with 0% HIGH time.
    reg_pointer = (volatile uint32_t *)TIMER1_CCMR1_REGISTER;
    *reg_pointer = TIMER1_CCMR1_CH1_PWM1;
    *reg_pointer = *reg_pointer | TIMER1_CCMR1_CH1_PRELOAD;
    reg_pointer = (volatile uint32_t *)TIMER1_EGR_REGISTER;
    *reg_pointer = TIMER_UPDATE_GENERATE;
    reg_pointer = (volatile uint32_t *)TIMER1_SR_REGISTER;
    *reg_pointer = 0;
    reg_pointer = (volatile uint32_t *)TIMER1_CCER_REGISTER;
    *reg_pointer = TIMER1_CCER_CH1_ENABLE;
    reg_pointer = (volatile uint32_t *)TIMER1_BDTR_REGISTER;
    *reg_pointer = TIMER1_BDTR_MAIN_OUTPUT_ENABLE;  // Advanced timers require this output gate.
    reg_pointer = (volatile uint32_t *)TIMER1_CR1_REGISTER;
    *reg_pointer = TIMER_COUNTER_ENABLE;
}

void writeTimer1FanDuty(uint32_t duty)
{
    volatile uint32_t * reg_pointer;

    // Duty scale: 0 = 0% HIGH time; 1000 = 100% HIGH time.
    if(duty > FAN_DUTY_SCALE)
    {
        duty = FAN_DUTY_SCALE;
    }
    reg_pointer = (volatile uint32_t *)TIMER1_CCR1_REGISTER;
    *reg_pointer = duty * FAN_PWM_PERIOD_COUNTS / FAN_DUTY_SCALE;  // Multiply first so integer division keeps partial duty values.
}
