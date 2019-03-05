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

#if defined(USE_CMS) && defined(USE_VTX_TRAMP)

#include "common/printf.h"
#include "common/utils.h"

#include "cms/cms.h"
#include "cms/cms_types.h"

#include "drivers/vtx_common.h"

#include "fc/config.h"

#include "io/vtx_string.h"
#include "io/vtx_tramp.h"
#include "io/vtx.h"

static bool trampCmsDrawStatusString(char *buf, unsigned bufsize)
{
    const char *defaultString = "- -- ---- ----";
//                               m bc ffff tppp
//                               01234567890123

    if (bufsize < strlen(defaultString) + 1) {
        return false;
    }

    strcpy(buf, defaultString);

    vtxDevice_t *vtxDevice = vtxCommonDevice();
    if (!vtxDevice || vtxCommonGetDeviceType(vtxDevice) != VTXDEV_TRAMP || !vtxCommonDeviceIsReady(vtxDevice)) {
        return true;
    }

    buf[0] = '*';
    buf[1] = ' ';
    buf[2] = vtx58BandLetter[trampData.band];
    buf[3] = vtx58ChannelNames[trampData.channel][0];
    buf[4] = ' ';

    if (trampData.curFreq)
        tfp_sprintf(&buf[5], "%4d", trampData.curFreq);
    else
        tfp_sprintf(&buf[5], "----");

    if (trampData.power) {
        tfp_sprintf(&buf[9], " %c%3d", (trampData.power == trampData.configuredPower) ? ' ' : '*', trampData.power);
    } else {
        tfp_sprintf(&buf[9], " ----");
    }

    return true;
}

uint8_t trampCmsPitMode = 0;
uint8_t trampCmsBand = 1;
uint8_t trampCmsChan = 1;
uint16_t trampCmsFreqRef;

static const OSD_TAB_t trampCmsEntBand = { &trampCmsBand, VTX_TRAMP_BAND_COUNT, vtx58BandNames };

static const OSD_TAB_t trampCmsEntChan = { &trampCmsChan, VTX_TRAMP_CHANNEL_COUNT, vtx58ChannelNames };

static uint8_t trampCmsPower = 1;

static const OSD_TAB_t trampCmsEntPower = { &trampCmsPower, VTX_TRAMP_POWER_COUNT, trampPowerNames };

static void trampCmsUpdateFreqRef(void)
{
    if (trampCmsBand > 0 && trampCmsChan > 0)
        trampCmsFreqRef = vtx58frequencyTable[trampCmsBand - 1][trampCmsChan - 1];
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

    trampSetBandAndChannel(trampCmsBand, trampCmsChan);
    trampSetRFPower(trampPowerTable[trampCmsPower-1]);

    // If it fails, the user should retry later
    trampCommitChanges();

    // update'vtx_' settings
    vtxSettingsConfigMutable()->band = trampCmsBand;
    vtxSettingsConfigMutable()->channel = trampCmsChan;
    vtxSettingsConfigMutable()->power = trampCmsPower;
    vtxSettingsConfigMutable()->freq = vtx58_Bandchan2Freq(trampCmsBand, trampCmsChan);

    saveConfigAndNotify();

    return MENU_CHAIN_BACK;
}

static void trampCmsInitSettings(void)
{
    if (trampData.band > 0) trampCmsBand = trampData.band;
    if (trampData.channel > 0) trampCmsChan = trampData.channel;

    trampCmsUpdateFreqRef();
    trampCmsPitMode = trampData.pitMode + 1;

    if (trampData.configuredPower > 0) {
        for (uint8_t i = 0; i < VTX_TRAMP_POWER_COUNT; i++) {
            if (trampData.configuredPower <= trampPowerTable[i]) {
                trampCmsPower = i + 1;
                break;
            }
        }
    }
}

static long trampCmsOnEnter(const OSD_Entry *from)
{
    UNUSED(from);

    trampCmsInitSettings();
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

    OSD_LABEL_FUNC_DYN_ENTRY("", trampCmsDrawStatusString),
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
