#include "stm32f446xx_interruptcontroller_mort.h"
#include "stm32f446xx_rcc_mort.h"

// Register addresses.
#define EXTI_IMR_REGISTER (EXTI_BASE_MORT + 0x00)
#define SYSCFG_EXTICR4_REGISTER (SYSCFG_BASE_MORT + 0x14)
#define EXTI_RTSR_REGISTER (EXTI_BASE_MORT + 0x08)
#define EXTI_FTSR_REGISTER (EXTI_BASE_MORT + 0x0C)
#define EXTI_PR_REGISTER (EXTI_BASE_MORT + 0x14)
#define NVIC_ISER1_REGISTER ((uint32_t)0xE000E104)

// Flags.
#define EXTI_IMR_LINE12_ENABLE ((uint32_t)0x1000) // Bit 12: allow EXTI12 interrupts.
#define SYSCFG_EXTICR4_LINE12_PORT_MASK ((uint32_t)0x0F) // Bits 3:0 select the port for line 12.
#define SYSCFG_EXTICR4_LINE12_PORTD ((uint32_t)0x03) // Bits 3:0 = 0011: PD12.
#define EXTI_RTSR_LINE12_ENABLE ((uint32_t)0x1000) // Bit 12: detect rising edges.
#define EXTI_FTSR_LINE12_MASK ((uint32_t)0x1000) // Bit 12: falling-edge setting.
#define EXTI_PR_LINE12_FLAG_MASK ((uint32_t)0x1000) // Bit 12: pending edge on line 12.
#define NVIC_ISER1_EXTI15_10_ENABLE ((uint32_t)0x100) // Bit 8: interrupt 40, EXTI lines 10-15.

// Only the interrupt handler changes this count after initialization.
// A 16-bit count wraps from 65535 to 0.
static volatile uint16_t fanTachPulses;

void initFanTachInterrupt(void)
{
    volatile uint32_t * reg_pointer;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

    // Disable line 12 while setting up its input and pulse count.
    reg_pointer = (volatile uint32_t *)EXTI_IMR_REGISTER;
    *reg_pointer = *reg_pointer & ~EXTI_IMR_LINE12_ENABLE;
    fanTachPulses = 0;

    // Connect Port D pin 12 to EXTI line 12.
    reg_pointer = (volatile uint32_t *)SYSCFG_EXTICR4_REGISTER;
    *reg_pointer = *reg_pointer & ~SYSCFG_EXTICR4_LINE12_PORT_MASK;
    *reg_pointer = *reg_pointer | SYSCFG_EXTICR4_LINE12_PORTD;

    // Count rising edges only: RTSR bit 12 = 1, FTSR bit 12 = 0.
    reg_pointer = (volatile uint32_t *)EXTI_RTSR_REGISTER;
    *reg_pointer = *reg_pointer | EXTI_RTSR_LINE12_ENABLE;
    reg_pointer = (volatile uint32_t *)EXTI_FTSR_REGISTER;
    *reg_pointer = *reg_pointer & ~EXTI_FTSR_LINE12_MASK;

    // Write 1 to clear an old pending flag for line 12.
    reg_pointer = (volatile uint32_t *)EXTI_PR_REGISTER;
    *reg_pointer = EXTI_PR_LINE12_FLAG_MASK;

    reg_pointer = (volatile uint32_t *)EXTI_IMR_REGISTER;
    *reg_pointer = *reg_pointer | EXTI_IMR_LINE12_ENABLE;

    // Enable the CPU interrupt shared by EXTI lines 10-15.
    // Writing 0 to the other bits leaves their enables unchanged.
    reg_pointer = (volatile uint32_t *)NVIC_ISER1_REGISTER;
    *reg_pointer = NVIC_ISER1_EXTI15_10_ENABLE;
}

uint32_t readFanTachPulses(void)
{
    return fanTachPulses;
}

// This name must match the interrupt vector in the supplied startup file.
void EXTI15_10_IRQHandler(void)
{
    uint32_t pending;
    uint32_t enabled;
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)EXTI_PR_REGISTER;
    pending = *reg_pointer;
    reg_pointer = (volatile uint32_t *)EXTI_IMR_REGISTER;
    enabled = *reg_pointer;

    if((pending & enabled & EXTI_PR_LINE12_FLAG_MASK) != 0)
    {
        // Clear this flag before counting, so another edge can be recorded.
        reg_pointer = (volatile uint32_t *)EXTI_PR_REGISTER;
        *reg_pointer = EXTI_PR_LINE12_FLAG_MASK;
        fanTachPulses = (uint16_t)(fanTachPulses + 1);
    }
}
