/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F446XX_TIMER_H_
#define __STM32F446XX_TIMER_H_


/* Includes ------------------------------------------------------------------*/
#include "stm32f446xx_mort.h"

//NOTE 1.3
#include "stm32f446xx_rcc_mort.h"


/* Macros ------------------------------------------------------------------*/



/* Function Definitions-------------------------------------------------------*/
//NOTE 1.3
void initTimer3CH3(void);

//NOTE 1.4
void initTimer3CH1 (void);
uint32_t checkTimer3CH1Capture(void);
uint32_t readTimer3CH1Capture(void);

#endif /*__STM32F446XX_TIMER_H_ */