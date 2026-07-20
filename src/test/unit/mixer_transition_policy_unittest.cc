#include <gtest/gtest.h>

extern "C" {
#include "flight/mixer_transition_logic.h"
}

TEST(MixerTransitionPolicyTest, RequestGatingWorksWhenFixedWingProfileIsIndexZero)
{
    const flyingPlatformType_e profileTypes[] = {
        PLATFORM_AIRPLANE,
        PLATFORM_TRICOPTER,
    };
    const int targetFwIndex = 0;

    EXPECT_TRUE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_MANUAL_TO_FW,
        false,
        true,
        true,
        false,
        profileTypes[targetFwIndex] == PLATFORM_AIRPLANE,
        isMultirotorTypePlatform(profileTypes[targetFwIndex])));

    EXPECT_FALSE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_RTH,
        false,
        true,
        true,
        false,
        profileTypes[targetFwIndex] == PLATFORM_AIRPLANE,
        isMultirotorTypePlatform(profileTypes[targetFwIndex])));

    EXPECT_TRUE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_RTH,
        false,
        true,
        true,
        true,
        profileTypes[targetFwIndex] == PLATFORM_AIRPLANE,
        isMultirotorTypePlatform(profileTypes[targetFwIndex])));
}

TEST(MixerTransitionPolicyTest, RequestGatingWorksWhenFixedWingProfileIsIndexOne)
{
    const flyingPlatformType_e profileTypes[] = {
        PLATFORM_TRICOPTER,
        PLATFORM_AIRPLANE,
    };
    const int targetFwIndex = 1;

    EXPECT_TRUE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_MISSION_TO_FW,
        false,
        true,
        true,
        false,
        profileTypes[targetFwIndex] == PLATFORM_AIRPLANE,
        isMultirotorTypePlatform(profileTypes[targetFwIndex])));

    EXPECT_FALSE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_MISSION_TO_FW,
        false,
        true,
        false,
        false,
        profileTypes[targetFwIndex] == PLATFORM_AIRPLANE,
        isMultirotorTypePlatform(profileTypes[targetFwIndex])));
}

TEST(MixerTransitionPolicyTest, LandRequestNeedsAutomatedSwitchWhenMultirotorProfileIsIndexZero)
{
    const flyingPlatformType_e profileTypes[] = {
        PLATFORM_TRICOPTER,
        PLATFORM_AIRPLANE,
    };
    const int targetMcIndex = 0;

    EXPECT_FALSE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_LAND,
        true,
        false,
        true,
        false,
        profileTypes[targetMcIndex] == PLATFORM_AIRPLANE,
        isMultirotorTypePlatform(profileTypes[targetMcIndex])));

    EXPECT_TRUE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_LAND,
        true,
        false,
        true,
        true,
        profileTypes[targetMcIndex] == PLATFORM_AIRPLANE,
        isMultirotorTypePlatform(profileTypes[targetMcIndex])));
}

TEST(MixerTransitionPolicyTest, LandRequestNeedsAutomatedSwitchWhenMultirotorProfileIsIndexOne)
{
    const flyingPlatformType_e profileTypes[] = {
        PLATFORM_AIRPLANE,
        PLATFORM_TRICOPTER,
    };
    const int targetMcIndex = 1;

    EXPECT_FALSE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_LAND,
        true,
        false,
        true,
        false,
        profileTypes[targetMcIndex] == PLATFORM_AIRPLANE,
        isMultirotorTypePlatform(profileTypes[targetMcIndex])));

    EXPECT_TRUE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_LAND,
        true,
        false,
        true,
        true,
        profileTypes[targetMcIndex] == PLATFORM_AIRPLANE,
        isMultirotorTypePlatform(profileTypes[targetMcIndex])));
}

TEST(MixerTransitionPolicyTest, NavigationFwToMcProtectionNeedsAutomatedSwitch)
{
    EXPECT_FALSE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_FW_TO_MC_PROTECTION,
        true,
        false,
        true,
        false,
        false,
        true));

    EXPECT_TRUE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_FW_TO_MC_PROTECTION,
        true,
        false,
        true,
        true,
        false,
        true));

    EXPECT_FALSE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_FW_TO_MC_PROTECTION,
        true,
        false,
        true,
        true,
        true,
        false));
}

