/**
  ******************************************************************************
  * @file    stm32f446xx_gpio_mort.c 
  * @author  mortamar@andrew.cmu.edu
  * @version 2.0
  * @date    June-2026
  * @brief   Controls STM32F446ze GPIO
  ******************************************************************************
  */

/* Includes----------------------------------------------------------*/
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_rcc_mort.h"

#define PORTB_BASE_ADDRESS      GPIOB_BASE_MORT
#define PORTB_MODER_REGISTER  (PORTB_BASE_ADDRESS + 0x00)
#define PORTB_OTYPER_REGISTER (PORTB_BASE_ADDRESS + 0x04)
#define PORTB_PUPDR_REGISTER  (PORTB_BASE_ADDRESS + 0x0C)
#define PORTB_AFR1_REGISTER (PORTB_BASE_ADDRESS + 0x20)
#define PORTC_BASE_ADDRESS		GPIOC_BASE_MORT
#define PORTC_MODER_REGISTER  (PORTC_BASE_ADDRESS + 0x00)
#define PORTC_PUPDR_REGISTER  (PORTC_BASE_ADDRESS + 0x0C)
#define PORTC_IDR_REGISTER    (PORTC_BASE_ADDRESS + 0x10)
#define PORTD_BASE_ADDRESS		GPIOD_BASE_MORT
#define PORTD_MODER_REGISTER (PORTD_BASE_ADDRESS + 0x00)
#define PORTD_OTYPER_REGISTER (PORTD_BASE_ADDRESS + 0x04)
#define PORTD_OSPEEDR_REGISTER (PORTD_BASE_ADDRESS + 0x08)
#define PORTD_PUPDR_REGISTER (PORTD_BASE_ADDRESS + 0x0C)
#define PORTD_ODR_REGISTER (PORTD_BASE_ADDRESS + 0x14)
#define PORTD_AFR2_REGISTER (PORTD_BASE_ADDRESS + 0x24)
#define GPIO_0_MODER                0x03
#define GPIO_0_MODER_AF             0x02
#define GPIO_8_MODER                (uint32_t)(0x03<<16)
#define GPIO_8_MODER_AF             (uint32_t)(0x02<<16)
#define GPIO_9_MODER                (uint32_t)(0x03<<18)
#define GPIO_9_MODER_AF             (uint32_t)(0x02<<18)
#define GPIO_0_OTYPER               0x01
#define GPIO_0_OTYPER_PP            0x00
#define GPIO_8_OTYPER               (((uint32_t)0x01)<<8)
#define GPIO_8_OTYPER_PP            0x00
#define GPIO_9_OTYPER               (((uint32_t)0x01)<<9)
#define GPIO_9_OTYPER_PP            0x00
#define GPIO_8_OSPEEDR_HIGH_SPEED   (((uint32_t)0x03)<<16)
#define GPIO_9_OSPEEDR_HIGH_SPEED   (((uint32_t)0x03)<<18)
#define GPIO_0_PUPDR                0x03
#define GPIO_0_PUPDR_NOPULL         0x00
#define GPIO_8_PUPDR                (uint32_t)(0x03<<16)
#define GPIO_8_PUPDR_PU            (uint32_t)(0x01<<16)
#define GPIO_9_PUPDR                (uint32_t)(0x03<<18)
#define GPIO_9_PUPDR_PU            (uint32_t)(0x01<<18)
#define GPIO_0_AFR1_AF2             2
#define GPIO_0_AFR1                 0xF
#define GPIO_8_AFR2_SET_ALL				0xF
#define GPIO_8_AFR2_AF7					0x7
#define GPIO_9_AFR2_SET_ALL				(uint32_t)(0xF<<4)
#define GPIO_9_AFR2_AF7				(uint32_t)(0x7<<4)

//PART 1.1
#define PORTF_MODER_REGISTER (GPIOF_BASE_MORT + 0x00)
#define PORTF_PUPDR_REGISTER (GPIOF_BASE_MORT + 0x0C)

//PART 1.1
void initGpioD8D9AsAF7( void )
{
      volatile uint32_t  * reg_pointer;
      
      /* GPIOD Peripheral clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    /* GPIO D 8 and 9 as alternative function */ 
    reg_pointer = (volatile uint32_t *)PORTD_MODER_REGISTER;
    *reg_pointer = *reg_pointer & (~((uint32_t)GPIO_8_MODER + (uint32_t)GPIO_9_MODER));
    *reg_pointer = *reg_pointer | GPIO_8_MODER_AF | GPIO_9_MODER_AF;
    
    /* GPIOD pin 8 and 9 push pull -> no, open drain-> No another thing says yes to push pull!*/
    reg_pointer = (volatile uint32_t *)PORTD_OTYPER_REGISTER;
    *reg_pointer = *reg_pointer & (~((uint32_t)GPIO_8_OTYPER + (uint32_t)GPIO_9_OTYPER));
    *reg_pointer = *reg_pointer | GPIO_8_OTYPER_PP | GPIO_9_OTYPER_PP;
    
    /* GPIOD pin 8 and 9 as high speed */
    reg_pointer = (volatile uint32_t *)PORTD_OSPEEDR_REGISTER;
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_8_OSPEEDR_HIGH_SPEED | (uint32_t)GPIO_9_OSPEEDR_HIGH_SPEED;
    
    /* GPIOD pin 8 and 9 pulled up */
    reg_pointer = (volatile uint32_t *)PORTD_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & (~((uint32_t)GPIO_8_PUPDR + (uint32_t)GPIO_9_PUPDR));
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_8_PUPDR_PU | (uint32_t)GPIO_9_PUPDR_PU;
    
     /*GPIO D 8 and 9 as alternative function 7*/   
     reg_pointer = (volatile uint32_t *)PORTD_AFR2_REGISTER;
    *reg_pointer = *reg_pointer & (~((uint32_t)GPIO_8_AFR2_SET_ALL	+ (uint32_t)GPIO_9_AFR2_SET_ALL));
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_8_AFR2_AF7 | (uint32_t)GPIO_9_AFR2_AF7;    
	
}

