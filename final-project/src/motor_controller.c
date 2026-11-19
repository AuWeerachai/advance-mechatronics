#include "motor_controller.h"
#include "motor.h"
#include "sensors.h"

#define MOTOR_HAND_MIN_DISTANCE_CM ((uint32_t)20)
#define MOTOR_HAND_MAX_DISTANCE_CM ((uint32_t)70)

uint32_t runMotorController(void)
{
    uint32_t handReading;
    uint32_t handDistance;
    uint32_t duty;

    handReading = readHandSensor();  // Read A1 / PC0 from the ADC/DMA buffer.
    handDistance = calculateSharpDistanceCm(handReading);

    if(handDistance <= MOTOR_HAND_MIN_DISTANCE_CM)
    {
        duty = 0;  // 20 cm or an invalid reading commands 0% PWM duty.
    }
    else if(handDistance >= MOTOR_HAND_MAX_DISTANCE_CM)
    {
        duty = 1000;  // 70 cm gives 100% duty.
    }
    else
    {
        // Map 20-70 cm to 0-1000 duty units: (distance - 20) / (70 - 20).
        duty = (handDistance - MOTOR_HAND_MIN_DISTANCE_CM) * 1000;
        duty = duty / (MOTOR_HAND_MAX_DISTANCE_CM - MOTOR_HAND_MIN_DISTANCE_CM);
    }

    writeMotorPower(duty);          // Apply the hand-controlled fan PWM duty.
    return duty;                    // Main prints the command as a percentage.
}
