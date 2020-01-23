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

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <string.h>
#include <drivers/vtx_table.h>

#include "platform.h"

#if defined(USE_CMS) && defined(USE_VTX_SMARTAUDIO)

#include "common/log.h"
#include "common/printf.h"
#include "common/utils.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_vtx_smartaudio.h"

#include "drivers/vtx_common.h"

#include "fc/config.h"

#include "io/vtx_smartaudio.h"
#include "io/vtx.h"

// Interface to CMS

// Operational Model and RF modes (CMS)

#define SACMS_OPMODEL_UNDEF        0 // Not known yet
#define SACMS_OPMODEL_FREE         1 // Freestyle model: Power up transmitting
#define SACMS_OPMODEL_RACE         2 // Race model: Power up in pit mode

uint8_t  saCmsOpmodel = SACMS_OPMODEL_UNDEF;

#define SACMS_TXMODE_NODEF         0
#define SACMS_TXMODE_PIT_OUTRANGE  1
#define SACMS_TXMODE_PIT_INRANGE   2
#define SACMS_TXMODE_ACTIVE        3

uint8_t  saCmsRFState;          // RF state; ACTIVE, PIR, POR XXX Not currently used

uint8_t  saCmsBand = 0;
uint8_t  saCmsChan = 0;
uint8_t  saCmsPower = 0;

// Frequency derived from channel table (used for reference in band/channel mode)
uint16_t saCmsFreqRef = 0;

uint16_t saCmsDeviceFreq = 0;

uint8_t  saCmsDeviceStatus = 0;
uint8_t  saCmsPower;
uint8_t  saCmsPit;
uint8_t  saCmsPitFMode;          // Undef(0), In-Range(1) or Out-Range(2)

uint8_t  saCmsFselMode;          // Channel(0) or User defined(1)
uint8_t  saCmsFselModeNew;       // Channel(0) or User defined(1)

uint16_t saCmsORFreq = 0;       // POR frequency
uint16_t saCmsORFreqNew;        // POR frequency

uint16_t saCmsUserFreq = 0;     // User defined frequency
uint16_t saCmsUserFreqNew;      // User defined frequency

static long saCmsConfigOpmodelByGvar(displayPort_t *, const void *self);
static long saCmsConfigPitFModeByGvar(displayPort_t *, const void *self);
static long saCmsConfigBandByGvar(displayPort_t *, const void *self);
static long saCmsConfigChanByGvar(displayPort_t *, const void *self);
static long saCmsConfigPowerByGvar(displayPort_t *, const void *self);
static long saCmsConfigPitByGvar(displayPort_t *, const void *self);

void saCmsUpdate(void)
{
// XXX Take care of pit mode update somewhere???
    if (saCmsOpmodel == SACMS_OPMODEL_UNDEF) {
        // This is a first valid response to GET_SETTINGS.
        saCmsOpmodel = (saDevice.willBootIntoPitMode ) ? SACMS_OPMODEL_RACE : SACMS_OPMODEL_FREE;

        saCmsFselMode = (saDevice.mode & SA_MODE_GET_FREQ_BY_FREQ) && vtxSettingsConfig()->band == 0  ? 1 : 0;

        saCmsBand = vtxSettingsConfig()->band;
        saCmsChan = vtxSettingsConfig()->channel;
        saCmsFreqRef = vtxSettingsConfig()->freq;
        saCmsDeviceFreq = saCmsFreqRef;

        if ((saDevice.mode & SA_MODE_GET_PITMODE) == 0) {
            saCmsRFState = SACMS_TXMODE_ACTIVE;
        } else if (saDevice.mode & SA_MODE_GET_IN_RANGE_PITMODE) {
            saCmsRFState = SACMS_TXMODE_PIT_INRANGE;
        } else {
            saCmsRFState = SACMS_TXMODE_PIT_OUTRANGE;
        }

        saCmsPower = vtxSettingsConfig()->power;

        // if user-freq mode then track possible change
        if (saCmsFselMode && vtxSettingsConfig()->freq) {
            saCmsUserFreq = vtxSettingsConfig()->freq;
        }

        saCmsFselModeNew = saCmsFselMode;   //init mode for menu
    }

    saUpdateStatusString();
}

char saCmsStatusString[31] = "- -- ---- ---";
//                            m bc ffff ppp
//                            0123456789012

