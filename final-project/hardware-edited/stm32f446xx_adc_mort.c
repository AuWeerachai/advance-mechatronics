#include "stm32f446xx_adc_mort.h"
#include "stm32f446xx_rcc_mort.h"
#include "stm32f446xx_timer_mort.h"

// Final project: A0/PA3 = ball, A1/PC0 = hand, A2/PC3 = lever.
// STM32 RM0390: ADC1 register map and regular conversion sequence.
#define ADC1_BASE_ADDRESS ((uint32_t)0x40012000)
#define ADC1_SR_REGISTER (ADC1_BASE_ADDRESS + 0x00)
#define ADC1_CR1_REGISTER (ADC1_BASE_ADDRESS + 0x04)
#define ADC1_CR2_REGISTER (ADC1_BASE_ADDRESS + 0x08)
#define ADC1_SMPR1_REGISTER (ADC1_BASE_ADDRESS + 0x0C)
#define ADC1_SMPR2_REGISTER (ADC1_BASE_ADDRESS + 0x10)
#define ADC1_SQR1_REGISTER (ADC1_BASE_ADDRESS + 0x2C)
#define ADC1_SQR3_REGISTER (ADC1_BASE_ADDRESS + 0x34)
#define ADC_COMMON_CCR_REGISTER ((uint32_t)0x40012304)

#define ADC_CCR_PRESCALER_MASK ((uint32_t)0x30000)
#define ADC_CCR_PRESCALER_DIV4 ((uint32_t)0x10000)
#define ADC_CR1_SCAN ((uint32_t)0x100)
#define ADC_CR2_CONTINUOUS ((uint32_t)0x2)
#define ADC_CR2_DMA ((uint32_t)0x100)
#define ADC_CR2_DMA_CONTINUE ((uint32_t)0x200)
#define ADC_CR2_EOC_EACH_CONVERSION ((uint32_t)0x400)
#define ADC_CR2_ENABLE ((uint32_t)0x1)
#define ADC_CR2_START ((uint32_t)0x40000000)

void initADC1(void)
{
    volatile uint32_t * reg_pointer;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

    reg_pointer = (volatile uint32_t *)ADC1_CR2_REGISTER;
    *reg_pointer = 0;  // Stop ADC1 before setting the scan.

    reg_pointer = (volatile uint32_t *)ADC_COMMON_CCR_REGISTER;
    *reg_pointer = *reg_pointer & ~ADC_CCR_PRESCALER_MASK;
    *reg_pointer = *reg_pointer | ADC_CCR_PRESCALER_DIV4;  // 90 MHz / 4 = 22.5 MHz ADC clock.

    reg_pointer = (volatile uint32_t *)ADC1_CR1_REGISTER;
    *reg_pointer = ADC_CR1_SCAN;  // Scan mode, default 12-bit resolution.

    reg_pointer = (volatile uint32_t *)ADC1_CR2_REGISTER;
    *reg_pointer = *reg_pointer | ADC_CR2_CONTINUOUS;
    *reg_pointer = *reg_pointer | ADC_CR2_DMA;
    *reg_pointer = *reg_pointer | ADC_CR2_DMA_CONTINUE;
    *reg_pointer = *reg_pointer | ADC_CR2_EOC_EACH_CONVERSION;

    reg_pointer = (volatile uint32_t *)ADC1_SQR1_REGISTER;
    *reg_pointer = (uint32_t)0x200000;  // Sequence length field = 2, meaning 3 readings.

    reg_pointer = (volatile uint32_t *)ADC1_SQR3_REGISTER;
    *reg_pointer = (uint32_t)0x0003;                 // Slot 1, bits 4:0 = 00011: channel 3, PA3, ball.
    *reg_pointer = *reg_pointer | (uint32_t)0x0140;   // Slot 2, bits 9:5 = 01010: channel 10, PC0, hand.
    *reg_pointer = *reg_pointer | (uint32_t)0x3400;   // Slot 3, bits 14:10 = 01101: channel 13, PC3, lever.

    reg_pointer = (volatile uint32_t *)ADC1_SMPR2_REGISTER;
    *reg_pointer = (uint32_t)0xE00;  // Channel 3 bits 11:9 = 111: 480 sample cycles.

    reg_pointer = (volatile uint32_t *)ADC1_SMPR1_REGISTER;
    *reg_pointer = (uint32_t)0xE07;  // Channels 10 and 13: 480 sample cycles each.

    reg_pointer = (volatile uint32_t *)ADC1_SR_REGISTER;
    *reg_pointer = 0;
    reg_pointer = (volatile uint32_t *)ADC1_CR2_REGISTER;
    *reg_pointer = *reg_pointer | ADC_CR2_ENABLE;
    delayMicroseconds(10);  // Allow the ADC to power up before starting conversions.
}

void startADC1(void)
{
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)ADC1_CR2_REGISTER;
    *reg_pointer = *reg_pointer | ADC_CR2_START;  // Begin the repeating three-channel scan.
}
