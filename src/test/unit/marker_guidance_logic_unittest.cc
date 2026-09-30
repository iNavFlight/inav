#include <gtest/gtest.h>

extern "C" {
#include "navigation/marker_guidance_logic.h"
}

TEST(MarkerGuidanceLogicTest, AcceptsOnlyExactEightBytePayload)
{
    EXPECT_TRUE(markerGuidanceMspPayloadSizeIsValid(8));
    EXPECT_FALSE(markerGuidanceMspPayloadSizeIsValid(4));
    EXPECT_FALSE(markerGuidanceMspPayloadSizeIsValid(7));
    EXPECT_FALSE(markerGuidanceMspPayloadSizeIsValid(9));
}

TEST(MarkerGuidanceLogicTest, DecodesGoldenPayloadWordsWithSignedFields)
{
    const markerGuidancePoseUpdate_t update = markerGuidanceDecodeMspWords(0xFF85, 0x01C8, 0xFC7C, 0x0141);

    EXPECT_EQ(-123, update.offsetNorthCm);
    EXPECT_EQ(456, update.offsetEastCm);
    EXPECT_EQ(-900, update.yawErrorDeciDeg);
    EXPECT_EQ(321, update.markerAglCm);
}

TEST(MarkerGuidanceLogicTest, ValidatesYawAglAndHorizontalMagnitude)
{
    markerGuidancePoseUpdate_t update = { 300, 400, 1800, 1 };
    EXPECT_TRUE(markerGuidancePoseIsValid(&update, 500));

    update.yawErrorDeciDeg = 1801;
    EXPECT_FALSE(markerGuidancePoseIsValid(&update, 500));
    update.yawErrorDeciDeg = -1801;
    EXPECT_FALSE(markerGuidancePoseIsValid(&update, 500));

    update.yawErrorDeciDeg = 0;
    update.markerAglCm = 0;
    EXPECT_FALSE(markerGuidancePoseIsValid(&update, 500));

    update.markerAglCm = 100;
    EXPECT_FALSE(markerGuidancePoseIsValid(&update, 499));
    EXPECT_EQ(250000U, markerGuidanceHorizontalOffsetSquaredCm(&update));
}

TEST(MarkerGuidanceLogicTest, FreshnessUsesFcReceiveTimeAndHandlesTimerWrap)
{
    EXPECT_FALSE(markerGuidanceSampleIsFresh(false, 1000, 900, 200));
    EXPECT_TRUE(markerGuidanceSampleIsFresh(true, 1100, 900, 200));
    EXPECT_FALSE(markerGuidanceSampleIsFresh(true, 1101, 900, 200));
    EXPECT_TRUE(markerGuidanceSampleIsFresh(true, 1101, 900, 0));
    EXPECT_TRUE(markerGuidanceSampleIsFresh(true, 5, UINT32_MAX - 4, 10));
}

TEST(MarkerGuidanceLogicTest, LowAltitudeFallbackKeepsConfirmedMarkerTarget)
{
    float holdNorth = 0.0f;
    float holdEast = 0.0f;

    markerGuidanceSelectLowAltitudeHoldPosition(
        true, 120.0f, -80.0f, 95.0f, -60.0f, &holdNorth, &holdEast);

    EXPECT_FLOAT_EQ(120.0f, holdNorth);
    EXPECT_FLOAT_EQ(-80.0f, holdEast);
}

TEST(MarkerGuidanceLogicTest, LowAltitudeFallbackUsesCurrentPositionWithoutMarkerOwnership)
{
    float holdNorth = 0.0f;
    float holdEast = 0.0f;

    markerGuidanceSelectLowAltitudeHoldPosition(
        false, 120.0f, -80.0f, 95.0f, -60.0f, &holdNorth, &holdEast);

    EXPECT_FLOAT_EQ(95.0f, holdNorth);
    EXPECT_FLOAT_EQ(-60.0f, holdEast);
}

TEST(MarkerGuidanceLogicTest, LowAltitudeMarkerHoldRequiresExplicitUsableThreshold)
{
    EXPECT_TRUE(markerGuidanceShouldKeepLowAltitudeMarkerTarget(true, true, 100, 63));
    EXPECT_FALSE(markerGuidanceShouldKeepLowAltitudeMarkerTarget(false, true, 100, 63));
    EXPECT_FALSE(markerGuidanceShouldKeepLowAltitudeMarkerTarget(true, false, 100, 63));
    EXPECT_FALSE(markerGuidanceShouldKeepLowAltitudeMarkerTarget(true, true, 0, 63));
    EXPECT_FALSE(markerGuidanceShouldKeepLowAltitudeMarkerTarget(true, true, 100, 0));
    EXPECT_FALSE(markerGuidanceShouldKeepLowAltitudeMarkerTarget(true, true, 50, 63));
}

TEST(MarkerGuidanceLogicTest, ConfirmedLandTargetHoldRequiresExplicitLockAndOwnership)
{
    EXPECT_TRUE(markerGuidanceShouldKeepConfirmedLandTarget(true, true));
    EXPECT_FALSE(markerGuidanceShouldKeepConfirmedLandTarget(false, true));
    EXPECT_FALSE(markerGuidanceShouldKeepConfirmedLandTarget(true, false));
}

TEST(MarkerGuidanceLogicTest, TargetConsistencyToleranceUsesRadiusOrMarkerHeight)
{
    EXPECT_FLOAT_EQ(20.0f, markerGuidanceTargetConsistencyToleranceCm(100, 20));
    EXPECT_FLOAT_EQ(50.0f, markerGuidanceTargetConsistencyToleranceCm(1000, 20));
}

TEST(MarkerGuidanceLogicTest, ConfirmedReplacementReconcilesOnlyAnOwnedTarget)
{
    EXPECT_TRUE(markerGuidanceTargetReplacementNeedsControllerReconcile(true, true));
    EXPECT_FALSE(markerGuidanceTargetReplacementNeedsControllerReconcile(false, true));
    EXPECT_FALSE(markerGuidanceTargetReplacementNeedsControllerReconcile(true, false));
}

TEST(MarkerGuidanceLogicTest, ContinuousOwnershipIncludesInsideRadiusStandbyButNotLoss)
{
    EXPECT_TRUE(markerGuidanceTargetOwnershipIsContinuous(true, true, true, false));
    EXPECT_TRUE(markerGuidanceTargetOwnershipIsContinuous(true, true, false, true));
    EXPECT_FALSE(markerGuidanceTargetOwnershipIsContinuous(true, true, false, false));
    EXPECT_FALSE(markerGuidanceTargetOwnershipIsContinuous(false, true, true, false));
    EXPECT_FALSE(markerGuidanceTargetOwnershipIsContinuous(true, false, true, false));
}

