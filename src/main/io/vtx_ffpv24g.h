/*
 * This file is part of INAV Project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 */

#pragma once

#include "platform.h"

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>

#include "io/vtx.h"
#include "io/vtx_control.h"

#define VTX_FFPV_BAND_COUNT         2
#define VTX_FFPV_CHANNEL_COUNT      8
#define VTX_FFPV_POWER_COUNT        4

extern const char * ffpvBandLetters;
extern const char * const ffpvBandNames[VTX_FFPV_BAND_COUNT + 1];
extern const char * const ffpvChannelNames[VTX_FFPV_CHANNEL_COUNT + 1];
extern const char * const ffpvPowerNames[VTX_FFPV_POWER_COUNT + 1];
extern const uint16_t ffpvFrequencyTable[VTX_FFPV_BAND_COUNT][VTX_FFPV_CHANNEL_COUNT];

bool vtxFuriousFPVInit(void);
void ffpvSetBandAndChannel(uint8_t band, uint8_t channel);
void ffpvSetRFPowerByIndex(uint16_t index);

vtxRunState_t * ffpvGetRuntimeState(void);