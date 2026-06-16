#include <gtest/gtest.h>

extern "C" {
#include "navigation/navigation_vtol_mc_protection_logic.h"
}

TEST(VtolMcProtectionLogicTest, DetectsOnlyPairedMultirotorVtolMcMode)
{
    EXPECT_TRUE(vtolMcProtectionDetectVtolMcMode(true, true, true));
    EXPECT_FALSE(vtolMcProtectionDetectVtolMcMode(false, true, true));
    EXPECT_FALSE(vtolMcProtectionDetectVtolMcMode(true, false, true));
    EXPECT_FALSE(vtolMcProtectionDetectVtolMcMode(true, true, false));
}

TEST(VtolMcProtectionLogicTest, DefaultOffKeepsLegacyThrottleBounds)
{
    const vtolMcProtectionThrottleBounds_t bounds = vtolMcProtectionComputeThrottleBounds(
        false,
        1000,
        1500,
        2000,
        15);

    EXPECT_EQ(1000, bounds.min);
    EXPECT_EQ(2000, bounds.max);
    EXPECT_FALSE(bounds.reserveShrunk);
}

TEST(VtolMcProtectionLogicTest, AppliesThrottleReserveBeforePidBounds)
{
    const vtolMcProtectionThrottleBounds_t bounds = vtolMcProtectionComputeThrottleBounds(
        true,
        1000,
        1500,
        2000,
        15);

    EXPECT_EQ(1150, bounds.min);
    EXPECT_EQ(1850, bounds.max);
    EXPECT_FALSE(bounds.reserveShrunk);
}

TEST(VtolMcProtectionLogicTest, ShrinksReserveToKeepHoverThrottleReachable)
{
    const vtolMcProtectionThrottleBounds_t lowHoverBounds = vtolMcProtectionComputeThrottleBounds(
        true,
        1000,
        1100,
        2000,
        20);

    EXPECT_EQ(1100, lowHoverBounds.min);
    EXPECT_EQ(1800, lowHoverBounds.max);
    EXPECT_TRUE(lowHoverBounds.reserveShrunk);

    const vtolMcProtectionThrottleBounds_t highHoverBounds = vtolMcProtectionComputeThrottleBounds(
        true,
        1000,
        1900,
        2000,
        20);

    EXPECT_EQ(1200, highHoverBounds.min);
    EXPECT_EQ(1900, highHoverBounds.max);
    EXPECT_TRUE(highHoverBounds.reserveShrunk);
}

TEST(VtolMcProtectionLogicTest, ThrottleBoundsHandleDisabledReserveAndInvalidRange)
{
    const vtolMcProtectionThrottleBounds_t zeroReserveBounds = vtolMcProtectionComputeThrottleBounds(
        true,
        1000,
        1500,
        2000,
        0);

    EXPECT_EQ(1000, zeroReserveBounds.min);
    EXPECT_EQ(2000, zeroReserveBounds.max);
    EXPECT_FALSE(zeroReserveBounds.reserveShrunk);

    const vtolMcProtectionThrottleBounds_t invalidRangeBounds = vtolMcProtectionComputeThrottleBounds(
        true,
        1500,
        1500,
        1400,
        20);

    EXPECT_EQ(1500, invalidRangeBounds.min);
    EXPECT_EQ(1400, invalidRangeBounds.max);
    EXPECT_FALSE(invalidRangeBounds.reserveShrunk);
}