TEST(MarkerGuidanceLogicTest, ReconcileAxesKeepHandoffAndNewAxisButIgnoreCrossing)
{
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_BOTH,
        markerGuidancePositionControllerReconcileAxes(
            true, false, MARKER_GUIDANCE_AXIS_NONE, MARKER_GUIDANCE_AXIS_NORTH, MARKER_GUIDANCE_AXIS_NONE));
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_EAST,
        markerGuidancePositionControllerReconcileAxes(
            true, true, MARKER_GUIDANCE_AXIS_NORTH, MARKER_GUIDANCE_AXIS_BOTH, MARKER_GUIDANCE_AXIS_NONE));
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidancePositionControllerReconcileAxes(
            true, true, MARKER_GUIDANCE_AXIS_BOTH, MARKER_GUIDANCE_AXIS_BOTH, MARKER_GUIDANCE_AXIS_NORTH));
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidancePositionControllerReconcileAxes(
            true, true, MARKER_GUIDANCE_AXIS_BOTH, MARKER_GUIDANCE_AXIS_BOTH, MARKER_GUIDANCE_AXIS_NONE));
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidancePositionControllerReconcileAxes(
            false, false, MARKER_GUIDANCE_AXIS_NONE, MARKER_GUIDANCE_AXIS_BOTH, MARKER_GUIDANCE_AXIS_BOTH));
}

TEST(MarkerGuidanceLogicTest, FlightCrossingsPreserveTrimButReacquisitionStillReconciles)
{
    for (const uint8_t axis : {MARKER_GUIDANCE_AXIS_NORTH, MARKER_GUIDANCE_AXIS_EAST, MARKER_GUIDANCE_AXIS_BOTH}) {
        float northI = -31.0f;
        float eastI = 25.0f;
        const auto axes = markerGuidancePositionControllerReconcileAxes(
            true, true, MARKER_GUIDANCE_AXIS_BOTH, MARKER_GUIDANCE_AXIS_BOTH, axis);
        EXPECT_FALSE(markerGuidanceReconcileIntegratorForTargetHandoff(
            axes, 8.0f, -5.0f, -17.0f, 20.0f, &northI, &eastI));
        EXPECT_FLOAT_EQ(-31.0f, northI);
        EXPECT_FLOAT_EQ(25.0f, eastI);
    }

    float northI = -31.0f;
    float eastI = 25.0f;
    const auto axes = markerGuidancePositionControllerReconcileAxes(
        true, false, MARKER_GUIDANCE_AXIS_NONE, MARKER_GUIDANCE_AXIS_BOTH, MARKER_GUIDANCE_AXIS_NONE);
    EXPECT_TRUE(markerGuidanceReconcileIntegratorForTargetHandoff(
        axes, 8.0f, -5.0f, -17.0f, 20.0f, &northI, &eastI));
    EXPECT_FLOAT_EQ(-14.0f, northI);
    EXPECT_FLOAT_EQ(5.0f, eastI);
}

TEST(MarkerGuidanceLogicTest, TargetRequiresThreeConsistentSamplesBeforeConfirmation)
{
    markerGuidanceTargetConfirmationState_t state = { };
    float confirmedNorth = 0.0f;
    float confirmedEast = 0.0f;

    EXPECT_FALSE(markerGuidanceUpdateTargetConfirmation(
        &state, 100.0f, 200.0f, 100, 20, 1000, 500, &confirmedNorth, &confirmedEast));
    EXPECT_FALSE(markerGuidanceUpdateTargetConfirmation(
        &state, 110.0f, 190.0f, 100, 20, 1200, 500, &confirmedNorth, &confirmedEast));
    EXPECT_TRUE(markerGuidanceUpdateTargetConfirmation(
        &state, 105.0f, 195.0f, 100, 20, 1400, 500, &confirmedNorth, &confirmedEast));

    EXPECT_NEAR(105.0f, confirmedNorth, 0.001f);
    EXPECT_NEAR(195.0f, confirmedEast, 0.001f);
    EXPECT_FALSE(state.active);
}

TEST(MarkerGuidanceLogicTest, TargetJumpRestartsConfirmation)
{
    markerGuidanceTargetConfirmationState_t state = { };
    float confirmedNorth = 0.0f;
    float confirmedEast = 0.0f;

    EXPECT_FALSE(markerGuidanceUpdateTargetConfirmation(
        &state, 0.0f, 0.0f, 100, 20, 1000, 500, &confirmedNorth, &confirmedEast));
    EXPECT_FALSE(markerGuidanceUpdateTargetConfirmation(
        &state, 10.0f, 0.0f, 100, 20, 1200, 500, &confirmedNorth, &confirmedEast));
    EXPECT_FALSE(markerGuidanceUpdateTargetConfirmation(
        &state, 100.0f, 0.0f, 100, 20, 1400, 500, &confirmedNorth, &confirmedEast));
    EXPECT_EQ(1, state.sampleCount);
    EXPECT_FALSE(markerGuidanceUpdateTargetConfirmation(
        &state, 105.0f, 0.0f, 100, 20, 1600, 500, &confirmedNorth, &confirmedEast));
    EXPECT_TRUE(markerGuidanceUpdateTargetConfirmation(
        &state, 110.0f, 0.0f, 100, 20, 1800, 500, &confirmedNorth, &confirmedEast));
    EXPECT_NEAR(105.0f, confirmedNorth, 0.001f);
}

TEST(MarkerGuidanceLogicTest, TargetConfirmationRestartsAfterSampleGap)
{
    markerGuidanceTargetConfirmationState_t state = { };
    float confirmedNorth = 0.0f;
    float confirmedEast = 0.0f;

    EXPECT_FALSE(markerGuidanceUpdateTargetConfirmation(
        &state, 100.0f, 100.0f, 100, 20, 1000, 500, &confirmedNorth, &confirmedEast));
    EXPECT_FALSE(markerGuidanceUpdateTargetConfirmation(
        &state, 100.0f, 100.0f, 100, 20, 1600, 500, &confirmedNorth, &confirmedEast));
    EXPECT_EQ(1, state.sampleCount);
}

TEST(MarkerGuidanceLogicTest, RejectedPacketBreaksTargetConfirmationSequence)
{
    markerGuidanceTargetConfirmationState_t state = { };
    float confirmedNorth = 0.0f;
    float confirmedEast = 0.0f;

    EXPECT_FALSE(markerGuidanceUpdateTargetConfirmation(
        &state, 100.0f, 100.0f, 100, 20, 1000, 500, &confirmedNorth, &confirmedEast));
    EXPECT_FALSE(markerGuidanceUpdateTargetConfirmation(
        &state, 100.0f, 100.0f, 100, 20, 1100, 500, &confirmedNorth, &confirmedEast));
    ASSERT_EQ(2, state.sampleCount);

    // The MSP handler calls this for malformed, out-of-range, or
    // position-unavailable samples, so confirmation remains consecutive.
    markerGuidanceResetTargetConfirmation(&state);

    EXPECT_FALSE(markerGuidanceUpdateTargetConfirmation(
        &state, 100.0f, 100.0f, 100, 20, 1200, 500, &confirmedNorth, &confirmedEast));
    EXPECT_EQ(1, state.sampleCount);
}

TEST(MarkerGuidanceLogicTest, PrelandingReadinessRequiresConfirmedOwnedTargetAndLowSpeed)
{
    EXPECT_TRUE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 50.0f, 75, 100, 0, 20));
    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        true, true, true, true, true, 50.0f, 75, 100, 0, 20));
    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 76.0f, 75, 100, 0, 20));
    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 50.0f, 75, 441, 0, 20));
    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        true, false, true, false, true, 50.0f, 75, 100, 0, 20));
}

