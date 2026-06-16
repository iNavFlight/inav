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

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"

#include "flight/imu.h"
#include "flight/mixer.h"
#include "flight/mixer_profile.h"

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"
#include "navigation/navigation_vtol_mc_protection.h"

#ifdef USE_AUTO_TRANSITION

typedef enum {
    VTOL_MC_PROTECT_FLAG_CONFIGURED          = 1 << 0,
    VTOL_MC_PROTECT_FLAG_VTOL_MC            = 1 << 1,
    VTOL_MC_PROTECT_FLAG_NAV_ACTIVE         = 1 << 2,
    VTOL_MC_PROTECT_FLAG_STABILIZED_ACTIVE  = 1 << 3,
    VTOL_MC_PROTECT_FLAG_CAPTURE_ACTIVE     = 1 << 4,
    VTOL_MC_PROTECT_FLAG_LANDING_SETTLE     = 1 << 5,
    VTOL_MC_PROTECT_FLAG_BAILOUT_ACTIVE     = 1 << 6,
    VTOL_MC_PROTECT_FLAG_RESERVE_SHRUNK     = 1 << 7,
    VTOL_MC_PROTECT_FLAG_SOFT_ALTITUDE      = 1 << 8,
    VTOL_MC_PROTECT_FLAG_COMMAND_SHAPED     = 1 << 9,
    VTOL_MC_PROTECT_FLAG_VELOCITY_FALLBACK  = 1 << 10,
} vtolMcProtectionDebugFlags_e;

typedef struct vtolMcProtectionRuntimeState_s {
    vtolMcProtectionSettleState_t captureSettle;
    vtolMcProtectionSettleState_t landingSettle;
    vtolMcProtectionSettleState_t bailoutSettle;
    bool captureActive;
    bool softAltitudeActive;
    bool landingSettleActive;
    bool bailoutActive;
    bool reserveShrunk;
    bool stabilizedCommandShaped;
    bool velocityFallbackActive;
    timeMs_t bailoutStartMs;
    int16_t bailoutStartThrottle;
    uint16_t commandScalePermille;
    int16_t safeThrottleMin;
    int16_t safeThrottleMax;
    int16_t protectedThrottle;
} vtolMcProtectionRuntimeState_t;

static vtolMcProtectionRuntimeState_t vtolMcProtection = {
    .commandScalePermille = 1000,
};

static bool navigationVtolMcProtectionConfigured(void)
{
    return (vtolMcProtectionMode_e)systemConfig()->vtolMcProtectionMode != VTOL_MC_PROTECTION_OFF;
}

static bool navigationVtolMcProtectionPairedAirplaneProfileConfigured(void)
{
    for (int i = 0; i < MAX_MIXER_PROFILE_COUNT; i++) {
        if (i != currentMixerProfileIndex && mixerConfigByIndex(i)->platformType == PLATFORM_AIRPLANE) {
            return true;
        }
    }

    return platformTypeConfigured(PLATFORM_AIRPLANE);
}

bool navigationVtolMcProtectionIsVtolMcMode(void)
{
    return vtolMcProtectionDetectVtolMcMode(
        STATE(MULTIROTOR),
        isMultirotorTypePlatform(currentMixerConfig.platformType),
        navigationVtolMcProtectionPairedAirplaneProfileConfigured());
}

bool navigationVtolMcProtectionIsNavActive(void)
{
    return navigationVtolMcProtectionConfigured() &&
           ARMING_FLAG(ARMED) &&
           navigationVtolMcProtectionIsVtolMcMode();
}

bool navigationVtolMcProtectionVelocityUsable(void)
{
    return posControl.flags.estVelStatus == EST_TRUSTED;
}

static bool navigationVtolMcProtectionVerticalVelocityUsable(void)
{
    return posControl.flags.estAltStatus >= EST_USABLE;
}

uint16_t navigationVtolMcProtectionMaxAbsAttitudeDeciDeg(void)
{
    const uint16_t absRoll = ABS(attitude.values.roll);
    const uint16_t absPitch = ABS(attitude.values.pitch);
    return absRoll > absPitch ? absRoll : absPitch;
}

static uint16_t navigationVtolMcProtectionHorizontalSettleLimitCmS(void)
{
#ifdef USE_MR_BRAKING_MODE
    return vtolMcProtectionHorizontalSettleSpeedCmS(navConfig()->mc.braking_disengage_speed);
#else
    return vtolMcProtectionHorizontalSettleSpeedCmS(0);
#endif
}

