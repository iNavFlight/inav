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

#include "build/debug.h"

#include "common/utils.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_osd.h"

#include "fc/settings.h"

#include "io/osd.h"

#define OSD_ITEM_ENTRY(label, item_id)      ((OSD_Entry){ label, OME_Submenu, (void *)item_id, &cmsx_menuOsdElementActions, 0 })
#define OSD_ITEM_GET_ID(entry)              ((int)entry->func)

static int osdCurrentLayout = -1;
static int osdCurrentItem = -1;

static uint8_t osdCurrentElementRow = 0;
static uint8_t osdCurrentElementColumn = 0;
static uint8_t osdCurrentElementVisible = 0;

static long osdElementsOnEnter(const OSD_Entry *from);
static long osdElementsOnExit(const OSD_Entry *from);
static long osdElemActionsOnEnter(const OSD_Entry *from);

static const OSD_Entry cmsx_menuAlarmsEntries[] = {
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

static long cmsx_osdElementOnChange(displayPort_t *displayPort, const void *ptr)
{
    UNUSED(ptr);

    uint16_t *pos = &osdConfigMutable()->item_pos[osdCurrentLayout][osdCurrentItem];
    *pos = OSD_POS(osdCurrentElementColumn, osdCurrentElementRow);
    if (osdCurrentElementVisible) {
        *pos |= OSD_VISIBLE_FLAG;
    }
    cmsYieldDisplay(displayPort, 500);
    return 0;
}

static long osdElementPreview(displayPort_t *displayPort, const void *ptr)
{
    UNUSED(ptr);

    cmsYieldDisplay(displayPort, 2000);

    return 0;
}

static const OSD_Entry menuOsdElemActionsEntries[] = {

    OSD_BOOL_CALLBACK_ENTRY("ENABLED", cmsx_osdElementOnChange, &osdCurrentElementVisible),
    OSD_UINT8_CALLBACK_ENTRY("ROW", cmsx_osdElementOnChange, (&(const OSD_UINT8_t){ &osdCurrentElementRow, 0, OSD_X(OSD_POS_MAX), 1 })),
    OSD_UINT8_CALLBACK_ENTRY("COLUMN", cmsx_osdElementOnChange, (&(const OSD_UINT8_t){ &osdCurrentElementColumn, 0, OSD_Y(OSD_POS_MAX), 1 })),
    OSD_FUNC_CALL_ENTRY("PREVIEW", osdElementPreview),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

static const OSD_Entry menuOsdFixedElemActionsEntries[] = {

    OSD_BOOL_CALLBACK_ENTRY("ENABLED", cmsx_osdElementOnChange, &osdCurrentElementVisible),
    OSD_FUNC_CALL_ENTRY("PREVIEW", osdElementPreview),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

static CMS_Menu cmsx_menuOsdElementActions = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUOSDELEM",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = osdElemActionsOnEnter,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = menuOsdElemActionsEntries,
};

static long osdElemActionsOnEnter(const OSD_Entry *from)
{
    osdCurrentItem = OSD_ITEM_GET_ID(from);
    uint16_t pos = osdConfig()->item_pos[osdCurrentLayout][osdCurrentItem];
    osdCurrentElementColumn = OSD_X(pos);
    osdCurrentElementRow = OSD_Y(pos);
    osdCurrentElementVisible = OSD_VISIBLE(pos) ? 1 : 0;
    if (osdItemIsFixed(osdCurrentItem)) {
        cmsx_menuOsdElementActions.entries = menuOsdFixedElemActionsEntries;
    } else {
        cmsx_menuOsdElementActions.entries = menuOsdElemActionsEntries;
    }
    return 0;
}

#define OSD_ELEMENT_ENTRY(name, osd_item_id)    OSD_ITEM_ENTRY(name, osd_item_id)

static const OSD_Entry menuOsdElemsEntries[] =
{
    OSD_LABEL_ENTRY("--- OSD ---"),

    OSD_ELEMENT_ENTRY("RSSI", OSD_RSSI_VALUE),
    OSD_ELEMENT_ENTRY("MAIN BATTERY", OSD_MAIN_BATT_VOLTAGE),
    OSD_ELEMENT_ENTRY("HORIZON", OSD_ARTIFICIAL_HORIZON),
    OSD_ELEMENT_ENTRY("HORIZON SIDEBARS", OSD_HORIZON_SIDEBARS),
    OSD_ELEMENT_ENTRY("UPTIME", OSD_ONTIME),
    OSD_ELEMENT_ENTRY("FLY TIME", OSD_FLYTIME),
    OSD_ELEMENT_ENTRY("FLY MODE", OSD_FLYMODE),
    OSD_ELEMENT_ENTRY("NAME", OSD_CRAFT_NAME),
    OSD_ELEMENT_ENTRY("THROTTLE", OSD_THROTTLE_POS),
#ifdef VTX
    OSD_ELEMENT_ENTRY("VTX CHAN", OSD_VTX_CHANNEL),
#endif // VTX
    OSD_ELEMENT_ENTRY("CURRENT (A)", OSD_CURRENT_DRAW),
    OSD_ELEMENT_ENTRY("USED MAH", OSD_MAH_DRAWN),
#ifdef USE_GPS
    OSD_ELEMENT_ENTRY("HOME DIR.", OSD_HOME_DIR),
    OSD_ELEMENT_ENTRY("HOME DIST.", OSD_HOME_DIST),
    OSD_ELEMENT_ENTRY("GPS SPEED", OSD_GPS_SPEED),
    OSD_ELEMENT_ENTRY("GPS SATS.", OSD_GPS_SATS),
    OSD_ELEMENT_ENTRY("GPS LAT", OSD_GPS_LAT),
    OSD_ELEMENT_ENTRY("GPS LON.", OSD_GPS_LON),
    OSD_ELEMENT_ENTRY("HEADING", OSD_HEADING),
#endif // GPS
#if defined(USE_BARO) || defined(USE_GPS)
    OSD_ELEMENT_ENTRY("VARIO", OSD_VARIO),
    OSD_ELEMENT_ENTRY("VARIO NUM", OSD_VARIO_NUM),
#endif // defined
    OSD_ELEMENT_ENTRY("ALTITUDE", OSD_ALTITUDE),
    OSD_ELEMENT_ENTRY("AIR SPEED", OSD_AIR_SPEED),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

const CMS_Menu menuOsdElements = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENUOSDELEMS",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = osdElementsOnEnter,
    .onExit = osdElementsOnExit,
    .onGlobalExit = NULL,
    .entries = menuOsdElemsEntries,
};


#define OSD_LAYOUT_SUBMENU_ENTRY(label)   OSD_SUBMENU_ENTRY(label, &menuOsdElements)

static const OSD_Entry cmsx_menuOsdLayoutEntries[] =
{
    OSD_LABEL_ENTRY("---SCREEN LAYOUT---"),

    OSD_LAYOUT_SUBMENU_ENTRY("DEFAULT"),
#if OSD_ALTERNATE_LAYOUT_COUNT > 0
    OSD_LAYOUT_SUBMENU_ENTRY("ALTERNATE 1"),
#if OSD_ALTERNATE_LAYOUT_COUNT > 1
    OSD_LAYOUT_SUBMENU_ENTRY("ALTERNATE 2"),
#if OSD_ALTERNATE_LAYOUT_COUNT > 2
    OSD_LAYOUT_SUBMENU_ENTRY("ALTERNATE 3"),
#endif
#endif
#endif

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
    .entries = cmsx_menuOsdLayoutEntries,
};

static long osdElementsOnEnter(const OSD_Entry *from)
{
    // First entry is the label. Store the current layout
    // and override it on the OSD so previews so this layout.
    osdCurrentLayout = from - cmsx_menuOsdLayoutEntries - 1;
    osdOverrideLayout(osdCurrentLayout);
    return 0;
}

static long osdElementsOnExit(const OSD_Entry *from)
{
    UNUSED(from);

    // Stop overriding OSD layout
    osdOverrideLayout(-1);
    return 0;
}

#endif // CMS
