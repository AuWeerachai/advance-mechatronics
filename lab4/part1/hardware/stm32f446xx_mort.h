#ifndef __STM32F446XX_MORT_H_
#define __STM32F446XX_MORT_H_

#include <stdint.h>

/* !< ARM CORTEX-M4 INTERNAL CORES (SYSTEM CONTROL SPACE) */
#define SCS_BASE_MORT              0xE000E000U /*!< System Control Space Base Address */

/* < SCS sub-blocks */
#define SYSTICK_BASE_MORT          (SCS_BASE_MORT + 0x0010U) /*!< SysTick Timer Base Address */
#define NVIC_BASE_MORT             (SCS_BASE_MORT + 0x0100U) /*!< Nested Vectored Interrupt Controller Base Address */
#define SCB_BASE_MORT              (SCS_BASE_MORT + 0x0D00U) /*!< System Control Block Base Address */


/* <  SysTick Timer Registers  */
/* Used for: Creating highly accurate, deterministic hardware delay loops.   */
#define SYSTICK_CTRL_MORT          (SYSTICK_BASE_MORT + 0x0000U) /*!< SysTick Control and Status Register */
#define SYSTICK_LOAD_MORT          (SYSTICK_BASE_MORT + 0x0004U) /*!< SysTick Reload Value Register */
#define SYSTICK_VAL_MORT           (SYSTICK_BASE_MORT + 0x0008U) /*!< SysTick Current Value Register */
#define SYSTICK_CALIB_MORT         (SYSTICK_BASE_MORT + 0x000CU) /*!< SysTick Calibration Value Register */

/* <  NVIC Core Registers                                                */
/* Used for: Enabling, disabling, and clearing external peripheral interrupts. */
#define NVIC_ISER0_MORT            (NVIC_BASE_MORT + 0x0000U) /*!< Interrupt Set-Enable Register 0 (IRQs 0-31) */
#define NVIC_ISER1_MORT            (NVIC_BASE_MORT + 0x0004U) /*!< Interrupt Set-Enable Register 1 (IRQs 32-63) */
#define NVIC_ISER2_MORT            (NVIC_BASE_MORT + 0x0008U) /*!< Interrupt Set-Enable Register 2 (IRQs 64-95) */

#define NVIC_ICER0_MORT            (NVIC_BASE_MORT + 0x0080U) /*!< Interrupt Clear-Enable Register 0 (IRQs 0-31) */
#define NVIC_ICER1_MORT            (NVIC_BASE_MORT + 0x0084U) /*!< Interrupt Clear-Enable Register 1 (IRQs 32-63) */
#define NVIC_ICER2_MORT            (NVIC_BASE_MORT + 0x0088U) /*!< Interrupt Clear-Enable Register 2 (IRQs 64-95) */

#define NVIC_ISPR0_MORT            (NVIC_BASE_MORT + 0x0100U) /*!< Interrupt Set-Pending Register 0 (IRQs 0-31) */
#define NVIC_ICPR0_MORT            (NVIC_BASE_MORT + 0x0180U) /*!< Interrupt Clear-Pending Register 0 (IRQs 0-31) */


/* !<  System Control Block Registers (SCB)                                */
/* Used for: Interrupt grouping, FPU access, and low-power sleep modes.     */
#define SCB_ICSR_MORT              (SCB_BASE_MORT + 0x0004U) /*!< Interrupt Control and State Register */
#define SCB_VTOR_MORT              (SCB_BASE_MORT + 0x0008U) /*!< Vector Table Offset Register */
#define SCB_AIRCR_MORT             (SCB_BASE_MORT + 0x000CU) /*!< Application Interrupt and Reset Control Register */
#define SCB_SCR_MORT               (SCB_BASE_MORT + 0x0010U) /*!< System Control Register (Sleep/Deep-Sleep entry configuration) */
#define SCB_CCR_MORT               (SCB_BASE_MORT + 0x0014U) /*!< Configuration and Control Register */
#define SCB_CPACR_MORT             (SCB_BASE_MORT + 0x0088U) /*!< Coprocessor Access Control Register (FPU Enable) */



/* Peripheral Memory Map*/
#define FLASH_BASE_MORT            0x08000000U /*!< FLASH(up to 1 MB) base address in the alias region                         */
#define SRAM1_BASE_MORT            0x20000000U /*!< SRAM1(112 KB) base address in the alias region                             */
#define SRAM2_BASE_MORT            0x2001C000U /*!< SRAM2(16 KB) base address in the alias region                              */
#define PERIPH_BASE_MORT           0x40000000U /*!< Peripheral base address in the alias region                                */
#define BKPSRAM_BASE_MORT          0x40024000U /*!< Backup SRAM(4 KB) base address in the alias region                         */
#define FMC_R_BASE_MORT            0xA0000000U /*!< FMC registers base address                                                 */
#define QSPI_R_BASE_MORT            0xA0001000U /*!< QuadSPI registers base address                                            */