void saUpdateStatusString(void)
{
    if (saDevice.version == 0)
        return;

// XXX These should be done somewhere else
    if (saCmsDeviceStatus == 0 && saDevice.version != 0)
        saCmsDeviceStatus = saDevice.version;
    if (saCmsORFreq == 0 && saDevice.orfreq != 0)
        saCmsORFreq = saDevice.orfreq;
    if (saCmsUserFreq == 0 && saDevice.freq != 0)
        saCmsUserFreq = saDevice.freq;

    if (saDevice.version == 2) {
        if (saDevice.mode & SA_MODE_GET_OUT_RANGE_PITMODE)
            saCmsPitFMode = 1;
        else
            saCmsPitFMode = 0;
    } else if (saDevice.version == 3) {
        saCmsPitFMode = 1;//V2.1 only supports PIR
    }

    const vtxDevice_t *device = vtxCommonDevice();

    saCmsStatusString[0] = "-FR"[saCmsOpmodel];

    if (saCmsFselMode == 0) {
        uint8_t band;
        uint8_t channel;
        vtxCommonGetBandAndChannel(device, &band, &channel);
        saCmsStatusString[2] = vtxCommonLookupBandLetter(device, band);
        saCmsStatusString[3] = vtxCommonLookupChannelName(device, channel)[0];
    } else {
        saCmsStatusString[2] = 'U';
        saCmsStatusString[3] = 'F';
    }

    if ((saDevice.mode & SA_MODE_GET_PITMODE)
        && (saDevice.mode & SA_MODE_GET_OUT_RANGE_PITMODE)) {
        tfp_sprintf(&saCmsStatusString[5], "%4d", saDevice.orfreq);
    } else {
        uint16_t freq = 0;
        vtxCommonGetFrequency(device, &freq);
        tfp_sprintf(&saCmsStatusString[5], "%4d", freq);
    }

    saCmsStatusString[9] = ' ';

    if (saDevice.mode & SA_MODE_GET_PITMODE) {
        saCmsPit = 2;
        saCmsStatusString[10] = 'P';
        if (saDevice.mode & SA_MODE_GET_OUT_RANGE_PITMODE) {
            saCmsStatusString[11] = 'O';
        } else {
            saCmsStatusString[11] = 'I';
        }
        saCmsStatusString[12] = 'R';
        saCmsStatusString[13] = 0;
    } else {
        saCmsPit = 1;
        uint8_t powerIndex = 0;
        bool powerFound = vtxCommonGetPowerIndex(device, &powerIndex);
        tfp_sprintf(&saCmsStatusString[10], "%s", powerFound ? vtxCommonLookupPowerName(device, powerIndex) : "???");
    }

    if (vtxTableBandCount == 0 || vtxTablePowerLevels == 0) {
        strncpy(saCmsStatusString, "PLEASE CONFIGURE VTXTABLE", sizeof(saCmsStatusString));
    }
}

void saCmsResetOpmodel()
{
    // trigger data refresh in 'saCmsUpdate()'
    saCmsOpmodel = SACMS_OPMODEL_UNDEF;
}

static long saCmsConfigPitByGvar(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    LOG_D(VTX, "saCmsConfigPitByGvar: saCmsPit %d\r\n", saCmsPit);

    if (saDevice.version == 0) {
        // Bounce back; not online yet
        saCmsPit = 0;
        return 0;
    }

    if (saCmsPit == 0) {//trying to go back to undef state; bounce back
        saCmsPit = 1;
        return 0;
    }
    if (saDevice.power != saCmsPower) {//we can't change both power and pit mode at once
        saCmsPower = saDevice.power;
    }

    return 0;
}

static long saCmsConfigBandByGvar(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (saDevice.version == 0) {
        // Bounce back; not online yet
        saCmsBand = 0;
        return 0;
    }

    if (saCmsBand == 0) {
        // Bouce back, no going back to undef state
        saCmsBand = 1;
        return 0;
    }

    if ((saCmsOpmodel == SACMS_OPMODEL_FREE) && !saDeferred) {
        vtxCommonSetBandAndChannel(vtxCommonDevice(), saCmsBand, saCmsChan);
    }

    saCmsFreqRef = vtxCommonLookupFrequency(vtxCommonDevice(), saCmsBand, saCmsChan);

    return 0;
}

