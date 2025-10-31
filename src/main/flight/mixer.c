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

#include "config/config_reset.h"
#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "config/config_reset.h"

#include "drivers/pwm_output.h"
#include "drivers/pwm_mapping.h"
#include "drivers/time.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/controlrate_profile.h"
#include "fc/settings.h"

#include "flight/failsafe.h"
#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/pid.h"
#include "flight/servos.h"

#include "navigation/navigation.h"

#include "rx/rx.h"

#include "sensors/battery.h"

#define MAX_THROTTLE 2000
#define MAX_THROTTLE_ROVER 1850

FASTRAM int16_t motor[MAX_SUPPORTED_MOTORS];
FASTRAM int16_t motor_disarmed[MAX_SUPPORTED_MOTORS];
static float motorMixRange;
static float mixerScale = 1.0f;
static EXTENDED_FASTRAM motorMixer_t currentMixer[MAX_SUPPORTED_MOTORS];
static EXTENDED_FASTRAM uint8_t motorCount = 0;
EXTENDED_FASTRAM int mixerThrottleCommand;
static EXTENDED_FASTRAM int throttleIdleValue = 0;
static EXTENDED_FASTRAM int motorValueWhenStopped = 0;
static reversibleMotorsThrottleState_e reversibleMotorsThrottleState = MOTOR_DIRECTION_FORWARD;
static EXTENDED_FASTRAM int throttleDeadbandLow = 0;
static EXTENDED_FASTRAM int throttleDeadbandHigh = 0;
static EXTENDED_FASTRAM int throttleRangeMin = 0;
static EXTENDED_FASTRAM int throttleRangeMax = 0;
static EXTENDED_FASTRAM int8_t motorYawMultiplier = 1;

int motorZeroCommand = 0;

PG_REGISTER_WITH_RESET_TEMPLATE(reversibleMotorsConfig_t, reversibleMotorsConfig, PG_REVERSIBLE_MOTORS_CONFIG, 0);

PG_RESET_TEMPLATE(reversibleMotorsConfig_t, reversibleMotorsConfig,
    .deadband_low = SETTING_3D_DEADBAND_LOW_DEFAULT,
    .deadband_high = SETTING_3D_DEADBAND_HIGH_DEFAULT,
    .neutral = SETTING_3D_NEUTRAL_DEFAULT
);

PG_REGISTER_WITH_RESET_TEMPLATE(motorConfig_t, motorConfig, PG_MOTOR_CONFIG, 11);

PG_RESET_TEMPLATE(motorConfig_t, motorConfig,
    .motorPwmProtocol = SETTING_MOTOR_PWM_PROTOCOL_DEFAULT,
    .motorPwmRate = SETTING_MOTOR_PWM_RATE_DEFAULT,
    .mincommand = SETTING_MIN_COMMAND_DEFAULT,
    .motorPoleCount = SETTING_MOTOR_POLES_DEFAULT,            // Most brushless motors that we use are 14 poles
);
PG_REGISTER_ARRAY_WITH_RESET_FN(timerOverride_t, HARDWARE_TIMER_DEFINITION_COUNT, timerOverrides, PG_TIMER_OVERRIDE_CONFIG, 0);

#define CRASH_OVER_AFTER_CRASH_FLIP_STICK_MIN 0.15f

void pgResetFn_timerOverrides(timerOverride_t *instance)
{
    for (int i = 0; i < HARDWARE_TIMER_DEFINITION_COUNT; ++i) {
        RESET_CONFIG(timerOverride_t, &instance[i], .outputMode = OUTPUT_MODE_AUTO);
    }
}

int getThrottleIdleValue(void)
{
    if (!throttleIdleValue) {
        throttleIdleValue = motorConfig()->mincommand + (((getMaxThrottle() - motorConfig()->mincommand) / 100.0f) * currentBatteryProfile->motor.throttleIdle);
    }

    return throttleIdleValue;
}

static void computeMotorCount(void)
{
    static bool firstRun = true;
    if (!firstRun) {
        return;
    }
    motorCount = 0;
    for (int i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
        bool isMotorUsed = false;
        for(int j = 0; j< MAX_MIXER_PROFILE_COUNT; j++){
            if (mixerMotorMixersByIndex(j)[i].throttle != 0.0f) {
                isMotorUsed = true;
            }
        }
        // check if done
        if (!isMotorUsed) {
            break;
        }
        motorCount++;
    }
    firstRun = false;
}

