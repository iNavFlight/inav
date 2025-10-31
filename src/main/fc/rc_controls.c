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

#include <math.h>

#include "platform.h"

#include "build/debug.h"

#include "blackbox/blackbox.h"

#include "cms/cms.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/cli.h"
#include "fc/config.h"
#include "fc/controlrate_profile.h"
#include "fc/fc_core.h"
#include "fc/rc_controls.h"
#include "fc/rc_curves.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/pid.h"
#include "flight/failsafe.h"
#include "flight/mixer.h"

#include "io/gps.h"
#include "io/beeper.h"

#include "navigation/navigation.h"

#include "rx/rx.h"

#include "sensors/barometer.h"
#include "sensors/battery.h"
#include "sensors/sensors.h"
#include "sensors/gyro.h"
#include "sensors/acceleration.h"

#define AIRMODE_DEADBAND 25
#define MIN_RC_TICK_INTERVAL_MS             20
#define DEFAULT_RC_SWITCH_DISARM_DELAY_MS   250     // Wait at least 250ms before disarming via switch
#define DEFAULT_PREARM_TIMEOUT              10000   // Prearm is invalidated after 10 seconds

stickPositions_e rcStickPositions;

FASTRAM int16_t rcCommand[4];           // interval [1000;2000] for THROTTLE and [-500;+500] for ROLL/PITCH/YAW

PG_REGISTER_WITH_RESET_TEMPLATE(rcControlsConfig_t, rcControlsConfig, PG_RC_CONTROLS_CONFIG, 4);

PG_RESET_TEMPLATE(rcControlsConfig_t, rcControlsConfig,
    .deadband = SETTING_DEADBAND_DEFAULT,
    .yaw_deadband = SETTING_YAW_DEADBAND_DEFAULT,
    .pos_hold_deadband = SETTING_POS_HOLD_DEADBAND_DEFAULT,
    .alt_hold_deadband = SETTING_ALT_HOLD_DEADBAND_DEFAULT,
    .mid_throttle_deadband = SETTING_3D_DEADBAND_THROTTLE_DEFAULT,
    .airmodeHandlingType = SETTING_AIRMODE_TYPE_DEFAULT,
    .airmodeThrottleThreshold = SETTING_AIRMODE_THROTTLE_THRESHOLD_DEFAULT,
);

PG_REGISTER_WITH_RESET_TEMPLATE(armingConfig_t, armingConfig, PG_ARMING_CONFIG, 3);

PG_RESET_TEMPLATE(armingConfig_t, armingConfig,
    .fixed_wing_auto_arm = SETTING_FIXED_WING_AUTO_ARM_DEFAULT,
    .disarm_always = SETTING_DISARM_ALWAYS_DEFAULT,
    .switchDisarmDelayMs = SETTING_SWITCH_DISARM_DELAY_DEFAULT,
    .prearmTimeoutMs = SETTING_PREARM_TIMEOUT_DEFAULT,
);

bool areSticksInApModePosition(uint16_t ap_mode)
{
    return ABS(rcCommand[ROLL]) < ap_mode && ABS(rcCommand[PITCH]) < ap_mode;
}

bool areSticksDeflected(void)
{
    return (ABS(rcCommand[ROLL]) > CONTROL_DEADBAND) || (ABS(rcCommand[PITCH]) > CONTROL_DEADBAND) || (ABS(rcCommand[YAW]) > CONTROL_DEADBAND);
}

bool isRollPitchStickDeflected(uint8_t deadband)
{
    return (ABS(rcCommand[ROLL]) > deadband) || (ABS(rcCommand[PITCH]) > deadband);
}

throttleStatus_e FAST_CODE NOINLINE calculateThrottleStatus(throttleStatusType_e type)
{
    int value = rxGetChannelValue(THROTTLE);    // THROTTLE_STATUS_TYPE_RC
    if (type == THROTTLE_STATUS_TYPE_COMMAND) {
        value = rcCommand[THROTTLE];
    }

    bool midThrottle = value > (reversibleMotorsConfig()->deadband_low) && value < (reversibleMotorsConfig()->deadband_high);
    if ((feature(FEATURE_REVERSIBLE_MOTORS) && midThrottle) || (!feature(FEATURE_REVERSIBLE_MOTORS) && (value < rxConfig()->mincheck))) {
        return THROTTLE_LOW;
    }

    return THROTTLE_HIGH;
}

bool throttleStickIsLow(void)
{
    return calculateThrottleStatus(feature(FEATURE_REVERSIBLE_MOTORS) ? THROTTLE_STATUS_TYPE_COMMAND : THROTTLE_STATUS_TYPE_RC) == THROTTLE_LOW;
}

int16_t throttleStickMixedValue(void)
{
    int16_t throttleValue;
    uint16_t lowLimit = feature(FEATURE_REVERSIBLE_MOTORS) ? PWM_RANGE_MIN : rxConfig()->mincheck;

    throttleValue = constrain(rxGetChannelValue(THROTTLE), lowLimit, PWM_RANGE_MAX);
    throttleValue = (uint16_t)(throttleValue - lowLimit) * PWM_RANGE_MIN / (PWM_RANGE_MAX - lowLimit);  // [LOWLIMIT;2000] -> [0;1000]

    return rcLookupThrottle(throttleValue);
}

