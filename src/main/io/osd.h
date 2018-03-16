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

#pragma once

#include "common/time.h"
#include "config/parameter_group.h"

#define VISIBLE_FLAG  0x0800
#define VISIBLE(x)    (x & VISIBLE_FLAG)
#define OSD_POS_MAX   0x3FF
#define OSD_POS_MAX_CLI   (OSD_POS_MAX | VISIBLE_FLAG)

typedef enum {
    OSD_RSSI_VALUE,
    OSD_MAIN_BATT_VOLTAGE,
    OSD_CROSSHAIRS,
    OSD_ARTIFICIAL_HORIZON,
    OSD_HORIZON_SIDEBARS,
    OSD_ONTIME,
    OSD_FLYTIME,
    OSD_FLYMODE,
    OSD_CRAFT_NAME,
    OSD_THROTTLE_POS,
    OSD_VTX_CHANNEL,
    OSD_CURRENT_DRAW,
    OSD_MAH_DRAWN,
    OSD_GPS_SPEED,
    OSD_GPS_SATS,
    OSD_ALTITUDE,
    OSD_ROLL_PIDS,
    OSD_PITCH_PIDS,
    OSD_YAW_PIDS,
    OSD_POWER,
    OSD_GPS_LON,
    OSD_GPS_LAT,
    OSD_HOME_DIR,
    OSD_HOME_DIST,
    OSD_HEADING,
    OSD_VARIO,
    OSD_VARIO_NUM,
    OSD_AIR_SPEED,
    OSD_ONTIME_FLYTIME,
    OSD_RTC_TIME,
    OSD_MESSAGES,
    OSD_GPS_HDOP,
    OSD_MAIN_BATT_CELL_VOLTAGE,
    OSD_THROTTLE_POS_AUTO_THR,
    OSD_HEADING_GRAPH,
    OSD_EFFICIENCY_MAH_PER_KM,
    OSD_WH_DRAWN,
    OSD_BATTERY_REMAINING_CAPACITY,
    OSD_BATTERY_REMAINING_PERCENT,
    OSD_EFFICIENCY_WH_PER_KM,
    OSD_TRIP_DIST,
    OSD_ITEM_COUNT // MUST BE LAST
} osd_items_e;

typedef enum {
    OSD_UNIT_IMPERIAL,
    OSD_UNIT_METRIC,
    OSD_UNIT_UK, // Show speed in mp/h, other values in metric
} osd_unit_e;

typedef enum {
    OSD_STATS_ENERGY_UNIT_MAH,
    OSD_STATS_ENERGY_UNIT_WH,
} osd_stats_energy_unit_e;

typedef enum {
    OSD_CROSSHAIRS_STYLE_DEFAULT,
    OSD_CROSSHAIRS_STYLE_AIRCRAFT,
} osd_crosshairs_style_e;

typedef enum {
    OSD_SIDEBAR_SCROLL_NONE,
    OSD_SIDEBAR_SCROLL_ALTITUDE,
    OSD_SIDEBAR_SCROLL_GROUND_SPEED,
    OSD_SIDEBAR_SCROLL_HOME_DISTANCE,
} osd_sidebar_scroll_e;

typedef struct osdConfig_s {
    uint16_t item_pos[OSD_ITEM_COUNT];

    // Alarms
    uint8_t rssi_alarm; // rssi %
    uint16_t time_alarm; // fly minutes
    uint16_t alt_alarm; // positive altitude in m
    uint16_t dist_alarm; // home distance in m
    uint16_t neg_alt_alarm; // abs(negative altitude) in m

    uint8_t video_system;
    uint8_t row_shiftdown;

    // Preferences
    uint8_t main_voltage_decimals;
    uint8_t ahi_reverse_roll;
    osd_crosshairs_style_e crosshairs_style;
    osd_sidebar_scroll_e left_sidebar_scroll;
    osd_sidebar_scroll_e right_sidebar_scroll;
    uint8_t sidebar_scroll_arrows;

    osd_unit_e units;
    osd_stats_energy_unit_e stats_energy_unit;
} osdConfig_t;

PG_DECLARE(osdConfig_t, osdConfig);

struct displayPort_s;
void osdInit(struct displayPort_s *osdDisplayPort);
void osdUpdate(timeUs_t currentTimeUs);
void osdStartFullRedraw(void);
