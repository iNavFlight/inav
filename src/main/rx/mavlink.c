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
#include <stdlib.h>
#include <string.h>

#include "platform.h"
FILE_COMPILE_FOR_SPEED
#ifdef USE_SERIALRX_MAVLINK

#include "build/build_config.h"
#include "build/debug.h"

#include "common/crc.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/serial.h"
#include "drivers/serial_uart.h"

#include "io/serial.h"

#include "rx/rx.h"
#include "rx/mavlink.h"

#define MAVLINK_CHANNEL_COUNT 18
static uint16_t mavlinkChannelData[MAVLINK_CHANNEL_COUNT];

void mavlinkRxHandleMessage(const mavlink_rc_channels_override_t *msg) {
    mavlinkChannelData[0] = msg->chan1_raw;
    mavlinkChannelData[1] = msg->chan2_raw;
    mavlinkChannelData[2] = msg->chan3_raw;
    mavlinkChannelData[3] = msg->chan4_raw;
    mavlinkChannelData[4] = msg->chan5_raw;
    mavlinkChannelData[5] = msg->chan6_raw;
    mavlinkChannelData[6] = msg->chan7_raw;
    mavlinkChannelData[7] = msg->chan8_raw;
    mavlinkChannelData[8] = msg->chan9_raw;
    mavlinkChannelData[9] = msg->chan10_raw;
    mavlinkChannelData[10] = msg->chan11_raw;
    mavlinkChannelData[11] = msg->chan12_raw;
    mavlinkChannelData[12] = msg->chan13_raw;
    mavlinkChannelData[13] = msg->chan14_raw;
    mavlinkChannelData[14] = msg->chan15_raw;
    mavlinkChannelData[15] = msg->chan16_raw;
    mavlinkChannelData[16] = msg->chan17_raw;
    mavlinkChannelData[17] = msg->chan18_raw;
}

static uint8_t mavlinkFrameStatus(rxRuntimeConfig_t *rxRuntimeConfig)
{
    return RX_FRAME_COMPLETE;
}

static uint16_t mavlinkReadRawRC(const rxRuntimeConfig_t *rxRuntimeConfig, uint8_t chan)
{
    UNUSED(rxRuntimeConfig);
    // MAVLink values are sent as PWM values in microseconds so no conversion is needed
    return mavlinkChannelData[chan];
}

bool mavlinkRxInit(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig)
{
    // TODO failsafe

    UNUSED(rxConfig);

    rxRuntimeConfig->channelData = mavlinkChannelData;
    rxRuntimeConfig->channelCount = MAVLINK_CHANNEL_COUNT;
    rxRuntimeConfig->rxRefreshRate = 11000;
    rxRuntimeConfig->rcReadRawFn = mavlinkReadRawRC;
    rxRuntimeConfig->rcFrameStatusFn = mavlinkFrameStatus;

    for (int ii = 0; ii < MAVLINK_CHANNEL_COUNT; ++ii) {
        mavlinkChannelData[ii] = (16 * PWM_RANGE_MIDDLE) / 10 - 1408;
    }

    return true;
}

#endif
