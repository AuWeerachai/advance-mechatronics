#include <stdint.h>
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_adc_mort.h"
#include "stm32f446xx_dma_controller_mort.h"
#include "stm32f446xx_timer_mort.h"
#include "stm32f446xx_usart3_mort.h"
#include "motor.h"
#include "motor_controller.h"

//PART 1.1
// int main(void)
// {
//     static volatile uint16_t adcValue;
//
//     initUSART3();
//     initTimer2();
//     initGpioF7AsAnalog();
//     initDMA2Stream0(&adcValue, 1);
//     initADC3(1);
//     startADC3();
//
//     while(1)
//     {
//         delayMicroseconds(200000);
//         printfUSART3("PF7: %u\r\n", (uint32_t)adcValue);
//     }
// }


//PART 1.2
int main(void)
{
    static volatile uint16_t adcValues[3];
    uint32_t pf7;
    uint32_t pf8;
    uint32_t pf9;

    initUSART3();
    initTimer2();
    initGpioF7AsAnalog();
    initGpioF8AsAnalog();
    initGpioF9AsAnalog();
    initDMA2Stream0(adcValues, 3);
    initADC3(3);
    startADC3();

    while(1)
    {
        delayMicroseconds(200000);
        pf7 = adcValues[0];
        pf8 = adcValues[1];
        pf9 = adcValues[2];
        printfUSART3("PF7: %u, PF8: %u, PF9: %u\r\n", pf7, pf8, pf9);
    }
}


//PART 2
// int main(void)
// {
//     static volatile uint16_t adcValue;
//
//     initTimer2();
//     initMotor();
//     initGpioF7AsAnalog();
//     initDMA2Stream0(&adcValue, 1);
//     initADC3(1);
//     startADC3();
//
//     while(1)
//     {
//         delayMicroseconds(10000);
//         runMotorFromADC((uint32_t)adcValue);
//     }
// }


//PART 3
// int main(void)
// {
//     initTimer2();
//     initMotorController();
//
//     while(1)
//     {
//         runMotorController();
//     }
// }


