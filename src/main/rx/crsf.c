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
#ifdef USE_SERIALRX_CRSF

#include "build/build_config.h"
#include "build/debug.h"

#include "common/crc.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/time.h"
#include "drivers/serial.h"
#include "drivers/serial_uart.h"

#include "io/serial.h"
#include "io/osd.h"

#include "rx/rx.h"
#include "rx/crsf.h"

#include "telemetry/crsf.h"
#define CRSF_TIME_NEEDED_PER_FRAME_US   1750 // 700 ms + 400 ms for potential ad-hoc request
#define CRSF_TIME_BETWEEN_FRAMES_US     6667 // At fastest, frames are sent by the transmitter every 6.667 milliseconds, 150 Hz

#define CRSF_DIGITAL_CHANNEL_MIN 172
#define CRSF_DIGITAL_CHANNEL_MAX 1811
#define CRSF_PAYLOAD_OFFSET offsetof(crsfFrameDef_t, type)
#define CRSF_POWER_COUNT 9

typedef struct crsfLinkState_s {
    bool frameDone;
    crsfFrame_t frame;
    uint32_t channelData[CRSF_MAX_CHANNEL];
    timeUs_t frameStartAtUs;
    uint8_t framePosition;
} crsfLinkState_t;

// Per-link parser and channel state so a primary and secondary CRSF receiver
// don't share frame/channel buffers. Primary is index 0, so the single-RX path
// is unchanged.
static crsfLinkState_t crsfLinkStates[RX_LINK_COUNT];

static serialPort_t *serialPort;
static uint8_t telemetryBuf[CRSF_FRAME_SIZE_MAX];
static uint8_t telemetryBufLen = 0;

const uint16_t crsfTxPowerStatesmW[CRSF_POWER_COUNT] = {0, 10, 25, 100, 500, 1000, 2000, 250, 50};

/*
 * CRSF protocol
 *
 * CRSF protocol uses a single wire half duplex uart connection.
 * The master sends one frame every 4ms and the slave replies between two frames from the master.
 *
 * 420000 baud
 * not inverted
 * 8 Bit
 * 1 Stop bit
 * Big endian
 * 420000 bit/s = 46667 byte/s (including stop bit) = 21.43us per byte
 * Max frame size is 64 bytes
 * A 64 byte frame plus 1 sync byte can be transmitted in 1393 microseconds.
 *
 * CRSF_TIME_NEEDED_PER_FRAME_US is set conservatively at 1500 microseconds
 *
 * Every frame has the structure:
 * <Device address> <Frame length> < Type> <Payload> < CRC>
 *
 * Device address: (uint8_t)
 * Frame length:   length in  bytes including Type (uint8_t)
 * Type:           (uint8_t)
 * CRC:            (uint8_t)
 *
 */

struct crsfPayloadRcChannelsPacked_s {
    // 176 bits of data (11 bits per channel * 16 channels) = 22 bytes.
    unsigned int chan0 : 11;
    unsigned int chan1 : 11;
    unsigned int chan2 : 11;
    unsigned int chan3 : 11;
    unsigned int chan4 : 11;
    unsigned int chan5 : 11;
    unsigned int chan6 : 11;
    unsigned int chan7 : 11;
    unsigned int chan8 : 11;
    unsigned int chan9 : 11;
    unsigned int chan10 : 11;
    unsigned int chan11 : 11;
    unsigned int chan12 : 11;
    unsigned int chan13 : 11;
    unsigned int chan14 : 11;
    unsigned int chan15 : 11;
} __attribute__ ((__packed__));

typedef struct crsfPayloadRcChannelsPacked_s crsfPayloadRcChannelsPacked_t;

typedef struct crsfPayloadLinkStatistics_s {
    uint8_t     uplinkRSSIAnt1;
    uint8_t     uplinkRSSIAnt2;
    uint8_t     uplinkLQ;
    int8_t      uplinkSNR;
    uint8_t     activeAntenna;
    uint8_t     rfMode;
    uint8_t     uplinkTXPower;
    uint8_t     downlinkRSSI;
    uint8_t     downlinkLQ;
    int8_t      downlinkSNR;
} __attribute__ ((__packed__)) crsfPayloadLinkStatistics_t;

