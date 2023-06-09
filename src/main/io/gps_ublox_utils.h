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

#include "gps_ublox.h"

#ifdef __cplusplus
extern "C" {
#endif

int ubloxCfgFillBytes(ubx_config_data8_t *cfg, ubx_config_data8_payload_t *kvPairs, uint8_t count);

void ublox_update_checksum(uint8_t *data, uint8_t len, uint8_t *ck_a, uint8_t *ck_b);

#ifdef __cplusplus
}
#endif
