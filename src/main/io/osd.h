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

#include "drivers/osd.h"
#include "drivers/display.h"

#ifndef OSD_ALTERNATE_LAYOUT_COUNT
#define OSD_ALTERNATE_LAYOUT_COUNT 3
#endif
#define OSD_LAYOUT_COUNT (OSD_ALTERNATE_LAYOUT_COUNT + 1)

// 00vb yyyy yyxx xxxx
// (visible)(blink)(yCoord)(xCoord)

#define OSD_VISIBLE_FLAG    0x2000
#define OSD_VISIBLE(x)      ((x) & OSD_VISIBLE_FLAG)

#define OSD_POS(x,y)        (((x) & 0x3F) | (((y) & 0x3F) << 6))
#define OSD_X(x)            ((x) & 0x3F)
#define OSD_Y(x)            (((x) >> 6) & 0x3F)
#define OSD_POS_MAX         0xFFF

// For DJI compatibility
#define OSD_VISIBLE_FLAG_SD 0x0800
#define OSD_POS_SD(x,y)     (((x) & 0x1F) | (((y) & 0x1F) << 5))

#define OSD_POS_MAX_CLI     (OSD_POS_MAX | OSD_VISIBLE_FLAG)

#define OSD_HOMING_LIM_H1 6
#define OSD_HOMING_LIM_H2 16
#define OSD_HOMING_LIM_H3 38
#define OSD_HOMING_LIM_V1 5
#define OSD_HOMING_LIM_V2 10
#define OSD_HOMING_LIM_V3 15

