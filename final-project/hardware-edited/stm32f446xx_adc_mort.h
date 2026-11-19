#ifndef STM32F446XX_ADC_MORT_H
#define STM32F446XX_ADC_MORT_H
#include <stdint.h>

#define ADC1_DR_REGISTER ((uint32_t)0x4001204C)
void initADC1(void);
void startADC1(void);

#endif
