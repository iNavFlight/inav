#pragma once

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MARKER_GUIDANCE_MSP_PAYLOAD_SIZE 8U
#define MARKER_GUIDANCE_MAX_YAW_ERROR_DECIDEG 1800
#define MARKER_GUIDANCE_RETRY_SETTLE_TIME_MS 500U
#define MARKER_GUIDANCE_RETRY_SETTLE_MAX_SPEED_CM_S 75U
#define MARKER_GUIDANCE_RETRY_SETTLE_MAX_ATTITUDE_DECIDEG 100U
#define MARKER_GUIDANCE_RETARGET_MIN_VELOCITY_ERROR_CM_S 0.1f
#define MARKER_GUIDANCE_LAND_FULL_DESCENT_OFFSET_AGL_RATIO 0.10f
#define MARKER_GUIDANCE_LAND_HOLD_DESCENT_OFFSET_AGL_RATIO 0.30f
#define MARKER_GUIDANCE_TARGET_CONFIRMATION_SAMPLES 3U
#define MARKER_GUIDANCE_TARGET_CONSISTENCY_AGL_RATIO 0.05f
#define MARKER_GUIDANCE_TARGET_CONSISTENCY_MIN_CM 10.0f
#define MARKER_GUIDANCE_PRELANDING_MIN_ALIGNMENT_RADIUS_CM 10U

typedef enum {
    MARKER_GUIDANCE_CONTEXT_NONE = 0,
    MARKER_GUIDANCE_CONTEXT_POSHOLD,
    MARKER_GUIDANCE_CONTEXT_LAND,
} markerGuidanceContext_e;

typedef struct {
    int16_t offsetNorthCm;
    int16_t offsetEastCm;
    int16_t yawErrorDeciDeg;
    uint16_t markerAglCm;
} markerGuidancePoseUpdate_t;

typedef struct {
    float offsetNorthCm;
    float offsetEastCm;
    int32_t targetHeadingCd;
    uint16_t markerAglCm;
} markerGuidanceResolvedPose_t;

typedef struct {
    uint32_t stableSinceMs;
    bool active;
} markerGuidanceRetrySettleState_t;

typedef struct {
    float candidateNorthCm;
    float candidateEastCm;
    uint32_t lastSampleMs;
    uint8_t sampleCount;
    bool active;
} markerGuidanceTargetConfirmationState_t;

static inline bool markerGuidanceMspPayloadSizeIsValid(size_t dataSize)
{
    return dataSize == MARKER_GUIDANCE_MSP_PAYLOAD_SIZE;
}

static inline markerGuidancePoseUpdate_t markerGuidanceDecodeMspWords(
    uint16_t offsetNorthRaw,
    uint16_t offsetEastRaw,
    uint16_t yawErrorRaw,
    uint16_t markerAglRaw)
{
    const markerGuidancePoseUpdate_t update = {
        .offsetNorthCm = (int16_t)offsetNorthRaw,
        .offsetEastCm = (int16_t)offsetEastRaw,
        .yawErrorDeciDeg = (int16_t)yawErrorRaw,
        .markerAglCm = markerAglRaw,
    };
    return update;
}

static inline int32_t markerGuidanceWrapHeadingCd(int32_t headingCd)
{
    headingCd %= 36000;
    return headingCd < 0 ? headingCd + 36000 : headingCd;
}

static inline uint32_t markerGuidanceHorizontalOffsetSquaredCm(const markerGuidancePoseUpdate_t *update)
{
    if (!update) {
        return 0;
    }

    const int64_t north = update->offsetNorthCm;
    const int64_t east = update->offsetEastCm;
    return (uint32_t)((north * north) + (east * east));
}

static inline bool markerGuidancePoseIsValid(const markerGuidancePoseUpdate_t *update, uint16_t maxOffsetCm)
{
    if (!update ||
        update->yawErrorDeciDeg < -MARKER_GUIDANCE_MAX_YAW_ERROR_DECIDEG ||
        update->yawErrorDeciDeg > MARKER_GUIDANCE_MAX_YAW_ERROR_DECIDEG ||
        update->markerAglCm == 0) {
        return false;
    }

    if (maxOffsetCm > 0) {
        const uint64_t maxOffsetSquared = (uint64_t)maxOffsetCm * maxOffsetCm;
        if (markerGuidanceHorizontalOffsetSquaredCm(update) > maxOffsetSquared) {
            return false;
        }
    }

    return true;
}

static inline bool markerGuidanceSampleIsFresh(bool valid, uint32_t nowMs, uint32_t lastUpdateMs, uint16_t maxAgeMs)
{
    return valid && (maxAgeMs == 0 || (nowMs - lastUpdateMs) <= maxAgeMs);
}