// Message defines to be use in OSD and/or telemetry exports
#define OSD_MSG_RC_RX_LINK_LOST     "!RC RX LINK LOST!"
#define OSD_MSG_TURN_ARM_SW_OFF     "TURN ARM SWITCH OFF"
#define OSD_MSG_DISABLED_BY_FS      "DISABLED BY FAILSAFE"
#define OSD_MSG_AIRCRAFT_UNLEVEL    "AIRCRAFT IS NOT LEVEL"
#define OSD_MSG_SENSORS_CAL         "SENSORS CALIBRATING"
#define OSD_MSG_SYS_OVERLOADED      "SYSTEM OVERLOADED"
#define OSD_MSG_WAITING_GPS_FIX     "WAITING FOR GPS FIX"
#define OSD_MSG_DISABLE_NAV_FIRST   "DISABLE NAVIGATION FIRST"
#define OSD_MSG_JUMP_WP_MISCONFIG   "JUMP WAYPOINT MISCONFIGURED"
#define OSD_MSG_MAG_NOT_CAL         "COMPASS NOT CALIBRATED"
#define OSD_MSG_ACC_NOT_CAL         "ACCELEROMETER NOT CALIBRATED"
#define OSD_MSG_DISARM_1ST          "DISABLE ARM SWITCH FIRST"
#define OSD_MSG_GYRO_FAILURE        "GYRO FAILURE"
#define OSD_MSG_ACC_FAIL            "ACCELEROMETER FAILURE"
#define OSD_MSG_MAG_FAIL            "COMPASS FAILURE"
#define OSD_MSG_BARO_FAIL           "BAROMETER FAILURE"
#define OSD_MSG_GPS_FAIL            "GPS FAILURE"
#define OSD_MSG_RANGEFINDER_FAIL    "RANGE FINDER FAILURE"
#define OSD_MSG_PITOT_FAIL          "PITOT METER FAILURE"
#define OSD_MSG_HW_FAIL             "HARDWARE FAILURE"
#define OSD_MSG_FS_EN               "FAILSAFE MODE ENABLED"
#define OSD_MSG_NO_RC_LINK          "NO RC LINK"
#define OSD_MSG_THROTTLE_NOT_LOW    "THROTTLE IS NOT LOW"
#define OSD_MSG_ROLLPITCH_OFFCENTER "ROLLPITCH NOT CENTERED"
#define OSD_MSG_AUTOTRIM_ACTIVE     "AUTOTRIM IS ACTIVE"
#define OSD_MSG_NOT_ENOUGH_MEMORY   "NOT ENOUGH MEMORY"
#define OSD_MSG_INVALID_SETTING     "INVALID SETTING"
#define OSD_MSG_CLI_ACTIVE          "CLI IS ACTIVE"
#define OSD_MSG_PWM_INIT_ERROR      "PWM INIT ERROR"
#define OSD_MSG_NO_PREARM           "NO PREARM"
#define OSD_MSG_DSHOT_BEEPER        "MOTOR BEEPER ACTIVE"
#define OSD_MSG_RTH_FS              "(RTH)"
#define OSD_MSG_EMERG_LANDING_FS    "(EMERGENCY LANDING)"
#define OSD_MSG_MOVE_EXIT_FS        "!MOVE STICKS TO EXIT FS!"
#define OSD_MSG_STARTING_RTH        "STARTING RTH"
#define OSD_MSG_RTH_CLIMB           "ADJUSTING RTH ALTITUDE"
#define OSD_MSG_RTH_TRACKBACK       "RTH BACK TRACKING"
#define OSD_MSG_HEADING_HOME        "EN ROUTE TO HOME"
#define OSD_MSG_RTH_LINEAR_DESCENT  "BEGIN LINEAR DESCENT"
#define OSD_MSG_WP_FINISHED         "WP END>HOLDING POSITION"
#define OSD_MSG_WP_LANDED           "WP END>LANDED"
#define OSD_MSG_PREPARE_NEXT_WP     "PREPARING FOR NEXT WAYPOINT"
#define OSD_MSG_ADJUSTING_WP_ALT    "ADJUSTING WP ALTITUDE"
#define OSD_MSG_MISSION_PLANNER     "(WP MISSION PLANNER)"
#define OSD_MSG_WP_RTH_CANCEL       "CANCEL WP TO EXIT RTH"
#define OSD_MSG_WP_MISSION_LOADED   "* MISSION LOADED *"
#define OSD_MSG_EMERG_LANDING       "EMERGENCY LANDING"
#define OSD_MSG_LANDING             "LANDING"
#define OSD_MSG_LOITERING_HOME      "LOITERING AROUND HOME"
#define OSD_MSG_HOVERING            "HOVERING"
#define OSD_MSG_LANDED              "LANDED"
#define OSD_MSG_PREPARING_LAND      "PREPARING TO LAND"
#define OSD_MSG_AUTOLAUNCH          "AUTOLAUNCH"
#define OSD_MSG_AUTOLAUNCH_MANUAL   "AUTOLAUNCH (MANUAL)"
#define OSD_MSG_ALTITUDE_HOLD       "(ALTITUDE HOLD)"
#define OSD_MSG_AUTOTRIM            "(AUTOTRIM)"
#define OSD_MSG_AUTOTUNE            "(AUTOTUNE)"
#define OSD_MSG_AUTOTUNE_ACRO       "SWITCH TO ACRO"
#define OSD_MSG_AUTOLEVEL           "(AUTO LEVEL TRIM)"
#define OSD_MSG_HEADFREE            "(HEADFREE)"
#define OSD_MSG_NAV_SOARING         "(SOARING)"
#define OSD_MSG_UNABLE_ARM          "UNABLE TO ARM"
#define OSD_MSG_SAVING_SETTNGS      "** SAVING SETTINGS **"
#define OSD_MSG_SETTINGS_SAVED      "** SETTINGS SAVED **"
#define OSD_MSG_ANGLEHOLD_ROLL      "(ANGLEHOLD ROLL)"
#define OSD_MSG_ANGLEHOLD_PITCH     "(ANGLEHOLD PITCH)"
#define OSD_MSG_ANGLEHOLD_LEVEL     "(ANGLEHOLD LEVEL)"
#define OSD_MSG_MOVE_STICKS         "MOVE STICKS TO ABORT"

#ifdef USE_DEV_TOOLS
#define OSD_MSG_GRD_TEST_MODE       "GRD TEST > MOTORS DISABLED"
#endif

#if defined(USE_SAFE_HOME)
#define OSD_MSG_DIVERT_SAFEHOME     "DIVERTING TO SAFEHOME"
#define OSD_MSG_LOITERING_SAFEHOME  "LOITERING AROUND SAFEHOME"
#endif

