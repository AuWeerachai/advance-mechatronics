#ifndef GAME_CONFIG_H
#define GAME_CONFIG_H

#include <stdint.h>

// Game rules. Each round always uses the full reposition and holding periods.
#ifndef TARGET_ROUND_COUNT
#define TARGET_ROUND_COUNT 5U
#endif
#ifndef TARGET_REPOSITION_US
#define TARGET_REPOSITION_US 4000000U
#endif
#ifndef TARGET_HOLD_US
#define TARGET_HOLD_US 4000000U
#endif
#ifndef TARGET_REQUIRED_PERCENT
#define TARGET_REQUIRED_PERCENT 80U
#endif
#define TARGET_TOLERANCE_CM 3U
#define TARGET_LED_COUNT 3U

// Measured DOWN from the top SHARP sensor to the ball at each LED.
// 0 means not measured yet. The game stays in WELCOME until all three are set.
#ifndef TARGET_LED_1_DISTANCE_CM
#define TARGET_LED_1_DISTANCE_CM 0U
#endif
#ifndef TARGET_LED_2_DISTANCE_CM
#define TARGET_LED_2_DISTANCE_CM 0U
#endif
#ifndef TARGET_LED_3_DISTANCE_CM
#define TARGET_LED_3_DISTANCE_CM 0U
#endif

#define SENSOR_CHECK_INTERVAL_US 10000U
#define BALL_SAMPLE_MAX_GAP_US (2U * SENSOR_CHECK_INTERVAL_US)
// Accept a button press/release only after it stays unchanged for this long.
#ifndef START_BUTTON_DEBOUNCE_US
#define START_BUTTON_DEBOUNCE_US 30000U
#endif
#define STATUS_PRINT_INTERVAL_US 500000U

#define AIR_AT_START 100U
#define AIR_MAXIMUM 100U
#define AIR_PER_LED 10U
#define AIR_LED_COUNT 10U
#ifndef PUMPS_PER_AIR_LED
#define PUMPS_PER_AIR_LED 2U
#endif
#define AIR_DRAIN_PER_SECOND 3U

// EE-SX1042: collector pulled up, emitter grounded; blocked light gives HIGH.
#define CLOCK_SENSOR_BLOCKED 1U

#if TARGET_ROUND_COUNT == 0 || TARGET_REPOSITION_US == 0 || TARGET_HOLD_US == 0
#error "Round count and target periods must be greater than zero."
#endif
#if TARGET_REQUIRED_PERCENT > 100 || PUMPS_PER_AIR_LED == 0
#error "Check the holding percentage and pumps per air LED."
#endif
#if TARGET_REPOSITION_US > 60000000U || TARGET_HOLD_US > 60000000U
#error "Keep each target period within 60 seconds."
#endif

// Round upward so a fraction of a microsecond cannot lower the pass requirement.
#define TARGET_REQUIRED_TIME_US ((uint32_t)(((uint64_t)TARGET_HOLD_US * TARGET_REQUIRED_PERCENT + 99U) / 100U))

#endif
