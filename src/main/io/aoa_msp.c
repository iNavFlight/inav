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
 * This file is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#include <platform.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#if defined(USE_AOA_MSP)

#include "build/build_config.h"
#include "common/maths.h"
#include "common/utils.h"
#include "drivers/aoa/aoa_virtual.h"
#include "io/aoa.h"
#include "msp/msp_protocol_v2_sensor_msg.h"
#include "sensors/aoa.h"

static int16_t sensorAoa = AOA_NO_NEW_DATA;
static int16_t sensorSideslip = AOA_NO_NEW_DATA;
static bool hasNewData = false;

static bool aoaMspDetect(void)
{
    return true;
}

static void aoaMspInit(void)
{
    sensorAoa = AOA_NO_NEW_DATA;
    sensorSideslip = AOA_NO_NEW_DATA;
}

static void aoaMspUpdate(void)
{
}

static void aoaMspRead(int16_t * aoa, int16_t * sideslip)
{
    *aoa = sensorAoa;
    *sideslip = sensorSideslip;
}

virtualAoaVTable_t aoaMSPVtable = {
    .detect = aoaMspDetect,
    .init = aoaMspInit,
    .update = aoaMspUpdate,
    .read = aoaMspRead,
};

void mspAoaReceiveNewData(uint8_t * bufferPtr)
{
    const mspSensorAoaDataMessage_t * pkt = (const mspSensorAoaDataMessage_t *)bufferPtr;
    sensorAoa = pkt->aoa;
    sensorSideslip = pkt->sideslip;
    hasNewData = true;
}

#endif
