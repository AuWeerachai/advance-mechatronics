/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F446XX_GPIO_MORT_H_
#define __STM32F446XX_GPIO_MORT_H_


/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32f446xx_mort.h"

/* Macros for Everyone--------------------------------------------------------*/
#define PIN_0   0
#define PIN_1   1
#define PIN_2   2
#define PIN_3   3
#define PIN_4   4
#define PIN_5   5
#define PIN_6   6
#define PIN_7   7
#define PIN_8   8
#define PIN_9   9
#define PIN_10  10


/*Function declarations---------------------------------------------------------*/
// No changes needed here for NOTE 1.1. All required functions are already declared. Other source (.c) files can #include this header to use these declarations.
// No changes needed here for NOTE 1.2
void initGpioB0AsOutput( void );
void initGpioC6AsInput( void );
void initGpioD8D9AsAF7( void );
void toggleGPIOB0( void );
void setGPIOB0( void );
void clearGPIOB0( void );
uint32_t checkGPIOC6(void);
void setGPIOB0_withC6( void );

//NOTE 1.3 create a function connecting PB0 to clock 3 channel 3
void initGpioB0asAF2( void );

//NOTE 1.4 create a function connecting PC6 to clock 3 channel 1
void initGpioC6asAF2(void);


#endif /*__STM32F446XX_GPIO_MORT_H_ */



