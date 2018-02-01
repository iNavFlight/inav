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

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_misc.h"

#include "flight/mixer.h"

#include "fc/config.h"
#include "fc/rc_controls.h"

#include "rx/rx.h"

#include "sensors/battery.h"

//
// Misc
//

static long cmsx_menuRcConfirmBack(const OSD_Entry *self)
{
    if (self && self->type == OME_Back)
        return 0;
    else
        return -1;
}

//
// RC preview
//
static OSD_Entry cmsx_menuRcEntries[] =
{
    { "-- RC PREV --", OME_Label, NULL, NULL, 0},

    { "ROLL",  OME_INT16, NULL, &(OSD_INT16_t){ &rcData[ROLL],     1, 2500, 0 }, DYNAMIC },
    { "PITCH", OME_INT16, NULL, &(OSD_INT16_t){ &rcData[PITCH],    1, 2500, 0 }, DYNAMIC },
    { "THR",   OME_INT16, NULL, &(OSD_INT16_t){ &rcData[THROTTLE], 1, 2500, 0 }, DYNAMIC },
    { "YAW",   OME_INT16, NULL, &(OSD_INT16_t){ &rcData[YAW],      1, 2500, 0 }, DYNAMIC },

    { "AUX1",  OME_INT16, NULL, &(OSD_INT16_t){ &rcData[AUX1],     1, 2500, 0 }, DYNAMIC },
    { "AUX2",  OME_INT16, NULL, &(OSD_INT16_t){ &rcData[AUX2],     1, 2500, 0 }, DYNAMIC },
    { "AUX3",  OME_INT16, NULL, &(OSD_INT16_t){ &rcData[AUX3],     1, 2500, 0 }, DYNAMIC },
    { "AUX4",  OME_INT16, NULL, &(OSD_INT16_t){ &rcData[AUX4],     1, 2500, 0 }, DYNAMIC },

    { "BACK",  OME_Back, NULL, NULL, 0},
    {NULL, OME_END, NULL, NULL, 0}
};

CMS_Menu cmsx_menuRcPreview = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XRCPREV",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = cmsx_menuRcConfirmBack,
    .onGlobalExit = NULL,
    .entries = cmsx_menuRcEntries
};

static OSD_Entry menuMiscEntries[]=
{
    { "-- MISC --", OME_Label, NULL, NULL, 0 },

    OSD_SETTING_ENTRY("MIN THR", SETTING_MIN_THROTTLE),
#ifdef USE_ADC
    OSD_SETTING_ENTRY("VBATCELL MAX", SETTING_VBAT_MAX_CELL_VOLTAGE),
    OSD_SETTING_ENTRY("VBATCELL WARN", SETTING_VBAT_WARNING_CELL_VOLTAGE),
    OSD_SETTING_ENTRY("VBATCELL MIN", SETTING_VBAT_MIN_CELL_VOLTAGE),
    OSD_SETTING_ENTRY("BAT CAP UNIT", SETTING_BATTERY_CAPACITY_UNIT),
    OSD_SETTING_ENTRY("BAT CAPACITY", SETTING_BATTERY_CAPACITY),
    OSD_SETTING_ENTRY("BAT CAP WARN", SETTING_BATTERY_CAPACITY_WARNING),
    OSD_SETTING_ENTRY("BAT CAP CRIT", SETTING_BATTERY_CAPACITY_CRITICAL),
#ifdef USE_OSD
    OSD_SETTING_ENTRY("OSD VOLT DECIMALS", SETTING_OSD_MAIN_VOLTAGE_DECIMALS),
    OSD_SETTING_ENTRY("STATS ENERGY UNIT", SETTING_OSD_STATS_ENERGY_UNIT),
#endif /* USE_OSD */
#endif /* USE_ADC */

    { "RC PREV",    OME_Submenu, cmsMenuChange, &cmsx_menuRcPreview, 0},

    { "BACK", OME_Back, NULL, NULL, 0},
    { NULL, OME_END, NULL, NULL, 0}
};

CMS_Menu cmsx_menuMisc = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "XMISC",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = menuMiscEntries
};

#endif // CMS
