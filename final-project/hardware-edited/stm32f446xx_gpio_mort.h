#ifndef __STM32F446XX_GPIO_MORT_H_
#define __STM32F446XX_GPIO_MORT_H_

#include <stdint.h>
#include "stm32f446xx_mort.h"

// USART3 serial communication.
void initGpioD8D9AsAF7(void);

// Analog inputs: ball, hand and lever.
void initGpioA3AsAnalog(void);
void initGpioC0AsAnalog(void);
void initGpioC3AsAnalog(void);

// Clock-arm START and END sensor inputs.
void initGpioF15AsInput(void);
void initGpioE13AsInput(void);
uint32_t readGpioF15(void);
uint32_t readGpioE13(void);

// Fan PWM control input on D6 / PE9.
void initGpioE9AsFanOutput(void);

// Report tachometer connection; only initialized when FAN_TACH_CONNECTED is 1.
void initGpioD12AsFanTachInput(void);

#endif
