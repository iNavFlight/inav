#include <gtest/gtest.h>

extern "C" {
#include "common/fp_pid.h"
#include "navigation/marker_guidance_logic.h"
#include "navigation/sqrt_controller.h"
#include "navigation/navigation_vtol_mc_protection_logic.h"
}

TEST(NavigationControlReserveTest, MarkerAltitudeHoldCorrectsPositionDriftEvenAtZeroEstimatedVelocity)
{
    markerGuidanceAltitudeHoldState_t hold = {};
    sqrt_controller_t position = {};
    sqrtControllerInit(&position, 1.0f, -50.0f, 50.0f, 100.0f);
    ASSERT_TRUE(markerGuidanceUpdateAltitudeHold(&hold, true, true, 100.0f));
    // Entering the pause does not create an immediate climb demand.
    EXPECT_FLOAT_EQ(0.0f, sqrtControllerApply(&position, hold.altitudeCm, 100.0f, 0.02f));

    pidController_t velocity = {};
    navPidInit(&velocity, 1.0f, 0.25f, 0.0f, 0.0f, 0.0f, 0.0f);
    const float oldZeroRateOutput = navPidApply2(&velocity, 0.0f, 0.0f, 0.02f, -100, 100, (pidControllerFlags_e)0);
    ASSERT_TRUE(markerGuidanceUpdateAltitudeHold(&hold, true, true, 80.0f));
    const float requestedVelocity = sqrtControllerApply(&position, hold.altitudeCm, 80.0f, 0.02f);
    const float holdOutput = navPidApply2(&velocity, requestedVelocity, 0.0f, 0.02f, -100, 100, (pidControllerFlags_e)0);
    EXPECT_GT(requestedVelocity, 0.0f);
    EXPECT_LE(requestedVelocity, 50.0f);
    EXPECT_GT(holdOutput, oldZeroRateOutput);

    EXPECT_FALSE(markerGuidanceUpdateAltitudeHold(&hold, false, true, 80.0f));
    ASSERT_TRUE(markerGuidanceUpdateAltitudeHold(&hold, true, true, 30.0f));
    EXPECT_FLOAT_EQ(0.0f, sqrtControllerApply(&position, hold.altitudeCm, 30.0f, 0.02f));
}

TEST(NavigationControlReserveTest, TiltIsIncludedInPidBoundsBeforeAntiWindup)
{
    const auto outputBounds = vtolMcProtectionComputeThrottleBounds(true, 1080, 1700, 2000, 15);
    const float factor = 1.0f / 0.6f;
    const auto pidBounds = vtolMcProtectionBoundsBeforeTilt(outputBounds, 1080, factor);
    EXPECT_EQ(1218, outputBounds.min);
    EXPECT_EQ(1862, outputBounds.max);
    EXPECT_GE(1080 + (pidBounds.min - 1080) * factor, outputBounds.min);
    EXPECT_LE(1080 + (pidBounds.max - 1080) * factor, outputBounds.max);

    pidController_t pid = {};
    navPidInit(&pid, 1.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f);
    pid.integrator = 100.0f;
    const float output = navPidApply2(&pid, 0.0f, 0.0f, 0.02f,
        pidBounds.min - 1700, pidBounds.max - 1700, (pidControllerFlags_e)0);
    EXPECT_FLOAT_EQ(pidBounds.max - 1700, output);
    EXPECT_LT(pid.integrator, 100.0f);
}

TEST(NavigationControlReserveTest, BailoutHoverIsNotBoostedBackToFullThrottle)
{
    const auto bounds = vtolMcProtectionComputeThrottleBounds(true, 1080, 1700, 2000, 15);
    for (float factor : {1.0f, 1.1f, 1.4f, 1.0f / 0.6f}) {
        const auto pidBounds = vtolMcProtectionBoundsBeforeTilt(bounds, 1080, factor);
        const int16_t hoverBeforeTilt = vtolMcProtectionThrottleBeforeTilt(1700, 1080, factor);
        EXPECT_GE(hoverBeforeTilt, pidBounds.min);
        EXPECT_LE(hoverBeforeTilt, pidBounds.max);
        EXPECT_NEAR(1700, 1080 + (hoverBeforeTilt - 1080) * factor, 2.0f);
    }
}

TEST(NavigationControlReserveTest, FinalGuardHandlesAttitudeChangeBetweenNavUpdates)
{
    const auto bounds = vtolMcProtectionComputeThrottleBounds(true, 1080, 1700, 2000, 15);
    EXPECT_EQ(1862, vtolMcProtectionLimitCompensatedThrottle(true, 2000, &bounds));
    EXPECT_EQ(2000, vtolMcProtectionLimitCompensatedThrottle(false, 2000, &bounds));
    // Landing probe and motor reduction are not raised back to the reserve floor.
    EXPECT_EQ(1100, vtolMcProtectionLimitCompensatedThrottle(true, 1100, &bounds));
    EXPECT_EQ(1080, vtolMcProtectionLimitCompensatedThrottle(true, 1080, &bounds));
}

