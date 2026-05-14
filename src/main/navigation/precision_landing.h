#pragma once

#ifdef USE_PRECISION_LANDING

#include <stdbool.h>
#include <stdint.h>

#include "navigation/navigation.h"
#include "navigation/navigation_private.h"

typedef enum {
    PREC_LAND_FRAME_BODY_FRD = 0,
} precisionLandingFrame_e;

typedef enum {
    PREC_LAND_REASON_OK = 0,
    PREC_LAND_REASON_NOT_ENABLED,
    PREC_LAND_REASON_LOW_CONFIDENCE,
    PREC_LAND_REASON_BAD_FRAME,
    PREC_LAND_REASON_STALE,
    PREC_LAND_REASON_OFFSET_TOO_LARGE,
    PREC_LAND_REASON_NOT_MC_PROFILE,
    PREC_LAND_REASON_NOT_IN_POSHOLD_OR_LAND,
    PREC_LAND_REASON_FAILSAFE,
    PREC_LAND_REASON_INVALID_TARGET,
    PREC_LAND_REASON_NOT_ARMED,
} precisionLandingReason_e;

typedef enum {
    PREC_LAND_IDLE = 0,
    PREC_LAND_STANDBY,
    PREC_LAND_POSHOLD_CORRECTION,
    PREC_LAND_LAND_CORRECTION,
    PREC_LAND_TARGET_LOST_HOLD,
    PREC_LAND_CLIMB_AND_RETRY,
    PREC_LAND_FALLBACK_NORMAL_LAND,
    PREC_LAND_DONE,
} precisionLandingState_e;

typedef enum {
    PREC_LAND_LAND_CTRL_NONE = 0,
    PREC_LAND_LAND_CTRL_HOLD,
    PREC_LAND_LAND_CTRL_CLIMB,
} precisionLandingLandControlMode_e;

typedef struct {
    uint8_t valid;
    uint8_t confidence;
    uint8_t frame;
    int16_t offsetForwardCm;
    int16_t offsetRightCm;
    uint16_t distanceCm;
    uint32_t timestampMs;
} precisionLandingTargetUpdate_t;

typedef struct {
    uint8_t accepted;
    uint8_t usedNow;
    uint8_t navPrecisionState;
    uint8_t reason;
    uint8_t retryCount;
} precisionLandingMspResponse_t;

typedef struct {
    precisionLandingLandControlMode_e mode;
    float rateCmS;
} precisionLandingLandControl_t;

void precisionLandingReset(void);
void precisionLandingUpdate(navigationFSMStateFlags_t navStateFlags, timeUs_t currentTimeUs);
void precisionLandingApplyHorizontalVelocityCorrection(float *velX, float *velY);
void precisionLandingGetLandControl(precisionLandingLandControl_t *controlOut);
navSystemStatus_State_e precisionLandingOverrideNavStatusState(navSystemStatus_State_e defaultState);
bool precisionLandingHandleMspTargetUpdate(const precisionLandingTargetUpdate_t *update, precisionLandingMspResponse_t *responseOut);

#endif
