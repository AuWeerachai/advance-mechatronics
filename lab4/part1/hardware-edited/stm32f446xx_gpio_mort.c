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



/* MACRO definitions----------------------------------------------------------*/
//Port A addresses:




//Port B addresses: //Fill the table of Port B addresses
//#define PORTB_BASE_ADDRESS ((uint32_t)0x40020400)
#define PORTB_BASE_ADDRESS      GPIOB_BASE_MORT


// NOTE 1.1
// STM32 uses byte-addressed memory
// 0x14 = 1*16 + 4*1 = 20, so this offset is 20 bytes from the base.
// Each definition gives the starting address of each register. (registor can hold more than 1 byte)
#define PORTB_MODER_REGISTER  (PORTB_BASE_ADDRESS + 0x00)
#define PORTB_OTYPER_REGISTER (PORTB_BASE_ADDRESS + 0x04)
#define PORTB_PUPDR_REGISTER  (PORTB_BASE_ADDRESS + 0x0C)
#define PORTB_ODR_REGISTER    (PORTB_BASE_ADDRESS + 0x14)

// Note 1.3
#define PORTB_AFR1_REGISTER (PORTB_BASE_ADDRESS + 0x20)


//Port C addresses: //Fill the table of Port C addresses
//#define PORTC_BASE_ADDRESS ((uint32_t)0x40020800)
#define PORTC_BASE_ADDRESS		GPIOC_BASE_MORT

//NOTE 1.2
#define PORTC_MODER_REGISTER  (PORTC_BASE_ADDRESS + 0x00)
#define PORTC_OTYPER_REGISTER (PORTC_BASE_ADDRESS + 0x04)
#define PORTC_PUPDR_REGISTER  (PORTC_BASE_ADDRESS + 0x0C)
#define PORTC_IDR_REGISTER    (PORTC_BASE_ADDRESS + 0x10)

//NOTE 1.4
#define PORTC_AFR1_REGISTER (PORTC_BASE_ADDRESS + 0x20)


//Port D addresses: //These are needed for communication do not edit
//#define PORTD_BASE_ADDRESS ((uint32_t)0x40020C00U)
#define PORTD_BASE_ADDRESS		GPIOD_BASE_MORT
#define PORTD_MODER_REGISTER (PORTD_BASE_ADDRESS + 0x00)
#define PORTD_OTYPER_REGISTER (PORTD_BASE_ADDRESS + 0x04)
#define PORTD_OSPEEDR_REGISTER (PORTD_BASE_ADDRESS + 0x08)
#define PORTD_PUPDR_REGISTER (PORTD_BASE_ADDRESS + 0x0C)
#define PORTD_IDR_REGISTER (PORTD_BASE_ADDRESS + 0x10)
#define PORTD_ODR_REGISTER (PORTD_BASE_ADDRESS + 0x14)
#define PORTD_BSRRL_REGISTER (PORTD_BASE_ADDRESS + 0x18)
#define PORTD_BSRR_REGISTER (PORTD_BASE_ADDRESS + 0x18)
#define PORTD_BSRRH_REGISTER (PORTD_BASE_ADDRESS + 0x1A)
#define PORTD_LCKR_REGISTER (PORTD_BASE_ADDRESS + 0x1C)
#define PORTD_AFR1_REGISTER (PORTD_BASE_ADDRESS + 0x20)
#define PORTD_AFR2_REGISTER (PORTD_BASE_ADDRESS + 0x24)
#define PORTD_OSPEEDR_REGISTER (PORTD_BASE_ADDRESS + 0x08)

//Port E addresses:

//Port F addresses:


