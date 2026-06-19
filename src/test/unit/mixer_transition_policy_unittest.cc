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

TEST(MixerTransitionPolicyTest, HandoffProgressUsesAirspeedDirectlyWhenAvailable)
{
    EXPECT_FLOAT_EQ(0.35f, mixerTransitionResolveHandoffProgress(true, true, 0.80f, 0.35f));
}

TEST(MixerTransitionPolicyTest, HandoffProgressPreservesPreviousPeakWhenAirspeedDropsOut)
{
    EXPECT_FLOAT_EQ(0.62f, mixerTransitionResolveHandoffProgress(true, false, 0.62f, 0.41f));
    EXPECT_FLOAT_EQ(0.91f, mixerTransitionResolveHandoffProgress(true, false, 0.62f, 0.91f));
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
