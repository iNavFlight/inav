#include <gtest/gtest.h>

extern "C" {
#include "target/common.h"
#undef MAX_MIXER_PROFILE_COUNT
#define MAX_MIXER_PROFILE_COUNT 2
#include "build/debug.h"
#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"
#include "fc/config.h"
#include "fc/runtime_config.h"
#include "flight/mixer_profile.h"
#include "flight/mixer.h"
#include "flight/servos.h"
#include "flight/imu.h"
#include "flight/pid.h"
#include "navigation/navigation.h"
#define _Static_assert static_assert
#include "navigation/navigation_private.h"
#undef _Static_assert
#include "sensors/sensors.h"
#include "fc/rc_modes.h"
#include "fc/cli.h"
#include "drivers/time.h"

PG_REGISTER(systemConfig_t, systemConfig, PG_SYSTEM_CONFIG, 0);
void testResetMixerTransitionRuntime(void);

uint32_t armingFlags;
uint32_t stateFlags;
uint32_t flightModeFlags;
bool cliMode;
int32_t debug[DEBUG32_VALUE_COUNT];
uint8_t debugMode;
int16_t motor[MAX_SUPPORTED_MOTORS];
int16_t servo[MAX_SUPPORTED_SERVOS];
attitudeEulerAngles_t attitude;
navigationPosControl_t posControl;

static timeMs_t now;
static bool validAirspeed;
static float airspeed;
static bool rcTransition;
static bool rcProfile2;
static navigationFSMStateFlags_t testNavFlags;
static bool servoCarryover = true;
static bool carryoverAtLoad;

timeMs_t millis(void) { return now; }
bool hasUsableTransitionAirspeed(float *speed) { *speed = airspeed; return validAirspeed; }
bool areSensorsCalibrating(void) { return false; }
uint8_t getConfigMixerProfile(void) { return systemConfig()->current_mixer_profile_index; }
bool setConfigMixerProfile(uint8_t index) { systemConfigMutable()->current_mixer_profile_index = index; return true; }
bool setConfigProfile(uint8_t) { return true; }
void servosInit(void) { carryoverAtLoad = servoCarryover; }
void mixerInit(void) {}
void mixerUpdateStateFlags(void) { stateFlags = currentMixerConfig.platformType == PLATFORM_AIRPLANE ? AIRPLANE : MULTIROTOR; }
void pidInit(void) {}
bool pidInitFilters(void) { return true; }
void pidResetErrorAccumulators(void) {}
void schedulePidGainsUpdate(void) {}
void navigationUsePIDs(void) {}
uint8_t getMotorCount(void) { return 5; }
void servoMixerSetCarryoverOnNextLoad(bool enabled) { servoCarryover = enabled; }
int32_t servoMixerGetVtolTransitionDebug(uint8_t) { return 0; }
pidType_e pidIndexGetType(pidIndex_e) { return PID_TYPE_AUTO; }
bool isModeActivationConditionPresent(boxId_e mode) { return mode == BOXMIXERPROFILE; }
bool IS_RC_MODE_ACTIVE(boxId_e mode) { return mode == BOXMIXERPROFILE ? rcProfile2 : (mode == BOXMIXERTRANSITION && rcTransition); }
navigationFSMStateFlags_t navGetCurrentStateFlags(void) { return testNavFlags; }
}

class MixerTransitionOutputsTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        testResetMixerTransitionRuntime();
        now = 1000;
        armingFlags = 0;
        flightModeFlags = 0;
        rcTransition = false;
        rcProfile2 = false;
        validAirspeed = false;
        testNavFlags = static_cast<navigationFSMStateFlags_t>(0);
        posControl = {};
        posControl.navState = NAV_STATE_IDLE;
        *systemConfigMutable() = {};
        mixerProfileAT = {};
        attitude = {};
        for (int i = 0; i < MAX_MIXER_PROFILE_COUNT; i++) {
            *mixerProfilesMutable(i) = {};
            auto *config = &mixerProfilesMutable(i)->mixer_config;
            config->platformType = i == 0 ? PLATFORM_AIRPLANE : PLATFORM_MULTIROTOR;
            config->automated_switch = true;
            config->manualVtolTransitionController = true;
            config->vtolTransitionDynamicMixer = true;
            config->vtolTransitionScaleRampTimeMs = 700;
            config->switchTransitionTimer = 7;
        }
        systemConfigMutable()->vtolTransitionMcAuthorityMinPercent = 40;
        systemConfigMutable()->vtolTransitionLiftMinPercent = 20;
        systemConfigMutable()->vtolTransitionFwAuthorityMinPercent = 20;
        mixerConfigInit();
        outputProfileUpdateTask(0);
        armingFlags = ARMED;
        std::fill(std::begin(motor), std::end(motor), 1500);
    }

    void tailSitter()
    {
        mixerProfilesMutable(1)->mixer_config.tailsitterOrientationOffset = true;
    }

    void sourceMc()
    {
        setConfigMixerProfile(1);
        mixerConfigInit();
        rcProfile2 = true;
    }
};

