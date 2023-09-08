/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "platform.h"

#if defined(USE_POWER_LIMITS)

#include "flight/power_limits.h"

#include "build/debug.h"

#include "common/filter.h"
#include "common/maths.h"
#include "common/utils.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#include "drivers/time.h"

#include "fc/settings.h"

#include "rx/rx.h"

#include "sensors/battery.h"

#define LIMITING_THR_FILTER_TCONST 50

PG_REGISTER_WITH_RESET_TEMPLATE(powerLimitsConfig_t, powerLimitsConfig, PG_POWER_LIMITS_CONFIG, 1);

PG_RESET_TEMPLATE(powerLimitsConfig_t, powerLimitsConfig,
    .piP = SETTING_LIMIT_PI_P_DEFAULT,
    .piI = SETTING_LIMIT_PI_I_DEFAULT,
    .attnFilterCutoff = SETTING_LIMIT_ATTN_FILTER_CUTOFF_DEFAULT,   // Hz
);

static float burstCurrentReserve;               // cA.µs
static float burstCurrentReserveMax;            // cA.µs
static float burstCurrentReserveFalldown;       // cA.µs
static int32_t activeCurrentLimit;              // cA
static float currentThrAttnIntegrator = 0;
static pt1Filter_t currentThrAttnFilter;
static pt1Filter_t currentThrLimitingBaseFilter;
static bool wasLimitingCurrent = false;

#ifdef USE_ADC
static float burstPowerReserve;                 // cW.µs
static float burstPowerReserveMax;              // cW.µs
static float burstPowerReserveFalldown;         // cW.µs
static int32_t activePowerLimit;                // cW
static float powerThrAttnIntegrator = 0;
static pt1Filter_t powerThrAttnFilter;
static pt1Filter_t powerThrLimitingBaseFilter;
static bool wasLimitingPower = false;
#endif

void powerLimiterInit(void) {
    if (currentBatteryProfile->powerLimits.burstCurrent < currentBatteryProfile->powerLimits.continuousCurrent) {
        currentBatteryProfileMutable->powerLimits.burstCurrent = currentBatteryProfile->powerLimits.continuousCurrent;
    }

    activeCurrentLimit = currentBatteryProfile->powerLimits.burstCurrent;

    uint16_t currentBurstOverContinuous = currentBatteryProfile->powerLimits.burstCurrent - currentBatteryProfile->powerLimits.continuousCurrent;
    burstCurrentReserve = burstCurrentReserveMax = currentBurstOverContinuous * currentBatteryProfile->powerLimits.burstCurrentTime * 1e6;
    burstCurrentReserveFalldown = currentBurstOverContinuous * currentBatteryProfile->powerLimits.burstCurrentFalldownTime * 1e6;

    pt1FilterInit(&currentThrAttnFilter, powerLimitsConfig()->attnFilterCutoff, 0);
    pt1FilterInitRC(&currentThrLimitingBaseFilter, LIMITING_THR_FILTER_TCONST, 0);

#ifdef USE_ADC
    if (currentBatteryProfile->powerLimits.burstPower < currentBatteryProfile->powerLimits.continuousPower) {
        currentBatteryProfileMutable->powerLimits.burstPower = currentBatteryProfile->powerLimits.continuousPower;
    }

    activePowerLimit = currentBatteryProfile->powerLimits.burstPower;

    uint16_t powerBurstOverContinuous = currentBatteryProfile->powerLimits.burstPower - currentBatteryProfile->powerLimits.continuousPower;
    burstPowerReserve = burstPowerReserveMax = powerBurstOverContinuous * currentBatteryProfile->powerLimits.burstPowerTime * 1e6;
    burstPowerReserveFalldown = powerBurstOverContinuous * currentBatteryProfile->powerLimits.burstPowerFalldownTime * 1e6;

    pt1FilterInit(&powerThrAttnFilter, powerLimitsConfig()->attnFilterCutoff, 0);
    pt1FilterInitRC(&powerThrLimitingBaseFilter, LIMITING_THR_FILTER_TCONST, 0);
#endif
}

static uint32_t calculateActiveLimit(int32_t value, uint32_t continuousLimit, uint32_t burstLimit, float *burstReserve, float burstReserveFalldown, float burstReserveMax, timeDelta_t timeDelta) {
    int32_t continuousDiff = value - continuousLimit * 10;
    float spentReserveChunk = continuousDiff * timeDelta;
    *burstReserve = constrainf(*burstReserve - spentReserveChunk, 0, burstReserveMax);

    if (currentBatteryProfile->powerLimits.burstCurrentFalldownTime) {
        return scaleRangef(MIN(*burstReserve, burstReserveFalldown), 0, burstReserveFalldown, continuousLimit, burstLimit) * 10;
    }

    return (*burstReserve ? burstLimit : continuousLimit) * 10;
}

void currentLimiterUpdate(timeDelta_t timeDelta) {
    activeCurrentLimit = calculateActiveLimit(getAmperage(),
                            currentBatteryProfile->powerLimits.continuousCurrent, currentBatteryProfile->powerLimits.burstCurrent,
                            &burstCurrentReserve, burstCurrentReserveFalldown, burstCurrentReserveMax,
                            timeDelta);
}

#ifdef USE_ADC
void powerLimiterUpdate(timeDelta_t timeDelta) {
    activePowerLimit = calculateActiveLimit(getPower(),
                            currentBatteryProfile->powerLimits.continuousPower, currentBatteryProfile->powerLimits.burstPower,
                            &burstPowerReserve, burstPowerReserveFalldown, burstPowerReserveMax,
                            timeDelta);
}
#endif

