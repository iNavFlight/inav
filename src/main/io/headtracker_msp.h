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


#pragma once

#include <platform.h>

#if (defined(USE_HEADTRACKER_MSP) && defined(USE_HEADTRACKER))

#include <stdint.h>
#include <unistd.h>

#include "drivers/headtracker_common.h"

typedef struct headtrackerMspMessage_s {
    uint8_t version;	    // 0
    int16_t pan;            // -2048~2047. Scale is min/max angle for gimbal
    int16_t tilt;           // -2048~2047. Scale is min/max angle for gimbal
    int16_t roll;           // -2048~2047. Scale is min/max angle for gimbal
    int16_t sensitivity;    // -16~15. Scale is min/max angle for gimbal
} __attribute__((packed)) headtrackerMspMessage_t;

void mspHeadTrackerInit(void);

void mspHeadTrackerReceiverNewData(uint8_t *data, unsigned int dataSize);

headTrackerDevType_e heatTrackerMspGetDeviceType(const headTrackerDevice_t *headTrackerDevice);
bool heatTrackerMspIsReady(const headTrackerDevice_t *headTrackerDevice);

#endif
