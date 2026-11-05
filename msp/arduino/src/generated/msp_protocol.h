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

// Message ids below are generated from the MSP YAML schema by msp/generator/gen_c.py.

/**
 * MSP Guidelines, emphasis is used to clarify.
 *
 * Each FlightController (FC, Server) MUST change the API version when any MSP command is added, deleted, or changed.
 *
 * If you fork the FC source code and release your own version, you MUST change the Flight Controller Identifier.
 *
 * NEVER release a modified copy of this code that shares the same Flight controller IDENT and API version
 * if the API doesn't match EXACTLY.
 *
 * Consumers of the API (API clients) SHOULD first attempt to get a response from the MSP_API_VERSION command.
 *
 * API consumers should ALWAYS handle communication failures gracefully and attempt to continue
 * without the information if possible.  Clients MAY log/display a suitable message.
 *
 * API clients should NOT attempt any communication if they can't handle the returned API MAJOR VERSION.
 *
 * API clients SHOULD attempt communication if the API MINOR VERSION has increased from the time
 * the API client was written and handle command failures gracefully.  Clients MAY disable
 * functionality that depends on the commands while still leaving other functionality intact.
 * Clients SHOULD operate in READ-ONLY mode and SHOULD present a warning to the user to state
 * that the newer API version may cause problems before using API commands that change FC state.
 *
 * It is for this reason that each MSP command should be specific as possible, such that changes
 * to commands break as little functionality as possible.
 *
 * API client authors MAY use a compatibility matrix/table when determining if they can support
 * a given command from a given flight controller at a given api version level.
 *
 * Developers MUST NOT create new MSP commands that do more than one thing.
 *
 * Failure to follow these guidelines will likely invoke the wrath of developers trying to write tools
 * that use the API and the users of those tools.
 */
/* Protocol numbers used both by the wire format, config system, and
   field setters.
*/
#define MSP_PROTOCOL_VERSION                0   // Same version over MSPv1 & MSPv2 - message format didn't change and it backward compatible
#define API_VERSION_MAJOR                   2   // increment when major changes are made
#define API_VERSION_MINOR                   6   // increment when any change is made, reset to zero when major changes are released after changing API_VERSION_MAJOR
#define API_VERSION_LENGTH                  2
#define MULTIWII_IDENTIFIER "MWII";
#define BASEFLIGHT_IDENTIFIER "BAFL";
#define BETAFLIGHT_IDENTIFIER "BTFL"
#define CLEANFLIGHT_IDENTIFIER "CLFL"
#define INAV_IDENTIFIER "INAV"
#define RACEFLIGHT_IDENTIFIER "RCFL"
#define FLIGHT_CONTROLLER_IDENTIFIER_LENGTH 4
#define FLIGHT_CONTROLLER_VERSION_LENGTH    3
#define FLIGHT_CONTROLLER_VERSION_MASK      0xFFF
#define BOARD_IDENTIFIER_LENGTH             4 // 4 UPPER CASE alpha numeric characters that identify the board being used.
#define BOARD_HARDWARE_REVISION_LENGTH      2
// These are baseflight specific flags but they are useless now since MW 2.3 uses the upper 4 bits for the navigation version.
#define CAP_PLATFORM_32BIT          ((uint32_t)1 << 31)
#define CAP_BASEFLIGHT_CONFIG       ((uint32_t)1 << 30)
// MW 2.3 stores NAVI_VERSION in the top 4 bits of the capability mask.
#define CAP_NAVI_VERSION_BIT_4_MSB  ((uint32_t)1 << 31)
#define CAP_NAVI_VERSION_BIT_3      ((uint32_t)1 << 30)
#define CAP_NAVI_VERSION_BIT_2      ((uint32_t)1 << 29)
#define CAP_NAVI_VERSION_BIT_1_LSB  ((uint32_t)1 << 28)
#define CAP_DYNBALANCE              ((uint32_t)1 << 2)
#define CAP_FLAPS                   ((uint32_t)1 << 3)
#define CAP_NAVCAP                  ((uint32_t)1 << 4)
#define CAP_EXTAUX                  ((uint32_t)1 << 5)
//
// MSP commands for Cleanflight original features
//
// private - only to be used by the configurator, the commands are likely to change
//
// Baseflight MSP commands (if enabled they exist in Cleanflight)
//
// Betaflight Additional Commands
//
// OSD specific
//
//
// Multwii original MSP commands
//
// #define MSP_BIND                 240    //in message          no param
// #define MSP_ALARMS               242
// Additional commands that are not compatible with MultiWii
// MSPv2 includes

