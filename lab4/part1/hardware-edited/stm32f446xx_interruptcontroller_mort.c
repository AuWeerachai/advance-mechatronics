/**
  ******************************************************************************
  * @file    stm32f446xx_interruptcontroller_mort.c 
  * @author  YOURNAME
  * @version 
  * @date    
  * @brief   Controls STM32F446ze NVIC access and interrupt control
  ******************************************************************************
  */

/* Includes----------------------------------------------------------*/
#include "stm32f446xx_interruptcontroller_mort.h"

//NOTE 1.7
#include "stm32f446xx_rcc_mort.h"
#include "nucleo_led.h"



/* MACRO definitions----------------------------------------------------------*/

//REGISTER ADDRESSES:

// NOTE 1.6
#define NVIC_ISER0_REGISTER ((uint32_t)0xE000E100) // Enables CPU interrupts numbered 0–31.


// NOTE 1.7 EXTI == External Interrupt == detects an edge on a pin and requests CPU attention
#define SYSCFG_EXTICR2_REGISTER (SYSCFG_BASE_MORT + 0x0C) // Select GPIO ports for INPUT source ofEXTI lines 4–7. SYSCFG_EXTICR2 selects which port supplies each EXTI line 4–7; the line number matches the pin number. We use line 6 in this case
#define EXTI_RTSR_REGISTER     (EXTI_BASE_MORT + 0x08)   // Enable rising-edge detection.
#define EXTI_FTSR_REGISTER     (EXTI_BASE_MORT + 0x0C)   // Enable falling-edge detection.
#define EXTI_PR_REGISTER       (EXTI_BASE_MORT + 0x14)   // Pending interrupt flags.
#define EXTI_IMR_REGISTER      (EXTI_BASE_MORT + 0x00)   // Allow selected EXTI interrupt requests.




// FLAGS:

// NOTE 1.6
#define NVIC_ISER0_TIMER3_ENABLE 0x20000000 // Bit 29: enable Timer 3's interrupt.

// NOTE 1.7
#define SYSCFG_EXTICR2_LINE6_PORT_MASK  0xF00    // Bits 11:8: port selection for EXTI6.
#define SYSCFG_EXTICR2_LINE6_PORTC      0x200    // Bits 11:8 = 0010: select PC6.
#define EXTI_RTSR_LINE6_ENABLE          0x40     // Bit 6: enable rising-edge detection.
#define EXTI_FTSR_LINE6_MASK            0x40     // Bit 6: clear to disable falling-edge detection.
#define EXTI_PR_LINE6_FLAG_MASK         0x40     // Bit 6: pending interrupt on line 6.
#define EXTI_IMR_LINE6_ENABLE           0x40     // Bit 6: allow line 6 interrupt requests.
#define NVIC_ISER0_EXTI9_5_ENABLE       0x800000 // Bit 23: enable EXTI lines 5–9 interrupt. let selected EXTI line interrupts the CPU


/* function definitions-------------------------------------------------------*/



// NOTE 1.6: Allow Timer 3 interrupts to reach the CPU.
// ***TRIGGER BY TIMER EVENTS***
void enableTimer3Interrupt(void)
{
  volatile uint32_t * reg_pointer;

  // Setting bit 29 to 1. enables this interrupt; zeros leave other enables unchanged.
  reg_pointer = (volatile uint32_t*) NVIC_ISER0_REGISTER;
  *reg_pointer = (uint32_t)NVIC_ISER0_TIMER3_ENABLE;
}







// NOTE 1.7: Configure PC6 to request an interrupt on each rising edge. == Similar to attachInterrupt() in arduino!
// Line 6 = interrupt-detection path inside the MCU
// ***TRIGGER BY PIN-EDGE EVENT***
void initPC6Interrupt(void)
{
  volatile uint32_t * reg_pointer;

  // Enable the system configuration controller's clock.
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  //EXTI is hardware already built into the MCU, we just internal wiring it 
  // Connect EXTI line 6 to Port C pin 6.
  reg_pointer = (volatile uint32_t*) SYSCFG_EXTICR2_REGISTER;
  *reg_pointer = *reg_pointer & ~((uint32_t)SYSCFG_EXTICR2_LINE6_PORT_MASK);
  *reg_pointer = *reg_pointer | (uint32_t)SYSCFG_EXTICR2_LINE6_PORTC;

  // Enable rising-edge detection on line 6.
  
  reg_pointer = (volatile uint32_t*) EXTI_RTSR_REGISTER;
  *reg_pointer = *reg_pointer | (uint32_t)EXTI_RTSR_LINE6_ENABLE;

  // Disable falling-edge detection on line 6.
  reg_pointer = (volatile uint32_t*) EXTI_FTSR_REGISTER;
  *reg_pointer = *reg_pointer & ~((uint32_t)EXTI_FTSR_LINE6_MASK);

  // Clear any pending line 6 interrupt.
  // Unlike Timer SR, EXTI PR requires writing 1 to clear a flag.
  reg_pointer = (volatile uint32_t*) EXTI_PR_REGISTER;
  *reg_pointer = (uint32_t)EXTI_PR_LINE6_FLAG_MASK;

  // Allow line 6 to request interrupts.
  reg_pointer = (volatile uint32_t*) EXTI_IMR_REGISTER;
  *reg_pointer = *reg_pointer | (uint32_t)EXTI_IMR_LINE6_ENABLE;

  // Enable the CPU interrupt shared by EXTI lines 5–9.
  // Writing 1 enables it; zeros leave other interrupt enables unchanged.
  reg_pointer = (volatile uint32_t*) NVIC_ISER0_REGISTER;
  *reg_pointer = (uint32_t)NVIC_ISER0_EXTI9_5_ENABLE;
}





/* Interrupt Handler----------------------------------------------------------*/

// NOTE 1.7: The name is provided by the startup interrupt table. we must use it. can't name it freely like attachInterrupt() in Arduino
void EXTI9_5_IRQHandler(void)
{
  uint32_t status;
  volatile uint32_t * reg_pointer;

  // Read the pending external-interrupt flags.
  reg_pointer = (volatile uint32_t*) EXTI_PR_REGISTER;
  status = *reg_pointer;

  // If PC6 had a rising edge, clear its flag and toggle LED1.
  if((status & (uint32_t)EXTI_PR_LINE6_FLAG_MASK) != 0)
  {
    // Write 1 to clear only line 6's flag.
    *reg_pointer = (uint32_t)EXTI_PR_LINE6_FLAG_MASK;
    toggle_LED1();
  }
}



//EOF