static long saCmsConfigChanByGvar(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (saDevice.version == 0) {
        // Bounce back; not online yet
        saCmsChan = 0;
        return 0;
    }

    if (saCmsChan == 0) {
        // Bounce back; no going back to undef state
        saCmsChan = 1;
        return 0;
    }

    if ((saCmsOpmodel == SACMS_OPMODEL_FREE) && !saDeferred) {
        vtxCommonSetBandAndChannel(vtxCommonDevice(), saCmsBand, saCmsChan);
    }

    saCmsFreqRef = vtxCommonLookupFrequency(vtxCommonDevice(), saCmsBand, saCmsChan);

    return 0;
}

static long saCmsConfigPowerByGvar(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (saDevice.version == 0) {
        // Bounce back; not online yet
        saCmsPower = 0;
        return 0;
    }

    if (saCmsPower == 0) {
        // Bouce back; no going back to undef state
        saCmsPower = 1;
        return 0;
    }

    if (saCmsPit > 0 && saCmsPit != 1 ) {
        saCmsPit = 1;
    }

    if (saCmsOpmodel == SACMS_OPMODEL_FREE && !saDeferred) {
        vtxSettingsConfigMutable()->power = saCmsPower;
    }
    LOG_D(VTX, "saCmsConfigPowerByGvar: power index is now %d\r\n", saCmsPower);

    return 0;
}

static long saCmsConfigPitFModeByGvar(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (saDevice.version == 1) {
        // V1 device doesn't support PIT mode; bounce back.
        saCmsPitFMode = 0;
        return 0;
    }

    LOG_D(VTX, "saCmsConfigPitFmodeByGbar: saCmsPitFMode %d", saCmsPitFMode);

    if (saCmsPitFMode == 0) {
        // Bounce back
        saCmsPitFMode = 1;
        return 0;
    }

    if (saDevice.version >= 3) {
        // V2.1 device only supports PIR mode. and setting any flag immediately enables pit mode.
        //therefore: bounce back.
        saCmsPitFMode = 1;
        return 0;
    }

    if (saCmsPitFMode == 1) {
        saSetMode(SA_MODE_SET_IN_RANGE_PITMODE);
    } else {
        saSetMode(SA_MODE_SET_OUT_RANGE_PITMODE);
    }

    return 0;
}

static long saCmsConfigFreqModeByGvar(displayPort_t *pDisp, const void *self); // Forward

static long saCmsConfigOpmodelByGvar(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (saDevice.version == 1) {
        if (saCmsOpmodel != SACMS_OPMODEL_FREE)
            saCmsOpmodel = SACMS_OPMODEL_FREE;
        return 0;
    }

    uint8_t opmodel = saCmsOpmodel;

    LOG_D(VTX, "saCmsConfigOpmodelByGvar: opmodel %d", opmodel);

    if (opmodel == SACMS_OPMODEL_FREE) {
        // VTX should power up transmitting.
        // Turn off In-Range and Out-Range bits
        saSetMode(0);
    } else if (opmodel == SACMS_OPMODEL_RACE) {
        // VTX should power up in pit mode.
        // Default PitFMode is in-range to prevent users without
        // out-range receivers from getting blinded.
        saCmsPitFMode = 0;
        saCmsConfigPitFModeByGvar(pDisp, self);
        if (saDevice.version >= 3) {
            saSetMode(SA_MODE_SET_IN_RANGE_PITMODE | ((saDevice.mode & SA_MODE_GET_PITMODE) ? 0 : SA_MODE_CLR_PITMODE));
        }

        // Direct frequency mode is not available in RACE opmodel
        saCmsFselModeNew = 0;
        saCmsConfigFreqModeByGvar(pDisp, self);
    } else {
        // Trying to go back to unknown state; bounce back
        saCmsOpmodel = SACMS_OPMODEL_UNDEF + 1;
    }

    return 0;
}

#ifdef USE_EXTENDED_CMS_MENUS
static const char * const saCmsDeviceStatusNames[] = {
    "OFFL",
    "ONL V1",
    "ONL V2",
    "ONLV21",
};

static const OSD_TAB_t saCmsEntOnline = { &saCmsDeviceStatus, 2, saCmsDeviceStatusNames };

