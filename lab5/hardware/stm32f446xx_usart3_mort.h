/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F446XX_USART3_MORT_H_
#define __STM32F446XX_USART3_MORT_H_


/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32f446xx_mort.h"

/* Macros for Everyone--------------------------------------------------------*/


/*Function declarations---------------------------------------------------------*/

//basic functions:
void initUSART3(void);

void transmitCharUSART3(char c);
char receiveCharUSART3(void);

uint16_t returnStatusRegisterUART3(void);

//wrappers:
void transmitStringUSART3(const char *s);
void transmitLineUSART3(const char *s);
//helpers for printf:
void transmitIntUSART3(int32_t value);
void transmitUIntUSART3(uint32_t value);
void transmitHex32USART3(uint32_t value);

void printfUSART3(const char *fmt, ...);

#endif /*__STM32F446XX_USART3_MORT_H_ */