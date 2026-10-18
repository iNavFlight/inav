#include <cstring>
#include "gtest/gtest.h"

extern "C" {
#include "platform.h"
#include "fc/runtime_config.h"
#include "flight/imu.h"
#include "build/debug.h"
#define _Static_assert static_assert
#include "navigation/marker_guidance.h"
#undef _Static_assert
#include "navigation/navigation_vtol_mc_protection.h"

navConfig_t navConfig_System;
navigationPosControl_t posControl;
attitudeEulerAngles_t attitude;
uint32_t armingFlags;
uint32_t flightModeFlags;
uint32_t stateFlags;
bool isMixerTransitionMixing;

static timeMs_t nowMs;
static navigationFSMStateFlags_t currentFlags;
static bool recoveryActive;
static bool captureActive;
static bool transitionActive;

timeMs_t millis(void) { return nowMs; }
bool areSensorsCalibrating(void) { return false; }
navigationFSMStateFlags_t navGetCurrentStateFlags(void) { return currentFlags; }
const navEstimatedPosVel_t *navGetCurrentActualPositionAndVelocity(void) { return &posControl.actualState.abs; }
bool navigationRTHAllowsLanding(void) { return true; }
bool navigationVtolMcProtectionGuidanceRecoveryActive(void) { return recoveryActive; }
bool navigationVtolMcProtectionPositionCaptureActive(void) { return captureActive; }
bool navigationVtolMcProtectionPositionCapturePending(uint32_t) { return captureActive; }
bool mixerATIsActive(void) { return transitionActive; }
void navigationVtolMcProtectionResetLandingSettle(void) {}
void setDesiredPosition(const fpVector3_t *position, int32_t yaw, navSetWaypointFlags_t mask)
{
    if (mask & NAV_POS_UPDATE_XY) {
        posControl.desiredState.pos.x = position->x;
        posControl.desiredState.pos.y = position->y;
    }
    if (mask & NAV_POS_UPDATE_Z) {
        posControl.desiredState.pos.z = position->z;
    }
    if (mask & NAV_POS_UPDATE_HEADING) {
        posControl.desiredState.yaw = yaw;
    }
}
}

class MarkerGuidanceRuntimeTest : public ::testing::Test {
protected:
    uint64_t captureTimeUs;
    void SetUp() override
    {
        markerGuidanceReset();
        memset(&navConfig_System, 0, sizeof(navConfig_System));
        memset(&posControl, 0, sizeof(posControl));
        memset(&attitude, 0, sizeof(attitude));
        navConfig_System.general.marker_guidance_source = NAV_MARKER_GUIDANCE_SOURCE_MSP;
        navConfig_System.general.marker_guidance_mode = NAV_MARKER_GUIDANCE_MODE_PL;
        navConfig_System.general.marker_guidance_radius_cm = 5;
        navConfig_System.general.marker_guidance_max_offset_cm = 5000;
        navConfig_System.general.marker_guidance_max_target_age_ms = 500;
        navConfig_System.general.marker_guidance_lost_hold_time_ms = 1500;
        navConfig_System.general.marker_guidance_retry_min_alt_cm = 50;
        navConfig_System.general.marker_guidance_low_alt_lock_xy = true;
        armingFlags = ARMED;
        flightModeFlags = NAV_POSHOLD_MODE;
        stateFlags = MULTIROTOR;
        currentFlags = static_cast<navigationFSMStateFlags_t>(NAV_CTL_POS | NAV_CTL_ALT | NAV_CTL_YAW);
        posControl.navState = NAV_STATE_POSHOLD_3D_IN_PROGRESS;
        posControl.flags.estPosStatus = EST_TRUSTED;
        posControl.flags.estVelStatus = EST_TRUSTED;
        posControl.flags.estAltStatus = EST_TRUSTED;
        nowMs = 1000;
        captureTimeUs = 1000000;
        recoveryActive = false;
        captureActive = false;
        transitionActive = false;
        isMixerTransitionMixing = false;
        debugMode = DEBUG_NONE;
    }

    void tick(unsigned elapsed = 20)
    {
        nowMs += elapsed;
        markerGuidanceUpdate(currentFlags);
    }

    void sample(int north = 100, int east = 100, unsigned agl = 100)
    {
        markerGuidanceTargetUpdate_t pose = {};
        pose.offsetNorthCm = north - posControl.actualState.abs.pos.x;
        pose.offsetEastCm = east - posControl.actualState.abs.pos.y;
        pose.markerAglCm = agl;
        pose.captureTimeUs = captureTimeUs;
        captureTimeUs += 100000;
        markerGuidanceMspResponse_t response = {};
        ASSERT_TRUE(markerGuidanceHandleMspTargetUpdate(&pose, &response));
        ASSERT_EQ(1, response.accepted);
        tick(100);
    }

