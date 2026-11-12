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
#include <stddef.h>
#include <limits.h>

#include "platform.h"

#if defined(USE_BATTERY_SENSOR_CRSF)

#include "common/maths.h"
#include "common/utils.h"
#include "drivers/time.h"
#include "sensors/battery_sensor_crsf.h"

#define CRSF_BATTERY_TIMEOUT_MS     2000

static int16_t crsfVoltage;     // 0.01V units
static int16_t crsfCurrent;     // 0.01A units (centamps)
static uint32_t crsfCapacity;   // mAh
static uint8_t crsfRemaining;   // percent
static timeMs_t crsfBatteryLastUpdateMs;

void crsfBatterySensorReceiveNewData(uint16_t voltage, uint16_t current, uint32_t capacity, uint8_t remaining)
{
    crsfVoltage = MIN(voltage, INT16_MAX);
    crsfCurrent = MIN(current, INT16_MAX);
    crsfCapacity = capacity;
    crsfRemaining = remaining;
    crsfBatteryLastUpdateMs = millis();
}

int16_t *crsfBatterySensorGetVoltageData(void)
{
    if (crsfBatteryLastUpdateMs == 0 || (millis() - crsfBatteryLastUpdateMs) > CRSF_BATTERY_TIMEOUT_MS) {
        return NULL;
    }
    return &crsfVoltage;
}

int16_t *crsfBatterySensorGetCurrentData(void)
{
    if (crsfBatteryLastUpdateMs == 0 || (millis() - crsfBatteryLastUpdateMs) > CRSF_BATTERY_TIMEOUT_MS) {
        return NULL;
    }
    return &crsfCurrent;
}

#endif