TEST(MixerTransitionPolicyTest, ManualRequestsNeedMixerProfileModeAndMatchingTargetType)
{
    EXPECT_FALSE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_MANUAL_TO_FW,
        false,
        true,
        false,
        false,
        true,
        false));

    EXPECT_FALSE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_MANUAL_TO_FW,
        false,
        true,
        true,
        false,
        false,
        false));

    EXPECT_FALSE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_MANUAL_TO_MC,
        true,
        false,
        true,
        false,
        true,
        false));

    EXPECT_TRUE(mixerTransitionIsRequestAllowed(
        MIXERAT_REQUEST_MANUAL_TO_MC,
        true,
        false,
        true,
        false,
        false,
        true));
}

TEST(MixerTransitionPolicyTest, OnlyNavigationOwnedRequestsMayContinueDuringFailsafe)
{
    EXPECT_TRUE(mixerTransitionRequestAllowedDuringFailsafe(MIXERAT_REQUEST_RTH));
    EXPECT_TRUE(mixerTransitionRequestAllowedDuringFailsafe(MIXERAT_REQUEST_LAND));

    EXPECT_FALSE(mixerTransitionRequestAllowedDuringFailsafe(MIXERAT_REQUEST_NONE));
    EXPECT_FALSE(mixerTransitionRequestAllowedDuringFailsafe(MIXERAT_REQUEST_MISSION_TO_FW));
    EXPECT_FALSE(mixerTransitionRequestAllowedDuringFailsafe(MIXERAT_REQUEST_MISSION_TO_MC));
    EXPECT_FALSE(mixerTransitionRequestAllowedDuringFailsafe(MIXERAT_REQUEST_MANUAL_TO_FW));
    EXPECT_FALSE(mixerTransitionRequestAllowedDuringFailsafe(MIXERAT_REQUEST_MANUAL_TO_MC));
    EXPECT_TRUE(mixerTransitionRequestAllowedDuringFailsafe(MIXERAT_REQUEST_FW_TO_MC_PROTECTION));
    EXPECT_FALSE(mixerTransitionRequestAllowedDuringFailsafe(MIXERAT_REQUEST_ABORT));
}

TEST(MixerTransitionPolicyTest, FailsafeKeepsPostSwitchOutputRampAfterRequestIsCleared)
{
    EXPECT_FALSE(mixerTransitionShouldAbortForFailsafe(MIXERAT_REQUEST_RTH, false, false));
    EXPECT_FALSE(mixerTransitionShouldAbortForFailsafe(MIXERAT_REQUEST_LAND, false, false));
    EXPECT_FALSE(mixerTransitionShouldAbortForFailsafe(MIXERAT_REQUEST_FW_TO_MC_PROTECTION, false, false));

    EXPECT_TRUE(mixerTransitionShouldAbortForFailsafe(MIXERAT_REQUEST_NONE, false, false));
    EXPECT_TRUE(mixerTransitionShouldAbortForFailsafe(MIXERAT_REQUEST_NONE, true, false));
    EXPECT_TRUE(mixerTransitionShouldAbortForFailsafe(MIXERAT_REQUEST_NONE, false, true));

    EXPECT_FALSE(mixerTransitionShouldAbortForFailsafe(MIXERAT_REQUEST_NONE, true, true));
}

TEST(MixerTransitionPolicyTest, FwToMcProtectionAirspeedTriggerNeedsTrustedLowAirspeed)
{
    EXPECT_FALSE(mixerTransitionFwToMcProtectionTriggered(false, true, 700, true, 650.0f));
    EXPECT_FALSE(mixerTransitionFwToMcProtectionTriggered(true, false, 700, true, 650.0f));
    EXPECT_FALSE(mixerTransitionFwToMcProtectionTriggered(true, true, 0, true, 650.0f));
    EXPECT_FALSE(mixerTransitionFwToMcProtectionTriggered(true, true, 700, false, 650.0f));
    EXPECT_FALSE(mixerTransitionFwToMcProtectionTriggered(true, true, 700, true, 701.0f));

    EXPECT_TRUE(mixerTransitionFwToMcProtectionTriggered(true, true, 700, true, 700.0f));
    EXPECT_TRUE(mixerTransitionFwToMcProtectionTriggered(true, true, 700, true, 650.0f));
}

