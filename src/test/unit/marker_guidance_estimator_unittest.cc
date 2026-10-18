#include <cmath>
#include <cstring>
#include <limits>
#include "gtest/gtest.h"

extern "C" {
#include "platform.h"
#include "build/debug.h"
#include "navigation/navigation.h"
#define _Static_assert static_assert
#include "navigation/navigation_private.h"
#include "navigation/navigation_pos_estimator_private.h"
#include "navigation/navigation_pos_estimator_pl.h"
#include "navigation/marker_guidance.h"
#undef _Static_assert
#include "sensors/opflow.h"

navigationPosEstimator_t posEstimator;
positionEstimationConfig_t positionEstimationConfig_System;
opflow_t opflow;
static bool plActive;
static bool headingValid;
static float rotation[3][3];
static markerGuidanceEstimatorDebug_t diagnostics;
static markerGuidanceVelocityObservation_t observation;
static bool observationValid;

bool markerGuidanceUsesLocalVelocity(void) { return plActive; }
bool isImuHeadingValid(void) { return headingValid; }
bool markerGuidanceGetVelocityObservation(markerGuidanceVelocityObservation_t *out)
{
    *out = observation;
    return observationValid;
}
void markerGuidanceSetEstimatorDebug(const markerGuidanceEstimatorDebug_t *value) { diagnostics = *value; }
void imuTransformVectorBodyToEarth(fpVector3_t *value)
{
    const fpVector3_t input = *value;
    for (unsigned i = 0; i < 3; ++i) {
        value->v[i] = 0;
        for (unsigned j = 0; j < 3; ++j) value->v[i] += rotation[i][j] * input.v[j];
    }
}
}

class MarkerEstimatorTest : public ::testing::Test {
protected:
    estimationContext_t ctx;
    void SetUp() override
    {
        memset(&posEstimator, 0, sizeof(posEstimator));
        memset(&opflow, 0, sizeof(opflow));
        memset(&ctx, 0, sizeof(ctx));
        plActive = false;
        estimationCalculateCorrection_XY_PL(&ctx, 0);
        observation = {};
        observationValid = false;
        memset(rotation, 0, sizeof(rotation));
        rotation[0][0] = rotation[2][2] = 1;
        rotation[1][1] = -1; // INAV body to NEU convention at zero attitude.
        plActive = headingValid = true;
        debugMode = DEBUG_MARKER_GUIDANCE;
        positionEstimationConfig_System.max_surface_altitude = 300;
        positionEstimationConfig_System.w_xy_flow_v = 2;
        posEstimator.surface.sampleValid = true;
        posEstimator.surface.reliability = 1;
        posEstimator.surface.alt = 200;
        posEstimator.flow.lastUpdateTime = 1000000;
        opflow.lastValidUpdate = 1000000;
        opflow.isHwHealty = true;
        opflow.flowQuality = OPFLOW_QUALITY_VALID;
        opflow.rawQuality = 100;
        opflow.updateDt = 0.01f;
        ctx.newFlags = EST_GPS_XY_VALID | EST_XY_VALID | EST_Z_VALID | EST_FLOW_VALID | EST_SURFACE_VALID;
        ctx.dt = 0.02f;
        ctx.inertialPredictionXYValid = true;
        ctx.estPosCorr = (fpVector3_t){{1, 2, 3}};
        ctx.estVelCorr = (fpVector3_t){{4, 5, 6}};
        ctx.accBiasCorr = (fpVector3_t){{7, 8, 9}};
    }
    bool update() { return estimationCalculateCorrection_XY_PL(&ctx, 1000000); }
    uint32_t now = 0;
    bool step(float acceleration = 0)
    {
        now += 20000;
        posEstimator.imu.accelNEU.x = acceleration;
        if (observationValid) observation.captureAgeMs += 20;
        ctx.applyCorrectionsXY = false;
        ctx.estVelCorr = (fpVector3_t){{4, 5, 6}};
        ctx.accBiasCorr = (fpVector3_t){{7, 8, 9}};
        return estimationCalculateCorrection_XY_PL(&ctx, now);
    }
    void pose(uint64_t capture, float north, unsigned age = 100)
    {
        observationValid = true;
        observation.captureTimeUs = capture;
        observation.offset = (fpVector2_t){north, 0};
        observation.captureAgeMs = age;
        observation.maxAgeMs = 500;
        observation.sequence++;
    }
    void warmup()
    {
        ctx.newFlags &= ~EST_FLOW_VALID;
        for (int i = 0; i < 30; i++) step();
        pose(1000000, 100);
        step();
        for (int i = 0; i < 9; i++) step();
    }
};