TEST(MarkerGuidanceLogicTest, HeldPrelandingTargetWaitsThenRequiresPositionAndSpeed)
{
    EXPECT_FALSE(markerGuidancePrelandingHeldTargetReady(
        false, false, true, true, 20.0f, 75, 25.0f, 80, 5));
    EXPECT_TRUE(markerGuidancePrelandingHeldTargetReady(
        true, false, true, true, 20.0f, 75, 25.0f, 80, 5));
    EXPECT_FALSE(markerGuidancePrelandingHeldTargetReady(
        true, true, true, true, 20.0f, 75, 25.0f, 80, 5));
    EXPECT_FALSE(markerGuidancePrelandingHeldTargetReady(
        true, false, false, true, 20.0f, 75, 25.0f, 80, 5));
    EXPECT_FALSE(markerGuidancePrelandingHeldTargetReady(
        true, false, true, false, 20.0f, 75, 25.0f, 80, 5));
    EXPECT_FALSE(markerGuidancePrelandingHeldTargetReady(
        true, false, true, true, 76.0f, 75, 25.0f, 80, 5));
    EXPECT_FALSE(markerGuidancePrelandingHeldTargetReady(
        true, false, true, true, 20.0f, 75, 121.0f, 80, 5));
}

TEST(MarkerGuidanceLogicTest, PrelandingHoldRequiresExplicitLockAndConfirmedOwnedPlTarget)
{
    EXPECT_TRUE(markerGuidanceShouldHoldPrelandingTarget(true, true, true, true));
    EXPECT_FALSE(markerGuidanceShouldHoldPrelandingTarget(false, true, true, true));
    EXPECT_FALSE(markerGuidanceShouldHoldPrelandingTarget(true, false, true, true));
    EXPECT_FALSE(markerGuidanceShouldHoldPrelandingTarget(true, true, false, true));
    EXPECT_FALSE(markerGuidanceShouldHoldPrelandingTarget(true, true, true, false));
}

TEST(MarkerGuidanceLogicTest, PrelandingRadiusUsesMarkerAglLandingGeometry)
{
    EXPECT_TRUE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 50.0f, 75, 10000, 1000, 5));
    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 50.0f, 75, 10201, 1000, 5));
    EXPECT_TRUE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 50.0f, 75, 100, 0, 5));
}

TEST(MarkerGuidanceLogicTest, PrelandingRadiusTightensContinuouslyDuringDescent)
{
    const uint32_t eightyCentimetresSquared = 80U * 80U;

    EXPECT_TRUE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 50.0f, 75,
        eightyCentimetresSquared, 1000, 5));
    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 50.0f, 75,
        eightyCentimetresSquared, 500, 5));
}

TEST(MarkerGuidanceLogicTest, PrelandingRadiusUsesInclusiveBoundaryAndRejectsBeyondIt)
{
    EXPECT_TRUE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 75.0f, 75,
        100U * 100U, 1000, 5));
    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 75.0f, 75,
        101U * 101U, 1000, 5));
}

TEST(MarkerGuidanceLogicTest, PrelandingRadiusHonorsConfiguredRadiusAndAbsoluteMinimum)
{
    EXPECT_FLOAT_EQ(80.0f, markerGuidanceFullDescentOffsetCm(100, 80));
    EXPECT_FLOAT_EQ(100.0f, markerGuidanceFullDescentOffsetCm(1000, 5));

    EXPECT_TRUE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 0.0f, 75,
        80U * 80U, 100, 80));
    EXPECT_TRUE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 0.0f, 75,
        10U * 10U, 0, 0));
    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 0.0f, 75,
        11U * 11U, 0, 0));
}

TEST(MarkerGuidanceLogicTest, PrelandingReadinessRequiresEverySafetyInput)
{
    constexpr uint32_t centeredOffsetSquared = 5U * 5U;

    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        false, true, true, false, true, 0.0f, 75,
        centeredOffsetSquared, 1000, 5));
    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        true, false, true, false, true, 0.0f, 75,
        centeredOffsetSquared, 1000, 5));
    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        true, true, false, false, true, 0.0f, 75,
        centeredOffsetSquared, 1000, 5));
    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        true, true, true, true, true, 0.0f, 75,
        centeredOffsetSquared, 1000, 5));
    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        true, true, true, false, false, 0.0f, 75,
        centeredOffsetSquared, 1000, 5));
    EXPECT_FALSE(markerGuidancePrelandingXyReady(
        true, true, true, false, true, 75.1f, 75,
        centeredOffsetSquared, 1000, 5));
}

TEST(MarkerGuidanceLogicTest, RetrySettleUsesConservativeSpeedLimit)
{
    EXPECT_EQ(75, markerGuidanceRetrySettleSpeedLimit(0));
    EXPECT_EQ(50, markerGuidanceRetrySettleSpeedLimit(50));
    EXPECT_EQ(75, markerGuidanceRetrySettleSpeedLimit(75));
    EXPECT_EQ(75, markerGuidanceRetrySettleSpeedLimit(500));
}

TEST(MarkerGuidanceLogicTest, RetrySettleRequiresTrustedLowSpeedAndLevelAttitude)
{
    EXPECT_TRUE(markerGuidanceRetrySettleConditionsMet(true, 75.0f, 100, 75));
    EXPECT_FALSE(markerGuidanceRetrySettleConditionsMet(false, 0.0f, 0, 75));
    EXPECT_FALSE(markerGuidanceRetrySettleConditionsMet(true, 75.1f, 0, 75));
    EXPECT_FALSE(markerGuidanceRetrySettleConditionsMet(true, 0.0f, 101, 75));
}

TEST(MarkerGuidanceLogicTest, RetrySettleMustRemainStableAndResetsOnMotion)
{
    markerGuidanceRetrySettleState_t state = { };

    EXPECT_FALSE(markerGuidanceUpdateRetrySettle(&state, true, 1000));
    EXPECT_FALSE(markerGuidanceUpdateRetrySettle(&state, true, 1499));
    EXPECT_TRUE(markerGuidanceUpdateRetrySettle(&state, true, 1500));

    EXPECT_FALSE(markerGuidanceUpdateRetrySettle(&state, false, 1501));
    EXPECT_FALSE(state.active);
    EXPECT_FALSE(markerGuidanceUpdateRetrySettle(&state, true, 1600));
    EXPECT_TRUE(markerGuidanceUpdateRetrySettle(&state, true, 2100));
}

TEST(MarkerGuidanceLogicTest, RetrySettleHandlesTimerWrap)
{
    markerGuidanceRetrySettleState_t state = { };

    const uint32_t startMs = UINT32_MAX - 99;
    EXPECT_FALSE(markerGuidanceUpdateRetrySettle(&state, true, startMs));
    EXPECT_FALSE(markerGuidanceUpdateRetrySettle(&state, true, 399));
    EXPECT_TRUE(markerGuidanceUpdateRetrySettle(&state, true, 400));
}

