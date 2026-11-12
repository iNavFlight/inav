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

// Reproduces a bug in flight/pid.c updatePIDCoefficients(): with fixed-wing
// airspeed-based PID attenuation (APA, apa_pow > 100) the I-term uses its own
// factor, pow(ref/airspeed, apa_pow/100 - 1) clamped to [0.3, 1.5], while P/D/FF
// use tpaFactor = pow(ref/airspeed, apa_pow/100) clamped to [0.3, 2.0]. The
// "gains need recompute" decision only watches tpaFactor, so once tpaFactor is
// pinned at its clamp while airspeed (and therefore iTermFactor) keeps moving,
// pidState[axis].kI is never recomputed and stops tracking airspeed.
//
// Links the real flight/pid.c and drives updatePIDCoefficients(); only the
// airspeed source (getAirspeedEstimate / pitotGetValidForAirspeed) and the
// unrelated dependencies of the rest of pid.c are stubbed.

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

    // Provided by pid_apa_iterm_update_access.c (wraps the real flight/pid.c)
    void testPidSelectPiff(void);
    float testPidGetKP(int axis);
    float testPidGetKI(int axis);
}

namespace {

constexpr float REF_AIRSPEED = 1500.0f;  // cm/s
constexpr int APA_POW = 150;             // tpa exponent 1.5, iTerm exponent 0.5

class PidApaItermTest : public ::testing::Test {
protected:
    controlConfig_t controlCfg;

    void SetUp() override
    {
        memset(&controlCfg, 0, sizeof(controlCfg));
        controlCfg.throttle.apa_pow = APA_POW;
        currentControlProfile = &controlCfg;
        g_pitotValid = true;

        // Real PG reset gives realistic defaults for the PID bank (P/I/D/FF)
        pidProfile_Storage[0] = pgResetTemplate_pidProfile;
        pidProfile_ProfileCurrent = &pidProfile_Storage[0];
        pidProfileMutable()->fixedWingReferenceAirspeed = (uint16_t)REF_AIRSPEED;

        testPidSelectPiff();
        // Force a first computation, then let the change-detection logic run
        schedulePidGainsUpdate();
    }

