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

#include <stdbool.h>
#include <stdint.h>

#include "gps_ublox.h"

#ifdef __cplusplus
extern "C" {
#endif

int ubloxCfgFillBytes(ubx_config_data8_t *cfg, ubx_config_data8_payload_t *kvPairs, uint8_t count);
int ubloxCfgFillU2(ubx_config_data16_t *cfg, ubx_config_data16_payload_t *kvPairs, uint8_t count);

void ublox_update_checksum(uint8_t *data, uint8_t len, uint8_t *ck_a, uint8_t *ck_b);

void ubloxNavSat2NavSig(const ubx_nav_svinfo_channel *navSat, ubx_nav_sig_info *navSig);

// Which of INAV's GNSS keys the receiver firmware knows, from one CFG-VALGET of the whole CFG-SIGNAL group
#define UBLOX_CFG_SIGNAL_ALL        0x1031ffff
enum {
    UBLOX_GNSS10_GLO = 4,
    UBLOX_GNSS10_ENABLES = 5,
    UBLOX_GNSS10_BDS_B1C = 8,
    UBLOX_GNSS10_QZSS_L1S = 10,
    UBLOX_GNSS10_GLO_L1 = 11,
    UBLOX_GNSS10_GAL_E5A = 12,
    UBLOX_GNSS10_BDS_B2A = 13,
    UBLOX_GNSS10_QZSS_L5 = 14,
    UBLOX_GNSS10_KEY_COUNT = 15
};
#define UBLOX_GNSS10_ALL_KEYS       ((1 << UBLOX_GNSS10_KEY_COUNT) - 1)
#define UBLOX_GNSS10_L5_KEYS        ((1 << UBLOX_GNSS10_GAL_E5A) | (1 << UBLOX_GNSS10_BDS_B2A) | (1 << UBLOX_GNSS10_QZSS_L5))
uint16_t ubloxGnss10KeysFromValget(const uint8_t *payload, uint16_t length);
int ubloxGnss10Pairs(ubx_config_data8_payload_t *pairs, uint8_t gnss, bool sbas, uint16_t known, bool enablesOnly);

#ifdef __cplusplus
}
#endif
