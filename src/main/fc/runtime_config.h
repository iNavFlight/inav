/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// FIXME some of these are flight modes, some of these are general status indicators
typedef enum {
    ARMED                                           = (1 << 2),
    WAS_EVER_ARMED                                  = (1 << 3),
    SIMULATOR_MODE_HITL                             = (1 << 4),
    SIMULATOR_MODE_SITL                             = (1 << 5),
    ARMING_DISABLED_GEOZONE                         = (1 << 6),
    ARMING_DISABLED_FAILSAFE_SYSTEM                 = (1 << 7),
    ARMING_DISABLED_NOT_LEVEL                       = (1 << 8),
    ARMING_DISABLED_SENSORS_CALIBRATING             = (1 << 9),
    ARMING_DISABLED_SYSTEM_OVERLOADED               = (1 << 10),
    ARMING_DISABLED_NAVIGATION_UNSAFE               = (1 << 11),
    ARMING_DISABLED_COMPASS_NOT_CALIBRATED          = (1 << 12),
    ARMING_DISABLED_ACCELEROMETER_NOT_CALIBRATED    = (1 << 13),
    ARMING_DISABLED_ARM_SWITCH                      = (1 << 14),
    ARMING_DISABLED_HARDWARE_FAILURE                = (1 << 15),
    ARMING_DISABLED_BOXFAILSAFE                     = (1 << 16),

    ARMING_DISABLED_RC_LINK                         = (1 << 18),
    ARMING_DISABLED_THROTTLE                        = (1 << 19),
    ARMING_DISABLED_CLI                             = (1 << 20),
    ARMING_DISABLED_CMS_MENU                        = (1 << 21),
    ARMING_DISABLED_OSD_MENU                        = (1 << 22),
    ARMING_DISABLED_ROLLPITCH_NOT_CENTERED          = (1 << 23),
    ARMING_DISABLED_SERVO_AUTOTRIM                  = (1 << 24),
    ARMING_DISABLED_OOM                             = (1 << 25),
    ARMING_DISABLED_INVALID_SETTING                 = (1 << 26),
    ARMING_DISABLED_PWM_OUTPUT_ERROR                = (1 << 27),
    ARMING_DISABLED_NO_PREARM                       = (1 << 28),
    ARMING_DISABLED_DSHOT_BEEPER                    = (1 << 29),
    ARMING_DISABLED_LANDING_DETECTED                = (1 << 30),

    ARMING_DISABLED_ALL_FLAGS                       = (ARMING_DISABLED_GEOZONE | ARMING_DISABLED_FAILSAFE_SYSTEM | ARMING_DISABLED_NOT_LEVEL | 
                                                       ARMING_DISABLED_SENSORS_CALIBRATING | ARMING_DISABLED_SYSTEM_OVERLOADED | ARMING_DISABLED_NAVIGATION_UNSAFE |
                                                       ARMING_DISABLED_COMPASS_NOT_CALIBRATED | ARMING_DISABLED_ACCELEROMETER_NOT_CALIBRATED |
                                                       ARMING_DISABLED_ARM_SWITCH | ARMING_DISABLED_HARDWARE_FAILURE | ARMING_DISABLED_BOXFAILSAFE |
                                                       ARMING_DISABLED_RC_LINK | ARMING_DISABLED_THROTTLE | ARMING_DISABLED_CLI |
                                                       ARMING_DISABLED_CMS_MENU | ARMING_DISABLED_OSD_MENU | ARMING_DISABLED_ROLLPITCH_NOT_CENTERED |
                                                       ARMING_DISABLED_SERVO_AUTOTRIM | ARMING_DISABLED_OOM | ARMING_DISABLED_INVALID_SETTING |
                                                       ARMING_DISABLED_PWM_OUTPUT_ERROR | ARMING_DISABLED_NO_PREARM | ARMING_DISABLED_DSHOT_BEEPER |
                                                       ARMING_DISABLED_LANDING_DETECTED),
} armingFlag_e;

