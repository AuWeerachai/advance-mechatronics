#define _GNU_SOURCE
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include "stm32f446xx_rcc_mort.h"
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_adc_mort.h"
#include "stm32f446xx_dma_controller_mort.h"
#include "stm32f446xx_timer_mort.h"
#include "sensors.h"
#include "motor.h"
#include "motor_controller.h"

#define REG(a) (*(volatile uint32_t *)(uintptr_t)(a))
static uint32_t ahbClock, apbClock, apb1Clock;
void RCC_AHB1PeriphClockCmd(uint32_t bits, FunctionalState_mort s) { assert(s == ENABLE); ahbClock |= bits; }
void RCC_APB2PeriphClockCmd(uint32_t bits, FunctionalState_mort s) { assert(s == ENABLE); apbClock |= bits; }
void RCC_APB1PeriphClockCmd(uint32_t bits, FunctionalState_mort s) { assert(s == ENABLE); apb1Clock |= bits; }
void delayMicroseconds(uint32_t time) { assert(time == 10); }
static uint16_t readings[3];

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
    for(uint32_t seed=0;seed<128;seed++)
    {
        uint32_t value = seed * 1664525u + 1013904223u;
        REG(0x40020000)=value;REG(0x4002000C)=value;
        REG(0x40020800)=value;REG(0x4002080C)=value;
        initGpioA3AsAnalog();initGpioC0AsAnalog();initGpioC3AsAnalog();
        assert(REG(0x40020000)==(value|0xC0));assert(REG(0x4002000C)==(value&~0xC0u));
        assert(REG(0x40020800)==(value|0xC3));assert(REG(0x4002080C)==(value&~0xC3u));
        REG(0x40021000)=value;REG(0x4002100C)=value;
        REG(0x40021400)=value;REG(0x4002140C)=value;
        initGpioF15AsInput();initGpioE13AsInput();
        assert(REG(0x40021400)==(value&~0xC0000000u));
        assert(REG(0x4002140C)==((value&~0xC0000000u)|0x40000000u));
        assert(REG(0x40021000)==(value&~0x0C000000u));
        assert(REG(0x4002100C)==((value&~0x0C000000u)|0x04000000u));
        // Unrelated high input bits must not be mistaken for a sensor's bit.
        REG(0x40021410)=value&~0x8000u;REG(0x40021010)=value&~0x2000u;
        assert(readGpioF15()==0 && readGpioE13()==0);
        REG(0x40021410)=value|0x8000u;
        assert(readGpioF15()==1 && readGpioE13()==0);
        REG(0x40021410)=value&~0x8000u;REG(0x40021010)=value|0x2000u;
        assert(readGpioF15()==0 && readGpioE13()==1);
        REG(0x40021410)=value|0x8000u;
        assert(readGpioF15()==1 && readGpioE13()==1);

        // Configure only PE9; preserve every other pin, including the END sensor on PE13.
        REG(0x40021000)=value;REG(0x40021004)=value;
        REG(0x4002100C)=value;REG(0x40021024)=value;
        initGpioE9AsFanOutput();
        assert(REG(0x40021000)==((value&~0xC0000u)|0x80000u));
        assert(REG(0x40021004)==(value|0x200u));
        assert(REG(0x4002100C)==(value&~0xC0000u));
        assert(REG(0x40021024)==((value&~0xF0u)|0x10u));
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
    initTimer2();
    assert(REG(0x40000028)==89 && REG(0x4000002C)==0xFFFFFFFF);
    assert(REG(0x40000014)==1 && REG(0x40000010)==0 && REG(0x40000000)==1);
    REG(0x40000024)=123456;
    assert(readTimer2()==123456);
    initSensors();
    volatile uint16_t * samples = (volatile uint16_t *)(uintptr_t)REG(0x4002641C);
    samples[0]=1234;samples[1]=2345;samples[2]=170;
    assert(readBallSensor()==1234 && readHandSensor()==2345 && readLeverSensor()==170);
    assert(REG(0x40026414)==3 && REG(0x40026418)==0x4001204C);
    assert(REG(0x40012008)==0x40000703);
    puts("PASS: sensor GPIO preservation, clock input levels, ADC/DMA channel order and sample access, Timer 2 setup.");

    // Exercise the actual DMA buffer -> calibration -> controller -> fan PWM path.
    // PB0 and PD14/PD15 must remain untouched: the fan has no direction outputs.
    const uint32_t unusedGpio[] = {0x40020400,0x40020404,0x4002040C,0x40020420,
                                  0x40020C00,0x40020C04,0x40020C0C,0x40020C14,0x40020C24};
    for(uint32_t i=0;i<sizeof(unusedGpio)/sizeof(unusedGpio[0]);i++)
    {
        REG(unusedGpio[i])=0x5A5A1234;
    }
    initMotor();
    assert(REG(0x40010034)==0); // Startup command is 0% duty.
    assert(REG(0x40010028)==0 && REG(0x4001002C)==7199); // 180 MHz / 7200 = 25 kHz.
    assert(REG(0x40010018)==0x68 && REG(0x40010020)==1); // PWM1, preload, CH1 enabled.
    assert(REG(0x40010000)==1 && REG(0x4001000C)==0);
    assert(REG(0x40010030)==0 && REG(0x40010044)==0x8000); // No repetition; main output enabled.
    assert(REG(0x40010014)==1 && REG(0x40010010)==0);
    assert(apbClock&RCC_APB2Periph_TIM1);
    assert(ahbClock&RCC_AHB1Periph_GPIOE);
    assert((apb1Clock&RCC_APB1Periph_TIM3)==0);
    assert((ahbClock&RCC_AHB1Periph_GPIOB)==0);
    assert(readTimer2()==123456 && REG(0x40000028)==89); // Fan init leaves the timebase alone.

    const uint16_t adc[] = {3175,2435,1825,1780,1520,1360,1250,1180,1140,1130,3200,1129,3201,0,4095};
    const uint32_t duty[] = {0,200,400,500,600,700,800,900,1000,1000,0,0,0,0,0};
    for(uint32_t i=0;i<sizeof(adc)/sizeof(adc[0]);i++)
    {
        samples[1]=adc[i];
        assert(runMotorController()==duty[i]);
        assert(REG(0x40010034)==duty[i]*7200/1000);
    }
    uint32_t previousDuty=1000;
    for(uint32_t raw=0;raw<=4095;raw++)
    {
        samples[0]=4095-raw;samples[1]=raw;samples[2]=(raw*7)%4096;
        uint32_t command=runMotorController();
        assert(command<=1000 && REG(0x40010034)==command*7200/1000);
        if(raw<1130 || raw>3200)
        {
            assert(command==0);
        }
        else
        {
            assert(command<=previousDuty);
            previousDuty=command;
        }
    }

    writeMotorPower(500);assert(REG(0x40010034)==3600);
    writeMotorPower(1000);assert(REG(0x40010034)==7200);
    writeMotorPower(1001);assert(REG(0x40010034)==7200);
    writeMotorPower(UINT32_MAX);assert(REG(0x40010034)==7200);
    writeMotorPower(0);assert(REG(0x40010034)==0);
    for(uint32_t i=0;i<sizeof(unusedGpio)/sizeof(unusedGpio[0]);i++)
    {
        assert(REG(unusedGpio[i])==0x5A5A1234);
    }
    puts("PASS: PE9 open-drain AF1, 25 kHz fan PWM, calibrated ADC-to-duty mapping, invalid-input zero duty and no direction-pin writes.");
}
