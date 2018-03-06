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

//
// Built-in menu contents and support functions
//

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "platform.h"

#ifdef USE_CMS

#include "build/version.h"

#include "drivers/time.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_builtin.h"

// Sub menus

#include "cms/cms_menu_imu.h"
#include "cms/cms_menu_blackbox.h"
#include "cms/cms_menu_navigation.h"
#include "cms/cms_menu_vtx.h"
#include "cms/cms_menu_osd.h"
#include "cms/cms_menu_ledstrip.h"
#include "cms/cms_menu_misc.h"

// VTX supplied menus

#include "cms/cms_menu_vtx_smartaudio.h"
#include "cms/cms_menu_vtx_tramp.h"


// Info

static char infoGitRev[GIT_SHORT_REVISION_LENGTH + 1];
static char infoTargetName[] = __TARGET__;

#include "msp/msp_protocol.h" // XXX for FC identification... not available elsewhere

static long cmsx_InfoInit(void)
{
    int i;
    for ( i = 0 ; i < GIT_SHORT_REVISION_LENGTH ; i++) {
        if (shortGitRevision[i] >= 'a' && shortGitRevision[i] <= 'f')
            infoGitRev[i] = shortGitRevision[i] - 'a' + 'A';
        else
            infoGitRev[i] = shortGitRevision[i];
    }

    infoGitRev[i] = 0x0; // Terminate string
    return 0;
}

static const OSD_Entry menuInfoEntries[] = {
    OSD_LABEL_ENTRY("--- INFO ---"),
    OSD_STRING_ENTRY("FWID", INAV_IDENTIFIER),
    OSD_STRING_ENTRY("FWVER", FC_VERSION_STRING),
    OSD_STRING_ENTRY("GITREV", infoGitRev),
    OSD_STRING_ENTRY("TARGET", infoTargetName),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

static const CMS_Menu menuInfo = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUINFO",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = cmsx_InfoInit,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = menuInfoEntries
};

// Features

static const OSD_Entry menuFeaturesEntries[] =
{
    OSD_LABEL_ENTRY("--- FEATURES ---"),
    OSD_SUBMENU_ENTRY("BLACKBOX", &cmsx_menuBlackbox),
#if defined(USE_NAV)
    OSD_SUBMENU_ENTRY("NAVIGATION", &cmsx_menuNavigation),
#endif
#if defined(VTX) || defined(USE_RTC6705)
    OSD_SUBMENU_ENTRY("VTX", &cmsx_menuVtx),
#endif // VTX || USE_RTC6705
#if defined(VTX_CONTROL)
#if defined(VTX_SMARTAUDIO)
    OSD_SUBMENU_ENTRY("VTX SA", &cmsx_menuVtxSmartAudio),
#endif
#if defined(VTX_TRAMP)
    OSD_SUBMENU_ENTRY("VTX TR", &cmsx_menuVtxTramp),
#endif
#endif // VTX_CONTROL
#ifdef USE_LED_STRIP
    OSD_SUBMENU_ENTRY("LED STRIP", &cmsx_menuLedstrip),
#endif // LED_STRIP

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

static const CMS_Menu menuFeatures = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUFEATURES",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = menuFeaturesEntries,
};

// Main

static const OSD_Entry menuMainEntries[] =
{
    OSD_LABEL_ENTRY("-- MAIN --"),

    OSD_SUBMENU_ENTRY("PID TUNING", &cmsx_menuImu),
    OSD_SUBMENU_ENTRY("FEATURES", &menuFeatures),
#ifdef USE_OSD
    OSD_SUBMENU_ENTRY("SCR LAYOUT", &cmsx_menuOsdLayout),
    OSD_SUBMENU_ENTRY("ALARMS", &cmsx_menuAlarms),
#endif
    OSD_SUBMENU_ENTRY("FC&FW INFO", &menuInfo),
    OSD_SUBMENU_ENTRY("MISC", &cmsx_menuMisc),

    {"SAVE&REBOOT", OME_OSD_Exit, cmsMenuExit,   (void*)CMS_EXIT_SAVEREBOOT, 0},
    {"EXIT",        OME_OSD_Exit, cmsMenuExit,   (void*)CMS_EXIT, 0},
#ifdef CMS_MENU_DEBUG
    OSD_SUBMENU_ENTRY("ERR SAMPLE", &menuInfoEntries[0]),
#endif

    OSD_END_ENTRY,
};

const CMS_Menu menuMain = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUMAIN",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = menuMainEntries,
};
#endif
