/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Authors:
 * Thomas Miric - marv-t
 *
 * Jeti EX Bus Communication Protocol:
 * http://www.jetimodel.com/en/show-file/642/
 *
 * JETI Telemetry Communication Protocol:
 * http://www.jetimodel.com/en/show-file/26/
 *
 * Following restrictions:
 * Communication speed: 125000 bps
 * Number of channels: 16
 *
 * Connect as follows:
 * Jeti EX Bus -> Serial TX (connect directly)
 */


#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#ifdef USE_SERIALRX_JETIEXBUS

#include "build/build_config.h"
#include "build/debug.h"

#include "common/utils.h"
#include "common/maths.h"

#include "drivers/time.h"

#include "io/serial.h"

#include "rx/rx.h"
#include "rx/jetiexbus.h"


//
// Serial driver for Jeti EX Bus receiver
//
#define JETIEXBUS_BAUDRATE 125000                       // EX Bus 125000; EX Bus HS 250000 not supported
#define JETIEXBUS_OPTIONS (SERIAL_STOPBITS_1 | SERIAL_PARITY_NO)
#define JETIEXBUS_MIN_FRAME_GAP     1000

#ifdef USE_34CHANNELS
#define JETIEXBUS_CHANNEL_COUNT 24
#else
#define JETIEXBUS_CHANNEL_COUNT 16
#endif


#define EXBUS_START_CHANNEL_FRAME       (0x3E)
#define EXBUS_START_REQUEST_FRAME       (0x3D)
#define EXBUS_JETIBOX_REQUEST           (0x3B)

#define EXBUS_CHANNELDATA               (0x3E03)        // Frame contains Channel Data
#define EXBUS_CHANNELDATA_DATA_REQUEST  (0x3E01)        // Frame contains Channel Data, but with a request for data
#define EXBUS_TELEMETRY_REQUEST         (0x3D01)        // Frame is a request Frame



serialPort_t *jetiExBusPort;

volatile uint32_t jetiTimeStampRequest = 0;

volatile bool jetiExBusCanTx = false;

static uint8_t jetiExBusFramePosition;
static uint8_t jetiExBusFrameLength;

static volatile uint8_t jetiExBusFrameState = EXBUS_STATE_ZERO;
volatile uint8_t jetiExBusRequestState = EXBUS_STATE_ZERO;

// Use max values for ram areas
static uint8_t jetiExBusChannelFrame[EXBUS_MAX_CHANNEL_FRAME_SIZE];
uint8_t jetiExBusRequestFrame[EXBUS_MAX_REQUEST_FRAME_SIZE];

static uint16_t jetiExBusChannelData[JETIEXBUS_CHANNEL_COUNT];

// Jeti Ex Bus CRC calculations for a frame
uint16_t jetiExBusCalcCRC16(uint8_t *pt, uint8_t msgLen)
{
    uint16_t crc16_data = 0;
    uint8_t data=0;

    for (uint8_t mlen = 0; mlen < msgLen; mlen++) {
        data = pt[mlen] ^ ((uint8_t)(crc16_data) & (uint8_t)(0xFF));
        data ^= data << 4;
        crc16_data = ((((uint16_t)data << 8) | ((crc16_data & 0xFF00) >> 8))
                      ^ (uint8_t)(data >> 4)
                      ^ ((uint16_t)data << 3));
    }
    return(crc16_data);
}

void jetiExBusDecodeChannelFrame(uint8_t *exBusFrame)
{
    uint16_t value;
    uint8_t frameAddr;
    uint8_t channelDataLen = exBusFrame[EXBUS_HEADER_LEN - 1];
    uint8_t receivedChannelCount = MIN((channelDataLen) / 2, JETIEXBUS_CHANNEL_COUNT);

    // Decode header
    switch (((uint16_t)exBusFrame[EXBUS_HEADER_SYNC] << 8) | ((uint16_t)exBusFrame[EXBUS_HEADER_REQ])) {

    case EXBUS_CHANNELDATA_DATA_REQUEST:                   // not yet specified
    case EXBUS_CHANNELDATA:
        for (uint8_t i = 0; i < receivedChannelCount; i++) {
            frameAddr = EXBUS_HEADER_LEN + (i * 2);
            value = ((uint16_t)exBusFrame[frameAddr + 1]) << 8;
            value |= (uint16_t)exBusFrame[frameAddr];
            // Convert to internal format
            jetiExBusChannelData[i] = value >> 3;
        }
        break;
    }
}

void jetiExBusFrameReset(void)
{
    jetiExBusFramePosition = 0;
    jetiExBusFrameLength = EXBUS_MAX_CHANNEL_FRAME_SIZE;
}

/*
  supported:
  0x3E 0x01 LEN Packet_ID 0x31 SUB_LEN Data_array CRC16      // Channel Data with telemetry request (2nd byte 0x01)
  0x3E 0x03 LEN Packet_ID 0x31 SUB_LEN Data_array CRC16      // Channel Data forbids answering (2nd byte 0x03)
  0x3D 0x01 0x08 Packet_ID 0x3A 0x00 CRC16                   // Telemetry Request EX telemetry (5th byte 0x3A)

  other messages - not supported:
  0x3D 0x01 0x09 Packet_ID 0x3B 0x01 0xF0 CRC16              // Jetibox request (5th byte 0x3B)
  ...
*/

