#ifndef STM32F446XX_ADC_MORT_H
#define STM32F446XX_ADC_MORT_H

#include <stdint.h>

//PART 1.1
#define ADC3_DR_REGISTER ((uint32_t)0x4001224C)  // ADC3 base 0x40012200 + offset 0x4C; hardware puts each result here for DMA to read

//PART 1.1
void initADC3(uint32_t numberOfChannels);
void startADC3(void);

#endif
