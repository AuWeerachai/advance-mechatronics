
#ifndef __STM32F446XX_RCC_H_MORT
#define __STM32F446XX_RCC_H_MORT

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "stm32f446xx_mort.h"

/* Macros for Everyone--------------------------------------------------------*/

/*RCC Control Register*/
#define RCC_CR          (RCC_BASE_MORT + 0x00)
/*RCC PLL configuration register*/
#define RCC_PLLCFGR     (RCC_BASE_MORT + 0x04)
/*RCC clock configuration register*/
#define RCC_CFGR        (RCC_BASE_MORT + 0x08)
/*RCC clock interrupt register*/
#define RCC_CIR        (RCC_BASE_MORT + 0x0C)


/*RCC AHB1 peripheral reset register (RCC_AHB1RSTR)*/
#define RCC_AHB1_RSTR (RCC_BASE_MORT + 0x10)
/*RCC AHB2 peripheral reset register (RCC_AHB2RSTR)*/
#define RCC_AHB2_RSTR (RCC_BASE_MORT + 0x14)
/*RCC AHB3 peripheral reset register (RCC_AHB3RSTR)*/
#define RCC_AHB3_RSTR (RCC_BASE_MORT + 0x18)
/*RCC APB1 peripheral reset register (RCC_APB1RSTR)*/
#define RCC_APB1_RSTR (RCC_BASE_MORT + 0x20)
/*RCC APB2 peripheral reset register (RCC_APB2RSTR)*/
#define RCC_APB2_RSTR (RCC_BASE_MORT + 0x24)
/*RCC AHB1 peripheral clock enable register (RCC_AHB1ENR)*/
#define RCC_AHB1_ENR (RCC_BASE_MORT + 0x30)
/*RCC AHB2 peripheral clock enable register (RCC_AHB2ENR)*/
#define RCC_AHB2_ENR (RCC_BASE_MORT + 0x34)
/*RCC AHB3 peripheral clock enable register (RCC_AHB3ENR)*/
#define RCC_AHB3_ENR (RCC_BASE_MORT + 0x38)
/*RCC APB1 peripheral clock enable register (RCC_APB1ENR)*/
#define RCC_APB1_ENR (RCC_BASE_MORT + 0x40)
/*RCC APB2 peripheral clock enable register (RCC_APB2ENR)*/
#define RCC_APB2_ENR (RCC_BASE_MORT + 0x44)

/*AHB1 Peripherals flags*/
#define RCC_AHB1Periph_GPIOA             ((uint32_t)0x00000001)
#define RCC_AHB1Periph_GPIOB             ((uint32_t)0x00000002)
#define RCC_AHB1Periph_GPIOC             ((uint32_t)0x00000004)
#define RCC_AHB1Periph_GPIOD             ((uint32_t)0x00000008)
#define RCC_AHB1Periph_GPIOE             ((uint32_t)0x00000010)
#define RCC_AHB1Periph_GPIOF             ((uint32_t)0x00000020)
#define RCC_AHB1Periph_GPIOG             ((uint32_t)0x00000040)
#define RCC_AHB1Periph_GPIOH             ((uint32_t)0x00000080)
#define RCC_AHB1Periph_GPIOI             ((uint32_t)0x00000100) 
#define RCC_AHB1Periph_GPIOJ             ((uint32_t)0x00000200)
#define RCC_AHB1Periph_GPIOK             ((uint32_t)0x00000400)
#define RCC_AHB1Periph_CRC               ((uint32_t)0x00001000)
#define RCC_AHB1Periph_FLITF             ((uint32_t)0x00008000)
#define RCC_AHB1Periph_SRAM1             ((uint32_t)0x00010000)
#define RCC_AHB1Periph_SRAM2             ((uint32_t)0x00020000)
#define RCC_AHB1Periph_BKPSRAM           ((uint32_t)0x00040000)
#define RCC_AHB1Periph_SRAM3             ((uint32_t)0x00080000)
#define RCC_AHB1Periph_CCMDATARAMEN      ((uint32_t)0x00100000)
#define RCC_AHB1Periph_DMA1              ((uint32_t)0x00200000)
#define RCC_AHB1Periph_DMA2              ((uint32_t)0x00400000)
#define RCC_AHB1Periph_DMA2D             ((uint32_t)0x00800000)
#define RCC_AHB1Periph_ETH_MAC           ((uint32_t)0x02000000)
#define RCC_AHB1Periph_ETH_MAC_Tx        ((uint32_t)0x04000000)
#define RCC_AHB1Periph_ETH_MAC_Rx        ((uint32_t)0x08000000)
#define RCC_AHB1Periph_ETH_MAC_PTP       ((uint32_t)0x10000000)
#define RCC_AHB1Periph_OTG_HS            ((uint32_t)0x20000000)
#define RCC_AHB1Periph_OTG_HS_ULPI       ((uint32_t)0x40000000)

