#include <cstring>
#include "gtest/gtest.h"

extern "C" {
#include "platform.h"
#include "fc/runtime_config.h"
#include "flight/imu.h"
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

static timeMs_t nowMs;
static navigationFSMStateFlags_t currentFlags;
static bool recoveryActive;
static bool captureActive;

timeMs_t millis(void) { return nowMs; }
bool areSensorsCalibrating(void) { return false; }
navigationFSMStateFlags_t navGetCurrentStateFlags(void) { return currentFlags; }
const navEstimatedPosVel_t *navGetCurrentActualPositionAndVelocity(void) { return &posControl.actualState.abs; }
bool navigationRTHAllowsLanding(void) { return true; }
bool navigationVtolMcProtectionGuidanceRecoveryActive(void) { return recoveryActive; }
bool navigationVtolMcProtectionPositionCaptureActive(void) { return captureActive; }
bool navigationVtolMcProtectionPositionCapturePending(uint32_t) { return captureActive; }
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
        recoveryActive = false;
        captureActive = false;
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
        markerGuidanceMspResponse_t response = {};
        ASSERT_TRUE(markerGuidanceHandleMspTargetUpdate(&pose, &response));
        ASSERT_EQ(1, response.accepted);
        tick(100);
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