TEST_F(MixerTransitionOutputsTest, ManualRampUpdatesBetweenSlowOwnerTicksWithoutSwitching)
{
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    ASSERT_EQ(MIXERAT_PHASE_TRANSITIONING, mixerProfileAT.phase);
    for (unsigned elapsed = 10; elapsed <= 800; elapsed += 10) {
        now = 1000 + elapsed;
        mixerATUpdateOutputs();
        EXPECT_NEAR(std::min(elapsed / 700.0f, 1.0f), mixerProfileAT.motorRampProgress, 0.00001f);
        EXPECT_EQ(0, currentMixerProfileIndex);
        EXPECT_EQ(MIXERAT_PHASE_TRANSITIONING, mixerProfileAT.phase);
    }
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    EXPECT_EQ(1, currentMixerProfileIndex);
}

TEST_F(MixerTransitionOutputsTest, TailsitterCaptureUsesFwRampNotTargetMcRamp)
{
    tailSitter();
    mixerProfilesMutable(1)->mixer_config.vtolTransitionScaleRampTimeMs = 4000;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    now += 700;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    ASSERT_EQ(MIXERAT_PHASE_TAILSITTER_TO_MC_CAPTURE, mixerProfileAT.phase);
    EXPECT_FLOAT_EQ(0.4f, mixerATGetTailSitterMcAuthorityScale());
    EXPECT_EQ(0, mixerProfileAT.postSwitchFadeMotorMask);
    attitude.values.pitch = 450;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    now += 350;
    mixerATUpdateOutputs();
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    EXPECT_FLOAT_EQ(0.7f, mixerATGetTailSitterMcAuthorityScale());
    EXPECT_EQ(MIXERAT_PHASE_TAILSITTER_TO_MC_CAPTURE, mixerProfileAT.phase);
    now += 350;
    mixerATUpdateOutputs();
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    EXPECT_EQ(MIXERAT_PHASE_IDLE, mixerProfileAT.phase);
    EXPECT_FLOAT_EQ(1.0f, mixerATGetTailSitterMcAuthorityScale());
}

TEST_F(MixerTransitionOutputsTest, CaptureReversalDoesNotJumpAuthority)
{
    tailSitter();
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    now += 700;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    now += 350;
    ASSERT_FLOAT_EQ(0.7f, mixerATGetTailSitterMcAuthorityScale());
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_FW);
    EXPECT_EQ(MIXERAT_PHASE_TRANSITIONING, mixerProfileAT.phase);
    EXPECT_FLOAT_EQ(0.7f, mixerATGetTailSitterMcAuthorityScale());
}

TEST_F(MixerTransitionOutputsTest, CaptureReversalIntoDynamicOffFinishesPreviousRecovery)
{
    tailSitter();
    mixerProfilesMutable(1)->mixer_config.vtolTransitionDynamicMixer = false;
    mixerProfilesMutable(1)->mixer_config.vtolTransitionScaleRampTimeMs = 4000;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    now += 700;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    now += 350;
    ASSERT_FLOAT_EQ(0.7f, mixerATGetTailSitterMcAuthorityScale());
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_FW);
    EXPECT_FLOAT_EQ(0.7f, mixerATGetTailSitterMcAuthorityScale());
    now += 350;
    mixerATUpdateOutputs();
    EXPECT_FLOAT_EQ(0.85f, mixerATGetTailSitterMcAuthorityScale());
}

