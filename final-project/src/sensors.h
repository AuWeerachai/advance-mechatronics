#ifndef SENSORS_H
#define SENSORS_H
#include <stdint.h>

void initSensors(void);
uint32_t readBallSensor(void);
uint32_t readHandSensor(void);
// Shared hand/ball estimate, 20-70 cm; returns 0 outside the calibration's raw band.
uint32_t calculateSharpDistanceCm(uint32_t reading);
uint32_t readLeverSensor(void);
uint32_t readClockStartSensor(void);
uint32_t readClockEndSensor(void);

#endif