rollPitchStatus_e calculateRollPitchCenterStatus(void)
{
    if (((rxGetChannelValue(PITCH) < (PWM_RANGE_MIDDLE + AIRMODE_DEADBAND)) && (rxGetChannelValue(PITCH) > (PWM_RANGE_MIDDLE -AIRMODE_DEADBAND)))
            && ((rxGetChannelValue(ROLL) < (PWM_RANGE_MIDDLE + AIRMODE_DEADBAND)) && (rxGetChannelValue(ROLL) > (PWM_RANGE_MIDDLE -AIRMODE_DEADBAND))))
        return CENTERED;

    return NOT_CENTERED;
}

stickPositions_e getRcStickPositions(void)
{
    return rcStickPositions;
}

bool checkStickPosition(stickPositions_e stickPos)
{
    const uint8_t mask[4] = { ROL_LO | ROL_HI, PIT_LO | PIT_HI, YAW_LO | YAW_HI, THR_LO | THR_HI };
    for (int i = 0; i < 4; i++) {
        if (((stickPos & mask[i]) != 0) && ((stickPos & mask[i]) != (rcStickPositions & mask[i]))) {
            return false;
        }
    }

    return true;
}

static void updateRcStickPositions(void)
{
    stickPositions_e tmp = 0;

    tmp |= ((rxGetChannelValue(ROLL) > rxConfig()->mincheck) ? 0x02 : 0x00) << (ROLL * 2);
    tmp |= ((rxGetChannelValue(ROLL) < rxConfig()->maxcheck) ? 0x01 : 0x00) << (ROLL * 2);

    tmp |= ((rxGetChannelValue(PITCH) > rxConfig()->mincheck) ? 0x02 : 0x00) << (PITCH * 2);
    tmp |= ((rxGetChannelValue(PITCH) < rxConfig()->maxcheck) ? 0x01 : 0x00) << (PITCH * 2);

    tmp |= ((rxGetChannelValue(YAW) > rxConfig()->mincheck) ? 0x02 : 0x00) << (YAW * 2);
    tmp |= ((rxGetChannelValue(YAW) < rxConfig()->maxcheck) ? 0x01 : 0x00) << (YAW * 2);

    tmp |= ((rxGetChannelValue(THROTTLE) > rxConfig()->mincheck) ? 0x02 : 0x00) << (THROTTLE * 2);
    tmp |= ((rxGetChannelValue(THROTTLE) < rxConfig()->maxcheck) ? 0x01 : 0x00) << (THROTTLE * 2);

    rcStickPositions = tmp;
}

