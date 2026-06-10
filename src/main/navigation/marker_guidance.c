#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#include "common/maths.h"

#include "drivers/time.h"

#include "fc/fc_core.h"
#include "fc/runtime_config.h"

#include "navigation/marker_guidance.h"

#include "sensors/sensors.h"

#ifdef USE_MARKER_GUIDANCE

typedef enum {
    MARKER_GUIDANCE_CONTEXT_NONE = 0,
    MARKER_GUIDANCE_CONTEXT_POSHOLD,
    MARKER_GUIDANCE_CONTEXT_LAND,
} markerGuidanceContext_e;

typedef struct {
    int16_t offsetForwardCm;
    int16_t offsetRightCm;
    timeMs_t lastUpdateMs;
} markerGuidanceTargetCache_t;

typedef struct {
    markerGuidanceState_e state;
    markerGuidanceTargetCache_t target;
    markerGuidanceContext_e activeContext;
    timeMs_t stateDeadlineMs;
    uint8_t retryCount;
    bool targetAcquiredInContext;
} markerGuidanceRuntime_t;

static markerGuidanceRuntime_t markerGuidance;

static navMarkerGuidanceMode_e markerGuidanceMode(void)
{
    return (navMarkerGuidanceMode_e)navConfig()->general.marker_guidance_mode;
}

static bool markerGuidanceFeatureEnabled(void)
{
    return navConfig()->general.marker_guidance_source == NAV_MARKER_GUIDANCE_SOURCE_MSP &&
           markerGuidanceMode() != NAV_MARKER_GUIDANCE_MODE_OFF;
}

static bool markerGuidanceIsContainmentMode(void)
{
    return markerGuidanceMode() == NAV_MARKER_GUIDANCE_MODE_CONTAINMENT;
}

static bool markerGuidanceIsPlMode(void)
{
    return markerGuidanceMode() == NAV_MARKER_GUIDANCE_MODE_PL;
}

static bool isMcHoverCapableProfileActive(void)
{
    return STATE(MULTIROTOR) && !STATE(AIRPLANE);
}

static bool markerGuidanceIsPosholdContext(navigationFSMStateFlags_t navStateFlags)
{
    return (posControl.navState == NAV_STATE_POSHOLD_3D_IN_PROGRESS) ||
           ((navStateFlags & NAV_CTL_POS) && !(navStateFlags & NAV_CTL_LAND) && FLIGHT_MODE(NAV_POSHOLD_MODE));
}

static bool markerGuidanceIsLandContext(navigationFSMStateFlags_t navStateFlags)
{
    return (navStateFlags & NAV_CTL_LAND) && !STATE(AIRPLANE);
}

static bool markerGuidanceManualTakeoverActive(void)
{
    return posControl.flags.isAdjustingAltitude ||
           posControl.flags.isAdjustingPosition ||
           posControl.flags.isAdjustingHeading;
}

static markerGuidanceContext_e markerGuidanceSelectContext(navigationFSMStateFlags_t navStateFlags)
{
    if (!isMcHoverCapableProfileActive()) {
        return MARKER_GUIDANCE_CONTEXT_NONE;
    }

    switch (markerGuidanceMode()) {
    case NAV_MARKER_GUIDANCE_MODE_PL:
        if (markerGuidanceIsLandContext(navStateFlags)) {
            return MARKER_GUIDANCE_CONTEXT_LAND;
        }
        if (markerGuidanceIsPosholdContext(navStateFlags)) {
            return MARKER_GUIDANCE_CONTEXT_POSHOLD;
        }
        break;

    case NAV_MARKER_GUIDANCE_MODE_CONTAINMENT:
        if (markerGuidanceIsPosholdContext(navStateFlags)) {
            return MARKER_GUIDANCE_CONTEXT_POSHOLD;
        }
        break;

    default:
        break;
    }

    return MARKER_GUIDANCE_CONTEXT_NONE;
}

static void setMarkerGuidanceState(markerGuidanceState_e state)
{
    markerGuidance.state = state;
}

static void clearTargetCache(void)
{
    markerGuidance.target.offsetForwardCm = 0;
    markerGuidance.target.offsetRightCm = 0;
    markerGuidance.target.lastUpdateMs = 0;
}

static void clearContextRuntime(void)
{
    markerGuidance.activeContext = MARKER_GUIDANCE_CONTEXT_NONE;
    markerGuidance.stateDeadlineMs = 0;
    markerGuidance.retryCount = 0;
    markerGuidance.targetAcquiredInContext = false;
}

