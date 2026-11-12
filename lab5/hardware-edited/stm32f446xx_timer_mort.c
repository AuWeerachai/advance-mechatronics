#include "stm32f446xx_timer_mort.h"
#include "stm32f446xx_rcc_mort.h"

//PART 1.1
#define TIMER2_BASE_ADDRESS ((uint32_t)0x40000000)
#define TIMER2_CR1_REGISTER (TIMER2_BASE_ADDRESS + 0x00)
#define TIMER2_SR_REGISTER (TIMER2_BASE_ADDRESS + 0x10)
#define TIMER2_EGR_REGISTER (TIMER2_BASE_ADDRESS + 0x14)
#define TIMER2_CNT_REGISTER (TIMER2_BASE_ADDRESS + 0x24)
#define TIMER2_PSC_REGISTER (TIMER2_BASE_ADDRESS + 0x28)
#define TIMER2_ARR_REGISTER (TIMER2_BASE_ADDRESS + 0x2C)

//PART 2
#define TIMER3_BASE_ADDRESS ((uint32_t)0x40000400)
#define TIMER3_CR1_REGISTER (TIMER3_BASE_ADDRESS + 0x00)
#define TIMER3_DIER_REGISTER (TIMER3_BASE_ADDRESS + 0x0C)
#define TIMER3_SR_REGISTER (TIMER3_BASE_ADDRESS + 0x10)
#define TIMER3_EGR_REGISTER (TIMER3_BASE_ADDRESS + 0x14)
#define TIMER3_CCMR2_REGISTER (TIMER3_BASE_ADDRESS + 0x1C)
#define TIMER3_CCER_REGISTER (TIMER3_BASE_ADDRESS + 0x20)
#define TIMER3_PSC_REGISTER (TIMER3_BASE_ADDRESS + 0x28)
#define TIMER3_ARR_REGISTER (TIMER3_BASE_ADDRESS + 0x2C)
#define TIMER3_CCR3_REGISTER (TIMER3_BASE_ADDRESS + 0x3C)

//PART 1.1
#define TIMER_COUNTER_ENABLE ((uint32_t)1)
#define TIMER_UPDATE_GENERATE ((uint32_t)1)

//PART 2
#define TIMER3_CH3_PWM1 ((uint32_t)0x60)
#define TIMER3_CH3_PRELOAD ((uint32_t)0x08)
#define TIMER3_CH3_ENABLE ((uint32_t)0x100)

//PART 1.1
void initTimer2(void)
{
    volatile uint32_t * reg_pointer;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    reg_pointer = (volatile uint32_t *)TIMER2_CR1_REGISTER;
    *reg_pointer = 0;

    reg_pointer = (volatile uint32_t *)TIMER2_PSC_REGISTER;
    *reg_pointer = 89;

    reg_pointer = (volatile uint32_t *)TIMER2_ARR_REGISTER;
    *reg_pointer = (uint32_t)0xFFFFFFFF;

    reg_pointer = (volatile uint32_t *)TIMER2_EGR_REGISTER;
    *reg_pointer = TIMER_UPDATE_GENERATE;

    reg_pointer = (volatile uint32_t *)TIMER2_SR_REGISTER;
    *reg_pointer = 0;

    reg_pointer = (volatile uint32_t *)TIMER2_CR1_REGISTER;
    *reg_pointer = *reg_pointer | TIMER_COUNTER_ENABLE;
}

//PART 1.1
uint32_t readTimer2(void)
{
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)TIMER2_CNT_REGISTER;
    return *reg_pointer;
}

//PART 1.1
void delayMicroseconds(uint32_t duration)
{
    uint32_t startTime;

    startTime = readTimer2();
    while((uint32_t)(readTimer2() - startTime) < duration)
    {
    }
}

//PART 2
void initTimer3PWM(void)
{
    volatile uint32_t * reg_pointer;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    reg_pointer = (volatile uint32_t *)TIMER3_CR1_REGISTER;
    *reg_pointer = 0;

    reg_pointer = (volatile uint32_t *)TIMER3_DIER_REGISTER;
    *reg_pointer = 0;

    reg_pointer = (volatile uint32_t *)TIMER3_CCER_REGISTER;
    *reg_pointer = 0;

    reg_pointer = (volatile uint32_t *)TIMER3_PSC_REGISTER;
    *reg_pointer = 0;

    reg_pointer = (volatile uint32_t *)TIMER3_ARR_REGISTER;
    *reg_pointer = 2999;

    reg_pointer = (volatile uint32_t *)TIMER3_CCR3_REGISTER;
    *reg_pointer = 0;

    reg_pointer = (volatile uint32_t *)TIMER3_CCMR2_REGISTER;
    *reg_pointer = 0;
    *reg_pointer = *reg_pointer | TIMER3_CH3_PWM1;
    *reg_pointer = *reg_pointer | TIMER3_CH3_PRELOAD;

    reg_pointer = (volatile uint32_t *)TIMER3_EGR_REGISTER;
    *reg_pointer = TIMER_UPDATE_GENERATE;

    reg_pointer = (volatile uint32_t *)TIMER3_SR_REGISTER;
    *reg_pointer = 0;

    reg_pointer = (volatile uint32_t *)TIMER3_CCER_REGISTER;
    *reg_pointer = *reg_pointer | TIMER3_CH3_ENABLE;

    reg_pointer = (volatile uint32_t *)TIMER3_CR1_REGISTER;
    *reg_pointer = *reg_pointer | TIMER_COUNTER_ENABLE;
}

//PART 2
void writeTimer3Duty(uint32_t duty)
{
    volatile uint32_t * reg_pointer;

    if(duty > 1000)
    {
        duty = 1000;
    }

    reg_pointer = (volatile uint32_t *)TIMER3_CCR3_REGISTER;
    *reg_pointer = duty * 3;
}