TEST(MixerTransitionPolicyTest, NavigationOwnsProfileSwitchOnlyForArmedVtolAutoStates)
{
    EXPECT_FALSE(mixerTransitionNavigationOwnsProfileSwitch(
        false,
        true,
        true,
        false,
        false,
        false));

    EXPECT_FALSE(mixerTransitionNavigationOwnsProfileSwitch(
        true,
        false,
        true,
        false,
        false,
        false));

    EXPECT_FALSE(mixerTransitionNavigationOwnsProfileSwitch(
        true,
        true,
        false,
        false,
        false,
        false));

    EXPECT_TRUE(mixerTransitionNavigationOwnsProfileSwitch(
        true,
        true,
        true,
        false,
        false,
        false));

    EXPECT_TRUE(mixerTransitionNavigationOwnsProfileSwitch(
        true,
        true,
        false,
        true,
        false,
        false));

    EXPECT_TRUE(mixerTransitionNavigationOwnsProfileSwitch(
        true,
        true,
        false,
        false,
        true,
        false));

    EXPECT_TRUE(mixerTransitionNavigationOwnsProfileSwitch(
        true,
        true,
        false,
        false,
        false,
        true));
}

TEST(MixerTransitionPolicyTest, DynamicScalingDisabledKeepsAllScalesAtFullValues)
{
    const mixerTransitionScaleState_t scales = mixerTransitionComputeScales(
        false,
        MIXERAT_DIRECTION_TO_FW,
        0.20f,
        0.30f,
        0.40f,
        0.50f,
        0.25f);

    EXPECT_FLOAT_EQ(1.0f, scales.pusherScale);
    EXPECT_FLOAT_EQ(1.0f, scales.liftScale);
    EXPECT_FLOAT_EQ(1.0f, scales.mcAuthorityScale);
    EXPECT_FLOAT_EQ(1.0f, scales.fwAuthorityScale);
    EXPECT_FLOAT_EQ(1.0f, scales.blendToFw);
}

TEST(MixerTransitionPolicyTest, McToFwDynamicScalingUsesHandoffAndRampProgress)
{
    const mixerTransitionScaleState_t scales = mixerTransitionComputeScales(
        true,
        MIXERAT_DIRECTION_TO_FW,
        0.20f,
        0.30f,
        0.40f,
        0.50f,
        0.25f);

    EXPECT_FLOAT_EQ(0.25f, scales.pusherScale);
    EXPECT_FLOAT_EQ(0.60f, scales.liftScale);
    EXPECT_FLOAT_EQ(0.65f, scales.mcAuthorityScale);
    EXPECT_FLOAT_EQ(0.70f, scales.fwAuthorityScale);
    EXPECT_FLOAT_EQ(0.70f, scales.blendToFw);
}

TEST(MixerTransitionPolicyTest, FwToMcDynamicScalingUsesTimerRampForLiftAndPusher)
{
    const mixerTransitionScaleState_t scales = mixerTransitionComputeScales(
        true,
        MIXERAT_DIRECTION_TO_MC,
        0.20f,
        0.30f,
        0.40f,
        0.50f,
        0.25f);

    EXPECT_FLOAT_EQ(0.75f, scales.pusherScale);
    EXPECT_FLOAT_EQ(0.40f, scales.liftScale);
    EXPECT_FLOAT_EQ(0.475f, scales.mcAuthorityScale);
    EXPECT_FLOAT_EQ(0.70f, scales.fwAuthorityScale);
    EXPECT_FLOAT_EQ(0.70f, scales.blendToFw);
}

TEST(MixerTransitionPolicyTest, AirspeedProgressRejectsAnIsolatedHighSpike)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = 0.50f;

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.50f, 0, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.95f, 100, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.50f, 200, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.55f, 300, 300, 100, &filter);

    EXPECT_FLOAT_EQ(0.50f, accepted);

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.55f, 400, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.55f, 500, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.55f, 600, 300, 100, &filter);

    EXPECT_FLOAT_EQ(0.55f, accepted);
}

TEST(MixerTransitionPolicyTest, AirspeedProgressRejectsAnIsolatedLowSpike)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = 0.80f;

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.80f, 0, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.25f, 100, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.80f, 200, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.75f, 300, 300, 100, &filter);

    EXPECT_FLOAT_EQ(0.80f, accepted);

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.75f, 400, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.75f, 500, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.75f, 600, 300, 100, &filter);

    EXPECT_FLOAT_EQ(0.75f, accepted);
}

