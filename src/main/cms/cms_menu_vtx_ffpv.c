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

#include "platform.h"

#if defined(USE_CMS) && defined(USE_VTX_FFPV)

#include "common/printf.h"
#include "common/utils.h"

#include "cms/cms.h"
#include "cms/cms_types.h"

#include "drivers/vtx_common.h"

#include "fc/config.h"

#include "io/vtx_string.h"
#include "io/vtx_ffpv24g.h"
#include "io/vtx.h"

static bool ffpvCmsDrawStatusString(char *buf, unsigned bufsize)
{
    const char *defaultString = "- -- ---- ---";
//                               m bc ffff ppp
//                               01234567890123

    if (bufsize < strlen(defaultString) + 1) {
        return false;
    }

    strcpy(buf, defaultString);

    vtxDevice_t *vtxDevice = vtxCommonDevice();
    if (!vtxDevice || vtxCommonGetDeviceType(vtxDevice) != VTXDEV_FFPV || !vtxCommonDeviceIsReady(vtxDevice)) {
        return true;
    }

    buf[0] = '*';
    buf[1] = ' ';
    buf[2] = ffpvBandLetters[ffpvGetRuntimeState()->band];
    buf[3] = ffpvChannelNames[ffpvGetRuntimeState()->channel][0];
    buf[4] = ' ';

    tfp_sprintf(&buf[5], "%4d", ffpvGetRuntimeState()->frequency);
    tfp_sprintf(&buf[9], " %3d", ffpvGetRuntimeState()->powerMilliwatt);

    return true;
}

uint8_t ffpvCmsBand = 1;
uint8_t ffpvCmsChan = 1;
uint16_t ffpvCmsFreqRef;
static uint8_t ffpvCmsPower = 1;

static const OSD_TAB_t ffpvCmsEntBand = { &ffpvCmsBand, VTX_FFPV_BAND_COUNT, ffpvBandNames };
static const OSD_TAB_t ffpvCmsEntChan = { &ffpvCmsChan, VTX_FFPV_CHANNEL_COUNT, ffpvChannelNames };
static const OSD_TAB_t ffpvCmsEntPower = { &ffpvCmsPower, VTX_FFPV_POWER_COUNT, ffpvPowerNames };

static void ffpvCmsUpdateFreqRef(void)
{
    if (ffpvCmsBand > 0 && ffpvCmsChan > 0) {
        ffpvCmsFreqRef = ffpvFrequencyTable[ffpvCmsBand - 1][ffpvCmsChan - 1];
    }
}

static long ffpvCmsConfigBand(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (ffpvCmsBand == 0) {
        // Bounce back
        ffpvCmsBand = 1;
    }
    else {
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
    }
    else {
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

    // call driver directly
    ffpvSetBandAndChannel(ffpvCmsBand, ffpvCmsChan);
    ffpvSetRFPowerByIndex(ffpvCmsPower);

    // update'vtx_' settings
    vtxSettingsConfigMutable()->band = ffpvCmsBand;
    vtxSettingsConfigMutable()->channel = ffpvCmsChan;
    vtxSettingsConfigMutable()->power = ffpvCmsPower;
    vtxSettingsConfigMutable()->freq = ffpvFrequencyTable[ffpvCmsBand - 1][ffpvCmsChan - 1];

    saveConfigAndNotify();

    return MENU_CHAIN_BACK;
}

static void ffpvCmsInitSettings(void)
{
    ffpvCmsBand = ffpvGetRuntimeState()->band;
    ffpvCmsChan = ffpvGetRuntimeState()->channel;
    ffpvCmsPower = ffpvGetRuntimeState()->powerIndex;

    ffpvCmsUpdateFreqRef();
}

static long ffpvCmsOnEnter(const OSD_Entry *from)
{
    UNUSED(from);

    ffpvCmsInitSettings();
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
    OSD_LABEL_ENTRY("- TRAMP -"),

    OSD_LABEL_FUNC_DYN_ENTRY("", ffpvCmsDrawStatusString),
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