//flags MODER Register:
#define GPIO_0_MODER                0x03
#define GPIO_0_MODER_OUT            0x01
#define GPIO_0_MODER_AF             0x02
#define GPIO_1_MODER                (uint32_t)(0x03<<2)
#define GPIO_1_MODER_OUT            (uint32_t)(0x01<<2)
#define GPIO_1_MODER_AF             (uint32_t)(0x02<<2)
#define GPIO_2_MODER                (uint32_t)(0x03<<4)
#define GPIO_2_MODER_OUT            (uint32_t)(0x01<<4)
#define GPIO_2_MODER_AF             (uint32_t)(0x02<<4)
#define GPIO_3_MODER                (uint32_t)(0x03<<6)
#define GPIO_3_MODER_OUT            (uint32_t)(0x01<<6)
#define GPIO_3_MODER_AF             (uint32_t)(0x02<<6)
#define GPIO_4_MODER                (uint32_t)(0x03<<8)
#define GPIO_4_MODER_OUT            (uint32_t)(0x01<<8)
#define GPIO_4_MODER_AF             (uint32_t)(0x02<<8)
#define GPIO_5_MODER                (uint32_t)(0x03<<10)
#define GPIO_5_MODER_OUT            (uint32_t)(0x01<<10)
#define GPIO_5_MODER_AF             (uint32_t)(0x02<<10)
#define GPIO_6_MODER                0x3000
#define GPIO_6_MODER_AF             0x2000
#define GPIO_6_MODER_OUT            0x1000
#define GPIO_6_MODER_IN             0x0000
#define GPIO_6_MODER_AN             0x3000
#define GPIO_7_MODER                (((uint32_t)0x03)<<14)
#define GPIO_7_MODER_OUT            (uint32_t)(0x01<<14)
#define GPIO_7_MODER_AF             (uint32_t)(0x02<<14)
#define GPIO_8_MODER                (uint32_t)(0x03<<16)
#define GPIO_8_MODER_OUT            (uint32_t)(0x01<<16)
#define GPIO_8_MODER_AF             (uint32_t)(0x02<<16)
#define GPIO_9_MODER                (uint32_t)(0x03<<18)
#define GPIO_9_MODER_OUT            (uint32_t)(0x01<<18)
#define GPIO_9_MODER_AF             (uint32_t)(0x02<<18)
#define GPIO_10_MODER                (uint32_t)(0x03<<20)
#define GPIO_10_MODER_OUT            (uint32_t)(0x01<<20)
#define GPIO_10_MODER_AF             (uint32_t)(0x02<<20)
#define GPIO_10_MODER                (uint32_t)(0x03<<20)
#define GPIO_10_MODER_OUT            (uint32_t)(0x01<<20)
#define GPIO_10_MODER_AF             (uint32_t)(0x02<<20)
#define GPIO_14_MODER                (((uint32_t)0x03)<<28)
#define GPIO_14_MODER_OUT            (((uint32_t)0x01)<<28)
#define GPIO_14_MODER_AF             (((uint32_t)0x02)<<28)
#define GPIO_15_MODER                (((uint32_t)0x03)<<30)
#define GPIO_15_MODER_OUT            (((uint32_t)0x01)<<30)
#define GPIO_15_MODER_AF             (((uint32_t)0x02)<<30)
//flags OTYPER Register:
#define GPIO_0_OTYPER               0x01
#define GPIO_0_OTYPER_PP            0x00
#define GPIO_1_OTYPER               (uint32_t)(0x01<<1)
#define GPIO_1_OTYPER_PP            0x00
#define GPIO_2_OTYPER               (uint32_t)(0x01<<2)
#define GPIO_2_OTYPER_PP            0x00
#define GPIO_3_OTYPER               (uint32_t)(0x01<<3)
#define GPIO_3_OTYPER_PP            0x00
#define GPIO_4_OTYPER               (uint32_t)(0x01<<4)
#define GPIO_4_OTYPER_PP            0x00
#define GPIO_5_OTYPER               (uint32_t)(0x01<<5)
#define GPIO_5_OTYPER_PP            0x00
#define GPIO_6_OTYPER               0x40
#define GPIO_6_OTYPER_PP            0x00
#define GPIO_7_OTYPER               (((uint32_t)0x01)<<7)
#define GPIO_7_OTYPER_PP            0x00
#define GPIO_8_OTYPER               (((uint32_t)0x01)<<8)
#define GPIO_8_OTYPER_PP            0x00
#define GPIO_9_OTYPER               (((uint32_t)0x01)<<9)
#define GPIO_9_OTYPER_PP            0x00
#define GPIO_10_OTYPER               (((uint32_t)0x01)<<10)
#define GPIO_10_OTYPER_PP            0x00
#define GPIO_14_OTYPER               (((uint32_t)0x01)<<14)
#define GPIO_14_OTYPER_PP            0x00
#define GPIO_15_OTYPER               (((uint32_t)0x01)<<15)
#define GPIO_15_OTYPER_PP            0x00
//flags OSPEEDR Register:
#define GPIO_0_OSPEEDR_HIGH_SPEED   3 
#define GPIO_6_OSPEEDR              0x3000
#define GPIO_6_OSPEEDR_HIGH_SPEED   0x3000
#define GPIO_8_OSPEEDR              (((uint32_t)0x03)<<16)
#define GPIO_8_OSPEEDR_FAST_SPEED   (((uint32_t)0x02)<<16)
#define GPIO_8_OSPEEDR_HIGH_SPEED   (((uint32_t)0x03)<<16)
#define GPIO_9_OSPEEDR              (((uint32_t)0x03)<<18)
#define GPIO_9_OSPEEDR_FAST_SPEED   (((uint32_t)0x02)<<18)
#define GPIO_9_OSPEEDR_HIGH_SPEED   (((uint32_t)0x03)<<18)
//flags PUPDR Register:
#define GPIO_0_PUPDR                0x03
#define GPIO_0_PUPDR_NOPULL         0x00
#define GPIO_0_PUPDR_PU            0x01
#define GPIO_0_PUPDR_PD            0x02
#define GPIO_1_PUPDR                (uint32_t)(0x03<<2)
#define GPIO_1_PUPDR_NOPULL         0x00
#define GPIO_1_PUPDR_PU            (uint32_t)(0x01<<2)
#define GPIO_1_PUPDR_PD            (uint32_t)(0x02<<2)
#define GPIO_2_PUPDR                (uint32_t)(0x03<<4)
#define GPIO_2_PUPDR_NOPULL         0x00
#define GPIO_2_PUPDR_PU            (uint32_t)(0x01<<4)
#define GPIO_2_PUPDR_PD            (uint32_t)(0x02<<4)
#define GPIO_3_PUPDR                (uint32_t)(0x03<<6)
#define GPIO_3_PUPDR_NOPULL         0x00
#define GPIO_3_PUPDR_PU            (uint32_t)(0x01<<6)
#define GPIO_3_PUPDR_PD            (uint32_t)(0x02<<6)
#define GPIO_4_PUPDR                (uint32_t)(0x03<<8)
#define GPIO_4_PUPDR_NOPULL         0x00
#define GPIO_4_PUPDR_PU            (uint32_t)(0x01<<8)
#define GPIO_4_PUPDR_PD            (uint32_t)(0x02<<8)
#define GPIO_5_PUPDR                (uint32_t)(0x03<<10)
#define GPIO_5_PUPDR_NOPULL         0x00
#define GPIO_5_PUPDR_PU            (uint32_t)(0x01<<10)
#define GPIO_5_PUPDR_PD            (uint32_t)(0x02<<10)
#define GPIO_6_PUPDR                0x3000
#define GPIO_6_PUPDR_NOPULL         0x00
#define GPIO_6_PUPDR_PD             0x2000
#define GPIO_6_PUPDR_PU             0x1000
#define GPIO_7_PUPDR                (((uint32_t)0x03)<<14)
#define GPIO_7_PUPDR_NOPULL         0x00
#define GPIO_7_PUPDR_PU            (uint32_t)(0x01<<14)
#define GPIO_7_PUPDR_PD            (uint32_t)(0x02<<14)
#define GPIO_8_PUPDR                (uint32_t)(0x03<<16)
#define GPIO_8_PUPDR_NOPULL         0x00
#define GPIO_8_PUPDR_PU            (uint32_t)(0x01<<16)
#define GPIO_8_PUPDR_PD            (uint32_t)(0x02<<16)
#define GPIO_9_PUPDR                (uint32_t)(0x03<<18)
#define GPIO_9_PUPDR_NOPULL         0x00
#define GPIO_9_PUPDR_PU            (uint32_t)(0x01<<18)
#define GPIO_9_PUPDR_PD            (uint32_t)(0x02<<18)
#define GPIO_10_PUPDR                (uint32_t)(0x03<<20)
#define GPIO_10_PUPDR_NOPULL         0x00
#define GPIO_10_PUPDR_PU            (uint32_t)(0x01<<20)
#define GPIO_10_PUPDR_PD            (uint32_t)(0x02<<20)
#define GPIO_14_PUPDR                (((uint32_t)0x03)<<28)
#define GPIO_14_PUPDR_NOPULL         0x00
#define GPIO_14_PUPDR_PU            (((uint32_t)0x01)<<28)
#define GPIO_14_PUPDR_PD            (((uint32_t)0x02)<<28)
#define GPIO_15_PUPDR                (((uint32_t)0x03)<<30)
#define GPIO_15_PUPDR_NOPULL         0x00
#define GPIO_15_PUPDR_PU            (((uint32_t)0x01)<<30)
#define GPIO_15_PUPDR_PD            (((uint32_t)0x02)<<30)
//input data register:
#define GPIO_0_IDR                0x01
#define GPIO_1_IDR               (uint32_t)(0x01<<1)
#define GPIO_2_IDR               (uint32_t)(0x01<<2)
#define GPIO_3_IDR               (uint32_t)(0x01<<3)
#define GPIO_4_IDR               (uint32_t)(0x01<<4)
#define GPIO_5_IDR               (uint32_t)(0x01<<5)
#define GPIO_6_IDR               0x40
#define GPIO_7_IDR               (uint32_t)(0x01<<7)
#define GPIO_8_IDR               (uint32_t)(0x01<<8)
#define GPIO_9_IDR               (uint32_t)(0x01<<9)
#define GPIO_10_IDR              (uint32_t)(0x01<<10)
#define GPIO_14_IDR              (((uint32_t)0x01)<<14)
#define GPIO_15_IDR              (((uint32_t)0x01)<<15)
//flags AFR1 Register:
#define GPIO_0_AFR1_AF2             2
#define GPIO_0_AFR1                 0xF
#define GPIO_6_AFR1                 0xF000000
#define GPIO_6_AFR1_AF2             0x2000000
//flags AFR2 or AFRH Register
#define GPIO_8_AFR2_SET_ALL				0xF
#define GPIO_8_AFR2_AF7					0x7
#define GPIO_9_AFR2_SET_ALL				(uint32_t)(0xF<<4)
#define GPIO_9_AFR2_AF7				(uint32_t)(0x7<<4)

