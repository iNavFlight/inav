#pragma once

#include <math.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define MARKER_GUIDANCE_MSP_PAYLOAD_SIZE 8U
#define MARKER_GUIDANCE_MAX_YAW_ERROR_DECIDEG 1800

typedef enum {
    MARKER_GUIDANCE_CONTEXT_NONE = 0,
    MARKER_GUIDANCE_CONTEXT_POSHOLD,
    MARKER_GUIDANCE_CONTEXT_LAND,
} markerGuidanceContext_e;

typedef struct {
    int16_t offsetForwardCm;
    int16_t offsetRightCm;
    int16_t yawErrorDeciDeg;
    uint16_t markerAglCm;
} markerGuidancePoseUpdate_t;

typedef struct {
    float offsetNorthCm;
    float offsetEastCm;
    int32_t targetHeadingCd;
    uint16_t markerAglCm;
} markerGuidanceResolvedPose_t;

static inline bool markerGuidanceMspPayloadSizeIsValid(size_t dataSize)
{
    return dataSize == MARKER_GUIDANCE_MSP_PAYLOAD_SIZE;
}

static inline markerGuidancePoseUpdate_t markerGuidanceDecodeMspWords(
    uint16_t offsetForwardRaw,
    uint16_t offsetRightRaw,
    uint16_t yawErrorRaw,
    uint16_t markerAglRaw)
{
    const markerGuidancePoseUpdate_t update = {
        .offsetForwardCm = (int16_t)offsetForwardRaw,
        .offsetRightCm = (int16_t)offsetRightRaw,
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

    const int64_t forward = update->offsetForwardCm;
    const int64_t right = update->offsetRightCm;
    return (uint32_t)((forward * forward) + (right * right));
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

static inline bool markerGuidanceTryResolvePose(
    const markerGuidancePoseUpdate_t *update,
    uint16_t maxOffsetCm,
    float cosYaw,
    float sinYaw,
    int32_t currentHeadingCd,
    markerGuidanceResolvedPose_t *resolvedOut)
{
    if (!resolvedOut || !markerGuidancePoseIsValid(update, maxOffsetCm)) {
        return false;
    }

    const markerGuidanceResolvedPose_t resolved = {
        .offsetNorthCm = update->offsetForwardCm * cosYaw - update->offsetRightCm * sinYaw,
        .offsetEastCm = update->offsetForwardCm * sinYaw + update->offsetRightCm * cosYaw,
        .targetHeadingCd = markerGuidanceWrapHeadingCd(currentHeadingCd + ((int32_t)update->yawErrorDeciDeg * 10)),
        .markerAglCm = update->markerAglCm,
    };

    *resolvedOut = resolved;
    return true;
}

static inline bool markerGuidanceComputeHorizontalCorrection(
    float offsetNorthCm,
    float offsetEastCm,
    float desiredVehicleRelNorthCm,
    float desiredVehicleRelEastCm,
    float radiusCm,
    float maxCorrectionSpeedCmS,
    float *velocityNorthOut,
    float *velocityEastOut)
{
    if (!velocityNorthOut || !velocityEastOut) {
        return false;
    }

    float errorNorth = offsetNorthCm + desiredVehicleRelNorthCm;
    float errorEast = offsetEastCm + desiredVehicleRelEastCm;
    float errorMagnitude = sqrtf((errorNorth * errorNorth) + (errorEast * errorEast));

    if (errorMagnitude <= 0.0f || (radiusCm > 0.0f && errorMagnitude <= radiusCm)) {
        return false;
    }

    if (radiusCm > 0.0f) {
        const float scale = (errorMagnitude - radiusCm) / errorMagnitude;
        errorNorth *= scale;
        errorEast *= scale;
        errorMagnitude -= radiusCm;
    }

    if (maxCorrectionSpeedCmS > 0.0f && errorMagnitude > maxCorrectionSpeedCmS) {
        const float scale = maxCorrectionSpeedCmS / errorMagnitude;
        errorNorth *= scale;
        errorEast *= scale;
    }

    *velocityNorthOut = errorNorth;
    *velocityEastOut = errorEast;
    return true;
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
    bool landHeadingLatched,
    int32_t latchedLandHeadingCd,
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

    if (context == MARKER_GUIDANCE_CONTEXT_LAND && landHeadingLatched) {
        *headingOut = latchedLandHeadingCd;
        return true;
    }

    return false;
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
