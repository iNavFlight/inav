/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include <limits.h>
#include <math.h>

extern "C" {
    #include "sensors/gyro.h"
    #include "sensors/compass.h"
    #include "sensors/acceleration.h"

    #include "scheduler/scheduler.h"
    #include "fc/runtime_config.h"

    #include "io/gps.h"

    #include "flight/imu.h"
}

#include "unittest_macros.h"
#include "gtest/gtest.h"

extern "C" { 
STATIC_UNIT_TESTED void imuUpdateEulerAngles(void);
STATIC_UNIT_TESTED void imuComputeQuaternionFromRPY(int16_t initialRoll, int16_t initialPitch, int16_t initialYaw);
STATIC_UNIT_TESTED void imuMahonyAHRSupdate(float dt, const fpVector3_t * gyroBF, const fpVector3_t * accBF, const fpVector3_t * magBF, bool useCOG, float courseOverGround, float accWScaler, float magWScaler, const fpVector3_t * velEF, bool velEFNew);
}

TEST(FlightImuTest, TestEulerAngleCalculation)
{
    imuComputeQuaternionFromRPY(0, 0, 0);
    imuUpdateEulerAngles();
    EXPECT_NEAR(attitude.values.roll, 0, 1);
    EXPECT_NEAR(attitude.values.pitch, 0, 1);
    EXPECT_NEAR(attitude.values.yaw, 0, 1);

    imuComputeQuaternionFromRPY(450, 450, 0);
    imuUpdateEulerAngles();
    EXPECT_NEAR(attitude.values.roll, 450, 1);
    EXPECT_NEAR(attitude.values.pitch, 450, 1);
    EXPECT_NEAR(attitude.values.yaw, 0, 1);

    imuComputeQuaternionFromRPY(-450, -450, 0);
    imuUpdateEulerAngles();
    EXPECT_NEAR(attitude.values.roll, -450, 1);
    EXPECT_NEAR(attitude.values.pitch, -450, 1);
    EXPECT_NEAR(attitude.values.yaw, 0, 1);

    imuComputeQuaternionFromRPY(1790, 0, 0);
    imuUpdateEulerAngles();
    EXPECT_NEAR(attitude.values.roll, 1790, 1);
    EXPECT_NEAR(attitude.values.pitch, 0, 1);
    EXPECT_NEAR(attitude.values.yaw, 0, 1);

    imuComputeQuaternionFromRPY(-1790, 0, 0);
    imuUpdateEulerAngles();
    EXPECT_NEAR(attitude.values.roll, -1790, 1);
    EXPECT_NEAR(attitude.values.pitch, 0, 1);
    EXPECT_NEAR(attitude.values.yaw, 0, 1);

    imuComputeQuaternionFromRPY(0, 0, 900);
    imuUpdateEulerAngles();
    EXPECT_NEAR(attitude.values.roll, 0, 1);
    EXPECT_NEAR(attitude.values.pitch, 0, 1);
    EXPECT_NEAR(attitude.values.yaw, 900, 1);

    imuComputeQuaternionFromRPY(0, 0, 2700);
    imuUpdateEulerAngles();
    EXPECT_NEAR(attitude.values.roll, 0, 1);
    EXPECT_NEAR(attitude.values.pitch, 0, 1);
    EXPECT_NEAR(attitude.values.yaw, 2700, 1);
}

