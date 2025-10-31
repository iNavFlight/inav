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
#include <stdlib.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <math.h>
#include <stdarg.h>

#include "platform.h"
#include "build/build_config.h"

#if defined(USE_GPS) && defined(USE_GPS_PROTO_UBLOX)

#include "build/debug.h"


#include "common/axis.h"
#include "common/typeconversion.h"
#include "common/gps_conversion.h"
#include "common/maths.h"
#include "common/utils.h"

#include "drivers/serial.h"
#include "drivers/time.h"

#include "fc/config.h"
#include "fc/runtime_config.h"
#include "fc/settings.h"
#include "fc/cli.h"

#include "io/serial.h"
#include "io/gps.h"
#include "io/gps_private.h"

#include "scheduler/protothreads.h"

#include "gps_ublox.h"
#include "gps_ublox_utils.h"


// SBAS_AUTO, SBAS_EGNOS, SBAS_WAAS, SBAS_MSAS, SBAS_GAGAN, SBAS_NONE
// note PRNs last upadted 2023-08-10
// https://www.gps.gov/technical/prn-codes/L1-CA-PRN-code-assignments-2023-Apr.pdf

#define SBASMASK1_BASE 120
#define SBASMASK1_BITS(prn) (1 << (prn-SBASMASK1_BASE))

static const uint32_t ubloxScanMode1[] = {
    0x00000000, // AUTO
    (SBASMASK1_BITS(121) | SBASMASK1_BITS(123) | SBASMASK1_BITS(126) | SBASMASK1_BITS(136) | SBASMASK1_BITS(150)), // SBAS_EGNOS
    (SBASMASK1_BITS(131) | SBASMASK1_BITS(133) | SBASMASK1_BITS(135) | SBASMASK1_BITS(138)), // WAAS

    (SBASMASK1_BITS(129) | SBASMASK1_BITS(137) | SBASMASK1_BITS(139)), // MSAS

    (SBASMASK1_BITS(127) | SBASMASK1_BITS(128) | SBASMASK1_BITS(132)), // GAGAN
    (SBASMASK1_BITS(122)), // SPAN
    0x00000000, // NONE
};

static const char * baudInitDataNMEA[GPS_BAUDRATE_COUNT] = {
    "$PUBX,41,1,0003,0001,115200,0*1E\r\n",     // GPS_BAUDRATE_115200
    "$PUBX,41,1,0003,0001,57600,0*2D\r\n",      // GPS_BAUDRATE_57600
    "$PUBX,41,1,0003,0001,38400,0*26\r\n",      // GPS_BAUDRATE_38400
    "$PUBX,41,1,0003,0001,19200,0*23\r\n",      // GPS_BAUDRATE_19200
    "$PUBX,41,1,0003,0001,9600,0*16\r\n",       // GPS_BAUDRATE_9600
    "$PUBX,41,1,0003,0001,230400,0*1C\r\n",     // GPS_BAUDRATE_230400
    "$PUBX,41,1,0003,0001,460800,0*13\r\n",     // GPS_BAUDRATE_460800
    "$PUBX,41,1,0003,0001,921600,0*15\r\n"      // GPS_BAUDRATE_921600
};

static ubx_nav_sig_info satelites[UBLOX_MAX_SIGNALS] = {};

// Packet checksum accumulators
static uint8_t _ck_a;
static uint8_t _ck_b;

// State machine state
static bool _skip_packet;
static uint8_t _step;
static uint8_t _msg_id;
static uint16_t _payload_length;
static uint16_t _payload_counter;

static uint8_t next_fix_type;
static uint8_t _class;
static uint8_t _ack_state;
static uint8_t _ack_waiting_msg;

// do we have new position information?
static bool _new_position;

// do we have new speed information?
static bool _new_speed;

// Need this to determine if Galileo capable only
static struct {
    uint8_t supported;
    int capMaxGnss;
    uint8_t defaultGnss;
    uint8_t enabledGnss;
} ubx_capabilities = { };

// Example packet sizes from UBlox u-center from a Glonass capable GPS receiver.
//15:17:55  R -> UBX NAV-STATUS,  Size  24,  'Navigation Status'
//15:17:55  R -> UBX NAV-POSLLH,  Size  36,  'Geodetic Position'
//15:17:55  R -> UBX NAV-VELNED,  Size  44,  'Velocity in WGS 84'
//15:17:55  R -> UBX NAV-CLOCK,  Size  28,  'Clock Status'
//15:17:55  R -> UBX NAV-AOPSTATUS,  Size  24,  'AOP Status'
//15:17:55  R -> UBX 03-09,  Size 208,  'Unknown'
//15:17:55  R -> UBX 03-10,  Size 336,  'Unknown'
//15:17:55  R -> UBX NAV-SOL,  Size  60,  'Navigation Solution'
//15:17:55  R -> UBX NAV,  Size 100,  'Navigation'
//15:17:55  R -> UBX NAV-SVINFO,  Size 328,  'Satellite Status and Information'


// Send buffer
static union send_buffer_t {
    ubx_message message;
    uint8_t bytes[58];
} send_buffer;

// Receive buffer
static union {
    ubx_nav_posllh posllh;
    ubx_nav_status status;
    ubx_nav_solution solution;
    ubx_nav_velned velned;
    ubx_nav_pvt pvt;
    ubx_nav_svinfo svinfo;
    ubx_mon_ver ver;
    ubx_nav_timeutc timeutc;
    ubx_ack_ack ack;
    ubx_mon_gnss gnss;
    ubx_nav_sig navsig;
    uint8_t bytes[UBLOX_BUFFER_SIZE];
} _buffer;

bool gpsUbloxHasGalileo(void)
{
    return (ubx_capabilities.supported & UBX_MON_GNSS_GALILEO_MASK);
}

bool gpsUbloxHasBeidou(void)
{
    return ubx_capabilities.supported & UBX_MON_GNSS_BEIDOU_MASK;
}

bool gpsUbloxHasGlonass(void)
{
    return ubx_capabilities.supported & UBX_MON_GNSS_GLONASS_MASK;
}

bool gpsUbloxGalileoDefault(void)
{
    return ubx_capabilities.defaultGnss & UBX_MON_GNSS_GALILEO_MASK;
}

bool gpsUbloxBeidouDefault(void)
{
    return ubx_capabilities.defaultGnss & UBX_MON_GNSS_BEIDOU_MASK;
}

bool gpsUbloxGlonassDefault(void)
{
    return ubx_capabilities.defaultGnss & UBX_MON_GNSS_GLONASS_MASK;
}

bool gpsUbloxGalileoEnabled(void)
{
    return ubx_capabilities.enabledGnss & UBX_MON_GNSS_GALILEO_MASK;
}

bool gpsUbloxBeidouEnabled(void)
{
    return ubx_capabilities.enabledGnss & UBX_MON_GNSS_BEIDOU_MASK;
}

