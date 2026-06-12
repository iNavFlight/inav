#include <gtest/gtest.h>

extern "C" {
#include "flight/mixer_transition_logic.h"
}

namespace {

struct ManualTransitionScenario {
    explicit ManualTransitionScenario(const bool transitionToFixedWing)
        : transitionToFw(transitionToFixedWing)
    {
    }

    void setTransitionMode(const bool active, const bool manualControllerConfigured, const bool clearSession = false)
    {
        const bool risingEdge = active && !transitionModeActive;
        const bool fallingEdge = !active && transitionModeActive;

        sessionMode = mixerTransitionUpdateManualSessionMode(
            sessionMode,
            risingEdge,
            fallingEdge,
            manualControllerConfigured,
            clearSession);
        transitionModeActive = active;
    }

    bool autoControllerEnabled(const bool manualControllerConfigured) const
    {
        return mixerTransitionManualControllerEnabled(manualControllerConfigured, sessionMode);
    }

    int16_t updateServo(
        const bool transitionMixingActive,
        const bool autoTransitionActive,
        const bool postSwitchFadeToFwActive,
        const float blendToFw)
    {
        servoInput = mixerTransitionUpdateServoInput(
            servoInput,
            sessionMode == MIXER_TRANSITION_MANUAL_SESSION_LEGACY,
            transitionMixingActive,
            autoTransitionActive,
            postSwitchFadeToFwActive,
            transitionToFw,
            blendToFw);
        return servoInput;
    }

    mixerTransitionManualSessionMode_e sessionMode = MIXER_TRANSITION_MANUAL_SESSION_NONE;
    int16_t servoInput = 0;
    bool transitionModeActive = false;
    bool transitionToFw = true;
};

float computeBlendToFwForStep(
    const mixerProfileATDirection_e direction,
    float *handoffProgress,
    const bool usedAirspeed,
    const float rawProgress,
    const float motorRampProgress)
{
    *handoffProgress = mixerTransitionResolveHandoffProgress(
        true,
        usedAirspeed,
        *handoffProgress,
        rawProgress);

    const mixerTransitionScaleState_t scales = mixerTransitionComputeScales(
        true,
        direction,
        0.20f,
        0.30f,
        0.40f,
        *handoffProgress,
        motorRampProgress);

    return scales.blendToFw;
}

} // namespace

TEST(MixerTransitionScenarioTest, LegacyManualMcToFwSessionStaysLegacyAcrossProfileHotSwitch)
{
    ManualTransitionScenario scenario(true);

    scenario.setTransitionMode(true, false);
    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_LEGACY, scenario.sessionMode);
    EXPECT_FALSE(scenario.autoControllerEnabled(false));
    EXPECT_EQ(500, scenario.updateServo(true, false, false, 0.18f));

    // A hot-switch onto a profile that enables the new controller must not
    // retroactively convert the already-running legacy session.
    scenario.setTransitionMode(true, true);
    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_LEGACY, scenario.sessionMode);
    EXPECT_FALSE(scenario.autoControllerEnabled(true));
    EXPECT_EQ(500, scenario.updateServo(true, true, false, 0.02f));

    scenario.setTransitionMode(false, true);
    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_NONE, scenario.sessionMode);
    EXPECT_EQ(0, scenario.updateServo(false, false, false, 0.0f));
}

TEST(MixerTransitionScenarioTest, AutoMcToFwSessionStaysMonotonicAcrossAirspeedDropoutAndHotSwitch)
{
    ManualTransitionScenario scenario(true);
    float handoffProgress = 0.0f;

    scenario.setTransitionMode(true, true);
    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_AUTO, scenario.sessionMode);

    mixerTransitionHotSwitchProgress_t hotSwitchProgress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_FW,
        1000,
        true,
        320.0f,
        false,
        0.0f,
        0,
        1000);
    EXPECT_FALSE(hotSwitchProgress.readyForHotSwitch);
    EXPECT_EQ(296, scenario.updateServo(
        true,
        scenario.autoControllerEnabled(true),
        false,
        computeBlendToFwForStep(
            MIXERAT_DIRECTION_TO_FW,
            &handoffProgress,
            hotSwitchProgress.usedAirspeed,
            hotSwitchProgress.progress,
            0.25f)));

    hotSwitchProgress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_FW,
        1000,
        true,
        650.0f,
        false,
        0.0f,
        0,
        1000);
    EXPECT_FALSE(hotSwitchProgress.readyForHotSwitch);
    EXPECT_EQ(395, scenario.updateServo(
        true,
        scenario.autoControllerEnabled(true),
        false,
        computeBlendToFwForStep(
            MIXERAT_DIRECTION_TO_FW,
            &handoffProgress,
            hotSwitchProgress.usedAirspeed,
            hotSwitchProgress.progress,
            0.55f)));

    // After a profile hot-switch, the active auto session must survive even if
    // the target profile itself has the manual controller disabled.
    scenario.setTransitionMode(true, false);
    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_AUTO, scenario.sessionMode);
    EXPECT_TRUE(scenario.autoControllerEnabled(false));

    hotSwitchProgress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_FW,
        1000,
        false,
        0.0f,
        false,
        0.0f,
        500,
        1000);
    EXPECT_FALSE(hotSwitchProgress.readyForHotSwitch);
    EXPECT_EQ(395, scenario.updateServo(
        true,
        scenario.autoControllerEnabled(false),
        false,
        computeBlendToFwForStep(
            MIXERAT_DIRECTION_TO_FW,
            &handoffProgress,
            hotSwitchProgress.usedAirspeed,
            hotSwitchProgress.progress,
            0.75f)));

    // Even if the raw blend source drops during the same control cycle as the
    // hot-switch, the MC->FW servo source must not move backwards.
    EXPECT_EQ(395, scenario.updateServo(
        false,
        scenario.autoControllerEnabled(false),
        false,
        0.20f));
    EXPECT_EQ(500, scenario.updateServo(
        false,
        scenario.autoControllerEnabled(false),
        true,
        0.20f));

    scenario.setTransitionMode(false, false);
    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_NONE, scenario.sessionMode);
    EXPECT_EQ(0, scenario.updateServo(false, false, false, 0.0f));
}

