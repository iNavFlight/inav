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
 *  field in the vehicle body frame (front, right, down). Only the first sensor
 * that reports is used.
 */

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

void dronecanMagReceiveNewData(uint8_t sensorId, const float fieldGauss[3])
{
    // Lock onto the first sensor that reports
    static int16_t lockedSensorId = -1;

    if (lockedSensorId < 0) {
        lockedSensorId = sensorId;
    } else if (sensorId != lockedSensorId) {
        return;
    }

    // Gauss -> milliGauss, same units and frame as the MSP compass
    dronecanMagData[X] = fieldGauss[0] * 1000.0f;
    dronecanMagData[Y] = fieldGauss[1] * 1000.0f;
    dronecanMagData[Z] = fieldGauss[2] * 1000.0f;

    applySensorAlignment(dronecanMagData, dronecanMagData, CW90_DEG_FLIP);

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
