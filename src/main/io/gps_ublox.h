/*
 * This file is part of INAV
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

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define GPS_CFG_CMD_TIMEOUT_MS              200
#define GPS_VERSION_RETRY_TIMES             2
#define MAX_UBLOX_PAYLOAD_SIZE              256
#define UBLOX_BUFFER_SIZE                   MAX_UBLOX_PAYLOAD_SIZE
#define UBLOX_SBAS_MESSAGE_LENGTH           16

#define UBX_DYNMODEL_PEDESTRIAN 3
#define UBX_DYNMODEL_AIR_1G     6
#define UBX_DYNMODEL_AIR_4G     8

#define UBX_FIXMODE_2D_ONLY 1
#define UBX_FIXMODE_3D_ONLY 2
#define UBX_FIXMODE_AUTO    3

#define UBX_VALID_GPS_DATE(valid) (valid & 1 << 0)
#define UBX_VALID_GPS_TIME(valid) (valid & 1 << 1)
#define UBX_VALID_GPS_DATE_TIME(valid) (UBX_VALID_GPS_DATE(valid) && UBX_VALID_GPS_TIME(valid))

#define UBX_HW_VERSION_UNKNOWN  0
#define UBX_HW_VERSION_UBLOX5   500
#define UBX_HW_VERSION_UBLOX6   600
#define UBX_HW_VERSION_UBLOX7   700
#define UBX_HW_VERSION_UBLOX8   800
#define UBX_HW_VERSION_UBLOX9   900
#define UBX_HW_VERSION_UBLOX10  1000


#define UBLOX_CFG_SIGNAL_SBAS_ENA       0x10310020
#define UBLOX_CFG_SIGNAL_SBAS_L1CA_ENA  0x10310005

#define UBLOX_CFG_SIGNAL_GAL_ENA        0x10310021
#define UBLOX_CFG_SIGNAL_GAL_E1_ENA     0x10310007

#define UBLOX_CFG_SIGNAL_BDS_ENA        0x10310022
#define UBLOX_CFG_SIGNAL_BDS_B1_ENA     0x1031000d
#define UBLOX_CFG_SIGNAL_BDS_B1C_ENA    0x1031000f // default off

#define UBLOX_CFG_QZSS_ENA              0x10310024
#define UBLOX_CFG_QZSS_L1CA_ENA         0x10310012
#define UBLOX_CFG_QZSS_L1S_ENA          0x10310014

#define UBLOX_CFG_GLO_ENA               0x10310025
#define UBLOX_CFG_GLO_L1_ENA            0x10310018

// payload types
typedef struct {
    uint8_t mode;
    uint8_t usage;
    uint8_t maxSBAS;
    uint8_t scanmode2;
    uint32_t scanmode1;
} ubx_sbas;

typedef struct {
    uint8_t msg_class;
    uint8_t id;
    uint8_t rate;
} ubx_msg;

typedef struct {
    uint16_t meas;
    uint16_t nav;
    uint16_t time;
} ubx_rate;

typedef struct {
     uint8_t gnssId;
     uint8_t resTrkCh;
     uint8_t maxTrkCh;
     uint8_t reserved1;
// flags
     uint8_t enabled;
     uint8_t undefined0;
     uint8_t sigCfgMask;
     uint8_t undefined1;
} ubx_gnss_element_t;

typedef struct {
     uint8_t msgVer;
     uint8_t numTrkChHw;
     uint8_t numTrkChUse;
     uint8_t numConfigBlocks;
     ubx_gnss_element_t config[0];
} ubx_gnss_msg_t;

typedef struct {
    uint8_t version;
    uint8_t layers;
    uint8_t reserved;
} ubx_config_data_header_t;


#define MAX_GNSS 7
#define MAX_GNSS_SIZE_BYTES (sizeof(ubx_gnss_msg_t) + sizeof(ubx_gnss_element_t)*MAX_GNSS)

typedef union {
    uint8_t bytes[MAX_GNSS_SIZE_BYTES]; // placeholder
    ubx_sbas sbas;
    ubx_msg msg;
    ubx_rate rate;
    ubx_gnss_msg_t gnss;
} ubx_payload;

// UBX support
typedef struct {
    uint8_t preamble1;
    uint8_t preamble2;
    uint8_t msg_class;
    uint8_t msg_id;
    uint16_t length;
} ubx_header;

typedef struct {
    uint32_t key;
    uint8_t value;
} ubx_config_data8_payload_t;

typedef struct {
    ubx_header header;
    ubx_config_data_header_t configHeader;
    union {
        ubx_config_data8_payload_t payload[0];
        uint8_t buffer[62]; // 12 key/value pairs + 2 checksum bytes
    } data;
} __attribute__((packed)) ubx_config_data8_t;

typedef struct {
    ubx_header header;
    ubx_payload payload;
} __attribute__((packed)) ubx_message;

typedef struct {
    char swVersion[30];      // Zero-terminated Software Version String
    char hwVersion[10];      // Zero-terminated Hardware Version String
} ubx_mon_ver;

typedef struct {
    uint32_t time;              // GPS msToW
    int32_t longitude;
    int32_t latitude;
    int32_t altitude_ellipsoid;
    int32_t altitude_msl;
    uint32_t horizontal_accuracy;
    uint32_t vertical_accuracy;
} ubx_nav_posllh;

typedef struct {
    uint32_t time;              // GPS msToW
    uint8_t fix_type;
    uint8_t fix_status;
    uint8_t differential_status;
    uint8_t res;
    uint32_t time_to_first_fix;
    uint32_t uptime;            // milliseconds
} ubx_nav_status;

typedef struct {
    uint32_t time;
    int32_t time_nsec;
    int16_t week;
    uint8_t fix_type;
    uint8_t fix_status;
    int32_t ecef_x;
    int32_t ecef_y;
    int32_t ecef_z;
    uint32_t position_accuracy_3d;
    int32_t ecef_x_velocity;
    int32_t ecef_y_velocity;
    int32_t ecef_z_velocity;
    uint32_t speed_accuracy;
    uint16_t position_DOP;
    uint8_t res;
    uint8_t satellites;
    uint32_t res2;
} ubx_nav_solution;

typedef struct {
    uint32_t time;              // GPS msToW
    int32_t ned_north;
    int32_t ned_east;
    int32_t ned_down;
    uint32_t speed_3d;
    uint32_t speed_2d;
    int32_t heading_2d;
    uint32_t speed_accuracy;
    uint32_t heading_accuracy;
} ubx_nav_velned;

typedef struct {
    uint8_t chn;                // Channel number, 255 for SVx not assigned to channel
    uint8_t svid;               // Satellite ID
    uint8_t flags;              // Bitmask
    uint8_t quality;            // Bitfield
    uint8_t cno;                // Carrier to Noise Ratio (Signal Strength) // dbHz, 0-55.
    uint8_t elev;               // Elevation in integer degrees
    int16_t azim;               // Azimuth in integer degrees
    int32_t prRes;              // Pseudo range residual in centimetres
} ubx_nav_svinfo_channel;

typedef struct {
    uint32_t time;              // GPS Millisecond time of week
    uint8_t numCh;              // Number of channels
    uint8_t globalFlags;        // Bitmask, Chip hardware generation 0:Antaris, 1:u-blox 5, 2:u-blox 6
    uint16_t reserved2;         // Reserved
    ubx_nav_svinfo_channel channel[16];         // 16 satellites * 12 byte
} ubx_nav_svinfo;

typedef struct {
    uint32_t time;              // GPS msToW
    uint32_t tAcc;
    int32_t nano;
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t valid;
} ubx_nav_timeutc;

typedef struct {
    uint32_t time; // GPS msToW
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint8_t valid;
    uint32_t tAcc;
    int32_t nano;
    uint8_t fix_type;
    uint8_t fix_status;
    uint8_t reserved1;
    uint8_t satellites;
    int32_t longitude;
    int32_t latitude;
    int32_t altitude_ellipsoid;
    int32_t altitude_msl;
    uint32_t horizontal_accuracy;
    uint32_t vertical_accuracy;
    int32_t ned_north;
    int32_t ned_east;
    int32_t ned_down;
    int32_t speed_2d;
    int32_t heading_2d;
    uint32_t speed_accuracy;
    uint32_t heading_accuracy;
    uint16_t position_DOP;
    uint16_t reserved2;
    uint16_t reserved3;
} ubx_nav_pvt;

typedef struct {
    uint8_t msg_class;
    uint8_t msg;
} ubx_ack_ack;

enum {
    PREAMBLE1 = 0xB5,
    PREAMBLE2 = 0x62,
    CLASS_NAV = 0x01,
    CLASS_ACK = 0x05,
    CLASS_CFG = 0x06,
    CLASS_MON = 0x0A,
    MSG_CLASS_UBX = 0x01,
    MSG_CLASS_NMEA = 0xF0,
    MSG_VER = 0x04,
    MSG_ACK_NACK = 0x00,
    MSG_ACK_ACK = 0x01,
    MSG_NMEA_GGA = 0x0,
    MSG_NMEA_GLL = 0x1,
    MSG_NMEA_GSA = 0x2,
    MSG_NMEA_GSV = 0x3,
    MSG_NMEA_RMC = 0x4,
    MSG_NMEA_VGS = 0x5,
    MSG_POSLLH = 0x2,
    MSG_STATUS = 0x3,
    MSG_SOL = 0x6,
    MSG_PVT = 0x7,
    MSG_VELNED = 0x12,
    MSG_TIMEUTC = 0x21,
    MSG_SVINFO = 0x30,
    MSG_NAV_SAT = 0x35,
    MSG_NAV_SIG = 0x35,
    MSG_CFG_PRT = 0x00,
    MSG_CFG_RATE = 0x08,
    MSG_CFG_SET_RATE = 0x01,
    MSG_CFG_NAV_SETTINGS = 0x24,
    MSG_CFG_SBAS = 0x16,
    MSG_CFG_GNSS = 0x3e
} ubx_protocol_bytes;

enum {
    FIX_NONE = 0,
    FIX_DEAD_RECKONING = 1,
    FIX_2D = 2,
    FIX_3D = 3,
    FIX_GPS_DEAD_RECKONING = 4,
    FIX_TIME = 5
} ubs_nav_fix_type;

enum {
    NAV_STATUS_FIX_VALID = 1
} ubx_nav_status_bits;

enum {
    UBX_ACK_WAITING = 0,
    UBX_ACK_GOT_ACK = 1,
    UBX_ACK_GOT_NAK = 2
} ubx_ack_state;

void ubloxCfgFillBytes(ubx_config_data8_t *cfg, ubx_config_data8_payload_t *kvPairs, uint8_t count);

#ifdef __cplusplus
}
#endif