void processRcStickPositions(bool isThrottleLow)
{
    static timeMs_t lastTickTimeMs = 0;
    static uint8_t rcDelayCommand;      // this indicates the number of time (multiple of RC measurement at 50Hz) the sticks must be maintained to run or switch off motors
    static uint32_t rcSticks;           // this hold sticks position for command combos
    static timeMs_t rcDisarmTimeMs;     // this is an extra guard for disarming through switch to prevent that one frame can disarm it
    const timeMs_t currentTimeMs = millis();

    updateRcStickPositions();

    uint32_t stTmp = getRcStickPositions();
    if (stTmp == rcSticks) {
        if (rcDelayCommand < 250) {
            if ((currentTimeMs - lastTickTimeMs) >= MIN_RC_TICK_INTERVAL_MS) {
                lastTickTimeMs = currentTimeMs;
                rcDelayCommand++;
            }
        }
    } else {
        rcDelayCommand = 0;
    }

    rcSticks = stTmp;

    // perform actions
    bool armingSwitchIsActive = IS_RC_MODE_ACTIVE(BOXARM);

    if (STATE(AIRPLANE) && ifMotorstopFeatureEnabled() && armingConfig()->fixed_wing_auto_arm) {
        // Auto arm on throttle when using fixedwing and motorstop
        if (!isThrottleLow) {
            tryArm();
            return;
        }
    }
    else {
        if (armingSwitchIsActive) {
            rcDisarmTimeMs = currentTimeMs;
            tryArm();
        } else {
            emergencyArmingUpdate(armingSwitchIsActive, false);
            // Disarming via ARM BOX
            // Don't disarm via switch if failsafe is active or receiver doesn't receive data - we can't trust receiver
            // and can't afford to risk disarming in the air
            if (ARMING_FLAG(ARMED) && !IS_RC_MODE_ACTIVE(BOXFAILSAFE) && rxIsReceivingSignal() && !failsafeIsActive()) {
                const timeMs_t disarmDelay = currentTimeMs - rcDisarmTimeMs;
                if (disarmDelay > armingConfig()->switchDisarmDelayMs) {
                    if (armingConfig()->disarm_always || isThrottleLow) {
                        disarm(DISARM_SWITCH);
                    }
                }
            }
            else {
                rcDisarmTimeMs = currentTimeMs;
            }
        }
    }

    if (rcDelayCommand != 20) {
        return;
    }

    /* Disable stick commands when armed, in CLI mode or CMS is active */
    bool disableStickCommands = ARMING_FLAG(ARMED) || cliMode;
#ifdef USE_CMS
    disableStickCommands = disableStickCommands || cmsInMenu;
#endif
    if (disableStickCommands) {
        return;
    }

    /* REMAINING SECTION HANDLES STICK COMANDS ONLY */

    // GYRO calibration
    if (rcSticks == THR_LO + YAW_LO + PIT_LO + ROL_CE) {
        gyroStartCalibration();
        return;
    }


#if defined(NAV_NON_VOLATILE_WAYPOINT_STORAGE)
    // Save waypoint list
    if (rcSticks == THR_LO + YAW_CE + PIT_HI + ROL_LO) {
        const bool success = saveNonVolatileWaypointList();
        beeper(success ? BEEPER_ACTION_SUCCESS : BEEPER_ACTION_FAIL);
    }

    // Load waypoint list
    if (rcSticks == THR_LO + YAW_CE + PIT_HI + ROL_HI) {
        const bool success = loadNonVolatileWaypointList(false);
        beeper(success ? BEEPER_ACTION_SUCCESS : BEEPER_ACTION_FAIL);
    }
#ifdef USE_MULTI_MISSION
    // Increment multi mission index up
    if (rcSticks == THR_LO + YAW_CE + PIT_CE + ROL_HI) {
        selectMultiMissionIndex(1);
        rcDelayCommand = 0;
        return;
    }

    // Decrement multi mission index down
    if (rcSticks == THR_LO + YAW_CE + PIT_CE + ROL_LO) {
        selectMultiMissionIndex(-1);
        rcDelayCommand = 0;
        return;
    }
#endif
    if (rcSticks == THR_LO + YAW_CE + PIT_LO + ROL_HI) {
        resetWaypointList();
        beeper(BEEPER_ACTION_FAIL); // The above cannot fail, but traditionally, we play FAIL for not-loading
    }
#endif

    // Multiple configuration profiles
    if (feature(FEATURE_TX_PROF_SEL)) {

        uint8_t i = 0;

        if (rcSticks == THR_LO + YAW_LO + PIT_CE + ROL_LO)          // ROLL left  -> Profile 1
            i = 1;
        else if (rcSticks == THR_LO + YAW_LO + PIT_HI + ROL_CE)     // PITCH up   -> Profile 2
            i = 2;
        else if (rcSticks == THR_LO + YAW_LO + PIT_CE + ROL_HI)     // ROLL right -> Profile 3
            i = 3;

        if (i) {
            setConfigProfileAndWriteEEPROM(i - 1);
            return;
        }

        i = 0;

        // Multiple battery configuration profiles
        if (rcSticks == THR_HI + YAW_LO + PIT_CE + ROL_LO)          // ROLL left  -> Profile 1
            i = 1;
        else if (rcSticks == THR_HI + YAW_LO + PIT_HI + ROL_CE)     // PITCH up   -> Profile 2
            i = 2;
        else if (rcSticks == THR_HI + YAW_LO + PIT_CE + ROL_HI)     // ROLL right -> Profile 3
            i = 3;

        if (i) {
            setConfigBatteryProfileAndWriteEEPROM(i - 1);
            batteryDisableProfileAutoswitch();
            activateBatteryProfile();
            return;
        }

    }

    // Save config
    if (rcSticks == THR_LO + YAW_LO + PIT_LO + ROL_HI) {
        saveConfigAndNotify();
    }

    // Calibrating Acc
    if (rcSticks == THR_HI + YAW_LO + PIT_LO + ROL_CE) {
        accStartCalibration();
        return;
    }

    // Calibrating Mag
    if (rcSticks == THR_HI + YAW_HI + PIT_LO + ROL_CE) {
        ENABLE_STATE(CALIBRATE_MAG);
        return;
    }

    // Accelerometer Trim
    if (rcSticks == THR_HI + YAW_CE + PIT_HI + ROL_CE) {
        applyAndSaveBoardAlignmentDelta(0, -2);
        rcDelayCommand = 10;
        return;
    } else if (rcSticks == THR_HI + YAW_CE + PIT_LO + ROL_CE) {
        applyAndSaveBoardAlignmentDelta(0, 2);
        rcDelayCommand = 10;
        return;
    } else if (rcSticks == THR_HI + YAW_CE + PIT_CE + ROL_HI) {
        applyAndSaveBoardAlignmentDelta(-2, 0);
        rcDelayCommand = 10;
        return;
    } else if (rcSticks == THR_HI + YAW_CE + PIT_CE + ROL_LO) {
        applyAndSaveBoardAlignmentDelta(2, 0);
        rcDelayCommand = 10;
        return;
    }
}

int32_t getRcStickDeflection(int32_t axis) {
    return MIN(ABS(rxGetChannelValue(axis) - PWM_RANGE_MIDDLE), 500);
}
