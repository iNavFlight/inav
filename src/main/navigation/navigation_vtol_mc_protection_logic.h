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

#include <stdbool.h>
#include <stdint.h>

#include "common/time.h"

#define VTOL_MC_LANDING_CAPTURE_RADIUS_CAP_CM      100
#define VTOL_MC_VERTICAL_SETTLE_SPEED_CAP_CM_S     100
#define VTOL_MC_SETTLE_ATTITUDE_CAP_DEG            20
#define VTOL_MC_BAILOUT_ANGLE_EXTRA_DEG            15
#define VTOL_MC_BAILOUT_ANGLE_MIN_DEG              45
#define VTOL_MC_BAILOUT_ANGLE_MAX_DEG              60
#define VTOL_MC_COMMAND_SHAPE_START_CM_S           300
#define VTOL_MC_COMMAND_SHAPE_FULL_CM_S            800
#define VTOL_MC_COMMAND_SHAPE_MIN_SCALE            0.5f
#define VTOL_MC_SETTLE_TIME_MS                     1000
#define VTOL_MC_FALLBACK_SETTLE_TIME_MS            3000
#define VTOL_MC_BAILOUT_MIN_RAMP_TIME_MS           500
#define VTOL_MC_DEFAULT_BRAKING_DISENGAGE_CM_S     75

typedef struct vtolMcProtectionThrottleBounds_s {
    int16_t min;
    int16_t max;
    bool reserveShrunk;
} vtolMcProtectionThrottleBounds_t;

typedef struct vtolMcProtectionSettleState_s {
    timeMs_t stableSinceMs;
    uint16_t elapsedMs;
} vtolMcProtectionSettleState_t;

static inline bool vtolMcProtectionDetectVtolMcMode(
    const bool currentStateIsMultirotor,
    const bool currentPlatformIsMultirotorType,
    const bool pairedAirplaneProfileConfigured)
{
    return currentStateIsMultirotor && currentPlatformIsMultirotorType && pairedAirplaneProfileConfigured;
}

static inline uint16_t vtolMcProtectionLandingCaptureRadiusCm(const uint16_t navWpRadiusCm)
{
    return navWpRadiusCm < VTOL_MC_LANDING_CAPTURE_RADIUS_CAP_CM ? navWpRadiusCm : VTOL_MC_LANDING_CAPTURE_RADIUS_CAP_CM;
}

static inline uint16_t vtolMcProtectionHorizontalSettleSpeedCmS(const uint16_t brakingDisengageSpeedCmS)
{
    return brakingDisengageSpeedCmS > 0 ? brakingDisengageSpeedCmS : VTOL_MC_DEFAULT_BRAKING_DISENGAGE_CM_S;
}

static inline uint16_t vtolMcProtectionVerticalSettleSpeedCmS(const uint16_t landMinAltVspdCmS)
{
    return landMinAltVspdCmS < VTOL_MC_VERTICAL_SETTLE_SPEED_CAP_CM_S ? landMinAltVspdCmS : VTOL_MC_VERTICAL_SETTLE_SPEED_CAP_CM_S;
}

static inline uint16_t vtolMcProtectionSettleAttitudeLimitDeciDeg(const uint8_t navMcBankAngleDeg)
{
    const uint8_t limitedAngleDeg = navMcBankAngleDeg < VTOL_MC_SETTLE_ATTITUDE_CAP_DEG ? navMcBankAngleDeg : VTOL_MC_SETTLE_ATTITUDE_CAP_DEG;
    return (uint16_t)limitedAngleDeg * 10U;
}

static inline uint16_t vtolMcProtectionBailoutAngleLimitDeciDeg(const uint8_t navMcBankAngleDeg)
{
    uint8_t bailoutAngleDeg = navMcBankAngleDeg + VTOL_MC_BAILOUT_ANGLE_EXTRA_DEG;

    if (bailoutAngleDeg < VTOL_MC_BAILOUT_ANGLE_MIN_DEG) {
        bailoutAngleDeg = VTOL_MC_BAILOUT_ANGLE_MIN_DEG;
    } else if (bailoutAngleDeg > VTOL_MC_BAILOUT_ANGLE_MAX_DEG) {
        bailoutAngleDeg = VTOL_MC_BAILOUT_ANGLE_MAX_DEG;
    }

    return (uint16_t)bailoutAngleDeg * 10U;
}

static inline vtolMcProtectionThrottleBounds_t vtolMcProtectionComputeThrottleBounds(
    const bool active,
    const int16_t idleThrottle,
    const int16_t hoverThrottle,
    const int16_t maxThrottle,
    const uint8_t reservePercent)
{
    vtolMcProtectionThrottleBounds_t bounds = {
        .min = idleThrottle,
        .max = maxThrottle,
        .reserveShrunk = false,
    };

    if (!active || reservePercent == 0 || maxThrottle <= idleThrottle) {
        return bounds;
    }

    const int16_t throttleRange = maxThrottle - idleThrottle;
    const int16_t requestedReserve = (throttleRange * reservePercent) / 100;
    const int16_t lowReserveLimit = hoverThrottle > idleThrottle ? hoverThrottle - idleThrottle : 0;
    const int16_t highReserveLimit = maxThrottle > hoverThrottle ? maxThrottle - hoverThrottle : 0;
    const int16_t lowReserve = requestedReserve < lowReserveLimit ? requestedReserve : lowReserveLimit;
    const int16_t highReserve = requestedReserve < highReserveLimit ? requestedReserve : highReserveLimit;

    bounds.min = idleThrottle + lowReserve;
    bounds.max = maxThrottle - highReserve;
    bounds.reserveShrunk = lowReserve != requestedReserve || highReserve != requestedReserve;

    return bounds;
}