bool gpsUbloxGlonassEnabled(void)
{
    return ubx_capabilities.enabledGnss & UBX_MON_GNSS_GLONASS_MASK;
}

uint8_t gpsUbloxMaxGnss(void)
{
    return ubx_capabilities.capMaxGnss;
}

timeMs_t gpsUbloxCapLastUpdate(void)
{
    return gpsState.lastCapaUpdMs;
}

static uint8_t gpsMapFixType(bool fixValid, uint8_t ubloxFixType)
{
    if (fixValid && ubloxFixType == FIX_2D)
        return GPS_FIX_2D;
    if (fixValid && ubloxFixType == FIX_3D)
        return GPS_FIX_3D;
    return GPS_NO_FIX;
}

bool gpsUbloxSendCommand(uint8_t *rawCommand, uint16_t commandLen, uint16_t timeout)
{
    UNUSED(timeout);

    serialWriteBuf(gpsState.gpsPort, rawCommand, commandLen);

    union send_buffer_t *sb = (union send_buffer_t *)(rawCommand);

    _ack_waiting_msg = sb->message.header.msg_id;
    _ack_state = UBX_ACK_WAITING;

    return true;
}

static void sendConfigMessageUBLOX(void)
{
    uint8_t ck_a=0, ck_b=0;
    send_buffer.message.header.preamble1=PREAMBLE1;
    send_buffer.message.header.preamble2=PREAMBLE2;
    ublox_update_checksum(&send_buffer.bytes[2], send_buffer.message.header.length+4, &ck_a, &ck_b);
    send_buffer.bytes[send_buffer.message.header.length+6] = ck_a;
    send_buffer.bytes[send_buffer.message.header.length+7] = ck_b;
    serialWriteBuf(gpsState.gpsPort, send_buffer.bytes, send_buffer.message.header.length+8);

    // Save state for ACK waiting
    _ack_waiting_msg = send_buffer.message.header.msg_id;
    _ack_state = UBX_ACK_WAITING;
}

static void pollVersion(void)
{
    send_buffer.message.header.msg_class = CLASS_MON;
    send_buffer.message.header.msg_id = MSG_VER;
    send_buffer.message.header.length = 0;
    sendConfigMessageUBLOX();
}

static void pollGnssCapabilities(void)
{
    send_buffer.message.header.msg_class = CLASS_MON;
    send_buffer.message.header.msg_id = MSG_MON_GNSS;
    send_buffer.message.header.length = 0;
    sendConfigMessageUBLOX();
}


