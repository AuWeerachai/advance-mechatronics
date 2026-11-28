#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <stdint.h>

// Apply the hand-distance command, capped by the available air percentage.
uint32_t runMotorController(uint32_t handDistance, uint32_t airLevel);

#endif
