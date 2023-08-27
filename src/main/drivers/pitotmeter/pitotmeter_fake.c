/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>
#include <stdint.h>

#include <platform.h>

#include "build/build_config.h"

#include "common/utils.h"

#include "drivers/pitotmeter/pitotmeter.h"
#include "drivers/pitotmeter/pitotmeter_fake.h"

#ifdef USE_PITOT_FAKE
static float fakePressure;
static float fakeTemperature;
static float fakeAirspeed;

static bool fakePitotStart(pitotDev_t *pitot)
{
    UNUSED(pitot);
    return true;
}

void fakePitotSet(float pressure, float temperature)
{
    fakePressure = pressure;
    fakeTemperature = temperature;
}

void fakePitotSetAirspeed(float airSpeed)
{
    fakeAirspeed = airSpeed;
}

float fakePitotGetAirspeed(void)
{
    return fakeAirspeed;
}

bool fakePitotRead(pitotDev_t *pitot)
{
    pitot->calculate(pitot, &fakePressure, &fakeTemperature);
    return true;
}

static void fakePitotCalculate(pitotDev_t *pitot, float *pressure, float *temperature)
{
    UNUSED(pitot);
    if (pressure)
        *pressure = fakePressure;    // Pa
    if (temperature)
        *temperature = fakeTemperature; // K
}

bool fakePitotDetect(pitotDev_t *pitot)
{
    fakePressure = 0;           // 0Pa
    fakeTemperature = 273;      // 0C

    pitot->delay = 10000;
    pitot->start = fakePitotStart;
    pitot->get = fakePitotRead;
    pitot->calculate = fakePitotCalculate;
    return true;
}
#endif
