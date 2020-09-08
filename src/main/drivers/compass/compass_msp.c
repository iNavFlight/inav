/*
 * This file is part of INAV Project.
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

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"

#if defined(USE_MAG_MSP)

#include "build/build_config.h"

#include "common/axis.h"
#include "common/utils.h"
#include "common/time.h"

#include "drivers/time.h"
#include "drivers/compass/compass.h"
#include "drivers/compass/compass_msp.h"

#include "sensors/boardalignment.h"

#include "msp/msp_protocol_v2_sensor_msg.h"

#define MSP_MAG_TIMEOUT_MS      250     // Less than 4Hz updates is considered a failure

static int32_t mspMagData[XYZ_AXIS_COUNT];
static timeMs_t mspMagLastUpdateMs;

static bool mspMagInit(magDev_t *magDev)
{
    UNUSED(magDev);
    mspMagData[X] = 0;
    mspMagData[Y] = 0;
    mspMagData[Z] = 0;
    mspMagLastUpdateMs = 0;
    return true;
}

void mspMagReceiveNewData(uint8_t * bufferPtr)
{
    const mspSensorCompassDataMessage_t * pkt = (const mspSensorCompassDataMessage_t *)bufferPtr;

    mspMagData[X] = pkt->magX;
    mspMagData[Y] = pkt->magY;
    mspMagData[Z] = pkt->magZ;

    applySensorAlignment(mspMagData, mspMagData, CW90_DEG_FLIP);

    mspMagLastUpdateMs = millis();
}

static bool mspMagRead(magDev_t *magDev)
{
    UNUSED(magDev);

    if ((millis() - mspMagLastUpdateMs) > MSP_MAG_TIMEOUT_MS) {
        return false;
    }

    magDev->magADCRaw[X] = mspMagData[X];
    magDev->magADCRaw[Y] = mspMagData[Y];
    magDev->magADCRaw[Z] = mspMagData[Z];

    return true;
}

bool mspMagDetect(magDev_t *mag)
{
    mag->init = mspMagInit;
    mag->read = mspMagRead;
    return true;
}

#endif
