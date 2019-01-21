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
#include <stdlib.h>
#include <string.h>

#include "platform.h"

#include "build/debug.h"
#include "build/build_config.h"

#include "common/axis.h"
#include "common/filter.h"
#include "common/maths.h"

#include "config/config_reset.h"
#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/pwm_output.h"
#include "drivers/time.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/controlrate_profile.h"

#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/pid.h"
#include "flight/servos.h"

#include "rx/rx.h"

#include "sensors/gyro.h"

#define SERVO_VALUE_MIN -1.0f
#define SERVO_VALUE_CENTER 0.0f
#define SERVO_VALUE_MAX 1.0f
#define SERVO_VALUE_RANGE (SERVO_VALUE_MAX - SERVO_VALUE_MIN)

#define PWM_SERVO_VALUE_FACTOR ((float)((PWM_RANGE_MAX - PWM_RANGE_MIN) / SERVO_VALUE_RANGE))
#define PWM_VALUE_TO_SERVO(x) ((x - PWM_RANGE_MIDDLE) * (1 / PWM_SERVO_VALUE_FACTOR))
#define SERVO_VALUE_TO_PWM(x) (((int)(x * PWM_SERVO_VALUE_FACTOR)) + PWM_RANGE_MIDDLE)
#define RC_COMMAND_RPY_TO_SERVO(x) (x * (1 / PWM_SERVO_VALUE_FACTOR))
#define RC_DATA_TO_SERVO(x) PWM_VALUE_TO_SERVO(x)
#define AXIS_PID_TO_SERVO(x) (x * (1 / PWM_SERVO_VALUE_FACTOR))

PG_REGISTER_WITH_RESET_TEMPLATE(servoConfig_t, servoConfig, PG_SERVO_CONFIG, 0);

PG_RESET_TEMPLATE(servoConfig_t, servoConfig,
    .servoCenterPulse = DEFAULT_SERVO_MIDDLE,
    .servoPwmRate = 50,             // Default for analog servos
    .servo_lowpass_freq = 20,       // Default servo update rate is 50Hz, everything above Nyquist frequency (25Hz) is going to fold and cause distortions
    .flaperon_throw_offset = FLAPERON_THROW_DEFAULT,
    .tri_unarmed_servo = 1
);

PG_REGISTER_ARRAY(servoMixer_t, MAX_SERVO_RULES, customServoMixers, PG_SERVO_MIXER, 0);

PG_REGISTER_ARRAY_WITH_RESET_FN(servoParam_t, MAX_SUPPORTED_SERVOS, servoParams, PG_SERVO_PARAMS, 2);

void pgResetFn_servoParams(servoParam_t *instance)
{
    for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
        RESET_CONFIG(servoParam_t, &instance[i],
            .min = DEFAULT_SERVO_MIN,
            .max = DEFAULT_SERVO_MAX,
            .middle = DEFAULT_SERVO_MIDDLE,
            .rate = 100
        );
    }
}

STATIC_FASTRAM float servo[MAX_SUPPORTED_SERVOS]; // [SERVO_VALUE_MIN; SERVO_VALUE_MAX]

static uint8_t servoRuleCount = 0;
static servoMixer_t currentServoMixer[MAX_SERVO_RULES];

static uint8_t minServoIndex;
static uint8_t maxServoIndex;

static biquadFilter_t servoFilter[MAX_SUPPORTED_SERVOS];
static bool servoFilterIsSet;

STATIC_FASTRAM servoMetadata_t servoMetadata[MAX_SUPPORTED_SERVOS];
static rateLimitFilter_t servoSpeedLimitFilter[MAX_SERVO_RULES];

void servosInit(void)
{
    servosLoadMixer();

    // give all servos a default command and compute
    // their metadata
    for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
        servo[i] = SERVO_VALUE_CENTER;
        servosComputeMetadata(i);
    }
}

bool servosAreEnabledForCurrentMixer(void)
{
    return servoRuleCount > 0;
}

