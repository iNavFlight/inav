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


#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "gps_ublox_utils.h"

#define UBLOX_PROTVER(major, minor)         ((uint16_t)((major) * 100 + (minor)))

#define UBLOX_PROTVER_NAV_RATE              UBLOX_PROTVER(15, 0)
#define UBLOX_PROTVER_GNSS_CONFIG           UBLOX_PROTVER(15, 0)
#define UBLOX_PROTVER_LAST_BEFORE_VALSET    UBLOX_PROTVER(23, 1)
#define UBLOX_PROTVER_FAST_MEAS_RATE        UBLOX_PROTVER(24, 0)
// Below this the documented firmware lacks the BDS_B1C signal key, and M9/F9P still take CFG-GNSS
#define UBLOX_PROTVER_VALSET_ONLY           UBLOX_PROTVER(34, 0)

void ublox_update_checksum(uint8_t *data, uint8_t len, uint8_t *ck_a, uint8_t *ck_b)
{
    *ck_a = *ck_b = 0;
    while (len--) {
        *ck_a += *data;
        *ck_b += *ck_a;
        data++;
    }
}

int ubloxCfgFillBytes(ubx_config_data8_t *cfg, ubx_config_data8_payload_t *kvPairs, uint8_t count)
{
    if (count > MAX_CONFIG_SET_VAL_VALUES)
        count = MAX_CONFIG_SET_VAL_VALUES;

    cfg->header.preamble1 = 0xb5;
    cfg->header.preamble2 = 0x62;
    cfg->header.msg_class = 0x06;
    cfg->header.msg_id = 0x8A;
    cfg->header.length = sizeof(ubx_config_data_header_v1_t) + ((sizeof(ubx_config_data8_payload_t) * count));
    cfg->configHeader.layers = 0x1;
    cfg->configHeader.transaction = 0;
    cfg->configHeader.reserved = 0;
    cfg->configHeader.version = 1;
    

    for (int i = 0; i < count; ++i) {
        cfg->data.payload[i].key = kvPairs[i].key;
        cfg->data.payload[i].value = kvPairs[i].value;
    }

    uint8_t *buf = (uint8_t *)cfg;
    uint8_t ck_a, ck_b;
    ublox_update_checksum(buf + 2, cfg->header.length + 4, &ck_a, &ck_b);
    buf[cfg->header.length + 6] = ck_a;
    buf[cfg->header.length + 7] = ck_b;

    return count;
}

int ubloxCfgFillU2(ubx_config_data16_t *cfg, ubx_config_data16_payload_t *kvPairs, uint8_t count)
{
    if (count > MAX_CONFIG_SET_VAL_VALUES)
        count = MAX_CONFIG_SET_VAL_VALUES;

    cfg->header.preamble1 = 0xb5;
    cfg->header.preamble2 = 0x62;
    cfg->header.msg_class = 0x06;
    cfg->header.msg_id = 0x8A;
    cfg->header.length = sizeof(ubx_config_data_header_v1_t) + ((sizeof(ubx_config_data16_payload_t) * count));
    cfg->configHeader.layers = 0x1;
    cfg->configHeader.transaction = 0;
    cfg->configHeader.reserved = 0;
    cfg->configHeader.version = 1;

    for (int i = 0; i < count; ++i) {
        cfg->data.payload[i].key = kvPairs[i].key;
        cfg->data.payload[i].value = kvPairs[i].value;
    }

    uint8_t *buf = (uint8_t *)cfg;
    uint8_t ck_a, ck_b;
    ublox_update_checksum(buf + 2, cfg->header.length + 4, &ck_a, &ck_b);
    buf[cfg->header.length + 6] = ck_a;
    buf[cfg->header.length + 7] = ck_b;

    return count;
}

void ubloxNavSat2NavSig(const ubx_nav_svinfo_channel *navSat, ubx_nav_sig_info *navSig)
{
    memset(navSig, 0, sizeof(ubx_nav_sig_info));
    navSig->svId = navSat->svId;
    navSig->gnssId = navSat->gnssId;
    navSig->cno = navSat->cno;
    navSig->prRes = navSat->prRes;
    navSig->quality = navSat->flags & (BIT(0)|BIT(1)|BIT(2));
    navSig->sigFlags = (navSat->flags >> 4) & (BIT(0)|BIT(1));  // Healthy, not healthy
    // non-converted items:
    //uint8_t sigId;    // signal ID 
    //uint8_t freqId;   // 0-13 slot +, 0-13, glonass only
    //uint8_t corrSource; // Correction source: 0 = no correction, 1 = SBAS, 2 = BeiDou, 3 = RTCM2, 4 = RTCM3 OSR, 5 = RTCM3 SSR, 6 = QZSS SLAS, 7 = SPARTN
    //uint8_t ionoModel;  // 0 = no mode, 1 = Klobuchar GPS, 2 = SBAS, 3 = Klobuchar BeiDou, 8 = Iono derived from dual frequency observations
    //uint16_t sigFlags;
                        // bit2: pseudorange smoothed,
                        // bit3: pseudorange used,
                        // bit4: carrioer range used;
                        // bit5: doppler used
                        // bit6: pseudorange corrections used
                        // bit7: carrier correction used
                        // bit8: doper corrections used
    //uint8_t reserved[4];
}