TEST_F(MarkerEstimatorTest, MarkerFallbackChangesOnlyXYVelocityOncePerWindow)
{
    warmup();
    pose(1200000, 90);
    ASSERT_TRUE(step());
    EXPECT_EQ(MARKER_VELOCITY_SELECTED, diagnostics.markerReason);
    EXPECT_TRUE(diagnostics.markerApplied);
    EXPECT_NEAR(50, diagnostics.markerVelocity.x, 0.001);
    EXPECT_NEAR(50 * 0.4f / 1.4f, ctx.estVelCorr.x, 0.001);
    EXPECT_NEAR(0.2f, ctx.markerUpdateDt, 0.0001);
    EXPECT_FLOAT_EQ(6, ctx.estVelCorr.z);
    EXPECT_FLOAT_EQ(9, ctx.accBiasCorr.z);
    EXPECT_FLOAT_EQ(1, ctx.estPosCorr.x);
    EXPECT_FLOAT_EQ(2, ctx.estPosCorr.y);
    EXPECT_FLOAT_EQ(0, ctx.accBiasCorr.x);
    ASSERT_TRUE(step());
    EXPECT_FALSE(diagnostics.markerApplied);
    EXPECT_FLOAT_EQ(0, ctx.estVelCorr.x);
    EXPECT_FALSE(ctx.applyCorrectionsXY);
    EXPECT_FLOAT_EQ(0, ctx.markerUpdateDt);
}

TEST_F(MarkerEstimatorTest, DelayedIntervalIsPropagatedWithAccelerationNotGpsVelocity)
{
    ctx.newFlags &= ~EST_FLOW_VALID;
    for (int i = 0; i < 30; i++) step(100);
    pose(1000000, 100);
    step(100);
    for (int i = 0; i < 9; i++) step(100);
    pose(1200000, 90, 100);
    posEstimator.gps.vel.x = -300;
    ASSERT_TRUE(step(100));
    // 50 cm/s interval mean + 100 cm/s^2 * (120 ms age + 100 ms half-window).
    EXPECT_NEAR(72, diagnostics.markerVelocity.x, 0.02);
    EXPECT_NEAR(72 * 0.4f / 1.4f, ctx.estVelCorr.x, 0.02);
}

TEST_F(MarkerEstimatorTest, FlowHasPriorityAndDoesNotReplayOldMarkerCorrection)
{
    warmup();
    pose(1200000, 90);
    ctx.newFlags |= EST_FLOW_VALID;
    opflow.updateDt = 0.01f;
    ASSERT_TRUE(step());
    EXPECT_TRUE(diagnostics.markerReady);
    EXPECT_FALSE(diagnostics.markerSelected);
    EXPECT_FALSE(diagnostics.markerApplied);
    ctx.newFlags &= ~EST_FLOW_VALID;
    ASSERT_TRUE(step());
    EXPECT_TRUE(diagnostics.markerSelected);
    EXPECT_FALSE(diagnostics.markerApplied);
    EXPECT_FLOAT_EQ(0, ctx.estVelCorr.x);
}

TEST_F(MarkerEstimatorTest, StaleLostTakeoverAndInvalidImuReleaseToGps)
{
    for (int fault = 0; fault < 5; fault++) {
        SetUp(); now = 0;
        warmup(); pose(1200000, 90); ASSERT_TRUE(step());
        switch (fault) {
        case 0: observation.captureAgeMs = 501; break;
        case 1: observationValid = false; break;
        case 2: plActive = false; break;
        case 3: ctx.inertialPredictionXYValid = false; break;
        case 4: ctx.newFlags &= ~EST_GPS_XY_VALID; break;
        }
        EXPECT_FALSE(step()) << fault;
        EXPECT_FLOAT_EQ(4, ctx.estVelCorr.x);
        EXPECT_FLOAT_EQ(7, ctx.accBiasCorr.x);
        EXPECT_FALSE(diagnostics.markerSelected);
    }
}

