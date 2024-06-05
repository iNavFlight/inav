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

#include "platform.h"

#ifdef USE_SERIAL_GIMBAL

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <config/parameter_group_ids.h>

#include "common/time.h"

#include "fc/cli.h"

#include "drivers/gimbal_common.h"


PG_REGISTER(gimbalConfig_t, gimbalConfig, PG_GIMBAL_CONFIG, 0);


static gimbalDevice_t *commonGimbalDevice = NULL;

void gimbalCommonInit(void)
{
}

void gimbalCommonSetDevice(gimbalDevice_t *gimbalDevice)
{
    commonGimbalDevice = gimbalDevice;
}

gimbalDevice_t *gimbalCommonDevice(void)
{
    return commonGimbalDevice;
}

void gimbalCommonProcess(gimbalDevice_t *gimbalDevice, timeUs_t currentTimeUs)
{
    if (gimbalDevice && gimbalDevice->vTable->process && gimbalCommonIsReady(gimbalDevice)) {
        gimbalDevice->vTable->process(gimbalDevice, currentTimeUs);
    }
}

gimbalDevType_e gimbalCommonGetDeviceType(gimbalDevice_t *gimbalDevice)
{
    if (!gimbalDevice || !gimbalDevice->vTable->getDeviceType) {
        return GIMBAL_DEV_UNKNOWN;
    }

    return gimbalDevice->vTable->getDeviceType(gimbalDevice);
}

bool gimbalCommonIsReady(gimbalDevice_t *gimbalDevice)
{
    if (gimbalDevice && gimbalDevice->vTable->isReady) {
        return gimbalDevice->vTable->isReady(gimbalDevice);
    }
    return false;
}

void taskUpdateGimbal(timeUs_t currentTimeUs)
{
    if (cliMode) {
        return;
    }

    gimbalDevice_t *gimbalDevice = gimbalCommonDevice();

    if(gimbalDevice) {
        gimbalCommonProcess(gimbalDevice, currentTimeUs);
    }
}

#endif