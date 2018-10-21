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


#define THRF_ZERO       (0.000f)
#define THRF_MIN        (0.001f)
#define THRF_MAX        (1.000f)

static uint8_t motorCount;

float motorValue[MAX_SUPPORTED_MOTORS];
float motorValueDisarmed[MAX_SUPPORTED_MOTORS];
static float motorMixRange;

PG_REGISTER_WITH_RESET_TEMPLATE(flight3DConfig_t, flight3DConfig, PG_MOTOR_3D_CONFIG, 0);

PG_RESET_TEMPLATE(flight3DConfig_t, flight3DConfig,
    .deadband3d_low = 1406,
    .deadband3d_high = 1514,
    .neutral3d = 1460
);

PG_REGISTER_WITH_RESET_TEMPLATE(mixerConfig_t, mixerConfig, PG_MIXER_CONFIG, 1);

PG_RESET_TEMPLATE(mixerConfig_t, mixerConfig,
    .yaw_motor_direction = 1,
    .yaw_jump_prevention_limit = 200,
    .platformType = PLATFORM_MULTIROTOR,
    .hasFlaps = false,
    .appliedMixerPreset = -1, //This flag is not available in CLI and used by Configurator only
    .fwMinThrottleDownPitchAngle = 0
);

#ifdef BRUSHED_MOTORS
#define DEFAULT_PWM_PROTOCOL    PWM_TYPE_BRUSHED
#define DEFAULT_PWM_RATE        16000
#define DEFAULT_MIN_THROTTLE    1000
#else
#define DEFAULT_PWM_PROTOCOL    PWM_TYPE_STANDARD
#define DEFAULT_PWM_RATE        400
#define DEFAULT_MIN_THROTTLE    1150
#endif

PG_REGISTER_WITH_RESET_TEMPLATE(motorConfig_t, motorConfig, PG_MOTOR_CONFIG, 2);

PG_RESET_TEMPLATE(motorConfig_t, motorConfig,
    .minthrottle = DEFAULT_MIN_THROTTLE,
    .motorPwmProtocol = DEFAULT_PWM_PROTOCOL,
    .motorPwmRate = DEFAULT_PWM_RATE,
    .maxthrottle = 1850,
    .mincommand = 1000,
    .motorAccelTimeMs = 0,
    .motorDecelTimeMs = 0,
    .digitalIdleOffsetValue = 450   // Same scale as in Betaflight
);

static motorMixer_t currentMixer[MAX_SUPPORTED_MOTORS];

PG_REGISTER_ARRAY(motorMixer_t, MAX_SUPPORTED_MOTORS, customMotorMixer, PG_MOTOR_MIXER, 0);

uint8_t getMotorCount(void)
{
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
        DISABLE_STATE(HELICOPTER);
    } else if (mixerConfig()->platformType == PLATFORM_HELICOPTER) {
        DISABLE_STATE(FIXED_WING);
        ENABLE_STATE(HELICOPTER);
    } else {
        DISABLE_STATE(FIXED_WING);
        DISABLE_STATE(HELICOPTER);
    }

    if (mixerConfig()->hasFlaps) {
        ENABLE_STATE(FLAPERON_AVAILABLE);
    } else {
        DISABLE_STATE(FLAPERON_AVAILABLE);
    }
}

void mixerUsePWMIOConfiguration(void)
{
    motorCount = 0;

    // load custom mixer into currentMixer
    for (int i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
        // check if done
        if (customMotorMixer(i)->throttle == 0.0f)
            break;
        currentMixer[i] = *customMotorMixer(i);
        motorCount++;
    }

    // in 3D mode, mixer gain has to be halved
    if (feature(FEATURE_3D)) {
        if (motorCount > 1) {
            for (int i = 0; i < motorCount; i++) {
                currentMixer[i].pitch *= 0.5f;
                currentMixer[i].roll *= 0.5f;
                currentMixer[i].yaw *= 0.5f;
            }
        }
    }

    mixerResetDisarmedMotors();
}