TEST(MixerTransitionPolicyTest, AirspeedProgressUsesTheConservativeEdgeOfASustainedRise)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = 8.0f / 13.0f;

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 11.0f / 13.0f, 0, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 12.0f / 13.0f, 100, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 1.0f, 200, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 1.0f, 300, 300, 100, &filter);

    EXPECT_FLOAT_EQ(11.0f / 13.0f, accepted);
}

TEST(MixerTransitionPolicyTest, AirspeedProgressUsesTheConservativeEdgeOfASustainedFall)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = 0.80f;

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.75f, 0, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.60f, 100, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.50f, 200, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.45f, 300, 300, 100, &filter);

    EXPECT_FLOAT_EQ(0.75f, accepted);
}

TEST(MixerTransitionPolicyTest, ConfirmedMcToFwAirspeedLossRestoresScalesTogether)
{
    const mixerTransitionScaleState_t beforeLoss = mixerTransitionComputeScales(
        true,
        MIXERAT_DIRECTION_TO_FW,
        0.20f,
        0.30f,
        0.40f,
        0.80f,
        0.65f);
    const mixerTransitionScaleState_t afterLoss = mixerTransitionComputeScales(
        true,
        MIXERAT_DIRECTION_TO_FW,
        0.20f,
        0.30f,
        0.40f,
        0.60f,
        0.65f);

    EXPECT_FLOAT_EQ(beforeLoss.pusherScale, afterLoss.pusherScale);
    EXPECT_GT(afterLoss.liftScale, beforeLoss.liftScale);
    EXPECT_GT(afterLoss.mcAuthorityScale, beforeLoss.mcAuthorityScale);
    EXPECT_LT(afterLoss.fwAuthorityScale, beforeLoss.fwAuthorityScale);
}

TEST(MixerTransitionPolicyTest, TimerFallbackIsMonotonicAndResetsAirspeedHistory)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = 0.40f;

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.90f, 0, 300, 100, &filter);
    EXPECT_TRUE(filter.active);

    accepted = mixerTransitionResolveHandoffProgress(true, false, accepted, 0.55f, 100, 300, 100, &filter);
    EXPECT_FLOAT_EQ(0.55f, accepted);
    EXPECT_FALSE(filter.active);

    accepted = mixerTransitionResolveHandoffProgress(true, false, accepted, 0.45f, 200, 300, 100, &filter);
    EXPECT_FLOAT_EQ(0.55f, accepted);
}

TEST(MixerTransitionPolicyTest, ReturningAirspeedNeedsANewConfirmationWindow)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = mixerTransitionResolveHandoffProgress(true, false, 0.60f, 0.60f, 0, 300, 100, &filter);

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.30f, 100, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.30f, 300, 300, 100, &filter);
    EXPECT_FLOAT_EQ(0.60f, accepted);

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.30f, 400, 300, 100, &filter);
    EXPECT_FLOAT_EQ(0.30f, accepted);
}

TEST(MixerTransitionPolicyTest, InterruptedAirspeedSamplesRestartConfirmation)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = 0.40f;

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.80f, 0, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.80f, 100, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.80f, 500, 300, 100, &filter);

    EXPECT_FLOAT_EQ(0.40f, accepted);

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.80f, 700, 300, 100, &filter);
    EXPECT_FLOAT_EQ(0.40f, accepted);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.80f, 800, 300, 100, &filter);
    EXPECT_FLOAT_EQ(0.80f, accepted);
}

TEST(MixerTransitionPolicyTest, DynamicScalingOffAndTimerOnlyKeepExistingBehavior)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};

    EXPECT_FLOAT_EQ(1.0f, mixerTransitionResolveHandoffProgress(
        false, true, 0.25f, 0.10f, 0, 300, 100, &filter));
    EXPECT_TRUE(filter.active);
    EXPECT_FALSE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));

    EXPECT_FLOAT_EQ(0.65f, mixerTransitionResolveHandoffProgress(
        true, false, 0.50f, 0.65f, 100, 300, 100, &filter));
    EXPECT_FALSE(filter.active);
}

