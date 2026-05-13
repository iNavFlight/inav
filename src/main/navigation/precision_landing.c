#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "platform.h"

#include "common/maths.h"

#include "drivers/time.h"

#include "fc/fc_core.h"
#include "fc/runtime_config.h"

#include "navigation/precision_landing.h"

#include "sensors/sensors.h"

typedef struct {
    bool cached;
    bool valid;
    uint8_t confidence;
    uint8_t frame;
    int16_t offsetForwardCm;
    int16_t offsetRightCm;
    uint16_t distanceCm;
    uint32_t companionTimestampMs;
    timeMs_t receivedAtMs;
} precisionLandingTargetCache_t;

typedef struct {
    precisionLandingState_e state;
    precisionLandingTargetCache_t target;
    uint8_t retryCount;
    bool landCorrectionEverActive;
    timeMs_t stateDeadlineMs;
    fpVector2_t correctionVelNeu;
    precisionLandingLandControl_t landControl;
} precisionLandingRuntime_t;

static precisionLandingRuntime_t precisionLanding;

static bool precisionLandingFeatureEnabled(void)
{
    return navConfig()->general.precision_landing &&
           navConfig()->general.precision_landing_source == NAV_PRECISION_LANDING_SOURCE_MSP;
}

static bool isMcHoverCapableProfileActive(void)
{
    return STATE(MULTIROTOR) && !STATE(AIRPLANE);
}

static bool precisionLandingIsPosholdContext(navigationFSMStateFlags_t navStateFlags)
{
    return (posControl.navState == NAV_STATE_POSHOLD_3D_IN_PROGRESS) ||
           ((navStateFlags & NAV_CTL_POS) && !(navStateFlags & NAV_CTL_LAND) && FLIGHT_MODE(NAV_POSHOLD_MODE));
}

static bool precisionLandingIsLandContext(navigationFSMStateFlags_t navStateFlags)
{
    return (navStateFlags & NAV_CTL_LAND) && !STATE(AIRPLANE);
}

static bool precisionLandingManualTakeoverActive(void)
{
    return posControl.flags.isAdjustingAltitude ||
           posControl.flags.isAdjustingPosition ||
           posControl.flags.isAdjustingHeading;
}

static void setPrecisionState(precisionLandingState_e state)
{
    precisionLanding.state = state;
}

static float precisionLandingTargetOffsetMagnitudeCm(const precisionLandingTargetCache_t *target)
{
    return calc_length_pythagorean_2D((float)target->offsetForwardCm, (float)target->offsetRightCm);
}

static bool precisionLandingTargetIsFresh(timeMs_t nowMs, precisionLandingReason_e *reasonOut)
{
    if (!precisionLanding.target.cached || !precisionLanding.target.valid) {
        if (reasonOut) {
            *reasonOut = PREC_LAND_REASON_INVALID_TARGET;
        }
        return false;
    }

    if (precisionLanding.target.frame != PREC_LAND_FRAME_BODY_FRD) {
        if (reasonOut) {
            *reasonOut = PREC_LAND_REASON_BAD_FRAME;
        }
        return false;
    }

    if (precisionLanding.target.confidence < navConfig()->general.precision_landing_min_confidence) {
        if (reasonOut) {
            *reasonOut = PREC_LAND_REASON_LOW_CONFIDENCE;
        }
        return false;
    }

    if (navConfig()->general.precision_landing_max_target_age_ms > 0 &&
        (nowMs - precisionLanding.target.receivedAtMs) > navConfig()->general.precision_landing_max_target_age_ms) {
        if (reasonOut) {
            *reasonOut = PREC_LAND_REASON_STALE;
        }
        return false;
    }

    if (navConfig()->general.precision_landing_max_offset_cm > 0 &&
        precisionLandingTargetOffsetMagnitudeCm(&precisionLanding.target) > navConfig()->general.precision_landing_max_offset_cm) {
        if (reasonOut) {
            *reasonOut = PREC_LAND_REASON_OFFSET_TOO_LARGE;
        }
        return false;
    }

    if (reasonOut) {
        *reasonOut = PREC_LAND_REASON_OK;
    }
    return true;
}

static void clearRuntimeOutputs(void)
{
    precisionLanding.correctionVelNeu.x = 0.0f;
    precisionLanding.correctionVelNeu.y = 0.0f;
    precisionLanding.landControl.mode = PREC_LAND_LAND_CTRL_NONE;
    precisionLanding.landControl.rateCmS = 0.0f;
}

static void clearRuntimeStateKeepCache(void)
{
    precisionLanding.retryCount = 0;
    precisionLanding.landCorrectionEverActive = false;
    precisionLanding.stateDeadlineMs = 0;
    clearRuntimeOutputs();
    setPrecisionState(PREC_LAND_IDLE);
}

