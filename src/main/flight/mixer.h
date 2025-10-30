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

#include "config/parameter_group.h"

#include "drivers/timer.h"

#if defined(TARGET_MOTOR_COUNT)
#define MAX_SUPPORTED_MOTORS TARGET_MOTOR_COUNT
#else
#define MAX_SUPPORTED_MOTORS 12
#endif

// Digital protocol has fixed values
#define DSHOT_DISARM_COMMAND      0
#define DSHOT_MIN_THROTTLE       48
#define DSHOT_MAX_THROTTLE     2047
#define DSHOT_3D_DEADBAND_LOW  1047
#define DSHOT_3D_DEADBAND_HIGH 1048

typedef enum {
    PLATFORM_MULTIROTOR     = 0,
    PLATFORM_AIRPLANE       = 1,
    PLATFORM_HELICOPTER     = 2,
    PLATFORM_TRICOPTER      = 3,
    PLATFORM_ROVER          = 4,
    PLATFORM_BOAT           = 5,
    PLATFORM_BOOMERANG      = 6
} flyingPlatformType_e;


typedef enum {
    OUTPUT_MODE_AUTO     = 0,
    OUTPUT_MODE_MOTORS,
    OUTPUT_MODE_SERVOS,
    OUTPUT_MODE_LED
} outputMode_e;

typedef struct motorAxisCorrectionLimits_s {
    int16_t min;
    int16_t max;
} motorAxisCorrectionLimits_t;

// Custom mixer data per motor
typedef struct motorMixer_s {
    float throttle;
    float roll;
    float pitch;
    float yaw;
} motorMixer_t;


typedef struct timerOverride_s {
    uint8_t outputMode;
} timerOverride_t;

PG_DECLARE_ARRAY(timerOverride_t, HARDWARE_TIMER_DEFINITION_COUNT, timerOverrides);

typedef struct reversibleMotorsConfig_s {
    uint16_t deadband_low;                // min 3d value
    uint16_t deadband_high;               // max 3d value
    uint16_t neutral;                     // center 3d value
} reversibleMotorsConfig_t;

PG_DECLARE(reversibleMotorsConfig_t, reversibleMotorsConfig);

typedef struct motorConfig_s {
    // PWM values, in milliseconds, common range is 1000-2000 (1ms to 2ms)
    uint16_t mincommand;                    // This is the value for the ESCs when they are not armed. In some cases, this value must be lowered down to 900 for some specific ESCs
    uint16_t motorPwmRate;                  // The update rate of motor outputs (50-498Hz)
    uint8_t  motorPwmProtocol;
    uint16_t digitalIdleOffsetValue;
    uint8_t motorPoleCount;                 // Magnetic poles in the motors for calculating actual RPM from eRPM provided by ESC telemetry
} motorConfig_t;

PG_DECLARE(motorConfig_t, motorConfig);

typedef enum {
    MOTOR_STOPPED_USER,
    MOTOR_STOPPED_AUTO,
    MOTOR_RUNNING
} motorStatus_e;

typedef enum {
    MOTOR_DIRECTION_FORWARD,
    MOTOR_DIRECTION_BACKWARD,
    MOTOR_DIRECTION_DEADBAND
} reversibleMotorsThrottleState_e;

extern int16_t motor[MAX_SUPPORTED_MOTORS];
extern int16_t motor_disarmed[MAX_SUPPORTED_MOTORS];
extern int mixerThrottleCommand;

bool ifMotorstopFeatureEnabled(void);
int getThrottleIdleValue(void);
int16_t getThrottlePercent(bool);
uint16_t setDesiredThrottle(uint16_t throttle, bool allowMotorStop);
uint8_t getMotorCount(void);
float getMotorMixRange(void);
bool mixerIsOutputSaturated(void);
motorStatus_e getMotorStatus(void);

void writeAllMotors(int16_t mc);
void mixerInit(void);
void mixerUpdateStateFlags(void);
void mixerResetDisarmedMotors(void);
void mixTable(void);
void writeMotors(void);
void processServoAutotrim(const float dT);
void processServoAutotrimMode(void);
void processContinuousServoAutotrim(const float dT);
void stopMotors(void);
void stopMotorsNoDelay(void);
void stopPwmAllMotors(void);

void loadPrimaryMotorMixer(void);
bool areMotorsRunning(void);

uint16_t getMaxThrottle(void);
