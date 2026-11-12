// Lab 4 Part 2: GPIO hardware access.
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_rcc_mort.h"

// REGISTER ADDRESSES:

// Port C: PC6 = D16 comparator output input.
#define PORTC_BASE_ADDRESS		GPIOC_BASE_MORT
#define PORTC_MODER_REGISTER  (PORTC_BASE_ADDRESS + 0x00) // Select input/output mode.
#define PORTC_PUPDR_REGISTER  (PORTC_BASE_ADDRESS + 0x0C) // Select pull-up/pull-down/no pull.
#define PORTC_IDR_REGISTER    (PORTC_BASE_ADDRESS + 0x10) // Read input pins as HIGH or LOW.

// Port D: PD8 = TX, PD9 = RX to serial monitor; PD15 = D9 blue illumination.
#define PORTD_BASE_ADDRESS		GPIOD_BASE_MORT
#define PORTD_MODER_REGISTER (PORTD_BASE_ADDRESS + 0x00)
#define PORTD_OTYPER_REGISTER (PORTD_BASE_ADDRESS + 0x04)
#define PORTD_OSPEEDR_REGISTER (PORTD_BASE_ADDRESS + 0x08)
#define PORTD_PUPDR_REGISTER (PORTD_BASE_ADDRESS + 0x0C)
#define PORTD_ODR_REGISTER (PORTD_BASE_ADDRESS + 0x14)
#define PORTD_AFR2_REGISTER (PORTD_BASE_ADDRESS + 0x24)

// Port E: PE13 = D3 green detection, PE11 = D5 white detection, PE9 = D6 red illumination.
#define PORTE_BASE_ADDRESS    GPIOE_BASE_MORT
#define PORTE_MODER_REGISTER  (PORTE_BASE_ADDRESS + 0x00) // Select input/output mode.
#define PORTE_PUPDR_REGISTER  (PORTE_BASE_ADDRESS + 0x0C) // Select pull-up/pull-down/no pull.
#define PORTE_ODR_REGISTER    (PORTE_BASE_ADDRESS + 0x14) // Set output pins HIGH or LOW.

// Port F: PF15 = D2 red detection, PF14 = D4 orange detection, PF13 = D7 green illumination.
#define PORTF_BASE_ADDRESS    GPIOF_BASE_MORT
#define PORTF_MODER_REGISTER  (PORTF_BASE_ADDRESS + 0x00) // Select input/output mode.
#define PORTF_PUPDR_REGISTER  (PORTF_BASE_ADDRESS + 0x0C) // Select pull-up/pull-down/no pull.
#define PORTF_ODR_REGISTER    (PORTF_BASE_ADDRESS + 0x14) // Set output pins HIGH or LOW.

// FLAGS:

// PC6: comparator input (D16).
#define GPIO_6_MODER                   0x3000
#define GPIO_6_MODER_IN                0x0000
#define GPIO_6_PUPDR                   0x3000
#define GPIO_6_PUPDR_NOPULL            0x0000
#define GPIO_6_IDR                     0x40

// PD8: USART3 TX.
#define GPIO_8_MODER                   (uint32_t)(0x03<<16)
#define GPIO_8_MODER_AF                (uint32_t)(0x02<<16)
#define GPIO_8_OTYPER                  (((uint32_t)0x01)<<8)
#define GPIO_8_OTYPER_PP               0x00
#define GPIO_8_OSPEEDR_HIGH_SPEED      (((uint32_t)0x03)<<16)
#define GPIO_8_PUPDR                   (uint32_t)(0x03<<16)
#define GPIO_8_PUPDR_PU                (uint32_t)(0x01<<16)
#define GPIO_8_AFR2_SET_ALL            0xF
#define GPIO_8_AFR2_AF7                0x7

// PE9: red illumination (D6).
#define GPIO_9_MODER                   (uint32_t)(0x03<<18)
#define GPIO_9_MODER_OUT               (((uint32_t)0x01)<<18)
#define GPIO_9_PUPDR                   (uint32_t)(0x03<<18)
#define GPIO_9_PUPDR_NOPULL            0x00
#define GPIO_9_ODR_HIGH                (((uint32_t)0x01)<<9)

// PE11: white detection LED (D5)
#define GPIO_11_MODER                  (((uint32_t)0x03)<<22)
#define GPIO_11_MODER_OUT              (((uint32_t)0x01)<<22)
#define GPIO_11_PUPDR                  (((uint32_t)0x03)<<22)
#define GPIO_11_PUPDR_NOPULL           0x00
#define GPIO_11_ODR_HIGH               (((uint32_t)0x01)<<11)

