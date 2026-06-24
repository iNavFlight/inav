#pragma once

#ifdef USE_MARKER_GUIDANCE

#include <stdbool.h>
#include <stdint.h>

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

typedef enum {
    MARKER_GUIDANCE_REASON_OK = 0,
    MARKER_GUIDANCE_REASON_NOT_ENABLED,
    MARKER_GUIDANCE_REASON_LOW_CONFIDENCE,
    MARKER_GUIDANCE_REASON_BAD_FRAME,
    MARKER_GUIDANCE_REASON_STALE,
    MARKER_GUIDANCE_REASON_OFFSET_TOO_LARGE,
    MARKER_GUIDANCE_REASON_NOT_MC_PROFILE,
    MARKER_GUIDANCE_REASON_NOT_IN_POSHOLD_OR_LAND,
    MARKER_GUIDANCE_REASON_FAILSAFE,
    MARKER_GUIDANCE_REASON_INVALID_TARGET,
    MARKER_GUIDANCE_REASON_NOT_ARMED,
} markerGuidanceReason_e;

typedef enum {
    MARKER_GUIDANCE_IDLE = 0,
    MARKER_GUIDANCE_STANDBY,
    MARKER_GUIDANCE_POSHOLD_CORRECTION,
    MARKER_GUIDANCE_LAND_CORRECTION,
    MARKER_GUIDANCE_TARGET_LOST_HOLD,
    MARKER_GUIDANCE_CLIMB_AND_RETRY,
    MARKER_GUIDANCE_FALLBACK_NORMAL_LAND,
    MARKER_GUIDANCE_DONE,
} markerGuidanceState_e;

typedef enum {
    MARKER_GUIDANCE_LAND_CTRL_NONE = 0,
    MARKER_GUIDANCE_LAND_CTRL_HOLD,
    MARKER_GUIDANCE_LAND_CTRL_CLIMB,
} markerGuidanceLandControlMode_e;

typedef struct {
    int16_t offsetForwardCm;
    int16_t offsetRightCm;
} markerGuidanceTargetUpdate_t;

typedef struct {
    uint8_t accepted;
    uint8_t usedNow;
    uint8_t navGuidanceState;
    uint8_t reason;
    uint8_t retryCount;
} markerGuidanceMspResponse_t;

typedef struct {
    markerGuidanceLandControlMode_e mode;
    float rateCmS;
} markerGuidanceLandControl_t;

void markerGuidanceReset(void);
void markerGuidanceUpdate(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs);
void markerGuidanceApplyHorizontalVelocityCorrection(float *velX, float *velY);
void markerGuidanceGetLandControl(markerGuidanceLandControl_t *controlOut);
navSystemStatus_State_e markerGuidanceOverrideNavStatusState(navSystemStatus_State_e defaultState);
bool markerGuidanceHandleMspTargetUpdate(const markerGuidanceTargetUpdate_t *update, markerGuidanceMspResponse_t *responseOut);

#endif
