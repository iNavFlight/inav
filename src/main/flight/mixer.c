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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "build/debug.h"

#include "common/axis.h"
#include "common/filter.h"
#include "common/maths.h"
#include "common/utils.h"
#include "common/global_functions.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/pwm_output.h"
#include "drivers/pwm_mapping.h"
#include "drivers/time.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"

#include "flight/failsafe.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/pid.h"
#include "flight/servos.h"

#include "navigation/navigation.h"

#include "rx/rx.h"

#include "sensors/battery.h"

FASTRAM int16_t motor[MAX_SUPPORTED_MOTORS];
FASTRAM int16_t motor_disarmed[MAX_SUPPORTED_MOTORS];
static float motorMixRange;
static float mixerScale = 1.0f;
static EXTENDED_FASTRAM motorMixer_t currentMixer[MAX_SUPPORTED_MOTORS];
static EXTENDED_FASTRAM uint8_t motorCount = 0;
EXTENDED_FASTRAM int mixerThrottleCommand;
static EXTENDED_FASTRAM int throttleIdleValue = 0; 

PG_REGISTER_WITH_RESET_TEMPLATE(flight3DConfig_t, flight3DConfig, PG_MOTOR_3D_CONFIG, 0);

PG_RESET_TEMPLATE(flight3DConfig_t, flight3DConfig,
    .deadband3d_low = 1406,
    .deadband3d_high = 1514,
    .neutral3d = 1460
);

PG_REGISTER_WITH_RESET_TEMPLATE(mixerConfig_t, mixerConfig, PG_MIXER_CONFIG, 2);

PG_RESET_TEMPLATE(mixerConfig_t, mixerConfig,
    .yaw_motor_direction = 1,
    .platformType = PLATFORM_MULTIROTOR,
    .hasFlaps = false,
    .appliedMixerPreset = -1, //This flag is not available in CLI and used by Configurator only
    .fwMinThrottleDownPitchAngle = 0
);

#ifdef BRUSHED_MOTORS
#define DEFAULT_PWM_PROTOCOL    PWM_TYPE_BRUSHED
#define DEFAULT_PWM_RATE        16000
#else
#define DEFAULT_PWM_PROTOCOL    PWM_TYPE_STANDARD
#define DEFAULT_PWM_RATE        400
#endif

#define DEFAULT_MAX_THROTTLE    1850

PG_REGISTER_WITH_RESET_TEMPLATE(motorConfig_t, motorConfig, PG_MOTOR_CONFIG, 5);

PG_RESET_TEMPLATE(motorConfig_t, motorConfig,
    .motorPwmProtocol = DEFAULT_PWM_PROTOCOL,
    .motorPwmRate = DEFAULT_PWM_RATE,
    .maxthrottle = DEFAULT_MAX_THROTTLE,
    .mincommand = 1000, 
    .motorAccelTimeMs = 0,
    .motorDecelTimeMs = 0,
    .throttleIdle = 15.0f,
    .throttleScale = 1.0f,
    .motorPoleCount = 14            // Most brushless motors that we use are 14 poles
);

PG_REGISTER_ARRAY(motorMixer_t, MAX_SUPPORTED_MOTORS, primaryMotorMixer, PG_MOTOR_MIXER, 0);

typedef void (*motorRateLimitingApplyFnPtr)(const float dT);
static EXTENDED_FASTRAM motorRateLimitingApplyFnPtr motorRateLimitingApplyFn;

int getThrottleIdleValue(void)
{
    if (!throttleIdleValue) {
        throttleIdleValue = motorConfig()->mincommand + (((motorConfig()->maxthrottle - motorConfig()->mincommand) / 100.0f) * motorConfig()->throttleIdle);
    }

    return throttleIdleValue;
}

static void computeMotorCount(void)
{
    motorCount = 0;
    for (int i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
        // check if done
        if (primaryMotorMixer(i)->throttle == 0.0f) {
            break;
        }
        motorCount++;
    }
}

uint8_t FAST_CODE NOINLINE getMotorCount(void) {
    return motorCount;
}

float getMotorMixRange(void)
{
    return motorMixRange;
}

bool mixerIsOutputSaturated(void)
{
    return motorMixRange >= 1.0f;
}

void mixerUpdateStateFlags(void)
{
    // set flag that we're on something with wings
    if (mixerConfig()->platformType == PLATFORM_AIRPLANE) {
        ENABLE_STATE(FIXED_WING);
    } else if (mixerConfig()->platformType == PLATFORM_HELICOPTER) {
        DISABLE_STATE(FIXED_WING);
    } else {
        DISABLE_STATE(FIXED_WING);
    }

    if (mixerConfig()->hasFlaps) {
        ENABLE_STATE(FLAPERON_AVAILABLE);
    } else {
        DISABLE_STATE(FLAPERON_AVAILABLE);
    }
}