TEST_F(MarkerEstimatorTest, RejectsMissingHistoryInnovationAndClockRestart)
{
    warmup();
    pose(1200000, -1000);
    EXPECT_FALSE(step());
    EXPECT_EQ(MARKER_VELOCITY_INNOVATION, diagnostics.markerReason);
    EXPECT_FLOAT_EQ(4, ctx.estVelCorr.x);
    EXPECT_FALSE(step()); // Do not retry the rejected measurement.
    pose(1, 100);
    EXPECT_FALSE(step());
    EXPECT_FALSE(diagnostics.markerReady);
    for (int i = 0; i < 9; i++) step();
    pose(200001, 90, 490);
    // Leave reference valid but the old capture interval beyond retained history.
    observation.captureAgeMs = 10000;
    EXPECT_FALSE(step());
    EXPECT_FALSE(diagnostics.markerSelected);
}

TEST_F(MarkerEstimatorTest, WindowNeedsDisplacementTimeAndResetsOnGap)
{
    warmup();
    pose(1100000, 95);
    EXPECT_FALSE(step());
    EXPECT_FALSE(diagnostics.markerReady);
    observationValid = false; step();
    pose(1200000, 90); EXPECT_FALSE(step());
    pose(1300000, 85); EXPECT_FALSE(step());
    EXPECT_FALSE(diagnostics.markerReady);
}

TEST_F(MarkerEstimatorTest, InertialHistoryHandlesWrapAndRebase)
{
    now = UINT32_MAX - 700000;
    ctx.newFlags &= ~EST_FLOW_VALID;
    for (int i = 0; i < 80; i++) step(100);
    pose(1000000, 100); step(100);
    for (int i = 0; i < 9; i++) step(100);
    pose(1200000, 90); ASSERT_TRUE(step(100));
    EXPECT_NEAR(72, diagnostics.markerVelocity.x, 0.03);
    for (int i = 0; i < 9; i++) step(100);
    pose(1400000, 76); ASSERT_TRUE(step(100));
    EXPECT_NEAR(92, diagnostics.markerVelocity.x, 0.03);
}

TEST_F(MarkerEstimatorTest, CameraCadenceJitterDoesNotAddAnExtraFrameOfDelay)
{
    warmup(); pose(1199988, 90);
    ASSERT_TRUE(step());
    EXPECT_NEAR(50.003, diagnostics.markerVelocity.x, 0.01);
}

TEST_F(MarkerEstimatorTest, InsHistoryMustCoverEntireExposureInterval)
{
    ctx.newFlags &= ~EST_FLOW_VALID;
    pose(1000000, 100, 300); step();
    for (int i = 0; i < 9; i++) step();
    pose(1200000, 90, 300);
    EXPECT_FALSE(step());
    EXPECT_EQ(MARKER_VELOCITY_NO_HISTORY, diagnostics.markerReason);
    EXPECT_FLOAT_EQ(4, ctx.estVelCorr.x);
}

TEST_F(MarkerEstimatorTest, NewShortFramesCannotKeepAnOldVelocityAliveForever)
{
    warmup(); pose(1200000, 90); ASSERT_TRUE(step());
    for (int i = 0; i < 30; i++) {
        // A broken upstream clock barely advances despite fresh arrivals.
        pose(1200001 + i, 90, 0);
        step();
    }
    EXPECT_FALSE(diagnostics.markerReady);
    EXPECT_FALSE(diagnostics.markerSelected);
    EXPECT_FLOAT_EQ(4, ctx.estVelCorr.x);
}

TEST_F(MarkerEstimatorTest, ExcessiveTiltAndSchedulerGapDiscardOldHistory)
{
    warmup(); pose(1200000, 90); ASSERT_TRUE(step());
    rotation[2][2] = 0.5f;
    EXPECT_FALSE(step());
    rotation[2][2] = 1;
    EXPECT_FALSE(step());
    now += 1000000;
    pose(1400000, 80);
    EXPECT_FALSE(step());
    EXPECT_FALSE(diagnostics.markerReady);
}