bool ifMotorstopFeatureEnabled(void){
    return currentMixerConfig.motorstopOnLow;
}

uint8_t getMotorCount(void) {
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
    DISABLE_STATE(FIXED_WING_LEGACY);
    DISABLE_STATE(MULTIROTOR);
    DISABLE_STATE(ROVER);
    DISABLE_STATE(BOAT);
    DISABLE_STATE(AIRPLANE);
    DISABLE_STATE(MOVE_FORWARD_ONLY);
    DISABLE_STATE(TAILSITTER);

    if (currentMixerConfig.platformType == PLATFORM_AIRPLANE) {
        ENABLE_STATE(FIXED_WING_LEGACY);
        ENABLE_STATE(AIRPLANE);
        ENABLE_STATE(ALTITUDE_CONTROL);
        ENABLE_STATE(MOVE_FORWARD_ONLY);
    } if (currentMixerConfig.platformType == PLATFORM_ROVER) {
        ENABLE_STATE(ROVER);
        ENABLE_STATE(FIXED_WING_LEGACY);
        ENABLE_STATE(MOVE_FORWARD_ONLY);
    } if (currentMixerConfig.platformType == PLATFORM_BOAT) {
        ENABLE_STATE(BOAT);
        ENABLE_STATE(FIXED_WING_LEGACY);
        ENABLE_STATE(MOVE_FORWARD_ONLY);
    } else if (currentMixerConfig.platformType == PLATFORM_MULTIROTOR) {
        ENABLE_STATE(MULTIROTOR);
        ENABLE_STATE(ALTITUDE_CONTROL);
    } else if (currentMixerConfig.platformType == PLATFORM_TRICOPTER) {
        ENABLE_STATE(MULTIROTOR);
        ENABLE_STATE(ALTITUDE_CONTROL);
    } else if (currentMixerConfig.platformType == PLATFORM_HELICOPTER) {
        ENABLE_STATE(MULTIROTOR);
        ENABLE_STATE(ALTITUDE_CONTROL);
    }

    if (currentMixerConfig.tailsitterOrientationOffset) {
        ENABLE_STATE(TAILSITTER);
    } else {
        DISABLE_STATE(TAILSITTER);
    }

    if (currentMixerConfig.hasFlaps) {
        ENABLE_STATE(FLAPERON_AVAILABLE);
    } else {
        DISABLE_STATE(FLAPERON_AVAILABLE);
    }
    if (
        currentMixerConfig.platformType == PLATFORM_BOAT ||
        currentMixerConfig.platformType == PLATFORM_ROVER ||
        navConfig()->fw.useFwNavYawControl
    ) {
        ENABLE_STATE(FW_HEADING_USE_YAW);
    } else {
        DISABLE_STATE(FW_HEADING_USE_YAW);
    }
}

void nullMotorRateLimiting(const float dT)
{
    UNUSED(dT);
}

void mixerInit(void)
{
    computeMotorCount();
    loadPrimaryMotorMixer();
    // in 3D mode, mixer gain has to be halved
    if (feature(FEATURE_REVERSIBLE_MOTORS)) {
        mixerScale = 0.5f;
    }

    throttleDeadbandLow = PWM_RANGE_MIDDLE - rcControlsConfig()->mid_throttle_deadband;
    throttleDeadbandHigh = PWM_RANGE_MIDDLE + rcControlsConfig()->mid_throttle_deadband;

    mixerResetDisarmedMotors();

    if (currentMixerConfig.motorDirectionInverted) {
        motorYawMultiplier = -1;
    } else {
        motorYawMultiplier = 1;
    }
}

