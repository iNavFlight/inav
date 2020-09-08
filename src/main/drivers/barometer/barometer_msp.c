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

#if defined(USE_BARO_MSP)

#include "build/build_config.h"
#include "build/debug.h"

#include "common/utils.h"
#include "common/time.h"

#include "drivers/time.h"
#include "drivers/barometer/barometer.h"
#include "drivers/barometer/barometer_msp.h"

#include "msp/msp_protocol_v2_sensor_msg.h"

#define MSP_BARO_TIMEOUT_MS      250     // Less than 4Hz updates is considered a failure

static int32_t mspBaroPressure;
static int32_t mspBaroTemperature;
static timeMs_t mspBaroLastUpdateMs;

static bool mspBaroStartGet(baroDev_t * baro)
{
    UNUSED(baro);
    return true;
}

static bool mspBaroCalculate(baroDev_t * baro, int32_t *pressure, int32_t *temperature)
{
    UNUSED(baro);

    if ((millis() - mspBaroLastUpdateMs) > MSP_BARO_TIMEOUT_MS) {
        return false;
    }

    if (pressure)
        *pressure = mspBaroPressure;

    if (temperature)
        *temperature = mspBaroTemperature;

    return true;
}

void mspBaroReceiveNewData(uint8_t * bufferPtr)
{
    const mspSensorBaroDataMessage_t * pkt = (const mspSensorBaroDataMessage_t *)bufferPtr;

    mspBaroPressure = pkt->pressurePa;
    mspBaroTemperature = pkt->temp;
    mspBaroLastUpdateMs = millis();
}

bool mspBaroDetect(baroDev_t *baro)
{
    mspBaroPressure = 101325;    // pressure in Pa (0m MSL)
    mspBaroTemperature = 2500;   // temperature in 0.01 C = 25 deg

    // these are dummy as temperature is measured as part of pressure
    baro->ut_delay = 10000;
    baro->get_ut = mspBaroStartGet;
    baro->start_ut = mspBaroStartGet;

    // only _up part is executed, and gets both temperature and pressure
    baro->up_delay = 10000;
    baro->start_up = mspBaroStartGet;
    baro->get_up = mspBaroStartGet;

    baro->calculate = mspBaroCalculate;

    return true;
}

#endif