#define SRAM1_BB_BASE_MORT         0x22000000U /*!< SRAM1(112 KB) base address in the bit-band region                          */
#define SRAM2_BB_BASE_MORT         0x22380000U /*!< SRAM2(16 KB) base address in the bit-band region                           */
#define PERIPH_BB_BASE_MORT        0x42000000U /*!< Peripheral base address in the bit-band region                             */
#define BKPSRAM_BB_BASE_MORT       0x42480000U /*!< Backup SRAM(4 KB) base address in the bit-band region                      */
#define FLASH_END_MORT             0x0807FFFFU /*!< FLASH end address                                                          */


#define APB1PERIPH_BASE_MORT       PERIPH_BASE_MORT
#define APB2PERIPH_BASE_MORT       (PERIPH_BASE_MORT + 0x00010000U)
#define AHB1PERIPH_BASE_MORT       (PERIPH_BASE_MORT + 0x00020000U)
#define AHB2PERIPH_BASE_MORT       (PERIPH_BASE_MORT + 0x10000000U)

/*!< APB1 peripherals */
#define TIM2_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x0000U)
#define TIM3_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x0400U)
#define TIM4_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x0800U)
#define TIM5_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x0C00U)
#define TIM6_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x1000U)
#define TIM7_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x1400U)
#define TIM12_BASE_MORT            (APB1PERIPH_BASE_MORT + 0x1800U)
#define TIM13_BASE_MORT            (APB1PERIPH_BASE_MORT + 0x1C00U)
#define TIM14_BASE_MORT            (APB1PERIPH_BASE_MORT + 0x2000U)
#define RTC_BASE_MORT              (APB1PERIPH_BASE_MORT + 0x2800U)
#define WWDG_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x2C00U)
#define IWDG_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x3000U)
#define SPI2_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x3800U)
#define SPI3_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x3C00U)
#define SPDIFRX_BASE_MORT          (APB1PERIPH_BASE_MORT + 0x4000U)
#define USART2_BASE_MORT           (APB1PERIPH_BASE_MORT + 0x4400U)
#define USART3_BASE_MORT           (APB1PERIPH_BASE_MORT + 0x4800U)
#define UART4_BASE_MORT            (APB1PERIPH_BASE_MORT + 0x4C00U)
#define UART5_BASE_MORT            (APB1PERIPH_BASE_MORT + 0x5000U)
#define I2C1_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x5400U)
#define I2C2_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x5800U)
#define I2C3_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x5C00U)
#define FMPI2C1_BASE_MORT          (APB1PERIPH_BASE_MORT + 0x6000U)
#define CAN1_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x6400U)
#define CAN2_BASE_MORT             (APB1PERIPH_BASE_MORT + 0x6800U)
#define CEC_BASE_MORT              (APB1PERIPH_BASE_MORT + 0x6C00U)
#define PWR_BASE_MORT              (APB1PERIPH_BASE_MORT + 0x7000U)
#define DAC_BASE_MORT              (APB1PERIPH_BASE_MORT + 0x7400U)

