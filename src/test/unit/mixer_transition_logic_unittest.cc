#include <gtest/gtest.h>

extern "C" {
#include "flight/mixer_transition_logic.h"
}

TEST(MixerTransitionLogicTest, LegacyManualSessionStaysLegacyAfterHotSwitch)
{
    mixerTransitionManualSessionMode_e sessionMode = MIXER_TRANSITION_MANUAL_SESSION_NONE;

    sessionMode = mixerTransitionUpdateManualSessionMode(
        sessionMode,
        true,
        false,
        false,
        false);

    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_LEGACY, sessionMode);
    EXPECT_FALSE(mixerTransitionManualControllerEnabled(true, sessionMode));

    sessionMode = mixerTransitionUpdateManualSessionMode(
        sessionMode,
        false,
        true,
        true,
        false);

    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_NONE, sessionMode);
}

TEST(MixerTransitionLogicTest, AutoManualSessionStaysAutoAcrossProfileChanges)
{
    mixerTransitionManualSessionMode_e sessionMode = MIXER_TRANSITION_MANUAL_SESSION_NONE;

    sessionMode = mixerTransitionUpdateManualSessionMode(
        sessionMode,
        true,
        false,
        true,
        false);

    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_AUTO, sessionMode);
    EXPECT_TRUE(mixerTransitionManualControllerEnabled(false, sessionMode));
}

TEST(MixerTransitionLogicTest, TailsitterPairRequiresTheOrientationOffsetOnlyInMcProfile)
{
    EXPECT_TRUE(mixerTransitionIsTailSitterProfilePair(
        false, true, true,
        true, false, false));

    EXPECT_TRUE(mixerTransitionIsTailSitterProfilePair(
        true, false, false,
        false, true, true));

    EXPECT_FALSE(mixerTransitionIsTailSitterProfilePair(
        false, true, false,
        true, false, false));

    EXPECT_FALSE(mixerTransitionIsTailSitterProfilePair(
        true, false, true,
        false, true, true));
}

TEST(MixerTransitionLogicTest, TailsitterFirstVersionAllowsOnlyManualNonDynamicRequests)
{
    EXPECT_TRUE(mixerTransitionTailSitterRequestIsSupported(
        MIXERAT_REQUEST_MANUAL_TO_FW, true, false, false));
    EXPECT_TRUE(mixerTransitionTailSitterRequestIsSupported(
        MIXERAT_REQUEST_MANUAL_TO_MC, true, false, false));

    EXPECT_FALSE(mixerTransitionTailSitterRequestIsSupported(
        MIXERAT_REQUEST_MISSION_TO_FW, true, false, false));
    EXPECT_FALSE(mixerTransitionTailSitterRequestIsSupported(
        MIXERAT_REQUEST_RTH, true, false, false));
    EXPECT_FALSE(mixerTransitionTailSitterRequestIsSupported(
        MIXERAT_REQUEST_MANUAL_TO_FW, true, true, false));
    EXPECT_FALSE(mixerTransitionTailSitterRequestIsSupported(
        MIXERAT_REQUEST_MANUAL_TO_MC, true, false, true));

    // Non-tailsitter profile pairs preserve their existing request policy.
    EXPECT_TRUE(mixerTransitionTailSitterRequestIsSupported(
        MIXERAT_REQUEST_MISSION_TO_FW, false, true, true));
}

TEST(MixerTransitionLogicTest, TailsitterMcCaptureUsesTheExistingFortyFiveDegreeTarget)
{
    EXPECT_FALSE(mixerTransitionTailSitterCapturePitchReached(349));
    EXPECT_TRUE(mixerTransitionTailSitterCapturePitchReached(350));
    EXPECT_TRUE(mixerTransitionTailSitterCapturePitchReached(450));
    EXPECT_TRUE(mixerTransitionTailSitterCapturePitchReached(550));
    EXPECT_FALSE(mixerTransitionTailSitterCapturePitchReached(551));

    EXPECT_TRUE(mixerTransitionTailSitterNeedsMcCapture(MIXERAT_REQUEST_MANUAL_TO_MC, true));
    EXPECT_FALSE(mixerTransitionTailSitterNeedsMcCapture(MIXERAT_REQUEST_MANUAL_TO_FW, true));
    EXPECT_FALSE(mixerTransitionTailSitterNeedsMcCapture(MIXERAT_REQUEST_MANUAL_TO_MC, false));
}