//flags ODR Register:
#define GPIO_0_ODR_HIGH                0x01
#define GPIO_1_ODR_HIGH               (uint32_t)(0x01<<1)
#define GPIO_2_ODR_HIGH               (uint32_t)(0x01<<2)
#define GPIO_3_ODR_HIGH               (uint32_t)(0x01<<3)
#define GPIO_4_ODR_HIGH               (uint32_t)(0x01<<4)
#define GPIO_5_ODR_HIGH               (uint32_t)(0x01<<5)
#define GPIO_6_ODR_HIGH                0x40
#define GPIO_7_ODR_HIGH               (uint32_t)(0x01<<7)
#define GPIO_8_ODR_HIGH               (uint32_t)(0x01<<8)
#define GPIO_9_ODR_HIGH               (uint32_t)(0x01<<9)
#define GPIO_10_ODR_HIGH              (uint32_t)(0x01<<10)

/* function definitions----------------------------------------------------------*/

/* hardware initializations */
/**
  * @brief  Initializes GPIO B0 to be an output
  * @note   GPIO B0 is the one connected to LED 1. EDIT THIS function!
  * @param  None
  * @retval None
  */
void initGpioB0AsOutput( void )
{
    //NOTE 1.1
    volatile uint32_t  * reg_pointer;                         // Volatile = perform each requested register read or write; don’t skip it or reuse an earlier value
    /* GPIOB Peripheral clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    
    /* GPIOB0 configured as output */
    // 2 bits, bit 1:0
    reg_pointer = (volatile uint32_t*) PORTB_MODER_REGISTER;         // Store MODER's address in reg_pointer, changing (casting) the numeric address to a pointer to volatile uint32_t.
    *reg_pointer = *reg_pointer & ~((uint32_t) GPIO_0_MODER);        // GPIO_0_MODER = 0x03 hex = 3 decimal = 11 binary. ~11 = 111..00 = CLEAR (bits 1:0 = 00)
    *reg_pointer = *reg_pointer |   (uint32_t) GPIO_0_MODER_OUT;     // GPIO_0_MODER_OUT = 0x01 hex = 1 decimal = 01 binary = SET PIN OUT (bits 1:0 = 01)

    
    /*GPIOB0 configured as push-pull */
    // 1 bit, bit 0
    reg_pointer = (volatile uint32_t*) PORTB_OTYPER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t) GPIO_0_OTYPER);       //cast uint32_t on mask to make the type explicit and safe
    *reg_pointer = *reg_pointer | (uint32_t) GPIO_0_OTYPER_PP;
    
    /*GPIOB0 configured floating */
    // 2 bits, bit 1:0
    // PB0 configured with no pull-up or pull-down. 
    reg_pointer = (volatile uint32_t*) PORTB_PUPDR_REGISTER;         //select port B registor
    *reg_pointer = *reg_pointer & ~((uint32_t) GPIO_0_PUPDR);        //select pin 0 in port B == configuring PB0
    *reg_pointer = *reg_pointer | (uint32_t) GPIO_0_PUPDR_NOPULL;    //GPIO_0_PUPDR_NOPULL is a constant mask, so it (and other constant mark) does not need volatile
    
    
    /* GPIOB0 driven high to start out with */
    setGPIOB0();
}

