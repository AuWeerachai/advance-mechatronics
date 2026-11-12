#include "motor.h"
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_timer_mort.h"

static int32_t previousDirection;

//PART 2
void initMotor(void)
{
    initGpioD14D15AsOutput();
    initTimer3PWM();
    initGpioB0asAF2();
    previousDirection = 0;
}

//PART 2
void writeMotorPower(int32_t power)
{
    int32_t direction;
    uint32_t duty;

    if(power > 1000)
    {
        power = 1000;
    }
    else if(power < -1000)
    {
        power = -1000;
    }

    if(power > 0)
    {
        direction = 1;
        duty = (uint32_t)power;
    }
    else if(power < 0)
    {
        direction = -1;
        duty = (uint32_t)(-power);
    }
    else
    {
        direction = 0;
        duty = 0;
    }

    if(direction != previousDirection)
    {
        writeTimer3Duty(0);
        delayMicroseconds(40);

        writeGpioD14(0);
        writeGpioD15(0);
        if(direction == 1)
        {
            writeGpioD14(1);
        }
        else if(direction == -1)
        {
            writeGpioD15(1);
        }
        previousDirection = direction;
    }

    writeTimer3Duty(duty);
}

//PART 2
void runMotorFromADC(uint32_t adcValue)
{
    uint32_t duty;

    if(adcValue <= POTENTIOMETER_MIN_ADC)
    {
        duty = 0;
    }
    else if(adcValue >= POTENTIOMETER_MAX_ADC)
    {
        duty = 1000;
    }
    else
    {
        duty = (adcValue - POTENTIOMETER_MIN_ADC) * 1000;
        duty = duty / (POTENTIOMETER_MAX_ADC - POTENTIOMETER_MIN_ADC);
    }
    writeMotorPower((int32_t)duty);
}
