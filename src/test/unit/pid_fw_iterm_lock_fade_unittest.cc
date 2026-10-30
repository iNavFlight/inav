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

// The fixed-wing I-term lock (iTermLockApply) damps P and D and freezes the I-term while the stick asks for a big
// rate, leaving the feed-forward to fly the manoeuvre. An I-term built in a motion the pilot did not ask for (a
// pitch rate coupled from a full-aileron roll, say) is then held against the stick and eats that feed-forward: a
// trainer's pull-out at 34 m/s got +30 us of elevator from a demand of 111 deg/s, with the I-term at +87 against a
// feed-forward of -115, and hit the ground one second short. The fix fades an I-term that opposes the rate target
// while the lock attenuates the I-term (time constant FW_ITERM_LOCK_FADE_S), and leaves one that helps it alone.
//
// Links the real flight/pid.c through an access shim and drives its fixed-wing rate controller on one axis; the
// dependencies of the rest of pid.c are stubbed as in pid_apa_iterm_update_unittest.cc.

#include "gtest/gtest.h"

extern "C" {
    #include <stdbool.h>
    #include <stdint.h>
    #include <string.h>
    #include <math.h>

    #include "build/debug.h"
    #include "common/axis.h"
    #include "common/filter.h"
    #include "common/fp_pid.h"
    #include "common/maths.h"
    #include "common/vector.h"
    #include "config/parameter_group.h"
    #include "config/parameter_group_ids.h"
    #include "fc/config.h"
    #include "fc/control_profile.h"
    #include "fc/rc_controls.h"
    #include "fc/rc_modes.h"
    #include "fc/runtime_config.h"
    #include "flight/adaptive_filter.h"
    #include "flight/imu.h"
    #include "flight/kalman.h"
    #include "flight/mixer.h"
    #include "flight/mixer_profile.h"
    #include "flight/pid.h"
    #include "flight/smith_predictor.h"
    #include "navigation/navigation.h"
    #include "programming/logic_condition.h"
    #include "rx/rx.h"
    #include "sensors/battery.h"
    #include "sensors/gyro.h"
    #include "sensors/pitotmeter.h"
}

// Test-controlled inputs
static float g_airspeedCmS = 0.0f;
static bool g_pitotValid = true;