/**
  * @brief  Initializes GPIO C6 to be an input
  * @note   EDIT this function!
  * @param  None
  * @retval None
  */
void initGpioC6AsInput( void )
{
  //NOTE 1.2
      volatile uint32_t  * reg_pointer; 
      /* GPIOC Peripheral clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    /* GPIOC Pin 6 as input*/
    reg_pointer = (volatile uint32_t *) PORTC_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_6_MODER);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_6_MODER_IN;

    /*PUSH-PULL Pin*/
    reg_pointer = (volatile uint32_t *) PORTC_OTYPER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_6_OTYPER);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_6_OTYPER_PP;

    
    /*Configure pulled-down*/
     reg_pointer = (volatile uint32_t *) PORTC_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_6_PUPDR);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_6_PUPDR_PD;
    
}

/**
  * @brief  Initializes GPIO D8 and D9 as alternative function to be used by USART3 to communicate to the computer
  * @note   Do NOT EDIT!
  * @param  None
  * @retval None
  */
void initGpioD8D9AsAF7( void )
{
      volatile uint32_t  * reg_pointer;
      
      /* GPIOD Peripheral clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
    
    /* GPIO D 8 and 9 as alternative function */ 
    reg_pointer = (volatile uint32_t *)PORTD_MODER_REGISTER;
    *reg_pointer = *reg_pointer & (~((uint32_t)GPIO_8_MODER + (uint32_t)GPIO_9_MODER));
    *reg_pointer = *reg_pointer | GPIO_8_MODER_AF | GPIO_9_MODER_AF;
    //to test that the clock is working:
    //*reg_pointer = *reg_pointer | (uint32_t)GPIO_8_MODER_OUT | (uint32_t)GPIO_9_MODER_OUT;
    
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
	
	//Drive them high to test: (this was just to test that the clock was turning on, it is)
	 /* GPIOD 8 and 9 driven high to start out with */
    //reg_pointer = (volatile uint32_t *)PORTD_ODR_REGISTER;
    //*reg_pointer = *reg_pointer | GPIO_8_ODR_HIGH | GPIO_9_ODR_HIGH;

}


