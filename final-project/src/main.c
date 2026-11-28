#include "stm32f446xx_timer_mort.h"
#include "sensors.h"
#include "motor.h"
#include "serial.h"
#include "events.h"
#include "framework.h"
#include "services.h"
#include "state_machine.h"
#include "targets.h"
#include "tachometer.h"

void initProject(void)
{
    initTimer2();
    initSerial();
    initMotor();
    initSensors();
    initTachometer();

    initFramework();
    initTargets();
    initServices();
    initStateMachines();
    initEvents();
    printGameCommands();
}

int main(void)
{
    initProject();
    while(1)
    {
        checkEvents();         // Detect changes and post events.
        serviceEventQueue();   // Handle one event through its service/state machine.
        serviceSerialOutput(); // Send at most one character, only when USART3 is ready.
    }
}