    markerGuidanceMspResponse_t timedSample(uint64_t captureUs, unsigned ageMs = 0, int north = 100, int east = 100)
    {
        markerGuidanceTargetUpdate_t pose = {};
        pose.offsetNorthCm = north;
        pose.offsetEastCm = east;
        pose.markerAglCm = 100;
        pose.captureTimeUs = captureUs;
        pose.captureAgeMs = ageMs;
        markerGuidanceMspResponse_t response = {};
        EXPECT_TRUE(markerGuidanceHandleMspTargetUpdate(&pose, &response));
        return response;
    }

    bool observedVelocityValid()
    {
        debugMode = DEBUG_MARKER_GUIDANCE;
        markerGuidanceUpdateDebug();
        return (debug[0] & (1U << 24)) != 0;
    }

    void acquire(unsigned agl = 100)
    {
        sample(100, 100, agl);
        sample(100, 100, agl);
        sample(100, 100, agl);
        ASSERT_TRUE(markerGuidanceOwnsPositionTarget());
        EXPECT_EQ(MARKER_GUIDANCE_AXIS_BOTH, markerGuidanceConsumePositionControllerRetargetAxes());
    }

    void selectLanding(bool mission = false)
    {
        flightModeFlags = mission ? NAV_WP_MODE : NAV_RTH_MODE;
        currentFlags = static_cast<navigationFSMStateFlags_t>(NAV_CTL_POS | NAV_CTL_ALT | NAV_CTL_YAW | NAV_CTL_LAND);
        posControl.navState = NAV_STATE_RTH_LANDING;
    }

    navSystemStatus_State_e guidanceState()
    {
        return markerGuidanceOverrideNavStatusState(MW_NAV_STATE_LAND_IN_PROGRESS);
    }
};

TEST_F(MarkerGuidanceRuntimeTest, LocalVelocityRequiresActivePlOwnership)
{
    markerGuidanceVelocityObservation_t observation = {};
    EXPECT_FALSE(markerGuidanceUsesLocalVelocity());
    acquire();
    EXPECT_TRUE(markerGuidanceUsesLocalVelocity());
    EXPECT_FALSE(markerGuidanceGetVelocityObservation(&observation)); // Confirmation is not a velocity baseline.
    sample();
    ASSERT_TRUE(markerGuidanceGetVelocityObservation(&observation));
    EXPECT_FLOAT_EQ(100, observation.offset.x);
    EXPECT_EQ(1300000, observation.captureTimeUs);
    EXPECT_EQ(100, observation.captureAgeMs);
    posControl.flags.isAdjustingAltitude = true;
    posControl.flags.isAdjustingHeading = true;
    EXPECT_TRUE(markerGuidanceUsesLocalVelocity());
    EXPECT_TRUE(markerGuidanceGetVelocityObservation(&observation));
    posControl.flags.isAdjustingPosition = true;
    EXPECT_FALSE(markerGuidanceUsesLocalVelocity());
    EXPECT_FALSE(markerGuidanceGetVelocityObservation(&observation));
    posControl.flags.isAdjustingPosition = false;
    for (int interruption = 0; interruption < 8; ++interruption) {
        SetUp();
        acquire();
        sample();
        ASSERT_TRUE(markerGuidanceGetVelocityObservation(&observation));
        switch (interruption) {
        case 0: recoveryActive = true; break;
        case 1: captureActive = true; break;
        case 2: transitionActive = true; break;
        case 3: armingFlags = 0; break;
        case 4: stateFlags = AIRPLANE; break;
        case 5: flightModeFlags |= FAILSAFE_MODE; break;
        case 6: posControl.navState = NAV_STATE_WAYPOINT_IN_PROGRESS; flightModeFlags = NAV_WP_MODE; break;
        case 7: isMixerTransitionMixing = true; break;
        }
        EXPECT_FALSE(markerGuidanceUsesLocalVelocity()) << interruption;
        EXPECT_FALSE(markerGuidanceGetVelocityObservation(&observation)) << interruption;
    }
}

TEST_F(MarkerGuidanceRuntimeTest, VelocityObservationRejectsOldRejectedAndUnconfirmedFrames)
{
    markerGuidanceVelocityObservation_t observation = {};
    acquire();
    sample();
    ASSERT_TRUE(markerGuidanceGetVelocityObservation(&observation));
    tick(501);
    EXPECT_FALSE(markerGuidanceGetVelocityObservation(&observation));
    SetUp(); acquire(); sample();
    timedSample(0);
    EXPECT_FALSE(markerGuidanceGetVelocityObservation(&observation));
    SetUp(); acquire(); sample();
    sample(300, 100);
    EXPECT_FALSE(markerGuidanceGetVelocityObservation(&observation));
    EXPECT_FALSE(markerGuidanceGetVelocityObservation(nullptr));
}