TEST(MarkerGuidanceLogicTest, RetryClimbStopsAtRequestedAltitudeOrTimeout)
{
    EXPECT_FALSE(markerGuidanceRetryClimbFinished(true, 1000.0f, 1199.9f, 200, false));
    EXPECT_TRUE(markerGuidanceRetryClimbFinished(true, 1000.0f, 1200.0f, 200, false));
    EXPECT_TRUE(markerGuidanceRetryClimbFinished(true, 1000.0f, 1300.0f, 200, false));
    EXPECT_TRUE(markerGuidanceRetryClimbFinished(true, 1000.0f, 1000.0f, 200, true));
}

TEST(MarkerGuidanceLogicTest, RetryClimbUsesTimeoutWhenAltitudeIsUnavailable)
{
    EXPECT_FALSE(markerGuidanceRetryClimbFinished(false, 1000.0f, 1500.0f, 200, false));
    EXPECT_TRUE(markerGuidanceRetryClimbFinished(false, 1000.0f, 1000.0f, 200, true));
}

TEST(MarkerGuidanceLogicTest, InvalidPoseDoesNotModifyResolvedOutput)
{
    const markerGuidancePoseUpdate_t invalidUpdate = { 10, 20, 1801, 100 };
    markerGuidanceResolvedPose_t resolved = { 1.0f, 2.0f, 300, 400 };

    EXPECT_FALSE(markerGuidanceTryResolvePose(&invalidUpdate, 1000, 0, &resolved));
    EXPECT_FLOAT_EQ(1.0f, resolved.offsetNorthCm);
    EXPECT_FLOAT_EQ(2.0f, resolved.offsetEastCm);
    EXPECT_EQ(300, resolved.targetHeadingCd);
    EXPECT_EQ(400, resolved.markerAglCm);
}

TEST(MarkerGuidanceLogicTest, NorthEastOffsetsDoNotDependOnCurrentYaw)
{
    const markerGuidancePoseUpdate_t update = { 100, -50, 0, 100 };
    markerGuidanceResolvedPose_t resolved = { };

    ASSERT_TRUE(markerGuidanceTryResolvePose(&update, 1000, 27000, &resolved));
    EXPECT_FLOAT_EQ(100.0f, resolved.offsetNorthCm);
    EXPECT_FLOAT_EQ(-50.0f, resolved.offsetEastCm);
}

TEST(MarkerGuidanceLogicTest, PositionTargetUsesResolvedEarthOffsetWithoutCurrentYaw)
{
    const markerGuidancePoseUpdate_t update = { 100, 0, 0, 100 };
    markerGuidanceResolvedPose_t resolved = { };
    ASSERT_TRUE(markerGuidanceTryResolvePose(&update, 1000, 0, &resolved));

    float targetNorth = 0.0f;
    float targetEast = 0.0f;
    ASSERT_TRUE(markerGuidanceComputeHorizontalPositionTarget(
        1000.0f, 2000.0f,
        1000.0f + resolved.offsetNorthCm, 2000.0f + resolved.offsetEastCm,
        0.0f, 0.0f, 0.0f,
        &targetNorth, &targetEast));

    EXPECT_FLOAT_EQ(1100.0f, targetNorth);
    EXPECT_FLOAT_EQ(2000.0f, targetEast);
}

TEST(MarkerGuidanceLogicTest, AbsoluteMarkerTargetDoesNotMoveWithVehicle)
{
    float targetNorth = 0.0f;
    float targetEast = 0.0f;

    ASSERT_TRUE(markerGuidanceComputeHorizontalPositionTarget(
        0.0f, 0.0f, 200.0f, 0.0f, 0.0f, 0.0f, 100.0f,
        &targetNorth, &targetEast));
    EXPECT_FLOAT_EQ(100.0f, targetNorth);

    ASSERT_TRUE(markerGuidanceComputeHorizontalPositionTarget(
        50.0f, 0.0f, 200.0f, 0.0f, 0.0f, 0.0f, 100.0f,
        &targetNorth, &targetEast));
    EXPECT_FLOAT_EQ(100.0f, targetNorth);
    EXPECT_FLOAT_EQ(0.0f, targetEast);
}

TEST(MarkerGuidanceLogicTest, PositionTargetStopsAtCurrentPositionInsideRadius)
{
    float targetNorth = 0.0f;
    float targetEast = 0.0f;

    EXPECT_FALSE(markerGuidanceComputeHorizontalPositionTarget(
        20.0f, -30.0f, 100.0f, -30.0f, 0.0f, 0.0f, 100.0f,
        &targetNorth, &targetEast));
    EXPECT_FLOAT_EQ(20.0f, targetNorth);
    EXPECT_FLOAT_EQ(-30.0f, targetEast);
}

TEST(MarkerGuidanceLogicTest, CorrectionDirectionReportsCrossedAxesIndependently)
{
    markerGuidanceCorrectionDirectionState_t state = {};

    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 5, 100.0f, 100.0f));
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_BOTH, markerGuidanceCorrectionDirectionValidAxes(&state));

    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 5, 2.0f, 40.0f));
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NORTH,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 5, -10.0f, 30.0f));

    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 5, -20.0f, 3.0f));
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_EAST,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 5, -30.0f, -8.0f));
}

TEST(MarkerGuidanceLogicTest, CorrectionDirectionRejectsUnobservedAndSameSideCrossings)
{
    markerGuidanceCorrectionDirectionState_t state = {};

    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 5, -20.0f, 30.0f));

    // A sign change outside the deadband is not enough to prove a crossing.
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 5, 20.0f, 30.0f));

    // Entering and leaving from the original side consumes the gate.
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 5, -2.0f, 30.0f));
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 5, -10.0f, 30.0f));

    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 5, -2.0f, 30.0f));
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NORTH,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 5, 10.0f, 30.0f));
}

TEST(MarkerGuidanceLogicTest, CorrectionDirectionResetClearsBothAxes)
{
    markerGuidanceCorrectionDirectionState_t state = {};

    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 5, 100.0f, -100.0f));
    markerGuidanceResetCorrectionDirection(&state);

    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE, markerGuidanceCorrectionDirectionValidAxes(&state));
    EXPECT_FALSE(state.north.deadbandEntered);
    EXPECT_FALSE(state.east.deadbandEntered);
    EXPECT_FLOAT_EQ(0.0f, state.north.referenceCm);
    EXPECT_FLOAT_EQ(0.0f, state.east.referenceCm);
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidanceCorrectionCrossedTargetAxes(nullptr, 5, 100.0f, 0.0f));
    markerGuidanceResetCorrectionDirection(nullptr);
}

TEST(MarkerGuidanceLogicTest, ZeroRadiusUsesDirectPerAxisSideChange)
{
    markerGuidanceCorrectionDirectionState_t state = {};

    EXPECT_EQ(MARKER_GUIDANCE_AXIS_NONE,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 0, 20.0f, 30.0f));
    EXPECT_EQ(MARKER_GUIDANCE_AXIS_EAST,
        markerGuidanceCorrectionCrossedTargetAxes(&state, 0, 10.0f, -0.1f));
}

TEST(MarkerGuidanceLogicTest, TargetHandoffRemovesOnlyEnoughIntegratorToStopWrongOutput)
{
    float integratorNorth = -30.0f;
    float integratorEast = 40.0f;

    ASSERT_TRUE(markerGuidanceReconcileIntegratorForTargetHandoff(
        MARKER_GUIDANCE_AXIS_BOTH,
        100.0f, 0.0f, -10.0f, 20.0f, &integratorNorth, &integratorEast));
    EXPECT_NEAR(-20.0f, integratorNorth, 0.001f);
    EXPECT_FLOAT_EQ(40.0f, integratorEast);
}

