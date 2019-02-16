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

#include "platform.h"

#ifdef USE_CMS

#include "common/utils.h"

#include "config/feature.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_misc.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/settings.h"

#include "sensors/battery.h"

// Battery menu

static uint8_t battDispProfileIndex;
static uint8_t battProfileIndex;
static bool featureProfAutoswitchEnabled;
static char battProfileIndexString[] = " p";


static long cmsx_menuBattery_onEnter(const OSD_Entry *from)
{
    UNUSED(from);

    battProfileIndex = getConfigBatteryProfile();
    battDispProfileIndex = battProfileIndex + 1;
    battProfileIndexString[1] = '0' + battDispProfileIndex;
    featureProfAutoswitchEnabled = feature(FEATURE_BAT_PROFILE_AUTOSWITCH);

    return 0;
}

static long cmsx_menuBattery_onExit(const OSD_Entry *self)
{
    UNUSED(self);

    setConfigBatteryProfile(battProfileIndex);
    activateBatteryProfile();

    if (featureProfAutoswitchEnabled) {
        featureSet(FEATURE_BAT_PROFILE_AUTOSWITCH);
    } else {
        featureClear(FEATURE_BAT_PROFILE_AUTOSWITCH);
    }

    return 0;
}

static long cmsx_onBatteryProfileIndexChange(displayPort_t *displayPort, const void *ptr)
{
    UNUSED(displayPort);
    UNUSED(ptr);

    battProfileIndex = battDispProfileIndex - 1;
    battProfileIndexString[1] = '0' + battDispProfileIndex;
    batteryDisableProfileAutoswitch();

    return 0;
}

static long cmsx_menuBattSettings_onEnter(const OSD_Entry *from)
{
    UNUSED(from);

    setConfigBatteryProfile(battProfileIndex);

    return 0;
}

static const OSD_Entry menuBattSettingsEntries[]=
{
    OSD_LABEL_DATA_ENTRY("-- BATT SETTINGS --", battProfileIndexString),

#ifdef USE_ADC
    OSD_SETTING_ENTRY("CELLS", SETTING_BAT_CELLS),
    OSD_SETTING_ENTRY("CELL DET.", SETTING_VBAT_CELL_DETECT_VOLTAGE),
    OSD_SETTING_ENTRY("CELL MAX", SETTING_VBAT_MAX_CELL_VOLTAGE),
    OSD_SETTING_ENTRY("CELL WARN", SETTING_VBAT_WARNING_CELL_VOLTAGE),
    OSD_SETTING_ENTRY("CELL MIN", SETTING_VBAT_MIN_CELL_VOLTAGE),
#endif /* USE_ADC */
    OSD_SETTING_ENTRY("CAP UNIT", SETTING_BATTERY_CAPACITY_UNIT),
    OSD_SETTING_ENTRY("CAPACITY", SETTING_BATTERY_CAPACITY),
    OSD_SETTING_ENTRY("CAP WARN", SETTING_BATTERY_CAPACITY_WARNING),
    OSD_SETTING_ENTRY("CAP CRIT", SETTING_BATTERY_CAPACITY_CRITICAL),

    OSD_BACK_AND_END_ENTRY,
};

static CMS_Menu cmsx_menuBattSettings = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XBATT",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = cmsx_menuBattSettings_onEnter,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = menuBattSettingsEntries
};

static OSD_Entry menuBatteryEntries[]=
{
    OSD_LABEL_ENTRY("-- BATTERY --"),

#ifdef USE_ADC
    OSD_BOOL_ENTRY("PROF AUTOSWITCH", &featureProfAutoswitchEnabled),
#endif
    OSD_UINT8_CALLBACK_ENTRY("PROF", cmsx_onBatteryProfileIndexChange, (&(const OSD_UINT8_t){ &battDispProfileIndex, 1, MAX_BATTERY_PROFILE_COUNT, 1})),
    OSD_SUBMENU_ENTRY("SETTINGS", &cmsx_menuBattSettings),

    OSD_BACK_AND_END_ENTRY,
};

CMS_Menu cmsx_menuBattery = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XBATT",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = cmsx_menuBattery_onEnter,
    .onExit = cmsx_menuBattery_onExit,
    .onGlobalExit = NULL,
    .entries = menuBatteryEntries
};

#endif // CMS
