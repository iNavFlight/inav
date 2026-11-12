#pragma once
// Generated from the MSP YAML schema by msp/generator/gen_c.py (constants.yaml). Do not edit by hand.

#ifndef ADSB_CALL_SIGN_MAX_LENGTH
#  define ADSB_CALL_SIGN_MAX_LENGTH 9
#endif
#ifndef BUILD_DATE_LENGTH
#  define BUILD_DATE_LENGTH 11
#endif
#ifndef BUILD_TIME_LENGTH
#  define BUILD_TIME_LENGTH 8
#endif
#ifndef CUSTOM_ELEMENTS_PARTS
#  define CUSTOM_ELEMENTS_PARTS 3
#endif
#ifndef DEBUG32_VALUE_COUNT
#  define DEBUG32_VALUE_COUNT 8
#endif
#ifndef GIT_SHORT_REVISION_LENGTH
#  define GIT_SHORT_REVISION_LENGTH 8
#endif
#ifndef LED_CONFIGURABLE_COLOR_COUNT
#  define LED_CONFIGURABLE_COLOR_COUNT 16
#endif
#ifndef LED_MAX_STRIP_LENGTH
#  define LED_MAX_STRIP_LENGTH 128
#endif
#ifndef MAX_ADJUSTMENT_RANGE_COUNT
#  define MAX_ADJUSTMENT_RANGE_COUNT 20
#endif
#ifndef MAX_GLOBAL_VARIABLES
#  define MAX_GLOBAL_VARIABLES 8
#endif
#ifndef MAX_LOGIC_CONDITIONS
#  define MAX_LOGIC_CONDITIONS 64
#endif
#ifndef MAX_MAPPABLE_RX_INPUTS
#  define MAX_MAPPABLE_RX_INPUTS 4
#endif
#ifndef MAX_MODE_ACTIVATION_CONDITION_COUNT
#  define MAX_MODE_ACTIVATION_CONDITION_COUNT 40
#endif
#ifndef MAX_PROGRAMMING_PID_COUNT
#  define MAX_PROGRAMMING_PID_COUNT 4
#endif
#ifndef MAX_SERVO_RULES
#  define MAX_SERVO_RULES 36
#endif
#ifndef MAX_SUPPORTED_SERVOS
#  define MAX_SUPPORTED_SERVOS 18
#endif
#ifndef MAX_TEMP_SENSORS
#  define MAX_TEMP_SENSORS 8
#endif
#ifndef OSD_CHAR_BYTES
#  define OSD_CHAR_BYTES 64
#endif
#ifndef OSD_CHAR_VISIBLE_BYTES
#  define OSD_CHAR_VISIBLE_BYTES 54
#endif
#ifndef OSD_ITEM_COUNT
#  define OSD_ITEM_COUNT 172
#endif
#ifndef PID_ITEM_COUNT
#  define PID_ITEM_COUNT 12
#endif
#ifndef RADAR_MAX_POIS
#  define RADAR_MAX_POIS 5
#endif
#ifndef TEMPERATURE_LABEL_LEN
#  define TEMPERATURE_LABEL_LEN 4
#endif

// Build-configuration dependent: supplied by the target, not the schema.
// Generation fails rather than guessing, so each must be defined by the build.
#if !defined(HARDWARE_TIMER_DEFINITION_COUNT)
#  error "HARDWARE_TIMER_DEFINITION_COUNT is build-configuration dependent (from HARDWARE_TIMER_DEFINITION_COUNT) and must be defined by the target"
#endif
#if !defined(MAX_SUPPORTED_MOTORS)
#  error "MAX_SUPPORTED_MOTORS is build-configuration dependent (from MAX_SUPPORTED_MOTORS) and must be defined by the target"
#endif
