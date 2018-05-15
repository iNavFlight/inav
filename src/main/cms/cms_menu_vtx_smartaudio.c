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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "platform.h"

#if defined(USE_CMS) && defined(VTX_SMARTAUDIO)

#include "common/printf.h"
#include "common/utils.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_vtx_smartaudio.h"

#include "drivers/vtx_common.h"

#include "fc/config.h"

#include "io/vtx_string.h"
#include "io/vtx_smartaudio.h"

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
uint8_t  saCmsPitFMode;         // In-Range or Out-Range
uint8_t  saCmsFselMode;          // Channel(0) or User defined(1)

uint16_t saCmsORFreq = 0;       // POR frequency
uint16_t saCmsORFreqNew;        // POR frequency

uint16_t saCmsUserFreq = 0;     // User defined frequency
uint16_t saCmsUserFreqNew;      // User defined frequency

void saCmsUpdate(void)
{
// XXX Take care of pit mode update somewhere???

    if (saCmsOpmodel == SACMS_OPMODEL_UNDEF) {
        // This is a first valid response to GET_SETTINGS.
        saCmsOpmodel = (saDevice.mode & SA_MODE_GET_PITMODE) ? SACMS_OPMODEL_RACE : SACMS_OPMODEL_FREE;

        saCmsFselMode = (saDevice.mode & SA_MODE_GET_FREQ_BY_FREQ) ? 1 : 0;

        saCmsBand = (saDevice.channel / 8) + 1;
        saCmsChan = (saDevice.channel % 8) + 1;
        saCmsFreqRef = vtx58frequencyTable[saDevice.channel / 8][saDevice.channel % 8];

        saCmsDeviceFreq = vtx58frequencyTable[saDevice.channel / 8][saDevice.channel % 8];

        if ((saDevice.mode & SA_MODE_GET_PITMODE) == 0) {
            saCmsRFState = SACMS_TXMODE_ACTIVE;
        } else if (saDevice.mode & SA_MODE_GET_IN_RANGE_PITMODE) {
            saCmsRFState = SACMS_TXMODE_PIT_INRANGE;
        } else {
            saCmsRFState = SACMS_TXMODE_PIT_OUTRANGE;
        }

        if (saDevice.version == 2) {
            saCmsPower = saDevice.power + 1; // XXX Take care V1
        } else {
            saCmsPower = saDacToPowerIndex(saDevice.power) + 1;
        }
    }
}

static long saCmsConfigOpmodelByGvar(displayPort_t *, const void *self);
static long saCmsConfigPitFModeByGvar(displayPort_t *, const void *self);
static long saCmsConfigBandByGvar(displayPort_t *, const void *self);
static long saCmsConfigChanByGvar(displayPort_t *, const void *self);
static long saCmsConfigPowerByGvar(displayPort_t *, const void *self);

static bool saCmsDrawStatusString(char *buf, unsigned bufsize)
{
    const char *defaultString = "- -- ---- ---";
//                               m bc ffff ppp
//                               0123456789012

    if (bufsize < strlen(defaultString) + 1) {
        return false;
    }

    strcpy(buf, defaultString);

    if (saDevice.version == 0)
        return true;

    // XXX These should be done somewhere else
    if (saCmsDeviceStatus == 0 && saDevice.version != 0)
        saCmsDeviceStatus = saDevice.version;
    if (saCmsORFreq == 0 && saDevice.orfreq != 0)
        saCmsORFreq = saDevice.orfreq;
    if (saCmsUserFreq == 0 && saDevice.freq != 0)
        saCmsUserFreq = saDevice.freq;

    if (saDevice.mode & SA_MODE_GET_OUT_RANGE_PITMODE)
        saCmsPitFMode = 1;
    else
        saCmsPitFMode = 0;

    buf[0] = "-FR"[saCmsOpmodel];

    if (saCmsFselMode == 0) {
        buf[2] = "ABEFR"[saDevice.channel / 8];
        buf[3] = '1' + (saDevice.channel % 8);
    } else {
        buf[2] = 'U';
        buf[3] = 'F';
    }

    if ((saDevice.mode & SA_MODE_GET_PITMODE)
       && (saDevice.mode & SA_MODE_GET_OUT_RANGE_PITMODE))
        tfp_sprintf(&buf[5], "%4d", saDevice.orfreq);
    else if (saDevice.mode & SA_MODE_GET_FREQ_BY_FREQ)
        tfp_sprintf(&buf[5], "%4d", saDevice.freq);
    else
        tfp_sprintf(&buf[5], "%4d",
            vtx58frequencyTable[saDevice.channel / 8][saDevice.channel % 8]);

    buf[9] = ' ';

    if (saDevice.mode & SA_MODE_GET_PITMODE) {
        buf[10] = 'P';
        if (saDevice.mode & SA_MODE_GET_IN_RANGE_PITMODE) {
            buf[11] = 'I';
        } else {
            buf[11] = 'O';
        }
        buf[12] = 'R';
        buf[13] = 0;
    } else {
        tfp_sprintf(&buf[10], "%3d", (saDevice.version == 2) ?  saPowerTable[saDevice.power].rfpower : saPowerTable[saDacToPowerIndex(saDevice.power)].rfpower);
    }
    return true;
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

    if ((saCmsOpmodel == SACMS_OPMODEL_FREE) && !saDeferred)
        saSetBandAndChannel(saCmsBand - 1, saCmsChan - 1);

    saCmsFreqRef = vtx58frequencyTable[saCmsBand - 1][saCmsChan - 1];

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

    if ((saCmsOpmodel == SACMS_OPMODEL_FREE) && !saDeferred)
        saSetBandAndChannel(saCmsBand - 1, saCmsChan - 1);

    saCmsFreqRef = vtx58frequencyTable[saCmsBand - 1][saCmsChan - 1];

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

    if (saCmsOpmodel == SACMS_OPMODEL_FREE)
        saSetPowerByIndex(saCmsPower - 1);

    return 0;
}