/*AHB2 Peripherals flags*/
#define RCC_AHB1Periph_DCMIEN             ((uint32_t)0x00000001)
#define RCC_AHB1Periph_OTGFSEN             ((uint32_t)0x00000080)

/*APB1 Peripherals flags*/
#define RCC_APB1Periph_TIM2              ((uint32_t)0x00000001)
#define RCC_APB1Periph_TIM3              ((uint32_t)0x00000002)
#define RCC_APB1Periph_TIM4              ((uint32_t)0x00000004)
#define RCC_APB1Periph_TIM5              ((uint32_t)0x00000008)
#define RCC_APB1Periph_TIM6              ((uint32_t)0x00000010)
#define RCC_APB1Periph_TIM7              ((uint32_t)0x00000020)
#define RCC_APB1Periph_TIM12             ((uint32_t)0x00000040)
#define RCC_APB1Periph_TIM13             ((uint32_t)0x00000080)
#define RCC_APB1Periph_TIM14             ((uint32_t)0x00000100)
#define RCC_APB1Periph_WWDG              ((uint32_t)0x00000800)
#define RCC_APB1Periph_SPI2              ((uint32_t)0x00004000)
#define RCC_APB1Periph_SPI3              ((uint32_t)0x00008000)
#define RCC_APB1Periph_SPDIFRX           ((uint32_t)0x00010000)
#define RCC_APB1Periph_USART2            ((uint32_t)0x00020000)
#define RCC_APB1Periph_USART3            ((uint32_t)0x00040000)
#define RCC_APB1Periph_UART4             ((uint32_t)0x00080000)
#define RCC_APB1Periph_UART5             ((uint32_t)0x00100000)
#define RCC_APB1Periph_I2C1              ((uint32_t)0x00200000)
#define RCC_APB1Periph_I2C2              ((uint32_t)0x00400000)
#define RCC_APB1Periph_I2C3              ((uint32_t)0x00800000)
#define RCC_APB1Periph_FMPI2C1           ((uint32_t)0x01000000)
#define RCC_APB1Periph_CAN1              ((uint32_t)0x02000000)
#define RCC_APB1Periph_CAN2              ((uint32_t)0x04000000)
#define RCC_APB1Periph_CEC               ((uint32_t)0x08000000)
#define RCC_APB1Periph_PWR               ((uint32_t)0x10000000)
#define RCC_APB1Periph_DAC               ((uint32_t)0x20000000)
#define RCC_APB1Periph_UART7             ((uint32_t)0x40000000)
#define RCC_APB1Periph_UART8             ((uint32_t)0x80000000)

/*APB2 Peripherals flags*/
#define RCC_APB2Periph_TIM1              ((uint32_t)0x00000001)
#define RCC_APB2Periph_TIM8              ((uint32_t)0x00000002)
#define RCC_APB2Periph_USART1            ((uint32_t)0x00000010)
#define RCC_APB2Periph_USART6            ((uint32_t)0x00000020)
#define RCC_APB2Periph_ADC               ((uint32_t)0x00000100)
#define RCC_APB2Periph_ADC1              ((uint32_t)0x00000100)
#define RCC_APB2Periph_ADC2              ((uint32_t)0x00000200)
#define RCC_APB2Periph_ADC3              ((uint32_t)0x00000400)
#define RCC_APB2Periph_SDIO              ((uint32_t)0x00000800)
#define RCC_APB2Periph_SPI1              ((uint32_t)0x00001000)
#define RCC_APB2Periph_SPI4              ((uint32_t)0x00002000)
#define RCC_APB2Periph_SYSCFG            ((uint32_t)0x00004000)
#define RCC_APB2Periph_EXTIT             ((uint32_t)0x00008000)
#define RCC_APB2Periph_TIM9              ((uint32_t)0x00010000)
#define RCC_APB2Periph_TIM10             ((uint32_t)0x00020000)
#define RCC_APB2Periph_TIM11             ((uint32_t)0x00040000)
#define RCC_APB2Periph_SPI5              ((uint32_t)0x00100000)
#define RCC_APB2Periph_SPI6              ((uint32_t)0x00200000)
#define RCC_APB2Periph_SAI1              ((uint32_t)0x00400000)
#define RCC_APB2Periph_SAI2              ((uint32_t)0x00800000)
#define RCC_APB2Periph_LTDC              ((uint32_t)0x04000000)

typedef enum 
{
  DISABLE = 0U, 
  ENABLE = !DISABLE
} FunctionalState_mort;

/*Function declarations---------------------------------------------------------*/
void RCC_AHB1PeriphClockCmd(uint32_t RCC_AHB1Periph, FunctionalState_mort NewState);
void RCC_AHB2PeriphClockCmd(uint32_t RCC_AHB2Periph, FunctionalState_mort NewState);
void RCC_APB1PeriphClockCmd(uint32_t RCC_APB1Periph, FunctionalState_mort NewState);
void RCC_APB2PeriphClockCmd(uint32_t RCC_APB2Periph, FunctionalState_mort NewState);

#endif /* __STM32F446XX_RCC_H_MORT */
