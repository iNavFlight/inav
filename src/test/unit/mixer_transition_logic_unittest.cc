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

TEST(MixerTransitionLogicTest, DirectSwitchServoHoldIsIndependentFromHandoffDuration)
{
    EXPECT_TRUE(mixerTransitionServoHandoffHoldActive(300, 0));
    EXPECT_TRUE(mixerTransitionServoHandoffHoldActive(300, 299));
    EXPECT_FALSE(mixerTransitionServoHandoffHoldActive(300, 300));
    EXPECT_FALSE(mixerTransitionServoHandoffHoldActive(0, 0));
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
