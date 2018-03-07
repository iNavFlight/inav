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

#if defined(USE_OSD) && defined(USE_CMS)

#include "common/utils.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_osd.h"

#include "fc/settings.h"

#include "io/osd.h"

static const OSD_Entry cmsx_menuAlarmsEntries[] =
{
    OSD_LABEL_ENTRY("--- ALARMS ---"),

    OSD_SETTING_ENTRY_STEP("RSSI", SETTING_OSD_RSSI_ALARM, 5),
    OSD_SETTING_ENTRY("FLY TIME", SETTING_OSD_TIME_ALARM),
    OSD_SETTING_ENTRY("MAX ALT", SETTING_OSD_ALT_ALARM),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

const CMS_Menu cmsx_menuAlarms = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUALARMS",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = cmsx_menuAlarmsEntries,
};


#define OSD_OSD_ELEMENT_ENTRY(name, osd_item_id) {name, OME_VISIBLE, NULL, (void *)osd_item_id, 0}

static const OSD_Entry menuOsdActiveElemsEntries[] =
{
    OSD_LABEL_ENTRY("--- ACTIV ELEM ---"),

    OSD_OSD_ELEMENT_ENTRY("RSSI", OSD_RSSI_VALUE),
    OSD_OSD_ELEMENT_ENTRY("MAIN BATTERY", OSD_MAIN_BATT_VOLTAGE),
    OSD_OSD_ELEMENT_ENTRY("HORIZON", OSD_ARTIFICIAL_HORIZON),
    OSD_OSD_ELEMENT_ENTRY("HORIZON SIDEBARS", OSD_HORIZON_SIDEBARS),
    OSD_OSD_ELEMENT_ENTRY("UPTIME", OSD_ONTIME),
    OSD_OSD_ELEMENT_ENTRY("FLY TIME", OSD_FLYTIME),
    OSD_OSD_ELEMENT_ENTRY("FLY MODE", OSD_FLYMODE),
    OSD_OSD_ELEMENT_ENTRY("NAME", OSD_CRAFT_NAME),
    OSD_OSD_ELEMENT_ENTRY("THROTTLE", OSD_THROTTLE_POS),
#ifdef VTX
    OSD_OSD_ELEMENT_ENTRY("VTX CHAN", OSD_VTX_CHANNEL),
#endif // VTX
    OSD_OSD_ELEMENT_ENTRY("CURRENT (A)", OSD_CURRENT_DRAW),
    OSD_OSD_ELEMENT_ENTRY("USED MAH", OSD_MAH_DRAWN),
#ifdef USE_GPS
    OSD_OSD_ELEMENT_ENTRY("HOME DIR.", OSD_HOME_DIR),
    OSD_OSD_ELEMENT_ENTRY("HOME DIST.", OSD_HOME_DIST),
    OSD_OSD_ELEMENT_ENTRY("GPS SPEED", OSD_GPS_SPEED),
    OSD_OSD_ELEMENT_ENTRY("GPS SATS.", OSD_GPS_SATS),
    OSD_OSD_ELEMENT_ENTRY("GPS LAT", OSD_GPS_LAT),
    OSD_OSD_ELEMENT_ENTRY("GPS LON.", OSD_GPS_LON),
    OSD_OSD_ELEMENT_ENTRY("HEADING", OSD_HEADING),
#endif // GPS
#if defined(USE_BARO) || defined(USE_GPS)
    OSD_OSD_ELEMENT_ENTRY("VARIO", OSD_VARIO),
    OSD_OSD_ELEMENT_ENTRY("VARIO NUM", OSD_VARIO_NUM),
#endif // defined
    OSD_OSD_ELEMENT_ENTRY("ALTITUDE", OSD_ALTITUDE),
    OSD_OSD_ELEMENT_ENTRY("AIR SPEED", OSD_AIR_SPEED),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

const CMS_Menu menuOsdActiveElems = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUOSDACT",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = menuOsdActiveElemsEntries
};

static const OSD_Entry cmsx_menuOsdLayoutEntries[] =
{
    OSD_LABEL_ENTRY("---SCREEN LAYOUT---"),
    OSD_SUBMENU_ENTRY("ACTIVE ELEM", &menuOsdActiveElems),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

const CMS_Menu cmsx_menuOsdLayout = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENULAYOUT",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = cmsx_menuOsdLayoutEntries
};
#endif // CMS
