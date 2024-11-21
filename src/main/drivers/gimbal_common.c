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

#include <build/debug.h>
#include <config/parameter_group_ids.h>

#include "common/time.h"

#include "fc/cli.h"

#include "drivers/gimbal_common.h"
#include "rx/rx.h"

#include "settings_generated.h"


PG_REGISTER_WITH_RESET_TEMPLATE(gimbalConfig_t, gimbalConfig, PG_GIMBAL_CONFIG, 2);

PG_RESET_TEMPLATE(gimbalConfig_t, gimbalConfig, 
    .panChannel = SETTING_GIMBAL_PAN_CHANNEL_DEFAULT,
    .tiltChannel = SETTING_GIMBAL_TILT_CHANNEL_DEFAULT,
    .rollChannel = SETTING_GIMBAL_ROLL_CHANNEL_DEFAULT,
    .sensitivity = SETTING_GIMBAL_SENSITIVITY_DEFAULT,
    .panTrim = SETTING_GIMBAL_PAN_TRIM_DEFAULT,
    .tiltTrim = SETTING_GIMBAL_TILT_TRIM_DEFAULT,
    .rollTrim = SETTING_GIMBAL_ROLL_TRIM_DEFAULT
);

static gimbalDevice_t *commonGimbalDevice = NULL;

void gimbalCommonInit(void)
{
}

void gimbalCommonSetDevice(gimbalDevice_t *gimbalDevice)
{
    SD(fprintf(stderr, "[GIMBAL]: device added %p\n", gimbalDevice));
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

#ifdef GIMBAL_UNIT_TEST
void taskUpdateGimbal(timeUs_t currentTimeUs)
{
}
#else
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

// TODO: check if any gimbal types are enabled
bool gimbalCommonIsEnabled(void)
{
    return true;
}


bool gimbalCommonHtrkIsEnabled(void)
{
    const gimbalDevice_t *dev = gimbalCommonDevice();
    if(dev && dev->vTable->hasHeadTracker) {
        bool ret = dev->vTable->hasHeadTracker(dev);
        return ret;
    }

    return false;
}


int16_t gimbalCommonGetPanPwm(const gimbalDevice_t *gimbalDevice)
{
    if (gimbalDevice && gimbalDevice->vTable->getGimbalPanPWM) {
        return gimbalDevice->vTable->getGimbalPanPWM(gimbalDevice);
    }

    return gimbalDevice ? gimbalDevice->currentPanPWM : PWM_RANGE_MIDDLE + gimbalConfig()->panTrim;
}

#endif