TEST(NavigationControlReserveTest, LevelFlightPreservesBoundsAndDisabledReserve)
{
    const auto bounds = vtolMcProtectionComputeThrottleBounds(true, 1080, 1700, 2000, 0);
    const auto beforeTilt = vtolMcProtectionBoundsBeforeTilt(bounds, 1080, 1.0f);
    EXPECT_EQ(1080, beforeTilt.min);
    EXPECT_EQ(2000, beforeTilt.max);
    EXPECT_EQ(1700, vtolMcProtectionThrottleBeforeTilt(1700, 1080, 1.0f));
    EXPECT_EQ(2000, vtolMcProtectionLimitCompensatedThrottle(true, 2000, &bounds));
}

TEST(NavigationControlReserveTest, ShrunkReserveAndDegenerateRangeRemainOrdered)
{
    for (int hover : {1080, 1100, 1700, 1990, 2000}) {
        const auto bounds = vtolMcProtectionComputeThrottleBounds(true, 1080, hover, 2000, 15);
        for (int step = 0; step <= 100; ++step) {
            const float factor = 1.0f + step / 150.0f;
            const auto beforeTilt = vtolMcProtectionBoundsBeforeTilt(bounds, 1080, factor);
            EXPECT_LE(beforeTilt.min, beforeTilt.max);
            EXPECT_LE(1080 + (beforeTilt.max - 1080) * factor, bounds.max + 0.001f);
        }
    }
    const auto bounds = vtolMcProtectionComputeThrottleBounds(true, 1080, 1080, 1080, 15);
    const auto beforeTilt = vtolMcProtectionBoundsBeforeTilt(bounds, 1080, 1.5f);
    EXPECT_EQ(1080, beforeTilt.min);
    EXPECT_EQ(1080, beforeTilt.max);
}

TEST(NavigationControlReserveTest, ContinuousMarkerOffsetRetainsUsefulVelocityIntegral)
{
    pidController_t pid = {};
    navPidInit(&pid, 27.0f / 20.0f, 5.0f / 100.0f, 0.45f, 0.2f, 2.0f, 0.0f);
    markerGuidanceCorrectionDirectionState_t direction = {};
    markerGuidanceCorrectionCrossedTargetAxes(&direction, 5, 35.0f, 0.0f);
    float eastIntegrator = 0.0f;
    for (int i = 0; i < 1500; ++i) {
        const float output = navPidApply3(&pid, 12.0f, 2.0f, 0.02f, -250.0f, 250.0f,
            (pidControllerFlags_e)0, 1.0f, 1.0f);
        const auto axes = markerGuidancePositionControllerReconcileAxes(true, true,
            MARKER_GUIDANCE_AXIS_NORTH, MARKER_GUIDANCE_AXIS_NORTH,
            markerGuidanceCorrectionCrossedTargetAxes(&direction, 5, 35.0f, 0.0f));
        EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, axes);
        EXPECT_FALSE(markerGuidanceReconcileIntegratorForTargetHandoff(
            axes, 10.0f, 0.0f, output, 0.0f, &pid.integrator, &eastIntegrator));
    }
    // Measured persistent error must accumulate, not be treated as a new
    // marker handoff on every packet. This test does not model airframe forces.
    EXPECT_NEAR(15.0f, pid.integrator, 0.01f);
    EXPECT_NEAR(30.89f, pid.output_constrained, 0.02f);
}

TEST(NavigationControlReserveTest, CrossingUsesNormalIntegrationAndStillAllowsAntiWindup)
{
    pidController_t pid = {};
    navPidInit(&pid, 1.35f, 0.09f, 0.0f, 0.0f, 0.0f, 0.0f);
    pid.integrator = -31.0f;
    const float output = navPidApply2(&pid, 8.0f, 0.0f, 0.02f, -250, 250, (pidControllerFlags_e)0);
    ASSERT_LT(output, 0.0f);
    const auto axes = markerGuidancePositionControllerReconcileAxes(true, true,
        MARKER_GUIDANCE_AXIS_BOTH, MARKER_GUIDANCE_AXIS_BOTH, MARKER_GUIDANCE_AXIS_NORTH);
    float eastIntegrator = 25.0f;
    EXPECT_FALSE(markerGuidanceReconcileIntegratorForTargetHandoff(
        axes, 8.0f, -5.0f, output, 20.0f, &pid.integrator, &eastIntegrator));
    EXPECT_NEAR(-31.0f + 8.0f * 0.09f * 0.02f, pid.integrator, 0.0001f);
    EXPECT_FLOAT_EQ(25.0f, eastIntegrator);

    // Removing marker-specific trimming must not disable PID anti-windup.
    const float beforeSaturation = pid.integrator;
    navPidApply2(&pid, 0.0f, 0.0f, 0.02f, -10, 10, (pidControllerFlags_e)0);
    EXPECT_GT(pid.integrator, beforeSaturation);
}