TEST_F(MarkerEstimatorTest, MarkerFusionDoesNotDependOnDebugMode)
{
    warmup(); pose(1200000, 90); ASSERT_TRUE(step());
    const auto expected = ctx;
    SetUp(); now = 0; debugMode = DEBUG_NONE;
    warmup(); pose(1200000, 90); ASSERT_TRUE(step());
    EXPECT_EQ(0, memcmp(&expected, &ctx, sizeof(ctx)));
}

TEST_F(MarkerEstimatorTest, EarthFrameMarkerVelocityDoesNotRotateAgainWithYaw)
{
    warmup();
    rotation[0][0] = -1;
    rotation[1][1] = 1;
    pose(1200000, 90);
    observation.offset.y = -12;
    ASSERT_TRUE(step());
    EXPECT_NEAR(50, diagnostics.markerVelocity.x, 0.001);
    EXPECT_NEAR(60, diagnostics.markerVelocity.y, 0.001);
    EXPECT_GT(ctx.estVelCorr.x, 0);
    EXPECT_GT(ctx.estVelCorr.y, 0);
}

TEST_F(MarkerEstimatorTest, ZeroGainAndNonFiniteInertiaNeverSuppressGpsForMarker)
{
    warmup(); pose(1200000, 90);
    positionEstimationConfig_System.w_xy_flow_v = 0;
    EXPECT_FALSE(step());
    EXPECT_TRUE(diagnostics.markerReady);
    EXPECT_FLOAT_EQ(4, ctx.estVelCorr.x);
    positionEstimationConfig_System.w_xy_flow_v = 2;
    EXPECT_FALSE(step(std::numeric_limits<float>::quiet_NaN()));
    EXPECT_FALSE(diagnostics.markerReady);
    EXPECT_FLOAT_EQ(4, ctx.estVelCorr.x);
}

TEST_F(MarkerEstimatorTest, FlowReplacesOnlyHorizontalVelocityAndBias)
{
    posEstimator.flow.flowRate[Y] = -0.5f;
    posEstimator.flow.flowRate[X] = -0.25f;
    ASSERT_TRUE(update());
    EXPECT_NEAR(100, diagnostics.flowVelocity.x, 0.001);
    EXPECT_NEAR(50, diagnostics.flowVelocity.y, 0.001);
    EXPECT_NEAR(100 * 0.02 / 1.02, ctx.estVelCorr.x, 0.001);
    EXPECT_FLOAT_EQ(6, ctx.estVelCorr.z);
    EXPECT_FLOAT_EQ(1, ctx.estPosCorr.x);
    EXPECT_FLOAT_EQ(2, ctx.estPosCorr.y);
    EXPECT_FLOAT_EQ(3, ctx.estPosCorr.z);
    EXPECT_FLOAT_EQ(0, ctx.accBiasCorr.x);
    EXPECT_FLOAT_EQ(0, ctx.accBiasCorr.y);
    EXPECT_FLOAT_EQ(9, ctx.accBiasCorr.z);
    EXPECT_FLOAT_EQ(0, opflow.updateDt);
    EXPECT_FLOAT_EQ(posEstimator.est.pos.x + ctx.estPosCorr.x, posEstimator.est.flowCoordinates[X]);
    EXPECT_FLOAT_EQ(posEstimator.est.pos.y + ctx.estPosCorr.y, posEstimator.est.flowCoordinates[Y]);
    EXPECT_TRUE(diagnostics.applied);
    ctx.estVelCorr.x = 123; // New GPS sample between flow frames must not fight flow.
    ctx.applyCorrectionsXY = false;
    ASSERT_TRUE(update());
    EXPECT_FLOAT_EQ(0, ctx.estVelCorr.x);
    EXPECT_FALSE(diagnostics.applied);
    EXPECT_FALSE(ctx.applyCorrectionsXY);
}