#if defined(USE_GEOZONE)
#define OSD_MSG_NFZ                 "NO FLY ZONE"
#define OSD_MSG_LEAVING_FZ          "LEAVING FZ IN %s"
#define OSD_MSG_OUTSIDE_FZ          "OUTSIDE FZ"
#define OSD_MSG_ENTERING_NFZ        "ENTERING NFZ IN %s %s"
#define OSD_MSG_AVOIDING_FB         "AVOIDING FENCE BREACH"
#define OSD_MSG_RETURN_TO_ZONE      "RETURN TO FZ"
#define OSD_MSG_FLYOUT_NFZ          "FLY OUT NFZ"
#define OSD_MSG_AVOIDING_ALT_BREACH "REACHED ZONE ALTITUDE LIMIT"
#define OSD_MSG_AVOID_ZONES_RTH     "AVOIDING NO FLY ZONES"
#define OSD_MSG_GEOZONE_ACTION      "PERFORM ACTION IN %s %s"
#endif

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
    OSD_SCALED_THROTTLE_POS,
    OSD_HEADING_GRAPH,
    OSD_EFFICIENCY_MAH_PER_KM,
    OSD_WH_DRAWN,
    OSD_BATTERY_REMAINING_CAPACITY,
    OSD_BATTERY_REMAINING_PERCENT,
    OSD_EFFICIENCY_WH_PER_KM,
    OSD_TRIP_DIST,
    OSD_ATTITUDE_PITCH,
    OSD_ATTITUDE_ROLL,
    OSD_MAP_NORTH,
    OSD_MAP_TAKEOFF,
    OSD_RADAR,
    OSD_WIND_SPEED_HORIZONTAL,
    OSD_WIND_SPEED_VERTICAL,
    OSD_REMAINING_FLIGHT_TIME_BEFORE_RTH,
    OSD_REMAINING_DISTANCE_BEFORE_RTH,
    OSD_HOME_HEADING_ERROR,
    OSD_COURSE_HOLD_ERROR,
    OSD_COURSE_HOLD_ADJUSTMENT,
    OSD_SAG_COMPENSATED_MAIN_BATT_VOLTAGE,
    OSD_MAIN_BATT_SAG_COMPENSATED_CELL_VOLTAGE,
    OSD_POWER_SUPPLY_IMPEDANCE,
    OSD_LEVEL_PIDS,
    OSD_POS_XY_PIDS,
    OSD_POS_Z_PIDS,
    OSD_VEL_XY_PIDS,
    OSD_VEL_Z_PIDS,
    OSD_HEADING_P,
    OSD_BOARD_ALIGN_ROLL,
    OSD_BOARD_ALIGN_PITCH,
    OSD_RC_EXPO,
    OSD_RC_YAW_EXPO,
    OSD_THROTTLE_EXPO,
    OSD_PITCH_RATE,
    OSD_ROLL_RATE,
    OSD_YAW_RATE,
    OSD_MANUAL_RC_EXPO,
    OSD_MANUAL_RC_YAW_EXPO,
    OSD_MANUAL_PITCH_RATE,
    OSD_MANUAL_ROLL_RATE,
    OSD_MANUAL_YAW_RATE,
    OSD_NAV_FW_CRUISE_THR,
    OSD_NAV_FW_PITCH2THR,
    OSD_FW_MIN_THROTTLE_DOWN_PITCH_ANGLE,
    OSD_DEBUG, // Intentionally absent from configurator and CMS. Set it from CLI.
    OSD_FW_ALT_PID_OUTPUTS,
    OSD_FW_POS_PID_OUTPUTS,
    OSD_MC_VEL_X_PID_OUTPUTS,
    OSD_MC_VEL_Y_PID_OUTPUTS,
    OSD_MC_VEL_Z_PID_OUTPUTS,
    OSD_MC_POS_XYZ_P_OUTPUTS,
    OSD_3D_SPEED,
    OSD_IMU_TEMPERATURE,
    OSD_BARO_TEMPERATURE,
    OSD_TEMP_SENSOR_0_TEMPERATURE,
    OSD_TEMP_SENSOR_1_TEMPERATURE,
    OSD_TEMP_SENSOR_2_TEMPERATURE,
    OSD_TEMP_SENSOR_3_TEMPERATURE,
    OSD_TEMP_SENSOR_4_TEMPERATURE,
    OSD_TEMP_SENSOR_5_TEMPERATURE,
    OSD_TEMP_SENSOR_6_TEMPERATURE,
    OSD_TEMP_SENSOR_7_TEMPERATURE,
    OSD_ALTITUDE_MSL,
    OSD_PLUS_CODE,
    OSD_MAP_SCALE,
    OSD_MAP_REFERENCE,
    OSD_GFORCE,
    OSD_GFORCE_X,
    OSD_GFORCE_Y,
    OSD_GFORCE_Z,
    OSD_RC_SOURCE,
    OSD_VTX_POWER,
    OSD_ESC_RPM,
    OSD_ESC_TEMPERATURE,
    OSD_AZIMUTH,
    OSD_RSSI_DBM,
    OSD_LQ_UPLINK,
    OSD_SNR_DB,
    OSD_TX_POWER_UPLINK,
    OSD_GVAR_0,
    OSD_GVAR_1,
    OSD_GVAR_2,
    OSD_GVAR_3,
    OSD_TPA,
    OSD_NAV_FW_CONTROL_SMOOTHNESS,
    OSD_VERSION,
    OSD_RANGEFINDER,
    OSD_PLIMIT_REMAINING_BURST_TIME,
    OSD_PLIMIT_ACTIVE_CURRENT_LIMIT,
    OSD_PLIMIT_ACTIVE_POWER_LIMIT,
    OSD_GLIDESLOPE,
    OSD_GPS_MAX_SPEED,
    OSD_3D_MAX_SPEED,
    OSD_AIR_MAX_SPEED,
    OSD_ACTIVE_PROFILE,
    OSD_MISSION,
    OSD_SWITCH_INDICATOR_0,
    OSD_SWITCH_INDICATOR_1,
    OSD_SWITCH_INDICATOR_2,
    OSD_SWITCH_INDICATOR_3,
    OSD_TPA_TIME_CONSTANT,
    OSD_FW_LEVEL_TRIM,
    OSD_GLIDE_TIME_REMAINING,
    OSD_GLIDE_RANGE,
    OSD_CLIMB_EFFICIENCY,
    OSD_NAV_WP_MULTI_MISSION_INDEX,
    OSD_GROUND_COURSE,      // 140
    OSD_CROSS_TRACK_ERROR,
    OSD_PILOT_NAME,
    OSD_PAN_SERVO_CENTRED,
    OSD_MULTI_FUNCTION,
    OSD_ODOMETER,
    OSD_PILOT_LOGO,
    OSD_CUSTOM_ELEMENT_1,
    OSD_CUSTOM_ELEMENT_2,
    OSD_CUSTOM_ELEMENT_3,
    OSD_ADSB_WARNING, //150
    OSD_ADSB_INFO,
    OSD_BLACKBOX,
    OSD_FORMATION_FLIGHT,
    OSD_CUSTOM_ELEMENT_4,
    OSD_CUSTOM_ELEMENT_5,
    OSD_CUSTOM_ELEMENT_6,
    OSD_CUSTOM_ELEMENT_7,
    OSD_CUSTOM_ELEMENT_8,
    OSD_LQ_DOWNLINK,
    OSD_RX_POWER_DOWNLINK, // 160
    OSD_RX_BAND,
    OSD_RX_MODE,
    OSD_COURSE_TO_FENCE,
    OSD_H_DIST_TO_FENCE,
    OSD_V_DIST_TO_FENCE,
    OSD_NAV_FW_ALT_CONTROL_RESPONSE,
    OSD_ITEM_COUNT // MUST BE LAST
} osd_items_e;