extern "C" {
    // Globals normally provided by other translation units
    uint32_t stateFlags = 0;
    uint32_t flightModeFlags = 0;
    uint32_t armingFlags = 0;
    int16_t rcCommand[4] = {};
    int32_t debug[DEBUG32_VALUE_COUNT] = {};
    uint8_t debugMode = 0;
    attitudeEulerAngles_t attitude = {};
    fpVector3_t HeadVecEFFiltered = {};
    gyro_t gyro = {};
    const controlConfig_t *currentControlProfile = nullptr;
    const batteryProfile_t *currentBatteryProfile = nullptr;
    mixerConfig_t currentMixerConfig;
    bool isMixerTransitionMixing = false;
    navConfig_t navConfig_System;
    navConfig_t navConfig_Copy;

    // Airspeed source: the one thing this test actually drives
    float getAirspeedEstimate(void) { return g_airspeedCmS; }
    bool pitotGetValidForAirspeed(void) { return g_pitotValid; }
    bool pitotIsHealthy(void) { return g_pitotValid; }

    // Everything below is only referenced from code paths this test never runs
    void adaptiveFilterPushRate(const flight_dynamics_index_t, const float, const uint8_t) {}
    float applySmithPredictor(uint8_t, smithPredictor_t *, float sample) { return sample; }
    void smithPredictorInit(smithPredictor_t *, float, float, uint16_t, uint32_t) {}
    bool areSticksDeflected(void) { return false; }
    void autotuneFixedWingUpdate(const flight_dynamics_index_t, float, float, float) {}
    float calculateCosTiltAngle(void) { return 1.0f; }
    rollPitchStatus_e calculateRollPitchCenterStatus(void) { return CENTERED; }
    float getEstimatedActualVelocity(int) { return 0.0f; }
    float getFlightAxisAngleOverride(uint8_t, float angle) { return angle; }
    float getFlightAxisRateOverride(uint8_t, float rate) { return rate; }
    bool isFlightAxisAngleOverrideActive(uint8_t) { return false; }
    int16_t getRcCommandOverride(int16_t command[], uint8_t axis) { return command[axis]; }
    uint32_t getLooptime(void) { return 1000; }
    uint16_t getMaxThrottle(void) { return 2000; }
    int getThrottleIdleValue(void) { return 1150; }
    float getMotorMixRange(void) { return 0.0f; }
    bool mixerIsOutputSaturated(void) { return false; }
    int32_t getRcStickDeflection(int32_t) { return 0; }
    void gyroKalmanUpdateSetpoint(uint8_t, float) {}
    void imuTransformVectorEarthToBody(fpVector3_t *) {}
    bool isAdjustingPosition(void) { return false; }
    bool IS_RC_MODE_ACTIVE(boxId_e) { return false; }
    bool isFwAutoModeActive(boxId_e) { return false; }
    timeMs_t millis(void) { return 0; }
    int8_t navCheckActiveAngleHoldAxis(void) { return 0; }
    int8_t navigationGetHeadingControlState(void) { return 0; }
    bool navigationIsControllingAltitude(void) { return false; }
    bool navigationIsControllingThrottle(void) { return false; }
    float navPidApply3(pidController_t *, const float, const float, const float, const float, const float, const pidControllerFlags_e, const float, const float) { return 0.0f; }
    void navPidInit(pidController_t *, float, float, float, float, float, float) {}
    void navPidReset(pidController_t *) {}
    int16_t rxGetChannelValue(unsigned) { return 1500; }  // sticks centred
    bool sensors(uint32_t) { return false; }

    // Defined by PG_REGISTER_PROFILE_WITH_RESET_TEMPLATE in pid.c
    extern pidProfile_t pidProfile_Storage[];
    extern const pidProfile_t pgResetTemplate_pidProfile;

    // Provided by pid_fw_iterm_lock_fade_access.c (wraps the real flight/pid.c)
    void testPidFwSetAxis(int axis, float kP, float kI, float kFF, float iTerm, float gyroRate, float rateTarget);
    void testPidFwStep(int axis, float dT);
    float testPidFwGetIterm(int axis);
    float testPidFwGetAttenuationI(int axis);
    int16_t testPidFwGetOutput(int axis);
}

namespace {

// The trainer's pitch loop: P 10, I 10, FF 85 (fw_p/i/ff_pitch), pitch_rate 12 = 120 deg/s, lock at its defaults
constexpr float KP = 10.0f / FP_PID_RATE_P_MULTIPLIER;
constexpr float KI = 10.0f / FP_PID_RATE_I_MULTIPLIER;
constexpr float KFF = 85.0f / FP_PID_RATE_FF_MULTIPLIER;
constexpr float FULL_PULL = -111.0f;      // deg/s, the stick at its stop
constexpr float DT = 0.001f;

class PidFwItermLockFadeTest : public ::testing::Test {
protected:
    controlConfig_t controlCfg;

    void SetUp() override
    {
        memset(&controlCfg, 0, sizeof(controlCfg));
        controlCfg.stabilized.rates[FD_PITCH] = 12;
        currentControlProfile = &controlCfg;
        pidProfile_Storage[0] = pgResetTemplate_pidProfile;
        pidProfile_ProfileCurrent = &pidProfile_Storage[0];
        flightModeFlags = 0;
    }