TEST(MixerTransitionPolicyTest, DynamicScalingOffStillRejectsAnEndpointSpike)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = 0.0f;

    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 0.80f, 0, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.00f, 100, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 0.80f, 200, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.00f, 300, 300, 100, &filter);

    EXPECT_FLOAT_EQ(1.0f, accepted);
    EXPECT_FALSE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));
}

TEST(MixerTransitionPolicyTest, AirspeedEndpointConfirmationRestartsAfterTimerFallback)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = 0.0f;

    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 0, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 100, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(false, false, accepted, 0.2f, 200, 300, 100, &filter);
    EXPECT_FALSE(filter.active);

    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 300, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 400, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 500, 300, 100, &filter);
    EXPECT_FALSE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));

    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 600, 300, 100, &filter);
    EXPECT_TRUE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));
}

TEST(MixerTransitionPolicyTest, AirspeedConfirmationHandlesMillisWraparound)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = 0.0f;
    const timeMs_t startTime = UINT32_MAX - 150U;

    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, startTime, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, startTime + 100U, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, startTime + 200U, 300, 100, &filter);
    EXPECT_FALSE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));

    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, startTime + 300U, 300, 100, &filter);
    EXPECT_TRUE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));
}

TEST(MixerTransitionPolicyTest, RecentAirspeedSampleSuppressesOnlyBriefSourceDropouts)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = mixerTransitionResolveHandoffProgress(
        false, true, 0.0f, 0.5f, 100, 300, 100, &filter);

    EXPECT_FLOAT_EQ(1.0f, accepted);
    EXPECT_TRUE(mixerTransitionAirspeedProgressHasRecentSample(&filter, 200, 300));
    EXPECT_TRUE(mixerTransitionAirspeedProgressHasRecentSample(&filter, 399, 300));
    EXPECT_FALSE(mixerTransitionAirspeedProgressHasRecentSample(&filter, 400, 300));

    mixerTransitionResetAirspeedProgressFilter(&filter);
    EXPECT_FALSE(mixerTransitionAirspeedProgressHasRecentSample(&filter, 200, 300));
}

TEST(MixerTransitionPolicyTest, FwToMcAirspeedProgressUsesTheSameSpikeFilter)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = 0.0f;
    bool startCaptured = false;
    float startAirspeedCmS = 0.0f;
    const float samples[] = { 1400.0f, 1200.0f, 1000.0f, 800.0f };

    for (uint32_t i = 0; i < 4; i++) {
        const mixerTransitionHotSwitchProgress_t raw = mixerTransitionEvaluateHotSwitch(
            MIXERAT_DIRECTION_TO_MC,
            800,
            true,
            samples[i],
            startCaptured,
            startAirspeedCmS,
            i * 100,
            5000);
        startCaptured = raw.transitionStartAirspeedCaptured;
        startAirspeedCmS = raw.transitionStartAirspeedCmS;
        accepted = mixerTransitionResolveHandoffProgress(
            true, raw.usedAirspeed, accepted, raw.progress, i * 100, 300, 100, &filter);
    }

    EXPECT_FLOAT_EQ(0.0f, accepted);

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 1.0f, 400, 300, 100, &filter);
    EXPECT_NEAR(1.0f / 3.0f, accepted, 0.0001f);
}

TEST(MixerTransitionPolicyTest, MotorRampProgressFallsBackToFullWhenDisabledOrZeroRamp)
{
    EXPECT_FLOAT_EQ(1.0f, mixerTransitionComputeMotorRampProgress(false, 500, 100));
    EXPECT_FLOAT_EQ(1.0f, mixerTransitionComputeMotorRampProgress(true, 0, 100));
    EXPECT_FLOAT_EQ(0.40f, mixerTransitionComputeMotorRampProgress(true, 500, 200));
}

