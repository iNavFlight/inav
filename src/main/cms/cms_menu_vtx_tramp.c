/*
 * This file is part of Cleanflight and Betaflight.
 *
 * Cleanflight and Betaflight are free software. You can redistribute
 * this software and/or modify this software under the terms of the
 * GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Cleanflight and Betaflight are distributed in the hope that they
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this software.
 *
 * If not, see <http://www.gnu.org/licenses/>.
 */

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <drivers/vtx_table.h>

#include "platform.h"

#if defined(USE_CMS) && defined(USE_VTX_TRAMP)

#include "common/printf.h"
#include "common/utils.h"

#include "cms/cms.h"
#include "cms/cms_types.h"

#include "drivers/vtx_common.h"

#include "fc/config.h"

#include "io/vtx_tramp.h"
#include "io/vtx.h"

char trampCmsStatusString[31] = "- -- ---- ----";
//                               m bc ffff tppp
//                               01234567890123

void trampCmsUpdateStatusString(void)
{
    vtxDevice_t *vtxDevice = vtxCommonDevice();

    if (vtxTableBandCount == 0 || vtxTablePowerLevels == 0) {
        strncpy(trampCmsStatusString, "PLEASE CONFIGURE VTXTABLE", sizeof(trampCmsStatusString));
        return;
    }

    trampCmsStatusString[0] = '*';
    trampCmsStatusString[1] = ' ';
    uint8_t band;
    uint8_t chan;
    if (!vtxCommonGetBandAndChannel(vtxDevice, &band, &chan) || (band == 0 && chan == 0)) {
        trampCmsStatusString[2] = 'U';//user freq
        trampCmsStatusString[3] = 'F';
    } else {
        trampCmsStatusString[2] = vtxCommonLookupBandLetter(vtxDevice, band);
        trampCmsStatusString[3] = vtxCommonLookupChannelName(vtxDevice, chan)[0];
    }
    trampCmsStatusString[4] = ' ';

    if (trampData.curFreq)
        tfp_sprintf(&trampCmsStatusString[5], "%4d", trampData.curFreq);
    else
        tfp_sprintf(&trampCmsStatusString[5], "----");

    if (trampData.power) {
        tfp_sprintf(&trampCmsStatusString[9], " %c%3d", (trampData.power == trampData.configuredPower) ? ' ' : '*', trampData.power);
    } else {
        tfp_sprintf(&trampCmsStatusString[9], " ----");
    }
}

uint8_t trampCmsPitMode = 0;
uint8_t trampCmsBand = 1;
uint8_t trampCmsChan = 1;
uint16_t trampCmsFreqRef;

static OSD_TAB_t trampCmsEntBand;

static OSD_TAB_t trampCmsEntChan;

static uint8_t trampCmsPower = 1;

static OSD_TAB_t trampCmsEntPower;

static void trampCmsUpdateFreqRef(void)
{
    if (trampCmsBand > 0 && trampCmsChan > 0)
        trampCmsFreqRef = vtxCommonLookupFrequency(vtxCommonDevice(), trampCmsBand, trampCmsChan);
}

static long trampCmsConfigBand(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (trampCmsBand == 0)
        // Bounce back
        trampCmsBand = 1;
    else
        trampCmsUpdateFreqRef();

    return 0;
}

static long trampCmsConfigChan(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (trampCmsChan == 0)
        // Bounce back
        trampCmsChan = 1;
    else
        trampCmsUpdateFreqRef();

    return 0;
}

static long trampCmsConfigPower(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (trampCmsPower == 0)
        // Bounce back
        trampCmsPower = 1;

    return 0;
}

static const char * const trampCmsPitModeNames[] = {
    "---", "OFF", "ON "
};

static const OSD_TAB_t trampCmsEntPitMode = { &trampCmsPitMode, 2, trampCmsPitModeNames };