TEST(MarkerGuidanceLogicTest, TargetHandoffPreservesIntegratorWhenCompleteOutputIsCorrect)
{
    float integratorNorth = -30.0f;
    float integratorEast = 40.0f;

    EXPECT_FALSE(markerGuidanceReconcileIntegratorForTargetHandoff(
        MARKER_GUIDANCE_AXIS_BOTH,
        100.0f, 0.0f, 20.0f, 0.0f, &integratorNorth, &integratorEast));
    EXPECT_FLOAT_EQ(-30.0f, integratorNorth);
    EXPECT_FLOAT_EQ(40.0f, integratorEast);
}

TEST(MarkerGuidanceLogicTest, TargetHandoffNeutralizesWrongOutputOnEachAxis)
{
    const float errorNorth = -16.0f;
    const float errorEast = -27.0f;
    const float outputNorth = 37.0f;
    const float outputEast = 9.0f;
    float integratorNorth = 60.0f;
    float integratorEast = 47.0f;

    ASSERT_TRUE(markerGuidanceReconcileIntegratorForTargetHandoff(
        MARKER_GUIDANCE_AXIS_BOTH,
        errorNorth,
        errorEast,
        outputNorth,
        outputEast,
        &integratorNorth,
        &integratorEast));

    EXPECT_NEAR(23.0f, integratorNorth, 0.001f);
    EXPECT_NEAR(38.0f, integratorEast, 0.001f);
}

TEST(MarkerGuidanceLogicTest, TargetHandoffFixesOnlyTheCrossedAxisFromFlightRegression)
{
    float integratorNorth = -1.0f;
    float integratorEast = 32.0f;

    ASSERT_TRUE(markerGuidanceReconcileIntegratorForTargetHandoff(
        MARKER_GUIDANCE_AXIS_EAST,
        8.0f, -4.0f, 14.0f, 23.0f, &integratorNorth, &integratorEast));
    EXPECT_FLOAT_EQ(-1.0f, integratorNorth);
    EXPECT_NEAR(9.0f, integratorEast, 0.001f);
}

TEST(MarkerGuidanceLogicTest, TargetHandoffCannotRemoveMoreThanOpposingIntegrator)
{
    float integratorNorth = -30.0f;
    float integratorEast = 25.0f;

    ASSERT_TRUE(markerGuidanceReconcileIntegratorForTargetHandoff(
        MARKER_GUIDANCE_AXIS_BOTH,
        100.0f, 0.0f, -100.0f, 20.0f, &integratorNorth, &integratorEast));
    EXPECT_NEAR(0.0f, integratorNorth, 0.001f);
    EXPECT_FLOAT_EQ(25.0f, integratorEast);
}

TEST(MarkerGuidanceLogicTest, TargetHandoffUsesUnconstrainedOutputMagnitude)
{
    float integratorNorth = -500.0f;
    float integratorEast = 25.0f;

    ASSERT_TRUE(markerGuidanceReconcileIntegratorForTargetHandoff(
        MARKER_GUIDANCE_AXIS_BOTH,
        100.0f, 0.0f, -400.0f, 20.0f, &integratorNorth, &integratorEast));
    EXPECT_NEAR(-100.0f, integratorNorth, 0.001f);
    EXPECT_FLOAT_EQ(25.0f, integratorEast);
}

TEST(MarkerGuidanceLogicTest, TargetHandoffPreservesSupportingAndInvalidIntegratorInputs)
{
    float integratorNorth = 30.0f;
    float integratorEast = -10.0f;

    EXPECT_FALSE(markerGuidanceReconcileIntegratorForTargetHandoff(
        MARKER_GUIDANCE_AXIS_BOTH,
        100.0f, 0.0f, -5.0f, 0.0f, &integratorNorth, &integratorEast));
    EXPECT_FLOAT_EQ(30.0f, integratorNorth);
    EXPECT_FLOAT_EQ(-10.0f, integratorEast);
    EXPECT_FALSE(markerGuidanceReconcileIntegratorForTargetHandoff(
        MARKER_GUIDANCE_AXIS_NONE,
        0.0f, 0.0f, -5.0f, 0.0f, &integratorNorth, &integratorEast));
    EXPECT_FALSE(markerGuidanceReconcileIntegratorForTargetHandoff(
        MARKER_GUIDANCE_AXIS_BOTH,
        0.1f, 0.0f, -5.0f, 0.0f, &integratorNorth, &integratorEast));
    EXPECT_FALSE(markerGuidanceReconcileIntegratorForTargetHandoff(
        MARKER_GUIDANCE_AXIS_BOTH,
        100.0f, 0.0f, -5.0f, 0.0f, nullptr, &integratorEast));
    EXPECT_FALSE(markerGuidanceReconcileIntegratorForTargetHandoff(
        MARKER_GUIDANCE_AXIS_BOTH,
        100.0f, 0.0f, -5.0f, 0.0f, &integratorNorth, nullptr));
}

TEST(MarkerGuidanceLogicTest, LandingAltitudePauseKeepsOnePositionInsteadOfFollowingDescent)
{
    markerGuidanceAltitudeHoldState_t hold = {};
    ASSERT_TRUE(markerGuidanceUpdateAltitudeHold(&hold, true, true, 100.0f));
    for (float altitude : {95.0f, 80.0f, 67.0f, 34.0f}) {
        ASSERT_TRUE(markerGuidanceUpdateAltitudeHold(&hold, true, true, altitude));
        EXPECT_FLOAT_EQ(100.0f, hold.altitudeCm);
    }
}

TEST(MarkerGuidanceLogicTest, LandingAltitudePauseReleasesAndRecapturesAtNewAltitude)
{
    markerGuidanceAltitudeHoldState_t hold = {};
    ASSERT_TRUE(markerGuidanceUpdateAltitudeHold(&hold, true, true, 100.0f));
    // Resume, retry, manual Z input and recovery all release the hold request.
    EXPECT_FALSE(markerGuidanceUpdateAltitudeHold(&hold, false, true, 80.0f));
    EXPECT_FALSE(hold.active);
    ASSERT_TRUE(markerGuidanceUpdateAltitudeHold(&hold, true, true, 60.0f));
    EXPECT_FLOAT_EQ(60.0f, hold.altitudeCm);
}

TEST(MarkerGuidanceLogicTest, LandingAltitudePauseCannotReviveLockAfterEstimateLoss)
{
    markerGuidanceAltitudeHoldState_t hold = {};
    ASSERT_TRUE(markerGuidanceUpdateAltitudeHold(&hold, true, true, 100.0f));
    EXPECT_FALSE(markerGuidanceUpdateAltitudeHold(&hold, true, false, 80.0f));
    ASSERT_TRUE(markerGuidanceUpdateAltitudeHold(&hold, true, true, 60.0f));
    EXPECT_FLOAT_EQ(60.0f, hold.altitudeCm);
    EXPECT_FALSE(markerGuidanceUpdateAltitudeHold(&hold, true, true, NAN));
    EXPECT_FALSE(markerGuidanceUpdateAltitudeHold(&hold, true, true, INFINITY));
    ASSERT_TRUE(markerGuidanceUpdateAltitudeHold(&hold, true, true, -200.0f));
    EXPECT_FLOAT_EQ(-200.0f, hold.altitudeCm); // Local altitude need not be above Home.
}