TEST(MixerTransitionScenarioTest, AutoFwToMcSessionUsesCapturedAirspeedAndReturnsTowardMultirotor)
{
    ManualTransitionScenario scenario(false);
    float handoffProgress = 0.0f;

    scenario.setTransitionMode(true, true);
    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_AUTO, scenario.sessionMode);

    mixerTransitionHotSwitchProgress_t hotSwitchProgress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_MC,
        1000,
        true,
        1600.0f,
        false,
        0.0f,
        0,
        1000);
    EXPECT_TRUE(hotSwitchProgress.transitionStartAirspeedCaptured);
    EXPECT_EQ(500, scenario.updateServo(
        true,
        scenario.autoControllerEnabled(true),
        false,
        computeBlendToFwForStep(
            MIXERAT_DIRECTION_TO_MC,
            &handoffProgress,
            hotSwitchProgress.usedAirspeed,
            hotSwitchProgress.progress,
            0.0f)));

    hotSwitchProgress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_MC,
        1000,
        true,
        1300.0f,
        hotSwitchProgress.transitionStartAirspeedCaptured,
        hotSwitchProgress.transitionStartAirspeedCmS,
        0,
        1000);
    EXPECT_FALSE(hotSwitchProgress.readyForHotSwitch);
    EXPECT_EQ(350, scenario.updateServo(
        true,
        scenario.autoControllerEnabled(true),
        false,
        computeBlendToFwForStep(
            MIXERAT_DIRECTION_TO_MC,
            &handoffProgress,
            hotSwitchProgress.usedAirspeed,
            hotSwitchProgress.progress,
            0.50f)));

    hotSwitchProgress = mixerTransitionEvaluateHotSwitch(
        MIXERAT_DIRECTION_TO_MC,
        1000,
        true,
        900.0f,
        hotSwitchProgress.transitionStartAirspeedCaptured,
        hotSwitchProgress.transitionStartAirspeedCmS,
        0,
        1000);
    EXPECT_TRUE(hotSwitchProgress.readyForHotSwitch);
    EXPECT_EQ(200, scenario.updateServo(
        true,
        scenario.autoControllerEnabled(true),
        false,
        computeBlendToFwForStep(
            MIXERAT_DIRECTION_TO_MC,
            &handoffProgress,
            hotSwitchProgress.usedAirspeed,
            hotSwitchProgress.progress,
            1.0f)));

    scenario.setTransitionMode(false, true);
    EXPECT_EQ(MIXER_TRANSITION_MANUAL_SESSION_NONE, scenario.sessionMode);
    EXPECT_EQ(0, scenario.updateServo(false, false, false, 0.0f));
}

TEST(MixerTransitionScenarioTest, PusherHotSwitchFadeScenarioCapturesOnlyNonSharedMotors)
{
    motorMixer_t currentMixer[MAX_SUPPORTED_MOTORS] = {};
    motorMixer_t targetMixer[MAX_SUPPORTED_MOTORS] = {};
    ManualTransitionScenario scenario(true);
    float handoffProgress = 0.0f;

    currentMixer[0].throttle = 1.0f; // shared tilt motor
    currentMixer[1].throttle = 1.0f; // lift motor disappears after switch
    targetMixer[0].throttle = 1.0f;  // shared tilt motor
    targetMixer[2].throttle = 1.0f;  // FW pusher appears after switch

    scenario.setTransitionMode(true, true);
    EXPECT_EQ(500, scenario.updateServo(
        true,
        scenario.autoControllerEnabled(true),
        false,
        computeBlendToFwForStep(
            MIXERAT_DIRECTION_TO_FW,
            &handoffProgress,
            true,
            1.0f,
            1.0f)));

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
    EXPECT_EQ(500, scenario.updateServo(
        false,
        scenario.autoControllerEnabled(true),
        true,
        0.0f));
}

TEST(MixerTransitionScenarioTest, SharedTiltOnlyHotSwitchScenarioNeedsNoFadeAndStaysAtFwEndpoint)
{
    motorMixer_t currentMixer[MAX_SUPPORTED_MOTORS] = {};
    motorMixer_t targetMixer[MAX_SUPPORTED_MOTORS] = {};
    ManualTransitionScenario scenario(true);
    float handoffProgress = 0.0f;

    currentMixer[0].throttle = 1.0f;
    targetMixer[0].throttle = 1.0f;

    scenario.setTransitionMode(true, true);
    EXPECT_EQ(500, scenario.updateServo(
        true,
        scenario.autoControllerEnabled(true),
        false,
        computeBlendToFwForStep(
            MIXERAT_DIRECTION_TO_FW,
            &handoffProgress,
            true,
            1.0f,
            1.0f)));

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

    // With shared tilt motors there is no separate fade mask, but the FW
    // endpoint still must not fall back during the handoff cycle.
    EXPECT_EQ(500, scenario.updateServo(
        false,
        scenario.autoControllerEnabled(true),
        false,
        0.0f));
}
