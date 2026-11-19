#include "fan.h"
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_timer_mort.h"

void initFan(void)
{
    // Prepare zero-duty PWM before routing Timer 1 channel 1 to the fan.
    initTimer1FanPWM();
    initGpioE9AsFanOutput();
}

void writeFanPower(uint32_t duty)
{
    // 0 = control line LOW; 1000 = fully released HIGH by the fan's pull-up.
    writeTimer1FanDuty(duty);
}