    void run(float seconds, int axis = FD_PITCH)
    {
        for (int i = 0; i < (int)(seconds / DT + 0.5f); i++) {
            testPidFwStep(axis, DT);
        }
    }
};

// The crash case: +87 of I-term against a full pull. Held, it left -28 of output; fading, the feed-forward comes back
TEST_F(PidFwItermLockFadeTest, OpposingItermFadesWhileTheStickIsOut)
{
    testPidFwSetAxis(FD_PITCH, KP, KI, KFF, 87.0f, -20.0f, FULL_PULL);
    testPidFwStep(FD_PITCH, DT);
    ASSERT_NEAR(0.0f, testPidFwGetAttenuationI(FD_PITCH), 1e-3f);   // the lock holds the I-term at a full stick
    EXPECT_GT(testPidFwGetOutput(FD_PITCH), -250);                    // most of the feed-forward is eaten at first

    run(0.5f);
    EXPECT_NEAR(87.0f * expf(-1.0f), testPidFwGetIterm(FD_PITCH), 3.0f);
    run(0.5f);
    EXPECT_NEAR(87.0f * expf(-2.0f), testPidFwGetIterm(FD_PITCH), 3.0f);
    EXPECT_LT(testPidFwGetOutput(FD_PITCH), -270);                    // FF -304 plus what is left of the I-term
}

// A trim that helps the pull (same sign as the demand) is what the lock is meant to keep: untouched
TEST_F(PidFwItermLockFadeTest, HelpingItermIsKept)
{
    testPidFwSetAxis(FD_PITCH, KP, KI, KFF, -87.0f, -20.0f, FULL_PULL);
    run(1.0f);
    EXPECT_NEAR(-87.0f, testPidFwGetIterm(FD_PITCH), 0.5f);
}

// With the stick centred nothing fades, whatever the I-term holds
TEST_F(PidFwItermLockFadeTest, NothingFadesAtACentredStick)
{
    testPidFwSetAxis(FD_PITCH, KP, KI, KFF, 87.0f, 0.0f, 0.0f);
    run(1.0f);
    EXPECT_FLOAT_EQ(87.0f, testPidFwGetIterm(FD_PITCH));
}

// A mild stick attenuates the I-term only in part, and the fade follows the attenuation
TEST_F(PidFwItermLockFadeTest, MildStickFadesSlower)
{
    // 36 deg/s is 30 % of the maximum rate: the lock's Gaussian leaves the I-term about a fifth alive
    testPidFwSetAxis(FD_PITCH, KP, KI, KFF, 87.0f, -36.0f, -36.0f);
    testPidFwStep(FD_PITCH, DT);
    const float aI = testPidFwGetAttenuationI(FD_PITCH);
    ASSERT_GT(aI, 0.1f);
    ASSERT_LT(aI, 0.4f);
    run(0.5f);
    const float expected = 87.0f * expf(-(1.0f - aI));
    EXPECT_NEAR(expected, testPidFwGetIterm(FD_PITCH), 3.0f);
    EXPECT_GT(testPidFwGetIterm(FD_PITCH), 87.0f * expf(-1.0f) + 3.0f);
}

// A small correction (under 20 % of the maximum rate, where the lock does not count the stick as big) keeps the trim
TEST_F(PidFwItermLockFadeTest, SmallCorrectionKeepsTheTrim)
{
    testPidFwSetAxis(FD_PITCH, KP, KI, KFF, 87.0f, -18.0f, -18.0f);
    run(2.0f);
    EXPECT_FLOAT_EQ(87.0f, testPidFwGetIterm(FD_PITCH));
}

// The yaw axis is left out: in a banked turn its rate target is the turn's and its I-term is the rudder trim
TEST_F(PidFwItermLockFadeTest, YawTrimIsKeptInATurn)
{
    controlCfg.stabilized.rates[FD_YAW] = 3;
    testPidFwSetAxis(FD_YAW, KP, KI, KFF, 40.0f, -25.0f, -25.0f);
    run(2.0f, FD_YAW);
    EXPECT_FLOAT_EQ(40.0f, testPidFwGetIterm(FD_YAW));
}

}  // namespace