static long saCmsConfigPitFModeByGvar(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    dprintf(("saCmsConfigPitFmodeByGbar: saCmsPitFMode %d\r\n", saCmsPitFMode));

    if (saCmsPitFMode == 0) {
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

    uint8_t opmodel = saCmsOpmodel;

    dprintf(("saCmsConfigOpmodelByGvar: opmodel %d\r\n", opmodel));

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

        // Direct frequency mode is not available in RACE opmodel
        saCmsFselMode = 0;
        saCmsConfigFreqModeByGvar(pDisp, self);
    } else {
        // Trying to go back to unknown state; bounce back
        saCmsOpmodel = SACMS_OPMODEL_UNDEF + 1;
    }

    return 0;
}

static const char * const saCmsDeviceStatusNames[] = {
    "OFFL",
    "ONL V1",
    "ONL V2",
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

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
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

static const OSD_TAB_t saCmsEntBand = { &saCmsBand, 5, vtx58BandNames };

static const OSD_TAB_t saCmsEntChan = { &saCmsChan, 8, vtx58ChannelNames };

static const char * const saCmsPowerNames[] = {
    "---",
    "25 ",
    "200",
    "500",
    "800",
};

static const OSD_TAB_t saCmsEntPower = { &saCmsPower, 4, saCmsPowerNames};

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
    "PIR",
    "POR"
};

static const OSD_TAB_t saCmsEntPitFMode = { &saCmsPitFMode, 1, saCmsPitFModeNames };

static long sacms_SetupTopMenu(void); // Forward

static long saCmsConfigFreqModeByGvar(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (saCmsFselMode == 0) {
        // CHAN
        saSetBandAndChannel(saCmsBand - 1, saCmsChan - 1);
    } else {
        // USER: User frequency mode is only available in FREE opmodel.
        if (saCmsOpmodel == SACMS_OPMODEL_FREE) {
            saSetFreq(saCmsUserFreq);
        } else {
            // Bounce back
            saCmsFselMode = 0;
        }
    }

    sacms_SetupTopMenu();

    return 0;
}

static long saCmsCommence(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    if (saCmsOpmodel == SACMS_OPMODEL_RACE) {
        // Race model
        // Setup band, freq and power.

        saSetBandAndChannel(saCmsBand - 1, saCmsChan - 1);

        // If in pit mode, cancel it.

        if (saCmsPitFMode == 0)
            saSetMode(SA_MODE_CLR_PITMODE|SA_MODE_SET_IN_RANGE_PITMODE);
        else
            saSetMode(SA_MODE_CLR_PITMODE|SA_MODE_SET_OUT_RANGE_PITMODE);
    } else {
        // Freestyle model
        // Setup band and freq / user freq
        if (saCmsFselMode == 0)
            saSetBandAndChannel(saCmsBand - 1, saCmsChan - 1);
        else
            saSetFreq(saCmsUserFreq);
    }

    saSetPowerByIndex(saCmsPower - 1);

    return MENU_CHAIN_BACK;
}

static long saCmsSetPORFreqOnEnter(void)
{
    saCmsORFreqNew = saCmsORFreq;

    return 0;
}