uint8_t ubloxDecodeHardwareVersion(const char *field, size_t len)
{
    // ublox_5   hwVersion 00040005
    if (strncmp(field, "00040005", len) == 0) {
        return UBX_HW_VERSION_UBLOX5;
    }

    // ublox_6   hwVersion 00040007
    if (strncmp(field, "00040007", len) == 0) {
        return UBX_HW_VERSION_UBLOX6;
    }

    // ublox_7   hwVersion 00070000
    if (strncmp(field, "00070000", len) == 0) {
        return UBX_HW_VERSION_UBLOX7;
    }

    // ublox_M8  hwVersion 00080000
    if (strncmp(field, "00080000", len) == 0) {
        return UBX_HW_VERSION_UBLOX8;
    }

    // ublox_M9  hwVersion 00190000
    if (strncmp(field, "00190000", len) == 0) {
        return UBX_HW_VERSION_UBLOX9;
    }

    // ublox_M10 hwVersion 000A0000
    if (strncmp(field, "000A0000", len) == 0) {
        return UBX_HW_VERSION_UBLOX10;
    }

    // ublox_X20 hwVersion 000B0000
    if (strncmp(field, "000B0000", len) == 0) {
        return UBX_HW_VERSION_UBLOX20;
    }

    return UBX_HW_VERSION_UNKNOWN;
}

bool ubloxParseProtocolVersion(const char *field, size_t len, uint8_t *major, uint8_t *minor)
{
    // Parsed digit by digit because a float round-trip turns 34.10 into 34.09
    if (len < 14 || (strncmp(field, "PROTVER=", 8) && strncmp(field, "PROTVER ", 8))) {
        return false;
    }
    if (!isdigit((unsigned char)field[8]) || !isdigit((unsigned char)field[9]) || field[10] != '.' ||
        !isdigit((unsigned char)field[11]) || !isdigit((unsigned char)field[12]) || field[13] != '\0') {
        return false;
    }

    *major = (field[8] - '0') * 10 + field[9] - '0';
    *minor = (field[11] - '0') * 10 + field[12] - '0';
    return true;
}

bool ubloxCanConfigureNavRate(uint8_t hwVersion, uint8_t protMajor, uint8_t protMinor)
{
    return hwVersion >= UBX_HW_VERSION_UBLOX7 || UBLOX_PROTVER(protMajor, protMinor) >= UBLOX_PROTVER_NAV_RATE;
}

bool ubloxCanConfigureGnss(uint8_t hwVersion, uint8_t protMajor, uint8_t protMinor)
{
    return hwVersion >= UBX_HW_VERSION_UBLOX8 || UBLOX_PROTVER(protMajor, protMinor) >= UBLOX_PROTVER_GNSS_CONFIG;
}

bool ubloxUseM10GnssKeys(uint8_t hwVersion, uint8_t protMajor, uint8_t protMinor)
{
    const uint16_t protocol = UBLOX_PROTVER(protMajor, protMinor);
    // Unknown hardware stays on CFG-GNSS wherever its firmware still accepts it
    return protocol > UBLOX_PROTVER_LAST_BEFORE_VALSET &&
        (hwVersion >= UBX_HW_VERSION_UBLOX10 || (hwVersion == UBX_HW_VERSION_UNKNOWN && protocol >= UBLOX_PROTVER_VALSET_ONLY));
}

uint8_t ubloxNavHzFor(uint8_t hwVersion, uint8_t protMajor, uint8_t protMinor, uint8_t configuredHz)
{
    if (!ubloxCanConfigureNavRate(hwVersion, protMajor, protMinor)) {
        return 5;
    }

    const uint16_t minMeasPeriodMs = UBLOX_PROTVER(protMajor, protMinor) >= UBLOX_PROTVER_FAST_MEAS_RATE ? 25 : 50;
    return MIN(configuredHz, 1000 / minMeasPeriodMs);
}
