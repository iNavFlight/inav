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

// Links the real flight/pid.c pidTurnAssistant(); see pid_turn_assist_pitch_access.c.

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
    #include "common/quaternion.h"
}

// Test-controlled inputs

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
    float getAirspeedEstimate(void) { return 0.0f; }
    bool pitotGetValidForAirspeed(void) { return false; }
    bool pitotIsHealthy(void) { return false; }

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

    // Provided by pid_turn_assist_pitch_access.c (wraps the real flight/pid.c)
    void testPidTurnAssistantReset(void);
    void testPidTurnAssistantCall(float bankRad);
    float testPidRateTarget(int axis);

    // imuTransformVectorEarthToBody(): imu.c cannot be linked here (it drags in
    // gyro/acc/baro/GPS), so this reproduces ONLY its two-line body, driven by
    // a quaternion built from the same roll/pitch the test sets in `attitude`.
    fpQuaternion_t orientation;
    void imuTransformVectorEarthToBody(fpVector3_t *v)
    {
        v->y = -v->y;   // NED -> NEU, as in imu.c
        quaternionRotateVector(v, v, &orientation);
    }
}


namespace {

constexpr float kRefAirspeedCmS = 1500.0f;   // fw_reference_airspeed
constexpr float kG = 980.665f;

// Set the IMU attitude (roll/pitch only, yaw 0) the same way imu.c builds its
// quaternion from RPY.
void setAttitude(float rollDeg, float pitchDeg)
{
    const float r = DEGREES_TO_RADIANS(rollDeg) * 0.5f, p = DEGREES_TO_RADIANS(pitchDeg) * 0.5f;
    const float cr = cosf(r), sr = sinf(r), cp = cosf(p), sp = sinf(p);
    orientation.q0 = cr * cp;
    orientation.q1 = sr * cp;
    orientation.q2 = cr * sp;
    orientation.q3 = -sr * sp;
}

class PidTurnAssistPitchTest : public ::testing::Test {
protected:
    controlConfig_t controlCfg;

    void SetUp() override
    {
        memset(&controlCfg, 0, sizeof(controlCfg));
        for (int i = 0; i < 3; i++) controlCfg.stabilized.rates[i] = 70;   // clamp = 700 deg/s
        currentControlProfile = &controlCfg;
        stateFlags = AIRPLANE;
        pidProfile_Storage[0] = pgResetTemplate_pidProfile;
        pidProfile_ProfileCurrent = &pidProfile_Storage[0];
        pidProfileMutable()->fixedWingReferenceAirspeed = (uint16_t)kRefAirspeedCmS;
        pidProfileMutable()->fixedWingCoordinatedYawGain = 1.0f;
        pidProfileMutable()->fixedWingCoordinatedPitchGain = 1.0f;
        testPidTurnAssistantReset();
    }

    // Expected body-frame rates: earth-frame coordinated-turn yaw rate
    // g*tan(-bank)/V (no pitch factor) rotated by the attitude.
    void expectedBody(float bankDeg, float pitchDeg, fpVector3_t *out)
    {
        const float yawEarthDegS = RADIANS_TO_DEGREES(kG * tanf(-DEGREES_TO_RADIANS(bankDeg)) / kRefAirspeedCmS);
        out->x = 0; out->y = 0; out->z = yawEarthDegS;
        setAttitude(bankDeg, pitchDeg);
        imuTransformVectorEarthToBody(out);
    }

    void runCase(float bankDeg, float pitchDeg)
    {
        fpVector3_t exp;
        expectedBody(bankDeg, pitchDeg, &exp);
        testPidTurnAssistantCall(DEGREES_TO_RADIANS(bankDeg));
        const float tolPct = 0.5f;   // approx-trig tolerance, far below the ~6% bug at 20 deg
        EXPECT_NEAR(exp.z, testPidRateTarget(FD_YAW), fabsf(exp.z) * tolPct / 100.0f + 0.01f)
            << "yaw: bank " << bankDeg << " pitch " << pitchDeg;
        EXPECT_NEAR(exp.y, testPidRateTarget(FD_PITCH), fabsf(exp.y) * tolPct / 100.0f + 0.01f)
            << "pitch: bank " << bankDeg << " pitch " << pitchDeg;
        EXPECT_NEAR(exp.x, testPidRateTarget(FD_ROLL), fabsf(exp.x) * tolPct / 100.0f + 0.01f)
            << "roll: bank " << bankDeg << " pitch " << pitchDeg;
    }
};

TEST_F(PidTurnAssistPitchTest, LevelPitchMatchesCoordinatedTurn)
{
    runCase(30.0f, 0.0f);
}

TEST_F(PidTurnAssistPitchTest, Pitch20MatchesCoordinatedTurn)
{
    runCase(30.0f, 20.0f);
}

TEST_F(PidTurnAssistPitchTest, Pitch30MatchesCoordinatedTurn)
{
    runCase(30.0f, 30.0f);
}

TEST_F(PidTurnAssistPitchTest, NegativePitchSymmetric)
{
    runCase(30.0f, -20.0f);
}

TEST_F(PidTurnAssistPitchTest, NegativeBank)
{
    runCase(-30.0f, 20.0f);
}

} // namespace
