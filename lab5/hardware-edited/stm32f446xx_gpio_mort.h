#ifndef __STM32F446XX_GPIO_MORT_H_
#define __STM32F446XX_GPIO_MORT_H_

#include <stdint.h>
#include "stm32f446xx_mort.h"

//PART 1.1
void initGpioD8D9AsAF7( void );
void initGpioF7AsAnalog(void);

//PART 1.2
void initGpioF8AsAnalog(void);
void initGpioF9AsAnalog(void);

//PART 2
void initGpioB0asAF2( void );
void initGpioD14D15AsOutput(void);
void writeGpioD14(uint32_t value);
void writeGpioD15(uint32_t value);

//PART 3
void initGpioC6C7AsInput(void);
uint32_t readGpioC6C7(void);
void initGpioC13AsInput(void);
uint32_t readGpioC13(void);

#endif