// PE13: green detection (D3), PF13: green illumination (D7).
#define GPIO_13_MODER                  (((uint32_t)0x03)<<26)
#define GPIO_13_MODER_OUT              (((uint32_t)0x01)<<26)
#define GPIO_13_PUPDR                  (((uint32_t)0x03)<<26)
#define GPIO_13_PUPDR_NOPULL           0x00
#define GPIO_13_ODR_HIGH               (((uint32_t)0x01)<<13)

// PF14: orange detection (D4).
#define GPIO_14_MODER                  (((uint32_t)0x03)<<28)
#define GPIO_14_MODER_OUT              (((uint32_t)0x01)<<28)
#define GPIO_14_PUPDR                  (((uint32_t)0x03)<<28)
#define GPIO_14_PUPDR_NOPULL           0x00
#define GPIO_14_ODR_HIGH               (((uint32_t)0x01)<<14)

// PF15: red detection (D2), PD15: blue illumination (D9).
#define GPIO_15_MODER                  (((uint32_t)0x03)<<30)
#define GPIO_15_MODER_OUT              (((uint32_t)0x01)<<30)
#define GPIO_15_PUPDR                  (((uint32_t)0x03)<<30)
#define GPIO_15_PUPDR_NOPULL           0x00
#define GPIO_15_ODR_HIGH               (((uint32_t)0x01)<<15)



// FUNCTION DEFINITIONS:


///////////////////////////////////////////////////////// COMPARATOR /////////////////////////////////////////////////////////////

// Configure PC6 as a DIGITAL INPUT from COMPARATOR
void initGpioC6AsInput( void )
{
    volatile uint32_t  * reg_pointer; 
      /* GPIOC Peripheral clock enable */
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    /* GPIOC Pin 6 as input*/
    reg_pointer = (volatile uint32_t *) PORTC_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_6_MODER);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_6_MODER_IN;

    /* No internal pull: comparator output has an external pull-up to 3.3 V. */
     reg_pointer = (volatile uint32_t *) PORTC_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_6_PUPDR);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_6_PUPDR_NOPULL;
    
}

// Read COMPARATOR OUTPUT (PC6) and return 1 for HIGH or 0 for LOW.
uint32_t checkGPIOC6(void)
{
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

///////////////////////////////////////////////////////// LED /////////////////////////////////////////////////////////////

// Configure D2 / PF15 as a DIGITAL OUTPUT for red detection.
void initGpioPF15AsOutput( void )
{
    volatile uint32_t  * reg_pointer;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    reg_pointer = (volatile uint32_t *) PORTF_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_15_MODER);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_15_MODER_OUT;

    reg_pointer = (volatile uint32_t *) PORTF_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_15_PUPDR);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_15_PUPDR_NOPULL;
}

// Configure D3 / PE13 as a DIGITAL OUTPUT for green detection.
void initGpioPE13AsOutput( void )
{
    volatile uint32_t  * reg_pointer; 
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    reg_pointer = (volatile uint32_t *) PORTE_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_13_MODER);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_13_MODER_OUT;

     reg_pointer = (volatile uint32_t *) PORTE_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_13_PUPDR);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_13_PUPDR_NOPULL ;
}

// Configure D4 / PF14 as a DIGITAL OUTPUT for orange detection.
void initGpioPF14AsOutput( void )
{
    volatile uint32_t  * reg_pointer;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    reg_pointer = (volatile uint32_t *) PORTF_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_14_MODER);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_14_MODER_OUT;

    reg_pointer = (volatile uint32_t *) PORTF_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_14_PUPDR);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_14_PUPDR_NOPULL;
}


// Configure D5 / PE11 as a DIGITAL OUTPUT for white detection.
void initGpioPE11AsOutput( void )
{
    volatile uint32_t  * reg_pointer;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    reg_pointer = (volatile uint32_t *) PORTE_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_11_MODER);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_11_MODER_OUT;

    reg_pointer = (volatile uint32_t *) PORTE_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_11_PUPDR);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_11_PUPDR_NOPULL;
}

// Configure D6 / PE9 as a DIGITAL OUTPUT for red illumination.
void initGpioPE9AsOutput( void )
{
    volatile uint32_t  * reg_pointer;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

    reg_pointer = (volatile uint32_t *) PORTE_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_9_MODER);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_9_MODER_OUT;

    reg_pointer = (volatile uint32_t *) PORTE_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_9_PUPDR);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_9_PUPDR_NOPULL;
}



// Configure D7 / PF13 as a DIGITAL OUTPUT for green illumination.
void initGpioPF13AsOutput( void )
{
    volatile uint32_t  * reg_pointer;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    reg_pointer = (volatile uint32_t *) PORTF_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_13_MODER);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_13_MODER_OUT;

    reg_pointer = (volatile uint32_t *) PORTF_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_13_PUPDR);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_13_PUPDR_NOPULL;
}


