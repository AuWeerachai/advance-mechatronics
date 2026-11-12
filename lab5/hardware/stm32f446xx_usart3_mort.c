/**
  ******************************************************************************
  * @file    stm32f446xx_usart3_mort.c 
  * @author  mortamar@andrew.cmu.edu
  * @version 2
  * @date    June 2026
  * @brief   Controls STM32F446ze usart3 to communicate to the computer to debug
  ******************************************************************************
  */

/* Includes----------------------------------------------------------*/

#include <stdarg.h>

#include "stm32f446xx_rcc_mort.h"
#include "stm32f446xx_mort.h"
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_usart3_mort.h"

/* Macros----------------------------------------------------------*/

//Registers
#define USART3_SR           (USART3_BASE_MORT + 0x0000) /*!< Status Register */
#define USART3_DR             (USART3_BASE_MORT + 0x0004) /*!< Data Register */
#define USART3_BRR            (USART3_BASE_MORT + 0x0008) /*!< Baud Rate Register */
#define USART3_CR1            (USART3_BASE_MORT + 0x000C) /*!< Control Register 1 */
#define USART3_CR2           (USART3_BASE_MORT + 0x0010)/*!< Control Register 2 */
#define USART3_CR3            (USART3_BASE_MORT + 0x0014) /*!< Control Register 3 */



/* Functions----------------------------------------------------------*/

/**
  * @brief  Initializes USART3 to communicate to the computer
  * @note   Sets the Baud rate to 115200 assuming that the clock is running at 180 MHz
  * @param  None
  * @retval None
  */
void initUSART3(void)
{
    volatile uint16_t *reg_pointer;
	
    /*init PD8 and PD9 as alternate function mode*/
    initGpioD8D9AsAF7();
    
    /* USART3 Peripheral clock enable */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);
    
    /*Complete strip-down of CR1 (Disables UE, sets 8-bit mode, No Parity) */
     reg_pointer = (volatile uint16_t *)USART3_CR1;
    *reg_pointer = 0x0000; 
    /* Write to USART CR2 (Clears stop bits to standard 1 Stop Bit) */
    reg_pointer = (volatile uint16_t *)USART3_CR2;
    *reg_pointer = 0x0000;
    /* Write to USART CR3 (Clears hardware flow control properties) */
    reg_pointer = (volatile uint16_t *)USART3_CR3;
    *reg_pointer = 0x0000;

	/* Configure Baud Rate to 115200 (using 45 MHz APB1 clock calculation) */
    reg_pointer = (volatile uint16_t *)USART3_BRR;
    *reg_pointer = 0x187; //this is the right one when the clock is running at 180MHz
    //*reg_uart_brr = 0x30D;
    //*reg_uart_brr = 0x008B;
    
    /*  Write to USART CR1 (Configures Word Length, Parity, TE, RE) */
    /*Bit 3 = TE (Tx Enable), Bit 2 = RE (Rx Enable)*/
    /* Note: We do NOT set bit 13 (UE) here! */
    reg_pointer = (volatile uint16_t *)USART3_CR1;
    *reg_pointer |= (uint16_t)((1 << 3) | (1 << 2)); // Enable Transmitter (TE) and Receiver (RE)


    /*  Enable the USART peripheral, Transmitter, and Receiver */
    /* Bit 13 = UE (USART Enable) */
    *reg_pointer |= (uint16_t)(1 << 13); 
}
/**
  * @brief  Transmits one char
  * @note   Just one char with no terminating character.
  * @param  char c : the character to send
  * @retval None
  */
void transmitCharUSART3(char c) 
{
    volatile uint16_t *reg_uart_sr = (volatile uint16_t *)USART3_SR;
    volatile uint16_t *reg_uart_dr = (volatile uint16_t *)USART3_DR;

    /* Poll status register Bit 7 (TXE: Transmit Data Register Empty) until it becomes 1 */
    while (((*reg_uart_sr) & (1 << 7)) == 0);

    /* Drop the character directly into the physical shift data container */
    *reg_uart_dr = (c & 0xFF);
}
/**
  * @brief  Receives one char
  * @note   Just one char with no terminating character.
  * @param  None
  * @retval char c : the character it received
  */
