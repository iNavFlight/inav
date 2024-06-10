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

#ifdef USE_HEADTRACKER

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <build/debug.h>
#include <config/parameter_group_ids.h>

#include "common/time.h"
#include "common/maths.h"

#include "fc/cli.h"

#include "rx/rx.h"

#include "drivers/headtracker_common.h"
#include "drivers/time.h"


PG_REGISTER(headTrackerConfig_t, headTrackerConfig, PG_HEADTRACKER_CONFIG, 0);


static headTrackerDevice_t *commonHeadTrackerDevice = NULL;

void headTrackerCommonInit(void)
{
}

void headTrackerCommonSetDevice(headTrackerDevice_t *headTrackerDevice)
{
    SD(fprintf(stderr, "[headTracker]: device added %p\n", headTrackerDevice));
    commonHeadTrackerDevice = headTrackerDevice;
}

headTrackerDevice_t *headTrackerCommonDevice(void)
{
    return commonHeadTrackerDevice;
}

void headTrackerCommonProcess(headTrackerDevice_t *headTrackerDevice, timeUs_t currentTimeUs)
{
    if (headTrackerDevice && headTrackerDevice->vTable->process && headTrackerCommonIsReady(headTrackerDevice)) {
        headTrackerDevice->vTable->process(headTrackerDevice, currentTimeUs);
    }
}

headTrackerDevType_e headTrackerCommonGetDeviceType(headTrackerDevice_t *headTrackerDevice)
{
    if (!headTrackerDevice || !headTrackerDevice->vTable->getDeviceType) {
        return HEADTRACKER_UNKNOWN;
    }

    return headTrackerDevice->vTable->getDeviceType(headTrackerDevice);
}

bool headTrackerCommonIsReady(headTrackerDevice_t *headTrackerDevice)
{
    if (headTrackerDevice && headTrackerDevice->vTable->isReady) {
        return headTrackerDevice->vTable->isReady(headTrackerDevice);
    }
    return false;
}

int headTrackerCommonGetPanPWM(const headTrackerDevice_t *headTrackerDevice)
{
    if(headTrackerDevice && headTrackerDevice->vTable && headTrackerDevice->vTable->getPanPWM) {
        return headTrackerDevice->vTable->getPanPWM(headTrackerDevice);
    }

    return constrain(headTrackerDevice->pan, PWM_RANGE_MIN, PWM_RANGE_MAX);
}

int headTrackerCommonGetTiltPWM(const headTrackerDevice_t *headTrackerDevice)
{
    if(headTrackerDevice && headTrackerDevice->vTable && headTrackerDevice->vTable->getPanPWM) {
        return headTrackerDevice->vTable->getTiltPWM(headTrackerDevice);
    }

    return constrain(headTrackerDevice->tilt, PWM_RANGE_MIN, PWM_RANGE_MAX);
}

int headTrackerCommonGetRollPWM(const headTrackerDevice_t *headTrackerDevice)
{
    if(headTrackerDevice && headTrackerDevice->vTable && headTrackerDevice->vTable->getPanPWM) {
        return headTrackerDevice->vTable->getTiltPWM(headTrackerDevice);
    }

    return constrain(headTrackerDevice->roll, PWM_RANGE_MIN, PWM_RANGE_MAX);
}


#ifdef headTracker_UNIT_TEST
void taskUpdateHeadTracker(timeUs_t currentTimeUs)
{
}
#else
void taskUpdateHeadTracker(timeUs_t currentTimeUs)
{
    if (cliMode) {
        return;
    }

    headTrackerDevice_t *headTrackerDevice = headTrackerCommonDevice();

    if(headTrackerDevice) {
        headTrackerCommonProcess(headTrackerDevice, currentTimeUs);
    }
}

// TODO: check if any headTracker types are enabled
bool headTrackerCommonIsEnabled(void)
{
    if (commonHeadTrackerDevice && headTrackerCommonIsReady(commonHeadTrackerDevice)) {
        return true;
    }

    return false;
}

bool headTrackerCommonIsValid(headTrackerDevice_t *dev) {
    if(dev && dev->vTable && dev->vTable->isValid) {
        return dev->vTable->isValid(dev);
    }

    return micros() < dev->expires;
}


#endif


#endif