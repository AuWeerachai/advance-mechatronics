/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F446XX_TIMER_H_
#define __STM32F446XX_TIMER_H_


/* Includes ------------------------------------------------------------------*/
#include "stm32f446xx_mort.h"

//NOTE 1.3
#include "stm32f446xx_rcc_mort.h"


/* Macros ------------------------------------------------------------------*/



/* Function Definitions-------------------------------------------------------*/
void initTimer3(void);
void restartTimer3(void);
uint32_t checkTimer3Overflow(void);

#endif /*__STM32F446XX_TIMER_H_ */