static uint16_t navigationVtolMcProtectionVerticalSettleLimitCmS(void)
{
    return vtolMcProtectionVerticalSettleSpeedCmS(navConfig()->general.land_minalt_vspd);
}

static uint16_t navigationVtolMcProtectionAttitudeSettleLimitDeciDeg(void)
{
    return vtolMcProtectionSettleAttitudeLimitDeciDeg(navConfig()->mc.max_bank_angle);
}

static uint16_t navigationVtolMcProtectionBailoutLimitDeciDeg(void)
{
    return vtolMcProtectionBailoutAngleLimitDeciDeg(navConfig()->mc.max_bank_angle);
}

static bool navigationVtolMcProtectionSettleConditionsMet(uint16_t *settleTimeMs)
{
    const bool horizontalVelocityUsable = navigationVtolMcProtectionVelocityUsable();
    const bool verticalVelocityUsable = navigationVtolMcProtectionVerticalVelocityUsable();

    vtolMcProtection.velocityFallbackActive = vtolMcProtectionUsingVelocityFallback(horizontalVelocityUsable, verticalVelocityUsable);
    *settleTimeMs = vtolMcProtectionSettleTimeMs(horizontalVelocityUsable);

    return vtolMcProtectionSettleConditionsMetWithFallback(
        horizontalVelocityUsable,
        verticalVelocityUsable,
        posControl.actualState.velXY,
        navGetCurrentActualPositionAndVelocity()->vel.z,
        navigationVtolMcProtectionMaxAbsAttitudeDeciDeg(),
        navigationVtolMcProtectionHorizontalSettleLimitCmS(),
        navigationVtolMcProtectionVerticalSettleLimitCmS(),
        navigationVtolMcProtectionAttitudeSettleLimitDeciDeg());
}

static void navigationVtolMcProtectionPublishDebug(void)
{
    uint32_t flags = 0;

    if (navigationVtolMcProtectionConfigured()) {
        flags |= VTOL_MC_PROTECT_FLAG_CONFIGURED;
    }
    if (navigationVtolMcProtectionIsVtolMcMode()) {
        flags |= VTOL_MC_PROTECT_FLAG_VTOL_MC;
    }
    if (navigationVtolMcProtectionIsNavActive()) {
        flags |= VTOL_MC_PROTECT_FLAG_NAV_ACTIVE;
    }
    if ((vtolMcProtectionMode_e)systemConfig()->vtolMcProtectionMode == VTOL_MC_PROTECTION_NAV_AND_STABILIZED &&
        ARMING_FLAG(ARMED) && navigationVtolMcProtectionIsVtolMcMode() &&
        (FLIGHT_MODE(ANGLE_MODE) || FLIGHT_MODE(HORIZON_MODE))) {
        flags |= VTOL_MC_PROTECT_FLAG_STABILIZED_ACTIVE;
    }
    if (vtolMcProtection.captureActive) {
        flags |= VTOL_MC_PROTECT_FLAG_CAPTURE_ACTIVE;
    }
    if (vtolMcProtection.landingSettleActive) {
        flags |= VTOL_MC_PROTECT_FLAG_LANDING_SETTLE;
    }
    if (vtolMcProtection.bailoutActive) {
        flags |= VTOL_MC_PROTECT_FLAG_BAILOUT_ACTIVE;
    }
    if (vtolMcProtection.reserveShrunk) {
        flags |= VTOL_MC_PROTECT_FLAG_RESERVE_SHRUNK;
    }
    if (vtolMcProtection.softAltitudeActive) {
        flags |= VTOL_MC_PROTECT_FLAG_SOFT_ALTITUDE;
    }
    if (vtolMcProtection.stabilizedCommandShaped) {
        flags |= VTOL_MC_PROTECT_FLAG_COMMAND_SHAPED;
    }
    if (vtolMcProtection.velocityFallbackActive &&
        (vtolMcProtection.captureActive || vtolMcProtection.landingSettleActive || vtolMcProtection.bailoutActive)) {
        flags |= VTOL_MC_PROTECT_FLAG_VELOCITY_FALLBACK;
    }

    uint16_t debugProgress = 1000;
    if (vtolMcProtection.captureActive) {
        debugProgress = vtolMcProtection.captureSettle.elapsedMs;
    } else if (vtolMcProtection.landingSettleActive) {
        debugProgress = vtolMcProtection.landingSettle.elapsedMs;
    } else if (vtolMcProtection.bailoutActive) {
        debugProgress = vtolMcProtection.bailoutSettle.elapsedMs;
    } else if (vtolMcProtection.stabilizedCommandShaped) {
        debugProgress = vtolMcProtection.commandScalePermille;
    }

    DEBUG_SET(DEBUG_VTOL_MC_PROTECT, 0, (int32_t)flags);
    DEBUG_SET(DEBUG_VTOL_MC_PROTECT, 1, vtolMcProtection.safeThrottleMin);
    DEBUG_SET(DEBUG_VTOL_MC_PROTECT, 2, vtolMcProtection.safeThrottleMax);
    DEBUG_SET(DEBUG_VTOL_MC_PROTECT, 3, vtolMcProtection.protectedThrottle);
    DEBUG_SET(DEBUG_VTOL_MC_PROTECT, 4, (int32_t)lrintf(posControl.actualState.velXY));
    DEBUG_SET(DEBUG_VTOL_MC_PROTECT, 5, (int32_t)lrintf(navGetCurrentActualPositionAndVelocity()->vel.z));
    DEBUG_SET(DEBUG_VTOL_MC_PROTECT, 6, navigationVtolMcProtectionMaxAbsAttitudeDeciDeg());
    DEBUG_SET(DEBUG_VTOL_MC_PROTECT, 7, debugProgress);
}

