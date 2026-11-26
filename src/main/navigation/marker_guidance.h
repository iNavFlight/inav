#pragma once

#ifdef USE_MARKER_GUIDANCE

#include <stdbool.h>
#include <stdint.h>

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"
#include "navigation/marker_guidance_logic.h"

typedef enum {
    MARKER_GUIDANCE_REASON_OK = 0,
    MARKER_GUIDANCE_REASON_NOT_ENABLED,
    MARKER_GUIDANCE_REASON_STALE,
    MARKER_GUIDANCE_REASON_OFFSET_TOO_LARGE,
    MARKER_GUIDANCE_REASON_NOT_MC_PROFILE,
    MARKER_GUIDANCE_REASON_NOT_IN_POSHOLD_OR_LAND,
    MARKER_GUIDANCE_REASON_FAILSAFE,
    MARKER_GUIDANCE_REASON_INVALID_TARGET,
    MARKER_GUIDANCE_REASON_NOT_ARMED,
    MARKER_GUIDANCE_REASON_POSITION_UNAVAILABLE,
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

typedef markerGuidancePoseUpdate_t markerGuidanceTargetUpdate_t;

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
    float descentScale;
    float holdAltitudeCm;
    bool holdAltitudeValid;
} markerGuidanceLandControl_t;

typedef enum {
    MARKER_ESTIMATOR_FLOW_SELECTED = 0,
    MARKER_ESTIMATOR_INACTIVE,
    MARKER_ESTIMATOR_UNAVAILABLE,
    MARKER_ESTIMATOR_FLOW_INVALID,
    MARKER_ESTIMATOR_RANGE_INVALID,
    MARKER_ESTIMATOR_TILT,
    MARKER_ESTIMATOR_REFERENCE_INVALID,
    MARKER_ESTIMATOR_SAMPLE_GAP,
    MARKER_ESTIMATOR_INNOVATION,
    MARKER_ESTIMATOR_GAIN_DISABLED,
} markerGuidanceEstimatorReason_e;

typedef enum {
    MARKER_VELOCITY_INACTIVE = 0,
    MARKER_VELOCITY_WARMUP,
    MARKER_VELOCITY_NO_SAMPLE,
    MARKER_VELOCITY_NO_HISTORY,
    MARKER_VELOCITY_INNOVATION,
    MARKER_VELOCITY_READY,
    MARKER_VELOCITY_SELECTED,
} markerGuidanceVelocityReason_e;

typedef struct {
    fpVector2_t offset;
    uint64_t captureTimeUs;
    uint32_t sequence;
    uint32_t captureAgeMs;
    uint16_t maxAgeMs;
} markerGuidanceVelocityObservation_t;

typedef struct {
    fpVector2_t flowVelocity;
    fpVector2_t gpsVelocity;
    fpVector2_t insVelocity;
    uint16_t rangeCm;
    uint16_t flowAgeMs;
    uint8_t quality;
    uint8_t reliability;
    markerGuidanceEstimatorReason_e reason;
    bool applied;
    bool gpsValid;
    bool flowHealthy;
    bool flowQualityValid;
    fpVector2_t markerVelocity;
    fpVector2_t markerResidual;
    markerGuidanceVelocityReason_e markerReason;
    bool markerReady;
    bool markerSelected;
    bool markerApplied;
} markerGuidanceEstimatorDebug_t;

bool markerGuidanceUsesLocalVelocity(void);
bool markerGuidanceGetVelocityObservation(markerGuidanceVelocityObservation_t *observation);
void markerGuidanceSetEstimatorDebug(const markerGuidanceEstimatorDebug_t *diagnostics);

void markerGuidanceReset(void);
void markerGuidanceUpdate(navigationFSMStateFlags_t navStateFlags);
void markerGuidanceUpdateDebug(void);
bool markerGuidanceApplyHeadingOverride(int32_t *desiredYawCd);
bool markerGuidanceOwnsHeading(void);
bool markerGuidanceOwnsPositionTarget(void);
float markerGuidanceGetPositionResponseScale(void);
uint8_t markerGuidanceConsumePositionControllerRetargetAxes(void);
bool markerGuidanceGetActiveLandingHeading(int32_t *headingCdOut);
bool markerGuidanceRthPrelandingReady(void);
bool markerGuidanceGetActiveLandingPositionTarget(fpVector3_t *targetOut);
void markerGuidanceGetLandControl(markerGuidanceLandControl_t *controlOut, float nominalDescentCmS);
navSystemStatus_State_e markerGuidanceOverrideNavStatusState(navSystemStatus_State_e defaultState);
bool markerGuidanceHandleMspTargetUpdate(const markerGuidanceTargetUpdate_t *update, markerGuidanceMspResponse_t *responseOut);

#endif