char receiveCharUSART3(void)
{
	volatile uint32_t *reg_uart_sr = (volatile uint32_t *)USART3_SR;
    volatile uint32_t *reg_uart_dr = (volatile uint32_t *)USART3_DR;

    /* Poll status register Bit 5 (RXNE: Read Data Register Not Empty) until it becomes 1 */
    while (((*reg_uart_sr) & (1 << 5)) == 0);

    /* Pull the raw character out of the internal wire queue buffer */
    return (char)(*reg_uart_dr & 0xFF);
}
/**
  * @brief  Returns the value of the USART3 status register
  * @note   
  * @param  None
  * @retval uint16_t : the current flags of the status USART3 register 
  */
uint16_t returnStatusRegisterUART3(void)
{
	volatile uint16_t *reg_uart_sr = (volatile uint16_t *)USART3_SR;
	return *reg_uart_sr;
}


/**
  * @brief  Transmits a string using USART3
  * @note   waits until the terminating character of a string to stop transmitting
  *         if you declare a string as s = "abc", the compiler adds the null character
  *         automatically. If you do not, and delcare the array manually, you need to add the
  *         terminating null character manually. I.e. char s[4]; char[0] = 'a'; ... char[3] = '\0';
  *         doing char s[4] = "abc"; is also ok and will add the \0 in the 3rd and last position. 
  * @param  const char *s : a pointer to a string 
  * @retval None
  */
void transmitStringUSART3(const char *s)
{
    if (s == 0)
    {
        return;
    }

    while (*s != '\0') 
    {
        transmitCharUSART3(*s);
        s++;
    }
}
/**
  * @brief  Transmits a line using USART3
  * @note   transmits a string and then adds a newline character at the end for a newline
  * @param  const char *s : a pointer to a string 
  * @retval None
  */
void transmitLineUSART3(const char *s)
{
    transmitStringUSART3(s);
    transmitStringUSART3("\r\n");
}

/*Helper functions for printf emulator
(not as flexible as printf! Check below what it can do):*/

/**
  * @brief  Converts an unsigned number into ASCII text in a chosen base, then transmits the characters one at a time over USART3.
  * @note   Ex usage:
  *                 transmitUnsignedBaseUSART3(1234, 10, 0, 0);      // prints 1234
  *                 transmitUnsignedBaseUSART3(0xABCD, 16, 0, 1);   // prints ABCD
  *                 transmitUnsignedBaseUSART3(5, 2, 8, 0);         // prints 00000101
  * @param  unsigned long value :  the number you want to print
  *             ex: 1234 or 0xABCD 
  *          unsigned int base : the number base to use. the largest supported base is 16. 
  *              ex: base = 10 //decimal
  *                  base = 16 //hexadecimal
  *                  base = 2  //binary
  *          unsigned int minimum_width : the minimum number of characters to print. If the number is shorter than this, the function adds leading zeros
  *          unsigned int uppercase : if upercase is nonzero, use ABCDEF. Else, abcdef
  * @retval None
  */