TEST(FlightImuTest, TestGpsCentrifugalCompensation)
{
    const float radius = 100 * 100;
    const float speed = 10 * 100;
    const float duration = 60;
    const float gpsUpdateRate = 10;
    const float imuUpdateRate = 1000;
    const float imuUpdatePeriod = 1 / imuUpdateRate;
    const float gpsUpdatePeriod = 1 / gpsUpdateRate;
    const float turnRate = speed / radius;
    const float accWScaler = 1;
    const float magWScaler = 1;
    const fpVector3_t gyroBF {0, 0, -turnRate};
    const float centrifugalAcc = turnRate * turnRate * radius;
    const fpVector3_t accBF {0, centrifugalAcc, GRAVITY_CMSS};

    bool velEFNew = false;
    float currentTime = 0;
    float nextGpsUpdate = gpsUpdatePeriod;
    float nextImuUpdate = imuUpdatePeriod;
    float absRollErrorSum = 0;
    float absPitchErrorSum = 0;
    float count = 0;

    imuConfigMutable()->dcm_kp_acc = .1 * 10000;
    imuConfigMutable()->dcm_ki_acc = .005 * 10000;
    imuConfigMutable()->dcm_kp_mag = .5 * 10000;
    imuConfigMutable()->dcm_ki_mag = 0.0 * 10000;
    imuConfigure();

    imuSetMagneticDeclination(0);
    imuComputeQuaternionFromRPY(0, 0, 0);

    while (currentTime < duration) {
        bool updateImu = false;
        if (nextImuUpdate < nextGpsUpdate) {
            currentTime = nextImuUpdate;
            nextImuUpdate += imuUpdatePeriod;
            updateImu = true;
        } else {
            currentTime = nextGpsUpdate;
            nextGpsUpdate += gpsUpdatePeriod;
        }

        float yaw = turnRate * currentTime;
        if (updateImu) {
            // Body frame seems to be forward, left, down
            fpVector3_t magBF {cosf(-yaw), -sinf(-yaw), 0};
            // Earth frame of IMU has y pointing west instead of east
            fpVector3_t velEF = { gpsSol.velNED[X], -gpsSol.velNED[Y], gpsSol.velNED[Z] };
            imuMahonyAHRSupdate(imuUpdatePeriod, &gyroBF, &accBF, &magBF, false, yaw, accWScaler, magWScaler, &velEF, velEFNew);
            imuUpdateEulerAngles();
            float rollError = fabs(attitude.values.roll);
            float pitchError = fabs(attitude.values.pitch);
            absRollErrorSum += rollError;
            absPitchErrorSum += pitchError;
            count += 1;
            velEFNew = false;
        } else {
            gpsSol.groundCourse = RADIANS_TO_DECIDEGREES(yaw);
            gpsSol.velNED[X] = cosf(yaw) * speed;
            gpsSol.velNED[Y] = sinf(yaw) * speed;
            gpsSol.velNED[Z] = 0;
            velEFNew = true;
        }
    }

    float meanAbsRollError = absRollErrorSum / count;
    float meanAbsPitchError = absPitchErrorSum / count;

    EXPECT_LE(meanAbsRollError, 1);
    EXPECT_LE(meanAbsPitchError, 1);
}

// STUBS

extern "C" {

uint32_t stateFlags;
uint32_t flightModeFlags;
uint32_t armingFlags;

acc_t acc;
mag_t mag;

gpsSolutionData_t gpsSol;

compassConfig_t compassConfig_System;

uint8_t detectedSensors[] = { GYRO_NONE, ACC_NONE };

bool sensors(uint32_t mask)
{
    UNUSED(mask);
    return false;
};
uint32_t millis(void) { return 0; }
timeDelta_t getLooptime(void) { return gyro.targetLooptime; }
timeDelta_t getGyroLooptime(void) { return gyro.targetLooptime; }
void schedulerResetTaskStatistics(cfTaskId_e) {}
void sensorsSet(uint32_t) {}
bool compassIsHealthy(void) { return true; }
void accGetVibrationLevels(fpVector3_t *accVibeLevels)
{
    accVibeLevels->x = sqrtf(acc.accVibeSq[X]);
    accVibeLevels->y = sqrtf(acc.accVibeSq[Y]);
    accVibeLevels->z = sqrtf(acc.accVibeSq[Z]);
}
void accGetMeasuredAcceleration(fpVector3_t *measuredAcc)
{
    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        measuredAcc->v[axis] = acc.accADCf[axis] * GRAVITY_CMSS;
    }
}
uint32_t accGetClipCount(void)
{
    return acc.accClipCount;
}
void accUpdate(void)
{
}
void resetHeadingHoldTarget(int16_t heading)
{
    UNUSED(heading);
}
bool isGPSHeadingValid(void) { return true; }
}