void mixerResetDisarmedMotors(void)
{
    // set disarmed motor values
    for (int i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
        motorValueDisarmed[i] = THRF_ZERO;
    }
}

static inline float mapThrottleValue(float throttle, float min_in, float max_in, float min_out, float max_out)
{
    return constrainf(scaleRangef(throttle, min_in, max_in, min_out, max_out), min_out, max_out);
}

static inline uint16_t mapMixerToESC(float thr, uint16_t thr_disarm, uint16_t thr3d_neg, uint16_t thr3d_min_low, uint16_t thr3d_min_high, uint16_t thr3d_pos, uint16_t thr_low, uint16_t thr_high)
{
    if (feature(FEATURE_3D)) {
        // Negative throttle
        if (thr <= -THRF_MIN) {
            return mapThrottleValue(thr, -THRF_MAX, -THRF_MIN, thr3d_neg, thr3d_min_low);
        }
        // Positive throttle
        else if (thr >= THRF_MIN) {
            return mapThrottleValue(thr, THRF_MIN, THRF_MAX, thr3d_min_high, thr3d_pos);
        }
        // Zero throttle
        else {
            return thr_disarm;
        }
    }
    else {
        if (thr < THRF_MIN) {
            return thr_disarm;
        }
        else {
            return mapThrottleValue(thr, THRF_MIN, THRF_MAX, thr_low, thr_high);
        }
    }
}

void writeMotors(void)
{
    for (int i = 0; i < motorCount; i++) {
        uint16_t motorRawValue;

#ifdef USE_DSHOT
        // If we use DSHOT we need to convert motorValue to DSHOT ranges
        if (isMotorProtocolDshot()) {
            const float dshotMinThrottleOffset = (DSHOT_MAX_THROTTLE - DSHOT_MIN_THROTTLE) / 10000.0f * motorConfig()->digitalIdleOffsetValue;
            
            motorRawValue = mapMixerToESC(motorValue[i],                                    // thr
                                          DSHOT_DISARM_COMMAND,                             // thr_disarm
                                          DSHOT_3D_DEADBAND_LOW,                            // thr3d_neg
                                          dshotMinThrottleOffset + DSHOT_MIN_THROTTLE,      // thr3d_min_low
                                          dshotMinThrottleOffset + DSHOT_3D_DEADBAND_HIGH,  // thr3d_min_high
                                          DSHOT_MAX_THROTTLE,                               // thr3d_pos
                                          dshotMinThrottleOffset + DSHOT_MIN_THROTTLE,      // thr_low
                                          DSHOT_MAX_THROTTLE);                              // thr_high
        }
        else
#endif
        // Don't define DSHOT - use motorConfig() values
        {
            motorRawValue = mapMixerToESC(motorValue[i],                        // thr
                                          motorConfig()->mincommand,            // thr_disarm
                                          motorConfig()->minthrottle,           // thr3d_neg
                                          flight3DConfig()->deadband3d_low,     // thr3d_min_low
                                          flight3DConfig()->deadband3d_high,    // thr3d_min_high
                                          motorConfig()->maxthrottle,           // thr3d_pos
                                          motorConfig()->minthrottle,           // thr_low
                                          motorConfig()->maxthrottle);          // thr_high
        }

        pwmWriteMotor(i, motorRawValue);
    }
}

void stopMotors(void)
{
    // Sends commands to all motors
    for (int i = 0; i < motorCount; i++) {
        motorValue[i] = THRF_ZERO;
    }

    writeMotors();

    delay(50); // give the timers and ESCs a chance to react.
}

void stopPwmAllMotors(void)
{
    pwmShutdownPulsesForAllMotors(motorCount);
}

