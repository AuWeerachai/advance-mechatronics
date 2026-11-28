#ifndef FAN_CONFIG_H
#define FAN_CONFIG_H

// Ordered fan: Sanyo Denki 9GA0812P4G0011, 12 V, four wires.
// San Ace catalog: model family p. 101; pulse sensor description p. 618.
#define FAN_PWM_FREQUENCY_HZ 25000U
#define FAN_TIMER_CLOCK_HZ 180000000U
#define FAN_PWM_PERIOD_COUNTS (FAN_TIMER_CLOCK_HZ / FAN_PWM_FREQUENCY_HZ)
#define FAN_DUTY_SCALE 1000U
#define FAN_HAND_MIN_DISTANCE_CM 20U
#define FAN_HAND_MAX_DISTANCE_CM 70U

// PD12 is the report's tach pin; EXTI line 12 counts its rising edges.
// Leave it disabled until the hardware team confirms this connection.
#ifndef FAN_TACH_CONNECTED
#define FAN_TACH_CONNECTED 0U
#endif
#define FAN_TACH_PULSES_PER_REVOLUTION 2U
#define FAN_TACH_INTERVAL_US 500000U

#if FAN_TACH_INTERVAL_US < 1000U || FAN_TACH_PULSES_PER_REVOLUTION == 0U
#error "Tach interval must be at least 1 ms and pulses per revolution must be positive."
#endif

#if FAN_PWM_PERIOD_COUNTS > 65535U || FAN_PWM_PERIOD_COUNTS == 0U
#error "Fan PWM period must fit Timer 1."
#endif
#if FAN_HAND_MAX_DISTANCE_CM <= FAN_HAND_MIN_DISTANCE_CM
#error "Fan hand-distance limits must be increasing."
#endif

#endif