TEST(MixerTransitionLogicTest, TailsitterMcCaptureReversesThroughTheMcToFwPath)
{
    EXPECT_TRUE(mixerTransitionTailSitterCaptureShouldReverse(
        true, true, false, true, 1, 0));

    EXPECT_FALSE(mixerTransitionTailSitterCaptureShouldReverse(
        true, true, true, true, 1, 0));
    EXPECT_FALSE(mixerTransitionTailSitterCaptureShouldReverse(
        true, true, false, false, 1, 0));
    EXPECT_FALSE(mixerTransitionTailSitterCaptureShouldReverse(
        true, true, false, true, 1, 1));
    EXPECT_FALSE(mixerTransitionTailSitterCaptureShouldReverse(
        false, true, false, true, 1, 0));
}

TEST(MixerTransitionLogicTest, CompletedAutoSessionReleasesOnFallingEdgeToEndpoint)
{
    EXPECT_FALSE(mixerTransitionKeepCompletedAutoSession(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        true,
        true,
        0,
        1));

    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_NONE, mixerTransitionUpdateManualSessionMode(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        false,
        true,
        true,
        false));
}

TEST(MixerTransitionLogicTest, CompletedAutoSessionReleasesWhenSwitchMatchesActiveProfile)
{
    EXPECT_FALSE(mixerTransitionKeepCompletedAutoSession(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        true,
        true,
        0,
        0));

    EXPECT_FALSE(mixerTransitionCompletedAutoSessionOwnsProfileSwitch(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        true,
        0,
        0));

    EXPECT_FALSE(mixerTransitionKeepCompletedAutoSession(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        true,
        false,
        0,
        1));
}

TEST(MixerTransitionLogicTest, CompletedAutoSessionEndpointConfirmationIsDetected)
{
    EXPECT_TRUE(mixerTransitionCompletedAutoSessionEndpointConfirmed(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        true,
        false,
        0,
        0));

    EXPECT_FALSE(mixerTransitionCompletedAutoSessionEndpointConfirmed(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        true,
        true,
        0,
        0));

    EXPECT_FALSE(mixerTransitionCompletedAutoSessionEndpointConfirmed(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        true,
        false,
        0,
        1));

    EXPECT_FALSE(mixerTransitionCompletedAutoSessionEndpointConfirmed(
        MIXER_TRANSITION_MANUAL_SESSION_LEGACY,
        true,
        false,
        0,
        0));
}

TEST(MixerTransitionLogicTest, DirectSwitchEndpointReleasesAnyStaleServoHandoff)
{
    EXPECT_TRUE(mixerTransitionDirectSwitchEndpointOwnsServoOutput(
        MIXER_TRANSITION_MANUAL_SESSION_NONE,
        false,
        false,
        false,
        false,
        false,
        1,
        1));

    EXPECT_FALSE(mixerTransitionDirectSwitchEndpointOwnsServoOutput(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        false,
        false,
        false,
        false,
        false,
        1,
        1));
    EXPECT_FALSE(mixerTransitionDirectSwitchEndpointOwnsServoOutput(
        MIXER_TRANSITION_MANUAL_SESSION_NONE,
        true,
        false,
        false,
        false,
        false,
        1,
        1));
    EXPECT_FALSE(mixerTransitionDirectSwitchEndpointOwnsServoOutput(
        MIXER_TRANSITION_MANUAL_SESSION_NONE,
        false,
        true,
        false,
        false,
        false,
        1,
        1));
    EXPECT_FALSE(mixerTransitionDirectSwitchEndpointOwnsServoOutput(
        MIXER_TRANSITION_MANUAL_SESSION_NONE,
        false,
        false,
        true,
        false,
        false,
        1,
        1));
    EXPECT_FALSE(mixerTransitionDirectSwitchEndpointOwnsServoOutput(
        MIXER_TRANSITION_MANUAL_SESSION_NONE,
        false,
        false,
        false,
        true,
        false,
        1,
        1));
    EXPECT_TRUE(mixerTransitionDirectSwitchEndpointOwnsServoOutput(
        MIXER_TRANSITION_MANUAL_SESSION_NONE,
        false,
        false,
        false,
        true,
        true,
        1,
        1));
    EXPECT_FALSE(mixerTransitionDirectSwitchEndpointOwnsServoOutput(
        MIXER_TRANSITION_MANUAL_SESSION_NONE,
        false,
        false,
        false,
        false,
        false,
        0,
        1));
}