static const uint8_t default_payload[] = {
    0xFF, 0xFF, 0x03, 0x03, 0x00,           // CFG-NAV5 - Set engine settings (original MWII code)
    0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00,           // Collected by resetting a GPS unit to defaults. Changing mode to Pedistrian and
    0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x3C, 0x00, 0x00, 0x00,           // capturing the data from the U-Center binary console.
    0x00, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

#define GNSSID_SBAS 1
#define GNSSID_GALILEO 2
#define GNSSID_BEIDOU   3
#define GNSSID_GZSS     5
#define GNSSID_GLONASS  6

// M10 ublox protocol info:
// https://content.u-blox.com/sites/default/files/u-blox-M10-SPG-5.10_InterfaceDescription_UBX-21035062.pdf
static void ubloxSendSetCfgBytes(ubx_config_data8_payload_t *kvPairs, uint8_t count)
{
    ubx_config_data8_t cfg = {};

    ubloxCfgFillBytes(&cfg, kvPairs, count);

    serialWriteBuf(gpsState.gpsPort, (uint8_t *)&cfg, cfg.header.length+8);
    _ack_waiting_msg = cfg.header.msg_id;
    _ack_state = UBX_ACK_WAITING;
}

// M10 ublox protocol info:
// https://content.u-blox.com/sites/default/files/u-blox-M10-SPG-5.10_InterfaceDescription_UBX-21035062.pdf
static void ubloxSendSetCfgU2(ubx_config_data16_payload_t *kvPairs, uint8_t count)
{
    ubx_config_data16_t cfg = {};

    ubloxCfgFillU2(&cfg, kvPairs, count);

    serialWriteBuf(gpsState.gpsPort, (uint8_t *)&cfg, cfg.header.length+8);
    _ack_waiting_msg = cfg.header.msg_id;
    _ack_state = UBX_ACK_WAITING;
}

// Info on protocol used by M8-M9, check UBX-CFG-GNSS for gnss configuration
// https://content.u-blox.com/sites/default/files/products/documents/u-blox8-M8_ReceiverDescrProtSpec_UBX-13003221.pdf
// https://content.u-blox.com/sites/default/files/documents/u-blox-F9-HPG-1.32_InterfaceDescription_UBX-22008968.pdf
static int configureGNSS_SBAS(ubx_gnss_element_t * gnss_block)
{
    gnss_block->gnssId = GNSSID_SBAS;
    gnss_block->maxTrkCh = 3;
    gnss_block->sigCfgMask = 1;
    if (gpsState.gpsConfig->sbasMode == SBAS_NONE) {
         gnss_block->enabled = 0;
         gnss_block->resTrkCh = 0;
    } else {
         gnss_block->enabled = 1;
         gnss_block->resTrkCh = 1;
    }

    return 1;
}

static int configureGNSS_GALILEO(ubx_gnss_element_t * gnss_block)
{
    if (!gpsUbloxHasGalileo()) {
        return 0;
    }

    gnss_block->gnssId = GNSSID_GALILEO;
    gnss_block->maxTrkCh = 8;
    // sigCfgMask
    // 0x01 = Galileo E1 (not supported for protocol versions less than 18.00)
    // 0x10 = Galileo E5a // off by default
    // 0x20 = Galileo E5b // off by default
    gnss_block->sigCfgMask = 0x01;
    if (gpsState.gpsConfig->ubloxUseGalileo) {
        gnss_block->enabled = 1;
        gnss_block->resTrkCh = 4;
    } else {
        gnss_block->enabled = 0;
        gnss_block->resTrkCh = 0;
    }

    return 1;
}

static int configureGNSS_BEIDOU(ubx_gnss_element_t * gnss_block)
{
    if (!gpsUbloxHasBeidou()) {
        return 0;
    }

    gnss_block->gnssId = GNSSID_BEIDOU;
    gnss_block->maxTrkCh = 8;
    // sigCfgMask
    // 0x01 = BeiDou B1I
    // 0x10 = BeiDou B2I // off by default
    // 0x80 = BeiDou B2A // off by default
    gnss_block->sigCfgMask = 0x01;
    if (gpsState.gpsConfig->ubloxUseBeidou) {
        gnss_block->enabled = 1;
        gnss_block->resTrkCh = 4;
    } else {
        gnss_block->enabled = 0;
        gnss_block->resTrkCh = 0;
    }

    return 1;
}

/*
static int configureGNSS_GZSS(ubx_gnss_element_t * gnss_block)
{
    if (!ubx_capabilities.capGzss) {
        return 0;
    }

    gnss_block->gnssId = GNSSID_GZSS;
    gnss_block->maxTrkCh = 8;
    // L1C = 0x01
    // L1S = 0x04
    // L2C = 0x10
    gnss_block->sigCfgMask = 0x01 | 0x04;
    gnss_block->enabled = 1;
    gnss_block->resTrkCh = 4;

    return 1;
}
*/

static int configureGNSS_GLONASS(ubx_gnss_element_t * gnss_block)
{
    if(!gpsUbloxHasGlonass()) {
        return 0;
    }

    gnss_block->gnssId = GNSSID_GLONASS;
    gnss_block->maxTrkCh = 8;
    // 0x01 = GLONASS L1
    // 0x10 = GLONASS L2 // off by default
    gnss_block->sigCfgMask = 0x01;
    if (gpsState.gpsConfig->ubloxUseGlonass) {
        gnss_block->enabled = 1;
        gnss_block->resTrkCh = 4;
    } else {
        gnss_block->enabled = 0;
        gnss_block->resTrkCh = 0;
    }

    return 1;
}

static void configureGNSS10(void)
{
        ubx_config_data8_payload_t gnssConfigValues[] = {
            // SBAS
            {UBLOX_CFG_SIGNAL_SBAS_ENA, gpsState.gpsConfig->sbasMode == SBAS_NONE ? 0 : 1},
            {UBLOX_CFG_SIGNAL_SBAS_L1CA_ENA, gpsState.gpsConfig->sbasMode == SBAS_NONE ? 0 : 1},

            // Galileo
            {UBLOX_CFG_SIGNAL_GAL_ENA, gpsState.gpsConfig->ubloxUseGalileo},
            {UBLOX_CFG_SIGNAL_GAL_E1_ENA, gpsState.gpsConfig->ubloxUseGalileo},

            // Beidou
            // M10 can't use BDS_B1I and Glonass together. Instead, use BDS_B1C
            {UBLOX_CFG_SIGNAL_BDS_ENA, gpsState.gpsConfig->ubloxUseBeidou},
            {UBLOX_CFG_SIGNAL_BDS_B1_ENA, gpsState.gpsConfig->ubloxUseBeidou && ! gpsState.gpsConfig->ubloxUseGlonass},
            {UBLOX_CFG_SIGNAL_BDS_B1C_ENA, gpsState.gpsConfig->ubloxUseBeidou && gpsState.gpsConfig->ubloxUseGlonass},

            // Should be enabled with GPS
            {UBLOX_CFG_QZSS_ENA, 1},
            {UBLOX_CFG_QZSS_L1CA_ENA, 1},
            {UBLOX_CFG_QZSS_L1S_ENA, 1},

            // Glonass
            {UBLOX_CFG_GLO_ENA, gpsState.gpsConfig->ubloxUseGlonass},
            {UBLOX_CFG_GLO_L1_ENA, gpsState.gpsConfig->ubloxUseGlonass}
        };

        ubloxSendSetCfgBytes(gnssConfigValues, 12);
}

static void configureGNSS(void)
{
    int blocksUsed = 0;
    send_buffer.message.header.msg_class = CLASS_CFG;
    send_buffer.message.header.msg_id = MSG_CFG_GNSS; // message deprecated in protocol > 23.01, should use UBX-CFG-VALSET/UBX-CFG-VALGET
    send_buffer.message.payload.gnss.msgVer = 0;
    send_buffer.message.payload.gnss.numTrkChHw = 0; // read only, so unset
    send_buffer.message.payload.gnss.numTrkChUse = 0xFF; // If set to 0xFF will use hardware max

    /* SBAS, always generated */
    blocksUsed += configureGNSS_SBAS(&send_buffer.message.payload.gnss.config[blocksUsed]);

    /* Galileo */
    blocksUsed += configureGNSS_GALILEO(&send_buffer.message.payload.gnss.config[blocksUsed]);

    /* BeiDou */
    blocksUsed += configureGNSS_BEIDOU(&send_buffer.message.payload.gnss.config[blocksUsed]);

    /* GLONASS */
    blocksUsed += configureGNSS_GLONASS(&send_buffer.message.payload.gnss.config[blocksUsed]);

    send_buffer.message.payload.gnss.numConfigBlocks = blocksUsed;
    send_buffer.message.header.length = (sizeof(ubx_gnss_msg_t) + sizeof(ubx_gnss_element_t)* blocksUsed);
    sendConfigMessageUBLOX();
}

static void configureNAV5(uint8_t dynModel, uint8_t fixMode)
{
    send_buffer.message.header.msg_class = CLASS_CFG;
    send_buffer.message.header.msg_id = MSG_CFG_NAV_SETTINGS;
    send_buffer.message.header.length = 0x24;
    memcpy(send_buffer.message.payload.bytes, default_payload, sizeof(default_payload));
    send_buffer.message.payload.bytes[2] = dynModel;
    send_buffer.message.payload.bytes[3] = fixMode;
    sendConfigMessageUBLOX();
}

static void configureMSG(uint8_t msg_class, uint8_t id, uint8_t rate)
{
    send_buffer.message.header.msg_class = CLASS_CFG;
    send_buffer.message.header.msg_id = MSG_CFG_SET_RATE;
    send_buffer.message.header.length = 3;
    send_buffer.message.payload.msg.msg_class = msg_class;
    send_buffer.message.payload.msg.id = id;
    send_buffer.message.payload.msg.rate = rate;
    sendConfigMessageUBLOX();
}

/*
 * measRate in ms
 * navRate cycles
 * timeRef 0 UTC, 1 GPS
 */
static void configureRATE(uint16_t measRate)
{
    if(ubloxVersionLT(24, 0)) {
        measRate = MAX(50, measRate);
    } else {
        measRate = MAX(25, measRate);
    }

    if (ubloxVersionLTE(23, 1)) {
        send_buffer.message.header.msg_class = CLASS_CFG;
        send_buffer.message.header.msg_id = MSG_CFG_RATE;
        send_buffer.message.header.length = 6;
        send_buffer.message.payload.rate.meas = measRate;
        send_buffer.message.payload.rate.nav = 1;
        send_buffer.message.payload.rate.time = 1;
        sendConfigMessageUBLOX();
    } else { // M10+
        // 1 is already default, for TIMEREF.
        // The wait the configuration happens,
        // it is tricky to wait for multiple commands.
        // SendSetCfg could be refactored to support U1, U2, U3 and U4 messages
        // at the same time. For now, leave it out.
        //
        //ubx_config_data8_payload_t rateValues[] = {
        //    {UBLOX_CFG_RATE_TIMEREF, 1}, // 0
        //};
        //ubloxSendSetCfgBytes(rateValues, 1);

        ubx_config_data16_payload_t rate16Values[] = {
            {UBLOX_CFG_RATE_MEAS, measRate},
            {UBLOX_CFG_RATE_NAV, 1}
        };
        ubloxSendSetCfgU2(rate16Values, 2);
    }
}

/*
 */
static void configureSBAS(void)
{
    send_buffer.message.header.msg_class = CLASS_CFG;
    send_buffer.message.header.msg_id = MSG_CFG_SBAS;
    send_buffer.message.header.length = 8;
    send_buffer.message.payload.sbas.mode=(gpsState.gpsConfig->sbasMode == SBAS_NONE?2:3);
    send_buffer.message.payload.sbas.usage=3;
    send_buffer.message.payload.sbas.maxSBAS=3;
    send_buffer.message.payload.sbas.scanmode2=0;
    send_buffer.message.payload.sbas.scanmode1=ubloxScanMode1[gpsState.gpsConfig->sbasMode];
    sendConfigMessageUBLOX();
}

static void gpsDecodeProtocolVersion(const char *proto, size_t bufferLength)
{
    if (bufferLength > 13 && (!strncmp(proto, "PROTVER=", 8) || !strncmp(proto, "PROTVER ", 8))) {
        proto+=8;

        float ver = atof(proto);

        gpsState.swVersionMajor = (uint8_t)ver;
        gpsState.swVersionMinor = (uint8_t)((ver - gpsState.swVersionMajor) * 100.0f);
    }
}

static uint32_t gpsDecodeHardwareVersion(const char * szBuf, unsigned nBufSize)
{
    // ublox_5   hwVersion 00040005
    if (strncmp(szBuf, "00040005", nBufSize) == 0) {
        return UBX_HW_VERSION_UBLOX5;
    }

    // ublox_6   hwVersion 00040007
    if (strncmp(szBuf, "00040007", nBufSize) == 0) {
        return UBX_HW_VERSION_UBLOX6;
    }

    // ublox_7   hwVersion 00070000
    if (strncmp(szBuf, "00070000", nBufSize) == 0) {
        return UBX_HW_VERSION_UBLOX7;
    }

    // ublox_M8  hwVersion 00080000
    if (strncmp(szBuf, "00080000", nBufSize) == 0) {
        return UBX_HW_VERSION_UBLOX8;
    }

    // ublox_M9  hwVersion 00190000
    if (strncmp(szBuf, "00190000", nBufSize) == 0) {
        return UBX_HW_VERSION_UBLOX9;
    }

    // ublox_M10 hwVersion 000A0000
    if (strncmp(szBuf, "000A0000", nBufSize) == 0) {
        return UBX_HW_VERSION_UBLOX10;
    }

    return UBX_HW_VERSION_UNKNOWN;
}

static bool gpsParseFrameUBLOX(void)
{
    switch (_msg_id) {
    case MSG_POSLLH:
        gpsSolDRV.llh.lon = _buffer.posllh.longitude;
        gpsSolDRV.llh.lat = _buffer.posllh.latitude;
        gpsSolDRV.llh.alt = _buffer.posllh.altitude_msl / 10;  //alt in cm
        gpsSolDRV.eph = gpsConstrainEPE(_buffer.posllh.horizontal_accuracy / 10);
        gpsSolDRV.epv = gpsConstrainEPE(_buffer.posllh.vertical_accuracy / 10);
        gpsSolDRV.flags.validEPE = true;
        if (next_fix_type != GPS_NO_FIX)
            gpsSolDRV.fixType = next_fix_type;
        _new_position = true;
        break;
    case MSG_STATUS:
        next_fix_type = gpsMapFixType(_buffer.status.fix_status & NAV_STATUS_FIX_VALID, _buffer.status.fix_type);
        if (next_fix_type == GPS_NO_FIX)
            gpsSolDRV.fixType = GPS_NO_FIX;
        break;
    case MSG_SOL:
        next_fix_type = gpsMapFixType(_buffer.solution.fix_status & NAV_STATUS_FIX_VALID, _buffer.solution.fix_type);
        if (next_fix_type == GPS_NO_FIX)
            gpsSolDRV.fixType = GPS_NO_FIX;
        gpsSolDRV.numSat = _buffer.solution.satellites;
        gpsSolDRV.hdop = gpsConstrainHDOP(_buffer.solution.position_DOP);
        break;
    case MSG_VELNED:
        gpsSolDRV.groundSpeed = _buffer.velned.speed_2d;    // cm/s
        gpsSolDRV.groundCourse = (uint16_t) (_buffer.velned.heading_2d / 10000);     // Heading 2D deg * 100000 rescaled to deg * 10
        gpsSolDRV.velNED[X] = _buffer.velned.ned_north;
        gpsSolDRV.velNED[Y] = _buffer.velned.ned_east;
        gpsSolDRV.velNED[Z] = _buffer.velned.ned_down;
        gpsSolDRV.flags.validVelNE = true;
        gpsSolDRV.flags.validVelD = true;
        _new_speed = true;
        break;
    case MSG_TIMEUTC:
        if (UBX_VALID_GPS_DATE_TIME(_buffer.timeutc.valid)) {
            gpsSolDRV.time.year = _buffer.timeutc.year;
            gpsSolDRV.time.month = _buffer.timeutc.month;
            gpsSolDRV.time.day = _buffer.timeutc.day;
            gpsSolDRV.time.hours = _buffer.timeutc.hour;
            gpsSolDRV.time.minutes = _buffer.timeutc.min;
            gpsSolDRV.time.seconds = _buffer.timeutc.sec;
            gpsSolDRV.time.millis = _buffer.timeutc.nano / (1000*1000);

            gpsSolDRV.flags.validTime = true;
        } else {
            gpsSolDRV.flags.validTime = false;
        }
        break;
    case MSG_PVT:
        {
            static int pvtCount = 0;
            DEBUG_SET(DEBUG_GPS, 0, pvtCount++);
        }

        gpsState.flags.pvt = 1;
        next_fix_type = gpsMapFixType(_buffer.pvt.fix_status & NAV_STATUS_FIX_VALID, _buffer.pvt.fix_type);
        gpsSolDRV.fixType = next_fix_type;
        gpsSolDRV.llh.lon = _buffer.pvt.longitude;
        gpsSolDRV.llh.lat = _buffer.pvt.latitude;
        gpsSolDRV.llh.alt = _buffer.pvt.altitude_msl / 10;  //alt in cm
        gpsSolDRV.velNED[X]=_buffer.pvt.ned_north / 10;  // to cm/s
        gpsSolDRV.velNED[Y]=_buffer.pvt.ned_east / 10;   // to cm/s
        gpsSolDRV.velNED[Z]=_buffer.pvt.ned_down / 10;   // to cm/s
        gpsSolDRV.groundSpeed = _buffer.pvt.speed_2d / 10;    // to cm/s
        gpsSolDRV.groundCourse = (uint16_t) (_buffer.pvt.heading_2d / 10000);     // Heading 2D deg * 100000 rescaled to deg * 10
        gpsSolDRV.numSat = _buffer.pvt.satellites;
        gpsSolDRV.eph = gpsConstrainEPE(_buffer.pvt.horizontal_accuracy / 10);
        gpsSolDRV.epv = gpsConstrainEPE(_buffer.pvt.vertical_accuracy / 10);
        gpsSolDRV.hdop = gpsConstrainHDOP(_buffer.pvt.position_DOP);
        gpsSolDRV.flags.validVelNE = true;
        gpsSolDRV.flags.validVelD = true;
        gpsSolDRV.flags.validEPE = true;

        if (UBX_VALID_GPS_DATE_TIME(_buffer.pvt.valid)) {
            gpsSolDRV.time.year = _buffer.pvt.year;
            gpsSolDRV.time.month = _buffer.pvt.month;
            gpsSolDRV.time.day = _buffer.pvt.day;
            gpsSolDRV.time.hours = _buffer.pvt.hour;
            gpsSolDRV.time.minutes = _buffer.pvt.min;
            gpsSolDRV.time.seconds = _buffer.pvt.sec;
            gpsSolDRV.time.millis = _buffer.pvt.nano / (1000*1000);

            gpsSolDRV.flags.validTime = true;
        } else {
            gpsSolDRV.flags.validTime = false;
        }

        _new_position = true;
        _new_speed = true;
        break;
    case MSG_VER:
        if (_class == CLASS_MON) {
            gpsState.hwVersion = gpsDecodeHardwareVersion(_buffer.ver.hwVersion, sizeof(_buffer.ver.hwVersion));
            if (gpsState.hwVersion >= UBX_HW_VERSION_UBLOX8) {
                if (_buffer.ver.swVersion[9] > '2' || true) {
                    // check extensions;
                    // after hw + sw vers; each is 30 bytes
                    bool found = false;
                    for (int j = 40; j < _payload_length && !found; j += 30)
                    {
                        // Example content: GPS;GAL;BDS;GLO
                        if (strnstr((const char *)(_buffer.bytes + j), "GAL", 30))
                        {
                            ubx_capabilities.supported |= UBX_MON_GNSS_GALILEO_MASK;
                            found = true;
                        }
                        if (strnstr((const char *)(_buffer.bytes + j), "BDS", 30))
                        {
                            ubx_capabilities.supported |= UBX_MON_GNSS_BEIDOU_MASK;
                            found = true;
                        }
                        if (strnstr((const char *)(_buffer.bytes + j), "GLO", 30))
                        {
                            ubx_capabilities.supported |= UBX_MON_GNSS_GLONASS_MASK;
                            found = true;
                        }
                    }
                }
                for(int j = 40; j < _payload_length; j += 30) {
                    if (strnstr((const char *)(_buffer.bytes + j), "PROTVER", 30)) {
                        gpsDecodeProtocolVersion((const char *)(_buffer.bytes + j), 30);
                        break;
                    }
                }
            }
        }
        break;
    case MSG_MON_GNSS:
        if(_class == CLASS_MON) {
            if (_buffer.gnss.version == 0) {
                ubx_capabilities.supported = _buffer.gnss.supported;
                ubx_capabilities.defaultGnss = _buffer.gnss.defaultGnss;
                ubx_capabilities.enabledGnss = _buffer.gnss.enabled;
                ubx_capabilities.capMaxGnss = _buffer.gnss.maxConcurrent;
                gpsState.lastCapaUpdMs = millis();
            }
        }
        break;
    case MSG_NAV_SAT:
        if (_class == CLASS_NAV) {
            static int satInfoCount = 0;
            gpsState.flags.sat = 1;
            DEBUG_SET(DEBUG_GPS, 1, satInfoCount++);
            DEBUG_SET(DEBUG_GPS, 3, _buffer.svinfo.numSvs);
            if (!gpsState.flags.pvt) { // PVT is the prefered source
                gpsSolDRV.numSat = _buffer.svinfo.numSvs;
            }

            for(int i = 0; i < MIN(_buffer.svinfo.numSvs, UBLOX_MAX_SIGNALS); ++i) {
                ubloxNavSat2NavSig(&_buffer.svinfo.channel[i], &satelites[i]);
            }
            for(int i =_buffer.svinfo.numSvs; i < UBLOX_MAX_SIGNALS; ++i) {
                satelites[i].gnssId = 0xFF;
                satelites[i].svId = 0xFF;
            }
        }
        break;
    case MSG_NAV_SIG:
        if (_class == CLASS_NAV && _buffer.navsig.version == 0) {
            static int sigInfoCount = 0;
            DEBUG_SET(DEBUG_GPS, 2, sigInfoCount++);
            DEBUG_SET(DEBUG_GPS, 4, _buffer.navsig.numSigs);
            gpsState.flags.sig = 1;

            if(_buffer.navsig.numSigs > 0) 
            {
                for(int i=0; i < MIN(UBLOX_MAX_SIGNALS, _buffer.navsig.numSigs); ++i)
                {
                    memcpy(&satelites[i], &_buffer.navsig.sig[i], sizeof(ubx_nav_sig_info));
                }
                for(int i = _buffer.navsig.numSigs; i < UBLOX_MAX_SIGNALS; ++i)
                {
                    satelites[i].svId = 0xFF; // no used
                    satelites[i].gnssId = 0xFF;
                }
            }
        }
        break;
    case MSG_ACK_ACK:
        if ((_ack_state == UBX_ACK_WAITING) && (_buffer.ack.msg == _ack_waiting_msg)) {
            _ack_state = UBX_ACK_GOT_ACK;
        }
        break;
    case MSG_ACK_NACK:
        if ((_ack_state == UBX_ACK_WAITING) && (_buffer.ack.msg == _ack_waiting_msg)) {
            _ack_state = UBX_ACK_GOT_NAK;
        }
        break;
    default:
        return false;
    }

    DEBUG_SET(DEBUG_GPS, 5, gpsState.flags.pvt);
    DEBUG_SET(DEBUG_GPS, 6, gpsState.flags.sat);
    DEBUG_SET(DEBUG_GPS, 7, gpsState.flags.sig);

    // we only return true when we get new position and speed data
    // this ensures we don't use stale data
    if (_new_position && _new_speed) {
        _new_speed = _new_position = false;
        return true;
    }

    return false;
}

static bool gpsNewFrameUBLOX(uint8_t data)
{
    bool parsed = false;

    switch (_step) {
        case 0: // Sync char 1 (0xB5)
            if (PREAMBLE1 == data) {
                _skip_packet = false;
                _step++;
            }
            break;
        case 1: // Sync char 2 (0x62)
            if (PREAMBLE2 != data) {
                _step = 0;
                break;
            }
            _step++;
            break;
        case 2: // Class
            _step++;
            _class = data;
            _ck_b = _ck_a = data;   // reset the checksum accumulators
            break;
        case 3: // Id
            _step++;
            _ck_b += (_ck_a += data);       // checksum byte
            _msg_id = data;
            break;
        case 4: // Payload length (part 1)
            _step++;
            _ck_b += (_ck_a += data);       // checksum byte
            _payload_length = data; // payload length low byte
            break;
        case 5: // Payload length (part 2)
            _step++;
            _ck_b += (_ck_a += data);       // checksum byte
            _payload_length |= (uint16_t)(data << 8);
            if (_payload_length > MAX_UBLOX_PAYLOAD_SIZE ) {
                // we can't receive the whole packet, just log the error and start searching for the next packet.
                gpsStats.errors++;
                _step = 0;
                break;
            }
            // prepare to receive payload
            _payload_counter = 0;
            if (_payload_length == 0) {
                _step = 7;
            }
            break;
        case 6:
            _ck_b += (_ck_a += data);       // checksum byte
            if (_payload_counter < MAX_UBLOX_PAYLOAD_SIZE) {
                _buffer.bytes[_payload_counter] = data;
            }
            // NOTE: check counter BEFORE increasing so that a payload_size of 65535 is correctly handled.  This can happen if garbage data is received.
            if (_payload_counter ==  _payload_length - 1) {
                _step++;
            }
            _payload_counter++;
            break;
        case 7:
            _step++;
            if (_ck_a != data) {
                _skip_packet = true;          // bad checksum
                gpsStats.errors++;
                _step = 0;
            }
            break;
        case 8:
            _step = 0;

            if (_ck_b != data) {
                gpsStats.errors++;
                break;              // bad checksum
            }

            gpsStats.packetCount++;

            if (_skip_packet) {
                break;
            }

            if (gpsParseFrameUBLOX()) {
                parsed = true;
            }
    }

    return parsed;
}

static uint16_t hz2rate(uint8_t hz)
{
    return 1000 / hz;
}

STATIC_PROTOTHREAD(gpsConfigure)
{
    ptBegin(gpsConfigure);

    // Reset timeout
    gpsSetProtocolTimeout(GPS_SHORT_TIMEOUT);

    // Set dynamic model
    if (ubloxVersionGT(23, 1)) {
        ubx_config_data8_payload_t dynmodelCfg[] = {
            {UBLOX_CFG_NAVSPG_DYNMODEL, UBX_DYNMODEL_AIR_2G},
            {UBLOX_CFG_NAVSPG_FIXMODE, UBX_FIXMODE_AUTO}
        };

        switch (gpsState.gpsConfig->dynModel) {
            case GPS_DYNMODEL_PEDESTRIAN:
                dynmodelCfg[0].value = UBX_DYNMODEL_PEDESTRIAN;
                ubloxSendSetCfgBytes(dynmodelCfg, 2);
                break;
            case GPS_DYNMODEL_AUTOMOTIVE:
                dynmodelCfg[0].value = UBX_DYNMODEL_AUTOMOVITE;
                ubloxSendSetCfgBytes(dynmodelCfg, 2);
                break;
            case GPS_DYNMODEL_AIR_1G:
                dynmodelCfg[0].value = UBX_DYNMODEL_AIR_1G;
                ubloxSendSetCfgBytes(dynmodelCfg, 2);
                break;
            case GPS_DYNMODEL_AIR_2G:  // Default to this
            default:
                dynmodelCfg[0].value = UBX_DYNMODEL_AIR_2G;
                ubloxSendSetCfgBytes(dynmodelCfg, 2);
                break;
            case GPS_DYNMODEL_AIR_4G:
                dynmodelCfg[0].value = UBX_DYNMODEL_AIR_4G;
                ubloxSendSetCfgBytes(dynmodelCfg, 2);
                break;
            case GPS_DYNMODEL_SEA:
                dynmodelCfg[0].value = UBX_DYNMODEL_SEA;
                ubloxSendSetCfgBytes(dynmodelCfg, 2);
                break;
            case GPS_DYNMODEL_MOWER:
                dynmodelCfg[0].value = UBX_DYNMODEL_MOWER;
                ubloxSendSetCfgBytes(dynmodelCfg, 2);
                break;
        }
        ptWait(_ack_state == UBX_ACK_GOT_ACK);
    } else {
        switch (gpsState.gpsConfig->dynModel) {
            case GPS_DYNMODEL_PEDESTRIAN:
                configureNAV5(UBX_DYNMODEL_PEDESTRIAN, UBX_FIXMODE_AUTO);
                break;
            case GPS_DYNMODEL_AUTOMOTIVE:
                configureNAV5(UBX_DYNMODEL_AUTOMOVITE, UBX_FIXMODE_AUTO);
                break;
            case GPS_DYNMODEL_AIR_1G:
                configureNAV5(UBX_DYNMODEL_AIR_1G, UBX_FIXMODE_AUTO);
                break;
            case GPS_DYNMODEL_AIR_2G:  // Default to this
            default:
                configureNAV5(UBX_DYNMODEL_AIR_2G, UBX_FIXMODE_AUTO);
                break;
            case GPS_DYNMODEL_AIR_4G:
                configureNAV5(UBX_DYNMODEL_AIR_4G, UBX_FIXMODE_AUTO);
                break;
            case GPS_DYNMODEL_SEA:
                configureNAV5(UBX_DYNMODEL_SEA, UBX_FIXMODE_AUTO);
                break;
            case GPS_DYNMODEL_MOWER:
                configureNAV5(UBX_DYNMODEL_MOWER, UBX_FIXMODE_AUTO);
                break;
        }
        ptWait(_ack_state == UBX_ACK_GOT_ACK);
    }

    gpsSetProtocolTimeout(GPS_SHORT_TIMEOUT);
    // Disable NMEA messages
    if (ubloxVersionGT(23, 1)) {
        ubx_config_data8_payload_t nmeaValues[] = {
            { UBLOX_CFG_MSGOUT_NMEA_ID_GGA_UART1, 0 },
            { UBLOX_CFG_MSGOUT_NMEA_ID_GLL_UART1, 0 },
            { UBLOX_CFG_MSGOUT_NMEA_ID_GSA_UART1, 0 },
            { UBLOX_CFG_MSGOUT_NMEA_ID_RMC_UART1, 0 },
            { UBLOX_CFG_MSGOUT_NMEA_ID_VTG_UART1, 0 },
        };

        ubloxSendSetCfgBytes(nmeaValues, 5);
        ptWaitTimeout((_ack_state == UBX_ACK_GOT_ACK), GPS_CFG_CMD_TIMEOUT_MS);
    } else {
        configureMSG(MSG_CLASS_NMEA, MSG_NMEA_GGA, 0);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_NMEA, MSG_NMEA_GLL, 0);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_NMEA, MSG_NMEA_GSA, 0);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_NMEA, MSG_NMEA_GSV, 0);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_NMEA, MSG_NMEA_RMC, 0);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_NMEA, MSG_NMEA_VGS, 0);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);
    }

    // Configure UBX binary messages
    gpsSetProtocolTimeout(GPS_SHORT_TIMEOUT);

    // M9N & M10 does not support some of the UBX 6/7/8 messages, so we have to configure it using special sequence
    if (ubloxVersionGT(23, 1)) { // M9+, new setting API, PVT and NAV_SIG
        ubx_config_data8_payload_t rateValues[] = {
            {UBLOX_CFG_MSGOUT_NAV_POSLLH_UART1, 0}, // 0
            {UBLOX_CFG_MSGOUT_NAV_STATUS_UART1, 0}, // 1
            {UBLOX_CFG_MSGOUT_NAV_VELNED_UART1, 0}, // 2
            {UBLOX_CFG_MSGOUT_NAV_TIMEUTC_UART1, 0}, // 3
            {UBLOX_CFG_MSGOUT_NAV_PVT_UART1, 1}, // 4
            {UBLOX_CFG_MSGOUT_NAV_SIG_UART1, 1}, // 5
            {UBLOX_CFG_MSGOUT_NAV_SAT_UART1, 0}  // 6
        };

        ubloxSendSetCfgBytes(rateValues, 7);
        ptWaitTimeout((_ack_state == UBX_ACK_GOT_ACK || _ack_state == UBX_ACK_GOT_NAK), GPS_CFG_CMD_TIMEOUT_MS);
    } else if(ubloxVersionGTE(15,0)) { // M8, PVT, NAV_SAT, old setting API
        configureMSG(MSG_CLASS_UBX, MSG_POSLLH, 0);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_UBX, MSG_STATUS, 0);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_UBX, MSG_SOL, 0);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_UBX, MSG_VELNED, 0);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_UBX, MSG_TIMEUTC, 0);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_UBX, MSG_SVINFO, 0);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_UBX, MSG_PVT, 1);
        ptWait(_ack_state == UBX_ACK_GOT_ACK || _ack_state == UBX_ACK_GOT_NAK);

        configureMSG(MSG_CLASS_UBX, MSG_NAV_SAT, 1);
        ptWait(_ack_state == UBX_ACK_GOT_ACK || _ack_state == UBX_ACK_GOT_NAK);
    } else { // Really old stuff, consider upgrading :), ols setting API, no PVT or NAV_SAT or NAV_SIG
        // TODO: remove in INAV 9.0.0
        configureMSG(MSG_CLASS_UBX, MSG_POSLLH, 1);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_UBX, MSG_STATUS, 1);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_UBX, MSG_SOL, 1);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_UBX, MSG_VELNED, 1);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_UBX, MSG_TIMEUTC, 10);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);

        configureMSG(MSG_CLASS_UBX, MSG_SVINFO, 0);
        ptWait(_ack_state == UBX_ACK_GOT_ACK);
    }// end message config

    ptWaitTimeout((_ack_state == UBX_ACK_GOT_ACK || _ack_state == UBX_ACK_GOT_NAK), GPS_SHORT_TIMEOUT);
    if ((gpsState.hwVersion >= UBX_HW_VERSION_UBLOX7)) {
        configureRATE(hz2rate(gpsState.gpsConfig->ubloxNavHz)); // default 10Hz
    } else {
        configureRATE(hz2rate(5)); // 5Hz
        gpsConfigMutable()->ubloxNavHz = SETTING_GPS_UBLOX_NAV_HZ_DEFAULT;
    }
    ptWait(_ack_state == UBX_ACK_GOT_ACK || _ack_state == UBX_ACK_GOT_NAK);

    if(_ack_state == UBX_ACK_GOT_NAK) { // Fallback to safe 5Hz in case of error
        configureRATE(hz2rate(5)); // 5Hz
        ptWait(_ack_state == UBX_ACK_GOT_ACK);
    }


    gpsState.flags.pvt = 0;
    gpsState.flags.sat = 0;
    gpsState.flags.sig = 0;

    // Configure SBAS
    // If particular SBAS setting is not supported by the hardware we'll get a NAK,
    // however GPS would be functional. We are waiting for any response - ACK/NACK
    gpsSetProtocolTimeout(GPS_SHORT_TIMEOUT);
    configureSBAS();
    ptWaitTimeout((_ack_state == UBX_ACK_GOT_ACK || _ack_state == UBX_ACK_GOT_NAK), GPS_CFG_CMD_TIMEOUT_MS);

    // Configure GNSS for M8N and later
    if (gpsState.hwVersion >= UBX_HW_VERSION_UBLOX8) { // TODO: This check can be remove in INAV 9.0.0
        gpsSetProtocolTimeout(GPS_SHORT_TIMEOUT);
        bool use_VALSET = 0;
        if (ubloxVersionGT(23,1)) {
            use_VALSET = 1;
        }

        if ( use_VALSET && (gpsState.hwVersion >= UBX_HW_VERSION_UBLOX10) ) {
            configureGNSS10();
        } else {
            configureGNSS();
        }

        ptWaitTimeout((_ack_state == UBX_ACK_GOT_ACK || _ack_state == UBX_ACK_GOT_NAK), GPS_CFG_CMD_TIMEOUT_MS);

        if(_ack_state == UBX_ACK_GOT_NAK) {
            gpsConfigMutable()->ubloxUseGalileo = SETTING_GPS_UBLOX_USE_GALILEO_DEFAULT;
            gpsConfigMutable()->ubloxUseBeidou = SETTING_GPS_UBLOX_USE_BEIDOU_DEFAULT;
            gpsConfigMutable()->ubloxUseGlonass = SETTING_GPS_UBLOX_USE_GLONASS_DEFAULT;
        }
    }

	for(int i = 0; i < UBLOX_MAX_SIGNALS; ++i)
	{
        // Mark satelites as unused
        satelites[i].svId = 0xFF;
        satelites[i].gnssId = 0xFF;
	}

    ptEnd(0);
}