//NOTE 1.3 seems like I have to create new functions since there is no place holder
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


// NOTE 1.4: Connect PC6 to Timer 3 channel 1.
void initGpioC6asAF2(void)
{
  volatile uint32_t * reg_pointer;

  // Enable GPIOC's peripheral clock.
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

  // Configure PC6 as alternate function: MODER bits 13:12 = 0b10.
  reg_pointer = (volatile uint32_t*) PORTC_MODER_REGISTER;
  *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_6_MODER);
  *reg_pointer = *reg_pointer | (uint32_t)GPIO_6_MODER_AF;

  // Configure PC6 with pull-down: PUPDR bits 13:12 = 0b10.
  // This keeps PC6 LOW when nothing drives the input.
  reg_pointer = (volatile uint32_t*) PORTC_PUPDR_REGISTER;
  *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_6_PUPDR);
  *reg_pointer = *reg_pointer | (uint32_t)GPIO_6_PUPDR_PD;

  // Select AF2: AFR1 bits 27:24 = 0010.
  // Connect PC6 internally to Timer 3 channel 1's input (connect to AF2).
  reg_pointer = (volatile uint32_t*) PORTC_AFR1_REGISTER;
  *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_6_AFR1);
  *reg_pointer = *reg_pointer | (uint32_t)GPIO_6_AFR1_AF2;
}





















/* hardware actions */