static void transmitUnsignedBaseUSART3(unsigned long value,
                                       unsigned int base,
                                       unsigned int minimum_width,
                                       unsigned int uppercase)
{
    char buffer[32];
    unsigned int i;
    const char *digits;

    /*Checks that base is in the supported range. 
    Min is 2, max is 16*/
    if ((base < 2U) || (base > 16U))
    {
        return;
    }
    /*assign "0123456789ABCDEF" to digits if uppercase is true and "0123456789abcdef" if false*/
    digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef"; 

    /*Convert the number into digits.
    It repeatedly extracts the last digit of the number and stores it backwards.*/
    i = 0U;

    do
    {
        buffer[i] = digits[value % base];
        i++;
        value = value / base;
    } while ((value != 0U) && (i < sizeof(buffer)));

    /*Add leading zeros if necessary*/
    while ((i < minimum_width) && (i < sizeof(buffer)))
    {
        buffer[i] = '0';
        i++;
    }

    /*Transmit the characters in the correct order*/
    while (i > 0U)
    {
        i--;
        transmitCharUSART3(buffer[i]);
    }
}
/**
  * @brief  Transmits a signed integer
  * @note   transmits asigned long integer (64 bits)
  *         by first transmitting the negative character -
  *         and then transmitting the number
  * @param  long value: the signed long integer to be transmitted
  * @retval None
  */
static void transmitSignedUSART3(long value)
{
    if (value < 0)
    {
        transmitCharUSART3('-');

        /*
         * Avoid signed overflow for the most-negative number.
         * Example: -2147483648 cannot be represented as +2147483648
         * in a signed 32-bit integer, so do the negation in unsigned math.
         */
        transmitUnsignedBaseUSART3(0UL - (unsigned long)value, 10U, 0U, 0U);
    }
    else
    {
        transmitUnsignedBaseUSART3((unsigned long)value, 10U, 0U, 0U);
    }
}
/**
  * @brief  Transmits a 32 bit integer
  * @note   
  * @param  int32_t value: the value to be transmitted
  * @retval None
  */
void transmitIntUSART3(int32_t value)
{
    transmitSignedUSART3((long)value);
}
/**
  * @brief  Transmits an unsigned 32 bit integer
  * @note   
  * @param  uint32_t value: the value to be transmitted
  * @retval None
  */
void transmitUIntUSART3(uint32_t value)
{
    transmitUnsignedBaseUSART3((unsigned long)value, 10U, 0U, 0U);
}
/**
  * @brief  Transmits a hex value
  * @note   first prints the characters 0x, then the actual value
  * @param  uint32_t value: the value to be transmitted
  * @retval None
  */
void transmitHex32USART3(uint32_t value)
{
    transmitStringUSART3("0x");
    transmitUnsignedBaseUSART3((unsigned long)value, 16U, 8U, 1U);
}


/**
  * @brief  Printf-like function. 
  * @note   This is not printf. It is a small interpretation of it
  *         It supports:
  *         %s   string
  *         %c   character
  *         %d   signed int
  *         %i   signed int
  *         %u   unsigned int
  *         %x   unsigned hex, lowercase
  *         %X   unsigned hex, uppercase
  *         %p   pointer
  *         %%   literal percent sign
  *         %ld  signed long
  *         %lu  unsigned long
  *         %lx  unsigned long hex
  *         %lX  unsigned long hex uppercase
  * @param  const char *fmt: the formatting string -> const here means the string will not be modified by the function
  *                 ex: "x = %d\r\n"
  *         ... -> the function can receive extra arguments after fmt
  *         NOTE: DOES NOT SUPPORT ANY SORT OF FLOATS!
  * @retval None
  */
