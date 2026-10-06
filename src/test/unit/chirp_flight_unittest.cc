/* SPDX-License-Identifier: GPL-3.0-or-later */

#include <cmath>
#include <cstring>
#include <functional>
#include <limits>
#include "gtest/gtest.h"

extern "C" {
#include "platform.h"
#include "build/debug.h"
#include "common/axis.h"
#include "common/maths.h"
#include "drivers/time.h"
#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/rc_modes.h"
#include "fc/runtime_config.h"
#include "flight/chirp.h"
#include "flight/chirp_flight.h"
#include "flight/imu.h"
#include "flight/mixer_profile.h"
#include "blackbox/blackbox.h"
#include "programming/logic_condition.h"
#include "rx/rx.h"
#include "sensors/gyro.h"

uint32_t armingFlags;
uint32_t stateFlags;
uint32_t flightModeFlags;
uint64_t logicConditionsGlobalFlags;
int32_t debug[DEBUG32_VALUE_COUNT];
uint8_t debugMode;
int16_t rcCommand[4];
gyro_t gyro;
attitudeEulerAngles_t attitude;
rxRuntimeConfig_t rxRuntimeConfig;
systemConfig_t systemConfig_System;
mixerProfile_t mixerProfiles_SystemArray[MAX_MIXER_PROFILE_COUNT];
blackboxConfig_t blackboxConfig_System;
bool isMixerTransitionMixing;

static timeUs_t now;
static int16_t channels[MAX_SUPPORTED_RC_CHANNEL_COUNT];
static bool receiving, validChannels, failsafe, receivingFailsafeData, mspOverride, axisOverride;
static uint32_t features;
static uint8_t profile, mixerProfile;
static float mixRange;
static BlackboxState logState;

timeUs_t micros(void) { return now; }
bool feature(uint32_t mask) { return features & mask; }
bool failsafeIsActive(void) { return failsafe; }
bool failsafeIsReceivingRxData(void) { return receivingFailsafeData; }
bool rxIsReceivingSignal(void) { return receiving; }
bool rxAreFlightChannelsValid(void) { return validChannels; }
int16_t rxGetChannelValue(unsigned channel) { return channels[channel]; }
bool IS_RC_MODE_ACTIVE(boxId_e id) { return id == BOXMSPRCOVERRIDE && mspOverride; }
bool isFlightAxisAngleOverrideActive(uint8_t) { return axisOverride; }
bool isFlightAxisRateOverrideActive(uint8_t) { return axisOverride; }
float getMotorMixRange(void) { return mixRange; }
BlackboxState getBlackboxState(void) { return logState; }
uint8_t getConfigProfile(void) { return profile; }
uint8_t getConfigMixerProfile(void) { return mixerProfile; }
}

class ChirpFlightTest : public testing::Test {
protected:
    void SetUp() override
    {
        chirpConfigMutable()->axis = CHIRP_AXIS_OFF;
        chirpFlightUpdate(0.001f, true);
        now = 0;
        armingFlags = ARMED;
        stateFlags = MULTIROTOR;
        flightModeFlags = ANGLE_MODE;
        logicConditionsGlobalFlags = 0;
        systemConfig_System = {};
        mixerProfiles_SystemArray[0] = {};
        mixerConfigMutable()->platformType = PLATFORM_MULTIROTOR;
        chirpConfigMutable()->axis = CHIRP_AXIS_ROLL;
        chirpConfigMutable()->triggerChannel = 5;
        chirpConfigMutable()->amplitude = 10;
        blackboxConfigMutable()->rate_num = 1;
        blackboxConfigMutable()->rate_denom = 1;
        rxRuntimeConfig.channelCount = 16;
        for (auto &channel : channels) channel = 1500;
        channels[4] = 1000;
        std::memset(rcCommand, 0, sizeof(rcCommand));
        rcCommand[THROTTLE] = 1500;
        gyro = {};
        attitude = {};
        receiving = receivingFailsafeData = validChannels = true;
        failsafe = mspOverride = axisOverride = isMixerTransitionMixing = false;
        features = profile = mixerProfile = 0;
        mixRange = 0;
        logState = BLACKBOX_STATE_RUNNING;
        debugMode = DEBUG_CHIRP;
    }

    void tick(float dt = 0.001f)
    {
        now += 1000;
        chirpFlightUpdate(dt, true);
    }

    void start()
    {
        channels[4] = 1000;
        tick();
        ASSERT_EQ(debug[0], CHIRP_READY);
        channels[4] = 2000;
        for (int i = 0; i < 3333; ++i) tick();
        ASSERT_EQ(debug[0], CHIRP_RUNNING);
        ASSERT_NE(chirpApplyRate(0, 0), 0);
    }
};

TEST_F(ChirpFlightTest, AppliesOnlyToSelectedAxisAndRecordsActualResponse)
{
    start();
    EXPECT_NE(chirpApplyRate(0, 42), 42);
    EXPECT_FLOAT_EQ(chirpApplyRate(1, 42), 42);
    EXPECT_FLOAT_EQ(chirpApplyRate(2, 42), 42);
    chirpLogResponse(0, 12.25f, -3.5f, 99);
    EXPECT_EQ(debug[5], 1225);
    EXPECT_EQ(debug[6], -350);
    EXPECT_EQ(debug[7], 9900);
    chirpLogResponse(1, 0, 0, 0);
    EXPECT_EQ(debug[5], 1225);
}