typedef enum {
    OSD_UNIT_IMPERIAL,
    OSD_UNIT_METRIC,
    OSD_UNIT_METRIC_MPH,    // Old UK units, all metric except speed in mph
    OSD_UNIT_UK,            // Show everything in imperial, temperature in C
    OSD_UNIT_GA,            // General Aviation: Knots, Nautical Miles, Feet, Degrees C

    OSD_UNIT_MAX = OSD_UNIT_GA,
} osd_unit_e;

typedef enum {
    OSD_STATS_ENERGY_UNIT_MAH,
    OSD_STATS_ENERGY_UNIT_WH,
} osd_stats_energy_unit_e;

typedef enum {
    OSD_CROSSHAIRS_STYLE_DEFAULT,
    OSD_CROSSHAIRS_STYLE_AIRCRAFT,
    OSD_CROSSHAIRS_STYLE_TYPE3,
    OSD_CROSSHAIRS_STYLE_TYPE4,
    OSD_CROSSHAIRS_STYLE_TYPE5,
    OSD_CROSSHAIRS_STYLE_TYPE6,
    OSD_CROSSHAIRS_STYLE_TYPE7,
} osd_crosshairs_style_e;

typedef enum {
    OSD_SIDEBAR_SCROLL_NONE,
    OSD_SIDEBAR_SCROLL_ALTITUDE,
    OSD_SIDEBAR_SCROLL_SPEED,
    OSD_SIDEBAR_SCROLL_HOME_DISTANCE,

    OSD_SIDEBAR_SCROLL_MAX = OSD_SIDEBAR_SCROLL_HOME_DISTANCE,
} osd_sidebar_scroll_e;