void navigationVtolMcProtectionResetTransientStates(void)
{
    vtolMcProtection.captureSettle.stableSinceMs = 0;
    vtolMcProtection.captureSettle.elapsedMs = 0;
    vtolMcProtection.landingSettle.stableSinceMs = 0;
    vtolMcProtection.landingSettle.elapsedMs = 0;
    vtolMcProtection.bailoutSettle.stableSinceMs = 0;
    vtolMcProtection.bailoutSettle.elapsedMs = 0;
    vtolMcProtection.captureActive = false;
    vtolMcProtection.softAltitudeActive = false;
    vtolMcProtection.landingSettleActive = false;
    vtolMcProtection.bailoutActive = false;
    vtolMcProtection.reserveShrunk = false;
    vtolMcProtection.stabilizedCommandShaped = false;
    vtolMcProtection.velocityFallbackActive = false;
    vtolMcProtection.commandScalePermille = 1000;
    vtolMcProtection.protectedThrottle = 0;
}

vtolMcProtectionThrottleBounds_t navigationVtolMcProtectionGetThrottleBounds(const int16_t idleThrottle, const int16_t hoverThrottle, const int16_t maxThrottle)
{
    const bool active = navigationVtolMcProtectionIsNavActive() && navigationInAutomaticThrottleMode();
    vtolMcProtectionThrottleBounds_t bounds = vtolMcProtectionComputeThrottleBounds(
        active,
        idleThrottle,
        hoverThrottle,
        maxThrottle,
        systemConfig()->vtolMcThrReservePercent);

    vtolMcProtection.safeThrottleMin = bounds.min;
    vtolMcProtection.safeThrottleMax = bounds.max;
    vtolMcProtection.reserveShrunk = bounds.reserveShrunk;

    if (!active) {
        vtolMcProtection.bailoutActive = false;
        vtolMcProtection.softAltitudeActive = false;
        vtolMcProtection.velocityFallbackActive = false;
    }

    navigationVtolMcProtectionPublishDebug();
    return bounds;
}

static bool navigationVtolMcProtectionBailoutEntryCondition(void)
{
    return navigationVtolMcProtectionIsNavActive() &&
           navigationInAutomaticThrottleMode() &&
           navigationVtolMcProtectionMaxAbsAttitudeDeciDeg() >= navigationVtolMcProtectionBailoutLimitDeciDeg();
}

bool navigationVtolMcProtectionShouldFreezeAltitudeIntegrator(void)
{
    return vtolMcProtection.bailoutActive ||
           vtolMcProtection.softAltitudeActive ||
           navigationVtolMcProtectionBailoutEntryCondition();
}

static int16_t navigationVtolMcProtectionBailoutTargetThrottle(const int16_t requestedThrottle, const vtolMcProtectionThrottleBounds_t *bounds, const int16_t hoverThrottle)
{
    int16_t targetThrottle = constrain(hoverThrottle, bounds->min, bounds->max);
    const float verticalSpeedCmS = navGetCurrentActualPositionAndVelocity()->vel.z;

    if (verticalSpeedCmS < -(float)navigationVtolMcProtectionVerticalSettleLimitCmS() && targetThrottle < requestedThrottle) {
        targetThrottle = requestedThrottle;
    }

    return constrain(targetThrottle, bounds->min, bounds->max);
}