static void clearRuntimeStateAndTarget(void)
{
    precisionLanding.target.cached = false;
    precisionLanding.target.valid = false;
    clearRuntimeStateKeepCache();
}

void precisionLandingReset(void)
{
    clearRuntimeStateAndTarget();
}

static void computeHorizontalCorrectionVelocity(void)
{
    const float offsetForward = precisionLanding.target.offsetForwardCm;
    const float offsetRight = precisionLanding.target.offsetRightCm;
    const float offsetMagnitude = precisionLandingTargetOffsetMagnitudeCm(&precisionLanding.target);
    const float alignRadiusCm = navConfig()->general.precision_landing_align_radius_cm;

    if (alignRadiusCm > 0.0f && offsetMagnitude <= alignRadiusCm) {
        precisionLanding.correctionVelNeu.x = 0.0f;
        precisionLanding.correctionVelNeu.y = 0.0f;
        return;
    }

    const float cosYaw = posControl.actualState.cosYaw;
    const float sinYaw = posControl.actualState.sinYaw;

    float velN = offsetForward * cosYaw - offsetRight * sinYaw;
    float velE = offsetForward * sinYaw + offsetRight * cosYaw;

    const float maxCorrectionSpeed = navConfig()->general.precision_landing_max_correction_speed_cm_s;
    const float speed = calc_length_pythagorean_2D(velN, velE);

    if (maxCorrectionSpeed > 0.0f && speed > maxCorrectionSpeed) {
        const float scale = maxCorrectionSpeed / speed;
        velN *= scale;
        velE *= scale;
    }

    if (alignRadiusCm > 0.0f && offsetMagnitude > alignRadiusCm) {
        // Ease corrections just outside alignment radius to reduce jitter around the target center.
        const float ease = constrainf((offsetMagnitude - alignRadiusCm) / offsetMagnitude, 0.0f, 1.0f);
        velN *= ease;
        velE *= ease;
    }

    precisionLanding.correctionVelNeu.x = velN;
    precisionLanding.correctionVelNeu.y = velE;
}

static uint16_t getRetryTimeoutMs(void)
{
    const uint16_t configuredRetryTimeoutMs = navConfig()->general.precision_landing_retry_timeout_ms;
    if (configuredRetryTimeoutMs > 0) {
        return MAX(configuredRetryTimeoutMs, 100);
    }

    // Auto mode: derive retry timeout from lost-hold duration to reduce required tuning.
    const uint16_t lostHoldMs = MAX(navConfig()->general.precision_landing_lost_hold_time_ms, 100);
    const uint32_t autoTimeoutMs = (uint32_t)lostHoldMs * 2U;
    return (uint16_t)MAX(MIN(autoTimeoutMs, 60000U), 100U);
}

static float getRetryClimbRateCmS(void)
{
    const uint16_t retryTimeoutMs = getRetryTimeoutMs();
    const float retryTimeoutS = retryTimeoutMs / 1000.0f;
    const float climbRate = navConfig()->general.precision_landing_retry_altitude_cm / retryTimeoutS;
    return constrainf(climbRate, 20.0f, navConfig()->mc.max_auto_climb_rate);
}

static void setLostHoldState(timeMs_t nowMs)
{
    setPrecisionState(PREC_LAND_TARGET_LOST_HOLD);
    precisionLanding.stateDeadlineMs = nowMs + MAX(navConfig()->general.precision_landing_lost_hold_time_ms, 100);
}

static void setClimbRetryState(timeMs_t nowMs)
{
    setPrecisionState(PREC_LAND_CLIMB_AND_RETRY);
    precisionLanding.stateDeadlineMs = nowMs + getRetryTimeoutMs();
}

