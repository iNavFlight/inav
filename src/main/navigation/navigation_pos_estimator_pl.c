#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "platform.h"

#ifdef USE_MARKER_GUIDANCE
#include "build/debug.h"
#include "common/maths.h"
#include "flight/imu.h"
#include "navigation/navigation.h"
#include "navigation/navigation_private.h"
#include "navigation/navigation_pos_estimator_private.h"
#include "navigation/navigation_pos_estimator_pl.h"
#include "navigation/marker_guidance.h"
#include "sensors/opflow.h"

// MTF-01 flow needs >8 cm. Keep the generic, GPS-free FLOW path unchanged.
#define PL_FLOW_MIN_HEIGHT_CM 8.0f
#define PL_FLOW_MIN_TILT_COS 0.70710678f

// A 200 ms displacement reduces the 1 cm wire quantization to 5 cm/s.
#define PL_VELOCITY_WINDOW_US 200000U
#define PL_IMU_HISTORY_STEP_US 20000U
#define PL_IMU_HISTORY_COUNT 64

typedef struct {
    uint32_t timeUs;
    fpVector2_t velocity;
} plInertialSample_t;

static struct {
    plInertialSample_t history[PL_IMU_HISTORY_COUNT];
    fpVector2_t inertialVelocity;
    uint32_t lastTimeUs;
    uint8_t count;
    uint8_t next;
    markerGuidanceVelocityObservation_t anchor;
    uint32_t sequence;
    bool anchorValid;
    fpVector2_t observedVelocity;
    fpVector2_t observedInertialMean;
    uint32_t observationEndUs;
    float observationDt;
    bool ready;
    bool pending;
} plVelocity;

static void resetMarkerVelocity(void)
{
    // History storage need not be cleared: count controls every read.
    plVelocity.count = plVelocity.next = 0;
    plVelocity.inertialVelocity = (fpVector2_t){0};
    plVelocity.anchorValid = plVelocity.ready = plVelocity.pending = false;
}

static void recordMarkerInertialHistory(const estimationContext_t *ctx, uint32_t nowUs)
{
    if (plVelocity.count && nowUs - plVelocity.lastTimeUs > PL_VELOCITY_WINDOW_US) {
        resetMarkerVelocity();
    }
    plVelocity.lastTimeUs = nowUs;
    plVelocity.inertialVelocity.x += posEstimator.imu.accelNEU.x * ctx->dt;
    plVelocity.inertialVelocity.y += posEstimator.imu.accelNEU.y * ctx->dt;
    const unsigned last = (plVelocity.next + PL_IMU_HISTORY_COUNT - 1) % PL_IMU_HISTORY_COUNT;
    if (plVelocity.count && nowUs - plVelocity.history[last].timeUs < PL_IMU_HISTORY_STEP_US) {
        return;
    }
    if (plVelocity.next == 0) {
        // Rebase once per ring to avoid accumulating float precision loss on long flights.
        for (unsigned i = 0; i < plVelocity.count; i++) {
            plVelocity.history[i].velocity.x -= plVelocity.inertialVelocity.x;
            plVelocity.history[i].velocity.y -= plVelocity.inertialVelocity.y;
        }
        plVelocity.observedInertialMean.x -= plVelocity.inertialVelocity.x;
        plVelocity.observedInertialMean.y -= plVelocity.inertialVelocity.y;
        plVelocity.inertialVelocity = (fpVector2_t){0};
    }
    plVelocity.history[plVelocity.next] = (plInertialSample_t){nowUs, plVelocity.inertialVelocity};
    plVelocity.next = (plVelocity.next + 1) % PL_IMU_HISTORY_COUNT;
    plVelocity.count = MIN(plVelocity.count + 1, PL_IMU_HISTORY_COUNT);
}