static void updateContextRuntime(markerGuidanceContext_e newContext)
{
    if (markerGuidance.activeContext == newContext) {
        return;
    }

    markerGuidance.activeContext = newContext;
    markerGuidance.stateDeadlineMs = 0;
    markerGuidance.retryCount = 0;
    markerGuidance.targetAcquiredInContext = false;
}

static float markerGuidanceTargetOffsetMagnitudeCm(void)
{
    return calc_length_pythagorean_2D((float)markerGuidance.target.offsetForwardCm, (float)markerGuidance.target.offsetRightCm);
}

static bool markerGuidanceTargetIsFresh(timeMs_t nowMs, markerGuidanceReason_e *reasonOut)
{
    if (markerGuidance.target.lastUpdateMs == 0) {
        if (reasonOut) {
            *reasonOut = MARKER_GUIDANCE_REASON_INVALID_TARGET;
        }
        return false;
    }

    if (navConfig()->general.marker_guidance_max_target_age_ms > 0 &&
        (nowMs - markerGuidance.target.lastUpdateMs) > navConfig()->general.marker_guidance_max_target_age_ms) {
        if (reasonOut) {
            *reasonOut = MARKER_GUIDANCE_REASON_STALE;
        }
        return false;
    }

    if (navConfig()->general.marker_guidance_max_offset_cm > 0 &&
        markerGuidanceTargetOffsetMagnitudeCm() > navConfig()->general.marker_guidance_max_offset_cm) {
        if (reasonOut) {
            *reasonOut = MARKER_GUIDANCE_REASON_OFFSET_TOO_LARGE;
        }
        return false;
    }

    if (reasonOut) {
        *reasonOut = MARKER_GUIDANCE_REASON_OK;
    }
    return true;
}

static void markerGuidanceComputeMarkerErrorNeu(float desiredVehicleRelN, float desiredVehicleRelE, float *errorNOut, float *errorEOut)
{
    const float offsetForward = markerGuidance.target.offsetForwardCm;
    const float offsetRight = markerGuidance.target.offsetRightCm;

    const float cosYaw = posControl.actualState.cosYaw;
    const float sinYaw = posControl.actualState.sinYaw;

    // Marker offset in local NE from the vehicle perspective.
    const float markerOffsetN = offsetForward * cosYaw - offsetRight * sinYaw;
    const float markerOffsetE = offsetForward * sinYaw + offsetRight * cosYaw;

    // error = desired(vehicle rel marker) - actual(vehicle rel marker)
    // actual(vehicle rel marker) = -markerOffsetNE
    *errorNOut = markerOffsetN + desiredVehicleRelN;
    *errorEOut = markerOffsetE + desiredVehicleRelE;
}

static bool markerGuidanceComputeCorrectionVelocity(markerGuidanceContext_e context, float *velNOut, float *velEOut)
{
    if (!velNOut || !velEOut || context == MARKER_GUIDANCE_CONTEXT_NONE) {
        return false;
    }

    *velNOut = 0.0f;
    *velEOut = 0.0f;

    const bool containmentMode = markerGuidanceIsContainmentMode() && (context == MARKER_GUIDANCE_CONTEXT_POSHOLD);

    const float desiredRelN = containmentMode ? navConfig()->general.marker_containment_hold_north_cm : 0.0f;
    const float desiredRelE = containmentMode ? navConfig()->general.marker_containment_hold_east_cm : 0.0f;

    float errorN = 0.0f;
    float errorE = 0.0f;
    markerGuidanceComputeMarkerErrorNeu(desiredRelN, desiredRelE, &errorN, &errorE);

    const float errorMagnitude = calc_length_pythagorean_2D(errorN, errorE);
    if (errorMagnitude <= 0.0f) {
        return false;
    }

    const float radiusCm = navConfig()->general.marker_guidance_radius_cm;
    if (radiusCm > 0.0f) {
        if (errorMagnitude <= radiusCm) {
            return false;
        }

        // Shared radius behavior:
        // PL mode: center-alignment deadband around marker center.
        // CONTAINMENT mode: allowed boundary around configured hold point.
        const float scale = (errorMagnitude - radiusCm) / errorMagnitude;
        errorN *= scale;
        errorE *= scale;
    }

    const float maxCorrectionSpeed = getActiveSpeed();
    const float speed = calc_length_pythagorean_2D(errorN, errorE);

    if (maxCorrectionSpeed > 0.0f && speed > maxCorrectionSpeed) {
        const float scale = maxCorrectionSpeed / speed;
        errorN *= scale;
        errorE *= scale;
    }

    *velNOut = errorN;
    *velEOut = errorE;

    return true;
}