static const OSD_Entry saCmsMenuStatsEntries[] = {
    OSD_LABEL_ENTRY("- SA STATS -"),

    OSD_TAB_DYN_ENTRY("STATUS", &saCmsEntOnline),
    OSD_UINT16_RO_ENTRY("BAUDRATE", &sa_smartbaud),
    OSD_UINT16_RO_ENTRY("SENT", &saStat.pktsent),
    OSD_UINT16_RO_ENTRY("RCVD", &saStat.pktrcvd),
    OSD_UINT16_RO_ENTRY("BADPRE", &saStat.badpre),
    OSD_UINT16_RO_ENTRY("BADLEN", &saStat.badlen),
    OSD_UINT16_RO_ENTRY("CRCERR", &saStat.crc),
    OSD_UINT16_RO_ENTRY("OOOERR", &saStat.ooopresp),

    OSD_BACK_AND_END_ENTRY,
};

static const CMS_Menu saCmsMenuStats = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XSAST",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = saCmsMenuStatsEntries
};
#endif /* USE_EXTENDED_CMS_MENUS */

static OSD_TAB_t saCmsEntBand;
static OSD_TAB_t saCmsEntChan;
static OSD_TAB_t saCmsEntPower;

static void saCmsInitNames(void)
{
    saCmsEntBand.val = &saCmsBand;
    saCmsEntBand.max = vtxTableBandCount;
    saCmsEntBand.names = vtxTableBandNames;

    saCmsEntChan.val = &saCmsChan;
    saCmsEntChan.max = vtxTableChannelCount;
    saCmsEntChan.names = vtxTableChannelNames;

    saCmsEntPower.val = &saCmsPower;
    saCmsEntPower.max = vtxTablePowerLevels;
    saCmsEntPower.names = vtxTablePowerLabels;
}

static const char * const saCmsOpmodelNames[] = {
    "----",
    "FREE",
    "RACE",
};

static const char * const saCmsFselModeNames[] = {
    "CHAN",
    "USER"
};

static const char * const saCmsPitFModeNames[] = {
    "---",
    "PIR",
    "POR"
};

static const char * const saCmsPitNames[] = {
    "---",
    "OFF",
    "ON ",
};

static const OSD_TAB_t saCmsEntPitFMode = { &saCmsPitFMode, 1, saCmsPitFModeNames };
static const OSD_TAB_t saCmsEntPit = {&saCmsPit, 2, saCmsPitNames};

static long sacms_SetupTopMenu(const OSD_Entry *from); // Forward

static long saCmsConfigFreqModeByGvar(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    // if trying to do user frequency mode in RACE opmodel then
    // revert because user-freq only available in FREE opmodel
    if (saCmsFselModeNew != 0 && saCmsOpmodel != SACMS_OPMODEL_FREE) {
        saCmsFselModeNew = 0;
    }

    // don't call 'saSetBandAndChannel()' / 'saSetFreq()' here,
    // wait until SET / 'saCmsCommence()' is activated

    sacms_SetupTopMenu(NULL);

    return 0;
}

static long saCmsCommence(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    const vtxSettingsConfig_t prevSettings = {
        .band = vtxSettingsConfig()->band,
        .channel = vtxSettingsConfig()->channel,
        .freq = vtxSettingsConfig()->freq,
        .power = vtxSettingsConfig()->power,
        .lowPowerDisarm = vtxSettingsConfig()->lowPowerDisarm,
    };
    vtxSettingsConfig_t newSettings = prevSettings;

    if (saCmsOpmodel == SACMS_OPMODEL_RACE) {
        // Race model
        // Setup band, freq and power.

        newSettings.band = saCmsBand;
        newSettings.channel = saCmsChan;
        newSettings.freq = vtxCommonLookupFrequency(vtxCommonDevice(), saCmsBand, saCmsChan);
    } else {
        // Freestyle model
        // Setup band and freq / user freq
        if (saCmsFselModeNew == 0) {
            newSettings.band = saCmsBand;
            newSettings.channel = saCmsChan;
            newSettings.freq = vtxCommonLookupFrequency(vtxCommonDevice(), saCmsBand, saCmsChan);
        } else {
            saSetMode(0);    //make sure FREE mode is setup
            newSettings.band = 0;
            newSettings.freq = saCmsUserFreq;
        }
    }

    if (newSettings.power == saCmsPower && saCmsPit > 0) {
        vtxCommonSetPitMode(vtxCommonDevice(), saCmsPit == 2);
    }
    newSettings.power = saCmsPower;

    if (memcmp(&prevSettings, &newSettings, sizeof(vtxSettingsConfig_t))) {
        vtxSettingsConfigMutable()->band = newSettings.band;
        vtxSettingsConfigMutable()->channel = newSettings.channel;
        vtxSettingsConfigMutable()->power = newSettings.power;
        vtxSettingsConfigMutable()->freq = newSettings.freq;
        saveConfigAndNotify();
    }

    return MENU_CHAIN_BACK;
}

