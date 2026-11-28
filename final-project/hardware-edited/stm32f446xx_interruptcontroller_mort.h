#ifndef __STM32F446XX_INTERRUPTCONTROLLER_MORT_H_
#define __STM32F446XX_INTERRUPTCONTROLLER_MORT_H_

#include "stm32f446xx_mort.h"

void initFanTachInterrupt(void);
uint32_t readFanTachPulses(void);
void EXTI15_10_IRQHandler(void);

#endif