/**
  * @brief  Toggles the value of GPIOB0
  * @note   EDIT this function!
  *               Read the current value of GPIOB0
  *               Change the value depending on the current value
  * @param  None
  * @retval None
  */
void toggleGPIOB0( void )
{
  // NOTE 1.1
    uint32_t value;
    volatile uint32_t  * reg_pointer;

    //get the current value of the pin 
    reg_pointer = (volatile uint32_t *)PORTB_ODR_REGISTER;
    value = *reg_pointer & (uint32_t)GPIO_0_ODR_HIGH; // Read GPIOB's ODR register onto value and keep only the value in bit 0, which controls PB0 (Port B, pin 0 using ODR register)
                                                      // GPIO_0_ODR_HIGH = 0x01, so this mask set every bit to 0 except bit 0, and extract actual value of bit 0
                                                        

    //do some logic based on the current value of the pin to change its value
    
    //The extracted value is numerical. Hexadecimal is just how the mask is written in the source code; it does not change how values compare.
    //in general, value == 12 (decimal), value == 0xC (hexadecimal), and value == 0b1100 (binary), value == 014 (octal) all compare against the same number. Just write it correctly to use each base
    //but normally plain number (noprexix) is decimal in c
    if(value == 0){
      setGPIOB0();
    }
    else{
      clearGPIOB0();
    }

}
/**
  * @brief  Sets the value of GPIOB0 high
  * @note   EDIT this function!
  *               Set the value of GPIOB0 high
  * @param  None
  * @retval None
  */
void setGPIOB0( void )
{
   // NOTE 1.1: ODR uses one bit per pin. PB0 uses bit 0.
   // OR with 0x01 makes PB0's bit 0 becomes 1, preserving the other pins (other bits = other pins)
    volatile uint32_t * reg_pointer;
    reg_pointer = (volatile uint32_t*) PORTB_ODR_REGISTER;
    *reg_pointer = *reg_pointer | (uint32_t) GPIO_0_ODR_HIGH;
}
/**
  * @brief  Clears the value of GPIOB0 
  * @note   EDIT this function!
  *               Set the value of GPIOB0 low
  * @param  None
  * @retval None
  */
void clearGPIOB0( void )
{
    // NOTE 1.1: Use AND with an inverted mask to clear PB0.
    // GPIO_0_ODR_HIGH = 000...0001. 
    // ~GPIO_0_ODR_HIGH = 111...1110.
    // x AND 1 = x. preserve the other bits.
    // x AND 0 = 0. clear bit 0, driving PB0 low.
    volatile uint32_t  * reg_pointer;
    reg_pointer = (volatile uint32_t*) PORTB_ODR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t) GPIO_0_ODR_HIGH);
}
/**
  * @brief  Returns the current value of GPIOC6
  * @note   EDIT this function!
  *               reads the value of GPIOC6 and returns it
  *               will return high at the PIN 6 position if on
  *               will return low at the PIN 6 position if off.
  * @param  None
  * @retval value: the value at GPIOC6 as on off BIT 6
  */
uint32_t checkGPIOC6(void)
{
  // NOTE 1.2
    uint32_t value;
    volatile uint32_t  * reg_pointer;
    //get the current value of the pin 
    reg_pointer = (volatile uint32_t *)PORTC_IDR_REGISTER; 
    value = *reg_pointer & (uint32_t)GPIO_6_IDR;              //Bit 6 has bit mask == 0x40 hexadecimal == 64 decimal == 01000000
                                                              //So, read value stored where pointer point to, AND THEN use and to only keep bit 6. And with 0 on other bits == 0, And with 1 on bit 6 == get value from bit 6)
    
    if(value == 64){    //64 decimal = 1000000 binary, we can also write (value == 0b1000000)
      value = 1;
    }
    else{
      value = 0;
    }
    return value;
}
/**
  * @brief  Checks the current value of GPIOC6, if high, sets GPIOB0 high
  *         if low, clears GPIOB0 to low
  * @note   EDIT this function!
  *               reads the value of GPIOC6 and reflects that
  *               value in GPIOBO
  * @param  None
  * @retval None
  */
void setGPIOB0_withC6( void )
{
    // NOTE 1.2: Read PC6. If high, set PB0 high; otherwise, set PB0 low.
    if(checkGPIOC6() == 1){
        setGPIOB0();
    }
    else{
        clearGPIOB0();
    }
}






//EOF