TEST_F(MarkerGuidanceRuntimeTest, MarkerFusionDiagnosticsKeepRawVelocitySeparate)
{
    acquire();
    markerGuidanceEstimatorDebug_t value = {};
    value.markerVelocity = (fpVector2_t){42, -17};
    value.markerResidual = (fpVector2_t){9, -3};
    value.markerReason = MARKER_VELOCITY_SELECTED;
    value.markerReady = value.markerSelected = value.markerApplied = true;
    markerGuidanceSetEstimatorDebug(&value);
    debugMode = DEBUG_MARKER_GUIDANCE;
    nowMs = 1500; markerGuidanceUpdateDebug();
    EXPECT_TRUE(static_cast<uint32_t>(debug[6]) & (1U << 29));
    EXPECT_TRUE(static_cast<uint32_t>(debug[6]) & (1U << 30));
    EXPECT_TRUE(static_cast<uint32_t>(debug[6]) & (1U << 31));
    EXPECT_EQ(MARKER_VELOCITY_SELECTED, static_cast<uint32_t>(debug[0]) >> 25);
    markerGuidanceUpdateDebug();
    EXPECT_FALSE(static_cast<uint32_t>(debug[6]) & (1U << 31));
    nowMs = 1700; markerGuidanceUpdateDebug();
    EXPECT_EQ(42, static_cast<int16_t>(debug[5]));
    EXPECT_EQ(-17, static_cast<int16_t>(static_cast<uint32_t>(debug[5]) >> 16));
    EXPECT_EQ(9, static_cast<int16_t>(debug[6]));
}

TEST_F(MarkerGuidanceRuntimeTest, DebugPagesKeepOffsetsAndExposeIndependentVelocity)
{
    acquire();
    posControl.actualState.abs.pos.x = 5;
    posControl.actualState.abs.pos.y = -3;
    sample(); // Static target: received offset changes by -5,+3 over 100 ms.
    debugMode = DEBUG_MARKER_GUIDANCE;
    nowMs = 1400; // page 2
    markerGuidanceUpdateDebug();
    EXPECT_EQ(2, (debug[0] >> 8) & 0xff);
    EXPECT_EQ(95, static_cast<int16_t>(debug[4]));
    EXPECT_EQ(103, static_cast<int16_t>(debug[4] >> 16));
    EXPECT_EQ(50, static_cast<int16_t>(debug[5]));
    EXPECT_EQ(-30, static_cast<int16_t>(debug[5] >> 16));
    markerGuidanceEstimatorDebug_t status = {};
    status.reason = MARKER_ESTIMATOR_FLOW_SELECTED;
    status.applied = true;
    status.rangeCm = 200;
    status.flowAgeMs = 4;
    status.quality = 100;
    status.reliability = 90;
    status.gpsValid = true;
    status.flowHealthy = true;
    status.flowQualityValid = true;
    markerGuidanceSetEstimatorDebug(&status);
    status.applied = false;
    markerGuidanceSetEstimatorDebug(&status);
    nowMs = 1500; // page 3
    markerGuidanceUpdateDebug();
    EXPECT_EQ(200, debug[5] & 0xffff);
    EXPECT_EQ(4, (debug[5] >> 16) & 0xffff);
    EXPECT_TRUE(debug[6] & (1U << 24));
    EXPECT_TRUE(debug[6] & (1U << 25));
    EXPECT_TRUE(debug[6] & (1U << 26));
    EXPECT_TRUE(debug[6] & (1U << 27));
    EXPECT_TRUE(debug[6] & (1U << 28));
    EXPECT_EQ(100, (debug[6] >> 8) & 0xff);
    EXPECT_EQ(90, (debug[6] >> 16) & 0xff);
    status.flowAgeMs = UINT16_MAX;
    status.flowHealthy = false;
    status.flowQualityValid = false;
    markerGuidanceSetEstimatorDebug(&status);
    nowMs = 2100;
    markerGuidanceUpdateDebug();
    EXPECT_FALSE(debug[6] & (1U << 25)); // Marker velocity is stale, not zero speed.
    EXPECT_TRUE(debug[6] & (1U << 26));
    EXPECT_FALSE(debug[6] & (1U << 27));
    EXPECT_FALSE(debug[6] & (1U << 28));
    EXPECT_EQ(UINT16_MAX, (static_cast<uint32_t>(debug[5]) >> 16) & 0xffff);
}

TEST_F(MarkerGuidanceRuntimeTest, CaptureVelocityIgnoresArrivalJitterAndGpsVelocity)
{
    acquire(); // Last capture 1.2 s, received at FC 1200 ms.
    const float previousTargetNorth = posControl.desiredState.pos.x;
    nowMs = 1450;
    posControl.actualState.abs.pos.x = 10;
    posControl.actualState.abs.vel.x = -200;
    ASSERT_EQ(1, timedSample(1450000, 50, 90).accepted);
    EXPECT_TRUE(observedVelocityValid());
    nowMs = 1460; // page 2
    markerGuidanceUpdateDebug();
    EXPECT_EQ(40, static_cast<int16_t>(debug[5]));
    nowMs = 1790; // Only 340 ms later at FC, but 250 ms between captures.
    posControl.actualState.abs.pos.x = 20;
    ASSERT_EQ(1, timedSample(1700000, 140, 80).accepted);
    nowMs = 2000; // page 2
    markerGuidanceUpdateDebug();
    EXPECT_EQ(40, static_cast<int16_t>(debug[5]));
    EXPECT_FLOAT_EQ(-200, posControl.actualState.abs.vel.x); // Diagnostic only.
    EXPECT_FLOAT_EQ(previousTargetNorth, posControl.desiredState.pos.x);
    nowMs = 2200; // page 4: capture interval and age, not FC clock subtraction.
    markerGuidanceUpdateDebug();
    EXPECT_EQ(250000, debug[5]);
    EXPECT_EQ(550, debug[6]);
    nowMs = 2291;
    EXPECT_FALSE(observedVelocityValid());
}

