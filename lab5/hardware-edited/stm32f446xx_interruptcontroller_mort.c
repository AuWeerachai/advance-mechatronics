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
#define EXTI_LINE6 ((uint32_t)0x40)               // 0x40 = 2^6; bit 6 controls interrupt line 6, routed below to PC6
#define EXTI_LINE7 ((uint32_t)0x80)               // 0x80 = 2^7; bit 7 controls interrupt line 7, routed below to PC7
#define NVIC_EXTI9_5_ENABLE ((uint32_t)0x800000)  // 0x800000 = 2^23; IRQ 23 is the shared EXTI9_5 handler in the startup vector

//PART 3
void initEncoderInterrupts(void)
{
    volatile uint32_t * reg_pointer;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);  // Turn on the clock for SYSCFG

    reg_pointer = (volatile uint32_t *)SYSCFG_EXTICR2_REGISTER;  // Select the interrupt pin selection register
    *reg_pointer = *reg_pointer & ~((uint32_t)0xF00);            // 0xF00 = 15 * 2^8; clear bits 11-8, the port choice for EXTI6
    *reg_pointer = *reg_pointer | ((uint32_t)0x200);             // 0x200 = 2 * 2^8; bits 11-8 = 0010; port code 2 = GPIOC, so EXTI6 reads PC6
    *reg_pointer = *reg_pointer & ~((uint32_t)0xF000);           // 0xF000 = 15 * 2^12; clear bits 15-12, the port choice for EXTI7
    *reg_pointer = *reg_pointer | ((uint32_t)0x2000);            // 0x2000 = 2 * 2^12; bits 15-12 = 0010; port code 2 = GPIOC, so EXTI7 reads PC7

    reg_pointer = (volatile uint32_t *)EXTI_RTSR_REGISTER;  // Select the rising-edge interrupt register
    *reg_pointer = *reg_pointer | EXTI_LINE6;               // Trigger when PC6 goes low to high
    *reg_pointer = *reg_pointer | EXTI_LINE7;               // Trigger when PC7 goes low to high

    reg_pointer = (volatile uint32_t *)EXTI_FTSR_REGISTER;  // Select the falling-edge interrupt register
    *reg_pointer = *reg_pointer | EXTI_LINE6;               // Trigger when PC6 goes high to low
    *reg_pointer = *reg_pointer | EXTI_LINE7;               // Trigger when PC7 goes high to low

    reg_pointer = (volatile uint32_t *)EXTI_PR_REGISTER;  // Select the pending interrupt register
    *reg_pointer = EXTI_LINE6 | EXTI_LINE7;               // Write ones to clear old pending interrupts

    reg_pointer = (volatile uint32_t *)EXTI_IMR_REGISTER;  // Select the interrupt enable mask
    *reg_pointer = *reg_pointer | EXTI_LINE6;              // Allow interrupts from PC6
    *reg_pointer = *reg_pointer | EXTI_LINE7;              // Allow interrupts from PC7

    reg_pointer = (volatile uint32_t *)NVIC_ISER0_REGISTER;  // Select the CPU interrupt enable register
    *reg_pointer = NVIC_EXTI9_5_ENABLE;                      // Allow the CPU to handle interrupts on lines 5-9
}

//PART 3
void EXTI9_5_IRQHandler(void)
{
    uint32_t status;
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)EXTI_PR_REGISTER;  // Select the pending interrupt register
    status = *reg_pointer & (EXTI_LINE6 | EXTI_LINE7);    // Check whether either encoder pin triggered an interrupt
    if(status != 0)
    {
        *reg_pointer = status;                            // Pending flags clear when written as 1; acknowledge only the encoder flags detected above
        updateEncoderCount(readGpioC6C7());               // PC6 becomes bit 0, PC7 bit 1; the old/new 2-bit states determine rotation direction
    }
}