TEST(MixerTransitionPolicyTest, PostSwitchMotorOutputBlendIsBounded)
{
    EXPECT_EQ(1800, mixerTransitionBlendCapturedMotorOutput(1800, 1000, 0.0f));
    EXPECT_EQ(1400, mixerTransitionBlendCapturedMotorOutput(1800, 1000, 0.5f));
    EXPECT_EQ(1000, mixerTransitionBlendCapturedMotorOutput(1800, 1000, 1.0f));

    EXPECT_EQ(1000, mixerTransitionBlendCapturedMotorOutput(1000, 1800, 0.0f));
    EXPECT_EQ(1400, mixerTransitionBlendCapturedMotorOutput(1000, 1800, 0.5f));
    EXPECT_EQ(1800, mixerTransitionBlendCapturedMotorOutput(1000, 1800, 1.0f));

    EXPECT_EQ(1800, mixerTransitionBlendCapturedMotorOutput(1800, 1000, -0.25f));
    EXPECT_EQ(1000, mixerTransitionBlendCapturedMotorOutput(1800, 1000, 1.25f));
}

TEST(MixerTransitionPolicyTest, DirectionNoneKeepsScalesAndHotSwitchIdle)
{
    const mixerTransitionScaleState_t scales = mixerTransitionComputeScales(
        true,
        MIXERAT_DIRECTION_NONE,
        0.20f,
        0.30f,
        0.40f,
        0.50f,
        0.25f);

    EXPECT_FLOAT_EQ(1.0f, scales.pusherScale);
    EXPECT_FLOAT_EQ(1.0f, scales.liftScale);
    EXPECT_FLOAT_EQ(1.0f, scales.mcAuthorityScale);
    EXPECT_FLOAT_EQ(1.0f, scales.fwAuthorityScale);
    EXPECT_FLOAT_EQ(1.0f, scales.blendToFw);

    const mixerTransitionHotSwitchProgress_t progress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_NONE,
        1500,
        true,
        1800.0f,
        false,
        0.0f,
        300,
        1000);

    EXPECT_FALSE(progress.readyForHotSwitch);
    EXPECT_FALSE(progress.usedAirspeed);
    EXPECT_FALSE(progress.transitionStartAirspeedCaptured);
    EXPECT_FLOAT_EQ(0.0f, progress.progress);
}

TEST(MixerTransitionPolicyTest, HotSwitchFallsBackToTimerWhenAirspeedIsUnavailable)
{
    const mixerTransitionHotSwitchProgress_t progress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_FW,
        1500,
        false,
        0.0f,
        false,
        0.0f,
        300,
        1000);

    EXPECT_FALSE(progress.readyForHotSwitch);
    EXPECT_FALSE(progress.usedAirspeed);
    EXPECT_FALSE(progress.transitionStartAirspeedCaptured);
    EXPECT_FLOAT_EQ(0.30f, progress.progress);
}

TEST(MixerTransitionPolicyTest, HotSwitchUsesAirspeedWhenAvailableForMcToFw)
{
    const mixerTransitionHotSwitchProgress_t progress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_FW,
        1500,
        true,
        1800.0f,
        false,
        0.0f,
        0,
        1000);

    EXPECT_TRUE(progress.readyForHotSwitch);
    EXPECT_TRUE(progress.usedAirspeed);
    EXPECT_FLOAT_EQ(1.0f, progress.progress);
}

TEST(MixerTransitionPolicyTest, AirspeedEndpointUsesTheConfirmedProgressWindow)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = 0.0f;

    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 0, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 100, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 200, 300, 100, &filter);
    EXPECT_FALSE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));

    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 300, 300, 100, &filter);
    EXPECT_TRUE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));
    EXPECT_FLOAT_EQ(1.0f, accepted);
}

TEST(MixerTransitionPolicyTest, AirspeedEndpointRejectsABriefDropBelowThreshold)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    float accepted = 0.0f;

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 1.0f, 0, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 0.95f, 100, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 1.0f, 200, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 1.0f, 300, 300, 100, &filter);
    EXPECT_FALSE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));

    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 1.0f, 400, 300, 100, &filter);
    EXPECT_FALSE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));
    accepted = mixerTransitionResolveHandoffProgress(true, true, accepted, 1.0f, 500, 300, 100, &filter);
    EXPECT_TRUE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));
}

TEST(MixerTransitionPolicyTest, HotSwitchWithoutAirspeedCompletesImmediatelyWhenTimerIsZero)
{
    const mixerTransitionHotSwitchProgress_t progress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_FW,
        1500,
        false,
        0.0f,
        false,
        0.0f,
        0,
        0);

    EXPECT_TRUE(progress.readyForHotSwitch);
    EXPECT_FALSE(progress.usedAirspeed);
    EXPECT_FALSE(progress.transitionStartAirspeedCaptured);
    EXPECT_FLOAT_EQ(1.0f, progress.progress);
}