TEST_F(MarkerGuidanceRuntimeTest, DuplicateFramesCannotConfirmOrRefreshTarget)
{
    ASSERT_EQ(1, timedSample(1000000).accepted);
    for (int i = 0; i < 5; ++i) {
        tick(100);
        const auto response = timedSample(1000000);
        EXPECT_EQ(0, response.accepted);
        EXPECT_EQ(MARKER_GUIDANCE_REASON_STALE, response.reason);
        EXPECT_FALSE(markerGuidanceOwnsPositionTarget());
    }
    SetUp();
    acquire();
    tick(250);
    ASSERT_EQ(0, timedSample(1200000).accepted);
    debugMode = DEBUG_MARKER_GUIDANCE;
    markerGuidanceUpdateDebug();
    EXPECT_EQ(350, debug[3]); // Still the original confirmed packet's receive time.
}

TEST_F(MarkerGuidanceRuntimeTest, InvalidTimingDoesNotReplaceTarget)
{
    acquire();
    const float previousTargetNorth = posControl.desiredState.pos.x;
    EXPECT_EQ(MARKER_GUIDANCE_REASON_INVALID_TARGET, timedSample(0).reason);
    EXPECT_EQ(MARKER_GUIDANCE_REASON_INVALID_TARGET, timedSample(1300000, UINT32_MAX).reason);
    EXPECT_EQ(MARKER_GUIDANCE_REASON_STALE, timedSample(1300000, 501).reason);
    EXPECT_EQ(MARKER_GUIDANCE_REASON_STALE, timedSample(1100000).reason);
    EXPECT_FLOAT_EQ(previousTargetNorth, posControl.desiredState.pos.x);
    EXPECT_FALSE(observedVelocityValid());
    ASSERT_EQ(1, timedSample(1300000).accepted);
    EXPECT_FALSE(observedVelocityValid()); // Re-seed after rejected timing.
    tick(100);
    ASSERT_EQ(1, timedSample(1400000).accepted);
    EXPECT_TRUE(observedVelocityValid());
}

TEST_F(MarkerGuidanceRuntimeTest, ProducerRestartAndSampleGapNeverCreateVelocitySpikes)
{
    acquire();
    EXPECT_EQ(0, timedSample(1000).accepted);
    tick(401); // Old accepted receive timestamp was 1200 ms.
    ASSERT_EQ(1, timedSample(1000).accepted);
    EXPECT_FALSE(observedVelocityValid());
    EXPECT_EQ(MARKER_CAPTURE_CLOCK_RESET, (debug[0] >> 16) & 0xff);
    tick(100);
    ASSERT_EQ(1, timedSample(101000).accepted);
    EXPECT_TRUE(observedVelocityValid());
    tick(100);
    ASSERT_EQ(1, timedSample(1001000).accepted); // Capture gap despite timely transport.
    EXPECT_FALSE(observedVelocityValid());
    tick(100);
    ASSERT_EQ(1, timedSample(1101000).accepted);
    EXPECT_TRUE(observedVelocityValid());
}

TEST_F(MarkerGuidanceRuntimeTest, PendingAndChangedTargetsResetVelocityBaseline)
{
    acquire();
    sample(100, 100);
    ASSERT_TRUE(observedVelocityValid());
    sample(300, 100);
    EXPECT_FALSE(observedVelocityValid());
    sample(300, 100);
    EXPECT_FALSE(observedVelocityValid());
    sample(300, 100);
    EXPECT_FALSE(observedVelocityValid());
    sample(300, 100);
    EXPECT_TRUE(observedVelocityValid());
}

TEST_F(MarkerGuidanceRuntimeTest, CaptureMetadataDoesNotChangeControlForFreshOrderedPackets)
{
    fpVector2_t referenceTargets[5] = {};
    for (int variant = 0; variant < 2; ++variant) {
        SetUp();
        acquire();
        for (int i = 0; i < 5; ++i) {
            posControl.actualState.abs.pos.x += 2;
            const uint64_t captureUs = variant ? UINT64_C(500000000000) + i * 250000 : 1300000 + i * 100000;
            const auto response = timedSample(captureUs, variant ? 120 : 0, 100 - posControl.actualState.abs.pos.x);
            EXPECT_EQ(1, response.accepted);
            tick(100);
            EXPECT_TRUE(markerGuidanceOwnsPositionTarget());
            if (variant == 0) {
                referenceTargets[i] = { posControl.desiredState.pos.x, posControl.desiredState.pos.y };
            } else {
                EXPECT_FLOAT_EQ(referenceTargets[i].x, posControl.desiredState.pos.x);
                EXPECT_FLOAT_EQ(referenceTargets[i].y, posControl.desiredState.pos.y);
            }
            EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, markerGuidanceConsumePositionControllerRetargetAxes());
        }
    }
}