static void applyMotorRateLimiting(const float dT)
{
    static float motorPrevious[MAX_SUPPORTED_MOTORS] = { 0 };

    if (feature(FEATURE_3D)) {
        // FIXME: Don't apply rate limiting in 3D mode
        for (int i = 0; i < motorCount; i++) {
            motorPrevious[i] = motorValue[i];
        }
    }
    else {
        // Calculate max motor step
        const float motorMaxInc = (motorConfig()->motorAccelTimeMs == 0) ? 2.0f : 1.000f * dT / (motorConfig()->motorAccelTimeMs * 1e-3f);
        const float motorMaxDec = (motorConfig()->motorDecelTimeMs == 0) ? 2.0f : 1.000f * dT / (motorConfig()->motorDecelTimeMs * 1e-3f);

        for (int i = 0; i < motorCount; i++) {
            // Apply motor rate limiting
            motorPrevious[i] = constrainf(motorValue[i], motorPrevious[i] - motorMaxDec, motorPrevious[i] + motorMaxInc);

            // Handle throttle below min_throttle (motor start/stop)
            if (motorPrevious[i] < THRF_MIN) {
                if (motorValue[i] < THRF_MIN) {
                    motorPrevious[i] = motorValue[i];
                }
                else {
                    motorPrevious[i] = THRF_MIN;
                }
            }
        }
    }

    // Update motor values
    for (int i = 0; i < motorCount; i++) {
        motorValue[i] = motorPrevious[i];
    }
}