TEST_F(ChirpFlightTest, AllAxesCanBeSelected)
{
    for (int axis = 1; axis <= 3; ++axis) {
        SetUp();
        chirpConfigMutable()->axis = axis;
        channels[4] = 1000;
        tick();
        channels[4] = 2000;
        for (int i = 0; i < 3333; ++i) tick();
        ASSERT_EQ(debug[0], CHIRP_RUNNING);
        for (int target = 0; target < 3; ++target) {
            if (target + 1 == axis) EXPECT_NE(chirpApplyRate(target, 0), 0);
            else EXPECT_FLOAT_EQ(chirpApplyRate(target, 0), 0);
        }
    }
}

TEST_F(ChirpFlightTest, RealFlightGuardsStopOutputAndLatchUntilNewTrigger)
{
    const std::function<void()> failures[] = {
        [] { armingFlags = 0; },
        [] { stateFlags = AIRPLANE; },
        [] { stateFlags |= LANDING_DETECTED; },
        [] { mixerConfigMutable()->platformType = PLATFORM_TRICOPTER; },
        [] { isMixerTransitionMixing = true; },
        [] { features = FEATURE_REVERSIBLE_MOTORS; },
        [] { flightModeFlags = 0; },
        [] { flightModeFlags |= NAV_RTH_MODE; },
        [] { flightModeFlags |= NAV_POSHOLD_MODE; },
        [] { flightModeFlags |= NAV_ALTHOLD_MODE; },
        [] { failsafe = true; },
        [] { receiving = false; },
        [] { validChannels = false; },
        [] { receivingFailsafeData = false; },
        [] { mspOverride = true; },
        [] { channels[0] = 1600; },
        [] { channels[2] = 1400; },
        [] { rcCommand[0] = 100; },
        [] { rcCommand[THROTTLE] = 1200; },
        [] { rcCommand[THROTTLE] = 1650; },
        [] { axisOverride = true; },
        [] { logicConditionsGlobalFlags = LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_THROTTLE; },
        [] { logicConditionsGlobalFlags = LOGIC_CONDITION_GLOBAL_FLAG_OVERRIDE_RC_CHANNEL; },
        [] { attitude.values.pitch = 201; },
        [] { attitude.values.roll = -201; },
        [] { gyro.gyroADCf[0] = 201; },
        [] { gyro.gyroADCf[1] = std::numeric_limits<float>::quiet_NaN(); },
        [] { mixRange = 0.91f; },
        [] { mixRange = std::numeric_limits<float>::quiet_NaN(); },
        [] { logState = BLACKBOX_STATE_PAUSED; },
        [] { blackboxConfigMutable()->rate_denom = 2; },
        [] { blackboxConfigMutable()->rate_num = 0; },
        [] { blackboxConfigMutable()->rate_denom = 0; },
        [] { profile = 1; },
        [] { mixerProfile = 1; },
        [] { chirpConfigMutable()->axis = CHIRP_AXIS_YAW; },
        [] { chirpConfigMutable()->amplitude = 20; },
        [] { chirpConfigMutable()->triggerChannel = 6; },
        [] { chirpConfigMutable()->triggerChannel = 255; },
        [] { chirpConfigMutable()->axis = 255; },
    };
    int index = 0;
    for (const auto &fail : failures) {
        SCOPED_TRACE(index++);
        SetUp();
        start();
        fail();
        tick();
        ASSERT_EQ(debug[0], CHIRP_ABORTED);
        ASSERT_NE(debug[4], 0);
        for (int axis = 0; axis < 3; ++axis) EXPECT_FLOAT_EQ(chirpApplyRate(axis, 42), 42);
    }
}

TEST_F(ChirpFlightTest, ReceiverRecoveryDoesNotRestartWhileSwitchHigh)
{
    start();
    receiving = false;
    tick();
    receiving = true;
    for (int i = 0; i < 3000; ++i) tick();
    EXPECT_EQ(debug[0], CHIRP_ABORTED);
    EXPECT_FLOAT_EQ(chirpApplyRate(0, 0), 0);
    start();
}

TEST_F(ChirpFlightTest, MissingDebugLoggingAndInvalidLoopTimesStopOutput)
{
    start();
    debugMode = DEBUG_NONE;
    tick();
    EXPECT_FLOAT_EQ(chirpApplyRate(0, 0), 0);
    for (float dt : {0.0f, -0.001f, 0.002f, std::numeric_limits<float>::quiet_NaN()}) {
        SetUp();
        start();
        tick(dt);
        EXPECT_EQ(debug[0], CHIRP_ABORTED);
        EXPECT_FLOAT_EQ(chirpApplyRate(0, 0), 0);
    }
}

TEST_F(ChirpFlightTest, InvalidChannelsAndMspOverlayChannelsCannotTrigger)
{
    for (int channel : {0, 4, 13, 34, 255}) {
        SetUp();
        chirpConfigMutable()->triggerChannel = channel;
        tick();
        EXPECT_EQ(debug[0], CHIRP_ABORTED);
        EXPECT_EQ(debug[4] & CHIRP_INHIBIT_CONFIG, CHIRP_INHIBIT_CONFIG);
    }
}

TEST_F(ChirpFlightTest, UnreadyOrWrongControllerCannotExcite)
{
    start();
    chirpFlightUpdate(0.001f, false);
    EXPECT_EQ(debug[0], CHIRP_ABORTED);
    EXPECT_FLOAT_EQ(chirpApplyRate(0, 0), 0);
}