TEST_F(MarkerGuidanceRuntimeTest, UntrustedLowAglDoesNotSkipLostHold)
{
    for (const bool mission : {false, true}) {
        for (const auto status : {EST_NONE, EST_USABLE}) {
            SetUp();
            selectLanding(mission);
            posControl.flags.estAglStatus = status;
            posControl.actualState.agl.pos.z = 40;
            acquire(95);
            tick(501);
            EXPECT_EQ(MW_NAV_STATE_MARKER_GUIDANCE_TARGET_LOST_HOLD, guidanceState());
            markerGuidanceLandControl_t control = {};
            markerGuidanceGetLandControl(&control, 50);
            EXPECT_EQ(MARKER_GUIDANCE_LAND_CTRL_HOLD, control.mode);
            EXPECT_TRUE(markerGuidanceOwnsPositionTarget());
            tick(1499);
            EXPECT_EQ(MW_NAV_STATE_MARKER_GUIDANCE_TARGET_LOST_HOLD, guidanceState());
            tick(1);
            // retry_count=0 still falls back after the normal hold, not forever.
            EXPECT_EQ(MW_NAV_STATE_MARKER_GUIDANCE_FALLBACK_NORMAL_LAND, guidanceState());
        }
    }
}

TEST_F(MarkerGuidanceRuntimeTest, TrustedLowAglStillSuppressesRetry)
{
    selectLanding();
    posControl.flags.estAglStatus = EST_TRUSTED;
    posControl.actualState.agl.pos.z = 50;
    acquire(95);
    tick(501);
    EXPECT_EQ(MW_NAV_STATE_MARKER_GUIDANCE_FALLBACK_NORMAL_LAND, guidanceState());
}

TEST_F(MarkerGuidanceRuntimeTest, LowMarkerStillSuppressesRetryWithoutTrustedAgl)
{
    for (const auto status : {EST_NONE, EST_USABLE, EST_TRUSTED}) {
        SetUp();
        selectLanding();
        posControl.flags.estAglStatus = status;
        posControl.actualState.agl.pos.z = 200;
        acquire(50);
        tick(501);
        EXPECT_EQ(MW_NAV_STATE_MARKER_GUIDANCE_FALLBACK_NORMAL_LAND, guidanceState());
    }
}

TEST_F(MarkerGuidanceRuntimeTest, NewHighMarkerClearsPreviousLowMarkerSuppression)
{
    selectLanding();
    posControl.flags.estAglStatus = EST_USABLE;
    posControl.actualState.agl.pos.z = 40;
    acquire(30);
    sample(100, 100, 95);
    tick(501);
    EXPECT_EQ(MW_NAV_STATE_MARKER_GUIDANCE_TARGET_LOST_HOLD, guidanceState());
}

TEST_F(MarkerGuidanceRuntimeTest, ZeroThresholdDisablesLowAltitudeSuppression)
{
    selectLanding();
    navConfig_System.general.marker_guidance_retry_min_alt_cm = 0;
    posControl.flags.estAglStatus = EST_TRUSTED;
    posControl.actualState.agl.pos.z = 0;
    acquire(30);
    tick(501);
    EXPECT_EQ(MW_NAV_STATE_MARKER_GUIDANCE_TARGET_LOST_HOLD, guidanceState());
}

TEST_F(MarkerGuidanceRuntimeTest, FreshMarkerReacquiresDuringUntrustedAglHold)
{
    selectLanding();
    posControl.flags.estAglStatus = EST_USABLE;
    posControl.actualState.agl.pos.z = 40;
    acquire(95);
    tick(501);
    EXPECT_EQ(MW_NAV_STATE_MARKER_GUIDANCE_TARGET_LOST_HOLD, guidanceState());
    sample(100, 100, 95);
    EXPECT_EQ(MW_NAV_STATE_MARKER_GUIDANCE_LAND_CORRECTION, guidanceState());
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, markerGuidanceConsumePositionControllerRetargetAxes());
}

TEST_F(MarkerGuidanceRuntimeTest, ContinuousCrossingsDoNotReconcileIntegrator)
{
    acquire();
    for (const int position : {100, 120, 100, 80, 100, 130, 100, 70}) {
        posControl.actualState.abs.pos.x = position;
        posControl.actualState.abs.pos.y = position;
        sample();
        EXPECT_TRUE(markerGuidanceOwnsPositionTarget());
        EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, markerGuidanceConsumePositionControllerRetargetAxes());
    }
}