int16_t navigationVtolMcProtectionApplyBailoutThrottle(const int16_t requestedThrottle, const vtolMcProtectionThrottleBounds_t *bounds, const int16_t hoverThrottle)
{
    const bool entryCondition = navigationVtolMcProtectionBailoutEntryCondition();
    const timeMs_t nowMs = millis();

    if (!entryCondition && vtolMcProtection.bailoutActive) {
        uint16_t settleTimeMs;
        const bool settleConditionsMet = navigationVtolMcProtectionSettleConditionsMet(&settleTimeMs);
        const bool readyToExit = vtolMcProtectionUpdateSettleState(
            &vtolMcProtection.bailoutSettle,
            settleConditionsMet,
            settleTimeMs,
            nowMs);

        if (readyToExit) {
            vtolMcProtection.bailoutActive = false;
            vtolMcProtection.bailoutSettle.stableSinceMs = 0;
            vtolMcProtection.bailoutSettle.elapsedMs = 0;
            updateClimbRateToAltitudeController(0, 0, ROC_TO_ALT_CURRENT);
            vtolMcProtection.protectedThrottle = requestedThrottle;
            navigationVtolMcProtectionPublishDebug();
            return requestedThrottle;
        }
    }

    if (!entryCondition && !vtolMcProtection.bailoutActive) {
        vtolMcProtection.protectedThrottle = requestedThrottle;
        navigationVtolMcProtectionPublishDebug();
        return requestedThrottle;
    }

    if (!vtolMcProtection.bailoutActive) {
        vtolMcProtection.bailoutActive = true;
        vtolMcProtection.bailoutStartMs = nowMs;
        vtolMcProtection.bailoutStartThrottle = requestedThrottle;
        vtolMcProtection.bailoutSettle.stableSinceMs = 0;
        vtolMcProtection.bailoutSettle.elapsedMs = 0;
        updateClimbRateToAltitudeController(0, 0, ROC_TO_ALT_CURRENT);
    }

    const int16_t targetThrottle = navigationVtolMcProtectionBailoutTargetThrottle(requestedThrottle, bounds, hoverThrottle);
    const uint16_t rampTimeMs = VTOL_MC_SETTLE_TIME_MS > VTOL_MC_BAILOUT_MIN_RAMP_TIME_MS ?
                                VTOL_MC_SETTLE_TIME_MS :
                                VTOL_MC_BAILOUT_MIN_RAMP_TIME_MS;
    const float progress = constrainf((nowMs - vtolMcProtection.bailoutStartMs) / (float)rampTimeMs, 0.0f, 1.0f);
    const int16_t protectedThrottle = lrintf(vtolMcProtection.bailoutStartThrottle +
                                            (targetThrottle - vtolMcProtection.bailoutStartThrottle) * progress);

    vtolMcProtection.protectedThrottle = constrain(protectedThrottle, bounds->min, bounds->max);
    navigationVtolMcProtectionPublishDebug();
    return vtolMcProtection.protectedThrottle;
}

bool navigationVtolMcProtectionApplyCapture(const uint32_t navStateFlags)
{
    const bool captureAllowed = navigationVtolMcProtectionIsNavActive() &&
                                (navStateFlags & NAV_CTL_POS) &&
                                (navStateFlags & NAV_CTL_HOLD);

    if (!captureAllowed) {
        vtolMcProtection.captureActive = false;
        vtolMcProtection.captureSettle.stableSinceMs = 0;
        vtolMcProtection.captureSettle.elapsedMs = 0;
        navigationVtolMcProtectionPublishDebug();
        return false;
    }

    uint16_t settleTimeMs;
    const bool settleConditionsMet = navigationVtolMcProtectionSettleConditionsMet(&settleTimeMs);
    const bool ready = vtolMcProtectionUpdateSettleState(
        &vtolMcProtection.captureSettle,
        settleConditionsMet,
        settleTimeMs,
        millis());

    vtolMcProtection.captureActive = !ready;
    navigationVtolMcProtectionPublishDebug();
    return vtolMcProtection.captureActive;
}