void mixerResetDisarmedMotors(void)
{
    getThrottleIdleValue();

    if (feature(FEATURE_REVERSIBLE_MOTORS)) {
        motorZeroCommand = reversibleMotorsConfig()->neutral;
        throttleRangeMin = throttleDeadbandHigh;
        throttleRangeMax = getMaxThrottle();
    } else {
        motorZeroCommand = motorConfig()->mincommand;
        throttleRangeMin = throttleIdleValue;
        throttleRangeMax = getMaxThrottle();
    }

    reversibleMotorsThrottleState = MOTOR_DIRECTION_FORWARD;

    if (ifMotorstopFeatureEnabled()) {
        motorValueWhenStopped = motorZeroCommand;
    } else {
        motorValueWhenStopped = throttleIdleValue;
    }

    // set disarmed motor values
    for (int i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
        motor_disarmed[i] = motorZeroCommand;
    }
}
#if !defined(SITL_BUILD)
static uint16_t handleOutputScaling(
    int16_t input,          // Input value from the mixer
    int16_t stopThreshold,  // Threshold value to check if motor should be rotating or not
    int16_t onStopValue,    // Value sent to the ESC when min rotation is required - on motor_stop it is STOP command, without motor_stop it's a value that keeps rotation
    int16_t inputScaleMin,  // Input range - min value
    int16_t inputScaleMax,  // Input range - max value
    int16_t outputScaleMin, // Output range - min value
    int16_t outputScaleMax, // Output range - max value
    bool moveForward        // If motor should be rotating FORWARD or BACKWARD
)
{
    int16_t value;
    if ((moveForward && input < stopThreshold) || (!moveForward && input > stopThreshold)) {
        //Send motor stop command
        value = onStopValue;
    }
    else {
        //Scale input to protocol output values
        value = scaleRangef(input, inputScaleMin, inputScaleMax, outputScaleMin, outputScaleMax);
        value = constrain(value, outputScaleMin, outputScaleMax);
    }

    return value;
}
#endif
#ifdef USE_DSHOT
static void applyTurtleModeToMotors(void) {

    if (ARMING_FLAG(ARMED)) {
        const float flipPowerFactor = ((float)currentBatteryProfile->motor.turtleModePowerFactor)/100.0f;
        const float stickDeflectionPitchAbs = ABS(((float) rcCommand[PITCH]) / 500.0f);
        const float stickDeflectionRollAbs = ABS(((float) rcCommand[ROLL]) / 500.0f);
        const float stickDeflectionYawAbs = ABS(((float) rcCommand[YAW]) / 500.0f);
        //deflection stick position

        const float stickDeflectionPitchExpo =
                flipPowerFactor * stickDeflectionPitchAbs + power3(stickDeflectionPitchAbs) * (1 - flipPowerFactor);
        const float stickDeflectionRollExpo =
                flipPowerFactor * stickDeflectionRollAbs + power3(stickDeflectionRollAbs) * (1 - flipPowerFactor);
        const float stickDeflectionYawExpo =
                flipPowerFactor * stickDeflectionYawAbs + power3(stickDeflectionYawAbs) * (1 - flipPowerFactor);

        float signPitch = rcCommand[PITCH] < 0 ? 1 : -1;
        float signRoll = rcCommand[ROLL] < 0 ? 1 : -1;
        float signYaw = (float)((rcCommand[YAW] < 0 ? 1 : -1) * (currentMixerConfig.motorDirectionInverted ? 1 : -1));

        float stickDeflectionLength = calc_length_pythagorean_2D(stickDeflectionPitchAbs, stickDeflectionRollAbs);
        float stickDeflectionExpoLength = calc_length_pythagorean_2D(stickDeflectionPitchExpo, stickDeflectionRollExpo);

        if (stickDeflectionYawAbs > MAX(stickDeflectionPitchAbs, stickDeflectionRollAbs)) {
            // If yaw is the dominant, disable pitch and roll
            stickDeflectionLength = stickDeflectionYawAbs;
            stickDeflectionExpoLength = stickDeflectionYawExpo;
            signRoll = 0;
            signPitch = 0;
        } else {
            // If pitch/roll dominant, disable yaw
            signYaw = 0;
        }

        const float cosPhi = (stickDeflectionLength > 0) ? (stickDeflectionPitchAbs + stickDeflectionRollAbs) /
                                                           (fast_fsqrtf(2.0f) * stickDeflectionLength) : 0;
        const float cosThreshold = fast_fsqrtf(3.0f) / 2.0f; // cos(PI/6.0f)

        if (cosPhi < cosThreshold) {
            // Enforce either roll or pitch exclusively, if not on diagonal
            if (stickDeflectionRollAbs > stickDeflectionPitchAbs) {
                signPitch = 0;
            } else {
                signRoll = 0;
            }
        }

        // Apply a reasonable amount of stick deadband
        const float crashFlipStickMinExpo =
                flipPowerFactor * CRASH_OVER_AFTER_CRASH_FLIP_STICK_MIN + power3(CRASH_OVER_AFTER_CRASH_FLIP_STICK_MIN) * (1 - flipPowerFactor);
        const float flipStickRange = 1.0f - crashFlipStickMinExpo;
        const float flipPower = MAX(0.0f, stickDeflectionExpoLength - crashFlipStickMinExpo) / flipStickRange;

        for (int i = 0; i < motorCount; ++i) {

            float motorOutputNormalised =
                    signPitch * currentMixer[i].pitch +
                    signRoll * currentMixer[i].roll +
                    signYaw * currentMixer[i].yaw;

            if (motorOutputNormalised < 0) {
                motorOutputNormalised = 0;
            }

            motorOutputNormalised = MIN(1.0f, flipPower * motorOutputNormalised);

            motor[i] = (int16_t)scaleRangef(motorOutputNormalised, 0, 1, motorConfig()->mincommand, getMaxThrottle());
        }
    } else {
        // Disarmed mode
        stopMotors();
    }
}
#endif

