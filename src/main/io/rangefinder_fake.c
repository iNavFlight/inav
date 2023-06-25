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
#include <ctype.h>
#include <math.h>

#include "platform.h"

#include "build/build_config.h"
#include "build/debug.h"

#include "common/maths.h"

#include "io/serial.h"

#if defined(USE_RANGEFINDER_FAKE)
#include "drivers/rangefinder/rangefinder_virtual.h"
#include "drivers/time.h"
#include "io/rangefinder.h"

static bool hasNewData = false;
static int32_t sensorData = RANGEFINDER_NO_NEW_DATA;

static bool fakeRangefinderDetect(void)
{
    // Always detectable
    return true;
}

static void fakeRangefinderInit(void)
{
}

static void fakeRangefinderUpdate(void)
{
}

static int32_t fakeRangefinderGetDistance(void)
{
    if (hasNewData) {
        hasNewData = false;
        return (sensorData > 0) ? sensorData : RANGEFINDER_OUT_OF_RANGE;
    }
    else {
        return RANGEFINDER_NO_NEW_DATA;
    }
}

void fakeRangefindersSetData(int32_t data)
{
    sensorData = data;
    hasNewData = true;
}

virtualRangefinderVTable_t rangefinderFakeVtable = {
    .detect = fakeRangefinderDetect,
    .init = fakeRangefinderInit,
    .update = fakeRangefinderUpdate,
    .read = fakeRangefinderGetDistance
};


#endif