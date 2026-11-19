#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

void initMotor(void);
// Fan PWM command: 0 to 1000 represents 0% to 100% HIGH time.
void writeMotorPower(uint32_t duty);

#endif
