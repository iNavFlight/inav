/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify this software
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * INAV is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of FITNESS FOR A PARTICULAR
 * PURPOSE. See the GNU General Public License for more details.
 */

// Reproduces a real bug: mixerInit() only *sets* throttleRateLimit when
// currentBatteryProfile->motor.throttleRateLimiter is non-zero; there is no
// else branch to clear it. setBatteryProfile() swaps currentBatteryProfile
// but never re-derives throttleRateLimit. So arming on a battery profile
// with the limiter enabled, then hot-switching (in flight, via CLI or MSP)
// to a profile with the limiter disabled, leaves the OLD rate limit active
// for the rest of the flight (see mixTable()'s "FW throttle rate limiter"
// section, which just checks `if (STATE(AIRPLANE) && throttleRateLimit)`).
//
// This links the real flight/mixer.c and sensors/battery.c so the test
// drives the actual production entry points (mixerInit(), setBatteryProfile())
// rather than reimplementing their logic. Neither .c file is fully
// self-contained, so - following the same pattern as battery_ina226_unittest.cc
// and telemetry_hott_unittest.cc - everything else they touch is provided
// here as either a minimal PG registration or a stub function/global.

#include "gtest/gtest.h"

extern "C" {
    #include <stdbool.h>
    #include <stdint.h>
    #include <string.h>

    #include "build/debug.h"
    #include "common/time.h"
    #include "config/feature.h"
    #include "config/parameter_group.h"
    #include "config/parameter_group_ids.h"
    #include "fc/config.h"
    #include "fc/rc_controls.h"
    #include "fc/runtime_config.h"
    #include "flight/mixer.h"
    #include "flight/mixer_profile.h"
    #include "flight/pid.h"
    #include "io/beeper.h"
    #include "navigation/navigation.h"
    #include "programming/logic_condition.h"
    #include "rx/rx.h"
    #include "sensors/battery.h"
}

// mixer.c and battery.c pull in PG storage that is normally provided by
// mixer_profile.c / fc/rc_controls.c / fc/config.c / fc/runtime_config.c.
// None of those .c files are linked into this test binary (they carry a
// much larger dependency graph than this test needs), so - exactly like
// telemetry_hott_unittest.cc does for navConfig/telemetryConfig - this test
// provides its own minimal PG storage for them instead.
extern "C" {
    PG_REGISTER_ARRAY(mixerProfile_t, MAX_MIXER_PROFILE_COUNT, mixerProfiles, PG_MIXER_PROFILE, 1);
    PG_REGISTER(rcControlsConfig_t, rcControlsConfig, PG_RC_CONTROLS_CONFIG, 0);

    void pgResetFn_batteryProfiles(batteryProfile_t *instance);
}

// Plain globals normally defined in mixer_profile.c / fc/runtime_config.c /
// flight/pid.c / programming/logic_condition.c / fc/config.c /
// navigation/navigation_private.c. mixerInit()/setBatteryProfile() never
// touch these at runtime in this test (they only matter to code paths in
// mixTable()/batteryUpdate()/etc. that this test does not call), but the
// whole flight/mixer.c and sensors/battery.c translation units are linked
// in, so every symbol they reference anywhere must resolve at link time.
extern "C" {
    mixerConfig_t currentMixerConfig;
    bool isMixerTransitionMixing = false;

    uint32_t stateFlags = 0;
    uint32_t flightModeFlags = 0;
    uint32_t armingFlags = 0;

    int16_t rcCommand[4] = {};
    int16_t axisPID[FLIGHT_DYNAMICS_INDEX_COUNT] = {};

    uint64_t logicConditionsGlobalFlags = 0;
    int logicConditionValuesByType[LOGIC_CONDITION_LAST] = {};

    systemConfig_t systemConfig_System = {};
    systemConfig_t systemConfig_Copy = {};
    navConfig_t navConfig_System = {};
    navConfig_t navConfig_Copy = {};

    int32_t debug[DEBUG32_VALUE_COUNT] = {};
    uint8_t debugMode = 0;
    simulatorData_t simulatorData = {};
}

// battery.c includes navigation/navigation_private.h itself and is compiled
// against the real (large) navigationPosControl_t there; this test never
// calls the battery.c functions that dereference posControl's fields
// (currentMeterUpdate()), so a minimal stand-in only needs to satisfy the
// linker. Matches the pattern already used by battery_ina226_unittest.cc.
typedef int navigationFSMStateFlags_t;
typedef struct navigationPosControl_s {
    int navState;
} navigationPosControl_t;

