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
 * Virtual pitot airspeed validity must track CURRENT GPS velocity validity.
 *
 * The real sensors/pitotmeter.c is linked. pitotUpdate() refreshes the cache
 * returned by pitotGetValidForAirspeed(). For PITOT_VIRTUAL the validity must
 * require gpsSol.flags.validVelNE && validVelD (the same gate wind_estimator.c
 * uses before updating the wind estimate); otherwise losing GPS velocity while
 * the (stale) wind estimate is still valid keeps airspeed-based PID attenuation
 * active instead of falling back to throttle TPA.
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
    // navigation_private.h uses C11 _Static_assert; map for C++
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

    static bool windValid = true;
    static timeMs_t fakeMillis = 100000;

    // Stubs for the rest of pitotmeter.c's link surface (unreachable for the virtual pitot path)
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
}

class VirtualPitotValidity : public ::testing::Test {
protected:
    void SetUp() override {
        memset(&gpsSol, 0, sizeof(gpsSol));
        memset(&posControl, 0, sizeof(posControl));
        detectedSensors[SENSOR_INDEX_PITOT] = PITOT_VIRTUAL;
        stateFlags = 0;
        armingFlags = 0;
        ENABLE_STATE(GPS_FIX);
        windValid = true;
        gpsSol.flags.validVelNE = true;
        gpsSol.flags.validVelD = true;
    }

    bool airspeedValid() {
        pitotUpdate();   // virtual pitot: sets lastSeenHealthyMs, refreshes validity cache
        return pitotGetValidForAirspeed();
    }
};

TEST_F(VirtualPitotValidity, ValidWhenGpsVelocityValid) {
    EXPECT_TRUE(airspeedValid());
}

TEST_F(VirtualPitotValidity, NotValidWithoutGpsFix) {
    DISABLE_STATE(GPS_FIX);
    EXPECT_FALSE(airspeedValid());
}

TEST_F(VirtualPitotValidity, NotValidWithoutWindEstimate) {
    windValid = false;
    EXPECT_FALSE(airspeedValid());
}

TEST_F(VirtualPitotValidity, NotValidWhenHorizontalVelocityLost) {
    ASSERT_TRUE(airspeedValid());
    gpsSol.flags.validVelNE = false;
    EXPECT_FALSE(airspeedValid());
}

TEST_F(VirtualPitotValidity, NotValidWhenVerticalVelocityLost) {
    ASSERT_TRUE(airspeedValid());
    gpsSol.flags.validVelD = false;
    EXPECT_FALSE(airspeedValid());
}

TEST_F(VirtualPitotValidity, RecoversWhenVelocityReturns) {
    gpsSol.flags.validVelNE = false;
    ASSERT_FALSE(airspeedValid());
    gpsSol.flags.validVelNE = true;
    EXPECT_TRUE(airspeedValid());
}
