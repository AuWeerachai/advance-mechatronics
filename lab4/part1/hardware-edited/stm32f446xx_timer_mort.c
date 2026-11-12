/**
  ******************************************************************************
  * @file    stm32f446xx_timer_mort.c 
  * @author  YOURNAME
  * @version 
  * @date    
  * @brief   Controls STM32F446ze timers
  ******************************************************************************
  */

/* Includes----------------------------------------------------------*/
#include "stm32f446xx_timer_mort.h"

//NOTE 1.6 
#include "nucleo_led.h"
#include "stm32f446xx_interruptcontroller_mort.h"


/* MACRO definitions----------------------------------------------------------*/

// REGISTER ADDRESSES:
//PSC = prescalar
//ARR = auto reload
//CCMR1 = select mode for ch 1,2 (input capture, output compare, pwm)
//CCMR2 = select mode for ch 3,4
//CCER = enable selected mode is CCMR, set polarity (rising falling)
//EGR = event generation 
//SR = status (Holds flags indicating events)
//CR1 = control registor 1 (control timer overall)
//CNT = counter (hold current count)

// NOTE 1.3: Order used in initTimer3CH3().
#define TIMER3_BASE_ADDRESS ((uint32_t)0x40000400)
#define TIMER3_PSC_REGISTER  (TIMER3_BASE_ADDRESS + 0x28) //Prescaler: controls how fast the counter counts.
#define TIMER3_ARR_REGISTER  (TIMER3_BASE_ADDRESS + 0x2C) //Auto-reload number: the counter’s maximum value.

#define TIMER3_CCR3_REGISTER (TIMER3_BASE_ADDRESS + 0x3C) //Compare value: the count that we will use to triggers channel 3.
#define TIMER3_CCMR2_REGISTER (TIMER3_BASE_ADDRESS + 0x1C) //Configure channels 3 and 4.
#define TIMER3_CCER_REGISTER  (TIMER3_BASE_ADDRESS + 0x20) //Enable channels and select their polarity.
#define TIMER3_EGR_REGISTER   (TIMER3_BASE_ADDRESS + 0x14) //Apply the prescaler setting before starting
#define TIMER3_SR_REGISTER    (TIMER3_BASE_ADDRESS + 0x10) //Clear pending event flags
#define TIMER3_CR1_REGISTER   (TIMER3_BASE_ADDRESS + 0x00) //Configure counting and start the timer

// NOTE 1.4: Additional addresses; shared addresses are defined above.
#define TIMER3_CCR1_REGISTER  (TIMER3_BASE_ADDRESS + 0x34) // Holds the counter value captured by channel 1.
#define TIMER3_CCMR1_REGISTER (TIMER3_BASE_ADDRESS + 0x18) // Mode configure for channels 1 and 2.


// NOTE 1.6 for interrupt
#define TIMER3_DIER_REGISTER (TIMER3_BASE_ADDRESS + 0x0C)  // DMA/Interrupt Enable Register: selects which Timer 3 events can request interrupts or DMA transfers.






// FLAGS:
// NOTE 1.3: Order used in initTimer3CH3().
#define TIMER3_CCMR2_CH3_SELECTION_MASK           0x03   //00000011 binary: mask for bits 1:0. AND with its inverse clears 2 rightmost bits to 00, selecting channel 3 as output.
#define TIMER3_CCMR2_CH3_MODE_MASK                0x70   //01110000 binary: mask for bits 6:4. AND with its inverse to clear previous CH3 mode
#define TIMER3_CCMR2_CH3_MODE_TOGGLE              0x30   //00110000 binary: OR with this sets bits 6:4 to 011 after clearing, selecting toggle on each CCR3 match.
#define TIMER3_CCER_CH3_ENABLE                    0x100  // Bit 8: enable channel 3's output.
#define TIMER3_EGR_UPDATE_GENERATE                0x01   // Bit 0 in EGR: generate an update to apply the prescaler.
#define TIMER3_CR1_COUNTER_ENABLE                 0x01   // Bit 0 in CR1: enable the timer counter.

