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


#include <stdint.h>
#include <string.h>

#include "gps_ublox_utils.h"

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