/*
 * This file is part of INAV.
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
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <platform.h>
#include <stdio.h>

#if (defined(USE_HEADTRACKER_MSP) && defined(USE_HEADTRACKER))

#include "build/debug.h"

#include "common/utils.h"
#include "common/time.h"
#include "common/maths.h"

#include "drivers/headtracker_common.h"

#include "io/headtracker_msp.h"

static bool isReady = false;
static headTrackerVTable_t headTrackerMspVTable = {
    .process = NULL,
    .getDeviceType = heatTrackerMspGetDeviceType,
    .isReady = heatTrackerMspIsReady,
    .isValid = NULL,
};

static headTrackerDevice_t headTrackerMspDevice = {
    .vTable = &headTrackerMspVTable,
    .pan = 0,
    .tilt = 0,
    .roll = 0,
    .expires = 0,
};

void mspHeadTrackerInit(void)
{
    if(headTrackerConfig()->devType == HEADTRACKER_MSP) {
        headTrackerCommonSetDevice(&headTrackerMspDevice);
    }
}

void mspHeadTrackerReceiverNewData(uint8_t *data, unsigned int dataSize)
{
    if(dataSize != sizeof(headtrackerMspMessage_t)) {
        SD(fprintf(stderr, "[headTracker]: invalid data size %d\n", dataSize));
        static int errorCount = 0;
        DEBUG_SET(DEBUG_HEADTRACKING, 7, errorCount++);
        DEBUG_SET(DEBUG_HEADTRACKING, 5, (sizeof(headtrackerMspMessage_t)));
        DEBUG_SET(DEBUG_HEADTRACKING, 6, dataSize);
        return;
    }
    isReady = true;
    DEBUG_SET(DEBUG_HEADTRACKING, 6, dataSize);

    headtrackerMspMessage_t *status = (headtrackerMspMessage_t *)data;

    headTrackerMspDevice.pan = constrain(status->pan, HEADTRACKER_RANGE_MIN, HEADTRACKER_RANGE_MAX);
    headTrackerMspDevice.tilt = constrain(status->tilt, HEADTRACKER_RANGE_MIN, HEADTRACKER_RANGE_MAX);
    headTrackerMspDevice.roll = constrain(status->roll, HEADTRACKER_RANGE_MIN, HEADTRACKER_RANGE_MAX);
    headTrackerMspDevice.expires = micros() + MAX_HEADTRACKER_DATA_AGE_US;

    DEBUG_SET(DEBUG_HEADTRACKING, 0, headTrackerMspDevice.pan);
    DEBUG_SET(DEBUG_HEADTRACKING, 1, headTrackerMspDevice.tilt);
    DEBUG_SET(DEBUG_HEADTRACKING, 2, headTrackerMspDevice.roll);
    DEBUG_SET(DEBUG_HEADTRACKING, 3, headTrackerMspDevice.expires);
}

headTrackerDevType_e heatTrackerMspGetDeviceType(const headTrackerDevice_t *headTrackerDevice) {
    UNUSED(headTrackerDevice);
    return HEADTRACKER_MSP;
}


bool heatTrackerMspIsReady(const headTrackerDevice_t *headTrackerDevice)
{
    UNUSED(headTrackerDevice);
    return headTrackerConfig()->devType == HEADTRACKER_MSP && isReady; 
}

#endif