extern "C" {
    navigationPosControl_t posControl = {};

    uint32_t fakeFeatures = 0;
    bool feature(uint32_t mask)
    {
        return fakeFeatures & mask;
    }

    bool setConfigProfile(uint8_t profileIndex)
    {
        (void)profileIndex;
        return true;
    }

    timeMs_t fakeMillis = 0;
    timeMs_t millis(void) { return fakeMillis; }
    timeUs_t micros(void) { return 0; }
    void delay(timeMs_t ms) { (void)ms; }

    bool failsafeIsActive(void) { return false; }
    void beeper(beeperMode_e mode) { (void)mode; }

    uint16_t adcGetChannel(uint8_t channel) { (void)channel; return 0; }
    uint16_t fakeBattSensorGetVBat(void) { return 0; }
    uint16_t fakeBattSensorGetAmerperage(void) { return 0; }

    navigationFSMStateFlags_t navGetCurrentStateFlags(void) { return 0; }
    bool throttleStickIsLow(void) { return true; }

    bool navigationRequiresAutoThrottleMode(void) { return false; }
    bool navigationIsFlyingAutonomousMode(void) { return false; }
    bool isFixedwingAutoSpeedActive(void) { return false; }
    bool navigationIsExecutingAnEmergencyLanding(void) { return false; }
    int16_t rxGetChannelValue(unsigned channelNumber) { (void)channelNumber; return 1500; }

    float getThrottleScale(float globalThrottleScale) { return globalThrottleScale; }

    float getFlightTime(void) { return 0.0f; }
    uint32_t getFlyingEnergy(void) { return 0; }
    uint32_t getTotalTravelDistance(void) { return 0; }
}

namespace {

// Sets up two battery profiles (limiter on / limiter off) plus a
// multirotor mixer config, mirroring the setup style of
// battery_ina226_unittest.cc: reset PG storage via the real
// pgResetFn_batteryProfiles(), point currentBatteryProfile/currentMixerConfig
// at it, then drive the real production entry points (mixerInit(),
// setBatteryProfile()) rather than poking internal state directly.
class ThrottleRateLimitTest : public ::testing::Test {
protected:
    void SetUp() override
    {
        memset(&currentMixerConfig, 0, sizeof(currentMixerConfig));
        currentMixerConfig.platformType = PLATFORM_MULTIROTOR;

        pgResetFn_batteryProfiles(*batteryProfiles_array());

        batteryProfile_t *profileWithLimiter = batteryProfilesMutable(0);
        profileWithLimiter->motor.throttleIdle = 15.0f;
        profileWithLimiter->motor.throttleScale = 1.0f;
        profileWithLimiter->motor.throttleRateLimiter = 100; // 100ms min->max, limiter ON

        batteryProfile_t *profileWithoutLimiter = batteryProfilesMutable(1);
        profileWithoutLimiter->motor.throttleIdle = 15.0f;
        profileWithoutLimiter->motor.throttleScale = 1.0f;
        profileWithoutLimiter->motor.throttleRateLimiter = 0;  // limiter OFF

        currentBatteryProfile = batteryProfiles(0);

        // A quad: mixerInit() latches the motor count on its first call in this process
        for (int i = 0; i < 4; i++) {
            mixerProfilesMutable(0)->MotorMixers[i].throttle = 1.0f;
        }

        armingFlags = 0;
        fakeMillis = 0;
        fakeFeatures = 0;
    }
};

TEST_F(ThrottleRateLimitTest, MixerInitActivatesLimitWhenProfileEnablesIt)
{
    mixerInit();

    EXPECT_NE(0.0f, mixerGetThrottleRateLimit())
        << "mixerInit() should derive a non-zero throttleRateLimit from "
           "a battery profile with motor.throttleRateLimiter set";
}

// This is the actual bug reproduction: after arming on a profile with the
// limiter enabled, hot-switching to a profile with the limiter disabled
// (in flight, via CLI, or via MSP) must clear the limit. It currently does
// not, because setBatteryProfile() never re-derives throttleRateLimit and
// mixerInit()'s branch has no else to clear the stale value.
TEST_F(ThrottleRateLimitTest, SwitchingToProfileWithLimiterDisabledClearsLimit)
{
    mixerInit();
    ASSERT_NE(0.0f, mixerGetThrottleRateLimit())
        << "Precondition failed: limiter should be active on profile 0";

    setBatteryProfile(1);

    EXPECT_EQ(0.0f, mixerGetThrottleRateLimit())
        << "BUG: throttleRateLimit still holds the previous profile's value "
           "after switching to a battery profile with the rate limiter "
           "disabled (motor.throttleRateLimiter == 0). mixTable()'s "
           "\"FW throttle rate limiter\" check (if (STATE(AIRPLANE) && "
           "throttleRateLimit)) will keep applying the OLD limit for the "
           "rest of the flight.";
}

// Motor wizard: the firmware itself times the twitching of one motor while
// disarmed. It shares this file because it needs the same mixer.c harness.
class MotorWizardTest : public ThrottleRateLimitTest {
protected:
    void SetUp() override
    {
        ThrottleRateLimitTest::SetUp();
        mixerInit();
        motorWizardStop();
    }

    // mixer.c keeps what mixerInit() derived from the features in statics
    void TearDown() override
    {
        motorWizardStop();
        fakeFeatures = 0;
        rcControlsConfigMutable()->mid_throttle_deadband = 0;
        mixerInit();
    }