static long trampCmsSetPitMode(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (trampCmsPitMode == 0) {
        // Bouce back
        trampCmsPitMode = 1;
    } else {
        trampSetPitMode(trampCmsPitMode - 1);
    }

    return 0;
}

static long trampCmsCommence(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    vtxDevice_t *device = vtxCommonDevice();
    vtxCommonSetBandAndChannel(device, trampCmsBand, trampCmsChan);
    vtxCommonSetPowerByIndex(device, trampCmsPower);

    // If it fails, the user should retry later
    trampCommitChanges();

    // update'vtx_' settings
    vtxSettingsConfigMutable()->band = trampCmsBand;
    vtxSettingsConfigMutable()->channel = trampCmsChan;
    vtxSettingsConfigMutable()->power = trampCmsPower;
    vtxSettingsConfigMutable()->freq = vtxCommonLookupFrequency(vtxCommonDevice(), trampCmsBand, trampCmsChan);

    saveConfigAndNotify();

    return MENU_CHAIN_BACK;
}

static bool trampCmsInitSettings(void)
{
    vtxDevice_t *device = vtxCommonDevice();

    if (!device) {
        return false;
    }

    vtxCommonGetBandAndChannel(device, &trampCmsBand, &trampCmsChan);

    trampCmsUpdateFreqRef();
    trampCmsPitMode = trampData.pitMode + 1;

    if (trampData.configuredPower > 0) {
        if (!vtxCommonGetPowerIndex(vtxCommonDevice(), &trampCmsPower)) {
            trampCmsPower = 1;
        }
    }

    trampCmsEntBand.val = &trampCmsBand;
    trampCmsEntBand.max = vtxTableBandCount;
    trampCmsEntBand.names = vtxTableBandNames;

    trampCmsEntChan.val = &trampCmsChan;
    trampCmsEntChan.max = vtxTableChannelCount;
    trampCmsEntChan.names = vtxTableChannelNames;

    trampCmsEntPower.val = &trampCmsPower;
    trampCmsEntPower.max = vtxTablePowerLevels;
    trampCmsEntPower.names = vtxTablePowerLabels;

    return true;
}

static long trampCmsOnEnter(const OSD_Entry *from)
{
    UNUSED(from);

    if (!trampCmsInitSettings()) {
        return MENU_CHAIN_BACK;
    }

    return 0;
}

static const OSD_Entry trampCmsMenuCommenceEntries[] =
{
    OSD_LABEL_ENTRY("CONFIRM"),
    OSD_FUNC_CALL_ENTRY("YES", trampCmsCommence),

    OSD_BACK_AND_END_ENTRY,
};

static const CMS_Menu trampCmsMenuCommence = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XVTXTRC",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = trampCmsMenuCommenceEntries,
};

static const OSD_Entry trampMenuEntries[] =
{
    OSD_LABEL_ENTRY("- TRAMP -"),

    OSD_LABEL_DATA_DYN_ENTRY("", trampCmsStatusString),
    OSD_TAB_CALLBACK_ENTRY("PIT", trampCmsSetPitMode, &trampCmsEntPitMode),
    OSD_TAB_CALLBACK_ENTRY("BAND", trampCmsConfigBand, &trampCmsEntBand),
    OSD_TAB_CALLBACK_ENTRY("CHAN", trampCmsConfigChan, &trampCmsEntChan),
    OSD_UINT16_RO_ENTRY("(FREQ)", &trampCmsFreqRef),
    OSD_TAB_CALLBACK_ENTRY("POWER", trampCmsConfigPower, &trampCmsEntPower),
    OSD_INT16_RO_ENTRY("T(C)", &trampData.temperature),
    OSD_SUBMENU_ENTRY("SET", &trampCmsMenuCommence),

    OSD_BACK_AND_END_ENTRY,
};

const CMS_Menu cmsx_menuVtxTramp = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XVTXTR",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = trampCmsOnEnter,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = trampMenuEntries,
};
#endif