// NOTE 1.4: Channel 1 input-capture flags.
#define TIMER3_CCMR1_CH1_SELECTION_MASK           0x03   // CCMR1 bits 1:0: mask for channel 1 input/output selection.
#define TIMER3_CCMR1_CH1_INPUT_TI1                0x01   // CCMR1 bits 1:0 = 01: connect capture channel 1 to input TI1.
#define TIMER3_CCMR1_CH1_CONFIG_MASK              0xFF   // Bits 7:0: channel 1 selection, capture prescaler, and input filter.
#define TIMER3_CCER_CH1_POLARITY_P_MASK           0x02   // CCER bit 1: polarity selection; clear for rising edge.
#define TIMER3_CCER_CH1_POLARITY_NP_MASK          0x08   // CCER bit 3: also clear for rising-edge capture.
#define TIMER3_CCER_CH1_ENABLE                    0x01   // CCER bit 0: enable channel 1 capture.
#define TIMER3_SR_CH1_CAPTURE_FLAG_MASK           0x02   // SR bit 1: flag indicating a channel 1 capture occurred.

//NOTE 1.5  PWM time!
#define TIMER3_CCMR2_CH3_MODE_PWM1 0x60                  // Bits 6:4 = 110: PWM mode 1.

//NOTE 1.6
#define TIMER3_CCMR2_CH3_MODE_FROZEN 0x00                // Bits 6:4 = 000: compare matches do not change the timer output.
#define TIMER3_DIER_CH3_INTERRUPT_ENABLE 0x08            // DIER bit 3 = 1: enable an interrupt when Timer 3's counter matches CCR3.
#define TIMER3_DIER_UPDATE_INTERRUPT_ENABLE 0x01         // DIER bit 0 = 1: enable update interrupts, including when the counter overflows and restarts at 0.

#define TIMER3_SR_COMPARE_MATCH_FLAG_MASK 0x08             // SR bit 3: channel 3 compare match occurred. at 20000 count. == toggle 1
#define TIMER3_SR_OVERFLOW_FLAG_MASK      0x01            // SR bit 0: update/overflow occurred. when the counter changes from 39999 to 0. (reload new round)





/* function definitions-------------------------------------------------------*/

// NOTE 1.3 timer toggles output on compare matches. (flip output at match)
// void initTimer3CH3 (void){
// 
//   //step
//   // we need 0.5 second clock 
//   // the supplied clock give timer 3 = 90 MHz, we need to slow down
//   // PSC == use to slow down counitng == set PSC to 8999 so that  90,000,000 / (8999 + 1) == 10000 == 10KHz
//   // ARR == set when counter restart == set to 4999 so it counts from 0-4999 = 5000 counts == 5000/10000 = 0.5 s (10000 counts/second, set recount to 5000, so 0.5 second)
//   
//   //initialize timer -> slow it down -> set repeating loop interval -> set trigger -> set channel as output -> set action when triggered (toggle) 
//   //-> enable that channel as output -> apply prescale setting + initialize timer -> clear event flags generated during initialize -> start counting
// 
//   volatile uint32_t * reg_pointer;
// 
//   //initialize timer that is reference
//   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
// 
//   //PSC
//   reg_pointer = (volatile uint32_t *) TIMER3_PSC_REGISTER;
//   *reg_pointer = 8999;
// 
//   //ARR
//   // We need the timer counter to repeat every 0.5 seconds.
//   reg_pointer = (volatile uint32_t*) TIMER3_ARR_REGISTER;
//   *reg_pointer = (uint32_t)4999;
// 
// 
//   // SPECIFIC TO CHANNEL 3 BEGIN HERE--------------------------------------------------------
//   
//   //When Timer 3's counter reaches 4999, channel 3 of timer 3 got trigger
//   //like setting a trigger point
//   reg_pointer = (volatile uint32_t*) TIMER3_CCR3_REGISTER;
//   *reg_pointer = (uint32_t)4999;     //select by choice. as long as it's 0.5 seconds apart (which is any number from 0-ARR since we set PSC,ARR already) is fine
// 
// 
//   //Configure channel 3 of timer 3 as output 
//   reg_pointer = (volatile uint32_t*) TIMER3_CCMR2_REGISTER;
//   *reg_pointer = *reg_pointer & ~((uint32_t)TIMER3_CCMR2_CH3_SELECTION_MASK);
//   // Configure channel 3 to toggle when counter matches CCR3
//   *reg_pointer = *reg_pointer & ~((uint32_t)TIMER3_CCMR2_CH3_MODE_MASK);
//   *reg_pointer = *reg_pointer | (uint32_t)TIMER3_CCMR2_CH3_MODE_TOGGLE;
// 
// 
//   // Enable channel 3's output 
//   reg_pointer = (volatile uint32_t*) TIMER3_CCER_REGISTER;
//   *reg_pointer = *reg_pointer | (uint32_t)TIMER3_CCER_CH3_ENABLE;
// 
// 
//   // Apply the prescaler setting and initialize the counter 
//   reg_pointer = (volatile uint32_t*) TIMER3_EGR_REGISTER;
//   *reg_pointer = (uint32_t)TIMER3_EGR_UPDATE_GENERATE;
// 
// 
//   // Clear event flags generated during initialization 
//   reg_pointer = (volatile uint32_t*) TIMER3_SR_REGISTER;
//   *reg_pointer = (uint32_t)0;
// 
//   
//   // Start counting 
//   reg_pointer = (volatile uint32_t*) TIMER3_CR1_REGISTER;
//   *reg_pointer = *reg_pointer | (uint32_t)TIMER3_CR1_COUNTER_ENABLE;
// }






