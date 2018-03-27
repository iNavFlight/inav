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
#include "fc/runtime_config.h"

#include "flight/failsafe.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/pid.h"
#include "flight/servos.h"

#include "navigation/navigation.h"

#include "rx/rx.h"


//#define MIXER_DEBUG

static uint8_t motorCount;

int16_t motor[MAX_SUPPORTED_MOTORS];
int16_t motor_disarmed[MAX_SUPPORTED_MOTORS];

static bool motorLimitReached = false;

PG_REGISTER_WITH_RESET_TEMPLATE(flight3DConfig_t, flight3DConfig, PG_MOTOR_3D_CONFIG, 0);

PG_RESET_TEMPLATE(flight3DConfig_t, flight3DConfig,
    .deadband3d_low = 1406,
    .deadband3d_high = 1514,
    .neutral3d = 1460
);

PG_REGISTER_WITH_RESET_TEMPLATE(mixerConfig_t, mixerConfig, PG_MIXER_CONFIG, 1);

PG_RESET_TEMPLATE(mixerConfig_t, mixerConfig,
    .mixerMode = MIXER_QUADX,
    .yaw_motor_direction = 1,
    .yaw_jump_prevention_limit = 200,
    .platformType = PLATFORM_MULTIROTOR,
    .hasFlaps = false
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

PG_REGISTER_WITH_RESET_TEMPLATE(motorConfig_t, motorConfig, PG_MOTOR_CONFIG, 0);

PG_RESET_TEMPLATE(motorConfig_t, motorConfig,
    .minthrottle = DEFAULT_MIN_THROTTLE,
    .motorPwmProtocol = DEFAULT_PWM_PROTOCOL,
    .motorPwmRate = DEFAULT_PWM_RATE,
    .maxthrottle = 1850,
    .mincommand = 1000
);

static motorMixer_t currentMixer[MAX_SUPPORTED_MOTORS];

PG_REGISTER_ARRAY(motorMixer_t, MAX_SUPPORTED_MOTORS, customMotorMixer, PG_MOTOR_MIXER, 0);

uint8_t getMotorCount(void)
{
    return motorCount;
}

bool mixerIsOutputSaturated(void)
{
    return motorLimitReached;
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
        motor_disarmed[i] = feature(FEATURE_3D) ? flight3DConfig()->neutral3d : motorConfig()->mincommand;
    }
}