// Receive ISR callback
FAST_CODE NOINLINE static void jetiExBusDataReceive(uint16_t c, void *data)
{
    UNUSED(data);

    static timeUs_t jetiExBusTimeLast = 0;
    static uint8_t *jetiExBusFrame;
    static uint8_t jetiExBusFrameMaxSize;
    const timeUs_t now = microsISR();

    // Check if we shall reset frame position due to time
    if (cmpTimeUs(now, jetiExBusTimeLast) > JETIEXBUS_MIN_FRAME_GAP) {
        jetiExBusFrameReset();
        jetiExBusFrameState = EXBUS_STATE_ZERO;
        jetiExBusRequestState = EXBUS_STATE_ZERO;
    }
    jetiExBusTimeLast = now;

    // Check if we shall start a frame?
    if (jetiExBusFramePosition == 0) {
        switch (c) {
        case EXBUS_START_CHANNEL_FRAME:
            jetiExBusFrameState = EXBUS_STATE_IN_PROGRESS;
            jetiExBusFrame = jetiExBusChannelFrame;
            jetiExBusFrameMaxSize = EXBUS_MAX_CHANNEL_FRAME_SIZE;
            break;

        case EXBUS_START_REQUEST_FRAME:
            jetiExBusRequestState = EXBUS_STATE_IN_PROGRESS;
            jetiExBusFrame = jetiExBusRequestFrame;
            jetiExBusFrameMaxSize = EXBUS_MAX_CHANNEL_FRAME_SIZE;
            break;

        default:
            return;
        }
    }

    if(jetiExBusFramePosition == 1) {
        if(c == 0x01) {
            jetiExBusCanTx = true;
        } else {
            jetiExBusCanTx = false;
        }
    }

    if (jetiExBusFramePosition == jetiExBusFrameMaxSize) {
        // frame overrun
        jetiExBusFrameReset();
        jetiExBusFrameState = EXBUS_STATE_ZERO;
        jetiExBusRequestState = EXBUS_STATE_ZERO;

        return;
    }

    // Store in frame copy
    jetiExBusFrame[jetiExBusFramePosition] = (uint8_t)c;
    jetiExBusFramePosition++;

    // Check the header for the message length
    if (jetiExBusFramePosition == EXBUS_HEADER_LEN) {
        if ((jetiExBusFrameState == EXBUS_STATE_IN_PROGRESS) && (jetiExBusFrame[EXBUS_HEADER_MSG_LEN] <= EXBUS_MAX_CHANNEL_FRAME_SIZE)) {
            jetiExBusFrameLength = jetiExBusFrame[EXBUS_HEADER_MSG_LEN];
            return;
        }

        if ((jetiExBusRequestState == EXBUS_STATE_IN_PROGRESS) && (jetiExBusFrame[EXBUS_HEADER_MSG_LEN] <= EXBUS_MAX_REQUEST_FRAME_SIZE)) {
            jetiExBusFrameLength = jetiExBusFrame[EXBUS_HEADER_MSG_LEN];
            return;
        }

        jetiExBusFrameReset();                  // not a valid frame
        jetiExBusFrameState = EXBUS_STATE_ZERO;
        jetiExBusRequestState = EXBUS_STATE_ZERO;
        return;
    }

    // Done?
    if (jetiExBusFrameLength == jetiExBusFramePosition) {
        if (jetiExBusFrameState == EXBUS_STATE_IN_PROGRESS) {
            jetiExBusFrameState = EXBUS_STATE_RECEIVED;
            jetiExBusRequestState = EXBUS_STATE_ZERO;
        }
        if (jetiExBusRequestState == EXBUS_STATE_IN_PROGRESS) {
            jetiExBusFrameState = EXBUS_STATE_ZERO;
            jetiExBusRequestState = EXBUS_STATE_RECEIVED;
            jetiTimeStampRequest = now;
        }

        jetiExBusFrameReset();
    }
}

// Check if it is time to read a frame from the data...
static uint8_t jetiExBusFrameStatus(rxRuntimeConfig_t *rxRuntimeConfig)
{
    UNUSED(rxRuntimeConfig);

    uint8_t frameStatus = RX_FRAME_PENDING;

    if (jetiExBusFrameState == EXBUS_STATE_RECEIVED) {
        if (jetiExBusCalcCRC16(jetiExBusChannelFrame, jetiExBusChannelFrame[EXBUS_HEADER_MSG_LEN]) == 0) {
            jetiExBusDecodeChannelFrame(jetiExBusChannelFrame);
            frameStatus = RX_FRAME_COMPLETE;
        }
        jetiExBusFrameState = EXBUS_STATE_ZERO;
    }

    return frameStatus;
}

static uint16_t jetiExBusReadRawRC(const rxRuntimeConfig_t *rxRuntimeConfig, uint8_t chan)
{
    if (chan >= rxRuntimeConfig->channelCount)
        return 0;

    return (jetiExBusChannelData[chan]);
}

bool jetiExBusInit(const rxConfig_t *rxConfig, rxRuntimeConfig_t *rxRuntimeConfig)
{
    UNUSED(rxConfig);

    rxRuntimeConfig->channelCount = JETIEXBUS_CHANNEL_COUNT;
    rxRuntimeConfig->rcReadRawFn = jetiExBusReadRawRC;
    rxRuntimeConfig->rcFrameStatusFn = jetiExBusFrameStatus;

    memset(jetiExBusChannelData, 0, sizeof(uint16_t) * JETIEXBUS_CHANNEL_COUNT);

    jetiExBusFrameReset();

    const serialPortConfig_t *portConfig = findSerialPortConfig(FUNCTION_RX_SERIAL);

    if (!portConfig) {
        return false;
    }

    jetiExBusPort = openSerialPort(portConfig->identifier,
        FUNCTION_RX_SERIAL,
        jetiExBusDataReceive,
        NULL,
        JETIEXBUS_BAUDRATE,
        MODE_RXTX,
        JETIEXBUS_OPTIONS | (rxConfig->serialrx_inverted ? SERIAL_INVERTED : 0) | SERIAL_BIDIR
        );
    return jetiExBusPort != NULL;
}
#endif // SERIAL_RX