void servosLoadMixer(void)
{
    // reset settings
    servoRuleCount = 0;
    minServoIndex = 255;
    maxServoIndex = 0;
    memset(currentServoMixer, 0, sizeof(currentServoMixer));

    // load custom mixer into currentServoMixer
    for (int i = 0; i < MAX_SERVO_RULES; i++) {
        // check if done
        if (customServoMixers(i)->rate == 0)
            break;

        if (customServoMixers(i)->targetChannel < minServoIndex) {
            minServoIndex = customServoMixers(i)->targetChannel;
        }

        if (customServoMixers(i)->targetChannel > maxServoIndex) {
            maxServoIndex = customServoMixers(i)->targetChannel;
        }

        memcpy(&currentServoMixer[i], customServoMixers(i), sizeof(servoMixer_t));
        servoRuleCount++;
    }
}

static float servosGetRCValue(rc_alias_e ch)
{
    // center the RC input value around the RC middle value
    // by subtracting the RC middle value from the RC input value, we get:
    // data - middle = input
    // 2000 - 1500 = +500
    // 1500 - 1500 = 0
    // 1000 - 1500 = -500
    // Then divide by 500 to get to [-1, 1]
    return (rcData[ch] - PWM_RANGE_MIDDLE) / 500.0f;
}

static void filterServos(void)
{
    if (servoConfig()->servo_lowpass_freq) {
        // Initialize servo lowpass filter (servos are calculated at looptime rate)
        if (!servoFilterIsSet) {
            for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
                biquadFilterInitLPF(&servoFilter[i], servoConfig()->servo_lowpass_freq, gyro.targetLooptime);
                biquadFilterReset(&servoFilter[i], servo[i]);
            }
            servoFilterIsSet = true;
        }

        for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
            // Apply servo lowpass filter and do sanity cheching
            servo[i] = biquadFilterApply(&servoFilter[i], servo[i]);
        }
    }

    for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
        servo[i] = constrainf(servo[i], servoMetadata[i].values.min, servoMetadata[i].values.max);
    }
}

static void servosWrite(void)
{
    filterServos();

    int servoIndex = 0;

    /*
     * in case of tricopters, there might me a need to zero servo output when unarmed
     */
    if (mixerConfig()->platformType == PLATFORM_TRICOPTER && !ARMING_FLAG(ARMED) && !servoConfig()->tri_unarmed_servo) {
        for (int i = minServoIndex; i <= maxServoIndex; i++) {
            pwmWriteServo(servoIndex++, 0);
        }
    } else {
        for (int i = minServoIndex; i <= maxServoIndex; i++) {
            pwmWriteServo(servoIndex++, SERVO_VALUE_TO_PWM(servo[i]));
        }
    }
}