TEST(MixerTransitionLogicTest, EndpointConfirmationMustBeCheckedBeforeFallingEdgeClearsSession)
{
    mixerTransitionManualSessionMode_e sessionMode = MIXER_TRANSITION_MANUAL_SESSION_AUTO;
    const bool shouldClearHandoff = mixerTransitionCompletedAutoSessionEndpointConfirmed(
        sessionMode,
        true,
        false,
        0,
        0);

    sessionMode = mixerTransitionUpdateManualSessionMode(
        sessionMode,
        false,
        true,
        true,
        false);

    EXPECT_TRUE(shouldClearHandoff);
    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_NONE, sessionMode);
}

TEST(MixerTransitionLogicTest, CompletedAutoSessionClearsStaleMixingWhileSwitchRemainsInTransition)
{
    EXPECT_TRUE(mixerTransitionShouldClearCompletedAutoMixingRequest(
        true,
        false,
        false,
        true));

    EXPECT_FALSE(mixerTransitionShouldClearCompletedAutoMixingRequest(
        true,
        false,
        true,
        true));

    EXPECT_FALSE(mixerTransitionShouldClearCompletedAutoMixingRequest(
        false,
        false,
        false,
        true));

    EXPECT_FALSE(mixerTransitionShouldClearCompletedAutoMixingRequest(
        true,
        true,
        false,
        true));
}

TEST(MixerTransitionLogicTest, ManualSwitchReminderShowsAfterCompletedAutoSwitchUntilEndpointConfirmed)
{
    EXPECT_EQ(MIXERAT_DIRECTION_TO_MC, mixerTransitionManualSwitchReminderDirection(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        false,
        true,
        true,
        1,
        1,
        true));

    EXPECT_EQ(MIXERAT_DIRECTION_TO_FW, mixerTransitionManualSwitchReminderDirection(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        false,
        true,
        false,
        0,
        1,
        false));
}

TEST(MixerTransitionLogicTest, ManualSwitchReminderClearsWhenEndpointMatches)
{
    EXPECT_EQ(MIXERAT_DIRECTION_NONE, mixerTransitionManualSwitchReminderDirection(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        false,
        true,
        false,
        1,
        1,
        true));

    EXPECT_EQ(MIXERAT_DIRECTION_NONE, mixerTransitionManualSwitchReminderDirection(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        false,
        true,
        false,
        0,
        0,
        false));
}

TEST(MixerTransitionLogicTest, ManualSwitchReminderDoesNotShowForLegacyOrActiveTransition)
{
    EXPECT_EQ(MIXERAT_DIRECTION_NONE, mixerTransitionManualSwitchReminderDirection(
        MIXER_TRANSITION_MANUAL_SESSION_LEGACY,
        false,
        true,
        true,
        1,
        1,
        true));

    EXPECT_EQ(MIXERAT_DIRECTION_NONE, mixerTransitionManualSwitchReminderDirection(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        true,
        true,
        true,
        1,
        1,
        true));

    EXPECT_EQ(MIXERAT_DIRECTION_NONE, mixerTransitionManualSwitchReminderDirection(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        false,
        false,
        true,
        1,
        1,
        true));
}

TEST(MixerTransitionLogicTest, NavigationHandbackHoldsProfileWhenManualSwitchDiffers)
{
    EXPECT_TRUE(mixerTransitionNavigationHandbackShouldHoldProfile(
        true,
        false,
        true,
        false,
        false,
        0,
        1));

    EXPECT_FALSE(mixerTransitionNavigationHandbackShouldHoldProfile(
        false,
        false,
        true,
        false,
        false,
        0,
        1));

    EXPECT_FALSE(mixerTransitionNavigationHandbackShouldHoldProfile(
        true,
        true,
        true,
        false,
        false,
        0,
        1));
}

TEST(MixerTransitionLogicTest, NavigationHandbackHoldsProfileWhenSwitchStillInTransitionPosition)
{
    EXPECT_TRUE(mixerTransitionNavigationHandbackShouldHoldProfile(
        true,
        false,
        true,
        false,
        true,
        1,
        1));

    EXPECT_FALSE(mixerTransitionNavigationHandbackShouldClear(
        false,
        false,
        true,
        1,
        1));

    EXPECT_TRUE(mixerTransitionNavigationHandbackShouldClear(
        false,
        false,
        false,
        1,
        1));
}

