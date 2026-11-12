/**
  ******************************************************************************
  * @file    stm32f446xx_system_mort.c 
  * @author  mortamar@andrew.cmu.edu
  * @version 2.0
  * @date    June-2026
  * @brief   Contains the function that the startup script automatically jumps to.
  *            Initializes the system clocks, flash, etc. DO NOT EDIT
  ******************************************************************************
  */

/* Includes----------------------------------------------------------*/
#include "stm32f446xx_system_mort.h"
#include "stm32f446xx_mort.h"
#include "stm32f446xx_rcc_mort.h"


/* MACRO definitions----------------------------------------------------------*/
#define FLASH_ACR			(FLASH_R_BASE_MORT + 0x00)

#define PWR_CR          (PWR_BASE_MORT + 0x00)
#define PWR_CSR         (PWR_BASE_MORT + 0x04)

/* function definitions----------------------------------------------------------*/

 /* MANDATORY C-OS STUBS */
void __libc_init_array(void) 
{
    /* Left empty intentionally */
    // Mandatory stub: The startup script calls this to initialize C library arrays.
	// We leave it empty because we are not using standard C/C++ constructors.
}


 /* HARDWARE SYSTEM INITIALIZATION */
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
void SystemInit(void) 
{
    /* Declare explicit hardware pointers */
    volatile uint32_t *reg_cpacr;
    volatile uint32_t *reg_flash_acr;
    volatile uint32_t *reg_rcc_apb1enr;
    volatile uint32_t *reg_pwr_cr;
    volatile uint32_t *reg_pwr_csr;
    volatile uint32_t *reg_rcc_cr;
    volatile uint32_t *reg_rcc_pllcfgr;
    volatile uint32_t *reg_rcc_cfgr;

    /* ENABLE THE FLOATING POINT UNIT (FPU) */
    reg_cpacr = (volatile uint32_t *)SCB_CPACR_MORT;
    /* Set bits 20, 21, 22, and 23 to grant full access to CP10 and CP11 */
    *reg_cpacr = *reg_cpacr | ((3UL << 20) | (3UL << 22));
    /*force the CPU core to pause and wait for that configuration write to firmly land in physical memory*/
    __asm__("dsb"); 
    /*(Instruction Synchronization Barrier) flushes the processor's pre-fetched instruction pipeline, ensuring that any subsequent code is fetched using the newly updated hardware contex*/
    __asm__("isb"); 

   
     /* FLASH MEMORY & VOLTAGE CONFIGURATION FOR 180 MHz */
    reg_flash_acr   = (volatile uint32_t *)FLASH_ACR;
    reg_rcc_apb1enr = (volatile uint32_t *)RCC_APB1_ENR;
    reg_pwr_cr      = (volatile uint32_t *)PWR_CR;
    reg_pwr_csr     = (volatile uint32_t *)PWR_CSR;

    /* Set 5 flash wait states, enable data cache, and enable instruction cache */
    *reg_flash_acr = (1 << 9) | (1 << 10) | (5 << 0);

    /* Turn on the clock for the Power Interface peripheral */
    *reg_rcc_apb1enr = *reg_rcc_apb1enr | (1 << 28);

    /* Turn on Over-Drive Mode (Bit 16 of PWR_CR) */
    *reg_pwr_cr = *reg_pwr_cr | (1 << 16);
    
    /* Poll until Over-Drive Ready flag (Bit 16 of PWR_CSR) becomes 1 */
    while (((*reg_pwr_csr) & (1 << 16)) == 0);

    /* Switch internal voltage regulator to Over-Drive (Bit 17 of PWR_CR) */
    *reg_pwr_cr = *reg_pwr_cr | (1 << 17);
    
    /* Poll until Over-Drive Switching Ready flag (Bit 17 of PWR_CSR) becomes 1 */
    while (((*reg_pwr_csr) & (1 << 17)) == 0);

    
     /* PHASE-LOCKED LOOP (PLL) CONFIGURATION (16 MHz HSI -> 180 MHz Clock)*/
    reg_rcc_cr       = (volatile uint32_t *)RCC_CR;
    reg_rcc_pllcfgr  = (volatile uint32_t *)RCC_PLLCFGR;
    reg_rcc_cfgr     = (volatile uint32_t *)RCC_CFGR;

    /* Poll until the High Speed Internal clock (Bit 1 of RCC_CR) is fully stable */
    while (((*reg_rcc_cr) & (1 << 1)) == 0);

    /* Write out the raw parameters directly to the main PLL configuration register
     * Formula: (16 MHz HSI input / 16 [M]) * 360 [N] / 2 [P] = 180 MHz System Core
     * Also set Q divider to 7 for standard peripheral clocks */
    *reg_rcc_pllcfgr = (16 << 0) | (360 << 6) | (0 << 16) | (0 << 22) | (7 << 24);

    /* Turn on the main PLL (Bit 24 of RCC_CR) */
    *reg_rcc_cr = *reg_rcc_cr | (1 << 24);
    
    /* Poll until the main PLL locks and becomes ready (Bit 25 of RCC_CR) */
    while (((*reg_rcc_cr) & (1 << 25)) == 0);

    /* PERIPHERAL BUS PRESCALERS & SYSTEM CLOCK SWITCH */
    
    /* Set AHB division to 1, APB1 division to 4, and APB2 division to 2 
     * This keeps downstream buses within safe hardware timing limitations */
    *reg_rcc_cfgr = *reg_rcc_cfgr | ((0 << 4) | (5 << 10) | (4 << 13));

    /* Clear existing system clock source bits and select the PLL as the master source */
    *reg_rcc_cfgr = (*reg_rcc_cfgr & ~(3 << 0));
    *reg_rcc_cfgr = *reg_rcc_cfgr | (2 << 0);

    /* Poll status bits (Bits 2-3 of RCC_CFGR) until hardware reports PLL is the active source */
    while ((((*reg_rcc_cfgr) >> 2) & 0x3) != 0x2);
}
//EOF