TEST_F(MarkerGuidanceRuntimeTest, HeldMarkerCrossingsDoNotReconcileIntegrator)
{
    acquire(30);
    tick(501);
    for (const int position : {100, 120, 100, 80, 100, 130, 100, 70}) {
        posControl.actualState.abs.pos.x = position;
        posControl.actualState.abs.pos.y = position;
        tick();
        EXPECT_TRUE(markerGuidanceOwnsPositionTarget());
        EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, markerGuidanceConsumePositionControllerRetargetAxes());
    }
}

TEST_F(MarkerGuidanceRuntimeTest, ConfirmedReplacementStillReconciles)
{
    acquire();
    sample(-100, -100);
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, markerGuidanceConsumePositionControllerRetargetAxes());
    sample(-100, -100);
    sample(-100, -100);
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_BOTH, markerGuidanceConsumePositionControllerRetargetAxes());
}

TEST_F(MarkerGuidanceRuntimeTest, BriefRetainedTargetLossDoesNotReconcileOnReturn)
{
    for (const bool landing : {false, true}) {
        SetUp();
        if (landing) {
            flightModeFlags = NAV_RTH_MODE;
            currentFlags = static_cast<navigationFSMStateFlags_t>(NAV_CTL_POS | NAV_CTL_ALT | NAV_CTL_YAW | NAV_CTL_LAND);
            posControl.navState = NAV_STATE_RTH_LANDING;
        }
        acquire(30);
        for (int gap = 0; gap < 3; ++gap) {
            tick(501);
            ASSERT_TRUE(markerGuidanceOwnsPositionTarget());
            ASSERT_GT(posControl.desiredState.pos.x, 90);
            sample(100, 100, 30);
            EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, markerGuidanceConsumePositionControllerRetargetAxes());
        }
    }
}

TEST_F(MarkerGuidanceRuntimeTest, ReturnAfterBrakingInPlaceStillReconciles)
{
    acquire(100);
    tick(501);
    ASSERT_FLOAT_EQ(0, posControl.desiredState.pos.x);
    sample();
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_BOTH, markerGuidanceConsumePositionControllerRetargetAxes());
}

TEST_F(MarkerGuidanceRuntimeTest, LandRetainsSameTargetAboveLowAltitudeCutoff)
{
    flightModeFlags = NAV_RTH_MODE;
    currentFlags = static_cast<navigationFSMStateFlags_t>(NAV_CTL_POS | NAV_CTL_ALT | NAV_CTL_YAW | NAV_CTL_LAND);
    posControl.navState = NAV_STATE_RTH_LANDING;
    acquire(100);
    tick(501);
    ASSERT_GT(posControl.desiredState.pos.x, 90);
    sample();
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, markerGuidanceConsumePositionControllerRetargetAxes());
}

TEST_F(MarkerGuidanceRuntimeTest, RetryClimbStillReconcilesEvenWithRetainedXY)
{
    flightModeFlags = NAV_RTH_MODE;
    currentFlags = static_cast<navigationFSMStateFlags_t>(NAV_CTL_POS | NAV_CTL_ALT | NAV_CTL_YAW | NAV_CTL_LAND);
    posControl.navState = NAV_STATE_RTH_LANDING;
    navConfig_System.general.marker_guidance_retry_count = 1;
    navConfig_System.general.marker_guidance_retry_altitude_cm = 100;
    acquire(100);
    tick(501);
    tick(600);
    tick(1000);
    markerGuidanceLandControl_t control = {};
    markerGuidanceGetLandControl(&control, 50);
    ASSERT_EQ(MARKER_GUIDANCE_LAND_CTRL_CLIMB, control.mode);
    sample();
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_BOTH, markerGuidanceConsumePositionControllerRetargetAxes());
}

TEST_F(MarkerGuidanceRuntimeTest, RetainedTargetWindowHandlesClockWrap)
{
    nowMs = UINT32_MAX - 700;
    acquire(30);
    tick(501);
    sample(100, 100, 30);
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, markerGuidanceConsumePositionControllerRetargetAxes());
}

TEST_F(MarkerGuidanceRuntimeTest, ReturnAfterHoldWindowStillReconciles)
{
    acquire(30);
    tick(501);
    tick(1500);
    sample(100, 100, 30);
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_BOTH, markerGuidanceConsumePositionControllerRetargetAxes());
}

TEST_F(MarkerGuidanceRuntimeTest, ReplacementDuringRetainedHoldStillReconciles)
{
    acquire(30);
    tick(501);
    sample(-100, -100, 30);
    sample(-100, -100, 30);
    sample(-100, -100, 30);
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_BOTH, markerGuidanceConsumePositionControllerRetargetAxes());
}