static long saCmsSetPORFreqOnEnter(const OSD_Entry *from)
{
    UNUSED(from);

    if (saDevice.version == 1)
        return MENU_CHAIN_BACK;

    saCmsORFreqNew = saCmsORFreq;

    return 0;
}

static long saCmsSetPORFreq(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    saSetPitFreq(saCmsORFreqNew);

    return 0;
}

static char *saCmsORFreqGetString(void)
{
    static char pbuf[5];

    tfp_sprintf(pbuf, "%4d", saCmsORFreq);

    return pbuf;
}

static char *saCmsUserFreqGetString(void)
{
    static char pbuf[5];

    tfp_sprintf(pbuf, "%4d", saCmsUserFreq);

    return pbuf;
}

static long saCmsSetUserFreqOnEnter(const OSD_Entry *from)
{
    UNUSED(from);

    saCmsUserFreqNew = saCmsUserFreq;

    return 0;
}

static long saCmsConfigUserFreq(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    saCmsUserFreq = saCmsUserFreqNew;

    return MENU_CHAIN_BACK;
}

static const OSD_Entry saCmsMenuPORFreqEntries[] =
{
    OSD_LABEL_ENTRY("- POR FREQ -"),

    OSD_UINT16_RO_ENTRY("CUR FREQ", &saCmsORFreq),
    OSD_UINT16_ENTRY("NEW FREQ", (&(const OSD_UINT16_t){ &saCmsORFreqNew, 5000, 5900, 1 })),
    OSD_FUNC_CALL_ENTRY("SET", saCmsSetPORFreq),

    OSD_BACK_AND_END_ENTRY,
};

static const CMS_Menu saCmsMenuPORFreq =
{
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XSAPOR",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = saCmsSetPORFreqOnEnter,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = saCmsMenuPORFreqEntries,
};

static const OSD_Entry saCmsMenuUserFreqEntries[] =
{
    OSD_LABEL_ENTRY("- USER FREQ -"),

    OSD_UINT16_RO_ENTRY("CUR FREQ", &saCmsUserFreq),
    OSD_UINT16_ENTRY("NEW FREQ", (&(const OSD_UINT16_t){ &saCmsUserFreqNew, 5000, 5900, 1 })),
    OSD_FUNC_CALL_ENTRY("SET", saCmsConfigUserFreq),

    OSD_BACK_AND_END_ENTRY,
};

static const CMS_Menu saCmsMenuUserFreq =
{
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XSAUFQ",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = saCmsSetUserFreqOnEnter,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = saCmsMenuUserFreqEntries,
};

static const OSD_TAB_t saCmsEntFselMode = { &saCmsFselModeNew, 1, saCmsFselModeNames };

static const OSD_Entry saCmsMenuConfigEntries[] =
{
    OSD_LABEL_ENTRY("- SA CONFIG -"),

    { "OP MODEL",  {.func = saCmsConfigOpmodelByGvar}, &(const OSD_TAB_t){ &saCmsOpmodel, 2, saCmsOpmodelNames }, OME_TAB, DYNAMIC },
    { "FSEL MODE", {.func = saCmsConfigFreqModeByGvar}, &saCmsEntFselMode, OME_TAB, DYNAMIC },
    OSD_TAB_CALLBACK_ENTRY("PIT FMODE", saCmsConfigPitFModeByGvar, &saCmsEntPitFMode),
    { "POR FREQ",  {.menufunc = saCmsORFreqGetString}, (void *)&saCmsMenuPORFreq, OME_Submenu, OPTSTRING },
#ifdef USE_EXTENDED_CMS_MENUS
    OSD_SUBMENU_ENTRY("STATX", &saCmsMenuStats),
#endif /* USE_EXTENDED_CMS_MENUS */

    OSD_BACK_AND_END_ENTRY,
};