// Arming blockers that can be overriden by emergency arming.
// Keep in mind that this feature is intended to allow arming in
// situations where we might just need the motors to spin so the
// aircraft can move (even unpredictably) and get unstuck (e.g.
// crashed into a high tree).
#define ARMING_DISABLED_EMERGENCY_OVERRIDE  (ARMING_DISABLED_GEOZONE \
                                            | ARMING_DISABLED_NOT_LEVEL \
                                            | ARMING_DISABLED_NAVIGATION_UNSAFE \
                                            | ARMING_DISABLED_COMPASS_NOT_CALIBRATED \
                                            | ARMING_DISABLED_ACCELEROMETER_NOT_CALIBRATED \
                                            | ARMING_DISABLED_ARM_SWITCH \
                                            | ARMING_DISABLED_HARDWARE_FAILURE)


extern uint32_t armingFlags;

extern const char *armingDisableFlagNames[];

#define isArmingDisabled()          (armingFlags & (ARMING_DISABLED_ALL_FLAGS))
#define DISABLE_ARMING_FLAG(mask)   (armingFlags &= ~(mask))
#define ENABLE_ARMING_FLAG(mask)    (armingFlags |= (mask))
#define ARMING_FLAG(mask)           (armingFlags & (mask))

// Returns the 1st flag from ARMING_DISABLED_ALL_FLAGS which is
// preventing arming, or zero if arming is not disabled.
armingFlag_e isArmingDisabledReason(void);

typedef enum {
    ANGLE_MODE            = (1 << 0),
    HORIZON_MODE          = (1 << 1),
    HEADING_MODE          = (1 << 2),
    NAV_ALTHOLD_MODE      = (1 << 3),
    NAV_RTH_MODE          = (1 << 4),
    NAV_POSHOLD_MODE      = (1 << 5),
    HEADFREE_MODE         = (1 << 6),
    NAV_LAUNCH_MODE       = (1 << 7),
    MANUAL_MODE           = (1 << 8),
    FAILSAFE_MODE         = (1 << 9),
    AUTO_TUNE             = (1 << 10),
    NAV_WP_MODE           = (1 << 11),
    NAV_COURSE_HOLD_MODE  = (1 << 12),
    FLAPERON              = (1 << 13),
    TURN_ASSISTANT        = (1 << 14),
    TURTLE_MODE           = (1 << 15),
    SOARING_MODE          = (1 << 16),
    ANGLEHOLD_MODE        = (1 << 17),
    NAV_FW_AUTOLAND       = (1 << 18),
    NAV_SEND_TO           = (1 << 19),
} flightModeFlags_e;

extern uint32_t flightModeFlags;

#define DISABLE_FLIGHT_MODE(mask) (flightModeFlags &= ~(mask))
#define ENABLE_FLIGHT_MODE(mask) (flightModeFlags |= (mask))
#define FLIGHT_MODE(mask) (flightModeFlags & (mask))

