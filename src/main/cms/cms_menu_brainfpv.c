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

// Menu contents for PID, RATES, RC preview, misc
// Should be part of the relevant .c file.

#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>

#include "platform.h"
#include "common/utils.h"

#ifdef USE_CMS

#include "build/version.h"

#include "drivers/system.h"

//#include "common/typeconversion.h"

#include "cms/cms.h"
#include "cms/cms_types.h"
#include "cms/cms_menu_imu.h"

#include "brainfpv/video.h"
#include "brainfpv/osd_utils.h"
#include "brainfpv/ir_transponder.h"
#include "brainfpv/brainfpv_osd.h"

bfOsdConfig_t bfOsdConfigCms;

static long menuBrainFPVOnEnter(const OSD_Entry *from)
{
    UNUSED(from);

    memcpy(&bfOsdConfigCms, bfOsdConfig(), sizeof(bfOsdConfig_t));
    return 0;
}

static long menuBrainFPVOnExit(const OSD_Entry *from)
{
    UNUSED(from);

    memcpy(bfOsdConfigMutable(), &bfOsdConfigCms, sizeof(bfOsdConfig_t));
    return 0;
}

const char *STICKS_DISPLAY_NAMES[] = {"OFF", "MODE2", "MODE1"};
const char *FONT_NAMES[] = {"DEFAULT", "LARGE", "CLARITY", "VISION"};

OSD_Entry cmsx_menuBrainFPVOsdEntries[] =
{
    OSD_LABEL_ENTRY("-- BRAIN OSD ------"),

    OSD_TAB_ENTRY("FONT", (&(const OSD_TAB_t){&bfOsdConfigCms.font, 3, &FONT_NAMES[0]})),
    OSD_UINT8_ENTRY("OSD WHITE", (&(const OSD_UINT8_t){ &bfOsdConfigCms.white_level, 100, 120, 1 })),
    OSD_UINT8_ENTRY("OSD BLACK", (&(const OSD_UINT8_t){ &bfOsdConfigCms.black_level, 15, 40, 1 })),
    OSD_BOOL_ENTRY("INVERT",  &bfOsdConfigCms.invert),
    OSD_UINT8_ENTRY("OSD SYNC TH", (&(const OSD_UINT8_t){ &bfOsdConfigCms.sync_threshold, BRAINFPV_OSD_SYNC_TH_MIN, BRAINFPV_OSD_SYNC_TH_MAX, 1 })),
    OSD_INT8_ENTRY("OSD X OFF", (&(const OSD_INT8_t){ &bfOsdConfigCms.x_offset, -8, 7, 1 })),
    OSD_UINT8_ENTRY("OSD X SC", (&(const OSD_UINT8_t){ &bfOsdConfigCms.x_scale, 0, 15, 1 })),
    OSD_INT8_ENTRY("OSD Y OFF", (&(const OSD_INT8_t){ &bfOsdConfigCms.center_mark_offset, -100, 100, 1 })),
    OSD_BOOL_ENTRY("3D MODE",  &bfOsdConfigCms.sbs_3d_enabled),
    OSD_UINT8_ENTRY("3D R SHIFT", (&(const OSD_UINT8_t){ &bfOsdConfigCms.sbs_3d_right_eye_offset, 10, 40, 1 })),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

CMS_Menu cmsx_menuBrainFPVOsd = {
    .onEnter = NULL,
    .onExit = NULL,
    .entries = cmsx_menuBrainFPVOsdEntries,
};

OSD_Entry cmsx_menuBrainFPVEntires[] =
{
    OSD_LABEL_ENTRY("-- BRAINFPV --"),

    OSD_SUBMENU_ENTRY("BRAIN OSD", &cmsx_menuBrainFPVOsd),
    OSD_UINT8_ENTRY("AHI STEPS", (&(const OSD_UINT8_t){ &bfOsdConfigCms.ahi_steps, 0, 9, 1 })),
    OSD_BOOL_ENTRY("ALTITUDE SCALE",  &bfOsdConfigCms.altitude_scale),
    OSD_BOOL_ENTRY("SPEED SCALE",  &bfOsdConfigCms.speed_scale),
    OSD_UINT16_ENTRY("RADAR MAX DIST M", (&(const OSD_UINT16_t){ &bfOsdConfigCms.radar_max_dist_m, 10, 32767, 10 })),
    OSD_TAB_ENTRY("SHOW STICKS", (&(const OSD_TAB_t){&bfOsdConfigCms.sticks_display, 2, &STICKS_DISPLAY_NAMES[0]})),
    OSD_BOOL_ENTRY("SHOW LOGO ON ARM",  &bfOsdConfigCms.show_logo_on_arm),
    OSD_BOOL_ENTRY("SHOW PILOT LOGO",  &bfOsdConfigCms.show_pilot_logo),

    OSD_BACK_ENTRY,
    OSD_END_ENTRY,
};

CMS_Menu cmsx_menuBrainFPV = {
    .onEnter = menuBrainFPVOnEnter,
    .onExit = menuBrainFPVOnExit,
    .entries = cmsx_menuBrainFPVEntires,
};
#endif // CMS
