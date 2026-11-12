#include "motor.h"
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_timer_mort.h"

static int32_t previousDirection;

//PART 2
void initMotor(void)
{
    initGpioD14D15AsOutput();  // PD14 and PD15 drive the two L293B direction inputs
    initTimer3PWM();           // Start PWM with zero duty
    initGpioB0asAF2();         // Chip AF2 connects Timer3 channel 3 to PB0; PB0 drives L293B enable
    previousDirection = 0;     // Start with no drive direction
}

//PART 2
void writeMotorPower(int32_t power)
{
    int32_t direction;
    uint32_t duty;

    if(power > 1000)        // Limit forward power to 100% duty
    {
        power = 1000;
    }
    else if(power < -1000)  // Limit reverse power to 100% duty
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
        duty = (uint32_t)(-power);  // Use the positive size of the reverse command
    }
    else
    {
        direction = 0;
        duty = 0;
    }

    if(direction != previousDirection)  // Change direction only when needed
    {
        writeTimer3Duty(0);             // Disable the PWM drive before changing direction
        delayMicroseconds(40);          // 30 kHz period = 33.3 us; wait 40 us so the buffered zero duty takes effect

        writeGpioD14(0);      // Set the first direction input low
        writeGpioD15(0);      // Set the second direction input low
        if(direction == 1)
        {
            writeGpioD14(1);  // PD14 high + PD15 low selects one motor polarity; direction depends on motor wiring
        }
        else if(direction == -1)
        {
            writeGpioD15(1);  // PD14 low + PD15 high reverses the motor polarity
        }
        previousDirection = direction;
    }

    writeTimer3Duty(duty);  // Apply the requested PWM duty
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
        duty = (adcValue - POTENTIOMETER_MIN_ADC) * 1000;               // Start scaling: subtract ADC minimum, then multiply by the 1000-step duty range
        duty = duty / (POTENTIOMETER_MAX_ADC - POTENTIOMETER_MIN_ADC);  // Divide by ADC range: with 0-4095 limits, duty = adcValue * 1000 / 4095
    }
    writeMotorPower((int32_t)duty);                                     // Drive the motor forward at this duty
}
