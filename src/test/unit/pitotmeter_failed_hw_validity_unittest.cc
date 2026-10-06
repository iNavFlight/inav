/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Validity of a HARDWARE pitot that has been declared failed.
 *
 * Real sensors/pitotmeter.c is linked and driven through pitotUpdate() with a
 * fake pitot device. While armed, implausible readings (vs. GPS-derived virtual
 * airspeed) latch pitotHardwareFailed. Once failed, getAirspeedEstimate() falls
 * back to the virtual estimate, so pitotGetValidForAirspeed() must be true only
 * when GPS_FIX && validVelNE && validVelD && virtual airspeed > 0.
 */

#include <cstdint>
#include <cstring>
#include <cmath>

#include "gtest/gtest.h"

extern "C" {
    #include "platform.h"

    #include "common/axis.h"
    #include "common/maths.h"
    #include "common/time.h"
    #include "config/parameter_group.h"
    #include "config/parameter_group_ids.h"
    #include "drivers/pitotmeter/pitotmeter.h"
    #include "fc/runtime_config.h"
    #include "flight/pid.h"
    #include "flight/wind_estimator.h"
    #include "io/gps.h"
    #include "sensors/barometer.h"
    #include "navigation/navigation.h"
    #define _Static_assert static_assert
    #include "navigation/navigation_private.h"
    #undef _Static_assert
    #include "sensors/pitotmeter.h"
    #include "sensors/sensors.h"

    gpsSolutionData_t gpsSol;
    uint8_t detectedSensors[SENSOR_INDEX_COUNT] = { 0 };
    uint32_t stateFlags = 0;
    uint32_t armingFlags = 0;
    baro_t baro;
    navigationPosControl_t posControl;

    static bool windValid = false;
    static timeMs_t fakeMillis = 100000;
    static float fakePressure = 0.0f;   // Pa, fed to pitot.calculate()

    pidProfile_t pidProfile_Instance;
    pidProfile_t *pidProfile_ProfileCurrent = &pidProfile_Instance;
    uint8_t requestedSensors[SENSOR_INDEX_COUNT] = { 0 };
    simulatorData_t simulatorData;
    bool ms4525Detect(pitotDev_t *) { return false; }
    bool ms5525Detect(pitotDev_t *) { return false; }
    bool dlvrDetect(pitotDev_t *) { return false; }
    bool mspPitotmeterDetect(struct pitotDev_s *) { return false; }
    float _logf(float x) { return logf(x); }
    bool feature(uint32_t) { return false; }
    void sensorsSet(uint32_t) {}
    void sensorsClear(uint32_t) {}
    float getEstimatedWindSpeed(int) { return 0.0f; }
    void imuTransformVectorEarthToBody(fpVector3_t *) {}

    timeMs_t millis(void) { return fakeMillis; }
    timeUs_t micros(void) { return (timeUs_t)fakeMillis * 1000; }
    bool isEstimatedWindSpeedValid(void) { return windValid; }

    static bool fakeStartGet(pitotDev_t *) { return true; }
    static void fakeCalculate(pitotDev_t *, float *pressure, float *temperature) {
        *pressure = fakePressure;
        *temperature = 293.15f;
    }
}

// Pressure (Pa) producing a given IAS in cm/s with pitot_scale == 1
static float pressureForCmS(float cmS) {
    const float v = cmS / 100.0f;
    return 0.5f * 1.225f * v * v;
}

class FailedHardwarePitotValidity : public ::testing::Test {
protected:
    void tick() { fakeMillis += 20; pitotUpdate(); }

    void SetUp() override {
        memset(&gpsSol, 0, sizeof(gpsSol));
        memset(&posControl, 0, sizeof(posControl));
        memset(&pidProfile_Instance, 0, sizeof(pidProfile_Instance));
        detectedSensors[SENSOR_INDEX_PITOT] = PITOT_MS4525;
        stateFlags = 0;
        armingFlags = 0;
        windValid = false;
        fakePressure = 0.0f;
        pitotmeterConfigMutable()->pitot_scale = 1.0f;
        pitotmeterConfigMutable()->pitot_lpf_milli_hz = 0;

        memset(&pitot, 0, sizeof(pitot));
        pitot.dev.delay = 0;
        pitot.dev.calibThreshold = 1.0f;
        pitot.dev.start = fakeStartGet;
        pitot.dev.get = fakeStartGet;
        pitot.dev.calculate = fakeCalculate;

        // Zero calibration (disarmed). Also clears any failure latched by a prior test.
        pitotStartCalibration();
        for (int i = 0; i < 400 && !pitotIsCalibrationComplete(); i++) tick();
        ASSERT_TRUE(pitotIsCalibrationComplete());
        tick();
        ASSERT_FALSE(pitotHasFailed());

        ENABLE_STATE(GPS_FIX);
        gpsSol.flags.validVelNE = true;
        gpsSol.flags.validVelD = true;
        posControl.actualState.vel3D = 2000.0f;   // 72 km/h ground speed (wind estimate invalid)
    }

    // Arm and feed a pitot reading far below 30% of virtual airspeed until declared failed.
    void armAndFailPitot() {
        ENABLE_ARMING_FLAG(ARMED);
        fakePressure = pressureForCmS(300.0f);
        for (int i = 0; i < 20 && !pitotHasFailed(); i++) tick();
        ASSERT_TRUE(pitotHasFailed());
    }
};

TEST_F(FailedHardwarePitotValidity, FailedPitotValidWhenGpsVelocitiesValid) {
    armAndFailPitot();
    tick();
    EXPECT_TRUE(pitotGetValidForAirspeed());
}

TEST_F(FailedHardwarePitotValidity, FailedPitotNotValidWhenHorizontalVelocityInvalid) {
    armAndFailPitot();
    gpsSol.flags.validVelNE = false;
    tick();
    ASSERT_TRUE(pitotHasFailed());
    EXPECT_FALSE(pitotGetValidForAirspeed());
}

TEST_F(FailedHardwarePitotValidity, FailedPitotNotValidWhenVerticalVelocityInvalid) {
    armAndFailPitot();
    gpsSol.flags.validVelD = false;
    tick();
    ASSERT_TRUE(pitotHasFailed());
    EXPECT_FALSE(pitotGetValidForAirspeed());
}

TEST_F(FailedHardwarePitotValidity, FailedPitotNotValidWhenVirtualAirspeedZero) {
    armAndFailPitot();
    posControl.actualState.vel3D = 0.0f;   // wind estimate invalid -> virtual airspeed 0
    tick();
    ASSERT_TRUE(pitotHasFailed());
    EXPECT_FALSE(pitotGetValidForAirspeed());
}

TEST_F(FailedHardwarePitotValidity, FailedPitotNotValidWithoutGpsFix) {
    armAndFailPitot();
    DISABLE_STATE(GPS_FIX);
    tick();
    ASSERT_TRUE(pitotHasFailed());
    EXPECT_FALSE(pitotGetValidForAirspeed());
}

TEST_F(FailedHardwarePitotValidity, HealthyPitotValidRegardlessOfGpsVelocityFlags) {
    ENABLE_ARMING_FLAG(ARMED);
    fakePressure = pressureForCmS(2000.0f);   // plausible vs 2000 cm/s virtual
    for (int i = 0; i < 20; i++) tick();
    ASSERT_FALSE(pitotHasFailed());
    gpsSol.flags.validVelNE = false;
    gpsSol.flags.validVelD = false;
    tick();
    EXPECT_TRUE(pitotGetValidForAirspeed());
}