void servosUpdate(float dT)
{
    float input[INPUT_SOURCE_COUNT]; // Range [SERVO_VALUE_MIN:SERVO_VALUE_MAX]

    if (FLIGHT_MODE(MANUAL_MODE)) {
        input[INPUT_STABILIZED_ROLL] = RC_COMMAND_RPY_TO_SERVO(rcCommand[ROLL]);
        input[INPUT_STABILIZED_PITCH] = RC_COMMAND_RPY_TO_SERVO(rcCommand[PITCH]);
        input[INPUT_STABILIZED_YAW] = RC_COMMAND_RPY_TO_SERVO(rcCommand[YAW]);
    } else {
        // Assisted modes (gyro only or gyro+acc according to AUX configuration in Gui
        input[INPUT_STABILIZED_ROLL] = AXIS_PID_TO_SERVO(axisPID[ROLL]);
        input[INPUT_STABILIZED_PITCH] = AXIS_PID_TO_SERVO(axisPID[PITCH]);
        input[INPUT_STABILIZED_YAW] = AXIS_PID_TO_SERVO(axisPID[YAW]);

        // Reverse yaw servo when inverted in 3D mode only for multirotor and tricopter
        if (feature(FEATURE_3D) && (rcData[THROTTLE] < PWM_RANGE_MIDDLE) &&
        (mixerConfig()->platformType == PLATFORM_MULTIROTOR || mixerConfig()->platformType == PLATFORM_TRICOPTER)) {
            input[INPUT_STABILIZED_YAW] *= -1;
        }
    }

    input[INPUT_FEATURE_FLAPS] = FLIGHT_MODE(FLAPERON) ? (servoConfig()->flaperon_throw_offset * (1 / 500.0f)) : 0;

    if (IS_RC_MODE_ACTIVE(BOXCAMSTAB)) {
        input[INPUT_GIMBAL_PITCH] = scaleRangef(attitude.values.pitch, -1800, 1800, SERVO_VALUE_MIN, SERVO_VALUE_MAX);
        input[INPUT_GIMBAL_ROLL] = scaleRangef(attitude.values.roll, -1800, 1800, SERVO_VALUE_MIN, SERVO_VALUE_MAX);
    } else {
        input[INPUT_GIMBAL_PITCH] = 0;
        input[INPUT_GIMBAL_ROLL] = 0;
    }

    input[INPUT_STABILIZED_THROTTLE] = (motor[0] - 1000 - 500) * (1 / 500.0f);  // Since it derives from rcCommand or mincommand and must be [-500:+500]

    // Copy raw channels. This needs to be performed as 2 loops,
    // since there's a discontinuity between INPUT_RC_CH8 and INPUT_RC_CH9
    for (int ii = INPUT_RC_ROLL, jj = ROLL; ii <= INPUT_RC_CH8; ii++, jj++) {
        input[ii] = servosGetRCValue(jj);
    }
    for (int ii = INPUT_RC_CH9, jj = AUX5; ii <= INPUT_RC_CH16; ii++, jj++) {
        input[ii] = servosGetRCValue(jj);
    }

    for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
        servo[i] = 0.0f;
    }

    // mix servos according to rules
    for (int i = 0; i < servoRuleCount; i++) {
        const uint8_t target = currentServoMixer[i].targetChannel;
        const uint8_t from = currentServoMixer[i].inputSource;

        /*
         * Apply mixer speed limit. 1 [one] speed unit is defined as 10us/s:
         * 0 = no limiting
         * 1 = 10us/s -> full servo sweep (from 1000 to 2000) is performed in 100s
         * 10 = 100us/s -> full sweep (from 1000 to 2000)  is performed in 10s
         * 100 = 1000us/s -> full sweep in 1s
         */
        float rateLimit = currentServoMixer[i].speed * (1 / (100.0f / (SERVO_VALUE_MAX - SERVO_VALUE_MIN)));
        float inputLimited = rateLimitFilterApply4(&servoSpeedLimitFilter[i], input[from], rateLimit, dT);

        servo[target] += (inputLimited * currentServoMixer[i].rate) * (1 / 100.0f);
    }

    for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
        /*
         * Perform acumulated servo output scaling to match servo min and max values
         * Scales are already premultiplied by each servo rate.
         * Important: is servo rate is > 100%, total servo output might be bigger than
         * min/max
         */
        servo[i] *= servoMetadata[i].scales.maxmin[signbit(servo[i])];

        /*
         * Add a servo midpoint to the calculation
         */
        servo[i] += servoMetadata[i].values.center;
        // Servos are constrained in filterServos
    }

    servosWrite();
}

/*
 * Compute cached metadata for the servo
 */
void servosComputeMetadata(uint8_t servoIndex)
{
    const servoParam_t *params = servoParams(servoIndex);
    servoMetadata_t *metadata = &servoMetadata[servoIndex];

    // Apply servo rate to the scales, so we can avoid one
    // multiplication per servo while updating their values
    float rate = params->rate / 100.0f;

    metadata->scales.max = ((params->max - params->middle) / 500.0f) * rate;
    metadata->scales.min = ((params->middle - params->min) / 500.0f) * rate;

    metadata->values.min = PWM_VALUE_TO_SERVO(params->min);
    metadata->values.max = PWM_VALUE_TO_SERVO(params->max);
    metadata->values.center = PWM_VALUE_TO_SERVO(params->middle);
}

