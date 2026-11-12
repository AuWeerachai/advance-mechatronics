#ifndef MOTOR_H
#define MOTOR_H

#include <stdint.h>

//PART 2
#define POTENTIOMETER_MIN_ADC 0     // ADC result at 0 V: motor command 0
#define POTENTIOMETER_MAX_ADC 4095  // 12 bits: 2^12 - 1 = 4095 at ADC reference voltage; motor command 1000

//PART 2
void initMotor(void);
void writeMotorPower(int32_t power);
void runMotorFromADC(uint32_t adcValue);

#endif