TEST(MixerTransitionLogicTest, NavigationHandbackClearsForNewNavigationOrExplicitTransition)
{
    EXPECT_TRUE(mixerTransitionNavigationHandbackShouldClear(
        true,
        false,
        false,
        0,
        1));

    EXPECT_TRUE(mixerTransitionNavigationHandbackShouldClear(
        false,
        true,
        true,
        0,
        1));

    EXPECT_FALSE(mixerTransitionNavigationHandbackShouldHoldProfile(
        true,
        false,
        true,
        true,
        false,
        0,
        1));
}

TEST(MixerTransitionLogicTest, NavigationOwnershipIgnoresManualTransitionInput)
{
    EXPECT_FALSE(mixerTransitionManualInputAllowed(true));
    EXPECT_FALSE(mixerTransitionManualMixingRequestMayUpdate(false, false, true));
    EXPECT_TRUE(mixerTransitionManualMixingRequestMayUpdate(false, false, false));

    // Entering navigation clears a previous manual session. A physical switch
    // movement during the mission cannot create another manual session.
    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_NONE, mixerTransitionUpdateManualSessionMode(
        MIXER_TRANSITION_MANUAL_SESSION_AUTO,
        false,
        false,
        true,
        true));
    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_NONE, mixerTransitionUpdateManualSessionMode(
        MIXER_TRANSITION_MANUAL_SESSION_NONE,
        true,
        false,
        true,
        true));
}

TEST(MixerTransitionLogicTest, ActiveModesReportEffectiveVtolStateInsteadOfRawSwitchState)
{
    EXPECT_FALSE(mixerTransitionProfile2ShouldReportActive(0));
    EXPECT_TRUE(mixerTransitionProfile2ShouldReportActive(1));

    // A transition switch in the middle position is not an active transition
    // when navigation ignores it. Both real transition phases remain visible.
    EXPECT_FALSE(mixerTransitionModeShouldReportActive(false, false));
    EXPECT_TRUE(mixerTransitionModeShouldReportActive(false, true));
    EXPECT_TRUE(mixerTransitionModeShouldReportActive(true, false));
}

TEST(MixerTransitionLogicTest, ProfileSwitchAutotransitionStartsOnlyWhenArmedAndClear)
{
    EXPECT_TRUE(mixerTransitionProfileSwitchShouldStartAutoTransition(
        true,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        false,
        false,
        1,
        0));

    EXPECT_FALSE(mixerTransitionProfileSwitchShouldStartAutoTransition(
        true,
        false,
        true,
        true,
        true,
        false,
        false,
        false,
        false,
        false,
        1,
        0));

    EXPECT_FALSE(mixerTransitionProfileSwitchShouldStartAutoTransition(
        false,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        false,
        false,
        1,
        0));
}

TEST(MixerTransitionLogicTest, ProfileSwitchAutotransitionKeepsDisarmedDirectSwitchForPreflight)
{
    EXPECT_FALSE(mixerTransitionProfileSwitchDirectSwitchAllowed(
        true,
        true,
        true));

    EXPECT_TRUE(mixerTransitionProfileSwitchDirectSwitchAllowed(
        true,
        false,
        true));

    EXPECT_TRUE(mixerTransitionProfileSwitchDirectSwitchAllowed(
        false,
        true,
        true));

    EXPECT_TRUE(mixerTransitionProfileSwitchDirectSwitchAllowed(
        true,
        true,
        false));
}

TEST(MixerTransitionLogicTest, ProfileSwitchAutotransitionDoesNotFightOwnedStates)
{
    EXPECT_FALSE(mixerTransitionProfileSwitchShouldStartAutoTransition(
        true,
        true,
        true,
        true,
        true,
        false,
        false,
        false,
        true,
        false,
        0,
        1));

    EXPECT_FALSE(mixerTransitionProfileSwitchShouldStartAutoTransition(
        true,
        true,
        true,
        true,
        true,
        false,
        true,
        false,
        false,
        false,
        0,
        1));

    EXPECT_FALSE(mixerTransitionProfileSwitchShouldStartAutoTransition(
        true,
        true,
        true,
        true,
        true,
        false,
        false,
        true,
        false,
        false,
        0,
        1));
}