TEST_F(MarkerEstimatorTest, RejectionLeavesLegacyCorrectionsUntouched)
{
    for (unsigned fault = 0; fault < 17; ++fault) {
        SetUp();
        switch (fault) {
        case 0: plActive = false; break;
        case 1: ctx.newFlags &= ~EST_FLOW_VALID; break;
        case 2: ctx.newFlags &= ~EST_SURFACE_VALID; break;
        case 3: posEstimator.surface.sampleValid = false; break;
        case 4: posEstimator.surface.reliability = 0.5f; break;
        case 5: posEstimator.surface.alt = 8; break;
        case 6: posEstimator.surface.alt = 301; break;
        case 7: rotation[2][2] = 0.5f; break;
        case 8: headingValid = false; break;
        case 9: ctx.newFlags &= ~EST_Z_VALID; break;
        case 10: ctx.newFlags &= ~EST_XY_VALID; break;
        case 11: opflow.updateDt = 1; break;
        case 12: posEstimator.flow.flowRate[X] = std::numeric_limits<float>::quiet_NaN(); break;
        case 13: positionEstimationConfig_System.w_xy_flow_v = 0; break;
        case 14: ctx.newFlags &= ~EST_GPS_XY_VALID; break;
        case 15: opflow.updateDt = std::numeric_limits<float>::quiet_NaN(); break;
        case 16: posEstimator.est.vel.x = std::numeric_limits<float>::quiet_NaN(); break;
        }
        const estimationContext_t before = ctx;
        EXPECT_FALSE(update()) << fault;
        EXPECT_EQ(0, memcmp(&before, &ctx, sizeof(ctx))) << fault;
        EXPECT_FALSE(diagnostics.applied);
    }
}

TEST_F(MarkerEstimatorTest, ExcessiveInnovationFallsBackRatherThanSnappingVelocity)
{
    posEstimator.flow.flowRate[X] = 10;
    EXPECT_FALSE(update());
    EXPECT_EQ(MARKER_ESTIMATOR_INNOVATION, diagnostics.reason);
    EXPECT_FLOAT_EQ(5, ctx.estVelCorr.y);
}

TEST_F(MarkerEstimatorTest, TiltedVerticalDescentDoesNotBecomeHorizontalMotion)
{
    const float c = std::cos(0.5f), s = std::sin(0.5f);
    rotation[0][0] = c;
    rotation[0][2] = s;
    rotation[2][0] = -s;
    rotation[2][2] = c;
    posEstimator.est.vel.z = -100;
    const float bodyX = -s * posEstimator.est.vel.z;
    posEstimator.flow.flowRate[Y] = -bodyX / (posEstimator.surface.alt / c);
    ASSERT_TRUE(update());
    EXPECT_NEAR(0, diagnostics.flowVelocity.x, 0.001);
    EXPECT_NEAR(0, diagnostics.flowVelocity.y, 0.001);
}

TEST_F(MarkerEstimatorTest, HeadingReversalChangesNorthAndEastTogether)
{
    rotation[0][0] = -1;
    rotation[1][1] = 1;
    posEstimator.flow.flowRate[Y] = -0.5f;
    posEstimator.flow.flowRate[X] = -0.25f;
    ASSERT_TRUE(update());
    EXPECT_NEAR(-100, diagnostics.flowVelocity.x, 0.001);
    EXPECT_NEAR(-50, diagnostics.flowVelocity.y, 0.001);
}

TEST_F(MarkerEstimatorTest, RecoveryUsesOnlyNewSampleTimeAndHasBoundedGain)
{
    posEstimator.flow.flowRate[Y] = -0.5f;
    opflow.updateDt = 2.0f;
    EXPECT_FALSE(update());
    EXPECT_EQ(MARKER_ESTIMATOR_SAMPLE_GAP, diagnostics.reason);
    opflow.updateDt = 0.01f;
    positionEstimationConfig_System.w_xy_flow_v = 100;
    ASSERT_TRUE(update());
    EXPECT_FLOAT_EQ(50, ctx.estVelCorr.x);
    EXPECT_LT(ctx.estVelCorr.x, diagnostics.flowVelocity.x);
}

