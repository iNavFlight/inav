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
    float altitudeCm;
    bool active;
} markerGuidanceAltitudeHoldState_t;

static inline bool markerGuidanceUpdateAltitudeHold(
    markerGuidanceAltitudeHoldState_t *state, bool holdRequested, bool altitudeUsable, float altitudeCm)
{
    if (!holdRequested || !altitudeUsable || !isfinite(altitudeCm)) {
        state->active = false;
        return false;
    }
    if (!state->active) {
        state->altitudeCm = altitudeCm;
        state->active = true;
    }
    return true;
}

typedef struct {
    float candidateNorthCm;
    float candidateEastCm;
    uint32_t lastSampleMs;
    uint8_t sampleCount;
    bool active;
} markerGuidanceTargetConfirmationState_t;

typedef enum {
    MARKER_GUIDANCE_AXIS_NONE = 0,
    MARKER_GUIDANCE_AXIS_NORTH = 1 << 0,
    MARKER_GUIDANCE_AXIS_EAST = 1 << 1,
    MARKER_GUIDANCE_AXIS_BOTH = MARKER_GUIDANCE_AXIS_NORTH | MARKER_GUIDANCE_AXIS_EAST,
} markerGuidanceAxisMask_e;

typedef struct {
    float referenceCm;
    bool deadbandEntered;
    bool valid;
} markerGuidanceCorrectionAxisState_t;

typedef struct {
    markerGuidanceCorrectionAxisState_t north;
    markerGuidanceCorrectionAxisState_t east;
} markerGuidanceCorrectionDirectionState_t;

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

static inline void markerGuidanceSelectLowAltitudeHoldPosition(
    bool markerTargetOwned,
    float markerTargetNorthCm,
    float markerTargetEastCm,
    float currentNorthCm,
    float currentEastCm,
    float *holdNorthOut,
    float *holdEastOut)
{
    if (!holdNorthOut || !holdEastOut) {
        return;
    }

    *holdNorthOut = markerTargetOwned ? markerTargetNorthCm : currentNorthCm;
    *holdEastOut = markerTargetOwned ? markerTargetEastCm : currentEastCm;
}

static inline bool markerGuidanceShouldKeepLowAltitudeMarkerTarget(
    bool lowAltitudeLockEnabled,
    bool markerTargetOwned,
    uint16_t retryMinAltitudeCm,
    uint16_t lastMarkerAglCm)
{
    return lowAltitudeLockEnabled &&
           markerTargetOwned &&
           retryMinAltitudeCm > 0 &&
           lastMarkerAglCm > 0 &&
           lastMarkerAglCm <= retryMinAltitudeCm;
}

