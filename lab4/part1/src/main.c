
/**
  ******************************************************************************
  * @file    main.c 
  * @author  YOU
  * @version 
  * @date    
  * @brief   after the system functions, main is the next function that gets called.
  *             You should modify main and call other functions from here.
  ******************************************************************************
  */
#include <stdint.h>
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_system_mort.h"
#include "stm32f446xx_usart3_mort.h" //always include usart where you want to do communications
#include "nucleo_led.h"

//NOTE 1.4
#include "stm32f446xx_timer_mort.h"

//NOTE 1.7
#include "stm32f446xx_interruptcontroller_mort.h"



// Simple delay function using software loops, not very good.
// you should implement your own eventually using timers once
// you learn about them.
void delay(volatile int count) {
    while (count--) {
        __asm__("nop");
    }
}


int main (void)
{
    /* Variables*/
    // uint32_t number = 0;
    /* Initializations */
    initUSART3(); //always init USART if you want communication!
    init_LED1();
    delay(8000); //give some time for USART to come alive.
    transmitLineUSART3("USART3 is alive!");

    //NOTE 1.2
    // init_Reading();

    //NOTE 1.3 with toggle mode
    //NOTE 1.5 with PWM mode
    // turnOn_LED1_with_timer();

    // NOTE 1.4
    // initGpioC6asAF2(); // Connect PC6 internally to Timer 3 channel 1 using AF2.
    // initTimer3CH1();   //Start Timer 3 and configure channel 1 to capture the counter value when PC6 changes from LOW to HIGH.

    //NOTE 1.6 only init. The timer interrupt handler toggles LED1
    // and then Initialize the timer for compare-match and overflow interrupts.
    // initGpioB0AsOutput();
    // initTimer3CH3();

    // NOTE 1.7
    // initGpioC6AsInput(); // Configure PC6 as a normal input with pull-down.
    // initPC6Interrupt(); // Enable rising-edge interrupts from PC6.
    

    while(1)
    {
    	// number = number + 1;
        //USART supports a very simple printf function, check its function declaration for learning what it can do.
        // transmitLineUSART3("This board is alive!");
        // printfUSART3("This number keeps increasing: %u \n", number);

        //NOTE 1.1
        delay(800000);
        toggle_LED1();

        //NOTE 1.2
        // turnON_LED1_with_reading();


        // NOTE 1.4: Check for a capture, read the saved count, and print it.
        // if(checkTimer3CH1Capture() != 0)
        // {
        //     uint32_t captured_value;
        //     captured_value = readTimer3CH1Capture();
        //     printfUSART3("Captured count: %u\n", captured_value);
        // }


    }
}