typedef struct crsfPayloadLinkStatistics_s crsfPayloadLinkStatistics_t;

STATIC_UNIT_TESTED uint8_t crsfFrameCRC(const crsfFrame_t *crsfFrame)
{
    // CRC includes type and payload
    uint8_t crc = crc8_dvb_s2(0, crsfFrame->frame.type);
    for (int ii = 0; ii < crsfFrame->frame.frameLength - CRSF_FRAME_LENGTH_TYPE_CRC; ++ii) {
        crc = crc8_dvb_s2(crc, crsfFrame->frame.payload[ii]);
    }
    return crc;
}

// Receive ISR callback, called back from serial port
STATIC_UNIT_TESTED void crsfDataReceive(uint16_t c, void *rxCallbackData)
{
    crsfLinkState_t *link = rxCallbackData;

    const timeUs_t currentTimeUs = microsISR();

#ifdef DEBUG_CRSF_PACKETS
    debug[2] = now - crsfFrameStartAt;
#endif

    if (cmpTimeUs(currentTimeUs, link->frameStartAtUs) > CRSF_TIME_NEEDED_PER_FRAME_US) {
        // We've received a character after max time needed to complete a frame,
        // so this must be the start of a new frame.
        link->framePosition = 0;
    }

    if (link->framePosition == 0) {
        link->frameStartAtUs = currentTimeUs;
    }
    // assume frame is 5 bytes long until we have received the frame length
    // full frame length includes the length of the address and framelength fields
    const int fullFrameLength = link->framePosition < 3 ? 5 : link->frame.frame.frameLength + CRSF_FRAME_LENGTH_ADDRESS + CRSF_FRAME_LENGTH_FRAMELENGTH;

    if (fullFrameLength > CRSF_FRAME_SIZE_MAX) {
        link->framePosition = 0;
        return;
    }

    if (link->framePosition < fullFrameLength) {
        link->frame.bytes[link->framePosition++] = (uint8_t)c;
        link->frameDone = link->framePosition < fullFrameLength ? false : true;
        if (link->frameDone) {
            link->framePosition = 0;
            if (link->frame.frame.type != CRSF_FRAMETYPE_RC_CHANNELS_PACKED) {
                const uint8_t crc = crsfFrameCRC(&link->frame);
                if (crc == link->frame.bytes[fullFrameLength - 1]) {
                    switch (link->frame.frame.type)
                    {
#if defined(USE_MSP_OVER_TELEMETRY)
                        case CRSF_FRAMETYPE_MSP_REQ:
                        case CRSF_FRAMETYPE_MSP_WRITE: {
                            if (link->frame.frame.frameLength >= 4) {
                                uint8_t *frameStart = (uint8_t *)&link->frame.frame.payload + CRSF_FRAME_ORIGIN_DEST_SIZE;
                                if (bufferCrsfMspFrame(frameStart, link->frame.frame.frameLength - 4)) {
                                    crsfScheduleMspResponse(link->frame.frame.payload[1]);
                                }
                            } else {
                                link->frameDone = false;
                            }
                            break;
                        }
#endif
                        default:
                            break;
                    }
                }
            }
        }
    }
}