void FAST_CODE writeMotors(void)
{
#if !defined(SITL_BUILD)
    for (int i = 0; i < motorCount; i++) {
        uint16_t motorValue;
#ifdef USE_DSHOT
        if (isMotorProtocolDigital()) {
            // If we use DSHOT we need to convert motorValue to DSHOT ranges
            if (feature(FEATURE_REVERSIBLE_MOTORS)) {
                if (reversibleMotorsThrottleState == MOTOR_DIRECTION_FORWARD) {
                    motorValue = handleOutputScaling(
                        motor[i],
                        throttleRangeMin,
                        DSHOT_DISARM_COMMAND,
                        throttleRangeMin,
                        throttleRangeMax,
                        DSHOT_3D_DEADBAND_HIGH,
                        DSHOT_MAX_THROTTLE,
                        true
                    );
                } else {
                    motorValue = handleOutputScaling(
                        motor[i],
                        throttleRangeMax,
                        DSHOT_DISARM_COMMAND,
                        throttleRangeMin,
                        throttleRangeMax,
                        DSHOT_MIN_THROTTLE,
                        DSHOT_3D_DEADBAND_LOW,
                        false
                    );
                }
            }
            else {
                motorValue = handleOutputScaling(
                    motor[i],
                    throttleIdleValue,
                    DSHOT_DISARM_COMMAND,
                    motorConfig()->mincommand,
                    getMaxThrottle(),
                    DSHOT_MIN_THROTTLE,
                    DSHOT_MAX_THROTTLE,
                    true
                );
            }
        }
        else
#endif
        {
            if (feature(FEATURE_REVERSIBLE_MOTORS)) {
                if (reversibleMotorsThrottleState == MOTOR_DIRECTION_FORWARD) {
                    motorValue = handleOutputScaling(
                        motor[i],
                        throttleRangeMin,
                        motor[i],
                        throttleRangeMin,
                        throttleRangeMax,
                        reversibleMotorsConfig()->deadband_high,
                        getMaxThrottle(),
                        true
                    );
                } else {
                    motorValue = handleOutputScaling(
                        motor[i],
                        throttleRangeMax,
                        motor[i],
                        throttleRangeMin,
                        throttleRangeMax,
                        motorConfig()->mincommand,
                        reversibleMotorsConfig()->deadband_low,
                        false
                    );
                }
            } else {
                motorValue = motor[i];
            }
        }

        pwmWriteMotor(i, motorValue);
    }
#endif
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
    writeAllMotors(feature(FEATURE_REVERSIBLE_MOTORS) ? reversibleMotorsConfig()->neutral : motorConfig()->mincommand);

    delay(50); // give the timers and ESCs a chance to react.
}

void stopPwmAllMotors(void)
{
#if !defined(SITL_BUILD)
    pwmShutdownPulsesForAllMotors(motorCount);
#endif

}

static int getReversibleMotorsThrottleDeadband(void)
{
    int directionValue;

    if (reversibleMotorsThrottleState == MOTOR_DIRECTION_BACKWARD) {
        directionValue = reversibleMotorsConfig()->deadband_low;
    } else {
        directionValue = reversibleMotorsConfig()->deadband_high;
    }

    return ifMotorstopFeatureEnabled() ? reversibleMotorsConfig()->neutral : directionValue;
}

