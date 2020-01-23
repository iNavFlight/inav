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

#if defined(USE_CMS) && defined(USE_VTX_FFPV)

#include "common/printf.h"
#include "common/utils.h"

#include "cms/cms.h"
#include "cms/cms_types.h"

#include "drivers/vtx_common.h"

#include "fc/config.h"

#include "io/vtx_ffpv24g.h"
#include "io/vtx.h"

char ffpvCmsStatusString[31] = "- -- ---- ---";
//                              m bc ffff ppp
//                              0123456789012

void ffpvCmsUpdateStatusString(void)
{
    vtxDevice_t *vtxDevice = vtxCommonDevice();

    if (vtxTableBandCount == 0 || vtxTablePowerLevels == 0) {
        strncpy(ffpvCmsStatusString, "PLEASE CONFIGURE VTXTABLE", sizeof(ffpvCmsStatusString));
        return;
    }

    ffpvCmsStatusString[0] = '*';
    ffpvCmsStatusString[1] = ' ';
    uint8_t band;
    uint8_t chan;
    vtxCommonGetBandAndChannel(vtxDevice, &band, &chan);
    ffpvCmsStatusString[2] = vtxCommonLookupBandLetter(vtxDevice, band);
    ffpvCmsStatusString[3] = vtxCommonLookupChannelName(vtxDevice, chan)[0];
    ffpvCmsStatusString[4] = ' ';

    tfp_sprintf(&ffpvCmsStatusString[5], "%4d", ffpvGetRuntimeState()->frequency);
    ffpvCmsStatusString[9] = ' ';

    uint8_t powerIndex = 0;
    bool powerFound = vtxCommonGetPowerIndex(vtxDevice, &powerIndex);
    tfp_sprintf(&ffpvCmsStatusString[10], "%s", powerFound ? vtxCommonLookupPowerName(vtxDevice, powerIndex) : "???");

    return;
}

uint8_t ffpvCmsBand = 1;
uint8_t ffpvCmsChan = 1;
uint16_t ffpvCmsFreqRef;
static uint8_t ffpvCmsPower = 1;

static OSD_TAB_t ffpvCmsEntBand;
static OSD_TAB_t ffpvCmsEntChan;
static OSD_TAB_t ffpvCmsEntPower;

static void ffpvCmsUpdateFreqRef(void)
{
    if (ffpvCmsBand > 0 && ffpvCmsChan > 0) {
        ffpvCmsFreqRef = vtxCommonLookupFrequency(vtxCommonDevice(), ffpvCmsBand, ffpvCmsChan);
    }
}

static long ffpvCmsConfigBand(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (ffpvCmsBand == 0) {
        // Bounce back
        ffpvCmsBand = 1;
    } else {
        ffpvCmsUpdateFreqRef();
    }

    return 0;
}

static long ffpvCmsConfigChan(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (ffpvCmsChan == 0) {
        // Bounce back
        ffpvCmsChan = 1;
    } else {
        ffpvCmsUpdateFreqRef();
    }

    return 0;
}

static long ffpvCmsConfigPower(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (ffpvCmsPower == 0) {
        // Bounce back
        ffpvCmsPower = 1;
    }

    return 0;
}

static long ffpvCmsCommence(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    vtxDevice_t *device = vtxCommonDevice();
    vtxCommonSetBandAndChannel(device, ffpvCmsBand, ffpvCmsChan);
    vtxCommonSetPowerByIndex(device, ffpvCmsPower);

    // update'vtx_' settings
    vtxSettingsConfigMutable()->band = ffpvCmsBand;
    vtxSettingsConfigMutable()->channel = ffpvCmsChan;
    vtxSettingsConfigMutable()->power = ffpvCmsPower;
    vtxSettingsConfigMutable()->freq = vtxCommonLookupFrequency(vtxCommonDevice(), ffpvCmsBand, ffpvCmsChan);

    saveConfigAndNotify();

    return MENU_CHAIN_BACK;
}

static bool ffpvCmsInitSettings(void)
{
    vtxDevice_t *device = vtxCommonDevice();

    if (!device) {
        return false;
    }

    vtxCommonGetBandAndChannel(device, &ffpvCmsBand, &ffpvCmsChan);

    ffpvCmsUpdateFreqRef();

    if (ffpvGetRuntimeState()->powerIndex > 0) {
        if (!vtxCommonGetPowerIndex(vtxCommonDevice(), &ffpvCmsPower)) {
            ffpvCmsPower = 1;
        }
    }

    ffpvCmsEntBand.val = &ffpvCmsBand;
    ffpvCmsEntBand.max = vtxTableBandCount;
    ffpvCmsEntBand.names = vtxTableBandNames;

    ffpvCmsEntChan.val = &ffpvCmsChan;
    ffpvCmsEntChan.max = vtxTableChannelCount;
    ffpvCmsEntChan.names = vtxTableChannelNames;

    ffpvCmsEntPower.val = &ffpvCmsPower;
    ffpvCmsEntPower.max = vtxTablePowerLevels;
    ffpvCmsEntPower.names = vtxTablePowerLabels;

    return true;
}

static long ffpvCmsOnEnter(const OSD_Entry *from)
{
    UNUSED(from);

    if (!ffpvCmsInitSettings()) {
        return MENU_CHAIN_BACK;
    }

    return 0;
}

static const OSD_Entry ffpvCmsMenuCommenceEntries[] =
{
    OSD_LABEL_ENTRY("CONFIRM"),
    OSD_FUNC_CALL_ENTRY("YES", ffpvCmsCommence),

    OSD_BACK_AND_END_ENTRY,
};

static const CMS_Menu ffpvCmsMenuCommence = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XVTXTRC",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = ffpvCmsMenuCommenceEntries,
};

static const OSD_Entry ffpvMenuEntries[] =
{
    OSD_LABEL_ENTRY("- Furious FPV -"),

    OSD_LABEL_DATA_DYN_ENTRY("", ffpvCmsStatusString),
    OSD_TAB_CALLBACK_ENTRY("BAND", ffpvCmsConfigBand, &ffpvCmsEntBand),
    OSD_TAB_CALLBACK_ENTRY("CHAN", ffpvCmsConfigChan, &ffpvCmsEntChan),
    OSD_UINT16_RO_ENTRY("(FREQ)", &ffpvCmsFreqRef),
    OSD_TAB_CALLBACK_ENTRY("POWER", ffpvCmsConfigPower, &ffpvCmsEntPower),
    OSD_SUBMENU_ENTRY("SET", &ffpvCmsMenuCommence),

    OSD_BACK_AND_END_ENTRY,
};

const CMS_Menu cmsx_menuVtxFFPV = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XVTXTR",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = ffpvCmsOnEnter,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = ffpvMenuEntries,
};
#endif