TEST(MarkerGuidanceLogicTest, LandingMotionGateSlowsAtCentreWithResidualSidewaysSpeed)
{
    EXPECT_NEAR(0.5f, markerGuidanceLandingMotionDescentScale(0, 0, 0, 10, true, 100, 5, 50), 0.001f);
    EXPECT_FLOAT_EQ(0.0f, markerGuidanceLandingMotionDescentScale(0, 0, 0, 20, true, 100, 5, 50));
    EXPECT_FLOAT_EQ(1.0f, markerGuidanceLandingMotionDescentScale(0, 0, 0, 0, true, 100, 5, 50));
}

TEST(MarkerGuidanceLogicTest, LandingMotionGateIncludesOwnedStandbyInsideMarkerRadius)
{
    // No XY correction is needed inside the radius, but STANDBY still owns XY
    // braking. It must not bypass the residual-speed descent check.
    EXPECT_TRUE(markerGuidanceTargetOwnershipIsContinuous(true, true, false, true));
    EXPECT_LT(markerGuidanceLandingMotionDescentScale(0, 0, 0, 10, true, 100, 5, 50), 1.0f);
    EXPECT_FALSE(markerGuidanceTargetOwnershipIsContinuous(false, true, false, true));
    EXPECT_FALSE(markerGuidanceTargetOwnershipIsContinuous(true, false, false, true));
    EXPECT_FALSE(markerGuidanceTargetOwnershipIsContinuous(true, true, false, false));
}

TEST(MarkerGuidanceLogicTest, LandingMotionGateNeverRelaxesCurrentOffsetLimit)
{
    // Approaching the target is allowed, but not at full descent while off centre.
    EXPECT_NEAR(0.5f, markerGuidanceLandingMotionDescentScale(20, 0, 10, 0, true, 100, 5, 50), 0.001f);
    EXPECT_FLOAT_EQ(0.0f, markerGuidanceLandingMotionDescentScale(40, 0, 20, 0, true, 100, 5, 50));
    // The same speed away from the marker must be more restrictive.
    EXPECT_FLOAT_EQ(0.0f, markerGuidanceLandingMotionDescentScale(20, 0, -10, 0, true, 100, 5, 50));
}

TEST(MarkerGuidanceLogicTest, LandingMotionGateIsIndependentOfAxesAndDirection)
{
    const float reference = markerGuidanceLandingMotionDescentScale(3, 4, -6, -8, true, 100, 5, 50);
    EXPECT_FLOAT_EQ(reference, markerGuidanceLandingMotionDescentScale(-3, -4, 6, 8, true, 100, 5, 50));
    EXPECT_FLOAT_EQ(reference, markerGuidanceLandingMotionDescentScale(4, 3, -8, -6, true, 100, 5, 50));
}

TEST(MarkerGuidanceLogicTest, LandingMotionGatePreservesGeometryWithoutTrustedVelocity)
{
    const float geometry = markerGuidanceLandingDescentScale(20, 100, 5);
    EXPECT_FLOAT_EQ(geometry, markerGuidanceLandingMotionDescentScale(20, 0, 100, 0, false, 100, 5, 50));
    EXPECT_FLOAT_EQ(geometry, markerGuidanceLandingMotionDescentScale(20, 0, NAN, 0, true, 100, 5, 50));
    EXPECT_FLOAT_EQ(geometry, markerGuidanceLandingMotionDescentScale(20, 0, 0, INFINITY, true, 100, 5, 50));
    for (float rate : {0.0f, -50.0f, INFINITY, NAN}) {
        EXPECT_FLOAT_EQ(geometry, markerGuidanceLandingMotionDescentScale(20, 0, 100, 0, true, 100, 5, rate));
    }
    EXPECT_FLOAT_EQ(1.0f, markerGuidanceLandingMotionDescentScale(20, 0, 100, 0, true, 0, 5, 50));
}

TEST(MarkerGuidanceLogicTest, LandingMotionGateIsContinuousAndBounded)
{
    float previous = 1.0f;
    for (int i = 0; i <= 200; ++i) {
        const float scale = markerGuidanceLandingMotionDescentScale(0, 0, i * 0.1f, 0, true, 100, 5, 50);
        EXPECT_GE(scale, 0.0f);
        EXPECT_LE(scale, previous);
        EXPECT_LE(previous - scale, 0.011f);
        previous = scale;
    }
}

TEST(MarkerGuidanceLogicTest, LandingMotionGateDoesNotChangeStationaryGeometryOrZeroRadius)
{
    for (unsigned height : {0, 7, 50, 100, 1000, 65535}) {
        for (unsigned radius : {0, 5, 20, 5000}) {
            for (float offset : {0.0f, 5.0f, 20.0f, 100.0f, 5000.0f}) {
                EXPECT_FLOAT_EQ(markerGuidanceLandingDescentScale(offset, height, radius),
                    markerGuidanceLandingMotionDescentScale(offset, 0, 0, 0, true, height, radius, 50));
            }
        }
    }
}

TEST(MarkerGuidanceLogicTest, LandingDescentIsFullWhenMarkerIsCentered)
{
    EXPECT_FLOAT_EQ(1.0f, markerGuidanceLandingDescentScale(20.0f, 200, 20));
    EXPECT_FLOAT_EQ(1.0f, markerGuidanceLandingDescentScale(80.0f, 1000, 20));
}

TEST(MarkerGuidanceLogicTest, LandingDescentSlowsContinuouslyWithVisualOffset)
{
    EXPECT_NEAR(0.5f, markerGuidanceLandingDescentScale(200.0f, 1000, 20), 0.001f);
    EXPECT_NEAR(0.75f, markerGuidanceLandingDescentScale(150.0f, 1000, 20), 0.001f);
}

TEST(MarkerGuidanceLogicTest, LandingDescentHoldsWhenMarkerApproachesViewEdge)
{
    EXPECT_FLOAT_EQ(0.0f, markerGuidanceLandingDescentScale(300.0f, 1000, 20));
    EXPECT_FLOAT_EQ(0.0f, markerGuidanceLandingDescentScale(60.0f, 100, 20));
}

TEST(MarkerGuidanceLogicTest, LandingDescentUsesRadiusAndIgnoresMissingAgl)
{
    EXPECT_FLOAT_EQ(1.0f, markerGuidanceLandingDescentScale(50.0f, 100, 50));
    EXPECT_NEAR(0.5f, markerGuidanceLandingDescentScale(100.0f, 100, 50), 0.001f);
    EXPECT_FLOAT_EQ(1.0f, markerGuidanceLandingDescentScale(500.0f, 0, 20));
}

TEST(MarkerGuidanceLogicTest, LandingPositionResponseIncreasesAsDescentIsRestricted)
{
    EXPECT_FLOAT_EQ(1.0f, markerGuidanceLandingPositionResponseScale(20.0f, 200, 20));
    EXPECT_NEAR(1.5f, markerGuidanceLandingPositionResponseScale(200.0f, 1000, 20), 0.001f);
    EXPECT_FLOAT_EQ(2.0f, markerGuidanceLandingPositionResponseScale(300.0f, 1000, 20));
}