TEST(MixerTransitionLogicTest, ProfileSwitchAutotransitionAbortsWhenSwitchReturnsToCurrentProfile)
{
    EXPECT_TRUE(mixerTransitionProfileSwitchShouldAbortToCurrentProfile(
        true,
        true,
        false,
        1,
        1));

    EXPECT_FALSE(mixerTransitionProfileSwitchShouldAbortToCurrentProfile(
        true,
        true,
        true,
        1,
        1));

    EXPECT_FALSE(mixerTransitionProfileSwitchShouldAbortToCurrentProfile(
        true,
        true,
        false,
        1,
        0));
}

TEST(MixerTransitionLogicTest, LegacySessionIgnoresAutoControllerAfterProfileHotSwitch)
{
    mixerTransitionManualSessionMode_e sessionMode = MIXER_TRANSITION_MANUAL_SESSION_NONE;

    sessionMode = mixerTransitionUpdateManualSessionMode(
        sessionMode,
        true,
        false,
        false,
        false);

    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_LEGACY, sessionMode);
    EXPECT_FALSE(mixerTransitionManualControllerEnabled(true, sessionMode));
    EXPECT_EQ(500, mixerTransitionUpdateServoInput(
        0,
        sessionMode == MIXER_TRANSITION_MANUAL_SESSION_LEGACY,
        true,
        false,
        false,
        true,
        0.18f));
}

TEST(MixerTransitionLogicTest, LegacyServoInputMatchesPrePrFixedEndpoint)
{
    EXPECT_EQ(500, mixerTransitionUpdateServoInput(
        0,
        true,
        true,
        false,
        false,
        true,
        0.12f));

    EXPECT_EQ(0, mixerTransitionUpdateServoInput(
        500,
        true,
        false,
        false,
        false,
        true,
        0.12f));
}

TEST(MixerTransitionLogicTest, AutoServoInputDoesNotMoveBackwardsDuringMcToFw)
{
    int16_t servoInput = 0;

    servoInput = mixerTransitionUpdateServoInput(
        servoInput,
        false,
        true,
        true,
        false,
        true,
        0.30f);
    EXPECT_EQ(150, servoInput);

    servoInput = mixerTransitionUpdateServoInput(
        servoInput,
        false,
        true,
        true,
        false,
        true,
        0.62f);
    EXPECT_EQ(310, servoInput);

    servoInput = mixerTransitionUpdateServoInput(
        servoInput,
        false,
        true,
        true,
        false,
        true,
        0.58f);
    EXPECT_EQ(310, servoInput);

    servoInput = mixerTransitionUpdateServoInput(
        servoInput,
        false,
        false,
        true,
        false,
        true,
        0.20f);
    EXPECT_EQ(310, servoInput);

    servoInput = mixerTransitionUpdateServoInput(
        servoInput,
        false,
        false,
        true,
        true,
        true,
        0.20f);
    EXPECT_EQ(500, servoInput);

    servoInput = mixerTransitionUpdateServoInput(
        servoInput,
        false,
        false,
        false,
        false,
        false,
        0.0f);
    EXPECT_EQ(0, servoInput);
}

TEST(MixerTransitionLogicTest, AutoServoBlendUsesScaleRampTimerInsteadOfAirspeedProgress)
{
    const float blendAtTransitionStart = mixerTransitionComputeServoBlendToFw(
        false,
        true,
        true,
        false,
        true,
        MIXERAT_DIRECTION_TO_FW,
        16000,
        10);

    EXPECT_LT(blendAtTransitionStart, 0.01f);
    EXPECT_EQ(0, mixerTransitionUpdateServoInput(
        0,
        false,
        true,
        true,
        false,
        true,
        blendAtTransitionStart));

    EXPECT_FLOAT_EQ(0.65f, mixerTransitionComputeServoBlendToFw(
        false,
        true,
        true,
        false,
        true,
        MIXERAT_DIRECTION_TO_FW,
        1000,
        650));
}

TEST(MixerTransitionLogicTest, AutoServoBlendCountsBackDownDuringFwToMc)
{
    EXPECT_FLOAT_EQ(0.75f, mixerTransitionComputeServoBlendToFw(
        false,
        true,
        true,
        false,
        true,
        MIXERAT_DIRECTION_TO_MC,
        1000,
        250));

    EXPECT_FLOAT_EQ(1.0f, mixerTransitionComputeServoBlendToFw(
        false,
        false,
        true,
        true,
        true,
        MIXERAT_DIRECTION_TO_FW,
        0,
        0));
}

