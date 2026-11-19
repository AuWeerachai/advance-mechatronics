#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdint.h>

// Lever on A2 / PC3: raw ADC thresholds from testing its mechanical travel.
#define PUMP_LOW_ADC ((uint32_t)15)   // Below 15 is the returned position.
#define PUMP_HIGH_ADC ((uint32_t)170) // At least 170 marks the top; return low to finish the cycle.

// Shared Sharp calibration: measured with the hand sensor on A1 / PC0.
#define SHARP_ADC_20_CM ((uint32_t)3175) 
#define SHARP_ADC_30_CM ((uint32_t)2435) 
#define SHARP_ADC_40_CM ((uint32_t)1825) 
#define SHARP_ADC_45_CM ((uint32_t)1780) 
#define SHARP_ADC_50_CM ((uint32_t)1520) 
#define SHARP_ADC_55_CM ((uint32_t)1360) 
#define SHARP_ADC_60_CM ((uint32_t)1250) 
#define SHARP_ADC_65_CM ((uint32_t)1180) 
#define SHARP_ADC_70_CM ((uint32_t)1140) 
#define SHARP_ADC_MIN ((uint32_t)1130)  // 70cm
#define SHARP_ADC_MAX ((uint32_t)3200)  // 20cm


#endif