static long saCmsSetPORFreq(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    saSetFreq(saCmsORFreqNew|SA_FREQ_SETPIT);

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

static long saCmsSetUserFreqOnEnter(void)
{
    saCmsUserFreqNew = saCmsUserFreq;

    return 0;
}

static long saCmsConfigUserFreq(displayPort_t *pDisp, const void *self)
{
    UNUSED(pDisp);
    UNUSED(self);

    saCmsUserFreq = saCmsUserFreqNew;
    //saSetFreq(saCmsUserFreq);

    return MENU_CHAIN_BACK;
}

static const OSD_Entry saCmsMenuPORFreqEntries[] =
{
    OSD_LABEL_ENTRY("- POR FREQ -"),

    OSD_UINT16_RO_ENTRY("CUR FREQ", &saCmsORFreq),
    OSD_UINT16_ENTRY("NEW FREQ", (&(const OSD_UINT16_t){ &saCmsORFreqNew, 5000, 5900, 1 })),
    OSD_FUNC_CALL_ENTRY("SET", saCmsSetPORFreq),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
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

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
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

static const OSD_TAB_t saCmsEntFselMode = { &saCmsFselMode, 1, saCmsFselModeNames };

static const OSD_Entry saCmsMenuConfigEntries[] =
{
    OSD_LABEL_ENTRY("- SA CONFIG -"),

    { "OP MODEL",  OME_TAB,     saCmsConfigOpmodelByGvar,              &(const OSD_TAB_t){ &saCmsOpmodel, 2, saCmsOpmodelNames }, DYNAMIC },
    { "FSEL MODE", OME_TAB,     saCmsConfigFreqModeByGvar,             &saCmsEntFselMode,                                   DYNAMIC },
    OSD_TAB_CALLBACK_ENTRY("PIT FMODE", saCmsConfigPitFModeByGvar, &saCmsEntPitFMode),
    { "POR FREQ",  OME_Submenu, (CMSEntryFuncPtr)saCmsORFreqGetString, (void *)&saCmsMenuPORFreq,                                   OPTSTRING },
    OSD_SUBMENU_ENTRY("STATX", &saCmsMenuStats),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
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

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
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

static const OSD_Entry saCmsMenuFreqModeEntries[] =
{
    OSD_LABEL_ENTRY("- SMARTAUDIO -"),

    OSD_LABEL_FUNC_DYN_ENTRY("", saCmsDrawStatusString),
    { "FREQ",   OME_Submenu, (CMSEntryFuncPtr)saCmsUserFreqGetString,  &saCmsMenuUserFreq, OPTSTRING },
    OSD_TAB_CALLBACK_ENTRY("POWER", saCmsConfigPowerByGvar, &saCmsEntPower),
    OSD_SUBMENU_ENTRY("SET", &saCmsMenuCommence),
    OSD_SUBMENU_ENTRY("CONFIG", &saCmsMenuConfig),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

static const OSD_Entry saCmsMenuChanModeEntries[] =
{
    OSD_LABEL_ENTRY("- SMARTAUDIO -"),

    OSD_LABEL_FUNC_DYN_ENTRY("", saCmsDrawStatusString),
    OSD_TAB_CALLBACK_ENTRY("BAND", saCmsConfigBandByGvar, &saCmsEntBand),
    OSD_TAB_CALLBACK_ENTRY("CHAN", saCmsConfigChanByGvar, &saCmsEntChan),
    OSD_UINT16_RO_ENTRY("(FREQ)", &saCmsFreqRef),
    OSD_TAB_CALLBACK_ENTRY("POWER", saCmsConfigPowerByGvar, &saCmsEntPower),
    OSD_SUBMENU_ENTRY("SET",  &saCmsMenuCommence),
    OSD_SUBMENU_ENTRY("CONFIG", &saCmsMenuConfig),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

static const OSD_Entry saCmsMenuOfflineEntries[] =
{
    OSD_LABEL_ENTRY("- VTX SMARTAUDIO -"),

    OSD_LABEL_FUNC_DYN_ENTRY("", saCmsDrawStatusString),
    OSD_SUBMENU_ENTRY("STATX", &saCmsMenuStats),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

CMS_Menu cmsx_menuVtxSmartAudio; // Forward

static long sacms_SetupTopMenu(void)
{
    if (saCmsDeviceStatus) {
        if (saCmsFselMode == 0)
            cmsx_menuVtxSmartAudio.entries = saCmsMenuChanModeEntries;
        else
            cmsx_menuVtxSmartAudio.entries = saCmsMenuFreqModeEntries;
    } else {
        cmsx_menuVtxSmartAudio.entries = saCmsMenuOfflineEntries;
    }

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
