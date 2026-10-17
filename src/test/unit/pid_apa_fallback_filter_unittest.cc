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

// Reproduces a bug in flight/pid.c updatePIDCoefficients(): the fixed-wing
// throttle-TPA low-pass filter (fixedWingTpaFilter, tau = fw_tpa_time_constant)
// is only fed inside the throttle-TPA branch. While airspeed-based attenuation
// (APA) is active the filter state is frozen, so when APA falls back to
// throttle TPA (pitot lost) the filtered throttle is stale and needs ~tau to
// converge, giving a wrong tpaFactor for that time.
//
// Links the real flight/pid.c; only unrelated dependencies are stubbed.

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

    // Provided by pid_apa_fallback_filter_access.c (wraps the real flight/pid.c)
    void testPidSelectPiff(void);
    float testPidGetKP(int axis);
    float testPidGetKI(int axis);
    void testPidSetArmed(void);
}

namespace {

constexpr int IDLE = 1150;       // matches getThrottleIdleValue() stub
constexpr int BREAKPOINT = 1500;
constexpr int DYNPID = 50;
constexpr int TAU_MS = 2000;
constexpr float REF_AIRSPEED = 1500.0f;
constexpr float kTol = 3e-3f;

class PidApaFallbackFilterTest : public ::testing::Test {
protected:
    controlConfig_t controlCfg;

    void SetUp() override
    {
        memset(&controlCfg, 0, sizeof(controlCfg));
        controlCfg.throttle.apa_pow = 150;
        controlCfg.throttle.dynPID = DYNPID;
        controlCfg.throttle.tpa_breakpoint = BREAKPOINT;
        controlCfg.throttle.fixedWingTauMs = TAU_MS;
        controlCfg.throttle.tpa_pitch_compensation = 0;
        currentControlProfile = &controlCfg;
        g_pitotValid = false;
        g_airspeedCmS = 1500.0f;
        flightModeFlags = 0;
        armingFlags = 0;
        rcCommand[THROTTLE] = IDLE;

        pidProfile_Storage[0] = pgResetTemplate_pidProfile;
        pidProfile_ProfileCurrent = &pidProfile_Storage[0];
        pidProfileMutable()->fixedWingReferenceAirspeed = (uint16_t)REF_AIRSPEED;

        testPidSelectPiff();
        testPidSetArmed();
        pidResetTPAFilter();   // real production reset of the filter
        schedulePidGainsUpdate();
    }

    float tpaForThrottle(float throttle) const
    {
        float t = 0.5f + 0.5f * ((BREAKPOINT - IDLE) / (throttle - IDLE));
        t = 1.0f + (t - 1.0f) * (0.01f * DYNPID);
        return fminf(fmaxf(t, 0.3f), 2.0f);
    }
    float kPForTpa(float tpa) const
    {
        return pidBank()->pid[FD_ROLL].P / FP_PID_RATE_P_MULTIPLIER * tpa;
    }
    void runCycles(int n, int throttle, bool pitot)
    {
        g_pitotValid = pitot;
        rcCommand[THROTTLE] = throttle;
        for (int i = 0; i < n; i++) updatePIDCoefficients();
    }
};

// The bug: filter state is stale (throttle A) after an APA phase during which
// throttle moved to B; fallback must use throttle B, not the stale filtered A.
TEST_F(PidApaFallbackFilterTest, FallbackFromApaUsesCurrentThrottleNotStaleFilter)
{
    const int A = 1300, B = 1900;
    ASSERT_NE(tpaForThrottle(A), tpaForThrottle(B));

    runCycles(5000, A, false);                     // settle filter at A
    // tolerance covers pid.c truncating the filtered throttle to uint16_t
    ASSERT_NEAR(kPForTpa(tpaForThrottle(A)), testPidGetKP(FD_ROLL), kTol);

    runCycles(2000, B, true);                      // APA active, throttle moved to B

    runCycles(1, B, false);                        // fallback, ONE update
    EXPECT_NEAR(kPForTpa(tpaForThrottle(B)), testPidGetKP(FD_ROLL), kTol)
        << "kP " << testPidGetKP(FD_ROLL) << " reflects stale throttle A (expected "
        << kPForTpa(tpaForThrottle(A)) << "); wanted throttle-B value "
        << kPForTpa(tpaForThrottle(B));
}

// Negative: staying on throttle TPA must still filter (step is smoothed).
TEST_F(PidApaFallbackFilterTest, ThrottleTpaStillFiltersStepChange)
{
    const int A = 1300, B = 1900;
    runCycles(5000, A, false);
    const float kP_A = kPForTpa(tpaForThrottle(A));
    const float kP_B = kPForTpa(tpaForThrottle(B));
    ASSERT_NE(kP_A, kP_B);

    runCycles(1, B, false);
    const float kP1 = testPidGetKP(FD_ROLL);
    // after one cycle: moved toward B but nowhere near it (not an instant step)
    EXPECT_LT(kP1, kP_A);
    EXPECT_GT(kP1, kP_B);
    EXPECT_LT(fabsf(kP1 - kP_A), 0.5f * fabsf(kP_B - kP_A));

    runCycles(5000, B, false);                     // converges eventually
    EXPECT_NEAR(kP_B, testPidGetKP(FD_ROLL), kTol);
}

} // namespace
