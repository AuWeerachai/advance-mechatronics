#ifndef STM32F446XX_TIMER_MORT_H
#define STM32F446XX_TIMER_MORT_H

#include <stdint.h>

//PART 1.1
void initTimer2(void);
uint32_t readTimer2(void);
void delayMicroseconds(uint32_t duration);

//PART 2
void initTimer3PWM(void);
void writeTimer3Duty(uint32_t duty);

#endif
