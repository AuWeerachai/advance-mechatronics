#ifndef FAN_H
#define FAN_H
#include <stdint.h>

void initFan(void);
void writeFanPower(uint32_t duty);

#endif
