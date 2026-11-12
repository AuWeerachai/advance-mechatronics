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

#define PORTB_BASE_ADDRESS      GPIOB_BASE_MORT
#define PORTB_MODER_REGISTER  (PORTB_BASE_ADDRESS + 0x0)
#define PORTB_OTYPER_REGISTER (PORTB_BASE_ADDRESS + 0x4)
#define PORTB_PUPDR_REGISTER  (PORTB_BASE_ADDRESS + 0xC)
#define PORTB_AFR1_REGISTER (PORTB_BASE_ADDRESS + 0x20)
#define PORTC_BASE_ADDRESS		GPIOC_BASE_MORT
#define PORTC_MODER_REGISTER  (PORTC_BASE_ADDRESS + 0x0)
#define PORTC_PUPDR_REGISTER  (PORTC_BASE_ADDRESS + 0xC)
#define PORTC_IDR_REGISTER    (PORTC_BASE_ADDRESS + 0x10)
#define PORTD_BASE_ADDRESS		GPIOD_BASE_MORT
#define PORTD_MODER_REGISTER (PORTD_BASE_ADDRESS + 0x0)
#define PORTD_OTYPER_REGISTER (PORTD_BASE_ADDRESS + 0x4)
#define PORTD_OSPEEDR_REGISTER (PORTD_BASE_ADDRESS + 0x8)
#define PORTD_PUPDR_REGISTER (PORTD_BASE_ADDRESS + 0xC)
#define PORTD_ODR_REGISTER (PORTD_BASE_ADDRESS + 0x14)
#define PORTD_AFR2_REGISTER (PORTD_BASE_ADDRESS + 0x24)
#define GPIO_0_MODER                0x3
#define GPIO_0_MODER_AF             0x2
#define GPIO_8_MODER                ((uint32_t)0x30000)
#define GPIO_8_MODER_AF             ((uint32_t)0x20000)
#define GPIO_9_MODER                ((uint32_t)0xC0000)
#define GPIO_9_MODER_AF             ((uint32_t)0x80000)
#define GPIO_0_OTYPER               0x1
#define GPIO_0_OTYPER_PP            0x0
#define GPIO_8_OTYPER               ((uint32_t)0x100)
#define GPIO_8_OTYPER_PP            0x0
#define GPIO_9_OTYPER               ((uint32_t)0x200)
#define GPIO_9_OTYPER_PP            0x0
#define GPIO_8_OSPEEDR_HIGH_SPEED   ((uint32_t)0x30000)
#define GPIO_9_OSPEEDR_HIGH_SPEED   ((uint32_t)0xC0000)
#define GPIO_0_PUPDR                0x3
#define GPIO_0_PUPDR_NOPULL         0x0
#define GPIO_8_PUPDR                ((uint32_t)0x30000)
#define GPIO_8_PUPDR_PU            ((uint32_t)0x10000)
#define GPIO_9_PUPDR                ((uint32_t)0xC0000)
#define GPIO_9_PUPDR_PU            ((uint32_t)0x40000)
#define GPIO_0_AFR1_AF2             2
#define GPIO_0_AFR1                 0xF
#define GPIO_8_AFR2_SET_ALL				0xF  // 0xF + 0xF0 masks AFR2 bits 3-0 and 7-4: clear PD8/PD9 function selections
#define GPIO_8_AFR2_AF7					0x7
#define GPIO_9_AFR2_SET_ALL				((uint32_t)0xF0)
#define GPIO_9_AFR2_AF7				((uint32_t)0x70)

//PART 1.1
#define PORTF_MODER_REGISTER (GPIOF_BASE_MORT + 0x0)
#define PORTF_PUPDR_REGISTER (GPIOF_BASE_MORT + 0xC)

//PART 1.1
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

//PART 1.1
void initGpioF7AsAnalog(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);  // Turn on the clock for GPIOF

    reg_pointer = (volatile uint32_t *)PORTF_MODER_REGISTER;  // Select the pin mode register
    *reg_pointer = *reg_pointer & ~((uint32_t)0xC000);        // 0xC000 = 3 * 2^14; AND with its inverse makes bits 15-14 = 00 on PF7: clear mode before setting it
    *reg_pointer = *reg_pointer | ((uint32_t)0xC000);         // 0xC000 = 3 * 2^14; bits 15-14 = 11 selects analog mode on PF7

    reg_pointer = (volatile uint32_t *)PORTF_PUPDR_REGISTER;  // Select the pull-up and pull-down register
    *reg_pointer = *reg_pointer & ~((uint32_t)0xC000);        // 0xC000 = 3 * 2^14; AND with its inverse makes bits 15-14 = 00 on PF7: no pull-up or pull-down
}