void precisionLandingUpdate(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);

    clearRuntimeOutputs();

    const timeMs_t nowMs = millis();

    if (!precisionLandingFeatureEnabled()) {
        clearRuntimeStateKeepCache();
        return;
    }

    if (!ARMING_FLAG(ARMED) || FLIGHT_MODE(FAILSAFE_MODE) || areSensorsCalibrating() || STATE(LANDING_DETECTED)) {
        clearRuntimeStateAndTarget();
        return;
    }

    const bool mcProfileActive = isMcHoverCapableProfileActive();
    const bool posholdContext = precisionLandingIsPosholdContext(navStateFlags);
    const bool landContext = precisionLandingIsLandContext(navStateFlags);
    const bool precisionContextActive = mcProfileActive && (posholdContext || landContext);

    if (!precisionContextActive || precisionLandingManualTakeoverActive()) {
        precisionLanding.landCorrectionEverActive = false;
        precisionLanding.retryCount = 0;
        precisionLanding.stateDeadlineMs = 0;

        precisionLandingReason_e reason;
        if (precisionLandingTargetIsFresh(nowMs, &reason)) {
            setPrecisionState(PREC_LAND_STANDBY);
        } else {
            setPrecisionState(PREC_LAND_IDLE);
        }
        return;
    }

    precisionLandingReason_e freshnessReason = PREC_LAND_REASON_OK;
    const bool targetFresh = precisionLandingTargetIsFresh(nowMs, &freshnessReason);

    if (posholdContext) {
        precisionLanding.retryCount = 0;
        precisionLanding.landCorrectionEverActive = false;
        precisionLanding.stateDeadlineMs = 0;

        if (targetFresh) {
            setPrecisionState(PREC_LAND_POSHOLD_CORRECTION);
            computeHorizontalCorrectionVelocity();
        } else {
            setPrecisionState(PREC_LAND_STANDBY);
        }
        return;
    }

    if (!landContext) {
        setPrecisionState(PREC_LAND_STANDBY);
        return;
    }

    if (precisionLanding.state != PREC_LAND_TARGET_LOST_HOLD &&
        precisionLanding.state != PREC_LAND_CLIMB_AND_RETRY &&
        precisionLanding.state != PREC_LAND_FALLBACK_NORMAL_LAND) {
        if (targetFresh) {
            setPrecisionState(PREC_LAND_LAND_CORRECTION);
        } else if (precisionLanding.landCorrectionEverActive) {
            setLostHoldState(nowMs);
        } else {
            setPrecisionState(PREC_LAND_STANDBY);
        }
    }

    switch (precisionLanding.state) {
    case PREC_LAND_LAND_CORRECTION:
        if (!targetFresh) {
            if (precisionLanding.landCorrectionEverActive) {
                setLostHoldState(nowMs);
            } else {
                setPrecisionState(PREC_LAND_STANDBY);
            }
            break;
        }

        precisionLanding.landCorrectionEverActive = true;
        computeHorizontalCorrectionVelocity();
        break;

    case PREC_LAND_TARGET_LOST_HOLD:
        precisionLanding.landControl.mode = PREC_LAND_LAND_CTRL_HOLD;
        precisionLanding.landControl.rateCmS = 0.0f;

        if (targetFresh) {
            setPrecisionState(PREC_LAND_LAND_CORRECTION);
            break;
        }

        if (nowMs >= precisionLanding.stateDeadlineMs) {
            if (precisionLanding.retryCount < navConfig()->general.precision_landing_retry_count) {
                setClimbRetryState(nowMs);
            } else {
                setPrecisionState(PREC_LAND_FALLBACK_NORMAL_LAND);
            }
        }
        break;

    case PREC_LAND_CLIMB_AND_RETRY:
        precisionLanding.landControl.mode = PREC_LAND_LAND_CTRL_CLIMB;
        precisionLanding.landControl.rateCmS = getRetryClimbRateCmS();

        if (targetFresh) {
            setPrecisionState(PREC_LAND_LAND_CORRECTION);
            break;
        }

        if (nowMs >= precisionLanding.stateDeadlineMs) {
            precisionLanding.retryCount++;
            if (precisionLanding.retryCount >= navConfig()->general.precision_landing_retry_count) {
                setPrecisionState(PREC_LAND_FALLBACK_NORMAL_LAND);
            } else {
                setLostHoldState(nowMs);
            }
        }
        break;

    case PREC_LAND_FALLBACK_NORMAL_LAND:
        precisionLanding.landControl.mode = PREC_LAND_LAND_CTRL_NONE;
        precisionLanding.landControl.rateCmS = 0.0f;
        break;

    default:
        break;
    }
}

void precisionLandingApplyHorizontalVelocityCorrection(float *velX, float *velY)
{
    if (!velX || !velY) {
        return;
    }

    if (precisionLanding.state == PREC_LAND_POSHOLD_CORRECTION || precisionLanding.state == PREC_LAND_LAND_CORRECTION) {
        *velX += precisionLanding.correctionVelNeu.x;
        *velY += precisionLanding.correctionVelNeu.y;
    }
}

void precisionLandingGetLandControl(precisionLandingLandControl_t *controlOut)
{
    if (!controlOut) {
        return;
    }

    *controlOut = precisionLanding.landControl;
}

