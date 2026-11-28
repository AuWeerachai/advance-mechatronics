/**
  ******************************************************************************
  * @file    stm32f446xx_gpio_mort.c
  * @author  mortamar@andrew.cmu.edu
  * @version 2.0
  * @date    June-2026
  * @brief   Controls STM32F446ze GPIO
  ******************************************************************************
  */

#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_rcc_mort.h"

// Register addresses.
#define PORTC_BASE_ADDRESS GPIOC_BASE_MORT
#define PORTC_MODER_REGISTER (PORTC_BASE_ADDRESS + 0x0)
#define PORTC_PUPDR_REGISTER (PORTC_BASE_ADDRESS + 0xC)
#define PORTD_BASE_ADDRESS GPIOD_BASE_MORT
#define PORTD_MODER_REGISTER (PORTD_BASE_ADDRESS + 0x0)
#define PORTD_OTYPER_REGISTER (PORTD_BASE_ADDRESS + 0x4)
#define PORTD_OSPEEDR_REGISTER (PORTD_BASE_ADDRESS + 0x8)
#define PORTD_PUPDR_REGISTER (PORTD_BASE_ADDRESS + 0xC)
#define PORTD_AFR2_REGISTER (PORTD_BASE_ADDRESS + 0x24)
#define PORTF_MODER_REGISTER (GPIOF_BASE_MORT + 0x0)
#define PORTF_PUPDR_REGISTER (GPIOF_BASE_MORT + 0xC)
#define PORTF_IDR_REGISTER (GPIOF_BASE_MORT + 0x10)
#define PORTE_MODER_REGISTER (GPIOE_BASE_MORT + 0x00)
#define PORTE_OTYPER_REGISTER (GPIOE_BASE_MORT + 0x04)
#define PORTE_PUPDR_REGISTER (GPIOE_BASE_MORT + 0x0C)
#define PORTE_IDR_REGISTER (GPIOE_BASE_MORT + 0x10)
#define PORTE_AFR2_REGISTER (GPIOE_BASE_MORT + 0x24)
#define PORTA_MODER_REGISTER (GPIOA_BASE_MORT + 0x00)
#define PORTA_PUPDR_REGISTER (GPIOA_BASE_MORT + 0x0C)

// Pin masks and settings.
#define GPIO_8_MODER ((uint32_t)0x30000)
#define GPIO_8_MODER_AF ((uint32_t)0x20000)
#define GPIO_9_MODER ((uint32_t)0xC0000)
#define GPIO_9_MODER_AF ((uint32_t)0x80000)
#define GPIO_8_OTYPER ((uint32_t)0x100)
#define GPIO_8_OTYPER_PP 0x0
#define GPIO_9_OTYPER ((uint32_t)0x200)
#define GPIO_9_OTYPER_PP 0x0
#define GPIO_8_OSPEEDR_HIGH_SPEED ((uint32_t)0x30000)
#define GPIO_9_OSPEEDR_HIGH_SPEED ((uint32_t)0xC0000)
#define GPIO_8_PUPDR ((uint32_t)0x30000)
#define GPIO_8_PUPDR_PU ((uint32_t)0x10000)
#define GPIO_9_PUPDR ((uint32_t)0xC0000)
#define GPIO_9_PUPDR_PU ((uint32_t)0x40000)
#define GPIO_8_AFR2_SET_ALL 0xF  // 0xF + 0xF0 masks AFR2 bits 3-0 and 7-4: clear PD8/PD9 function selections
#define GPIO_8_AFR2_AF7 0x7
#define GPIO_9_AFR2_SET_ALL ((uint32_t)0xF0)
#define GPIO_9_AFR2_AF7 ((uint32_t)0x70)
#define GPIO_PA3_MODE_MASK ((uint32_t)0xC0)
#define GPIO_PC0_MODE_MASK ((uint32_t)0x3)
#define GPIO_PC3_MODE_MASK ((uint32_t)0xC0)
#define PORTF_MODER_PIN15_MASK ((uint32_t)0xC0000000)
#define PORTF_PUPDR_PIN15_MASK ((uint32_t)0xC0000000)
#define PORTF_PUPDR_PIN15_PULLUP ((uint32_t)0x40000000)
#define PORTF_IDR_PIN15_MASK ((uint32_t)0x8000)
#define PORTE_MODER_PIN13_MASK ((uint32_t)0x0C000000)
#define PORTE_PUPDR_PIN13_MASK ((uint32_t)0x0C000000)
#define PORTE_PUPDR_PIN13_PULLUP ((uint32_t)0x04000000)
#define PORTE_IDR_PIN13_MASK ((uint32_t)0x2000)

