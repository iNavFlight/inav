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

#include <stdint.h>

#include "build/debug.h"

#include "common/printf.h"

#include "io/vtx_table.h"

#include "config/parameter_group.h"
#include "config/parameter_group_ids.h"

#ifdef USE_VTX_TABLE

// vtxtable band 1 BOSCAM_A A FACTORY 5865 5845 5825 5805 5785 5765 5745 5725
// vtxtable band 2 BOSCAM_B B FACTORY 5733 5752 5771 5790 5809 5828 5847 5866
// vtxtable band 3 BOSCAM_E E FACTORY 5705 5685 5665 5645 5885 5905 5925 5945
// vtxtable band 4 FATSHARK F FACTORY 5740 5760 5780 5800 5820 5840 5860 5880
// vtxtable band 5 RACEBAND R FACTORY 5658 5695 5732 5769 5806 5843 5880 5917
// vtxtable band 6 LOWBAND  L FACTORY 5333 5373 5413 5453 5493 5533 5573 5613

uint8_t mspConfMaxBands = VTX_TABLE_MAX_BANDS;
uint8_t mspConfMaxPowerLevels = VTX_TABLE_MAX_POWER_LEVELS;
/*
mspBandTable_t bandTable[VTX_TABLE_MAX_BANDS] = {
    { "BOSCAM_A", "A", true, { 5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725 } },
    { "BOSCAM_B", "B", true, { 5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866 } },
    { "BOSCAM_E", "C", true, { 5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945 } },
    { "FATSHARK", "F", true, { 5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880 } },
    { "RACEBAND", "R", true, { 5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917 } },
    { "LOWBAND ", "L", true, { 5333, 5373, 5413, 5453, 5493, 5533, 5573, 5613 } },
};

mspPowerTable_t powerTable[VTX_MSP_TABLE_MAX_POWER_LEVELS] = {
    { "0", 0 },
    { "1", 1 },
    { "2", 2 },
    { "3", 3 },
    { "4", 4 },
};
*/

PG_REGISTER_WITH_RESET_FN(vtxTableConfig_t, vtxTableConfig, PG_VTX_TABLE_CONFIG, 0);

// Prune a band to "channels"
void vtxTableConfigClearChannels(vtxTableConfig_t *config, int band, int channels)
{
    for (int channel = channels; channel < VTX_TABLE_MAX_CHANNELS; channel++) {
        config->frequency[band][channel] = 0;
    }
}

// Clear a channel name for "channel"
void vtxTableConfigClearChannelNames(vtxTableConfig_t *config, int channel)
{
    tfp_sprintf(config->channelNames[channel], "%d", channel + 1);
}

void vtxTableConfigClearBand(vtxTableConfig_t *config, int band)
{
    vtxTableConfigClearChannels(config, band, 0);
    for (int channel = 0; channel < VTX_TABLE_MAX_CHANNELS; channel++) {
        vtxTableConfigClearChannelNames(config, channel);
    }
    char tempbuf[6];
    tfp_sprintf(tempbuf, "BAND%d", band + 1);
    vtxTableStrncpyWithPad(config->bandNames[band], tempbuf, VTX_TABLE_BAND_NAME_LENGTH);
    config->bandLetters[band] = '1' + band;
    config->isFactoryBand[band] = false;
}

void vtxTableConfigClearPowerValues(vtxTableConfig_t *config, int start)
{
    for (int i = start; i < VTX_TABLE_MAX_POWER_LEVELS; i++) {
        config->powerValues[i] = 0;
    }
}

void vtxTableConfigClearPowerLabels(vtxTableConfig_t *config, int start)
{
    for (int i = start; i < VTX_TABLE_MAX_POWER_LEVELS; i++) {
        char tempbuf[4];
        tfp_sprintf(tempbuf, "LV%d", i);
        vtxTableStrncpyWithPad(config->powerLabels[i], tempbuf, VTX_TABLE_POWER_LABEL_LENGTH);
    }
}

void pgResetFn_vtxTableConfig(vtxTableConfig_t *config)
{
    // Clear band names, letters and frequency values

    config->bands = 0;
    config->channels = 0;

    for (int band = 0; band < VTX_TABLE_MAX_BANDS; band++) {
        vtxTableConfigClearBand(config, band);
    }

    // Clear power values and labels

    config->powerLevels = 0;
    vtxTableConfigClearPowerValues(config, 0);
    vtxTableConfigClearPowerLabels(config, 0);
}

void vtxTableStrncpyWithPad(char *dst, const char *src, int length)
{
    char c;

    while (length && (c = *src++)) {
        *dst++ = c;
        length--;
    }

    while (length--) {
        *dst++ = ' ';
    }

    *dst = 0;
}


uint8_t vtxTableGetMaxBands(void)
{
    return mspConfMaxBands;
}

uint8_t vtxTableGetMaxPowerLevels(void)
{
    return mspConfMaxPowerLevels;
}

void vtxTableSetMaxBands(uint8_t bands)
{
    mspConfMaxBands = bands;
}

void vtxTableSetMaxPowerLevels(uint8_t powerLevels)
{
    mspConfMaxPowerLevels = powerLevels;
}



#endif