static inline uint16_t markerGuidanceRetrySettleSpeedLimit(uint16_t brakingDisengageSpeedCmS)
{
    if (brakingDisengageSpeedCmS > 0 && brakingDisengageSpeedCmS < MARKER_GUIDANCE_RETRY_SETTLE_MAX_SPEED_CM_S) {
        return brakingDisengageSpeedCmS;
    }

    return MARKER_GUIDANCE_RETRY_SETTLE_MAX_SPEED_CM_S;
}

static inline bool markerGuidanceRetrySettleConditionsMet(
    bool horizontalVelocityTrusted,
    float horizontalSpeedCmS,
    uint16_t maxAbsAttitudeDeciDeg,
    uint16_t speedLimitCmS)
{
    return horizontalVelocityTrusted &&
           horizontalSpeedCmS <= speedLimitCmS &&
           maxAbsAttitudeDeciDeg <= MARKER_GUIDANCE_RETRY_SETTLE_MAX_ATTITUDE_DECIDEG;
}

static inline void markerGuidanceResetRetrySettle(markerGuidanceRetrySettleState_t *state)
{
    if (state) {
        state->stableSinceMs = 0;
        state->active = false;
    }
}

static inline bool markerGuidanceUpdateRetrySettle(
    markerGuidanceRetrySettleState_t *state,
    bool conditionsMet,
    uint32_t nowMs)
{
    if (!state || !conditionsMet) {
        markerGuidanceResetRetrySettle(state);
        return false;
    }

    if (!state->active) {
        state->stableSinceMs = nowMs;
        state->active = true;
        return false;
    }

    return (nowMs - state->stableSinceMs) >= MARKER_GUIDANCE_RETRY_SETTLE_TIME_MS;
}

static inline bool markerGuidanceRetryClimbFinished(
    bool altitudeUsable,
    float startAltitudeCm,
    float currentAltitudeCm,
    uint16_t requestedClimbCm,
    bool timeoutReached)
{
    return timeoutReached ||
           (altitudeUsable && currentAltitudeCm >= startAltitudeCm + requestedClimbCm);
}

static inline bool markerGuidanceTryResolvePose(
    const markerGuidancePoseUpdate_t *update,
    uint16_t maxOffsetCm,
    int32_t currentHeadingCd,
    markerGuidanceResolvedPose_t *resolvedOut)
{
    if (!resolvedOut || !markerGuidancePoseIsValid(update, maxOffsetCm)) {
        return false;
    }

    const markerGuidanceResolvedPose_t resolved = {
        .offsetNorthCm = (float)update->offsetNorthCm,
        .offsetEastCm = (float)update->offsetEastCm,
        .targetHeadingCd = markerGuidanceWrapHeadingCd(currentHeadingCd + ((int32_t)update->yawErrorDeciDeg * 10)),
        .markerAglCm = update->markerAglCm,
    };

    *resolvedOut = resolved;
    return true;
}

static inline void markerGuidanceResetTargetConfirmation(markerGuidanceTargetConfirmationState_t *state)
{
    if (state) {
        state->candidateNorthCm = 0.0f;
        state->candidateEastCm = 0.0f;
        state->lastSampleMs = 0;
        state->sampleCount = 0;
        state->active = false;
    }
}

static inline float markerGuidanceTargetConsistencyToleranceCm(uint16_t markerAglCm, uint16_t alignmentRadiusCm)
{
    return fmaxf(
        MARKER_GUIDANCE_TARGET_CONSISTENCY_MIN_CM,
        fmaxf(alignmentRadiusCm, markerAglCm * MARKER_GUIDANCE_TARGET_CONSISTENCY_AGL_RATIO));
}

static inline bool markerGuidanceTargetPositionIsConsistent(
    float referenceNorthCm,
    float referenceEastCm,
    float sampleNorthCm,
    float sampleEastCm,
    float toleranceCm)
{
    const float deltaNorthCm = sampleNorthCm - referenceNorthCm;
    const float deltaEastCm = sampleEastCm - referenceEastCm;
    return (deltaNorthCm * deltaNorthCm) + (deltaEastCm * deltaEastCm) <= toleranceCm * toleranceCm;
}