TEST(VtolMcProtectionLogicTest, SettleTimerRequiresContinuousStableConditions)
{
    vtolMcProtectionSettleState_t state = {};

    EXPECT_EQ(1000, VTOL_MC_SETTLE_TIME_MS);
    EXPECT_EQ(3000, VTOL_MC_FALLBACK_SETTLE_TIME_MS);
    EXPECT_EQ(VTOL_MC_SETTLE_TIME_MS, vtolMcProtectionSettleTimeMs(true));
    EXPECT_EQ(VTOL_MC_FALLBACK_SETTLE_TIME_MS, vtolMcProtectionSettleTimeMs(false));

    EXPECT_FALSE(vtolMcProtectionUpdateSettleState(&state, true, VTOL_MC_SETTLE_TIME_MS, 100));
    EXPECT_EQ(0, state.elapsedMs);

    EXPECT_FALSE(vtolMcProtectionUpdateSettleState(&state, true, VTOL_MC_SETTLE_TIME_MS, 600));
    EXPECT_EQ(500, state.elapsedMs);

    EXPECT_FALSE(vtolMcProtectionUpdateSettleState(&state, false, VTOL_MC_SETTLE_TIME_MS, 700));
    EXPECT_EQ(0, state.elapsedMs);

    EXPECT_FALSE(vtolMcProtectionUpdateSettleState(&state, true, VTOL_MC_SETTLE_TIME_MS, 800));
    EXPECT_TRUE(vtolMcProtectionUpdateSettleState(&state, true, VTOL_MC_SETTLE_TIME_MS, 1800));
}

TEST(VtolMcProtectionLogicTest, SettleTimerSaturatesElapsedTime)
{
    vtolMcProtectionSettleState_t state = {};

    EXPECT_FALSE(vtolMcProtectionUpdateSettleState(&state, true, UINT16_MAX, 1));
    EXPECT_TRUE(vtolMcProtectionUpdateSettleState(&state, true, UINT16_MAX, 70000));
    EXPECT_EQ(UINT16_MAX, state.elapsedMs);
}

TEST(VtolMcProtectionLogicTest, SettleConditionsUseDerivedThresholds)
{
    EXPECT_EQ(75, vtolMcProtectionHorizontalSettleSpeedCmS(0));
    EXPECT_EQ(50, vtolMcProtectionHorizontalSettleSpeedCmS(50));
    EXPECT_EQ(50, vtolMcProtectionVerticalSettleSpeedCmS(50));
    EXPECT_EQ(100, vtolMcProtectionVerticalSettleSpeedCmS(500));
    EXPECT_EQ(200, vtolMcProtectionSettleAttitudeLimitDeciDeg(35));
    EXPECT_EQ(150, vtolMcProtectionSettleAttitudeLimitDeciDeg(15));

    EXPECT_TRUE(vtolMcProtectionSettleConditionsMet(70.0f, -50.0f, 150, 75, 100, 200));
    EXPECT_FALSE(vtolMcProtectionSettleConditionsMet(80.0f, -50.0f, 150, 75, 100, 200));
    EXPECT_FALSE(vtolMcProtectionSettleConditionsMet(70.0f, -120.0f, 150, 75, 100, 200));
    EXPECT_FALSE(vtolMcProtectionSettleConditionsMet(70.0f, -50.0f, 250, 75, 100, 200));
}

TEST(VtolMcProtectionLogicTest, VelocityFallbackSettleUsesVerticalSpeedAndAttitudeOnly)
{
    EXPECT_FALSE(vtolMcProtectionUsingVelocityFallback(true, true));
    EXPECT_FALSE(vtolMcProtectionUsingVelocityFallback(false, false));
    EXPECT_TRUE(vtolMcProtectionUsingVelocityFallback(false, true));

    EXPECT_TRUE(vtolMcProtectionSettleConditionsMetWithFallback(
        false, true, 500.0f, -50.0f, 150, 75, 100, 200));
    EXPECT_FALSE(vtolMcProtectionSettleConditionsMetWithFallback(
        false, false, 0.0f, -50.0f, 150, 75, 100, 200));
    EXPECT_FALSE(vtolMcProtectionSettleConditionsMetWithFallback(
        false, true, 0.0f, -120.0f, 150, 75, 100, 200));
    EXPECT_FALSE(vtolMcProtectionSettleConditionsMetWithFallback(
        false, true, 0.0f, -50.0f, 250, 75, 100, 200));

    EXPECT_FALSE(vtolMcProtectionSettleConditionsMetWithFallback(
        true, true, 500.0f, -50.0f, 150, 75, 100, 200));
    EXPECT_FALSE(vtolMcProtectionSettleConditionsMetWithFallback(
        true, false, 0.0f, 0.0f, 0, 75, 100, 200));
}