TEST_F(MixerTransitionOutputsTest, TailsitterAbortFromManualSwitchRecoversWithoutStartingAnotherTransition)
{
    tailSitter();
    sourceMc();
    rcTransition = true;
    outputProfileUpdateTask(0);
    ASSERT_EQ(MIXERAT_PHASE_TRANSITIONING, mixerProfileAT.phase);
    now += 350;
    mixerATUpdateOutputs();
    const float before = mixerATGetTailSitterMcAuthorityScale();
    ASSERT_LT(before, 1.0f);
    rcTransition = false;
    outputProfileUpdateTask(0);
    EXPECT_EQ(MIXERAT_PHASE_IDLE, mixerProfileAT.phase);
    EXPECT_FLOAT_EQ(before, mixerATGetTailSitterMcAuthorityScale());
    now += 700;
    mixerATUpdateOutputs();
    EXPECT_FLOAT_EQ(1.0f, mixerATGetTailSitterMcAuthorityScale());
}

TEST_F(MixerTransitionOutputsTest, TailsitterDynamicOffKeepsFullAuthority)
{
    tailSitter();
    mixerProfilesMutable(0)->mixer_config.vtolTransitionDynamicMixer = false;
    mixerProfilesMutable(1)->mixer_config.vtolTransitionDynamicMixer = false;
    sourceMc();
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_FW);
    now += 350;
    mixerATUpdateOutputs();
    EXPECT_FLOAT_EQ(1.0f, mixerATGetTailSitterMcAuthorityScale());
    EXPECT_FLOAT_EQ(1.0f, mixerATGetPusherScale());
    EXPECT_FLOAT_EQ(1.0f, mixerATGetLiftScale());
}

TEST_F(MixerTransitionOutputsTest, AbortRecoversMcAuthorityButDirectSwitchClearsIt)
{
    tailSitter();
    sourceMc();
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_FW);
    now += 350;
    mixerATUpdateOutputs();
    const float before = mixerATGetTailSitterMcAuthorityScale();
    ASSERT_LT(before, 1.0f);
    mixerATUpdateState(MIXERAT_REQUEST_ABORT);
    EXPECT_FLOAT_EQ(before, mixerATGetTailSitterMcAuthorityScale());
    now += 350;
    mixerATUpdateOutputs();
    EXPECT_NEAR((before + 1.0f) / 2, mixerATGetTailSitterMcAuthorityScale(), 0.00001f);
    ASSERT_TRUE(outputProfileDirectSwitchImmediate(0));
    EXPECT_FALSE(mixerProfileAT.tailSitterAuthorityRecovering);
    EXPECT_FLOAT_EQ(1.0f, mixerATGetTailSitterMcAuthorityScale());
    EXPECT_FALSE(carryoverAtLoad);
}

TEST_F(MixerTransitionOutputsTest, DirectSwitchClearsMotorFadeAndServoHandoff)
{
    mixerProfileAT.phase = MIXERAT_PHASE_POST_SWITCH_FADE;
    mixerProfileAT.hotSwitchDone = true;
    mixerProfileAT.postSwitchFadeMotorMask = 0x1f;
    mixerProfileAT.servoHandoffMask = 3;
    isMixerTransitionMixing = true;
    ASSERT_TRUE(outputProfileDirectSwitchImmediate(1));
    EXPECT_EQ(MIXERAT_PHASE_IDLE, mixerProfileAT.phase);
    EXPECT_EQ(0, mixerProfileAT.postSwitchFadeMotorMask);
    EXPECT_EQ(0, mixerProfileAT.servoHandoffMask);
    EXPECT_FALSE(carryoverAtLoad);
    EXPECT_TRUE(servoCarryover);
    EXPECT_FALSE(isMixerTransitionMixing);
}

TEST_F(MixerTransitionOutputsTest, FailedDirectSwitchPreservesFade)
{
    mixerProfileAT.phase = MIXERAT_PHASE_POST_SWITCH_FADE;
    mixerProfileAT.postSwitchFadeMotorMask = 0x1f;
    mixerProfileAT.servoHandoffMask = 3;
    EXPECT_FALSE(outputProfileDirectSwitchImmediate(-1));
    EXPECT_EQ(MIXERAT_PHASE_POST_SWITCH_FADE, mixerProfileAT.phase);
    EXPECT_EQ(0x1f, mixerProfileAT.postSwitchFadeMotorMask);
    EXPECT_EQ(3, mixerProfileAT.servoHandoffMask);
    EXPECT_TRUE(servoCarryover);
}