// D6 / PE9: fan PWM on Timer 1 channel 1, AF1.
#define PORTE_MODER_PIN9_MASK ((uint32_t)0xC0000)
#define PORTE_MODER_PIN9_AF ((uint32_t)0x80000)
#define PORTE_OTYPER_PIN9_OPEN_DRAIN ((uint32_t)0x200)
#define PORTE_PUPDR_PIN9_MASK ((uint32_t)0xC0000)
#define PORTE_AFR2_PIN9_MASK ((uint32_t)0xF0)
#define PORTE_AFR2_PIN9_AF1 ((uint32_t)0x10)

// PD12: digital input for the fan tachometer interrupt.
#define PORTD_MODER_PIN12_MASK ((uint32_t)0x03000000)
#define PORTD_PUPDR_PIN12_MASK ((uint32_t)0x03000000)

// USART3 serial communication.
void initGpioD8D9AsAF7( void )
{
      volatile uint32_t  * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);  // Turn on the clock for GPIOD

    reg_pointer = (volatile uint32_t *)PORTD_MODER_REGISTER;                             // Select the pin mode register
    *reg_pointer = *reg_pointer & (~((uint32_t)GPIO_8_MODER + (uint32_t)GPIO_9_MODER));  // 0x30000 + 0xC0000 masks bits 17-16 and 19-18; clear both PD8/PD9 mode fields
    *reg_pointer = *reg_pointer | GPIO_8_MODER_AF | GPIO_9_MODER_AF;                     // MODER bits 17-16 and 19-18 = 10: PD8 and PD9 use the peripheral selected in AFR2

    reg_pointer = (volatile uint32_t *)PORTD_OTYPER_REGISTER;                              // Select the output type register
    *reg_pointer = *reg_pointer & (~((uint32_t)GPIO_8_OTYPER + (uint32_t)GPIO_9_OTYPER));  // Clear OTYPER bits 8 and 9: PD8/PD9 use push-pull, not open-drain
    *reg_pointer = *reg_pointer | GPIO_8_OTYPER_PP | GPIO_9_OTYPER_PP;                     // Both PP constants = 0; OR adds no bits after the clear above

    reg_pointer = (volatile uint32_t *)PORTD_OSPEEDR_REGISTER;                                                // Select the output speed register
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_8_OSPEEDR_HIGH_SPEED | (uint32_t)GPIO_9_OSPEEDR_HIGH_SPEED;  // OSPEEDR fields for pins 8/9 = 11: fastest output-edge setting, not USART baud rate

    reg_pointer = (volatile uint32_t *)PORTD_PUPDR_REGISTER;                              // Select the pull-up and pull-down register
    *reg_pointer = *reg_pointer & (~((uint32_t)GPIO_8_PUPDR + (uint32_t)GPIO_9_PUPDR));   // Clear PUPDR bits 17-16 and 19-18 before setting the pull-up fields
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_8_PUPDR_PU | (uint32_t)GPIO_9_PUPDR_PU;  // PUPDR bits 17-16 and 19-18 = 01: enable pull-ups on PD8/PD9

     reg_pointer = (volatile uint32_t *)PORTD_AFR2_REGISTER;                                            // Select the alternate function register for pins 8-15
    *reg_pointer = *reg_pointer & (~((uint32_t)GPIO_8_AFR2_SET_ALL	+ (uint32_t)GPIO_9_AFR2_SET_ALL));  // 0xF + 0xF0 masks AFR2 bits 3-0 and 7-4: clear PD8/PD9 function selections
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_8_AFR2_AF7 | (uint32_t)GPIO_9_AFR2_AF7;                // 7 in bits 3-0 selects PD8 AF7 = USART3 TX; 0x70 puts 7 in bits 7-4 for PD9 RX

}

