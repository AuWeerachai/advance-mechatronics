#ifndef STM32F446XX_DMA_CONTROLLER_MORT_H
#define STM32F446XX_DMA_CONTROLLER_MORT_H

#include <stdint.h>

//PART 1.1
void initDMA2Stream0(volatile uint16_t * destination, uint32_t numberOfValues);

#endif