navSystemStatus_State_e precisionLandingOverrideNavStatusState(navSystemStatus_State_e defaultState)
{
    switch (precisionLanding.state) {
    case PREC_LAND_STANDBY:
        return MW_NAV_STATE_PRECISION_LANDING_STANDBY;
    case PREC_LAND_POSHOLD_CORRECTION:
        return MW_NAV_STATE_PRECISION_LANDING_POSHOLD_CORRECTION;
    case PREC_LAND_LAND_CORRECTION:
        return MW_NAV_STATE_PRECISION_LANDING_LAND_CORRECTION;
    case PREC_LAND_TARGET_LOST_HOLD:
        return MW_NAV_STATE_PRECISION_LANDING_TARGET_LOST_HOLD;
    case PREC_LAND_CLIMB_AND_RETRY:
        return MW_NAV_STATE_PRECISION_LANDING_CLIMB_AND_RETRY;
    case PREC_LAND_FALLBACK_NORMAL_LAND:
        return MW_NAV_STATE_PRECISION_LANDING_FALLBACK_NORMAL_LAND;
    default:
        return defaultState;
    }
}

bool precisionLandingHandleMspTargetUpdate(const precisionLandingTargetUpdate_t *update, precisionLandingMspResponse_t *responseOut)
{
    if (!update || !responseOut) {
        return false;
    }

    responseOut->accepted = 0;
    responseOut->usedNow = 0;
    responseOut->navPrecisionState = (uint8_t)precisionLanding.state;
    responseOut->reason = PREC_LAND_REASON_INVALID_TARGET;
    responseOut->retryCount = precisionLanding.retryCount;

    if (!precisionLandingFeatureEnabled()) {
        responseOut->accepted = 1;
        responseOut->reason = PREC_LAND_REASON_NOT_ENABLED;
        return true;
    }

    if (!update->valid) {
        precisionLanding.target.cached = false;
        precisionLanding.target.valid = false;
        responseOut->accepted = 1;
        responseOut->reason = PREC_LAND_REASON_INVALID_TARGET;
        return true;
    }

    if (update->frame != PREC_LAND_FRAME_BODY_FRD) {
        responseOut->reason = PREC_LAND_REASON_BAD_FRAME;
        return true;
    }

    if (update->confidence < navConfig()->general.precision_landing_min_confidence) {
        responseOut->reason = PREC_LAND_REASON_LOW_CONFIDENCE;
        return true;
    }

    const float offsetMagnitude = calc_length_pythagorean_2D((float)update->offsetForwardCm, (float)update->offsetRightCm);
    if (navConfig()->general.precision_landing_max_offset_cm > 0 &&
        offsetMagnitude > navConfig()->general.precision_landing_max_offset_cm) {
        responseOut->reason = PREC_LAND_REASON_OFFSET_TOO_LARGE;
        return true;
    }

    precisionLanding.target.cached = true;
    precisionLanding.target.valid = true;
    precisionLanding.target.confidence = update->confidence;
    precisionLanding.target.frame = update->frame;
    precisionLanding.target.offsetForwardCm = update->offsetForwardCm;
    precisionLanding.target.offsetRightCm = update->offsetRightCm;
    precisionLanding.target.distanceCm = update->distanceCm;
    precisionLanding.target.companionTimestampMs = update->timestampMs;
    precisionLanding.target.receivedAtMs = millis();

    responseOut->accepted = 1;
    responseOut->reason = PREC_LAND_REASON_OK;

    const bool mcProfileActive = isMcHoverCapableProfileActive();
    const navigationFSMStateFlags_t navStateFlags = navGetCurrentStateFlags();
    const bool posholdContext = precisionLandingIsPosholdContext(navStateFlags);
    const bool landContext = precisionLandingIsLandContext(navStateFlags);
    const bool inContext = posholdContext || landContext;

    if (!ARMING_FLAG(ARMED)) {
        responseOut->usedNow = 0;
        responseOut->reason = PREC_LAND_REASON_NOT_ARMED;
    } else if (FLIGHT_MODE(FAILSAFE_MODE)) {
        responseOut->usedNow = 0;
        responseOut->reason = PREC_LAND_REASON_FAILSAFE;
    } else if (!mcProfileActive) {
        responseOut->usedNow = 0;
        responseOut->reason = PREC_LAND_REASON_NOT_MC_PROFILE;
    } else if (!inContext) {
        responseOut->usedNow = 0;
        responseOut->reason = PREC_LAND_REASON_NOT_IN_POSHOLD_OR_LAND;
    } else {
        responseOut->usedNow = (precisionLanding.state == PREC_LAND_POSHOLD_CORRECTION || precisionLanding.state == PREC_LAND_LAND_CORRECTION);
        responseOut->reason = responseOut->usedNow ? PREC_LAND_REASON_OK : PREC_LAND_REASON_NOT_IN_POSHOLD_OR_LAND;
    }

    responseOut->navPrecisionState = (uint8_t)precisionLanding.state;
    responseOut->retryCount = precisionLanding.retryCount;
    return true;
}