static bool markerInertialIntervalMean(uint32_t nowUs, uint32_t endAgeUs, uint32_t intervalUs, fpVector2_t *mean)
{
    const uint32_t startAgeUs = endAgeUs + intervalUs;
    const unsigned oldest = (plVelocity.next + PL_IMU_HISTORY_COUNT - plVelocity.count) % PL_IMU_HISTORY_COUNT;
    if (!plVelocity.count || nowUs - plVelocity.history[oldest].timeUs < startAgeUs) {
        return false;
    }
    *mean = (fpVector2_t){0};
    plInertialSample_t previous = plVelocity.history[oldest];
    for (unsigned i = 1; i <= plVelocity.count; i++) {
        const plInertialSample_t sample = i == plVelocity.count ?
            (plInertialSample_t){nowUs, plVelocity.inertialVelocity} :
            plVelocity.history[(oldest + i) % PL_IMU_HISTORY_COUNT];
        const uint32_t olderAge = nowUs - previous.timeUs;
        const uint32_t youngerAge = nowUs - sample.timeUs;
        const uint32_t older = MIN(olderAge, startAgeUs);
        const uint32_t younger = MAX(youngerAge, endAgeUs);
        if (older > younger) {
            const float fraction = ((float)(olderAge - older) + (olderAge - younger)) /
                (2.0f * (olderAge - youngerAge));
            const float weight = (float)(older - younger) / intervalUs;
            mean->x += (previous.velocity.x + fraction * (sample.velocity.x - previous.velocity.x)) * weight;
            mean->y += (previous.velocity.y + fraction * (sample.velocity.y - previous.velocity.y)) * weight;
        }
        previous = sample;
    }
    return true;
}

static void updateMarkerVelocity(estimationContext_t *ctx, uint32_t nowUs, bool flowSelected,
    markerGuidanceEstimatorDebug_t *diagnostics)
{
    markerGuidanceVelocityObservation_t observation;
    if (!markerGuidanceGetVelocityObservation(&observation) || observation.captureAgeMs > observation.maxAgeMs) {
        plVelocity.anchorValid = plVelocity.ready = plVelocity.pending = false;
        diagnostics->markerReason = MARKER_VELOCITY_NO_SAMPLE;
        return;
    }
    diagnostics->markerReason = MARKER_VELOCITY_WARMUP;
    if (!plVelocity.anchorValid) {
        plVelocity.anchor = observation;
        plVelocity.sequence = observation.sequence;
        plVelocity.anchorValid = true;
    } else if (observation.sequence != plVelocity.sequence) {
        plVelocity.sequence = observation.sequence;
        const uint64_t interval = observation.captureTimeUs - plVelocity.anchor.captureTimeUs;
        if (observation.captureTimeUs <= plVelocity.anchor.captureTimeUs ||
            interval > PL_IMU_HISTORY_COUNT * PL_IMU_HISTORY_STEP_US) {
            plVelocity.anchor = observation;
            plVelocity.ready = plVelocity.pending = false;
        } else if (interval + PL_IMU_HISTORY_STEP_US >= PL_VELOCITY_WINDOW_US) {
            // Allow one history tick of camera-clock jitter around the nominal window.
            const float inverseDt = 1000000.0f / interval;
            plVelocity.observedVelocity = (fpVector2_t){
                .x = (plVelocity.anchor.offset.x - observation.offset.x) * inverseDt,
                .y = (plVelocity.anchor.offset.y - observation.offset.y) * inverseDt,
            };
            plVelocity.observationDt = interval * 1e-6f;
            plVelocity.observationEndUs = nowUs - observation.captureAgeMs * 1000U;
            plVelocity.ready = markerInertialIntervalMean(nowUs, observation.captureAgeMs * 1000U,
                interval, &plVelocity.observedInertialMean);
            plVelocity.pending = plVelocity.ready;
            plVelocity.anchor = observation;
            if (!plVelocity.ready) {
                diagnostics->markerReason = MARKER_VELOCITY_NO_HISTORY;
            }
        }
    }
    if (!plVelocity.ready || nowUs - plVelocity.observationEndUs > observation.maxAgeMs * 1000U) {
        plVelocity.ready = plVelocity.pending = false;
        return;
    }
    diagnostics->markerVelocity = (fpVector2_t){
        .x = plVelocity.observedVelocity.x + plVelocity.inertialVelocity.x - plVelocity.observedInertialMean.x,
        .y = plVelocity.observedVelocity.y + plVelocity.inertialVelocity.y - plVelocity.observedInertialMean.y,
    };
    diagnostics->markerResidual = (fpVector2_t){
        .x = diagnostics->markerVelocity.x - posEstimator.est.vel.x,
        .y = diagnostics->markerVelocity.y - posEstimator.est.vel.y,
    };
    const float innovationLimit = INAV_GPS_GLITCH_ACCEL * MS2S(INAV_FLOW_TIMEOUT_MS);
    if (!isfinite(diagnostics->markerResidual.x) || !isfinite(diagnostics->markerResidual.y) ||
        sq(diagnostics->markerResidual.x) + sq(diagnostics->markerResidual.y) > sq(innovationLimit)) {
        plVelocity.ready = plVelocity.pending = false;
        diagnostics->markerReason = MARKER_VELOCITY_INNOVATION;
        return;
    }
    diagnostics->markerReady = true;
    diagnostics->markerReason = MARKER_VELOCITY_READY;
    if (!flowSelected && positionEstimationConfig()->w_xy_flow_v > 0) {
        diagnostics->markerSelected = true;
        diagnostics->markerReason = MARKER_VELOCITY_SELECTED;
        ctx->estVelCorr.x = ctx->estVelCorr.y = 0;
        ctx->accBiasCorr.x = ctx->accBiasCorr.y = 0;
        posEstimator.est.flowCoordinates[X] = posEstimator.est.pos.x + ctx->estPosCorr.x;
        posEstimator.est.flowCoordinates[Y] = posEstimator.est.pos.y + ctx->estPosCorr.y;
        if (plVelocity.pending) {
            const float weightDt = positionEstimationConfig()->w_xy_flow_v * plVelocity.observationDt;
            const float alpha = weightDt / (1.0f + weightDt);
            ctx->estVelCorr.x = diagnostics->markerResidual.x * alpha;
            ctx->estVelCorr.y = diagnostics->markerResidual.y * alpha;
            ctx->markerUpdateDt = plVelocity.observationDt;
            ctx->applyCorrectionsXY = true;
            diagnostics->markerApplied = true;
        }
    }
    // Do not replay a measurement already superseded by healthy optical flow.
    plVelocity.pending = false;
}

