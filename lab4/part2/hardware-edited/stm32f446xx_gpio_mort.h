#ifndef __STM32F446XX_GPIO_MORT_H_
#define __STM32F446XX_GPIO_MORT_H_

#include <stdint.h>
#include "stm32f446xx_mort.h"

void initGpioC6AsInput(void);
void initGpioD8AsAF7(void);
uint32_t checkGPIOC6(void);

void initGpioPE13AsOutput(void); // D3 / PE13: green detection.
void initGpioPE11AsOutput(void); // D5 / PE11: white detection.
void initGpioPE9AsOutput(void); // D6 / PE9: red illumination.
void initGpioPF15AsOutput(void); // D2 / PF15: red detection.
void initGpioPF14AsOutput(void); // D4 / PF14: orange detection.
void initGpioPF13AsOutput(void); // D7 / PF13: green illumination.
void initGpioPD15AsOutput(void); // D9 / PD15: blue illumination.

void writeGPIOPE13(uint32_t value); // D3 / PE13: green detection.
void writeGPIOPE11(uint32_t value); // D5 / PE11: white detection.
void writeGPIOPE9(uint32_t value); // D6 / PE9: red illumination.
void writeGPIOPF15(uint32_t value); // D2 / PF15: red detection.
void writeGPIOPF14(uint32_t value); // D4 / PF14: orange detection.
void writeGPIOPF13(uint32_t value); // D7 / PF13: green illumination.
void writeGPIOPD15(uint32_t value); // D9 / PD15: blue illumination.

#endif