void nullMotorRateLimiting(const float dT)
{
    UNUSED(dT);
}

void applyMotorRateLimiting(const float dT)
{
    static float motorPrevious[MAX_SUPPORTED_MOTORS] = { 0 };

    if (feature(FEATURE_3D)) {
        // FIXME: Don't apply rate limiting in 3D mode
        for (int i = 0; i < motorCount; i++) {
            motorPrevious[i] = motor[i];
        }
    }
    else {
        // Calculate max motor step
        const uint16_t motorRange = motorConfig()->maxthrottle - throttleIdleValue;
        const float motorMaxInc = (motorConfig()->motorAccelTimeMs == 0) ? 2000 : motorRange * dT / (motorConfig()->motorAccelTimeMs * 1e-3f);
        const float motorMaxDec = (motorConfig()->motorDecelTimeMs == 0) ? 2000 : motorRange * dT / (motorConfig()->motorDecelTimeMs * 1e-3f);

        for (int i = 0; i < motorCount; i++) {
            // Apply motor rate limiting
            motorPrevious[i] = constrainf(motor[i], motorPrevious[i] - motorMaxDec, motorPrevious[i] + motorMaxInc);

            // Handle throttle below min_throttle (motor start/stop)
            if (motorPrevious[i] < throttleIdleValue) {
                if (motor[i] < throttleIdleValue) {
                    motorPrevious[i] = motor[i];
                }
                else {
                    motorPrevious[i] = throttleIdleValue;
                }
            }
        }
    }

    // Update motor values
    for (int i = 0; i < motorCount; i++) {
        motor[i] = motorPrevious[i];
    }
}

void mixerInit(void)
{
    computeMotorCount();
    loadPrimaryMotorMixer();
    // in 3D mode, mixer gain has to be halved
    if (feature(FEATURE_3D)) {
        mixerScale = 0.5f;
    }

    mixerResetDisarmedMotors();

    if (motorConfig()->motorAccelTimeMs || motorConfig()->motorDecelTimeMs) {
        motorRateLimitingApplyFn = applyMotorRateLimiting;
    } else {
        motorRateLimitingApplyFn = nullMotorRateLimiting;
    }
}

void mixerResetDisarmedMotors(void)
{
    // set disarmed motor values
    for (int i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
        motor_disarmed[i] = feature(FEATURE_3D) ? flight3DConfig()->neutral3d : motorConfig()->mincommand;
    }
}

void FAST_CODE NOINLINE writeMotors(void)
{
    for (int i = 0; i < motorCount; i++) {
        uint16_t motorValue;

#ifdef USE_DSHOT
        // If we use DSHOT we need to convert motorValue to DSHOT ranges
        if (isMotorProtocolDigital()) {

            if (feature(FEATURE_3D)) {
                if (motor[i] >= throttleIdleValue && motor[i] <= flight3DConfig()->deadband3d_low) {
                    motorValue = scaleRangef(motor[i], motorConfig()->mincommand, flight3DConfig()->deadband3d_low, DSHOT_3D_DEADBAND_LOW, DSHOT_MIN_THROTTLE);
                    motorValue = constrain(motorValue, DSHOT_MIN_THROTTLE, DSHOT_3D_DEADBAND_LOW);
                }
                else if (motor[i] >= flight3DConfig()->deadband3d_high && motor[i] <= motorConfig()->maxthrottle) {
                    motorValue = scaleRangef(motor[i], flight3DConfig()->deadband3d_high, motorConfig()->maxthrottle, DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE);
                    motorValue = constrain(motorValue, DSHOT_3D_DEADBAND_HIGH, DSHOT_MAX_THROTTLE);
                }
                else {
                    motorValue = DSHOT_DISARM_COMMAND;
                }
            }
            else {
                if (motor[i] < throttleIdleValue) {    // motor disarmed
                    motorValue = DSHOT_DISARM_COMMAND;
                }
                else {
                    motorValue = scaleRangef(motor[i], motorConfig()->mincommand, motorConfig()->maxthrottle, DSHOT_MIN_THROTTLE, DSHOT_MAX_THROTTLE);
                    motorValue = constrain(motorValue, DSHOT_MIN_THROTTLE, DSHOT_MAX_THROTTLE);
                }
            }
        }
        else {
            motorValue = motor[i];
        }
#else
        // We don't define USE_DSHOT
        motorValue = motor[i];
#endif

        pwmWriteMotor(i, motorValue);
    }
}

