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

    EXPECT_EQ(-123, update.offsetForwardCm);
    EXPECT_EQ(456, update.offsetRightCm);
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

    EXPECT_FALSE(markerGuidanceTryResolvePose(&invalidUpdate, 1000, 1.0f, 0.0f, 0, &resolved));
    EXPECT_FLOAT_EQ(1.0f, resolved.offsetNorthCm);
    EXPECT_FLOAT_EQ(2.0f, resolved.offsetEastCm);
    EXPECT_EQ(300, resolved.targetHeadingCd);
    EXPECT_EQ(400, resolved.markerAglCm);
}

static void expectNorthTarget(const markerGuidancePoseUpdate_t &update, float cosYaw, float sinYaw)
{
    markerGuidanceResolvedPose_t resolved = { };
    ASSERT_TRUE(markerGuidanceTryResolvePose(&update, 1000, cosYaw, sinYaw, 0, &resolved));
    EXPECT_NEAR(100.0f, resolved.offsetNorthCm, 0.001f);
    EXPECT_NEAR(0.0f, resolved.offsetEastCm, 0.001f);
}

TEST(MarkerGuidanceLogicTest, BodyOffsetsResolveToSameEarthTargetAtZeroNinetyAndOneEightyDegrees)
{
    expectNorthTarget({ 100, 0, 0, 100 }, 1.0f, 0.0f);
    expectNorthTarget({ 0, -100, 0, 100 }, 0.0f, 1.0f);
    expectNorthTarget({ -100, 0, 0, 100 }, -1.0f, 0.0f);
}

TEST(MarkerGuidanceLogicTest, PositionTargetUsesResolvedEarthOffsetWithoutCurrentYaw)
{
    const markerGuidancePoseUpdate_t update = { 100, 0, 0, 100 };
    markerGuidanceResolvedPose_t resolved = { };
    ASSERT_TRUE(markerGuidanceTryResolvePose(&update, 1000, 1.0f, 0.0f, 0, &resolved));

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

    EXPECT_TRUE(markerGuidanceTryResolvePose(&update, 1000, 1.0f, 0.0f, 1000, &resolved));
    EXPECT_EQ(10000, resolved.targetHeadingCd);

    update.yawErrorDeciDeg = 200;
    EXPECT_TRUE(markerGuidanceTryResolvePose(&update, 1000, 1.0f, 0.0f, 35000, &resolved));
    EXPECT_EQ(1000, resolved.targetHeadingCd);

    update.yawErrorDeciDeg = -200;
    EXPECT_TRUE(markerGuidanceTryResolvePose(&update, 1000, 1.0f, 0.0f, 1000, &resolved));
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

TEST(MarkerGuidanceLogicTest, LowMarkerOrInavAglSuppressesRetry)
{
    EXPECT_TRUE(markerGuidanceRetryIsSuppressedByAltitude(100, false, 0.0f, true));
    EXPECT_TRUE(markerGuidanceRetryIsSuppressedByAltitude(100, true, 99.0f, false));
    EXPECT_FALSE(markerGuidanceRetryIsSuppressedByAltitude(100, true, 101.0f, false));
}

TEST(MarkerGuidanceLogicTest, HighMarkerAglDoesNotForceRetryAndZeroThresholdDisablesSuppression)
{
    EXPECT_FALSE(markerGuidanceRetryIsSuppressedByAltitude(100, false, 0.0f, false));
    EXPECT_FALSE(markerGuidanceRetryIsSuppressedByAltitude(0, true, 1.0f, true));
}
