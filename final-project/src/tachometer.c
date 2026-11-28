#include "tachometer.h"
#include "fan_config.h"
#include "framework.h"
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_interruptcontroller_mort.h"
#include "stm32f446xx_timer_mort.h"

static uint32_t previousCount;
static uint32_t previousTime;

void initTachometer(void)
{
    previousCount = 0;
    previousTime = readTimer2();
    if(FAN_TACH_CONNECTED != 0)
    {
        initGpioD12AsFanTachInput();
        initFanTachInterrupt();
        previousCount = readFanTachPulses();
    }
}

void checkTachometerEvent(void)
{
    uint32_t now;
    uint32_t elapsed;
    uint32_t count;
    uint32_t pulses;
    uint32_t rpm;

    if(FAN_TACH_CONNECTED == 0)
    {
        return;
    }
    now = readTimer2();
    elapsed = now - previousTime;
    if(elapsed < FAN_TACH_INTERVAL_US)
    {
        return;
    }

    count = readFanTachPulses();
    pulses = (count - previousCount) & (uint32_t)0xFFFF; // The interrupt pulse count wraps after 65535.
    // RPM = pulses / pulses-per-revolution / elapsed seconds * 60.
    // Use whole milliseconds: even 65535 pulses * 60000 fits in uint32_t.
    // This uses the MCU's 32-bit division, without a 64-bit division library.
    rpm = pulses * 60000U / (elapsed / 1000U) / FAN_TACH_PULSES_PER_REVOLUTION;
    if(postEvent(FAN_RPM_READY, rpm) != 0)
    {
        previousCount = count;
        previousTime = now;
    }
}