STATIC_UNIT_TESTED uint8_t crsfFrameStatus(rxRuntimeConfig_t *rxRuntimeConfig)
{
    crsfLinkState_t *link = rxRuntimeConfig->frameData;

    if (link->frameDone) {
        link->frameDone = false;
        if (link->frame.frame.type == CRSF_FRAMETYPE_RC_CHANNELS_PACKED) {
            // CRC includes type and payload of each frame
            const uint8_t crc = crsfFrameCRC(&link->frame);
            if (crc != link->frame.frame.payload[CRSF_FRAME_RC_CHANNELS_PAYLOAD_SIZE]) {
                return RX_FRAME_PENDING;
            }
            link->frame.frame.frameLength = CRSF_FRAME_RC_CHANNELS_PAYLOAD_SIZE + CRSF_FRAME_LENGTH_TYPE_CRC;

            // unpack the RC channels
            const crsfPayloadRcChannelsPacked_t* rcChannels = (crsfPayloadRcChannelsPacked_t*)&link->frame.frame.payload;
            link->channelData[0] = rcChannels->chan0;
            link->channelData[1] = rcChannels->chan1;
            link->channelData[2] = rcChannels->chan2;
            link->channelData[3] = rcChannels->chan3;
            link->channelData[4] = rcChannels->chan4;
            link->channelData[5] = rcChannels->chan5;
            link->channelData[6] = rcChannels->chan6;
            link->channelData[7] = rcChannels->chan7;
            link->channelData[8] = rcChannels->chan8;
            link->channelData[9] = rcChannels->chan9;
            link->channelData[10] = rcChannels->chan10;
            link->channelData[11] = rcChannels->chan11;
            link->channelData[12] = rcChannels->chan12;
            link->channelData[13] = rcChannels->chan13;
            link->channelData[14] = rcChannels->chan14;
            link->channelData[15] = rcChannels->chan15;
            return RX_FRAME_COMPLETE;
        }
        else if (link->frame.frame.type == CRSF_FRAMETYPE_LINK_STATISTICS) {
            // CRC includes type and payload of each frame
            const uint8_t crc = crsfFrameCRC(&link->frame);
            if (crc != link->frame.frame.payload[CRSF_FRAME_LINK_STATISTICS_PAYLOAD_SIZE]) {
                return RX_FRAME_PENDING;
            }
            link->frame.frame.frameLength = CRSF_FRAME_LINK_STATISTICS_PAYLOAD_SIZE + CRSF_FRAME_LENGTH_TYPE_CRC;

            const crsfPayloadLinkStatistics_t* linkStats = (crsfPayloadLinkStatistics_t*)&link->frame.frame.payload;
            const uint8_t crsftxpowerindex = (linkStats->uplinkTXPower < CRSF_POWER_COUNT) ? linkStats->uplinkTXPower : 0;

            rxLinkStatistics_t *statistics = rxRuntimeConfig->linkStatistics;
            if (statistics) {
                statistics->uplinkRSSI = -1* (linkStats->activeAntenna ? linkStats->uplinkRSSIAnt2 : linkStats->uplinkRSSIAnt1);
                statistics->uplinkLQ = linkStats->uplinkLQ;
                statistics->uplinkSNR = linkStats->uplinkSNR;
                statistics->rfMode = linkStats->rfMode;
                statistics->uplinkTXPower = crsfTxPowerStatesmW[crsftxpowerindex];
                statistics->activeAntenna = linkStats->activeAntenna;
            }

#ifdef USE_OSD
            if (statistics && statistics->uplinkLQ > 0) {
                int16_t uplinkStrength;   // RSSI dBm converted to %
                uplinkStrength = constrain((100 * sq((osdConfig()->rssi_dbm_max - osdConfig()->rssi_dbm_min)) - (100 * sq((osdConfig()->rssi_dbm_max  - statistics->uplinkRSSI)))) / sq((osdConfig()->rssi_dbm_max - osdConfig()->rssi_dbm_min)),0,100);
                if (statistics->uplinkRSSI >= osdConfig()->rssi_dbm_max )
                    uplinkStrength = 99;
                else if (statistics->uplinkRSSI < osdConfig()->rssi_dbm_min)
                    uplinkStrength = 0;
                lqTrackerSet(rxRuntimeConfig->lqTracker, scaleRange(uplinkStrength, 0, 99, 0, RSSI_MAX_VALUE));
            } else {
                lqTrackerSet(rxRuntimeConfig->lqTracker, 0);
            }
#endif
            // This is not RC channels frame, update channel value but don't indicate frame completion
            return RX_FRAME_PENDING;
        }
    }
    return RX_FRAME_PENDING;
}

