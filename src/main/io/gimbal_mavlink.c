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

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>

#include "platform.h"

#ifdef USE_GIMBAL_MAVLINK

#include <common/crc.h>
#include <common/utils.h>
#include <common/maths.h>
#include <build/debug.h>

#include <drivers/gimbal_common.h>
#include <drivers/headtracker_common.h>
#include <drivers/serial.h>
#include <drivers/time.h>

#include <io/gimbal_mavlink.h>
#include <io/serial.h>

#include <rx/rx.h>
#include <fc/rc_modes.h>

#include <config/parameter_group_ids.h>

#include <telemetry/mavlink.h>

#include "settings_generated.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#ifndef MAVLINK_COMM_NUM_BUFFERS
#define MAVLINK_COMM_NUM_BUFFERS 1
#endif
#include "common/mavlink.h"
#pragma GCC diagnostic pop


gimbalVTable_t gimbalMavlinkVTable = {
    .process = gimbalMavlinkProcess,
    .getDeviceType = gimbalMavlinkGetDeviceType,
    .isReady = gimbalMavlinkIsReady,
    .hasHeadTracker = gimbalMavlinkHasHeadTracker,
};

static gimbalDevice_t mavlinkGimbalDevice = {
    .vTable = &gimbalMavlinkVTable,
    .currentPanPWM = PWM_RANGE_MIDDLE
};

gimbalDevType_e gimbalMavlinkGetDeviceType(const gimbalDevice_t *gimbalDevice)
{
    UNUSED(gimbalDevice);
    return GIMBAL_DEV_MAVLINK;
}

bool gimbalMavlinkIsReady(const gimbalDevice_t *gimbalDevice)
{
    return (gimbalCommonGetDeviceType(gimbalDevice) == GIMBAL_DEV_MAVLINK) && isMAVLinkTelemetryEnabled();
}

bool gimbalMavlinkHasHeadTracker(const gimbalDevice_t *gimbalDevice)
{
    UNUSED(gimbalDevice);
    return false;
}

bool gimbalMavlinkInit(void)
{
    if (gimbalMavlinkDetect()) {
        SD(fprintf(stderr, "Setting gimbal device\n"));
        gimbalCommonSetDevice(&mavlinkGimbalDevice);
        return true;
    }

    return false;
}

bool gimbalMavlinkDetect(void)
{
    
    SD(fprintf(stderr, "[GIMBAL]: serial Detect...\n"));
    serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_TELEMETRY_MAVLINK);
    SD(fprintf(stderr, "[GIMBAL]: portConfig: %p\n", portConfig));
    SD(fprintf(stderr, "[GIMBAL]: MAVLINK: %s\n", gimbalConfig()->gimbalType == GIMBAL_DEV_MAVLINK ? "yes" : "no"));
    return portConfig && gimbalConfig()->gimbalType == GIMBAL_DEV_MAVLINK;
}

void gimbalMavlinkProcess(const gimbalDevice_t *gimbalDevice, timeUs_t currentTime)
{
    UNUSED(currentTime);
    UNUSED(gimbalDevice);

    if (!gimbalCommonIsReady(gimbalDevice)) {
        SD(fprintf(stderr, "[GIMBAL] gimbal not ready...\n"));
        return;
    }

    const gimbalConfig_t *cfg = gimbalConfig();

    int panPWM = PWM_RANGE_MIDDLE + cfg->panTrim;
    int tiltPWM = PWM_RANGE_MIDDLE + cfg->tiltTrim;
    int rollPWM = PWM_RANGE_MIDDLE + cfg->rollTrim;

    if (IS_RC_MODE_ACTIVE(BOXGIMBALTLOCK)) {
        //attitude.mode |= GIMBAL_MODE_TILT_LOCK;
    }

    if (IS_RC_MODE_ACTIVE(BOXGIMBALRLOCK)) {
        //attitude.mode |= GIMBAL_MODE_ROLL_LOCK;
    }

    // Follow center overrides all
    if (IS_RC_MODE_ACTIVE(BOXGIMBALCENTER) || IS_RC_MODE_ACTIVE(BOXGIMBALHTRK)) {
        //attitude.mode = GIMBAL_MODE_FOLLOW;
    }
    
    if (rxAreFlightChannelsValid() && !IS_RC_MODE_ACTIVE(BOXGIMBALCENTER)) {
        if (cfg->panChannel > 0) {
            panPWM = rxGetChannelValue(cfg->panChannel - 1) + cfg->panTrim;
            panPWM = constrain(panPWM, PWM_RANGE_MIN, PWM_RANGE_MAX);
        }

        if (cfg->tiltChannel > 0) {
            tiltPWM = rxGetChannelValue(cfg->tiltChannel - 1) + cfg->tiltTrim;
            tiltPWM = constrain(tiltPWM, PWM_RANGE_MIN, PWM_RANGE_MAX);
        }

        if (cfg->rollChannel > 0) {
            rollPWM = rxGetChannelValue(cfg->rollChannel - 1) + cfg->rollTrim;
            rollPWM = constrain(rollPWM, PWM_RANGE_MIN, PWM_RANGE_MAX);
        }
    }

    {
        DEBUG_SET(DEBUG_HEADTRACKING, 4, 0);
        // Radio endpoints may need to be adjusted, as it seems ot go a bit
        // bananas at the extremes
        //attitude.pan = gimbal_scale12(PWM_RANGE_MIN, PWM_RANGE_MAX, panPWM);
        //attitude.tilt = gimbal_scale12(PWM_RANGE_MIN, PWM_RANGE_MAX, tiltPWM);
        //attitude.roll = gimbal_scale12(PWM_RANGE_MIN, PWM_RANGE_MAX, rollPWM);
    }

    //DEBUG_SET(DEBUG_HEADTRACKING, 5, attitude.pan);
    //DEBUG_SET(DEBUG_HEADTRACKING, 6, attitude.tilt);
    //DEBUG_SET(DEBUG_HEADTRACKING, 7, attitude.roll);

    //attitude.sensibility = cfg->sensitivity;

    //serialGimbalDevice.currentPanPWM = gimbal2pwm(attitude.pan);

    //serialBeginWrite(gimbalPort);
    //serialWriteBuf(gimbalPort, (uint8_t *)&attitude, sizeof(gimbalHtkAttitudePkt_t));
    //serialEndWrite(gimbalPort);
}
#endif