void writeAllMotors(int16_t mc)
{
    // Sends commands to all motors
    for (int i = 0; i < motorCount; i++) {
        motor[i] = mc;
    }
    writeMotors();
}

void stopMotors(void)
{
    writeAllMotors(feature(FEATURE_3D) ? flight3DConfig()->neutral3d : motorConfig()->mincommand);

    delay(50); // give the timers and ESCs a chance to react.
}

void stopPwmAllMotors(void)
{
    pwmShutdownPulsesForAllMotors(motorCount);
}

void FAST_CODE NOINLINE mixTable(const float dT)
{
    int16_t input[3];   // RPY, range [-500:+500]
    // Allow direct stick input to motors in passthrough mode on airplanes
    if (STATE(FIXED_WING) && FLIGHT_MODE(MANUAL_MODE)) {
        // Direct passthru from RX
        input[ROLL] = rcCommand[ROLL];
        input[PITCH] = rcCommand[PITCH];
        input[YAW] = rcCommand[YAW];
    }
    else {
        input[ROLL] = axisPID[ROLL];
        input[PITCH] = axisPID[PITCH];
        input[YAW] = axisPID[YAW];
    }

    // Initial mixer concept by bdoiron74 reused and optimized for Air Mode
    int16_t rpyMix[MAX_SUPPORTED_MOTORS];
    int16_t rpyMixMax = 0; // assumption: symetrical about zero.
    int16_t rpyMixMin = 0;

    // motors for non-servo mixes
    for (int i = 0; i < motorCount; i++) {
        rpyMix[i] =
            (input[PITCH] * currentMixer[i].pitch +
            input[ROLL] * currentMixer[i].roll +
            -mixerConfig()->yaw_motor_direction * input[YAW] * currentMixer[i].yaw) * mixerScale;

        if (rpyMix[i] > rpyMixMax) rpyMixMax = rpyMix[i];
        if (rpyMix[i] < rpyMixMin) rpyMixMin = rpyMix[i];
    }

    int16_t rpyMixRange = rpyMixMax - rpyMixMin;
    int16_t throttleRange;
    int16_t throttleMin, throttleMax;
    static int16_t throttlePrevious = 0;   // Store the last throttle direction for deadband transitions

    // Find min and max throttle based on condition.
#ifdef USE_GLOBAL_FUNCTIONS
    if (GLOBAL_FUNCTION_FLAG(GLOBAL_FUNCTION_FLAG_OVERRIDE_THROTTLE)) {
        throttleMin = throttleIdleValue;
        throttleMax = motorConfig()->maxthrottle;
        mixerThrottleCommand = constrain(globalFunctionValues[GLOBAL_FUNCTION_ACTION_OVERRIDE_THROTTLE], throttleMin, throttleMax); 
    } else
#endif
    if (feature(FEATURE_3D)) {
        if (!ARMING_FLAG(ARMED)) throttlePrevious = PWM_RANGE_MIDDLE; // When disarmed set to mid_rc. It always results in positive direction after arming.

        if ((rcCommand[THROTTLE] <= (PWM_RANGE_MIDDLE - rcControlsConfig()->deadband3d_throttle))) { // Out of band handling
            throttleMax = flight3DConfig()->deadband3d_low;
            throttleMin = throttleIdleValue;
            throttlePrevious = mixerThrottleCommand = rcCommand[THROTTLE];
        } else if (rcCommand[THROTTLE] >= (PWM_RANGE_MIDDLE + rcControlsConfig()->deadband3d_throttle)) { // Positive handling
            throttleMax = motorConfig()->maxthrottle;
            throttleMin = flight3DConfig()->deadband3d_high;
            throttlePrevious = mixerThrottleCommand = rcCommand[THROTTLE];
        } else if ((throttlePrevious <= (PWM_RANGE_MIDDLE - rcControlsConfig()->deadband3d_throttle)))  { // Deadband handling from negative to positive
            mixerThrottleCommand = throttleMax = flight3DConfig()->deadband3d_low;
            throttleMin = throttleIdleValue;
        } else {  // Deadband handling from positive to negative
            throttleMax = motorConfig()->maxthrottle;
            mixerThrottleCommand = throttleMin = flight3DConfig()->deadband3d_high;
        }
    } else {
        mixerThrottleCommand = rcCommand[THROTTLE];
        throttleMin = throttleIdleValue;
        throttleMax = motorConfig()->maxthrottle;

        // Throttle scaling to limit max throttle when battery is full
    #ifdef USE_GLOBAL_FUNCTIONS
        mixerThrottleCommand = ((mixerThrottleCommand - throttleMin) * getThrottleScale(motorConfig()->throttleScale)) + throttleMin;
    #else
        mixerThrottleCommand = ((mixerThrottleCommand - throttleMin) * motorConfig()->throttleScale) + throttleMin;
    #endif
        // Throttle compensation based on battery voltage
        if (feature(FEATURE_THR_VBAT_COMP) && isAmperageConfigured() && feature(FEATURE_VBAT)) {                
            mixerThrottleCommand = MIN(throttleMin + (mixerThrottleCommand - throttleMin) * calculateThrottleCompensationFactor(), throttleMax);
        }
    }

    throttleRange = throttleMax - throttleMin;

    #define THROTTLE_CLIPPING_FACTOR    0.33f
    motorMixRange = (float)rpyMixRange / (float)throttleRange;
    if (motorMixRange > 1.0f) {
        for (int i = 0; i < motorCount; i++) {
            rpyMix[i] /= motorMixRange;
        }

        // Allow some clipping on edges to soften correction response
        throttleMin = throttleMin + (throttleRange / 2) - (throttleRange * THROTTLE_CLIPPING_FACTOR / 2);
        throttleMax = throttleMin + (throttleRange / 2) + (throttleRange * THROTTLE_CLIPPING_FACTOR / 2);
    } else {
        throttleMin = MIN(throttleMin + (rpyMixRange / 2), throttleMin + (throttleRange / 2) - (throttleRange * THROTTLE_CLIPPING_FACTOR / 2));
        throttleMax = MAX(throttleMax - (rpyMixRange / 2), throttleMin + (throttleRange / 2) + (throttleRange * THROTTLE_CLIPPING_FACTOR / 2));
    }

    // Now add in the desired throttle, but keep in a range that doesn't clip adjusted
    // roll/pitch/yaw. This could move throttle down, but also up for those low throttle flips.
    if (ARMING_FLAG(ARMED)) {
        for (int i = 0; i < motorCount; i++) {
            motor[i] = rpyMix[i] + constrain(mixerThrottleCommand * currentMixer[i].throttle, throttleMin, throttleMax);

            if (failsafeIsActive()) {
                motor[i] = constrain(motor[i], motorConfig()->mincommand, motorConfig()->maxthrottle);
            } else if (feature(FEATURE_3D)) {
                if (throttlePrevious <= (PWM_RANGE_MIDDLE - rcControlsConfig()->deadband3d_throttle)) {
                    motor[i] = constrain(motor[i], throttleIdleValue, flight3DConfig()->deadband3d_low);
                } else {
                    motor[i] = constrain(motor[i], flight3DConfig()->deadband3d_high, motorConfig()->maxthrottle);
                }
            } else {
                motor[i] = constrain(motor[i], throttleIdleValue, motorConfig()->maxthrottle);
            }

            // Motor stop handling
            if (ARMING_FLAG(ARMED) && (getMotorStatus() != MOTOR_RUNNING)) {
                if (feature(FEATURE_MOTOR_STOP)) {
                    motor[i] = (feature(FEATURE_3D) ? PWM_RANGE_MIDDLE : motorConfig()->mincommand);
                } else {
                    motor[i] = throttleIdleValue;
                }
            }
        }
    } else {
        for (int i = 0; i < motorCount; i++) {
            motor[i] = motor_disarmed[i];
        }
    }

    /* Apply motor acceleration/deceleration limit */
    motorRateLimitingApplyFn(dT);
}

motorStatus_e getMotorStatus(void)
{
    if (failsafeRequiresMotorStop() || (!failsafeIsActive() && STATE(NAV_MOTOR_STOP_OR_IDLE))) {
        return MOTOR_STOPPED_AUTO;
    }

    if (rxGetChannelValue(THROTTLE) < rxConfig()->mincheck) {
        if ((STATE(FIXED_WING) || !STATE(AIRMODE_ACTIVE)) && (!(navigationIsFlyingAutonomousMode() && navConfig()->general.flags.auto_overrides_motor_stop)) && (!failsafeIsActive())) {
            return MOTOR_STOPPED_USER;
        }
    }

    return MOTOR_RUNNING;
}

void loadPrimaryMotorMixer(void) {
    for (int i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
        currentMixer[i] = *primaryMotorMixer(i);
    }
}