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
#include "fc/rc_control.h"
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

PG_REGISTER_WITH_RESET_TEMPLATE(servoConfig_t, servoConfig, PG_SERVO_CONFIG, 0);

PG_RESET_TEMPLATE(servoConfig_t, servoConfig,
    .servoCenterPulse = 1500,
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

static float servo[MAX_SUPPORTED_SERVOS];

static uint8_t servoRuleCount = 0;
static servoMixer_t currentServoMixer[MAX_SERVO_RULES];
static int servoOutputEnabled;

static uint8_t mixerUsesServos;
static uint8_t minServoIndex;
static uint8_t maxServoIndex;

static biquadFilter_t servoFilter[MAX_SUPPORTED_SERVOS];
static bool servoFilterIsSet;

static servoMetadata_t servoMetadata[MAX_SUPPORTED_SERVOS];
static rateLimitFilter_t servoSpeedLimitFilter[MAX_SERVO_RULES];

int16_t getFlaperonDirection(uint8_t servoPin)
{
    if (servoPin == SERVO_FLAPPERON_2) {
        return -1;
    } else {
        return 1;
    }
}

/*
 * Compute scaling factor for upper and lower servo throw
 */
void servoComputeMetadata(uint8_t servoIndex) {
    // TODO: Fix scaleMax and scaleMin
    servoMetadata[servoIndex].scaleMax = (servoParams(servoIndex)->max - servoParams(servoIndex)->middle) / 500.0f;
    servoMetadata[servoIndex].scaleMin = (servoParams(servoIndex)->middle - servoParams(servoIndex)->min) / 500.0f;
    servoMetadata[servoIndex].minOutput = rcCommandMapPWMValue(servoParams(servoIndex)->min);
    servoMetadata[servoIndex].maxOutput = rcCommandMapPWMValue(servoParams(servoIndex)->max);
    servoMetadata[servoIndex].middleOutput = rcCommandMapPWMValue(servoParams(servoIndex)->middle);
}

void servosInit(void)
{
    // give all servos a default command
    for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
        servo[i] = RC_COMMAND_CENTER;
    }

    /*
     * load mixer
     */
    loadCustomServoMixer();

    // If there are servo rules after all, update variables
    if (servoRuleCount > 0) {
        servoOutputEnabled = 1;
        mixerUsesServos = 1;
    }

    for (uint8_t i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
        servoComputeMetadata(i);
    }
}

void loadCustomServoMixer(void)
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
            servo[i] = (int16_t)lrintf(biquadFilterApply(&servoFilter[i], (float)servo[i]));
        }
    }

    for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
        servo[i] = constrain(servo[i], servoParams(i)->min, servoParams(i)->max);
    }
}

void writeServos(void)
{
    filterServos();

    int servoIndex = 0;

    /*
     * in case of tricopters, there might me a need to zero servo output when unarmed
     */
    if (mixerConfig()->platformType == PLATFORM_TRICOPTER && !ARMING_FLAG(ARMED) && !servoConfig()->tri_unarmed_servo) {
        for (int ii = minServoIndex; ii <= maxServoIndex; ii++) {
            pwmWriteServo(servoIndex++, 0);
        }
        return;
    }

    for (int ii = minServoIndex; ii <= maxServoIndex; ii++) {
        pwmWriteServo(servoIndex++, rcCommandToPWMValue(servo[ii]));
    }
}