void printfUSART3(const char *fmt, ...)
{

    va_list args; //types and macros declared in <stdarg.h> allowing to use the ... argument. Can be thought of as a cursor that points to the next unnamed argument. 
    int long_argument; //used as a flag to indicate if we encounter a "long" value: 0 means normal int / unsigned int. 1 means long / unsinged long

    /* Check for null format string*/
    if (fmt == 0)
    {
        return;
    }

    /*initializes args
    It tells the C varargs system that the last named argunment is fmt, 
    and that the unnamed arguments begin after fmt*/
    va_start(args, fmt);

    /*loop through the format string one character at a time until i treaches the null terminator '\0'*/
    while (*fmt != '\0')
    {
        /*This handles the ordinary characters:
        If the current character is not %, then it is not a format command.
        So the function just transmits it.*/
        if (*fmt != '%')
        {
            transmitCharUSART3(*fmt);
            fmt++;
            continue; //jumps back to the top of the while loop immediately
        }

        /* Skip '%' . Reaching here means that the if above did find a formatting character.
        You then skip it. This is because the % itself is not printed. 
        We are just using it as a sign that the next character is special.*/
        fmt++;

        /*This handles a bad trailing percent sign. If there is nothing after the 
        percent sign, we exit and don't print, but we still call va_end(args);*/
        if (*fmt == '\0')
        {
            break;
        }

        /*Assume normal argument length*/
        long_argument = 0;

        /* Support %ld, %lu, %lx, %lX */
        if (*fmt == 'l')
        {
            long_argument = 1; //however if we do encpunter the l then we handle it.
            fmt++; //move on to the next letter after the l

            /*If you encounter a null character after the l, break but still call va_end(args)*/
            if (*fmt == '\0')
            {
                break;
            }
        }

        /*The next bit now handles the format characters:*/
        switch (*fmt)
        {
            /* %%  prints % */
            case '%':
            {
                transmitCharUSART3('%');
                break;
            }
            /* %c  prints a character  */
            case 'c':
            {
                char c;

                c = (char)va_arg(args, int);
                transmitCharUSART3(c);
                break;
            }
             /* %s  prints a string  */
            case 's':
            {
                const char *s;

                s = va_arg(args, const char *);

                if (s == 0)
                {
                    transmitStringUSART3("(null)");
                }
                else
                {
                    transmitStringUSART3(s);
                }

                break;
            }
            /* %d and %i  prints signed decimal */
            case 'd':
            case 'i':
            {
                if (long_argument)
                {
                    transmitSignedUSART3(va_arg(args, long));
                }
                else
                {
                    transmitSignedUSART3((long)va_arg(args, int));
                }

                break;
            }
            /* %u  prints unsigned decimal */
            case 'u':
            {
                if (long_argument)
                {
                    transmitUnsignedBaseUSART3(va_arg(args, unsigned long),
                                               10U,
                                               0U,
                                               0U);
                }
                else
                {
                    transmitUnsignedBaseUSART3((unsigned long)va_arg(args, unsigned int),
                                               10U,
                                               0U,
                                               0U);
                }

                break;
            }
            /* %x prints lower case hexadecimal */
            case 'x':
            {
                if (long_argument)
                {
                    transmitUnsignedBaseUSART3(va_arg(args, unsigned long),
                                               16U,
                                               0U,
                                               0U);
                }
                else
                {
                    transmitUnsignedBaseUSART3((unsigned long)va_arg(args, unsigned int),
                                               16U,
                                               0U,
                                               0U);
                }

                break;
            }
            /* %X  prints uppercase hexadecimal */
            case 'X':
            {
                if (long_argument)
                {
                    transmitUnsignedBaseUSART3(va_arg(args, unsigned long),
                                               16U,
                                               0U,
                                               1U);
                }
                else
                {
                    transmitUnsignedBaseUSART3((unsigned long)va_arg(args, unsigned int),
                                               16U,
                                               0U,
                                               1U);
                }

                break;
            }
            /* %p  prints pointer address */
            case 'p':
            {
                void *ptr;

                ptr = va_arg(args, void *);

                transmitStringUSART3("0x");
                transmitUnsignedBaseUSART3((unsigned long)(uintptr_t)ptr,
                                           16U,
                                           8U,
                                           1U);

                break;
            }

            default:
            {
                /*
                 * Unknown format. Print it literally so debugging output
                 * does not silently disappear.
                 */
                transmitCharUSART3('%');

                if (long_argument)
                {
                    transmitCharUSART3('l');
                }

                transmitCharUSART3(*fmt);
                break;
            }
        }

        fmt++;
    }

    va_end(args);
}
//EOF