#define SERVO_AUTOTRIM_TIMER_MS     2000

typedef enum {
    AUTOTRIM_IDLE,
    AUTOTRIM_COLLECTING,
    AUTOTRIM_SAVE_PENDING,
    AUTOTRIM_DONE,
} servoAutotrimState_e;

void servosUpdateAutotrim(void)
{
    static servoAutotrimState_e trimState = AUTOTRIM_IDLE;
    static timeMs_t trimStartedAt;

    static float servoMiddleBackup[MAX_SUPPORTED_SERVOS];
    static float servoMiddleAccum[MAX_SUPPORTED_SERVOS];
    static int32_t servoMiddleAccumCount;

    if (IS_RC_MODE_ACTIVE(BOXAUTOTRIM)) {
        switch (trimState) {
            case AUTOTRIM_IDLE:
                if (ARMING_FLAG(ARMED)) {
                    // We are activating servo trim - backup current middles and prepare to average the data
                    for (int servoIndex = SERVO_ELEVATOR; servoIndex <= MIN(SERVO_RUDDER, MAX_SUPPORTED_SERVOS); servoIndex++) {
                        servoMiddleBackup[servoIndex] = servoMetadata[servoIndex].values.center;
                        servoMiddleAccum[servoIndex] = 0;
                    }

                    trimStartedAt = millis();
                    servoMiddleAccumCount = 0;
                    trimState = AUTOTRIM_COLLECTING;
                }
                else {
                    break;
                }
                // Fallthru

            case AUTOTRIM_COLLECTING:
                if (ARMING_FLAG(ARMED)) {
                    servoMiddleAccumCount++;

                    for (int servoIndex = SERVO_ELEVATOR; servoIndex <= MIN(SERVO_RUDDER, MAX_SUPPORTED_SERVOS); servoIndex++) {
                        servoMiddleAccum[servoIndex] += servo[servoIndex];
                    }

                    if ((millis() - trimStartedAt) > SERVO_AUTOTRIM_TIMER_MS) {
                        for (int servoIndex = SERVO_ELEVATOR; servoIndex <= MIN(SERVO_RUDDER, MAX_SUPPORTED_SERVOS); servoIndex++) {
                            float center = servoMiddleAccum[servoIndex] / servoMiddleAccumCount;
                            servoMetadata[servoIndex].values.center = center;
                            servoParamsMutable(servoIndex)->middle = SERVO_VALUE_TO_PWM(center);
                        }
                        trimState = AUTOTRIM_SAVE_PENDING;
                        pidResetErrorAccumulators(); //Reset Iterm since new midpoints override previously acumulated errors
                    }
                }
                else {
                    trimState = AUTOTRIM_IDLE;
                }
                break;

            case AUTOTRIM_SAVE_PENDING:
                // Wait for disarm and save to EEPROM
                if (!ARMING_FLAG(ARMED)) {
                    saveConfigAndNotify();
                    trimState = AUTOTRIM_DONE;
                }
                break;

            case AUTOTRIM_DONE:
                break;
        }
    }
    else {
        // We are deactivating servo trim - restore servo midpoints
        if (trimState == AUTOTRIM_SAVE_PENDING) {
            for (int servoIndex = SERVO_ELEVATOR; servoIndex <= MIN(SERVO_RUDDER, MAX_SUPPORTED_SERVOS); servoIndex++) {
                servoMetadata[servoIndex].values.center = servoMiddleBackup[servoIndex];
            }
        }

        trimState = AUTOTRIM_IDLE;
    }
}

int16_t servosGetPWM(uint8_t servoIndex)
{
    return SERVO_VALUE_TO_PWM(servo[servoIndex]);
}