TEST(MarkerGuidanceLogicTest, LandingPositionResponseKeepsNormalNavBehaviorWithoutAgl)
{
    EXPECT_FLOAT_EQ(1.0f, markerGuidanceLandingPositionResponseScale(500.0f, 0, 20));
}

TEST(MarkerGuidanceLogicTest, LostLowAltitudeTargetSlowsDescentWhileRealigning)
{
    EXPECT_NEAR(0.33f, markerGuidanceLostTargetDescentScale(
        true, true, true, 0.0f, 0.0f, 11.7f, 0.0f, 30, 5), 0.01f);
    EXPECT_FLOAT_EQ(0.0f, markerGuidanceLostTargetDescentScale(
        true, true, true, 0.0f, 0.0f, 15.0f, 0.0f, 30, 5));
}

TEST(MarkerGuidanceLogicTest, LostLowAltitudeTargetDoesNotBlockNormalFallback)
{
    EXPECT_FLOAT_EQ(1.0f, markerGuidanceLostTargetDescentScale(
        false, true, true, 0.0f, 0.0f, 15.0f, 0.0f, 30, 5));
    EXPECT_FLOAT_EQ(1.0f, markerGuidanceLostTargetDescentScale(
        true, false, true, 0.0f, 0.0f, 15.0f, 0.0f, 30, 5));
    EXPECT_FLOAT_EQ(1.0f, markerGuidanceLostTargetDescentScale(
        true, true, false, 0.0f, 0.0f, 15.0f, 0.0f, 30, 5));
}

TEST(MarkerGuidanceLogicTest, ContainmentOffsetAndRadiusSelectNearestAllowedBoundary)
{
    float targetNorth = 0.0f;
    float targetEast = 0.0f;

    ASSERT_TRUE(markerGuidanceComputeHorizontalPositionTarget(
        300.0f, 0.0f, 0.0f, 0.0f, 100.0f, 0.0f, 50.0f,
        &targetNorth, &targetEast));
    EXPECT_FLOAT_EQ(150.0f, targetNorth);
    EXPECT_FLOAT_EQ(0.0f, targetEast);
}

TEST(MarkerGuidanceLogicTest, ResolvesHeadingSignAndWrapExamples)
{
    markerGuidanceResolvedPose_t resolved = { };
    markerGuidancePoseUpdate_t update = { 0, 0, 900, 100 };

    EXPECT_TRUE(markerGuidanceTryResolvePose(&update, 1000, 1000, &resolved));
    EXPECT_EQ(10000, resolved.targetHeadingCd);

    update.yawErrorDeciDeg = 200;
    EXPECT_TRUE(markerGuidanceTryResolvePose(&update, 1000, 35000, &resolved));
    EXPECT_EQ(1000, resolved.targetHeadingCd);

    update.yawErrorDeciDeg = -200;
    EXPECT_TRUE(markerGuidanceTryResolvePose(&update, 1000, 1000, &resolved));
    EXPECT_EQ(35000, resolved.targetHeadingCd);
}

TEST(MarkerGuidanceLogicTest, FreshMarkerHeadingOverridesLatestNavigationHeading)
{
    int32_t desiredHeadingCd = 27000;
    EXPECT_TRUE(markerGuidanceSelectHeadingOverride(
        true, MARKER_GUIDANCE_CONTEXT_LAND, true, false, false, false,
        true, true, 9000, false, 0, &desiredHeadingCd));
    EXPECT_EQ(9000, desiredHeadingCd);

    desiredHeadingCd = 18000;
    EXPECT_TRUE(markerGuidanceSelectHeadingOverride(
        true, MARKER_GUIDANCE_CONTEXT_LAND, true, false, false, false,
        true, true, 9000, false, 0, &desiredHeadingCd));
    EXPECT_EQ(9000, desiredHeadingCd);
}

TEST(MarkerGuidanceLogicTest, ManualYawFailsafeDisarmAndFixedWingBlockHeadingOverride)
{
    int32_t desiredHeadingCd = 27000;
    EXPECT_FALSE(markerGuidanceSelectHeadingOverride(
        true, MARKER_GUIDANCE_CONTEXT_LAND, true, false, false, true,
        true, true, 9000, false, 0, &desiredHeadingCd));
    EXPECT_FALSE(markerGuidanceSelectHeadingOverride(
        true, MARKER_GUIDANCE_CONTEXT_LAND, true, false, true, false,
        true, true, 9000, false, 0, &desiredHeadingCd));
    EXPECT_FALSE(markerGuidanceSelectHeadingOverride(
        true, MARKER_GUIDANCE_CONTEXT_LAND, false, false, false, false,
        true, true, 9000, false, 0, &desiredHeadingCd));
    EXPECT_FALSE(markerGuidanceSelectHeadingOverride(
        true, MARKER_GUIDANCE_CONTEXT_LAND, true, true, false, false,
        true, true, 9000, false, 0, &desiredHeadingCd));
    EXPECT_EQ(27000, desiredHeadingCd);
}

TEST(MarkerGuidanceLogicTest, LandRetainsLatchedHeadingAfterTargetLoss)
{
    int32_t desiredHeadingCd = 27000;
    EXPECT_TRUE(markerGuidanceSelectHeadingOverride(
        true, MARKER_GUIDANCE_CONTEXT_LAND, true, false, false, false,
        false, true, 0, true, 12000, &desiredHeadingCd));
    EXPECT_EQ(12000, desiredHeadingCd);
}

TEST(MarkerGuidanceLogicTest, PosholdRetainsLatchedHeadingAfterTargetLoss)
{
    int32_t desiredHeadingCd = 27000;
    EXPECT_TRUE(markerGuidanceSelectHeadingOverride(
        true, MARKER_GUIDANCE_CONTEXT_POSHOLD, true, false, false, false,
        false, true, 0, true, 12000, &desiredHeadingCd));
    EXPECT_EQ(12000, desiredHeadingCd);
}

TEST(MarkerGuidanceLogicTest, ManualYawRejectedSampleRequiresANewerPacket)
{
    EXPECT_FALSE(markerGuidanceHeadingSampleAllowed(10, 10));
    EXPECT_TRUE(markerGuidanceHeadingSampleAllowed(11, 10));
    EXPECT_FALSE(markerGuidanceHeadingSampleAllowed(0, 0));
}

TEST(MarkerGuidanceLogicTest, PositionTargetWaitsForBrakingCaptureAndManualControl)
{
    EXPECT_TRUE(markerGuidancePositionTargetAllowed(true, true, false, false, false));
    EXPECT_FALSE(markerGuidancePositionTargetAllowed(false, true, false, false, false));
    EXPECT_FALSE(markerGuidancePositionTargetAllowed(true, false, false, false, false));
    EXPECT_FALSE(markerGuidancePositionTargetAllowed(true, true, true, false, false));
    EXPECT_FALSE(markerGuidancePositionTargetAllowed(true, true, false, true, false));
    EXPECT_FALSE(markerGuidancePositionTargetAllowed(true, true, false, false, true));
}