TEST(MixerTransitionPolicyTest, HotSwitchCapturesAndReusesStartAirspeedForFwToMc)
{
    mixerTransitionHotSwitchProgress_t progress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_MC,
        1000,
        true,
        1600.0f,
        false,
        0.0f,
        0,
        1000);

    EXPECT_FALSE(progress.readyForHotSwitch);
    EXPECT_TRUE(progress.usedAirspeed);
    EXPECT_TRUE(progress.transitionStartAirspeedCaptured);
    EXPECT_FLOAT_EQ(1600.0f, progress.transitionStartAirspeedCmS);
    EXPECT_FLOAT_EQ(0.0f, progress.progress);

    progress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_MC,
        1000,
        true,
        1300.0f,
        progress.transitionStartAirspeedCaptured,
        progress.transitionStartAirspeedCmS,
        0,
        1000);

    EXPECT_FALSE(progress.readyForHotSwitch);
    EXPECT_FLOAT_EQ(0.50f, progress.progress);

    progress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_MC,
        1000,
        true,
        900.0f,
        progress.transitionStartAirspeedCaptured,
        progress.transitionStartAirspeedCmS,
        0,
        1000);

    EXPECT_TRUE(progress.readyForHotSwitch);
    EXPECT_FLOAT_EQ(1.0f, progress.progress);
}

TEST(MixerTransitionPolicyTest, FwToMcProgressLeavesEndpointIfSpeedRisesAfterStartingBelowThreshold)
{
    mixerTransitionAirspeedProgressFilter_t filter = {};
    mixerTransitionHotSwitchProgress_t progress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_MC,
        850,
        true,
        800.0f,
        false,
        0.0f,
        0,
        5000);

    EXPECT_TRUE(progress.readyForHotSwitch);
    EXPECT_FLOAT_EQ(1.0f, progress.progress);

    float accepted = mixerTransitionResolveHandoffProgress(
        false, progress.usedAirspeed, 0.0f, progress.progress, 0, 300, 100, &filter);

    progress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_MC,
        850,
        true,
        1000.0f,
        progress.transitionStartAirspeedCaptured,
        progress.transitionStartAirspeedCmS,
        100,
        5000);

    EXPECT_FALSE(progress.readyForHotSwitch);
    EXPECT_LT(progress.progress, 1.0f);

    accepted = mixerTransitionResolveHandoffProgress(
        false, progress.usedAirspeed, accepted, progress.progress, 100, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 200, 300, 100, &filter);
    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 300, 300, 100, &filter);
    EXPECT_FALSE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));

    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 400, 300, 100, &filter);
    EXPECT_FALSE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));
    accepted = mixerTransitionResolveHandoffProgress(false, true, accepted, 1.0f, 500, 300, 100, &filter);
    EXPECT_TRUE(mixerTransitionAirspeedProgressEndpointConfirmed(&filter));
}

TEST(MixerTransitionPolicyTest, AirspeedEndpointProgressMatchesTheHotSwitchConditionInBothDirections)
{
    const float airspeedsCmS[] = { 0.0f, 800.0f, 850.0f, 851.0f, 1000.0f, 1600.0f };

    for (const float airspeedCmS : airspeedsCmS) {
        const mixerTransitionHotSwitchProgress_t toFw = mixerTransitionEvaluateHotSwitch(
            MIXERAT_DIRECTION_TO_FW,
            850,
            true,
            airspeedCmS,
            false,
            0.0f,
            0,
            5000);
        EXPECT_EQ(toFw.readyForHotSwitch, toFw.progress >= 1.0f);

        const mixerTransitionHotSwitchProgress_t toMcFromAbove = mixerTransitionEvaluateHotSwitch(
            MIXERAT_DIRECTION_TO_MC,
            850,
            true,
            airspeedCmS,
            true,
            1600.0f,
            0,
            5000);
        EXPECT_EQ(toMcFromAbove.readyForHotSwitch, toMcFromAbove.progress >= 1.0f);

        const mixerTransitionHotSwitchProgress_t toMcFromBelow = mixerTransitionEvaluateHotSwitch(
            MIXERAT_DIRECTION_TO_MC,
            850,
            true,
            airspeedCmS,
            true,
            800.0f,
            0,
            5000);
        EXPECT_EQ(toMcFromBelow.readyForHotSwitch, toMcFromBelow.progress >= 1.0f);
    }
}

