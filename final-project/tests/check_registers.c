#define _GNU_SOURCE
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <string.h>
#include "stm32f446xx_rcc_mort.h"
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_adc_mort.h"
#include "stm32f446xx_dma_controller_mort.h"
#include "stm32f446xx_timer_mort.h"
#include "sensors.h"
#include "led.h"

#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
static uint32_t ahbClock, apbClock;
void RCC_AHB1PeriphClockCmd(uint32_t bits, FunctionalState_mort s) { assert(s == ENABLE); ahbClock |= bits; }
void RCC_APB2PeriphClockCmd(uint32_t bits, FunctionalState_mort s) { assert(s == ENABLE); apbClock |= bits; }
void RCC_APB1PeriphClockCmd(uint32_t bits, FunctionalState_mort s) { (void)bits; assert(s == ENABLE); }
void delayMicroseconds(uint32_t time) { assert(time == 10); }
static uint16_t readings[3];

static void checkAirLEDs(void)
{
    const uint32_t bases[] = {0x40020800, 0x40020C00, 0x40021800}; // C, D, G.
    const uint32_t pins[] = {0x1F00, 0xC4, 0xC};
    const uint32_t modes[] = {0x03FF0000, 0xF030, 0xF0};
    const uint32_t outputs[] = {0x01550000, 0x5010, 0x50};
    // Expected ODR bits in C, D, G for the physical D43-D52 wiring order.
    const uint32_t expected[11][3] =
    {
        {0x0000, 0x00, 0x0}, {0x0100, 0x00, 0x0}, {0x0300, 0x00, 0x0},
        {0x0700, 0x00, 0x0}, {0x0F00, 0x00, 0x0}, {0x1F00, 0x00, 0x0},
        {0x1F00, 0x04, 0x0}, {0x1F00, 0x04, 0x4}, {0x1F00, 0x04, 0xC},
        {0x1F00, 0x84, 0xC}, {0x1F00, 0xC4, 0xC}
    };
    const uint32_t counts[] = {10, 3, 3, 0, 8, 1, 9, 2, 7, 4, 6, 5, 11, UINT32_MAX};

    for(uint32_t seed = 0; seed < 128; seed++)
    {
        uint32_t initial = seed * 1664525u + 1013904223u;
        for(uint32_t port = 0; port < 3; port++)
        {
            REG(bases[port]) = initial;
            REG(bases[port] + 0x04) = initial;
            REG(bases[port] + 0x0C) = initial;
            REG(bases[port] + 0x14) = initial;
        }
        initAirLEDs();
        for(uint32_t port = 0; port < 3; port++)
        {
            assert(REG(bases[port]) == ((initial & ~modes[port]) | outputs[port]));
            assert(REG(bases[port] + 0x04) == (initial & ~pins[port]));
            assert(REG(bases[port] + 0x0C) == (initial & ~modes[port]));
            assert(REG(bases[port] + 0x14) == (initial & ~pins[port]));
        }
        for(uint32_t step = 0; step < sizeof(counts) / sizeof(counts[0]); step++)
        {
            // Disturb all output bits before each call, including repeated counts.
            uint32_t before = initial ^ (0x55555555u * step);
            uint32_t count = counts[step] > 10 ? 10 : counts[step];
            for(uint32_t port = 0; port < 3; port++)
            {
                REG(bases[port] + 0x14) = before;
            }
            writeAirLEDs(counts[step]);
            for(uint32_t port = 0; port < 3; port++)
            {
                assert(REG(bases[port] + 0x14) == ((before & ~pins[port]) | expected[count][port]));
                assert(REG(bases[port]) == ((initial & ~modes[port]) | outputs[port]));
                assert(REG(bases[port] + 0x04) == (initial & ~pins[port]));
                assert(REG(bases[port] + 0x0C) == (initial & ~modes[port]));
            }
        }
    }
    assert((ahbClock & (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG))
           == (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOG));
    puts("PASS: D43-D52 LED order, output setup, counts 0-10, repeated absolute writes and unrelated-pin preservation.");
}