    bool isSpinningAt(timeMs_t ms, int motorIndex)
    {
        fakeMillis = startMs + ms;
        mixTable(0.001f);
        for (int i = 0; i < 4; i++) {
            if (i != motorIndex && motor[i] != motor_disarmed[i]) {
                ADD_FAILURE() << "motor " << i << " moved at " << ms << " ms";
            }
        }
        return motor[motorIndex] > motor_disarmed[motorIndex];
    }

    const timeMs_t startMs = 100000;
};

TEST_F(MotorWizardTest, StartIsRefusedWhenArmedOrIndexOutOfRange)
{
    EXPECT_FALSE(motorWizardStart(4));
    EXPECT_FALSE(motorWizardStart(255));

    armingFlags = ARMED;
    EXPECT_FALSE(motorWizardStart(1));

    // The refused start must not be waiting for the disarm
    armingFlags = 0;
    EXPECT_FALSE(areMotorsRunning());
    EXPECT_FALSE(isSpinningAt(0, 1));
}

TEST_F(MotorWizardTest, TwitchesOneMotorAndEndsByItself)
{
    fakeMillis = startMs;
    ASSERT_TRUE(motorWizardStart(2));
    EXPECT_TRUE(areMotorsRunning());

    for (timeMs_t ms = 0; ms < MOTOR_WIZARD_DURATION_MS; ms++) {
        ASSERT_EQ(ms % MOTOR_WIZARD_CYCLE_MS < MOTOR_WIZARD_SPIN_MS, isSpinningAt(ms, 2)) << "at " << ms << " ms";
    }
    EXPECT_TRUE(areMotorsRunning());

    // Hard limit: nothing after it although nobody sent a stop
    EXPECT_FALSE(isSpinningAt(MOTOR_WIZARD_DURATION_MS, 2));
    EXPECT_FALSE(areMotorsRunning());
    EXPECT_FALSE(isSpinningAt(MOTOR_WIZARD_DURATION_MS + 10, 2));
}

TEST_F(MotorWizardTest, StopEndsTheSequenceAndStartReplacesIt)
{
    fakeMillis = startMs;
    ASSERT_TRUE(motorWizardStart(0));
    EXPECT_TRUE(isSpinningAt(10, 0));

    ASSERT_TRUE(motorWizardStart(3));
    EXPECT_TRUE(isSpinningAt(20, 3));

    motorWizardStop();
    EXPECT_FALSE(isSpinningAt(30, 3));
    EXPECT_FALSE(areMotorsRunning());
}

// The caller repeats the start while the user is still looking for the motor.
// That must only push the time limit away, not restart the spin/pause rhythm.
TEST_F(MotorWizardTest, RepeatedStartKeepsTheRhythmAndExtendsTheLimit)
{
    fakeMillis = startMs;
    ASSERT_TRUE(motorWizardStart(1));

    // Repeated at a rate unrelated to the cycle, well past the time limit
    const timeMs_t repeatMs = MOTOR_WIZARD_CYCLE_MS + MOTOR_WIZARD_SPIN_MS / 2 + 7;
    const timeMs_t lastMs = 4 * MOTOR_WIZARD_DURATION_MS;
    timeMs_t lastStartMs = 0;
    for (timeMs_t ms = 0; ms < lastMs; ms++) {
        if (ms % repeatMs == repeatMs - 1) {
            fakeMillis = startMs + ms;
            ASSERT_TRUE(motorWizardStart(1));
            lastStartMs = ms;
        }
        ASSERT_EQ(ms % MOTOR_WIZARD_CYCLE_MS < MOTOR_WIZARD_SPIN_MS, isSpinningAt(ms, 1)) << "at " << ms << " ms";
    }

    // No more starts: it ends by itself, at most one cycle before the full limit
    EXPECT_TRUE(isSpinningAt(lastMs / MOTOR_WIZARD_CYCLE_MS * MOTOR_WIZARD_CYCLE_MS + MOTOR_WIZARD_CYCLE_MS, 1));
    EXPECT_TRUE(areMotorsRunning());
    EXPECT_FALSE(isSpinningAt(lastStartMs + MOTOR_WIZARD_DURATION_MS, 1));
    EXPECT_FALSE(areMotorsRunning());
}

// Reversible motors rest at neutral and writeMotors() spins them forward only
// above the centre throttle deadband, so the twitch has to be above it.
TEST_F(MotorWizardTest, ReversibleMotorsTwitchAboveTheCentreDeadband)
{
    fakeFeatures = FEATURE_REVERSIBLE_MOTORS;
    rcControlsConfigMutable()->mid_throttle_deadband = 150;
    mixerInit();

    fakeMillis = startMs;
    ASSERT_TRUE(motorWizardStart(0));
    ASSERT_TRUE(isSpinningAt(0, 0));
    EXPECT_GT(motor[0], 1500 + 150);
}

} // namespace