TEST_F(MarkerGuidanceRuntimeTest, PilotTakeoverAfterRetainedHoldStillReconciles)
{
    acquire(30);
    tick(501);
    posControl.flags.isAdjustingPosition = true;
    tick();
    posControl.flags.isAdjustingPosition = false;
    tick();
    sample(100, 100, 30);
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_BOTH, markerGuidanceConsumePositionControllerRetargetAxes());
}

TEST_F(MarkerGuidanceRuntimeTest, PoshAndLandUseSameBoundedPositionResponse)
{
    for (const bool landing : {false, true}) {
        SetUp();
        if (landing) {
            flightModeFlags = NAV_RTH_MODE;
            currentFlags = static_cast<navigationFSMStateFlags_t>(NAV_CTL_POS | NAV_CTL_ALT | NAV_CTL_YAW | NAV_CTL_LAND);
            posControl.navState = NAV_STATE_RTH_LANDING;
        }
        acquire();
        EXPECT_FLOAT_EQ(2, markerGuidanceGetPositionResponseScale());
        posControl.actualState.abs.pos.y = 100;
        posControl.actualState.abs.pos.x = 80;
        sample();
        EXPECT_FLOAT_EQ(1.5f, markerGuidanceGetPositionResponseScale());
        posControl.actualState.abs.pos.x = 90;
        sample();
        EXPECT_FLOAT_EQ(1, markerGuidanceGetPositionResponseScale());
        posControl.actualState.abs.pos.x = 100;
        sample();
        EXPECT_FLOAT_EQ(1, markerGuidanceGetPositionResponseScale());
    }
}

TEST_F(MarkerGuidanceRuntimeTest, PositionResponseDoesNotOverrideTakeoverOrRecovery)
{
    acquire();
    posControl.flags.isAdjustingAltitude = true;
    posControl.flags.isAdjustingHeading = true;
    EXPECT_FLOAT_EQ(2, markerGuidanceGetPositionResponseScale());
    posControl.flags.isAdjustingPosition = true;
    EXPECT_FLOAT_EQ(1, markerGuidanceGetPositionResponseScale());
    posControl.flags.isAdjustingPosition = false;
    recoveryActive = true;
    EXPECT_FLOAT_EQ(1, markerGuidanceGetPositionResponseScale());
    tick();
    recoveryActive = false;
    tick();
    EXPECT_FLOAT_EQ(1, markerGuidanceGetPositionResponseScale());
}

TEST_F(MarkerGuidanceRuntimeTest, PositionResponseDisabledOutsideUsablePl)
{
    for (int condition = 0; condition < 6; ++condition) {
        SetUp();
        acquire();
        switch (condition) {
        case 0: armingFlags = 0; break;
        case 1: flightModeFlags |= FAILSAFE_MODE; break;
        case 2: stateFlags = AIRPLANE; break;
        case 3: navConfig_System.general.marker_guidance_mode = NAV_MARKER_GUIDANCE_MODE_OFF; break;
        case 4: stateFlags |= LANDING_DETECTED; break;
        case 5: tick(501); break; // Ordinary POSH loss brakes in place.
        }
        EXPECT_FLOAT_EQ(1, markerGuidanceGetPositionResponseScale());
    }
}

TEST_F(MarkerGuidanceRuntimeTest, RecoveryCapturesCurrentPositionNotPreviouslyHeldMarker)
{
    acquire(30);
    tick(501);
    ASSERT_GT(posControl.desiredState.pos.x, 90);
    posControl.actualState.abs.pos.x = 40;
    posControl.actualState.abs.pos.y = 30;
    recoveryActive = true;
    tick();
    EXPECT_FLOAT_EQ(40, posControl.desiredState.pos.x);
    EXPECT_FLOAT_EQ(30, posControl.desiredState.pos.y);
    posControl.actualState.abs.pos.x = 50;
    tick();
    EXPECT_FLOAT_EQ(40, posControl.desiredState.pos.x);
}

TEST_F(MarkerGuidanceRuntimeTest, FirstAcquisitionWaitsForRecoveryAndNewPacket)
{
    recoveryActive = true;
    sample();
    sample();
    sample();
    EXPECT_FALSE(markerGuidanceOwnsPositionTarget());
    recoveryActive = false;
    tick();
    EXPECT_FALSE(markerGuidanceOwnsPositionTarget());
    sample();
    EXPECT_TRUE(markerGuidanceOwnsPositionTarget());
}

TEST_F(MarkerGuidanceRuntimeTest, LandingCrossingsAlsoKeepIntegrator)
{
    flightModeFlags = NAV_RTH_MODE;
    currentFlags = static_cast<navigationFSMStateFlags_t>(NAV_CTL_POS | NAV_CTL_ALT | NAV_CTL_YAW | NAV_CTL_LAND);
    posControl.navState = NAV_STATE_RTH_LANDING;
    acquire(30);
    posControl.actualState.abs.pos.x = 100;
    sample(100, 100, 30);
    posControl.actualState.abs.pos.x = 130;
    sample(100, 100, 30);
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, markerGuidanceConsumePositionControllerRetargetAxes());
    tick(501);
    posControl.actualState.abs.pos.x = 100;
    tick();
    posControl.actualState.abs.pos.x = 70;
    tick();
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, markerGuidanceConsumePositionControllerRetargetAxes());
    fpVector3_t landingTarget = {};
    EXPECT_TRUE(markerGuidanceGetActiveLandingPositionTarget(&landingTarget));
    EXPECT_GT(landingTarget.x, 90);
}

