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
//     static volatile uint16_t adcValue;                    // One reading updated by DMA
//
//     initUSART3();                                         // Set up serial output to the computer
//     initTimer2();                                         // Start the microsecond timer
//     initGpioF7AsAnalog();                                 // Use PF7 as an analog input
//     initDMA2Stream0(&adcValue, 1);                        // Copy each ADC result into one variable
//     initADC3(1);                                          // Set ADC3 to repeatedly measure PF7
//     startADC3();                                          // Start measuring after DMA is ready
//
//     while(1)
//     {
//         delayMicroseconds(200000);                        // Wait 0.2 seconds between prints; ADC and DMA keep running
//         printfUSART3("PF7: %u\r\n", (uint32_t)adcValue);  // Print the saved ADC reading
//     }
// }


//PART 1.2
int main(void)
{
    static volatile uint16_t adcValuesFromDMA[3];  // 3 slots * 16 bits; volatile makes CPU read RAM again because DMA changes it
    uint32_t pf7;
    uint32_t pf8;
    uint32_t pf9;

    initUSART3();                          // Set up serial output to the computer
    initTimer2();                          // Start the microsecond timer
    initGpioF7AsAnalog();                  // Use PF7 as an analog input
    initGpioF8AsAnalog();                  // Use PF8 as an analog input
    initGpioF9AsAnalog();                  // Use PF9 as an analog input
    initDMA2Stream0(adcValuesFromDMA, 3);  // Prepare 3 slots before ADC starts; each ADC request advances one slot
    initADC3(3);                           // Set 3 readings in order: channel 5/PF7, channel 6/PF8, channel 7/PF9
    startADC3();                           // Start measuring after DMA is ready

    while(1)
    {
        delayMicroseconds(200000);                                     // Wait 0.2 seconds between prints; ADC and DMA keep running
        pf7 = adcValuesFromDMA[0];                                     // First ADC channel is PF7, so DMA slot [0] holds PF7; no pin ID is stored
        pf8 = adcValuesFromDMA[1];                                     // Second ADC channel is PF8, so DMA slot [1] holds PF8
        pf9 = adcValuesFromDMA[2];                                     // Third ADC channel is PF9, so DMA slot [2] holds PF9
        printfUSART3("PF7: %u, PF8: %u, PF9: %u\r\n", pf7, pf8, pf9);  // Print the saved ADC readings on one line
    }
}


//PART 2
// int main(void)
// {
//     static volatile uint16_t adcValue;        // One reading updated by DMA
//
//     initTimer2();                             // Start the microsecond timer
//     initMotor();
//     initGpioF7AsAnalog();                     // Use PF7 as an analog input
//     initDMA2Stream0(&adcValue, 1);            // Copy each ADC result into one variable
//     initADC3(1);                              // Set ADC3 to repeatedly measure PF7
//     startADC3();                              // Start measuring after DMA is ready
//
//     while(1)
//     {
//         delayMicroseconds(10000);             // Wait 10 ms between motor duty updates; ADC and DMA keep running
//         runMotorFromADC((uint32_t)adcValue);  // Scale the latest PF7 potentiometer reading to motor duty 0-1000
//     }
// }


//PART 3
// int main(void)
// {
//     initTimer2();              // Start the microsecond timer
//     initMotorController();     // Set up PWM, encoder and button; hold the starting position as target 0
//
//     while(1)
//     {
//         runMotorController();  // Check elapsed time, read encoder, then update motor drive toward the target
//     }
// }