TEST_F(MixerTransitionOutputsTest, AlwaysOnRequestsTransitionInsteadOfDirectSwitch)
{
    systemConfigMutable()->vtolAutotransitionAlways = true;
    rcProfile2 = true;
    outputProfileUpdateTask(0);
    ASSERT_EQ(MIXERAT_PHASE_TRANSITIONING, mixerProfileAT.phase);
    EXPECT_EQ(0, currentMixerProfileIndex);
    now += 350;
    mixerATUpdateOutputs();
    EXPECT_FLOAT_EQ(0.5f, mixerProfileAT.motorRampProgress);
    now += 350;
    outputProfileUpdateTask(0);
    EXPECT_EQ(1, currentMixerProfileIndex);
}

TEST_F(MixerTransitionOutputsTest, DirectSwitchDoesNotStartARampInEitherDirection)
{
    for (const bool profile2 : {true, false}) {
        rcProfile2 = profile2;
        outputProfileUpdateTask(0);
        EXPECT_EQ(profile2 ? 1 : 0, currentMixerProfileIndex);
        EXPECT_EQ(MIXERAT_PHASE_IDLE, mixerProfileAT.phase);
        EXPECT_EQ(0, mixerProfileAT.servoHandoffMask);
        EXPECT_EQ(0, mixerProfileAT.postSwitchFadeMotorMask);
        EXPECT_FALSE(carryoverAtLoad);
    }
}

TEST_F(MixerTransitionOutputsTest, NavigationHandbackStillRequiresMatchingSwitch)
{
    testNavFlags = NAV_AUTO_WP;
    rcProfile2 = true;
    outputProfileUpdateTask(0);
    EXPECT_EQ(0, currentMixerProfileIndex);
    testNavFlags = static_cast<navigationFSMStateFlags_t>(0);
    outputProfileUpdateTask(0);
    EXPECT_EQ(0, currentMixerProfileIndex);
    rcTransition = true;
    outputProfileUpdateTask(0);
    EXPECT_FALSE(mixerATIsActive());
    EXPECT_FALSE(isMixerTransitionMixing);
    rcTransition = false;
    rcProfile2 = false;
    outputProfileUpdateTask(0);
    rcProfile2 = true;
    outputProfileUpdateTask(0);
    EXPECT_EQ(1, currentMixerProfileIndex);
}

TEST_F(MixerTransitionOutputsTest, TailsitterRejectsZeroAuthorityAndAutonomousRequests)
{
    tailSitter();
    systemConfigMutable()->vtolTransitionMcAuthorityMinPercent = 0;
    EXPECT_FALSE(checkMixerATRequired(MIXERAT_REQUEST_MANUAL_TO_MC));
    systemConfigMutable()->vtolTransitionMcAuthorityMinPercent = 40;
    EXPECT_TRUE(checkMixerATRequired(MIXERAT_REQUEST_MANUAL_TO_MC));
    EXPECT_FALSE(checkMixerATRequired(MIXERAT_REQUEST_MISSION_TO_MC));
    EXPECT_FALSE(checkMixerATRequired(MIXERAT_REQUEST_LAND));
    sourceMc();
    EXPECT_FALSE(checkMixerATRequired(MIXERAT_REQUEST_MISSION_TO_FW));
    EXPECT_FALSE(checkMixerATRequired(MIXERAT_REQUEST_RTH));
}

TEST_F(MixerTransitionOutputsTest, TailsitterZeroRampDoesNotSkipAttitudeCapture)
{
    tailSitter();
    currentMixerConfig.vtolTransitionScaleRampTimeMs = 0;
    currentMixerConfig.switchTransitionTimer = 0;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    ASSERT_EQ(MIXERAT_PHASE_TAILSITTER_TO_MC_CAPTURE, mixerProfileAT.phase);
    EXPECT_FLOAT_EQ(1.0f, mixerATGetTailSitterMcAuthorityScale());
    now += 10000;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    EXPECT_EQ(MIXERAT_PHASE_TAILSITTER_TO_MC_CAPTURE, mixerProfileAT.phase);
}