//PART 1.2
void initGpioF8AsAnalog(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);  // Turn on the clock for GPIOF

    reg_pointer = (volatile uint32_t *)PORTF_MODER_REGISTER;  // Select the pin mode register
    *reg_pointer = *reg_pointer & ~((uint32_t)0x30000);       // 0x30000 = 3 * 2^16; AND with its inverse makes bits 17-16 = 00 on PF8: clear mode before setting it
    *reg_pointer = *reg_pointer | ((uint32_t)0x30000);        // 0x30000 = 3 * 2^16; bits 17-16 = 11 selects analog mode on PF8

    reg_pointer = (volatile uint32_t *)PORTF_PUPDR_REGISTER;  // Select the pull-up and pull-down register
    *reg_pointer = *reg_pointer & ~((uint32_t)0x30000);       // 0x30000 = 3 * 2^16; AND with its inverse makes bits 17-16 = 00 on PF8: no pull-up or pull-down
}

//PART 1.2
void initGpioF9AsAnalog(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);  // Turn on the clock for GPIOF

    reg_pointer = (volatile uint32_t *)PORTF_MODER_REGISTER;  // Select the pin mode register
    *reg_pointer = *reg_pointer & ~((uint32_t)0xC0000);       // 0xC0000 = 3 * 2^18; AND with its inverse makes bits 19-18 = 00 on PF9: clear mode before setting it
    *reg_pointer = *reg_pointer | ((uint32_t)0xC0000);        // 0xC0000 = 3 * 2^18; bits 19-18 = 11 selects analog mode on PF9

    reg_pointer = (volatile uint32_t *)PORTF_PUPDR_REGISTER;  // Select the pull-up and pull-down register
    *reg_pointer = *reg_pointer & ~((uint32_t)0xC0000);       // 0xC0000 = 3 * 2^18; AND with its inverse makes bits 19-18 = 00 on PF9: no pull-up or pull-down
}

//PART 2
void initGpioB0asAF2( void )
{

  volatile uint32_t* reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);   // Turn on the clock for GPIOB
  reg_pointer = (volatile uint32_t*) PORTB_MODER_REGISTER;  // Select the pin mode register
  *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_0_MODER);  // 0x3 = binary 11; clear PB0 mode bits 1-0 before selecting alternate function
  *reg_pointer = *reg_pointer | (uint32_t)GPIO_0_MODER_AF;  // 0x2 = binary 10 in MODER bits 1-0; PB0 uses the peripheral chosen in AFR1

    reg_pointer = (volatile uint32_t*) PORTB_OTYPER_REGISTER;      // Select the output type register
    *reg_pointer = *reg_pointer & ~((uint32_t) GPIO_0_OTYPER);     // 0x1 clears OTYPER bit 0: PB0 actively drives both high and low (push-pull)
    *reg_pointer = *reg_pointer | (uint32_t) GPIO_0_OTYPER_PP;     // PP = 0; OR adds no bits after the clear above, so PB0 stays push-pull
    reg_pointer = (volatile uint32_t*) PORTB_PUPDR_REGISTER;       // Select the pull-up and pull-down register
    *reg_pointer = *reg_pointer & ~((uint32_t) GPIO_0_PUPDR);      // 0x3 clears PUPDR bits 1-0 to 00: PB0 has no pull-up or pull-down
    *reg_pointer = *reg_pointer | (uint32_t) GPIO_0_PUPDR_NOPULL;  // NOPULL = 0; OR adds no bits after the clear above, keeping both pulls off
    reg_pointer = (volatile uint32_t*) PORTB_AFR1_REGISTER;        // Select the alternate function register for pins 0-7
    *reg_pointer = *reg_pointer & ~((uint32_t) GPIO_0_AFR1);       // 0xF = binary 1111; clear PB0 function selection in AFR1 bits 3-0
    *reg_pointer = *reg_pointer | (uint32_t) GPIO_0_AFR1_AF2;      // AFR1 bits 3-0 = 0010 (AF2); chip maps PB0 AF2 to Timer3 channel 3


}

//PART 2
void initGpioD14D15AsOutput(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);  // Turn on the clock for GPIOD

    reg_pointer = (volatile uint32_t *)PORTD_ODR_REGISTER;  // Select the output value register
    *reg_pointer = *reg_pointer & ~((uint32_t)0x4000);      // 0x4000 = 2^14; clear bit 14 to make PD14 low
    *reg_pointer = *reg_pointer & ~((uint32_t)0x8000);      // 0x8000 = 2^15; clear bit 15 to make PD15 low

    reg_pointer = (volatile uint32_t *)PORTD_MODER_REGISTER;  // Select the pin mode register
    *reg_pointer = *reg_pointer & ~((uint32_t)0x30000000);    // 0x30000000 = 3 * 2^28; AND with its inverse makes bits 29-28 = 00 on PD14: clear mode before setting it
    *reg_pointer = *reg_pointer | ((uint32_t)0x10000000);     // 0x10000000 = 1 * 2^28; bits 29-28 = 01 selects output mode on PD14
    *reg_pointer = *reg_pointer & ~((uint32_t)0xC0000000);    // 0xC0000000 = 3 * 2^30; AND with its inverse makes bits 31-30 = 00 on PD15: clear mode before setting it
    *reg_pointer = *reg_pointer | ((uint32_t)0x40000000);     // 0x40000000 = 1 * 2^30; bits 31-30 = 01 selects output mode on PD15

    reg_pointer = (volatile uint32_t *)PORTD_OTYPER_REGISTER;  // Select the output type register
    *reg_pointer = *reg_pointer & ~((uint32_t)0x4000);         // 0x4000 = 2^14; clear bit 14 to make PD14 push-pull
    *reg_pointer = *reg_pointer & ~((uint32_t)0x8000);         // 0x8000 = 2^15; clear bit 15 to make PD15 push-pull

    reg_pointer = (volatile uint32_t *)PORTD_PUPDR_REGISTER;  // Select the pull-up and pull-down register
    *reg_pointer = *reg_pointer & ~((uint32_t)0x30000000);    // 0x30000000 = 3 * 2^28; AND with its inverse makes bits 29-28 = 00 on PD14: no pull-up or pull-down
    *reg_pointer = *reg_pointer & ~((uint32_t)0xC0000000);    // 0xC0000000 = 3 * 2^30; AND with its inverse makes bits 31-30 = 00 on PD15: no pull-up or pull-down
}