static ptSemaphore_t semNewDataReady;

STATIC_PROTOTHREAD(gpsProtocolReceiverThread)
{
    ptBegin(gpsProtocolReceiverThread);

    while (1) {
        // Wait until there are bytes to consume
        ptWait(serialRxBytesWaiting(gpsState.gpsPort));

        // Consume bytes until buffer empty of until we have full message received
        while (serialRxBytesWaiting(gpsState.gpsPort)) {
            uint8_t newChar = serialRead(gpsState.gpsPort);
            if (gpsNewFrameUBLOX(newChar)) {
                gpsProcessNewDriverData();
                ptSemaphoreSignal(semNewDataReady);
                break;
            }
        }
    }

    ptEnd(0);
}

STATIC_PROTOTHREAD(gpsProtocolStateThread)
{
    ptBegin(gpsProtocolStateThread);

    // Change baud rate
    if (gpsState.gpsConfig->autoBaud != GPS_AUTOBAUD_OFF) {
        //  0. Wait for TX buffer to be empty
        ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));

        // Try sending baud rate switch command at all common baud rates
        gpsSetProtocolTimeout((GPS_BAUD_CHANGE_DELAY + 50) * (GPS_BAUDRATE_COUNT));
        for (gpsState.autoBaudrateIndex = 0; gpsState.autoBaudrateIndex < GPS_BAUDRATE_COUNT; gpsState.autoBaudrateIndex++) {
            if (gpsBaudRateToInt(gpsState.autoBaudrateIndex) > gpsBaudRateToInt(gpsState.gpsConfig->autoBaudMax)) {
                // trying higher baud rates fails on m8 gps
                // autoBaudRateIndex is not sorted by baud rate
                continue;
            }
            // 2. Set serial port to baud rate and send an $UBX command to switch the baud rate specified by portConfig [baudrateIndex]
            serialSetBaudRate(gpsState.gpsPort, baudRates[gpsToSerialBaudRate[gpsState.autoBaudrateIndex]]);
            serialPrint(gpsState.gpsPort, baudInitDataNMEA[gpsState.baudrateIndex]);

            // 3. Wait for serial port to finish transmitting
            ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));

            // 4. Extra wait to make sure GPS processed the command
            ptDelayMs(GPS_BAUD_CHANGE_DELAY);
        }
        serialSetBaudRate(gpsState.gpsPort, baudRates[gpsToSerialBaudRate[gpsState.baudrateIndex]]);
    }
    else {
        // No auto baud - set port baud rate to [baudrateIndex]
        // Wait for TX buffer to be empty
        ptWait(isSerialTransmitBufferEmpty(gpsState.gpsPort));

        // Set baud rate and reset GPS timeout
        serialSetBaudRate(gpsState.gpsPort, baudRates[gpsToSerialBaudRate[gpsState.baudrateIndex]]);
    }

    // Reset protocol timeout
    gpsSetProtocolTimeout(MAX(GPS_TIMEOUT, ((GPS_VERSION_RETRY_TIMES + 3) * GPS_CFG_CMD_TIMEOUT_MS)));

    // Attempt to detect GPS hw version
    gpsState.hwVersion = UBX_HW_VERSION_UNKNOWN;
    gpsState.autoConfigStep = 0;

    // Configure GPS module if enabled
    if (gpsState.gpsConfig->autoConfig) {
        do {
            pollVersion();
            gpsState.autoConfigStep++;
            ptWaitTimeout((gpsState.hwVersion != UBX_HW_VERSION_UNKNOWN), GPS_CFG_CMD_TIMEOUT_MS);
        } while(gpsState.autoConfigStep < GPS_VERSION_RETRY_TIMES && gpsState.hwVersion == UBX_HW_VERSION_UNKNOWN);

        gpsState.autoConfigStep = 0;
        ubx_capabilities.supported = ubx_capabilities.enabledGnss = ubx_capabilities.defaultGnss = 0;
        // M7 and earlier will never get pass this step, so skip it (#9440).
        // UBLOX documents that this is M8N and later
        if (gpsState.hwVersion > UBX_HW_VERSION_UBLOX7) {
            do {
                pollGnssCapabilities();
                gpsState.autoConfigStep++;
                ptWaitTimeout((ubx_capabilities.capMaxGnss != 0), GPS_CFG_CMD_TIMEOUT_MS);
            } while (gpsState.autoConfigStep < GPS_VERSION_RETRY_TIMES && ubx_capabilities.capMaxGnss == 0);
        }

        // Configure GPS
        ptSpawn(gpsConfigure);
    }

    // GPS setup done, reset timeout
    gpsSetProtocolTimeout(gpsState.baseTimeoutMs);

    // GPS is ready - execute the gpsProcessNewSolutionData() based on gpsProtocolReceiverThread semaphore
    while (1) {
        ptSemaphoreWait(semNewDataReady);
        gpsProcessNewSolutionData(false);

        if (gpsState.gpsConfig->autoConfig) {
            if ((millis() - gpsState.lastCapaPoolMs) > GPS_CAPA_INTERVAL) {
                gpsState.lastCapaPoolMs = millis();

                if (gpsState.hwVersion == UBX_HW_VERSION_UNKNOWN)
                {
                    pollVersion();
                    ptWaitTimeout((_ack_state == UBX_ACK_GOT_ACK || _ack_state == UBX_ACK_GOT_NAK), GPS_CFG_CMD_TIMEOUT_MS);
                }

                pollGnssCapabilities();
                ptWaitTimeout((_ack_state == UBX_ACK_GOT_ACK || _ack_state == UBX_ACK_GOT_NAK), GPS_CFG_CMD_TIMEOUT_MS);
            }
        }
    }

    ptEnd(0);
}