bool navigationVtolMcProtectionApplySoftAltitudeCapture(const uint32_t navStateFlags)
{
    const bool holdOrTransitionState = ((navStateFlags & NAV_CTL_POS) && (navStateFlags & NAV_CTL_HOLD)) ||
                                       (navStateFlags & NAV_MIXERAT);
    const bool shouldRelaxAltitude = vtolMcProtectionShouldRelaxAltitude(
        navigationVtolMcProtectionIsNavActive(),
        navigationInAutomaticThrottleMode(),
        navigationVtolMcProtectionVelocityUsable(),
        holdOrTransitionState,
        navStateFlags & NAV_CTL_LAND,
        navStateFlags & NAV_CTL_EMERG,
        posControl.flags.isAdjustingAltitude,
        posControl.actualState.velXY,
        navigationVtolMcProtectionHorizontalSettleLimitCmS(),
        vtolMcProtection.captureActive);

    vtolMcProtection.softAltitudeActive = shouldRelaxAltitude;

    if (shouldRelaxAltitude) {
        // While a winged VTOL is bleeding horizontal speed, avoid chasing a stale
        // altitude lock point; stabilize vertical rate and re-lock after settling.
        updateClimbRateToAltitudeController(0, 0, ROC_TO_ALT_CURRENT);
    }

    navigationVtolMcProtectionPublishDebug();
    return shouldRelaxAltitude;
}

bool navigationVtolMcProtectionLandingSettleReady(const fpVector3_t *landingPos)
{
    if (!navigationVtolMcProtectionIsNavActive()) {
        vtolMcProtection.landingSettleActive = false;
        vtolMcProtection.landingSettle.stableSinceMs = 0;
        vtolMcProtection.landingSettle.elapsedMs = 0;
        navigationVtolMcProtectionPublishDebug();
        return true;
    }

    const uint16_t landingCaptureRadiusCm = vtolMcProtectionLandingCaptureRadiusCm(navConfig()->general.waypoint_radius);
    const bool insideLandingRadius = calculateDistanceToDestination(landingPos) <= landingCaptureRadiusCm;
    uint16_t settleTimeMs;
    const bool settleConditionsMet = navigationVtolMcProtectionSettleConditionsMet(&settleTimeMs);
    const bool ready = vtolMcProtectionUpdateSettleState(
        &vtolMcProtection.landingSettle,
        insideLandingRadius && settleConditionsMet,
        settleTimeMs,
        millis());

    vtolMcProtection.landingSettleActive = !ready;
    navigationVtolMcProtectionPublishDebug();
    return ready;
}

void navigationVtolMcProtectionApplyStabilizedCommandShaping(int16_t *rollCommand, int16_t *pitchCommand, int16_t *yawCommand)
{
    vtolMcProtection.stabilizedCommandShaped = false;
    vtolMcProtection.commandScalePermille = 1000;

    if ((vtolMcProtectionMode_e)systemConfig()->vtolMcProtectionMode != VTOL_MC_PROTECTION_NAV_AND_STABILIZED ||
        !ARMING_FLAG(ARMED) ||
        !navigationVtolMcProtectionIsVtolMcMode() ||
        !(FLIGHT_MODE(ANGLE_MODE) || FLIGHT_MODE(HORIZON_MODE)) ||
        !navigationVtolMcProtectionVelocityUsable()) {
        navigationVtolMcProtectionPublishDebug();
        return;
    }

    const float commandScale = vtolMcProtectionCommandScaleForSpeed(posControl.actualState.velXY);
    vtolMcProtection.commandScalePermille = vtolMcProtectionCommandScalePermille(commandScale);

    if (commandScale >= 0.999f) {
        navigationVtolMcProtectionPublishDebug();
        return;
    }

    const int16_t originalRoll = *rollCommand;
    const int16_t originalPitch = *pitchCommand;
    const int16_t originalYaw = *yawCommand;

    *rollCommand = vtolMcProtectionApplyCommandScale(*rollCommand, commandScale);
    *pitchCommand = vtolMcProtectionApplyCommandScale(*pitchCommand, commandScale);
    *yawCommand = vtolMcProtectionApplyCommandScale(*yawCommand, commandScale);

    vtolMcProtection.stabilizedCommandShaped = *rollCommand != originalRoll ||
                                               *pitchCommand != originalPitch ||
                                               *yawCommand != originalYaw;
    navigationVtolMcProtectionPublishDebug();
}

void navigationVtolMcProtectionPublishThrottleDebug(const vtolMcProtectionThrottleBounds_t *bounds, const int16_t protectedThrottle)
{
    vtolMcProtection.safeThrottleMin = bounds->min;
    vtolMcProtection.safeThrottleMax = bounds->max;
    vtolMcProtection.reserveShrunk = bounds->reserveShrunk;
    vtolMcProtection.protectedThrottle = protectedThrottle;
    navigationVtolMcProtectionPublishDebug();
}

#endif
