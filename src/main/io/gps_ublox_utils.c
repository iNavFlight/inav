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
// The CFG-SIGNAL keys INAV sets, the constellations first so that they can go out alone
static const uint32_t ubloxGnss10Keys[UBLOX_GNSS10_KEY_COUNT] = {
    UBLOX_CFG_SIGNAL_SBAS_ENA, UBLOX_CFG_SIGNAL_GAL_ENA, UBLOX_CFG_SIGNAL_BDS_ENA, UBLOX_CFG_QZSS_ENA, UBLOX_CFG_GLO_ENA,
    UBLOX_CFG_SIGNAL_SBAS_L1CA_ENA, UBLOX_CFG_SIGNAL_GAL_E1_ENA, UBLOX_CFG_SIGNAL_BDS_B1_ENA, UBLOX_CFG_SIGNAL_BDS_B1C_ENA,
    UBLOX_CFG_QZSS_L1CA_ENA, UBLOX_CFG_QZSS_L1S_ENA, UBLOX_CFG_GLO_L1_ENA,
    // A dual band receiver refuses a constellation turned on with one of its bands off
    UBLOX_CFG_SIGNAL_GAL_E5A_ENA, UBLOX_CFG_SIGNAL_BDS_B2A_ENA, UBLOX_CFG_QZSS_L5_ENA
};

uint16_t ubloxGnss10KeysFromValget(const uint8_t *payload, uint16_t length)
{
    // Value sizes by the key's size field: 1 bit and 1 byte take a byte each
    static const uint8_t valueBytes[8] = { 0, 1, 1, 2, 4, 8, 0, 0 };
    uint16_t known = 0;

    // version, layer and position, then the key-value pairs; the group fits one reply of 64
    for (uint16_t i = 4; i + 4 <= length; ) {
        uint32_t key;
        memcpy(&key, payload + i, sizeof(key));
        i += 4;

        const uint8_t bytes = valueBytes[(key >> 28) & 0x7];
        if (bytes == 0 || i + bytes > length) {
            break;
        }
        i += bytes;

        for (int k = 0; k < UBLOX_GNSS10_KEY_COUNT; k++) {
            if (key == ubloxGnss10Keys[k]) {
                known |= 1 << k;
            }
        }
    }

    return known;
}

int ubloxGnss10Pairs(ubx_config_data8_payload_t *pairs, uint8_t gnss, bool sbas, uint16_t known, bool enablesOnly)
{
    const bool galileo = gnss & UBX_MON_GNSS_GALILEO_MASK;
    const bool beidou = gnss & UBX_MON_GNSS_BEIDOU_MASK;
    const bool glonass = gnss & UBX_MON_GNSS_GLONASS_MASK;
    // A receiver with B2a takes BeiDou only as B1C + B2a, and an M10 runs Glonass only next to B1C
    const bool b2a = known & (1 << UBLOX_GNSS10_BDS_B2A);
    const bool b1c = (b2a || glonass) && (known & (1 << UBLOX_GNSS10_BDS_B1C));
    const uint8_t values[UBLOX_GNSS10_KEY_COUNT] = {
        sbas, galileo, beidou, 1, glonass,
        sbas, galileo, beidou && !b1c, beidou && b1c, 1, 1, glonass,
        galileo, beidou, 1
    };
    int count = 0;

    // A key the firmware doesn't know makes the receiver refuse the whole VALSET
    for (int k = 0; k < (enablesOnly ? UBLOX_GNSS10_ENABLES : UBLOX_GNSS10_KEY_COUNT); k++) {
        if (known & (1 << k)) {
            pairs[count].key = ubloxGnss10Keys[k];
            pairs[count].value = values[k];
            count++;
        }
    }

    return count;
}
