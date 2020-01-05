/*
 * This file is part of INAV.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU General Public License Version 3, as described below:
 *
 * This file is free software: you may copy, redistribute and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see http://www.gnu.org/licenses/.
 *
 * @author Alberto Garcia Hierro <alberto@garciahierro.com>
 */

#include "platform.h"

#if defined(USE_NAV)

#include <stdlib.h>
#include <string.h>

#include "cms/cms.h"
#include "cms/cms_types.h"

#include "fc/runtime_config.h"
#include "fc/settings.h"

#include "navigation/navigation.h"

static const OSD_Entry cmsx_menuNavSettingsEntries[] =
{
    OSD_LABEL_ENTRY("-- BASIC SETTINGS --"),

    OSD_SETTING_ENTRY("CONTROL MODE", SETTING_NAV_USER_CONTROL_MODE),
    OSD_SETTING_ENTRY("MAX NAV SPEED", SETTING_NAV_AUTO_SPEED),
    OSD_SETTING_ENTRY("MAX CRUISE SPEED", SETTING_NAV_MANUAL_SPEED),
    OSD_SETTING_ENTRY("MAX NAV CLIMB RATE", SETTING_NAV_AUTO_CLIMB_RATE),
    OSD_SETTING_ENTRY("MAX AH CLIMB RATE", SETTING_NAV_MANUAL_CLIMB_RATE),
    OSD_SETTING_ENTRY("MC MAX BANK ANGLE", SETTING_NAV_MC_BANK_ANGLE),
    OSD_SETTING_ENTRY("MID THR FOR AH", SETTING_NAV_USE_MIDTHR_FOR_ALTHOLD),
    OSD_SETTING_ENTRY("MC HOVER THR", SETTING_NAV_MC_HOVER_THR),

    OSD_BACK_AND_END_ENTRY,
 };

static const CMS_Menu cmsx_menuNavSettings = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUNAVSETTINGS",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = cmsx_menuNavSettingsEntries
};

 static const OSD_Entry cmsx_menuRTHEntries[] =
 {
    OSD_LABEL_ENTRY("-- RTH --"),

    OSD_SETTING_ENTRY("RTH ALT MODE", SETTING_NAV_RTH_ALT_MODE),
    OSD_SETTING_ENTRY("RTH ALT", SETTING_NAV_RTH_ALTITUDE),
    OSD_SETTING_ENTRY("CLIMB BEFORE RTH", SETTING_NAV_RTH_CLIMB_FIRST),
    OSD_SETTING_ENTRY("TAIL FIRST", SETTING_NAV_RTH_TAIL_FIRST),
    OSD_SETTING_ENTRY("LAND AFTER RTH", SETTING_NAV_RTH_ALLOW_LANDING),
    OSD_SETTING_ENTRY("LAND VERT SPEED", SETTING_NAV_LANDING_SPEED),
    OSD_SETTING_ENTRY("LAND SPEED MIN AT", SETTING_NAV_LAND_SLOWDOWN_MINALT),
    OSD_SETTING_ENTRY("LAND SPEED SLOW AT", SETTING_NAV_LAND_SLOWDOWN_MAXALT),
    OSD_SETTING_ENTRY("MIN RTH DISTANCE", SETTING_NAV_MIN_RTH_DISTANCE),
    OSD_SETTING_ENTRY("RTH ABORT THRES", SETTING_NAV_RTH_ABORT_THRESHOLD),
    OSD_SETTING_ENTRY("EMERG LANDING SPEED", SETTING_NAV_EMERG_LANDING_SPEED),

    OSD_BACK_AND_END_ENTRY,
 };

static const CMS_Menu cmsx_menuRTH = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUNAVRTH",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = cmsx_menuRTHEntries
};

static const OSD_Entry cmsx_menuFixedWingEntries[] =
{
    OSD_LABEL_ENTRY("-- FIXED WING --"),

    OSD_SETTING_ENTRY("CRUISE THROTTLE", SETTING_NAV_FW_CRUISE_THR),
    OSD_SETTING_ENTRY("MIN THROTTLE", SETTING_NAV_FW_MIN_THR),
    OSD_SETTING_ENTRY("MAX THROTTLE", SETTING_NAV_FW_MAX_THR),
    OSD_SETTING_ENTRY("MAX BANK ANGLE", SETTING_NAV_FW_BANK_ANGLE),
    OSD_SETTING_ENTRY("MAX CLIMB ANGLE", SETTING_NAV_FW_CLIMB_ANGLE),
    OSD_SETTING_ENTRY("MAX DIVE ANGLE", SETTING_NAV_FW_DIVE_ANGLE),
    OSD_SETTING_ENTRY("PITCH TO THR RATIO", SETTING_NAV_FW_PITCH2THR),
    OSD_SETTING_ENTRY("LOITER RADIUS", SETTING_NAV_FW_LOITER_RADIUS),

    OSD_BACK_AND_END_ENTRY,
};

static const CMS_Menu cmsx_menuFixedWing = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUNAVFW",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = cmsx_menuFixedWingEntries
};

static const OSD_Entry cmsx_menuNavigationEntries[] =
{
    OSD_LABEL_ENTRY("-- NAVIGATION --"),

    OSD_SUBMENU_ENTRY("BASIC SETTINGS", &cmsx_menuNavSettings),
    OSD_SUBMENU_ENTRY("RTH", &cmsx_menuRTH),
    OSD_SUBMENU_ENTRY("FIXED WING", &cmsx_menuFixedWing),

    OSD_BACK_AND_END_ENTRY,
};

const CMS_Menu cmsx_menuNavigation = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUNAV",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = cmsx_menuNavigationEntries
};

 #endif