//PART 2
void writeGpioD14(uint32_t value)
{
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)PORTD_ODR_REGISTER;  // Select the output value register
    if(value == 0)
    {
        *reg_pointer = *reg_pointer & ~((uint32_t)0x4000);  // 0x4000 = 2^14; clear bit 14 to make PD14 low
    }
    else
    {
        *reg_pointer = *reg_pointer | ((uint32_t)0x4000);   // 0x4000 = 2^14; set bit 14 to make PD14 high
    }
}

//PART 2
void writeGpioD15(uint32_t value)
{
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)PORTD_ODR_REGISTER;  // Select the output value register
    if(value == 0)
    {
        *reg_pointer = *reg_pointer & ~((uint32_t)0x8000);  // 0x8000 = 2^15; clear bit 15 to make PD15 low
    }
    else
    {
        *reg_pointer = *reg_pointer | ((uint32_t)0x8000);   // 0x8000 = 2^15; set bit 15 to make PD15 high
    }
}

//PART 3
void initGpioC6C7AsInput(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);  // Turn on the clock for GPIOC

    reg_pointer = (volatile uint32_t *)PORTC_MODER_REGISTER;  // Select the pin mode register
    *reg_pointer = *reg_pointer & ~((uint32_t)0x3000);        // 0x3000 = 3 * 2^12; AND with its inverse makes bits 13-12 = 00 on PC6: input mode
    *reg_pointer = *reg_pointer & ~((uint32_t)0xC000);        // 0xC000 = 3 * 2^14; AND with its inverse makes bits 15-14 = 00 on PC7: input mode

    reg_pointer = (volatile uint32_t *)PORTC_PUPDR_REGISTER;  // Select the pull-up and pull-down register
    *reg_pointer = *reg_pointer & ~((uint32_t)0x3000);        // 0x3000 = 3 * 2^12; AND with its inverse makes bits 13-12 = 00 on PC6: no pull-up or pull-down
    *reg_pointer = *reg_pointer & ~((uint32_t)0xC000);        // 0xC000 = 3 * 2^14; AND with its inverse makes bits 15-14 = 00 on PC7: no pull-up or pull-down
}

//PART 3
uint32_t readGpioC6C7(void)
{
    uint32_t value;
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)PORTC_IDR_REGISTER;  // Select the input value register
    value = *reg_pointer;
    value = value / 0x40;                                   // 0x40 = 64 = 2^6; divide by 64 so original PC6/PC7 bits become bits 0/1
    value = value & (uint32_t)3;                            // 3 = binary 11; keep PC7:PC6 only: 00=0, 01=1, 10=2, 11=3
    return value;
}

//PART 3
void initGpioC13AsInput(void)
{
    volatile uint32_t * reg_pointer;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);  // Turn on the clock for GPIOC

    reg_pointer = (volatile uint32_t *)PORTC_MODER_REGISTER;  // Select the pin mode register
    *reg_pointer = *reg_pointer & ~((uint32_t)0xC000000);     // 0xC000000 = 3 * 2^26; AND with its inverse makes bits 27-26 = 00 on PC13: input mode

    reg_pointer = (volatile uint32_t *)PORTC_PUPDR_REGISTER;  // Select the pull-up and pull-down register
    *reg_pointer = *reg_pointer & ~((uint32_t)0xC000000);     // 0xC000000 = 3 * 2^26; AND with its inverse makes bits 27-26 = 00 on PC13: no pull-up or pull-down
    *reg_pointer = *reg_pointer | ((uint32_t)0x8000000);      // 0x8000000 = 2 * 2^26; bits 27-26 = 10 selects pull-down on PC13
}

//PART 3
uint32_t readGpioC13(void)
{
    uint32_t value;
    volatile uint32_t * reg_pointer;

    reg_pointer = (volatile uint32_t *)PORTC_IDR_REGISTER;  // Select the input value register
    value = *reg_pointer & ((uint32_t)0x2000);              // 0x2000 = 2^13; keep only PC13: nonzero means button high, returned as 1
    if(value != 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