void mixTable(const float dT)
{
    float input[4];   // RPY, range [-500:+500]

    // Allow direct stick input to motors in passthrough mode on airplanes
    if (STATE(FIXED_WING) && FLIGHT_MODE(MANUAL_MODE)) {
        // Direct passthru from RX
        input[ROLL]     = rcCommand[ROLL] / 500.0f;
        input[PITCH]    = rcCommand[PITCH] / 500.0f;
        input[YAW]      = rcCommand[YAW] / 500.0f;
        input[THROTTLE] = mapThrottleValue(rcCommand[THROTTLE], motorConfig()->minthrottle, motorConfig()->maxthrottle, THRF_MIN, THRF_MAX);
    }
    else {
        input[ROLL]     = axisPID[ROLL] / 500.0f;
        input[PITCH]    = axisPID[PITCH] / 500.0f;
        input[YAW]      = axisPID[YAW] / 500.0f;

        if (feature(FEATURE_3D)) {
            if ((rcCommand[THROTTLE] <= (PWM_RANGE_MIDDLE - rcControlsConfig()->deadband3d_throttle))) {
                input[THROTTLE] = mapThrottleValue(rcCommand[THROTTLE], motorConfig()->minthrottle, PWM_RANGE_MIDDLE - rcControlsConfig()->deadband3d_throttle, -THRF_MAX, -THRF_MIN);
            }
            else if (rcCommand[THROTTLE] >= (PWM_RANGE_MIDDLE + rcControlsConfig()->deadband3d_throttle)) { // Positive handling
                input[THROTTLE] = mapThrottleValue(rcCommand[THROTTLE], PWM_RANGE_MIDDLE + rcControlsConfig()->deadband3d_throttle, motorConfig()->maxthrottle, THRF_MIN, THRF_MAX);
            }
            else {
                input[THROTTLE] = THRF_ZERO;
            }
        }
        else {
            input[THROTTLE] = mapThrottleValue(rcCommand[THROTTLE], motorConfig()->minthrottle, motorConfig()->maxthrottle, THRF_MIN, THRF_MAX);
        }

        if (motorCount >= 4 && mixerConfig()->yaw_jump_prevention_limit < YAW_JUMP_PREVENTION_LIMIT_HIGH) {
            // prevent "yaw jump" during yaw correction
            const float yawLimit = (mixerConfig()->yaw_jump_prevention_limit + ABS(rcCommand[YAW])) / 500.0f;
            input[YAW] = constrainf(input[YAW], -yawLimit, +yawLimit);
        }
    }

    // Initial mixer concept by bdoiron74 reused and optimized for Air Mode
    float rpyMix[MAX_SUPPORTED_MOTORS];
    float rpyMixMax = 0; // assumption: symetrical about zero.
    float rpyMixMin = 0;

    // motors for non-servo mixes
    for (int i = 0; i < motorCount; i++) {
        rpyMix[i] =
            input[PITCH] * currentMixer[i].pitch +
            input[ROLL] * currentMixer[i].roll +
            -mixerConfig()->yaw_motor_direction * input[YAW] * currentMixer[i].yaw;

        if (rpyMix[i] > rpyMixMax) rpyMixMax = rpyMix[i];
        if (rpyMix[i] < rpyMixMin) rpyMixMin = rpyMix[i];
    }

    float rpyMixRange = rpyMixMax - rpyMixMin;
    float throttleRange, throttleCommand;
    float throttleMin, throttleMax;
    static float throttlePrevious = 0;   // Store the last throttle direction for deadband transitions

    // Find min and max throttle based on condition.
    if (feature(FEATURE_3D)) {
        if (!ARMING_FLAG(ARMED)) {
            throttlePrevious = THRF_ZERO;
        }

        if (input[THROTTLE] <= -THRF_MIN) {
            throttleMin = -THRF_MAX;
            throttleMax = -THRF_MIN;
            throttleCommand = input[THROTTLE];
            throttlePrevious = input[THROTTLE];
        } else if (input[THROTTLE] >= -THRF_MIN) {
            throttleMin = THRF_MIN;
            throttleMax = THRF_MAX;
            throttleCommand = input[THROTTLE];
            throttlePrevious = input[THROTTLE];
        } else if (throttlePrevious < 0) {
            throttleMin = -THRF_MAX;
            throttleMax = -THRF_MIN;
            throttleCommand = -THRF_MIN;
        } else {
            throttleMin = THRF_MIN;
            throttleMax = THRF_MAX;
            throttleCommand = THRF_MIN;
        }
    } else {
        throttleCommand = input[THROTTLE];
        throttleMin = THRF_MIN;
        throttleMax = THRF_MAX;

        // Throttle compensation based on battery voltage
        if (feature(FEATURE_THR_VBAT_COMP) && feature(FEATURE_VBAT) && isAmperageConfigured()) {
            throttleCommand = MIN(throttleMin + (throttleCommand - throttleMin) * calculateThrottleCompensationFactor(), throttleMax);
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
            motorValue[i] = rpyMix[i] + constrainf(throttleCommand * currentMixer[i].throttle, throttleMin, throttleMax);

            if (feature(FEATURE_3D)) {
                if (throttlePrevious < 0) {
                    motorValue[i] = constrainf(motorValue[i], -THRF_MAX, -THRF_MIN);
                } else {
                    motorValue[i] = constrainf(motorValue[i], THRF_MIN, THRF_MAX);
                }
            }
            else {
                motorValue[i] = constrainf(motorValue[i], THRF_MIN, THRF_MAX);
            }

            // Motor stop handling
            if (ARMING_FLAG(ARMED) && (getMotorStatus() != MOTOR_RUNNING)) {
                if (feature(FEATURE_MOTOR_STOP)) {
                    motorValue[i] = THRF_ZERO;
                } else {
                    motorValue[i] = THRF_MIN;
                }
            }
        }
    } else {
        for (int i = 0; i < motorCount; i++) {
            motorValue[i] = motorValueDisarmed[i];
        }
    }

    /* Apply motor acceleration/deceleration limit */
    applyMotorRateLimiting(dT);
}

motorStatus_e getMotorStatus(void)
{
    if (failsafeRequiresMotorStop() || (!failsafeIsActive() && STATE(NAV_MOTOR_STOP_OR_IDLE))) {
        return MOTOR_STOPPED_AUTO;
    }

    if (rcData[THROTTLE] < rxConfig()->mincheck) {
        if ((STATE(FIXED_WING) || !isAirmodeActive()) && (!(navigationIsFlyingAutonomousMode() && navConfig()->general.flags.auto_overrides_motor_stop)) && (!failsafeIsActive())) {
            return MOTOR_STOPPED_USER;
        }
    }

    return MOTOR_RUNNING;
}
