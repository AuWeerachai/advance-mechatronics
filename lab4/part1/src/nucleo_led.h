#ifndef __NUCLEO_LED_H_
#define __NUCLEO_LED_H_



/* Includes ------------------------------------------------------------------*/


/* Macros for Everyone--------------------------------------------------------*/

/*Function definitions---------------------------------------------------------*/

//NOTE 1.1 declare ON OFF function in addition.
void init_LED1(void);
void toggle_LED1(void);
void turnOn_LED1(void);
void turnOff_LED1(void);

//NOTE 1.2
void init_Reading(void);
void turnON_LED1_with_reading(void);

//NOTE 1.3
void turnOn_LED1_with_timer(void);

#endif /*__NUCLEO_LED_H */