void servoMixer(float dT)
{
    float input[INPUT_SOURCE_COUNT]; // Range [-1:+1]

    const rcCommand_t *controlOutput = rcControlGetOutput();

    if (FLIGHT_MODE(MANUAL_MODE)) {
        input[INPUT_STABILIZED_ROLL] = controlOutput->roll;
        input[INPUT_STABILIZED_PITCH] = controlOutput->pitch;
        input[INPUT_STABILIZED_YAW] = controlOutput->yaw;
    } else {
        // TODO: axisPID is not [-1,1] yet
        // Assisted modes (gyro only or gyro+acc according to AUX configuration in Gui
        input[INPUT_STABILIZED_ROLL] = axisPID[ROLL];
        input[INPUT_STABILIZED_PITCH] = axisPID[PITCH];
        input[INPUT_STABILIZED_YAW] = axisPID[YAW];

        // Reverse yaw servo when applying negative throttle for multirotor and tricopter
        if (controlOutput->throttle < RC_COMMAND_CENTER &&
        (mixerConfig()->platformType == PLATFORM_MULTIROTOR || mixerConfig()->platformType == PLATFORM_TRICOPTER)) {
            input[INPUT_STABILIZED_YAW] *= -1;
        }
    }

    input[INPUT_FEATURE_FLAPS] = FLIGHT_MODE(FLAPERON) ? servoConfig()->flaperon_throw_offset : 0;

    if (IS_RC_MODE_ACTIVE(BOXCAMSTAB)) {
        input[INPUT_GIMBAL_PITCH] = scaleRangef(attitude.values.pitch, -1800, 1800, RC_COMMAND_MIN, RC_COMMAND_MAX);
        input[INPUT_GIMBAL_ROLL] = scaleRangef(attitude.values.roll, -1800, 1800, RC_COMMAND_MIN, RC_COMMAND_MAX);
    } else {
        input[INPUT_GIMBAL_PITCH] = 0;
        input[INPUT_GIMBAL_ROLL] = 0;
    }

    input[INPUT_STABILIZED_THROTTLE] = controlOutput->throttle;

#define MAP_RC_INPUT(x) rcCommandMapPWMValue(rxGetChannelValue(x))
    input[INPUT_RC_ROLL]     = MAP_RC_INPUT(ROLL);
    input[INPUT_RC_PITCH]    = MAP_RC_INPUT(PITCH);
    input[INPUT_RC_YAW]      = MAP_RC_INPUT(YAW);
    input[INPUT_RC_THROTTLE] = MAP_RC_INPUT(THROTTLE);
    input[INPUT_RC_CH5]      = MAP_RC_INPUT(AUX1);
    input[INPUT_RC_CH6]      = MAP_RC_INPUT(AUX2);
    input[INPUT_RC_CH7]      = MAP_RC_INPUT(AUX3);
    input[INPUT_RC_CH8]      = MAP_RC_INPUT(AUX4);
    input[INPUT_RC_CH9]      = MAP_RC_INPUT(AUX5);
    input[INPUT_RC_CH10]     = MAP_RC_INPUT(AUX6);
    input[INPUT_RC_CH11]     = MAP_RC_INPUT(AUX7);
    input[INPUT_RC_CH12]     = MAP_RC_INPUT(AUX8);
    input[INPUT_RC_CH13]     = MAP_RC_INPUT(AUX9);
    input[INPUT_RC_CH14]     = MAP_RC_INPUT(AUX10);
    input[INPUT_RC_CH15]     = MAP_RC_INPUT(AUX11);
    input[INPUT_RC_CH16]     = MAP_RC_INPUT(AUX12);

    for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {
        servo[i] = 0;
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
        float rateLimit = currentServoMixer[i].speed / (100.0f / RC_COMMAND_RANGE);
        float inputLimited = rateLimitFilterApply4(&servoSpeedLimitFilter[i], input[from], rateLimit, dT);

        servo[target] += (inputLimited * currentServoMixer[i].rate) / 100.0f;
    }

    for (int i = 0; i < MAX_SUPPORTED_SERVOS; i++) {

        /*
         * Apply servo rate
         */
        servo[i] *= servoParams(i)->rate / 100.0f;

        /*
         * Perform acumulated servo output scaling to match servo min and max values
         * Important: is servo rate is > 100%, total servo output might be bigger than
         * min/max
         */
        if (servo[i] > 0) {
            servo[i] *= servoMetadata[i].scaleMax;
        } else {
            servo[i] *= servoMetadata[i].scaleMin;
        }

        /*
         * Add a servo midpoint to the calculation
         */
        servo[i] += servoMetadata[i].middleOutput;

        /*
         * Constrain servo position to min/max to prevent servo damage
         * If servo was saturated above min/max, that means that user most probably
         * allowed the situation when smix weight sum for an output was above 100
         */
        servo[i] = constrain(servo[i], servoMetadata[i].minOutput, servoMetadata[i].maxOutput);
    }
}

#define SERVO_AUTOTRIM_TIMER_MS     2000

typedef enum {
    AUTOTRIM_IDLE,
    AUTOTRIM_COLLECTING,
    AUTOTRIM_SAVE_PENDING,
    AUTOTRIM_DONE,
} servoAutotrimState_e;

void processServoAutotrim(void)
{
    static servoAutotrimState_e trimState = AUTOTRIM_IDLE;
    static timeMs_t trimStartedAt;

    static int16_t servoMiddleBackup[MAX_SUPPORTED_SERVOS];
    static float servoMiddleAccum[MAX_SUPPORTED_SERVOS];
    static int32_t servoMiddleAccumCount;

    if (IS_RC_MODE_ACTIVE(BOXAUTOTRIM)) {
        switch (trimState) {
            case AUTOTRIM_IDLE:
                if (ARMING_FLAG(ARMED)) {
                    // We are activating servo trim - backup current middles and prepare to average the data
                    for (int servoIndex = SERVO_ELEVATOR; servoIndex <= MIN(SERVO_RUDDER, MAX_SUPPORTED_SERVOS); servoIndex++) {
                        servoMiddleBackup[servoIndex] = servoParams(servoIndex)->middle;
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
                            servoParamsMutable(servoIndex)->middle = rcCommandToPWMValue(servoMiddleAccum[servoIndex] / servoMiddleAccumCount);
                            servoComputeMetadata(servoIndex);
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
                servoParamsMutable(servoIndex)->middle = servoMiddleBackup[servoIndex];
                servoComputeMetadata(servoIndex);
            }
        }

        trimState = AUTOTRIM_IDLE;
    }
}

bool isServoOutputEnabled(void)
{
    return servoOutputEnabled;
}

bool isMixerUsingServos(void)
{
    return mixerUsesServos;
}
