#define _GNU_SOURCE
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include "tachometer.h"
#include "fan_config.h"
#include "events.h"
#include "stm32f446xx_interruptcontroller_mort.h"
#include "stm32f446xx_rcc_mort.h"

#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
#define IMR REG(0x40013C00)
#define PR REG(0x40013C14)
static uint32_t now, gpioInit, clockInit, events, lastRPM;
static uint32_t acceptEvent = 1, pulseDuringPost;
uint32_t readTimer2(void) { return now; }
void initGpioD12AsFanTachInput(void) { gpioInit++; }
void RCC_APB2PeriphClockCmd(uint32_t bits, FunctionalState_mort state)
{
    assert(bits == RCC_APB2Periph_SYSCFG && state == ENABLE);
    clockInit++;
}

static void pulse(uint32_t count)
{
    for(uint32_t i = 0; i < count; i++)
    {
        PR = 0x1000;
        EXTI15_10_IRQHandler();
        assert(PR == 0x1000); // Check the W1C write, then emulate flag clearing.
        PR = 0;
    }
}

uint32_t postEvent(enum EventType type, uint32_t value)
{
    assert(type == FAN_RPM_READY);
    events++; lastRPM = value;
    if(pulseDuringPost) { pulse(1); pulseDuringPost = 0; }
    return acceptEvent;
}

int main(void)
{
    assert(mmap((void *)0x40000000, 0x30000, PROT_READ|PROT_WRITE,
                MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED, -1, 0) != MAP_FAILED);
    assert(mmap((void *)0xE000E000, 4096, PROT_READ|PROT_WRITE,
                MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED, -1, 0) != MAP_FAILED);
    IMR = 0x2000;
    REG(0x40013814) = 0xAAAA;
    REG(0x40013C08) = 0x2000;
    REG(0x40013C0C) = 0x3000;
    PR = 0x3000;
    REG(0x40000800) = 0x12345678; // Timer 4 must remain untouched.
    initTachometer();
    if(FAN_TACH_CONNECTED == 0)
    {
        now = 1000000; checkTachometerEvent();
        assert(!gpioInit && !clockInit && !events);
        assert(IMR == 0x2000 && REG(0x40013814) == 0xAAAA);
        assert(REG(0xE000E104) == 0);
        puts("PASS: unconfirmed tach wiring leaves GPIO and interrupt setup untouched.");
        return 0;
    }
    assert(gpioInit == 1 && clockInit == 1 && readFanTachPulses() == 0);
    assert(REG(0x40013814) == 0xAAA3); // EXTI12 selects D, other lines preserved.
    assert(REG(0x40013C08) == 0x3000 && REG(0x40013C0C) == 0x2000);
    assert(PR == 0x1000 && IMR == 0x3000);
    assert(REG(0xE000E104) == 0x100); // NVIC IRQ40 enable, W1S.
    assert(REG(0x40000800) == 0x12345678);

    PR = 0x2000; EXTI15_10_IRQHandler(); // Other line cannot count as a tach pulse.
    assert(readFanTachPulses() == 0 && PR == 0x2000);
    IMR = 0x2000; PR = 0x1000; EXTI15_10_IRQHandler();
    assert(readFanTachPulses() == 0); // Masked tach interrupt does not count.
    IMR = 0x3000; PR = 0x3000; EXTI15_10_IRQHandler();
    assert(readFanTachPulses() == 1 && PR == 0x1000);
    PR = 0; EXTI15_10_IRQHandler(); assert(readFanTachPulses() == 1);
    pulse(99);
    assert(events == 0); // The handler never posts into the main-loop event queue.
    puts("PASS: PD12 EXTI routing, rising-edge enable, shared IRQ filtering and pulse counting.");

    now = FAN_TACH_INTERVAL_US - 1; checkTachometerEvent(); assert(!events);
    now++; checkTachometerEvent(); assert(events == 1 && lastRPM == 6000);
    now += FAN_TACH_INTERVAL_US; checkTachometerEvent(); assert(lastRPM == 0);
    now += 750000; pulse(150); checkTachometerEvent(); assert(lastRPM == 6000);

    // Wrap both the pulse count and Timer 2; no main-loop reset of the pulse count.
    now = UINT32_MAX - 1000; initTachometer(); PR = 0;
    pulse(65530); now += FAN_TACH_INTERVAL_US; checkTachometerEvent();
    pulse(100); now += FAN_TACH_INTERVAL_US; checkTachometerEvent();
    assert(readFanTachPulses() == 94 && lastRPM == 6000);

    // A pulse after the snapshot must be retained for the next reading.
    pulseDuringPost = 1;
    now += FAN_TACH_INTERVAL_US; checkTachometerEvent(); assert(lastRPM == 0);
    pulse(99); now += FAN_TACH_INTERVAL_US; checkTachometerEvent(); assert(lastRPM == 6000);

    // A full event queue must not discard the interval's pulses.
    acceptEvent = 0; pulse(100); now += FAN_TACH_INTERVAL_US; checkTachometerEvent();
    acceptEvent = 1; pulse(100); now += FAN_TACH_INTERVAL_US; checkTachometerEvent();
    assert(lastRPM == 6000);
    puts("PASS: tach RPM, actual elapsed time, zero pulses, count/time wrap and pulses retained across reads.");
}