static uint16_t getRetryTimeoutMs(void)
{
    const uint16_t configuredRetryTimeoutMs = navConfig()->general.marker_guidance_retry_timeout_ms;
    if (configuredRetryTimeoutMs > 0) {
        return MAX(configuredRetryTimeoutMs, 100);
    }

    const uint16_t lostHoldMs = MAX(navConfig()->general.marker_guidance_lost_hold_time_ms, 100);
    const uint32_t autoTimeoutMs = (uint32_t)lostHoldMs * 2U;
    return (uint16_t)MAX(MIN(autoTimeoutMs, 60000U), 100U);
}

static float getRetryClimbRateCmS(void)
{
    const uint16_t retryTimeoutMs = getRetryTimeoutMs();
    const float retryTimeoutS = retryTimeoutMs / 1000.0f;
    const float climbRate = navConfig()->general.marker_guidance_retry_altitude_cm / retryTimeoutS;
    return constrainf(climbRate, 20.0f, navConfig()->mc.max_auto_climb_rate);
}

static bool markerGuidanceRetrySuppressedByAltitude(void)
{
    const uint16_t retryMinAltitudeCm = navConfig()->general.marker_guidance_retry_min_alt_cm;
    return retryMinAltitudeCm > 0 &&
           posControl.flags.estAglStatus >= EST_USABLE &&
           posControl.actualState.agl.pos.z >= 0.0f &&
           posControl.actualState.agl.pos.z <= retryMinAltitudeCm;
}

static void setLowAltitudeFallbackNormalLandState(void)
{
    if (markerGuidance.state != MARKER_GUIDANCE_FALLBACK_NORMAL_LAND &&
        navConfig()->general.marker_guidance_low_alt_lock_xy) {
        setDesiredPosition(&navGetCurrentActualPositionAndVelocity()->pos, 0, NAV_POS_UPDATE_XY);
    }

    setMarkerGuidanceState(MARKER_GUIDANCE_FALLBACK_NORMAL_LAND);
}

static void setLostHoldState(timeMs_t nowMs)
{
    setMarkerGuidanceState(MARKER_GUIDANCE_TARGET_LOST_HOLD);
    markerGuidance.stateDeadlineMs = nowMs + MAX(navConfig()->general.marker_guidance_lost_hold_time_ms, 100U);
}

static void setClimbRetryState(timeMs_t nowMs)
{
    setMarkerGuidanceState(MARKER_GUIDANCE_CLIMB_AND_RETRY);
    markerGuidance.stateDeadlineMs = nowMs + getRetryTimeoutMs();
}

void markerGuidanceReset(void)
{
    clearTargetCache();
    clearContextRuntime();
    setMarkerGuidanceState(MARKER_GUIDANCE_IDLE);
}

