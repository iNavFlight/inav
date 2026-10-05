# INAV MSP messages

**Generated from `msp/schema/msp_v2.yaml` by `./msp/build.sh generate` — do not edit by hand.**

`msp/schema/msp_v2.yaml` is the single hand-authored source of truth for the MSP protocol. Enums and constants are harvested from the INAV C source. To change the protocol, edit the schema and run `./msp/build.sh generate`; CI fails the build if this file is out of date.

Enum reference: [`enums.md`](enums.md). See also [`format.md`](format.md) and [`msp-message-routing-architecture.md`](msp-message-routing-architecture.md).

281 messages.

## Contents

- [MSP_API_VERSION](#msp_api_version)
- [MSP_FC_VARIANT](#msp_fc_variant)
- [MSP_FC_VERSION](#msp_fc_version)
- [MSP_BOARD_INFO](#msp_board_info)
- [MSP_BUILD_INFO](#msp_build_info)
- [MSP_INAV_PID](#msp_inav_pid)
- [MSP_SET_INAV_PID](#msp_set_inav_pid)
- [MSP_NAME](#msp_name)
- [MSP_SET_NAME](#msp_set_name)
- [MSP_NAV_POSHOLD](#msp_nav_poshold)
- [MSP_SET_NAV_POSHOLD](#msp_set_nav_poshold)
- [MSP_CALIBRATION_DATA](#msp_calibration_data)
- [MSP_SET_CALIBRATION_DATA](#msp_set_calibration_data)
- [MSP_POSITION_ESTIMATION_CONFIG](#msp_position_estimation_config)
- [MSP_SET_POSITION_ESTIMATION_CONFIG](#msp_set_position_estimation_config)
- [MSP_WP_MISSION_LOAD](#msp_wp_mission_load)
- [MSP_WP_MISSION_SAVE](#msp_wp_mission_save)
- [MSP_WP_GETINFO](#msp_wp_getinfo)
- [MSP_RTH_AND_LAND_CONFIG](#msp_rth_and_land_config)
- [MSP_SET_RTH_AND_LAND_CONFIG](#msp_set_rth_and_land_config)
- [MSP_FW_CONFIG](#msp_fw_config)
- [MSP_SET_FW_CONFIG](#msp_set_fw_config)
- [MSP_MODE_RANGES](#msp_mode_ranges)
- [MSP_SET_MODE_RANGE](#msp_set_mode_range)
- [MSP_FEATURE](#msp_feature)
- [MSP_SET_FEATURE](#msp_set_feature)
- [MSP_BOARD_ALIGNMENT](#msp_board_alignment)
- [MSP_SET_BOARD_ALIGNMENT](#msp_set_board_alignment)
- [MSP_CURRENT_METER_CONFIG](#msp_current_meter_config)
- [MSP_SET_CURRENT_METER_CONFIG](#msp_set_current_meter_config)
- [MSP_MIXER](#msp_mixer)
- [MSP_SET_MIXER](#msp_set_mixer)
- [MSP_RX_CONFIG](#msp_rx_config)
- [MSP_SET_RX_CONFIG](#msp_set_rx_config)
- [MSP_LED_COLORS](#msp_led_colors)
- [MSP_SET_LED_COLORS](#msp_set_led_colors)
- [MSP_LED_STRIP_CONFIG](#msp_led_strip_config)
- [MSP_SET_LED_STRIP_CONFIG](#msp_set_led_strip_config)
- [MSP_RSSI_CONFIG](#msp_rssi_config)
- [MSP_SET_RSSI_CONFIG](#msp_set_rssi_config)
- [MSP_ADJUSTMENT_RANGES](#msp_adjustment_ranges)
- [MSP_SET_ADJUSTMENT_RANGE](#msp_set_adjustment_range)
- [MSP_CF_SERIAL_CONFIG](#msp_cf_serial_config)
- [MSP_SET_CF_SERIAL_CONFIG](#msp_set_cf_serial_config)
- [MSP_VOLTAGE_METER_CONFIG](#msp_voltage_meter_config)
- [MSP_SET_VOLTAGE_METER_CONFIG](#msp_set_voltage_meter_config)
- [MSP_SONAR_ALTITUDE](#msp_sonar_altitude)
- [MSP_RX_MAP](#msp_rx_map)
- [MSP_SET_RX_MAP](#msp_set_rx_map)
- [MSP_REBOOT](#msp_reboot)
- [MSP_DATAFLASH_SUMMARY](#msp_dataflash_summary)
- [MSP_DATAFLASH_READ](#msp_dataflash_read)
- [MSP_DATAFLASH_ERASE](#msp_dataflash_erase)
- [MSP_LOOP_TIME](#msp_loop_time)
- [MSP_SET_LOOP_TIME](#msp_set_loop_time)
- [MSP_FAILSAFE_CONFIG](#msp_failsafe_config)
- [MSP_SET_FAILSAFE_CONFIG](#msp_set_failsafe_config)
- [MSP_SDCARD_SUMMARY](#msp_sdcard_summary)
- [MSP_BLACKBOX_CONFIG](#msp_blackbox_config)
- [MSP_SET_BLACKBOX_CONFIG](#msp_set_blackbox_config)
- [MSP_TRANSPONDER_CONFIG](#msp_transponder_config)
- [MSP_SET_TRANSPONDER_CONFIG](#msp_set_transponder_config)
- [MSP_OSD_CONFIG](#msp_osd_config)
- [MSP_SET_OSD_CONFIG](#msp_set_osd_config)
- [MSP_OSD_CHAR_READ](#msp_osd_char_read)
- [MSP_OSD_CHAR_WRITE](#msp_osd_char_write)
- [MSP_VTX_CONFIG](#msp_vtx_config)
- [MSP_SET_VTX_CONFIG](#msp_set_vtx_config)
- [MSP_ADVANCED_CONFIG](#msp_advanced_config)
- [MSP_SET_ADVANCED_CONFIG](#msp_set_advanced_config)
- [MSP_FILTER_CONFIG](#msp_filter_config)
- [MSP_SET_FILTER_CONFIG](#msp_set_filter_config)
- [MSP_PID_ADVANCED](#msp_pid_advanced)
- [MSP_SET_PID_ADVANCED](#msp_set_pid_advanced)
- [MSP_SENSOR_CONFIG](#msp_sensor_config)
- [MSP_SET_SENSOR_CONFIG](#msp_set_sensor_config)
- [MSP_SPECIAL_PARAMETERS](#msp_special_parameters)
- [MSP_SET_SPECIAL_PARAMETERS](#msp_set_special_parameters)
- [MSP_STATUS](#msp_status)
- [MSP_RAW_IMU](#msp_raw_imu)
- [MSP_SERVO](#msp_servo)
- [MSP_MOTOR](#msp_motor)
- [MSP_RC](#msp_rc)
- [MSP_RAW_GPS](#msp_raw_gps)
- [MSP_COMP_GPS](#msp_comp_gps)
- [MSP_ATTITUDE](#msp_attitude)
- [MSP_ALTITUDE](#msp_altitude)
- [MSP_ANALOG](#msp_analog)
- [MSP_RC_TUNING](#msp_rc_tuning)
- [MSP_ACTIVEBOXES](#msp_activeboxes)
- [MSP_MISC](#msp_misc)
- [MSP_BOXNAMES](#msp_boxnames)
- [MSP_PIDNAMES](#msp_pidnames)
- [MSP_WP](#msp_wp)
- [MSP_BOXIDS](#msp_boxids)
- [MSP_SERVO_CONFIGURATIONS](#msp_servo_configurations)
- [MSP_NAV_STATUS](#msp_nav_status)
- [MSP_NAV_CONFIG](#msp_nav_config)
- [MSP_3D](#msp_3d)
- [MSP_RC_DEADBAND](#msp_rc_deadband)
- [MSP_SENSOR_ALIGNMENT](#msp_sensor_alignment)
- [MSP_LED_STRIP_MODECOLOR](#msp_led_strip_modecolor)
- [MSP_BATTERY_STATE](#msp_battery_state)
- [MSP_VTXTABLE_BAND](#msp_vtxtable_band)
- [MSP_VTXTABLE_POWERLEVEL](#msp_vtxtable_powerlevel)
- [MSP_STATUS_EX](#msp_status_ex)
- [MSP_SENSOR_STATUS](#msp_sensor_status)
- [MSP_UID](#msp_uid)
- [MSP_GPSSVINFO](#msp_gpssvinfo)
- [MSP_GPSSTATISTICS](#msp_gpsstatistics)
- [MSP_OSD_VIDEO_CONFIG](#msp_osd_video_config)
- [MSP_SET_OSD_VIDEO_CONFIG](#msp_set_osd_video_config)
- [MSP_DISPLAYPORT](#msp_displayport)
- [MSP_SET_TX_INFO](#msp_set_tx_info)
- [MSP_TX_INFO](#msp_tx_info)
- [MSP_SET_RAW_RC](#msp_set_raw_rc)
- [MSP_SET_RAW_GPS](#msp_set_raw_gps)
- [MSP_SET_BOX](#msp_set_box)
- [MSP_SET_RC_TUNING](#msp_set_rc_tuning)
- [MSP_ACC_CALIBRATION](#msp_acc_calibration)
- [MSP_MAG_CALIBRATION](#msp_mag_calibration)
- [MSP_SET_MISC](#msp_set_misc)
- [MSP_RESET_CONF](#msp_reset_conf)
- [MSP_SET_WP](#msp_set_wp)
- [MSP_SELECT_SETTING](#msp_select_setting)
- [MSP_SET_HEAD](#msp_set_head)
- [MSP_SET_SERVO_CONFIGURATION](#msp_set_servo_configuration)
- [MSP_SET_MOTOR](#msp_set_motor)
- [MSP_SET_NAV_CONFIG](#msp_set_nav_config)
- [MSP_SET_3D](#msp_set_3d)
- [MSP_SET_RC_DEADBAND](#msp_set_rc_deadband)
- [MSP_SET_RESET_CURR_PID](#msp_set_reset_curr_pid)
- [MSP_SET_SENSOR_ALIGNMENT](#msp_set_sensor_alignment)
- [MSP_SET_LED_STRIP_MODECOLOR](#msp_set_led_strip_modecolor)
- [MSP_SET_ACC_TRIM](#msp_set_acc_trim)
- [MSP_ACC_TRIM](#msp_acc_trim)
- [MSP_SERVO_MIX_RULES](#msp_servo_mix_rules)
- [MSP_SET_SERVO_MIX_RULE](#msp_set_servo_mix_rule)
- [MSP_SET_PASSTHROUGH](#msp_set_passthrough)
- [MSP_RTC](#msp_rtc)
- [MSP_SET_RTC](#msp_set_rtc)
- [MSP_EEPROM_WRITE](#msp_eeprom_write)
- [MSP_RESERVE_1](#msp_reserve_1)
- [MSP_RESERVE_2](#msp_reserve_2)
- [MSP_DEBUGMSG](#msp_debugmsg)
- [MSP_DEBUG](#msp_debug)
- [MSP_V2_FRAME](#msp_v2_frame)
- [MSP2_COMMON_TZ](#msp2_common_tz)
- [MSP2_COMMON_SET_TZ](#msp2_common_set_tz)
- [MSP2_COMMON_SETTING](#msp2_common_setting)
- [MSP2_COMMON_SET_SETTING](#msp2_common_set_setting)
- [MSP2_COMMON_MOTOR_MIXER](#msp2_common_motor_mixer)
- [MSP2_COMMON_SET_MOTOR_MIXER](#msp2_common_set_motor_mixer)
- [MSP2_COMMON_SETTING_INFO](#msp2_common_setting_info)
- [MSP2_COMMON_PG_LIST](#msp2_common_pg_list)
- [MSP2_COMMON_SERIAL_CONFIG](#msp2_common_serial_config)
- [MSP2_COMMON_SET_SERIAL_CONFIG](#msp2_common_set_serial_config)
- [MSP2_COMMON_SET_RADAR_POS](#msp2_common_set_radar_pos)
- [MSP2_COMMON_SET_RADAR_ITD](#msp2_common_set_radar_itd)
- [MSP2_COMMON_SET_MSP_RC_LINK_STATS](#msp2_common_set_msp_rc_link_stats)
- [MSP2_COMMON_SET_MSP_RC_INFO](#msp2_common_set_msp_rc_info)
- [MSP2_COMMON_GET_RADAR_GPS](#msp2_common_get_radar_gps)
- [MSP2_SENSOR_RANGEFINDER](#msp2_sensor_rangefinder)
- [MSP2_SENSOR_OPTIC_FLOW](#msp2_sensor_optic_flow)
- [MSP2_SENSOR_GPS](#msp2_sensor_gps)
- [MSP2_SENSOR_COMPASS](#msp2_sensor_compass)
- [MSP2_SENSOR_BAROMETER](#msp2_sensor_barometer)
- [MSP2_SENSOR_AIRSPEED](#msp2_sensor_airspeed)
- [MSP2_SENSOR_HEADTRACKER](#msp2_sensor_headtracker)
- [MSP2_INAV_STATUS](#msp2_inav_status)
- [MSP2_INAV_OPTICAL_FLOW](#msp2_inav_optical_flow)
- [MSP2_INAV_ANALOG](#msp2_inav_analog)
- [MSP2_INAV_MISC](#msp2_inav_misc)
- [MSP2_INAV_SET_MISC](#msp2_inav_set_misc)
- [MSP2_INAV_BATTERY_CONFIG](#msp2_inav_battery_config)
- [MSP2_INAV_SET_BATTERY_CONFIG](#msp2_inav_set_battery_config)
- [MSP2_INAV_RATE_PROFILE](#msp2_inav_rate_profile)
- [MSP2_INAV_SET_RATE_PROFILE](#msp2_inav_set_rate_profile)
- [MSP2_INAV_AIR_SPEED](#msp2_inav_air_speed)
- [MSP2_INAV_OUTPUT_MAPPING](#msp2_inav_output_mapping)
- [MSP2_INAV_MC_BRAKING](#msp2_inav_mc_braking)
- [MSP2_INAV_SET_MC_BRAKING](#msp2_inav_set_mc_braking)
- [MSP2_INAV_OUTPUT_MAPPING_EXT](#msp2_inav_output_mapping_ext)
- [MSP2_INAV_TIMER_OUTPUT_MODE](#msp2_inav_timer_output_mode)
- [MSP2_INAV_SET_TIMER_OUTPUT_MODE](#msp2_inav_set_timer_output_mode)
- [MSP2_INAV_MIXER](#msp2_inav_mixer)
- [MSP2_INAV_SET_MIXER](#msp2_inav_set_mixer)
- [MSP2_INAV_OSD_LAYOUTS](#msp2_inav_osd_layouts)
- [MSP2_INAV_OSD_SET_LAYOUT_ITEM](#msp2_inav_osd_set_layout_item)
- [MSP2_INAV_OSD_ALARMS](#msp2_inav_osd_alarms)
- [MSP2_INAV_OSD_SET_ALARMS](#msp2_inav_osd_set_alarms)
- [MSP2_INAV_OSD_PREFERENCES](#msp2_inav_osd_preferences)
- [MSP2_INAV_OSD_SET_PREFERENCES](#msp2_inav_osd_set_preferences)
- [MSP2_INAV_SELECT_BATTERY_PROFILE](#msp2_inav_select_battery_profile)
- [MSP2_INAV_DEBUG](#msp2_inav_debug)
- [MSP2_BLACKBOX_CONFIG](#msp2_blackbox_config)
- [MSP2_SET_BLACKBOX_CONFIG](#msp2_set_blackbox_config)
- [MSP2_INAV_TEMP_SENSOR_CONFIG](#msp2_inav_temp_sensor_config)
- [MSP2_INAV_SET_TEMP_SENSOR_CONFIG](#msp2_inav_set_temp_sensor_config)
- [MSP2_INAV_TEMPERATURES](#msp2_inav_temperatures)
- [MSP_SIMULATOR](#msp_simulator)
- [MSP2_INAV_SERVO_MIXER](#msp2_inav_servo_mixer)
- [MSP2_INAV_SET_SERVO_MIXER](#msp2_inav_set_servo_mixer)
- [MSP2_INAV_LOGIC_CONDITIONS](#msp2_inav_logic_conditions)
- [MSP2_INAV_SET_LOGIC_CONDITIONS](#msp2_inav_set_logic_conditions)
- [MSP2_INAV_GLOBAL_FUNCTIONS](#msp2_inav_global_functions)
- [MSP2_INAV_SET_GLOBAL_FUNCTIONS](#msp2_inav_set_global_functions)
- [MSP2_INAV_LOGIC_CONDITIONS_STATUS](#msp2_inav_logic_conditions_status)
- [MSP2_INAV_GVAR_STATUS](#msp2_inav_gvar_status)
- [MSP2_INAV_PROGRAMMING_PID](#msp2_inav_programming_pid)
- [MSP2_INAV_SET_PROGRAMMING_PID](#msp2_inav_set_programming_pid)
- [MSP2_INAV_PROGRAMMING_PID_STATUS](#msp2_inav_programming_pid_status)
- [MSP2_PID](#msp2_pid)
- [MSP2_SET_PID](#msp2_set_pid)
- [MSP2_INAV_OPFLOW_CALIBRATION](#msp2_inav_opflow_calibration)
- [MSP2_INAV_FWUPDT_PREPARE](#msp2_inav_fwupdt_prepare)
- [MSP2_INAV_FWUPDT_STORE](#msp2_inav_fwupdt_store)
- [MSP2_INAV_FWUPDT_EXEC](#msp2_inav_fwupdt_exec)
- [MSP2_INAV_FWUPDT_ROLLBACK_PREPARE](#msp2_inav_fwupdt_rollback_prepare)
- [MSP2_INAV_FWUPDT_ROLLBACK_EXEC](#msp2_inav_fwupdt_rollback_exec)
- [MSP2_INAV_SAFEHOME](#msp2_inav_safehome)
- [MSP2_INAV_SET_SAFEHOME](#msp2_inav_set_safehome)
- [MSP2_INAV_MISC2](#msp2_inav_misc2)
- [MSP2_INAV_LOGIC_CONDITIONS_SINGLE](#msp2_inav_logic_conditions_single)
- [MSP2_INAV_LOGIC_CONDITIONS_CONFIGURED](#msp2_inav_logic_conditions_configured)
- [MSP2_INAV_ESC_RPM](#msp2_inav_esc_rpm)
- [MSP2_INAV_ESC_TELEM](#msp2_inav_esc_telem)
- [MSP2_INAV_DRONECAN_NODES](#msp2_inav_dronecan_nodes)
- [MSP2_INAV_DRONECAN_ASYNC_REQUEST](#msp2_inav_dronecan_async_request)
- [MSP2_INAV_DRONECAN_ASYNC_RESULT](#msp2_inav_dronecan_async_result)
- [MSP2_INAV_LED_STRIP_CONFIG_EX](#msp2_inav_led_strip_config_ex)
- [MSP2_INAV_SET_LED_STRIP_CONFIG_EX](#msp2_inav_set_led_strip_config_ex)
- [MSP2_INAV_FW_APPROACH](#msp2_inav_fw_approach)
- [MSP2_INAV_SET_FW_APPROACH](#msp2_inav_set_fw_approach)
- [MSP2_INAV_GPS_UBLOX_COMMAND](#msp2_inav_gps_ublox_command)
- [MSP2_INAV_RATE_DYNAMICS](#msp2_inav_rate_dynamics)
- [MSP2_INAV_SET_RATE_DYNAMICS](#msp2_inav_set_rate_dynamics)
- [MSP2_INAV_EZ_TUNE](#msp2_inav_ez_tune)
- [MSP2_INAV_EZ_TUNE_SET](#msp2_inav_ez_tune_set)
- [MSP2_INAV_SELECT_MIXER_PROFILE](#msp2_inav_select_mixer_profile)
- [MSP2_ADSB_VEHICLE_LIST](#msp2_adsb_vehicle_list)
- [MSP2_ADSB_LIMITS](#msp2_adsb_limits)
- [MSP2_ADSB_WARNING_VEHICLE_ICAO](#msp2_adsb_warning_vehicle_icao)
- [MSP2_ADSB_VEHICLE](#msp2_adsb_vehicle)
- [MSP2_ADSB_VEHICLE_COUNT](#msp2_adsb_vehicle_count)
- [MSP2_INAV_CUSTOM_OSD_ELEMENTS](#msp2_inav_custom_osd_elements)
- [MSP2_INAV_CUSTOM_OSD_ELEMENT](#msp2_inav_custom_osd_element)
- [MSP2_INAV_SET_CUSTOM_OSD_ELEMENTS](#msp2_inav_set_custom_osd_elements)
- [MSP2_INAV_GET_LINK_STATS](#msp2_inav_get_link_stats)
- [MSP2_INAV_OUTPUT_MAPPING_EXT2](#msp2_inav_output_mapping_ext2)
- [MSP2_INAV_OUTPUT_ASSIGNMENT](#msp2_inav_output_assignment)
- [MSP2_INAV_QUERY_OUTPUT_ASSIGNMENT](#msp2_inav_query_output_assignment)
- [MSP2_INAV_OSD_UPDATE_POSITION](#msp2_inav_osd_update_position)
- [MSP2_INAV_SERVO_CONFIG](#msp2_inav_servo_config)
- [MSP2_INAV_SET_SERVO_CONFIG](#msp2_inav_set_servo_config)
- [MSP2_INAV_GEOZONE](#msp2_inav_geozone)
- [MSP2_INAV_SET_GEOZONE](#msp2_inav_set_geozone)
- [MSP2_INAV_GEOZONE_VERTEX](#msp2_inav_geozone_vertex)
- [MSP2_INAV_SET_GEOZONE_VERTEX](#msp2_inav_set_geozone_vertex)
- [MSP2_INAV_SET_GVAR](#msp2_inav_set_gvar)
- [MSP2_INAV_SET_ALT_TARGET](#msp2_inav_set_alt_target)
- [MSP2_INAV_FLIGHT_AXIS_ANGLE_OVERRIDE](#msp2_inav_flight_axis_angle_override)
- [MSP2_INAV_FLIGHT_AXIS_RATE_OVERRIDE](#msp2_inav_flight_axis_rate_override)
- [MSP2_INAV_SET_LOCAL_TARGET](#msp2_inav_set_local_target)
- [MSP2_INAV_LOCAL_TARGET](#msp2_inav_local_target)
- [MSP2_INAV_SET_GLOBAL_TARGET](#msp2_inav_set_global_target)
- [MSP2_INAV_NAV_TARGET](#msp2_inav_nav_target)
- [MSP2_INAV_FULL_LOCAL_POSE](#msp2_inav_full_local_pose)
- [MSP2_INAV_SET_WP_INDEX](#msp2_inav_set_wp_index)
- [MSP2_INAV_SET_CRUISE_HEADING](#msp2_inav_set_cruise_heading)
- [MSP2_INAV_ACTIVATE_LANDING](#msp2_inav_activate_landing)
- [MSP2_INAV_ACTIVATE_RTH](#msp2_inav_activate_rth)
- [MSP2_INAV_ARM_DISARM](#msp2_inav_arm_disarm)
- [MSP2_INAV_TIMESYNC](#msp2_inav_timesync)
- [MSP2_INAV_SET_AUX_RC](#msp2_inav_set_aux_rc)
- [MSP2_INAV_WIND](#msp2_inav_wind)
- [MSP2_INAV_MAG_UNALIGNED](#msp2_inav_mag_unaligned)
- [MSP2_INAV_ESC_SRXL2_STATUS](#msp2_inav_esc_srxl2_status)
- [MSP2_INAV_ESC_SRXL2_CALIBRATE](#msp2_inav_esc_srxl2_calibrate)
- [MSP2_BETAFLIGHT_BIND](#msp2_betaflight_bind)
- [MSP2_RX_BIND](#msp2_rx_bind)

---
## MSP_API_VERSION

id `1` · MSPv1 · group `v1`

since INAV 1.0

Provides the MSP protocol version and the INAV API version.

> Used by configurators to check compatibility.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| mspProtocolVersion | `uint8` |   | MSP Protocol version (`MSP_PROTOCOL_VERSION`, typically 0) |
| apiVersionMajor | `uint8` |   | INAV API Major version (`API_VERSION_MAJOR`) |
| apiVersionMinor | `uint8` |   | INAV API Minor version (`API_VERSION_MINOR`) |

---
## MSP_FC_VARIANT

id `2` · MSPv1 · group `v1`

since INAV 1.0

Identifies the flight controller firmware variant (e.g., INAV, Betaflight).

> See `FLIGHT_CONTROLLER_IDENTIFIER_LENGTH`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| fcVariantIdentifier | `char[4]` |   | 4-character identifier string (e.g., "INAV"). Defined by `flightControllerIdentifier`. |

---
## MSP_FC_VERSION

id `3` · MSPv1 · group `v1`

since INAV 1.0

Provides the specific version number of the flight controller firmware.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| fcVersionMajor | `uint8` |   | Firmware Major version (`FC_VERSION_MAJOR`) |
| fcVersionMinor | `uint8` |   | Firmware Minor version (`FC_VERSION_MINOR`) |
| fcVersionPatch | `uint8` |   | Firmware Patch level (`FC_VERSION_PATCH_LEVEL`) |

---
## MSP_BOARD_INFO

id `4` · MSPv1 · group `v1`

since INAV 1.0

Provides information about the specific hardware board and its capabilities.

> `BOARD_IDENTIFIER_LENGTH` is 4.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| boardIdentifier | `char[4]` |   | 4-character UPPER CASE board identifier (`TARGET_BOARD_IDENTIFIER`) |
| hardwareRevision | `uint16` |   | Hardware revision number. 0 if not detected (`USE_HARDWARE_REVISION_DETECTION`) |
| osdSupport | `uint8` |   | OSD chip type: 0=None, 2=Onboard (`USE_OSD`). INAV does not support slave OSD (1) |
| commCapabilities | `uint8` | `bitmask`  | Bitmask: Communication capabilities: Bit 0=VCP support (`USE_VCP`), Bit 1=SoftSerial support (`USE_SOFTSERIAL1`/`2`), Bit 2=compass orientation auto-detection during mag calibration (`USE_MAG_CALIBRATION_ORIENTATION`) |
| targetNameLength | `uint8` |   | Length of the target name string that follows |
| targetName | `char[]` |   | Target name string (e.g., "MATEKF405"). Length given by previous field |

---
## MSP_BUILD_INFO

id `5` · MSPv1 · group `v1`

since INAV 1.0

Provides build date, time, and Git revision of the firmware.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| buildDate | `char[BUILD_DATE_LENGTH]` |   | Build date string (e.g., "Dec 31 2023"). `BUILD_DATE_LENGTH`. |
| buildTime | `char[BUILD_TIME_LENGTH]` |   | Build time string (e.g., "23:59:59"). `BUILD_TIME_LENGTH`. |
| gitRevision | `char[GIT_SHORT_REVISION_LENGTH]` |   | Short Git revision string. `GIT_SHORT_REVISION_LENGTH`. |

---
## MSP_INAV_PID

id `6` · MSPv1 · group `v1`

since INAV 1.0

Retrieves legacy INAV-specific PID controller related settings. Many fields are now obsolete or placeholders.

> Superseded by `MSP2_PID` for core PIDs and other specific messages for filter settings.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| legacyAsyncProcessing | `uint8` |   | Legacy, unused. Always 0 |
| legacyAsyncValue1 | `uint16` |   | Legacy, unused. Always 0 |
| legacyAsyncValue2 | `int16` |   | Legacy, unused. Always 0 |
| headingHoldRateLimit | `uint8` |  deg/s | Max rate for heading hold P term (`pidProfile()->heading_hold_rate_limit`) |
| headingHoldLpfFreq | `uint8` |  Hz | Fixed LPF frequency for heading hold error (`HEADING_HOLD_ERROR_LPF_FREQ`) |
| legacyYawJumpLimit | `int16` |   | Legacy, unused. Always 0 |
| legacyGyroLpf | `uint8` |  Hz | Fixed value `GYRO_LPF_256HZ` |
| accLpfHz | `uint8` |  Hz | Accelerometer LPF frequency (`accelerometerConfig()->acc_lpf_hz`) cutoff frequency for the low pass filter used on the acc z-axis for althold in Hz |
| reserved1 | `uint8` |   | Reserved. Always 0 |
| reserved2 | `uint8` |   | Reserved. Always 0 |
| reserved3 | `uint8` |   | Reserved. Always 0 |
| reserved4 | `uint8` |   | Reserved. Always 0 |

---
## MSP_SET_INAV_PID

id `7` · MSPv1 · group `v1`

since INAV 1.0

Sets legacy INAV-specific PID controller related settings.

> Expects 15 bytes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| legacyAsyncProcessing | `uint8` |   | Legacy, ignored |
| legacyAsyncValue1 | `int16` |   | Legacy, ignored |
| legacyAsyncValue2 | `int16` |   | Legacy, ignored |
| headingHoldRateLimit | `uint8` |  deg/s | Sets `pidProfileMutable()->heading_hold_rate_limit`. |
| headingHoldLpfFreq | `uint8` |  Hz | Ignored (fixed value `HEADING_HOLD_ERROR_LPF_FREQ` used) |
| legacyYawJumpLimit | `int16` |   | Legacy, ignored |
| legacyGyroLpf | `uint8` |   | Ignored (historically mapped to `gyro_lpf_e` values). |
| accLpfHz | `uint8` |  Hz | Sets `accelerometerConfigMutable()->acc_lpf_hz`. |
| reserved1 | `uint8` |   | Ignored |
| reserved2 | `uint8` |   | Ignored |
| reserved3 | `uint8` |   | Ignored |
| reserved4 | `uint8` |   | Ignored |

*reply:* none

---
## MSP_NAME

id `10` · MSPv1 · group `v1`

since INAV 1.0

Returns the user-defined craft name.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| craftName | `char[]` |   | The craft name string (`systemConfig()->craftName`). Null termination is *not* explicitly sent, the length is determined by the payload size |

---
## MSP_SET_NAME

id `11` · MSPv1 · group `v1`

since INAV 1.0

Sets the user-defined craft name.

> Maximum length is `MAX_NAME_LENGTH`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| craftName | `cstring` |   | The new craft name string. Automatically null-terminated by the FC |

*reply:* none

---
## MSP_NAV_POSHOLD

id `12` · MSPv1 · group `v1`

since INAV 1.0

Retrieves navigation position hold and general manual/auto flight parameters. Some parameters depend on the platform type (Multirotor vs Fixed Wing).

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| userControlMode | `uint8` |   | Navigation user control mode NAV_GPS_ATTI (0) or NAV_GPS_CRUISE (1) |
| maxAutoSpeed | `uint16` |  cm/s | Max speed in autonomous modes (`navConfig()->general.max_auto_speed`) |
| maxAutoClimbRate | `uint16` |  cm/s | Max climb rate in autonomous modes (uses `fw.max_auto_climb_rate` or `mc.max_auto_climb_rate` based on platform) |
| maxManualSpeed | `uint16` |  cm/s | Max speed in manual modes with GPS aiding (`navConfig()->general.max_manual_speed`) |
| maxManualClimbRate | `uint16` |  cm/s | Max climb rate in manual modes with GPS aiding (uses `fw.max_manual_climb_rate` or `mc.max_manual_climb_rate`) |
| mcMaxBankAngle | `uint8` |  degrees | Max bank angle for multirotor position hold (`navConfig()->mc.max_bank_angle`) |
| mcAltHoldThrottleType | `uint8` | `navMcAltHoldThrottle_e`  | Enum `navMcAltHoldThrottle_e` mirrored from `navConfig()->mc.althold_throttle_type`. |
| mcHoverThrottle | `uint16` |  PWM | Multirotor hover throttle PWM value (`currentBatteryProfile->nav.mc.hover_throttle`). |

---
## MSP_SET_NAV_POSHOLD

id `13` · MSPv1 · group `v1`

since INAV 1.0

Sets navigation position hold and general manual/auto flight parameters.

> Expects 13 bytes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| userControlMode | `uint8` | `nav_control_type_e`  | Sets `navConfigMutable()->general.flags.user_control_mode`. WARNING: uses unnamed enum in navigation.h 'NAV_GPS_ATTI/NAV_GPS_CRUISE' |
| maxAutoSpeed | `uint16` |  cm/s | Sets `navConfigMutable()->general.max_auto_speed`. |
| maxAutoClimbRate | `uint16` |  cm/s | Sets `navConfigMutable()->fw.max_auto_climb_rate` or `navConfigMutable()->mc.max_auto_climb_rate` based on `mixerConfig()->platformType`. |
| maxManualSpeed | `uint16` |  cm/s | Sets `navConfigMutable()->general.max_manual_speed`. |
| maxManualClimbRate | `uint16` |  cm/s | Sets `navConfigMutable()->fw.max_manual_climb_rate` or `navConfigMutable()->mc.max_manual_climb_rate`. |
| mcMaxBankAngle | `uint8` |  degrees | Sets `navConfigMutable()->mc.max_bank_angle`. |
| mcAltHoldThrottleType | `uint8` | `navMcAltHoldThrottle_e`  | Enum `navMcAltHoldThrottle_e`; updates `navConfigMutable()->mc.althold_throttle_type`. |
| mcHoverThrottle | `uint16` |  PWM | Sets `currentBatteryProfileMutable->nav.mc.hover_throttle`. |

*reply:* none

---
## MSP_CALIBRATION_DATA

id `14` · MSPv1 · group `v1`

since INAV 1.0

Retrieves sensor calibration data (Accelerometer zero/gain, Magnetometer zero/gain, Optical Flow scale).

> Total size 27 bytes. Fields related to optional sensors are zero if the sensor is not used.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| accCalibAxisFlags | `uint8` | `bitmask`  | Bitmask: Flags indicating which axes of the accelerometer have been calibrated (`accGetCalibrationAxisFlags()`) |
| accZeroX | `int16` |  Raw ADC | Accelerometer zero offset for X-axis (`accelerometerConfig()->accZero.raw[X]`) |
| accZeroY | `int16` |  Raw ADC | Accelerometer zero offset for Y-axis (`accelerometerConfig()->accZero.raw[Y]`) |
| accZeroZ | `int16` |  Raw ADC | Accelerometer zero offset for Z-axis (`accelerometerConfig()->accZero.raw[Z]`) |
| accGainX | `int16` |  Raw ADC | Accelerometer gain/scale for X-axis (`accelerometerConfig()->accGain.raw[X]`) |
| accGainY | `int16` |  Raw ADC | Accelerometer gain/scale for Y-axis (`accelerometerConfig()->accGain.raw[Y]`) |
| accGainZ | `int16` |  Raw ADC | Accelerometer gain/scale for Z-axis (`accelerometerConfig()->accGain.raw[Z]`) |
| magZeroX | `int16` |  Raw ADC | Magnetometer zero offset for X-axis (`compassConfig()->magZero.raw[X]`). 0 if `USE_MAG` disabled |
| magZeroY | `int16` |  Raw ADC | Magnetometer zero offset for Y-axis (`compassConfig()->magZero.raw[Y]`). 0 if `USE_MAG` disabled |
| magZeroZ | `int16` |  Raw ADC | Magnetometer zero offset for Z-axis (`compassConfig()->magZero.raw[Z]`). 0 if `USE_MAG` disabled |
| opflowScale | `uint16` |  Scale * 256 | Optical flow scale factor (`opticalFlowConfig()->opflow_scale * 256`). 0 if `USE_OPFLOW` disabled |
| magGainX | `int16` |  Raw ADC | Magnetometer gain/scale for X-axis (`compassConfig()->magGain[X]`). 0 if `USE_MAG` disabled |
| magGainY | `int16` |  Raw ADC | Magnetometer gain/scale for Y-axis (`compassConfig()->magGain[Y]`). 0 if `USE_MAG` disabled |
| magGainZ | `int16` |  Raw ADC | Magnetometer gain/scale for Z-axis (`compassConfig()->magGain[Z]`). 0 if `USE_MAG` disabled |

---
## MSP_SET_CALIBRATION_DATA

id `15` · MSPv1 · group `v1`

since INAV 1.0

Sets sensor calibration data.

> Minimum payload 18 bytes. Adds +6 bytes for magnetometer zeros, +2 for optical flow scale, and +6 for magnetometer gains when those features (`USE_MAG`, `USE_OPFLOW`) are compiled in.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| accZeroX | `int16` |  Raw ADC | Sets `accelerometerConfigMutable()->accZero.raw[X]`. |
| accZeroY | `int16` |  Raw ADC | Sets `accelerometerConfigMutable()->accZero.raw[Y]`. |
| accZeroZ | `int16` |  Raw ADC | Sets `accelerometerConfigMutable()->accZero.raw[Z]`. |
| accGainX | `int16` |  Raw ADC | Sets `accelerometerConfigMutable()->accGain.raw[X]`. |
| accGainY | `int16` |  Raw ADC | Sets `accelerometerConfigMutable()->accGain.raw[Y]`. |
| accGainZ | `int16` |  Raw ADC | Sets `accelerometerConfigMutable()->accGain.raw[Z]`. |
| magZeroX | `int16` |  Raw ADC | Sets `compassConfigMutable()->magZero.raw[X]` (if `USE_MAG`) |
| magZeroY | `int16` |  Raw ADC | Sets `compassConfigMutable()->magZero.raw[Y]` (if `USE_MAG`) |
| magZeroZ | `int16` |  Raw ADC | Sets `compassConfigMutable()->magZero.raw[Z]` (if `USE_MAG`) |
| opflowScale | `optional uint16` |  Scale * 256 | Sets `opticalFlowConfigMutable()->opflow_scale = value / 256.0f` (if `USE_OPFLOW`) |
| magGainX | `optional int16` |  Raw ADC | Sets `compassConfigMutable()->magGain[X]` (if `USE_MAG`) |
| magGainY | `optional int16` |  Raw ADC | Sets `compassConfigMutable()->magGain[Y]` (if `USE_MAG`) |
| magGainZ | `optional int16` |  Raw ADC | Sets `compassConfigMutable()->magGain[Z]` (if `USE_MAG`) |

*reply:* none

---
## MSP_POSITION_ESTIMATION_CONFIG

id `16` · MSPv1 · group `v1`

since INAV 1.0

Retrieves parameters related to the INAV position estimation fusion weights and GPS minimum satellite count.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| weightZBaroP | `uint16` |  Weight * 100 | Barometer Z position fusion weight (`positionEstimationConfig()->w_z_baro_p * 100`) |
| weightZGPSP | `uint16` |  Weight * 100 | GPS Z position fusion weight (`positionEstimationConfig()->w_z_gps_p * 100`) |
| weightZGPSV | `uint16` |  Weight * 100 | GPS Z velocity fusion weight (`positionEstimationConfig()->w_z_gps_v * 100`) |
| weightXYGPSP | `uint16` |  Weight * 100 | GPS XY position fusion weight (`positionEstimationConfig()->w_xy_gps_p * 100`) |
| weightXYGPSV | `uint16` |  Weight * 100 | GPS XY velocity fusion weight (`positionEstimationConfig()->w_xy_gps_v * 100`) |
| minSats | `uint8` |  Count | Minimum satellites required for GPS use (`gpsConfigMutable()->gpsMinSats`) |
| useGPSVelNED | `uint8` |  Boolean | Legacy flag, always 1 (GPS velocity is always used if available) |

---
## MSP_SET_POSITION_ESTIMATION_CONFIG

id `17` · MSPv1 · group `v1`

since INAV 1.0

Sets parameters related to the INAV position estimation fusion weights and GPS minimum satellite count.

> Expects 12 bytes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| weightZBaroP | `uint16` |  Weight * 100 | Sets `positionEstimationConfigMutable()->w_z_baro_p = value / 100.0f` (constrained 0.0-10.0) |
| weightZGPSP | `uint16` |  Weight * 100 | Sets `positionEstimationConfigMutable()->w_z_gps_p = value / 100.0f` (constrained 0.0-10.0) |
| weightZGPSV | `uint16` |  Weight * 100 | Sets `positionEstimationConfigMutable()->w_z_gps_v = value / 100.0f` (constrained 0.0-10.0) |
| weightXYGPSP | `uint16` |  Weight * 100 | Sets `positionEstimationConfigMutable()->w_xy_gps_p = value / 100.0f` (constrained 0.0-10.0) |
| weightXYGPSV | `uint16` |  Weight * 100 | Sets `positionEstimationConfigMutable()->w_xy_gps_v = value / 100.0f` (constrained 0.0-10.0) |
| minSats | `uint8` |  Count | Sets `gpsConfigMutable()->gpsMinSats` (constrained 5-10) |
| useGPSVelNED | `uint8` |  Boolean | Legacy flag, ignored |

*reply:* none

---
## MSP_WP_MISSION_LOAD

id `18` · MSPv1 · group `v1`

since INAV 1.0

Commands the FC to load the waypoint mission stored in non-volatile memory (e.g., EEPROM or FlashFS) into the active mission buffer.

> Only functional if `NAV_NON_VOLATILE_WAYPOINT_STORAGE` is defined. Requires 1 byte payload. Returns error if loading fails.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| missionID | `uint8` |   | Reserved for future use, currently ignored |

*reply:* none

---
## MSP_WP_MISSION_SAVE

id `19` · MSPv1 · group `v1`

since INAV 1.0

Commands the FC to save the currently active waypoint mission from RAM to non-volatile memory (e.g., EEPROM or FlashFS).

> Only functional if `NAV_NON_VOLATILE_WAYPOINT_STORAGE` is defined. Requires 1 byte payload. Returns error if saving fails.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| missionID | `uint8` |   | Reserved for future use, currently ignored |

*reply:* none

---
## MSP_WP_GETINFO

id `20` · MSPv1 · group `v1`

since INAV 1.0

Retrieves information about the waypoint mission capabilities and the status of the currently loaded mission.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| wpCapabilities | `uint8` |   | Reserved for future waypoint capabilities flags. Currently always 0 |
| maxWaypoints | `uint8` |   | Maximum number of waypoints supported (`NAV_MAX_WAYPOINTS`) |
| missionValid | `uint8` |   | Boolean flag indicating if the current mission in RAM is valid (`isWaypointListValid()`) |
| waypointCount | `uint8` |   | Number of waypoints currently defined in the mission (`getWaypointCount()`) |

---
## MSP_RTH_AND_LAND_CONFIG

id `21` · MSPv1 · group `v1`

since INAV 1.0

Retrieves configuration parameters related to Return-to-Home (RTH) and automatic landing behaviors.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| minRthDistance | `uint16` |  cm | Minimum distance from home required for RTH to engage (`navConfig()->general.min_rth_distance`) |
| rthClimbFirst | `uint8` |  Boolean | Flag: Climb to RTH altitude before returning (`navConfig()->general.flags.rth_climb_first`) |
| rthClimbIgnoreEmerg | `uint8` |  Boolean | Flag: Climb even in emergency RTH (`navConfig()->general.flags.rth_climb_ignore_emerg`) |
| rthTailFirst | `uint8` |  Boolean | Flag: Return tail-first during RTH (`navConfig()->general.flags.rth_tail_first`) |
| rthAllowLanding | `uint8` |  Boolean | Flag: Allow automatic landing after RTH (`navConfig()->general.flags.rth_allow_landing`) |
| rthAltControlMode | `uint8` | `nav_rth_alt_profile_e`  | RTH altitude control mode (`navConfig()->general.flags.rth_alt_control_mode`). WARNING: uses unnamed enum in navigation.h:253 'NAV_RTH_NO_ALT...' |
| rthAbortThreshold | `uint16` |  cm | Distance increase threshold to abort RTH (`navConfig()->general.rth_abort_threshold`) |
| rthAltitude | `uint16` |  cm | Target RTH altitude (`navConfig()->general.rth_altitude`) |
| landMinAltVspd | `uint16` |  cm/s | Landing vertical speed at minimum slowdown altitude (`navConfig()->general.land_minalt_vspd`) |
| landMaxAltVspd | `uint16` |  cm/s | Landing vertical speed at maximum slowdown altitude (`navConfig()->general.land_maxalt_vspd`) |
| landSlowdownMinAlt | `uint16` |  cm | Altitude below which `landMinAltVspd` applies (`navConfig()->general.land_slowdown_minalt`) |
| landSlowdownMaxAlt | `uint16` |  cm | Altitude above which `landMaxAltVspd` applies (`navConfig()->general.land_slowdown_maxalt`) |
| emergDescentRate | `uint16` |  cm/s | Vertical speed during emergency landing descent (`navConfig()->general.emerg_descent_rate`) |

---
## MSP_SET_RTH_AND_LAND_CONFIG

id `22` · MSPv1 · group `v1`

since INAV 1.0

Sets configuration parameters related to Return-to-Home (RTH) and automatic landing behaviors.

> Expects 21 bytes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| minRthDistance | `uint16` |  cm | Sets `navConfigMutable()->general.min_rth_distance`. |
| rthClimbFirst | `uint8` |  Boolean | Sets `navConfigMutable()->general.flags.rth_climb_first`. |
| rthClimbIgnoreEmerg | `uint8` |  Boolean | Sets `navConfigMutable()->general.flags.rth_climb_ignore_emerg`. |
| rthTailFirst | `uint8` |  Boolean | Sets `navConfigMutable()->general.flags.rth_tail_first`. |
| rthAllowLanding | `uint8` |  Boolean | Sets `navConfigMutable()->general.flags.rth_allow_landing`. |
| rthAltControlMode | `uint8` | `nav_rth_alt_profile_e`  | Sets `navConfigMutable()->general.flags.rth_alt_control_mode`. WARNING: uses unnamed enum in navigation.h:253 |
| rthAbortThreshold | `uint16` |  cm | Sets `navConfigMutable()->general.rth_abort_threshold`. |
| rthAltitude | `uint16` |  cm | Sets `navConfigMutable()->general.rth_altitude`. |
| landMinAltVspd | `uint16` |  cm/s | Sets `navConfigMutable()->general.land_minalt_vspd`. |
| landMaxAltVspd | `uint16` |  cm/s | Sets `navConfigMutable()->general.land_maxalt_vspd`. |
| landSlowdownMinAlt | `uint16` |  cm | Sets `navConfigMutable()->general.land_slowdown_minalt`. |
| landSlowdownMaxAlt | `uint16` |  cm | Sets `navConfigMutable()->general.land_slowdown_maxalt`. |
| emergDescentRate | `uint16` |  cm/s | Sets `navConfigMutable()->general.emerg_descent_rate`. |

*reply:* none

---
## MSP_FW_CONFIG

id `23` · MSPv1 · group `v1`

since INAV 1.0

Retrieves configuration parameters specific to Fixed Wing navigation.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| cruiseThrottle | `uint16` |  PWM | Cruise throttle command (`currentBatteryProfile->nav.fw.cruise_throttle`). |
| minThrottle | `uint16` |  PWM | Minimum throttle during autonomous flight (`currentBatteryProfile->nav.fw.min_throttle`). |
| maxThrottle | `uint16` |  PWM | Maximum throttle during autonomous flight (`currentBatteryProfile->nav.fw.max_throttle`). |
| maxBankAngle | `uint8` |  degrees | Maximum bank angle allowed (`navConfig()->fw.max_bank_angle`) |
| maxClimbAngle | `uint8` |  degrees | Maximum pitch angle during climb (`navConfig()->fw.max_climb_angle`) |
| maxDiveAngle | `uint8` |  degrees | Maximum negative pitch angle during descent (`navConfig()->fw.max_dive_angle`) |
| pitchToThrottle | `uint8` |  us/deg | Pitch-to-throttle gain (`currentBatteryProfile->nav.fw.pitch_to_throttle`); PWM microseconds per degree (10 units ≈ 1% throttle). |
| loiterRadius | `uint16` |  cm | Default loiter radius (`navConfig()->fw.loiter_radius`). |

---
## MSP_SET_FW_CONFIG

id `24` · MSPv1 · group `v1`

since INAV 1.0

Sets configuration parameters specific to Fixed Wing navigation.

> Expects 12 bytes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| cruiseThrottle | `uint16` |  PWM | Sets `currentBatteryProfileMutable->nav.fw.cruise_throttle`. |
| minThrottle | `uint16` |  PWM | Sets `currentBatteryProfileMutable->nav.fw.min_throttle`. |
| maxThrottle | `uint16` |  PWM | Sets `currentBatteryProfileMutable->nav.fw.max_throttle`. |
| maxBankAngle | `uint8` |  degrees | Sets `navConfigMutable()->fw.max_bank_angle`. |
| maxClimbAngle | `uint8` |  degrees | Sets `navConfigMutable()->fw.max_climb_angle`. |
| maxDiveAngle | `uint8` |  degrees | Sets `navConfigMutable()->fw.max_dive_angle`. |
| pitchToThrottle | `uint8` |  us/deg | Sets `currentBatteryProfileMutable->nav.fw.pitch_to_throttle` (PWM microseconds per degree; 10 units ≈ 1% throttle). |
| loiterRadius | `uint16` |  cm | Sets `navConfigMutable()->fw.loiter_radius`. |

*reply:* none

---
## MSP_MODE_RANGES

id `34` · MSPv1 · group `v1`

since INAV 1.0

Returns all defined mode activation ranges (aux channel assignments for flight modes).

> The number of steps and mapping to PWM values depends on internal range calculations.

*request:* none

*reply:* (repeat: MAX_MODE_ACTIVATION_CONDITION_COUNT)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| modePermanentId | `uint8` |  ID | Permanent ID of the flight mode (maps to `boxId` via `findBoxByActiveBoxId`). 0 if entry unused |
| auxChannelIndex | `uint8` |  Index | 0-based index of the AUX channel used for activation |
| rangeStartStep | `uint8` |  step | Start step (0-48). Each step is 25 PWM units; 0 is <=900 and 48 is >=2100. |
| rangeEndStep | `uint8` |  step | End step (0-48). Uses the same 25-PWM step mapping as rangeStartStep. |

---
## MSP_SET_MODE_RANGE

id `35` · MSPv1 · group `v1`

since INAV 1.0

Sets a single mode activation range by its index.

> Expects 5 bytes. Updates the mode configuration and recalculates used mode flags. Returns error if `rangeIndex` or `modePermanentId` is invalid.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rangeIndex | `uint8` |  Index | Index of the mode range to set (0 to `MAX_MODE_ACTIVATION_CONDITION_COUNT - 1`) |
| modePermanentId | `uint8` |  ID | Permanent ID of the flight mode to assign |
| auxChannelIndex | `uint8` |  Index | 0-based index of the AUX channel |
| rangeStartStep | `uint8` |  step | Start step (0-48). Each step is 25 PWM units; 0 is <=900 and 48 is >=2100. |
| rangeEndStep | `uint8` |  step | End step (0-48). Uses the same 25-PWM step mapping as rangeStartStep. |

*reply:* none

---
## MSP_FEATURE

id `36` · MSPv1 · group `v1`

since INAV 1.0

Returns a bitmask of enabled features.

> Feature bits are defined in `feature.h`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| featureMask | `uint32` | `features_e (bitmask)`  | Bitmask: active features (see `featureMask()`) |

---
## MSP_SET_FEATURE

id `37` · MSPv1 · group `v1`

since INAV 1.0

Sets the enabled features using a bitmask. Clears all previous features first.

> Expects 4 bytes. Updates feature configuration and related settings (e.g., RSSI source).

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| featureMask | `uint32` | `features_e (bitmask)`  | Bitmask: features to enable |

*reply:* none

---
## MSP_BOARD_ALIGNMENT

id `38` · MSPv1 · group `v1`

since INAV 1.0

Returns the sensor board alignment angles relative to the craft frame.

> Ranges are typically -1800 to +1800 (i.e. -180.0° to +180.0°).

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rollAlign | `int16` |  deci-degrees | Board alignment roll angle (`boardAlignment()->rollDeciDegrees`). Negative values tilt left. |
| pitchAlign | `int16` |  deci-degrees | Board alignment pitch angle (`boardAlignment()->pitchDeciDegrees`). Negative values nose down. |
| yawAlign | `int16` |  deci-degrees | Board alignment yaw angle (`boardAlignment()->yawDeciDegrees`). Negative values rotate counter-clockwise. |

---
## MSP_SET_BOARD_ALIGNMENT

id `39` · MSPv1 · group `v1`

since INAV 1.0

Sets the sensor board alignment angles.

> Expects 6 bytes encoded as little-endian signed deci-degrees (-1800 to +1800 typical).

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rollAlign | `int16` |  deci-degrees | Sets `boardAlignmentMutable()->rollDeciDegrees`. |
| pitchAlign | `int16` |  deci-degrees | Sets `boardAlignmentMutable()->pitchDeciDegrees`. |
| yawAlign | `int16` |  deci-degrees | Sets `boardAlignmentMutable()->yawDeciDegrees`. |

*reply:* none

---
## MSP_CURRENT_METER_CONFIG

id `40` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the configuration for the current sensor.

> Scale and offset are signed values matching `batteryMetersConfig()->current` fields.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| scale | `int16` |  0.1 mV/A | Current sensor scale factor (`batteryMetersConfig()->current.scale`). Stored in 0.1 mV/A; signed for calibration. |
| offset | `int16` |  mV | Current sensor offset (`batteryMetersConfig()->current.offset`). Signed millivolt adjustment. |
| type | `uint8` | `currentSensor_e`  | Enum `currentSensor_e` Type of current sensor hardware |
| capacity | `uint16` |  mAh (legacy) | Battery capacity (constrained 0-65535) (`currentBatteryProfile->capacity.value`). Note: This is legacy, use `MSP2_INAV_BATTERY_CONFIG` for full 32-bit capacity |

---
## MSP_SET_CURRENT_METER_CONFIG

id `41` · MSPv1 · group `v1`

since INAV 1.0

Sets the configuration for the current sensor.

> Expects 7 bytes. Signed values use little-endian two's complement.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| scale | `int16` |  0.1 mV/A | Sets `batteryMetersConfigMutable()->current.scale` (0.1 mV/A, signed). |
| offset | `int16` |  mV | Sets `batteryMetersConfigMutable()->current.offset` (signed millivolts). |
| type | `uint8` | `currentSensor_e`  | Enum `currentSensor_e` Sets `batteryMetersConfigMutable()->current.type`. |
| capacity | `uint16` |  mAh (legacy) | Sets `currentBatteryProfileMutable->capacity.value` (truncated to 16 bits) |

*reply:* none

---
## MSP_MIXER

id `42` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the mixer type (Legacy, INAV always returns QuadX).

> This command is largely obsolete. Mixer configuration is handled differently in INAV (presets, custom mixes). See `MSP2_INAV_MIXER`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| mixerMode | `uint8` |   | Always 3 (QuadX) in INAV for compatibility |

---
## MSP_SET_MIXER

id `43` · MSPv1 · group `v1`

since INAV 1.0

Sets the mixer type (Legacy, ignored by INAV).

> Expects 1 byte. Calls `mixerUpdateStateFlags()` for potential side effects related to presets.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| mixerMode | `uint8` |   | Mixer mode to set (ignored by INAV) |

*reply:* none

---
## MSP_RX_CONFIG

id `44` · MSPv1 · group `v1`

since INAV 1.0

Retrieves receiver configuration settings. Some fields are Betaflight compatibility placeholders.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| serialRxProvider | `uint8` | `rxSerialReceiverType_e`  | Enum `rxSerialReceiverType_e`. Serial RX provider (`rxConfig()->serialrx_provider`). |
| maxCheck | `uint16` |  PWM | Upper channel value threshold for stick commands (`rxConfig()->maxcheck`) |
| midRc | `uint16` |  PWM | Center channel value (`PWM_RANGE_MIDDLE`, typically 1500) |
| minCheck | `uint16` |  PWM | Lower channel value threshold for stick commands (`rxConfig()->mincheck`) |
| spektrumSatBind | `uint8` |  Count/Flag | Spektrum bind pulses (`rxConfig()->spektrum_sat_bind`). 0 if `USE_SPEKTRUM_BIND` disabled. |
| rxMinUsec | `uint16` |  PWM | Minimum expected pulse width (`rxConfig()->rx_min_usec`) |
| rxMaxUsec | `uint16` |  PWM | Maximum expected pulse width (`rxConfig()->rx_max_usec`) |
| bfCompatRcInterpolation | `uint8` |   | BF compatibility. Always 0 |
| bfCompatRcInterpolationInt | `uint8` |   | BF compatibility. Always 0 |
| bfCompatAirModeThreshold | `uint16` |   | BF compatibility. Always 0 |
| reserved1 | `uint8` |   | Reserved/Padding. Always 0 |
| reserved2 | `uint32` |   | Reserved/Padding. Always 0 |
| reserved3 | `uint8` |   | Reserved/Padding. Always 0 |
| bfCompatFpvCamAngle | `uint8` |   | BF compatibility. Always 0 |
| receiverType | `uint8` | `rxReceiverType_e`  | Enum `rxReceiverType_e` Receiver type (Parallel PWM, PPM, Serial) ('rxConfig()->receiverType') |

---
## MSP_SET_RX_CONFIG

id `45` · MSPv1 · group `v1`

since INAV 1.0

Sets receiver configuration settings.

> Expects 24 bytes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| serialRxProvider | `uint8` | `rxSerialReceiverType_e`  | Enum `rxSerialReceiverType_e`. Sets `rxConfigMutable()->serialrx_provider`. |
| maxCheck | `uint16` |  PWM | Sets `rxConfigMutable()->maxcheck`. |
| midRc | `uint16` |  PWM | Ignored (`PWM_RANGE_MIDDLE` is used) |
| minCheck | `uint16` |  PWM | Sets `rxConfigMutable()->mincheck`. |
| spektrumSatBind | `uint8` |  Count/Flag | Sets `rxConfigMutable()->spektrum_sat_bind` (if `USE_SPEKTRUM_BIND`). |
| rxMinUsec | `uint16` |  PWM | Sets `rxConfigMutable()->rx_min_usec`. |
| rxMaxUsec | `uint16` |  PWM | Sets `rxConfigMutable()->rx_max_usec`. |
| bfCompatRcInterpolation | `uint8` |   | Ignored |
| bfCompatRcInterpolationInt | `uint8` |   | Ignored |
| bfCompatAirModeThreshold | `uint16` |   | Ignored |
| reserved1 | `uint8` |   | Ignored |
| reserved2 | `uint32` |   | Ignored |
| reserved3 | `uint8` |   | Ignored |
| bfCompatFpvCamAngle | `uint8` |   | Ignored |
| receiverType | `uint8` | `rxReceiverType_e`  | Enum `rxReceiverType_e` Sets `rxConfigMutable()->receiverType`. |

*reply:* none

---
## MSP_LED_COLORS

id `46` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the HSV color definitions for configurable LED colors.

> Only available if `USE_LED_STRIP` is defined.

*request:* none

*reply:* (repeat: LED_CONFIGURABLE_COLOR_COUNT)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| hue | `uint16` |   | Hue value (0-359) |
| saturation | `uint8` |   | Saturation value (0-255) |
| value | `uint8` |   | Value/Brightness (0-255) |

---
## MSP_SET_LED_COLORS

id `47` · MSPv1 · group `v1`

since INAV 1.0

Sets the HSV color definitions for configurable LED colors.

> Only available if `USE_LED_STRIP` is defined. Expects `LED_CONFIGURABLE_COLOR_COUNT * 4` bytes.

*request:* (repeat: LED_CONFIGURABLE_COLOR_COUNT)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| hue | `uint16` |   | Hue value (0-359) |
| saturation | `uint8` |   | Saturation value (0-255) |
| value | `uint8` |   | Value/Brightness (0-255) |

*reply:* none

---
## MSP_LED_STRIP_CONFIG

id `48` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the configuration for each LED on the strip (legacy packed format).

> Only available if `USE_LED_STRIP` is defined. Superseded by `MSP2_INAV_LED_STRIP_CONFIG_EX` which uses a clearer struct.

*request:* none

*reply:* (repeat: LED_MAX_STRIP_LENGTH)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| legacyLedConfig | `uint32` |   | Packed LED configuration (position, function, overlay, color, direction, params). See C code for bit packing details |

---
## MSP_SET_LED_STRIP_CONFIG

id `49` · MSPv1 · group `v1`

since INAV 1.0

Sets the configuration for a single LED on the strip using the legacy packed format.

> Only available if `USE_LED_STRIP` is defined. Expects 5 bytes. Calls `reevaluateLedConfig()`. Superseded by `MSP2_INAV_SET_LED_STRIP_CONFIG_EX`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| ledIndex | `uint8` |   | Index of the LED to configure (0 to `LED_MAX_STRIP_LENGTH - 1`) |
| legacyLedConfig | `uint32` |   | Packed LED configuration to set |

*reply:* none

---
## MSP_RSSI_CONFIG

id `50` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the channel used for analog RSSI input.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rssiChannel | `uint8` |   | AUX channel index (1-based) used for RSSI, or 0 if disabled (`rxConfig()->rssi_channel`) |

---
## MSP_SET_RSSI_CONFIG

id `51` · MSPv1 · group `v1`

since INAV 1.0

Sets the channel used for analog RSSI input.

> Expects 1 byte. Input value is constrained 0 to `MAX_SUPPORTED_RC_CHANNEL_COUNT`. Updates the effective RSSI source.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rssiChannel | `uint8` |   | AUX channel index (1-based) to use for RSSI, or 0 to disable |

*reply:* none

---
## MSP_ADJUSTMENT_RANGES

id `52` · MSPv1 · group `v1`

since INAV 1.0

Returns all defined RC adjustment ranges (tuning via aux channels).

> See `adjustmentRange_t`.

*request:* none

*reply:* (repeat: MAX_ADJUSTMENT_RANGE_COUNT)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| adjustmentIndex | `uint8` |   | Index of the adjustment slot (0 to `MAX_SIMULTANEOUS_ADJUSTMENT_COUNT - 1`) |
| auxChannelIndex | `uint8` |   | 0-based index of the AUX channel controlling the adjustment value |
| rangeStartStep | `uint8` |  step | Start step (0-48). Each step is 25 PWM units; 0 is <=900 and 48 is >=2100. |
| rangeEndStep | `uint8` |  step | End step (0-48). Uses the same 25-PWM step mapping as rangeStartStep. |
| adjustmentFunction | `uint8` | `adjustmentFunction_e`  | Function/parameter being adjusted (see `adjustmentFunction_e`). |
| auxSwitchChannelIndex | `uint8` |   | 0-based index of the AUX channel acting as an enable switch (or 0 if always enabled) |

---
## MSP_SET_ADJUSTMENT_RANGE

id `53` · MSPv1 · group `v1`

since INAV 1.0

Sets a single RC adjustment range configuration by its index.

> Expects 7 bytes. Returns error if `rangeIndex` or `adjustmentIndex` is invalid.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rangeIndex | `uint8` |   | Index of the adjustment range to set (0 to `MAX_ADJUSTMENT_RANGE_COUNT - 1`) |
| adjustmentIndex | `uint8` |   | Adjustment slot index (0 to `MAX_SIMULTANEOUS_ADJUSTMENT_COUNT - 1`) |
| auxChannelIndex | `uint8` |   | 0-based index of the control AUX channel |
| rangeStartStep | `uint8` |  step | Start step (0-48). Each step is 25 PWM units; 0 is <=900 and 48 is >=2100. |
| rangeEndStep | `uint8` |  step | End step (0-48). Uses the same 25-PWM step mapping as rangeStartStep. |
| adjustmentFunction | `uint8` | `adjustmentFunction_e`  | Function/parameter being adjusted. |
| auxSwitchChannelIndex | `uint8` |   | 0-based index of the enable switch AUX channel (or 0) |

*reply:* none

---
## MSP_CF_SERIAL_CONFIG

id `54` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

Deprecated command to get serial port configuration.

> Not implemented in INAV `fc_msp.c`. Use `MSP2_COMMON_SERIAL_CONFIG`.

*request:* none

*reply:* none

---
## MSP_SET_CF_SERIAL_CONFIG

id `55` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

Deprecated command to set serial port configuration.

> Not implemented in INAV `fc_msp.c`. Use `MSP2_COMMON_SET_SERIAL_CONFIG`.

*request:* none

*reply:* none

---
## MSP_VOLTAGE_METER_CONFIG

id `56` · MSPv1 · group `v1`

since INAV 1.0

Retrieves legacy voltage meter configuration (scaled values).

> Superseded by `MSP2_INAV_BATTERY_CONFIG`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| vbatScale | `uint8` |  Scale / 10 | Voltage sensor scale factor / 10 (`batteryMetersConfig()->voltage.scale / 10`). 0 if `USE_ADC` disabled |
| vbatMinCell | `uint8` |  0.1V | Minimum cell voltage / 10 (`currentBatteryProfile->voltage.cellMin / 10`). 0 if `USE_ADC` disabled |
| vbatMaxCell | `uint8` |  0.1V | Maximum cell voltage / 10 (`currentBatteryProfile->voltage.cellMax / 10`). 0 if `USE_ADC` disabled |
| vbatWarningCell | `uint8` |  0.1V | Warning cell voltage / 10 (`currentBatteryProfile->voltage.cellWarning / 10`). 0 if `USE_ADC` disabled |

---
## MSP_SET_VOLTAGE_METER_CONFIG

id `57` · MSPv1 · group `v1`

since INAV 1.0

Sets legacy voltage meter configuration (scaled values).

> Expects 4 bytes. Superseded by `MSP2_INAV_SET_BATTERY_CONFIG`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| vbatScale | `uint8` |  Scale / 10 | Sets `batteryMetersConfigMutable()->voltage.scale = value * 10` (if `USE_ADC`) |
| vbatMinCell | `uint8` |  0.1V | Sets `currentBatteryProfileMutable->voltage.cellMin = value * 10` (if `USE_ADC`) |
| vbatMaxCell | `uint8` |  0.1V | Sets `currentBatteryProfileMutable->voltage.cellMax = value * 10` (if `USE_ADC`) |
| vbatWarningCell | `uint8` |  0.1V | Sets `currentBatteryProfileMutable->voltage.cellWarning = value * 10` (if `USE_ADC`) |

*reply:* none

---
## MSP_SONAR_ALTITUDE

id `58` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the altitude measured by the primary rangefinder (sonar or lidar).

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rangefinderAltitude | `int32` |  cm | Latest altitude reading from the rangefinder (`rangefinderGetLatestAltitude()`). 0 if `USE_RANGEFINDER` disabled or no reading. |

---
## MSP_RX_MAP

id `64` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the RC channel mapping array (AETR, etc.).

> `MAX_MAPPABLE_RX_INPUTS` is currently 4 (Roll, Pitch, Yaw, Throttle).

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rcMap | `uint8[MAX_MAPPABLE_RX_INPUTS]` |   | Array defining the mapping from input channel index to logical function (Roll, Pitch, Yaw, Throttle, Aux1...) |

---
## MSP_SET_RX_MAP

id `65` · MSPv1 · group `v1`

since INAV 1.0

Sets the RC channel mapping array.

> Expects `MAX_MAPPABLE_RX_INPUTS` bytes (currently 4).

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rcMap | `uint8[MAX_MAPPABLE_RX_INPUTS]` |   | Array defining the new channel mapping |

*reply:* none

---
## MSP_REBOOT

id `68` · MSPv1 · group `v1`

since INAV 1.0

Commands the flight controller to reboot.

> The FC sends an ACK *before* rebooting. The `mspPostProcessFn` is set to `mspRebootFn` to perform the reboot after the reply is sent. Will fail if the craft is armed.

*request:* none

*reply:* none

---
## MSP_DATAFLASH_SUMMARY

id `70` · MSPv1 · group `v1`

since INAV 1.0

Retrieves summary information about the onboard dataflash chip (if present and used for Blackbox via FlashFS).

> Requires `USE_FLASHFS`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| flashReady | `uint8` |   | Boolean: 1 if flash chip is ready, 0 otherwise. (`flashIsReady()`). 0 if `USE_FLASHFS` disabled |
| sectorCount | `uint32` |   | Total number of sectors on the flash chip (`geometry->sectors`). 0 if `USE_FLASHFS` disabled |
| totalSize | `uint32` |   | Total size of the flash chip in bytes (`geometry->totalSize`). 0 if `USE_FLASHFS` disabled |
| usedSize | `uint32` |   | Currently used size in bytes (FlashFS offset) (`flashfsGetOffset()`). 0 if `USE_FLASHFS` disabled |

---
## MSP_DATAFLASH_READ

id `71` · MSPv1 · group `v1`

since INAV 1.0

Reads a block of data from the onboard dataflash (FlashFS).

> Requires `USE_FLASHFS`. Read length may be truncated by buffer size or end of flashfs volume.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| address | `uint32` |   | Starting address to read from within the FlashFS volume |
| size | `optional uint16` |   | (Optional) Number of bytes to read. Defaults to 128 if not provided |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| address | `uint32` |   | The starting address from which data was actually read |
| data | `uint8[]` |   | The data read from flash. Length is MIN(requested size, remaining buffer space, remaining flashfs data) |

---
## MSP_DATAFLASH_ERASE

id `72` · MSPv1 · group `v1`

since INAV 1.0

Erases the entire onboard dataflash chip (FlashFS volume).

> Requires `USE_FLASHFS`. This is a potentially long operation. Use with caution.

*request:* none

*reply:* none

---
## MSP_LOOP_TIME

id `73` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the configured loop time (PID loop frequency denominator).

> This is the *configured* target loop time, not necessarily the *actual* measured cycle time (see `MSP_STATUS`).

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| looptime | `uint16` |  PWM | Configured loop time (`gyroConfig()->looptime`) |

---
## MSP_SET_LOOP_TIME

id `74` · MSPv1 · group `v1`

since INAV 1.0

Sets the configured loop time.

> Expects 2 bytes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| looptime | `uint16` |  PWM | New loop time to set (`gyroConfigMutable()->looptime`) |

*reply:* none

---
## MSP_FAILSAFE_CONFIG

id `75` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the failsafe configuration settings.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| failsafeDelay | `uint8` |  0.1s | Delay before failsafe stage 1 activates (`failsafeConfig()->failsafe_delay`) |
| failsafeOffDelay | `uint8` |  0.1s | Delay after signal recovery before returning control (`failsafeConfig()->failsafe_off_delay`) |
| failsafeThrottle | `uint16` |  PWM | Throttle level during failsafe stage 2 (`currentBatteryProfile->failsafe_throttle`) |
| legacyKillSwitch | `uint8` |   | Legacy flag, always 0 |
| failsafeThrottleLowDelay | `uint16` |  0.1s | Delay for throttle-based failsafe detection (`failsafeConfig()->failsafe_throttle_low_delay`). Units of 0.1 seconds. |
| failsafeProcedure | `uint8` | `failsafeProcedure_e`  | Enum `failsafeProcedure_e` Failsafe procedure (Drop, RTH, Land, etc.) ('failsafeConfig()->failsafe_procedure') |
| failsafeRecoveryDelay | `uint8` |  0.1s | Delay after RTH finishes before attempting recovery (`failsafeConfig()->failsafe_recovery_delay`) |
| failsafeFWRollAngle | `int16` |  deci-degrees | Fixed-wing failsafe roll angle (`failsafeConfig()->failsafe_fw_roll_angle`). Signed deci-degrees. |
| failsafeFWPitchAngle | `int16` |  deci-degrees | Fixed-wing failsafe pitch angle (`failsafeConfig()->failsafe_fw_pitch_angle`). Signed deci-degrees. |
| failsafeFWYawRate | `int16` |  deg/s | Fixed-wing failsafe yaw rate (`failsafeConfig()->failsafe_fw_yaw_rate`). Signed degrees per second. |
| failsafeStickThreshold | `uint16` |  PWM units | Stick movement threshold to exit failsafe (`failsafeConfig()->failsafe_stick_motion_threshold`) |
| failsafeMinDistance | `uint16` |  cm | Minimum distance from home for RTH failsafe (`failsafeConfig()->failsafe_min_distance`). Units of centimeters. |
| failsafeMinDistanceProc | `uint8` | `failsafeProcedure_e`  | Enum `failsafeProcedure_e` Failsafe procedure if below min distance ('failsafeConfig()->failsafe_min_distance_procedure') |

---
## MSP_SET_FAILSAFE_CONFIG

id `76` · MSPv1 · group `v1`

since INAV 1.0

Sets the failsafe configuration settings.

> Expects 20 bytes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| failsafeDelay | `uint8` |  0.1s | Sets `failsafeConfigMutable()->failsafe_delay`. |
| failsafeOffDelay | `uint8` |  0.1s | Sets `failsafeConfigMutable()->failsafe_off_delay`. |
| failsafeThrottle | `uint16` |  PWM | Sets `currentBatteryProfileMutable->failsafe_throttle`. |
| legacyKillSwitch | `uint8` |   | Ignored |
| failsafeThrottleLowDelay | `uint16` |  0.1s | Sets `failsafeConfigMutable()->failsafe_throttle_low_delay`. Units of 0.1 seconds. |
| failsafeProcedure | `uint8` | `failsafeProcedure_e`  | Enum `failsafeProcedure_e`. Sets `failsafeConfigMutable()->failsafe_procedure`. |
| failsafeRecoveryDelay | `uint8` |  0.1s | Sets `failsafeConfigMutable()->failsafe_recovery_delay`. |
| failsafeFWRollAngle | `int16` |  deci-degrees | Sets `failsafeConfigMutable()->failsafe_fw_roll_angle`. Signed deci-degrees. |
| failsafeFWPitchAngle | `int16` |  deci-degrees | Sets `failsafeConfigMutable()->failsafe_fw_pitch_angle`. Signed deci-degrees. |
| failsafeFWYawRate | `int16` |  deg/s | Sets `failsafeConfigMutable()->failsafe_fw_yaw_rate`. Signed degrees per second. |
| failsafeStickThreshold | `uint16` |  PWM units | Sets `failsafeConfigMutable()->failsafe_stick_motion_threshold`. |
| failsafeMinDistance | `uint16` |  cm | Sets `failsafeConfigMutable()->failsafe_min_distance`. Units of centimeters. |
| failsafeMinDistanceProc | `uint8` | `failsafeProcedure_e`  | Enum `failsafeProcedure_e`. Sets `failsafeConfigMutable()->failsafe_min_distance_procedure`. |

*reply:* none

---
## MSP_SDCARD_SUMMARY

id `79` · MSPv1 · group `v1`

since INAV 1.0

Retrieves summary information about the SD card status and filesystem.

> Requires `USE_SDCARD` and `USE_ASYNCFATFS`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| sdCardSupported | `uint8` | `bitmask`  | Bitmask: Bit 0 = 1 if SD card support compiled in (`USE_SDCARD`) |
| sdCardState | `uint8` | `mspSDCardState_e`  | Enum (`mspSDCardState_e`): Current state (Not Present, Fatal, Card Init, FS Init, Ready). 0 if `USE_SDCARD` disabled |
| fsError | `uint8` |   | Last filesystem error code (`afatfs_getLastError()`). 0 if `USE_SDCARD` disabled |
| freeSpaceKB | `uint32` |   | Free space in KiB (`afatfs_getContiguousFreeSpace() / 1024`). 0 if `USE_SDCARD` disabled |
| totalSpaceKB | `uint32` |   | Total space in KiB (`sdcard_getMetadata()->numBlocks / 2`). 0 if `USE_SDCARD` disabled |

---
## MSP_BLACKBOX_CONFIG

id `80` · MSPv1 · group `v1`

since INAV 1.0

Legacy command to retrieve Blackbox configuration. Superseded by `MSP2_BLACKBOX_CONFIG`.

> Returns fixed zero values. Use `MSP2_BLACKBOX_CONFIG`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| blackboxDevice | `uint8` |   | Always 0 (API no longer supported) |
| blackboxRateNum | `uint8` |   | Always 0 |
| blackboxRateDenom | `uint8` |   | Always 0 |
| blackboxPDenom | `uint8` |   | Always 0 |

---
## MSP_SET_BLACKBOX_CONFIG

id `81` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

Legacy command to set Blackbox configuration. Superseded by `MSP2_SET_BLACKBOX_CONFIG`.

> Not implemented in `fc_msp.c`. Use `MSP2_SET_BLACKBOX_CONFIG`.

*request:* none

*reply:* none

---
## MSP_TRANSPONDER_CONFIG

id `82` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

Get VTX Transponder settings (likely specific to RaceFlight/Betaflight, not standard INAV VTX).

> Not implemented in INAV `fc_msp.c`.

*request:* none

*reply:* none

---
## MSP_SET_TRANSPONDER_CONFIG

id `83` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

Set VTX Transponder settings.

> Not implemented in INAV `fc_msp.c`.

*request:* none

*reply:* none

---
## MSP_OSD_CONFIG

id `84` · MSPv1 · group `v1`

since INAV 1.0

Retrieves OSD configuration settings and layout for screen 0. Coordinates are packed as `(Y << 8) | X`. When `USE_OSD` is not compiled in, only `osdDriverType` = `OSD_DRIVER_NONE` is returned.

> 1 byte if `USE_OSD` disabled; full payload (1 + fields + 2*OSD_ITEM_COUNT bytes) otherwise.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| osdDriverType | `uint8` | `osdDriver_e`  | Enum `osdDriver_e`: `OSD_DRIVER_MAX7456` if `USE_OSD`, else `OSD_DRIVER_NONE`. |
| videoSystem | `optional uint8` | `videoSystem_e`  | Enum `videoSystem_e`: Video system (Auto/PAL/NTSC) (`osdConfig()->video_system`). Absent when `USE_OSD` is not compiled in |
| units | `optional uint8` | `osd_unit_e`  | Enum `osd_unit_e` Measurement units (Metric/Imperial) (`osdConfig()->units`). Absent when `USE_OSD` is not compiled in |
| rssiAlarm | `optional uint8` |  % | RSSI alarm threshold (`osdConfig()->rssi_alarm`). Absent when `USE_OSD` is not compiled in |
| capAlarm | `optional uint16` |  mAh/mWh | Capacity alarm threshold (`currentBatteryProfile->capacity.warning`). Truncated to 16 bits. Absent when `USE_OSD` is not compiled in. |
| timerAlarm | `optional uint16` |  minutes | Timer alarm threshold in minutes (`osdConfig()->time_alarm`). Absent when `USE_OSD` is not compiled in. |
| altAlarm | `optional uint16` |  meters | Altitude alarm threshold (`osdConfig()->alt_alarm`). Absent when `USE_OSD` is not compiled in |
| distAlarm | `optional uint16` |  meters | Distance alarm threshold (`osdConfig()->dist_alarm`). Absent when `USE_OSD` is not compiled in |
| negAltAlarm | `optional uint16` |  meters | Negative altitude alarm threshold (`osdConfig()->neg_alt_alarm`). Absent when `USE_OSD` is not compiled in |
| itemPositions | `optional uint16[OSD_ITEM_COUNT]` |  packed | Packed X/Y position for each OSD item on screen 0 (`osdLayoutsConfig()->item_pos[0][i]`). Absent when `USE_OSD` is not compiled in |

---
## MSP_SET_OSD_CONFIG

id `85` · MSPv1 · group `v1`

since INAV 1.0

Sets OSD configuration or a single item's position on screen 0.

> Requires `USE_OSD`. Distinguishes formats based on the first byte. Format 1 requires at least 10 bytes. Format 2 requires 3 bytes. Triggers an OSD redraw. See `MSP2_INAV_OSD_SET_*` for more advanced control.

**variant: dataSize >= 10**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| selector | `uint8` |   | Must be 0xFF (-1) to indicate a configuration update. |
| videoSystem | `uint8` | `videoSystem_e`  | Enum `videoSystem_e`: Video system (Auto/PAL/NTSC) (`osdConfig()->video_system`). |
| units | `uint8` | `osd_unit_e`  | Enum `osd_unit_e` Measurement units (Metric/Imperial) (`osdConfig()->units`). |
| rssiAlarm | `uint8` |  % | RSSI alarm threshold (`osdConfig()->rssi_alarm`). |
| capAlarm | `uint16` |  mAh/mWh | Capacity alarm threshold (`currentBatteryProfile->capacity.warning`). Truncated to 16 bits. |
| timerAlarm | `uint16` |  minutes | Timer alarm threshold in minutes (`osdConfig()->time_alarm`). |
| altAlarm | `uint16` |  meters | Altitude alarm threshold (`osdConfig()->alt_alarm`). |
| distAlarm | `optional uint16` |  meters | Distance alarm threshold (`osdConfig()->dist_alarm`). Optional trailing field. |
| negAltAlarm | `optional uint16` |  meters | Negative altitude alarm threshold (`osdConfig()->neg_alt_alarm`). Optional trailing field. |

*reply:* none

**variant: dataSize == 3**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| itemIndex | `uint8` |  Index | Index of the OSD item to update (0 to `OSD_ITEM_COUNT - 1`). |
| itemPosition | `uint16` |  packed | Packed X/Y position (`(Y << 8) \| X`) for the specified item. |

*reply:* none

---
## MSP_OSD_CHAR_READ

id `86` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

Reads character data from the OSD font memory.

> Not implemented in INAV `fc_msp.c`. Requires direct hardware access, typically done via DisplayPort.

*request:* none

*reply:* none

---
## MSP_OSD_CHAR_WRITE

id `87` · MSPv1 · group `v1`

since INAV 1.0

Writes character data to the OSD font memory.

> Requires `USE_OSD`. Minimum payload is `OSD_CHAR_VISIBLE_BYTES + 1` (8-bit address + 54 bytes). Payload size determines the address width and whether the extra metadata bytes are present. Writes characters via `displayWriteFontCharacter()`.

**variant: payloadSize >= OSD_CHAR_BYTES + 2 (>=66 bytes)**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| address | `uint16` |   | Character slot index (0-1023). |
| charData | `uint8[OSD_CHAR_BYTES]` |   | All 64 bytes, including driver metadata. |

*reply:* none

**variant: payloadSize == OSD_CHAR_BYTES + 1 (65 bytes)**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| address | `uint8` |   | Character slot index (0-255). |
| charData | `uint8[OSD_CHAR_BYTES]` |   | All 64 bytes, including driver metadata. |

*reply:* none

**variant: payloadSize == OSD_CHAR_VISIBLE_BYTES + 2 (56 bytes)**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| address | `uint16` |   | Character slot index (0-1023). |
| charData | `uint8[OSD_CHAR_VISIBLE_BYTES]` |   | Visible pixel data only (no metadata). |

*reply:* none

**variant: payloadSize == OSD_CHAR_VISIBLE_BYTES + 1 (55 bytes)**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| address | `uint8` |   | Character slot index (0-255). |
| charData | `uint8[OSD_CHAR_VISIBLE_BYTES]` |   | Visible pixel data only (no metadata). |

*reply:* none

---
## MSP_VTX_CONFIG

id `88` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the current VTX (Video Transmitter) configuration and capabilities.

> Returns 1 byte (`VTXDEV_UNKNOWN`) when no VTX is detected or `USE_VTX_CONTROL` is disabled; otherwise sends full payload. BF compatibility field `frequency` (uint16) is missing compared to some BF versions. Use `MSP_VTXTABLE_BAND` and `MSP_VTXTABLE_POWERLEVEL` for details.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| vtxDeviceType | `uint8` | `vtxDevType_e`  | Enum (`vtxDevType_e`): Type of VTX device detected/configured. `VTXDEV_UNKNOWN` if none |
| band | `optional uint8` |   | VTX band number (from `vtxSettingsConfig`) |
| channel | `optional uint8` |   | VTX channel number (from `vtxSettingsConfig`) |
| power | `optional uint8` |   | VTX power level index (from `vtxSettingsConfig()`). |
| pitMode | `optional uint8` |   | Boolean: 1 if VTX is currently in pit mode, 0 otherwise. |
| vtxReady | `optional uint8` |   | Boolean: 1 if VTX device reported ready, 0 otherwise |
| lowPowerDisarm | `optional uint8` | `vtxLowerPowerDisarm_e`  | Enum `vtxLowerPowerDisarm_e`: Low-power behaviour while disarmed (`vtxSettingsConfig()->lowPowerDisarm`). |
| vtxTableAvailable | `optional uint8` |   | Boolean: 1 if VTX tables (band/power) are available for query |
| bandCount | `optional uint8` |   | Number of bands supported by the VTX device |
| channelCount | `optional uint8` |   | Number of channels per band supported by the VTX device |
| powerCount | `optional uint8` |   | Number of power levels supported by the VTX device |
| minPowerIndex | `optional uint8` |   | Lowest selectable power index; 0 for `VTXDEV_MSP`, otherwise 1. |

---
## MSP_SET_VTX_CONFIG

id `89` · MSPv1 · group `v1`

since INAV 1.0

Sets VTX band/channel and related options. Fields are a progressive superset based on payload length.

> Requires dataSize >= 2. If no VTX device or device type is VTXDEV_UNKNOWN, fields are read and discarded. The first uint16 is interpreted as band/channel when value <= VTXCOMMON_MSP_BANDCHAN_CHKVAL, otherwise treated as a frequency value that is not applied by this path. Subsequent fields are applied only if present. If dataSize < 2 the command returns MSP_RESULT_ERROR.

**variant: payloadSize >= 14**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| bandChanOrFreq | `uint16` |   | Encoded band/channel if <= `VTXCOMMON_MSP_BANDCHAN_CHKVAL`; otherwise frequency placeholder. |
| power | `uint8` | | |
| pitMode | `uint8` | | |
| lowPowerDisarm | `uint8` | `vtxLowerPowerDisarm_e`  |  |
| pitModeFreq | `uint16` | | |
| band | `uint8` | | |
| channel | `uint8` | | |
| frequency | `uint16` | | |
| bandCount | `uint8` |   | Read and ignored. |
| channelCount | `uint8` |   | Read and ignored. |
| powerCount | `uint8` |   | If 0 < value < current capability, caps `vtxDevice->capability.powerCount`. |

*reply:* none

**variant: payloadSize >= 11**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| bandChanOrFreq | `uint16` | | |
| power | `uint8` | | |
| pitMode | `uint8` | | |
| lowPowerDisarm | `uint8` | `vtxLowerPowerDisarm_e`  |  |
| pitModeFreq | `uint16` | | |
| band | `uint8` | | |
| channel | `uint8` | | |
| frequency | `uint16` |   | Read and ignored by INAV. |

*reply:* none

**variant: payloadSize >= 9**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| bandChanOrFreq | `uint16` | | |
| power | `uint8` | | |
| pitMode | `uint8` | | |
| lowPowerDisarm | `uint8` | `vtxLowerPowerDisarm_e`  |  |
| pitModeFreq | `uint16` | | |
| band | `uint8` |   | 1..N; overrides band when present. |
| channel | `uint8` |   | 1..8; overrides channel when present. |

*reply:* none

**variant: payloadSize >= 7**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| bandChanOrFreq | `uint16` | | |
| power | `uint8` | | |
| pitMode | `uint8` | | |
| lowPowerDisarm | `uint8` | `vtxLowerPowerDisarm_e`  |  |
| pitModeFreq | `uint16` |   | Read and skipped. |

*reply:* none

**variant: payloadSize >= 5**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| bandChanOrFreq | `uint16` | | |
| power | `uint8` | | |
| pitMode | `uint8` | | |
| lowPowerDisarm | `uint8` | `vtxLowerPowerDisarm_e`  | 0=Off, 1=Always, 2=Until first arm. |

*reply:* none

**variant: payloadSize >= 4**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| bandChanOrFreq | `uint16` | | |
| power | `uint8` | | |
| pitMode | `uint8` | | |

*reply:* none

**variant: payloadSize == 2**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| bandChanOrFreq | `uint16` |   | If <= `VTXCOMMON_MSP_BANDCHAN_CHKVAL`, decoded as band/channel; otherwise treated as a frequency placeholder. |

*reply:* none

---
## MSP_ADVANCED_CONFIG

id `90` · MSPv1 · group `v1`

since INAV 1.0

Retrieves advanced hardware-related configuration (PWM protocols, rates). Some fields are BF compatibility placeholders.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| gyroSyncDenom | `uint8` |   | Always 1 (BF compatibility) |
| pidProcessDenom | `uint8` |   | Always 1 (BF compatibility) |
| useUnsyncedPwm | `uint8` |   | Always 1 (BF compatibility, INAV uses async PWM based on protocol) |
| motorPwmProtocol | `uint8` | `motorPwmProtocolTypes_e`  | Motor PWM protocol type (`motorConfig()->motorPwmProtocol`). |
| motorPwmRate | `uint16` |  Hz | Motor PWM rate (if applicable) (`motorConfig()->motorPwmRate`). |
| servoPwmRate | `uint16` |  Hz | Servo PWM rate (`servoConfig()->servoPwmRate`). |
| legacyGyroSync | `uint8` |   | Always 0 (BF compatibility) |

---
## MSP_SET_ADVANCED_CONFIG

id `91` · MSPv1 · group `v1`

since INAV 1.0

Sets advanced hardware-related configuration (PWM protocols, rates).

> Expects 9 bytes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| gyroSyncDenom | `uint8` |   | Ignored (legacy Betaflight field). |
| pidProcessDenom | `uint8` |   | Ignored (legacy Betaflight field). |
| useUnsyncedPwm | `uint8` |   | Ignored (legacy Betaflight field). |
| motorPwmProtocol | `uint8` | `motorPwmProtocolTypes_e`  | Sets `motorConfigMutable()->motorPwmProtocol`. |
| motorPwmRate | `uint16` |  Hz | Sets `motorConfigMutable()->motorPwmRate`. |
| servoPwmRate | `uint16` |  Hz | Sets `servoConfigMutable()->servoPwmRate`. |
| legacyGyroSync | `uint8` |   | Ignored (legacy Betaflight field). |

*reply:* none

---
## MSP_FILTER_CONFIG

id `92` · MSPv1 · group `v1`

since INAV 1.0

Retrieves filter configuration settings (Gyro, D-term, Yaw, Accel). Some fields are BF compatibility placeholders or legacy.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| gyroMainLpfHz | `uint8` |  Hz | Gyro main low-pass filter cutoff frequency (`gyroConfig()->gyro_main_lpf_hz`) |
| dtermLpfHz | `uint16` |  Hz | D-term low-pass filter cutoff frequency (`pidProfile()->dterm_lpf_hz`) |
| yawLpfHz | `uint16` |  Hz | Yaw low-pass filter cutoff frequency (`pidProfile()->yaw_lpf_hz`) |
| legacyGyroNotchHz | `uint16` |   | Always 0 (Legacy) |
| legacyGyroNotchCutoff | `uint16` |   | Always 1 (Legacy) |
| bfCompatDtermNotchHz | `uint16` |   | Always 0 (BF compatibility) |
| bfCompatDtermNotchCutoff | `uint16` |   | Always 1 (BF compatibility) |
| bfCompatGyroNotch2Hz | `uint16` |   | Always 0 (BF compatibility) |
| bfCompatGyroNotch2Cutoff | `uint16` |   | Always 1 (BF compatibility) |
| accNotchHz | `uint16` |  Hz | Accelerometer notch filter center frequency (`accelerometerConfig()->acc_notch_hz`) |
| accNotchCutoff | `uint16` |  Hz | Accelerometer notch filter cutoff frequency (`accelerometerConfig()->acc_notch_cutoff`) |
| legacyGyroStage2LpfHz | `uint16` |   | Always 0 (Legacy) |

---
## MSP_SET_FILTER_CONFIG

id `93` · MSPv1 · group `v1`

since INAV 1.0

Sets filter configuration settings. Handles different payload lengths for backward compatibility.

> Requires at least 22 bytes; intermediate length checks enforce legacy Betaflight frame layout and call `pidInitFilters()` once the D-term notch placeholders are consumed.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| gyroMainLpfHz | `uint8` |  Hz | Sets `gyroConfigMutable()->gyro_main_lpf_hz`. (Size >= 5) |
| dtermLpfHz | `uint16` |  Hz | Sets `pidProfileMutable()->dterm_lpf_hz` (constrained 0-500). (Size >= 5) |
| yawLpfHz | `uint16` |  Hz | Sets `pidProfileMutable()->yaw_lpf_hz` (constrained 0-255). (Size >= 5) |
| legacyGyroNotchHz | `uint16` |   | Ignored. (Size >= 9) |
| legacyGyroNotchCutoff | `uint16` |   | Ignored. (Size >= 9) |
| bfCompatDtermNotchHz | `uint16` |   | Ignored. (Size >= 13) |
| bfCompatDtermNotchCutoff | `uint16` |   | Ignored. (Size >= 13) |
| bfCompatGyroNotch2Hz | `uint16` |   | Ignored. (Size >= 17) |
| bfCompatGyroNotch2Cutoff | `uint16` |   | Ignored. (Size >= 17) |
| accNotchHz | `uint16` |  Hz | Sets `accelerometerConfigMutable()->acc_notch_hz` (constrained 0-255). (Size >= 21) |
| accNotchCutoff | `uint16` |  Hz | Sets `accelerometerConfigMutable()->acc_notch_cutoff` (constrained 1-255). (Size >= 21) |
| legacyGyroStage2LpfHz | `uint16` |   | Ignored. (Size >= 22) |

*reply:* none

---
## MSP_PID_ADVANCED

id `94` · MSPv1 · group `v1`

since INAV 1.0

Retrieves advanced PID tuning parameters. Many fields are BF compatibility placeholders.

> Acceleration limits are scaled by 10 for compatibility.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| legacyRollPitchItermIgnore | `uint16` |   | Always 0 (Legacy) |
| legacyYawItermIgnore | `uint16` |   | Always 0 (Legacy) |
| legacyYawPLimit | `uint16` |   | Always 0 (Legacy) |
| bfCompatDeltaMethod | `uint8` |   | Always 0 (BF compatibility) |
| bfCompatVbatPidComp | `uint8` |   | Always 0 (BF compatibility) |
| bfCompatSetpointRelaxRatio | `uint8` |   | Always 0 (BF compatibility) |
| reserved1 | `uint8` |   | Always 0 |
| legacyPidSumLimit | `uint16` |   | Always 0 (Legacy) |
| bfCompatItermThrottleGain | `uint8` |   | Always 0 (BF compatibility) |
| accelLimitRollPitch | `uint16` |  dps / 10 | Axis acceleration limit for Roll/Pitch / 10 (`pidProfile()->axisAccelerationLimitRollPitch / 10`) |
| accelLimitYaw | `uint16` |  dps / 10 | Axis acceleration limit for Yaw / 10 (`pidProfile()->axisAccelerationLimitYaw / 10`) |

---
## MSP_SET_PID_ADVANCED

id `95` · MSPv1 · group `v1`

since INAV 1.0

Sets advanced PID tuning parameters.

> Expects 17 bytes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| legacyRollPitchItermIgnore | `uint16` |   | Ignored (legacy compatibility). |
| legacyYawItermIgnore | `uint16` |   | Ignored (legacy compatibility). |
| legacyYawPLimit | `uint16` |   | Ignored (legacy compatibility). |
| bfCompatDeltaMethod | `uint8` |   | Ignored (BF compatibility). |
| bfCompatVbatPidComp | `uint8` |   | Ignored (BF compatibility). |
| bfCompatSetpointRelaxRatio | `uint8` |   | Ignored (BF compatibility). |
| reserved1 | `uint8` |   | Ignored (reserved). |
| legacyPidSumLimit | `uint16` |   | Ignored (legacy compatibility). |
| bfCompatItermThrottleGain | `uint8` |   | Ignored (BF compatibility). |
| accelLimitRollPitch | `uint16` |  dps / 10 | Sets `pidProfileMutable()->axisAccelerationLimitRollPitch = value * 10`. |
| accelLimitYaw | `uint16` |  dps / 10 | Sets `pidProfileMutable()->axisAccelerationLimitYaw = value * 10`. |

*reply:* none

---
## MSP_SENSOR_CONFIG

id `96` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the configured hardware type for various sensors.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| accHardware | `uint8` | `accelerationSensor_e`  | Enum (`accelerationSensor_e`): Accelerometer hardware type (`accelerometerConfig()->acc_hardware`) |
| baroHardware | `uint8` | `baroSensor_e`  | Enum (`baroSensor_e`): Barometer hardware type (`barometerConfig()->baro_hardware`). 0 if `USE_BARO` disabled |
| magHardware | `uint8` | `magSensor_e`  | Enum (`magSensor_e`): Magnetometer hardware type (`compassConfig()->mag_hardware`). 0 if `USE_MAG` disabled |
| pitotHardware | `uint8` | `pitotSensor_e`  | Enum (`pitotSensor_e`): Pitot tube hardware type (`pitotmeterConfig()->pitot_hardware`). 0 if `USE_PITOT` disabled |
| rangefinderHardware | `uint8` | `rangefinderType_e`  | Enum (`rangefinderType_e`): Rangefinder hardware type (`rangefinderConfig()->rangefinder_hardware`). 0 if `USE_RANGEFINDER` disabled |
| opflowHardware | `uint8` | `opticalFlowSensor_e`  | Enum (`opticalFlowSensor_e`): Optical flow hardware type (`opticalFlowConfig()->opflow_hardware`). 0 if `USE_OPFLOW` disabled |

---
## MSP_SET_SENSOR_CONFIG

id `97` · MSPv1 · group `v1`

since INAV 1.0

Sets the configured hardware type for various sensors.

> Expects 6 bytes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| accHardware | `uint8` | `accelerationSensor_e`  | Sets `accelerometerConfigMutable()->acc_hardware` |
| baroHardware | `uint8` | `baroSensor_e`  | Sets `barometerConfigMutable()->baro_hardware` (if `USE_BARO`) |
| magHardware | `uint8` | `magSensor_e`  | Sets `compassConfigMutable()->mag_hardware` (if `USE_MAG`) |
| pitotHardware | `uint8` | `pitotSensor_e`  | Sets `pitotmeterConfigMutable()->pitot_hardware` (if `USE_PITOT`) |
| rangefinderHardware | `uint8` | `rangefinderType_e`  | Sets `rangefinderConfigMutable()->rangefinder_hardware` (if `USE_RANGEFINDER`) |
| opflowHardware | `uint8` | `opticalFlowSensor_e`  | Sets `opticalFlowConfigMutable()->opflow_hardware` (if `USE_OPFLOW`) |

*reply:* none

---
## MSP_SPECIAL_PARAMETERS

id `98` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

Betaflight specific

> Not implemented in INAV `fc_msp.c`.

*request:* none

*reply:* none

---
## MSP_SET_SPECIAL_PARAMETERS

id `99` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

Betaflight specific

> Not implemented in INAV `fc_msp.c`.

*request:* none

*reply:* none

---
## MSP_STATUS

id `101` · MSPv1 · group `v1`

since INAV 1.0

Provides basic flight controller status including cycle time, errors, sensor status, active modes (first 32), and the current configuration profile.

> Superseded by `MSP_STATUS_EX` and `MSP2_INAV_STATUS`. `sensorStatus` bitmask: (Bit 0: ACC, 1: BARO, 2: MAG, 3: GPS, 4: RANGEFINDER, 5: OPFLOW, 6: PITOT, 7: TEMP; Bit 15: hardware failure). `activeModesLow` only contains the first 32 modes; use `MSP_ACTIVEBOXES` for the full set.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| cycleTime | `uint16` |  µs | Main loop cycle time (`cycleTime`) |
| i2cErrors | `uint16` |  Count | Number of I2C errors encountered (`i2cGetErrorCounter()`). 0 if `USE_I2C` not defined |
| sensorStatus | `uint16` | `sensors_e (bitmask)`  | Bitmask: available/active sensors (`packSensorStatus()`). See notes |
| activeModesLow | `uint32` | `bitmask`  | Bitmask: First 32 bits of the active flight modes bitmask (`packBoxModeFlags()`) |
| profile | `uint8` |  Index | Current configuration profile index (0-based) (`getConfigProfile()`) |

---
## MSP_RAW_IMU

id `102` · MSPv1 · group `v1`

since INAV 1.0

Provides raw sensor readings from the IMU (Accelerometer, Gyroscope, Magnetometer).

> Acc scaling is approximate (512 LSB/G). Mag units depend on the sensor.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| accX | `int16` |  ~1/512 G | Raw accelerometer X reading, scaled (`acc.accADCf[X] * 512`) |
| accY | `int16` |  ~1/512 G | Raw accelerometer Y reading, scaled (`acc.accADCf[Y] * 512`) |
| accZ | `int16` |  ~1/512 G | Raw accelerometer Z reading, scaled (`acc.accADCf[Z] * 512`) |
| gyroX | `int16` |  deg/s | Gyroscope X-axis rate (`gyroRateDps(X)`) |
| gyroY | `int16` |  deg/s | Gyroscope Y-axis rate (`gyroRateDps(Y)`) |
| gyroZ | `int16` |  deg/s | Gyroscope Z-axis rate (`gyroRateDps(Z)`) |
| magX | `int16` |  Raw units | Raw magnetometer X reading (`mag.magADC[X]`). 0 if `USE_MAG` disabled |
| magY | `int16` |  Raw units | Raw magnetometer Y reading (`mag.magADC[Y]`). 0 if `USE_MAG` disabled |
| magZ | `int16` |  Raw units | Raw magnetometer Z reading (`mag.magADC[Z]`). 0 if `USE_MAG` disabled |

---
## MSP_SERVO

id `103` · MSPv1 · group `v1`

since INAV 1.0

Provides the current output values for all supported servos.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| servoOutputs | `int16[MAX_SUPPORTED_SERVOS]` |  PWM | Array of current servo output values (typically 1000-2000) |

---
## MSP_MOTOR

id `104` · MSPv1 · group `v1`

since INAV 1.0

Provides the current output values for the first 8 motors.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| motorOutputs | `int16[8]` |  PWM | Array of current motor output values (typically 1000-2000). Values beyond `MAX_SUPPORTED_MOTORS` are 0 |

---
## MSP_RC

id `105` · MSPv1 · group `v1`

since INAV 1.0

Provides the current values of the received RC channels.

> Array length equals `rxRuntimeConfig.channelCount`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rcChannels | `int16[]` |  PWM | Array of current RC channel values (typically 1000-2000). Length depends on detected channels |

---
## MSP_RAW_GPS

id `106` · MSPv1 · group `v1`

since INAV 1.0

Provides raw GPS data (fix status, coordinates, altitude, speed, course).

> Only available if `USE_GPS` is defined. Altitude is truncated to meters.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| fixType | `uint8` | `gpsFixType_e`  | Enum `gpsFixType_e` GPS fix type (`gpsSol.fixType`) |
| numSat | `uint8` |  Count | Number of satellites used in solution (`gpsSol.numSat`) |
| latitude | `int32` |  deg * 1e7 | Latitude (`gpsSol.llh.lat`) |
| longitude | `int32` |  deg * 1e7 | Longitude (`gpsSol.llh.lon`) |
| altitude | `int16` |  m | Altitude above MSL, sent as whole metres (`gpsSol.llh.alt / 100`) |
| speed | `int16` |  cm/s | Ground speed (`gpsSol.groundSpeed`) |
| groundCourse | `int16` |  deci-degrees | Ground course (`gpsSol.groundCourse`) |
| hdop | `uint16` |  HDOP * 100 | Horizontal Dilution of Precision (`gpsSol.hdop`) |

---
## MSP_COMP_GPS

id `107` · MSPv1 · group `v1`

since INAV 1.0

Provides computed GPS values: distance and direction to home.

> Only available if `USE_GPS` is defined.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| distanceToHome | `uint16` |  meters | Distance to the home point (`GPS_distanceToHome`) |
| directionToHome | `int16` |  degrees | Direction to the home point (0-360) (`GPS_directionToHome`) |
| gpsHeartbeat | `uint8` |  Boolean | Indicates if GPS data is being received (`gpsSol.flags.gpsHeartbeat`) |

---
## MSP_ATTITUDE

id `108` · MSPv1 · group `v1`

since INAV 1.0

Provides the current attitude estimate (roll, pitch, yaw).

> Yaw is in degrees.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| roll | `int16` |  deci-degrees | Roll angle (`attitude.values.roll`) |
| pitch | `int16` |  deci-degrees | Pitch angle (`attitude.values.pitch`) |
| yaw | `int16` |  degrees | Yaw/Heading angle (`DECIDEGREES_TO_DEGREES(attitude.values.yaw)`) |

---
## MSP_ALTITUDE

id `109` · MSPv1 · group `v1`

since INAV 1.0

Provides estimated altitude, vertical speed (variometer), and raw barometric altitude.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| estimatedAltitude | `int32` |  cm | Estimated altitude above home/sea level (`getEstimatedActualPosition(Z)`) |
| variometer | `int16` |  cm/s | Estimated vertical speed (`getEstimatedActualVelocity(Z)`) |
| baroAltitude | `int32` |  cm | Latest raw altitude from barometer (`baroGetLatestAltitude()`). 0 if `USE_BARO` disabled |

---
## MSP_ANALOG

id `110` · MSPv1 · group `v1`

since INAV 1.0

Provides analog sensor readings: battery voltage, current consumption (mAh), RSSI, and current draw (Amps).

> Superseded by `MSP2_INAV_ANALOG` which provides higher precision and more fields.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| vbat | `uint8` |  0.1V | Battery voltage, scaled (`getBatteryVoltage() / 10`), constrained 0-255 |
| mAhDrawn | `uint16` |  mAh | Consumed battery capacity (`getMAhDrawn()`), constrained 0-65535 |
| rssi | `uint16` |  0-1023 or % | Received Signal Strength Indicator (`getRSSI()`). Units depend on source |
| amperage | `int16` |  0.01A | Current draw (`getAmperage()`), constrained -32768 to 32767 |

---
## MSP_RC_TUNING

id `111` · MSPv1 · group `v1`

since INAV 1.0

Retrieves RC tuning parameters (rates, expos, TPA) for the current control rate profile.

> Superseded by `MSP2_INAV_RATE_PROFILE` which includes manual rates/expos.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| legacyRcRate | `uint8` |   | Always 100 (Legacy, unused) |
| rcExpo | `uint8` |   | Roll/Pitch RC Expo (`currentControlRateProfile->stabilized.rcExpo8`) |
| rollRate | `uint8` |   | Roll Rate (`currentControlRateProfile->stabilized.rates[FD_ROLL]`) |
| pitchRate | `uint8` |   | Pitch Rate (`currentControlRateProfile->stabilized.rates[FD_PITCH]`) |
| yawRate | `uint8` |   | Yaw Rate (`currentControlRateProfile->stabilized.rates[FD_YAW]`) |
| dynamicThrottlePID | `uint8` |   | Dynamic Throttle PID (TPA) value (`currentControlRateProfile->throttle.dynPID`) |
| throttleMid | `uint8` |   | Throttle Midpoint (`currentControlRateProfile->throttle.rcMid8`) |
| throttleExpo | `uint8` |   | Throttle Expo (`currentControlRateProfile->throttle.rcExpo8`) |
| tpaBreakpoint | `uint16` |   | Throttle PID Attenuation (TPA) breakpoint (`currentControlRateProfile->throttle.tpa_breakpoint`) |
| rcYawExpo | `uint8` |   | Yaw RC Expo (`currentControlRateProfile->stabilized.rcYawExpo8`) |

---
## MSP_ACTIVEBOXES

id `113` · MSPv1 · group `v1`

since INAV 1.0

Provides the full bitmask of currently active flight modes (boxes).

> Use this instead of `MSP_STATUS` or `MSP_STATUS_EX` if more than 32 modes are possible.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| activeModes | `boxBitmask_t` | `bitmask`  | Bitmask: all active modes (`packBoxModeFlags()`). Size depends on `boxBitmask_t` definition |

---
## MSP_MISC

id `114` · MSPv1 · group `v1`

since INAV 1.0

Retrieves miscellaneous configuration settings, mostly related to RC, GPS, Mag, and Battery voltage (legacy formats).

> Superseded by `MSP2_INAV_MISC` and other specific commands which offer better precision and more fields.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| midRc | `uint16` |  PWM | Mid RC value (`PWM_RANGE_MIDDLE`, typically 1500) |
| legacyMinThrottle | `uint16` |   | Always 0 (Legacy) |
| maxThrottle | `uint16` |  PWM | Maximum throttle command (`getMaxThrottle()`) |
| minCommand | `uint16` |  PWM | Minimum motor command when disarmed (`motorConfig()->mincommand`) |
| failsafeThrottle | `uint16` |  PWM | Failsafe throttle level (`currentBatteryProfile->failsafe_throttle`) |
| gpsType | `uint8` | `gpsProvider_e`  | Enum `gpsProvider_e` GPS provider type (`gpsConfig()->provider`). 0 if `USE_GPS` disabled |
| legacyGpsBaud | `uint8` |   | Always 0 (Legacy) |
| gpsSbasMode | `uint8` | `sbasMode_e`  | Enum `sbasMode_e` GPS SBAS mode (`gpsConfig()->sbasMode`). 0 if `USE_GPS` disabled |
| legacyMwCurrentOut | `uint8` |   | Always 0 (Legacy) |
| rssiChannel | `uint8` |  Index | RSSI channel index (1-based) (`rxConfig()->rssi_channel`) |
| reserved1 | `uint8` |   | Always 0 |
| magDeclination | `uint16` |  0.1 degrees | Magnetic declination / 10 (`compassConfig()->mag_declination / 10`). 0 if `USE_MAG` disabled |
| vbatScale | `uint8` |  Scale / 10 | Voltage scale / 10 (`batteryMetersConfig()->voltage.scale / 10`). 0 if `USE_ADC` disabled |
| vbatMinCell | `uint8` |  0.1V | Min cell voltage / 10 (`currentBatteryProfile->voltage.cellMin / 10`). 0 if `USE_ADC` disabled |
| vbatMaxCell | `uint8` |  0.1V | Max cell voltage / 10 (`currentBatteryProfile->voltage.cellMax / 10`). 0 if `USE_ADC` disabled |
| vbatWarningCell | `uint8` |  0.1V | Warning cell voltage / 10 (`currentBatteryProfile->voltage.cellWarning / 10`). 0 if `USE_ADC` disabled |

---
## MSP_BOXNAMES

id `116` · MSPv1 · group `v1`

since INAV 1.0

Provides a semicolon-separated string containing the names of all available flight modes (boxes).

> The exact set of names depends on compiled features and configuration. Due to the size of the payload, it is recommended that [`MSP_BOXIDS`](#msp_boxids-119--0x77) is used instead.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| boxNamesString | `char[]` |   | String containing mode names separated by ';'. Null termination not guaranteed by MSP, relies on payload size. (`serializeBoxNamesReply()`) |

---
## MSP_PIDNAMES

id `117` · MSPv1 · group `v1`

since INAV 1.0

Provides a semicolon-separated string containing the names of the PID controllers.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| pidNamesString | `char[]` |   | String "ROLL;PITCH;YAW;ALT;Pos;PosR;NavR;LEVEL;MAG;VEL;". Null termination not guaranteed by MSP |

---
## MSP_WP

id `118` · MSPv1 · group `v1`

since INAV 1.0

Get/Set a single waypoint from the mission plan.

> See `navWaypoint_t` and `navWaypointActions_e`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| waypointIndex | `uint8` |   | Index of the waypoint to retrieve (0 to `NAV_MAX_WAYPOINTS - 1`) |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| waypointIndex | `uint8` |  Index | Index of the returned waypoint |
| action | `uint8` | `navWaypointActions_e`  | Enum `navWaypointActions_e` Waypoint action type |
| latitude | `int32` |  deg * 1e7 | Latitude coordinate |
| longitude | `int32` |  deg * 1e7 | Longitude coordinate |
| altitude | `int32` |  cm | Altitude coordinate (relative to home or sea level, see flag) |
| param1 | `int16` |  Varies | Parameter 1 (meaning depends on action) |
| param2 | `int16` |  Varies | Parameter 2 (meaning depends on action) |
| param3 | `int16` |  Varies | Parameter 3 (meaning depends on action) |
| flag | `uint8` | `bitmask`  | Bitmask: Waypoint flags (`NAV_WP_FLAG_*`) |

---
## MSP_BOXIDS

id `119` · MSPv1 · group `v1`

since INAV 1.0

Provides a list of permanent IDs associated with the available flight modes (boxes).

> Useful for mapping mode range configurations (`MSP_MODE_RANGES`) back to user-understandable modes via `MSP_BOXNAMES`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| boxIds | `uint8[]` |   | Array of permanent IDs for each configured box (`serializeBoxReply()`). Length depends on number of boxes |

---
## MSP_SERVO_CONFIGURATIONS

id `120` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the configuration parameters for all supported servos (min, max, middle, rate). Legacy format with unused fields.

> Superseded by `MSP2_INAV_SERVO_CONFIG` which has a cleaner structure.

*request:* none

*reply:* (repeat: MAX_SUPPORTED_SERVOS)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| min | `int16` |  PWM | Minimum servo endpoint (`servoParams(i)->min`) |
| max | `int16` |  PWM | Maximum servo endpoint (`servoParams(i)->max`) |
| middle | `int16` |  PWM | Middle/Neutral servo position (`servoParams(i)->middle`) |
| rate | `int8` |  % (-100 to 100) | Servo rate/scaling (`servoParams(i)->rate`, -125..125). Encoded as two's complement |
| reserved1 | `uint8` |   | Always 0 |
| reserved2 | `uint8` |   | Always 0 |
| legacyForwardChan | `uint8` |   | Always 255 (Legacy) |
| legacyReversedSources | `uint32` |   | Always 0 (Legacy) |

---
## MSP_NAV_STATUS

id `121` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the current status of the navigation system.

> Requires `USE_GPS`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| navMode | `uint8` | `navSystemStatus_Mode_e`  | Enum (`navSystemStatus_Mode_e`): Current navigation mode (None, RTH, NAV, Hold, etc.) (`NAV_Status.mode`) |
| navState | `uint8` | `navSystemStatus_State_e`  | Enum (`navSystemStatus_State_e`): Current navigation state (`NAV_Status.state`) |
| activeWpAction | `uint8` | `navWaypointActions_e`  | Enum (`navWaypointActions_e`): Action of the currently executing waypoint (`NAV_Status.activeWpAction`) |
| activeWpNumber | `uint8` |   | Index: Index of the currently executing waypoint (`NAV_Status.activeWpNumber`) |
| navError | `uint8` | `navSystemStatus_Error_e`  | Enum (`navSystemStatus_Error_e`): Current navigation error code (`NAV_Status.error`) |
| targetHeading | `int16` |  degrees | Target heading for heading controller (`getHeadingHoldTarget()`) |
| desiredHeading | `uint16` |  centi-degrees | Guidance course/track the navigation controller is steering to (`navDesiredHeading`, `wrap_36000()` of the desired yaw) |

---
## MSP_NAV_CONFIG

id `122` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

*request:* none

*reply:* none

---
## MSP_3D

id `124` · MSPv1 · group `v1`

since INAV 1.0

Retrieves settings related to 3D/reversible motor operation.

> Requires reversible motor support.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| deadbandLow | `uint16` |  PWM | Lower deadband limit for 3D mode (`reversibleMotorsConfig()->deadband_low`) |
| deadbandHigh | `uint16` |  PWM | Upper deadband limit for 3D mode (`reversibleMotorsConfig()->deadband_high`) |
| neutral | `uint16` |  PWM | Neutral throttle point for 3D mode (`reversibleMotorsConfig()->neutral`) |

---
## MSP_RC_DEADBAND

id `125` · MSPv1 · group `v1`

since INAV 1.0

Retrieves RC input deadband settings.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| deadband | `uint8` |  PWM | General RC deadband for Roll/Pitch (`rcControlsConfig()->deadband`) |
| yawDeadband | `uint8` |  PWM | Specific deadband for Yaw (`rcControlsConfig()->yaw_deadband`) |
| altHoldDeadband | `uint8` |  PWM | Deadband for altitude hold adjustments (`rcControlsConfig()->alt_hold_deadband`) |
| throttleDeadband | `uint16` |  PWM | Deadband around throttle mid-stick (`rcControlsConfig()->mid_throttle_deadband`) |

---
## MSP_SENSOR_ALIGNMENT

id `126` · MSPv1 · group `v1`

since INAV 1.0

Retrieves sensor alignment settings (legacy format).

> Board alignment is now typically handled by `MSP_BOARD_ALIGNMENT`. This returns legacy enum values where applicable.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| gyroAlign | `uint8` |   | Always 0 (Legacy alignment enum) |
| accAlign | `uint8` |   | Always 0 (Legacy alignment enum) |
| magAlign | `uint8` |   | Magnetometer alignment (`compassConfig()->mag_align`). 0 if `USE_MAG` disabled |
| opflowAlign | `uint8` |   | Optical flow alignment (`opticalFlowConfig()->opflow_align`). 0 if `USE_OPFLOW` disabled |

---
## MSP_LED_STRIP_MODECOLOR

id `127` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the color index assigned to each LED mode and function/direction combination, including special colors.

> Only available if `USE_LED_STRIP` is defined. Entries where `modeIndex == LED_MODE_COUNT` describe special colors.

*request:* none

*reply:* (repeat: 51)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| modeIndex | `uint8` | `ledModeIndex_e`  | Index of the LED mode Enum (`ledModeIndex_e`). `LED_MODE_COUNT` for special colors |
| directionOrSpecialIndex | `uint8` |   | Index of the direction (`ledDirectionId_e`) or special color (`ledSpecialColorIds_e`) |
| colorIndex | `uint8` |   | Index of the color assigned from `ledStripConfig()->colors` |

---
## MSP_BATTERY_STATE

id `130` · MSPv1 · group `v1`

since INAV 1.0

Provides battery state information, formatted primarily for DJI FPV Goggles compatibility.

> Only available if `USE_DJI_HD_OSD` or `USE_MSP_DISPLAYPORT` is defined. Some values are duplicated from `MSP_ANALOG` / `MSP2_INAV_ANALOG` but potentially with different scaling/types.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| cellCount | `uint8` |  Count | Number of battery cells (`getBatteryCellCount()`) |
| capacity | `uint16` |  mAh | Battery capacity (`currentBatteryProfile->capacity.value`) |
| vbatScaled | `uint8` |  0.1V | Battery voltage / 10 (`getBatteryVoltage() / 10`) |
| mAhDrawn | `uint16` |  mAh | Consumed capacity (`getMAhDrawn()`) |
| amperage | `int16` |  0.01A | Current draw (`getAmperage()`) |
| batteryState | `uint8` | `batteryState_e`  | Enum `batteryState_e` Current battery state (`getBatteryState()`, see `BATTERY_STATE_*`) |
| vbatActual | `uint16` |  0.01V | Actual battery voltage (`getBatteryVoltage()`) |

---
## MSP_VTXTABLE_BAND

id `137` · MSPv1 · group `v1`

since INAV 7.0

Retrieves information about a specific VTX band from the VTX table. (Implementation missing in provided `fc_msp.c`)

> The ID is defined, but no handler exists in the provided C code. Likely intended to query band names and frequencies.

*request:* none

*reply:* none

---
## MSP_VTXTABLE_POWERLEVEL

id `138` · MSPv1 · group `v1`

since INAV 7.0

Retrieves information about a specific VTX power level from the VTX table.

> Requires `USE_VTX_CONTROL`. Returns error if index is out of bounds. The `powerValue` field is unused.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| powerLevelIndex | `uint8` |   | 1-based index of the power level to query |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| powerLevelIndex | `uint8` |   | 1-based index of the returned power level |
| powerValue | `uint16` |   | Always 0 (Actual power value in mW is not stored/returned via MSP) |
| labelLength | `uint8` |   | Length of the power level label string that follows |
| label | `char[]` |   | Power level label string (e.g., "25", "200"). Length given by previous field |

---
## MSP_STATUS_EX

id `150` · MSPv1 · group `v1`

since INAV 1.0

Provides extended flight controller status, including CPU load, arming flags, and calibration status, in addition to `MSP_STATUS` fields.

> Superseded by `MSP2_INAV_STATUS` which provides the full 32-bit `armingFlags` and other enhancements. The `accCalibAxisFlags` field is not present in `MSP2_INAV_STATUS` but is available via `MSP_CALIBRATION_DATA`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| cycleTime | `uint16` |  µs | Main loop cycle time |
| i2cErrors | `uint16` |  Count | I2C errors |
| sensorStatus | `uint16` | `sensors_e (bitmask)`  | Bitmask: Sensor status |
| activeModesLow | `uint32` | `bitmask`  | Bitmask: First 32 active modes |
| profile | `uint8` |  Index | Current config profile index |
| cpuLoad | `uint16` |  % | Average system load percentage (`averageSystemLoadPercent`) |
| armingFlags | `uint16` | `armingFlag_e (bitmask)`  | Bitmask: Flight controller arming flags (`armingFlags`). Note: Truncated to 16 bits |
| accCalibAxisFlags | `uint8` | `bitmask`  | Bitmask: Accelerometer calibrated axes flags (`accGetCalibrationAxisFlags()`) |

---
## MSP_SENSOR_STATUS

id `151` · MSPv1 · group `v1`

since INAV 1.0

Provides the hardware status for each individual sensor system.

> Status values map to the `hardwareSensorStatus_e` enum: `HW_SENSOR_NONE`, `HW_SENSOR_OK`, `HW_SENSOR_UNAVAILABLE`, `HW_SENSOR_UNHEALTHY`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| overallHealth | `uint8` |  Boolean | 1 if all essential hardware is healthy, 0 otherwise (`isHardwareHealthy()`) |
| gyroStatus | `uint8` | `hardwareSensorStatus_e`  | Enum `hardwareSensorStatus_e` Gyro hardware status (`getHwGyroStatus()`) |
| accStatus | `uint8` | `hardwareSensorStatus_e`  | Enum `hardwareSensorStatus_e` Accelerometer hardware status (`getHwAccelerometerStatus()`) |
| magStatus | `uint8` | `hardwareSensorStatus_e`  | Enum `hardwareSensorStatus_e` Compass hardware status (`getHwCompassStatus()`) |
| baroStatus | `uint8` | `hardwareSensorStatus_e`  | Enum `hardwareSensorStatus_e` Barometer hardware status (`getHwBarometerStatus()`) |
| gpsStatus | `uint8` | `hardwareSensorStatus_e`  | Enum `hardwareSensorStatus_e` GPS hardware status (`getHwGPSStatus()`) |
| rangefinderStatus | `uint8` | `hardwareSensorStatus_e`  | Enum `hardwareSensorStatus_e` Rangefinder hardware status (`getHwRangefinderStatus()`) |
| pitotStatus | `uint8` | `hardwareSensorStatus_e`  | Enum `hardwareSensorStatus_e` Pitot hardware status (`getHwPitotmeterStatus()`) |
| opflowStatus | `uint8` | `hardwareSensorStatus_e`  | Enum `hardwareSensorStatus_e` Optical Flow hardware status (`getHwOpticalFlowStatus()`) |

---
## MSP_UID

id `160` · MSPv1 · group `v1`

since INAV 1.0

Provides the unique identifier of the microcontroller.

> Total 12 bytes, representing a 96-bit unique ID.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| uid0 | `uint32` |   | First 32 bits of the unique ID (`U_ID_0`) |
| uid1 | `uint32` |   | Middle 32 bits of the unique ID (`U_ID_1`) |
| uid2 | `uint32` |   | Last 32 bits of the unique ID (`U_ID_2`) |

---
## MSP_GPSSVINFO

id `164` · MSPv1 · group `v1`

since INAV 1.0

Provides satellite signal strength information (legacy U-Blox compatibility stub).

> Requires `USE_GPS`. This is just a stub in INAV and does not provide actual per-satellite signal info. HDOP digits are not formatted correctly: tens and units both contain `gpsSol.hdop / 100`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| protocolVersion | `uint8` |   | Always 1 (Stub version) |
| numChannels | `uint8` |   | Always 0 (Number of SV info channels reported) |
| hdopHundredsDigit | `uint8` |   | Hundreds digit of HDOP (stub always writes 0) |
| hdopTensDigit | `uint8` |   | Tens digit of HDOP (`gpsSol.hdop / 100`, truncated) |
| hdopUnitsDigit | `uint8` |   | Units digit of HDOP (`gpsSol.hdop / 100`, duplicated by stub) |

---
## MSP_GPSSTATISTICS

id `166` · MSPv1 · group `v1`

since INAV 1.0

Provides debugging statistics for the GPS communication link.

> Requires `USE_GPS`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| lastMessageDt | `uint16` |  ms | Time since last valid GPS message (`gpsStats.lastMessageDt`) |
| errors | `uint32` |  Count | Number of GPS communication errors (`gpsStats.errors`) |
| timeouts | `uint32` |  Count | Number of GPS communication timeouts (`gpsStats.timeouts`) |
| packetCount | `uint32` |  Count | Number of valid GPS packets received (`gpsStats.packetCount`) |
| hdop | `uint16` |  HDOP * 100 | Horizontal Dilution of Precision (`gpsSol.hdop`) |
| eph | `uint16` |  cm | Estimated Horizontal Position Accuracy (`gpsSol.eph`) |
| epv | `uint16` |  cm | Estimated Vertical Position Accuracy (`gpsSol.epv`) |
| hwVersion | `uint8` |   | GPS hardware version bit-field: bits[7:6]=series (0b01=u-blox Neo/M), bits[5:0]=generation. E.g. 0x48=M8, 0x49=M9, 0x4A=M10, 0=unknown. |

---
## MSP_OSD_VIDEO_CONFIG

id `180` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

*request:* none

*reply:* none

---
## MSP_SET_OSD_VIDEO_CONFIG

id `181` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

*request:* none

*reply:* none

---
## MSP_DISPLAYPORT

id `182` · MSPv1 · group `v1`

since INAV 1.0

Drives an external MSP DisplayPort OSD (DJI, HDZero, Walksnail). Sent by the flight controller to the display device rather than requested from it, so it carries a reply payload with no request and expects no response.

> Requires an MSP DisplayPort OSD device. Sub-commands are emitted by `io/displayport_msp_osd.c`; `MSP_DP_OPTIONS` is reserved and unused by INAV.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| subCommand | `uint8` | `displayportMspCommand_e`  | DisplayPort sub-command (`displayportMspCommand_e` in `io/displayport_msp.h`) |
| subCommandData | `uint8[]` |   | Sub-command payload. Empty for `MSP_DP_HEARTBEAT`, `MSP_DP_RELEASE`, `MSP_DP_CLEAR_SCREEN` and `MSP_DP_DRAW_SCREEN`. For `MSP_DP_WRITE_STRING`: row, column, attributes (font page in bits 0-1, blink in bit 3), then the character bytes. |

---
## MSP_SET_TX_INFO

id `186` · MSPv1 · group `v1`

since INAV 1.0

Allows a transmitter LUA script (or similar) to send runtime information (currently only RSSI) to the firmware.

> Calls `setRSSIFromMSP()`. Expects 1 byte.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rssi | `uint8` |  Raw | RSSI value (0-255) provided by the external source; firmware scales it to 10-bit (`value << 2`) |

*reply:* none

---
## MSP_TX_INFO

id `187` · MSPv1 · group `v1`

since INAV 1.0

Provides information potentially useful for transmitter LUA scripts.

> See `rssiSource_e`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rssiSource | `uint8` | `rssiSource_e`  | Enum: Source of the RSSI value (`getRSSISource()`, see `rssiSource_e`) |
| rtcDateTimeIsSet | `uint8` |   | Boolean: 1 if the RTC has been set, 0 otherwise |

---
## MSP_SET_RAW_RC

id `200` · MSPv1 · group `v1`

since INAV 1.0

Provides raw RC channel data to the flight controller, typically used when the receiver is connected via MSP (e.g., MSP RX feature).

> Requires `USE_RX_MSP`. Maximum channels `MAX_SUPPORTED_RC_CHANNEL_COUNT`. Calls `rxMspFrameReceive()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rcChannels | `uint16[]` |  PWM | Array of RC channel values (typically 1000-2000). Number of channels determined by payload size |

*reply:* none

---
## MSP_SET_RAW_GPS

id `201` · MSPv1 · group `v1`

since INAV 1.0

Provides raw GPS data to the flight controller, typically for simulation or external GPS injection.

> Requires `USE_GPS`. Expects 14 bytes. Updates `gpsSol` structure and calls `onNewGPSData()`. Note the altitude unit mismatch (meters in MSP, cm internal). Does not provide velocity components.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| fixType | `uint8` | `gpsFixType_e`  | Enum `gpsFixType_e` GPS fix type |
| numSat | `uint8` |  Count | Number of satellites |
| latitude | `int32` |  deg * 1e7 | Latitude |
| longitude | `int32` |  deg * 1e7 | Longitude |
| altitude | `uint16` |  m | Altitude in meters (converted to centimeters internally; limited to 0-65535 m) |
| speed | `uint16` |  cm/s | Ground speed (`gpsSol.groundSpeed`) |

*reply:* none

---
## MSP_SET_BOX

id `203` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

Sets the state of flight modes (boxes). (Likely unused/obsolete in INAV).

> Not implemented in INAV `fc_msp.c`. Mode changes are typically handled via RC channels (`MSP_MODE_RANGES`).

*request:* none

*reply:* none

---
## MSP_SET_RC_TUNING

id `204` · MSPv1 · group `v1`

since INAV 1.0

Sets RC tuning parameters (rates, expos, TPA) for the current control rate profile.

> Expects 10 or 11 bytes. Calls `schedulePidGainsUpdate()`. Superseded by `MSP2_INAV_SET_RATE_PROFILE`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| legacyRcRate | `uint8` |   | Ignored |
| rcExpo | `uint8` |   | Sets `currentControlRateProfile->stabilized.rcExpo8` |
| rollRate | `uint8` |   | Sets `currentControlRateProfile->stabilized.rates[FD_ROLL]` (constrained) |
| pitchRate | `uint8` |   | Sets `currentControlRateProfile->stabilized.rates[FD_PITCH]` (constrained) |
| yawRate | `uint8` |   | Sets `currentControlRateProfile->stabilized.rates[FD_YAW]` (constrained) |
| dynamicThrottlePID | `uint8` |   | Sets `currentControlRateProfile->throttle.dynPID` (constrained) |
| throttleMid | `uint8` |   | Sets `currentControlRateProfile->throttle.rcMid8` |
| throttleExpo | `uint8` |   | Sets `currentControlRateProfile->throttle.rcExpo8` |
| tpaBreakpoint | `uint16` |   | Sets `currentControlRateProfile->throttle.tpa_breakpoint` |
| rcYawExpo | `optional uint8` |   | (Optional) Sets `currentControlRateProfile->stabilized.rcYawExpo8` |

*reply:* none

---
## MSP_ACC_CALIBRATION

id `205` · MSPv1 · group `v1`

since INAV 1.0

Starts the accelerometer calibration procedure.

> Will fail if armed. Calls `accStartCalibration()`.

*request:* none

*reply:* none

---
## MSP_MAG_CALIBRATION

id `206` · MSPv1 · group `v1`

since INAV 1.0

Starts the magnetometer calibration procedure.

> Will fail if armed. Enables the `CALIBRATE_MAG` state flag.

*request:* none

*reply:* none

---
## MSP_SET_MISC

id `207` · MSPv1 · group `v1`

since INAV 1.0

Sets miscellaneous configuration settings (legacy formats/scaling).

> Expects 22 bytes. Superseded by `MSP2_INAV_SET_MISC`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| midRc | `uint16` |  PWM | Ignored |
| legacyMinThrottle | `uint16` |   | Ignored |
| legacyMaxThrottle | `uint16` |   | Ignored |
| minCommand | `uint16` |  PWM | Sets `motorConfigMutable()->mincommand` (constrained 0-PWM_RANGE_MAX) |
| failsafeThrottle | `uint16` |  PWM | Sets `currentBatteryProfileMutable->failsafe_throttle` (constrained PWM_RANGE_MIN/MAX) |
| gpsType | `uint8` | `gpsProvider_e`  | Enum `gpsProvider_e` (Sets `gpsConfigMutable()->provider`) |
| legacyGpsBaud | `uint8` |   | Ignored |
| gpsSbasMode | `uint8` | `sbasMode_e`  | Enum `sbasMode_e` (Sets `gpsConfigMutable()->sbasMode`) |
| legacyMwCurrentOut | `uint8` |   | Ignored |
| rssiChannel | `uint8` |  Index | Sets `rxConfigMutable()->rssi_channel` (constrained 0-MAX_SUPPORTED_RC_CHANNEL_COUNT). Updates source |
| reserved1 | `uint8` |   | Ignored |
| magDeclination | `uint16` |  0.1 degrees | Sets `compassConfigMutable()->mag_declination = value * 10` (if `USE_MAG`) |
| vbatScale | `uint8` |  Scale / 10 | Sets `batteryMetersConfigMutable()->voltage.scale = value * 10` (if `USE_ADC`) |
| vbatMinCell | `uint8` |  0.1V | Sets `currentBatteryProfileMutable->voltage.cellMin = value * 10` (if `USE_ADC`) |
| vbatMaxCell | `uint8` |  0.1V | Sets `currentBatteryProfileMutable->voltage.cellMax = value * 10` (if `USE_ADC`) |
| vbatWarningCell | `uint8` |  0.1V | Sets `currentBatteryProfileMutable->voltage.cellWarning = value * 10` (if `USE_ADC`) |

*reply:* none

---
## MSP_RESET_CONF

id `208` · MSPv1 · group `v1`

since INAV 1.0

Resets all configuration settings to their default values and saves to EEPROM.

> Will fail if armed. Suspends RX, calls `resetEEPROM()`, `writeEEPROM()`, `readEEPROM()`, resumes RX. Use with caution!

*request:* none

*reply:* none

---
## MSP_SET_WP

id `209` · MSPv1 · group `v1`

since INAV 1.0

Sets a single waypoint in the mission plan.

> Expects 21 bytes. Calls `setWaypoint()`. If `USE_FW_AUTOLAND` is enabled, this also interacts with autoland approach settings based on waypoint index and flags.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| waypointIndex | `uint8` |  Index | Index of the waypoint to set (0 to `NAV_MAX_WAYPOINTS - 1`) |
| action | `uint8` | `navWaypointActions_e`  | Enum `navWaypointActions_e` Waypoint action type |
| latitude | `int32` |  deg * 1e7 | Latitude coordinate |
| longitude | `int32` |  deg * 1e7 | Longitude coordinate |
| altitude | `int32` |  cm | Altitude coordinate |
| param1 | `uint16` |  Varies | Parameter 1 |
| param2 | `uint16` |  Varies | Parameter 2 |
| param3 | `uint16` |  Varies | Parameter 3 |
| flag | `uint8` | `navWaypointFlags_e (bitmask)`  | Bitmask: Waypoint flags (`navWaypointFlags_e`) |

*reply:* none

---
## MSP_SELECT_SETTING

id `210` · MSPv1 · group `v1`

since INAV 1.0

Selects the active configuration profile and saves it.

> Will fail if armed. Calls `setConfigProfileAndWriteEEPROM()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| profileIndex | `uint8` |   | Index of the profile to activate (0-based) |

*reply:* none

---
## MSP_SET_HEAD

id `211` · MSPv1 · group `v1`

since INAV 1.0

Sets the target heading for the heading hold controller (e.g., during MAG mode).

> Expects 2 bytes. Calls `updateHeadingHoldTarget()`. Also synchronizes navigation yaw targets (including cruise/course) when NAV is controlling yaw.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| heading | `uint16` |  degrees | Target heading (0-359) |

*reply:* none

---
## MSP_SET_SERVO_CONFIGURATION

id `212` · MSPv1 · group `v1`

since INAV 1.0

Sets the configuration for a single servo (legacy format).

> Expects 15 bytes. Returns error if index is invalid. Calls `servoComputeScalingFactors()`. Superseded by `MSP2_INAV_SET_SERVO_CONFIG`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| servoIndex | `uint8` |  Index | Index of the servo to configure (0 to `MAX_SUPPORTED_SERVOS - 1`) |
| min | `uint16` |  PWM | Minimum servo endpoint |
| max | `uint16` |  PWM | Maximum servo endpoint |
| middle | `uint16` |  PWM | Middle/Neutral servo position |
| rate | `uint8` |  % | Servo rate/scaling |
| reserved1 | `uint8` |   | Ignored |
| reserved2 | `uint8` |   | Ignored |
| legacyForwardChan | `uint8` |   | Ignored |
| legacyReversedSources | `uint32` |   | Ignored |

*reply:* none

---
## MSP_SET_MOTOR

id `214` · MSPv1 · group `v1`

since INAV 1.0

Sets the disarmed motor values, typically used for motor testing or propeller balancing functions in a configurator.

> Expects 16 bytes. Modifies the `motor_disarmed` array. These values are *not* saved persistently.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| motorValues | `uint16[8]` |  PWM | Array of motor values to set when disarmed. Only affects first `MAX_SUPPORTED_MOTORS` entries |

*reply:* none

---
## MSP_SET_NAV_CONFIG

id `215` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

*request:* none

*reply:* none

---
## MSP_SET_3D

id `217` · MSPv1 · group `v1`

since INAV 1.0

Sets parameters related to 3D/reversible motor operation.

> Expects 6 bytes. Requires reversible motor support.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| deadbandLow | `uint16` |  PWM | Sets `reversibleMotorsConfigMutable()->deadband_low` |
| deadbandHigh | `uint16` |  PWM | Sets `reversibleMotorsConfigMutable()->deadband_high` |
| neutral | `uint16` |  PWM | Sets `reversibleMotorsConfigMutable()->neutral` |

*reply:* none

---
## MSP_SET_RC_DEADBAND

id `218` · MSPv1 · group `v1`

since INAV 1.0

Sets RC input deadband values.

> Expects 5 bytes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| deadband | `uint8` |  PWM | Sets `rcControlsConfigMutable()->deadband` |
| yawDeadband | `uint8` |  PWM | Sets `rcControlsConfigMutable()->yaw_deadband` |
| altHoldDeadband | `uint8` |  PWM | Sets `rcControlsConfigMutable()->alt_hold_deadband` |
| throttleDeadband | `uint16` |  PWM | Sets `rcControlsConfigMutable()->mid_throttle_deadband` |

*reply:* none

---
## MSP_SET_RESET_CURR_PID

id `219` · MSPv1 · group `v1`

since INAV 1.0

Resets the PIDs of the *current* profile to their default values. Does not save.

> Calls `PG_RESET_CURRENT(pidProfile)`. To save, follow with `MSP_EEPROM_WRITE`.

*request:* none

*reply:* none

---
## MSP_SET_SENSOR_ALIGNMENT

id `220` · MSPv1 · group `v1`

since INAV 1.0

Sets sensor alignment (legacy format).

> Expects 4 bytes. Use `MSP_SET_BOARD_ALIGNMENT` for primary board orientation.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| gyroAlign | `uint8` |   | Ignored |
| accAlign | `uint8` |   | Ignored |
| magAlign | `uint8` |   | Sets `compassConfigMutable()->mag_align` (if `USE_MAG`) |
| opflowAlign | `uint8` |   | Sets `opticalFlowConfigMutable()->opflow_align` (if `USE_OPFLOW`) |

*reply:* none

---
## MSP_SET_LED_STRIP_MODECOLOR

id `221` · MSPv1 · group `v1`

since INAV 1.0

Sets the color index for a specific LED mode/function combination.

> Only available if `USE_LED_STRIP` is defined. Expects 3 bytes. Returns error if setting fails (invalid index).

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| modeIndex | `uint8` | `ledModeIndex_e`  | Index of the LED mode (`ledModeIndex_e` or `LED_MODE_COUNT` for special) |
| directionOrSpecialIndex | `uint8` |   | Index of the direction (`ledDirectionId_e`) or special color (`ledSpecialColorIds_e`) |
| colorIndex | `uint8` |   | Index of the color to assign from `ledStripConfig()->colors` |

*reply:* none

---
## MSP_SET_ACC_TRIM

id `239` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

Sets the accelerometer trim values (leveling calibration).

> Not implemented in INAV `fc_msp.c`. Use `MSP_ACC_CALIBRATION`.

*request:* none

*reply:* none

---
## MSP_ACC_TRIM

id `240` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

Gets the accelerometer trim values.

> Not implemented in INAV `fc_msp.c`. Calibration data via `MSP_CALIBRATION_DATA`.

*request:* none

*reply:* none

---
## MSP_SERVO_MIX_RULES

id `241` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the custom servo mixer rules (legacy format).

> Superseded by `MSP2_INAV_SERVO_MIXER`.

*request:* none

*reply:* (repeat: MAX_SERVO_RULES)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| targetChannel | `uint8` |  Index | Servo output channel index (0-based) |
| inputSource | `uint8` | `inputSource_e`  | Enum `inputSource_e` Input source for the mix (RC chan, Roll, Pitch...) |
| rate | `int16` |  % | Mixing rate/weight (`-1000` to `+1000`, percent with sign) |
| speed | `uint8` |  0-255 | Speed/Slew rate limit (`0`=instant, higher slows response) |
| reserved1 | `uint8` |   | Always 0 |
| legacyMax | `uint8` |   | Always 100 (Legacy) |
| legacyBox | `uint8` |   | Always 0 (Legacy) |

---
## MSP_SET_SERVO_MIX_RULE

id `242` · MSPv1 · group `v1`

since INAV 1.0

Sets a single custom servo mixer rule (legacy format).

> Expects 9 bytes. Returns error if index invalid. Calls `loadCustomServoMixer()`. Superseded by `MSP2_INAV_SET_SERVO_MIXER`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| ruleIndex | `uint8` |  Index | Index of the rule to set (0 to `MAX_SERVO_RULES - 1`) |
| targetChannel | `uint8` |  Index | Servo output channel index |
| inputSource | `uint8` | `inputSource_e`  | Enum `inputSource_e` Input source for the mix |
| rate | `int16` |  % | Mixing rate/weight (`-1000` to `+1000`, percent with sign) |
| speed | `uint8` |  0-255 | Speed/Slew rate limit (`0`=instant, higher slows response) |
| legacyMinMax | `uint16` |   | Ignored |
| legacyBox | `uint8` |   | Ignored |

*reply:* none

---
## MSP_SET_PASSTHROUGH

id `245` · MSPv1 · group `v1`

since INAV 1.0

Enables serial passthrough mode to peripherals like ESCs (BLHeli 4-way) or other serial devices.

> Accepts 0 bytes (defaults to ESC 4-way) or up to 2 bytes for mode/argument. If successful, sets `mspPostProcessFn` to the appropriate handler (`mspSerialPassthroughFn` or `esc4wayProcess`). This handler takes over the serial port after the reply is sent. Requires `USE_SERIAL_4WAY_BLHELI_INTERFACE` for ESC passthrough. ESC 4-way passthrough will fail if the craft is armed: the reply carries `MSP_RESULT_ERROR` and the serial port is not handed over. Serial passthrough to another device is unaffected.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| status | `uint8` |   | 1 if passthrough started successfully, 0 on error (e.g., port not found). For 4way, returns number of ESCs found |

---
## MSP_RTC

id `246` · MSPv1 · group `v1`

since INAV 1.0

Retrieves the current Real-Time Clock time.

> Requires RTC hardware/support. Returns (0, 0) if time is not available/set.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| seconds | `int32` |  Seconds | Seconds since epoch (or relative time if not set). 0 if RTC time unknown |
| millis | `uint16` |  Milliseconds | Millisecond part of the time. 0 if RTC time unknown |

---
## MSP_SET_RTC

id `247` · MSPv1 · group `v1`

since INAV 1.0

Sets the Real-Time Clock time.

> Requires RTC hardware/support. Expects 6 bytes. Uses `rtcSet()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| seconds | `int32` |  Seconds | Seconds component of time to set |
| millis | `uint16` |  Milliseconds | Millisecond component of time to set |

*reply:* none

---
## MSP_EEPROM_WRITE

id `250` · MSPv1 · group `v1`

since INAV 1.0

Saves the current configuration from RAM to non-volatile memory (EEPROM/Flash).

> Will fail if armed. Suspends RX, calls `writeEEPROM()`, `readEEPROM()`, resumes RX.

*request:* none

*reply:* none

---
## MSP_RESERVE_1

id `251` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

*request:* none

*reply:* none

---
## MSP_RESERVE_2

id `252` · MSPv1 · group `v1`  ·  *not implemented*

since INAV 1.0

*request:* none

*reply:* none

---
## MSP_DEBUGMSG

id `253` · MSPv1 · group `v1`

since INAV 1.0

Retrieves debug ("serial printf") messages from the firmware.

> Published via the LOG UART or shared MSP/LOG port using `mspSerialPushPort()`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| messageText | `cstring` |   | Debug message text (not NUL-terminated). See [serial printf debugging](https://github.com/iNavFlight/inav/blob/master/docs/development/serial_printf_debugging.md) |

---
## MSP_DEBUG

id `254` · MSPv1 · group `v1`

since INAV 1.0

Retrieves values from the firmware's `debug[]` array (legacy 16-bit version).

> Useful for developers. Values are truncated to the lower 16 bits of each `debug[]` entry. See `MSP2_INAV_DEBUG` for full 32-bit values.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| debugValues | `uint16[4]` |   | First 4 values from the `debug` array |

---
## MSP_V2_FRAME

id `255` · MSPv1 · group `v1`

since INAV 1.0

This ID is used as a *payload indicator* within an MSPv1 message structure (`$M>`) to signify that the following payload conforms to the MSPv2 format. It's not a command itself.

> See MSPv2 documentation for the actual frame structure that follows this indicator.

*request:* none

*reply:* none

---
## MSP2_COMMON_TZ

id `0x1001` (4097) · MSPv2 · group `common`

since INAV 1.0

Gets the time zone offset configuration.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| tzOffsetMinutes | `int16` |  Minutes | Time zone offset from UTC (`timeConfig()->tz_offset`) |
| tzAutoDst | `uint8` |  Boolean | Automatic daylight saving time enabled (`timeConfig()->tz_automatic_dst`) |

---
## MSP2_COMMON_SET_TZ

id `0x1002` (4098) · MSPv2 · group `common`

since INAV 1.0

Sets the time zone offset configuration.

> Accepts 2 or 3 bytes.

**variant: dataSize == 2**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| tz_offset | `int16` |  minutes | Timezone offset from UTC. |

*reply:* none

**variant: dataSize == 3**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| tz_offset | `int16` |  minutes | Timezone offset from UTC. |
| tz_automatic_dst | `uint8` |  bool | Automatic DST enable (0/1). |

*reply:* none

---
## MSP2_COMMON_SETTING

id `0x1003` (4099) · MSPv2 · group `common`

since INAV 1.0

Gets the value of a specific configuration setting, identified by name or index.

> Returns error if setting not found. Use `MSP2_COMMON_SETTING_INFO` to discover settings, types, and sizes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| settingIdentifier | `` |   | Setting name, or an index when the first byte is `0x00`. |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| settingValue | `uint8[]` |   | Raw byte value of the setting. Size depends on the setting's type (`settingGetValueSize()`) |

---
## MSP2_COMMON_SET_SETTING

id `0x1004` (4100) · MSPv2 · group `common`

since INAV 1.0

Sets the value of a specific configuration setting, identified by name or index.

> Performs type checking and range validation (min/max). Returns error if setting not found, value size mismatch, or value out of range. Handles different data types (`uint8`, `int16`, `float`, `string`, etc.) internally.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| settingIdentifier | `` |   | Setting name, or an index when the first byte is `0x00`. |
| settingValue | `uint8[]` |   | Raw byte value to set for the setting. Size must match the setting's type |

*reply:* none

---
## MSP2_COMMON_MOTOR_MIXER

id `0x1005` (4101) · MSPv2 · group `common`

since INAV 1.0

Retrieves the current motor mixer configuration (throttle, roll, pitch, yaw weights) for each motor.

> Scaling is `(float_weight + 2.0) * 1000`. `primaryMotorMixer()` provides the data. If multiple mixer profiles are enabled (`MAX_MIXER_PROFILE_COUNT > 1`), an additional block of mixes for the next profile follows immediately.

*request:* none

*reply:* (repeat: MAX_SUPPORTED_MOTORS)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| motorMix | `uint16[4]` |  Scaled (0-4000) | Weights for a single motor `[throttle, roll, pitch, yaw]`, each encoded as `(mix + 2.0) * 1000` (range 0-4000) |

---
## MSP2_COMMON_SET_MOTOR_MIXER

id `0x1006` (4102) · MSPv2 · group `common`

since INAV 1.0

Sets the motor mixer weights for a single motor in the primary mixer profile.

> Expects 9 bytes. Modifies `primaryMotorMixerMutable()`. Returns error if index is invalid.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| motorIndex | `uint8` |  Index | Index of the motor to configure (0 to `MAX_SUPPORTED_MOTORS - 1`) |
| throttleWeight | `uint16` |  Scaled (0-4000) | Sets throttle weight from `(value / 1000.0) - 2.0 |
| rollWeight | `uint16` |  Scaled (0-4000) | Sets roll weight from `(value / 1000.0) - 2.0 |
| pitchWeight | `uint16` |  Scaled (0-4000) | Sets pitch weight from `(value / 1000.0) - 2.0 |
| yawWeight | `uint16` |  Scaled (0-4000) | Sets yaw weight from `(value / 1000.0) - 2.0 |

*reply:* none

---
## MSP2_COMMON_SETTING_INFO

id `0x1007` (4103) · MSPv2 · group `common`

since INAV 1.0

Gets detailed information about a specific configuration setting (name, type, range, flags, current value, etc.).

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| settingName | `cstring` |   | Null-terminated setting name |
| pgn | `uint16` |   | Parameter Group Number (PGN) ID |
| type | `uint8` |   | Variable type (`VAR_UINT8`, `VAR_FLOAT`, etc.) |
| section | `uint8` |   | Setting section (`MASTER_VALUE`, `PROFILE_VALUE`, etc.) |
| mode | `uint8` |   | Setting mode (`MODE_NORMAL`, `MODE_LOOKUP`, etc.) |
| minValue | `int32` |   | Minimum allowed value (as signed 32-bit) |
| maxValue | `uint32` |   | Maximum allowed value (as unsigned 32-bit) |
| settingIndex | `uint16` |   | Absolute index of the setting |
| profileIndex | `uint8` |   | Current profile index (if applicable, else 0) |
| profileCount | `uint8` |   | Total number of profiles (if applicable, else 0) |
| lookupNames | `cstring` |   | (If `mode == MODE_LOOKUP`) Series of null-terminated strings for each possible value from min to max |
| settingValue | `uint8[]` |   | Current raw byte value of the setting |

---
## MSP2_COMMON_PG_LIST

id `0x1008` (4104) · MSPv2 · group `common`

since INAV 1.0

Gets a list of Parameter Group Numbers (PGNs) used by settings, along with the start and end setting indexes for each group. Can request info for a single PGN.

> Allows efficient fetching of related settings by group. Record count is not a constant: mspParameterGroupsCommand(): caller may request one PGN or the full PG_ID_FIRST..PG_ID_LAST range, skipping absent groups; read until the payload is exhausted.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| pgn | `optional uint16` |   | (Optional) PGN ID to query. If omitted, returns all used PGNs |

*reply:* (repeat: until_end)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| pgn | `uint16` |   | Parameter Group Number (PGN) ID |
| startIndex | `uint16` |   | Absolute index of the first setting in this group |
| endIndex | `uint16` |   | Absolute index of the last setting in this group |

---
## MSP2_COMMON_SERIAL_CONFIG

id `0x1009` (4105) · MSPv2 · group `common`

since INAV 1.0

Retrieves the configuration for all available serial ports.

> Baud rate indexes map to actual baud rates (e.g., 9600, 115200). See `baudRates` array. Record count is not a constant: loops SERIAL_PORT_COUNT, emitting only ports where serialIsPortAvailable(); read until the payload is exhausted.

*request:* none

*reply:* (repeat: until_end)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| identifier | `int8` | `serialPortIdentifier_e`  | Port identifier Enum (`serialPortIdentifier_e`) |
| functionMask | `uint32` | `serialPortFunction_e (bitmask)`  | Bitmask: enabled functions (`FUNCTION_*`) |
| mspBaudIndex | `uint8` |   | Baud rate index for MSP function |
| gpsBaudIndex | `uint8` |   | Baud rate index for GPS function |
| telemetryBaudIndex | `uint8` |   | Baud rate index for Telemetry function |
| peripheralBaudIndex | `uint8` |   | Baud rate index for other peripheral functions |

---
## MSP2_COMMON_SET_SERIAL_CONFIG

id `0x100A` (4106) · MSPv2 · group `common`

since INAV 1.0

Sets the configuration for one or more serial ports.

> Payload size must be a multiple of the size of one port config entry (1 + 4 + 4 = 9 bytes). Returns error if identifier is invalid or size is incorrect. Baud rate indexes are constrained `BAUD_MIN` to `BAUD_MAX`. Record count is not a constant: mirrors MSP2_COMMON_SERIAL_CONFIG; the sender chooses how many ports to configure; read until the payload is exhausted.

*request:* (repeat: until_end)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| identifier | `int8` | `serialPortIdentifier_e`  | Port identifier Enum (`serialPortIdentifier_e`) |
| functionMask | `uint32` | `serialPortFunction_e (bitmask)`  | Bitmask: functions to enable |
| mspBaudIndex | `uint8` |   | Baud rate index for MSP |
| gpsBaudIndex | `uint8` |   | Baud rate index for GPS |
| telemetryBaudIndex | `uint8` |   | Baud rate index for Telemetry |
| peripheralBaudIndex | `uint8` |   | Baud rate index for peripherals |

*reply:* none

---
## MSP2_COMMON_SET_RADAR_POS

id `0x100B` (4107) · MSPv2 · group `common`

since INAV 1.0

Sets the position and status information for a "radar" Point of Interest (POI). Used for displaying other craft/objects on the OSD map.

> Expects 19 bytes. POI index is clamped to `RADAR_MAX_POIS - 1`. Updates the `radar_pois` array.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| poiIndex | `uint8` |  Index | Index of the POI slot (0 to `RADAR_MAX_POIS - 1`) |
| state | `uint8` |   | Status of the POI (0=undefined, 1=armed, 2=lost) |
| latitude | `int32` |  deg * 1e7 | Latitude of the POI |
| longitude | `int32` |  deg * 1e7 | Longitude of the POI |
| altitude | `int32` |  cm | Altitude of the POI |
| heading | `uint16` |  degrees | Heading of the POI |
| speed | `uint16` |  cm/s | Speed of the POI |
| linkQuality | `uint8` |  0-4 | Link quality indicator |

*reply:* none

---
## MSP2_COMMON_SET_RADAR_ITD

id `0x100C` (4108) · MSPv2 · group `common`  ·  *not implemented*

since INAV 1.0

Sets radar information to display (likely internal/unused).

> Not implemented in INAV `fc_msp.c`.

*request:* none

*reply:* none

---
## MSP2_COMMON_SET_MSP_RC_LINK_STATS

id `0x100D` (4109) · MSPv2 · group `common`

since INAV 8.0

Provides RC link statistics (RSSI, LQ) to the FC, typically from an MSP-based RC link (like ExpressLRS). Sent periodically by the RC link.

> Requires `USE_RX_MSP`. Expects at least 7 bytes. Updates `rxLinkStatistics` and sets RSSI via `setRSSIFromMSP_RC()` only if `sublinkID` is 0. This message expects **no reply** (`MSP_RESULT_NO_REPLY`).

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| sublinkID | `uint8` |   | Sublink identifier (usually 0) |
| validLink | `uint8` |  Boolean | Indicates if the link is currently valid (not in failsafe) |
| rssiPercent | `uint8` |  % | Uplink RSSI percentage (0-100) |
| uplinkRSSI_dBm | `uint8` |  -dBm | Uplink RSSI in dBm (sent as positive, e.g., 70 means -70dBm) |
| downlinkLQ | `uint8` |  % | Downlink Link Quality (0-100) |
| uplinkLQ | `uint8` |  % | Uplink Link Quality (0-100) |
| uplinkSNR | `int8` |  dB | Uplink Signal-to-Noise Ratio |

*reply:* none

---
## MSP2_COMMON_SET_MSP_RC_INFO

id `0x100E` (4110) · MSPv2 · group `common`

since INAV 8.0

Provides additional RC link information (power levels, band, mode) to the FC from an MSP-based RC link. Sent less frequently than link stats.

> Requires `USE_RX_MSP`. Expects at least 15 bytes. Updates `rxLinkStatistics` only if `sublinkID` is 0. Converts band/mode strings to uppercase. This message expects **no reply** (`MSP_RESULT_NO_REPLY`).

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| sublinkID | `uint8` |   | Sublink identifier (usually 0) |
| uplinkTxPower | `uint16` |  mW | Uplink transmitter power level |
| downlinkTxPower | `uint16` |  mW | Downlink transmitter power level |
| band | `char[4]` |   | Operating band string (e.g., "2G4", "900"), null-terminated/padded |
| mode | `char[6]` |   | Operating mode/rate string (e.g., "100HZ", "F1000"), null-terminated/padded |

*reply:* none

---
## MSP2_COMMON_GET_RADAR_GPS

id `0x100F` (4111) · MSPv2 · group `common`

since INAV 9.0

Provides the GPS positions (latitude, longitude, altitude) for each radar point of interest.

> Returns the stored GPS coordinates for all radar POIs (`radar_pois[i].gps`).

*request:* none

*reply:* (repeat: RADAR_MAX_POIS)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| poiLatitude | `int32` |  deg * 1e7 | Latitude of a radar POI |
| poiLongitude | `int32` |  deg * 1e7 | Longitude of a radar POI |
| poiAltitude | `int32` |  cm | Altitude of a radar POI |

---
## MSP2_SENSOR_RANGEFINDER

id `0x1F01` (7937) · MSPv2 · group `sensor`

since INAV 1.0

Provides rangefinder data (distance, quality) from an external MSP-based sensor.

> Requires `USE_RANGEFINDER_MSP`. Calls `mspRangefinderReceiveNewData()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| quality | `uint8` |  0-255 | Quality of the measurement |
| distanceMm | `int32` |  mm | Measured distance. Negative value indicates out of range |

*reply:* none

---
## MSP2_SENSOR_OPTIC_FLOW

id `0x1F02` (7938) · MSPv2 · group `sensor`

since INAV 1.0

Provides optical flow data (motion, quality) from an external MSP-based sensor.

> Requires `USE_OPFLOW_MSP`. Calls `mspOpflowReceiveNewData()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| quality | `uint8` |   | Quality of the measurement (0-255) |
| motionX | `int32` |   | Raw integrated flow value X |
| motionY | `int32` |   | Raw integrated flow value Y |

*reply:* none

---
## MSP2_SENSOR_GPS

id `0x1F03` (7939) · MSPv2 · group `sensor`

since INAV 1.0

Provides detailed GPS data from an external MSP-based GPS module.

> Requires `USE_GPS_PROTO_MSP`. Calls `mspGPSReceiveNewData()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| instance | `uint8` |   | Sensor instance number (for multi-GPS) |
| gpsWeek | `uint16` |   | GPS week number (0xFFFF if unavailable) |
| msTOW | `uint32` |  ms | Milliseconds Time of Week |
| fixType | `uint8` | `gpsFixType_e`  | Enum `gpsFixType_e` Type of GPS fix |
| satellitesInView | `uint8` |  Count | Number of satellites used in solution |
| hPosAccuracy | `uint16` |  mm | Horizontal position accuracy estimate in milimeters |
| vPosAccuracy | `uint16` |  mm | Vertical position accuracy estimate in milimeters |
| hVelAccuracy | `uint16` |  cm/s | Horizontal velocity accuracy estimate |
| hdop | `uint16` |  HDOP * 100 | Horizontal Dilution of Precision |
| longitude | `int32` |  deg * 1e7 | Longitude |
| latitude | `int32` |  deg * 1e7 | Latitude |
| mslAltitude | `int32` |  cm | Altitude above Mean Sea Level |
| nedVelNorth | `int32` |  cm/s | North velocity (NED frame) |
| nedVelEast | `int32` |  cm/s | East velocity (NED frame) |
| nedVelDown | `int32` |  cm/s | Down velocity (NED frame) |
| groundCourse | `uint16` |  deg * 100 | Ground course (0-36000) |
| trueYaw | `uint16` |  deg * 100 | True heading/yaw (0-36000, 65535 if unavailable) |
| year | `uint16` |   | Year (e.g., 2023) |
| month | `uint8` |   | Month (1-12) |
| day | `uint8` |   | Day of month (1-31) |
| hour | `uint8` |   | Hour (0-23) |
| min | `uint8` |   | Minute (0-59) |
| sec | `uint8` |   | Second (0-59) |

*reply:* none

---
## MSP2_SENSOR_COMPASS

id `0x1F04` (7940) · MSPv2 · group `sensor`

since INAV 1.0

Provides magnetometer data from an external MSP-based compass module.

> Requires `USE_MAG_MSP`. Calls `mspMagReceiveNewData()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| instance | `uint8` |   | Sensor instance number |
| timeMs | `uint32` |  ms | Timestamp from the sensor |
| magX | `int16` |  mGauss | Front component reading |
| magY | `int16` |  mGauss | Right component reading |
| magZ | `int16` |  mGauss | Down component reading |

*reply:* none

---
## MSP2_SENSOR_BAROMETER

id `0x1F05` (7941) · MSPv2 · group `sensor`

since INAV 1.0

Provides barometer data from an external MSP-based barometer module.

> Requires `USE_BARO_MSP`. Calls `mspBaroReceiveNewData()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| instance | `uint8` |   | Sensor instance number |
| timeMs | `uint32` |  ms | Timestamp from the sensor |
| pressurePa | `float32` |  Pa | Absolute pressure |
| temp | `int16` |  0.01 deg C | Temperature |

*reply:* none

---
## MSP2_SENSOR_AIRSPEED

id `0x1F06` (7942) · MSPv2 · group `sensor`

since INAV 1.0

Provides airspeed data from an external MSP-based pitot sensor module.

> Requires `USE_PITOT_MSP`. Calls `mspPitotmeterReceiveNewData()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| instance | `uint8` |   | Sensor instance number |
| timeMs | `uint32` |  ms | Timestamp from the sensor |
| diffPressurePa | `float32` |  Pa | Differential pressure |
| temp | `int16` |  0.01 deg C | Temperature |

*reply:* none

---
## MSP2_SENSOR_HEADTRACKER

id `0x1F07` (7943) · MSPv2 · group `sensor`

since INAV 8.0

Provides head tracker orientation data.

> Requires `USE_HEADTRACKER` and `USE_HEADTRACKER_MSP`. Calls `mspHeadTrackerReceiverNewData()`, which rejects any payload whose size is not exactly `sizeof(headtrackerMspMessage_t)` (9 bytes). Layout matches `headtrackerMspMessage_t` in `io/headtracker_msp.h`. `pan`, `tilt` and `roll` are constrained to `HEADTRACKER_RANGE_MIN`..`HEADTRACKER_RANGE_MAX` on receipt.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| version | `uint8` |   | Message version. Currently 0. |
| pan | `int16` |   | -2048~2047. Scale is min/max angle for gimbal |
| tilt | `int16` |   | -2048~2047. Scale is min/max angle for gimbal |
| roll | `int16` |   | -2048~2047. Scale is min/max angle for gimbal |
| sensitivity | `int16` |   | -16~15. Scale is min/max angle for gimbal |

*reply:* none

---
## MSP2_INAV_STATUS

id `0x2000` (8192) · MSPv2 · group `inav`

since INAV 1.0

Provides comprehensive flight controller status, extending `MSP_STATUS_EX` with full arming flags, battery profile, and mixer profile.

> `sensorStatus` bits follow `packSensorStatus()` (bit 15 indicates hardware failure). `profileAndBattProfile` packs the current config profile in the low nibble and the battery profile in the high nibble. `activeModes` is emitted as a little-endian array of 32-bit words sized to `CHECKBOX_ITEM_COUNT`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| cycleTime | `uint16` |  µs | Main loop cycle time |
| i2cErrors | `uint16` |  Count | I2C errors |
| sensorStatus | `uint16` | `sensors_e (bitmask)`  | Bitmask: Sensor status |
| cpuLoad | `uint16` |  % | Average system load percentage |
| profileAndBattProfile | `uint8` |  Packed | Bits 0-3: Config profile index (`getConfigProfile()`), Bits 4-7: Battery profile index (`getConfigBatteryProfile()`) |
| armingFlags | `uint32` | `armingFlag_e (bitmask)`  | Bitmask: Full 32-bit flight controller arming flags (`armingFlags`) |
| activeModes | `boxBitmask_t` | `bitmask`  | Bitmask words for active flight modes (`packBoxModeFlags()`) |
| mixerProfile | `uint8` |  Index | Current mixer profile index (`getConfigMixerProfile()`) |

---
## MSP2_INAV_OPTICAL_FLOW

id `0x2001` (8193) · MSPv2 · group `inav`

since INAV 1.0

Provides data from the optical flow sensor.

> Requires `USE_OPFLOW`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| quality | `uint8` |  0-255 | Raw quality indicator from the sensor (`opflow.rawQuality`). 0 if `USE_OPFLOW` disabled |
| flowRateX | `int16` |  degrees/s | Optical flow rate X (roll axis) (`RADIANS_TO_DEGREES(opflow.flowRate[X])`). 0 if `USE_OPFLOW` disabled |
| flowRateY | `int16` |  degrees/s | Optical flow rate Y (pitch axis) (`RADIANS_TO_DEGREES(opflow.flowRate[Y])`). 0 if `USE_OPFLOW` disabled |
| bodyRateX | `int16` |  degrees/s | Compensated body rate X (roll axis) (`RADIANS_TO_DEGREES(opflow.bodyRate[X])`). 0 if `USE_OPFLOW` disabled |
| bodyRateY | `int16` |  degrees/s | Compensated body rate Y (pitch axis) (`RADIANS_TO_DEGREES(opflow.bodyRate[Y])`). 0 if `USE_OPFLOW` disabled |

---
## MSP2_INAV_ANALOG

id `0x2002` (8194) · MSPv2 · group `inav`

since INAV 1.0

Provides detailed analog sensor readings, superseding `MSP_ANALOG` with higher precision and additional fields.

> Requires `USE_CURRENT_METER`/`USE_ADC` for current-related fields; values fall back to zero when unavailable. Capacity fields are reported in the units configured by `batteryMetersConfig()->capacity_unit` (mAh or mWh).

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| batteryFlags | `uint8` | `bitmask`  | Bitmask: Bit0=Full on plug-in, Bit1=Use capacity thresholds, Bits2-3=`batteryState_e` (`getBatteryState()`), Bits4-7=Cell count (`getBatteryCellCount()`) |
| vbat | `uint16` |  0.01V | Battery voltage (`getBatteryVoltage()`) |
| amperage | `int16` |  0.01A | Current draw (`getAmperage()`) |
| powerDraw | `uint32` |  0.01W | Power draw (`getPower()`) |
| mAhDrawn | `uint32` |  mAh | Consumed capacity (`getMAhDrawn()`) |
| mWhDrawn | `uint32` |  mWh | Consumed energy (`getMWhDrawn()`) |
| remainingCapacity | `uint32` |  Capacity unit (`batteryMetersConfig()->capacity_unit`) | Estimated remaining capacity (`getBatteryRemainingCapacity()`) |
| percentageRemaining | `uint8` |  % | Estimated remaining capacity percentage (`calculateBatteryPercentage()`) |
| rssi | `uint16` |  Raw (0-1023) | RSSI value (`getRSSI()`) |

---
## MSP2_INAV_MISC

id `0x2003` (8195) · MSPv2 · group `inav`

since INAV 1.0

Retrieves miscellaneous configuration settings, superseding `MSP_MISC` with higher precision and capacity fields.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| midRc | `uint16` |  PWM | Mid RC value (`PWM_RANGE_MIDDLE`) |
| legacyMinThrottle | `uint16` |   | Always 0 (Legacy) |
| maxThrottle | `uint16` |  PWM | Maximum throttle command (`getMaxThrottle()`) |
| minCommand | `uint16` |  PWM | Minimum motor command (`motorConfig()->mincommand`) |
| failsafeThrottle | `uint16` |  PWM | Failsafe throttle level (`currentBatteryProfile->failsafe_throttle`) |
| gpsType | `uint8` | `gpsProvider_e`  | Enum `gpsProvider_e` GPS provider type (`gpsConfig()->provider`). 0 if `USE_GPS` disabled |
| legacyGpsBaud | `uint8` |   | Always 0 (Legacy) |
| gpsSbasMode | `uint8` | `sbasMode_e`  | Enum `sbasMode_e` GPS SBAS mode (`gpsConfig()->sbasMode`). 0 if `USE_GPS` disabled |
| rssiChannel | `uint8` |  Index | RSSI channel index (1-based, 0 disables) (`rxConfig()->rssi_channel`) |
| magDeclination | `int16` |  0.1 degrees | Magnetic declination / 10 (`compassConfig()->mag_declination / 10`). 0 if `USE_MAG` disabled |
| vbatScale | `uint16` |  Scale | Voltage scale (`batteryMetersConfig()->voltage.scale`). 0 if `USE_ADC` disabled |
| vbatSource | `uint8` | `batVoltageSource_e`  | Enum `batVoltageSource_e` Voltage source (`batteryMetersConfig()->voltageSource`). 0 if `USE_ADC` disabled |
| cellCount | `uint8` |  Count | Configured cell count (`currentBatteryProfile->cells`). 0 if `USE_ADC` disabled |
| vbatCellDetect | `uint16` |  0.01V | Cell detection voltage (`currentBatteryProfile->voltage.cellDetect`). 0 if `USE_ADC` disabled |
| vbatMinCell | `uint16` |  0.01V | Min cell voltage (`currentBatteryProfile->voltage.cellMin`). 0 if `USE_ADC` disabled |
| vbatMaxCell | `uint16` |  0.01V | Max cell voltage (`currentBatteryProfile->voltage.cellMax`). 0 if `USE_ADC` disabled |
| vbatWarningCell | `uint16` |  0.01V | Warning cell voltage (`currentBatteryProfile->voltage.cellWarning`). 0 if `USE_ADC` disabled |
| capacityValue | `uint32` |  mAh/mWh | Battery capacity (`currentBatteryProfile->capacity.value`) |
| capacityWarning | `uint32` |  mAh/mWh | Capacity warning threshold (`currentBatteryProfile->capacity.warning`) |
| capacityCritical | `uint32` |  mAh/mWh | Capacity critical threshold (`currentBatteryProfile->capacity.critical`) |
| capacityUnit | `uint8` | `batCapacityUnit_e`  | Enum `batCapacityUnit_e` Capacity unit (`batteryMetersConfig()->capacity_unit`) |

---
## MSP2_INAV_SET_MISC

id `0x2004` (8196) · MSPv2 · group `inav`

since INAV 1.0

Sets miscellaneous configuration settings, superseding `MSP_SET_MISC`.

> Expects 41 bytes. Performs validation on `vbatSource` and `capacityUnit`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| midRc | `uint16` |  PWM | Ignored |
| legacyMinThrottle | `uint16` |   | Ignored |
| legacyMaxThrottle | `uint16` |   | Ignored |
| minCommand | `uint16` |  PWM | Sets `motorConfigMutable()->mincommand` (constrained) |
| failsafeThrottle | `uint16` |  PWM | Sets `currentBatteryProfileMutable->failsafe_throttle` (constrained) |
| gpsType | `uint8` | `gpsProvider_e`  | Enum `gpsProvider_e` Sets `gpsConfigMutable()->provider` (if `USE_GPS`) |
| legacyGpsBaud | `uint8` |   | Ignored |
| gpsSbasMode | `uint8` | `sbasMode_e`  | Enum `sbasMode_e` Sets `gpsConfigMutable()->sbasMode` (if `USE_GPS`) |
| rssiChannel | `uint8` |  Index | Sets `rxConfigMutable()->rssi_channel` (1-based, 0 disables) when <= `MAX_SUPPORTED_RC_CHANNEL_COUNT` |
| magDeclination | `int16` |  0.1 degrees | Sets `compassConfigMutable()->mag_declination = value * 10` (if `USE_MAG`) |
| vbatScale | `uint16` |  Scale | Sets `batteryMetersConfigMutable()->voltage.scale` (if `USE_ADC`) |
| vbatSource | `uint8` | `batVoltageSource_e`  | Enum `batVoltageSource_e` Sets `batteryMetersConfigMutable()->voltageSource` (if `USE_ADC`, validated) |
| cellCount | `uint8` |  Count | Sets `currentBatteryProfileMutable->cells` (if `USE_ADC`) |
| vbatCellDetect | `uint16` |  0.01V | Sets `currentBatteryProfileMutable->voltage.cellDetect` (if `USE_ADC`) |
| vbatMinCell | `uint16` |  0.01V | Sets `currentBatteryProfileMutable->voltage.cellMin` (if `USE_ADC`) |
| vbatMaxCell | `uint16` |  0.01V | Sets `currentBatteryProfileMutable->voltage.cellMax` (if `USE_ADC`) |
| vbatWarningCell | `uint16` |  0.01V | Sets `currentBatteryProfileMutable->voltage.cellWarning` (if `USE_ADC`) |
| capacityValue | `uint32` |  mAh/mWh | Sets `currentBatteryProfileMutable->capacity.value` |
| capacityWarning | `uint32` |  mAh/mWh | Sets `currentBatteryProfileMutable->capacity.warning` |
| capacityCritical | `uint32` |  mAh/mWh | Sets `currentBatteryProfileMutable->capacity.critical` |
| capacityUnit | `uint8` | `batCapacityUnit_e`  | Enum `batCapacityUnit_e` Sets `batteryMetersConfigMutable()->capacity_unit` (validated, updates OSD energy unit if changed) |

*reply:* none

---
## MSP2_INAV_BATTERY_CONFIG

id `0x2005` (8197) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the configuration specific to the battery voltage and current sensors and capacity settings for the current battery profile.

> Fields are 0 if `USE_ADC` is not defined.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| vbatScale | `uint16` |  Scale | Voltage scale (`batteryMetersConfig()->voltage.scale`) |
| vbatSource | `uint8` | `batVoltageSource_e`  | Enum `batVoltageSource_e` Voltage source (`batteryMetersConfig()->voltageSource`) |
| cellCount | `uint8` |  Count | Configured cell count (`currentBatteryProfile->cells`) |
| vbatCellDetect | `uint16` |  0.01V | Cell detection voltage (`currentBatteryProfile->voltage.cellDetect`) |
| vbatMinCell | `uint16` |  0.01V | Min cell voltage (`currentBatteryProfile->voltage.cellMin`) |
| vbatMaxCell | `uint16` |  0.01V | Max cell voltage (`currentBatteryProfile->voltage.cellMax`) |
| vbatWarningCell | `uint16` |  0.01V | Warning cell voltage (`currentBatteryProfile->voltage.cellWarning`) |
| currentOffset | `int16` |  mV | Current sensor offset (`batteryMetersConfig()->current.offset`) |
| currentScale | `int16` |  0.1 mV/A | Current sensor scale (`batteryMetersConfig()->current.scale`) |
| capacityValue | `uint32` |  mAh/mWh | Battery capacity (`currentBatteryProfile->capacity.value`) |
| capacityWarning | `uint32` |  mAh/mWh | Capacity warning threshold (`currentBatteryProfile->capacity.warning`) |
| capacityCritical | `uint32` |  mAh/mWh | Capacity critical threshold (`currentBatteryProfile->capacity.critical`) |
| capacityUnit | `uint8` | `batCapacityUnit_e`  | Enum `batCapacityUnit_e` Capacity unit (`batteryMetersConfig()->capacity_unit`) |

---
## MSP2_INAV_SET_BATTERY_CONFIG

id `0x2006` (8198) · MSPv2 · group `inav`

since INAV 1.0

Sets the battery voltage/current sensor configuration and capacity settings for the current battery profile.

> Expects 29 bytes. Performs validation on `vbatSource` and `capacityUnit`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| vbatScale | `uint16` |  Scale | Sets `batteryMetersConfigMutable()->voltage.scale` (if `USE_ADC`) |
| vbatSource | `uint8` | `batVoltageSource_e`  | Enum `batVoltageSource_e` Sets `batteryMetersConfigMutable()->voltageSource` (if `USE_ADC`, validated) |
| cellCount | `uint8` |  Count | Sets `currentBatteryProfileMutable->cells` (if `USE_ADC`) |
| vbatCellDetect | `uint16` |  0.01V | Sets `currentBatteryProfileMutable->voltage.cellDetect` (if `USE_ADC`) |
| vbatMinCell | `uint16` |  0.01V | Sets `currentBatteryProfileMutable->voltage.cellMin` (if `USE_ADC`) |
| vbatMaxCell | `uint16` |  0.01V | Sets `currentBatteryProfileMutable->voltage.cellMax` (if `USE_ADC`) |
| vbatWarningCell | `uint16` |  0.01V | Sets `currentBatteryProfileMutable->voltage.cellWarning` (if `USE_ADC`) |
| currentOffset | `int16` |  mV | Sets `batteryMetersConfigMutable()->current.offset` |
| currentScale | `int16` |  0.1 mV/A | Sets `batteryMetersConfigMutable()->current.scale` |
| capacityValue | `uint32` |  mAh/mWh | Sets `currentBatteryProfileMutable->capacity.value` |
| capacityWarning | `uint32` |  mAh/mWh | Sets `currentBatteryProfileMutable->capacity.warning` |
| capacityCritical | `uint32` |  mAh/mWh | Sets `currentBatteryProfileMutable->capacity.critical` |
| capacityUnit | `uint8` | `batCapacityUnit_e`  | Enum `batCapacityUnit_e` Sets `batteryMetersConfigMutable()->capacity_unit` (validated, updates OSD energy unit if changed) |

*reply:* none

---
## MSP2_INAV_RATE_PROFILE

id `0x2007` (8199) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the rates and expos for the current control rate profile, including both stabilized and manual flight modes. Supersedes `MSP_RC_TUNING`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| throttleMid | `uint8` |   | Throttle Midpoint (`currentControlRateProfile->throttle.rcMid8`) |
| throttleExpo | `uint8` |   | Throttle Expo (`currentControlRateProfile->throttle.rcExpo8`) |
| dynamicThrottlePID | `uint8` |   | TPA value (`currentControlRateProfile->throttle.dynPID`) |
| tpaBreakpoint | `uint16` |   | TPA breakpoint (`currentControlRateProfile->throttle.tpa_breakpoint`) |
| stabRcExpo | `uint8` |   | Stabilized Roll/Pitch Expo (`currentControlRateProfile->stabilized.rcExpo8`) |
| stabRcYawExpo | `uint8` |   | Stabilized Yaw Expo (`currentControlRateProfile->stabilized.rcYawExpo8`) |
| stabRollRate | `uint8` |   | Stabilized Roll Rate (`currentControlRateProfile->stabilized.rates[FD_ROLL]`) |
| stabPitchRate | `uint8` |   | Stabilized Pitch Rate (`currentControlRateProfile->stabilized.rates[FD_PITCH]`) |
| stabYawRate | `uint8` |   | Stabilized Yaw Rate (`currentControlRateProfile->stabilized.rates[FD_YAW]`) |
| manualRcExpo | `uint8` |   | Manual Roll/Pitch Expo (`currentControlRateProfile->manual.rcExpo8`) |
| manualRcYawExpo | `uint8` |   | Manual Yaw Expo (`currentControlRateProfile->manual.rcYawExpo8`) |
| manualRollRate | `uint8` |   | Manual Roll Rate (`currentControlRateProfile->manual.rates[FD_ROLL]`) |
| manualPitchRate | `uint8` |   | Manual Pitch Rate (`currentControlRateProfile->manual.rates[FD_PITCH]`) |
| manualYawRate | `uint8` |   | Manual Yaw Rate (`currentControlRateProfile->manual.rates[FD_YAW]`) |

---
## MSP2_INAV_SET_RATE_PROFILE

id `0x2008` (8200) · MSPv2 · group `inav`

since INAV 1.0

Sets the rates and expos for the current control rate profile (stabilized and manual). Supersedes `MSP_SET_RC_TUNING`.

> Expects 15 bytes. Constraints applied to rates based on axis.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| throttleMid | `uint8` |   | Sets `currentControlRateProfile->throttle.rcMid8` |
| throttleExpo | `uint8` |   | Sets `currentControlRateProfile->throttle.rcExpo8` |
| dynamicThrottlePID | `uint8` |   | Sets `currentControlRateProfile->throttle.dynPID` |
| tpaBreakpoint | `uint16` |   | Sets `currentControlRateProfile->throttle.tpa_breakpoint` |
| stabRcExpo | `uint8` |   | Sets `currentControlRateProfile->stabilized.rcExpo8` |
| stabRcYawExpo | `uint8` |   | Sets `currentControlRateProfile->stabilized.rcYawExpo8` |
| stabRollRate | `uint8` |   | Sets `currentControlRateProfile->stabilized.rates[FD_ROLL]` (constrained) |
| stabPitchRate | `uint8` |   | Sets `currentControlRateProfile->stabilized.rates[FD_PITCH]` (constrained) |
| stabYawRate | `uint8` |   | Sets `currentControlRateProfile->stabilized.rates[FD_YAW]` (constrained) |
| manualRcExpo | `uint8` |   | Sets `currentControlRateProfile->manual.rcExpo8` |
| manualRcYawExpo | `uint8` |   | Sets `currentControlRateProfile->manual.rcYawExpo8` |
| manualRollRate | `uint8` |   | Sets `currentControlRateProfile->manual.rates[FD_ROLL]` (constrained) |
| manualPitchRate | `uint8` |   | Sets `currentControlRateProfile->manual.rates[FD_PITCH]` (constrained) |
| manualYawRate | `uint8` |   | Sets `currentControlRateProfile->manual.rates[FD_YAW]` (constrained) |

*reply:* none

---
## MSP2_INAV_AIR_SPEED

id `0x2009` (8201) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the estimated or measured airspeed.

> Requires `USE_PITOT`; returns 0 when pitot functionality is not enabled or calibrated.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| airspeed | `uint32` |  cm/s | Estimated/measured airspeed (`getAirspeedEstimate()`, cm/s). 0 if unavailable |

---
## MSP2_INAV_OUTPUT_MAPPING

id `0x200A` (8202) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the output mapping configuration (identifies which timer outputs are used for Motors/Servos). Legacy version sending only 8-bit usage flags.

> Superseded by `MSP2_INAV_OUTPUT_MAPPING_EXT2`. Only includes timers *not* used for PPM/PWM input. Record count is not a constant: loops timerHardwareCount, skipping timers flagged TIM_USE_PPM or TIM_USE_PWM; read until the payload is exhausted.

*request:* none

*reply:* (repeat: until_end)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| usageFlags | `uint8` |   | Timer usage flags (lower 8 bits of `timerHardware[i].usageFlags`, e.g. `TIM_USE_MOTOR`, `TIM_USE_SERVO`) |

---
## MSP2_INAV_MC_BRAKING

id `0x200B` (8203) · MSPv2 · group `inav`

since INAV 1.0

Retrieves configuration parameters for the multirotor braking mode feature.

> Payload is empty if `USE_MR_BRAKING_MODE` is not defined.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| brakingSpeedThreshold | `uint16` |  cm/s | Speed above which braking engages (`navConfig()->mc.braking_speed_threshold`) |
| brakingDisengageSpeed | `uint16` |  cm/s | Speed below which braking disengages (`navConfig()->mc.braking_disengage_speed`) |
| brakingTimeout | `uint16` |  ms | Timeout before braking force reduces (`navConfig()->mc.braking_timeout`) |
| brakingBoostFactor | `uint8` |  % | Boost factor applied during braking (`navConfig()->mc.braking_boost_factor`) |
| brakingBoostTimeout | `uint16` |  ms | Timeout for the boost factor (`navConfig()->mc.braking_boost_timeout`) |
| brakingBoostSpeedThreshold | `uint16` |  cm/s | Speed threshold for boost engagement (`navConfig()->mc.braking_boost_speed_threshold`) |
| brakingBoostDisengageSpeed | `uint16` |  cm/s | Speed threshold for boost disengagement (`navConfig()->mc.braking_boost_disengage_speed`) |
| brakingBankAngle | `uint8` |  degrees | Maximum bank angle allowed during braking (`navConfig()->mc.braking_bank_angle`) |

---
## MSP2_INAV_SET_MC_BRAKING

id `0x200C` (8204) · MSPv2 · group `inav`

since INAV 1.0

Sets configuration parameters for the multirotor braking mode feature.

> Expects 14 bytes. Returns error if `USE_MR_BRAKING_MODE` is not defined.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| brakingSpeedThreshold | `uint16` |  cm/s | Sets `navConfigMutable()->mc.braking_speed_threshold` |
| brakingDisengageSpeed | `uint16` |  cm/s | Sets `navConfigMutable()->mc.braking_disengage_speed` |
| brakingTimeout | `uint16` |  ms | Sets `navConfigMutable()->mc.braking_timeout` |
| brakingBoostFactor | `uint8` |  % | Sets `navConfigMutable()->mc.braking_boost_factor` |
| brakingBoostTimeout | `uint16` |  ms | Sets `navConfigMutable()->mc.braking_boost_timeout` |
| brakingBoostSpeedThreshold | `uint16` |  cm/s | Sets `navConfigMutable()->mc.braking_boost_speed_threshold` |
| brakingBoostDisengageSpeed | `uint16` |  cm/s | Sets `navConfigMutable()->mc.braking_boost_disengage_speed` |
| brakingBankAngle | `uint8` |  degrees | Sets `navConfigMutable()->mc.braking_bank_angle` |

*reply:* none

---
## MSP2_INAV_OUTPUT_MAPPING_EXT

id `0x200D` (8205) · MSPv2 · group `inav`

since INAV 7.0

Retrieves extended output mapping configuration (timer ID and usage flags). Obsolete, use `MSP2_INAV_OUTPUT_MAPPING_EXT2`.

> Usage flags are truncated to 8 bits. `timerId` mapping is target-specific. Record count is not a constant: loops timerHardwareCount, skipping timers flagged TIM_USE_PPM or TIM_USE_PWM; read until the payload is exhausted.

*request:* none

*reply:* (repeat: until_end)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| timerId | `uint8` |   | Hardware timer identifier (e.g., `TIM1`, `TIM2`). Value depends on target |
| usageFlags | `uint8` |   | Timer usage flags (lower 8 bits of `timerHardware[i].usageFlags`, e.g. `TIM_USE_MOTOR`, `TIM_USE_SERVO`) |

---
## MSP2_INAV_TIMER_OUTPUT_MODE

id `0x200E` (8206) · MSPv2 · group `inav`

since INAV 7.0

Reads timer output mode overrides.

> Non-SITL only. HARDWARE_TIMER_DEFINITION_COUNT is target specific. Returns MSP_RESULT_ACK on success, MSP_RESULT_ERROR on invalid timer index.

**variant: dataSize == 0**

*request:* none

*reply:* (repeat: HARDWARE_TIMER_DEFINITION_COUNT)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| timerIndex | `uint8` |   | Timer index |
| outputMode | `uint8` | `outputMode_e`  | OUTPUT_MODE_* |

**variant: dataSize == 1**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| timerIndex | `uint8` |   | 0..HARDWARE_TIMER_DEFINITION_COUNT-1 |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| timerIndex | `uint8` |   | Echoed timer index |
| outputMode | `uint8` | `outputMode_e`  | OUTPUT_MODE_* |

---
## MSP2_INAV_SET_TIMER_OUTPUT_MODE

id `0x200F` (8207) · MSPv2 · group `inav`

since INAV 7.0

Set the output mode override for a specific hardware timer.

> Only available on non-SITL builds. Expects 2 bytes. Returns error if `timerIndex` is invalid.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| timerIndex | `uint8` |   | Index of the hardware timer definition |
| outputMode | `uint8` | `outputMode_e`  | Output mode override (`outputMode_e` enum) to set |

*reply:* none

---
## MSP2_INAV_MIXER

id `0x2010` (8208) · MSPv2 · group `inav`

since INAV 1.0

Retrieves INAV-specific mixer configuration details.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| motorDirectionInverted | `uint8` |   | Boolean: 1 if motor direction is reversed globally (`mixerConfig()->motorDirectionInverted`) |
| reserved1 | `uint8` |   | Always 0 (Was yaw jump prevention limit) |
| motorStopOnLow | `uint8` |   | Boolean: 1 if motors stop at minimum throttle (`mixerConfig()->motorstopOnLow`) |
| platformType | `uint8` | `flyingPlatformType_e`  | Enum (`mixerConfig()->platformType`) |
| hasFlaps | `uint8` |   | Boolean: 1 if the current mixer configuration includes flaps (`mixerConfig()->hasFlaps`) |
| appliedMixerPreset | `int16` |   | Mixer preset currently applied (`mixerConfig()->appliedMixerPreset`). Plain integer, not an enum: the firmware never interprets it and only stores whatever the configurator wrote, defaulting to `SETTING_MODEL_PREVIEW_TYPE_DEFAULT`. |
| maxMotors | `uint8` |   | Constant: Maximum motors supported (`MAX_SUPPORTED_MOTORS`) |
| maxServos | `uint8` |   | Constant: Maximum servos supported (`MAX_SUPPORTED_SERVOS`) |

---
## MSP2_INAV_SET_MIXER

id `0x2011` (8209) · MSPv2 · group `inav`

since INAV 1.0

Sets INAV-specific mixer configuration details.

> Expects 9 bytes. Calls `mixerUpdateStateFlags()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| motorDirectionInverted | `uint8` |   | Sets `mixerConfigMutable()->motorDirectionInverted` |
| reserved1 | `uint8` |   | Ignored |
| motorStopOnLow | `uint8` |   | Sets `mixerConfigMutable()->motorstopOnLow` |
| platformType | `uint8` | `flyingPlatformType_e`  | Sets `mixerConfigMutable()->platformType` |
| hasFlaps | `uint8` |   | Sets `mixerConfigMutable()->hasFlaps` |
| appliedMixerPreset | `int16` |   | Sets `mixerConfigMutable()->appliedMixerPreset` |
| maxMotors | `uint8` |   | Ignored |
| maxServos | `uint8` |   | Ignored |

*reply:* none

---
## MSP2_INAV_OSD_LAYOUTS

id `0x2012` (8210) · MSPv2 · group `inav`

since INAV 1.0

Retrieves OSD layout metadata or item positions for specific layouts/items.

> Requires `USE_OSD`. Returns `MSP_RESULT_ACK` on success, `MSP_RESULT_ERROR` if indexes are out of range.

**variant: dataSize == 0**

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| layoutCount | `uint8` |   | Number of OSD layouts (`OSD_LAYOUT_COUNT`) |
| itemCount | `uint8` |   | Number of OSD items per layout (`OSD_ITEM_COUNT`) |

**variant: dataSize == 1**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| layoutIndex | `uint8` |   | Layout index (0 to `OSD_LAYOUT_COUNT - 1`) |

*reply:* (repeat: OSD_ITEM_COUNT)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| itemPosition | `uint16` |  packed coords | Packed X/Y position (`osdLayoutsConfig()->item_pos[layoutIndex][item]`) |

**variant: dataSize == 3**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| layoutIndex | `uint8` |   | Layout index (0 to `OSD_LAYOUT_COUNT - 1`) |
| itemIndex | `uint16` |   | OSD item index (0 to `OSD_ITEM_COUNT - 1`) |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| itemPosition | `uint16` |  packed coords | Packed X/Y position (`osdLayoutsConfig()->item_pos[layoutIndex][itemIndex]`) |

---
## MSP2_INAV_OSD_SET_LAYOUT_ITEM

id `0x2013` (8211) · MSPv2 · group `inav`

since INAV 1.0

Sets the position of a single OSD item within a specific layout.

> Requires `USE_OSD`. Expects 4 bytes. Returns error if indexes are invalid. If the modified layout is not the currently active one, it temporarily overrides the active layout for 10 seconds to show the change. Otherwise, triggers a full OSD redraw.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| layoutIndex | `uint8` |  Index | Index of the OSD layout (0 to `OSD_LAYOUT_COUNT - 1`) |
| itemIndex | `uint8` |  Index | Index of the OSD item |
| itemPosition | `uint16` |  Coordinates | Packed X/Y position using `OSD_POS(x, y)` with `OSD_VISIBLE_FLAG` bit |

*reply:* none

---
## MSP2_INAV_OSD_ALARMS

id `0x2014` (8212) · MSPv2 · group `inav`

since INAV 1.0

Retrieves OSD alarm threshold settings.

> Requires `USE_OSD`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rssiAlarm | `uint8` |  % | RSSI alarm threshold (`osdConfig()->rssi_alarm`) |
| timerAlarm | `uint16` |  seconds | Timer alarm threshold (`osdConfig()->time_alarm`) |
| altAlarm | `uint16` |  meters | Altitude alarm threshold (`osdConfig()->alt_alarm`) |
| distAlarm | `uint16` |  meters | Distance alarm threshold (`osdConfig()->dist_alarm`) |
| negAltAlarm | `uint16` |  meters | Negative altitude alarm threshold (`osdConfig()->neg_alt_alarm`) |
| gForceAlarm | `uint16` |  G * 1000 | G-force alarm threshold (`osdConfig()->gforce_alarm * 1000`) |
| gForceAxisMinAlarm | `int16` |  G * 1000 | Min G-force per-axis alarm (`osdConfig()->gforce_axis_alarm_min * 1000`) |
| gForceAxisMaxAlarm | `int16` |  G * 1000 | Max G-force per-axis alarm (`osdConfig()->gforce_axis_alarm_max * 1000`) |
| currentAlarm | `uint8` |  A | Current draw alarm threshold (`osdConfig()->current_alarm`) |
| imuTempMinAlarm | `int16` |  degrees C | Min IMU temperature alarm (`osdConfig()->imu_temp_alarm_min`) |
| imuTempMaxAlarm | `int16` |  degrees C | Max IMU temperature alarm (`osdConfig()->imu_temp_alarm_max`) |
| baroTempMinAlarm | `int16` |  degrees C | Min Baro temperature alarm (`osdConfig()->baro_temp_alarm_min`). 0 if `USE_BARO` disabled |
| baroTempMaxAlarm | `int16` |  degrees C | Max Baro temperature alarm (`osdConfig()->baro_temp_alarm_max`). 0 if `USE_BARO` disabled |
| adsbWarnDistance | `uint16` |  meters | ADSB warning distance (`osdConfig()->adsb_distance_warning`). 0 if `USE_ADSB` disabled |
| adsbAlertDistance | `uint16` |  meters | ADSB alert distance (`osdConfig()->adsb_distance_alert`). 0 if `USE_ADSB` disabled |

---
## MSP2_INAV_OSD_SET_ALARMS

id `0x2015` (8213) · MSPv2 · group `inav`

since INAV 1.0

Sets OSD alarm threshold settings.

> Requires `USE_OSD`. Expects 24 bytes. ADSB alarms are not settable via this message.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| rssiAlarm | `uint8` |  % | Sets `osdConfigMutable()->rssi_alarm |
| timerAlarm | `uint16` |  seconds | Sets `osdConfigMutable()->time_alarm |
| altAlarm | `uint16` |  meters | Sets `osdConfigMutable()->alt_alarm |
| distAlarm | `uint16` |  meters | Sets `osdConfigMutable()->dist_alarm |
| negAltAlarm | `uint16` |  meters | Sets `osdConfigMutable()->neg_alt_alarm` |
| gForceAlarm | `uint16` |  G * 1000 | Sets `osdConfigMutable()->gforce_alarm = value / 1000.0f` |
| gForceAxisMinAlarm | `int16` |  G * 1000 | Sets `osdConfigMutable()->gforce_axis_alarm_min = value / 1000.0f` |
| gForceAxisMaxAlarm | `int16` |  G * 1000 | Sets `osdConfigMutable()->gforce_axis_alarm_max = value / 1000.0f` |
| currentAlarm | `uint8` |  A | Sets `osdConfigMutable()->current_alarm` |
| imuTempMinAlarm | `int16` |  degrees C | Sets `osdConfigMutable()->imu_temp_alarm_min` |
| imuTempMaxAlarm | `int16` |  degrees C | Sets `osdConfigMutable()->imu_temp_alarm_max` |
| baroTempMinAlarm | `int16` |  degrees C | Sets `osdConfigMutable()->baro_temp_alarm_min` (if `USE_BARO`) |
| baroTempMaxAlarm | `int16` |  degrees C | Sets `osdConfigMutable()->baro_temp_alarm_max` (if `USE_BARO`) |

*reply:* none

---
## MSP2_INAV_OSD_PREFERENCES

id `0x2016` (8214) · MSPv2 · group `inav`

since INAV 1.0

Retrieves OSD display preferences (video system, units, styles, etc.).

> Requires `USE_OSD`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| videoSystem | `uint8` | `videoSystem_e`  | Enum `videoSystem_e`: Video system (Auto/PAL/NTSC) (`osdConfig()->video_system`) |
| mainVoltageDecimals | `uint8` |   | Count: Decimal places for main voltage display (`osdConfig()->main_voltage_decimals`) |
| ahiReverseRoll | `uint8` |   | Boolean: Reverse roll direction on Artificial Horizon (`osdConfig()->ahi_reverse_roll`) |
| crosshairsStyle | `uint8` | `osd_crosshairs_style_e`  | Enum `osd_crosshairs_style_e`: Style of the center crosshairs (`osdConfig()->crosshairs_style`) |
| leftSidebarScroll | `uint8` | `osd_sidebar_scroll_e`  | Enum `osd_sidebar_scroll_e`: Left sidebar scroll behavior (`osdConfig()->left_sidebar_scroll`) |
| rightSidebarScroll | `uint8` | `osd_sidebar_scroll_e`  | Enum `osd_sidebar_scroll_e`: Right sidebar scroll behavior (`osdConfig()->right_sidebar_scroll`) |
| sidebarScrollArrows | `uint8` |   | Boolean: Show arrows for scrollable sidebars (`osdConfig()->sidebar_scroll_arrows`) |
| units | `uint8` | `osd_unit_e`  | Enum: `osd_unit_e` Measurement units (Metric/Imperial) (`osdConfig()->units`) |
| statsEnergyUnit | `uint8` | `osd_stats_energy_unit_e`  | Enum `osd_stats_energy_unit_e`: Unit for energy display in post-flight stats (`osdConfig()->stats_energy_unit`) |
| adsbWarningStyle | `uint8` |   | Enum `osd_adsb_warning_style_e`: How ADSB proximity warnings are drawn (`osdConfig()->adsb_warning_style`). 0 if `USE_ADSB` disabled |

---
## MSP2_INAV_OSD_SET_PREFERENCES

id `0x2017` (8215) · MSPv2 · group `inav`

since INAV 1.0

Sets OSD display preferences.

> Requires `USE_OSD`. Expects 9 bytes. Triggers a full OSD redraw.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| videoSystem | `uint8` | `videoSystem_e`  | Sets `osdConfigMutable()->video_system` |
| mainVoltageDecimals | `uint8` |   | Sets `osdConfigMutable()->main_voltage_decimals` |
| ahiReverseRoll | `uint8` |   | Sets `osdConfigMutable()->ahi_reverse_roll` |
| crosshairsStyle | `uint8` | `osd_crosshairs_style_e`  | Sets `osdConfigMutable()->crosshairs_style` |
| leftSidebarScroll | `uint8` | `osd_sidebar_scroll_e`  | Sets `osdConfigMutable()->left_sidebar_scroll` |
| rightSidebarScroll | `uint8` | `osd_sidebar_scroll_e`  | Sets `osdConfigMutable()->right_sidebar_scroll` |
| sidebarScrollArrows | `uint8` |   | Sets `osdConfigMutable()->sidebar_scroll_arrows` |
| units | `uint8` | `osd_unit_e`  | Sets `osdConfigMutable()->units` (enum `osd_unit_e`) |
| statsEnergyUnit | `uint8` | `osd_stats_energy_unit_e`  | Sets `osdConfigMutable()->stats_energy_unit` |
| adsbWarningStyle | `optional uint8` |   | Sets `osdConfigMutable()->adsb_warning_style`. Only read when the payload is at least 10 bytes and the firmware has `USE_ADSB` |

*reply:* none

---
## MSP2_INAV_SELECT_BATTERY_PROFILE

id `0x2018` (8216) · MSPv2 · group `inav`

since INAV 1.0

Selects the active battery profile and saves configuration.

> Expects 1 byte. Will fail if armed. Calls `setConfigBatteryProfileAndWriteEEPROM()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| batteryProfileIndex | `uint8` |   | Index of the battery profile to activate (0-based) |

*reply:* none

---
## MSP2_INAV_DEBUG

id `0x2019` (8217) · MSPv2 · group `inav`

since INAV 1.0

Retrieves values from the firmware's 32-bit `debug[]` array. Supersedes `MSP_DEBUG`.

> `DEBUG32_VALUE_COUNT` is usually 8.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| debugValues | `int32[DEBUG32_VALUE_COUNT]` |   | Values from the `debug` array (signed, typically 8 entries) |

---
## MSP2_BLACKBOX_CONFIG

id `0x201A` (8218) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the Blackbox configuration. Supersedes `MSP_BLACKBOX_CONFIG`.

> If `USE_BLACKBOX` is disabled, only the first four fields are returned (all zero).

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| blackboxSupported | `uint8` |   | Boolean: 1 if Blackbox is supported (`USE_BLACKBOX`), 0 otherwise |
| blackboxDevice | `uint8` | `BlackboxDevice`  | Enum `BlackboxDevice`: Target device for logging (`blackboxConfig()->device`). 0 if not supported |
| blackboxRateNum | `uint16` |   | Numerator for logging rate divider (`blackboxConfig()->rate_num`). 0 if not supported |
| blackboxRateDenom | `uint16` |   | Denominator for logging rate divider (`blackboxConfig()->rate_denom`). 0 if not supported |
| blackboxIncludeFlags | `optional uint32` | `bitmask`  | Bitmask: Flags for fields included/excluded from logging (`blackboxConfig()->includeFlags`). Absent when the firmware was built without `USE_BLACKBOX`, which replies with the first 6 bytes only |

---
## MSP2_SET_BLACKBOX_CONFIG

id `0x201B` (8219) · MSPv2 · group `inav`

since INAV 1.0

Sets the Blackbox configuration. Supersedes `MSP_SET_BLACKBOX_CONFIG`.

> Requires `USE_BLACKBOX`. Expects 9 bytes. Returns error if Blackbox is currently logging (`!blackboxMayEditConfig()`).

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| blackboxDevice | `uint8` | `BlackboxDevice`  | Sets `blackboxConfigMutable()->device` |
| blackboxRateNum | `uint16` |   | Sets `blackboxConfigMutable()->rate_num` |
| blackboxRateDenom | `uint16` |   | Sets `blackboxConfigMutable()->rate_denom` |
| blackboxIncludeFlags | `uint32` |   | Sets `blackboxConfigMutable()->includeFlags` |

*reply:* none

---
## MSP2_INAV_TEMP_SENSOR_CONFIG

id `0x201C` (8220) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the configuration for all onboard temperature sensors.

> Requires `USE_TEMPERATURE_SENSOR`.

*request:* none

*reply:* (repeat: MAX_TEMP_SENSORS)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| type | `uint8` | `tempSensorType_e`  | Enum (`tempSensorType_e`): Type of the temperature sensor |
| address | `uint64` |   | Sensor address/ID (e.g., for 1-Wire sensors) |
| alarmMin | `int16` |  0.1°C | Min temperature alarm threshold (`sensorConfig->alarm_min`) |
| alarmMax | `int16` |  0.1°C | Max temperature alarm threshold (`sensorConfig->alarm_max`) |
| osdSymbol | `uint8` |   | Index: OSD symbol to use for this sensor (0 to `TEMP_SENSOR_SYM_COUNT`) |
| label | `char[TEMPERATURE_LABEL_LEN]` |   | User-defined label for the sensor |

---
## MSP2_INAV_SET_TEMP_SENSOR_CONFIG

id `0x201D` (8221) · MSPv2 · group `inav`

since INAV 1.0

Sets the configuration for all onboard temperature sensors.

> Requires `USE_TEMPERATURE_SENSOR`. Payload must include `MAX_TEMP_SENSORS` consecutive `tempSensorConfig_t` structures (labels are uppercased).

*request:* (repeat: MAX_TEMP_SENSORS)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| type | `uint8` | `tempSensorType_e`  | Sets sensor type (`tempSensorType_e`) |
| address | `uint64` |   | Sets sensor address/ID |
| alarmMin | `int16` |  0.1°C | Sets min alarm threshold (`tempSensorConfigMutable(index)->alarm_min`) |
| alarmMax | `int16` |  0.1°C | Sets max alarm threshold (`tempSensorConfigMutable(index)->alarm_max`) |
| osdSymbol | `uint8` |   | Sets OSD symbol index (validated) |
| label | `char[TEMPERATURE_LABEL_LEN]` |   | Sets sensor label (converted to uppercase) |

*reply:* none

---
## MSP2_INAV_TEMPERATURES

id `0x201E` (8222) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the current readings from all configured temperature sensors.

> Requires `USE_TEMPERATURE_SENSOR`.

*request:* none

*reply:* (repeat: MAX_TEMP_SENSORS)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| temperature | `int16` |  0.1°C | Current temperature reading. -1000 if sensor is invalid or reading failed |

---
## MSP_SIMULATOR

id `0x201F` (8223) · MSPv2 · group `inav`

since INAV 1.0

Handles Hardware-in-the-Loop (HITL) simulation data exchange. Receives simulated sensor data and options, sends back control outputs and debug info.

> Requires `USE_SIMULATOR`. Complex message handling state changes for enabling/disabling HITL. Sensor data is injected directly. OSD data is sent using a custom RLE scheme. See `simulatorData` struct and associated code for details.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| simulatorVersion | `uint8` |   | Version of the simulator protocol (`SIMULATOR_MSP_VERSION`) |
| simulatorFlags_t | `uint8` | `simulatorFlags_t (bitmask)`  | Bitmask: Options for HITL (`HITL_*` flags) |
| gpsFixType | `uint8` | `gpsFixType_e`  | Enum `gpsFixType_e` Type of GPS fix (If `HITL_HAS_NEW_GPS_DATA`) |
| gpsNumSat | `uint8` |   | (If `HITL_HAS_NEW_GPS_DATA`) Simulated satellite count |
| gpsLat | `int32` |   | (If `HITL_HAS_NEW_GPS_DATA`) Simulated latitude (1e7 deg) |
| gpsLon | `int32` |   | (If `HITL_HAS_NEW_GPS_DATA`) Simulated longitude (1e7 deg) |
| gpsAlt | `int32` |   | (If `HITL_HAS_NEW_GPS_DATA`) Simulated altitude (cm) |
| gpsSpeed | `uint16` |   | (If `HITL_HAS_NEW_GPS_DATA`) Simulated ground speed (cm/s) |
| gpsCourse | `uint16` |   | (If `HITL_HAS_NEW_GPS_DATA`) Simulated ground course (deci-deg) |
| gpsVelN | `int16` |   | (If `HITL_HAS_NEW_GPS_DATA`) Simulated North velocity (cm/s) |
| gpsVelE | `int16` |   | (If `HITL_HAS_NEW_GPS_DATA`) Simulated East velocity (cm/s) |
| gpsVelD | `int16` |   | (If `HITL_HAS_NEW_GPS_DATA`) Simulated Down velocity (cm/s) |
| imuRoll | `int16` |   | (If NOT `HITL_USE_IMU`) Simulated Roll (deci-deg) |
| imuPitch | `int16` |   | (If NOT `HITL_USE_IMU`) Simulated Pitch (deci-deg) |
| imuYaw | `int16` |   | (If NOT `HITL_USE_IMU`) Simulated Yaw (deci-deg) |
| accX | `int16` |   | mG (G * 1000) |
| accY | `int16` |   | mG (G * 1000) |
| accZ | `int16` |   | mG (G * 1000) |
| gyroX | `int16` |   | dps * 16 |
| gyroY | `int16` |   | dps * 16 |
| gyroZ | `int16` |   | dps * 16 |
| baroPressure | `uint32` |   | Pa |
| magX | `int16` |   | Scaled |
| magY | `int16` |   | Scaled |
| magZ | `int16` |   | Scaled |
| vbat | `uint8` |   | (If `HITL_EXT_BATTERY_VOLTAGE`) Simulated battery voltage (0.1V units) |
| airspeed | `uint16` |   | (If `HITL_AIRSPEED`) Simulated airspeed (cm/s) |
| extFlags | `uint8` |   | (If `HITL_EXTENDED_FLAGS`) Additional flags (upper 8 bits) |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| stabilizedRoll | `uint16` |   | Stabilized Roll command output (-500 to 500) |
| stabilizedPitch | `uint16` |   | Stabilized Pitch command output (-500 to 500) |
| stabilizedYaw | `uint16` |   | Stabilized Yaw command output (-500 to 500) |
| stabilizedThrottle | `uint16` |   | Stabilized Throttle command output (-500 to 500 if armed, else -500) |
| debugFlags | `uint8` |   | Packed flags: Debug index (0-7), Platform type, Armed state, OSD feature status |
| debugValue | `uint32` |   | Current debug value (`debug[simulatorData.debugIndex]`) |
| attitudeRoll | `int16` |   | Current estimated Roll (deci-deg) |
| attitudePitch | `int16` |   | Current estimated Pitch (deci-deg) |
| attitudeYaw | `int16` |   | Current estimated Yaw (deci-deg) |
| osdHeader | `optional uint8` |   | OSD RLE Header (255) |
| osdRows | `optional uint8` |   | (If OSD supported) Number of OSD rows |
| osdCols | `optional uint8` |   | (If OSD supported) Number of OSD columns |
| osdStartY | `optional uint8` |   | (If OSD supported) Starting row for RLE data |
| osdStartX | `optional uint8` |   | (If OSD supported) Starting column for RLE data |
| osdRleData | `optional uint8[]` |   | (If OSD supported) Run-length encoded OSD character data. Terminated by `[0, 0]` |

---
## MSP2_INAV_SERVO_MIXER

id `0x2020` (8224) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the custom servo mixer rules, including programming framework condition IDs, for primary and secondary mixer profiles. Supersedes `MSP_SERVO_MIX_RULES`.

> `conditionId` requires `USE_PROGRAMMING_FRAMEWORK`. If multiple mixer profiles are enabled (`MAX_MIXER_PROFILE_COUNT > 1`), a second block of `MAX_SERVO_RULES` rules for the next profile follows immediately.

*request:* none

*reply:* (repeat: MAX_SERVO_RULES)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| targetChannel | `uint8` |   | Servo output channel index (0-based) |
| inputSource | `uint8` | `inputSource_e`  | Enum `inputSource_e` Input source |
| rate | `int16` |   | Mixing rate/weight |
| speed | `uint8` |   | Speed/Slew rate limit (0-100) |
| conditionId | `int8` |   | Logic Condition ID (0 to `MAX_LOGIC_CONDITIONS - 1`, or 255/-1 if none/disabled) |

---
## MSP2_INAV_SET_SERVO_MIXER

id `0x2021` (8225) · MSPv2 · group `inav`

since INAV 1.0

Sets a single custom servo mixer rule, including programming framework condition ID. Supersedes `MSP_SET_SERVO_MIX_RULE`.

> Expects 7 bytes. Returns error if index invalid. Calls `loadCustomServoMixer()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| ruleIndex | `uint8` |   | Index of the rule to set (0 to `MAX_SERVO_RULES - 1`) |
| targetChannel | `uint8` |   | Servo output channel index |
| inputSource | `uint8` | `inputSource_e`  | Enum `inputSource_e` Input source |
| rate | `int16` |   | Mixing rate/weight |
| speed | `uint8` |   | Speed/Slew rate limit (0-100) |
| conditionId | `int8` |   | Logic Condition ID (255/-1 if none). Ignored if `USE_PROGRAMMING_FRAMEWORK` is disabled |

*reply:* none

---
## MSP2_INAV_LOGIC_CONDITIONS

id `0x2022` (8226) · MSPv2 · group `inav`  ·  *not implemented*

since INAV 1.0

Retrieves the configuration of all defined Logic Conditions. Requires `USE_PROGRAMMING_FRAMEWORK`. See `logicCondition_t` structure.

> Deprecated, causes buffer overflow for 14*64 bytes

*request:* none

*reply:* none

---
## MSP2_INAV_SET_LOGIC_CONDITIONS

id `0x2023` (8227) · MSPv2 · group `inav`

since INAV 1.0

Sets the configuration for a single Logic Condition by its index.

> Requires `USE_PROGRAMMING_FRAMEWORK`. Expects 15 bytes. Returns error if index is invalid.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| conditionIndex | `uint8` |   | Index of the condition to set (0 to `MAX_LOGIC_CONDITIONS - 1`) |
| enabled | `uint8` |   | Boolean: 1 to enable the condition |
| activatorId | `int8` |   | Activator condition ID (-1/255 if none) |
| operation | `uint8` | `logicOperation_e`  | Enum `logicOperation_e` Logical operation |
| operandAType | `uint8` | `logicOperandType_e`  | Enum `logicOperandType_e` Type of operand A |
| operandAValue | `int32` |   | Value/ID of operand A |
| operandBType | `uint8` | `logicOperandType_e`  | Enum `logicOperandType_e` Type of operand B |
| operandBValue | `int32` |   | Value/ID of operand B |
| flags | `uint8` | `logicConditionFlags_e (bitmask)`  | Bitmask: Condition flags (`logicConditionFlags_e`) |

*reply:* none

---
## MSP2_INAV_GLOBAL_FUNCTIONS

id `0x2024` (8228) · MSPv2 · group `inav`  ·  *not implemented*

since INAV 1.0

*request:* none

*reply:* none

---
## MSP2_INAV_SET_GLOBAL_FUNCTIONS

id `0x2025` (8229) · MSPv2 · group `inav`  ·  *not implemented*

since INAV 1.0

*request:* none

*reply:* none

---
## MSP2_INAV_LOGIC_CONDITIONS_STATUS

id `0x2026` (8230) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the current evaluated status (true/false or numerical value) of all logic conditions.

> Requires `USE_PROGRAMMING_FRAMEWORK`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| conditionValues | `int32[MAX_LOGIC_CONDITIONS]` |   | Array of current values for each logic condition (`logicConditionGetValue(i)`). 1 for true, 0 for false, or numerical value depending on operation |

---
## MSP2_INAV_GVAR_STATUS

id `0x2027` (8231) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the current values of all Global Variables (GVARS).

> Requires `USE_PROGRAMMING_FRAMEWORK`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| gvarValues | `int32[MAX_GLOBAL_VARIABLES]` |   | Array of current values for each global variable (`gvGet(i)`) |

---
## MSP2_INAV_PROGRAMMING_PID

id `0x2028` (8232) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the configuration of all Programming PIDs.

> Requires `USE_PROGRAMMING_FRAMEWORK`. See `programmingPid_t` structure.

*request:* none

*reply:* (repeat: MAX_PROGRAMMING_PID_COUNT)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| enabled | `uint8` |   | Boolean: 1 if the PID is enabled |
| setpointType | `uint8` | `logicOperandType_e`  | Enum (`logicOperandType_e`) Type of the setpoint source |
| setpointValue | `int32` |   | Value/ID of the setpoint source |
| measurementType | `uint8` | `logicOperandType_e`  | Enum (`logicOperandType_e`) Type of the measurement source |
| measurementValue | `int32` |   | Value/ID of the measurement source |
| gainP | `uint16` |   | Proportional gain |
| gainI | `uint16` |   | Integral gain |
| gainD | `uint16` |   | Derivative gain |
| gainFF | `uint16` |   | Feed-forward gain |

---
## MSP2_INAV_SET_PROGRAMMING_PID

id `0x2029` (8233) · MSPv2 · group `inav`

since INAV 1.0

Sets the configuration for a single Programming PID by its index.

> Requires `USE_PROGRAMMING_FRAMEWORK`. Expects 20 bytes. Returns error if index is invalid.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| pidIndex | `uint8` |   | Index of the Programming PID to set (0 to `MAX_PROGRAMMING_PID_COUNT - 1`) |
| enabled | `uint8` |   | Boolean: 1 to enable the PID |
| setpointType | `uint8` | `logicOperandType_e`  | Enum (`logicOperandType_e`) Type of the setpoint source |
| setpointValue | `int32` |   | Value/ID of the setpoint source |
| measurementType | `uint8` | `logicOperandType_e`  | Enum (`logicOperandType_e`) Type of the measurement source |
| measurementValue | `int32` |   | Value/ID of the measurement source |
| gainP | `uint16` |   | Proportional gain |
| gainI | `uint16` |   | Integral gain |
| gainD | `uint16` |   | Derivative gain |
| gainFF | `uint16` |   | Feed-forward gain |

*reply:* none

---
## MSP2_INAV_PROGRAMMING_PID_STATUS

id `0x202A` (8234) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the current output value of all Programming PIDs.

> Requires `USE_PROGRAMMING_FRAMEWORK`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| pidOutputs | `int32[MAX_PROGRAMMING_PID_COUNT]` |   | Array of current output values for each Programming PID (`programmingPidGetOutput(i)`, signed) |

---
## MSP2_PID

id `0x2030` (8240) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the standard PID controller gains (P, I, D, FF) for the current PID profile.

> `PID_ITEM_COUNT` defines the number of standard PID controllers (Roll, Pitch, Yaw, Alt, Vel, etc.). Updates from EZ-Tune if enabled.

*request:* none

*reply:* (repeat: PID_ITEM_COUNT)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| P | `uint8` |   | Proportional gain (`pidBank()->pid[i].P`), constrained 0-255 |
| I | `uint8` |   | Integral gain (`pidBank()->pid[i].I`), constrained 0-255 |
| D | `uint8` |   | Derivative gain (`pidBank()->pid[i].D`), constrained 0-255 |
| FF | `uint8` |   | Feed-forward gain (`pidBank()->pid[i].FF`), constrained 0-255 |

---
## MSP2_SET_PID

id `0x2031` (8241) · MSPv2 · group `inav`

since INAV 1.0

Sets the standard PID controller gains (P, I, D, FF) for the current PID profile.

> Expects `PID_ITEM_COUNT * 4` bytes. Calls `schedulePidGainsUpdate()` and `navigationUsePIDs()`.

*request:* (repeat: PID_ITEM_COUNT)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| P | `uint8` |   | Sets Proportional gain (`pidBankMutable()->pid[i].P`) |
| I | `uint8` |   | Sets Integral gain (`pidBankMutable()->pid[i].I`) |
| D | `uint8` |   | Sets Derivative gain (`pidBankMutable()->pid[i].D`) |
| FF | `uint8` |   | Sets Feed-forward gain (`pidBankMutable()->pid[i].FF`) |

*reply:* none

---
## MSP2_INAV_OPFLOW_CALIBRATION

id `0x2032` (8242) · MSPv2 · group `inav`

since INAV 1.0

Starts the optical flow sensor calibration procedure.

> Requires `USE_OPFLOW`. Will fail if armed. Calls `opflowStartCalibration()`.

*request:* none

*reply:* none

---
## MSP2_INAV_FWUPDT_PREPARE

id `0x2033` (8243) · MSPv2 · group `inav`

since INAV 1.0

Prepares the flight controller to receive a firmware update via MSP.

> Requires `MSP_FIRMWARE_UPDATE`. Expects 4 bytes. Returns error if preparation fails (e.g., no storage, invalid size). Calls `firmwareUpdatePrepare()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| firmwareSize | `uint32` |   | Total size of the incoming firmware file in bytes |

*reply:* none

---
## MSP2_INAV_FWUPDT_STORE

id `0x2034` (8244) · MSPv2 · group `inav`

since INAV 1.0

Stores a chunk of firmware data received via MSP.

> Requires `MSP_FIRMWARE_UPDATE`. Returns error if storage fails (e.g., out of space, checksum error). Called repeatedly until the entire firmware is transferred. Calls `firmwareUpdateStore()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| firmwareChunk | `uint8[]` |   | Chunk of firmware data |

*reply:* none

---
## MSP2_INAV_FWUPDT_EXEC

id `0x2035` (8245) · MSPv2 · group `inav`

since INAV 1.0

Executes the firmware update process (flashes the stored firmware and reboots).

> Requires `MSP_FIRMWARE_UPDATE`. Expects 1 byte. Returns error if update cannot start (e.g., not fully received). Calls `firmwareUpdateExec()`. If successful, the device will reboot into the new firmware.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| updateType | `uint8` |   | Type of update (e.g., full flash, specific section - currently ignored/unused) |

*reply:* none

---
## MSP2_INAV_FWUPDT_ROLLBACK_PREPARE

id `0x2036` (8246) · MSPv2 · group `inav`

since INAV 1.0

Prepares the flight controller to perform a firmware rollback to the previously stored version.

> Requires `MSP_FIRMWARE_UPDATE`. Returns error if rollback preparation fails (e.g., no rollback image available). Calls `firmwareUpdateRollbackPrepare()`.

*request:* none

*reply:* none

---
## MSP2_INAV_FWUPDT_ROLLBACK_EXEC

id `0x2037` (8247) · MSPv2 · group `inav`

since INAV 1.0

Executes the firmware rollback process (flashes the stored backup firmware and reboots).

> Requires `MSP_FIRMWARE_UPDATE`. Returns error if rollback cannot start. Calls `firmwareUpdateRollbackExec()`. If successful, the device will reboot into the backup firmware.

*request:* none

*reply:* none

---
## MSP2_INAV_SAFEHOME

id `0x2038` (8248) · MSPv2 · group `inav`

since INAV 1.0

Get or Set configuration for a specific Safe Home location.

> Requires `USE_SAFE_HOME`. Used by `mspFcSafeHomeOutCommand`. See `MSP2_INAV_SET_SAFEHOME` for setting.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| safehomeIndex | `uint8` |   | Index of the safe home location (0 to `MAX_SAFE_HOMES - 1`) |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| safehomeIndex | `uint8` |   | Index requested |
| enabled | `uint8` |   | Boolean: 1 if this safe home is enabled |
| latitude | `int32` |   | Latitude (1e7 deg) |
| longitude | `int32` |   | Longitude (1e7 deg) |

---
## MSP2_INAV_SET_SAFEHOME

id `0x2039` (8249) · MSPv2 · group `inav`

since INAV 1.0

Sets the configuration for a specific Safe Home location.

> Requires `USE_SAFE_HOME`. Expects 10 bytes. Returns error if index invalid. Resets corresponding FW autoland approach if `USE_FW_AUTOLAND` is enabled.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| safehomeIndex | `uint8` |   | Index of the safe home location (0 to `MAX_SAFE_HOMES - 1`) |
| enabled | `uint8` |   | Boolean: 1 to enable this safe home |
| latitude | `int32` |   | Latitude (1e7 deg) |
| longitude | `int32` |   | Longitude (1e7 deg) |

*reply:* none

---
## MSP2_INAV_MISC2

id `0x203A` (8250) · MSPv2 · group `inav`

since INAV 1.0

Retrieves miscellaneous runtime information including timers and throttle status.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| uptimeSeconds | `uint32` |  Seconds | Time since boot (`micros() / 1000000`) |
| flightTimeSeconds | `uint32` |  Seconds | Accumulated flight time (`getFlightTime()`) |
| throttlePercent | `uint8` |  % | Current throttle output percentage (`getThrottlePercent(true)`) |
| autoThrottleFlag | `uint8` |  Boolean | 1 if navigation is controlling throttle, 0 otherwise (`navigationIsControllingThrottle()`) |

---
## MSP2_INAV_LOGIC_CONDITIONS_SINGLE

id `0x203B` (8251) · MSPv2 · group `inav`

since INAV 1.0

Gets the configuration for a single Logic Condition by its index.

> Requires `USE_PROGRAMMING_FRAMEWORK`. Used by `mspFcLogicConditionCommand`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| conditionIndex | `uint8` |   | Index of the condition to retrieve (0 to `MAX_LOGIC_CONDITIONS - 1`) |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| enabled | `uint8` |   | Boolean: 1 if enabled |
| activatorId | `int8` |   | Activator ID (-1/255 if none) |
| operation | `uint8` | `logicOperation_e`  | Enum `logicOperation_e` Logical operation |
| operandAType | `uint8` | `logicOperandType_e`  | Enum `logicOperandType_e` Type of operand A |
| operandAValue | `int32` |   | Value/ID of operand A |
| operandBType | `uint8` | `logicOperandType_e`  | Enum `logicOperandType_e` Type of operand B |
| operandBValue | `int32` |   | Value/ID of operand B |
| flags | `uint8` | `logicConditionFlags_e (bitmask)`  | Bitmask: Condition flags (`logicConditionFlags_e`) |

---
## MSP2_INAV_LOGIC_CONDITIONS_CONFIGURED

id `0x203C` (8252) · MSPv2 · group `inav`

since INAV 9.0

Returns a bitmask of which logic conditions are configured, so a client can fetch only the used slots instead of all of them.

> Requires `USE_PROGRAMMING_FRAMEWORK`. Fixed 8-byte reply carrying one 64-bit mask as two `uint32_t` halves, low half first. Only the first `MIN(MAX_LOGIC_CONDITIONS, 64)` bits are evaluated. A condition counts as configured when any of `enabled`, `activatorId` (default -1), `operation`, `operandA.type`, `operandA.value`, `operandB.type`, `operandB.value` or `flags` differs from its default.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| configuredMaskLow | `uint32` |   | Bits 0-31 of the bitmask. Bit N is set when logic condition N differs from its default values. |
| configuredMaskHigh | `uint32` |   | Bits 32-63 of the bitmask. Always 0 on targets where `MAX_LOGIC_CONDITIONS` is 32 or fewer. |

---
## MSP2_INAV_ESC_RPM

id `0x2040` (8256) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the RPM reported by each ESC via telemetry.

> Requires `USE_ESC_SENSOR`. Payload size depends on the number of detected motors with telemetry. Record count is not a constant: loops getMotorCount(); read until the payload is exhausted.

*request:* none

*reply:* (repeat: until_end)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| escRpm | `uint32` |  RPM | RPM reported by the ESC |

---
## MSP2_INAV_ESC_TELEM

id `0x2041` (8257) · MSPv2 · group `inav`

since INAV 8.0

Retrieves the full telemetry data structure reported by each ESC.

> Requires `USE_ESC_SENSOR`. See `escSensorData_t` in `sensors/esc_sensor.h` for the exact structure fields.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| motorCount | `uint8` |   | Number of motors reporting telemetry (`getMotorCount()`) |
| escData[] | *repeat: motorCount* | | |
|  esc | `escSensorData_t` |   | One ESC's telemetry (voltage, current, temperature, RPM, error count); see `escSensorData_t` |

---
## MSP2_INAV_DRONECAN_NODES

id `0x2042` (8258) · MSPv2 · group `inav`

since INAV 10.0

Returns the list of all detected DroneCAN nodes with their current status.

> Requires `USE_DRONECAN`. Response is `nodeCount` followed by `nodeCount` records of 13 bytes each: nodeID(1)+health(1)+mode(1)+last_seen_ms(4)+uptime_sec(4)+vendor_status_code(2). Maximum payload 1 + (DRONECAN_MAX_NODES * 13) = 417 bytes. For full node detail (name, SW/HW version, unique ID) use MSP2_INAV_DRONECAN_ASYNC_REQUEST with service_id=DRONECAN_SERVICE_GETNODEINFO(1).

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| nodeCount | `uint8` |   | Number of detected DroneCAN nodes |
| items[] | *repeat: nodeCount* | | |
|  nodeID | `uint8` |   | DroneCAN node ID (1-127) |
|  health | `uint8` |   | Node health: 0=OK, 1=WARNING, 2=ERROR, 3=CRITICAL |
|  mode | `uint8` |   | Node mode: 0=OPERATIONAL, 1=INITIALIZATION, 2=MAINTENANCE, 3=SOFTWARE_UPDATE, 7=OFFLINE |
|  last_seen_ms | `uint32` |  ms | Milliseconds since this node was last seen (FC-local timestamp delta) |
|  uptime_sec | `uint32` |  s | Node uptime in seconds (from NodeStatus broadcast) |
|  vendor_status_code | `uint16` |   | Vendor-specific status code |

---
## MSP2_INAV_DRONECAN_ASYNC_REQUEST

id `0x2043` (8259) · MSPv2 · group `inav`

since INAV 10.0

Initiates an asynchronous DroneCAN service request (GetNodeInfo, ParamGetSet, ExecuteOpcode, RestartNode) to a specific node. Result retrieved via MSP2_INAV_DRONECAN_ASYNC_RESULT.

> Requires `USE_DRONECAN`. Initiates an async DroneCAN service request; poll MSP2_INAV_DRONECAN_ASYNC_RESULT at ~100ms intervals until state=READY(2) or ERROR(3). Only one request in-flight at a time. Service-specific request fields follow the common header in the request payload: EXECUTE_OPCODE appends opcode(u8); PARAM_GETSET appends index(u16)+is_write(u8) and optionally value_type(u8)+value(variable) for writes, then req_name_len(u8)+req_name(bytes) for named lookup. Param value encoding: INT=lo(u32)+hi(u32), FLOAT=raw(u32), BOOL=u8, STRING=len(u8)+data. Requests time out after DRONECAN_ASYNC_TIMEOUT_MS (2000ms). If bus is not in STATE_DRONECAN_NORMAL, returns accepted=0xFF without dispatching.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| service_id | `uint16` |   | Service to invoke: 1=GETNODEINFO, 5=RESTART_NODE, 10=EXECUTE_OPCODE, 11=PARAM_GETSET. Transmitted as u16 for MSP alignment; only low 8 bits used. |
| nodeID | `uint8` |   | Target DroneCAN node ID (1-127) |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| accepted | `uint8` |   | 0=request accepted; 1=busy (slot in use) or unrecognised service_id; 0xFF=bus not in STATE_DRONECAN_NORMAL (not ready) |
| seq | `uint8` |   | Sequence number; correlate with MSP2_INAV_DRONECAN_ASYNC_RESULT to verify the result belongs to this request |

---
## MSP2_INAV_DRONECAN_ASYNC_RESULT

id `0x2044` (8260) · MSPv2 · group `inav`

since INAV 10.0

Polls the result of the most recent MSP2_INAV_DRONECAN_ASYNC_REQUEST. Poll at ~100ms intervals until state is READY(2) or ERROR(3).

> Requires `USE_DRONECAN`. When state=READY(2), service-specific result fields follow the 5-byte common header. GETNODEINFO: name_len(u8)+name(bytes)+sw_major(u8)+sw_minor(u8)+sw_optional_field_flags(u8)+sw_vcs_commit(u32)+hw_major(u8)+hw_minor(u8)+hw_unique_id(u8[16]). PARAM_GETSET: name_len(u8)+name(bytes)+type(u8)+value(variable)+min_type(u8)+min(variable)+max_type(u8)+max(variable); value/min/max encoding: INT=lo(u32)+hi(u32), FLOAT=raw(u32), BOOL=u8, STRING=len(u8)+data; EMPTY(0) min/max type means no bound is present. EXECUTE_OPCODE and RESTART_NODE: ok(u8) where 1=success. Reading result when state=READY transitions slot back to IDLE.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| state | `uint8` |   | Async slot state: 0=IDLE, 1=PENDING, 2=READY, 3=ERROR |
| seq | `uint8` |   | Sequence number matching the originating MSP2_INAV_DRONECAN_ASYNC_REQUEST reply |
| service_id | `uint16` |   | Service ID of the in-flight or just-completed request |
| node_id | `uint8` |   | Node ID of the target |

---
## MSP2_INAV_LED_STRIP_CONFIG_EX

id `0x2048` (8264) · MSPv2 · group `inav`

since INAV 1.0

Retrieves the full configuration for each LED on the strip using the `ledConfig_t` structure. Supersedes `MSP_LED_STRIP_CONFIG`.

> Requires `USE_LED_STRIP`. See `ledConfig_t` in `io/ledstrip.h` for structure fields (position, function, overlay, color, direction, params). `ledConfig_t` is a packed bitfield struct of 40 bits = 5 bytes (led_position:8, led_function:8, led_overlay:8, led_color:4, led_direction:6, led_params:6); the reply is `LED_MAX_STRIP_LENGTH` consecutive 5-byte records.

*request:* none

*reply:* (repeat: LED_MAX_STRIP_LENGTH)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| ledConfig | `ledConfig_t` |   | Raw `ledConfig_t` structure (5 bytes) holding position, function, overlay, color, direction, and params bitfields (`io/ledstrip.h`). |

---
## MSP2_INAV_SET_LED_STRIP_CONFIG_EX

id `0x2049` (8265) · MSPv2 · group `inav`

since INAV 1.0

Sets the configuration for a single LED on the strip using the `ledConfig_t` structure. Supersedes `MSP_SET_LED_STRIP_CONFIG`.

> Requires `USE_LED_STRIP`. Expects `1 + sizeof(ledConfig_t)` bytes. Returns error if index invalid. Calls `reevaluateLedConfig()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| ledIndex | `uint8` |   | Index of the LED to configure (0 to `LED_MAX_STRIP_LENGTH - 1`) |
| ledConfig | `ledConfig_t` |   | Raw `ledConfig_t` structure (6 bytes) mirroring the firmware layout. |

*reply:* none

---
## MSP2_INAV_FW_APPROACH

id `0x204A` (8266) · MSPv2 · group `inav`

since INAV 7.0

Get or Set configuration for a specific Fixed Wing Autoland approach.

> Requires `USE_FW_AUTOLAND`. Used by `mspFwApproachOutCommand`. See `MSP2_INAV_SET_FW_APPROACH` for setting.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| approachIndex | `uint8` |   | Index of the approach setting (0 to `MAX_FW_LAND_APPOACH_SETTINGS - 1`) |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| approachIndex | `uint8` |  Index | Index requested |
| approachAlt | `int32` |  cm | Signed altitude for the approach phase (`navFwAutolandApproach_t.approachAlt`) |
| landAlt | `int32` |  cm | Signed altitude for the final landing phase (`navFwAutolandApproach_t.landAlt`) |
| approachDirection | `uint8` | `fwAutolandApproachDirection_e`  | Enum `fwAutolandApproachDirection_e`: Direction of approach (From WP, Specific Heading) |
| landHeading1 | `int16` |  degrees | Primary landing heading (if approachDirection requires it) |
| landHeading2 | `int16` |  degrees | Secondary landing heading (if approachDirection requires it) |
| isSeaLevelRef | `uint8` |  Boolean | 1 if altitudes are relative to sea level, 0 if relative to home |

---
## MSP2_INAV_SET_FW_APPROACH

id `0x204B` (8267) · MSPv2 · group `inav`

since INAV 7.0

Sets the configuration for a specific Fixed Wing Autoland approach.

> Requires `USE_FW_AUTOLAND`. Expects 15 bytes. Returns error if index invalid.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| approachIndex | `uint8` |  Index | Index of the approach setting (0 to `MAX_FW_LAND_APPOACH_SETTINGS - 1`) |
| approachAlt | `int32` |  cm | Signed approach altitude (`navFwAutolandApproach_t.approachAlt`) |
| landAlt | `int32` |  cm | Signed landing altitude (`navFwAutolandApproach_t.landAlt`) |
| approachDirection | `uint8` | `fwAutolandApproachDirection_e`  | Enum `fwAutolandApproachDirection_e` Sets approach direction |
| landHeading1 | `int16` |  degrees | Sets primary landing heading |
| landHeading2 | `int16` |  degrees | Sets secondary landing heading |
| isSeaLevelRef | `uint8` |  Boolean | Sets altitude reference |

*reply:* none

---
## MSP2_INAV_GPS_UBLOX_COMMAND

id `0x2050` (8272) · MSPv2 · group `inav`

since INAV 8.0

Sends a raw command directly to a U-Blox GPS module connected to the FC.

> Requires GPS feature enabled (`FEATURE_GPS`) and the GPS driver to be U-Blox (`isGpsUblox()`). Payload must be at least 8 bytes (minimum UBX frame size). Use with extreme caution, incorrect commands can misconfigure the GPS module. Calls `gpsUbloxSendCommand()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| ubxCommand | `uint8[]` |   | Raw U-Blox UBX protocol command frame (including header, class, ID, length, payload, checksum) |

*reply:* none

---
## MSP2_INAV_RATE_DYNAMICS

id `0x2060` (8288) · MSPv2 · group `inav`

since INAV 7.0

Retrieves Rate Dynamics configuration parameters for the current control rate profile.

> Requires `USE_RATE_DYNAMICS`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| sensitivityCenter | `uint8` |  % | Sensitivity at stick center (`currentControlRateProfile->rateDynamics.sensitivityCenter`) |
| sensitivityEnd | `uint8` |  % | Sensitivity at stick ends (`currentControlRateProfile->rateDynamics.sensitivityEnd`) |
| correctionCenter | `uint8` |  % | Correction strength at stick center (`currentControlRateProfile->rateDynamics.correctionCenter`) |
| correctionEnd | `uint8` |  % | Correction strength at stick ends (`currentControlRateProfile->rateDynamics.correctionEnd`) |
| weightCenter | `uint8` |  % | Transition weight at stick center (`currentControlRateProfile->rateDynamics.weightCenter`) |
| weightEnd | `uint8` |  % | Transition weight at stick ends (`currentControlRateProfile->rateDynamics.weightEnd`) |

---
## MSP2_INAV_SET_RATE_DYNAMICS

id `0x2061` (8289) · MSPv2 · group `inav`

since INAV 7.0

Sets Rate Dynamics configuration parameters for the current control rate profile.

> Requires `USE_RATE_DYNAMICS`. Expects 6 bytes.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| sensitivityCenter | `uint8` |  % | Sets sensitivity at center |
| sensitivityEnd | `uint8` |  % | Sets sensitivity at ends |
| correctionCenter | `uint8` |  % | Sets correction at center |
| correctionEnd | `uint8` |  % | Sets correction at ends |
| weightCenter | `uint8` |  % | Sets weight at center |
| weightEnd | `uint8` |  % | Sets weight at ends |

*reply:* none

---
## MSP2_INAV_EZ_TUNE

id `0x2070` (8304) · MSPv2 · group `inav`

since INAV 7.0

Retrieves the current EZ-Tune parameters.

> Requires `USE_EZ_TUNE`. Calls `ezTuneUpdate()` before sending.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| enabled | `uint8` |   | Boolean: 1 if EZ-Tune is enabled (`ezTune()->enabled`) |
| filterHz | `uint16` |   | Filter frequency used during tuning (`ezTune()->filterHz`) |
| axisRatio | `uint8` |   | Roll vs Pitch axis tuning ratio (`ezTune()->axisRatio`) |
| response | `uint8` |   | Desired response characteristic (`ezTune()->response`) |
| damping | `uint8` |   | Desired damping characteristic (`ezTune()->damping`) |
| stability | `uint8` |   | Stability preference (`ezTune()->stability`) |
| aggressiveness | `uint8` |   | Aggressiveness preference (`ezTune()->aggressiveness`) |
| rate | `uint8` |   | Resulting rate setting (`ezTune()->rate`) |
| expo | `uint8` |   | Resulting expo setting (`ezTune()->expo`) |
| snappiness | `uint8` |   | Snappiness preference (`ezTune()->snappiness`) |

---
## MSP2_INAV_EZ_TUNE_SET

id `0x2071` (8305) · MSPv2 · group `inav`

since INAV 7.0

Sets the EZ-Tune parameters and triggers an update.

> Requires `USE_EZ_TUNE`. Expects 10 or 11 bytes. Calls `ezTuneUpdate()` after setting parameters.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| enabled | `uint8` |   | Sets enabled state |
| filterHz | `uint16` |   | Sets filter frequency |
| axisRatio | `uint8` |   | Sets axis ratio |
| response | `uint8` |   | Sets response characteristic |
| damping | `uint8` |   | Sets damping characteristic |
| stability | `uint8` |   | Sets stability preference |
| aggressiveness | `uint8` |   | Sets aggressiveness preference |
| rate | `uint8` |   | Sets rate setting |
| expo | `uint8` |   | Sets expo setting |
| snappiness | `optional uint8` |   | (Optional) Sets snappiness preference |

*reply:* none

---
## MSP2_INAV_SELECT_MIXER_PROFILE

id `0x2080` (8320) · MSPv2 · group `inav`

since INAV 7.0

Selects the active mixer profile and saves configuration.

> Expects 1 byte. Will fail if armed. Calls `setConfigMixerProfileAndWriteEEPROM()`. Only applicable if `MAX_MIXER_PROFILE_COUNT` > 1.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| mixerProfileIndex | `uint8` |   | Index of the mixer profile to activate (0-based) |

*reply:* none

---
## MSP2_ADSB_VEHICLE_LIST

id `0x2090` (8336) · MSPv2 · group `inav`

since INAV 8.0

Retrieves the list of currently tracked ADSB (Automatic Dependent Surveillance–Broadcast) vehicles. See `adsbVehicle_t` and `adsbVehicleValues_t` in `io/adsb.h` for the exact structure fields.

> Requires `USE_ADSB`. Only a subset of `adsbVehicle_t` is transmitted (callsign, core values, heading in whole degrees, TSLC, emitter type, TTL).

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| maxVehicles | `uint8` |   | Maximum number of vehicles tracked (`MAX_ADSB_VEHICLES`). 0 if `USE_ADSB` disabled |
| callsignLength | `uint8` |   | Maximum length of callsign string (`ADSB_CALL_SIGN_MAX_LENGTH`). 0 if `USE_ADSB` disabled |
| totalVehicleMsgs | `uint32` |   | Total vehicle messages received (`getAdsbStatus()->vehiclesMessagesTotal`). 0 if `USE_ADSB` disabled |
| totalHeartbeatMsgs | `uint32` |   | Total heartbeat messages received (`getAdsbStatus()->heartbeatMessagesTotal`). 0 if `USE_ADSB` disabled |
| items[] | *repeat: maxVehicles* | | |
|  callsign | `char[ADSB_CALL_SIGN_MAX_LENGTH]` |   | Fixed-length callsign from `adsbVehicle->vehicleValues.callsign` (padded with NULs if shorter). |
|  icao | `uint32` |   | ICAO address (`adsbVehicle->vehicleValues.icao`). |
|  lat | `int32` |  1e-7 deg | Latitude in degrees * 1e7 (`adsbVehicle->vehicleValues.gps.lat`). |
|  lon | `int32` |  1e-7 deg | Longitude in degrees * 1e7 (`adsbVehicle->vehicleValues.gps.lon`). |
|  alt | `int32` |  cm | Altitude above sea level (`adsbVehicle->vehicleValues.alt`). |
|  headingDeg | `uint16` |  deg | Course over ground in whole degrees (`CENTIDEGREES_TO_DEGREES(vehicleValues.heading)`). |
|  tslc | `uint8` |  s | Time since last communication (`adsbVehicle->vehicleValues.tslc`). |
|  emitterType | `uint8` |   | Emitter category (`adsbVehicle->vehicleValues.emitterType`) (refers to enum 'ADSB_EMITTER_TYPE', but none found) |
|  ttl | `uint8` |   | TTL counter used for list maintenance (`adsbVehicle->ttl`). |

---
## MSP2_ADSB_LIMITS

id `0x2091` (8337) · MSPv2 · group `inav`

since INAV 10.0

Retrieves the configured ADSB proximity distance limits used for OSD warnings and alerts.

> Requires `USE_ADSB`; all three fields are 0 when it is not compiled in.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| distanceWarning | `uint16` |  m | `osdConfig()->adsb_distance_warning` (setting `osd_adsb_distance_warning`). Distance within which an ADSB vehicle is displayed. |
| distanceAlert | `uint16` |  m | `osdConfig()->adsb_distance_alert` (setting `osd_adsb_distance_alert`). Distance inside which ADSB data flashes as a proximity warning. |
| ignorePlaneAboveMeLimit | `uint16` |  m | `osdConfig()->adsb_ignore_plane_above_me_limit` (setting `osd_adsb_ignore_plane_above_me_limit`). Vehicles higher than this above the craft are ignored; 0 disables the limit. |

---
## MSP2_ADSB_WARNING_VEHICLE_ICAO

id `0x2092` (8338) · MSPv2 · group `inav`

since INAV 10.0

Returns the ICAO address of the ADSB vehicle currently triggering a proximity warning or alert.

> Requires `USE_ADSB`. Alert takes priority: `findVehicleForAlert()` is tried first using `osd_adsb_distance_alert`, then `findVehicleForWarning()` using `osd_adsb_distance_warning`, both bounded by `osd_adsb_ignore_plane_above_me_limit`. Replies 0/0 when `USE_ADSB` is not compiled in, when `isEnvironmentOkForCalculatingADSBDistanceBearing()` is false, or when no vehicle matches.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| icao | `uint32` |   | ICAO address of the vehicle currently triggering an alert or warning; 0 when none applies. |
| isAlert | `uint8` |  Boolean | 1 when the vehicle matched the alert distance, 0 when it matched only the warning distance or when `icao` is 0. |

---
## MSP2_ADSB_VEHICLE

id `0x2093` (8339) · MSPv2 · group `inav`

since INAV 10.0

Retrieves a single tracked ADSB (Automatic Dependent Surveillance-Broadcast) vehicle by slot index. Intended for polling one slot at a time: query `MSP2_ADSB_VEHICLE_COUNT` for the iteration bound, then request indices `0 .. count-1`, skipping slots with `ttl == 0`, and identify each aircraft by its `icao`. See `adsbVehicle_t` / `adsbVehicleValues_t` in `io/adsb.h`.

> Requires `USE_ADSB`. Reads a single ADSB vehicle slot by index. THE INDEX IS NOT A STABLE HANDLE: slots are reused, so a given index may hold a different aircraft (or be empty, `ttl == 0`) between polls. Correlate aircraft by the `icao` field in the reply, never by index. Compared with the bulk `MSP2_ADSB_VEHICLE_LIST`, this message adds horizontal velocity and reports heading at full (centidegree) resolution, and orders the callsign last. Returns an error result for an out-of-range index.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| index | `uint8` |   | Slot index to read, `0 .. (MSP2_ADSB_VEHICLE_COUNT - 1)`. WARNING: this is an iteration cursor over fixed slots, NOT a stable identifier. The same index may return a different aircraft (or an empty slot) on a later poll. Always identify the aircraft by the `icao` field in the reply; never cache or correlate data by index. Returns an error result if the index is out of range. |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| icao | `uint32` |   | ICAO 24-bit address (`vehicleValues.icao`). This is the stable per-aircraft identifier; use it to correlate replies, not the request index. An empty slot reports `icao == 0` and `ttl == 0`. |
| lat | `int32` |  1e-7 deg | Latitude (`vehicleValues.gps.lat`). |
| lon | `int32` |  1e-7 deg | Longitude (`vehicleValues.gps.lon`). |
| alt | `int32` |  cm | Altitude above sea level (`vehicleValues.alt`). |
| heading | `uint16` |  1e-2 deg | Course over ground at full resolution (`vehicleValues.heading`). Unlike `MSP2_ADSB_VEHICLE_LIST`, this is in centidegrees, not whole degrees. |
| horVelocity | `uint16` |  cm/s | Horizontal (ground) speed (`vehicleValues.horVelocity`). Not present in `MSP2_ADSB_VEHICLE_LIST`. |
| tslc | `uint8` |  s | Time since last communication (`vehicleValues.tslc`). |
| emitterType | `uint8` |   | Emitter category (`vehicleValues.emitterType`). |
| ttl | `uint8` |  s | Remaining time-to-live for this slot (`adsbVehicle->ttl`). `ttl == 0` means the slot is empty/expired and its contents are stale; skip such entries. |
| callsign | `char[ADSB_CALL_SIGN_MAX_LENGTH]` |   | Fixed-length callsign (`vehicleValues.callsign`), padded with NULs if shorter. |

---
## MSP2_ADSB_VEHICLE_COUNT

id `0x2094` (8340) · MSPv2 · group `inav`

since INAV 10.0

Returns the number of ADSB vehicle slots available to iterate with `MSP2_ADSB_VEHICLE`.

> Requires `USE_ADSB`. Returns the iteration bound for `MSP2_ADSB_VEHICLE`: request indices `0 .. count-1` and skip any slot whose `ttl == 0`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| count | `uint8` |   | Number of vehicle slots to iterate (`MAX_ADSB_VEHICLES`). This is the slot capacity / iteration bound, not the number of currently active aircraft - some slots may be empty (`ttl == 0`). 0 if `USE_ADSB` is disabled. |

---
## MSP2_INAV_CUSTOM_OSD_ELEMENTS

id `0x2100` (8448) · MSPv2 · group `inav`

since INAV 7.0

Retrieves counts related to custom OSD elements defined by the programming framework.

> Requires `USE_PROGRAMMING_FRAMEWORK`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| maxElements | `uint8` |   | Maximum number of custom elements (`MAX_CUSTOM_ELEMENTS`) |
| maxTextLength | `uint8` |   | Maximum length of the text part (`OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1`) |
| maxParts | `uint8` |   | Maximum number of parts per element (`CUSTOM_ELEMENTS_PARTS`) |

---
## MSP2_INAV_CUSTOM_OSD_ELEMENT

id `0x2101` (8449) · MSPv2 · group `inav`

since INAV 8.0

Gets the configuration of a single custom OSD element defined by the programming framework.

> Reply emitted only if idx < MAX_CUSTOM_ELEMENTS; otherwise no body is written.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| elementIndex | `uint8` |   | Index of the custom element (0 to `MAX_CUSTOM_ELEMENTS - 1`) |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| items[] | *repeat: CUSTOM_ELEMENTS_PARTS* | | |
|  partType | `uint8` | `osdCustomElementType_e`  | Type of this part |
|  partValue | `uint16` |   | Value/ID associated with this part |
| visibilityType | `uint8` | `osdCustomElementTypeVisibility_e`  | Visibility condition source |
| visibilityValue | `uint16` |   | Value/ID of the visibility condition source |
| elementText | `char[15]` |   | Static text bytes |

---
## MSP2_INAV_SET_CUSTOM_OSD_ELEMENTS

id `0x2102` (8450) · MSPv2 · group `inav`

since INAV 7.0

Sets the configuration of one custom OSD element.

> Payload length must be (OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1) + (CUSTOM_ELEMENTS_PARTS * 3) + 4 bytes including elementIndex. elementIndex must be < MAX_CUSTOM_ELEMENTS. Each partType must be < CUSTOM_ELEMENT_TYPE_END. Firmware NUL-terminates elementText internally.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| elementIndex | `uint8` |   | Index of the custom element (0 to `MAX_CUSTOM_ELEMENTS - 1`) |
| items[] | *repeat: CUSTOM_ELEMENTS_PARTS* | | |
|  partType | `uint8` | `osdCustomElementType_e`  | Type of this part |
|  partValue | `uint16` |   | Value/ID associated with this part |
| visibilityType | `uint8` | `osdCustomElementTypeVisibility_e`  | Visibility condition source |
| visibilityValue | `uint16` |   | Value/ID of the visibility condition source |
| elementText | `char[15]` |   | Raw bytes |

*reply:* none

---
## MSP2_INAV_GET_LINK_STATS

id `0x2103` (8451) · MSPv2 · group `inav`

since INAV 9.0

Provides uplink RC link statistics for monitoring on a GCS.

> Useful for GCS monitoring of the active RC link quality and signal margin.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| uplinkRSSI_dBm | `uint8` |  -dBm | Uplink RSSI in dBm, sent as a positive magnitude (`getRSSI()`). For example, 70 means -70dBm. |
| uplinkLQ | `uint8` |  % | Uplink Link Quality (`rxLinkStatistics.uplinkLQ`) |
| uplinkSNR | `int8` |  dB | Uplink Signal-to-Noise Ratio (`rxLinkStatistics.uplinkSNR`) |

---
## MSP2_INAV_OUTPUT_MAPPING_EXT2

id `0x210D` (8461) · MSPv2 · group `inav`

since INAV 8.0

Retrieves the full extended output mapping configuration (timer ID, full 32-bit usage flags, and pin label). Supersedes `MSP2_INAV_OUTPUT_MAPPING_EXT`.

> Provides complete usage flags and helps identify pins repurposed for functions like LED strip. Record count is not a constant: loops timerHardwareCount, skipping timers flagged TIM_USE_PPM or TIM_USE_PWM; read until the payload is exhausted.

*request:* none

*reply:* (repeat: until_end)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| timerId | `uint8` |   | Hardware timer identifier (e.g., `TIM1`, `TIM2`). SITL uses index |
| usageFlags | `uint32` |   | Full 32-bit timer usage flags (`TIM_USE_*`) |
| pinLabel | `uint8` | `pinLabel_e`  | Label for special pin usage (`PIN_LABEL_*` enum, e.g., `PIN_LABEL_LED`). 0 (`PIN_LABEL_NONE`) otherwise |

---
## MSP2_INAV_OUTPUT_ASSIGNMENT

id `0x210E` (8462) · MSPv2 · group `inav`

since INAV 10.0

Returns the finalized post-boot mapping of timer outputs to motors, servos and the beeper.

> Not available on SITL builds (`#ifndef SITL_BUILD`). The reply is 3 bytes per assigned output with no leading count field: motors first (`maxTimMotorCount`), then servos (`maxTimServoCount`), then at most one beeper record, emitted only when some timer override is set to `OUTPUT_MODE_BEEPER`. Reads the assignment finalized at boot via `pwmGetOutputAssignment()`. Record count is not a constant: maxTimMotorCount motors, then maxTimServoCount servos, then at most one beeper entry (the loop breaks on the first match); read until the payload is exhausted.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| items[] | *repeat: until_end* | | |
|  outputIndex | `uint8` |  Index | Index into the target's `timerHardware[]` array for this output. |
|  usageType | `uint8` |   | Bit index of the `TIM_USE_*` flag, not the flag value itself: 2 = `TIM_USE_MOTOR`, 3 = `TIM_USE_SERVO`, 25 = `TIM_USE_BEEPER`. Derived in the firmware with `__builtin_ctz(TIM_USE_x)` and matching the `TIM_USE_*` constants in the configurator's `outputMapping.js`. |
|  functionIndex | `uint8` |  Index | 1-based ordinal within the usage type: motor 1..n, servo 1..n. Always 1 for the beeper entry. |

---
## MSP2_INAV_QUERY_OUTPUT_ASSIGNMENT

id `0x210F` (8463) · MSPv2 · group `inav`

since INAV 10.0

Previews the output assignment that would result from a proposed set of timer output-mode overrides, without applying them.

> Not available on SITL builds (`#ifndef SITL_BUILD`). Nothing is written to the configuration: `pwmCalculateAssignment()` is run against a proposed override array so a client can preview the effect of timer overrides before committing them with `MSP2_INAV_SET_TIMER_OUTPUT_MODE`. The reply has the same 3-byte record layout as `MSP2_INAV_OUTPUT_ASSIGNMENT`. Returns `MSP_RESULT_ERROR` if `timerCount` exceeds `HARDWARE_TIMER_DEFINITION_COUNT` or if the remaining request bytes are not exactly `timerCount * 2`. Pairs whose `timerId` is out of range are ignored rather than rejected. Record count is not a constant: maxTimMotorCount motors, then maxTimServoCount servos, then at most one beeper entry; read until the payload is exhausted.

**variant: dataSize == 0**

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| items[] | *repeat: until_end* | | |
|  outputIndex | `uint8` |  Index | Index into the target's `timerHardware[]` array for this output. |
|  usageType | `uint8` |   | Bit index of the `TIM_USE_*` flag, not the flag value itself: 2 = `TIM_USE_MOTOR`, 3 = `TIM_USE_SERVO`, 25 = `TIM_USE_BEEPER`. Derived in the firmware with `__builtin_ctz(TIM_USE_x)` and matching the `TIM_USE_*` constants in the configurator's `outputMapping.js`. |
|  functionIndex | `uint8` |  Index | 1-based ordinal within the usage type: motor 1..n, servo 1..n. Always 1 for the beeper entry. |

**variant: dataSize >= 1**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| timerCount | `uint8` |   | Number of override pairs that follow. Must be <= `HARDWARE_TIMER_DEFINITION_COUNT`. |
| items[] | *repeat: timerCount* | | |
|  timerId | `uint8` |  Index | Hardware timer index (0 to `HARDWARE_TIMER_DEFINITION_COUNT - 1`). Out-of-range values are silently skipped. |
|  outputMode | `uint8` | `outputMode_e`  | Proposed output mode override (`outputMode_e`) for that timer |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| items[] | *repeat: until_end* | | |
|  outputIndex | `uint8` |  Index | Index into the target's `timerHardware[]` array for this output. |
|  usageType | `uint8` |   | Bit index of the `TIM_USE_*` flag, not the flag value itself: 2 = `TIM_USE_MOTOR`, 3 = `TIM_USE_SERVO`, 25 = `TIM_USE_BEEPER`. Derived in the firmware with `__builtin_ctz(TIM_USE_x)` and matching the `TIM_USE_*` constants in the configurator's `outputMapping.js`. |
|  functionIndex | `uint8` |  Index | 1-based ordinal within the usage type: motor 1..n, servo 1..n. Always 1 for the beeper entry. |

---
## MSP2_INAV_OSD_UPDATE_POSITION

id `0x2118` (8472) · MSPv2 · group `inav`

since INAV 9.0

Moves a single OSD item within the active layout and redraws it immediately.

> Requires `USE_OSD`. Expects 3 bytes; returns `MSP_RESULT_ERROR` if fewer are supplied or if `itemIndex >= OSD_ITEM_COUNT`, otherwise `MSP_RESULT_ACK`. Writes to the currently active layout (`getCurrentLayout()`) and takes no layout argument; use `MSP2_INAV_OSD_SET_LAYOUT_ITEM` to address a specific layout. Erases the item at its old position and redraws it immediately rather than triggering a full OSD redraw; the erase step only clears custom elements (items 147-149 and 154-158), so moving other item types can leave the old glyphs on screen until the next full redraw.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| itemIndex | `uint8` |  Index | OSD item index (0 to `OSD_ITEM_COUNT - 1`) |
| itemPosition | `uint16` |  Coordinates | Packed X/Y position built with `OSD_POS(x, y)`. The firmware ORs in `OSD_VISIBLE_FLAG` (0x2000), so the item is always made visible regardless of the bit supplied. |

*reply:* none

---
## MSP2_INAV_SERVO_CONFIG

id `0x2200` (8704) · MSPv2 · group `inav`

since INAV 8.0

Retrieves the configuration parameters for all supported servos (min, max, middle, rate). Supersedes `MSP_SERVO_CONFIGURATIONS`.

*request:* none

*reply:* (repeat: MAX_SUPPORTED_SERVOS)

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| min | `int16` |  PWM | Minimum servo endpoint (`servoParams(i)->min`) |
| max | `int16` |  PWM | Maximum servo endpoint (`servoParams(i)->max`) |
| middle | `int16` |  PWM | Middle/Neutral servo position (`servoParams(i)->middle`) |
| rate | `int8` |  % (-125 to 125) | Servo rate/scaling (`servoParams(i)->rate`) |

---
## MSP2_INAV_SET_SERVO_CONFIG

id `0x2201` (8705) · MSPv2 · group `inav`

since INAV 8.0

Sets the configuration parameters for a single servo. Supersedes `MSP_SET_SERVO_CONFIGURATION`.

> Expects 8 bytes. Returns error if index invalid. Calls `servoComputeScalingFactors()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| servoIndex | `uint8` |  Index | Index of the servo to configure (0 to `MAX_SUPPORTED_SERVOS - 1`) |
| min | `int16` |  PWM | Sets minimum servo endpoint |
| max | `int16` |  PWM | Sets maximum servo endpoint |
| middle | `int16` |  PWM | Sets middle/neutral servo position |
| rate | `int8` |  % (-125 to 125) | Sets servo rate/scaling |

*reply:* none

---
## MSP2_INAV_GEOZONE

id `0x2210` (8720) · MSPv2 · group `inav`

since INAV 8.0

Get configuration for a specific Geozone.

> Requires `USE_GEOZONE`. Used by `mspFcGeozoneOutCommand`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| geozoneIndex | `uint8` |   | Index of the geozone (0 to `MAX_GEOZONES_IN_CONFIG - 1`) |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| geozoneIndex | `uint8` |   | Index requested |
| type | `uint8` |   | Define (`GEOZONE_TYPE_EXCLUSIVE/INCLUSIVE`): Zone type (Inclusion/Exclusion) |
| shape | `uint8` |   | Define (`GEOZONE_SHAPE_CIRCULAR/POLYGON`): Zone shape (Polygon/Circular) |
| minAltitude | `int32` |  cm | Minimum allowed altitude within the zone (`geoZonesConfig(idx)->minAltitude`) |
| maxAltitude | `int32` |  cm | Maximum allowed altitude within the zone (`geoZonesConfig(idx)->maxAltitude`) |
| isSeaLevelRef | `uint8` |   | Boolean: 1 if altitudes are relative to sea level, 0 if relative to home |
| fenceAction | `uint8` | `fenceAction_e`  | Enum (`fenceAction_e`): Action to take upon boundary violation |
| vertexCount | `uint8` |   | Number of vertices defined for this zone |

---
## MSP2_INAV_SET_GEOZONE

id `0x2211` (8721) · MSPv2 · group `inav`

since INAV 8.0

Sets the main configuration for a specific Geozone (type, shape, altitude, action). **This command resets (clears) all vertices associated with the zone.**

> Requires `USE_GEOZONE`. Expects 14 bytes. Returns error if index invalid. Calls `geozoneResetVertices()`. Vertices must be set subsequently using `MSP2_INAV_SET_GEOZONE_VERTEX`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| geozoneIndex | `uint8` |   | Index of the geozone (0 to `MAX_GEOZONES_IN_CONFIG - 1`) |
| type | `uint8` |   | Define (`GEOZONE_TYPE_EXCLUSIVE/INCLUSIVE`): Zone type (Inclusion/Exclusion) |
| shape | `uint8` |   | Define (`GEOZONE_SHAPE_CIRCULAR/POLYGON`): Zone shape (Polygon/Circular) |
| minAltitude | `int32` |  cm | Minimum allowed altitude (`geoZonesConfigMutable()->minAltitude`) |
| maxAltitude | `int32` |  cm | Maximum allowed altitude (`geoZonesConfigMutable()->maxAltitude`) |
| isSeaLevelRef | `uint8` |   | Boolean: Altitude reference |
| fenceAction | `uint8` | `fenceAction_e`  | Enum (`fenceAction_e`): Action to take upon boundary violation |
| vertexCount | `uint8` |   | Number of vertices to be defined (used for validation later) |

*reply:* none

---
## MSP2_INAV_GEOZONE_VERTEX

id `0x2212` (8722) · MSPv2 · group `inav`

since INAV 8.0

Get a specific vertex (or center+radius for circular zones) of a Geozone.

> Requires `USE_GEOZONE`. Returns error if indexes are invalid or vertex doesn't exist. For circular zones, the radius is stored internally as the 'latitude' of the vertex with index 1.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| geozoneIndex | `uint8` |   | Index of the geozone |
| vertexId | `uint8` |   | Index of the vertex within the zone (0-based). For circles, 0 = center |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| geozoneIndex | `uint8` |  Index | Geozone index requested |
| vertexId | `uint8` |  Index | Vertex index requested |
| latitude | `int32` |  deg * 1e7 | Vertex latitude |
| longitude | `int32` |  deg * 1e7 | Vertex longitude |
| radius | `optional int32` |  cm | If vertex is circle, Radius of the circular zone |

---
## MSP2_INAV_SET_GEOZONE_VERTEX

id `0x2213` (8723) · MSPv2 · group `inav`

since INAV 8.0

Sets a specific vertex (or center+radius for circular zones) for a Geozone.

> Requires `USE_GEOZONE`. Expects 10 bytes (Polygon) or 14 bytes (Circular). Returns error if indexes invalid or if trying to set vertex beyond `vertexCount` defined in `MSP2_INAV_SET_GEOZONE`. Calls `geozoneSetVertex()`. For circular zones, sets center (vertex 0) and radius (vertex 1's latitude).

**variant: polygon**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| geozoneIndex | `uint8` |  Index | Geozone index requested |
| vertexId | `uint8` |  Index | Vertex index requested |
| latitude | `int32` |  deg * 1e7 | Vertex latitude |
| longitude | `int32` |  deg * 1e7 | Vertex longitude |

*reply:* none

**variant: circle**

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| geozoneIndex | `uint8` |  Index | Geozone index requested |
| vertexId | `uint8` |  Index | Vertex index requested |
| latitude | `int32` |  deg * 1e7 | Vertex/Center latitude |
| longitude | `int32` |  deg * 1e7 | Vertex/Center longitude |
| radius | `int32` |  cm | Radius of the circular zone |

*reply:* none

---
## MSP2_INAV_SET_GVAR

id `0x2214` (8724) · MSPv2 · group `inav`

since INAV 9.0

Sets the specified Global Variable (GVAR) to the provided value.

> Requires `USE_PROGRAMMING_FRAMEWORK`. Expects 5 bytes. Returns error if index is outside `MAX_GLOBAL_VARIABLES`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| gvarIndex | `uint8` |  Index | Index of the Global Variable to set |
| value | `int32` |   | New value to store (clamped to configured min/max by `gvSet()`) |

*reply:* none

---
## MSP2_INAV_SET_ALT_TARGET

id `0x2215` (8725) · MSPv2 · group `inav`

since INAV 10.0

Set the active altitude hold target using updateClimbRateToAltitudeController.

> Set new altitude target. Requires 5-byte payload (datum + target) and is set-only. Valid only in NAV or ALTHOLD modes. Command is rejected unless altitude control is active, not landing/emergency landing, altitude estimation is valid, and datum is supported (MSL requires valid GPS origin; TERRAIN is reserved and rejected).

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| altitudeDatum | `uint8` | `geoAltitudeDatumFlag_e`  | Altitude reference datum flag (`geoAltitudeDatumFlag_e`): `NAV_WP_TAKEOFF_DATUM` (default), `NAV_WP_MSL_DATUM`, `NAV_WP_TERRAIN_DATUM` and `NAV_WP_RELATIVE_DATUM` (not implemented yet) |
| altitudeTarget | `int32` |  cm | Desired altitude target according to reference datum |

*reply:* none

---
## MSP2_INAV_FLIGHT_AXIS_ANGLE_OVERRIDE

id `0x2216` (8726) · MSPv2 · group `inav`

since INAV 10.0

Enables or disables a flight-axis angle override for the selected axis.

> Uses the same override path as logic conditions and bypasses stick-derived angle targets.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| overrideMask | `uint8` | `bitmask`  | Bitmask of desired-state fields that follow (Roll, Pitch, Yaw). Non-zero enables the override; zero disables it for that axis. |
| angleTargetRoll | `int16` |  deci-degrees | Angle target in deci-degrees. Roll/Pitch clamped to configured angle limits |
| angleTargetPitch | `int16` |  deci-degrees | Angle target in deci-degrees. Roll/Pitch clamped to configured angle limits |
| angleTargetYaw | `int16` |  deci-degrees | Angle target in deci-degrees. Yaw clamped to 0–3600. |

*reply:* none

---
## MSP2_INAV_FLIGHT_AXIS_RATE_OVERRIDE

id `0x2217` (8727) · MSPv2 · group `inav`

since INAV 10.0

Enables or disables a flight-axis rate override for the selected axis.

> Expects 7 bytes. Overrides rate targets just before control is applied, bypassing stick-derived setpoints.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| overrideMask | `uint8` | `bitmask`  | Bitmask of desired-state fields that follow (Roll, Pitch, Yaw). Non-zero enables the override; zero disables it for that axis. |
| rateTargetRoll | `int16` |  deg/s | Rate target, clamped to ±2000 |
| rateTargetPitch | `int16` |  deg/s | Rate target, clamped to ±2000 |
| rateTargetYaw | `int16` |  deg/s | Rate target, clamped to ±2000 |

*reply:* none

---
## MSP2_INAV_SET_LOCAL_TARGET

id `0x2218` (8728) · MSPv2 · group `inav`

since INAV 10.0

Sets a body-frame offset target relative to the current vehicle position.

> Offsets are in the vehicle body frame (forward/right/up, cm) and are rotated into the NEU frame using the current yaw, applied relative to current position. Z offset is always provided; Z=0 keeps current altitude, non-zero offsets are relative to current altitude. Requires GCSNAV/offboard to be active and a valid guided poshold; updates the navigation desired position via `setDesiredPosition()`.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| offsetForward | `int32` |  cm | Body-frame forward offset from the current position, rotated into NEU by the current yaw |
| offsetRight | `int32` |  cm | Body-frame right offset from the current position, rotated into NEU by the current yaw |
| offsetUp | `int32` |  cm | Offset above the current altitude (up-positive). 0 keeps the current altitude |

*reply:* none

---
## MSP2_INAV_LOCAL_TARGET

id `0x2219` (8729) · MSPv2 · group `inav`

since INAV 10.0

Returns the current navigation desired state (position, velocity, yaw, and climb rate).

> Local frame is NEU. Mirrors `posControl.desiredState` (position, velocity, yaw, climb rate) used by the position controller.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| posX | `int32` |  cm | Desired X in local NEU frame (`posControl.desiredState.pos.x`) |
| posY | `int32` |  cm | Desired Y in local NEU frame (`posControl.desiredState.pos.y`) |
| posZ | `int32` |  cm | Desired Z in local NEU frame (`posControl.desiredState.pos.z`, up-positive) |
| velX | `int16` |  cm/s | Desired X velocity (`posControl.desiredState.vel.x`) |
| velY | `int16` |  cm/s | Desired Y velocity (`posControl.desiredState.vel.y`) |
| velZ | `int16` |  cm/s | Desired Z velocity (`posControl.desiredState.vel.z`) |
| yaw | `int32` |  centi-degrees | Desired heading (`posControl.desiredState.yaw`) |
| climbRate | `int16` |  cm/s | Desired climb rate demand (`posControl.desiredState.climbRateDemand`) |

---
## MSP2_INAV_SET_GLOBAL_TARGET

id `0x221A` (8730) · MSPv2 · group `inav`

since INAV 10.0

Sets desired GCS Nav position with global coordinates (WP 254/GOTO).

> Uses the GCSNAV/offboard path; rejected when GCSNAV is not active. Rejects `NAV_WP_TERRAIN_DATUM`; other datums are converted to local NEU and applied through `setDesiredPosition()`. Altitude of 0 leaves current Z unchanged. Existing 13-byte payloads are still accepted; 17-byte payloads append `loiterRadius`, where `0` clears the temporary override and non-zero values are centimeters.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| latitude | `int32` |  deg * 1e7 | Latitude coordinate |
| longitude | `int32` |  deg * 1e7 | Longitude coordinate |
| altitudeTarget | `int32` |  cm | Desired altitude target according to reference datum (0 keeps current altitude) |
| altitudeDatum | `uint8` | `geoAltitudeDatumFlag_e`  | Altitude reference datum flag (`geoAltitudeDatumFlag_e`): `NAV_WP_TAKEOFF_DATUM`, `NAV_WP_MSL_DATUM`, `NAV_WP_TERRAIN_DATUM` (not implemented yet) |
| loiterRadius | `optional int32` |  cm | Optional temporary fixed-wing PosHold loiter radius override. Appended field; omit to leave unchanged. `0` clears the override and uses `navConfig()->fw.loiter_radius`. |

*reply:* none

---
## MSP2_INAV_NAV_TARGET

id `0x221B` (8731) · MSPv2 · group `inav`

since INAV 10.0

Returns the current navigation desired global target (lat/lon/alt, heading, climb rate).

> Altitude target is reported in the takeoff datum frame (local Z). Heading is sourced from the heading-hold target. Intended for monitoring the active navigation desired target (Goto/Followme/RTH/Safehome). The appended `loiterRadius` reports the temporary override only; `0` means the configured default is active.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| latTarget | `int32` |  1e-7 deg | Latitude in degrees * 1e7 |
| lonTarget | `int32` |  1e-7 deg | Longitude in degrees * 1e7 |
| altitudeTarget | `int32` |  cm | Desired altitude target (takeoff datum, cm) as used by altitude/position hold |
| headingTarget | `uint16` |  degrees | Current heading-hold target (`getHeadingHoldTarget()`), wrapped to 0–359.99 |
| climbRate | `int16` |  cm/s | Desired climb rate demand (`posControl.desiredState.climbRateDemand`) |
| loiterRadius | `uint32` |  cm | Temporary fixed-wing PosHold loiter radius override. `0` means no override; the configured `navConfig()->fw.loiter_radius` is used. |

---
## MSP2_INAV_FULL_LOCAL_POSE

id `0x2220` (8736) · MSPv2 · group `inav`

since INAV 9.0

Provides estimates of current attitude, local NEU position, and velocity.

> All attitude angles are in deci-degrees.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| roll | `int16` |  deci-degrees | Roll angle (`attitude.values.roll`) |
| pitch | `int16` |  deci-degrees | Pitch angle (`attitude.values.pitch`) |
| yaw | `int16` |  deci-degrees | Yaw/Heading angle (`attitude.values.yaw`) |
| localPositionNorth | `int32` |  cm | Estimated North coordinate in local NEU frame (`posControl.actualState.abs.pos.x`) |
| localVelocityNorth | `int16` |  cm/s | Estimated North component of velocity in local NEU frame (`posControl.actualState.abs.vel.x`) |
| localPositionEast | `int32` |  cm | Estimated East coordinate in local NEU frame (`posControl.actualState.abs.pos.y`) |
| localVelocityEast | `int16` |  cm/s | Estimated East component of velocity in local NEU frame (`posControl.actualState.abs.vel.y`) |
| localPositionUp | `int32` |  cm | Estimated Up coordinate in local NEU frame (`posControl.actualState.abs.pos.z`) |
| localVelocityUp | `int16` |  cm/s | Estimated Up component of velocity in local NEU frame (`posControl.actualState.abs.vel.z`) |

---
## MSP2_INAV_SET_WP_INDEX

id `0x2221` (8737) · MSPv2 · group `inav`

since INAV 9.0

Jumps to a specific waypoint during an active waypoint mission, causing the aircraft to immediately begin navigating toward the new target waypoint.

> Returns error if the aircraft is not armed, `NAV_WP_MODE` is not active, or the index is outside the valid mission range (`startWpIndex` to `startWpIndex + waypointCount - 1`). On success, sets `posControl.activeWaypointIndex` to the requested index and fires `NAV_FSM_EVENT_SWITCH_TO_WAYPOINT_JUMP`, transitioning the navigation FSM back to `NAV_STATE_WAYPOINT_PRE_ACTION` so the flight controller re-initialises navigation for the new target.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| wp_index | `uint8` |  - | 0-based waypoint index to jump to, relative to the mission start waypoint (`posControl.startWpIndex`) |

*reply:* none

---
## MSP2_INAV_SET_CRUISE_HEADING

id `0x2223` (8739) · MSPv2 · group `inav`

since INAV 9.0

Sets the course heading target while Cruise or Course Hold mode is active, causing the aircraft to turn to and maintain the new heading.

> Returns error if the aircraft is not armed or `NAV_COURSE_HOLD_MODE` is not active. On success, sets both `posControl.cruise.course` and `posControl.cruise.previousCourse` to the normalised value, preventing spurious heading adjustments from `getCruiseHeadingAdjustment()` on the next control cycle.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| heading_centidegrees | `int32` |  centidegrees | Target heading in centidegrees (0-35999). Values are wrapped modulo 36000 before being applied. |

*reply:* none

---
## MSP2_INAV_ACTIVATE_LANDING

id `0x2224` (8740) · MSPv2 · group `inav`

since INAV 10.0

Commands an immediate normal landing at the current position.

> Requires the aircraft to be armed with usable position, altitude, and heading estimates. Creates a transient LAND waypoint at the current position without changing the uploaded mission, then enters the normal `NAV_STATE_WAYPOINT_RTH_LAND` path. This is not emergency landing.

*request:* none

*reply:* none

---
## MSP2_INAV_ACTIVATE_RTH

id `0x2225` (8741) · MSPv2 · group `inav`

since INAV 10.0

Commands the aircraft to execute its configured return-to-home sequence.

> Requires the aircraft to be armed. Enters normal return-to-home mode through the same mode selector path as RC RTH, without setting the failsafe/geozone forced-RTH latch.

*request:* none

*reply:* none

---
## MSP2_INAV_ARM_DISARM

id `0x2227` (8743) · MSPv2 · group `inav`

since INAV 10.0

Arms or disarms the flight controller using the normal FC arming path.

> Returns an error for values other than 0 or 1, or when the requested armed state is not reached.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| arm | `uint8` |  Boolean | Requested armed state: 0 disarms, 1 arms through the normal arming checks. |

*reply:* none

---
## MSP2_INAV_TIMESYNC

id `0x2228` (8744) · MSPv2 · group `inav`

since INAV 10.0

Returns the local monotonic boot time in nanoseconds.

> The value is little-endian like other MSP integer fields and uses the same boot-time clock returned by MAVLink `TIMESYNC`.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| timeNs | `uint64` |  ns | Monotonic flight-controller boot time, calculated as `(uint64_t)micros() * 1000`. |

---
## MSP2_INAV_SET_AUX_RC

id `0x2230` (8752) · MSPv2 · group `inav`

since INAV 9.0

Bandwidth-efficient auxiliary RC channel update. Sets CH13-CH32 with configurable resolution (2/4/8/16-bit) without affecting primary flight controls. Designed for extending channel count beyond native RC link capacity via MSP passthrough.

> CH1-CH12 (index 0-11) are protected and will return `MSP_RESULT_ERROR`. Payload size must be 2-49 bytes. Constraint: `startChannel + channelCount <= 32`. Values persist until overwritten; no timeout. Applied as a post-RX overlay in `calculateRxChannelsAndUpdateFailsafe()` after MSP RC Override but before failsafe. Does not require `USE_RX_MSP` or MSP-RC-OVERRIDE flight mode. Does not affect failsafe detection. When MSP is the primary RX provider, channels covered by `MSP_SET_RAW_RC` are automatically skipped. Channels in the `mspOverrideChannels` bitmask are skipped when MSP RC Override mode is active. Recommended to send with `MSP_FLAG_DONT_REPLY` (flags=0x01) to save bandwidth on telemetry passthrough links. 16-bit mode requires even number of data bytes and values are clamped to 750-2250us.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| definitionByte | `uint8` |   | Packed start channel and resolution. Bits 7-3: start channel index (valid range 12-31 for CH13-CH32; 0-11 rejected as error). Bits 2-0: resolution mode (0=2-bit, 1=4-bit, 2=8-bit, 3=16-bit; 4-7 reserved/error). |
| channelData | `uint8[]` |  PWM (encoded) | Packed channel values, sequential from start channel. Number of channels is derived from data size and resolution. Value 0 means skip (no update). Sub-byte modes (2-bit, 4-bit) are packed MSB-first. 2-bit values 1-3 map to 1000/1500/2000us. 4-bit values 1-15 map to 1000 + (val-1)*1000/14 us. 8-bit values 1-255 map to 1000 + (val-1)*1000/254 us. 16-bit values are direct PWM, clamped to 750-2250us. |

*reply:* none

---
## MSP2_INAV_WIND

id `0x2231` (8753) · MSPv2 · group `inav`

since INAV 10.0

Retrieves the estimated horizontal wind speed and direction from the internal wind estimator.

> Requires `USE_WIND_ESTIMATOR`; returns zeroes when wind estimation is not compiled in or not yet valid. Check bit 0 of `flags` before using speed/angle values.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| windSpeed | `uint16` |  cm/s | Estimated horizontal wind speed (`getEstimatedHorizontalWindSpeed()`). 0 if unavailable. |
| windAngle | `uint16` |  degrees | Estimated wind direction in degrees (0–359, 0 = North). Derived from centidegree value divided by 100. 0 if unavailable. |
| flags | `uint8` |   | Validity flags. Bit 0: wind estimate valid (`isEstimatedWindSpeedValid()`). Remaining bits reserved. |

---
## MSP2_INAV_MAG_UNALIGNED

id `0x2232` (8754) · MSPv2 · group `inav`

since INAV 10.0

Reads the unaligned magnetometer vector.

> Returns rounded mag.magADCUnaligned values before board alignment. Returns three zeroes without USE_MAG.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| magADCUnaligned | `int16[3]` |   | X, Y and Z components, rounded to signed 16-bit values. |

---
## MSP2_INAV_ESC_SRXL2_STATUS

id `0x2233` (8755) · MSPv2 · group `inav`

since INAV 10.0

Reads SRXL2 ESC calibration and connection status.

> Requires USE_MOTOR_SRXL2. Counts report opened motor ports and the current mixer motor count, not hardware capacity.

*request:* none

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| phase | `uint8` |   | Calibration phase (srxl2CalPhase_e). |
| connected | `uint8` |   | 1 when every opened ESC is connected; otherwise 0. |
| lastResult | `uint8` |   | Last calibration start result (srxl2CalResult_e). |
| portCount | `uint8` |   | Number of opened SRXL2 motor ports. |
| motorCount | `uint8` |   | Number of motors in the current mixer. |

---
## MSP2_INAV_ESC_SRXL2_CALIBRATE

id `0x2234` (8756) · MSPv2 · group `inav`

since INAV 10.0

Controls SRXL2 ESC throttle-range calibration.

> Requires USE_MOTOR_SRXL2 and at least one request byte. Accepted commands: 0 abort, 1 automatic start, 4 manual high, 5 manual low. Driver safety checks can reject start requests; read MSP2_INAV_ESC_SRXL2_STATUS for the reason. Other command values return an MSP error. Remove propellers before calibration.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| phase | `uint8` |   | Requested calibration action (srxl2CalPhase_e): 0, 1, 4 or 5. |

*reply:* none

---
## MSP2_BETAFLIGHT_BIND

id `0x3000` (12288) · MSPv2 · group `common`

since INAV 8.0

Initiates the receiver binding procedure for supported serial protocols (CRSF, SRXL2).

> Requires `rxConfig()->receiverType == RX_TYPE_SERIAL`. Requires `USE_SERIALRX_CRSF` or `USE_SERIALRX_SRXL2`. Calls `crsfBind()` or `srxl2Bind()` respectively. Returns error if receiver type or provider is not supported for binding.

*request:* none

*reply:* none

---
## MSP2_RX_BIND

id `0x3001` (12289) · MSPv2 · group `common`

since INAV 9.0

Initiates binding for MSP receivers (mLRS).

> Requires a receiver using MSP as the protocol, sends MSP2_RX_BIND to the receiver.

*request:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| port_id | `uint8` |   | Port ID |
| reserved_for_custom_use | `uint8[3]` |   | Reserved for custom use |

*reply:*

| Field | Type | Enum / flags | Description |
|---|---|---|---|
| port_id | `uint8` |   | Port ID |
| reserved_for_custom_use | `uint8[3]` |   | Reserved for custom use |