static inline bool markerGuidanceUpdateTargetConfirmation(
    markerGuidanceTargetConfirmationState_t *state,
    float sampleNorthCm,
    float sampleEastCm,
    uint16_t markerAglCm,
    uint16_t alignmentRadiusCm,
    uint32_t nowMs,
    uint16_t maxSampleGapMs,
    float *confirmedNorthOut,
    float *confirmedEastOut)
{
    if (!state || !confirmedNorthOut || !confirmedEastOut) {
        return false;
    }

    const float toleranceCm = markerGuidanceTargetConsistencyToleranceCm(markerAglCm, alignmentRadiusCm);
    const bool sampleGapExpired = state->active && maxSampleGapMs > 0 &&
        (nowMs - state->lastSampleMs) > maxSampleGapMs;
    const bool candidateConsistent = state->active && !sampleGapExpired &&
        markerGuidanceTargetPositionIsConsistent(
            state->candidateNorthCm,
            state->candidateEastCm,
            sampleNorthCm,
            sampleEastCm,
            toleranceCm);

    if (!candidateConsistent) {
        state->candidateNorthCm = sampleNorthCm;
        state->candidateEastCm = sampleEastCm;
        state->sampleCount = 1;
        state->active = true;
    } else {
        state->sampleCount++;
        const float sampleWeight = 1.0f / state->sampleCount;
        state->candidateNorthCm += (sampleNorthCm - state->candidateNorthCm) * sampleWeight;
        state->candidateEastCm += (sampleEastCm - state->candidateEastCm) * sampleWeight;
    }
    state->lastSampleMs = nowMs;

    if (state->sampleCount < MARKER_GUIDANCE_TARGET_CONFIRMATION_SAMPLES) {
        return false;
    }

    *confirmedNorthOut = state->candidateNorthCm;
    *confirmedEastOut = state->candidateEastCm;
    markerGuidanceResetTargetConfirmation(state);
    return true;
}

static inline bool markerGuidancePrelandingXyReady(
    bool targetFresh,
    bool targetAcquired,
    bool positionTargetOwned,
    bool confirmationPending,
    bool horizontalVelocityTrusted,
    float horizontalSpeedCmS,
    uint16_t speedLimitCmS,
    uint32_t horizontalOffsetSquaredCm,
    uint16_t alignmentRadiusCm)
{
    const uint16_t effectiveRadiusCm = alignmentRadiusCm > MARKER_GUIDANCE_PRELANDING_MIN_ALIGNMENT_RADIUS_CM ?
        alignmentRadiusCm : MARKER_GUIDANCE_PRELANDING_MIN_ALIGNMENT_RADIUS_CM;
    return targetFresh && targetAcquired && positionTargetOwned && !confirmationPending &&
           horizontalVelocityTrusted && horizontalSpeedCmS <= speedLimitCmS &&
           horizontalOffsetSquaredCm <= (uint32_t)effectiveRadiusCm * effectiveRadiusCm;
}

static inline bool markerGuidanceComputeHorizontalPositionTarget(
    float currentNorthCm,
    float currentEastCm,
    float markerNorthCm,
    float markerEastCm,
    float desiredVehicleRelNorthCm,
    float desiredVehicleRelEastCm,
    float radiusCm,
    float *targetNorthOut,
    float *targetEastOut)
{
    if (!targetNorthOut || !targetEastOut) {
        return false;
    }

    *targetNorthOut = currentNorthCm;
    *targetEastOut = currentEastCm;

    const float desiredNorthCm = markerNorthCm + desiredVehicleRelNorthCm;
    const float desiredEastCm = markerEastCm + desiredVehicleRelEastCm;
    float errorNorthCm = desiredNorthCm - currentNorthCm;
    float errorEastCm = desiredEastCm - currentEastCm;
    const float errorMagnitudeCm = sqrtf((errorNorthCm * errorNorthCm) + (errorEastCm * errorEastCm));

    if (errorMagnitudeCm <= 0.0f || (radiusCm > 0.0f && errorMagnitudeCm <= radiusCm)) {
        return false;
    }

    if (radiusCm > 0.0f) {
        const float scale = (errorMagnitudeCm - radiusCm) / errorMagnitudeCm;
        errorNorthCm *= scale;
        errorEastCm *= scale;
    }

    *targetNorthOut = currentNorthCm + errorNorthCm;
    *targetEastOut = currentEastCm + errorEastCm;
    return true;
}

static inline bool markerGuidanceRemoveOpposingIntegratorComponent(
    float velocityErrorNorthCmS,
    float velocityErrorEastCmS,
    float *integratorNorth,
    float *integratorEast)
{
    if (!integratorNorth || !integratorEast) {
        return false;
    }

    const float correctionMagnitudeSquared =
        (velocityErrorNorthCmS * velocityErrorNorthCmS) + (velocityErrorEastCmS * velocityErrorEastCmS);
    if (correctionMagnitudeSquared <=
        (MARKER_GUIDANCE_RETARGET_MIN_VELOCITY_ERROR_CM_S * MARKER_GUIDANCE_RETARGET_MIN_VELOCITY_ERROR_CM_S)) {
        return false;
    }

    const float opposingProjection =
        ((*integratorNorth * velocityErrorNorthCmS) + (*integratorEast * velocityErrorEastCmS)) /
        correctionMagnitudeSquared;
    if (opposingProjection >= 0.0f) {
        return false;
    }

    // Preserve cross-wind compensation and remove only the component that
    // would initially drive the aircraft away from the newly acquired target.
    *integratorNorth -= opposingProjection * velocityErrorNorthCmS;
    *integratorEast -= opposingProjection * velocityErrorEastCmS;
    return true;
}