/*!< APB2 peripherals */
#define TIM1_BASE_MORT             (APB2PERIPH_BASE_MORT + 0x0000U)
#define TIM8_BASE_MORT             (APB2PERIPH_BASE_MORT + 0x0400U)
#define USART1_BASE_MORT           (APB2PERIPH_BASE_MORT + 0x1000U)
#define USART6_BASE_MORT           (APB2PERIPH_BASE_MORT + 0x1400U)
#define ADC1_BASE_MORT             (APB2PERIPH_BASE_MORT + 0x2000U)
#define ADC2_BASE_MORT             (APB2PERIPH_BASE_MORT + 0x2100U)
#define ADC3_BASE_MORT             (APB2PERIPH_BASE_MORT + 0x2200U)
#define ADC_BASE_MORT              (APB2PERIPH_BASE_MORT + 0x2300U)
#define SDIO_BASE_MORT             (APB2PERIPH_BASE_MORT + 0x2C00U)
#define SPI1_BASE_MORT             (APB2PERIPH_BASE_MORT + 0x3000U)
#define SPI4_BASE_MORT             (APB2PERIPH_BASE_MORT + 0x3400U)
#define SYSCFG_BASE_MORT           (APB2PERIPH_BASE_MORT + 0x3800U)
#define EXTI_BASE_MORT             (APB2PERIPH_BASE_MORT + 0x3C00U)
#define TIM9_BASE_MORT             (APB2PERIPH_BASE_MORT + 0x4000U)
#define TIM10_BASE_MORT            (APB2PERIPH_BASE_MORT + 0x4400U)
#define TIM11_BASE_MORT            (APB2PERIPH_BASE_MORT + 0x4800U)
#define SAI1_BASE_MORT             (APB2PERIPH_BASE_MORT + 0x5800U)
#define SAI1_Block_A_BASE_MORT     (SAI1_BASE_MORT + 0x004U)
#define SAI1_Block_B_BASE_MORT     (SAI1_BASE_MORT + 0x024U)
#define SAI2_BASE_MORT             (APB2PERIPH_BASE_MORT + 0x5C00U)
#define SAI2_Block_A_BASE_MORT     (SAI2_BASE_MORT + 0x004U)
#define SAI2_Block_B_BASE_MORT     (SAI2_BASE_MORT + 0x024U)

/*!< AHB1 peripherals */
#define GPIOA_BASE_MORT           (AHB1PERIPH_BASE_MORT + 0x0000U)
#define GPIOB_BASE_MORT            (AHB1PERIPH_BASE_MORT + 0x0400U)
#define GPIOC_BASE_MORT            (AHB1PERIPH_BASE_MORT + 0x0800U)
#define GPIOD_BASE_MORT            (AHB1PERIPH_BASE_MORT + 0x0C00U)
#define GPIOE_BASE_MORT            (AHB1PERIPH_BASE_MORT + 0x1000U)
#define GPIOF_BASE_MORT            (AHB1PERIPH_BASE_MORT + 0x1400U)
#define GPIOG_BASE_MORT            (AHB1PERIPH_BASE_MORT + 0x1800U)
#define GPIOH_BASE_MORT            (AHB1PERIPH_BASE_MORT + 0x1C00U)
#define CRC_BASE_MORT              (AHB1PERIPH_BASE_MORT + 0x3000U)
#define RCC_BASE_MORT              (AHB1PERIPH_BASE_MORT + 0x3800U)
#define FLASH_R_BASE_MORT          (AHB1PERIPH_BASE_MORT + 0x3C00U)
#define DMA1_BASE_MORT             (AHB1PERIPH_BASE_MORT + 0x6000U)
#define DMA1_Stream0_BASE_MORT     (DMA1_BASE_MORT + 0x010U)
#define DMA1_Stream1_BASE_MORT     (DMA1_BASE_MORT + 0x028U)
#define DMA1_Stream2_BASE_MORT     (DMA1_BASE_MORT + 0x040U)
#define DMA1_Stream3_BASE_MORT     (DMA1_BASE_MORT + 0x058U)
#define DMA1_Stream4_BASE_MORT     (DMA1_BASE_MORT + 0x070U)
#define DMA1_Stream5_BASE_MORT     (DMA1_BASE_MORT + 0x088U)
#define DMA1_Stream6_BASE_MORT     (DMA1_BASE_MORT + 0x0A0U)
#define DMA1_Stream7_BASE_MORT     (DMA1_BASE_MORT + 0x0B8U)
#define DMA2_BASE_MORT             (AHB1PERIPH_BASE_MORT + 0x6400U)
#define DMA2_Stream0_BASE_MORT     (DMA2_BASE_MORT + 0x010U)
#define DMA2_Stream1_BASE_MORT     (DMA2_BASE_MORT + 0x028U)
#define DMA2_Stream2_BASE_MORT     (DMA2_BASE_MORT + 0x040U)
#define DMA2_Stream3_BASE_MORT     (DMA2_BASE_MORT + 0x058U)
#define DMA2_Stream4_BASE_MORT     (DMA2_BASE_MORT + 0x070U)
#define DMA2_Stream5_BASE_MORT     (DMA2_BASE_MORT + 0x088U)
#define DMA2_Stream6_BASE_MORT     (DMA2_BASE_MORT + 0x0A0U)
#define DMA2_Stream7_BASE_MORT     (DMA2_BASE_MORT + 0x0B8U)

/*!< AHB2 peripherals */
#define DCMI_BASE_MORT             (AHB2PERIPH_BASE_MORT + 0x50000U)

#endif /*__STM32F446XX_MORT_H_*/
