#include "stm32f446xx_dma_controller_mort.h"
#include "stm32f446xx_adc_mort.h"
#include "stm32f446xx_rcc_mort.h"

//PART 1.1
#define DMA2_BASE_ADDRESS ((uint32_t)0x40026400)
#define DMA2_LIFCR_REGISTER (DMA2_BASE_ADDRESS + 0x08)
#define DMA2_S0CR_REGISTER (DMA2_BASE_ADDRESS + 0x10)
#define DMA2_S0NDTR_REGISTER (DMA2_BASE_ADDRESS + 0x14)
#define DMA2_S0PAR_REGISTER (DMA2_BASE_ADDRESS + 0x18)
#define DMA2_S0M0AR_REGISTER (DMA2_BASE_ADDRESS + 0x1C)
#define DMA2_S0FCR_REGISTER (DMA2_BASE_ADDRESS + 0x24)

//PART 1.1
#define DMA_STREAM_ENABLE ((uint32_t)0x01)
#define DMA_STREAM0_CLEAR_FLAGS ((uint32_t)0x3D)
#define DMA_CHANNEL2 ((uint32_t)2 << 25)
#define DMA_MEMORY_HALFWORD ((uint32_t)1 << 13)
#define DMA_PERIPHERAL_HALFWORD ((uint32_t)1 << 11)
#define DMA_MEMORY_INCREMENT ((uint32_t)1 << 10)
#define DMA_CIRCULAR ((uint32_t)1 << 8)

//PART 1.1
void initDMA2Stream0(volatile uint16_t * destination, uint32_t numberOfValues)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA2, ENABLE);

    reg_pointer = (volatile uint32_t *)DMA2_S0CR_REGISTER;
    *reg_pointer = *reg_pointer & ~DMA_STREAM_ENABLE;
    while((*reg_pointer & DMA_STREAM_ENABLE) != 0)
    {
    }

    reg_pointer = (volatile uint32_t *)DMA2_LIFCR_REGISTER;
    *reg_pointer = DMA_STREAM0_CLEAR_FLAGS;

    reg_pointer = (volatile uint32_t *)DMA2_S0PAR_REGISTER;
    *reg_pointer = ADC3_DR_REGISTER;

    reg_pointer = (volatile uint32_t *)DMA2_S0M0AR_REGISTER;
    *reg_pointer = (uint32_t)destination;

    reg_pointer = (volatile uint32_t *)DMA2_S0NDTR_REGISTER;
    *reg_pointer = numberOfValues;

    reg_pointer = (volatile uint32_t *)DMA2_S0FCR_REGISTER;
    *reg_pointer = 0;

    reg_pointer = (volatile uint32_t *)DMA2_S0CR_REGISTER;
    *reg_pointer = 0;
    *reg_pointer = *reg_pointer | DMA_CHANNEL2;
    *reg_pointer = *reg_pointer | DMA_MEMORY_HALFWORD;
    *reg_pointer = *reg_pointer | DMA_PERIPHERAL_HALFWORD;
    *reg_pointer = *reg_pointer | DMA_CIRCULAR;
    if(numberOfValues == 3)
    {
        *reg_pointer = *reg_pointer | DMA_MEMORY_INCREMENT;
    }
    *reg_pointer = *reg_pointer | DMA_STREAM_ENABLE;
}
