#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

//PART 3
#define ENCODER_COUNTS_PER_MOTOR_REVOLUTION 48.0f
#define MOTOR_GEAR_RATIO 4.4f
#define ENCODER_COUNTS_PER_OUTPUT_REVOLUTION (ENCODER_COUNTS_PER_MOTOR_REVOLUTION * MOTOR_GEAR_RATIO)

//PART 3
void initEncoder(void);
void updateEncoderCount(uint32_t pins);
void updateEncoderVelocity(uint32_t elapsedMicroseconds);
int32_t readEncoderPosition(void);
float readEncoderVelocity(void);

#endif
