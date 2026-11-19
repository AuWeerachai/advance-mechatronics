#include "sensors.h"
#include "calibration.h"
#include "stm32f446xx_gpio_mort.h"
#include "stm32f446xx_adc_mort.h"
#include "stm32f446xx_dma_controller_mort.h"

static volatile uint16_t sensorReadings[3];  // DMA updates ball, hand, then lever repeatedly.

void initSensors(void)
{
    // Configure the inputs and DMA destination before starting ADC conversion.
    initGpioA3AsAnalog();
    initGpioC0AsAnalog();
    initGpioC3AsAnalog();
    initGpioF15AsInput();
    initGpioE13AsInput();
    initDMA2Stream0(sensorReadings, 3);
    initADC1();
    startADC1();
}

uint32_t readBallSensor(void)
{
    return (uint32_t)sensorReadings[0];  // A0 / PA3: raw ball sensor reading, 0-4095.
}

uint32_t readHandSensor(void)
{
    return (uint32_t)sensorReadings[1];  // A1 / PC0: raw hand sensor reading, 0-4095.
}

uint32_t readLeverSensor(void)
{
    return (uint32_t)sensorReadings[2];  // A2 / PC3: raw lever position, 0-4095.
}


uint32_t calculateSharpDistanceCm(uint32_t reading)
{
    // Shared calibration assumes the hand or ball is physically at least 20 cm away.
    if(reading < SHARP_ADC_MIN || reading > SHARP_ADC_MAX)
    {
        return 0;
    }
    if(reading >= SHARP_ADC_20_CM)
    {
        return 20;
    }

    // Interpolate between the two surrounding measured points, using whole cm.
    // Distance = near distance + ADC drop * distance span / ADC span.
    if(reading >= SHARP_ADC_30_CM)
    {
        return 20 + (SHARP_ADC_20_CM - reading) * 10 / (SHARP_ADC_20_CM - SHARP_ADC_30_CM);
    }
    if(reading >= SHARP_ADC_40_CM)
    {
        return 30 + (SHARP_ADC_30_CM - reading) * 10 / (SHARP_ADC_30_CM - SHARP_ADC_40_CM);
    }
    if(reading >= SHARP_ADC_45_CM)
    {
        return 40 + (SHARP_ADC_40_CM - reading) * 5 / (SHARP_ADC_40_CM - SHARP_ADC_45_CM);
    }
    if(reading >= SHARP_ADC_50_CM)
    {
        return 45 + (SHARP_ADC_45_CM - reading) * 5 / (SHARP_ADC_45_CM - SHARP_ADC_50_CM);
    }
    if(reading >= SHARP_ADC_55_CM)
    {
        return 50 + (SHARP_ADC_50_CM - reading) * 5 / (SHARP_ADC_50_CM - SHARP_ADC_55_CM);
    }
    if(reading >= SHARP_ADC_60_CM)
    {
        return 55 + (SHARP_ADC_55_CM - reading) * 5 / (SHARP_ADC_55_CM - SHARP_ADC_60_CM);
    }
    if(reading >= SHARP_ADC_65_CM)
    {
        return 60 + (SHARP_ADC_60_CM - reading) * 5 / (SHARP_ADC_60_CM - SHARP_ADC_65_CM);
    }
    if(reading >= SHARP_ADC_70_CM)
    {
        return 65 + (SHARP_ADC_65_CM - reading) * 5 / (SHARP_ADC_65_CM - SHARP_ADC_70_CM);
    }
    return 70; // Include the low end of the measured 70 cm reading range.
}


uint32_t readClockStartSensor(void)
{
    return readGpioF15();  // D2 / PF15: raw input level, 0 = LOW, 1 = HIGH.
}

uint32_t readClockEndSensor(void)
{
    return readGpioE13();  // D3 / PE13: raw input level, 0 = LOW, 1 = HIGH.
}
