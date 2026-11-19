#include "motor.h"
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_timer_mort.h"

void initMotor(void)
{
    initTimer1FanPWM();       // Prepare 25 kHz PWM at 0% duty.
    initGpioE9AsFanOutput();  // Connect Timer 1 channel 1 to D6 / PE9, open-drain.
}

void writeMotorPower(uint32_t duty)
{
    // Control the fan's PWM input: 0 = LOW, 1000 = released HIGH by its pull-up.
    writeTimer1FanDuty(duty);
}