TEST_F(MixerTransitionOutputsTest, DisarmClearsTailsitterRecovery)
{
    tailSitter();
    sourceMc();
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_FW);
    now += 350;
    mixerATUpdateOutputs();
    mixerATUpdateState(MIXERAT_REQUEST_ABORT);
    ASSERT_TRUE(mixerProfileAT.tailSitterAuthorityRecovering);
    armingFlags = 0;
    outputProfileUpdateTask(0);
    EXPECT_FALSE(mixerProfileAT.tailSitterAuthorityRecovering);
    EXPECT_FLOAT_EQ(1.0f, mixerATGetTailSitterMcAuthorityScale());
}

TEST_F(MixerTransitionOutputsTest, FastOutputSamplesDoNotBypassAirspeedConfirmation)
{
    sourceMc();
    systemConfigMutable()->vtolTransitionToFwMinAirspeed = 1000;
    validAirspeed = true;
    airspeed = 0;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_FW);
    // A brief spike cannot complete the switch, even with frequent output updates.
    for (unsigned elapsed = 1; elapsed <= 700; elapsed++) {
        now = 1000 + elapsed;
        airspeed = elapsed <= 20 ? 1200 : 500;
        mixerATUpdateOutputs();
        if (elapsed % 100 == 0) {
            mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_FW);
        }
        ASSERT_EQ(1, currentMixerProfileIndex);
    }
    airspeed = 1200;
    for (unsigned elapsed = 1; elapsed <= 400; elapsed++) {
        now++;
        mixerATUpdateOutputs();
    }
    EXPECT_EQ(1, currentMixerProfileIndex);
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_FW);
    EXPECT_EQ(0, currentMixerProfileIndex);
}

TEST_F(MixerTransitionOutputsTest, PostSwitchOutputUpdateDoesNotCompleteOwnerState)
{
    mixerProfileAT.phase = MIXERAT_PHASE_POST_SWITCH_FADE;
    mixerProfileAT.direction = MIXERAT_DIRECTION_TO_FW;
    mixerProfileAT.postSwitchFadeDurationMs = 700;
    mixerProfileAT.postSwitchFadeStartTime = now;
    mixerProfileAT.postSwitchFadeInitialScale = 0.4f;
    for (unsigned elapsed = 10; elapsed <= 800; elapsed += 10) {
        now = 1000 + elapsed;
        mixerATUpdateOutputs();
        EXPECT_NEAR(0.4f * (1 - std::min(elapsed / 700.0f, 1.0f)), mixerATGetLiftScale(), 0.00001f);
        EXPECT_EQ(MIXERAT_PHASE_POST_SWITCH_FADE, mixerProfileAT.phase);
    }
    mixerATUpdateState(MIXERAT_REQUEST_NONE);
    EXPECT_EQ(MIXERAT_PHASE_IDLE, mixerProfileAT.phase);
}

TEST_F(MixerTransitionOutputsTest, RepeatedAlwaysOnTailsitterTransitionsDoNotRetainAuthorityOrCapture)
{
    tailSitter();
    systemConfigMutable()->vtolAutotransitionAlways = true;
    for (int cycle = 0; cycle < 10; cycle++) {
        for (const bool toMc : {true, false}) {
            rcProfile2 = toMc;
            attitude.values.pitch = 450;
            outputProfileUpdateTask(0);
            ASSERT_EQ(MIXERAT_PHASE_TRANSITIONING, mixerProfileAT.phase);
            for (unsigned elapsed = 10; elapsed <= 1500; elapsed += 10) {
                now += 10;
                mixerATUpdateOutputs();
                if (elapsed % 100 == 0) {
                    outputProfileUpdateTask(0);
                }
                EXPECT_GE(mixerATGetTailSitterMcAuthorityScale(), 0.4f);
                EXPECT_LE(mixerATGetTailSitterMcAuthorityScale(), 1.0f);
            }
            ASSERT_EQ(toMc ? 1 : 0, currentMixerProfileIndex);
            ASSERT_EQ(MIXERAT_PHASE_IDLE, mixerProfileAT.phase);
            EXPECT_FALSE(mixerProfileAT.tailSitterAuthorityRecovering);
            EXPECT_FALSE(isMixerTransitionMixing);
            EXPECT_FLOAT_EQ(1.0f, mixerATGetTailSitterMcAuthorityScale());
        }
    }
}

