#ifndef __STM32F446XX_SYSTEM_MORT_H_
#define __STM32F446XX_SYSTEM_MORT_H_

/* Includes ------------------------------------------------------------------*/

/* Macros for Everyone--------------------------------------------------------*/

/*Function declarations---------------------------------------------------------*/

/**
 * @brief  Initializes the Microcontroller System Core.
 *         Configures the Floating Point Unit (FPU) coprocessor access,
 *         sets up internal flash wait states, enables overdrive voltage,
 *         and accelerates the internal HSI clock up to 180 MHz via the PLL.
 * @note   This function is called automatically by the assembly startup script
 *         (startup_stm32f4xx.s) before jumping into the main() application block.
 * @param  None
 * @retval None
 */
void SystemInit(void);

#endif /* __STM32F446XX_SYSTEM_MORT_H_ */
