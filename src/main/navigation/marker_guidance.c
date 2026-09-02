#include <stdbool.h>
#include <stdint.h>
#include <limits.h>

#include "platform.h"

#include "common/maths.h"

#include "build/debug.h"

#include "drivers/time.h"

#include "fc/fc_core.h"
#include "fc/runtime_config.h"

#include "flight/imu.h"

#include "navigation/marker_guidance.h"
#include "navigation/navigation_vtol_mc_protection.h"

#include "sensors/sensors.h"

#ifdef USE_MARKER_GUIDANCE

typedef enum {
    MARKER_GUIDANCE_DEBUG_REASON_OK = 0,
    MARKER_GUIDANCE_DEBUG_REASON_DISABLED,
    MARKER_GUIDANCE_DEBUG_REASON_INVALID_TARGET,
    MARKER_GUIDANCE_DEBUG_REASON_STALE_TARGET,
    MARKER_GUIDANCE_DEBUG_REASON_OFFSET_TOO_LARGE,
    MARKER_GUIDANCE_DEBUG_REASON_DISARMED,
    MARKER_GUIDANCE_DEBUG_REASON_FAILSAFE,
    MARKER_GUIDANCE_DEBUG_REASON_NOT_MC,
    MARKER_GUIDANCE_DEBUG_REASON_NO_CONTEXT,
    MARKER_GUIDANCE_DEBUG_REASON_CALIBRATING,
    MARKER_GUIDANCE_DEBUG_REASON_LANDING_DETECTED,
    MARKER_GUIDANCE_DEBUG_REASON_MANUAL_TAKEOVER,
    MARKER_GUIDANCE_DEBUG_REASON_LOW_ALTITUDE,
    MARKER_GUIDANCE_DEBUG_REASON_LOST_HOLD_TIME,
    MARKER_GUIDANCE_DEBUG_REASON_VELOCITY_UNTRUSTED,
    MARKER_GUIDANCE_DEBUG_REASON_HORIZONTAL_SPEED,
    MARKER_GUIDANCE_DEBUG_REASON_ATTITUDE,
    MARKER_GUIDANCE_DEBUG_REASON_SETTLE_TIME,
    MARKER_GUIDANCE_DEBUG_REASON_RETRY_CLIMB,
    MARKER_GUIDANCE_DEBUG_REASON_FALLBACK,
    MARKER_GUIDANCE_DEBUG_REASON_VTOL_CAPTURE,
    MARKER_GUIDANCE_DEBUG_REASON_VTOL_RECOVERY,
    MARKER_GUIDANCE_DEBUG_REASON_POSITION_UNAVAILABLE,
    MARKER_GUIDANCE_DEBUG_REASON_WAIT_NEW_TARGET,
} markerGuidanceDebugReason_e;

typedef enum {
    MARKER_GUIDANCE_DEBUG_FLAG_ENABLED             = 1U << 0,
    MARKER_GUIDANCE_DEBUG_FLAG_ARMED               = 1U << 1,
    MARKER_GUIDANCE_DEBUG_FLAG_MC_PROFILE          = 1U << 2,
    MARKER_GUIDANCE_DEBUG_FLAG_TARGET_VALID        = 1U << 3,
    MARKER_GUIDANCE_DEBUG_FLAG_TARGET_FRESH        = 1U << 4,
    MARKER_GUIDANCE_DEBUG_FLAG_TARGET_ACQUIRED     = 1U << 5,
    MARKER_GUIDANCE_DEBUG_FLAG_PL_MODE             = 1U << 6,
    MARKER_GUIDANCE_DEBUG_FLAG_CONTAINMENT_MODE    = 1U << 7,
    MARKER_GUIDANCE_DEBUG_FLAG_POSHOLD_CONTEXT     = 1U << 8,
    MARKER_GUIDANCE_DEBUG_FLAG_LAND_CONTEXT        = 1U << 9,
    MARKER_GUIDANCE_DEBUG_FLAG_MANUAL_TAKEOVER     = 1U << 10,
    MARKER_GUIDANCE_DEBUG_FLAG_FAILSAFE            = 1U << 11,
    MARKER_GUIDANCE_DEBUG_FLAG_CALIBRATING         = 1U << 12,
    MARKER_GUIDANCE_DEBUG_FLAG_LANDING_DETECTED    = 1U << 13,
    MARKER_GUIDANCE_DEBUG_FLAG_CORRECTION_APPLIED  = 1U << 14,
    MARKER_GUIDANCE_DEBUG_FLAG_HEADING_APPLIED     = 1U << 15,
    MARKER_GUIDANCE_DEBUG_FLAG_VELOCITY_TRUSTED    = 1U << 16,
    MARKER_GUIDANCE_DEBUG_FLAG_RETRY_SPEED_OK      = 1U << 17,
    MARKER_GUIDANCE_DEBUG_FLAG_RETRY_ATTITUDE_OK   = 1U << 18,
    MARKER_GUIDANCE_DEBUG_FLAG_SETTLE_ACTIVE       = 1U << 19,
    MARKER_GUIDANCE_DEBUG_FLAG_SETTLE_READY        = 1U << 20,
    MARKER_GUIDANCE_DEBUG_FLAG_DEADLINE_REACHED    = 1U << 21,
    MARKER_GUIDANCE_DEBUG_FLAG_LOW_ALT_SUPPRESSED  = 1U << 22,
    MARKER_GUIDANCE_DEBUG_FLAG_LOST_HOLD_XY_ACTIVE = 1U << 23,
    MARKER_GUIDANCE_DEBUG_FLAG_RETRY_ALT_USABLE    = 1U << 24,
    MARKER_GUIDANCE_DEBUG_FLAG_LOW_ALT_XY_LOCK     = 1U << 25,
    MARKER_GUIDANCE_DEBUG_FLAG_POSITION_TRUSTED    = 1U << 26,
    MARKER_GUIDANCE_DEBUG_FLAG_ALTITUDE_USABLE     = 1U << 27,
    MARKER_GUIDANCE_DEBUG_FLAG_AGL_USABLE          = 1U << 28,
    MARKER_GUIDANCE_DEBUG_FLAG_POSITION_TARGET_OWNED = 1U << 29,
    MARKER_GUIDANCE_DEBUG_FLAG_HEADING_LATCHED     = 1U << 30,
    MARKER_GUIDANCE_DEBUG_FLAG_HEADING_SAMPLE_REJECTED = 1U << 31,
} markerGuidanceDebugFlag_e;

typedef struct {
    int16_t offsetForwardCm;
    int16_t offsetRightCm;
    float offsetNorthCm;
    float offsetEastCm;
    float markerPositionNorthCm;
    float markerPositionEastCm;
    int32_t targetHeadingCd;
    uint16_t markerAglCm;
    uint32_t horizontalOffsetSquaredCm;
    timeMs_t lastUpdateMs;
    uint32_t sequence;
    bool valid;
} markerGuidanceTargetCache_t;

