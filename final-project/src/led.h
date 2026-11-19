#ifndef LED_H
#define LED_H

#include <stdint.h>

// Air indicator: ordered from the first LED to the tenth LED.
#define AIR_LED_COUNT ((uint32_t)10)
void initAirLEDs(void);
void writeAirLEDs(uint32_t ledCount);

#endif