static inline bool vtolMcProtectionSettleConditionsMet(
    const float horizontalSpeedCmS,
    const float verticalSpeedCmS,
    const uint16_t maxAbsAttitudeDeciDeg,
    const uint16_t horizontalLimitCmS,
    const uint16_t verticalLimitCmS,
    const uint16_t attitudeLimitDeciDeg)
{
    const float absVerticalSpeedCmS = verticalSpeedCmS < 0.0f ? -verticalSpeedCmS : verticalSpeedCmS;

    return horizontalSpeedCmS <= horizontalLimitCmS &&
           absVerticalSpeedCmS <= verticalLimitCmS &&
           maxAbsAttitudeDeciDeg <= attitudeLimitDeciDeg;
}

static inline bool vtolMcProtectionUsingVelocityFallback(
    const bool horizontalVelocityUsable,
    const bool verticalVelocityUsable)
{
    return !horizontalVelocityUsable && verticalVelocityUsable;
}

static inline uint16_t vtolMcProtectionSettleTimeMs(const bool horizontalVelocityUsable)
{
    return horizontalVelocityUsable ? VTOL_MC_SETTLE_TIME_MS : VTOL_MC_FALLBACK_SETTLE_TIME_MS;
}

static inline bool vtolMcProtectionSettleConditionsMetWithFallback(
    const bool horizontalVelocityUsable,
    const bool verticalVelocityUsable,
    const float horizontalSpeedCmS,
    const float verticalSpeedCmS,
    const uint16_t maxAbsAttitudeDeciDeg,
    const uint16_t horizontalLimitCmS,
    const uint16_t verticalLimitCmS,
    const uint16_t attitudeLimitDeciDeg)
{
    if (!verticalVelocityUsable) {
        return false;
    }

    if (horizontalVelocityUsable) {
        return vtolMcProtectionSettleConditionsMet(
            horizontalSpeedCmS,
            verticalSpeedCmS,
            maxAbsAttitudeDeciDeg,
            horizontalLimitCmS,
            verticalLimitCmS,
            attitudeLimitDeciDeg);
    }

    const float absVerticalSpeedCmS = verticalSpeedCmS < 0.0f ? -verticalSpeedCmS : verticalSpeedCmS;
    return absVerticalSpeedCmS <= verticalLimitCmS &&
           maxAbsAttitudeDeciDeg <= attitudeLimitDeciDeg;
}

static inline bool vtolMcProtectionUpdateSettleState(
    vtolMcProtectionSettleState_t *state,
    const bool conditionsMet,
    const uint16_t settleTimeMs,
    const timeMs_t nowMs)
{
    if (!conditionsMet) {
        state->stableSinceMs = 0;
        state->elapsedMs = 0;
        return false;
    }

    if (settleTimeMs == 0) {
        state->stableSinceMs = nowMs;
        state->elapsedMs = 0;
        return true;
    }

    if (state->stableSinceMs == 0) {
        state->stableSinceMs = nowMs;
        state->elapsedMs = 0;
        return false;
    }

    const timeMs_t elapsedMs = nowMs - state->stableSinceMs;
    state->elapsedMs = elapsedMs > UINT16_MAX ? UINT16_MAX : elapsedMs;
    return elapsedMs >= settleTimeMs;
}

static inline bool vtolMcProtectionShouldRelaxAltitude(
    const bool active,
    const bool automaticThrottle,
    const bool velocityUsable,
    const bool holdOrTransitionState,
    const bool landingState,
    const bool emergencyState,
    const bool altitudeStickActive,
    const float horizontalSpeedCmS,
    const uint16_t horizontalLimitCmS,
    const bool captureActive)
{
    return active &&
           automaticThrottle &&
           holdOrTransitionState &&
           !landingState &&
           !emergencyState &&
           !altitudeStickActive &&
           (captureActive || (velocityUsable && horizontalSpeedCmS > horizontalLimitCmS));
}

static inline float vtolMcProtectionCommandScaleForSpeed(const float horizontalSpeedCmS)
{
    if (horizontalSpeedCmS <= VTOL_MC_COMMAND_SHAPE_START_CM_S) {
        return 1.0f;
    }

    if (horizontalSpeedCmS >= VTOL_MC_COMMAND_SHAPE_FULL_CM_S) {
        return VTOL_MC_COMMAND_SHAPE_MIN_SCALE;
    }

    const float progress = (horizontalSpeedCmS - VTOL_MC_COMMAND_SHAPE_START_CM_S) /
                           (float)(VTOL_MC_COMMAND_SHAPE_FULL_CM_S - VTOL_MC_COMMAND_SHAPE_START_CM_S);
    return 1.0f - progress * (1.0f - VTOL_MC_COMMAND_SHAPE_MIN_SCALE);
}

static inline uint16_t vtolMcProtectionCommandScalePermille(const float commandScale)
{
    return (uint16_t)(commandScale * 1000.0f + 0.5f);
}

static inline int16_t vtolMcProtectionApplyCommandScale(const int16_t command, const float commandScale)
{
    if (command == 0 || commandScale >= 0.999f) {
        return command;
    }

    int16_t shapedCommand = (int16_t)(command * commandScale + (command > 0 ? 0.5f : -0.5f));
    if (shapedCommand == 0) {
        shapedCommand = command > 0 ? 1 : -1;
    }

    return shapedCommand;
}