void gpsRestartUBLOX(void)
{
	for(int i = 0; i < UBLOX_MAX_SIGNALS; ++i)
	{
        memset(&satelites[i], 0, sizeof(ubx_nav_sig_info));
		satelites[i].svId = 0xFF;
		satelites[i].gnssId = 0xFF;
	}

    ptSemaphoreInit(semNewDataReady);
    ptRestart(ptGetHandle(gpsProtocolReceiverThread));
    ptRestart(ptGetHandle(gpsProtocolStateThread));
}

void gpsHandleUBLOX(void)
{
    // Run the protocol threads
    gpsProtocolReceiverThread();
    gpsProtocolStateThread();

    // If thread stopped - signal communication loss and restart
    if (ptIsStopped(ptGetHandle(gpsProtocolReceiverThread)) || ptIsStopped(ptGetHandle(gpsProtocolStateThread))) {
        gpsSetState(GPS_LOST_COMMUNICATION);
    }
}

bool isGpsUblox(void)
{
    if(gpsState.gpsPort != NULL && (gpsState.gpsConfig->provider == GPS_UBLOX)) {
        return true;
    }

    return false;
}


const ubx_nav_sig_info *gpsGetUbloxSatelite(uint8_t index)
{
    if(index < UBLOX_MAX_SIGNALS && satelites[index].svId != 0xFF && satelites[index].gnssId != 0xFF) {
        return &satelites[index];
    }

    return NULL;
}

bool ubloxVersionLT(uint8_t mj, uint8_t mn)
{
    return gpsState.swVersionMajor < mj || (gpsState.swVersionMajor == mj && gpsState.swVersionMinor < mn);
}

bool ubloxVersionGT(uint8_t mj, uint8_t mn)
{
    return gpsState.swVersionMajor > mj || (gpsState.swVersionMajor == mj && gpsState.swVersionMinor > mn);
}

bool ubloxVersionGTE(uint8_t mj, uint8_t mn)
{
    return ubloxVersionE(mj, mn) || ubloxVersionGT(mj, mn);
}

bool ubloxVersionLTE(uint8_t mj, uint8_t mn)
{
    return ubloxVersionE(mj, mn) || ubloxVersionLT(mj, mn);
}

bool ubloxVersionE(uint8_t mj, uint8_t mn)
{
    return gpsState.swVersionMajor == mj && gpsState.swVersionMinor == mn;
}

#endif