    // Uses powf_approx() like pid.c does, so expectations match its precision
    float expectedTpa(float airspeed) const
    {
        return fminf(fmaxf(powf_approx(REF_AIRSPEED / airspeed, APA_POW / 100.0f), 0.3f), 2.0f);
    }
    float expectedITerm(float airspeed) const
    {
        return fminf(fmaxf(powf_approx(REF_AIRSPEED / airspeed, APA_POW / 100.0f - 1.0f), 0.3f), 1.5f);
    }
    float expectedKI(float airspeed) const
    {
        return pidBank()->pid[FD_ROLL].I / FP_PID_RATE_I_MULTIPLIER * expectedITerm(airspeed);
    }
    float expectedKP(float airspeed) const
    {
        return pidBank()->pid[FD_ROLL].P / FP_PID_RATE_P_MULTIPLIER * expectedTpa(airspeed);
    }
};

// Sanity: both factors tracked correctly while tpaFactor is NOT clamped
TEST_F(PidApaItermTest, KiAndKpTrackAirspeedWhileTpaUnclamped)
{
    g_airspeedCmS = 1600.0f;
    updatePIDCoefficients();
    ASSERT_NEAR(expectedKP(1600.0f), testPidGetKP(FD_ROLL), 1e-4f);
    ASSERT_NEAR(expectedKI(1600.0f), testPidGetKI(FD_ROLL), 1e-4f);

    g_airspeedCmS = 1800.0f;
    updatePIDCoefficients();
    EXPECT_NEAR(expectedKP(1800.0f), testPidGetKP(FD_ROLL), 1e-4f);
    EXPECT_NEAR(expectedKI(1800.0f), testPidGetKI(FD_ROLL), 1e-4f);
}

// The bug: tpaFactor pinned at 0.3 (high airspeed), iTermFactor still changing
TEST_F(PidApaItermTest, KiTracksAirspeedWhenTpaClampedLow)
{
    // 1500/3000 = 0.5: tpa = 0.5^1.5 = 0.354 (not clamped); pick faster to clamp.
    // 4500 cm/s: tpa = (1/3)^1.5 = 0.192 -> clamped 0.3; iTerm = (1/3)^0.5 = 0.577 (unclamped)
    g_airspeedCmS = 4500.0f;
    ASSERT_FLOAT_EQ(0.3f, expectedTpa(g_airspeedCmS));
    ASSERT_GT(expectedITerm(g_airspeedCmS), 0.3f);
    updatePIDCoefficients();
    const float kP_before = testPidGetKP(FD_ROLL);
    const float kI_before = testPidGetKI(FD_ROLL);
    ASSERT_NEAR(expectedKP(4500.0f), kP_before, 1e-4f);
    ASSERT_NEAR(expectedKI(4500.0f), kI_before, 1e-4f);

    // 6000 cm/s: tpa still clamped at 0.3 (unchanged), iTerm = 0.25^0.5 = 0.5 -> changed
    g_airspeedCmS = 6000.0f;
    ASSERT_FLOAT_EQ(0.3f, expectedTpa(g_airspeedCmS));
    ASSERT_GT(expectedITerm(g_airspeedCmS), 0.3f);
    ASSERT_NE(expectedITerm(4500.0f), expectedITerm(6000.0f));
    updatePIDCoefficients();

    // kP depends only on tpaFactor, which did not change
    EXPECT_FLOAT_EQ(kP_before, testPidGetKP(FD_ROLL));
    // kI must follow the new iTermFactor
    EXPECT_NEAR(expectedKI(6000.0f), testPidGetKI(FD_ROLL), 1e-4f)
        << "kI stale: still " << testPidGetKI(FD_ROLL) << " (old value " << kI_before
        << "); gains were not recomputed although iTermFactor changed";
}

// Same bug at the other clamp: tpaFactor pinned at 2.0 (low airspeed)
TEST_F(PidApaItermTest, KiTracksAirspeedWhenTpaClampedHigh)
{
    // 833 cm/s (ratio 1.8): tpa = 2.4 -> clamped 2.0; iTerm = 1.34 (unclamped)
    g_airspeedCmS = 833.0f;
    ASSERT_FLOAT_EQ(2.0f, expectedTpa(g_airspeedCmS));
    ASSERT_LT(expectedITerm(g_airspeedCmS), 1.5f);
    updatePIDCoefficients();
    const float kP_before = testPidGetKP(FD_ROLL);

    // 714 cm/s (ratio 2.1): tpa = 3.0 -> still clamped 2.0; iTerm = 1.45 (changed)
    g_airspeedCmS = 714.0f;
    ASSERT_FLOAT_EQ(2.0f, expectedTpa(g_airspeedCmS));
    ASSERT_NE(expectedITerm(833.0f), expectedITerm(714.0f));
    updatePIDCoefficients();

    EXPECT_FLOAT_EQ(kP_before, testPidGetKP(FD_ROLL));
    EXPECT_NEAR(expectedKI(714.0f), testPidGetKI(FD_ROLL), 1e-4f);
}

// Negative: nothing changed at all -> coefficients stay put
TEST_F(PidApaItermTest, NoChangeInAirspeedLeavesGainsUntouched)
{
    g_airspeedCmS = 6000.0f;
    updatePIDCoefficients();
    const float kP = testPidGetKP(FD_ROLL);
    const float kI = testPidGetKI(FD_ROLL);
    updatePIDCoefficients();
    EXPECT_FLOAT_EQ(kP, testPidGetKP(FD_ROLL));
    EXPECT_FLOAT_EQ(kI, testPidGetKI(FD_ROLL));
}

} // namespace