typedef enum {
    OSD_ALIGN_LEFT,
    OSD_ALIGN_RIGHT
} osd_alignment_e;

typedef enum {
    OSD_AHI_STYLE_DEFAULT,
    OSD_AHI_STYLE_LINE,
} osd_ahi_style_e;

typedef enum {
    OSD_CRSF_LQ_TYPE1,
    OSD_CRSF_LQ_TYPE2,
    OSD_CRSF_LQ_TYPE3
} osd_crsf_lq_format_e;

typedef struct osdLayoutsConfig_s {
    // Layouts
    uint16_t item_pos[OSD_LAYOUT_COUNT][OSD_ITEM_COUNT];
} osdLayoutsConfig_t;

PG_DECLARE(osdLayoutsConfig_t, osdLayoutsConfig);

#define OSD_SWITCH_INDICATOR_NAME_LENGTH 4

typedef struct osdConfig_s {
    // Alarms
    uint8_t         rssi_alarm;                         // rssi %
    uint16_t        time_alarm;                         // fly minutes
    uint16_t        alt_alarm;                          // positive altitude in m
    uint16_t        dist_alarm;                         // home distance in m
    uint16_t        neg_alt_alarm;                      // abs(negative altitude) in m
    uint8_t         current_alarm;                      // current consumption in A
    int16_t         imu_temp_alarm_min;
    int16_t         imu_temp_alarm_max;
    int16_t         esc_temp_alarm_min;
    int16_t         esc_temp_alarm_max;
    float           gforce_alarm;
    float           gforce_axis_alarm_min;
    float           gforce_axis_alarm_max;
#if defined(USE_SERIALRX_CRSF) || defined(USE_RX_MSP)
    int8_t          snr_alarm;                          //CRSF SNR alarm in dB
    int8_t          link_quality_alarm;
    int16_t         rssi_dbm_alarm;                     // in dBm
    int16_t         rssi_dbm_max;                       // Perfect RSSI. Set to High end of curve. RSSI at 100%
    int16_t         rssi_dbm_min;                       // Worst RSSI. Set to low end of curve or RX sensitivity level. RSSI at 0%
#endif
#ifdef USE_BARO
    int16_t         baro_temp_alarm_min;
    int16_t         baro_temp_alarm_max;
#endif
#ifdef USE_TEMPERATURE_SENSOR
    osd_alignment_e temp_label_align;
#endif
#ifdef USE_PITOT
    float           airspeed_alarm_min;
    float           airspeed_alarm_max;
#endif

    videoSystem_e   video_system;
    uint8_t         row_shiftdown;
    int16_t         msp_displayport_fullframe_interval;

    // Preferences
    uint8_t         main_voltage_decimals;
    uint8_t         decimals_altitude;
    uint8_t         decimals_distance;
    uint8_t         ahi_reverse_roll;
    uint8_t         ahi_max_pitch;
    uint8_t         crosshairs_style;                   // from osd_crosshairs_style_e
    int8_t          horizon_offset;
    int8_t          camera_uptilt;
    bool            ahi_camera_uptilt_comp;
    uint8_t         camera_fov_h;
    uint8_t         camera_fov_v;
    uint8_t         hud_margin_h;
    uint8_t         hud_margin_v;
    bool            hud_homing;
    bool            hud_homepoint;
    bool            hud_flight_direction;
    uint8_t         hud_radar_disp;
    uint16_t        hud_radar_range_min;
    uint16_t        hud_radar_range_max;
    uint8_t         hud_radar_alt_difference_display_time;
    uint8_t         hud_radar_distance_display_time;
    uint8_t         hud_wp_disp;

    uint8_t         left_sidebar_scroll;                // from osd_sidebar_scroll_e
    uint8_t         right_sidebar_scroll;               // from osd_sidebar_scroll_e
    uint8_t         sidebar_scroll_arrows;

    uint8_t         units;                              // from osd_unit_e
    uint8_t         stats_energy_unit;                  // from osd_stats_energy_unit_e
    uint8_t         stats_page_auto_swap_time;          // stats page auto swap interval time (seconds)
    bool            stats_show_metric_efficiency;       // If true, show metric efficiency as well as for the selected units

#ifdef USE_WIND_ESTIMATOR
    bool            estimations_wind_compensation;      // use wind compensation for estimated remaining flight/distance
    bool            estimations_wind_mps;               // wind speed estimation in m/s
#endif
    uint8_t         coordinate_digits;
    bool            osd_failsafe_switch_layout;
    uint8_t         plus_code_digits;                   // Number of digits to use in OSD_PLUS_CODE
    uint8_t         plus_code_short;
    uint8_t         ahi_style;
    uint8_t         force_grid;                         // Force a pixel based OSD to use grid mode.
    uint8_t         ahi_bordered;                       // Only used by the AHI widget
    uint8_t         ahi_width;                          // In pixels, only used by the AHI widget
    uint8_t         ahi_height;                         // In pixels, only used by the AHI widget
    int8_t          ahi_vertical_offset;                // Offset from center in pixels. Positive moves the AHI down. Widget only.
    int8_t          sidebar_horizontal_offset;          // Horizontal offset from default position. Units are grid slots for grid OSDs, pixels for pixel based OSDs. Positive values move sidebars closer to the edges.
    uint8_t         left_sidebar_scroll_step;           // How many units each sidebar step represents. 0 means the default value for the scroll type.
    uint8_t         right_sidebar_scroll_step;          // Same as left_sidebar_scroll_step, but for the right sidebar.
    bool            osd_home_position_arm_screen;
    uint8_t         pan_servo_index;                    // Index of the pan servo used for home direction offset
    int8_t          pan_servo_pwm2centideg;             // Centidegrees of servo rotation per us pwm
    uint8_t         pan_servo_offcentre_warning;        // Degrees around the centre, that is assumed camera is wanted to be facing forwards, but isn't centred
    bool            pan_servo_indicator_show_degrees;   // Show the degrees of offset for the pan servo
    uint8_t         crsf_lq_format;
    uint8_t         sidebar_height;                     // sidebar height in rows, 0 turns off sidebars leaving only level indicator arrows
    uint8_t         telemetry;                          // use telemetry on displayed pixel line 0
    uint8_t         esc_rpm_precision;                  // Number of characters used for the RPM numbers.
    uint16_t        system_msg_display_time;            // system message display time for multiple messages (ms)
    uint8_t         mAh_precision;                      // Number of numbers used for mAh drawn. Plently of packs now are > 9999 mAh
    uint8_t         ahi_pitch_interval;                 // redraws AHI at set pitch interval (Not pixel OSD)
    char            osd_switch_indicator0_name[OSD_SWITCH_INDICATOR_NAME_LENGTH + 1]; // Name to use for switch indicator 0.
    uint8_t         osd_switch_indicator0_channel;      // RC Channel to use for switch indicator 0.
    char            osd_switch_indicator1_name[OSD_SWITCH_INDICATOR_NAME_LENGTH + 1]; // Name to use for switch indicator 1.
    uint8_t         osd_switch_indicator1_channel;      // RC Channel to use for switch indicator 1.
    char            osd_switch_indicator2_name[OSD_SWITCH_INDICATOR_NAME_LENGTH + 1]; // Name to use for switch indicator 2.
    uint8_t         osd_switch_indicator2_channel;      // RC Channel to use for switch indicator 2.
    char            osd_switch_indicator3_name[OSD_SWITCH_INDICATOR_NAME_LENGTH + 1]; // Name to use for switch indicator 3.
    uint8_t         osd_switch_indicator3_channel;      // RC Channel to use for switch indicator 3.
    bool            osd_switch_indicators_align_left;   // Align switch indicator name to left of the switch.
    bool            use_pilot_logo;                     // If enabled, the pilot logo (last 40 characters of page 2 font) will be used with the INAV logo.
    uint8_t         inav_to_pilot_logo_spacing;         // The space between the INAV and pilot logos, if pilot logo is used. This number may be adjusted so that it fits the odd/even col width.
    uint16_t        arm_screen_display_time;            // Length of time the arm screen is displayed
#ifndef DISABLE_MSP_DJI_COMPAT
    bool            highlight_djis_missing_characters;  // If enabled, show question marks where there is no character in DJI's font to represent an OSD element symbol
#endif
    bool            enable_broken_o4_workaround;        // If enabled, override STATUS/STATUS_EX messages to work around DJI's broken O4 air unit MSP DisplayPort implementation
#ifdef USE_ADSB
    uint16_t adsb_distance_warning;                     // in metres
    uint16_t adsb_distance_alert;                       // in metres
    uint16_t adsb_ignore_plane_above_me_limit;          // in metres
#endif
    uint8_t  radar_peers_display_time;                  // in seconds
#ifdef USE_GEOZONE
    uint8_t geozoneDistanceWarning;                     // Distance to fence or action
    bool geozoneDistanceType;                            // Shows a countdown timer or distance to fence/action
#endif
} osdConfig_t;

