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