typedef struct {
    markerGuidanceState_e state;
    markerGuidanceTargetCache_t target;
    markerGuidanceContext_e activeContext;
    timeMs_t stateDeadlineMs;
    uint8_t retryCount;
    bool targetAcquiredInContext;
    bool headingLatched;
    int32_t latchedHeadingCd;
    uint32_t rejectedHeadingTargetSequence;
    uint32_t rejectedPositionTargetSequence;
    bool lastFreshMarkerWasLow;
    uint32_t lastLandExitTargetSequence;
    markerGuidanceRetrySettleState_t retrySettle;
    float retryStartAltitudeCm;
    float lostHoldPositionNorthCm;
    float lostHoldPositionEastCm;
    float resumePositionNorthCm;
    float resumePositionEastCm;
    float ownedPositionNorthCm;
    float ownedPositionEastCm;
    float requestedCorrectionNorthCm;
    float requestedCorrectionEastCm;
    uint8_t lastMspReason;
    bool retryStartAltitudeUsable;
    bool lostHoldPositionValid;
    bool resumePositionValid;
    bool positionTargetOwned;
    bool headingOverrideApplied;
    bool vtolRecoveryPaused;
    bool recoveryHeadingLatched;
    int32_t recoveryHeadingCd;
} markerGuidanceRuntime_t;

static markerGuidanceRuntime_t markerGuidance;
static void releaseGuidancePositionTarget(void);

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

static bool markerGuidancePositionTakeoverActiveNow(void)
{
    return markerGuidancePositionTakeoverActive(posControl.flags.isAdjustingPosition);
}

static bool markerGuidanceAnyTakeoverActive(void)
{
    return markerGuidancePositionTakeoverActiveNow() ||
           posControl.flags.isAdjustingAltitude ||
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
    markerGuidance.target.offsetNorthCm = 0.0f;
    markerGuidance.target.offsetEastCm = 0.0f;
    markerGuidance.target.markerPositionNorthCm = 0.0f;
    markerGuidance.target.markerPositionEastCm = 0.0f;
    markerGuidance.target.targetHeadingCd = 0;
    markerGuidance.target.markerAglCm = 0;
    markerGuidance.target.horizontalOffsetSquaredCm = 0;
    markerGuidance.target.lastUpdateMs = 0;
    markerGuidance.target.sequence = 0;
    markerGuidance.target.valid = false;
}

static void clearContextRuntime(void)
{
    markerGuidance.activeContext = MARKER_GUIDANCE_CONTEXT_NONE;
    markerGuidance.stateDeadlineMs = 0;
    markerGuidance.retryCount = 0;
    markerGuidance.targetAcquiredInContext = false;
    markerGuidance.headingLatched = false;
    markerGuidance.latchedHeadingCd = 0;
    markerGuidance.rejectedHeadingTargetSequence = 0;
    markerGuidance.rejectedPositionTargetSequence = 0;
    markerGuidance.lastFreshMarkerWasLow = false;
    markerGuidance.lastLandExitTargetSequence = markerGuidance.target.sequence;
    markerGuidanceResetRetrySettle(&markerGuidance.retrySettle);
    markerGuidance.retryStartAltitudeCm = 0.0f;
    markerGuidance.retryStartAltitudeUsable = false;
    markerGuidance.lostHoldPositionNorthCm = 0.0f;
    markerGuidance.lostHoldPositionEastCm = 0.0f;
    markerGuidance.resumePositionNorthCm = 0.0f;
    markerGuidance.resumePositionEastCm = 0.0f;
    markerGuidance.ownedPositionNorthCm = 0.0f;
    markerGuidance.ownedPositionEastCm = 0.0f;
    markerGuidance.lostHoldPositionValid = false;
    markerGuidance.resumePositionValid = false;
    markerGuidance.positionTargetOwned = false;
    markerGuidance.requestedCorrectionNorthCm = 0.0f;
    markerGuidance.requestedCorrectionEastCm = 0.0f;
    markerGuidance.headingOverrideApplied = false;
    markerGuidance.vtolRecoveryPaused = false;
    markerGuidance.recoveryHeadingLatched = false;
    markerGuidance.recoveryHeadingCd = 0;
}

static void updateContextRuntime(markerGuidanceContext_e newContext)
{
    if (markerGuidance.activeContext == newContext) {
        return;
    }

    const bool recoveryWasPaused = markerGuidance.vtolRecoveryPaused;
    releaseGuidancePositionTarget();

    if (markerGuidance.activeContext == MARKER_GUIDANCE_CONTEXT_LAND) {
        // Re-entering LAND must wait for a new pose instead of reviving the previous context's heading latch.
        markerGuidance.lastLandExitTargetSequence = markerGuidance.target.sequence;
    }

    markerGuidance.activeContext = newContext;
    markerGuidance.stateDeadlineMs = 0;
    markerGuidance.retryCount = 0;
    markerGuidance.targetAcquiredInContext = false;
    markerGuidance.headingLatched = false;
    markerGuidance.latchedHeadingCd = 0;
    if (recoveryWasPaused) {
        markerGuidance.rejectedHeadingTargetSequence = markerGuidance.target.sequence;
        markerGuidance.rejectedPositionTargetSequence = markerGuidance.target.sequence;
    }
    markerGuidance.lastFreshMarkerWasLow = false;
    markerGuidanceResetRetrySettle(&markerGuidance.retrySettle);
    markerGuidance.retryStartAltitudeCm = 0.0f;
    markerGuidance.retryStartAltitudeUsable = false;
    markerGuidance.lostHoldPositionNorthCm = 0.0f;
    markerGuidance.lostHoldPositionEastCm = 0.0f;
    markerGuidance.resumePositionNorthCm = 0.0f;
    markerGuidance.resumePositionEastCm = 0.0f;
    markerGuidance.ownedPositionNorthCm = 0.0f;
    markerGuidance.ownedPositionEastCm = 0.0f;
    markerGuidance.lostHoldPositionValid = false;
    markerGuidance.resumePositionValid = false;
    markerGuidance.positionTargetOwned = false;
    markerGuidance.requestedCorrectionNorthCm = 0.0f;
    markerGuidance.requestedCorrectionEastCm = 0.0f;
    markerGuidance.headingOverrideApplied = false;
    markerGuidance.vtolRecoveryPaused = false;
    markerGuidance.recoveryHeadingLatched = false;
    markerGuidance.recoveryHeadingCd = 0;
}