static inline bool markerGuidanceShouldKeepConfirmedLandTarget(
    bool markerTargetLockEnabled,
    bool markerTargetOwned)
{
    return markerTargetLockEnabled && markerTargetOwned;
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

static inline bool markerGuidanceTargetReplacementNeedsControllerReconcile(
    bool targetReferenceChanged,
    bool positionTargetOwned)
{
    return targetReferenceChanged && positionTargetOwned;
}

static inline bool markerGuidanceTargetOwnershipIsContinuous(
    bool targetAcquired,
    bool positionTargetOwned,
    bool correctionStateActive,
    bool insideRadiusStandby)
{
    return targetAcquired && positionTargetOwned &&
        (correctionStateActive || insideRadiusStandby);
}

static inline uint8_t markerGuidanceCorrectionDirectionValidAxes(
    const markerGuidanceCorrectionDirectionState_t *state)
{
    if (!state) {
        return MARKER_GUIDANCE_AXIS_NONE;
    }

    uint8_t validAxes = MARKER_GUIDANCE_AXIS_NONE;
    if (state->north.valid) {
        validAxes |= MARKER_GUIDANCE_AXIS_NORTH;
    }
    if (state->east.valid) {
        validAxes |= MARKER_GUIDANCE_AXIS_EAST;
    }
    return validAxes;
}

static inline uint8_t markerGuidancePositionControllerReconcileAxes(
    bool correctionRequired,
    bool targetOwnershipContinuous,
    uint8_t previouslyValidAxes,
    uint8_t currentlyValidAxes,
    uint8_t crossedAxes)
{
    (void)crossedAxes;
    if (!correctionRequired) {
        return MARKER_GUIDANCE_AXIS_NONE;
    }

    if (!targetOwnershipContinuous) {
        return MARKER_GUIDANCE_AXIS_BOTH;
    }

    // A crossing does not invalidate wind compensation. Preserve acquisition
    // and first-axis initialization, but let the PID handle later crossings.
    return currentlyValidAxes & ~previouslyValidAxes;
}

static inline void markerGuidanceResetCorrectionDirection(
    markerGuidanceCorrectionDirectionState_t *state)
{
    if (!state) {
        return;
    }

    state->north.referenceCm = 0.0f;
    state->north.deadbandEntered = false;
    state->north.valid = false;
    state->east.referenceCm = 0.0f;
    state->east.deadbandEntered = false;
    state->east.valid = false;
}

static inline bool markerGuidanceCorrectionAxisCrossedTarget(
    markerGuidanceCorrectionAxisState_t *state,
    uint16_t deadbandCm,
    float correctionCm)
{
    if (!state) {
        return false;
    }

    if (deadbandCm > 0 && fabsf(correctionCm) <= deadbandCm) {
        state->deadbandEntered = state->valid;
        return false;
    }

    if (correctionCm == 0.0f) {
        return false;
    }

    if (!state->valid) {
        state->referenceCm = correctionCm;
        state->deadbandEntered = false;
        state->valid = true;
        return false;
    }

    if (deadbandCm > 0 && !state->deadbandEntered) {
        return false;
    }
    state->deadbandEntered = false;
    if (correctionCm * state->referenceCm >= 0.0f) {
        return false;
    }

    state->referenceCm = correctionCm;
    return true;
}

static inline uint8_t markerGuidanceCorrectionCrossedTargetAxes(
    markerGuidanceCorrectionDirectionState_t *state,
    uint16_t deadbandCm,
    float correctionNorthCm,
    float correctionEastCm)
{
    if (!state) {
        return MARKER_GUIDANCE_AXIS_NONE;
    }

    uint8_t crossedAxes = MARKER_GUIDANCE_AXIS_NONE;
    if (markerGuidanceCorrectionAxisCrossedTarget(&state->north, deadbandCm, correctionNorthCm)) {
        crossedAxes |= MARKER_GUIDANCE_AXIS_NORTH;
    }
    if (markerGuidanceCorrectionAxisCrossedTarget(&state->east, deadbandCm, correctionEastCm)) {
        crossedAxes |= MARKER_GUIDANCE_AXIS_EAST;
    }
    return crossedAxes;
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

static inline float markerGuidanceFullDescentOffsetCm(
    uint16_t markerAglCm,
    uint16_t alignmentRadiusCm)
{
    return fmaxf(
        alignmentRadiusCm,
        markerAglCm * MARKER_GUIDANCE_LAND_FULL_DESCENT_OFFSET_AGL_RATIO);
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
    uint16_t markerAglCm,
    uint16_t alignmentRadiusCm)
{
    const float effectiveRadiusCm = fmaxf(
        MARKER_GUIDANCE_PRELANDING_MIN_ALIGNMENT_RADIUS_CM,
        markerGuidanceFullDescentOffsetCm(markerAglCm, alignmentRadiusCm));
    return targetFresh && targetAcquired && positionTargetOwned && !confirmationPending &&
           horizontalVelocityTrusted && horizontalSpeedCmS <= speedLimitCmS &&
           horizontalOffsetSquaredCm <= effectiveRadiusCm * effectiveRadiusCm;
}

static inline bool markerGuidancePrelandingHeldTargetReady(
    bool lossHoldElapsed,
    bool confirmationPending,
    bool positionUsable,
    bool horizontalVelocityTrusted,
    float horizontalSpeedCmS,
    uint16_t speedLimitCmS,
    float horizontalOffsetSquaredCm,
    uint16_t markerAglCm,
    uint16_t alignmentRadiusCm)
{
    const float effectiveRadiusCm = fmaxf(
        MARKER_GUIDANCE_PRELANDING_MIN_ALIGNMENT_RADIUS_CM,
        markerGuidanceFullDescentOffsetCm(markerAglCm, alignmentRadiusCm));
    return lossHoldElapsed && !confirmationPending && positionUsable && horizontalVelocityTrusted &&
           horizontalSpeedCmS <= speedLimitCmS &&
           horizontalOffsetSquaredCm <= effectiveRadiusCm * effectiveRadiusCm;
}

static inline bool markerGuidanceShouldHoldPrelandingTarget(
    bool precisionLandingMode,
    bool holdEnabled,
    bool confirmedTargetAcquired,
    bool markerOwnsPosition)
{
    return precisionLandingMode && holdEnabled && confirmedTargetAcquired && markerOwnsPosition;
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

static inline bool markerGuidanceReconcileIntegratorForTargetHandoff(
    uint8_t axes,
    float velocityErrorNorthCmS,
    float velocityErrorEastCmS,
    float controllerOutputNorth,
    float controllerOutputEast,
    float *integratorNorth,
    float *integratorEast)
{
    if (!integratorNorth || !integratorEast) {
        return false;
    }

    bool changed = false;
    if ((axes & MARKER_GUIDANCE_AXIS_NORTH) &&
        fabsf(velocityErrorNorthCmS) > MARKER_GUIDANCE_RETARGET_MIN_VELOCITY_ERROR_CM_S &&
        *integratorNorth * velocityErrorNorthCmS < 0.0f &&
        controllerOutputNorth * velocityErrorNorthCmS < 0.0f) {
        const float removed = fminf(fabsf(controllerOutputNorth), fabsf(*integratorNorth));
        *integratorNorth -= copysignf(removed, controllerOutputNorth);
        changed = true;
    }
    if ((axes & MARKER_GUIDANCE_AXIS_EAST) &&
        fabsf(velocityErrorEastCmS) > MARKER_GUIDANCE_RETARGET_MIN_VELOCITY_ERROR_CM_S &&
        *integratorEast * velocityErrorEastCmS < 0.0f &&
        controllerOutputEast * velocityErrorEastCmS < 0.0f) {
        const float removed = fminf(fabsf(controllerOutputEast), fabsf(*integratorEast));
        *integratorEast -= copysignf(removed, controllerOutputEast);
        changed = true;
    }
    return changed;
}

static inline float markerGuidanceLandingDescentScale(
    float horizontalOffsetCm,
    uint16_t markerAglCm,
    uint16_t alignmentRadiusCm)
{
    if (markerAglCm == 0) {
        return 1.0f;
    }

    const float fullDescentOffsetCm = markerGuidanceFullDescentOffsetCm(
        markerAglCm,
        alignmentRadiusCm);
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

static inline float markerGuidanceLandingMotionDescentScale(
    float offsetNorthCm,
    float offsetEastCm,
    float velocityNorthCmS,
    float velocityEastCmS,
    bool velocityTrusted,
    uint16_t markerAglCm,
    uint16_t alignmentRadiusCm,
    float nominalDescentCmS)
{
    const float currentOffsetCm = sqrtf(offsetNorthCm * offsetNorthCm + offsetEastCm * offsetEastCm);
    const float currentScale = markerGuidanceLandingDescentScale(
        currentOffsetCm, markerAglCm, alignmentRadiusCm);
    if (!velocityTrusted || markerAglCm == 0 ||
        !isfinite(nominalDescentCmS) || nominalDescentCmS <= 0.0f ||
        !isfinite(velocityNorthCmS) || !isfinite(velocityEastCmS)) {
        return currentScale;
    }

    // Crossing the image centre is not the same as stopping there. Check the
    // remaining nominal descent time at constant XY velocity, without assuming
    // a particular airframe braking capability. Never relax the current gate.
    const float descentTimeS = markerAglCm / nominalDescentCmS;
    const float predictedNorthCm = offsetNorthCm - velocityNorthCmS * descentTimeS;
    const float predictedEastCm = offsetEastCm - velocityEastCmS * descentTimeS;
    const float predictedOffsetCm = sqrtf(predictedNorthCm * predictedNorthCm + predictedEastCm * predictedEastCm);
    return fminf(currentScale, markerGuidanceLandingDescentScale(
        predictedOffsetCm, markerAglCm, alignmentRadiusCm));
}

static inline float markerGuidanceLandingPositionResponseScale(
    float horizontalOffsetCm,
    uint16_t markerAglCm,
    uint16_t alignmentRadiusCm)
{
    // Use the same continuous geometry as descent control: retain normal NAV
    // response near image centre and add urgency only as descent is withheld.
    return 1.0f + (1.0f - markerGuidanceLandingDescentScale(
        horizontalOffsetCm,
        markerAglCm,
        alignmentRadiusCm));
}

static inline float markerGuidanceLostTargetDescentScale(
    bool correctionWindowActive,
    bool markerTargetHeld,
    bool positionEstimateUsable,
    float currentNorthCm,
    float currentEastCm,
    float targetNorthCm,
    float targetEastCm,
    uint16_t lastMarkerAglCm,
    uint16_t alignmentRadiusCm)
{
    if (!correctionWindowActive || !markerTargetHeld || !positionEstimateUsable) {
        return 1.0f;
    }

    const float northErrorCm = targetNorthCm - currentNorthCm;
    const float eastErrorCm = targetEastCm - currentEastCm;
    const float horizontalOffsetCm = sqrtf(
        (northErrorCm * northErrorCm) + (eastErrorCm * eastErrorCm));

    return markerGuidanceLandingDescentScale(
        horizontalOffsetCm,
        lastMarkerAglCm,
        alignmentRadiusCm);
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
    bool inavAglTrusted,
    float inavAglCm,
    bool lastFreshMarkerWasLow)
{
    if (retryMinAltitudeCm == 0) {
        return false;
    }

    const bool inavAglIsLow = inavAglTrusted && inavAglCm >= 0.0f && inavAglCm <= retryMinAltitudeCm;
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