#define MSP_API_VERSION                     1
#define MSP_FC_VARIANT                      2
#define MSP_FC_VERSION                      3
#define MSP_BOARD_INFO                      4
#define MSP_BUILD_INFO                      5
#define MSP_INAV_PID                        6
#define MSP_SET_INAV_PID                    7
#define MSP_NAME                            10
#define MSP_SET_NAME                        11
#define MSP_NAV_POSHOLD                     12
#define MSP_SET_NAV_POSHOLD                 13
#define MSP_CALIBRATION_DATA                14
#define MSP_SET_CALIBRATION_DATA            15
#define MSP_POSITION_ESTIMATION_CONFIG      16
#define MSP_SET_POSITION_ESTIMATION_CONFIG  17
#define MSP_WP_MISSION_LOAD                 18
#define MSP_WP_MISSION_SAVE                 19
#define MSP_WP_GETINFO                      20
#define MSP_RTH_AND_LAND_CONFIG             21
#define MSP_SET_RTH_AND_LAND_CONFIG         22
#define MSP_FW_CONFIG                       23
#define MSP_SET_FW_CONFIG                   24
#define MSP_MODE_RANGES                     34
#define MSP_SET_MODE_RANGE                  35
#define MSP_FEATURE                         36
#define MSP_SET_FEATURE                     37
#define MSP_BOARD_ALIGNMENT                 38
#define MSP_SET_BOARD_ALIGNMENT             39
#define MSP_CURRENT_METER_CONFIG            40
#define MSP_SET_CURRENT_METER_CONFIG        41
#define MSP_MIXER                           42
#define MSP_SET_MIXER                       43
#define MSP_RX_CONFIG                       44
#define MSP_SET_RX_CONFIG                   45
#define MSP_LED_COLORS                      46
#define MSP_SET_LED_COLORS                  47
#define MSP_LED_STRIP_CONFIG                48
#define MSP_SET_LED_STRIP_CONFIG            49
#define MSP_RSSI_CONFIG                     50
#define MSP_SET_RSSI_CONFIG                 51
#define MSP_ADJUSTMENT_RANGES               52
#define MSP_SET_ADJUSTMENT_RANGE            53
#define MSP_CF_SERIAL_CONFIG                54
#define MSP_SET_CF_SERIAL_CONFIG            55
#define MSP_VOLTAGE_METER_CONFIG            56
#define MSP_SET_VOLTAGE_METER_CONFIG        57
#define MSP_SONAR_ALTITUDE                  58
#define MSP_RX_MAP                          64
#define MSP_SET_RX_MAP                      65
#define MSP_REBOOT                          68
#define MSP_DATAFLASH_SUMMARY               70
#define MSP_DATAFLASH_READ                  71
#define MSP_DATAFLASH_ERASE                 72
#define MSP_LOOP_TIME                       73
#define MSP_SET_LOOP_TIME                   74
#define MSP_FAILSAFE_CONFIG                 75
#define MSP_SET_FAILSAFE_CONFIG             76
#define MSP_SDCARD_SUMMARY                  79
#define MSP_BLACKBOX_CONFIG                 80
#define MSP_SET_BLACKBOX_CONFIG             81
#define MSP_TRANSPONDER_CONFIG              82
#define MSP_SET_TRANSPONDER_CONFIG          83
#define MSP_OSD_CONFIG                      84
#define MSP_SET_OSD_CONFIG                  85
#define MSP_OSD_CHAR_READ                   86
#define MSP_OSD_CHAR_WRITE                  87
#define MSP_VTX_CONFIG                      88
#define MSP_SET_VTX_CONFIG                  89
#define MSP_ADVANCED_CONFIG                 90
#define MSP_SET_ADVANCED_CONFIG             91
#define MSP_FILTER_CONFIG                   92
#define MSP_SET_FILTER_CONFIG               93
#define MSP_PID_ADVANCED                    94
#define MSP_SET_PID_ADVANCED                95
#define MSP_SENSOR_CONFIG                   96
#define MSP_SET_SENSOR_CONFIG               97
#define MSP_SPECIAL_PARAMETERS              98
#define MSP_SET_SPECIAL_PARAMETERS          99
#define MSP_STATUS                          101
#define MSP_RAW_IMU                         102
#define MSP_SERVO                           103
#define MSP_MOTOR                           104
#define MSP_RC                              105
#define MSP_RAW_GPS                         106
#define MSP_COMP_GPS                        107
#define MSP_ATTITUDE                        108
#define MSP_ALTITUDE                        109
#define MSP_ANALOG                          110
#define MSP_RC_TUNING                       111
#define MSP_ACTIVEBOXES                     113
#define MSP_MISC                            114
#define MSP_BOXNAMES                        116
#define MSP_PIDNAMES                        117
#define MSP_WP                              118
#define MSP_BOXIDS                          119
#define MSP_SERVO_CONFIGURATIONS            120
#define MSP_NAV_STATUS                      121
#define MSP_NAV_CONFIG                      122
#define MSP_3D                              124
#define MSP_RC_DEADBAND                     125
#define MSP_SENSOR_ALIGNMENT                126
#define MSP_LED_STRIP_MODECOLOR             127
#define MSP_BATTERY_STATE                   130
#define MSP_VTXTABLE_BAND                   137
#define MSP_VTXTABLE_POWERLEVEL             138
#define MSP_STATUS_EX                       150
#define MSP_SENSOR_STATUS                   151
#define MSP_UID                             160
#define MSP_GPSSVINFO                       164
#define MSP_GPSSTATISTICS                   166
#define MSP_OSD_VIDEO_CONFIG                180
#define MSP_SET_OSD_VIDEO_CONFIG            181
#define MSP_DISPLAYPORT                     182
#define MSP_SET_TX_INFO                     186
#define MSP_TX_INFO                         187
#define MSP_SET_RAW_RC                      200
#define MSP_SET_RAW_GPS                     201
#define MSP_SET_BOX                         203
#define MSP_SET_RC_TUNING                   204
#define MSP_ACC_CALIBRATION                 205
#define MSP_MAG_CALIBRATION                 206
#define MSP_SET_MISC                        207
#define MSP_RESET_CONF                      208
#define MSP_SET_WP                          209
#define MSP_SELECT_SETTING                  210
#define MSP_SET_HEAD                        211
#define MSP_SET_SERVO_CONFIGURATION         212
#define MSP_SET_MOTOR                       214
#define MSP_SET_NAV_CONFIG                  215
#define MSP_SET_3D                          217
#define MSP_SET_RC_DEADBAND                 218
#define MSP_SET_RESET_CURR_PID              219
#define MSP_SET_SENSOR_ALIGNMENT            220
#define MSP_SET_LED_STRIP_MODECOLOR         221
#define MSP_SET_ACC_TRIM                    239
#define MSP_ACC_TRIM                        240
#define MSP_SERVO_MIX_RULES                 241
#define MSP_SET_SERVO_MIX_RULE              242
#define MSP_SET_PASSTHROUGH                 245
#define MSP_RTC                             246
#define MSP_SET_RTC                         247
#define MSP_EEPROM_WRITE                    250
#define MSP_RESERVE_1                       251
#define MSP_RESERVE_2                       252
#define MSP_DEBUGMSG                        253
#define MSP_DEBUG                           254
#define MSP_V2_FRAME                        255

#include "msp_protocol_v2_common.h"
#include "msp_protocol_v2_sensor.h"
#include "msp_protocol_v2_inav.h"