TEST(MixerTransitionLogicTest, AutoServoBlendStaysLegacyStaticWhenDynamicMixerIsDisabled)
{
    EXPECT_FLOAT_EQ(1.0f, mixerTransitionComputeServoBlendToFw(
        false,
        true,
        true,
        false,
        false,
        MIXERAT_DIRECTION_TO_FW,
        1000,
        0));

    EXPECT_FLOAT_EQ(1.0f, mixerTransitionComputeServoBlendToFw(
        false,
        true,
        true,
        false,
        false,
        MIXERAT_DIRECTION_TO_FW,
        1000,
        100));

    EXPECT_EQ(500, mixerTransitionUpdateServoInput(
        0,
        false,
        true,
        true,
        false,
        true,
        1.0f));
}

TEST(MixerTransitionLogicTest, ServoHandoffUsesFullScaleRampTimeAfterHotSwitchWhenDynamicMixerIsEnabled)
{
    EXPECT_EQ(1000, mixerTransitionComputeServoHandoffDurationMs(true, 1000, 0));
    EXPECT_EQ(1000, mixerTransitionComputeServoHandoffDurationMs(true, 1000, 250));
    EXPECT_EQ(1000, mixerTransitionComputeServoHandoffDurationMs(true, 1000, 1200));
}

TEST(MixerTransitionLogicTest, ServoHandoffUsesConfiguredScaleRampWhenDynamicMixerIsDisabled)
{
    EXPECT_EQ(1000, mixerTransitionComputeServoHandoffDurationMs(false, 1000, 0));
    EXPECT_EQ(1000, mixerTransitionComputeServoHandoffDurationMs(false, 1000, 750));
    EXPECT_EQ(0, mixerTransitionComputeServoHandoffDurationMs(false, 0, 750));
}

TEST(MixerTransitionLogicTest, ServoHandoffExpiresAtConfiguredDuration)
{
    EXPECT_FALSE(mixerTransitionServoHandoffExpired(0, 1000));
    EXPECT_FALSE(mixerTransitionServoHandoffExpired(999, 1000));
    EXPECT_TRUE(mixerTransitionServoHandoffExpired(1000, 1000));
    EXPECT_TRUE(mixerTransitionServoHandoffExpired(1500, 1000));
    EXPECT_TRUE(mixerTransitionServoHandoffExpired(0, 0));
}

TEST(MixerTransitionLogicTest, ServoHandoffProgressIsBounded)
{
    EXPECT_FLOAT_EQ(0.0f, mixerTransitionServoHandoffProgress(0, 1000));
    EXPECT_FLOAT_EQ(0.5f, mixerTransitionServoHandoffProgress(500, 1000));
    EXPECT_FLOAT_EQ(1.0f, mixerTransitionServoHandoffProgress(1000, 1000));
    EXPECT_FLOAT_EQ(1.0f, mixerTransitionServoHandoffProgress(1500, 1000));
    EXPECT_FLOAT_EQ(1.0f, mixerTransitionServoHandoffProgress(0, 0));
}

TEST(MixerTransitionLogicTest, ServoHandoffBlendStartsFromCapturedOutputAfterHotSwitch)
{
    EXPECT_EQ(1366, mixerTransitionBlendCapturedServoOutput(1366, 980, 0.0f));
    EXPECT_EQ(1270, mixerTransitionBlendCapturedServoOutput(1366, 980, 0.25f));
    EXPECT_EQ(1173, mixerTransitionBlendCapturedServoOutput(1366, 980, 0.50f));
    EXPECT_EQ(980, mixerTransitionBlendCapturedServoOutput(1366, 980, 1.0f));
}

TEST(MixerTransitionLogicTest, ServoHandoffBlendReturnsSmoothlyAfterAbort)
{
    EXPECT_EQ(1651, mixerTransitionBlendCapturedServoOutput(1651, 1224, 0.0f));
    EXPECT_EQ(1438, mixerTransitionBlendCapturedServoOutput(1651, 1224, 0.50f));
    EXPECT_EQ(1224, mixerTransitionBlendCapturedServoOutput(1651, 1224, 1.0f));
}
