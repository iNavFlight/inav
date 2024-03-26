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
#include <stdbool.h>

#include "common/time.h"

#ifdef __cplusplus
extern "C" {
#endif

#define GPS_CFG_CMD_TIMEOUT_MS              500
#define GPS_VERSION_RETRY_TIMES             3
#define MAX_UBLOX_PAYLOAD_SIZE              256
#define UBLOX_BUFFER_SIZE                   MAX_UBLOX_PAYLOAD_SIZE
#define UBLOX_SBAS_MESSAGE_LENGTH           16
#define GPS_CAPA_INTERVAL                   5000

#define UBX_DYNMODEL_PEDESTRIAN 3
#define UBX_DYNMODEL_AUTOMOVITE 4
#define UBX_DYNMODEL_AIR_1G     6
#define UBX_DYNMODEL_AIR_2G     7
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


#define UBLOX_CFG_SIGNAL_SBAS_ENA       0x10310020 // U1
#define UBLOX_CFG_SIGNAL_SBAS_L1CA_ENA  0x10310005 // U1

#define UBLOX_CFG_SIGNAL_GAL_ENA        0x10310021 // U1
#define UBLOX_CFG_SIGNAL_GAL_E1_ENA     0x10310007 // U1

#define UBLOX_CFG_SIGNAL_BDS_ENA        0x10310022 // U1
#define UBLOX_CFG_SIGNAL_BDS_B1_ENA     0x1031000d // U1
#define UBLOX_CFG_SIGNAL_BDS_B1C_ENA    0x1031000f // U1 default off

#define UBLOX_CFG_QZSS_ENA              0x10310024 // U1
#define UBLOX_CFG_QZSS_L1CA_ENA         0x10310012 // U1
#define UBLOX_CFG_QZSS_L1S_ENA          0x10310014 // U1

#define UBLOX_CFG_GLO_ENA               0x10310025 // U1 default off - may conflict with other constelations
#define UBLOX_CFG_GLO_L1_ENA            0x10310018 // U1 default off

#define UBLOX_CFG_SBAS_PRNSCANMASK      0x50360006 // 0 = auto // X8
#define UBLOX_SBAS_ALL                  0x0000000000000000 //Enable search for all SBAS PRNs
#define UBLOX_SBAS_PRN120               0x0000000000000001 //Enable search for SBAS PRN120
#define UBLOX_SBAS_PRN121               0x0000000000000002 //Enable search for SBAS PRN121
#define UBLOX_SBAS_PRN122               0x0000000000000004 //Enable search for SBAS PRN122
#define UBLOX_SBAS_PRN123               0x0000000000000008 //Enable search for SBAS PRN123
#define UBLOX_SBAS_PRN124               0x0000000000000010 //Enable search for SBAS PRN124
#define UBLOX_SBAS_PRN125               0x0000000000000020 //Enable search for SBAS PRN125
#define UBLOX_SBAS_PRN126               0x0000000000000040 //Enable search for SBAS PRN126
#define UBLOX_SBAS_PRN127               0x0000000000000080 //Enable search for SBAS PRN127
#define UBLOX_SBAS_PRN128               0x0000000000000100 //Enable search for SBAS PRN128
#define UBLOX_SBAS_PRN129               0x0000000000000200 //Enable search for SBAS PRN129
#define UBLOX_SBAS_PRN130               0x0000000000000400 //Enable search for SBAS PRN130
#define UBLOX_SBAS_PRN131               0x0000000000000800 //Enable search for SBAS PRN131
#define UBLOX_SBAS_PRN132               0x0000000000001000 //Enable search for SBAS PRN132
#define UBLOX_SBAS_PRN133               0x0000000000002000 //Enable search for SBAS PRN133
#define UBLOX_SBAS_PRN134               0x0000000000004000 //Enable search for SBAS PRN134
#define UBLOX_SBAS_PRN135               0x0000000000008000 //Enable search for SBAS PRN135
#define UBLOX_SBAS_PRN136               0x0000000000010000 //Enable search for SBAS PRN136
#define UBLOX_SBAS_PRN137               0x0000000000020000 //Enable search for SBAS PRN137
#define UBLOX_SBAS_PRN138               0x0000000000040000 //Enable search for SBAS PRN138
#define UBLOX_SBAS_PRN139               0x0000000000080000 //Enable search for SBAS PRN139
#define UBLOX_SBAS_PRN140               0x0000000000100000 //Enable search for SBAS PRN140
#define UBLOX_SBAS_PRN141               0x0000000000200000 //Enable search for SBAS PRN141
#define UBLOX_SBAS_PRN142               0x0000000000400000 //Enable search for SBAS PRN142
#define UBLOX_SBAS_PRN143               0x0000000000800000 //Enable search for SBAS PRN143
#define UBLOX_SBAS_PRN144               0x0000000001000000 //Enable search for SBAS PRN144
#define UBLOX_SBAS_PRN145               0x0000000002000000 //Enable search for SBAS PRN145
#define UBLOX_SBAS_PRN146               0x0000000004000000 //Enable search for SBAS PRN146
#define UBLOX_SBAS_PRN147               0x0000000008000000 //Enable search for SBAS PRN147
#define UBLOX_SBAS_PRN148               0x0000000010000000 //Enable search for SBAS PRN148
#define UBLOX_SBAS_PRN149               0x0000000020000000 //Enable search for SBAS PRN149
#define UBLOX_SBAS_PRN150               0x0000000040000000 //Enable search for SBAS PRN150
#define UBLOX_SBAS_PRN151               0x0000000080000000 //Enable search for SBAS PRN151
#define UBLOX_SBAS_PRN152               0x0000000100000000 //Enable search for SBAS PRN152
#define UBLOX_SBAS_PRN153               0x0000000200000000 //Enable search for SBAS PRN153
#define UBLOX_SBAS_PRN154               0x0000000400000000 //Enable search for SBAS PRN154
#define UBLOX_SBAS_PRN155               0x0000000800000000 //Enable search for SBAS PRN155
#define UBLOX_SBAS_PRN156               0x0000001000000000 //Enable search for SBAS PRN156
#define UBLOX_SBAS_PRN157               0x0000002000000000 //Enable search for SBAS PRN157
#define UBLOX_SBAS_PRN158               0x0000004000000000 //Enable search for SBAS PRN158

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
} __attribute__((packed)) ubx_config_data_header_v0_t;