void FAST_CODE mixTable(void)
{
#ifdef USE_DSHOT
    if (FLIGHT_MODE(TURTLE_MODE)) {
        applyTurtleModeToMotors();
        return;
    }
#endif
#ifdef USE_DEV_TOOLS
    bool isDisarmed = !ARMING_FLAG(ARMED) || systemConfig()->groundTestMode;
#else
    bool isDisarmed = !ARMING_FLAG(ARMED);
#endif
    bool motorStopIsActive = getMotorStatus() != MOTOR_RUNNING && !isDisarmed;
    if (isDisarmed || motorStopIsActive) {
        for (int i = 0; i < motorCount; i++) {
            motor[i] = isDisarmed ? motor_disarmed[i] : motorValueWhenStopped;
        }
        mixerThrottleCommand = motor[0];
        return;
    }

    int16_t input[3];   // RPY, range [-500:+500]
    // Allow direct stick input to motors in passthrough mode on airplanes
    if (STATE(FIXED_WING_LEGACY) && FLIGHT_MODE(MANUAL_MODE)) {
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
            -motorYawMultiplier * input[YAW] * currentMixer[i].yaw) * mixerScale;

        if (rpyMix[i] > rpyMixMax) rpyMixMax = rpyMix[i];
        if (rpyMix[i] < rpyMixMin) rpyMixMin = rpyMix[i];
    }

    int16_t rpyMixRange = rpyMixMax - rpyMixMin;
    int16_t throttleRange;
    int16_t throttleMin, throttleMax;

    // Find min and max throttle based on condition.
#ifdef USE_PROGRAMMING_FRAMEWORK
    if (LOGIC_CONDITION_GLOBAL_FLAG(LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_THROTTLE)) {
        throttleRangeMin = throttleIdleValue;
        throttleRangeMax = getMaxThrottle();
        mixerThrottleCommand = constrain(logicConditionValuesByType[LOGIC_CONDITION_OVERRIDE_THROTTLE], throttleRangeMin, throttleRangeMax);
    } else
#endif
    if (feature(FEATURE_REVERSIBLE_MOTORS)) {
        if (rcCommand[THROTTLE] >= (throttleDeadbandHigh) || STATE(SET_REVERSIBLE_MOTORS_FORWARD)) {
            /*
             * Throttle is above deadband, FORWARD direction
             */
            reversibleMotorsThrottleState = MOTOR_DIRECTION_FORWARD;
            throttleRangeMax = getMaxThrottle();
            throttleRangeMin = throttleDeadbandHigh;
            DISABLE_STATE(SET_REVERSIBLE_MOTORS_FORWARD);
        } else if (rcCommand[THROTTLE] <= throttleDeadbandLow) {
            /*
             * Throttle is below deadband, BACKWARD direction
             */
            reversibleMotorsThrottleState = MOTOR_DIRECTION_BACKWARD;
            throttleRangeMax = throttleDeadbandLow;
            throttleRangeMin = motorConfig()->mincommand;
        }

        motorValueWhenStopped = getReversibleMotorsThrottleDeadband();
        mixerThrottleCommand = constrain(rcCommand[THROTTLE], throttleRangeMin, throttleRangeMax);

#ifdef USE_DSHOT
        if(isMotorProtocolDigital() && reversibleMotorsThrottleState == MOTOR_DIRECTION_BACKWARD) {
            /*
             * We need to start the throttle output from stick input to start in the middle of the stick at the low and.
             * Without this, it's starting at the high side.
             */
            int throttleDistanceToMax = throttleRangeMax - rcCommand[THROTTLE];
            mixerThrottleCommand = throttleRangeMin + throttleDistanceToMax;
        }
#endif
    } else {
        mixerThrottleCommand = rcCommand[THROTTLE];
        throttleRangeMin = throttleIdleValue;
        throttleRangeMax = getMaxThrottle();

        // Throttle scaling to limit max throttle when battery is full
#ifdef USE_PROGRAMMING_FRAMEWORK
        mixerThrottleCommand = ((mixerThrottleCommand - throttleRangeMin) * getThrottleScale(currentBatteryProfile->motor.throttleScale)) + throttleRangeMin;
#else
        mixerThrottleCommand = ((mixerThrottleCommand - throttleRangeMin) * currentBatteryProfile->motor.throttleScale) + throttleRangeMin;
#endif
        // Throttle compensation based on battery voltage
        if (feature(FEATURE_THR_VBAT_COMP) && isAmperageConfigured() && feature(FEATURE_VBAT)) {
            mixerThrottleCommand = MIN(throttleRangeMin + (mixerThrottleCommand - throttleRangeMin) * calculateThrottleCompensationFactor(), throttleRangeMax);
        }
    }

    throttleMin = throttleRangeMin;
    throttleMax = throttleRangeMax;
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
    for (int i = 0; i < motorCount; i++) {
        motor[i] = rpyMix[i] + constrain(mixerThrottleCommand * currentMixer[i].throttle, throttleMin, throttleMax);

        if (failsafeIsActive()) {
            motor[i] = constrain(motor[i], motorConfig()->mincommand, getMaxThrottle());
        } else {
            motor[i] = constrain(motor[i], throttleRangeMin, throttleRangeMax);
        }

        //stop motors
        if (currentMixer[i].throttle <= 0.0f) {
            motor[i] = motorZeroCommand;
        }
        //spin stopped motors only in mixer transition mode
        if (isMixerTransitionMixing && currentMixer[i].throttle <= -1.05f && currentMixer[i].throttle >= -2.0f && !feature(FEATURE_REVERSIBLE_MOTORS)) {
            motor[i] = -currentMixer[i].throttle * 1000;
            motor[i] = constrain(motor[i], throttleRangeMin, throttleRangeMax);
        }
    }
}

