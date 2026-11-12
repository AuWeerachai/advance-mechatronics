#ifndef ENCODER_H
#define ENCODER_H

#include <stdint.h>

//PART 3
#define ENCODER_COUNTS_PER_MOTOR_REVOLUTION 48.0f                                                      // Lecture 16, magnetic encoder: 48 counts per motor revolution when counting both edges of A and B
#define MOTOR_GEAR_RATIO 4.4f                                                                          // Lecture 16, Pololu motor: 4.4 motor revolutions per output-shaft revolution
#define ENCODER_COUNTS_PER_OUTPUT_REVOLUTION (ENCODER_COUNTS_PER_MOTOR_REVOLUTION * MOTOR_GEAR_RATIO)  // 48 * 4.4 = 211.2 counts per output-shaft revolution

//PART 3
void initEncoder(void);
void updateEncoderCount(uint32_t pins);
void updateEncoderVelocity(uint32_t elapsedMicroseconds);
int32_t readEncoderPosition(void);
float readEncoderVelocity(void);

#endif