int main(void)
{
    assert(calculateSharpDistanceCm(3175)==20);
    assert(calculateSharpDistanceCm(2435)==30);
    assert(calculateSharpDistanceCm(1825)==40);
    assert(calculateSharpDistanceCm(1780)==45);
    assert(calculateSharpDistanceCm(1520)==50);
    assert(calculateSharpDistanceCm(1360)==55);
    assert(calculateSharpDistanceCm(1250)==60);
    assert(calculateSharpDistanceCm(1180)==65);
    assert(calculateSharpDistanceCm(1140)==70);
    assert(calculateSharpDistanceCm(2805)==25);
    assert(calculateSharpDistanceCm(2130)==35);
    assert(calculateSharpDistanceCm(1802)==42);
    assert(calculateSharpDistanceCm(1650)==47);
    assert(calculateSharpDistanceCm(1440)==52);
    assert(calculateSharpDistanceCm(1305)==57);
    assert(calculateSharpDistanceCm(1215)==62);
    assert(calculateSharpDistanceCm(1160)==67);
    assert(calculateSharpDistanceCm(1130)==70 && calculateSharpDistanceCm(3200)==20);
    assert(calculateSharpDistanceCm(1129)==0 && calculateSharpDistanceCm(3201)==0);
    assert(calculateSharpDistanceCm(0)==0 && calculateSharpDistanceCm(4095)==0);
    for(uint32_t reading=1130;reading<3200;reading++)
    {
        uint32_t distance=calculateSharpDistanceCm(reading);
        assert(distance>=20 && distance<=70);
        assert(calculateSharpDistanceCm(reading+1)<=distance);
    }
    puts("PASS: shared Sharp calibration points, interpolation, limits and monotonic response.");
    assert(mmap((void *)0x40000000,0x30000,PROT_READ|PROT_WRITE,MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED,-1,0) != MAP_FAILED);
    checkAirLEDs();
    for(uint32_t seed=0;seed<128;seed++)
    {
        uint32_t value = seed * 1664525u + 1013904223u;
        REG(0x40020000)=value;REG(0x4002000C)=value;
        REG(0x40020800)=value;REG(0x4002080C)=value;
        initGpioA3AsAnalog();initGpioC0AsAnalog();initGpioC3AsAnalog();
        assert(REG(0x40020000)==(value|0xC0));assert(REG(0x4002000C)==(value&~0xC0u));
        assert(REG(0x40020800)==(value|0xC3));assert(REG(0x4002080C)==(value&~0xC3u));
        REG(0x40021000)=value;REG(0x40021004)=value;REG(0x4002100C)=value;REG(0x40021024)=value;
        initGpioE9AsFanOutput();
        assert(REG(0x40021000)==((value&~0xC0000u)|0x80000));
        assert(REG(0x40021004)==(value|0x200));assert(REG(0x4002100C)==(value&~0xC0000u));
        assert(REG(0x40021024)==((value&~0xF0u)|0x10));
        uint32_t fanMode = REG(0x40021000);
        uint32_t fanPull = REG(0x4002100C);
        REG(0x40021400)=value;REG(0x4002140C)=value;
        initGpioF15AsInput();initGpioE13AsInput();
        assert(REG(0x40021400)==(value&~0xC0000000u));
        assert(REG(0x4002140C)==((value&~0xC0000000u)|0x40000000u));
        assert(REG(0x40021000)==(fanMode&~0x0C000000u));
        assert(REG(0x4002100C)==((fanPull&~0x0C000000u)|0x04000000u));
        // Unrelated high input bits must not be mistaken for a sensor's bit.
        REG(0x40021410)=value&~0x8000u;REG(0x40021010)=value&~0x2000u;
        assert(readGpioF15()==0 && readGpioE13()==0);
        REG(0x40021410)=value|0x8000u;
        assert(readGpioF15()==1 && readGpioE13()==0);
        REG(0x40021410)=value&~0x8000u;REG(0x40021010)=value|0x2000u;
        assert(readGpioF15()==0 && readGpioE13()==1);
        REG(0x40021410)=value|0x8000u;
        assert(readGpioF15()==1 && readGpioE13()==1);
    }
    initDMA2Stream0(readings,3);
    assert(REG(0x40026410)==0x2D01);assert(REG(0x40026414)==3);
    assert(REG(0x40026418)==0x4001204C);assert(REG(0x4002641C)==(uint32_t)(uintptr_t)readings);
    initADC1();
    assert(REG(0x40012004)==0x100);assert(REG(0x40012008)==0x703);
    assert(REG(0x4001202C)==0x200000);assert(REG(0x40012034)==0x3543);
    assert(REG(0x40012010)==0xE00 && REG(0x4001200C)==0xE07);
    startADC1();assert(REG(0x40012008)==0x40000703);
    assert(ahbClock&RCC_AHB1Periph_DMA2);assert(apbClock&RCC_APB2Periph_ADC1);
    initTimer1FanPWM();
    assert(REG(0x40010028)==0 && REG(0x4001002C)==7199);
    assert(REG(0x40010018)==0x68 && REG(0x40010020)==1);
    assert(REG(0x40010044)==0x8000 && REG(0x40010000)==1);
    assert(REG(0x40010034)==0);
    writeTimer1FanDuty(300);assert(REG(0x40010034)==2160);
    writeTimer1FanDuty(1000);assert(REG(0x40010034)==7200);
    writeTimer1FanDuty(4000);assert(REG(0x40010034)==7200);
    writeTimer1FanDuty(0);assert(REG(0x40010034)==0);
    puts("PASS: ADC1 channels/order, DMA source/request, clock sensor inputs, GPIO preservation and fan PWM settings.");
}
