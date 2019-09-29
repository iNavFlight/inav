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

#include "flight/secondary_imu.h"
#include "config/parameter_group_ids.h"
#include "sensors/boardalignment.h"

#include "build/debug.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro_bno055.h"

PG_REGISTER_WITH_RESET_TEMPLATE(secondaryImuConfig_t, secondaryImuConfig, PG_SECONDARY_IMU, 0);

PG_RESET_TEMPLATE(secondaryImuConfig_t, secondaryImuConfig,
    .enabled = 0,
    .rollDeciDegrees = 0,
    .pitchDeciDegrees = 0,
    .yawDeciDegrees = 0,
);

EXTENDED_FASTRAM secondaryImuState_t secondaryImuState;

void taskSecondaryImu(timeUs_t currentTimeUs)
{
    UNUSED(currentTimeUs);
    static bool secondaryImuPresent = false;
    static bool secondaryImuChecked = false;

    if (!secondaryImuChecked) {
        secondaryImuPresent = bno055Init();
        secondaryImuChecked = true;
    }

    if (secondaryImuPresent) 
    {
        int32_t angles[3];
        bno055FetchEulerAngles(angles);

        const fpVector3_t v = {
            .x = angles[0],
            .y = angles[1],
            .z = angles[2],
         };

        fpVector3_t rotated;

        fp_angles_t compassAngles = {
             .angles.roll = DECIDEGREES_TO_RADIANS(secondaryImuConfig()->rollDeciDegrees),
             .angles.pitch = DECIDEGREES_TO_RADIANS(secondaryImuConfig()->pitchDeciDegrees),
             .angles.yaw = DECIDEGREES_TO_RADIANS(secondaryImuConfig()->yawDeciDegrees),
        };
        fpMat3_t rotationMatrix;
        rotationMatrixFromAngles(&rotationMatrix, &compassAngles);
        rotationMatrixRotateVector(&rotated, &v, &rotationMatrix);
        rotated.z = ((int32_t)(rotated.z + DECIDEGREES_TO_DEGREES(secondaryImuConfig()->yawDeciDegrees))) % 360;

        DEBUG_SET(DEBUG_IMU2, 0, rotated.x);
        DEBUG_SET(DEBUG_IMU2, 1, rotated.y);
        DEBUG_SET(DEBUG_IMU2, 2, rotated.z);
        DEBUG_SET(DEBUG_IMU2, 3, angles[2]);
    }
}