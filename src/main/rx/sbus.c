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

#ifdef USE_SERIAL_RX

#include "build/debug.h"

#include "common/utils.h"

#include "drivers/time.h"

#include "io/serial.h"

#ifdef USE_TELEMETRY
#include "telemetry/telemetry.h"
#endif

#include "rx/rx.h"
#include "rx/sbus.h"
#include "rx/sbus_channels.h"

/*
 * Observations
 *
 * FrSky X8R
 * time between frames: 6ms.
 * time to send frame: 3ms.
*
 * Futaba R6208SB/R6303SB
 * time between frames: 11ms.
 * time to send frame: 3ms.
 */

#define SBUS_TIME_NEEDED_PER_FRAME 3000

#define SBUS_FRAME_SIZE (SBUS_CHANNEL_DATA_LENGTH + 2)

#define SBUS_FRAME_BEGIN_BYTE 0x0F

#define SBUS_BAUDRATE 100000

#if !defined(SBUS_PORT_OPTIONS)
#define SBUS_PORT_OPTIONS (SERIAL_STOPBITS_2 | SERIAL_PARITY_EVEN)
#endif

#define SBUS_DIGITAL_CHANNEL_MIN 173
#define SBUS_DIGITAL_CHANNEL_MAX 1812

enum {
    DEBUG_SBUS_FRAME_FLAGS = 0,
    DEBUG_SBUS_STATE_FLAGS,
    DEBUG_SBUS_FRAME_TIME,
};


typedef struct sbusFrame_s {
    uint8_t syncByte;
    sbusChannels_t channels;
    /**
     * The endByte is 0x00 on FrSky and some futaba RX's, on Some SBUS2 RX's the value indicates the telemetry byte that is sent after every 4th sbus frame.
     *
     * See https://github.com/cleanflight/cleanflight/issues/590#issuecomment-101027349
     * and
     * https://github.com/cleanflight/cleanflight/issues/590#issuecomment-101706023
     */
    uint8_t endByte;
} __attribute__ ((__packed__)) sbusFrame_t;

typedef struct sbusFrameData_s {
    volatile sbusFrame_t frame;
    volatile bool frameDone;
    uint8_t buffer[SBUS_FRAME_SIZE];
    uint8_t position;
    timeUs_t startAtUs;
} sbusFrameData_t;

STATIC_ASSERT(SBUS_FRAME_SIZE == sizeof(sbusFrame_t), SBUS_FRAME_SIZE_doesnt_match_sbusFrame_t);

// Receive ISR callback
static void sbusDataReceive(uint16_t c, void *data)
{
    sbusFrameData_t *sbusFrameData = data;

    const timeUs_t currentTimeUs = micros();
    const timeDelta_t sbusFrameTime = cmpTimeUs(currentTimeUs, sbusFrameData->startAtUs);

    // Reset buffer pointer if we've waited too long
    if (sbusFrameData->position != 0 && (sbusFrameTime > (long)(SBUS_TIME_NEEDED_PER_FRAME + 500))) {
        sbusFrameData->position = 0;
    }

    // Wait for start byte
    if (sbusFrameData->position == 0) {
        if (c != SBUS_FRAME_BEGIN_BYTE) {
            return;
        }
        sbusFrameData->startAtUs = currentTimeUs;
    }

    if (sbusFrameData->position < SBUS_FRAME_SIZE) {
        sbusFrameData->buffer[sbusFrameData->position++] = (uint8_t)c;

        // Complete frame received and frameDone flag is not set (meaning we've already consumed the data)
        if (sbusFrameData->position == SBUS_FRAME_SIZE) {
            if (!sbusFrameData->frameDone) {
                DEBUG_SET(DEBUG_SBUS, DEBUG_SBUS_FRAME_TIME, sbusFrameTime);

                // Copy frame data and set "done" flag
                memcpy((void *)&sbusFrameData->frame, (void *)&sbusFrameData->buffer[0], SBUS_FRAME_SIZE);
                sbusFrameData->frameDone = true;
            }

            // Reset buffer pointer
            sbusFrameData->position = 0;
        }
    }
}

static uint8_t sbusFrameStatus(rxRuntimeConfig_t *rxRuntimeConfig)
{
    sbusFrameData_t *sbusFrameData = rxRuntimeConfig->frameData;
    if (!sbusFrameData->frameDone) {
        return RX_FRAME_PENDING;
    }

    // Decode channel data and store return value
    const uint8_t retValue = sbusChannelsDecode(rxRuntimeConfig, (void *)&sbusFrameData->frame.channels);

    // Reset the frameDone flag - tell ISR that we're ready to receive next frame
    sbusFrameData->frameDone = false;

    return retValue;
}

bool sbusInit(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig)
{
    static uint16_t sbusChannelData[SBUS_MAX_CHANNEL];
    static sbusFrameData_t sbusFrameData;

    rxRuntimeConfig->channelData = sbusChannelData;
    rxRuntimeConfig->frameData = &sbusFrameData;
    sbusChannelsInit(rxConfig, rxRuntimeConfig);

    rxRuntimeConfig->channelCount = SBUS_MAX_CHANNEL;
    rxRuntimeConfig->rxRefreshRate = 11000;

    rxRuntimeConfig->rcFrameStatusFn = sbusFrameStatus;

    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_RX_SERIAL);
    if (!portConfig) {
        return false;
    }

#ifdef USE_TELEMETRY
    bool portShared = telemetryCheckRxPortShared(portConfig);
#else
    bool portShared = false;
#endif

    serialPort_t *sBusPort = openSerialPort(portConfig->identifier,
        FUNCTION_RX_SERIAL,
        sbusDataReceive,
        &sbusFrameData,
        SBUS_BAUDRATE,
        portShared ? MODE_RXTX : MODE_RX,
        SBUS_PORT_OPTIONS | (rxConfig->serialrx_inverted ? 0 : SERIAL_INVERTED) | (rxConfig->halfDuplex ? SERIAL_BIDIR : 0)
        );

#ifdef USE_TELEMETRY
    if (portShared) {
        telemetrySharedPort = sBusPort;
    }
#endif

    return sBusPort != NULL;
}
#endif
