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

#include <stdint.h>

// 5.8 GHz
#define VTX_TRAMP_5G8_BAND_COUNT        5
#define VTX_TRAMP_5G8_CHANNEL_COUNT     8

#define VTX_TRAMP_5G8_MAX_POWER_COUNT   5
#define VTX_TRAMP_5G8_DEFAULT_POWER     1

#define VTX_TRAMP_5G8_MIN_FREQUENCY_MHZ 5000             //min freq in MHz
#define VTX_TRAMP_5G8_MAX_FREQUENCY_MHZ 5999             //max freq in MHz

// 1.3 GHz
#define VTX_TRAMP_1G3_BAND_COUNT        2
#define VTX_TRAMP_1G3_CHANNEL_COUNT     8

#define VTX_TRAMP_1G3_MAX_POWER_COUNT   3
#define VTX_TRAMP_1G3_DEFAULT_POWER     1

#define VTX_TRAMP_1G3_MIN_FREQUENCY_MHZ 1000
#define VTX_TRAMP_1G3_MAX_FREQUENCY_MHZ 1399

bool vtxTrampInit(void);
