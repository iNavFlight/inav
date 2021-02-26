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

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#include "build/version.h"

#if defined(USE_CMS) && defined(USE_VTX_CONTROL)

#include "common/printf.h"
#include "common/utils.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_vtx.h"

#include "drivers/vtx_common.h"

#include "fc/config.h"

#include "io/vtx_string.h"
#include "io/vtx.h"


// Config-time settings
static uint8_t  vtxBand = 0;
static uint8_t  vtxChan = 0;
static uint8_t  vtxPower = 0;
static uint8_t  vtxPitMode = 0;

static const char * const vtxCmsPitModeNames[] = {
    "---", "OFF", "ON "
};

// Menus (these are not const because we update them at run-time )
static OSD_TAB_t cms_Vtx_EntBand = { &vtxBand, VTX_SETTINGS_BAND_COUNT, vtx58BandNames };
static OSD_TAB_t cms_Vtx_EntChan = { &vtxChan, VTX_SETTINGS_CHANNEL_COUNT, vtx58ChannelNames };
static OSD_TAB_t cms_Vtx_EntPower = { &vtxPower, VTX_SETTINGS_POWER_COUNT, vtx58DefaultPowerNames };
static const OSD_TAB_t cms_Vtx_EntPitMode = { &vtxPitMode, 2, vtxCmsPitModeNames };

static long cms_Vtx_configPitMode(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (vtxPitMode == 0) {
        vtxPitMode = 1;
    }

    // Pit mode changes are immediate, without saving
    vtxCommonSetPitMode(vtxCommonDevice(), vtxPitMode >= 2 ? 1 : 0);

    return 0;
}

static long cms_Vtx_configBand(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (vtxBand == 0) {
        vtxBand = 1;
    }
    return 0;
}

static long cms_Vtx_configChan(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (vtxChan == 0) {
        vtxChan = 1;
    }
    return 0;
}

static long cms_Vtx_configPower(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (vtxPower == 0) {
        vtxPower = 1;
    }
    return 0;
}

static void cms_Vtx_initSettings(void)
{
    vtxDevice_t * vtxDevice = vtxCommonDevice();
    vtxDeviceCapability_t vtxDeviceCapability;

    if (vtxCommonGetDeviceCapability(vtxDevice, &vtxDeviceCapability)) {
        cms_Vtx_EntBand.max = vtxDeviceCapability.bandCount;
        cms_Vtx_EntBand.names = (const char * const *)vtxDeviceCapability.bandNames;

        cms_Vtx_EntChan.max = vtxDeviceCapability.channelCount;
        cms_Vtx_EntChan.names = (const char * const *)vtxDeviceCapability.channelNames;

        cms_Vtx_EntPower.max = vtxDeviceCapability.powerCount;
        cms_Vtx_EntPower.names = (const char * const *)vtxDeviceCapability.powerNames;
    }
    else {
        cms_Vtx_EntBand.max = VTX_SETTINGS_BAND_COUNT;
        cms_Vtx_EntBand.names = vtx58BandNames;

        cms_Vtx_EntChan.max = VTX_SETTINGS_CHANNEL_COUNT;
        cms_Vtx_EntChan.names = vtx58ChannelNames;

        cms_Vtx_EntPower.max = VTX_SETTINGS_POWER_COUNT;
        cms_Vtx_EntPower.names = vtx58DefaultPowerNames;
    }

    vtxBand = vtxSettingsConfig()->band;
    vtxChan = vtxSettingsConfig()->channel;
    vtxPower = vtxSettingsConfig()->power;

    // If device is ready - read actual PIT mode
    if (vtxCommonDeviceIsReady(vtxDevice)) {
        uint8_t onoff;
        vtxCommonGetPitMode(vtxDevice, &onoff);
        vtxPitMode = onoff ? 2 : 1;
    }
    else {
        vtxPitMode = 0;
    }
}

static long cms_Vtx_onEnter(const OSD_Entry *self)
{
    UNUSED(self);
    cms_Vtx_initSettings();
    return 0;
}

static long cms_Vtx_Commence(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    vtxCommonSetBandAndChannel(vtxCommonDevice(), vtxBand, vtxChan);
    vtxCommonSetPowerByIndex(vtxCommonDevice(), vtxPower);
    vtxCommonSetPitMode(vtxCommonDevice(), vtxPitMode == 2 ? 1 : 0);

    vtxSettingsConfigMutable()->band = vtxBand;
    vtxSettingsConfigMutable()->channel = vtxChan;
    vtxSettingsConfigMutable()->power = vtxPower;

    saveConfigAndNotify();

    return MENU_CHAIN_BACK;
}

static bool cms_Vtx_drawStatusString(char *buf, unsigned bufsize)
{
    const char *defaultString = "-- ---- ----";
//                               bc ffff pppp
//                               012345678901

    if (bufsize < strlen(defaultString) + 1) {
        return false;
    }

    strcpy(buf, defaultString);

    vtxDevice_t * vtxDevice = vtxCommonDevice();
    vtxDeviceOsdInfo_t osdInfo;

    if (!vtxDevice || !vtxCommonGetOsdInfo(vtxDevice, &osdInfo) || !vtxCommonDeviceIsReady(vtxDevice)) {
        return true;
    }

    buf[0] = osdInfo.bandLetter;
    buf[1] = osdInfo.channelName[0];
    buf[2] = ' ';

    if (osdInfo.frequency)
        tfp_sprintf(&buf[3], "%4d", osdInfo.frequency);
    else
        tfp_sprintf(&buf[3], "----");

    if (osdInfo.powerIndex) {
        // If OSD driver provides power in milliwatt - display MW, otherwise - power level
        if (osdInfo.powerMilliwatt) {
            tfp_sprintf(&buf[7], " %4d", osdInfo.powerMilliwatt);
        }
        else {
            tfp_sprintf(&buf[7], " PL=%c", osdInfo.powerIndex);
        }
    } else {
        tfp_sprintf(&buf[7], " ----");
    }

    return true;
}

static const OSD_Entry cms_menuCommenceEntries[] =
{
    OSD_LABEL_ENTRY("CONFIRM"),
    OSD_FUNC_CALL_ENTRY("YES", cms_Vtx_Commence),

    OSD_BACK_AND_END_ENTRY,
};

static const CMS_Menu cms_menuCommence = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XVTXTRC",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = cms_menuCommenceEntries,
};

static const OSD_Entry cms_menuVtxEntries[] =
{
    OSD_LABEL_ENTRY("--- VTX ---"),
    OSD_LABEL_FUNC_DYN_ENTRY("", cms_Vtx_drawStatusString),
    OSD_TAB_CALLBACK_ENTRY("PIT",   cms_Vtx_configPitMode, &cms_Vtx_EntPitMode),
    OSD_TAB_CALLBACK_ENTRY("BAND",  cms_Vtx_configBand,    &cms_Vtx_EntBand),
    OSD_TAB_CALLBACK_ENTRY("CHAN",  cms_Vtx_configChan,    &cms_Vtx_EntChan),
    OSD_TAB_CALLBACK_ENTRY("POWER", cms_Vtx_configPower,   &cms_Vtx_EntPower),

    OSD_SUBMENU_ENTRY("SET", &cms_menuCommence),
    OSD_BACK_AND_END_ENTRY,
};

const CMS_Menu cmsx_menuVtxControl = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUVTX",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = cms_Vtx_onEnter,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = cms_menuVtxEntries
};

#endif // CMS