void markerGuidanceUpdate(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    const timeMs_t nowMs = millis();

    if (!markerGuidanceFeatureEnabled()) {
        clearContextRuntime();
        setMarkerGuidanceState(MARKER_GUIDANCE_IDLE);
        return;
    }

    if (!ARMING_FLAG(ARMED) || FLIGHT_MODE(FAILSAFE_MODE) || areSensorsCalibrating() || STATE(LANDING_DETECTED)) {
        markerGuidanceReset();
        return;
    }

    markerGuidanceReason_e freshnessReason = MARKER_GUIDANCE_REASON_OK;
    const bool targetFresh = markerGuidanceTargetIsFresh(nowMs, &freshnessReason);

    const bool manualTakeover = markerGuidanceManualTakeoverActive();
    const markerGuidanceContext_e selectedContext = manualTakeover ?
        MARKER_GUIDANCE_CONTEXT_NONE :
        markerGuidanceSelectContext(navStateFlags);

    updateContextRuntime(selectedContext);

    if (selectedContext == MARKER_GUIDANCE_CONTEXT_NONE) {
        setMarkerGuidanceState(targetFresh ? MARKER_GUIDANCE_STANDBY : MARKER_GUIDANCE_IDLE);
        return;
    }

    if (targetFresh) {
        markerGuidance.targetAcquiredInContext = true;

        float correctionVelN = 0.0f;
        float correctionVelE = 0.0f;
        if (markerGuidanceComputeCorrectionVelocity(selectedContext, &correctionVelN, &correctionVelE)) {
            if (selectedContext == MARKER_GUIDANCE_CONTEXT_LAND) {
                setMarkerGuidanceState(MARKER_GUIDANCE_LAND_CORRECTION);
            } else {
                setMarkerGuidanceState(MARKER_GUIDANCE_POSHOLD_CORRECTION);
            }
        } else {
            setMarkerGuidanceState(MARKER_GUIDANCE_STANDBY);
        }
        return;
    }

    if (!markerGuidanceIsPlMode() || selectedContext != MARKER_GUIDANCE_CONTEXT_LAND || !markerGuidance.targetAcquiredInContext) {
        if (markerGuidanceIsContainmentMode() && markerGuidance.target.lastUpdateMs != 0 && freshnessReason == MARKER_GUIDANCE_REASON_STALE) {
            setMarkerGuidanceState(MARKER_GUIDANCE_TARGET_LOST_HOLD);
        } else {
            setMarkerGuidanceState(MARKER_GUIDANCE_STANDBY);
        }
        return;
    }

    if (markerGuidanceRetrySuppressedByAltitude()) {
        setLowAltitudeFallbackNormalLandState();
        return;
    }

    switch (markerGuidance.state) {
    case MARKER_GUIDANCE_TARGET_LOST_HOLD:
        if (nowMs >= markerGuidance.stateDeadlineMs) {
            if (markerGuidance.retryCount < navConfig()->general.marker_guidance_retry_count) {
                setClimbRetryState(nowMs);
            } else {
                setMarkerGuidanceState(MARKER_GUIDANCE_FALLBACK_NORMAL_LAND);
            }
        }
        break;

    case MARKER_GUIDANCE_CLIMB_AND_RETRY:
        if (nowMs >= markerGuidance.stateDeadlineMs) {
            markerGuidance.retryCount++;
            if (markerGuidance.retryCount >= navConfig()->general.marker_guidance_retry_count) {
                setMarkerGuidanceState(MARKER_GUIDANCE_FALLBACK_NORMAL_LAND);
            } else {
                setLostHoldState(nowMs);
            }
        }
        break;

    case MARKER_GUIDANCE_FALLBACK_NORMAL_LAND:
        break;

    default:
        setLostHoldState(nowMs);
        break;
    }
}

void markerGuidanceApplyHorizontalVelocityCorrection(float *velX, float *velY)
{
    if (!velX || !velY) {
        return;
    }

    markerGuidanceContext_e correctionContext = MARKER_GUIDANCE_CONTEXT_NONE;
    if (markerGuidance.state == MARKER_GUIDANCE_LAND_CORRECTION) {
        correctionContext = MARKER_GUIDANCE_CONTEXT_LAND;
    } else if (markerGuidance.state == MARKER_GUIDANCE_POSHOLD_CORRECTION) {
        correctionContext = MARKER_GUIDANCE_CONTEXT_POSHOLD;
    } else {
        return;
    }

    if (!markerGuidanceTargetIsFresh(millis(), NULL)) {
        return;
    }

    float velN = 0.0f;
    float velE = 0.0f;
    if (!markerGuidanceComputeCorrectionVelocity(correctionContext, &velN, &velE)) {
        return;
    }

    *velX += velN;
    *velY += velE;
}

void markerGuidanceGetLandControl(markerGuidanceLandControl_t *controlOut)
{
    if (!controlOut) {
        return;
    }

    controlOut->mode = MARKER_GUIDANCE_LAND_CTRL_NONE;
    controlOut->rateCmS = 0.0f;

    if (!markerGuidanceIsPlMode() || markerGuidance.activeContext != MARKER_GUIDANCE_CONTEXT_LAND) {
        return;
    }

    if (markerGuidance.state == MARKER_GUIDANCE_TARGET_LOST_HOLD) {
        controlOut->mode = MARKER_GUIDANCE_LAND_CTRL_HOLD;
    } else if (markerGuidance.state == MARKER_GUIDANCE_CLIMB_AND_RETRY) {
        controlOut->mode = MARKER_GUIDANCE_LAND_CTRL_CLIMB;
        controlOut->rateCmS = getRetryClimbRateCmS();
    }
}