typedef struct {
    uint8_t version;
    uint8_t layers;
    uint8_t transaction;
    uint8_t reserved;
} __attribute__((packed)) ubx_config_data_header_v1_t;


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
} __attribute__((packed)) ubx_config_data8_payload_t;


#define MAX_CONFIG_SET_VAL_VALUES   32

typedef struct {
    ubx_header header;
    ubx_config_data_header_v1_t configHeader;
    union {
        ubx_config_data8_payload_t payload[0];
        uint8_t buffer[(MAX_CONFIG_SET_VAL_VALUES * sizeof(ubx_config_data8_payload_t)) + 2]; // 12 key/value pairs + 2 checksum bytes
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

#define UBX_MON_GNSS_GPS_MASK       (1 << 0)
#define UBX_MON_GNSS_GLONASS_MASK   (1 << 1)
#define UBX_MON_GNSS_BEIDOU_MASK    (1 << 2)
#define UBX_MON_GNSS_GALILEO_MASK   (1 << 3)

typedef struct {
    uint8_t version;
    uint8_t supported;     // bitfield for GNSS types: 0:GPS, 1:Glonass, 2:Beidou, 3:Galileo
    uint8_t defaultGnss;   // bitfield for GNSS types: 0:GPS, 1:Glonass, 2:Beidou, 3:Galileo
    uint8_t enabled;       // bitfield for GNSS types: 0:GPS, 1:Glonass, 2:Beidou, 3:Galileo
    uint8_t maxConcurrent;
    uint8_t reserverd1;
    uint8_t reserverd2;
    uint8_t reserverd3;
} ubx_mon_gnss;

typedef struct {
    uint8_t msg_class;
    uint8_t msg;
} ubx_ack_ack;

typedef enum {
    UBX_ACK_WAITING = 0,
    UBX_ACK_GOT_ACK = 1,
    UBX_ACK_GOT_NAK = 2
} ubx_ack_state_t;

typedef enum {
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
    MSG_CFG_GNSS = 0x3e,
    MSG_MON_GNSS = 0x28
} ubx_protocol_bytes_t;

typedef enum {
    FIX_NONE = 0,
    FIX_DEAD_RECKONING = 1,
    FIX_2D = 2,
    FIX_3D = 3,
    FIX_GPS_DEAD_RECKONING = 4,
    FIX_TIME = 5
} ubs_nav_fix_type_t;

typedef enum {
    NAV_STATUS_FIX_VALID = 1
} ubx_nav_status_bits_t;

uint8_t gpsUbloxMaxGnss(void);
timeMs_t gpsUbloxCapLastUpdate(void);

bool gpsUbloxHasGalileo(void);
bool gpsUbloxHasBeidou(void);
bool gpsUbloxHasGlonass(void);

bool gpsUbloxGalileoDefault(void);
bool gpsUbloxBeidouDefault(void);
bool gpsUbloxGlonassDefault(void);

bool gpsUbloxGalileoEnabled(void);
bool gpsUbloxBeidouEnabled(void);
bool gpsUbloxGlonassEnabled(void);


#ifdef __cplusplus
}
#endif
