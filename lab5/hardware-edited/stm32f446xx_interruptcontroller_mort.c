#include "stm32f446xx_interruptcontroller_mort.h"
#include "stm32f446xx_rcc_mort.h"
#include "stm32f446xx_gpio_mort.h"
#include "encoder.h"

//PART 3
#define SYSCFG_EXTICR2_REGISTER ((uint32_t)0x4001380C)
#define EXTI_IMR_REGISTER ((uint32_t)0x40013C00)
#define EXTI_RTSR_REGISTER ((uint32_t)0x40013C08)
#define EXTI_FTSR_REGISTER ((uint32_t)0x40013C0C)
#define EXTI_PR_REGISTER ((uint32_t)0x40013C14)
#define NVIC_ISER0_REGISTER ((uint32_t)0xE000E100)

//PART 3
#define EXTI_LINE6 ((uint32_t)1 << 6)
#define EXTI_LINE7 ((uint32_t)1 << 7)
#define NVIC_EXTI9_5_ENABLE ((uint32_t)1 << 23)

//PART 3
void initEncoderInterrupts(void)
{
    volatile uint32_t * reg_pointer;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    reg_pointer = (volatile uint32_t *)SYSCFG_EXTICR2_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)15 << 8);
    *reg_pointer = *reg_pointer | ((uint32_t)2 << 8);
    *reg_pointer = *reg_pointer & ~((uint32_t)15 << 12);
    *reg_pointer = *reg_pointer | ((uint32_t)2 << 12);

    reg_pointer = (volatile uint32_t *)EXTI_RTSR_REGISTER;
    *reg_pointer = *reg_pointer | EXTI_LINE6;
    *reg_pointer = *reg_pointer | EXTI_LINE7;

    reg_pointer = (volatile uint32_t *)EXTI_FTSR_REGISTER;
    *reg_pointer = *reg_pointer | EXTI_LINE6;
    *reg_pointer = *reg_pointer | EXTI_LINE7;

    reg_pointer = (volatile uint32_t *)EXTI_PR_REGISTER;
    *reg_pointer = EXTI_LINE6 | EXTI_LINE7;

    reg_pointer = (volatile uint32_t *)EXTI_IMR_REGISTER;
    *reg_pointer = *reg_pointer | EXTI_LINE6;
    *reg_pointer = *reg_pointer | EXTI_LINE7;

    reg_pointer = (volatile uint32_t *)NVIC_ISER0_REGISTER;
    *reg_pointer = NVIC_EXTI9_5_ENABLE;
}

//PART 3
void EXTI9_5_IRQHandler(void)
{
    uint32_t status;
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)EXTI_PR_REGISTER;
    status = *reg_pointer & (EXTI_LINE6 | EXTI_LINE7);
    if(status != 0)
    {
        *reg_pointer = status;
        updateEncoderCount(readGpioC6C7());
    }
}
