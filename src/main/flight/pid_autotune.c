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

#include <platform.h>

#include "blackbox/blackbox.h"
#include "blackbox/blackbox_fielddefs.h"

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"

#include "config/feature.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "fc/config.h"
#include "fc/controlrate_profile.h"
#include "fc/rc_controls.h"
#include "fc/rc_adjustments.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "flight/pid.h"

#define AUTOTUNE_FIXED_WING_MIN_FF              10
#define AUTOTUNE_FIXED_WING_MAX_FF              200
#define AUTOTUNE_FIXED_WING_MIN_ROLL_PITCH_RATE 40
#define AUTOTUNE_FIXED_WING_MIN_YAW_RATE        10
#define AUTOTUNE_FIXED_WING_MAX_RATE            720

PG_REGISTER_WITH_RESET_TEMPLATE(pidAutotuneConfig_t, pidAutotuneConfig, PG_PID_AUTOTUNE_CONFIG, 2);

PG_RESET_TEMPLATE(pidAutotuneConfig_t, pidAutotuneConfig,
    .fw_detect_time = SETTING_FW_AUTOTUNE_DETECT_TIME_DEFAULT,
    .fw_min_stick = SETTING_FW_AUTOTUNE_MIN_STICK_DEFAULT,
    .fw_ff_to_p_gain = SETTING_FW_AUTOTUNE_FF_TO_P_GAIN_DEFAULT,
    .fw_ff_to_i_time_constant = SETTING_FW_AUTOTUNE_FF_TO_I_TC_DEFAULT,
    .fw_p_to_d_gain = SETTING_FW_AUTOTUNE_P_TO_D_GAIN_DEFAULT,
    .fw_rate_adjustment = SETTING_FW_AUTOTUNE_RATE_ADJUSTMENT_DEFAULT,
    .fw_convergence_rate = SETTING_FW_AUTOTUNE_CONVERGENCE_RATE_DEFAULT,
    .fw_max_rate_deflection = SETTING_FW_AUTOTUNE_MAX_RATE_DEFLECTION_DEFAULT,
);

typedef enum {
    DEMAND_TOO_LOW,
    DEMAND_UNDERSHOOT,
    DEMAND_OVERSHOOT,
} pidAutotuneState_e;

typedef struct {
    pidAutotuneState_e  state;
    timeMs_t            stateEnterTime;
    float   gainP;
    float   gainI;
    float   gainD;
    float   gainFF;
    float   rate;
    float   initialRate;
    float   maxAbsDesiredRateDps;
    float   maxAbsReachedRateDps;
    float   maxAbsPidOutput;
} pidAutotuneData_t;

#define AUTOTUNE_SAVE_PERIOD        5000        // Save interval is 5 seconds - when we turn off autotune we'll restore values from previous update at most 5 sec ago

#if defined(USE_AUTOTUNE_FIXED_WING) || defined(USE_AUTOTUNE_MULTIROTOR)

static pidAutotuneData_t    tuneCurrent[XYZ_AXIS_COUNT];
static pidAutotuneData_t    tuneSaved[XYZ_AXIS_COUNT];
static timeMs_t             lastGainsUpdateTime;

void autotuneUpdateGains(pidAutotuneData_t * data)
{
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        pidBankMutable()->pid[axis].P = lrintf(data[axis].gainP);
        pidBankMutable()->pid[axis].I = lrintf(data[axis].gainI);
        pidBankMutable()->pid[axis].D = lrintf(data[axis].gainD);
        pidBankMutable()->pid[axis].FF = lrintf(data[axis].gainFF);
        ((controlRateConfig_t *)currentControlRateProfile)->stabilized.rates[axis] = lrintf(data[axis].rate/10.0f);
    }
    schedulePidGainsUpdate();
}

void autotuneCheckUpdateGains(void)
{
    const timeMs_t currentTimeMs = millis();

    if ((currentTimeMs - lastGainsUpdateTime) < AUTOTUNE_SAVE_PERIOD) {
        return;
    }

    // If pilot will exit autotune we'll restore values we are flying now
    memcpy(tuneSaved, tuneCurrent, sizeof(pidAutotuneData_t) * XYZ_AXIS_COUNT);
    autotuneUpdateGains(tuneSaved);
    lastGainsUpdateTime = currentTimeMs;
}

void autotuneStart(void)
{
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        tuneCurrent[axis].gainP = pidBank()->pid[axis].P;
        tuneCurrent[axis].gainI = pidBank()->pid[axis].I;
        tuneCurrent[axis].gainD = pidBank()->pid[axis].D;
        tuneCurrent[axis].gainFF = pidBank()->pid[axis].FF;
        tuneCurrent[axis].rate = currentControlRateProfile->stabilized.rates[axis] * 10.0f;
        tuneCurrent[axis].stateEnterTime = millis();
        tuneCurrent[axis].state = DEMAND_TOO_LOW;
        tuneCurrent[axis].initialRate = currentControlRateProfile->stabilized.rates[axis] * 10.0f;
        tuneCurrent[axis].maxAbsDesiredRateDps = 0;
        tuneCurrent[axis].maxAbsReachedRateDps = 0;
        tuneCurrent[axis].maxAbsPidOutput = 0;
    }

    memcpy(tuneSaved, tuneCurrent, sizeof(pidAutotuneData_t) * XYZ_AXIS_COUNT);
    lastGainsUpdateTime = millis();
}