static bool markerGuidanceTargetIsFresh(timeMs_t nowMs, markerGuidanceReason_e *reasonOut)
{
    if (!markerGuidance.target.valid) {
        if (reasonOut) {
            *reasonOut = MARKER_GUIDANCE_REASON_INVALID_TARGET;
        }
        return false;
    }

    if (!markerGuidanceSampleIsFresh(
            true,
            nowMs,
            markerGuidance.target.lastUpdateMs,
            navConfig()->general.marker_guidance_max_target_age_ms)) {
        if (reasonOut) {
            *reasonOut = MARKER_GUIDANCE_REASON_STALE;
        }
        return false;
    }

    const uint16_t maxOffsetCm = navConfig()->general.marker_guidance_max_offset_cm;
    if (maxOffsetCm > 0 &&
        markerGuidance.target.horizontalOffsetSquaredCm > ((uint32_t)maxOffsetCm * maxOffsetCm)) {
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

static bool markerGuidanceComputePositionTarget(markerGuidanceContext_e context, float *targetNOut, float *targetEOut)
{
    if (!targetNOut || !targetEOut || context == MARKER_GUIDANCE_CONTEXT_NONE) {
        return false;
    }

    const bool containmentMode = markerGuidanceIsContainmentMode() && (context == MARKER_GUIDANCE_CONTEXT_POSHOLD);
    const float desiredRelN = containmentMode ? navConfig()->general.marker_containment_hold_north_cm : 0.0f;
    const float desiredRelE = containmentMode ? navConfig()->general.marker_containment_hold_east_cm : 0.0f;
    const fpVector3_t *actualPosition = &navGetCurrentActualPositionAndVelocity()->pos;

    return markerGuidanceComputeHorizontalPositionTarget(
        actualPosition->x,
        actualPosition->y,
        markerGuidance.target.markerPositionNorthCm,
        markerGuidance.target.markerPositionEastCm,
        desiredRelN,
        desiredRelE,
        navConfig()->general.marker_guidance_radius_cm,
        targetNOut,
        targetEOut);
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
    return markerGuidanceRetryIsSuppressedByAltitude(
        retryMinAltitudeCm,
        posControl.flags.estAglStatus >= EST_USABLE,
        posControl.actualState.agl.pos.z,
        markerGuidance.lastFreshMarkerWasLow);
}

static void clearLostHoldPosition(void)
{
    markerGuidance.lostHoldPositionValid = false;
}

static void preserveNavigationPositionTarget(void)
{
    if (markerGuidance.positionTargetOwned) {
        if (posControl.desiredState.pos.x == markerGuidance.ownedPositionNorthCm &&
            posControl.desiredState.pos.y == markerGuidance.ownedPositionEastCm) {
            return;
        }
    } else if (markerGuidance.resumePositionValid) {
        return;
    }

    markerGuidance.resumePositionNorthCm = posControl.desiredState.pos.x;
    markerGuidance.resumePositionEastCm = posControl.desiredState.pos.y;
    markerGuidance.resumePositionValid = true;
}

static void applyGuidancePositionTarget(float targetNorthCm, float targetEastCm)
{
    preserveNavigationPositionTarget();

    markerGuidance.ownedPositionNorthCm = targetNorthCm;
    markerGuidance.ownedPositionEastCm = targetEastCm;
    markerGuidance.positionTargetOwned = true;

    fpVector3_t targetPosition = posControl.desiredState.pos;
    targetPosition.x = targetNorthCm;
    targetPosition.y = targetEastCm;
    setDesiredPosition(&targetPosition, 0, NAV_POS_UPDATE_XY);
}

static void releaseGuidancePositionTarget(void)
{
    if (!markerGuidance.positionTargetOwned && !markerGuidance.resumePositionValid) {
        clearLostHoldPosition();
        return;
    }

    preserveNavigationPositionTarget();

    if (markerGuidance.resumePositionValid) {
        fpVector3_t resumePosition = posControl.desiredState.pos;
        resumePosition.x = markerGuidance.resumePositionNorthCm;
        resumePosition.y = markerGuidance.resumePositionEastCm;
        setDesiredPosition(&resumePosition, 0, NAV_POS_UPDATE_XY);
    }

    clearLostHoldPosition();
    markerGuidance.resumePositionValid = false;
    markerGuidance.positionTargetOwned = false;
    markerGuidance.requestedCorrectionNorthCm = 0.0f;
    markerGuidance.requestedCorrectionEastCm = 0.0f;
}

static void suspendPositionAcquisitionUntilNewSample(void)
{
    releaseGuidancePositionTarget();
    markerGuidance.targetAcquiredInContext = false;
    markerGuidance.rejectedPositionTargetSequence = markerGuidance.target.sequence;
    markerGuidance.stateDeadlineMs = 0;
    markerGuidance.retryCount = 0;
    markerGuidance.lastFreshMarkerWasLow = false;
    markerGuidanceResetRetrySettle(&markerGuidance.retrySettle);
}

static void captureLostHoldPosition(void)
{
    const fpVector3_t *actualPosition = &navGetCurrentActualPositionAndVelocity()->pos;

    markerGuidance.lostHoldPositionNorthCm = actualPosition->x;
    markerGuidance.lostHoldPositionEastCm = actualPosition->y;
    markerGuidance.lostHoldPositionValid = true;
    applyGuidancePositionTarget(actualPosition->x, actualPosition->y);
}

static void applyLostHoldPosition(void)
{
    if (markerGuidance.lostHoldPositionValid) {
        applyGuidancePositionTarget(
            markerGuidance.lostHoldPositionNorthCm,
            markerGuidance.lostHoldPositionEastCm);
    }
}

static void setLowAltitudeFallbackNormalLandState(void)
{
    if (navConfig()->general.marker_guidance_low_alt_lock_xy) {
        if (!markerGuidance.lostHoldPositionValid) {
            captureLostHoldPosition();
        }
        applyLostHoldPosition();
    } else {
        releaseGuidancePositionTarget();
    }

    markerGuidanceResetRetrySettle(&markerGuidance.retrySettle);
    markerGuidance.stateDeadlineMs = 0;
    setMarkerGuidanceState(MARKER_GUIDANCE_FALLBACK_NORMAL_LAND);
}

static void setLostHoldState(timeMs_t nowMs)
{
    markerGuidanceResetRetrySettle(&markerGuidance.retrySettle);
    setMarkerGuidanceState(MARKER_GUIDANCE_TARGET_LOST_HOLD);
    markerGuidance.stateDeadlineMs = nowMs + MAX(navConfig()->general.marker_guidance_lost_hold_time_ms, 100U);
}

static void startLostTargetHold(timeMs_t nowMs)
{
    // Stop marker guidance immediately and let the position controller brake toward the current location.
    captureLostHoldPosition();
    setLostHoldState(nowMs);
}

static void setClimbRetryState(timeMs_t nowMs)
{
    markerGuidanceResetRetrySettle(&markerGuidance.retrySettle);
    markerGuidance.retryStartAltitudeCm = navGetCurrentActualPositionAndVelocity()->pos.z;
    markerGuidance.retryStartAltitudeUsable = posControl.flags.estAltStatus >= EST_USABLE;
    setMarkerGuidanceState(MARKER_GUIDANCE_CLIMB_AND_RETRY);
    markerGuidance.stateDeadlineMs = nowMs + getRetryTimeoutMs();
}

static bool markerGuidanceRetryIsSettled(timeMs_t nowMs)
{
    const uint16_t maxAbsAttitudeDeciDeg = MAX(ABS(attitude.values.roll), ABS(attitude.values.pitch));
    const uint16_t speedLimitCmS = markerGuidanceRetrySettleSpeedLimit(navConfig()->mc.braking_disengage_speed);
    const bool conditionsMet = markerGuidanceRetrySettleConditionsMet(
        posControl.flags.estVelStatus == EST_TRUSTED,
        posControl.actualState.velXY,
        maxAbsAttitudeDeciDeg,
        speedLimitCmS);

    return markerGuidanceUpdateRetrySettle(&markerGuidance.retrySettle, conditionsMet, nowMs);
}

static markerGuidanceDebugReason_e markerGuidanceDebugReason(
    bool targetFresh,
    markerGuidanceReason_e freshnessReason,
    bool velocityTrusted,
    bool retrySpeedOk,
    bool retryAttitudeOk,
    bool settleReady,
    bool deadlineReached,
    bool lowAltitudeSuppressed,
    bool vtolCaptureActive,
    bool vtolRecoveryActive,
    bool positionEstimateUsable,
    bool positionSampleAllowed)
{
    if (!markerGuidanceFeatureEnabled()) {
        return MARKER_GUIDANCE_DEBUG_REASON_DISABLED;
    }
    if (!ARMING_FLAG(ARMED)) {
        return MARKER_GUIDANCE_DEBUG_REASON_DISARMED;
    }
    if (FLIGHT_MODE(FAILSAFE_MODE)) {
        return MARKER_GUIDANCE_DEBUG_REASON_FAILSAFE;
    }
    if (areSensorsCalibrating()) {
        return MARKER_GUIDANCE_DEBUG_REASON_CALIBRATING;
    }
    if (STATE(LANDING_DETECTED)) {
        return MARKER_GUIDANCE_DEBUG_REASON_LANDING_DETECTED;
    }
    if (vtolRecoveryActive) {
        return MARKER_GUIDANCE_DEBUG_REASON_VTOL_RECOVERY;
    }
    if (!positionEstimateUsable) {
        return MARKER_GUIDANCE_DEBUG_REASON_POSITION_UNAVAILABLE;
    }
    if (!markerGuidance.target.valid || freshnessReason == MARKER_GUIDANCE_REASON_INVALID_TARGET) {
        return MARKER_GUIDANCE_DEBUG_REASON_INVALID_TARGET;
    }
    if (!targetFresh) {
        if (freshnessReason == MARKER_GUIDANCE_REASON_OFFSET_TOO_LARGE) {
            return MARKER_GUIDANCE_DEBUG_REASON_OFFSET_TOO_LARGE;
        }

        const bool handlingPlTargetLoss = markerGuidanceIsPlMode() &&
            markerGuidance.targetAcquiredInContext &&
            (markerGuidance.state == MARKER_GUIDANCE_TARGET_LOST_HOLD ||
             markerGuidance.state == MARKER_GUIDANCE_CLIMB_AND_RETRY ||
             markerGuidance.state == MARKER_GUIDANCE_FALLBACK_NORMAL_LAND);
        if (!handlingPlTargetLoss) {
            return MARKER_GUIDANCE_DEBUG_REASON_STALE_TARGET;
        }
    }
    if (!isMcHoverCapableProfileActive()) {
        return MARKER_GUIDANCE_DEBUG_REASON_NOT_MC;
    }
    if (markerGuidance.activeContext == MARKER_GUIDANCE_CONTEXT_NONE) {
        return MARKER_GUIDANCE_DEBUG_REASON_NO_CONTEXT;
    }
    if (markerGuidanceAnyTakeoverActive()) {
        return MARKER_GUIDANCE_DEBUG_REASON_MANUAL_TAKEOVER;
    }
    if (targetFresh && !positionSampleAllowed) {
        return MARKER_GUIDANCE_DEBUG_REASON_WAIT_NEW_TARGET;
    }
    if (vtolCaptureActive) {
        return MARKER_GUIDANCE_DEBUG_REASON_VTOL_CAPTURE;
    }
    if (lowAltitudeSuppressed) {
        return MARKER_GUIDANCE_DEBUG_REASON_LOW_ALTITUDE;
    }
    if (markerGuidance.state == MARKER_GUIDANCE_CLIMB_AND_RETRY) {
        return MARKER_GUIDANCE_DEBUG_REASON_RETRY_CLIMB;
    }
    if (markerGuidance.state == MARKER_GUIDANCE_FALLBACK_NORMAL_LAND) {
        return MARKER_GUIDANCE_DEBUG_REASON_FALLBACK;
    }
    if (markerGuidance.state != MARKER_GUIDANCE_TARGET_LOST_HOLD) {
        return MARKER_GUIDANCE_DEBUG_REASON_OK;
    }
    if (!deadlineReached) {
        return MARKER_GUIDANCE_DEBUG_REASON_LOST_HOLD_TIME;
    }
    if (!velocityTrusted) {
        return MARKER_GUIDANCE_DEBUG_REASON_VELOCITY_UNTRUSTED;
    }
    if (!retrySpeedOk) {
        return MARKER_GUIDANCE_DEBUG_REASON_HORIZONTAL_SPEED;
    }
    if (!retryAttitudeOk) {
        return MARKER_GUIDANCE_DEBUG_REASON_ATTITUDE;
    }
    if (!settleReady) {
        return MARKER_GUIDANCE_DEBUG_REASON_SETTLE_TIME;
    }

    return MARKER_GUIDANCE_DEBUG_REASON_OK;
}

static uint32_t markerGuidancePackSigned16(int32_t low, int32_t high)
{
    const uint16_t lowPacked = (uint16_t)(int16_t)constrain(low, INT16_MIN, INT16_MAX);
    const uint16_t highPacked = (uint16_t)(int16_t)constrain(high, INT16_MIN, INT16_MAX);
    return (uint32_t)lowPacked | ((uint32_t)highPacked << 16);
}

void markerGuidanceReset(void)
{
    releaseGuidancePositionTarget();
    clearTargetCache();
    clearContextRuntime();
    setMarkerGuidanceState(MARKER_GUIDANCE_IDLE);
}

void markerGuidanceUpdate(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    const timeMs_t nowMs = millis();
    markerGuidance.requestedCorrectionNorthCm = 0.0f;
    markerGuidance.requestedCorrectionEastCm = 0.0f;
    markerGuidance.headingOverrideApplied = false;

    if (!markerGuidanceFeatureEnabled()) {
        markerGuidanceReset();
        return;
    }

    if (!ARMING_FLAG(ARMED) || FLIGHT_MODE(FAILSAFE_MODE) || areSensorsCalibrating() || STATE(LANDING_DETECTED)) {
        markerGuidanceReset();
        return;
    }

    if (!isMcHoverCapableProfileActive()) {
        releaseGuidancePositionTarget();
        clearContextRuntime();
        // A body-frame pose observed in FW must not become an MC guidance
        // target after a profile switch without a new post-switch observation.
        markerGuidance.rejectedPositionTargetSequence = markerGuidance.target.sequence;
        markerGuidance.rejectedHeadingTargetSequence = markerGuidance.target.sequence;
        setMarkerGuidanceState(MARKER_GUIDANCE_STANDBY);
        return;
    }

    markerGuidanceReason_e freshnessReason = MARKER_GUIDANCE_REASON_OK;
    const bool targetFresh = markerGuidanceTargetIsFresh(nowMs, &freshnessReason);

    const bool positionEstimateUsable = posControl.flags.estPosStatus >= EST_USABLE;
    const markerGuidanceContext_e selectedContext = markerGuidanceSelectContext(navStateFlags);
    const bool positionTakeover = markerGuidancePositionTakeoverActiveNow();

    updateContextRuntime(selectedContext);

    if (posControl.flags.isAdjustingHeading) {
        // Do not restore a cached marker heading when the pilot releases yaw.
        // A subsequent marker packet explicitly returns heading ownership to PL.
        markerGuidance.headingLatched = false;
        markerGuidance.rejectedHeadingTargetSequence = markerGuidance.target.sequence;
    }

    if (selectedContext == MARKER_GUIDANCE_CONTEXT_NONE) {
        markerGuidance.vtolRecoveryPaused = false;
        markerGuidance.recoveryHeadingLatched = false;
        setMarkerGuidanceState(targetFresh ? MARKER_GUIDANCE_STANDBY : MARKER_GUIDANCE_IDLE);
        return;
    }

    if (!positionEstimateUsable) {
        suspendPositionAcquisitionUntilNewSample();
        markerGuidance.vtolRecoveryPaused = false;
        markerGuidance.recoveryHeadingLatched = false;
        setMarkerGuidanceState(MARKER_GUIDANCE_STANDBY);
        return;
    }

    const bool recoveryWasPaused = markerGuidance.vtolRecoveryPaused;
    const bool vtolRecoveryRequested = navigationVtolMcProtectionGuidanceRecoveryActive();
    const bool targetBelongsToContext = selectedContext != MARKER_GUIDANCE_CONTEXT_LAND ||
        markerGuidanceLandSampleIsNewForContext(
            markerGuidance.target.sequence,
            markerGuidance.lastLandExitTargetSequence);
    const bool vtolCaptureBlocksAcquisition = navigationVtolMcProtectionPositionCaptureActive() ||
        (!markerGuidance.targetAcquiredInContext &&
         navigationVtolMcProtectionPositionCapturePending(navStateFlags));
    const bool navigationBlocksMarkerAcquisition = STATE(NAV_CRUISE_BRAKING) || vtolCaptureBlocksAcquisition;

    // Position and heading ownership are independent. Roll/pitch releases XY,
    // while a fresh marker heading can still be acquired unless yaw itself is
    // under pilot control or another horizontal navigation owner is active.
    if (markerGuidanceIsPlMode() &&
        (selectedContext == MARKER_GUIDANCE_CONTEXT_POSHOLD || selectedContext == MARKER_GUIDANCE_CONTEXT_LAND) &&
        targetFresh && targetBelongsToContext && !recoveryWasPaused && !vtolRecoveryRequested &&
        !navigationBlocksMarkerAcquisition && !posControl.flags.isAdjustingHeading &&
        markerGuidanceHeadingSampleAllowed(markerGuidance.target.sequence, markerGuidance.rejectedHeadingTargetSequence)) {
        markerGuidance.headingLatched = true;
        markerGuidance.latchedHeadingCd = markerGuidance.target.targetHeadingCd;
        markerGuidance.rejectedHeadingTargetSequence = 0;
    }

    if (positionTakeover) {
        suspendPositionAcquisitionUntilNewSample();
        markerGuidance.vtolRecoveryPaused = false;
        markerGuidance.recoveryHeadingLatched = false;
        setMarkerGuidanceState(MARKER_GUIDANCE_STANDBY);
        return;
    }

    markerGuidance.vtolRecoveryPaused = markerGuidanceVtolRecoveryShouldPause(
        vtolRecoveryRequested,
        markerGuidance.targetAcquiredInContext,
        selectedContext);

    if (markerGuidance.vtolRecoveryPaused) {
        // Stop chasing the marker while the attitude controller is recovering.
        // The fixed current-position target lets the normal MC controller brake
        // without reactivating the ordinary VTOL entry capture on PL movement.
        if (!markerGuidance.lostHoldPositionValid) {
            captureLostHoldPosition();
        } else {
            applyLostHoldPosition();
        }

        if (markerGuidanceIsPlMode() && !markerGuidance.recoveryHeadingLatched) {
            markerGuidance.recoveryHeadingCd = posControl.actualState.yaw;
            markerGuidance.recoveryHeadingLatched = true;
        }

        markerGuidance.stateDeadlineMs = 0;
        markerGuidanceResetRetrySettle(&markerGuidance.retrySettle);
        setMarkerGuidanceState(MARKER_GUIDANCE_STANDBY);
        return;
    }

    if (recoveryWasPaused) {
        // A pose observed before recovery finished may have been measured while
        // the vehicle was tilted. Hold the recovered XY/yaw until a new packet
        // confirms that the marker is visible from the settled attitude.
        markerGuidance.rejectedPositionTargetSequence = markerGuidance.target.sequence;
        markerGuidance.rejectedHeadingTargetSequence = markerGuidance.target.sequence;
        if (markerGuidance.recoveryHeadingLatched) {
            markerGuidance.headingLatched = true;
            markerGuidance.latchedHeadingCd = markerGuidance.recoveryHeadingCd;
        }
        startLostTargetHold(nowMs);
    }
    markerGuidance.recoveryHeadingLatched = false;

    const bool positionSampleAllowed = markerGuidanceHeadingSampleAllowed(
        markerGuidance.target.sequence,
        markerGuidance.rejectedPositionTargetSequence);

    const bool positionTargetAllowed = markerGuidancePositionTargetAllowed(
        navStateFlags & NAV_CTL_POS,
        positionEstimateUsable,
        STATE(NAV_CRUISE_BRAKING),
        vtolCaptureBlocksAcquisition,
        positionTakeover);
    if (targetFresh && targetBelongsToContext && positionSampleAllowed && !positionTargetAllowed) {
        // Braking/capture owns the complete horizontal guidance path, including
        // marker yaw acquisition. Require a later packet after that owner exits.
        suspendPositionAcquisitionUntilNewSample();
        setMarkerGuidanceState(MARKER_GUIDANCE_STANDBY);
        return;
    }

    if (markerGuidanceTargetCanBeAcquired(
            targetFresh,
            targetBelongsToContext,
            positionSampleAllowed,
            positionTargetAllowed)) {
        clearLostHoldPosition();
        markerGuidance.stateDeadlineMs = 0;
        markerGuidance.targetAcquiredInContext = true;
        markerGuidance.rejectedPositionTargetSequence = 0;
        markerGuidanceResetRetrySettle(&markerGuidance.retrySettle);

        if (markerGuidanceIsPlMode() && selectedContext == MARKER_GUIDANCE_CONTEXT_LAND) {

            const uint16_t retryMinAltitudeCm = navConfig()->general.marker_guidance_retry_min_alt_cm;
            markerGuidance.lastFreshMarkerWasLow = retryMinAltitudeCm > 0 &&
                markerGuidance.target.markerAglCm <= retryMinAltitudeCm;
        }

        float targetNorthCm = 0.0f;
        float targetEastCm = 0.0f;
        const bool correctionRequired = markerGuidanceComputePositionTarget(
            selectedContext,
            &targetNorthCm,
            &targetEastCm);
        const fpVector3_t *actualPosition = &navGetCurrentActualPositionAndVelocity()->pos;
        markerGuidance.requestedCorrectionNorthCm = targetNorthCm - actualPosition->x;
        markerGuidance.requestedCorrectionEastCm = targetEastCm - actualPosition->y;
        applyGuidancePositionTarget(targetNorthCm, targetEastCm);

        if (correctionRequired) {
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

    if (selectedContext == MARKER_GUIDANCE_CONTEXT_POSHOLD) {
        if (STATE(NAV_CRUISE_BRAKING) || navigationVtolMcProtectionPositionCaptureActive()) {
            // Capturing before braking finishes could pull the aircraft back toward a point passed while stopping.
            releaseGuidancePositionTarget();
            markerGuidance.stateDeadlineMs = 0;
            setMarkerGuidanceState(MARKER_GUIDANCE_STANDBY);
            return;
        }

        if (markerGuidance.targetAcquiredInContext) {
            if (markerGuidance.state != MARKER_GUIDANCE_TARGET_LOST_HOLD) {
                startLostTargetHold(nowMs);
            } else {
                applyLostHoldPosition();
            }
        } else {
            releaseGuidancePositionTarget();
            setMarkerGuidanceState(MARKER_GUIDANCE_STANDBY);
        }
        return;
    }

    if (!markerGuidanceIsPlMode() || !markerGuidance.targetAcquiredInContext) {
        releaseGuidancePositionTarget();
        setMarkerGuidanceState(MARKER_GUIDANCE_STANDBY);
        return;
    }

    if (markerGuidanceRetrySuppressedByAltitude()) {
        setLowAltitudeFallbackNormalLandState();
        return;
    }

    switch (markerGuidance.state) {
    case MARKER_GUIDANCE_TARGET_LOST_HOLD: {
        applyLostHoldPosition();
        const bool retrySettled = markerGuidanceRetryIsSettled(nowMs);
        if (markerGuidanceDeadlineReached(nowMs, markerGuidance.stateDeadlineMs)) {
            if (markerGuidance.retryCount < navConfig()->general.marker_guidance_retry_count) {
                if (retrySettled) {
                    setClimbRetryState(nowMs);
                }
            } else {
                markerGuidanceResetRetrySettle(&markerGuidance.retrySettle);
                releaseGuidancePositionTarget();
                markerGuidance.stateDeadlineMs = 0;
                setMarkerGuidanceState(MARKER_GUIDANCE_FALLBACK_NORMAL_LAND);
            }
        }
        break;
    }

    case MARKER_GUIDANCE_CLIMB_AND_RETRY:
        applyLostHoldPosition();
        if (markerGuidanceRetryClimbFinished(
                markerGuidance.retryStartAltitudeUsable && posControl.flags.estAltStatus >= EST_USABLE,
                markerGuidance.retryStartAltitudeCm,
                navGetCurrentActualPositionAndVelocity()->pos.z,
                navConfig()->general.marker_guidance_retry_altitude_cm,
                markerGuidanceDeadlineReached(nowMs, markerGuidance.stateDeadlineMs))) {
            markerGuidance.retryCount++;
            if (markerGuidance.retryCount >= navConfig()->general.marker_guidance_retry_count) {
                markerGuidanceResetRetrySettle(&markerGuidance.retrySettle);
                releaseGuidancePositionTarget();
                markerGuidance.stateDeadlineMs = 0;
                setMarkerGuidanceState(MARKER_GUIDANCE_FALLBACK_NORMAL_LAND);
            } else {
                setLostHoldState(nowMs);
            }
        }
        break;

    case MARKER_GUIDANCE_FALLBACK_NORMAL_LAND:
        break;

    default:
        startLostTargetHold(nowMs);
        break;
    }
}

bool markerGuidanceApplyHeadingOverride(int32_t *desiredYawCd)
{
    if (markerGuidance.vtolRecoveryPaused && markerGuidance.recoveryHeadingLatched) {
        markerGuidance.headingOverrideApplied = markerGuidanceSelectHeadingOverride(
            markerGuidanceIsPlMode(),
            markerGuidance.activeContext,
            ARMING_FLAG(ARMED),
            !isMcHoverCapableProfileActive(),
            FLIGHT_MODE(FAILSAFE_MODE),
            posControl.flags.isAdjustingHeading,
            false,
            markerGuidance.targetAcquiredInContext,
            0,
            true,
            markerGuidance.recoveryHeadingCd,
            desiredYawCd);
        return markerGuidance.headingOverrideApplied;
    }

    const bool targetFresh = markerGuidanceTargetIsFresh(millis(), NULL) &&
        markerGuidanceHeadingSampleAllowed(markerGuidance.target.sequence, markerGuidance.rejectedHeadingTargetSequence);

    markerGuidance.headingOverrideApplied = markerGuidanceSelectHeadingOverride(
        markerGuidanceIsPlMode(),
        markerGuidance.activeContext,
        ARMING_FLAG(ARMED),
        !isMcHoverCapableProfileActive(),
        FLIGHT_MODE(FAILSAFE_MODE),
        posControl.flags.isAdjustingHeading,
        targetFresh,
        markerGuidance.targetAcquiredInContext,
        markerGuidance.target.targetHeadingCd,
        markerGuidance.headingLatched,
        markerGuidance.latchedHeadingCd,
        desiredYawCd);

    return markerGuidance.headingOverrideApplied;
}

bool markerGuidanceOwnsHeading(void)
{
    return markerGuidance.headingOverrideApplied;
}

bool markerGuidanceOwnsPositionTarget(void)
{
    return markerGuidance.positionTargetOwned;
}

bool markerGuidanceGetActiveLandingPositionTarget(fpVector3_t *targetOut)
{
    if (!targetOut ||
        !markerGuidanceIsPlMode() ||
        markerGuidance.activeContext != MARKER_GUIDANCE_CONTEXT_LAND ||
        !markerGuidance.targetAcquiredInContext ||
        !markerGuidance.positionTargetOwned ||
        !ARMING_FLAG(ARMED) ||
        !isMcHoverCapableProfileActive() ||
        FLIGHT_MODE(FAILSAFE_MODE) ||
        areSensorsCalibrating() ||
        markerGuidancePositionTakeoverActiveNow()) {
        return false;
    }

    targetOut->x = markerGuidance.ownedPositionNorthCm;
    targetOut->y = markerGuidance.ownedPositionEastCm;
    return true;
}

void markerGuidanceUpdateDebug(void)
{
    if (debugMode != DEBUG_MARKER_GUIDANCE) {
        return;
    }

    const timeMs_t nowMs = millis();
    markerGuidanceReason_e freshnessReason = MARKER_GUIDANCE_REASON_OK;
    const bool targetFresh = markerGuidanceTargetIsFresh(nowMs, &freshnessReason);
    const bool velocityTrusted = posControl.flags.estVelStatus == EST_TRUSTED;
    const uint16_t speedLimitCmS = markerGuidanceRetrySettleSpeedLimit(navConfig()->mc.braking_disengage_speed);
    const bool retrySpeedOk = posControl.actualState.velXY <= speedLimitCmS;
    const uint16_t maxAbsAttitudeDeciDeg = MAX(ABS(attitude.values.roll), ABS(attitude.values.pitch));
    const bool retryAttitudeOk = maxAbsAttitudeDeciDeg <= MARKER_GUIDANCE_RETRY_SETTLE_MAX_ATTITUDE_DECIDEG;
    const bool settleReady = markerGuidance.retrySettle.active &&
        (nowMs - markerGuidance.retrySettle.stableSinceMs) >= MARKER_GUIDANCE_RETRY_SETTLE_TIME_MS;
    const bool deadlineReached = markerGuidance.stateDeadlineMs != 0 &&
        markerGuidanceDeadlineReached(nowMs, markerGuidance.stateDeadlineMs);
    const bool lowAltitudeSuppressed = markerGuidanceIsPlMode() &&
        markerGuidance.activeContext == MARKER_GUIDANCE_CONTEXT_LAND &&
        markerGuidanceRetrySuppressedByAltitude();
    const bool vtolCaptureActive = navigationVtolMcProtectionPositionCaptureActive();
    const bool positionEstimateUsable = posControl.flags.estPosStatus >= EST_USABLE;
    const bool positionSampleAllowed = markerGuidanceHeadingSampleAllowed(
        markerGuidance.target.sequence,
        markerGuidance.rejectedPositionTargetSequence);

    uint32_t flags = 0;
    if (markerGuidanceFeatureEnabled()) flags |= MARKER_GUIDANCE_DEBUG_FLAG_ENABLED;
    if (ARMING_FLAG(ARMED)) flags |= MARKER_GUIDANCE_DEBUG_FLAG_ARMED;
    if (isMcHoverCapableProfileActive()) flags |= MARKER_GUIDANCE_DEBUG_FLAG_MC_PROFILE;
    if (markerGuidance.target.valid) flags |= MARKER_GUIDANCE_DEBUG_FLAG_TARGET_VALID;
    if (targetFresh) flags |= MARKER_GUIDANCE_DEBUG_FLAG_TARGET_FRESH;
    if (markerGuidance.targetAcquiredInContext) flags |= MARKER_GUIDANCE_DEBUG_FLAG_TARGET_ACQUIRED;
    if (markerGuidanceIsPlMode()) flags |= MARKER_GUIDANCE_DEBUG_FLAG_PL_MODE;
    if (markerGuidanceIsContainmentMode()) flags |= MARKER_GUIDANCE_DEBUG_FLAG_CONTAINMENT_MODE;
    if (markerGuidance.activeContext == MARKER_GUIDANCE_CONTEXT_POSHOLD) {
        flags |= MARKER_GUIDANCE_DEBUG_FLAG_POSHOLD_CONTEXT;
    }
    if (markerGuidance.activeContext == MARKER_GUIDANCE_CONTEXT_LAND) flags |= MARKER_GUIDANCE_DEBUG_FLAG_LAND_CONTEXT;
    if (markerGuidanceAnyTakeoverActive()) flags |= MARKER_GUIDANCE_DEBUG_FLAG_MANUAL_TAKEOVER;
    if (FLIGHT_MODE(FAILSAFE_MODE)) flags |= MARKER_GUIDANCE_DEBUG_FLAG_FAILSAFE;
    if (areSensorsCalibrating()) flags |= MARKER_GUIDANCE_DEBUG_FLAG_CALIBRATING;
    if (STATE(LANDING_DETECTED)) flags |= MARKER_GUIDANCE_DEBUG_FLAG_LANDING_DETECTED;
    if (markerGuidance.requestedCorrectionNorthCm != 0.0f || markerGuidance.requestedCorrectionEastCm != 0.0f) {
        flags |= MARKER_GUIDANCE_DEBUG_FLAG_CORRECTION_APPLIED;
    }
    if (markerGuidance.headingOverrideApplied) flags |= MARKER_GUIDANCE_DEBUG_FLAG_HEADING_APPLIED;
    if (velocityTrusted) flags |= MARKER_GUIDANCE_DEBUG_FLAG_VELOCITY_TRUSTED;
    if (retrySpeedOk) flags |= MARKER_GUIDANCE_DEBUG_FLAG_RETRY_SPEED_OK;
    if (retryAttitudeOk) flags |= MARKER_GUIDANCE_DEBUG_FLAG_RETRY_ATTITUDE_OK;
    if (markerGuidance.retrySettle.active) flags |= MARKER_GUIDANCE_DEBUG_FLAG_SETTLE_ACTIVE;
    if (settleReady) flags |= MARKER_GUIDANCE_DEBUG_FLAG_SETTLE_READY;
    if (deadlineReached) flags |= MARKER_GUIDANCE_DEBUG_FLAG_DEADLINE_REACHED;
    if (lowAltitudeSuppressed) flags |= MARKER_GUIDANCE_DEBUG_FLAG_LOW_ALT_SUPPRESSED;
    if (markerGuidance.lostHoldPositionValid) flags |= MARKER_GUIDANCE_DEBUG_FLAG_LOST_HOLD_XY_ACTIVE;
    if (markerGuidance.retryStartAltitudeUsable) flags |= MARKER_GUIDANCE_DEBUG_FLAG_RETRY_ALT_USABLE;
    if (navConfig()->general.marker_guidance_low_alt_lock_xy) flags |= MARKER_GUIDANCE_DEBUG_FLAG_LOW_ALT_XY_LOCK;
    if (posControl.flags.estPosStatus == EST_TRUSTED) flags |= MARKER_GUIDANCE_DEBUG_FLAG_POSITION_TRUSTED;
    if (posControl.flags.estAltStatus >= EST_USABLE) flags |= MARKER_GUIDANCE_DEBUG_FLAG_ALTITUDE_USABLE;
    if (posControl.flags.estAglStatus >= EST_USABLE) flags |= MARKER_GUIDANCE_DEBUG_FLAG_AGL_USABLE;
    if (markerGuidance.positionTargetOwned) flags |= MARKER_GUIDANCE_DEBUG_FLAG_POSITION_TARGET_OWNED;
    if (markerGuidance.headingLatched) flags |= MARKER_GUIDANCE_DEBUG_FLAG_HEADING_LATCHED;
    if (!markerGuidanceHeadingSampleAllowed(markerGuidance.target.sequence, markerGuidance.rejectedHeadingTargetSequence) &&
        markerGuidance.rejectedHeadingTargetSequence != 0) {
        flags |= MARKER_GUIDANCE_DEBUG_FLAG_HEADING_SAMPLE_REJECTED;
    }

    const markerGuidanceDebugReason_e debugReason = markerGuidanceDebugReason(
        targetFresh,
        freshnessReason,
        velocityTrusted,
        retrySpeedOk,
        retryAttitudeOk,
        settleReady,
        deadlineReached,
        lowAltitudeSuppressed,
        vtolCaptureActive,
        markerGuidance.vtolRecoveryPaused,
        positionEstimateUsable,
        positionSampleAllowed);
    const uint32_t packedStatus =
        ((uint32_t)markerGuidance.activeContext & 0xFFU) |
        (((uint32_t)debugReason & 0xFFU) << 8) |
        (((uint32_t)markerGuidance.retryCount & 0xFFU) << 16) |
        (((uint32_t)markerGuidance.lastMspReason & 0xFFU) << 24);
    const uint32_t targetAgeMs = markerGuidance.target.valid ? nowMs - markerGuidance.target.lastUpdateMs : 0;
    const int32_t targetAgeDebug = markerGuidance.target.valid
        ? (targetAgeMs <= INT32_MAX ? (int32_t)targetAgeMs : INT32_MAX)
        : -1;
    const uint32_t packedPose =
        (uint32_t)markerGuidance.target.markerAglCm |
        (((uint32_t)(markerGuidance.target.targetHeadingCd / 10) & 0xFFFFU) << 16);

    // MARKER_GUIDANCE debug channels:
    // [0] state
    // [1] flags bitmask
    // [2] context | (runtime reason << 8) | (retry count << 16) | (last MSP reason << 24)
    // [3] target age [ms], -1 when no valid cached target
    // [4] raw forward offset [cm] | (raw right offset [cm] << 16), signed int16 values
    // [5] resolved North offset [cm] | (resolved East offset [cm] << 16), signed int16 values
    // [6] requested North displacement [cm] | (requested East displacement [cm] << 16), signed int16 values
    // [7] marker AGL [cm] | (absolute target heading [deci-degrees] << 16)
    DEBUG_SET(DEBUG_MARKER_GUIDANCE, 0, markerGuidance.state);
    DEBUG_SET(DEBUG_MARKER_GUIDANCE, 1, (int32_t)flags);
    DEBUG_SET(DEBUG_MARKER_GUIDANCE, 2, (int32_t)packedStatus);
    DEBUG_SET(DEBUG_MARKER_GUIDANCE, 3, targetAgeDebug);
    DEBUG_SET(DEBUG_MARKER_GUIDANCE, 4, (int32_t)markerGuidancePackSigned16(
        markerGuidance.target.offsetForwardCm,
        markerGuidance.target.offsetRightCm));
    DEBUG_SET(DEBUG_MARKER_GUIDANCE, 5, (int32_t)markerGuidancePackSigned16(
        lrintf(markerGuidance.target.offsetNorthCm),
        lrintf(markerGuidance.target.offsetEastCm)));
    DEBUG_SET(DEBUG_MARKER_GUIDANCE, 6, (int32_t)markerGuidancePackSigned16(
        lrintf(markerGuidance.requestedCorrectionNorthCm),
        lrintf(markerGuidance.requestedCorrectionEastCm)));
    DEBUG_SET(DEBUG_MARKER_GUIDANCE, 7, (int32_t)packedPose);
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

    // Vertical pilot input releases only marker Z hold/climb ownership. Marker
    // XY and heading remain independent and continue through their own paths.
    if (posControl.flags.isAdjustingAltitude) {
        return;
    }

    if (markerGuidance.vtolRecoveryPaused &&
        !markerGuidancePositionTakeoverActiveNow()) {
        controlOut->mode = MARKER_GUIDANCE_LAND_CTRL_HOLD;
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
    if (markerGuidance.activeContext == MARKER_GUIDANCE_CONTEXT_NONE) {
        return defaultState;
    }

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

bool markerGuidanceHandleMspTargetUpdate(
    const markerGuidanceTargetUpdate_t *update,
    markerGuidanceMspResponse_t *responseOut)
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
        responseOut->reason = MARKER_GUIDANCE_REASON_NOT_ENABLED;
        markerGuidance.lastMspReason = responseOut->reason;
        return true;
    }

    if (!markerGuidancePoseIsValid(update, 0)) {
        markerGuidance.lastMspReason = responseOut->reason;
        return true;
    }

    markerGuidanceResolvedPose_t resolved;
    if (!markerGuidanceTryResolvePose(
            update,
            navConfig()->general.marker_guidance_max_offset_cm,
            posControl.actualState.cosYaw,
            posControl.actualState.sinYaw,
            posControl.actualState.yaw,
            &resolved)) {
        responseOut->reason = MARKER_GUIDANCE_REASON_OFFSET_TOO_LARGE;
        markerGuidance.lastMspReason = responseOut->reason;
        return true;
    }

    if (posControl.flags.estPosStatus < EST_USABLE) {
        responseOut->reason = MARKER_GUIDANCE_REASON_POSITION_UNAVAILABLE;
        markerGuidance.lastMspReason = responseOut->reason;
        return true;
    }

    const fpVector3_t *actualPosition = &navGetCurrentActualPositionAndVelocity()->pos;
    const markerGuidanceTargetCache_t newTarget = {
        .offsetForwardCm = update->offsetForwardCm,
        .offsetRightCm = update->offsetRightCm,
        .offsetNorthCm = resolved.offsetNorthCm,
        .offsetEastCm = resolved.offsetEastCm,
        .markerPositionNorthCm = actualPosition->x + resolved.offsetNorthCm,
        .markerPositionEastCm = actualPosition->y + resolved.offsetEastCm,
        .targetHeadingCd = resolved.targetHeadingCd,
        .markerAglCm = resolved.markerAglCm,
        .horizontalOffsetSquaredCm = markerGuidanceHorizontalOffsetSquaredCm(update),
        .lastUpdateMs = millis(),
        .sequence = markerGuidanceNextSampleSequence(markerGuidance.target.sequence),
        .valid = true,
    };
    markerGuidance.target = newTarget;

    responseOut->accepted = 1;
    responseOut->reason = MARKER_GUIDANCE_REASON_OK;

    const bool mcProfileActive = isMcHoverCapableProfileActive();
    const navigationFSMStateFlags_t navStateFlags = navGetCurrentStateFlags();
    const markerGuidanceContext_e selectedContext = markerGuidanceSelectContext(navStateFlags);
    const bool positionTakeover = markerGuidancePositionTakeoverActiveNow();

    if (!ARMING_FLAG(ARMED)) {
        responseOut->reason = MARKER_GUIDANCE_REASON_NOT_ARMED;
    } else if (FLIGHT_MODE(FAILSAFE_MODE)) {
        responseOut->reason = MARKER_GUIDANCE_REASON_FAILSAFE;
    } else if (!mcProfileActive) {
        responseOut->reason = MARKER_GUIDANCE_REASON_NOT_MC_PROFILE;
    } else if (selectedContext == MARKER_GUIDANCE_CONTEXT_NONE) {
        responseOut->reason = MARKER_GUIDANCE_REASON_NOT_IN_POSHOLD_OR_LAND;
    } else if (areSensorsCalibrating() || STATE(LANDING_DETECTED)) {
        // The navigation loop resets guidance in these states, so the accepted sample is cached but not in use.
        responseOut->reason = MARKER_GUIDANCE_REASON_OK;
    } else {
        const bool vtolRecoveryActive = navigationVtolMcProtectionGuidanceRecoveryActive();
        const bool vtolCaptureBlocksAcquisition = navigationVtolMcProtectionPositionCaptureActive() ||
            (!markerGuidance.targetAcquiredInContext &&
             navigationVtolMcProtectionPositionCapturePending(navStateFlags));
        const bool horizontalCorrectionUsed = !vtolRecoveryActive && markerGuidancePositionTargetAllowed(
            navStateFlags & NAV_CTL_POS,
            posControl.flags.estPosStatus >= EST_USABLE,
            STATE(NAV_CRUISE_BRAKING),
            vtolCaptureBlocksAcquisition,
            positionTakeover);

        int32_t headingCd = posControl.desiredState.yaw;
        const bool headingUsed = (navStateFlags & NAV_CTL_YAW) && markerGuidanceSelectHeadingOverride(
            markerGuidanceIsPlMode(),
            selectedContext,
            true,
            false,
            false,
            posControl.flags.isAdjustingHeading,
            true,
            true,
            markerGuidance.target.targetHeadingCd,
            markerGuidance.headingLatched,
            markerGuidance.latchedHeadingCd,
            &headingCd);

        responseOut->usedNow = horizontalCorrectionUsed || headingUsed;
        responseOut->reason = MARKER_GUIDANCE_REASON_OK;
    }

    responseOut->navGuidanceState = (uint8_t)markerGuidance.state;
    responseOut->retryCount = markerGuidance.retryCount;
    markerGuidance.lastMspReason = responseOut->reason;
    return true;
}

#endif
