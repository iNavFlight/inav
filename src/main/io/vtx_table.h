/*
 * This file is part of BetaFlight and INAV
 *
 * INAV is free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * INAV is distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once


#include <stdint.h>

#include "platform.h"

//#include "cms/cms_menu_vtx_msp.h"
#include "common/crc.h"
#include "common/log.h"
#include "config/feature.h"

//#include "drivers/vtx_common.h"
//#include "drivers/vtx_table.h"

#include "fc/runtime_config.h"


#include "build/build_config.h"

#include "msp/msp_protocol.h"
#include "msp/msp_serial.h"


#ifdef USE_VTX_TABLE

#define VTX_TABLE_MAX_BANDS             6 // default freq table has 5 bands
#define VTX_TABLE_MAX_CHANNELS          8 // and eight channels
#define VTX_TABLE_MAX_POWER_LEVELS      5 //max of VTX_TRAMP_POWER_COUNT, VTX_SMARTAUDIO_POWER_COUNT and VTX_RTC6705_POWER_COUNT
#define VTX_TABLE_CHANNEL_NAME_LENGTH   1
#define VTX_TABLE_BAND_NAME_LENGTH      8
#define VTX_TABLE_POWER_LABEL_LENGTH    3

#define VTX_BAND_UNUSED  0
#define VTX_POWER_UNUSED 0

typedef struct vtxTableConfig_s {
    uint8_t  bands;
    uint8_t  channels;
    uint16_t frequency[VTX_TABLE_MAX_BANDS][VTX_TABLE_MAX_CHANNELS];
    char     bandNames[VTX_TABLE_MAX_BANDS][VTX_TABLE_BAND_NAME_LENGTH + 1];
    char     bandLetters[VTX_TABLE_MAX_BANDS];
    char     channelNames[VTX_TABLE_MAX_CHANNELS][VTX_TABLE_CHANNEL_NAME_LENGTH + 1];
    bool     isFactoryBand[VTX_TABLE_MAX_BANDS];

    uint8_t  powerLevels;
    uint16_t powerValues[VTX_TABLE_MAX_POWER_LEVELS];
    char     powerLabels[VTX_TABLE_MAX_POWER_LEVELS][VTX_TABLE_POWER_LABEL_LENGTH + 1];
} vtxTableConfig_t;

struct vtxTableConfig_s;
PG_DECLARE(struct vtxTableConfig_s, vtxTableConfig);

extern uint8_t mspConfMaxBands;
extern uint8_t mspConfMaxPowerLevels;

uint8_t vtxTableGetMaxBands(void);
uint8_t vtxTableGetMaxPowerLevels(void);

void vtxTableSetMaxBands(uint8_t bands);
void vtxTableSetMaxPowerLevels(uint8_t powersLevels);

void vtxTableConfigClearChannels(vtxTableConfig_t *config, int band, int channels);
void vtxTableConfigClearChannelNames(vtxTableConfig_t *config, int channel);
void vtxTableConfigClearBand(vtxTableConfig_t *config, int band);
void vtxTableConfigClearPowerValues(vtxTableConfig_t *config, int start);
void vtxTableConfigClearPowerLabels(vtxTableConfig_t *config, int start);
void vtxTableStrncpyWithPad(char *dst, const char *src, int length);


void pgResetFn_vtxTableConfig(vtxTableConfig_t *config);

#endif