PG_DECLARE(osdConfig_t, osdConfig);

typedef struct displayPort_s displayPort_t;
typedef struct displayCanvas_s displayCanvas_t;

void osdInit(displayPort_t *osdDisplayPort);
bool osdDisplayIsPAL(void);
void osdUpdate(timeUs_t currentTimeUs);
void osdStartFullRedraw(void);
// Sets a fixed OSD layout ignoring the RC input. Set it
// to -1 to disable the override. If layout is >= 0 and
// duration is > 0, the override is automatically cleared by
// the OSD after the given duration. Otherwise, the caller must
// explicitely remove it.
void osdOverrideLayout(int layout, timeMs_t duration);
// Returns the current current layout as well as wether its
// set by the user configuration (modes, etc..) or by overriding it.
int osdGetActiveLayout(bool *overridden);
bool osdItemIsFixed(osd_items_e item);
uint8_t osdIncElementIndex(uint8_t elementIndex);

displayPort_t *osdGetDisplayPort(void);
displayCanvas_t *osdGetDisplayPortCanvas(void);

int16_t osdGetHeading(void);
int32_t osdGetAltitude(void);

bool osdUsingScaledThrottle(void);

void osdStartedSaveProcess(void);
void osdShowEEPROMSavedNotification(void);

void osdCrosshairPosition(uint8_t *x, uint8_t *y);
bool osdFormatCentiNumber(char *buff, int32_t centivalue, uint32_t scale, int maxDecimals, int maxScaledDecimals, int length, bool leadingZeros);
void osdFormatAltitudeSymbol(char *buff, int32_t alt);
void osdFormatVelocityStr(char* buff, int32_t vel, bool _3D, bool _max);
// Returns a heading angle in degrees normalized to [0, 360).
int osdGetHeadingAngle(int angle);

void osdResetWarningFlags(void);

int16_t osdGetPanServoOffset(void);

/**
 * @brief Get the OSD system message
 * @param buff pointer to the message buffer
 * @param buff_size size of the buffer array
 * @param isCenteredText if true, centered text based on buff_size
 * @return osd text attributes (Blink, Inverted, Solid)
 */
textAttributes_t osdGetSystemMessage(char *buff, size_t buff_size, bool isCenteredText);