int16_t getThrottlePercent(bool useScaled)
{
    int16_t thr = constrain(mixerThrottleCommand, PWM_RANGE_MIN, PWM_RANGE_MAX);

    if (useScaled) {
       thr = (thr - throttleIdleValue) * 100 / (getMaxThrottle() - throttleIdleValue);
    } else {
        thr = (rxGetChannelValue(THROTTLE) - PWM_RANGE_MIN) * 100 / (PWM_RANGE_MAX - PWM_RANGE_MIN);
    }
    return thr;
}

uint16_t setDesiredThrottle(uint16_t throttle, bool allowMotorStop)
{
    const uint16_t throttleIdleValue = getThrottleIdleValue();

    if (allowMotorStop && throttle < throttleIdleValue) {
        ENABLE_STATE(NAV_MOTOR_STOP_OR_IDLE);
        return throttle;
    }
    return constrain(throttle, throttleIdleValue, getMaxThrottle());
}

motorStatus_e getMotorStatus(void)
{
    if (STATE(NAV_MOTOR_STOP_OR_IDLE)) {
        return MOTOR_STOPPED_AUTO;
    }

    const bool fixedWingOrAirmodeNotActive = STATE(FIXED_WING_LEGACY) || !STATE(AIRMODE_ACTIVE);

    if (throttleStickIsLow() && fixedWingOrAirmodeNotActive) {
        if ((navConfig()->general.flags.nav_overrides_motor_stop == NOMS_OFF_ALWAYS) && failsafeIsActive()) {
            // If we are in failsafe and user was holding stick low before it was triggered and nav_overrides_motor_stop is set to OFF_ALWAYS
            // and either on a plane or on a quad with inactive airmode - stop motor
            return MOTOR_STOPPED_USER;

        } else if (!failsafeIsActive()) {
            // If user is holding stick low, we are not in failsafe and either on a plane or on a quad with inactive
            // airmode - we need to check if we are allowing navigation to override MOTOR_STOP

            switch (navConfig()->general.flags.nav_overrides_motor_stop) {
                case NOMS_ALL_NAV:
                    return navigationInAutomaticThrottleMode() ? MOTOR_RUNNING : MOTOR_STOPPED_USER;

                case NOMS_AUTO_ONLY:
                    return navigationIsFlyingAutonomousMode() ? MOTOR_RUNNING : MOTOR_STOPPED_USER;

                case NOMS_OFF:
                default:
                    return MOTOR_STOPPED_USER;
            }
        }
    }

    return MOTOR_RUNNING;
}

void loadPrimaryMotorMixer(void) {
    for (int i = 0; i < MAX_SUPPORTED_MOTORS; i++) {
        currentMixer[i] = *primaryMotorMixer(i);
    }
}

bool areMotorsRunning(void)
{
    if (ARMING_FLAG(ARMED)) {
        return true;
    } else {
        for (int i = 0; i < motorCount; i++) {
            if (motor_disarmed[i] != motorZeroCommand) {
                return true;
            }
        }
    }

    return false;
}

uint16_t getMaxThrottle(void) {

    static uint16_t throttle = 0;

    if (throttle == 0) {
        if (STATE(ROVER) || STATE(BOAT)) {
            throttle = MAX_THROTTLE_ROVER;
        } else {
            throttle = MAX_THROTTLE;
        }
    }

    return throttle;
}