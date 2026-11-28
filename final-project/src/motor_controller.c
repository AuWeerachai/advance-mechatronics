#include "motor_controller.h"
#include "motor.h"

#include "fan_config.h"
#include "game_config.h"

uint32_t runMotorController(uint32_t handDistance, uint32_t airLevel)
{
    uint32_t duty;
    uint32_t airLimit;

    if(handDistance <= FAN_HAND_MIN_DISTANCE_CM)
    {
        duty = 0;  // 20 cm or an invalid reading commands 0% PWM duty.
    }
    else if(handDistance >= FAN_HAND_MAX_DISTANCE_CM)
    {
        duty = FAN_DUTY_SCALE;  // 70 cm gives 100% duty.
    }
    else
    {
        // Map 20-70 cm to 0-1000 duty units: (distance - 20) / (70 - 20).
        duty = (handDistance - FAN_HAND_MIN_DISTANCE_CM) * FAN_DUTY_SCALE;
        duty = duty / (FAN_HAND_MAX_DISTANCE_CM - FAN_HAND_MIN_DISTANCE_CM);
    }

    if(airLevel > AIR_MAXIMUM)
    {
        airLevel = AIR_MAXIMUM;
    }
    airLimit = airLevel * FAN_DUTY_SCALE / AIR_MAXIMUM; // Available air limits fan duty.
    if(duty > airLimit)
    {
        duty = airLimit;
    }
    writeMotorPower(duty);
    return duty;
}