// // NOTE 1.5   Timer drives output using PWM. (set output low/high)
// void initTimer3CH3 (void){
// 
//   // Timer 3 receives a 90 MHz clock.
//   // PSC = 8999: 90,000,000 / (8999 + 1) = 10,000 counts per second.
//   // ARR = 39999: counts 0-39999, giving 40,000 counts = 4 seconds per cycle.
//   // CCR3 = 20000: PWM output is HIGH for counts 0-19999 and LOW for 20000-39999.
//   // This gives 2 seconds ON and 2 seconds OFF: 50% duty cycle at 0.25 Hz.
// 
//   // Enable clock -> set counting speed -> set period -> set duty cycle
//   // -> select channel 3 output and PWM mode -> enable output
//   // -> apply settings -> clear event flags -> start counting.
// 
//   volatile uint32_t * reg_pointer;
// 
//   // Enable Timer 3's peripheral clock.
//   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
// 
//   //PSC
//   reg_pointer = (volatile uint32_t *) TIMER3_PSC_REGISTER;
//   *reg_pointer = 8999; //90,000,000 / (8999 + 1) == 10000 == 10KHz = 10,000 counts/second
// 
//   //ARR
//   reg_pointer = (volatile uint32_t*) TIMER3_ARR_REGISTER;
//   *reg_pointer = (uint32_t)39999; // 40,000 counts: counter repeats every 4 seconds.
// 
// 
//   // SPECIFIC TO CHANNEL 3 BEGIN HERE--------------------------------------------------------
//   
//   // Set the PWM threshold: output goes LOW at count 20000.
//   // Output goes HIGH again when the counter restarts at 0.
//   reg_pointer = (volatile uint32_t*) TIMER3_CCR3_REGISTER;
//   *reg_pointer = (uint32_t)20000;  // 20,000 / 40,000 = 50% duty cycle: 2 seconds ON, 2 seconds OFF.
// 
// 
//   //Configure channel 3 of timer 3 as output 
//   reg_pointer = (volatile uint32_t*) TIMER3_CCMR2_REGISTER;
//   *reg_pointer = *reg_pointer & ~((uint32_t)TIMER3_CCMR2_CH3_SELECTION_MASK);
//   // Configure channel 3 to PWM for Question 1.5
//   *reg_pointer = *reg_pointer & ~((uint32_t)TIMER3_CCMR2_CH3_MODE_MASK);
//   *reg_pointer = *reg_pointer | (uint32_t)TIMER3_CCMR2_CH3_MODE_PWM1;
// 
// 
//   // Enable channel 3's output 
//   reg_pointer = (volatile uint32_t*) TIMER3_CCER_REGISTER;
//   *reg_pointer = *reg_pointer | (uint32_t)TIMER3_CCER_CH3_ENABLE;
// 
// 
//   // Apply the prescaler setting and initialize the counter 
//   reg_pointer = (volatile uint32_t*) TIMER3_EGR_REGISTER;
//   *reg_pointer = (uint32_t)TIMER3_EGR_UPDATE_GENERATE;
// 
// 
//   // Clear event flags generated during initialization 
//   reg_pointer = (volatile uint32_t*) TIMER3_SR_REGISTER;
//   *reg_pointer = (uint32_t)0;
// 
//   
//   // Start counting 
//   reg_pointer = (volatile uint32_t*) TIMER3_CR1_REGISTER;
//   *reg_pointer = *reg_pointer | (uint32_t)TIMER3_CR1_COUNTER_ENABLE;
// }