typedef enum {
    GPS_FIX_HOME                        = (1 << 0),
    GPS_FIX                             = (1 << 1),
    CALIBRATE_MAG                       = (1 << 2),
    SMALL_ANGLE                         = (1 << 3),
    FIXED_WING_LEGACY                   = (1 << 4),     // No new code should use this state. Use AIRPLANE, MULTIROTOR, ROVER, BOAT, ALTITUDE_CONTROL and MOVE_FORWARD_ONLY states
    ANTI_WINDUP                         = (1 << 5),
    FLAPERON_AVAILABLE                  = (1 << 6),
    NAV_MOTOR_STOP_OR_IDLE              = (1 << 7),     // navigation requests MOTOR_STOP or motor idle regardless of throttle stick, will only activate if MOTOR_STOP feature is available
    COMPASS_CALIBRATED                  = (1 << 8),
    ACCELEROMETER_CALIBRATED            = (1 << 9),
#ifdef USE_GPS_FIX_ESTIMATION
    GPS_ESTIMATED_FIX                   = (1 << 10),
#endif
    NAV_CRUISE_BRAKING                  = (1 << 11),
    NAV_CRUISE_BRAKING_BOOST            = (1 << 12),
    NAV_CRUISE_BRAKING_LOCKED           = (1 << 13),
    NAV_EXTRA_ARMING_SAFETY_BYPASSED    = (1 << 14),    // nav_extra_arming_safey was bypassed. Keep it until power cycle.
    AIRMODE_ACTIVE                      = (1 << 15),
    ESC_SENSOR_ENABLED                  = (1 << 16),
    AIRPLANE                            = (1 << 17),
    MULTIROTOR                          = (1 << 18),
    ROVER                               = (1 << 19),
    BOAT                                = (1 << 20),
    ALTITUDE_CONTROL                    = (1 << 21),    //It means it can fly
    MOVE_FORWARD_ONLY                   = (1 << 22),
    SET_REVERSIBLE_MOTORS_FORWARD       = (1 << 23),
    FW_HEADING_USE_YAW                  = (1 << 24),
    ANTI_WINDUP_DEACTIVATED             = (1 << 25),
    LANDING_DETECTED                    = (1 << 26),
    IN_FLIGHT_EMERG_REARM               = (1 << 27),
    TAILSITTER                          = (1 << 28), //offset the pitch angle by 90 degrees
} stateFlags_t;

#define DISABLE_STATE(mask) (stateFlags &= ~(mask))
#define ENABLE_STATE(mask) (stateFlags |= (mask))
#define STATE(mask) (stateFlags & (mask))

extern uint32_t stateFlags;

typedef enum {
    FLM_MANUAL,
    FLM_ACRO,
    FLM_ACRO_AIR,
    FLM_ANGLE,
    FLM_HORIZON,
    FLM_ALTITUDE_HOLD,
    FLM_POSITION_HOLD,
    FLM_RTH,
    FLM_MISSION,
    FLM_COURSE_HOLD,
    FLM_CRUISE,
    FLM_LAUNCH,
    FLM_FAILSAFE,
    FLM_ANGLEHOLD,
    FLM_COUNT
} flightModeForTelemetry_e;

flightModeForTelemetry_e getFlightModeForTelemetry(void);

#ifdef USE_SIMULATOR

#define SIMULATOR_MSP_VERSION  2     // Simulator MSP version
#define SIMULATOR_BARO_TEMP    25    // °C
#define SIMULATOR_FULL_BATTERY 126   // Volts*10
#define SIMULATOR_HAS_OPTION(flag) ((simulatorData.flags & flag) != 0)

typedef enum {
    HITL_RESET_FLAGS            = (0 << 0),
    HITL_ENABLE					= (1 << 0),
    HITL_SIMULATE_BATTERY		= (1 << 1),
    HITL_MUTE_BEEPER			= (1 << 2),
    HITL_USE_IMU			    = (1 << 3), // Use the Acc and Gyro data provided by XPlane to calculate Attitude (i.e. 100% of the calculations made by AHRS from INAV)
    HITL_HAS_NEW_GPS_DATA		= (1 << 4),
    HITL_EXT_BATTERY_VOLTAGE	= (1 << 5), // Extend MSP_SIMULATOR format 2
    HITL_AIRSPEED               = (1 << 6),
    HITL_EXTENDED_FLAGS         = (1 << 7), // Extend MSP_SIMULATOR format 2
    HITL_GPS_TIMEOUT            = (1 << 8),
    HITL_PITOT_FAILURE          = (1 << 9)
} simulatorFlags_t;

typedef struct {
    simulatorFlags_t flags;
    uint8_t debugIndex;
    uint8_t vbat;      // 126 -> 12.6V
    uint16_t airSpeed; // cm/s
    int16_t input[4];
} simulatorData_t;

extern simulatorData_t simulatorData;

#endif

void updateFlightModeChangeBeeper(void);

bool sensors(uint32_t mask);
void sensorsSet(uint32_t mask);
void sensorsClear(uint32_t mask);
uint32_t sensorsMask(void);
