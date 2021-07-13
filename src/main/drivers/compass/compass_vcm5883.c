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

#include "platform.h"

#ifdef USE_MAG_VCM5883

#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "build/build_config.h"

#include "common/axis.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/bus_i2c.h"

#include "sensors/boardalignment.h"
#include "sensors/sensors.h"

#include "drivers/sensor.h"
#include "drivers/compass/compass.h"

#include "drivers/compass/compass_vcm5883.h"

#include "build/debug.h"

#define DETECTION_MAX_RETRY_COUNT   5
static bool deviceDetect(magDev_t * mag)
{
    for (int retryCount = 0; retryCount < DETECTION_MAX_RETRY_COUNT; retryCount++) {
        busWrite(mag->busDev, 0x0A, 0b0100);
        delay(30);

        DEBUG_SET(DEBUG_ALWAYS, 1, 3);

        uint8_t sig = 0;
        bool ack = busRead(mag->busDev, 0x0C, &sig);

        DEBUG_SET(DEBUG_ALWAYS, 1, 5);
        DEBUG_SET(DEBUG_ALWAYS, 2, ack);
        DEBUG_SET(DEBUG_ALWAYS, 3, sig);

        if (ack && sig == 0x82) {
            return true;
        }
    }

    return false;
}

bool vcm5883Detect(magDev_t * mag)
{
    DEBUG_SET(DEBUG_ALWAYS, 0, 1);

    mag->busDev = busDeviceInit(BUSTYPE_ANY, DEVHW_VCM5883, mag->magSensorToUse, OWNER_COMPASS);
    if (mag->busDev == NULL) {
        return false;
    }
    DEBUG_SET(DEBUG_ALWAYS, 0, 2);

    if (!deviceDetect(mag)) {
        DEBUG_SET(DEBUG_ALWAYS, 0, 7);
        busDeviceDeInit(mag->busDev);
        return false;
    }
    DEBUG_SET(DEBUG_ALWAYS, 0, 3);

    // mag->init = qmc5883Init;
    // mag->read = qmc5883Read;

    return false;

    // return true;
}

#endif