static inline float markerGuidanceLandingDescentScale(
    float horizontalOffsetCm,
    uint16_t markerAglCm,
    uint16_t alignmentRadiusCm)
{
    if (markerAglCm == 0) {
        return 1.0f;
    }

    const float fullDescentOffsetCm = fmaxf(
        alignmentRadiusCm,
        markerAglCm * MARKER_GUIDANCE_LAND_FULL_DESCENT_OFFSET_AGL_RATIO);
    const float holdDescentOffsetCm = fmaxf(
        alignmentRadiusCm * 3.0f,
        markerAglCm * MARKER_GUIDANCE_LAND_HOLD_DESCENT_OFFSET_AGL_RATIO);

    if (horizontalOffsetCm <= fullDescentOffsetCm) {
        return 1.0f;
    }
    if (horizontalOffsetCm >= holdDescentOffsetCm) {
        return 0.0f;
    }

    return (holdDescentOffsetCm - horizontalOffsetCm) /
        (holdDescentOffsetCm - fullDescentOffsetCm);
}

static inline bool markerGuidanceSelectHeadingOverride(
    bool plMode,
    markerGuidanceContext_e context,
    bool armed,
    bool fixedWingProfile,
    bool failsafe,
    bool manualYawTakeover,
    bool targetFresh,
    bool targetAcquiredInContext,
    int32_t freshTargetHeadingCd,
    bool headingLatched,
    int32_t latchedHeadingCd,
    int32_t *headingOut)
{
    if (!headingOut || !plMode || !armed || fixedWingProfile || failsafe || manualYawTakeover) {
        return false;
    }

    if (targetFresh && targetAcquiredInContext &&
        (context == MARKER_GUIDANCE_CONTEXT_POSHOLD || context == MARKER_GUIDANCE_CONTEXT_LAND)) {
        *headingOut = freshTargetHeadingCd;
        return true;
    }

    if ((context == MARKER_GUIDANCE_CONTEXT_POSHOLD || context == MARKER_GUIDANCE_CONTEXT_LAND) && headingLatched) {
        *headingOut = latchedHeadingCd;
        return true;
    }

    return false;
}

static inline bool markerGuidanceHeadingSampleAllowed(uint32_t sampleSequence, uint32_t rejectedSequence)
{
    return sampleSequence != 0 && sampleSequence != rejectedSequence;
}

static inline bool markerGuidancePositionTargetAllowed(
    bool positionControlActive,
    bool positionEstimateUsable,
    bool cruiseBrakingActive,
    bool vtolCaptureActive,
    bool manualTakeover)
{
    return positionControlActive && positionEstimateUsable &&
           !cruiseBrakingActive && !vtolCaptureActive && !manualTakeover;
}

static inline bool markerGuidancePositionTakeoverActive(
    bool manualPosition)
{
    return manualPosition;
}

static inline bool markerGuidanceTargetCanBeAcquired(
    bool targetFresh,
    bool targetBelongsToContext,
    bool sampleSequenceAllowed,
    bool positionTargetAllowed)
{
    return targetFresh && targetBelongsToContext && sampleSequenceAllowed && positionTargetAllowed;
}

static inline bool markerGuidanceVtolRecoveryShouldPause(
    bool recoveryActive,
    bool targetAcquiredInContext,
    markerGuidanceContext_e context)
{
    return recoveryActive && targetAcquiredInContext && context != MARKER_GUIDANCE_CONTEXT_NONE;
}

static inline bool markerGuidanceRetryIsSuppressedByAltitude(
    uint16_t retryMinAltitudeCm,
    bool inavAglUsable,
    float inavAglCm,
    bool lastFreshMarkerWasLow)
{
    if (retryMinAltitudeCm == 0) {
        return false;
    }

    const bool inavAglIsLow = inavAglUsable && inavAglCm >= 0.0f && inavAglCm <= retryMinAltitudeCm;
    return inavAglIsLow || lastFreshMarkerWasLow;
}

static inline uint32_t markerGuidanceNextSampleSequence(uint32_t currentSequence)
{
    const uint32_t nextSequence = currentSequence + 1U;
    return nextSequence == 0 ? 1U : nextSequence;
}

static inline bool markerGuidanceLandSampleIsNewForContext(uint32_t sampleSequence, uint32_t lastLandExitSequence)
{
    return sampleSequence != 0 && sampleSequence != lastLandExitSequence;
}

static inline bool markerGuidanceDeadlineReached(uint32_t nowMs, uint32_t deadlineMs)
{
    return (int32_t)(nowMs - deadlineMs) >= 0;
}
