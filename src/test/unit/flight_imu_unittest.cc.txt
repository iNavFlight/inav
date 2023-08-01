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

extern "C" {
    #include "sensors/gyro.h"
    #include "sensors/compass.h"
    #include "sensors/acceleration.h"

    #include "scheduler/scheduler.h"
    #include "fc/runtime_config.h"

    #include "io/gps.h"
    #include "flight/pid.h"
    #include "flight/imu.h"
}

#include "unittest_macros.h"
#include "gtest/gtest.h"

extern "C" { 
STATIC_UNIT_TESTED void imuUpdateEulerAngles(void);
STATIC_UNIT_TESTED void imuComputeQuaternionFromRPY(int16_t initialRoll, int16_t initialPitch, int16_t initialYaw);
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

// STUBS

extern "C" {

uint32_t stateFlags;
uint32_t flightModeFlags;
uint32_t armingFlags;

acc_t acc;
mag_t mag;

gpsSolutionData_t gpsSol;

compassConfig_t compassConfig_System;

pidProfile_t* pidProfile_ProfileCurrent;

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
    accVibeLevels->x = fast_fsqrtf(acc.accVibeSq[X]);
    accVibeLevels->y = fast_fsqrtf(acc.accVibeSq[Y]);
    accVibeLevels->z = fast_fsqrtf(acc.accVibeSq[Z]);
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