void writeMotors(void)
{
    for (int i = 0; i < motorCount; i++) {
        pwmWriteMotor(i, motor[i]);
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

void mixTable(void)
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

        if (motorCount >= 4 && mixerConfig()->yaw_jump_prevention_limit < YAW_JUMP_PREVENTION_LIMIT_HIGH) {
            // prevent "yaw jump" during yaw correction
            input[YAW] = constrain(input[YAW], -mixerConfig()->yaw_jump_prevention_limit - ABS(rcCommand[YAW]), mixerConfig()->yaw_jump_prevention_limit + ABS(rcCommand[YAW]));
        }
    }

    // Initial mixer concept by bdoiron74 reused and optimized for Air Mode
    int16_t rpyMix[MAX_SUPPORTED_MOTORS];
    int16_t rpyMixMax = 0; // assumption: symetrical about zero.
    int16_t rpyMixMin = 0;

    // motors for non-servo mixes
    for (int i = 0; i < motorCount; i++) {
        rpyMix[i] =
            input[PITCH] * currentMixer[i].pitch +
            input[ROLL] * currentMixer[i].roll +
            -mixerConfig()->yaw_motor_direction * input[YAW] * currentMixer[i].yaw;

        if (rpyMix[i] > rpyMixMax) rpyMixMax = rpyMix[i];
        if (rpyMix[i] < rpyMixMin) rpyMixMin = rpyMix[i];
    }

    int16_t rpyMixRange = rpyMixMax - rpyMixMin;
    int16_t throttleRange, throttleCommand;
    int16_t throttleMin, throttleMax;
    static int16_t throttlePrevious = 0;   // Store the last throttle direction for deadband transitions

    // Find min and max throttle based on condition.
    if (feature(FEATURE_3D)) {
        if (!ARMING_FLAG(ARMED)) throttlePrevious = rxConfig()->midrc; // When disarmed set to mid_rc. It always results in positive direction after arming.

        if ((rcCommand[THROTTLE] <= (rxConfig()->midrc - rcControlsConfig()->deadband3d_throttle))) { // Out of band handling
            throttleMax = flight3DConfig()->deadband3d_low;
            throttleMin = motorConfig()->minthrottle;
            throttlePrevious = throttleCommand = rcCommand[THROTTLE];
        } else if (rcCommand[THROTTLE] >= (rxConfig()->midrc + rcControlsConfig()->deadband3d_throttle)) { // Positive handling
            throttleMax = motorConfig()->maxthrottle;
            throttleMin = flight3DConfig()->deadband3d_high;
            throttlePrevious = throttleCommand = rcCommand[THROTTLE];
        } else if ((throttlePrevious <= (rxConfig()->midrc - rcControlsConfig()->deadband3d_throttle)))  { // Deadband handling from negative to positive
            throttleCommand = throttleMax = flight3DConfig()->deadband3d_low;
            throttleMin = motorConfig()->minthrottle;
        } else {  // Deadband handling from positive to negative
            throttleMax = motorConfig()->maxthrottle;
            throttleCommand = throttleMin = flight3DConfig()->deadband3d_high;
        }
    } else {
        throttleCommand = rcCommand[THROTTLE];
        throttleMin = motorConfig()->minthrottle;
        throttleMax = motorConfig()->maxthrottle;
    }

    throttleRange = throttleMax - throttleMin;

    #define THROTTLE_CLIPPING_FACTOR    0.33f
    if (rpyMixRange > throttleRange) {
        motorLimitReached = true;
        float mixReduction = (float)throttleRange / rpyMixRange;

        for (int i = 0; i < motorCount; i++) {
            rpyMix[i] =  mixReduction  * rpyMix[i];
        }

        // Allow some clipping on edges to soften correction response
        throttleMin = throttleMin + (throttleRange / 2) - (throttleRange * THROTTLE_CLIPPING_FACTOR / 2);
        throttleMax = throttleMin + (throttleRange / 2) + (throttleRange * THROTTLE_CLIPPING_FACTOR / 2);
    } else {
        motorLimitReached = false;
        throttleMin = MIN(throttleMin + (rpyMixRange / 2), throttleMin + (throttleRange / 2) - (throttleRange * THROTTLE_CLIPPING_FACTOR / 2));
        throttleMax = MAX(throttleMax - (rpyMixRange / 2), throttleMin + (throttleRange / 2) + (throttleRange * THROTTLE_CLIPPING_FACTOR / 2));
    }

    // Now add in the desired throttle, but keep in a range that doesn't clip adjusted
    // roll/pitch/yaw. This could move throttle down, but also up for those low throttle flips.
    if (ARMING_FLAG(ARMED)) {
        for (int i = 0; i < motorCount; i++) {
            motor[i] = rpyMix[i] + constrain(throttleCommand * currentMixer[i].throttle, throttleMin, throttleMax);

            if (failsafeIsActive()) {
                motor[i] = constrain(motor[i], motorConfig()->mincommand, motorConfig()->maxthrottle);
            } else if (feature(FEATURE_3D)) {
                if (throttlePrevious <= (rxConfig()->midrc - rcControlsConfig()->deadband3d_throttle)) {
                    motor[i] = constrain(motor[i], motorConfig()->minthrottle, flight3DConfig()->deadband3d_low);
                } else {
                    motor[i] = constrain(motor[i], flight3DConfig()->deadband3d_high, motorConfig()->maxthrottle);
                }
            } else {
                motor[i] = constrain(motor[i], motorConfig()->minthrottle, motorConfig()->maxthrottle);
            }

            // Motor stop handling
            if (feature(FEATURE_MOTOR_STOP) && ARMING_FLAG(ARMED)) {
                bool failsafeMotorStop = failsafeRequiresMotorStop();
                bool navMotorStop = !failsafeIsActive() && STATE(NAV_MOTOR_STOP_OR_IDLE);
                bool userMotorStop = !navigationIsFlyingAutonomousMode() && !failsafeIsActive() && (rcData[THROTTLE] < rxConfig()->mincheck);
                if (failsafeMotorStop || navMotorStop || userMotorStop) {
                    if (feature(FEATURE_3D)) {
                        motor[i] = rxConfig()->midrc;
                    }
                    else {
                        motor[i] = motorConfig()->mincommand;
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < motorCount; i++) {
            motor[i] = motor_disarmed[i];
        }
    }
}