TEST_F(MarkerEstimatorTest, DebugDoesNotChangeControlResults)
{
    posEstimator.flow.flowRate[Y] = -0.5f;
    ASSERT_TRUE(update());
    const estimationContext_t withDebug = ctx;
    SetUp();
    debugMode = DEBUG_NONE;
    posEstimator.flow.flowRate[Y] = -0.5f;
    ASSERT_TRUE(update());
    EXPECT_EQ(0, memcmp(&withDebug, &ctx, sizeof(ctx)));
}

TEST_F(MarkerEstimatorTest, DebugAgeTracksSensorSampleNotEstimatorTopic)
{
    opflow.lastValidUpdate = 987001;
    ASSERT_TRUE(update());
    EXPECT_EQ(12, diagnostics.flowAgeMs);
    EXPECT_TRUE(diagnostics.flowHealthy);
    EXPECT_TRUE(diagnostics.flowQualityValid);
}

TEST_F(MarkerEstimatorTest, DebugDistinguishesFreshLowQualityFromTransportTimeout)
{
    ctx.newFlags &= ~EST_FLOW_VALID;
    opflow.flowQuality = OPFLOW_QUALITY_INVALID;
    opflow.rawQuality = 0;
    const estimationContext_t before = ctx;
    EXPECT_FALSE(update());
    EXPECT_EQ(MARKER_ESTIMATOR_FLOW_INVALID, diagnostics.reason);
    EXPECT_EQ(0, diagnostics.quality);
    EXPECT_EQ(0, diagnostics.flowAgeMs);
    EXPECT_TRUE(diagnostics.flowHealthy);
    EXPECT_FALSE(diagnostics.flowQualityValid);

    opflow.lastValidUpdate = 750000;
    opflow.isHwHealty = false;
    EXPECT_FALSE(update());
    EXPECT_EQ(MARKER_ESTIMATOR_FLOW_INVALID, diagnostics.reason);
    EXPECT_EQ(0, diagnostics.quality);
    EXPECT_EQ(250, diagnostics.flowAgeMs);
    EXPECT_FALSE(diagnostics.flowHealthy);
    EXPECT_FALSE(diagnostics.flowQualityValid);
    EXPECT_EQ(0, memcmp(&before, &ctx, sizeof(ctx)));

    opflow.lastValidUpdate = 998000;
    opflow.isHwHealty = true;
    opflow.flowQuality = OPFLOW_QUALITY_VALID;
    opflow.rawQuality = 80;
    ctx.newFlags |= EST_FLOW_VALID;
    ASSERT_TRUE(update());
    EXPECT_EQ(2, diagnostics.flowAgeMs);
    EXPECT_EQ(80, diagnostics.quality);
    EXPECT_TRUE(diagnostics.flowHealthy);
    EXPECT_TRUE(diagnostics.flowQualityValid);
}

TEST_F(MarkerEstimatorTest, DebugAgeIsUnknownBeforeFirstSample)
{
    opflow.lastValidUpdate = 0;
    opflow.isHwHealty = false;
    opflow.flowQuality = OPFLOW_QUALITY_INVALID;
    ctx.newFlags &= ~EST_FLOW_VALID;
    EXPECT_FALSE(update());
    EXPECT_EQ(UINT16_MAX, diagnostics.flowAgeMs);
    EXPECT_FALSE(diagnostics.flowHealthy);
}

TEST_F(MarkerEstimatorTest, DebugAgeSaturatesRatherThanWrapping)
{
    opflow.lastValidUpdate = 1;
    estimationCalculateCorrection_XY_PL(&ctx, 70000000);
    EXPECT_EQ(UINT16_MAX, diagnostics.flowAgeMs);
}

TEST_F(MarkerEstimatorTest, DebugAgeHandlesTimestampWrapAndSampleAtZero)
{
    opflow.lastValidUpdate = TIMEUS_MAX - 1499;
    estimationCalculateCorrection_XY_PL(&ctx, 500);
    EXPECT_EQ(2, diagnostics.flowAgeMs);

    opflow.lastValidUpdate = 0;
    estimationCalculateCorrection_XY_PL(&ctx, 2000);
    EXPECT_EQ(2, diagnostics.flowAgeMs);
}
