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

#if defined(USE_PITOT_MSP)

#include "build/build_config.h"
#include "build/debug.h"

#include "common/utils.h"
#include "common/time.h"

#include "drivers/time.h"
#include "drivers/pitotmeter/pitotmeter.h"
#include "drivers/pitotmeter/pitotmeter_msp.h"

#include "msp/msp_protocol_v2_sensor_msg.h"

#define MSP_PITOT_TIMEOUT_MS      500     // Less than 2Hz updates is considered a failure

static int32_t mspPitotPressure;
static int32_t mspPitotTemperature;
static timeMs_t mspPitotLastUpdateMs;

static bool mspPitotStart(pitotDev_t *pitot)
{
    UNUSED(pitot);
    return true;
}

static bool mspPitotRead(pitotDev_t *pitot)
{
    UNUSED(pitot);
    return true;
}

static void mspPitotCalculate(pitotDev_t *pitot, float *pressure, float *temperature)
{
    UNUSED(pitot);

    if (pressure) {
        *pressure = mspPitotPressure;
    }

    if (temperature) {
        *temperature = (mspPitotTemperature - 27315) / 100.0f;  // Pitot expects temp in Kelvin
    }
}

void mspPitotmeterReceiveNewData(uint8_t * bufferPtr)
{
    const mspSensorAirspeedDataMessage_t * pkt = (const mspSensorAirspeedDataMessage_t *)bufferPtr;

    mspPitotPressure = pkt->diffPressurePa;
    mspPitotTemperature = pkt->temp;
    mspPitotLastUpdateMs = millis();
}

bool mspPitotmeterDetect(pitotDev_t *pitot)
{
    mspPitotPressure = 0;
    mspPitotTemperature = 27315;    // 0 deg/c

    pitot->delay = 10000;
    pitot->start = mspPitotStart;
    pitot->get = mspPitotRead;
    pitot->calculate = mspPitotCalculate;

    return true;
}

#endif
