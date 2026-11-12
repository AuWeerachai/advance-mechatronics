#include "stm32f446xx_adc_mort.h"
#include "stm32f446xx_rcc_mort.h"
#include "stm32f446xx_timer_mort.h"

//PART 1.1
#define ADC3_BASE_ADDRESS ((uint32_t)0x40012200)
#define ADC3_SR_REGISTER (ADC3_BASE_ADDRESS + 0x0)
#define ADC3_CR1_REGISTER (ADC3_BASE_ADDRESS + 0x4)
#define ADC3_CR2_REGISTER (ADC3_BASE_ADDRESS + 0x8)
#define ADC3_SMPR2_REGISTER (ADC3_BASE_ADDRESS + 0x10)
#define ADC3_SQR1_REGISTER (ADC3_BASE_ADDRESS + 0x2C)
#define ADC3_SQR3_REGISTER (ADC3_BASE_ADDRESS + 0x34)
#define ADC_COMMON_CCR_REGISTER ((uint32_t)0x40012304)

//PART 1.1
#define ADC_CCR_PRESCALER_MASK ((uint32_t)0x30000)     // 0x30000 = 3 * 2^16; bits 17-16 select the ADC clock divider
#define ADC_CCR_PRESCALER_DIV4 ((uint32_t)0x10000)     // 0x10000 = 1 * 2^16; bits 17-16 = 01 means divide by 4
#define ADC_CR1_SCAN ((uint32_t)0x100)                 // 0x100 = 2^8; CR1 bit 8 enables reading a channel sequence
#define ADC_CR2_CONTINUOUS ((uint32_t)0x2)             // 0x2 = 2^1; CR2 bit 1 repeats the sequence
#define ADC_CR2_DMA ((uint32_t)0x100)                  // 0x100 = 2^8; CR2 bit 8 enables ADC requests to DMA
#define ADC_CR2_DMA_CONTINUE ((uint32_t)0x200)         // 0x200 = 2^9; CR2 bit 9 keeps requests active after each DMA group
#define ADC_CR2_EOC_EACH_CONVERSION ((uint32_t)0x400)  // 0x400 = 2^10; CR2 bit 10 selects a done flag for each reading
#define ADC_CR2_ENABLE ((uint32_t)0x1)                 // 0x1 = 2^0; CR2 bit 0 turns ADC3 on
#define ADC_CR2_START ((uint32_t)0x40000000)           // 0x40000000 = 2^30; CR2 bit 30 starts the selected sequence

//PART 1.1
void initADC3(uint32_t numberOfChannels)
{
    volatile uint32_t * reg_pointer;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC3, ENABLE);  // Turn on the clock for ADC3


    reg_pointer = (volatile uint32_t *)ADC3_CR2_REGISTER;  // Select control register 2
    *reg_pointer = 0;                                      // Clear settings and turn off ADC3 before setup

    reg_pointer = (volatile uint32_t *)ADC_COMMON_CCR_REGISTER;  // Select the shared ADC settings register
    *reg_pointer = *reg_pointer & ~ADC_CCR_PRESCALER_MASK;       // AND with ~0x30000 clears bits 17-16; other ADC settings stay unchanged
    *reg_pointer = *reg_pointer | ADC_CCR_PRESCALER_DIV4;        // Bits 17-16 = 01 selects /4: 90 MHz / 4 = 22.5 MHz; /2 gives 45 MHz, too fast

    reg_pointer = (volatile uint32_t *)ADC3_CR1_REGISTER;  // Select control register 1
    *reg_pointer = 0;                                      // Clear settings; use 12-bit readings
    if(numberOfChannels == 3)                              // Extra setup for multi pin reading
    {
        *reg_pointer = *reg_pointer | ADC_CR1_SCAN;        // SCAN follows the SQR3 channel order for the number of readings set in SQR1
    }

    reg_pointer = (volatile uint32_t *)ADC3_CR2_REGISTER;       // Select control register 2
    *reg_pointer = *reg_pointer | ADC_CR2_CONTINUOUS;           // After the last selected channel, start again at the first channel
    *reg_pointer = *reg_pointer | ADC_CR2_DMA;                  // Each finished conversion writes DR, then requests DMA to copy DR into memory
    *reg_pointer = *reg_pointer | ADC_CR2_DMA_CONTINUE;         // Keep requesting copies after DMA reaches the end of its 1 or 3 slots
    *reg_pointer = *reg_pointer | ADC_CR2_EOC_EACH_CONVERSION;  // Set SR done flag after every reading; DMA requests do not need CPU polling

    reg_pointer = (volatile uint32_t *)ADC3_SQR1_REGISTER;  // Select the sequence length register
    *reg_pointer = 0;                                       // Bits 23-20 = 0000; hardware reads length as 0 + 1 = 1 conversion
    if(numberOfChannels == 3)                               // Extra setup for PF7, PF8, and PF9
    {
        *reg_pointer = ((uint32_t)0x200000);                // 0x200000 = 2 * 2^20; bits 23-20 hold 2; hardware uses 2 + 1 = 3 readings
    }

    reg_pointer = (volatile uint32_t *)ADC3_SQR3_REGISTER;  // Select the channel order register
    *reg_pointer = (uint32_t)5;                             // 5 = binary 00101 in bits 4-0 (first slot); chip maps ADC3 channel 5 to PF7; other slots cleared
    if(numberOfChannels == 3)                               // OR adds settings in the cleared fields while keeping the earlier pin settings
    {
        *reg_pointer = *reg_pointer | ((uint32_t)0xC0);     // 0xC0 = 12 * 16 = 192 = binary 0011000000; bits 9-5 = 00110 = channel 6 = PF8 (second)
        *reg_pointer = *reg_pointer | ((uint32_t)0x1C00);   // 0x1C00 = 7168 = 7 * 1024; bits 14-10 = 00111 = channel 7 = PF9 (third)
    }

    reg_pointer = (volatile uint32_t *)ADC3_SMPR2_REGISTER;  // Select the sampling time register
    *reg_pointer = ((uint32_t)0x38000);                      // 0x38000 = 7 * 2^15; bits 17-15 = 111 = setting 7: sample channel 5 (PF7) for 480 cycles
    if(numberOfChannels == 3)                                // OR adds settings in the cleared fields while keeping the earlier pin settings
    {
        *reg_pointer = *reg_pointer | ((uint32_t)0x1C0000);  // 0x1C0000 = 7 * 2^18; bits 20-18 = 111: sample channel 6 (PF8) for 480 cycles
        *reg_pointer = *reg_pointer | ((uint32_t)0xE00000);  // 0xE00000 = 7 * 2^21; bits 23-21 = 111: sample channel 7 (PF9) for 480 cycles
    }

    reg_pointer = (volatile uint32_t *)ADC3_SR_REGISTER;  // Select the status register
    *reg_pointer = 0;                                     // Clear old status flags

    reg_pointer = (volatile uint32_t *)ADC3_CR2_REGISTER;  // Select control register 2
    *reg_pointer = *reg_pointer | ADC_CR2_ENABLE;          // Turn on ADC3
    delayMicroseconds(10);                                 // Wait for ADC3 to become ready
}

//PART 1.1
void startADC3(void)
{
    volatile uint32_t * reg_pointer;  // Pointer used to read and write hardware registers

    reg_pointer = (volatile uint32_t *)ADC3_CR2_REGISTER;  // Select control register 2
    *reg_pointer = *reg_pointer | ADC_CR2_START;           // Start ADC hardware: select pin, measure, write DR, request DMA; repeat because CONT is set
}
