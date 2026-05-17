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

#if defined(USE_BARO_CRSF)

#include "build/build_config.h"

#include "common/utils.h"
#include "common/time.h"

#include "drivers/time.h"
#include "drivers/barometer/barometer.h"
#include "drivers/barometer/barometer_crsf.h"

#include "sensors/sensors.h"
#include "sensors/barometer.h"
#include "fc/runtime_config.h"

#define CRSF_BARO_TIMEOUT_MS    250

static int32_t crsfBaroPressure;
static int32_t crsfBaroTemperature;
static timeMs_t crsfBaroLastUpdateMs;
static bool crsfBaroStarted = false;

static bool crsfBaroStartGet(baroDev_t *baro)
{
    UNUSED(baro);
    return true;
}

static bool crsfBaroCalculate(baroDev_t *baro, int32_t *pressure, int32_t *temperature)
{
    UNUSED(baro);

    if ((millis() - crsfBaroLastUpdateMs) > CRSF_BARO_TIMEOUT_MS) {
        sensorsClear(SENSOR_BARO);
        crsfBaroStarted = false;
        return false;
    }

    if (pressure)
        *pressure = crsfBaroPressure;

    if (temperature)
        *temperature = crsfBaroTemperature;

    return true;
}

void crsfBaroReceiveNewData(int32_t pressurePa, int32_t temperature)
{
    crsfBaroPressure = pressurePa;
    crsfBaroTemperature = temperature;
    crsfBaroLastUpdateMs = millis();

    if (crsfBaroStarted == false && !ARMING_FLAG(WAS_EVER_ARMED)) {
        baroStartCalibration();
        crsfBaroStarted = true;
        sensorsSet(SENSOR_BARO);
    }
}

bool crsfBaroDetect(baroDev_t *baro)
{
    crsfBaroPressure = 101325;
    crsfBaroTemperature = 2500;
    crsfBaroLastUpdateMs = 0;

    baro->ut_delay = 10000;
    baro->get_ut = crsfBaroStartGet;
    baro->start_ut = crsfBaroStartGet;

    baro->up_delay = 10000;
    baro->start_up = crsfBaroStartGet;
    baro->get_up = crsfBaroStartGet;

    baro->calculate = crsfBaroCalculate;

    return true;
}

#endif