TEST(VtolMcProtectionLogicTest, LandingCaptureRadiusCapsLargeWaypointRadius)
{
    EXPECT_EQ(100, vtolMcProtectionLandingCaptureRadiusCm(10000));
    EXPECT_EQ(80, vtolMcProtectionLandingCaptureRadiusCm(80));
}

TEST(VtolMcProtectionLogicTest, SoftAltitudeRelaxationOnlyAppliesDuringCaptureOrTransition)
{
    EXPECT_TRUE(vtolMcProtectionShouldRelaxAltitude(
        true, true, true, true, false, false, false, 120.0f, 75, false));
    EXPECT_TRUE(vtolMcProtectionShouldRelaxAltitude(
        true, true, true, true, false, false, false, 20.0f, 75, true));

    EXPECT_FALSE(vtolMcProtectionShouldRelaxAltitude(
        false, true, true, true, false, false, false, 120.0f, 75, false));
    EXPECT_FALSE(vtolMcProtectionShouldRelaxAltitude(
        true, false, true, true, false, false, false, 120.0f, 75, false));
    EXPECT_FALSE(vtolMcProtectionShouldRelaxAltitude(
        true, true, false, true, false, false, false, 120.0f, 75, false));
    EXPECT_TRUE(vtolMcProtectionShouldRelaxAltitude(
        true, true, false, true, false, false, false, 120.0f, 75, true));
    EXPECT_FALSE(vtolMcProtectionShouldRelaxAltitude(
        true, true, true, false, false, false, false, 120.0f, 75, false));
    EXPECT_FALSE(vtolMcProtectionShouldRelaxAltitude(
        true, true, true, true, true, false, false, 120.0f, 75, false));
    EXPECT_FALSE(vtolMcProtectionShouldRelaxAltitude(
        true, true, true, true, false, true, false, 120.0f, 75, false));
    EXPECT_FALSE(vtolMcProtectionShouldRelaxAltitude(
        true, true, true, true, false, false, true, 120.0f, 75, false));
    EXPECT_FALSE(vtolMcProtectionShouldRelaxAltitude(
        true, true, true, true, false, false, false, 75.0f, 75, false));
}

TEST(VtolMcProtectionLogicTest, BailoutAngleLimitUsesBankAngleWithSafeClamps)
{
    EXPECT_EQ(450, vtolMcProtectionBailoutAngleLimitDeciDeg(20));
    EXPECT_EQ(500, vtolMcProtectionBailoutAngleLimitDeciDeg(35));
    EXPECT_EQ(600, vtolMcProtectionBailoutAngleLimitDeciDeg(80));
}

TEST(VtolMcProtectionLogicTest, CommandShapingIsContinuousAndPreservesSign)
{
    EXPECT_FLOAT_EQ(1.0f, vtolMcProtectionCommandScaleForSpeed(250.0f));
    EXPECT_FLOAT_EQ(1.0f, vtolMcProtectionCommandScaleForSpeed(VTOL_MC_COMMAND_SHAPE_START_CM_S));
    EXPECT_FLOAT_EQ(0.75f, vtolMcProtectionCommandScaleForSpeed(550.0f));
    EXPECT_FLOAT_EQ(0.5f, vtolMcProtectionCommandScaleForSpeed(VTOL_MC_COMMAND_SHAPE_FULL_CM_S));
    EXPECT_FLOAT_EQ(0.5f, vtolMcProtectionCommandScaleForSpeed(900.0f));
    EXPECT_EQ(750, vtolMcProtectionCommandScalePermille(0.75f));

    EXPECT_EQ(0, vtolMcProtectionApplyCommandScale(0, 0.5f));
    EXPECT_EQ(150, vtolMcProtectionApplyCommandScale(200, 0.75f));
    EXPECT_EQ(-150, vtolMcProtectionApplyCommandScale(-200, 0.75f));
    EXPECT_EQ(1, vtolMcProtectionApplyCommandScale(1, 0.5f));
    EXPECT_EQ(-1, vtolMcProtectionApplyCommandScale(-1, 0.5f));
}