// NOTE 1.6: Configure Timer 3 for compare-match and overflow interrupts.
void initTimer3CH3(void)
{
// PSC = 8999: 10,000 counts per second.
// ARR = 39999: counter repeats every 4 seconds. (40,000 counts take 4 second at rate of 10,000 count/second)
// CCR3 = 20000: compare match halfway through each cycle.
// Once interrupts are enabled, the handler will toggle LED1
// at compare match and overflow: approximately every 2 seconds.
  volatile uint32_t * reg_pointer;

  // Enable Timer 3's peripheral clock.
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  // Set counting speed to 10,000 counts per second.
  reg_pointer = (volatile uint32_t*) TIMER3_PSC_REGISTER;
  *reg_pointer = (uint32_t)8999;

  // Counter repeats every 4 seconds.
  reg_pointer = (volatile uint32_t*) TIMER3_ARR_REGISTER;
  *reg_pointer = (uint32_t)39999;

  // Compare match occurs approximately 2 seconds into each cycle.
  reg_pointer = (volatile uint32_t*) TIMER3_CCR3_REGISTER;
  *reg_pointer = (uint32_t)20000;

  // Configure channel 3 for output compare. (0b00 == output compare)
  reg_pointer = (volatile uint32_t*) TIMER3_CCMR2_REGISTER;
  *reg_pointer = *reg_pointer & ~((uint32_t)TIMER3_CCMR2_CH3_SELECTION_MASK);

  // Frozen mode/ frozen to ensure that the timer does not change its output on a match by itself .
  *reg_pointer = *reg_pointer & ~((uint32_t)TIMER3_CCMR2_CH3_MODE_MASK);
  *reg_pointer = *reg_pointer | (uint32_t)TIMER3_CCMR2_CH3_MODE_FROZEN;

  // Apply the prescaler setting and initialize the counter.
  reg_pointer = (volatile uint32_t*) TIMER3_EGR_REGISTER;
  *reg_pointer = (uint32_t)TIMER3_EGR_UPDATE_GENERATE;

  // Clear initialization flags before enabling interrupts.
  reg_pointer = (volatile uint32_t*) TIMER3_SR_REGISTER;
  *reg_pointer = (uint32_t)0;

  // Enable channel 3 compare-match interrupts.: when match == request interrupt (@ 20,000 count)
  reg_pointer = (volatile uint32_t*) TIMER3_DIER_REGISTER;
  *reg_pointer = *reg_pointer | (uint32_t)TIMER3_DIER_CH3_INTERRUPT_ENABLE;

  // Enable overflow interrupts.: when about the end of clock cycle == request interrupt  (@ counter changes from 39999 (ARR) to 0)
  *reg_pointer = *reg_pointer | (uint32_t)TIMER3_DIER_UPDATE_INTERRUPT_ENABLE;

  // Allow Timer 3 interrupts through the CPU's interrupt controller.
  enableTimer3Interrupt();

  // Start counting.
  reg_pointer = (volatile uint32_t*) TIMER3_CR1_REGISTER;
  *reg_pointer = *reg_pointer | (uint32_t)TIMER3_CR1_COUNTER_ENABLE;
}