TEST(MixerTransitionPolicyTest, PostSwitchFadeMaskCapturesOldLiftAndNewPusherButNotSharedTiltMotors)
{
    motorMixer_t currentMixer[MAX_SUPPORTED_MOTORS] = {};
    motorMixer_t targetMixer[MAX_SUPPORTED_MOTORS] = {};

    currentMixer[0].throttle = 1.0f; // shared tilt motor
    currentMixer[1].throttle = 1.0f; // old lift motor
    targetMixer[0].throttle = 1.0f;  // shared tilt motor
    targetMixer[2].throttle = 1.0f;  // FW pusher appears after switch

    const mixerTransitionPostSwitchFadeMask_t fadeMask = mixerTransitionComputePostSwitchFadeMask(
        true,
        500,
        MIXERAT_DIRECTION_TO_FW,
        true,
        3,
        currentMixer,
        targetMixer);

    EXPECT_EQ((1U << 1) | (1U << 2), fadeMask.motorMask);
    EXPECT_EQ((1U << 2), fadeMask.toCurrentMotorMask);
}

TEST(MixerTransitionPolicyTest, PostSwitchFadeMaskCapturesOnlyOldPusherWhenReturningToMultirotor)
{
    motorMixer_t currentMixer[MAX_SUPPORTED_MOTORS] = {};
    motorMixer_t targetMixer[MAX_SUPPORTED_MOTORS] = {};

    currentMixer[0].throttle = 1.0f; // shared tilt motor
    currentMixer[2].throttle = 1.0f; // FW pusher disappears after switch
    targetMixer[0].throttle = 1.0f;  // shared tilt motor
    targetMixer[1].throttle = 1.0f;  // MC lift motor

    const mixerTransitionPostSwitchFadeMask_t fadeMask = mixerTransitionComputePostSwitchFadeMask(
        true,
        500,
        MIXERAT_DIRECTION_TO_MC,
        false,
        3,
        currentMixer,
        targetMixer);

    EXPECT_EQ((1U << 2), fadeMask.motorMask);
    EXPECT_EQ(0U, fadeMask.toCurrentMotorMask);
}

TEST(MixerTransitionPolicyTest, PostSwitchFadeMaskStaysEmptyWhenDynamicScalingIsDisabled)
{
    motorMixer_t currentMixer[MAX_SUPPORTED_MOTORS] = {};
    motorMixer_t targetMixer[MAX_SUPPORTED_MOTORS] = {};

    currentMixer[0].throttle = 1.0f;
    targetMixer[1].throttle = 1.0f;

    const mixerTransitionPostSwitchFadeMask_t fadeMask = mixerTransitionComputePostSwitchFadeMask(
        false,
        500,
        MIXERAT_DIRECTION_TO_FW,
        true,
        2,
        currentMixer,
        targetMixer);

    EXPECT_EQ(0U, fadeMask.motorMask);
    EXPECT_EQ(0U, fadeMask.toCurrentMotorMask);
}

TEST(MixerTransitionPolicyTest, PostSwitchFadeMaskStaysEmptyForSharedTiltOnlyConfiguration)
{
    motorMixer_t currentMixer[MAX_SUPPORTED_MOTORS] = {};
    motorMixer_t targetMixer[MAX_SUPPORTED_MOTORS] = {};

    currentMixer[0].throttle = 1.0f;
    targetMixer[0].throttle = 1.0f;

    const mixerTransitionPostSwitchFadeMask_t fadeMask = mixerTransitionComputePostSwitchFadeMask(
        true,
        500,
        MIXERAT_DIRECTION_TO_FW,
        true,
        1,
        currentMixer,
        targetMixer);

    EXPECT_EQ(0U, fadeMask.motorMask);
    EXPECT_EQ(0U, fadeMask.toCurrentMotorMask);
}

TEST(MixerTransitionPolicyTest, AutoServoInputMayDecreaseDuringFwToMc)
{
    EXPECT_EQ(100, mixerTransitionUpdateServoInput(
        500,
        false,
        true,
        true,
        false,
        false,
        0.20f));
}