static const CMS_Menu saCmsMenuConfig = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XSACFG",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = saCmsMenuConfigEntries
};

static const OSD_Entry saCmsMenuCommenceEntries[] =
{
    OSD_LABEL_ENTRY("CONFIRM"),
    OSD_FUNC_CALL_ENTRY("YES", saCmsCommence),

    OSD_BACK_AND_END_ENTRY,
};

static const CMS_Menu saCmsMenuCommence = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XVTXCOM",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = saCmsMenuCommenceEntries,
};

#pragma GCC diagnostic push
#if (__GNUC__ > 7)
    // This is safe on 32bit platforms, suppress warning for saCmsUserFreqGetString
#pragma GCC diagnostic ignored "-Wcast-function-type"
#endif
static const OSD_Entry saCmsMenuFreqModeEntries[] =
{
    OSD_LABEL_ENTRY("- SMARTAUDIO -"),

    OSD_LABEL_DATA_DYN_ENTRY("", saCmsStatusString),
    { "FREQ", {.menufunc = saCmsUserFreqGetString},  &saCmsMenuUserFreq, OME_Submenu, OPTSTRING },
    OSD_TAB_CALLBACK_DYN_ENTRY("POWER", saCmsConfigPowerByGvar, &saCmsEntPower),
    OSD_TAB_CALLBACK_DYN_ENTRY("PIT", saCmsConfigPitByGvar, &saCmsEntPit),
    OSD_SUBMENU_ENTRY("SET", &saCmsMenuCommence),
    OSD_SUBMENU_ENTRY("CONFIG", &saCmsMenuConfig),

    OSD_BACK_AND_END_ENTRY,
};
#pragma GCC diagnostic pop

static const OSD_Entry saCmsMenuChanModeEntries[] =
{
    OSD_LABEL_ENTRY("- SMARTAUDIO -"),

    OSD_LABEL_DATA_DYN_ENTRY("", saCmsStatusString),
    OSD_TAB_CALLBACK_ENTRY("BAND", saCmsConfigBandByGvar, &saCmsEntBand),
    OSD_TAB_CALLBACK_ENTRY("CHAN", saCmsConfigChanByGvar, &saCmsEntChan),
    OSD_UINT16_RO_ENTRY("(FREQ)", &saCmsFreqRef),
    OSD_TAB_CALLBACK_DYN_ENTRY("POWER", saCmsConfigPowerByGvar, &saCmsEntPower),
    OSD_TAB_CALLBACK_DYN_ENTRY("PIT", saCmsConfigPitByGvar, &saCmsEntPit),
    OSD_SUBMENU_ENTRY("SET", &saCmsMenuCommence),
    OSD_SUBMENU_ENTRY("CONFIG", &saCmsMenuConfig),

    OSD_BACK_AND_END_ENTRY,
};

static const OSD_Entry saCmsMenuOfflineEntries[] =
{
    OSD_LABEL_ENTRY("- VTX SMARTAUDIO -"),

    OSD_LABEL_DATA_DYN_ENTRY("", saCmsStatusString),
#ifdef USE_EXTENDED_CMS_MENUS
    OSD_SUBMENU_ENTRY("STATX", &saCmsMenuStats),
#endif /* USE_EXTENDED_CMS_MENUS */

    OSD_BACK_AND_END_ENTRY,
};

CMS_Menu cmsx_menuVtxSmartAudio; // Forward

static long sacms_SetupTopMenu(const OSD_Entry *from)
{
    UNUSED(from);

    if (saCmsDeviceStatus) {
        if (saCmsFselModeNew == 0)
            cmsx_menuVtxSmartAudio.entries = saCmsMenuChanModeEntries;
        else
            cmsx_menuVtxSmartAudio.entries = saCmsMenuFreqModeEntries;
    } else {
        cmsx_menuVtxSmartAudio.entries = saCmsMenuOfflineEntries;
    }

    saCmsInitNames();

    return 0;
}

CMS_Menu cmsx_menuVtxSmartAudio = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XVTXSA",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = sacms_SetupTopMenu,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = saCmsMenuOfflineEntries,
};

#endif // CMS