void autotuneUpdateState(void)
{
    if (IS_RC_MODE_ACTIVE(BOXAUTOTUNE) && ARMING_FLAG(ARMED)) {
        if (!FLIGHT_MODE(AUTO_TUNE)) {
            autotuneStart();
            ENABLE_FLIGHT_MODE(AUTO_TUNE);
        }
        else {
            autotuneCheckUpdateGains();
        }
    } else {
        if (FLIGHT_MODE(AUTO_TUNE)) {
            autotuneUpdateGains(tuneSaved);
        }

        DISABLE_FLIGHT_MODE(AUTO_TUNE);
    }
}

static void blackboxLogAutotuneEvent(adjustmentFunction_e adjustmentFunction, int32_t newValue)
{
#ifndef USE_BLACKBOX
    UNUSED(adjustmentFunction);
    UNUSED(newValue);
#else
    if (feature(FEATURE_BLACKBOX)) {
        flightLogEvent_inflightAdjustment_t eventData;
        eventData.adjustmentFunction = adjustmentFunction;
        eventData.newValue = newValue;
        eventData.floatFlag = false;
        blackboxLogEvent(FLIGHT_LOG_EVENT_INFLIGHT_ADJUSTMENT, (flightLogEventData_t*)&eventData);
    }
#endif
}

#if defined(USE_AUTOTUNE_FIXED_WING)