//PART 1.1
void initGpioF7AsAnalog(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    reg_pointer = (volatile uint32_t *)PORTF_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 14);
    *reg_pointer = *reg_pointer | ((uint32_t)3 << 14);

    reg_pointer = (volatile uint32_t *)PORTF_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 14);
}

//PART 1.2
void initGpioF8AsAnalog(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    reg_pointer = (volatile uint32_t *)PORTF_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 16);
    *reg_pointer = *reg_pointer | ((uint32_t)3 << 16);

    reg_pointer = (volatile uint32_t *)PORTF_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 16);
}

//PART 1.2
void initGpioF9AsAnalog(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    reg_pointer = (volatile uint32_t *)PORTF_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 18);
    *reg_pointer = *reg_pointer | ((uint32_t)3 << 18);

    reg_pointer = (volatile uint32_t *)PORTF_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 18);
}

//PART 2
void initGpioB0asAF2( void )
{

  volatile uint32_t* reg_pointer;

  /* GPIOB Peripheral clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);


  /* GPIOB0 configuring mode */
  //config PB0 mode as alternate function. Connect PB0 internally to some function
  reg_pointer = (volatile uint32_t*) PORTB_MODER_REGISTER; 
  *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_0_MODER);
  *reg_pointer = *reg_pointer | (uint32_t)GPIO_0_MODER_AF;

  /*GPIOB0 configured as push-pull */
    reg_pointer = (volatile uint32_t*) PORTB_OTYPER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t) GPIO_0_OTYPER);       
    *reg_pointer = *reg_pointer | (uint32_t) GPIO_0_OTYPER_PP;

  /*GPIOB0 configured floating */
    // PB0 configured with no pull-up or pull-down. 
    reg_pointer = (volatile uint32_t*) PORTB_PUPDR_REGISTER;         
    *reg_pointer = *reg_pointer & ~((uint32_t) GPIO_0_PUPDR);       
    *reg_pointer = *reg_pointer | (uint32_t) GPIO_0_PUPDR_NOPULL;    

  //configure to connect to timer 3 channel 3
  //config alternate function to be alternate function 2 == timer 3 channel 3
  //PB0’s output is now internally connected to the timer 3 channel 3 output
    reg_pointer = (volatile uint32_t*) PORTB_AFR1_REGISTER;         
    *reg_pointer = *reg_pointer & ~((uint32_t) GPIO_0_AFR1);       
    *reg_pointer = *reg_pointer | (uint32_t) GPIO_0_AFR1_AF2;  


}

//PART 2
void initGpioD14D15AsOutput(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    reg_pointer = (volatile uint32_t *)PORTD_ODR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)1 << 14);
    *reg_pointer = *reg_pointer & ~((uint32_t)1 << 15);

    reg_pointer = (volatile uint32_t *)PORTD_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 28);
    *reg_pointer = *reg_pointer | ((uint32_t)1 << 28);
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 30);
    *reg_pointer = *reg_pointer | ((uint32_t)1 << 30);

    reg_pointer = (volatile uint32_t *)PORTD_OTYPER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)1 << 14);
    *reg_pointer = *reg_pointer & ~((uint32_t)1 << 15);

    reg_pointer = (volatile uint32_t *)PORTD_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 28);
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 30);
}

//PART 2
void writeGpioD14(uint32_t value)
{
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)PORTD_ODR_REGISTER;
    if(value == 0)
    {
        *reg_pointer = *reg_pointer & ~((uint32_t)1 << 14);
    }
    else
    {
        *reg_pointer = *reg_pointer | ((uint32_t)1 << 14);
    }
}

//PART 2
void writeGpioD15(uint32_t value)
{
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)PORTD_ODR_REGISTER;
    if(value == 0)
    {
        *reg_pointer = *reg_pointer & ~((uint32_t)1 << 15);
    }
    else
    {
        *reg_pointer = *reg_pointer | ((uint32_t)1 << 15);
    }
}

//PART 3
void initGpioC6C7AsInput(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    reg_pointer = (volatile uint32_t *)PORTC_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 12);
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 14);

    reg_pointer = (volatile uint32_t *)PORTC_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 12);
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 14);
}

//PART 3
uint32_t readGpioC6C7(void)
{
    uint32_t value;
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)PORTC_IDR_REGISTER;
    value = *reg_pointer;
    value = value >> 6;
    value = value & (uint32_t)3;
    return value;
}

//PART 3
void initGpioC13AsInput(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

    reg_pointer = (volatile uint32_t *)PORTC_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 26);

    reg_pointer = (volatile uint32_t *)PORTC_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)3 << 26);
    *reg_pointer = *reg_pointer | ((uint32_t)2 << 26);
}

//PART 3
uint32_t readGpioC13(void)
{
    uint32_t value;
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)PORTC_IDR_REGISTER;
    value = *reg_pointer & ((uint32_t)1 << 13);
    if(value != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