navSystemStatus_State_e markerGuidanceOverrideNavStatusState(navSystemStatus_State_e defaultState)
{
    switch (markerGuidance.state) {
    case MARKER_GUIDANCE_STANDBY:
        return MW_NAV_STATE_MARKER_GUIDANCE_STANDBY;
    case MARKER_GUIDANCE_POSHOLD_CORRECTION:
        return MW_NAV_STATE_MARKER_GUIDANCE_POSHOLD_CORRECTION;
    case MARKER_GUIDANCE_LAND_CORRECTION:
        return MW_NAV_STATE_MARKER_GUIDANCE_LAND_CORRECTION;
    case MARKER_GUIDANCE_TARGET_LOST_HOLD:
        return MW_NAV_STATE_MARKER_GUIDANCE_TARGET_LOST_HOLD;
    case MARKER_GUIDANCE_CLIMB_AND_RETRY:
        return MW_NAV_STATE_MARKER_GUIDANCE_CLIMB_AND_RETRY;
    case MARKER_GUIDANCE_FALLBACK_NORMAL_LAND:
        return MW_NAV_STATE_MARKER_GUIDANCE_FALLBACK_NORMAL_LAND;
    default:
        return defaultState;
    }
}

bool markerGuidanceHandleMspTargetUpdate(const markerGuidanceTargetUpdate_t *update, markerGuidanceMspResponse_t *responseOut)
{
    if (!update || !responseOut) {
        return false;
    }

    responseOut->accepted = 0;
    responseOut->usedNow = 0;
    responseOut->navGuidanceState = (uint8_t)markerGuidance.state;
    responseOut->reason = MARKER_GUIDANCE_REASON_INVALID_TARGET;
    responseOut->retryCount = markerGuidance.retryCount;

    if (!markerGuidanceFeatureEnabled()) {
        responseOut->accepted = 1;
        responseOut->reason = MARKER_GUIDANCE_REASON_NOT_ENABLED;
        return true;
    }

    const float offsetMagnitude = calc_length_pythagorean_2D((float)update->offsetForwardCm, (float)update->offsetRightCm);
    if (navConfig()->general.marker_guidance_max_offset_cm > 0 &&
        offsetMagnitude > navConfig()->general.marker_guidance_max_offset_cm) {
        responseOut->reason = MARKER_GUIDANCE_REASON_OFFSET_TOO_LARGE;
        return true;
    }

    markerGuidance.target.offsetForwardCm = update->offsetForwardCm;
    markerGuidance.target.offsetRightCm = update->offsetRightCm;
    markerGuidance.target.lastUpdateMs = millis();

    responseOut->accepted = 1;
    responseOut->reason = MARKER_GUIDANCE_REASON_OK;

    const bool mcProfileActive = isMcHoverCapableProfileActive();
    const navigationFSMStateFlags_t navStateFlags = navGetCurrentStateFlags();
    const bool manualTakeover = markerGuidanceManualTakeoverActive();
    const markerGuidanceContext_e selectedContext = manualTakeover ?
        MARKER_GUIDANCE_CONTEXT_NONE :
        markerGuidanceSelectContext(navStateFlags);

    if (!ARMING_FLAG(ARMED)) {
        responseOut->reason = MARKER_GUIDANCE_REASON_NOT_ARMED;
    } else if (FLIGHT_MODE(FAILSAFE_MODE)) {
        responseOut->reason = MARKER_GUIDANCE_REASON_FAILSAFE;
    } else if (!mcProfileActive) {
        responseOut->reason = MARKER_GUIDANCE_REASON_NOT_MC_PROFILE;
    } else if (selectedContext == MARKER_GUIDANCE_CONTEXT_NONE) {
        responseOut->reason = MARKER_GUIDANCE_REASON_NOT_IN_POSHOLD_OR_LAND;
    } else {
        float velN = 0.0f;
        float velE = 0.0f;
        responseOut->usedNow = markerGuidanceComputeCorrectionVelocity(selectedContext, &velN, &velE) ? 1 : 0;
        responseOut->reason = MARKER_GUIDANCE_REASON_OK;
    }

    responseOut->navGuidanceState = (uint8_t)markerGuidance.state;
    responseOut->retryCount = markerGuidance.retryCount;
    return true;
}

#endif
