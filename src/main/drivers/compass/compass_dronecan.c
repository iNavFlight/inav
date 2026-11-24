/*
 * This file is part of INAV.
 *
 * INAV is free software. You can redistribute this software
 * and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that they will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

/*
 * Compass data received over DroneCAN
 * (uavcan.equipment.ahrs.MagneticFieldStrength / MagneticFieldStrength2).
 *
 * The data is handled exactly like the MSP compass: the peripheral sends the
 * field in the vehicle body frame (front, right, down). Only the first sensor
 * that reports is used, identified by its CAN node ID and sensor ID.
 */

#include <math.h>
#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#if defined(USE_MAG) && defined(USE_DRONECAN)

#include "common/axis.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/compass/compass.h"
#include "drivers/compass/compass_dronecan.h"

#include "sensors/boardalignment.h"

#define DRONECAN_MAG_TIMEOUT_MS     250     // Less than 4Hz updates is considered a failure

static float dronecanMagData[XYZ_AXIS_COUNT];
static timeMs_t dronecanMagLastUpdateMs;

static bool dronecanMagInit(magDev_t *magDev)
{
    UNUSED(magDev);
    dronecanMagData[X] = 0;
    dronecanMagData[Y] = 0;
    dronecanMagData[Z] = 0;
    dronecanMagLastUpdateMs = 0;
    return true;
}

void dronecanMagReceiveNewData(uint8_t nodeId, uint8_t sensorId, const float fieldGauss[3])
{
    // Lock onto the first sensor that reports (CAN node ID + sensor ID)
    static int32_t lockedSensor = -1;
    const int32_t sensor = ((int32_t)nodeId << 8) | sensorId;

    if (lockedSensor < 0) {
        lockedSensor = sensor;
    } else if (sensor != lockedSensor) {
        return;
    }

    // Gauss -> milliGauss, same units and frame as the MSP compass.
    // Drop the sample if a value is not finite or does not fit magADCRaw (int16).
    float field[XYZ_AXIS_COUNT];

    for (int axis = 0; axis < XYZ_AXIS_COUNT; axis++) {
        field[axis] = fieldGauss[axis] * 1000.0f;
        if (!isfinite(field[axis]) || fabsf(field[axis]) > INT16_MAX) {
            return;
        }
    }

    applySensorAlignment(dronecanMagData, field, CW90_DEG_FLIP);

    dronecanMagLastUpdateMs = millis();
}

static bool dronecanMagRead(magDev_t *magDev)
{
    if ((millis() - dronecanMagLastUpdateMs) > DRONECAN_MAG_TIMEOUT_MS) {
        return false;
    }

    magDev->magADCRaw[X] = dronecanMagData[X];
    magDev->magADCRaw[Y] = dronecanMagData[Y];
    magDev->magADCRaw[Z] = dronecanMagData[Z];

    return true;
}

bool dronecanMagDetect(magDev_t *mag)
{
    mag->init = dronecanMagInit;
    mag->read = dronecanMagRead;
    return true;
}

#endif