void autotuneFixedWingUpdate(const flight_dynamics_index_t axis, float desiredRateDps, float reachedRateDps, float pidOutput)
{
    const timeMs_t currentTimeMs = millis();
    const float convergenceRate = pidAutotuneConfig()->fw_convergence_rate / 100.0f;
    float maxDesiredRateDps = tuneCurrent[axis].rate;
    float gainFF = tuneCurrent[axis].gainFF;

    const float absDesiredRateDps = fabsf(desiredRateDps);
    const float absReachedRateDps = fabsf(reachedRateDps);
    const float absPidOutput = fabsf(pidOutput);

    const bool correctDirection = (desiredRateDps>0) == (reachedRateDps>0);
    const float stickInput = absDesiredRateDps / maxDesiredRateDps;

    float pidSumTarget;
    float pidSumLimit;
    float rateFullStick;
    float pidOutputRequired;
    pidAutotuneState_e newState;

    // Use different max desired rate in ANGLE for pitch and roll
    // Maximum reasonable error in ANGLE mode is 200% of angle inclination (control dublet), but we are conservative and tune for control singlet.
    if (FLIGHT_MODE(ANGLE_MODE) && (axis == FD_PITCH || axis == FD_ROLL)) {
        float maxDesiredRateInAngleMode = DECIDEGREES_TO_DEGREES(pidProfile()->max_angle_inclination[axis] * 1.0f) * pidBank()->pid[PID_LEVEL].P / FP_PID_LEVEL_P_MULTIPLIER;
        maxDesiredRateDps = MIN(maxDesiredRateDps, maxDesiredRateInAngleMode);
    }

    if (stickInput < (pidAutotuneConfig()->fw_min_stick / 100.0f) || !correctDirection) {
        // We can make decisions only when we are giving at least 80% stick input and the airplane is rotating in the requested direction
        newState = DEMAND_TOO_LOW;
    }
    else if (absReachedRateDps > absDesiredRateDps) {
        newState = DEMAND_OVERSHOOT;
    }
    else {
        newState = DEMAND_UNDERSHOOT;
    }

    if (newState != tuneCurrent[axis].state) {
        tuneCurrent[axis].state = newState;
        tuneCurrent[axis].stateEnterTime = currentTimeMs;
        tuneCurrent[axis].maxAbsDesiredRateDps = 0;
        tuneCurrent[axis].maxAbsReachedRateDps = 0;
        tuneCurrent[axis].maxAbsPidOutput = 0;
    }

    switch (tuneCurrent[axis].state){
        case DEMAND_TOO_LOW:
            break;
        case DEMAND_UNDERSHOOT:
        case DEMAND_OVERSHOOT:
            {
                const timeDelta_t stateTimeMs = currentTimeMs - tuneCurrent[axis].stateEnterTime;
                bool gainsUpdated = false;
                bool ratesUpdated = false;

                // Record max values
                tuneCurrent[axis].maxAbsDesiredRateDps = MAX(tuneCurrent[axis].maxAbsDesiredRateDps, absDesiredRateDps);
                tuneCurrent[axis].maxAbsReachedRateDps = MAX(tuneCurrent[axis].maxAbsReachedRateDps, absReachedRateDps);
                tuneCurrent[axis].maxAbsPidOutput = MAX(tuneCurrent[axis].maxAbsPidOutput, absPidOutput);

                if (stateTimeMs >= pidAutotuneConfig()->fw_detect_time) {
                    if (pidAutotuneConfig()->fw_rate_adjustment != FIXED && !FLIGHT_MODE(ANGLE_MODE)) {
                        // Tuning the rates is not compatible with ANGLE mode

                        // Target 80% control surface deflection to leave some room for P and I to work
                        pidSumLimit = (axis == FD_YAW) ? pidProfile()->pidSumLimitYaw : pidProfile()->pidSumLimit;
                        pidSumTarget = (pidAutotuneConfig()->fw_max_rate_deflection / 100.0f) * pidSumLimit;

                        // Theoretically achievable rate with target deflection
                        rateFullStick = (pidSumTarget / tuneCurrent[axis].maxAbsPidOutput) * tuneCurrent[axis].maxAbsReachedRateDps;

                        // Rate update
                        if (rateFullStick > (maxDesiredRateDps + 10.0f)) {
                            maxDesiredRateDps += 10.0f;
                        } else if (rateFullStick < (maxDesiredRateDps - 10.0f)) {
                            maxDesiredRateDps -= 10.0f;
                        }

                        // Constrain to safe values
                        uint16_t minRate = (axis == FD_YAW) ? AUTOTUNE_FIXED_WING_MIN_YAW_RATE : AUTOTUNE_FIXED_WING_MIN_ROLL_PITCH_RATE;
                        uint16_t maxRate = (pidAutotuneConfig()->fw_rate_adjustment == AUTO) ? AUTOTUNE_FIXED_WING_MAX_RATE : MAX(tuneCurrent[axis].initialRate, minRate);
                        tuneCurrent[axis].rate = constrainf(maxDesiredRateDps, minRate, maxRate);
                        ratesUpdated = true;
                    }

                    // Update FF towards value needed to achieve current rate target
                    pidOutputRequired = MIN(tuneCurrent[axis].maxAbsPidOutput * (tuneCurrent[axis].maxAbsDesiredRateDps / tuneCurrent[axis].maxAbsReachedRateDps), pidSumLimit);
                    gainFF += (pidOutputRequired / tuneCurrent[axis].maxAbsDesiredRateDps * FP_PID_RATE_FF_MULTIPLIER - gainFF) * convergenceRate * stickInput;
                    tuneCurrent[axis].gainFF = constrainf(gainFF, AUTOTUNE_FIXED_WING_MIN_FF, AUTOTUNE_FIXED_WING_MAX_FF);
                    gainsUpdated = true;

                    // Reset state flag
                    tuneCurrent[axis].state = DEMAND_TOO_LOW;
                }

                if (gainsUpdated) {
                    // Set P-gain to 10% of FF gain (quite agressive - FIXME)
                    tuneCurrent[axis].gainP = tuneCurrent[axis].gainFF * (pidAutotuneConfig()->fw_ff_to_p_gain / 100.0f);

                    // Set D-gain relative to P-gain (0 for now)
                    tuneCurrent[axis].gainD = tuneCurrent[axis].gainP * (pidAutotuneConfig()->fw_p_to_d_gain / 100.0f);

                    // Set integrator gain to reach the same response as FF gain in 0.667 second
                    tuneCurrent[axis].gainI = (tuneCurrent[axis].gainFF / FP_PID_RATE_FF_MULTIPLIER) * (1000.0f / pidAutotuneConfig()->fw_ff_to_i_time_constant) * FP_PID_RATE_I_MULTIPLIER;
                    tuneCurrent[axis].gainI = constrainf(tuneCurrent[axis].gainI, 2.0f, 50.0f);
                    autotuneUpdateGains(tuneCurrent);

                    switch (axis) {
                        case FD_ROLL:
                            blackboxLogAutotuneEvent(ADJUSTMENT_ROLL_FF, tuneCurrent[axis].gainFF);
                            break;

                        case FD_PITCH:
                            blackboxLogAutotuneEvent(ADJUSTMENT_PITCH_FF, tuneCurrent[axis].gainFF);
                            break;

                        case FD_YAW:
                            blackboxLogAutotuneEvent(ADJUSTMENT_YAW_FF, tuneCurrent[axis].gainFF);
                            break;
                    }
                    
                    // Debug
                    DEBUG_SET(DEBUG_AUTOTUNE, axis + 3, pidOutputRequired);
                }

                if (ratesUpdated) {
                    autotuneUpdateGains(tuneCurrent);

                    switch (axis) {
                        case FD_ROLL:
                            blackboxLogAutotuneEvent(ADJUSTMENT_ROLL_RATE, tuneCurrent[axis].rate);
                            break;

                        case FD_PITCH:
                            blackboxLogAutotuneEvent(ADJUSTMENT_PITCH_RATE, tuneCurrent[axis].rate);
                            break;

                        case FD_YAW:
                            blackboxLogAutotuneEvent(ADJUSTMENT_YAW_RATE, tuneCurrent[axis].rate);
                            break;
                    }

                    // Debug
                    DEBUG_SET(DEBUG_AUTOTUNE, axis, rateFullStick);
                }
            }
    }
}
#endif

#endif
