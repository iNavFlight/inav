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
#include <ctype.h>

#include "platform.h"

#include "build/version.h"

#ifdef USE_CMS

#include "common/axis.h"
#include "flight/pid.h"
#include "flight/mixer.h"
#include "flight/servos.h"

#include "fc/config.h"
#include "fc/rc_controls.h"
#include "fc/runtime_config.h"

#include "config/feature.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_ledstrip.h"

#ifdef USE_LED_STRIP

static bool cmsx_FeatureLedStrip_Enabled(bool *enabled)
{
    if (enabled) {
        if (*enabled) {
            featureSet(FEATURE_LED_STRIP);
        } else {
            featureClear(FEATURE_LED_STRIP);
        }
    }
    return featureConfigured(FEATURE_LED_STRIP);
}

static const OSD_Entry cmsx_menuLedstripEntries[] =
{
    OSD_LABEL_ENTRY("-- LED STRIP --"),
    OSD_BOOL_FUNC_ENTRY("ENABLED", cmsx_FeatureLedStrip_Enabled),

    OSD_BACK_AND_END_ENTRY,
};

const CMS_Menu cmsx_menuLedstrip = {
#ifdef CMS_MENU_DEBUG
    .GUARD_text = "MENULED",
    .GUARD_type = OME_MENU,
#endif
    .onEnter = NULL,
    .onExit = NULL,
    .onGlobalExit = NULL,
    .entries = cmsx_menuLedstripEntries
};
#endif // LED_STRIP
#endif // CMS
