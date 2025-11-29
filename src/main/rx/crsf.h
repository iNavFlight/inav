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

#pragma once

#define CRSF_BAUDRATE           420000
#define CRSF_PORT_OPTIONS       (SERIAL_STOPBITS_1 | SERIAL_PARITY_NO)
#define CRSF_PORT_MODE          MODE_RXTX

#define CRSF_MAX_CHANNEL        17

enum { CRSF_SYNC_BYTE = 0xC8 };

enum { CRSF_FRAME_SIZE_MAX = 64 }; // 62 bytes frame plus 2 bytes frame header(<length><type>)
enum { CRSF_PAYLOAD_SIZE_MAX = CRSF_FRAME_SIZE_MAX - 6 };

enum {
    CRSF_DISPLAYPORT_SUBCMD_UPDATE = 0x01, // transmit displayport buffer to remote
    CRSF_DISPLAYPORT_SUBCMD_CLEAR = 0X02, // clear client screen
    CRSF_DISPLAYPORT_SUBCMD_OPEN = 0x03,  // client request to open cms menu
    CRSF_DISPLAYPORT_SUBCMD_CLOSE = 0x04,  // client request to close cms menu
    CRSF_DISPLAYPORT_SUBCMD_POLL = 0x05,  // client request to poll/refresh cms menu
};

enum {
    CRSF_DISPLAYPORT_OPEN_ROWS_OFFSET = 1,
    CRSF_DISPLAYPORT_OPEN_COLS_OFFSET = 2,
};

enum {
    CRSF_FRAME_GPS_PAYLOAD_SIZE = 15,
    CRSF_FRAME_VARIO_SENSOR_PAYLOAD_SIZE = 2,
    CRSF_FRAME_BATTERY_SENSOR_PAYLOAD_SIZE = 8,
    CRSF_FRAME_LINK_STATISTICS_PAYLOAD_SIZE = 10,
    CRSF_FRAME_RC_CHANNELS_PAYLOAD_SIZE = 22, // 11 bits per channel * 16 channels = 22 bytes.
    CRSF_FRAME_ATTITUDE_PAYLOAD_SIZE = 6,
    CRSF_FRAME_LENGTH_ADDRESS = 1, // length of ADDRESS field
    CRSF_FRAME_LENGTH_FRAMELENGTH = 1, // length of FRAMELENGTH field
    CRSF_FRAME_LENGTH_TYPE = 1, // length of TYPE field
    CRSF_FRAME_LENGTH_CRC = 1, // length of CRC field
    CRSF_FRAME_LENGTH_TYPE_CRC = 2, // length of TYPE and CRC fields combined
    CRSF_FRAME_LENGTH_EXT_TYPE_CRC = 4, // length of Extended Dest/Origin, TYPE and CRC fields combined
    CRSF_FRAME_LENGTH_NON_PAYLOAD = 4, // combined length of all fields except payload
};

enum {
    CRSF_FRAME_TX_MSP_FRAME_SIZE = 58,
    CRSF_FRAME_RX_MSP_FRAME_SIZE = 8,
    CRSF_FRAME_ORIGIN_DEST_SIZE = 2,
};

// Clashes with CRSF_ADDRESS_FLIGHT_CONTROLLER
#define CRSF_TELEMETRY_SYNC_BYTE  0XC8

typedef enum {
    CRSF_ADDRESS_BROADCAST = 0x00,
    CRSF_ADDRESS_USB = 0x10,
    CRSF_ADDRESS_TBS_CORE_PNP_PRO = 0x80,
    CRSF_ADDRESS_RESERVED1 = 0x8A,
    CRSF_ADDRESS_CURRENT_SENSOR = 0xC0,
    CRSF_ADDRESS_GPS = 0xC2,
    CRSF_ADDRESS_TBS_BLACKBOX = 0xC4,
    CRSF_ADDRESS_FLIGHT_CONTROLLER = 0xC8,
    CRSF_ADDRESS_RESERVED2 = 0xCA,
    CRSF_ADDRESS_RACE_TAG = 0xCC,
    CRSF_ADDRESS_RADIO_TRANSMITTER = 0xEA,
    CRSF_ADDRESS_CRSF_RECEIVER = 0xEC,
    CRSF_ADDRESS_CRSF_TRANSMITTER = 0xEE
} crsfAddress_e;

typedef enum {
    CRSF_FRAMETYPE_GPS = 0x02,
    CRSF_FRAMETYPE_VARIO_SENSOR = 0x07,
    CRSF_FRAMETYPE_BATTERY_SENSOR = 0x08,
    CRSF_FRAMETYPE_LINK_STATISTICS = 0x14,
    CRSF_FRAMETYPE_RC_CHANNELS_PACKED = 0x16,
    CRSF_FRAMETYPE_ATTITUDE = 0x1E,
    CRSF_FRAMETYPE_FLIGHT_MODE = 0x21,
    // Extended Header Frames, range: 0x28 to 0x96
    CRSF_FRAMETYPE_DEVICE_PING = 0x28,
    CRSF_FRAMETYPE_DEVICE_INFO = 0x29,
    CRSF_FRAMETYPE_PARAMETER_SETTINGS_ENTRY = 0x2B,
    CRSF_FRAMETYPE_PARAMETER_READ = 0x2C,
    CRSF_FRAMETYPE_PARAMETER_WRITE = 0x2D,
    CRSF_FRAMETYPE_COMMAND = 0x32,
    // MSP commands
    CRSF_FRAMETYPE_MSP_REQ = 0x7A,   // response request using msp sequence as command
    CRSF_FRAMETYPE_MSP_RESP = 0x7B,  // reply with 58 byte chunked binary
    CRSF_FRAMETYPE_MSP_WRITE = 0x7C,  // write with 8 byte chunked binary (OpenTX outbound telemetry buffer limit)
    CRSF_FRAMETYPE_DISPLAYPORT_CMD = 0x7D, // displayport control command
} crsfFrameType_e;

typedef struct crsfFrameDef_s {
    uint8_t deviceAddress;
    uint8_t frameLength;
    uint8_t type;
    uint8_t payload[CRSF_PAYLOAD_SIZE_MAX + 1]; // +1 for CRC at end of payload
} crsfFrameDef_t;

typedef union crsfFrame_u {
    uint8_t bytes[CRSF_FRAME_SIZE_MAX];
    crsfFrameDef_t frame;
} crsfFrame_t;


void crsfRxWriteTelemetryData(const void *data, int len);
void crsfRxSendTelemetryData(void);

struct rxConfig_s;
struct rxRuntimeConfig_s;
bool crsfRxInit(const struct rxConfig_s *initialRxConfig, struct rxRuntimeConfig_s *rxRuntimeConfig);
bool crsfRxIsActive(void);