// NOTE 1.4: Channel 1 input capture.
void initTimer3CH1(void)
{
  volatile uint32_t * reg_pointer;

  // Enable Timer 3's peripheral clock.
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  // Set counting speed to 10,000 counts per second.
  reg_pointer = (volatile uint32_t*) TIMER3_PSC_REGISTER;
  *reg_pointer = (uint32_t)8999;

  // Count from 0 to 4999: repeat every 0.5 seconds.
  reg_pointer = (volatile uint32_t*) TIMER3_ARR_REGISTER;
  *reg_pointer = (uint32_t)4999;



  // SPECIFIC TO CHANNEL 1 BEGIN HERE-----------------------

  // Disable channel 1 capture before changing its configuration.
  reg_pointer = (volatile uint32_t*) TIMER3_CCER_REGISTER;
  *reg_pointer = *reg_pointer & ~((uint32_t)TIMER3_CCER_CH1_ENABLE);

  // Clear channel 1's selection, input filter, and capture prescaler.
  reg_pointer = (volatile uint32_t*) TIMER3_CCMR1_REGISTER;
  *reg_pointer = *reg_pointer & ~((uint32_t)TIMER3_CCMR1_CH1_CONFIG_MASK);

  // Select TI1 as channel 1's input, with no filter or capture prescaler.    *** TI1 is from PC6 from set up wiring later in gpio_mort
  *reg_pointer = *reg_pointer | (uint32_t)TIMER3_CCMR1_CH1_INPUT_TI1;

  // Select rising-edge capture: clear both polarity bits.                    *** When PC6 has a rising edge, Timer 3 channel 1 capture the current counter value into CCR1 + sets SR bit 1 to 1, indicating a capture occurred.
  reg_pointer = (volatile uint32_t*) TIMER3_CCER_REGISTER;
  *reg_pointer = *reg_pointer & ~((uint32_t)TIMER3_CCER_CH1_POLARITY_P_MASK);
  *reg_pointer = *reg_pointer & ~((uint32_t)TIMER3_CCER_CH1_POLARITY_NP_MASK);

  // Enable channel 1 capture.
  *reg_pointer = *reg_pointer | (uint32_t)TIMER3_CCER_CH1_ENABLE;

  // Apply the prescaler setting and initialize the counter.
  reg_pointer = (volatile uint32_t*) TIMER3_EGR_REGISTER;
  *reg_pointer = (uint32_t)TIMER3_EGR_UPDATE_GENERATE;

  // Clear event flags generated during initialization.
  reg_pointer = (volatile uint32_t*) TIMER3_SR_REGISTER;
  *reg_pointer = (uint32_t)0;

  // Start counting.
  reg_pointer = (volatile uint32_t*) TIMER3_CR1_REGISTER;
  *reg_pointer = *reg_pointer | (uint32_t)TIMER3_CR1_COUNTER_ENABLE;
}

// NOTE 1.4
uint32_t checkTimer3CH1Capture(void)
{
  volatile uint32_t * reg_pointer;

  reg_pointer = (volatile uint32_t*) TIMER3_SR_REGISTER;
  return *reg_pointer & (uint32_t)TIMER3_SR_CH1_CAPTURE_FLAG_MASK;
}

// NOTE 1.4
// Clear the capture flag and read the captured counter value.
uint32_t readTimer3CH1Capture(void)
{
  volatile uint32_t * reg_pointer;

  // Writing 0 clears the capture flag; writing 1 leaves other flags unchanged.
  reg_pointer = (volatile uint32_t*) TIMER3_SR_REGISTER;
  *reg_pointer = ~((uint32_t)TIMER3_SR_CH1_CAPTURE_FLAG_MASK);

  reg_pointer = (volatile uint32_t*) TIMER3_CCR1_REGISTER;  //CCR1 get updated automatically due to setting TI1 and rising edge capture commented as *** above.

  return *reg_pointer;
}




/* Interrupt Handler----------------------------------------------------------*/


// NOTE 1.6: Called automatically when Timer 3 interrupts. Name is provided by start up file though. 
// It's listed hardware/startup_stm32f4xx.s, function address is put in interrupt vector table. When timer 3 interrupt, CPU read this address and jump function for us automatically
void TIM3_IRQHandler(void)
{
  uint32_t status;
  volatile uint32_t * reg_pointer;

  // Read Timer 3's event flags.
  reg_pointer = (volatile uint32_t*) TIMER3_SR_REGISTER;
  status = *reg_pointer;

  // Compare match: check SR bit 3, if true then clear SR bit 3, then toggle LED1.
  if((status & (uint32_t)TIMER3_SR_COMPARE_MATCH_FLAG_MASK) != 0)
  {
    // Writing 0 clears this flag; writing 1 preserves other flags.
    *reg_pointer = ~((uint32_t)TIMER3_SR_COMPARE_MATCH_FLAG_MASK);
    toggle_LED1();
  }

  // Overflow: clear SR bit 0, then toggle LED1.
  if((status & (uint32_t)TIMER3_SR_OVERFLOW_FLAG_MASK) != 0)
  {
    *reg_pointer = ~((uint32_t)TIMER3_SR_OVERFLOW_FLAG_MASK);
    toggle_LED1();
  }
}


//EOF
