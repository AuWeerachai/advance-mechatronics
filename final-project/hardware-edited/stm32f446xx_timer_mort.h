#ifndef STM32F446XX_TIMER_MORT_H
#define STM32F446XX_TIMER_MORT_H

#include <stdint.h>

void initTimer2(void);
uint32_t readTimer2(void);
void delayMicroseconds(uint32_t duration);

void initTimer1FanPWM(void);
void writeTimer1FanDuty(uint32_t duty);

#endif