STATIC_UNIT_TESTED uint16_t crsfReadRawRC(const rxRuntimeConfig_t *rxRuntimeConfig, uint8_t chan)
{
    const crsfLinkState_t *link = rxRuntimeConfig->frameData;
    /* conversion from RC value to PWM
     *       RC     PWM
     * min  172 ->  988us
     * mid  992 -> 1500us
     * max 1811 -> 2012us
     * scale factor = (2012-988) / (1811-172) = 0.62477120195241
     * offset = 988 - 172 * 0.62477120195241 = 880.53935326418548
     */
    return (link->channelData[chan] * 1024 / 1639) + 881;
}

void crsfRxWriteTelemetryData(const void *data, int len)
{
    len = MIN(len, (int)sizeof(telemetryBuf));
    memcpy(telemetryBuf, data, len);
    telemetryBufLen = len;
}

void crsfRxSendTelemetryData(void)
{
    // if there is telemetry data to write
    if (telemetryBufLen > 0) {
        // check that we are not in bi dir mode or that we are not currently receiving data (ie in the middle of an RX frame)
        // and that there is time to send the telemetry frame before the next RX frame arrives
        if (CRSF_PORT_OPTIONS & SERIAL_BIDIR) {
            // Telemetry shares the primary link's port, so pace against its frame timing.
            const timeDelta_t timeSinceStartOfFrame = cmpTimeUs(micros(), crsfLinkStates[RX_LINK_PRIMARY].frameStartAtUs);
            if ((timeSinceStartOfFrame < CRSF_TIME_NEEDED_PER_FRAME_US) ||
                (timeSinceStartOfFrame > CRSF_TIME_BETWEEN_FRAMES_US - CRSF_TIME_NEEDED_PER_FRAME_US)) {
                return;
            }
        }
        serialWriteBuf(serialPort, telemetryBuf, telemetryBufLen);
        telemetryBufLen = 0; // reset telemetry buffer
    }
}

bool crsfRxIsTelemetryBufEmpty(void)
{
    return telemetryBufLen == 0;
}

bool crsfRxInit(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig, serialPortFunction_e portFunction)
{
    const unsigned linkIndex = (portFunction == FUNCTION_RX_SERIAL_SECONDARY) ? RX_LINK_SECONDARY : RX_LINK_PRIMARY;
    crsfLinkState_t *link = &crsfLinkStates[linkIndex];

    link->frameDone = false;
    link->framePosition = 0;
    for (int ii = 0; ii < CRSF_MAX_CHANNEL; ++ii) {
        link->channelData[ii] = (16 * PWM_RANGE_MIDDLE) / 10 - 1408;
    }

    rxRuntimeConfig->channelCount = CRSF_MAX_CHANNEL;
    rxRuntimeConfig->rcReadRawFn = crsfReadRawRC;
    rxRuntimeConfig->rcFrameStatusFn = crsfFrameStatus;
    rxRuntimeConfig->frameData = link;

    const serialPortConfig_t *portConfig = findSerialPortConfig(portFunction);
    if (!portConfig) {
        return false;
    }

    serialPort_t *port = openSerialPort(portConfig->identifier,
        portFunction,
        crsfDataReceive,
        link,
        CRSF_BAUDRATE,
        CRSF_PORT_MODE,
        CRSF_PORT_OPTIONS | (tristateWithDefaultOffIsActive(rxConfig->halfDuplex) ? SERIAL_BIDIR : 0)
        );

    // Telemetry replies go out the primary link's port.
    if (linkIndex == RX_LINK_PRIMARY) {
        serialPort = port;
    }

    return port != NULL;
}

bool crsfRxIsActive(void)
{
    return serialPort != NULL;
}


void crsfBind(void)
{
    if (serialPort != NULL) {
        uint8_t bindFrame[] = {
            CRSF_SYNC_BYTE,
            0x07,  // frame length
            CRSF_FRAMETYPE_COMMAND,
            CRSF_ADDRESS_CRSF_RECEIVER,
            CRSF_ADDRESS_FLIGHT_CONTROLLER,
            CRSF_COMMAND_SUBCMD_RX,
            CRSF_COMMAND_SUBCMD_RX_BIND,
            0x9E,  // Command CRC8
            0xE8,  // Packet CRC8
        };
        serialWriteBuf(serialPort, bindFrame, 9);
    }
}

#endif