// Analog inputs: ball, hand and lever.
void initGpioA3AsAnalog(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    reg_pointer = (volatile uint32_t *)PORTA_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~GPIO_PA3_MODE_MASK;
    *reg_pointer = *reg_pointer | GPIO_PA3_MODE_MASK;  // PA3 mode = 11, analog.
    reg_pointer = (volatile uint32_t *)PORTA_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~GPIO_PA3_MODE_MASK; // No internal pull.
}

// Analog inputs: hand 
void initGpioC0AsAnalog(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    reg_pointer = (volatile uint32_t *)PORTC_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~GPIO_PC0_MODE_MASK;
    *reg_pointer = *reg_pointer | GPIO_PC0_MODE_MASK;  // PC0 mode = 11, analog.
    reg_pointer = (volatile uint32_t *)PORTC_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~GPIO_PC0_MODE_MASK; // No internal pull.
}

//Analog inputs: lever.
void initGpioC3AsAnalog(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    reg_pointer = (volatile uint32_t *)PORTC_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~GPIO_PC3_MODE_MASK;
    *reg_pointer = *reg_pointer | GPIO_PC3_MODE_MASK;  // PC3 mode = 11, analog.
    reg_pointer = (volatile uint32_t *)PORTC_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~GPIO_PC3_MODE_MASK; // No internal pull.
}

// Clock-arm START sensor inputs.
void initGpioF15AsInput(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    reg_pointer = (volatile uint32_t *)PORTF_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~PORTF_MODER_PIN15_MASK; // Bits 31:30 = 00: input.

    reg_pointer = (volatile uint32_t *)PORTF_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~PORTF_PUPDR_PIN15_MASK;
    *reg_pointer = *reg_pointer | PORTF_PUPDR_PIN15_PULLUP; // Bits 31:30 = 01: pull up to 3.3 V.
}

// Clock-arm END sensor inputs.
void initGpioE13AsInput(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    reg_pointer = (volatile uint32_t *)PORTE_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~PORTE_MODER_PIN13_MASK; // Bits 27:26 = 00: input.

    reg_pointer = (volatile uint32_t *)PORTE_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~PORTE_PUPDR_PIN13_MASK;
    *reg_pointer = *reg_pointer | PORTE_PUPDR_PIN13_PULLUP; // Bits 27:26 = 01: pull up to 3.3 V.
}


uint32_t readGpioF15(void)
{
    uint32_t value;
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)PORTF_IDR_REGISTER;
    value = *reg_pointer & PORTF_IDR_PIN15_MASK;
    if(value != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

uint32_t readGpioE13(void)
{
    uint32_t value;
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)PORTE_IDR_REGISTER;
    value = *reg_pointer & PORTE_IDR_PIN13_MASK;
    if(value != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

// Fan PWM control input; the fan supplies the pull-up.
void initGpioE9AsFanOutput(void)
{
    volatile uint32_t * reg_pointer;

    // Use open-drain for the four-wire fan's PWM control input.
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
    reg_pointer = (volatile uint32_t *)PORTE_OTYPER_REGISTER;
    *reg_pointer = *reg_pointer | PORTE_OTYPER_PIN9_OPEN_DRAIN;
    reg_pointer = (volatile uint32_t *)PORTE_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~PORTE_PUPDR_PIN9_MASK;
    reg_pointer = (volatile uint32_t *)PORTE_AFR2_REGISTER;
    *reg_pointer = *reg_pointer & ~PORTE_AFR2_PIN9_MASK;
    *reg_pointer = *reg_pointer | PORTE_AFR2_PIN9_AF1;
    reg_pointer = (volatile uint32_t *)PORTE_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~PORTE_MODER_PIN9_MASK;
    *reg_pointer = *reg_pointer | PORTE_MODER_PIN9_AF;
}

void initGpioD12AsFanTachInput(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    reg_pointer = (volatile uint32_t *)PORTD_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~PORTD_MODER_PIN12_MASK; // Bits 25:24 = 00: input.
    reg_pointer = (volatile uint32_t *)PORTD_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~PORTD_PUPDR_PIN12_MASK; // External pull-up to 3.3 V.
}