bool estimationCalculateCorrection_XY_PL(estimationContext_t *ctx, timeUs_t currentTimeUs)
{
    const bool plActive = markerGuidanceUsesLocalVelocity();
    ctx->markerUpdateDt = 0;
    if (!plActive && debugMode != DEBUG_MARKER_GUIDANCE) {
        resetMarkerVelocity();
        return false;
    }
    fpVector3_t vertical = { .x = 0, .y = 0, .z = 1 };
    imuTransformVectorBodyToEarth(&vertical);
    const bool markerReferenceValid = plActive && isImuHeadingValid() && ctx->inertialPredictionXYValid &&
        vertical.z >= PL_FLOW_MIN_TILT_COS && (ctx->newFlags & EST_XY_VALID) && (ctx->newFlags & EST_Z_VALID) &&
        (ctx->newFlags & EST_GPS_XY_VALID) && isfinite(ctx->dt) && ctx->dt > 0 && ctx->dt <= 0.2f &&
        isfinite(posEstimator.imu.accelNEU.x) && isfinite(posEstimator.imu.accelNEU.y);
    if (markerReferenceValid) {
        recordMarkerInertialHistory(ctx, (uint32_t)currentTimeUs);
    } else {
        resetMarkerVelocity();
    }
    markerGuidanceEstimatorDebug_t diagnostics = {0};
    diagnostics.gpsVelocity = (fpVector2_t){ .x = posEstimator.gps.vel.x, .y = posEstimator.gps.vel.y };
    diagnostics.insVelocity = (fpVector2_t){ .x = posEstimator.est.vel.x, .y = posEstimator.est.vel.y };
    diagnostics.reason = MARKER_ESTIMATOR_UNAVAILABLE;
    diagnostics.flowAgeMs = UINT16_MAX;
    diagnostics.gpsValid = (ctx->newFlags & EST_GPS_XY_VALID) != 0;
    bool selected = false;

#if defined(USE_OPFLOW) && defined(USE_RANGEFINDER)
    diagnostics.quality = opflow.rawQuality;
    diagnostics.reliability = constrainf(posEstimator.surface.reliability * 100.0f, 0, 100);
    diagnostics.rangeCm = constrainf(posEstimator.surface.alt, 0, UINT16_MAX);
    // The estimator topic is refreshed even without a new sensor sample.
    // Use the driver's last successful update to expose transport dropouts.
    if (opflow.lastValidUpdate != 0 || opflow.isHwHealty) {
        diagnostics.flowAgeMs = MIN((currentTimeUs - opflow.lastValidUpdate) / 1000U, (timeUs_t)UINT16_MAX);
    }
    diagnostics.flowHealthy = opflow.isHwHealty;
    diagnostics.flowQualityValid = opflow.flowQuality == OPFLOW_QUALITY_VALID;
    const float sampleDt = opflow.updateDt;
    if (!(ctx->newFlags & EST_FLOW_VALID)) {
        diagnostics.reason = MARKER_ESTIMATOR_FLOW_INVALID;
    } else if (!(ctx->newFlags & EST_SURFACE_VALID) || !posEstimator.surface.sampleValid ||
        !(posEstimator.surface.reliability >= RANGEFINDER_RELIABILITY_HIGH_THRESHOLD) ||
        !(posEstimator.surface.alt > PL_FLOW_MIN_HEIGHT_CM) ||
        posEstimator.surface.alt > positionEstimationConfig()->max_surface_altitude) {
        diagnostics.reason = MARKER_ESTIMATOR_RANGE_INVALID;
    } else if (!(vertical.z >= PL_FLOW_MIN_TILT_COS)) {
        diagnostics.reason = MARKER_ESTIMATOR_TILT;
    } else if (!isImuHeadingValid() || !(ctx->newFlags & EST_XY_VALID) || !(ctx->newFlags & EST_Z_VALID) ||
        !(ctx->newFlags & EST_GPS_XY_VALID)) {
        diagnostics.reason = MARKER_ESTIMATOR_REFERENCE_INVALID;
    } else if (!isfinite(sampleDt) || sampleDt < 0 || sampleDt > MS2S(INAV_FLOW_TIMEOUT_MS)) {
        diagnostics.reason = MARKER_ESTIMATOR_SAMPLE_GAP;
    } else {
        // Surface altitude is vertical; flow measures motion at the camera's slant range.
        const float slantRangeCm = posEstimator.surface.alt / vertical.z;
        fpVector3_t velocity = {
            .x = -(posEstimator.flow.flowRate[Y] - posEstimator.flow.bodyRate[Y]) * slantRangeCm,
            .y = (posEstimator.flow.flowRate[X] - posEstimator.flow.bodyRate[X]) * slantRangeCm,
            .z = 0,
        };
        imuTransformVectorBodyToEarth(&velocity);
        // Solve the missing body-Z component using earth-Z velocity, rather than
        // rotating earth-Z as if it were body-Z during a tilted descent.
        const float bodyZ = (posEstimator.est.vel.z - velocity.z) / vertical.z;
        velocity.x += bodyZ * vertical.x;
        velocity.y += bodyZ * vertical.y;
        diagnostics.flowVelocity = (fpVector2_t){ .x = velocity.x, .y = velocity.y };
        const float residualX = velocity.x - posEstimator.est.vel.x;
        const float residualY = velocity.y - posEstimator.est.vel.y;
        const float innovationLimit = INAV_GPS_GLITCH_ACCEL * MS2S(INAV_FLOW_TIMEOUT_MS);

        if (!isfinite(velocity.x) || !isfinite(velocity.y) || !isfinite(residualX) || !isfinite(residualY) ||
            sq(residualX) + sq(residualY) > sq(innovationLimit)) {
            diagnostics.reason = MARKER_ESTIMATOR_INNOVATION;
        } else if (!plActive) {
            diagnostics.reason = MARKER_ESTIMATOR_INACTIVE;
        } else if (!(positionEstimationConfig()->w_xy_flow_v > 0)) {
            diagnostics.reason = MARKER_ESTIMATOR_GAIN_DISABLED;
        } else {
            selected = true;
            diagnostics.reason = MARKER_ESTIMATOR_FLOW_SELECTED;
            // Suppress GPS velocity/bias corrections also BETWEEN flow samples.
            // Keep its position/EPE correction and all vertical corrections intact.
            ctx->estVelCorr.x = ctx->estVelCorr.y = 0;
            ctx->accBiasCorr.x = ctx->accBiasCorr.y = 0;
            // Keep a continuous origin if GPS is subsequently lost and the
            // existing GPS-free FLOW/dead-reckoning path takes over.
            posEstimator.est.flowCoordinates[X] = posEstimator.est.pos.x + ctx->estPosCorr.x;
            posEstimator.est.flowCoordinates[Y] = posEstimator.est.pos.y + ctx->estPosCorr.y;
            if (sampleDt > 0) {
                const float weightDt = positionEstimationConfig()->w_xy_flow_v * sampleDt;
                const float alpha = weightDt / (1.0f + weightDt);
                ctx->estVelCorr.x = residualX * alpha;
                ctx->estVelCorr.y = residualY * alpha;
                ctx->applyCorrectionsXY = true;
                posEstimator.flow.updateDt = sampleDt;
                opflow.updateDt = 0;
                diagnostics.applied = true;
            }
        }
    }
#else
    UNUSED(ctx);
    UNUSED(currentTimeUs);
#endif
    if (markerReferenceValid) {
        updateMarkerVelocity(ctx, (uint32_t)currentTimeUs, selected, &diagnostics);
        selected |= diagnostics.markerSelected;
    }
    if (debugMode == DEBUG_MARKER_GUIDANCE) {
        markerGuidanceSetEstimatorDebug(&diagnostics);
    }
    return selected;
}
#endif
