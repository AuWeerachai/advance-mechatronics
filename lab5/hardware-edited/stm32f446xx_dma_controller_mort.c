#include "stm32f446xx_dma_controller_mort.h"
#include "stm32f446xx_adc_mort.h"
#include "stm32f446xx_rcc_mort.h"

//PART 1.1
#define DMA2_BASE_ADDRESS ((uint32_t)0x40026400)  // STM RM0390 memory map: DMA2 base address; S0 registers control Stream0
#define DMA2_LIFCR_REGISTER (DMA2_BASE_ADDRESS + 0x8)
#define DMA2_S0CR_REGISTER (DMA2_BASE_ADDRESS + 0x10)
#define DMA2_S0NDTR_REGISTER (DMA2_BASE_ADDRESS + 0x14)
#define DMA2_S0PAR_REGISTER (DMA2_BASE_ADDRESS + 0x18)
#define DMA2_S0M0AR_REGISTER (DMA2_BASE_ADDRESS + 0x1C)
#define DMA2_S0FCR_REGISTER (DMA2_BASE_ADDRESS + 0x24)

//PART 1.1
#define DMA_STREAM_ENABLE ((uint32_t)0x1)          // 0x1 = 2^0; stream control bit 0 enables transfers
#define DMA_STREAM0_CLEAR_FLAGS ((uint32_t)0x3D)   // 0x3D = bits 5,4,3,2,0; writing 1 clears each Stream0 status flag
#define DMA_CHANNEL2 ((uint32_t)0x4000000)         // 0x4000000 = 2 * 2^25; bits 27-25 = 010 selects ADC3 requests on DMA2 Stream0
#define DMA_MEMORY_HALFWORD ((uint32_t)0x2000)     // 0x2000 = 1 * 2^13; bits 14-13 = 01 means a 16-bit memory write
#define DMA_PERIPHERAL_HALFWORD ((uint32_t)0x800)  // 0x800 = 1 * 2^11; bits 12-11 = 01 means a 16-bit ADC read
#define DMA_MEMORY_INCREMENT ((uint32_t)0x400)     // 0x400 = 2^10; bit 10 advances the memory address after each copy
#define DMA_CIRCULAR ((uint32_t)0x100)             // 0x100 = 2^8; bit 8 reloads the count and starting memory address

//PART 1.1
void initDMA2Stream0(volatile uint16_t * destination, uint32_t numberOfValues)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);  // Turn on the clock for DMA2

    reg_pointer = (volatile uint32_t *)DMA2_S0CR_REGISTER;  // Select the DMA stream settings register
    *reg_pointer = *reg_pointer & ~DMA_STREAM_ENABLE;       // Stop this DMA stream before changing its settings
    while((*reg_pointer & DMA_STREAM_ENABLE) != 0)          // Wait until the DMA stream has stopped
    {
    }

    reg_pointer = (volatile uint32_t *)DMA2_LIFCR_REGISTER;  // Select the DMA flag clearing register
    *reg_pointer = DMA_STREAM0_CLEAR_FLAGS;                  // Clear old transfer and error flags

    reg_pointer = (volatile uint32_t *)DMA2_S0PAR_REGISTER;  // Select the DMA source address register
    *reg_pointer = ADC3_DR_REGISTER;                         // Set source address to ADC3 DR, not a pin; all selected pins use this one result register

    reg_pointer = (volatile uint32_t *)DMA2_S0M0AR_REGISTER;  // Select the DMA memory address register
    *reg_pointer = (uint32_t)destination;                     // 1 reading: address of adcValue; 3 readings: address of adcValuesFromDMA[0]

    reg_pointer = (volatile uint32_t *)DMA2_S0NDTR_REGISTER;  // Select the DMA transfer count register
    *reg_pointer = numberOfValues;                            // 1: copy to adcValue, reload 1; 3: copy to [0], [1], [2], reload 3; circular mode repeats

    reg_pointer = (volatile uint32_t *)DMA2_S0FCR_REGISTER;  // Select the DMA buffer settings register
    *reg_pointer = 0;                                        // Use direct transfers without the FIFO buffer

    reg_pointer = (volatile uint32_t *)DMA2_S0CR_REGISTER;   // Select the DMA stream settings register
    *reg_pointer = 0;                                        // Clear settings; keep ADC address fixed and copy to memory
    *reg_pointer = *reg_pointer | DMA_CHANNEL2;              // STM RM0390 Rev 6 table 29: DMA2 Stream0 channel 2 receives ADC3 requests
    *reg_pointer = *reg_pointer | DMA_MEMORY_HALFWORD;       // Write 16 bits into memory per reading
    *reg_pointer = *reg_pointer | DMA_PERIPHERAL_HALFWORD;   // Read 16 bits from the ADC result register
    *reg_pointer = *reg_pointer | DMA_CIRCULAR;              // Reload starting address and count after 1 or 3 copies; overwrite old readings, do not clear memory
    if(numberOfValues == 3)
    {
        *reg_pointer = *reg_pointer | DMA_MEMORY_INCREMENT;  // 16 bits = 2 bytes: advance destination by 2 bytes, matching uint16_t array slots
    }
    *reg_pointer = *reg_pointer | DMA_STREAM_ENABLE;         // Ready for the first ADC request; main starts ADC only after this setup
}
