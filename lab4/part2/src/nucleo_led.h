#ifndef __NUCLEO_LED_H_
#define __NUCLEO_LED_H_

#include <stdint.h>

void initLEDs(void);
void writeRedDetection(uint32_t value);
void writeGreenDetection(uint32_t value);
void writeOrangeDetection(uint32_t value);
void writeWhiteDetection(uint32_t value);
void writeRedIllumination(uint32_t value);
void writeGreenIllumination(uint32_t value);
void writeBlueIllumination(uint32_t value);

#endif