TEST(MarkerGuidanceLogicTest, MarkerPositionTakeoverOnlyFollowsRollPitchOwnership)
{
    EXPECT_TRUE(markerGuidancePositionTakeoverActive(true));
    EXPECT_FALSE(markerGuidancePositionTakeoverActive(false));
}

TEST(MarkerGuidanceLogicTest, AcquisitionRequiresFreshContextSampleAndAvailablePositionPath)
{
    EXPECT_TRUE(markerGuidanceTargetCanBeAcquired(true, true, true, true));
    EXPECT_FALSE(markerGuidanceTargetCanBeAcquired(false, true, true, true));
    EXPECT_FALSE(markerGuidanceTargetCanBeAcquired(true, false, true, true));
    EXPECT_FALSE(markerGuidanceTargetCanBeAcquired(true, true, false, true));
    EXPECT_FALSE(markerGuidanceTargetCanBeAcquired(true, true, true, false));
}

TEST(MarkerGuidanceLogicTest, SuspendedPositionSampleRequiresANewerPacket)
{
    EXPECT_FALSE(markerGuidanceHeadingSampleAllowed(42, 42));
    EXPECT_TRUE(markerGuidanceHeadingSampleAllowed(43, 42));
}

TEST(MarkerGuidanceLogicTest, VtolRecoveryPausesOnlyAcquiredMarkerContext)
{
    EXPECT_TRUE(markerGuidanceVtolRecoveryShouldPause(true, true, MARKER_GUIDANCE_CONTEXT_POSHOLD));
    EXPECT_TRUE(markerGuidanceVtolRecoveryShouldPause(true, true, MARKER_GUIDANCE_CONTEXT_LAND));
    EXPECT_FALSE(markerGuidanceVtolRecoveryShouldPause(false, true, MARKER_GUIDANCE_CONTEXT_LAND));
    EXPECT_FALSE(markerGuidanceVtolRecoveryShouldPause(true, false, MARKER_GUIDANCE_CONTEXT_LAND));
    EXPECT_FALSE(markerGuidanceVtolRecoveryShouldPause(true, true, MARKER_GUIDANCE_CONTEXT_NONE));
}

TEST(MarkerGuidanceLogicTest, LandWithoutAcquisitionRetainsNavigationHeading)
{
    int32_t desiredHeadingCd = 27000;
    EXPECT_FALSE(markerGuidanceSelectHeadingOverride(
        true, MARKER_GUIDANCE_CONTEXT_LAND, true, false, false, false,
        false, false, 0, false, 12000, &desiredHeadingCd));
    EXPECT_EQ(27000, desiredHeadingCd);
}

TEST(MarkerGuidanceLogicTest, FreshReacquisitionReplacesLatchedLandHeading)
{
    int32_t desiredHeadingCd = 27000;
    EXPECT_TRUE(markerGuidanceSelectHeadingOverride(
        true, MARKER_GUIDANCE_CONTEXT_LAND, true, false, false, false,
        true, true, 6000, true, 12000, &desiredHeadingCd));
    EXPECT_EQ(6000, desiredHeadingCd);
}

TEST(MarkerGuidanceLogicTest, LeavingAndReenteringLandRequiresANewSample)
{
    EXPECT_TRUE(markerGuidanceLandSampleIsNewForContext(10, 9));
    EXPECT_FALSE(markerGuidanceLandSampleIsNewForContext(10, 10));
    EXPECT_FALSE(markerGuidanceLandSampleIsNewForContext(0, 0));
    EXPECT_TRUE(markerGuidanceLandSampleIsNewForContext(11, 10));
}

TEST(MarkerGuidanceLogicTest, ReenteredLandDoesNotUseFreshCacheBeforeNewContextAcquisition)
{
    int32_t desiredHeadingCd = 27000;
    EXPECT_FALSE(markerGuidanceSelectHeadingOverride(
        true, MARKER_GUIDANCE_CONTEXT_LAND, true, false, false, false,
        true, false, 9000, false, 0, &desiredHeadingCd));
    EXPECT_EQ(27000, desiredHeadingCd);
}

TEST(MarkerGuidanceLogicTest, SampleSequenceSkipsInvalidZeroOnWrap)
{
    EXPECT_EQ(1U, markerGuidanceNextSampleSequence(0));
    EXPECT_EQ(11U, markerGuidanceNextSampleSequence(10));
    EXPECT_EQ(1U, markerGuidanceNextSampleSequence(UINT32_MAX));
}

TEST(MarkerGuidanceLogicTest, StateDeadlineComparisonHandlesMillisWrap)
{
    EXPECT_FALSE(markerGuidanceDeadlineReached(100, 101));
    EXPECT_TRUE(markerGuidanceDeadlineReached(101, 101));
    EXPECT_TRUE(markerGuidanceDeadlineReached(102, 101));

    const uint32_t deadlineAfterWrap = 5;
    EXPECT_FALSE(markerGuidanceDeadlineReached(UINT32_MAX - 4, deadlineAfterWrap));
    EXPECT_TRUE(markerGuidanceDeadlineReached(deadlineAfterWrap, deadlineAfterWrap));
}

TEST(MarkerGuidanceLogicTest, ContainmentNeverClaimsHeading)
{
    int32_t desiredHeadingCd = 27000;
    EXPECT_FALSE(markerGuidanceSelectHeadingOverride(
        false, MARKER_GUIDANCE_CONTEXT_POSHOLD, true, false, false, false,
        true, true, 6000, false, 0, &desiredHeadingCd));
    EXPECT_EQ(27000, desiredHeadingCd);
}

TEST(MarkerGuidanceLogicTest, LowMarkerOrTrustedInavAglSuppressesRetry)
{
    EXPECT_TRUE(markerGuidanceRetryIsSuppressedByAltitude(100, false, 0.0f, true));
    EXPECT_TRUE(markerGuidanceRetryIsSuppressedByAltitude(100, true, 99.0f, false));
    EXPECT_FALSE(markerGuidanceRetryIsSuppressedByAltitude(100, true, 101.0f, false));
}

TEST(MarkerGuidanceLogicTest, RetryAltitudeRequiresTrustedFiniteNonnegativeAgl)
{
    EXPECT_TRUE(markerGuidanceRetryIsSuppressedByAltitude(50, true, 50.0f, false));
    EXPECT_TRUE(markerGuidanceRetryIsSuppressedByAltitude(50, true, 0.0f, false));
    EXPECT_FALSE(markerGuidanceRetryIsSuppressedByAltitude(50, false, 40.0f, false));
    for (const float height : {-1.0f, 50.1f, INFINITY, -INFINITY, NAN}) {
        EXPECT_FALSE(markerGuidanceRetryIsSuppressedByAltitude(50, true, height, false));
        EXPECT_TRUE(markerGuidanceRetryIsSuppressedByAltitude(50, true, height, true));
    }
}

TEST(MarkerGuidanceLogicTest, HighMarkerAglDoesNotForceRetryAndZeroThresholdDisablesSuppression)
{
    EXPECT_FALSE(markerGuidanceRetryIsSuppressedByAltitude(100, false, 0.0f, false));
    EXPECT_FALSE(markerGuidanceRetryIsSuppressedByAltitude(0, true, 1.0f, true));
}