void powerLimiterApply(int16_t *throttleCommand) {

#ifdef USE_ADC
    if (!activeCurrentLimit && !activePowerLimit) {
        return;
    }
#else
    if (!activeCurrentLimit) {
        return;
    }
#endif

    static timeUs_t lastCallTimestamp = 0;
    timeUs_t currentTimeUs = micros();
    timeDelta_t callTimeDelta = cmpTimeUs(currentTimeUs, lastCallTimestamp);

    int16_t throttleBase;
    int16_t currentThrottleCommand;
#ifdef USE_ADC
    int16_t powerThrottleCommand;
#endif

    int16_t current = getAmperageSample();
#ifdef USE_ADC
    uint16_t voltage = getVBatSample();
    int32_t power = (int32_t)voltage * current / 100;
#endif

    // Current limiting
    int32_t overCurrent = current - activeCurrentLimit;

    if (lastCallTimestamp) {
        currentThrAttnIntegrator = constrainf(currentThrAttnIntegrator + overCurrent * powerLimitsConfig()->piI * callTimeDelta * 2e-7f, 0, PWM_RANGE_MAX - PWM_RANGE_MIN);
    }

    float currentThrAttnProportional = MAX(0, overCurrent) * powerLimitsConfig()->piP * 1e-3f;

    uint16_t currentThrAttn = lrintf(pt1FilterApply3(&currentThrAttnFilter, currentThrAttnProportional + currentThrAttnIntegrator, callTimeDelta * 1e-6f));

    throttleBase = wasLimitingCurrent ? lrintf(pt1FilterApply3(&currentThrLimitingBaseFilter, *throttleCommand, callTimeDelta * 1e-6f)) : *throttleCommand;
    uint16_t currentThrAttned = MAX(PWM_RANGE_MIN, (int16_t)throttleBase - currentThrAttn);

    if (activeCurrentLimit && currentThrAttned < *throttleCommand) {
        if (!wasLimitingCurrent && getAmperage() >= activeCurrentLimit) {
            pt1FilterReset(&currentThrLimitingBaseFilter, *throttleCommand);
            wasLimitingCurrent = true;
        }

        currentThrottleCommand = currentThrAttned;
    } else {
        wasLimitingCurrent = false;
        pt1FilterReset(&currentThrAttnFilter, 0);

        currentThrottleCommand = *throttleCommand;
    }

#ifdef USE_ADC
    // Power limiting
    int32_t overPower = power - activePowerLimit;

    if (lastCallTimestamp) {
        powerThrAttnIntegrator = constrainf(powerThrAttnIntegrator + overPower * powerLimitsConfig()->piI * callTimeDelta / voltage * 2e-5f, 0, PWM_RANGE_MAX - PWM_RANGE_MIN);
    }

    float powerThrAttnProportional = MAX(0, overPower) * powerLimitsConfig()->piP / voltage * 1e-1f;

    uint16_t powerThrAttn = lrintf(pt1FilterApply3(&powerThrAttnFilter, powerThrAttnProportional + powerThrAttnIntegrator, callTimeDelta * 1e-6f));

    throttleBase = wasLimitingPower ? lrintf(pt1FilterApply3(&powerThrLimitingBaseFilter, *throttleCommand, callTimeDelta * 1e-6)) : *throttleCommand;
    uint16_t powerThrAttned = MAX(PWM_RANGE_MIN, (int16_t)throttleBase - powerThrAttn);

    if (activePowerLimit && powerThrAttned < *throttleCommand) {
        if (!wasLimitingPower && getPower() >= activePowerLimit) {
            pt1FilterReset(&powerThrLimitingBaseFilter, *throttleCommand);
            wasLimitingPower = true;
        }

        powerThrottleCommand = powerThrAttned;
    } else {
        wasLimitingPower = false;
        pt1FilterReset(&powerThrAttnFilter, 0);

        powerThrottleCommand = *throttleCommand;
    }

    *throttleCommand = MIN(currentThrottleCommand, powerThrottleCommand);
#else
    *throttleCommand = currentThrottleCommand;
#endif

    lastCallTimestamp = currentTimeUs;
}

bool powerLimiterIsLimiting(void) {
#ifdef USE_ADC
    return wasLimitingPower || wasLimitingCurrent;
#else
    return wasLimitingCurrent;
#endif
}

bool powerLimiterIsLimitingCurrent(void) {
    return wasLimitingCurrent;
}

#ifdef USE_ADC
bool powerLimiterIsLimitingPower(void) {
    return wasLimitingPower;
}
#endif

// returns seconds
float powerLimiterGetRemainingBurstTime(void) {
    uint16_t currentBurstOverContinuous = currentBatteryProfile->powerLimits.burstCurrent - currentBatteryProfile->powerLimits.continuousCurrent;
    float remainingCurrentBurstTime = burstCurrentReserve / currentBurstOverContinuous / 1e7f;

#ifdef USE_ADC
    uint16_t powerBurstOverContinuous = currentBatteryProfile->powerLimits.burstPower - currentBatteryProfile->powerLimits.continuousPower;
    float remainingPowerBurstTime = burstPowerReserve / powerBurstOverContinuous / 1e7f;

    if (!currentBatteryProfile->powerLimits.continuousCurrent) {
        return remainingPowerBurstTime;
    }

    if (!currentBatteryProfile->powerLimits.continuousPower) {
        return remainingCurrentBurstTime;
    }

    return MIN(remainingCurrentBurstTime, remainingPowerBurstTime);
#else
    return remainingCurrentBurstTime;
#endif
}

// returns cA
uint16_t powerLimiterGetActiveCurrentLimit(void) {
    return activeCurrentLimit;
}

#ifdef USE_ADC
// returns cW
uint16_t powerLimiterGetActivePowerLimit(void) {
    return activePowerLimit;
}
#endif

#endif