TEST_F(MarkerGuidanceRuntimeTest, RecoveryExitRequiresNewSampleBeforePursuit)
{
    acquire();
    posControl.actualState.abs.pos.x = 40;
    recoveryActive = true;
    tick();
    sample();
    EXPECT_FLOAT_EQ(40, posControl.desiredState.pos.x);
    recoveryActive = false;
    tick();
    EXPECT_FLOAT_EQ(40, posControl.desiredState.pos.x);
    tick();
    EXPECT_FLOAT_EQ(40, posControl.desiredState.pos.x);
    sample();
    EXPECT_GT(posControl.desiredState.pos.x, 90);
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_BOTH, markerGuidanceConsumePositionControllerRetargetAxes());
}

TEST_F(MarkerGuidanceRuntimeTest, PilotPositionTakeoverWinsDuringRecovery)
{
    acquire();
    recoveryActive = true;
    tick();
    posControl.flags.isAdjustingPosition = true;
    posControl.desiredState.pos.x = -200;
    tick();
    EXPECT_FALSE(markerGuidanceOwnsPositionTarget());
    EXPECT_FLOAT_EQ(-200, posControl.desiredState.pos.x);
    posControl.flags.isAdjustingPosition = false;
    tick();
    sample();
    EXPECT_FALSE(markerGuidanceOwnsPositionTarget());
    recoveryActive = false;
    tick();
    EXPECT_FALSE(markerGuidanceOwnsPositionTarget());
    sample();
    EXPECT_TRUE(markerGuidanceOwnsPositionTarget());
}

TEST_F(MarkerGuidanceRuntimeTest, AltitudeAndYawTakeoverDoNotReleaseXY)
{
    acquire();
    posControl.flags.isAdjustingAltitude = true;
    posControl.flags.isAdjustingHeading = true;
    tick();
    EXPECT_TRUE(markerGuidanceOwnsPositionTarget());
    int32_t heading = 9000;
    EXPECT_FALSE(markerGuidanceApplyHeadingOverride(&heading));
    EXPECT_EQ(9000, heading);
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, markerGuidanceConsumePositionControllerRetargetAxes());
}

TEST_F(MarkerGuidanceRuntimeTest, ModeExitPreservesNewNavigationTarget)
{
    acquire();
    posControl.navState = NAV_STATE_WAYPOINT_IN_PROGRESS;
    flightModeFlags = NAV_WP_MODE;
    posControl.desiredState.pos.x = -500;
    posControl.desiredState.pos.y = 700;
    tick();
    EXPECT_FALSE(markerGuidanceOwnsPositionTarget());
    EXPECT_FLOAT_EQ(-500, posControl.desiredState.pos.x);
    EXPECT_FLOAT_EQ(700, posControl.desiredState.pos.y);
}

TEST_F(MarkerGuidanceRuntimeTest, CaptureNeedsFreshSampleAfterRelease)
{
    captureActive = true;
    sample();
    sample();
    sample();
    EXPECT_FALSE(markerGuidanceOwnsPositionTarget());
    captureActive = false;
    tick();
    EXPECT_FALSE(markerGuidanceOwnsPositionTarget());
    sample();
    EXPECT_TRUE(markerGuidanceOwnsPositionTarget());
}

TEST_F(MarkerGuidanceRuntimeTest, EmergencyDisarmFailsafeAndFixedWingReleaseMarker)
{
    acquire();
    posControl.navState = NAV_STATE_EMERGENCY_LANDING_IN_PROGRESS;
    currentFlags = static_cast<navigationFSMStateFlags_t>(NAV_CTL_EMERG | NAV_CTL_HOLD | NAV_REQUIRE_ANGLE);
    tick();
    EXPECT_FALSE(markerGuidanceOwnsPositionTarget());
    int32_t heading = 9000;
    EXPECT_FALSE(markerGuidanceApplyHeadingOverride(&heading));

    for (int condition = 0; condition < 4; ++condition) {
        SetUp();
        acquire();
        switch (condition) {
        case 0: armingFlags = 0; break;
        case 1: flightModeFlags |= FAILSAFE_MODE; break;
        case 2: stateFlags = AIRPLANE; break;
        case 3: navConfig_System.general.marker_guidance_mode = NAV_MARKER_GUIDANCE_MODE_OFF; break;
        }
        tick();
        EXPECT_FALSE(markerGuidanceOwnsPositionTarget());
        EXPECT_FALSE(markerGuidanceApplyHeadingOverride(&heading));
        EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, markerGuidanceConsumePositionControllerRetargetAxes());
    }
}
