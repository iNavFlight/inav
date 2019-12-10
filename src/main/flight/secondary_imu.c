/*
 * This file is part of INAV.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */
#include "common/utils.h"
#include "common/axis.h"
#include "flight/secondary_imu.h"
#include "config/parameter_group_ids.h"
#include "sensors/boardalignment.h"

#include "build/debug.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro_bno055.h"

PG_REGISTER_WITH_RESET_FN(secondaryImuConfig_t, secondaryImuConfig, PG_SECONDARY_IMU, 1);

void pgResetFn_secondaryImuConfig(secondaryImuConfig_t *instance)
{
    instance->enabled = 0;
    instance->rollDeciDegrees = 0;
    instance->pitchDeciDegrees = 0;
    instance->yawDeciDegrees = 0;
    instance->useForOsdHeading = 0;
    instance->useForOsdAHI = 0;

    for (uint8_t i = 0; i < 3 ; i++) {
        instance->calibrationOffsetGyro[i] = 0;
        instance->calibrationOffsetMag[i] = 0;
        instance->calibrationOffsetAcc[i] = 0;
    }
    instance->calibrationRadiusAcc = 0;
    instance->calibrationRadiusMag = 0;
}

EXTENDED_FASTRAM secondaryImuState_t secondaryImuState;

void taskSecondaryImu(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);
    static bool secondaryImuChecked = false;

    if (!secondaryImuChecked) {

        bno055CalibrationData_t calibrationData;
        calibrationData.offset[ACC][X] = secondaryImuConfig()->calibrationOffsetAcc[X];
        calibrationData.offset[ACC][Y] = secondaryImuConfig()->calibrationOffsetAcc[Y];
        calibrationData.offset[ACC][Z] = secondaryImuConfig()->calibrationOffsetAcc[Z];
        calibrationData.offset[MAG][X] = secondaryImuConfig()->calibrationOffsetMag[X];
        calibrationData.offset[MAG][Y] = secondaryImuConfig()->calibrationOffsetMag[Y];
        calibrationData.offset[MAG][Z] = secondaryImuConfig()->calibrationOffsetMag[Z];
        calibrationData.offset[GYRO][X] = secondaryImuConfig()->calibrationOffsetGyro[X];
        calibrationData.offset[GYRO][Y] = secondaryImuConfig()->calibrationOffsetGyro[Y];
        calibrationData.offset[GYRO][Z] = secondaryImuConfig()->calibrationOffsetGyro[Z];
        calibrationData.radius[ACC] = secondaryImuConfig()->calibrationRadiusAcc;
        calibrationData.radius[MAG] = secondaryImuConfig()->calibrationRadiusMag;

        secondaryImuState.active = bno055Init(calibrationData);
        secondaryImuChecked = true;
    }

    if (secondaryImuState.active) 
    {
        bno055FetchEulerAngles(secondaryImuState.eulerAngles.raw);

        //TODO this way of rotating a vector makes no sense, something simpler have to be developed
        const fpVector3_t v = {
            .x = secondaryImuState.eulerAngles.raw[0],
            .y = secondaryImuState.eulerAngles.raw[1],
            .z = secondaryImuState.eulerAngles.raw[2],
         };

        fpVector3_t rotated;

        fp_angles_t imuAngles = {
             .angles.roll = DECIDEGREES_TO_RADIANS(secondaryImuConfig()->rollDeciDegrees),
             .angles.pitch = DECIDEGREES_TO_RADIANS(secondaryImuConfig()->pitchDeciDegrees),
             .angles.yaw = DECIDEGREES_TO_RADIANS(secondaryImuConfig()->yawDeciDegrees),
        };
        fpMat3_t rotationMatrix;
        rotationMatrixFromAngles(&rotationMatrix, &imuAngles);
        rotationMatrixRotateVector(&rotated, &v, &rotationMatrix);
        rotated.z = ((int32_t)(rotated.z + secondaryImuConfig()->yawDeciDegrees)) % 3600;

        secondaryImuState.eulerAngles.values.roll = rotated.x;
        secondaryImuState.eulerAngles.values.pitch = rotated.y;
        secondaryImuState.eulerAngles.values.yaw = rotated.z;

        static uint8_t tick = 0;
        tick++;
        if (tick == 10) {
            secondaryImuState.calibrationStatus = bno055GetCalibStat();
            tick = 0;
        }

        DEBUG_SET(DEBUG_IMU2, 0, secondaryImuState.eulerAngles.values.roll);
        DEBUG_SET(DEBUG_IMU2, 1, secondaryImuState.eulerAngles.values.pitch);
        DEBUG_SET(DEBUG_IMU2, 2, secondaryImuState.eulerAngles.values.yaw);

        DEBUG_SET(DEBUG_IMU2, 3, secondaryImuState.calibrationStatus.mag);
        DEBUG_SET(DEBUG_IMU2, 4, secondaryImuState.calibrationStatus.gyr);
        DEBUG_SET(DEBUG_IMU2, 5, secondaryImuState.calibrationStatus.acc);
    }
}