// Configure D9 / PD15 as a DIGITAL OUTPUT for blue illumination.
void initGpioPD15AsOutput( void )
{
    volatile uint32_t  * reg_pointer;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    reg_pointer = (volatile uint32_t *) PORTD_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_15_MODER);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_15_MODER_OUT;

    reg_pointer = (volatile uint32_t *) PORTD_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_15_PUPDR);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_15_PUPDR_NOPULL;
}


/////////////////// Write 1 for HIGH or 0 for LOW to each LED ////////////////////////// 

// D2, PF15: red detection.
void writeGPIOPF15(uint32_t value)
{
    volatile uint32_t * reg_pointer;
    reg_pointer = (volatile uint32_t *) PORTF_ODR_REGISTER;

    if(value == 0)
    {
        *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_15_ODR_HIGH);
    }
    else
    {
        *reg_pointer = *reg_pointer | (uint32_t)GPIO_15_ODR_HIGH;
    }
}

// D3, PE13: green detection.
void writeGPIOPE13(uint32_t value)
{
    volatile uint32_t * reg_pointer;
    reg_pointer = (volatile uint32_t *) PORTE_ODR_REGISTER;

    if(value == 0)
    {
        *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_13_ODR_HIGH);
    }
    else
    {
        *reg_pointer = *reg_pointer | (uint32_t)GPIO_13_ODR_HIGH;
    }
}

// D4, PF14: orange detection.
void writeGPIOPF14(uint32_t value)
{
    volatile uint32_t * reg_pointer;
    reg_pointer = (volatile uint32_t *) PORTF_ODR_REGISTER;

    if(value == 0)
    {
        *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_14_ODR_HIGH);
    }
    else
    {
        *reg_pointer = *reg_pointer | (uint32_t)GPIO_14_ODR_HIGH;
    }
}

// D5, PE11: white detection.
void writeGPIOPE11(uint32_t value)
{
    volatile uint32_t * reg_pointer;
    reg_pointer = (volatile uint32_t *) PORTE_ODR_REGISTER;

    if(value == 0)
    {
        *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_11_ODR_HIGH);
    }
    else
    {
        *reg_pointer = *reg_pointer | (uint32_t)GPIO_11_ODR_HIGH;
    }
}

// D6, PE9: red illumination.
void writeGPIOPE9(uint32_t value)
{
    volatile uint32_t * reg_pointer;
    reg_pointer = (volatile uint32_t *) PORTE_ODR_REGISTER;

    if(value == 0)
    {
        *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_9_ODR_HIGH);
    }
    else
    {
        *reg_pointer = *reg_pointer | (uint32_t)GPIO_9_ODR_HIGH;
    }
}

// D7, PF13: green illumination.
void writeGPIOPF13(uint32_t value)
{
    volatile uint32_t * reg_pointer;
    reg_pointer = (volatile uint32_t *) PORTF_ODR_REGISTER;

    if(value == 0)
    {
        *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_13_ODR_HIGH);
    }
    else
    {
        *reg_pointer = *reg_pointer | (uint32_t)GPIO_13_ODR_HIGH;
    }
}

// D9, PD15: blue illumination.
void writeGPIOPD15(uint32_t value)
{
    volatile uint32_t * reg_pointer;
    reg_pointer = (volatile uint32_t *) PORTD_ODR_REGISTER;

    if(value == 0)
    {
        *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_15_ODR_HIGH);
    }
    else
    {
        *reg_pointer = *reg_pointer | (uint32_t)GPIO_15_ODR_HIGH;
    }
}


///////////////////////////////////////////////////////// TX /////////////////////////////////////////////////////////////

// Configure PD8 as USART3 TX for the serial monitor.
void initGpioD8AsAF7( void )
{
    volatile uint32_t * reg_pointer;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);

    // Alternate function mode.
    reg_pointer = (volatile uint32_t*) PORTD_MODER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_8_MODER);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_8_MODER_AF;

    // Push-pull output.
    reg_pointer = (volatile uint32_t*) PORTD_OTYPER_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_8_OTYPER);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_8_OTYPER_PP;

    // High speed.
    reg_pointer = (volatile uint32_t*) PORTD_OSPEEDR_REGISTER;
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_8_OSPEEDR_HIGH_SPEED;

    // Pull-up.
    reg_pointer = (volatile uint32_t*) PORTD_PUPDR_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_8_PUPDR);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_8_PUPDR_PU;

    // AF7 connects PD8 to USART3 TX.
    reg_pointer = (volatile uint32_t*) PORTD_AFR2_REGISTER;
    *reg_pointer = *reg_pointer & ~((uint32_t)GPIO_8_AFR2_SET_ALL);
    *reg_pointer = *reg_pointer | (uint32_t)GPIO_8_AFR2_AF7;
}