TEST_F(MixerTransitionOutputsTest, FailsafeDuringCaptureKeepsMcAndRecoversAuthority)
{
    tailSitter();
    systemConfigMutable()->vtolAutotransitionAlways = true;
    rcProfile2 = true;
    outputProfileUpdateTask(0);
    now += 700;
    outputProfileUpdateTask(0);
    ASSERT_EQ(MIXERAT_PHASE_TAILSITTER_TO_MC_CAPTURE, mixerProfileAT.phase);
    now += 100;
    const float before = mixerATGetTailSitterMcAuthorityScale();
    flightModeFlags = FAILSAFE_MODE;
    rcProfile2 = false;
    outputProfileUpdateTask(0);
    EXPECT_EQ(MIXERAT_PHASE_IDLE, mixerProfileAT.phase);
    EXPECT_EQ(1, currentMixerProfileIndex);
    EXPECT_FLOAT_EQ(before, mixerATGetTailSitterMcAuthorityScale());
    for (int tick = 0; tick < 8; tick++) {
        now += 100;
        mixerATUpdateOutputs();
        outputProfileUpdateTask(0);
        EXPECT_EQ(1, currentMixerProfileIndex);
        EXPECT_FALSE(isMixerTransitionMixing);
    }
    EXPECT_FLOAT_EQ(1.0f, mixerATGetTailSitterMcAuthorityScale());
    EXPECT_FALSE(mixerProfileAT.tailSitterAuthorityRecovering);
}

TEST_F(MixerTransitionOutputsTest, TimerRampAndOwnerSwitchSurviveClockWrap)
{
    now = UINT32_MAX - 349;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    for (unsigned elapsed = 10; elapsed <= 700; elapsed += 10) {
        now += 10;
        mixerATUpdateOutputs();
        EXPECT_NEAR(elapsed / 700.0f, mixerProfileAT.motorRampProgress, 0.00001f);
        EXPECT_EQ(0, currentMixerProfileIndex);
    }
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    EXPECT_EQ(1, currentMixerProfileIndex);
}

TEST_F(MixerTransitionOutputsTest, FullTailsitterAuthorityStillRequiresStableCaptureAttitude)
{
    tailSitter();
    systemConfigMutable()->vtolTransitionMcAuthorityMinPercent = 100;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    now += 700;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    attitude.values.pitch = 450;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    now += 200;
    attitude.values.pitch = 600;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    now += 200;
    attitude.values.pitch = 450;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    now += 200;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    EXPECT_EQ(MIXERAT_PHASE_TAILSITTER_TO_MC_CAPTURE, mixerProfileAT.phase);
    EXPECT_FLOAT_EQ(1.0f, mixerATGetTailSitterMcAuthorityScale());
    now += 100;
    mixerATUpdateState(MIXERAT_REQUEST_MANUAL_TO_MC);
    EXPECT_EQ(MIXERAT_PHASE_IDLE, mixerProfileAT.phase);
}

TEST_F(MixerTransitionOutputsTest, AlwaysOnRthHandbackBlocksStaleSwitchAndMiddlePosition)
{
    systemConfigMutable()->vtolAutotransitionAlways = true;
    testNavFlags = NAV_AUTO_RTH;
    rcProfile2 = true;
    outputProfileUpdateTask(0);
    EXPECT_EQ(0, currentMixerProfileIndex);
    testNavFlags = static_cast<navigationFSMStateFlags_t>(0);
    rcTransition = true;
    outputProfileUpdateTask(0);
    EXPECT_FALSE(mixerATIsActive());
    EXPECT_FALSE(isMixerTransitionMixing);
    rcTransition = false;
    rcProfile2 = false;
    outputProfileUpdateTask(0);
    EXPECT_FALSE(mixerATIsActive());
    rcProfile2 = true;
    outputProfileUpdateTask(0);
    EXPECT_EQ(MIXERAT_PHASE_TRANSITIONING, mixerProfileAT.phase);
    EXPECT_EQ(0, currentMixerProfileIndex);
}
