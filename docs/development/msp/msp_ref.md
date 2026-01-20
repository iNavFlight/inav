
# INAV MSP Messages reference
 
**This page is auto-generated from the [master INAV MSP definitions file](https://github.com/iNavFlight/inav/blob/master/docs/development/msp/msp_messages.json)**  

For details on the structure of MSP, see [The wiki page](https://github.com/iNavFlight/inav/wiki/MSP-V2)

For list of enums, see [Enum documentation page](https://github.com/iNavFlight/inav/wiki/Enums-reference)

For current generation code, see [documentation project](https://github.com/xznhj8129/msp_documentation) (temporary until official implementation)  


**JSON file rev: 4**

**Warning: Verification needed, exercise caution until completely verified for accuracy and cleared, especially for integer signs. Source-based generation/validation is forthcoming. Refer to source for absolute certainty** 

**If you find an error, it must be corrected in the JSON spec, not this markdown.**

**Guide:**

*   **MSP Versions:**
    *   **MSPv1:** The original protocol. Uses command IDs from 0 to 254.
    *   **MSPv2:** An extended version. Uses command IDs from 0x1000 onwards.
*   **Request Payload:** The request payload sent to the destination (usually the flight controller). May be empty or hold data for setting or requesting data from the FC. 
*   **Reply Payload:** The reply sent from the FC to the sender. May be empty or hold data.
*   **Notes:** Pay attention to message notes and description.

# Format:
## JSON format example:
```
    "MSP_API_VERSION": {
        "code": 1,
        "mspv": 1,
        "request": null,
        "reply": {
            "payload": [
                {
                    "name": "mspProtocolVersion",
                    "ctype": "uint8_t",
                    "units": "",
                    "desc": "MSP Protocol version (`MSP_PROTOCOL_VERSION`, typically 0)."
                },
                {
                    "name": "apiVersionMajor",
                    "ctype": "uint8_t",
                    "units": "",
                    "desc": "INAV API Major version (`API_VERSION_MAJOR`)."
                },
                {
                    "name": "apiVersionMinor",
                    "ctype": "uint8_t",
                    "units": "",
                    "desc": "INAV API Minor version (`API_VERSION_MINOR`)."
                }
            ],
        },
        "notes": "Used by configurators to check compatibility.",
        "description": "Provides the MSP protocol version and the INAV API version."
    },
```
## Message fields:
**name**: MSP message name\
**code**: Integer message code\
**description**: String with description of message\
**request**: null or dict of data sent\
**reply**: null or dict of data received\
**variable_len**: Optional boolean, if true, message does not have a predefined fixed length and needs appropriate handling\
**variants**: Optional special case, message has different cases of reply/request. Key/description is not a strict expression or code; just a readable condition\
**not_implemented**: Optional special case, message is not implemented (never or deprecated)\
**notes**: String with details of message

## Data dict fields:
**payload**: Array of payload fields\
**repeating**: Optional Special Case, integer or string of how many times the *entire* payload is repeated

## Payload fields:
### Fields:
**name**: field name from code\
**ctype**: Base C type of the value. Arrays list their element type here as well\
**desc**: Optional string with description and details of field\
**units**: Optional defined units\
**enum**: Optional string of enum struct if value is an enum
**array**: Optional boolean to denote field is array of more values\
**array_size**: If array, integer count of elements. Use `0` when the length is indeterminate/variable\
**array_size_define**: Optional string naming the source `#define` that provides the size (informational only)\
**repeating**: Optional Special case, contains array of more payload fields that are added Times * Key\
**payload**: If repeating, contains more payload fields\
**polymorph**: Optional boolean special case, field does not have a defined C type and could be anything

**Simple value**
```
{
    "name": "mspProtocolVersion",
    "ctype": "uint8_t",
    "units": "",
    "desc": "MSP Protocol version (`MSP_PROTOCOL_VERSION`, typically 0)."
},
```
**Fixed length array**
```
{
    "name": "fcVariantIdentifier",
    "ctype": "char",
    "desc": "4-character identifier string (e.g., \"INAV\"). Defined by `flightControllerIdentifier",
    "array": true,
    "array_size": 4,
    "units": ""
}
```
**Array sized via define**
```
{
    "name": "buildDate",
    "ctype": "char",
    "desc": "Build date string (e.g., \"Dec 31 2023\").",
    "array": true,
    "array_size": 11,
    "array_size_define": "BUILD_DATE_LENGTH",
    "units": ""
}
```
**Undefined length array**
```
{
    "name": "firmwareChunk",
    "ctype": "uint8_t",
    "desc": "Chunk of firmware data",
    "array": true,
    "array_size": 0,
}
```
**As of yet unknown length array**
```
{
    "name": "elementText",
    "ctype": "char",
    "desc": "Static text bytes, not NUL-terminated and not yet sized.",
    "array": true,
    "array_size": 0
}
```
**Nested array with struct**
```
{
    "repeating": "maxVehicles",
    "payload": [
        {
            "name": "adsbVehicle",
            "ctype": "adsbVehicle_t",
            "desc": "Array of `adsbVehicle_t` Repeated `maxVehicles` times",
            "repeating": "maxVehicles",
            "array": true,
            "array_size": 0,
            "units": ""
        }
    ]
}
```


---

## Index
### MSPv1
[1 - MSP_API_VERSION](#msp_api_version)  
[2 - MSP_FC_VARIANT](#msp_fc_variant)  
[3 - MSP_FC_VERSION](#msp_fc_version)  
[4 - MSP_BOARD_INFO](#msp_board_info)  
[5 - MSP_BUILD_INFO](#msp_build_info)  
[6 - MSP_INAV_PID](#msp_inav_pid)  
[7 - MSP_SET_INAV_PID](#msp_set_inav_pid)  
[10 - MSP_NAME](#msp_name)  
[11 - MSP_SET_NAME](#msp_set_name)  
[12 - MSP_NAV_POSHOLD](#msp_nav_poshold)  
[13 - MSP_SET_NAV_POSHOLD](#msp_set_nav_poshold)  
[14 - MSP_CALIBRATION_DATA](#msp_calibration_data)  
[15 - MSP_SET_CALIBRATION_DATA](#msp_set_calibration_data)  
[16 - MSP_POSITION_ESTIMATION_CONFIG](#msp_position_estimation_config)  
[17 - MSP_SET_POSITION_ESTIMATION_CONFIG](#msp_set_position_estimation_config)  
[18 - MSP_WP_MISSION_LOAD](#msp_wp_mission_load)  
[19 - MSP_WP_MISSION_SAVE](#msp_wp_mission_save)  
[20 - MSP_WP_GETINFO](#msp_wp_getinfo)  
[21 - MSP_RTH_AND_LAND_CONFIG](#msp_rth_and_land_config)  
[22 - MSP_SET_RTH_AND_LAND_CONFIG](#msp_set_rth_and_land_config)  
[23 - MSP_FW_CONFIG](#msp_fw_config)  
[24 - MSP_SET_FW_CONFIG](#msp_set_fw_config)  
[34 - MSP_MODE_RANGES](#msp_mode_ranges)  
[35 - MSP_SET_MODE_RANGE](#msp_set_mode_range)  
[36 - MSP_FEATURE](#msp_feature)  
[37 - MSP_SET_FEATURE](#msp_set_feature)  
[38 - MSP_BOARD_ALIGNMENT](#msp_board_alignment)  
[39 - MSP_SET_BOARD_ALIGNMENT](#msp_set_board_alignment)  
[40 - MSP_CURRENT_METER_CONFIG](#msp_current_meter_config)  
[41 - MSP_SET_CURRENT_METER_CONFIG](#msp_set_current_meter_config)  
[42 - MSP_MIXER](#msp_mixer)  
[43 - MSP_SET_MIXER](#msp_set_mixer)  
[44 - MSP_RX_CONFIG](#msp_rx_config)  
[45 - MSP_SET_RX_CONFIG](#msp_set_rx_config)  
[46 - MSP_LED_COLORS](#msp_led_colors)  
[47 - MSP_SET_LED_COLORS](#msp_set_led_colors)  
[48 - MSP_LED_STRIP_CONFIG](#msp_led_strip_config)  
[49 - MSP_SET_LED_STRIP_CONFIG](#msp_set_led_strip_config)  
[50 - MSP_RSSI_CONFIG](#msp_rssi_config)  
[51 - MSP_SET_RSSI_CONFIG](#msp_set_rssi_config)  
[52 - MSP_ADJUSTMENT_RANGES](#msp_adjustment_ranges)  
[53 - MSP_SET_ADJUSTMENT_RANGE](#msp_set_adjustment_range)  
[54 - MSP_CF_SERIAL_CONFIG](#msp_cf_serial_config)  
[55 - MSP_SET_CF_SERIAL_CONFIG](#msp_set_cf_serial_config)  
[56 - MSP_VOLTAGE_METER_CONFIG](#msp_voltage_meter_config)  
[57 - MSP_SET_VOLTAGE_METER_CONFIG](#msp_set_voltage_meter_config)  
[58 - MSP_SONAR_ALTITUDE](#msp_sonar_altitude)  
[64 - MSP_RX_MAP](#msp_rx_map)  
[65 - MSP_SET_RX_MAP](#msp_set_rx_map)  
[68 - MSP_REBOOT](#msp_reboot)  
[70 - MSP_DATAFLASH_SUMMARY](#msp_dataflash_summary)  
[71 - MSP_DATAFLASH_READ](#msp_dataflash_read)  
[72 - MSP_DATAFLASH_ERASE](#msp_dataflash_erase)  
[73 - MSP_LOOP_TIME](#msp_loop_time)  
[74 - MSP_SET_LOOP_TIME](#msp_set_loop_time)  
[75 - MSP_FAILSAFE_CONFIG](#msp_failsafe_config)  
[76 - MSP_SET_FAILSAFE_CONFIG](#msp_set_failsafe_config)  
[79 - MSP_SDCARD_SUMMARY](#msp_sdcard_summary)  
[80 - MSP_BLACKBOX_CONFIG](#msp_blackbox_config)  
[81 - MSP_SET_BLACKBOX_CONFIG](#msp_set_blackbox_config)  
[82 - MSP_TRANSPONDER_CONFIG](#msp_transponder_config)  
[83 - MSP_SET_TRANSPONDER_CONFIG](#msp_set_transponder_config)  
[84 - MSP_OSD_CONFIG](#msp_osd_config)  
[85 - MSP_SET_OSD_CONFIG](#msp_set_osd_config)  
[86 - MSP_OSD_CHAR_READ](#msp_osd_char_read)  
[87 - MSP_OSD_CHAR_WRITE](#msp_osd_char_write)  
[88 - MSP_VTX_CONFIG](#msp_vtx_config)  
[89 - MSP_SET_VTX_CONFIG](#msp_set_vtx_config)  
[90 - MSP_ADVANCED_CONFIG](#msp_advanced_config)  
[91 - MSP_SET_ADVANCED_CONFIG](#msp_set_advanced_config)  
[92 - MSP_FILTER_CONFIG](#msp_filter_config)  
[93 - MSP_SET_FILTER_CONFIG](#msp_set_filter_config)  
[94 - MSP_PID_ADVANCED](#msp_pid_advanced)  
[95 - MSP_SET_PID_ADVANCED](#msp_set_pid_advanced)  
[96 - MSP_SENSOR_CONFIG](#msp_sensor_config)  
[97 - MSP_SET_SENSOR_CONFIG](#msp_set_sensor_config)  
[98 - MSP_SPECIAL_PARAMETERS](#msp_special_parameters)  
[99 - MSP_SET_SPECIAL_PARAMETERS](#msp_set_special_parameters)  
[100 - MSP_IDENT](#msp_ident)  
[101 - MSP_STATUS](#msp_status)  
[102 - MSP_RAW_IMU](#msp_raw_imu)  
[103 - MSP_SERVO](#msp_servo)  
[104 - MSP_MOTOR](#msp_motor)  
[105 - MSP_RC](#msp_rc)  
[106 - MSP_RAW_GPS](#msp_raw_gps)  
[107 - MSP_COMP_GPS](#msp_comp_gps)  
[108 - MSP_ATTITUDE](#msp_attitude)  
[109 - MSP_ALTITUDE](#msp_altitude)  
[110 - MSP_ANALOG](#msp_analog)  
[111 - MSP_RC_TUNING](#msp_rc_tuning)  
[113 - MSP_ACTIVEBOXES](#msp_activeboxes)  
[114 - MSP_MISC](#msp_misc)  
[116 - MSP_BOXNAMES](#msp_boxnames)  
[117 - MSP_PIDNAMES](#msp_pidnames)  
[118 - MSP_WP](#msp_wp)  
[119 - MSP_BOXIDS](#msp_boxids)  
[120 - MSP_SERVO_CONFIGURATIONS](#msp_servo_configurations)  
[121 - MSP_NAV_STATUS](#msp_nav_status)  
[122 - MSP_NAV_CONFIG](#msp_nav_config)  
[124 - MSP_3D](#msp_3d)  
[125 - MSP_RC_DEADBAND](#msp_rc_deadband)  
[126 - MSP_SENSOR_ALIGNMENT](#msp_sensor_alignment)  
[127 - MSP_LED_STRIP_MODECOLOR](#msp_led_strip_modecolor)  
[130 - MSP_BATTERY_STATE](#msp_battery_state)  
[137 - MSP_VTXTABLE_BAND](#msp_vtxtable_band)  
[138 - MSP_VTXTABLE_POWERLEVEL](#msp_vtxtable_powerlevel)  
[150 - MSP_STATUS_EX](#msp_status_ex)  
[151 - MSP_SENSOR_STATUS](#msp_sensor_status)  
[160 - MSP_UID](#msp_uid)  
[164 - MSP_GPSSVINFO](#msp_gpssvinfo)  
[166 - MSP_GPSSTATISTICS](#msp_gpsstatistics)  
[180 - MSP_OSD_VIDEO_CONFIG](#msp_osd_video_config)  
[181 - MSP_SET_OSD_VIDEO_CONFIG](#msp_set_osd_video_config)  
[182 - MSP_DISPLAYPORT](#msp_displayport)  
[186 - MSP_SET_TX_INFO](#msp_set_tx_info)  
[187 - MSP_TX_INFO](#msp_tx_info)  
[200 - MSP_SET_RAW_RC](#msp_set_raw_rc)  
[201 - MSP_SET_RAW_GPS](#msp_set_raw_gps)  
[203 - MSP_SET_BOX](#msp_set_box)  
[204 - MSP_SET_RC_TUNING](#msp_set_rc_tuning)  
[205 - MSP_ACC_CALIBRATION](#msp_acc_calibration)  
[206 - MSP_MAG_CALIBRATION](#msp_mag_calibration)  
[207 - MSP_SET_MISC](#msp_set_misc)  
[208 - MSP_RESET_CONF](#msp_reset_conf)  
[209 - MSP_SET_WP](#msp_set_wp)  
[210 - MSP_SELECT_SETTING](#msp_select_setting)  
[211 - MSP_SET_HEAD](#msp_set_head)  
[212 - MSP_SET_SERVO_CONFIGURATION](#msp_set_servo_configuration)  
[214 - MSP_SET_MOTOR](#msp_set_motor)  
[215 - MSP_SET_NAV_CONFIG](#msp_set_nav_config)  
[217 - MSP_SET_3D](#msp_set_3d)  
[218 - MSP_SET_RC_DEADBAND](#msp_set_rc_deadband)  
[219 - MSP_SET_RESET_CURR_PID](#msp_set_reset_curr_pid)  
[220 - MSP_SET_SENSOR_ALIGNMENT](#msp_set_sensor_alignment)  
[221 - MSP_SET_LED_STRIP_MODECOLOR](#msp_set_led_strip_modecolor)  
[239 - MSP_SET_ACC_TRIM](#msp_set_acc_trim)  
[240 - MSP_ACC_TRIM](#msp_acc_trim)  
[241 - MSP_SERVO_MIX_RULES](#msp_servo_mix_rules)  
[242 - MSP_SET_SERVO_MIX_RULE](#msp_set_servo_mix_rule)  
[245 - MSP_SET_PASSTHROUGH](#msp_set_passthrough)  
[246 - MSP_RTC](#msp_rtc)  
[247 - MSP_SET_RTC](#msp_set_rtc)  
[250 - MSP_EEPROM_WRITE](#msp_eeprom_write)  
[251 - MSP_RESERVE_1](#msp_reserve_1)  
[252 - MSP_RESERVE_2](#msp_reserve_2)  
[253 - MSP_DEBUGMSG](#msp_debugmsg)  
[254 - MSP_DEBUG](#msp_debug)  

### MSPv2
[4097 - MSP2_COMMON_TZ](#msp2_common_tz)  
[4098 - MSP2_COMMON_SET_TZ](#msp2_common_set_tz)  
[4099 - MSP2_COMMON_SETTING](#msp2_common_setting)  
[4100 - MSP2_COMMON_SET_SETTING](#msp2_common_set_setting)  
[4101 - MSP2_COMMON_MOTOR_MIXER](#msp2_common_motor_mixer)  
[4102 - MSP2_COMMON_SET_MOTOR_MIXER](#msp2_common_set_motor_mixer)  
[4103 - MSP2_COMMON_SETTING_INFO](#msp2_common_setting_info)  
[4104 - MSP2_COMMON_PG_LIST](#msp2_common_pg_list)  
[4105 - MSP2_COMMON_SERIAL_CONFIG](#msp2_common_serial_config)  
[4106 - MSP2_COMMON_SET_SERIAL_CONFIG](#msp2_common_set_serial_config)  
[4107 - MSP2_COMMON_SET_RADAR_POS](#msp2_common_set_radar_pos)  
[4108 - MSP2_COMMON_SET_RADAR_ITD](#msp2_common_set_radar_itd)  
[4109 - MSP2_COMMON_SET_MSP_RC_LINK_STATS](#msp2_common_set_msp_rc_link_stats)  
[4110 - MSP2_COMMON_SET_MSP_RC_INFO](#msp2_common_set_msp_rc_info)  
[4111 - MSP2_COMMON_GET_RADAR_GPS](#msp2_common_get_radar_gps)  
[7937 - MSP2_SENSOR_RANGEFINDER](#msp2_sensor_rangefinder)  
[7938 - MSP2_SENSOR_OPTIC_FLOW](#msp2_sensor_optic_flow)  
[7939 - MSP2_SENSOR_GPS](#msp2_sensor_gps)  
[7940 - MSP2_SENSOR_COMPASS](#msp2_sensor_compass)  
[7941 - MSP2_SENSOR_BAROMETER](#msp2_sensor_barometer)  
[7942 - MSP2_SENSOR_AIRSPEED](#msp2_sensor_airspeed)  
[7943 - MSP2_SENSOR_HEADTRACKER](#msp2_sensor_headtracker)  
[8192 - MSP2_INAV_STATUS](#msp2_inav_status)  
[8193 - MSP2_INAV_OPTICAL_FLOW](#msp2_inav_optical_flow)  
[8194 - MSP2_INAV_ANALOG](#msp2_inav_analog)  
[8195 - MSP2_INAV_MISC](#msp2_inav_misc)  
[8196 - MSP2_INAV_SET_MISC](#msp2_inav_set_misc)  
[8197 - MSP2_INAV_BATTERY_CONFIG](#msp2_inav_battery_config)  
[8198 - MSP2_INAV_SET_BATTERY_CONFIG](#msp2_inav_set_battery_config)  
[8199 - MSP2_INAV_RATE_PROFILE](#msp2_inav_rate_profile)  
[8200 - MSP2_INAV_SET_RATE_PROFILE](#msp2_inav_set_rate_profile)  
[8201 - MSP2_INAV_AIR_SPEED](#msp2_inav_air_speed)  
[8202 - MSP2_INAV_OUTPUT_MAPPING](#msp2_inav_output_mapping)  
[8203 - MSP2_INAV_MC_BRAKING](#msp2_inav_mc_braking)  
[8204 - MSP2_INAV_SET_MC_BRAKING](#msp2_inav_set_mc_braking)  
[8205 - MSP2_INAV_OUTPUT_MAPPING_EXT](#msp2_inav_output_mapping_ext)  
[8206 - MSP2_INAV_TIMER_OUTPUT_MODE](#msp2_inav_timer_output_mode)  
[8207 - MSP2_INAV_SET_TIMER_OUTPUT_MODE](#msp2_inav_set_timer_output_mode)  
[8208 - MSP2_INAV_MIXER](#msp2_inav_mixer)  
[8209 - MSP2_INAV_SET_MIXER](#msp2_inav_set_mixer)  
[8210 - MSP2_INAV_OSD_LAYOUTS](#msp2_inav_osd_layouts)  
[8211 - MSP2_INAV_OSD_SET_LAYOUT_ITEM](#msp2_inav_osd_set_layout_item)  
[8212 - MSP2_INAV_OSD_ALARMS](#msp2_inav_osd_alarms)  
[8213 - MSP2_INAV_OSD_SET_ALARMS](#msp2_inav_osd_set_alarms)  
[8214 - MSP2_INAV_OSD_PREFERENCES](#msp2_inav_osd_preferences)  
[8215 - MSP2_INAV_OSD_SET_PREFERENCES](#msp2_inav_osd_set_preferences)  
[8216 - MSP2_INAV_SELECT_BATTERY_PROFILE](#msp2_inav_select_battery_profile)  
[8217 - MSP2_INAV_DEBUG](#msp2_inav_debug)  
[8218 - MSP2_BLACKBOX_CONFIG](#msp2_blackbox_config)  
[8219 - MSP2_SET_BLACKBOX_CONFIG](#msp2_set_blackbox_config)  
[8220 - MSP2_INAV_TEMP_SENSOR_CONFIG](#msp2_inav_temp_sensor_config)  
[8221 - MSP2_INAV_SET_TEMP_SENSOR_CONFIG](#msp2_inav_set_temp_sensor_config)  
[8222 - MSP2_INAV_TEMPERATURES](#msp2_inav_temperatures)  
[8223 - MSP_SIMULATOR](#msp_simulator)  
[8224 - MSP2_INAV_SERVO_MIXER](#msp2_inav_servo_mixer)  
[8225 - MSP2_INAV_SET_SERVO_MIXER](#msp2_inav_set_servo_mixer)  
[8226 - MSP2_INAV_LOGIC_CONDITIONS](#msp2_inav_logic_conditions)  
[8227 - MSP2_INAV_SET_LOGIC_CONDITIONS](#msp2_inav_set_logic_conditions)  
[8228 - MSP2_INAV_GLOBAL_FUNCTIONS](#msp2_inav_global_functions)  
[8229 - MSP2_INAV_SET_GLOBAL_FUNCTIONS](#msp2_inav_set_global_functions)  
[8230 - MSP2_INAV_LOGIC_CONDITIONS_STATUS](#msp2_inav_logic_conditions_status)  
[8231 - MSP2_INAV_GVAR_STATUS](#msp2_inav_gvar_status)  
[8232 - MSP2_INAV_PROGRAMMING_PID](#msp2_inav_programming_pid)  
[8233 - MSP2_INAV_SET_PROGRAMMING_PID](#msp2_inav_set_programming_pid)  
[8234 - MSP2_INAV_PROGRAMMING_PID_STATUS](#msp2_inav_programming_pid_status)  
[8240 - MSP2_PID](#msp2_pid)  
[8241 - MSP2_SET_PID](#msp2_set_pid)  
[8242 - MSP2_INAV_OPFLOW_CALIBRATION](#msp2_inav_opflow_calibration)  
[8243 - MSP2_INAV_FWUPDT_PREPARE](#msp2_inav_fwupdt_prepare)  
[8244 - MSP2_INAV_FWUPDT_STORE](#msp2_inav_fwupdt_store)  
[8245 - MSP2_INAV_FWUPDT_EXEC](#msp2_inav_fwupdt_exec)  
[8246 - MSP2_INAV_FWUPDT_ROLLBACK_PREPARE](#msp2_inav_fwupdt_rollback_prepare)  
[8247 - MSP2_INAV_FWUPDT_ROLLBACK_EXEC](#msp2_inav_fwupdt_rollback_exec)  
[8248 - MSP2_INAV_SAFEHOME](#msp2_inav_safehome)  
[8249 - MSP2_INAV_SET_SAFEHOME](#msp2_inav_set_safehome)  
[8250 - MSP2_INAV_MISC2](#msp2_inav_misc2)  
[8251 - MSP2_INAV_LOGIC_CONDITIONS_SINGLE](#msp2_inav_logic_conditions_single)  
[8256 - MSP2_INAV_ESC_RPM](#msp2_inav_esc_rpm)  
[8257 - MSP2_INAV_ESC_TELEM](#msp2_inav_esc_telem)  
[8264 - MSP2_INAV_LED_STRIP_CONFIG_EX](#msp2_inav_led_strip_config_ex)  
[8265 - MSP2_INAV_SET_LED_STRIP_CONFIG_EX](#msp2_inav_set_led_strip_config_ex)  
[8266 - MSP2_INAV_FW_APPROACH](#msp2_inav_fw_approach)  
[8267 - MSP2_INAV_SET_FW_APPROACH](#msp2_inav_set_fw_approach)  
[8272 - MSP2_INAV_GPS_UBLOX_COMMAND](#msp2_inav_gps_ublox_command)  
[8288 - MSP2_INAV_RATE_DYNAMICS](#msp2_inav_rate_dynamics)  
[8289 - MSP2_INAV_SET_RATE_DYNAMICS](#msp2_inav_set_rate_dynamics)  
[8304 - MSP2_INAV_EZ_TUNE](#msp2_inav_ez_tune)  
[8305 - MSP2_INAV_EZ_TUNE_SET](#msp2_inav_ez_tune_set)  
[8320 - MSP2_INAV_SELECT_MIXER_PROFILE](#msp2_inav_select_mixer_profile)  
[8336 - MSP2_ADSB_VEHICLE_LIST](#msp2_adsb_vehicle_list)  
[8448 - MSP2_INAV_CUSTOM_OSD_ELEMENTS](#msp2_inav_custom_osd_elements)  
[8449 - MSP2_INAV_CUSTOM_OSD_ELEMENT](#msp2_inav_custom_osd_element)  
[8450 - MSP2_INAV_SET_CUSTOM_OSD_ELEMENTS](#msp2_inav_set_custom_osd_elements)  
[8461 - MSP2_INAV_OUTPUT_MAPPING_EXT2](#msp2_inav_output_mapping_ext2)  
[8704 - MSP2_INAV_SERVO_CONFIG](#msp2_inav_servo_config)  
[8705 - MSP2_INAV_SET_SERVO_CONFIG](#msp2_inav_set_servo_config)  
[8720 - MSP2_INAV_GEOZONE](#msp2_inav_geozone)  
[8721 - MSP2_INAV_SET_GEOZONE](#msp2_inav_set_geozone)  
[8722 - MSP2_INAV_GEOZONE_VERTEX](#msp2_inav_geozone_vertex)  
[8723 - MSP2_INAV_SET_GEOZONE_VERTEX](#msp2_inav_set_geozone_vertex)  
[8724 - MSP2_INAV_SET_GVAR](#msp2_inav_set_gvar)  
[8736 - MSP2_INAV_FULL_LOCAL_POSE](#msp2_inav_full_local_pose)  
[12288 - MSP2_BETAFLIGHT_BIND](#msp2_betaflight_bind)
[12289 - MSP2_RX_BIND](#msp2_rx_bind)

## <a id="msp_api_version"></a>`MSP_API_VERSION (1 / 0x1)`
**Description:** Provides the MSP protocol version and the INAV API version.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `mspProtocolVersion` | `uint8_t` | 1 | MSP Protocol version (`MSP_PROTOCOL_VERSION`, typically 0) |
| `apiVersionMajor` | `uint8_t` | 1 | INAV API Major version (`API_VERSION_MAJOR`) |
| `apiVersionMinor` | `uint8_t` | 1 | INAV API Minor version (`API_VERSION_MINOR`) |

**Notes:** Used by configurators to check compatibility.

## <a id="msp_fc_variant"></a>`MSP_FC_VARIANT (2 / 0x2)`
**Description:** Identifies the flight controller firmware variant (e.g., INAV, Betaflight).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `fcVariantIdentifier` | `char[4]` | 4 | 4-character identifier string (e.g., "INAV"). Defined by `flightControllerIdentifier`. |

**Notes:** See `FLIGHT_CONTROLLER_IDENTIFIER_LENGTH`.

## <a id="msp_fc_version"></a>`MSP_FC_VERSION (3 / 0x3)`
**Description:** Provides the specific version number of the flight controller firmware.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `fcVersionMajor` | `uint8_t` | 1 | Firmware Major version (`FC_VERSION_MAJOR`) |
| `fcVersionMinor` | `uint8_t` | 1 | Firmware Minor version (`FC_VERSION_MINOR`) |
| `fcVersionPatch` | `uint8_t` | 1 | Firmware Patch level (`FC_VERSION_PATCH_LEVEL`) |

## <a id="msp_board_info"></a>`MSP_BOARD_INFO (4 / 0x4)`
**Description:** Provides information about the specific hardware board and its capabilities.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `boardIdentifier` | `char[4]` | 4 | - | 4-character UPPER CASE board identifier (`TARGET_BOARD_IDENTIFIER`) |
| `hardwareRevision` | `uint16_t` | 2 | - | Hardware revision number. 0 if not detected (`USE_HARDWARE_REVISION_DETECTION`) |
| `osdSupport` | `uint8_t` | 1 | - | OSD chip type: 0=None, 2=Onboard (`USE_OSD`). INAV does not support slave OSD (1) |
| `commCapabilities` | `uint8_t` | 1 | Bitmask | Bitmask: Communication capabilities: Bit 0=VCP support (`USE_VCP`), Bit 1=SoftSerial support (`USE_SOFTSERIAL1`/`2`) |
| `targetNameLength` | `uint8_t` | 1 | - | Length of the target name string that follows |
| `targetName` | `char[]` | array | - | Target name string (e.g., "MATEKF405"). Length given by previous field |

**Notes:** `BOARD_IDENTIFIER_LENGTH` is 4.

## <a id="msp_build_info"></a>`MSP_BUILD_INFO (5 / 0x5)`
**Description:** Provides build date, time, and Git revision of the firmware.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `buildDate` | `char[BUILD_DATE_LENGTH]` | 11 (BUILD_DATE_LENGTH) | Build date string (e.g., "Dec 31 2023"). `BUILD_DATE_LENGTH`. |
| `buildTime` | `char[BUILD_TIME_LENGTH]` | 8 (BUILD_TIME_LENGTH) | Build time string (e.g., "23:59:59"). `BUILD_TIME_LENGTH`. |
| `gitRevision` | `char[GIT_SHORT_REVISION_LENGTH]` | 8 (GIT_SHORT_REVISION_LENGTH) | Short Git revision string. `GIT_SHORT_REVISION_LENGTH`. |

## <a id="msp_inav_pid"></a>`MSP_INAV_PID (6 / 0x6)`
**Description:** Retrieves legacy INAV-specific PID controller related settings. Many fields are now obsolete or placeholders.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `legacyAsyncProcessing` | `uint8_t` | 1 | - | Legacy, unused. Always 0 |
| `legacyAsyncValue1` | `uint16_t` | 2 | - | Legacy, unused. Always 0 |
| `legacyAsyncValue2` | `int16_t` | 2 | - | Legacy, unused. Always 0 |
| `headingHoldRateLimit` | `uint8_t` | 1 | deg/s | Max rate for heading hold P term (`pidProfile()->heading_hold_rate_limit`) |
| `headingHoldLpfFreq` | `uint8_t` | 1 | Hz | Fixed LPF frequency for heading hold error (`HEADING_HOLD_ERROR_LPF_FREQ`) |
| `legacyYawJumpLimit` | `int16_t` | 2 | - | Legacy, unused. Always 0 |
| `legacyGyroLpf` | `uint8_t` | 1 | Hz | Fixed value `GYRO_LPF_256HZ` |
| `accLpfHz` | `uint8_t` | 1 | Hz | Accelerometer LPF frequency (`accelerometerConfig()->acc_lpf_hz`) cutoff frequency for the low pass filter used on the acc z-axis for althold in Hz |
| `reserved1` | `uint8_t` | 1 | - | Reserved. Always 0 |
| `reserved2` | `uint8_t` | 1 | - | Reserved. Always 0 |
| `reserved3` | `uint8_t` | 1 | - | Reserved. Always 0 |
| `reserved4` | `uint8_t` | 1 | - | Reserved. Always 0 |

**Notes:** Superseded by `MSP2_PID` for core PIDs and other specific messages for filter settings.

## <a id="msp_set_inav_pid"></a>`MSP_SET_INAV_PID (7 / 0x7)`
**Description:** Sets legacy INAV-specific PID controller related settings.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `legacyAsyncProcessing` | `uint8_t` | 1 | - | Legacy, ignored |
| `legacyAsyncValue1` | `int16_t` | 2 | - | Legacy, ignored |
| `legacyAsyncValue2` | `int16_t` | 2 | - | Legacy, ignored |
| `headingHoldRateLimit` | `uint8_t` | 1 | deg/s | Sets `pidProfileMutable()->heading_hold_rate_limit`. |
| `headingHoldLpfFreq` | `uint8_t` | 1 | Hz | Ignored (fixed value `HEADING_HOLD_ERROR_LPF_FREQ` used) |
| `legacyYawJumpLimit` | `int16_t` | 2 | - | Legacy, ignored |
| `legacyGyroLpf` | `uint8_t` | 1 | - | Ignored (historically mapped to `gyro_lpf_e` values). |
| `accLpfHz` | `uint8_t` | 1 | Hz | Sets `accelerometerConfigMutable()->acc_lpf_hz`. |
| `reserved1` | `uint8_t` | 1 | - | Ignored |
| `reserved2` | `uint8_t` | 1 | - | Ignored |
| `reserved3` | `uint8_t` | 1 | - | Ignored |
| `reserved4` | `uint8_t` | 1 | - | Ignored |

**Reply Payload:** **None**  

**Notes:** Expects 15 bytes.

## <a id="msp_name"></a>`MSP_NAME (10 / 0xa)`
**Description:** Returns the user-defined craft name.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `craftName` | `char[]` | array | The craft name string (`systemConfig()->craftName`). Null termination is *not* explicitly sent, the length is determined by the payload size |

## <a id="msp_set_name"></a>`MSP_SET_NAME (11 / 0xb)`
**Description:** Sets the user-defined craft name.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `craftName` | `char[]` | array | The new craft name string. Automatically null-terminated by the FC |

**Reply Payload:** **None**  

**Notes:** Maximum length is `MAX_NAME_LENGTH`.

## <a id="msp_nav_poshold"></a>`MSP_NAV_POSHOLD (12 / 0xc)`
**Description:** Retrieves navigation position hold and general manual/auto flight parameters. Some parameters depend on the platform type (Multirotor vs Fixed Wing).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `userControlMode` | `uint8_t` | 1 | - | Navigation user control mode NAV_GPS_ATTI (0) or NAV_GPS_CRUISE (1) |
| `maxAutoSpeed` | `uint16_t` | 2 | cm/s | Max speed in autonomous modes (`navConfig()->general.max_auto_speed`) |
| `maxAutoClimbRate` | `uint16_t` | 2 | cm/s | Max climb rate in autonomous modes (uses `fw.max_auto_climb_rate` or `mc.max_auto_climb_rate` based on platform) |
| `maxManualSpeed` | `uint16_t` | 2 | cm/s | Max speed in manual modes with GPS aiding (`navConfig()->general.max_manual_speed`) |
| `maxManualClimbRate` | `uint16_t` | 2 | cm/s | Max climb rate in manual modes with GPS aiding (uses `fw.max_manual_climb_rate` or `mc.max_manual_climb_rate`) |
| `mcMaxBankAngle` | `uint8_t` | 1 | degrees | Max bank angle for multirotor position hold (`navConfig()->mc.max_bank_angle`) |
| `mcAltHoldThrottleType` | `uint8_t` | 1 | [navMcAltHoldThrottle_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-navmcaltholdthrottle_e) | Enum `navMcAltHoldThrottle_e` mirrored from `navConfig()->mc.althold_throttle_type`. |
| `mcHoverThrottle` | `uint16_t` | 2 | PWM | Multirotor hover throttle PWM value (`currentBatteryProfile->nav.mc.hover_throttle`). |

## <a id="msp_set_nav_poshold"></a>`MSP_SET_NAV_POSHOLD (13 / 0xd)`
**Description:** Sets navigation position hold and general manual/auto flight parameters.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `userControlMode` | `uint8_t` | 1 | [navUserControlMode_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-navusercontrolmode_e) | Sets `navConfigMutable()->general.flags.user_control_mode`. WARNING: uses unnamed enum in navigation.h 'NAV_GPS_ATTI/NAV_GPS_CRUISE' |
| `maxAutoSpeed` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->general.max_auto_speed`. |
| `maxAutoClimbRate` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->fw.max_auto_climb_rate` or `navConfigMutable()->mc.max_auto_climb_rate` based on `mixerConfig()->platformType`. |
| `maxManualSpeed` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->general.max_manual_speed`. |
| `maxManualClimbRate` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->fw.max_manual_climb_rate` or `navConfigMutable()->mc.max_manual_climb_rate`. |
| `mcMaxBankAngle` | `uint8_t` | 1 | degrees | Sets `navConfigMutable()->mc.max_bank_angle`. |
| `mcAltHoldThrottleType` | `uint8_t` | 1 | [navMcAltHoldThrottle_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-navmcaltholdthrottle_e) | Enum `navMcAltHoldThrottle_e`; updates `navConfigMutable()->mc.althold_throttle_type`. |
| `mcHoverThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->nav.mc.hover_throttle`. |

**Reply Payload:** **None**  

**Notes:** Expects 13 bytes.

## <a id="msp_calibration_data"></a>`MSP_CALIBRATION_DATA (14 / 0xe)`
**Description:** Retrieves sensor calibration data (Accelerometer zero/gain, Magnetometer zero/gain, Optical Flow scale).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `accCalibAxisFlags` | `uint8_t` | 1 | Bitmask | Bitmask: Flags indicating which axes of the accelerometer have been calibrated (`accGetCalibrationAxisFlags()`) |
| `accZeroX` | `int16_t` | 2 | Raw ADC | Accelerometer zero offset for X-axis (`accelerometerConfig()->accZero.raw[X]`) |
| `accZeroY` | `int16_t` | 2 | Raw ADC | Accelerometer zero offset for Y-axis (`accelerometerConfig()->accZero.raw[Y]`) |
| `accZeroZ` | `int16_t` | 2 | Raw ADC | Accelerometer zero offset for Z-axis (`accelerometerConfig()->accZero.raw[Z]`) |
| `accGainX` | `int16_t` | 2 | Raw ADC | Accelerometer gain/scale for X-axis (`accelerometerConfig()->accGain.raw[X]`) |
| `accGainY` | `int16_t` | 2 | Raw ADC | Accelerometer gain/scale for Y-axis (`accelerometerConfig()->accGain.raw[Y]`) |
| `accGainZ` | `int16_t` | 2 | Raw ADC | Accelerometer gain/scale for Z-axis (`accelerometerConfig()->accGain.raw[Z]`) |
| `magZeroX` | `int16_t` | 2 | Raw ADC | Magnetometer zero offset for X-axis (`compassConfig()->magZero.raw[X]`). 0 if `USE_MAG` disabled |
| `magZeroY` | `int16_t` | 2 | Raw ADC | Magnetometer zero offset for Y-axis (`compassConfig()->magZero.raw[Y]`). 0 if `USE_MAG` disabled |
| `magZeroZ` | `int16_t` | 2 | Raw ADC | Magnetometer zero offset for Z-axis (`compassConfig()->magZero.raw[Z]`). 0 if `USE_MAG` disabled |
| `opflowScale` | `uint16_t` | 2 | Scale * 256 | Optical flow scale factor (`opticalFlowConfig()->opflow_scale * 256`). 0 if `USE_OPFLOW` disabled |
| `magGainX` | `int16_t` | 2 | Raw ADC | Magnetometer gain/scale for X-axis (`compassConfig()->magGain[X]`). 0 if `USE_MAG` disabled |
| `magGainY` | `int16_t` | 2 | Raw ADC | Magnetometer gain/scale for Y-axis (`compassConfig()->magGain[Y]`). 0 if `USE_MAG` disabled |
| `magGainZ` | `int16_t` | 2 | Raw ADC | Magnetometer gain/scale for Z-axis (`compassConfig()->magGain[Z]`). 0 if `USE_MAG` disabled |

**Notes:** Total size 27 bytes. Fields related to optional sensors are zero if the sensor is not used.

## <a id="msp_set_calibration_data"></a>`MSP_SET_CALIBRATION_DATA (15 / 0xf)`
**Description:** Sets sensor calibration data.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `accZeroX` | `int16_t` | 2 | Raw ADC | Sets `accelerometerConfigMutable()->accZero.raw[X]`. |
| `accZeroY` | `int16_t` | 2 | Raw ADC | Sets `accelerometerConfigMutable()->accZero.raw[Y]`. |
| `accZeroZ` | `int16_t` | 2 | Raw ADC | Sets `accelerometerConfigMutable()->accZero.raw[Z]`. |
| `accGainX` | `int16_t` | 2 | Raw ADC | Sets `accelerometerConfigMutable()->accGain.raw[X]`. |
| `accGainY` | `int16_t` | 2 | Raw ADC | Sets `accelerometerConfigMutable()->accGain.raw[Y]`. |
| `accGainZ` | `int16_t` | 2 | Raw ADC | Sets `accelerometerConfigMutable()->accGain.raw[Z]`. |
| `magZeroX` | `int16_t` | 2 | Raw ADC | Sets `compassConfigMutable()->magZero.raw[X]` (if `USE_MAG`) |
| `magZeroY` | `int16_t` | 2 | Raw ADC | Sets `compassConfigMutable()->magZero.raw[Y]` (if `USE_MAG`) |
| `magZeroZ` | `int16_t` | 2 | Raw ADC | Sets `compassConfigMutable()->magZero.raw[Z]` (if `USE_MAG`) |
| `opflowScale` | `uint16_t` | 2 | Scale * 256 | Sets `opticalFlowConfigMutable()->opflow_scale = value / 256.0f` (if `USE_OPFLOW`) |
| `magGainX` | `int16_t` | 2 | Raw ADC | Sets `compassConfigMutable()->magGain[X]` (if `USE_MAG`) |
| `magGainY` | `int16_t` | 2 | Raw ADC | Sets `compassConfigMutable()->magGain[Y]` (if `USE_MAG`) |
| `magGainZ` | `int16_t` | 2 | Raw ADC | Sets `compassConfigMutable()->magGain[Z]` (if `USE_MAG`) |

**Reply Payload:** **None**  

**Notes:** Minimum payload 18 bytes. Adds +6 bytes for magnetometer zeros, +2 for optical flow scale, and +6 for magnetometer gains when those features (`USE_MAG`, `USE_OPFLOW`) are compiled in.

## <a id="msp_position_estimation_config"></a>`MSP_POSITION_ESTIMATION_CONFIG (16 / 0x10)`
**Description:** Retrieves parameters related to the INAV position estimation fusion weights and GPS minimum satellite count.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `weightZBaroP` | `uint16_t` | 2 | Weight * 100 | Barometer Z position fusion weight (`positionEstimationConfig()->w_z_baro_p * 100`) |
| `weightZGPSP` | `uint16_t` | 2 | Weight * 100 | GPS Z position fusion weight (`positionEstimationConfig()->w_z_gps_p * 100`) |
| `weightZGPSV` | `uint16_t` | 2 | Weight * 100 | GPS Z velocity fusion weight (`positionEstimationConfig()->w_z_gps_v * 100`) |
| `weightXYGPSP` | `uint16_t` | 2 | Weight * 100 | GPS XY position fusion weight (`positionEstimationConfig()->w_xy_gps_p * 100`) |
| `weightXYGPSV` | `uint16_t` | 2 | Weight * 100 | GPS XY velocity fusion weight (`positionEstimationConfig()->w_xy_gps_v * 100`) |
| `minSats` | `uint8_t` | 1 | Count | Minimum satellites required for GPS use (`gpsConfigMutable()->gpsMinSats`) |
| `useGPSVelNED` | `uint8_t` | 1 | Boolean | Legacy flag, always 1 (GPS velocity is always used if available) |

## <a id="msp_set_position_estimation_config"></a>`MSP_SET_POSITION_ESTIMATION_CONFIG (17 / 0x11)`
**Description:** Sets parameters related to the INAV position estimation fusion weights and GPS minimum satellite count.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `weightZBaroP` | `uint16_t` | 2 | Weight * 100 | Sets `positionEstimationConfigMutable()->w_z_baro_p = value / 100.0f` (constrained 0.0-10.0) |
| `weightZGPSP` | `uint16_t` | 2 | Weight * 100 | Sets `positionEstimationConfigMutable()->w_z_gps_p = value / 100.0f` (constrained 0.0-10.0) |
| `weightZGPSV` | `uint16_t` | 2 | Weight * 100 | Sets `positionEstimationConfigMutable()->w_z_gps_v = value / 100.0f` (constrained 0.0-10.0) |
| `weightXYGPSP` | `uint16_t` | 2 | Weight * 100 | Sets `positionEstimationConfigMutable()->w_xy_gps_p = value / 100.0f` (constrained 0.0-10.0) |
| `weightXYGPSV` | `uint16_t` | 2 | Weight * 100 | Sets `positionEstimationConfigMutable()->w_xy_gps_v = value / 100.0f` (constrained 0.0-10.0) |
| `minSats` | `uint8_t` | 1 | Count | Sets `gpsConfigMutable()->gpsMinSats` (constrained 5-10) |
| `useGPSVelNED` | `uint8_t` | 1 | Boolean | Legacy flag, ignored |

**Reply Payload:** **None**  

**Notes:** Expects 12 bytes.

## <a id="msp_wp_mission_load"></a>`MSP_WP_MISSION_LOAD (18 / 0x12)`
**Description:** Commands the FC to load the waypoint mission stored in non-volatile memory (e.g., EEPROM or FlashFS) into the active mission buffer.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `missionID` | `uint8_t` | 1 | Reserved for future use, currently ignored |

**Reply Payload:** **None**  

**Notes:** Only functional if `NAV_NON_VOLATILE_WAYPOINT_STORAGE` is defined. Requires 1 byte payload. Returns error if loading fails.

## <a id="msp_wp_mission_save"></a>`MSP_WP_MISSION_SAVE (19 / 0x13)`
**Description:** Commands the FC to save the currently active waypoint mission from RAM to non-volatile memory (e.g., EEPROM or FlashFS).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `missionID` | `uint8_t` | 1 | Reserved for future use, currently ignored |

**Reply Payload:** **None**  

**Notes:** Only functional if `NAV_NON_VOLATILE_WAYPOINT_STORAGE` is defined. Requires 1 byte payload. Returns error if saving fails.

## <a id="msp_wp_getinfo"></a>`MSP_WP_GETINFO (20 / 0x14)`
**Description:** Retrieves information about the waypoint mission capabilities and the status of the currently loaded mission.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `wpCapabilities` | `uint8_t` | 1 | Reserved for future waypoint capabilities flags. Currently always 0 |
| `maxWaypoints` | `uint8_t` | 1 | Maximum number of waypoints supported (`NAV_MAX_WAYPOINTS`) |
| `missionValid` | `uint8_t` | 1 | Boolean flag indicating if the current mission in RAM is valid (`isWaypointListValid()`) |
| `waypointCount` | `uint8_t` | 1 | Number of waypoints currently defined in the mission (`getWaypointCount()`) |

## <a id="msp_rth_and_land_config"></a>`MSP_RTH_AND_LAND_CONFIG (21 / 0x15)`
**Description:** Retrieves configuration parameters related to Return-to-Home (RTH) and automatic landing behaviors.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `minRthDistance` | `uint16_t` | 2 | cm | Minimum distance from home required for RTH to engage (`navConfig()->general.min_rth_distance`) |
| `rthClimbFirst` | `uint8_t` | 1 | Boolean | Flag: Climb to RTH altitude before returning (`navConfig()->general.flags.rth_climb_first`) |
| `rthClimbIgnoreEmerg` | `uint8_t` | 1 | Boolean | Flag: Climb even in emergency RTH (`navConfig()->general.flags.rth_climb_ignore_emerg`) |
| `rthTailFirst` | `uint8_t` | 1 | Boolean | Flag: Return tail-first during RTH (`navConfig()->general.flags.rth_tail_first`) |
| `rthAllowLanding` | `uint8_t` | 1 | Boolean | Flag: Allow automatic landing after RTH (`navConfig()->general.flags.rth_allow_landing`) |
| `rthAltControlMode` | `uint8_t` | 1 | [navRthAltControlMode_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-navrthaltcontrolmode_e) | RTH altitude control mode (`navConfig()->general.flags.rth_alt_control_mode`). WARNING: uses unnamed enum in navigation.h:253 'NAV_RTH_NO_ALT...' |
| `rthAbortThreshold` | `uint16_t` | 2 | cm | Distance increase threshold to abort RTH (`navConfig()->general.rth_abort_threshold`) |
| `rthAltitude` | `uint16_t` | 2 | cm | Target RTH altitude (`navConfig()->general.rth_altitude`) |
| `landMinAltVspd` | `uint16_t` | 2 | cm/s | Landing vertical speed at minimum slowdown altitude (`navConfig()->general.land_minalt_vspd`) |
| `landMaxAltVspd` | `uint16_t` | 2 | cm/s | Landing vertical speed at maximum slowdown altitude (`navConfig()->general.land_maxalt_vspd`) |
| `landSlowdownMinAlt` | `uint16_t` | 2 | cm | Altitude below which `landMinAltVspd` applies (`navConfig()->general.land_slowdown_minalt`) |
| `landSlowdownMaxAlt` | `uint16_t` | 2 | cm | Altitude above which `landMaxAltVspd` applies (`navConfig()->general.land_slowdown_maxalt`) |
| `emergDescentRate` | `uint16_t` | 2 | cm/s | Vertical speed during emergency landing descent (`navConfig()->general.emerg_descent_rate`) |

## <a id="msp_set_rth_and_land_config"></a>`MSP_SET_RTH_AND_LAND_CONFIG (22 / 0x16)`
**Description:** Sets configuration parameters related to Return-to-Home (RTH) and automatic landing behaviors.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `minRthDistance` | `uint16_t` | 2 | cm | Sets `navConfigMutable()->general.min_rth_distance`. |
| `rthClimbFirst` | `uint8_t` | 1 | Boolean | Sets `navConfigMutable()->general.flags.rth_climb_first`. |
| `rthClimbIgnoreEmerg` | `uint8_t` | 1 | Boolean | Sets `navConfigMutable()->general.flags.rth_climb_ignore_emerg`. |
| `rthTailFirst` | `uint8_t` | 1 | Boolean | Sets `navConfigMutable()->general.flags.rth_tail_first`. |
| `rthAllowLanding` | `uint8_t` | 1 | Boolean | Sets `navConfigMutable()->general.flags.rth_allow_landing`. |
| `rthAltControlMode` | `uint8_t` | 1 | [navRthAltControlMode_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-navrthaltcontrolmode_e) | Sets `navConfigMutable()->general.flags.rth_alt_control_mode`. WARNING: uses unnamed enum in navigation.h:253 |
| `rthAbortThreshold` | `uint16_t` | 2 | cm | Sets `navConfigMutable()->general.rth_abort_threshold`. |
| `rthAltitude` | `uint16_t` | 2 | cm | Sets `navConfigMutable()->general.rth_altitude`. |
| `landMinAltVspd` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->general.land_minalt_vspd`. |
| `landMaxAltVspd` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->general.land_maxalt_vspd`. |
| `landSlowdownMinAlt` | `uint16_t` | 2 | cm | Sets `navConfigMutable()->general.land_slowdown_minalt`. |
| `landSlowdownMaxAlt` | `uint16_t` | 2 | cm | Sets `navConfigMutable()->general.land_slowdown_maxalt`. |
| `emergDescentRate` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->general.emerg_descent_rate`. |

**Reply Payload:** **None**  

**Notes:** Expects 21 bytes.

## <a id="msp_fw_config"></a>`MSP_FW_CONFIG (23 / 0x17)`
**Description:** Retrieves configuration parameters specific to Fixed Wing navigation.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `cruiseThrottle` | `uint16_t` | 2 | PWM | Cruise throttle command (`currentBatteryProfile->nav.fw.cruise_throttle`). |
| `minThrottle` | `uint16_t` | 2 | PWM | Minimum throttle during autonomous flight (`currentBatteryProfile->nav.fw.min_throttle`). |
| `maxThrottle` | `uint16_t` | 2 | PWM | Maximum throttle during autonomous flight (`currentBatteryProfile->nav.fw.max_throttle`). |
| `maxBankAngle` | `uint8_t` | 1 | degrees | Maximum bank angle allowed (`navConfig()->fw.max_bank_angle`) |
| `maxClimbAngle` | `uint8_t` | 1 | degrees | Maximum pitch angle during climb (`navConfig()->fw.max_climb_angle`) |
| `maxDiveAngle` | `uint8_t` | 1 | degrees | Maximum negative pitch angle during descent (`navConfig()->fw.max_dive_angle`) |
| `pitchToThrottle` | `uint8_t` | 1 | us/deg | Pitch-to-throttle gain (`currentBatteryProfile->nav.fw.pitch_to_throttle`); PWM microseconds per degree (10 units ≈ 1% throttle). |
| `loiterRadius` | `uint16_t` | 2 | cm | Default loiter radius (`navConfig()->fw.loiter_radius`). |

## <a id="msp_set_fw_config"></a>`MSP_SET_FW_CONFIG (24 / 0x18)`
**Description:** Sets configuration parameters specific to Fixed Wing navigation.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `cruiseThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->nav.fw.cruise_throttle`. |
| `minThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->nav.fw.min_throttle`. |
| `maxThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->nav.fw.max_throttle`. |
| `maxBankAngle` | `uint8_t` | 1 | degrees | Sets `navConfigMutable()->fw.max_bank_angle`. |
| `maxClimbAngle` | `uint8_t` | 1 | degrees | Sets `navConfigMutable()->fw.max_climb_angle`. |
| `maxDiveAngle` | `uint8_t` | 1 | degrees | Sets `navConfigMutable()->fw.max_dive_angle`. |
| `pitchToThrottle` | `uint8_t` | 1 | us/deg | Sets `currentBatteryProfileMutable->nav.fw.pitch_to_throttle` (PWM microseconds per degree; 10 units ≈ 1% throttle). |
| `loiterRadius` | `uint16_t` | 2 | cm | Sets `navConfigMutable()->fw.loiter_radius`. |

**Reply Payload:** **None**  

**Notes:** Expects 12 bytes.

## <a id="msp_mode_ranges"></a>`MSP_MODE_RANGES (34 / 0x22)`
**Description:** Returns all defined mode activation ranges (aux channel assignments for flight modes).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `modePermanentId` | `uint8_t` | 1 | ID | Permanent ID of the flight mode (maps to `boxId` via `findBoxByActiveBoxId`). 0 if entry unused |
| `auxChannelIndex` | `uint8_t` | 1 | Index | 0-based index of the AUX channel used for activation |
| `rangeStartStep` | `uint8_t` | 1 | step | Start step (0-48). Each step is 25 PWM units; 0 is <=900 and 48 is >=2100. |
| `rangeEndStep` | `uint8_t` | 1 | step | End step (0-48). Uses the same 25-PWM step mapping as rangeStartStep. |

**Notes:** The number of steps and mapping to PWM values depends on internal range calculations.

## <a id="msp_set_mode_range"></a>`MSP_SET_MODE_RANGE (35 / 0x23)`
**Description:** Sets a single mode activation range by its index.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `rangeIndex` | `uint8_t` | 1 | Index | Index of the mode range to set (0 to `MAX_MODE_ACTIVATION_CONDITION_COUNT - 1`) |
| `modePermanentId` | `uint8_t` | 1 | ID | Permanent ID of the flight mode to assign |
| `auxChannelIndex` | `uint8_t` | 1 | Index | 0-based index of the AUX channel |
| `rangeStartStep` | `uint8_t` | 1 | step | Start step (0-48). Each step is 25 PWM units; 0 is <=900 and 48 is >=2100. |
| `rangeEndStep` | `uint8_t` | 1 | step | End step (0-48). Uses the same 25-PWM step mapping as rangeStartStep. |

**Reply Payload:** **None**  

**Notes:** Expects 5 bytes. Updates the mode configuration and recalculates used mode flags. Returns error if `rangeIndex` or `modePermanentId` is invalid.

## <a id="msp_feature"></a>`MSP_FEATURE (36 / 0x24)`
**Description:** Returns a bitmask of enabled features.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `featureMask` | `uint32_t` | 4 | Bitmask | Bitmask: active features (see `featureMask()`) |

**Notes:** Feature bits are defined in `feature.h`.

## <a id="msp_set_feature"></a>`MSP_SET_FEATURE (37 / 0x25)`
**Description:** Sets the enabled features using a bitmask. Clears all previous features first.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `featureMask` | `uint32_t` | 4 | Bitmask | Bitmask: features to enable |

**Reply Payload:** **None**  

**Notes:** Expects 4 bytes. Updates feature configuration and related settings (e.g., RSSI source).

## <a id="msp_board_alignment"></a>`MSP_BOARD_ALIGNMENT (38 / 0x26)`
**Description:** Returns the sensor board alignment angles relative to the craft frame.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `rollAlign` | `int16_t` | 2 | deci-degrees | Board alignment roll angle (`boardAlignment()->rollDeciDegrees`). Negative values tilt left. |
| `pitchAlign` | `int16_t` | 2 | deci-degrees | Board alignment pitch angle (`boardAlignment()->pitchDeciDegrees`). Negative values nose down. |
| `yawAlign` | `int16_t` | 2 | deci-degrees | Board alignment yaw angle (`boardAlignment()->yawDeciDegrees`). Negative values rotate counter-clockwise. |

**Notes:** Ranges are typically -1800 to +1800 (i.e. -180.0° to +180.0°).

## <a id="msp_set_board_alignment"></a>`MSP_SET_BOARD_ALIGNMENT (39 / 0x27)`
**Description:** Sets the sensor board alignment angles.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `rollAlign` | `int16_t` | 2 | deci-degrees | Sets `boardAlignmentMutable()->rollDeciDegrees`. |
| `pitchAlign` | `int16_t` | 2 | deci-degrees | Sets `boardAlignmentMutable()->pitchDeciDegrees`. |
| `yawAlign` | `int16_t` | 2 | deci-degrees | Sets `boardAlignmentMutable()->yawDeciDegrees`. |

**Reply Payload:** **None**  

**Notes:** Expects 6 bytes encoded as little-endian signed deci-degrees (-1800 to +1800 typical).

## <a id="msp_current_meter_config"></a>`MSP_CURRENT_METER_CONFIG (40 / 0x28)`
**Description:** Retrieves the configuration for the current sensor.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `scale` | `int16_t` | 2 | 0.1 mV/A | Current sensor scale factor (`batteryMetersConfig()->current.scale`). Stored in 0.1 mV/A; signed for calibration. |
| `offset` | `int16_t` | 2 | mV | Current sensor offset (`batteryMetersConfig()->current.offset`). Signed millivolt adjustment. |
| `type` | `uint8_t` | 1 | [currentSensor_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-currentsensor_e) | Enum `currentSensor_e` Type of current sensor hardware |
| `capacity` | `uint16_t` | 2 | mAh (legacy) | Battery capacity (constrained 0-65535) (`currentBatteryProfile->capacity.value`). Note: This is legacy, use `MSP2_INAV_BATTERY_CONFIG` for full 32-bit capacity |

**Notes:** Scale and offset are signed values matching `batteryMetersConfig()->current` fields.

## <a id="msp_set_current_meter_config"></a>`MSP_SET_CURRENT_METER_CONFIG (41 / 0x29)`
**Description:** Sets the configuration for the current sensor.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `scale` | `int16_t` | 2 | 0.1 mV/A | Sets `batteryMetersConfigMutable()->current.scale` (0.1 mV/A, signed). |
| `offset` | `int16_t` | 2 | mV | Sets `batteryMetersConfigMutable()->current.offset` (signed millivolts). |
| `type` | `uint8_t` | 1 | [currentSensor_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-currentsensor_e) | Enum `currentSensor_e` Sets `batteryMetersConfigMutable()->current.type`. |
| `capacity` | `uint16_t` | 2 | mAh (legacy) | Sets `currentBatteryProfileMutable->capacity.value` (truncated to 16 bits) |

**Reply Payload:** **None**  

**Notes:** Expects 7 bytes. Signed values use little-endian two's complement.

## <a id="msp_mixer"></a>`MSP_MIXER (42 / 0x2a)`
**Description:** Retrieves the mixer type (Legacy, INAV always returns QuadX).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `mixerMode` | `uint8_t` | 1 | Always 3 (QuadX) in INAV for compatibility |

**Notes:** This command is largely obsolete. Mixer configuration is handled differently in INAV (presets, custom mixes). See `MSP2_INAV_MIXER`.

## <a id="msp_set_mixer"></a>`MSP_SET_MIXER (43 / 0x2b)`
**Description:** Sets the mixer type (Legacy, ignored by INAV).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `mixerMode` | `uint8_t` | 1 | Mixer mode to set (ignored by INAV) |

**Reply Payload:** **None**  

**Notes:** Expects 1 byte. Calls `mixerUpdateStateFlags()` for potential side effects related to presets.

## <a id="msp_rx_config"></a>`MSP_RX_CONFIG (44 / 0x2c)`
**Description:** Retrieves receiver configuration settings. Some fields are Betaflight compatibility placeholders.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `serialRxProvider` | `uint8_t` | 1 | [rxSerialReceiverType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-rxserialreceivertype_e) | Enum `rxSerialReceiverType_e`. Serial RX provider (`rxConfig()->serialrx_provider`). |
| `maxCheck` | `uint16_t` | 2 | PWM | Upper channel value threshold for stick commands (`rxConfig()->maxcheck`) |
| `midRc` | `uint16_t` | 2 | PWM | Center channel value (`PWM_RANGE_MIDDLE`, typically 1500) |
| `minCheck` | `uint16_t` | 2 | PWM | Lower channel value threshold for stick commands (`rxConfig()->mincheck`) |
| `spektrumSatBind` | `uint8_t` | 1 | Count/Flag | Spektrum bind pulses (`rxConfig()->spektrum_sat_bind`). 0 if `USE_SPEKTRUM_BIND` disabled. |
| `rxMinUsec` | `uint16_t` | 2 | PWM | Minimum expected pulse width (`rxConfig()->rx_min_usec`) |
| `rxMaxUsec` | `uint16_t` | 2 | PWM | Maximum expected pulse width (`rxConfig()->rx_max_usec`) |
| `bfCompatRcInterpolation` | `uint8_t` | 1 | - | BF compatibility. Always 0 |
| `bfCompatRcInterpolationInt` | `uint8_t` | 1 | - | BF compatibility. Always 0 |
| `bfCompatAirModeThreshold` | `uint16_t` | 2 | - | BF compatibility. Always 0 |
| `reserved1` | `uint8_t` | 1 | - | Reserved/Padding. Always 0 |
| `reserved2` | `uint32_t` | 4 | - | Reserved/Padding. Always 0 |
| `reserved3` | `uint8_t` | 1 | - | Reserved/Padding. Always 0 |
| `bfCompatFpvCamAngle` | `uint8_t` | 1 | - | BF compatibility. Always 0 |
| `receiverType` | `uint8_t` | 1 | [rxReceiverType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-rxreceivertype_e) | Enum `rxReceiverType_e` Receiver type (Parallel PWM, PPM, Serial) ('rxConfig()->receiverType') |

## <a id="msp_set_rx_config"></a>`MSP_SET_RX_CONFIG (45 / 0x2d)`
**Description:** Sets receiver configuration settings.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `serialRxProvider` | `uint8_t` | 1 | [rxSerialReceiverType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-rxserialreceivertype_e) | Enum `rxSerialReceiverType_e`. Sets `rxConfigMutable()->serialrx_provider`. |
| `maxCheck` | `uint16_t` | 2 | PWM | Sets `rxConfigMutable()->maxcheck`. |
| `midRc` | `uint16_t` | 2 | PWM | Ignored (`PWM_RANGE_MIDDLE` is used) |
| `minCheck` | `uint16_t` | 2 | PWM | Sets `rxConfigMutable()->mincheck`. |
| `spektrumSatBind` | `uint8_t` | 1 | Count/Flag | Sets `rxConfigMutable()->spektrum_sat_bind` (if `USE_SPEKTRUM_BIND`). |
| `rxMinUsec` | `uint16_t` | 2 | PWM | Sets `rxConfigMutable()->rx_min_usec`. |
| `rxMaxUsec` | `uint16_t` | 2 | PWM | Sets `rxConfigMutable()->rx_max_usec`. |
| `bfCompatRcInterpolation` | `uint8_t` | 1 | - | Ignored |
| `bfCompatRcInterpolationInt` | `uint8_t` | 1 | - | Ignored |
| `bfCompatAirModeThreshold` | `uint16_t` | 2 | - | Ignored |
| `reserved1` | `uint8_t` | 1 | - | Ignored |
| `reserved2` | `uint32_t` | 4 | - | Ignored |
| `reserved3` | `uint8_t` | 1 | - | Ignored |
| `bfCompatFpvCamAngle` | `uint8_t` | 1 | - | Ignored |
| `receiverType` | `uint8_t` | 1 | [rxReceiverType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-rxreceivertype_e) | Enum `rxReceiverType_e` Sets `rxConfigMutable()->receiverType`. |

**Reply Payload:** **None**  

**Notes:** Expects 24 bytes.

## <a id="msp_led_colors"></a>`MSP_LED_COLORS (46 / 0x2e)`
**Description:** Retrieves the HSV color definitions for configurable LED colors.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `hue` | `uint16_t` | 2 | Hue value (0-359) |
| `saturation` | `uint8_t` | 1 | Saturation value (0-255) |
| `value` | `uint8_t` | 1 | Value/Brightness (0-255) |

**Notes:** Only available if `USE_LED_STRIP` is defined.

## <a id="msp_set_led_colors"></a>`MSP_SET_LED_COLORS (47 / 0x2f)`
**Description:** Sets the HSV color definitions for configurable LED colors.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `hue` | `uint16_t` | 2 | Hue value (0-359) |
| `saturation` | `uint8_t` | 1 | Saturation value (0-255) |
| `value` | `uint8_t` | 1 | Value/Brightness (0-255) |

**Reply Payload:** **None**  

**Notes:** Only available if `USE_LED_STRIP` is defined. Expects `LED_CONFIGURABLE_COLOR_COUNT * 4` bytes.

## <a id="msp_led_strip_config"></a>`MSP_LED_STRIP_CONFIG (48 / 0x30)`
**Description:** Retrieves the configuration for each LED on the strip (legacy packed format).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `legacyLedConfig` | `uint32_t` | 4 | Packed LED configuration (position, function, overlay, color, direction, params). See C code for bit packing details |

**Notes:** Only available if `USE_LED_STRIP` is defined. Superseded by `MSP2_INAV_LED_STRIP_CONFIG_EX` which uses a clearer struct.

## <a id="msp_set_led_strip_config"></a>`MSP_SET_LED_STRIP_CONFIG (49 / 0x31)`
**Description:** Sets the configuration for a single LED on the strip using the legacy packed format.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `ledIndex` | `uint8_t` | 1 | Index of the LED to configure (0 to `LED_MAX_STRIP_LENGTH - 1`) |
| `legacyLedConfig` | `uint32_t` | 4 | Packed LED configuration to set |

**Reply Payload:** **None**  

**Notes:** Only available if `USE_LED_STRIP` is defined. Expects 5 bytes. Calls `reevaluateLedConfig()`. Superseded by `MSP2_INAV_SET_LED_STRIP_CONFIG_EX`.

## <a id="msp_rssi_config"></a>`MSP_RSSI_CONFIG (50 / 0x32)`
**Description:** Retrieves the channel used for analog RSSI input.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `rssiChannel` | `uint8_t` | 1 | AUX channel index (1-based) used for RSSI, or 0 if disabled (`rxConfig()->rssi_channel`) |

## <a id="msp_set_rssi_config"></a>`MSP_SET_RSSI_CONFIG (51 / 0x33)`
**Description:** Sets the channel used for analog RSSI input.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `rssiChannel` | `uint8_t` | 1 | AUX channel index (1-based) to use for RSSI, or 0 to disable |

**Reply Payload:** **None**  

**Notes:** Expects 1 byte. Input value is constrained 0 to `MAX_SUPPORTED_RC_CHANNEL_COUNT`. Updates the effective RSSI source.

## <a id="msp_adjustment_ranges"></a>`MSP_ADJUSTMENT_RANGES (52 / 0x34)`
**Description:** Returns all defined RC adjustment ranges (tuning via aux channels).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `adjustmentIndex` | `uint8_t` | 1 | - | Index of the adjustment slot (0 to `MAX_SIMULTANEOUS_ADJUSTMENT_COUNT - 1`) |
| `auxChannelIndex` | `uint8_t` | 1 | - | 0-based index of the AUX channel controlling the adjustment value |
| `rangeStartStep` | `uint8_t` | 1 | step | Start step (0-48). Each step is 25 PWM units; 0 is <=900 and 48 is >=2100. |
| `rangeEndStep` | `uint8_t` | 1 | step | End step (0-48). Uses the same 25-PWM step mapping as rangeStartStep. |
| `adjustmentFunction` | `uint8_t` | 1 | [adjustmentFunction_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-adjustmentfunction_e) | Function/parameter being adjusted (see `adjustmentFunction_e`). |
| `auxSwitchChannelIndex` | `uint8_t` | 1 | - | 0-based index of the AUX channel acting as an enable switch (or 0 if always enabled) |

**Notes:** See `adjustmentRange_t`.

## <a id="msp_set_adjustment_range"></a>`MSP_SET_ADJUSTMENT_RANGE (53 / 0x35)`
**Description:** Sets a single RC adjustment range configuration by its index.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `rangeIndex` | `uint8_t` | 1 | - | Index of the adjustment range to set (0 to `MAX_ADJUSTMENT_RANGE_COUNT - 1`) |
| `adjustmentIndex` | `uint8_t` | 1 | - | Adjustment slot index (0 to `MAX_SIMULTANEOUS_ADJUSTMENT_COUNT - 1`) |
| `auxChannelIndex` | `uint8_t` | 1 | - | 0-based index of the control AUX channel |
| `rangeStartStep` | `uint8_t` | 1 | step | Start step (0-48). Each step is 25 PWM units; 0 is <=900 and 48 is >=2100. |
| `rangeEndStep` | `uint8_t` | 1 | step | End step (0-48). Uses the same 25-PWM step mapping as rangeStartStep. |
| `adjustmentFunction` | `uint8_t` | 1 | [adjustmentFunction_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-adjustmentfunction_e) | Function/parameter being adjusted. |
| `auxSwitchChannelIndex` | `uint8_t` | 1 | - | 0-based index of the enable switch AUX channel (or 0) |

**Reply Payload:** **None**  

**Notes:** Expects 7 bytes. Returns error if `rangeIndex` or `adjustmentIndex` is invalid.

## <a id="msp_cf_serial_config"></a>`MSP_CF_SERIAL_CONFIG (54 / 0x36)`
**Description:** Deprecated command to get serial port configuration.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Not implemented in INAV `fc_msp.c`. Use `MSP2_COMMON_SERIAL_CONFIG`.

## <a id="msp_set_cf_serial_config"></a>`MSP_SET_CF_SERIAL_CONFIG (55 / 0x37)`
**Description:** Deprecated command to set serial port configuration.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Not implemented in INAV `fc_msp.c`. Use `MSP2_COMMON_SET_SERIAL_CONFIG`.

## <a id="msp_voltage_meter_config"></a>`MSP_VOLTAGE_METER_CONFIG (56 / 0x38)`
**Description:** Retrieves legacy voltage meter configuration (scaled values).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `vbatScale` | `uint8_t` | 1 | Scale / 10 | Voltage sensor scale factor / 10 (`batteryMetersConfig()->voltage.scale / 10`). 0 if `USE_ADC` disabled |
| `vbatMinCell` | `uint8_t` | 1 | 0.1V | Minimum cell voltage / 10 (`currentBatteryProfile->voltage.cellMin / 10`). 0 if `USE_ADC` disabled |
| `vbatMaxCell` | `uint8_t` | 1 | 0.1V | Maximum cell voltage / 10 (`currentBatteryProfile->voltage.cellMax / 10`). 0 if `USE_ADC` disabled |
| `vbatWarningCell` | `uint8_t` | 1 | 0.1V | Warning cell voltage / 10 (`currentBatteryProfile->voltage.cellWarning / 10`). 0 if `USE_ADC` disabled |

**Notes:** Superseded by `MSP2_INAV_BATTERY_CONFIG`.

## <a id="msp_set_voltage_meter_config"></a>`MSP_SET_VOLTAGE_METER_CONFIG (57 / 0x39)`
**Description:** Sets legacy voltage meter configuration (scaled values).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `vbatScale` | `uint8_t` | 1 | Scale / 10 | Sets `batteryMetersConfigMutable()->voltage.scale = value * 10` (if `USE_ADC`) |
| `vbatMinCell` | `uint8_t` | 1 | 0.1V | Sets `currentBatteryProfileMutable->voltage.cellMin = value * 10` (if `USE_ADC`) |
| `vbatMaxCell` | `uint8_t` | 1 | 0.1V | Sets `currentBatteryProfileMutable->voltage.cellMax = value * 10` (if `USE_ADC`) |
| `vbatWarningCell` | `uint8_t` | 1 | 0.1V | Sets `currentBatteryProfileMutable->voltage.cellWarning = value * 10` (if `USE_ADC`) |

**Reply Payload:** **None**  

**Notes:** Expects 4 bytes. Superseded by `MSP2_INAV_SET_BATTERY_CONFIG`.

## <a id="msp_sonar_altitude"></a>`MSP_SONAR_ALTITUDE (58 / 0x3a)`
**Description:** Retrieves the altitude measured by the primary rangefinder (sonar or lidar).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `rangefinderAltitude` | `int32_t` | 4 | cm | Latest altitude reading from the rangefinder (`rangefinderGetLatestAltitude()`). 0 if `USE_RANGEFINDER` disabled or no reading. |

## <a id="msp_rx_map"></a>`MSP_RX_MAP (64 / 0x40)`
**Description:** Retrieves the RC channel mapping array (AETR, etc.).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `rcMap` | `uint8_t[MAX_MAPPABLE_RX_INPUTS]` | 4 (MAX_MAPPABLE_RX_INPUTS) | Array defining the mapping from input channel index to logical function (Roll, Pitch, Yaw, Throttle, Aux1...) |

**Notes:** `MAX_MAPPABLE_RX_INPUTS` is currently 4 (Roll, Pitch, Yaw, Throttle).

## <a id="msp_set_rx_map"></a>`MSP_SET_RX_MAP (65 / 0x41)`
**Description:** Sets the RC channel mapping array.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `rcMap` | `uint8_t[MAX_MAPPABLE_RX_INPUTS]` | 4 (MAX_MAPPABLE_RX_INPUTS) | Array defining the new channel mapping |

**Reply Payload:** **None**  

**Notes:** Expects `MAX_MAPPABLE_RX_INPUTS` bytes (currently 4).

## <a id="msp_reboot"></a>`MSP_REBOOT (68 / 0x44)`
**Description:** Commands the flight controller to reboot.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** The FC sends an ACK *before* rebooting. The `mspPostProcessFn` is set to `mspRebootFn` to perform the reboot after the reply is sent. Will fail if the craft is armed.

## <a id="msp_dataflash_summary"></a>`MSP_DATAFLASH_SUMMARY (70 / 0x46)`
**Description:** Retrieves summary information about the onboard dataflash chip (if present and used for Blackbox via FlashFS).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `flashReady` | `uint8_t` | 1 | Boolean: 1 if flash chip is ready, 0 otherwise. (`flashIsReady()`). 0 if `USE_FLASHFS` disabled |
| `sectorCount` | `uint32_t` | 4 | Total number of sectors on the flash chip (`geometry->sectors`). 0 if `USE_FLASHFS` disabled |
| `totalSize` | `uint32_t` | 4 | Total size of the flash chip in bytes (`geometry->totalSize`). 0 if `USE_FLASHFS` disabled |
| `usedSize` | `uint32_t` | 4 | Currently used size in bytes (FlashFS offset) (`flashfsGetOffset()`). 0 if `USE_FLASHFS` disabled |

**Notes:** Requires `USE_FLASHFS`.

## <a id="msp_dataflash_read"></a>`MSP_DATAFLASH_READ (71 / 0x47)`
**Description:** Reads a block of data from the onboard dataflash (FlashFS).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `address` | `uint32_t` | 4 | Starting address to read from within the FlashFS volume |
| `size` | `uint16_t` | 2 | (Optional) Number of bytes to read. Defaults to 128 if not provided |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `address` | `uint32_t` | 4 | The starting address from which data was actually read |
| `data` | `uint8_t[]` | array | The data read from flash. Length is MIN(requested size, remaining buffer space, remaining flashfs data) |

**Notes:** Requires `USE_FLASHFS`. Read length may be truncated by buffer size or end of flashfs volume.

## <a id="msp_dataflash_erase"></a>`MSP_DATAFLASH_ERASE (72 / 0x48)`
**Description:** Erases the entire onboard dataflash chip (FlashFS volume).  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Requires `USE_FLASHFS`. This is a potentially long operation. Use with caution.

## <a id="msp_loop_time"></a>`MSP_LOOP_TIME (73 / 0x49)`
**Description:** Retrieves the configured loop time (PID loop frequency denominator).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `looptime` | `uint16_t` | 2 | PWM | Configured loop time (`gyroConfig()->looptime`) |

**Notes:** This is the *configured* target loop time, not necessarily the *actual* measured cycle time (see `MSP_STATUS`).

## <a id="msp_set_loop_time"></a>`MSP_SET_LOOP_TIME (74 / 0x4a)`
**Description:** Sets the configured loop time.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `looptime` | `uint16_t` | 2 | PWM | New loop time to set (`gyroConfigMutable()->looptime`) |

**Reply Payload:** **None**  

**Notes:** Expects 2 bytes.

## <a id="msp_failsafe_config"></a>`MSP_FAILSAFE_CONFIG (75 / 0x4b)`
**Description:** Retrieves the failsafe configuration settings.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `failsafeDelay` | `uint8_t` | 1 | 0.1s | Delay before failsafe stage 1 activates (`failsafeConfig()->failsafe_delay`) |
| `failsafeOffDelay` | `uint8_t` | 1 | 0.1s | Delay after signal recovery before returning control (`failsafeConfig()->failsafe_off_delay`) |
| `failsafeThrottle` | `uint16_t` | 2 | PWM | Throttle level during failsafe stage 2 (`currentBatteryProfile->failsafe_throttle`) |
| `legacyKillSwitch` | `uint8_t` | 1 | - | Legacy flag, always 0 |
| `failsafeThrottleLowDelay` | `uint16_t` | 2 | 0.1s | Delay for throttle-based failsafe detection (`failsafeConfig()->failsafe_throttle_low_delay`). Units of 0.1 seconds. |
| `failsafeProcedure` | `uint8_t` | 1 | [failsafeProcedure_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-failsafeprocedure_e) | Enum `failsafeProcedure_e` Failsafe procedure (Drop, RTH, Land, etc.) ('failsafeConfig()->failsafe_procedure') |
| `failsafeRecoveryDelay` | `uint8_t` | 1 | 0.1s | Delay after RTH finishes before attempting recovery (`failsafeConfig()->failsafe_recovery_delay`) |
| `failsafeFWRollAngle` | `int16_t` | 2 | deci-degrees | Fixed-wing failsafe roll angle (`failsafeConfig()->failsafe_fw_roll_angle`). Signed deci-degrees. |
| `failsafeFWPitchAngle` | `int16_t` | 2 | deci-degrees | Fixed-wing failsafe pitch angle (`failsafeConfig()->failsafe_fw_pitch_angle`). Signed deci-degrees. |
| `failsafeFWYawRate` | `int16_t` | 2 | deg/s | Fixed-wing failsafe yaw rate (`failsafeConfig()->failsafe_fw_yaw_rate`). Signed degrees per second. |
| `failsafeStickThreshold` | `uint16_t` | 2 | PWM units | Stick movement threshold to exit failsafe (`failsafeConfig()->failsafe_stick_motion_threshold`) |
| `failsafeMinDistance` | `uint16_t` | 2 | cm | Minimum distance from home for RTH failsafe (`failsafeConfig()->failsafe_min_distance`). Units of centimeters. |
| `failsafeMinDistanceProc` | `uint8_t` | 1 | [failsafeProcedure_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-failsafeprocedure_e) | Enum `failsafeProcedure_e` Failsafe procedure if below min distance ('failsafeConfig()->failsafe_min_distance_procedure') |

## <a id="msp_set_failsafe_config"></a>`MSP_SET_FAILSAFE_CONFIG (76 / 0x4c)`
**Description:** Sets the failsafe configuration settings.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `failsafeDelay` | `uint8_t` | 1 | 0.1s | Sets `failsafeConfigMutable()->failsafe_delay`. |
| `failsafeOffDelay` | `uint8_t` | 1 | 0.1s | Sets `failsafeConfigMutable()->failsafe_off_delay`. |
| `failsafeThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->failsafe_throttle`. |
| `legacyKillSwitch` | `uint8_t` | 1 | - | Ignored |
| `failsafeThrottleLowDelay` | `uint16_t` | 2 | 0.1s | Sets `failsafeConfigMutable()->failsafe_throttle_low_delay`. Units of 0.1 seconds. |
| `failsafeProcedure` | `uint8_t` | 1 | [failsafeProcedure_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-failsafeprocedure_e) | Enum `failsafeProcedure_e`. Sets `failsafeConfigMutable()->failsafe_procedure`. |
| `failsafeRecoveryDelay` | `uint8_t` | 1 | 0.1s | Sets `failsafeConfigMutable()->failsafe_recovery_delay`. |
| `failsafeFWRollAngle` | `int16_t` | 2 | deci-degrees | Sets `failsafeConfigMutable()->failsafe_fw_roll_angle`. Signed deci-degrees. |
| `failsafeFWPitchAngle` | `int16_t` | 2 | deci-degrees | Sets `failsafeConfigMutable()->failsafe_fw_pitch_angle`. Signed deci-degrees. |
| `failsafeFWYawRate` | `int16_t` | 2 | deg/s | Sets `failsafeConfigMutable()->failsafe_fw_yaw_rate`. Signed degrees per second. |
| `failsafeStickThreshold` | `uint16_t` | 2 | PWM units | Sets `failsafeConfigMutable()->failsafe_stick_motion_threshold`. |
| `failsafeMinDistance` | `uint16_t` | 2 | cm | Sets `failsafeConfigMutable()->failsafe_min_distance`. Units of centimeters. |
| `failsafeMinDistanceProc` | `uint8_t` | 1 | [failsafeProcedure_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-failsafeprocedure_e) | Enum `failsafeProcedure_e`. Sets `failsafeConfigMutable()->failsafe_min_distance_procedure`. |

**Reply Payload:** **None**  

**Notes:** Expects 20 bytes.

## <a id="msp_sdcard_summary"></a>`MSP_SDCARD_SUMMARY (79 / 0x4f)`
**Description:** Retrieves summary information about the SD card status and filesystem.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `sdCardSupported` | `uint8_t` | 1 | Bitmask | Bitmask: Bit 0 = 1 if SD card support compiled in (`USE_SDCARD`) |
| `sdCardState` | `uint8_t` | 1 | [mspSDCardState_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-mspsdcardstate_e) | Enum (`mspSDCardState_e`): Current state (Not Present, Fatal, Card Init, FS Init, Ready). 0 if `USE_SDCARD` disabled |
| `fsError` | `uint8_t` | 1 | - | Last filesystem error code (`afatfs_getLastError()`). 0 if `USE_SDCARD` disabled |
| `freeSpaceKB` | `uint32_t` | 4 | - | Free space in KiB (`afatfs_getContiguousFreeSpace() / 1024`). 0 if `USE_SDCARD` disabled |
| `totalSpaceKB` | `uint32_t` | 4 | - | Total space in KiB (`sdcard_getMetadata()->numBlocks / 2`). 0 if `USE_SDCARD` disabled |

**Notes:** Requires `USE_SDCARD` and `USE_ASYNCFATFS`.

## <a id="msp_blackbox_config"></a>`MSP_BLACKBOX_CONFIG (80 / 0x50)`
**Description:** Legacy command to retrieve Blackbox configuration. Superseded by `MSP2_BLACKBOX_CONFIG`.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `blackboxDevice` | `uint8_t` | 1 | Always 0 (API no longer supported) |
| `blackboxRateNum` | `uint8_t` | 1 | Always 0 |
| `blackboxRateDenom` | `uint8_t` | 1 | Always 0 |
| `blackboxPDenom` | `uint8_t` | 1 | Always 0 |

**Notes:** Returns fixed zero values. Use `MSP2_BLACKBOX_CONFIG`.

## <a id="msp_set_blackbox_config"></a>`MSP_SET_BLACKBOX_CONFIG (81 / 0x51)`
**Description:** Legacy command to set Blackbox configuration. Superseded by `MSP2_SET_BLACKBOX_CONFIG`.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Not implemented in `fc_msp.c`. Use `MSP2_SET_BLACKBOX_CONFIG`.

## <a id="msp_transponder_config"></a>`MSP_TRANSPONDER_CONFIG (82 / 0x52)`
**Description:** Get VTX Transponder settings (likely specific to RaceFlight/Betaflight, not standard INAV VTX).  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Not implemented in INAV `fc_msp.c`.

## <a id="msp_set_transponder_config"></a>`MSP_SET_TRANSPONDER_CONFIG (83 / 0x53)`
**Description:** Set VTX Transponder settings.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Not implemented in INAV `fc_msp.c`.

## <a id="msp_osd_config"></a>`MSP_OSD_CONFIG (84 / 0x54)`
**Description:** Retrieves OSD configuration settings and layout for screen 0. Coordinates are packed as `(Y << 8) | X`. When `USE_OSD` is not compiled in, only `osdDriverType` = `OSD_DRIVER_NONE` is returned.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `osdDriverType` | `uint8_t` | 1 | [osdDriver_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osddriver_e) | Enum `osdDriver_e`: `OSD_DRIVER_MAX7456` if `USE_OSD`, else `OSD_DRIVER_NONE`. |
| `videoSystem` | `uint8_t` | 1 | [videoSystem_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-videosystem_e) | Enum `videoSystem_e`: Video system (Auto/PAL/NTSC) (`osdConfig()->video_system`). Sent even if OSD disabled |
| `units` | `uint8_t` | 1 | [osd_unit_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osd_unit_e) | Enum `osd_unit_e` Measurement units (Metric/Imperial) (`osdConfig()->units`). Sent even if OSD disabled |
| `rssiAlarm` | `uint8_t` | 1 | % | RSSI alarm threshold (`osdConfig()->rssi_alarm`). Sent even if OSD disabled |
| `capAlarm` | `uint16_t` | 2 | mAh/mWh | Capacity alarm threshold (`currentBatteryProfile->capacity.warning`). Truncated to 16 bits. Sent even if OSD disabled. |
| `timerAlarm` | `uint16_t` | 2 | minutes | Timer alarm threshold in minutes (`osdConfig()->time_alarm`). Sent even if OSD disabled. |
| `altAlarm` | `uint16_t` | 2 | meters | Altitude alarm threshold (`osdConfig()->alt_alarm`). Sent even if OSD disabled |
| `distAlarm` | `uint16_t` | 2 | meters | Distance alarm threshold (`osdConfig()->dist_alarm`). Sent even if OSD disabled |
| `negAltAlarm` | `uint16_t` | 2 | meters | Negative altitude alarm threshold (`osdConfig()->neg_alt_alarm`). Sent even if OSD disabled |
| `itemPositions` | `uint16_t[OSD_ITEM_COUNT]` | OSD_ITEM_COUNT | packed | Packed X/Y position for each OSD item on screen 0 (`osdLayoutsConfig()->item_pos[0][i]`). Sent even if OSD disabled |

**Notes:** 1 byte if `USE_OSD` disabled; full payload (1 + fields + 2*OSD_ITEM_COUNT bytes) otherwise.

## <a id="msp_set_osd_config"></a>`MSP_SET_OSD_CONFIG (85 / 0x55)`
**Description:** Sets OSD configuration or a single item's position on screen 0.  
#### Variant: `dataSize >= 10`

**Description:** dataSize >= 10  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `selector` | `uint8_t` | 1 | - | Must be 0xFF (-1) to indicate a configuration update. |
| `videoSystem` | `uint8_t` | 1 | [videoSystem_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-videosystem_e) | Enum `videoSystem_e`: Video system (Auto/PAL/NTSC) (`osdConfig()->video_system`). |
| `units` | `uint8_t` | 1 | [osd_unit_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osd_unit_e) | Enum `osd_unit_e` Measurement units (Metric/Imperial) (`osdConfig()->units`). |
| `rssiAlarm` | `uint8_t` | 1 | % | RSSI alarm threshold (`osdConfig()->rssi_alarm`). |
| `capAlarm` | `uint16_t` | 2 | mAh/mWh | Capacity alarm threshold (`currentBatteryProfile->capacity.warning`). Truncated to 16 bits. |
| `timerAlarm` | `uint16_t` | 2 | minutes | Timer alarm threshold in minutes (`osdConfig()->time_alarm`). |
| `altAlarm` | `uint16_t` | 2 | meters | Altitude alarm threshold (`osdConfig()->alt_alarm`). |
| `distAlarm` | `uint16_t` | 2 | meters | Distance alarm threshold (`osdConfig()->dist_alarm`). Optional trailing field. |
| `negAltAlarm` | `uint16_t` | 2 | meters | Negative altitude alarm threshold (`osdConfig()->neg_alt_alarm`). Optional trailing field. |

**Reply Payload:** **None**  

#### Variant: `dataSize == 3`

**Description:** Single item position update  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `itemIndex` | `uint8_t` | 1 | Index | Index of the OSD item to update (0 to `OSD_ITEM_COUNT - 1`). |
| `itemPosition` | `uint16_t` | 2 | packed | Packed X/Y position (`(Y << 8) | X`) for the specified item. |

**Reply Payload:** **None**  


**Notes:** Requires `USE_OSD`. Distinguishes formats based on the first byte. Format 1 requires at least 10 bytes. Format 2 requires 3 bytes. Triggers an OSD redraw. See `MSP2_INAV_OSD_SET_*` for more advanced control.

## <a id="msp_osd_char_read"></a>`MSP_OSD_CHAR_READ (86 / 0x56)`
**Description:** Reads character data from the OSD font memory.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Not implemented in INAV `fc_msp.c`. Requires direct hardware access, typically done via DisplayPort.

## <a id="msp_osd_char_write"></a>`MSP_OSD_CHAR_WRITE (87 / 0x57)`
**Description:** Writes character data to the OSD font memory.  
#### Variant: `payloadSize >= OSD_CHAR_BYTES + 2 (>=66 bytes)`

**Description:** 16-bit character index with full 64-byte payload (visible + metadata).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `address` | `uint16_t` | 2 | Character slot index (0-1023). |
| `charData` | `uint8_t[OSD_CHAR_BYTES]` | 64 (OSD_CHAR_BYTES) | All 64 bytes, including driver metadata. |

**Reply Payload:** **None**  

#### Variant: `payloadSize == OSD_CHAR_BYTES + 1 (65 bytes)`

**Description:** 8-bit character index with full 64-byte payload.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `address` | `uint8_t` | 1 | Character slot index (0-255). |
| `charData` | `uint8_t[OSD_CHAR_BYTES]` | 64 (OSD_CHAR_BYTES) | All 64 bytes, including driver metadata. |

**Reply Payload:** **None**  

#### Variant: `payloadSize == OSD_CHAR_VISIBLE_BYTES + 2 (56 bytes)`

**Description:** 16-bit character index with only the 54 visible bytes.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `address` | `uint16_t` | 2 | Character slot index (0-1023). |
| `charData` | `uint8_t[OSD_CHAR_VISIBLE_BYTES]` | 54 (OSD_CHAR_VISIBLE_BYTES) | Visible pixel data only (no metadata). |

**Reply Payload:** **None**  

#### Variant: `payloadSize == OSD_CHAR_VISIBLE_BYTES + 1 (55 bytes)`

**Description:** 8-bit character index with only the 54 visible bytes.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `address` | `uint8_t` | 1 | Character slot index (0-255). |
| `charData` | `uint8_t[OSD_CHAR_VISIBLE_BYTES]` | 54 (OSD_CHAR_VISIBLE_BYTES) | Visible pixel data only (no metadata). |

**Reply Payload:** **None**  


**Notes:** Requires `USE_OSD`. Minimum payload is `OSD_CHAR_VISIBLE_BYTES + 1` (8-bit address + 54 bytes). Payload size determines the address width and whether the extra metadata bytes are present. Writes characters via `displayWriteFontCharacter()`.

## <a id="msp_vtx_config"></a>`MSP_VTX_CONFIG (88 / 0x58)`
**Description:** Retrieves the current VTX (Video Transmitter) configuration and capabilities.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `vtxDeviceType` | `uint8_t` | 1 | [vtxDevType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-vtxdevtype_e) | Enum (`vtxDevType_e`): Type of VTX device detected/configured. `VTXDEV_UNKNOWN` if none |
| `band` | `uint8_t` | 1 | - | VTX band number (from `vtxSettingsConfig`) |
| `channel` | `uint8_t` | 1 | - | VTX channel number (from `vtxSettingsConfig`) |
| `power` | `uint8_t` | 1 | - | VTX power level index (from `vtxSettingsConfig()`). |
| `pitMode` | `uint8_t` | 1 | - | Boolean: 1 if VTX is currently in pit mode, 0 otherwise. |
| `vtxReady` | `uint8_t` | 1 | - | Boolean: 1 if VTX device reported ready, 0 otherwise |
| `lowPowerDisarm` | `uint8_t` | 1 | [vtxLowerPowerDisarm_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-vtxlowerpowerdisarm_e) | Enum `vtxLowerPowerDisarm_e`: Low-power behaviour while disarmed (`vtxSettingsConfig()->lowPowerDisarm`). |
| `vtxTableAvailable` | `uint8_t` | 1 | - | Boolean: 1 if VTX tables (band/power) are available for query |
| `bandCount` | `uint8_t` | 1 | - | Number of bands supported by the VTX device |
| `channelCount` | `uint8_t` | 1 | - | Number of channels per band supported by the VTX device |
| `powerCount` | `uint8_t` | 1 | - | Number of power levels supported by the VTX device |

**Notes:** Returns 1 byte (`VTXDEV_UNKNOWN`) when no VTX is detected or `USE_VTX_CONTROL` is disabled; otherwise sends full payload. BF compatibility field `frequency` (uint16) is missing compared to some BF versions. Use `MSP_VTXTABLE_BAND` and `MSP_VTXTABLE_POWERLEVEL` for details.

## <a id="msp_set_vtx_config"></a>`MSP_SET_VTX_CONFIG (89 / 0x59)`
**Description:** Sets VTX band/channel and related options. Fields are a progressive superset based on payload length.  
#### Variant: `payloadSize >= 14`

**Description:** Full payload (Betaflight 1.42+): includes explicit band/channel/frequency and capability counts.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `bandChanOrFreq` | `uint16_t` | 2 | - | Encoded band/channel if <= `VTXCOMMON_MSP_BANDCHAN_CHKVAL`; otherwise frequency placeholder. |
| `power` | `uint8_t` | 1 | - |  |
| `pitMode` | `uint8_t` | 1 | - |  |
| `lowPowerDisarm` | `uint8_t` | 1 | [vtxLowerPowerDisarm_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-vtxlowerpowerdisarm_e) |  |
| `pitModeFreq` | `uint16_t` | 2 | - |  |
| `band` | `uint8_t` | 1 | - |  |
| `channel` | `uint8_t` | 1 | - |  |
| `frequency` | `uint16_t` | 2 | - |  |
| `bandCount` | `uint8_t` | 1 | - | Read and ignored. |
| `channelCount` | `uint8_t` | 1 | - | Read and ignored. |
| `powerCount` | `uint8_t` | 1 | - | If 0 < value < current capability, caps `vtxDevice->capability.powerCount`. |

**Reply Payload:** **None**  

#### Variant: `payloadSize >= 11`

**Description:** Extends payload with explicit frequency.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `bandChanOrFreq` | `uint16_t` | 2 | - |  |
| `power` | `uint8_t` | 1 | - |  |
| `pitMode` | `uint8_t` | 1 | - |  |
| `lowPowerDisarm` | `uint8_t` | 1 | [vtxLowerPowerDisarm_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-vtxlowerpowerdisarm_e) |  |
| `pitModeFreq` | `uint16_t` | 2 | - |  |
| `band` | `uint8_t` | 1 | - |  |
| `channel` | `uint8_t` | 1 | - |  |
| `frequency` | `uint16_t` | 2 | - | Read and ignored by INAV. |

**Reply Payload:** **None**  

#### Variant: `payloadSize >= 9`

**Description:** Adds explicit band/channel overrides (API 1.42 extension).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `bandChanOrFreq` | `uint16_t` | 2 | - |  |
| `power` | `uint8_t` | 1 | - |  |
| `pitMode` | `uint8_t` | 1 | - |  |
| `lowPowerDisarm` | `uint8_t` | 1 | [vtxLowerPowerDisarm_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-vtxlowerpowerdisarm_e) |  |
| `pitModeFreq` | `uint16_t` | 2 | - |  |
| `band` | `uint8_t` | 1 | - | 1..N; overrides band when present. |
| `channel` | `uint8_t` | 1 | - | 1..8; overrides channel when present. |

**Reply Payload:** **None**  

#### Variant: `payloadSize >= 7`

**Description:** Adds pit-mode frequency placeholder.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `bandChanOrFreq` | `uint16_t` | 2 | - |  |
| `power` | `uint8_t` | 1 | - |  |
| `pitMode` | `uint8_t` | 1 | - |  |
| `lowPowerDisarm` | `uint8_t` | 1 | [vtxLowerPowerDisarm_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-vtxlowerpowerdisarm_e) |  |
| `pitModeFreq` | `uint16_t` | 2 | - | Read and skipped. |

**Reply Payload:** **None**  

#### Variant: `payloadSize >= 5`

**Description:** Adds low-power disarm behaviour.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `bandChanOrFreq` | `uint16_t` | 2 | - |  |
| `power` | `uint8_t` | 1 | - |  |
| `pitMode` | `uint8_t` | 1 | - |  |
| `lowPowerDisarm` | `uint8_t` | 1 | [vtxLowerPowerDisarm_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-vtxlowerpowerdisarm_e) | 0=Off, 1=Always, 2=Until first arm. |

**Reply Payload:** **None**  

#### Variant: `payloadSize >= 4`

**Description:** Adds power index and pit mode flag.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `bandChanOrFreq` | `uint16_t` | 2 |  |
| `power` | `uint8_t` | 1 |  |
| `pitMode` | `uint8_t` | 1 |  |

**Reply Payload:** **None**  

#### Variant: `payloadSize == 2`

**Description:** Minimum payload (band/channel encoded in 0..63).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `bandChanOrFreq` | `uint16_t` | 2 | If <= `VTXCOMMON_MSP_BANDCHAN_CHKVAL`, decoded as band/channel; otherwise treated as a frequency placeholder. |

**Reply Payload:** **None**  


**Notes:** Requires dataSize >= 2. If no VTX device or device type is VTXDEV_UNKNOWN, fields are read and discarded. The first uint16 is interpreted as band/channel when value <= VTXCOMMON_MSP_BANDCHAN_CHKVAL, otherwise treated as a frequency value that is not applied by this path. Subsequent fields are applied only if present. If dataSize < 2 the command returns MSP_RESULT_ERROR.

## <a id="msp_advanced_config"></a>`MSP_ADVANCED_CONFIG (90 / 0x5a)`
**Description:** Retrieves advanced hardware-related configuration (PWM protocols, rates). Some fields are BF compatibility placeholders.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `gyroSyncDenom` | `uint8_t` | 1 | - | Always 1 (BF compatibility) |
| `pidProcessDenom` | `uint8_t` | 1 | - | Always 1 (BF compatibility) |
| `useUnsyncedPwm` | `uint8_t` | 1 | - | Always 1 (BF compatibility, INAV uses async PWM based on protocol) |
| `motorPwmProtocol` | `uint8_t` | 1 | [motorPwmProtocolTypes_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-motorpwmprotocoltypes_e) | Motor PWM protocol type (`motorConfig()->motorPwmProtocol`). |
| `motorPwmRate` | `uint16_t` | 2 | Hz | Motor PWM rate (if applicable) (`motorConfig()->motorPwmRate`). |
| `servoPwmRate` | `uint16_t` | 2 | Hz | Servo PWM rate (`servoConfig()->servoPwmRate`). |
| `legacyGyroSync` | `uint8_t` | 1 | - | Always 0 (BF compatibility) |

## <a id="msp_set_advanced_config"></a>`MSP_SET_ADVANCED_CONFIG (91 / 0x5b)`
**Description:** Sets advanced hardware-related configuration (PWM protocols, rates).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `gyroSyncDenom` | `uint8_t` | 1 | - | Ignored (legacy Betaflight field). |
| `pidProcessDenom` | `uint8_t` | 1 | - | Ignored (legacy Betaflight field). |
| `useUnsyncedPwm` | `uint8_t` | 1 | - | Ignored (legacy Betaflight field). |
| `motorPwmProtocol` | `uint8_t` | 1 | [motorPwmProtocolTypes_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-motorpwmprotocoltypes_e) | Sets `motorConfigMutable()->motorPwmProtocol`. |
| `motorPwmRate` | `uint16_t` | 2 | Hz | Sets `motorConfigMutable()->motorPwmRate`. |
| `servoPwmRate` | `uint16_t` | 2 | Hz | Sets `servoConfigMutable()->servoPwmRate`. |
| `legacyGyroSync` | `uint8_t` | 1 | - | Ignored (legacy Betaflight field). |

**Reply Payload:** **None**  

**Notes:** Expects 9 bytes.

## <a id="msp_filter_config"></a>`MSP_FILTER_CONFIG (92 / 0x5c)`
**Description:** Retrieves filter configuration settings (Gyro, D-term, Yaw, Accel). Some fields are BF compatibility placeholders or legacy.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `gyroMainLpfHz` | `uint8_t` | 1 | Hz | Gyro main low-pass filter cutoff frequency (`gyroConfig()->gyro_main_lpf_hz`) |
| `dtermLpfHz` | `uint16_t` | 2 | Hz | D-term low-pass filter cutoff frequency (`pidProfile()->dterm_lpf_hz`) |
| `yawLpfHz` | `uint16_t` | 2 | Hz | Yaw low-pass filter cutoff frequency (`pidProfile()->yaw_lpf_hz`) |
| `legacyGyroNotchHz` | `uint16_t` | 2 | - | Always 0 (Legacy) |
| `legacyGyroNotchCutoff` | `uint16_t` | 2 | - | Always 1 (Legacy) |
| `bfCompatDtermNotchHz` | `uint16_t` | 2 | - | Always 0 (BF compatibility) |
| `bfCompatDtermNotchCutoff` | `uint16_t` | 2 | - | Always 1 (BF compatibility) |
| `bfCompatGyroNotch2Hz` | `uint16_t` | 2 | - | Always 0 (BF compatibility) |
| `bfCompatGyroNotch2Cutoff` | `uint16_t` | 2 | - | Always 1 (BF compatibility) |
| `accNotchHz` | `uint16_t` | 2 | Hz | Accelerometer notch filter center frequency (`accelerometerConfig()->acc_notch_hz`) |
| `accNotchCutoff` | `uint16_t` | 2 | Hz | Accelerometer notch filter cutoff frequency (`accelerometerConfig()->acc_notch_cutoff`) |
| `legacyGyroStage2LpfHz` | `uint16_t` | 2 | - | Always 0 (Legacy) |

## <a id="msp_set_filter_config"></a>`MSP_SET_FILTER_CONFIG (93 / 0x5d)`
**Description:** Sets filter configuration settings. Handles different payload lengths for backward compatibility.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `gyroMainLpfHz` | `uint8_t` | 1 | Hz | Sets `gyroConfigMutable()->gyro_main_lpf_hz`. (Size >= 5) |
| `dtermLpfHz` | `uint16_t` | 2 | Hz | Sets `pidProfileMutable()->dterm_lpf_hz` (constrained 0-500). (Size >= 5) |
| `yawLpfHz` | `uint16_t` | 2 | Hz | Sets `pidProfileMutable()->yaw_lpf_hz` (constrained 0-255). (Size >= 5) |
| `legacyGyroNotchHz` | `uint16_t` | 2 | - | Ignored. (Size >= 9) |
| `legacyGyroNotchCutoff` | `uint16_t` | 2 | - | Ignored. (Size >= 9) |
| `bfCompatDtermNotchHz` | `uint16_t` | 2 | - | Ignored. (Size >= 13) |
| `bfCompatDtermNotchCutoff` | `uint16_t` | 2 | - | Ignored. (Size >= 13) |
| `bfCompatGyroNotch2Hz` | `uint16_t` | 2 | - | Ignored. (Size >= 17) |
| `bfCompatGyroNotch2Cutoff` | `uint16_t` | 2 | - | Ignored. (Size >= 17) |
| `accNotchHz` | `uint16_t` | 2 | Hz | Sets `accelerometerConfigMutable()->acc_notch_hz` (constrained 0-255). (Size >= 21) |
| `accNotchCutoff` | `uint16_t` | 2 | Hz | Sets `accelerometerConfigMutable()->acc_notch_cutoff` (constrained 1-255). (Size >= 21) |
| `legacyGyroStage2LpfHz` | `uint16_t` | 2 | - | Ignored. (Size >= 22) |

**Reply Payload:** **None**  

**Notes:** Requires at least 22 bytes; intermediate length checks enforce legacy Betaflight frame layout and call `pidInitFilters()` once the D-term notch placeholders are consumed.

## <a id="msp_pid_advanced"></a>`MSP_PID_ADVANCED (94 / 0x5e)`
**Description:** Retrieves advanced PID tuning parameters. Many fields are BF compatibility placeholders.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `legacyRollPitchItermIgnore` | `uint16_t` | 2 | - | Always 0 (Legacy) |
| `legacyYawItermIgnore` | `uint16_t` | 2 | - | Always 0 (Legacy) |
| `legacyYawPLimit` | `uint16_t` | 2 | - | Always 0 (Legacy) |
| `bfCompatDeltaMethod` | `uint8_t` | 1 | - | Always 0 (BF compatibility) |
| `bfCompatVbatPidComp` | `uint8_t` | 1 | - | Always 0 (BF compatibility) |
| `bfCompatSetpointRelaxRatio` | `uint8_t` | 1 | - | Always 0 (BF compatibility) |
| `reserved1` | `uint8_t` | 1 | - | Always 0 |
| `legacyPidSumLimit` | `uint16_t` | 2 | - | Always 0 (Legacy) |
| `bfCompatItermThrottleGain` | `uint8_t` | 1 | - | Always 0 (BF compatibility) |
| `accelLimitRollPitch` | `uint16_t` | 2 | dps / 10 | Axis acceleration limit for Roll/Pitch / 10 (`pidProfile()->axisAccelerationLimitRollPitch / 10`) |
| `accelLimitYaw` | `uint16_t` | 2 | dps / 10 | Axis acceleration limit for Yaw / 10 (`pidProfile()->axisAccelerationLimitYaw / 10`) |

**Notes:** Acceleration limits are scaled by 10 for compatibility.

## <a id="msp_set_pid_advanced"></a>`MSP_SET_PID_ADVANCED (95 / 0x5f)`
**Description:** Sets advanced PID tuning parameters.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `legacyRollPitchItermIgnore` | `uint16_t` | 2 | - | Ignored (legacy compatibility). |
| `legacyYawItermIgnore` | `uint16_t` | 2 | - | Ignored (legacy compatibility). |
| `legacyYawPLimit` | `uint16_t` | 2 | - | Ignored (legacy compatibility). |
| `bfCompatDeltaMethod` | `uint8_t` | 1 | - | Ignored (BF compatibility). |
| `bfCompatVbatPidComp` | `uint8_t` | 1 | - | Ignored (BF compatibility). |
| `bfCompatSetpointRelaxRatio` | `uint8_t` | 1 | - | Ignored (BF compatibility). |
| `reserved1` | `uint8_t` | 1 | - | Ignored (reserved). |
| `legacyPidSumLimit` | `uint16_t` | 2 | - | Ignored (legacy compatibility). |
| `bfCompatItermThrottleGain` | `uint8_t` | 1 | - | Ignored (BF compatibility). |
| `accelLimitRollPitch` | `uint16_t` | 2 | dps / 10 | Sets `pidProfileMutable()->axisAccelerationLimitRollPitch = value * 10`. |
| `accelLimitYaw` | `uint16_t` | 2 | dps / 10 | Sets `pidProfileMutable()->axisAccelerationLimitYaw = value * 10`. |

**Reply Payload:** **None**  

**Notes:** Expects 17 bytes.

## <a id="msp_sensor_config"></a>`MSP_SENSOR_CONFIG (96 / 0x60)`
**Description:** Retrieves the configured hardware type for various sensors.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `accHardware` | `uint8_t` | 1 | [accelerationSensor_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-accelerationsensor_e) | Enum (`accelerationSensor_e`): Accelerometer hardware type (`accelerometerConfig()->acc_hardware`) |
| `baroHardware` | `uint8_t` | 1 | [baroSensor_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-barosensor_e) | Enum (`baroSensor_e`): Barometer hardware type (`barometerConfig()->baro_hardware`). 0 if `USE_BARO` disabled |
| `magHardware` | `uint8_t` | 1 | [magSensor_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-magsensor_e) | Enum (`magSensor_e`): Magnetometer hardware type (`compassConfig()->mag_hardware`). 0 if `USE_MAG` disabled |
| `pitotHardware` | `uint8_t` | 1 | [pitotSensor_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-pitotsensor_e) | Enum (`pitotSensor_e`): Pitot tube hardware type (`pitotmeterConfig()->pitot_hardware`). 0 if `USE_PITOT` disabled |
| `rangefinderHardware` | `uint8_t` | 1 | [rangefinderType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-rangefindertype_e) | Enum (`rangefinderType_e`): Rangefinder hardware type (`rangefinderConfig()->rangefinder_hardware`). 0 if `USE_RANGEFINDER` disabled |
| `opflowHardware` | `uint8_t` | 1 | [opticalFlowSensor_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-opticalflowsensor_e) | Enum (`opticalFlowSensor_e`): Optical flow hardware type (`opticalFlowConfig()->opflow_hardware`). 0 if `USE_OPFLOW` disabled |

## <a id="msp_set_sensor_config"></a>`MSP_SET_SENSOR_CONFIG (97 / 0x61)`
**Description:** Sets the configured hardware type for various sensors.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `accHardware` | `uint8_t` | 1 | [accelerationSensor_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-accelerationsensor_e) | Sets `accelerometerConfigMutable()->acc_hardware` |
| `baroHardware` | `uint8_t` | 1 | [baroSensor_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-barosensor_e) | Sets `barometerConfigMutable()->baro_hardware` (if `USE_BARO`) |
| `magHardware` | `uint8_t` | 1 | [magSensor_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-magsensor_e) | Sets `compassConfigMutable()->mag_hardware` (if `USE_MAG`) |
| `pitotHardware` | `uint8_t` | 1 | [pitotSensor_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-pitotsensor_e) | Sets `pitotmeterConfigMutable()->pitot_hardware` (if `USE_PITOT`) |
| `rangefinderHardware` | `uint8_t` | 1 | [rangefinderType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-rangefindertype_e) | Sets `rangefinderConfigMutable()->rangefinder_hardware` (if `USE_RANGEFINDER`) |
| `opflowHardware` | `uint8_t` | 1 | [opticalFlowSensor_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-opticalflowsensor_e) | Sets `opticalFlowConfigMutable()->opflow_hardware` (if `USE_OPFLOW`) |

**Reply Payload:** **None**  

**Notes:** Expects 6 bytes.

## <a id="msp_special_parameters"></a>`MSP_SPECIAL_PARAMETERS (98 / 0x62)`
**Description:** Betaflight specific  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Not implemented in INAV `fc_msp.c`.

## <a id="msp_set_special_parameters"></a>`MSP_SET_SPECIAL_PARAMETERS (99 / 0x63)`
**Description:** Betaflight specific  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Not implemented in INAV `fc_msp.c`.

## <a id="msp_ident"></a>`MSP_IDENT (100 / 0x64)`
**Description:** Provides basic flight controller identity information. Not implemented in modern INAV, but used by legacy versions and MultiWii.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `MultiWii version` | `uint8_t` | 1 | n/a | Scaled version major*100+minor |
| `Mixer Mode` | `uint8_t` | 1 | Enum | Mixer type |
| `MSP Version` | `uint8_t` | 1 | n/a | Scaled version major*100+minor |
| `Platform Capability` | `uint32_t` | 4 | Bitmask | Bitmask: MW capabilities |

**Notes:** Obsolete. Listed for legacy compatibility only.

## <a id="msp_status"></a>`MSP_STATUS (101 / 0x65)`
**Description:** Provides basic flight controller status including cycle time, errors, sensor status, active modes (first 32), and the current configuration profile.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `cycleTime` | `uint16_t` | 2 | µs | Main loop cycle time (`cycleTime`) |
| `i2cErrors` | `uint16_t` | 2 | Count | Number of I2C errors encountered (`i2cGetErrorCounter()`). 0 if `USE_I2C` not defined |
| `sensorStatus` | `uint16_t` | 2 | Bitmask | Bitmask: available/active sensors (`packSensorStatus()`). See notes |
| `activeModesLow` | `uint32_t` | 4 | Bitmask | Bitmask: First 32 bits of the active flight modes bitmask (`packBoxModeFlags()`) |
| `profile` | `uint8_t` | 1 | Index | Current configuration profile index (0-based) (`getConfigProfile()`) |

**Notes:** Superseded by `MSP_STATUS_EX` and `MSP2_INAV_STATUS`. `sensorStatus` bitmask: (Bit 0: ACC, 1: BARO, 2: MAG, 3: GPS, 4: RANGEFINDER, 5: OPFLOW, 6: PITOT, 7: TEMP; Bit 15: hardware failure). `activeModesLow` only contains the first 32 modes; use `MSP_ACTIVEBOXES` for the full set.

## <a id="msp_raw_imu"></a>`MSP_RAW_IMU (102 / 0x66)`
**Description:** Provides raw sensor readings from the IMU (Accelerometer, Gyroscope, Magnetometer).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `accX` | `int16_t` | 2 | ~1/512 G | Raw accelerometer X reading, scaled (`acc.accADCf[X] * 512`) |
| `accY` | `int16_t` | 2 | ~1/512 G | Raw accelerometer Y reading, scaled (`acc.accADCf[Y] * 512`) |
| `accZ` | `int16_t` | 2 | ~1/512 G | Raw accelerometer Z reading, scaled (`acc.accADCf[Z] * 512`) |
| `gyroX` | `int16_t` | 2 | deg/s | Gyroscope X-axis rate (`gyroRateDps(X)`) |
| `gyroY` | `int16_t` | 2 | deg/s | Gyroscope Y-axis rate (`gyroRateDps(Y)`) |
| `gyroZ` | `int16_t` | 2 | deg/s | Gyroscope Z-axis rate (`gyroRateDps(Z)`) |
| `magX` | `int16_t` | 2 | Raw units | Raw magnetometer X reading (`mag.magADC[X]`). 0 if `USE_MAG` disabled |
| `magY` | `int16_t` | 2 | Raw units | Raw magnetometer Y reading (`mag.magADC[Y]`). 0 if `USE_MAG` disabled |
| `magZ` | `int16_t` | 2 | Raw units | Raw magnetometer Z reading (`mag.magADC[Z]`). 0 if `USE_MAG` disabled |

**Notes:** Acc scaling is approximate (512 LSB/G). Mag units depend on the sensor.

## <a id="msp_servo"></a>`MSP_SERVO (103 / 0x67)`
**Description:** Provides the current output values for all supported servos.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `servoOutputs` | `int16_t[MAX_SUPPORTED_SERVOS]` | 36 (MAX_SUPPORTED_SERVOS) | PWM | Array of current servo output values (typically 1000-2000) |

## <a id="msp_motor"></a>`MSP_MOTOR (104 / 0x68)`
**Description:** Provides the current output values for the first 8 motors.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `motorOutputs` | `int16_t[8]` | 16 | PWM | Array of current motor output values (typically 1000-2000). Values beyond `MAX_SUPPORTED_MOTORS` are 0 |

## <a id="msp_rc"></a>`MSP_RC (105 / 0x69)`
**Description:** Provides the current values of the received RC channels.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `rcChannels` | `int16_t[]` | array | PWM | Array of current RC channel values (typically 1000-2000). Length depends on detected channels |

**Notes:** Array length equals `rxRuntimeConfig.channelCount`.

## <a id="msp_raw_gps"></a>`MSP_RAW_GPS (106 / 0x6a)`
**Description:** Provides raw GPS data (fix status, coordinates, altitude, speed, course).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `fixType` | `uint8_t` | 1 | [gpsFixType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-gpsfixtype_e) | Enum `gpsFixType_e` GPS fix type (`gpsSol.fixType`) |
| `numSat` | `uint8_t` | 1 | Count | Number of satellites used in solution (`gpsSol.numSat`) |
| `latitude` | `int32_t` | 4 | deg * 1e7 | Latitude (`gpsSol.llh.lat`) |
| `longitude` | `int32_t` | 4 | deg * 1e7 | Longitude (`gpsSol.llh.lon`) |
| `altitude` | `int16_t` | 2 | cm | Altitude above MSL (`gpsSol.llh.alt`) sent as centimeters |
| `speed` | `int16_t` | 2 | cm/s | Ground speed (`gpsSol.groundSpeed`) |
| `groundCourse` | `int16_t` | 2 | deci-degrees | Ground course (`gpsSol.groundCourse`) |
| `hdop` | `uint16_t` | 2 | HDOP * 100 | Horizontal Dilution of Precision (`gpsSol.hdop`) |

**Notes:** Only available if `USE_GPS` is defined. Altitude is truncated to meters.

## <a id="msp_comp_gps"></a>`MSP_COMP_GPS (107 / 0x6b)`
**Description:** Provides computed GPS values: distance and direction to home.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `distanceToHome` | `uint16_t` | 2 | meters | Distance to the home point (`GPS_distanceToHome`) |
| `directionToHome` | `int16_t` | 2 | degrees | Direction to the home point (0-360) (`GPS_directionToHome`) |
| `gpsHeartbeat` | `uint8_t` | 1 | Boolean | Indicates if GPS data is being received (`gpsSol.flags.gpsHeartbeat`) |

**Notes:** Only available if `USE_GPS` is defined.

## <a id="msp_attitude"></a>`MSP_ATTITUDE (108 / 0x6c)`
**Description:** Provides the current attitude estimate (roll, pitch, yaw).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `roll` | `int16_t` | 2 | deci-degrees | Roll angle (`attitude.values.roll`) |
| `pitch` | `int16_t` | 2 | deci-degrees | Pitch angle (`attitude.values.pitch`) |
| `yaw` | `int16_t` | 2 | degrees | Yaw/Heading angle (`DECIDEGREES_TO_DEGREES(attitude.values.yaw)`) |

**Notes:** Yaw is in degrees.

## <a id="msp_altitude"></a>`MSP_ALTITUDE (109 / 0x6d)`
**Description:** Provides estimated altitude, vertical speed (variometer), and raw barometric altitude.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `estimatedAltitude` | `int32_t` | 4 | cm | Estimated altitude above home/sea level (`getEstimatedActualPosition(Z)`) |
| `variometer` | `int16_t` | 2 | cm/s | Estimated vertical speed (`getEstimatedActualVelocity(Z)`) |
| `baroAltitude` | `int32_t` | 4 | cm | Latest raw altitude from barometer (`baroGetLatestAltitude()`). 0 if `USE_BARO` disabled |

## <a id="msp_analog"></a>`MSP_ANALOG (110 / 0x6e)`
**Description:** Provides analog sensor readings: battery voltage, current consumption (mAh), RSSI, and current draw (Amps).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `vbat` | `uint8_t` | 1 | 0.1V | Battery voltage, scaled (`getBatteryVoltage() / 10`), constrained 0-255 |
| `mAhDrawn` | `uint16_t` | 2 | mAh | Consumed battery capacity (`getMAhDrawn()`), constrained 0-65535 |
| `rssi` | `uint16_t` | 2 | 0-1023 or % | Received Signal Strength Indicator (`getRSSI()`). Units depend on source |
| `amperage` | `int16_t` | 2 | 0.01A | Current draw (`getAmperage()`), constrained -32768 to 32767 |

**Notes:** Superseded by `MSP2_INAV_ANALOG` which provides higher precision and more fields.

## <a id="msp_rc_tuning"></a>`MSP_RC_TUNING (111 / 0x6f)`
**Description:** Retrieves RC tuning parameters (rates, expos, TPA) for the current control rate profile.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `legacyRcRate` | `uint8_t` | 1 | Always 100 (Legacy, unused) |
| `rcExpo` | `uint8_t` | 1 | Roll/Pitch RC Expo (`currentControlRateProfile->stabilized.rcExpo8`) |
| `rollRate` | `uint8_t` | 1 | Roll Rate (`currentControlRateProfile->stabilized.rates[FD_ROLL]`) |
| `pitchRate` | `uint8_t` | 1 | Pitch Rate (`currentControlRateProfile->stabilized.rates[FD_PITCH]`) |
| `yawRate` | `uint8_t` | 1 | Yaw Rate (`currentControlRateProfile->stabilized.rates[FD_YAW]`) |
| `dynamicThrottlePID` | `uint8_t` | 1 | Dynamic Throttle PID (TPA) value (`currentControlRateProfile->throttle.dynPID`) |
| `throttleMid` | `uint8_t` | 1 | Throttle Midpoint (`currentControlRateProfile->throttle.rcMid8`) |
| `throttleExpo` | `uint8_t` | 1 | Throttle Expo (`currentControlRateProfile->throttle.rcExpo8`) |
| `tpaBreakpoint` | `uint16_t` | 2 | Throttle PID Attenuation (TPA) breakpoint (`currentControlRateProfile->throttle.pa_breakpoint`) |
| `rcYawExpo` | `uint8_t` | 1 | Yaw RC Expo (`currentControlRateProfile->stabilized.rcYawExpo8`) |

**Notes:** Superseded by `MSP2_INAV_RATE_PROFILE` which includes manual rates/expos.

## <a id="msp_activeboxes"></a>`MSP_ACTIVEBOXES (113 / 0x71)`
**Description:** Provides the full bitmask of currently active flight modes (boxes).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `activeModes` | `boxBitmask_t` | - | Bitmask | Bitmask: all active modes (`packBoxModeFlags()`). Size depends on `boxBitmask_t` definition |

**Notes:** Use this instead of `MSP_STATUS` or `MSP_STATUS_EX` if more than 32 modes are possible.

## <a id="msp_misc"></a>`MSP_MISC (114 / 0x72)`
**Description:** Retrieves miscellaneous configuration settings, mostly related to RC, GPS, Mag, and Battery voltage (legacy formats).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `midRc` | `uint16_t` | 2 | PWM | Mid RC value (`PWM_RANGE_MIDDLE`, typically 1500) |
| `legacyMinThrottle` | `uint16_t` | 2 | - | Always 0 (Legacy) |
| `maxThrottle` | `uint16_t` | 2 | PWM | Maximum throttle command (`getMaxThrottle()`) |
| `minCommand` | `uint16_t` | 2 | PWM | Minimum motor command when disarmed (`motorConfig()->mincommand`) |
| `failsafeThrottle` | `uint16_t` | 2 | PWM | Failsafe throttle level (`currentBatteryProfile->failsafe_throttle`) |
| `gpsType` | `uint8_t` | 1 | [gpsProvider_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-gpsprovider_e) | Enum `gpsProvider_e` GPS provider type (`gpsConfig()->provider`). 0 if `USE_GPS` disabled |
| `legacyGpsBaud` | `uint8_t` | 1 | - | Always 0 (Legacy) |
| `gpsSbasMode` | `uint8_t` | 1 | [sbasMode_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-sbasmode_e) | Enum `sbasMode_e` GPS SBAS mode (`gpsConfig()->sbasMode`). 0 if `USE_GPS` disabled |
| `legacyMwCurrentOut` | `uint8_t` | 1 | - | Always 0 (Legacy) |
| `rssiChannel` | `uint8_t` | 1 | Index | RSSI channel index (1-based) (`rxConfig()->rssi_channel`) |
| `reserved1` | `uint8_t` | 1 | - | Always 0 |
| `magDeclination` | `uint16_t` | 2 | 0.1 degrees | Magnetic declination / 10 (`compassConfig()->mag_declination / 10`). 0 if `USE_MAG` disabled |
| `vbatScale` | `uint8_t` | 1 | Scale / 10 | Voltage scale / 10 (`batteryMetersConfig()->voltage.scale / 10`). 0 if `USE_ADC` disabled |
| `vbatMinCell` | `uint8_t` | 1 | 0.1V | Min cell voltage / 10 (`currentBatteryProfile->voltage.cellMin / 10`). 0 if `USE_ADC` disabled |
| `vbatMaxCell` | `uint8_t` | 1 | 0.1V | Max cell voltage / 10 (`currentBatteryProfile->voltage.cellMax / 10`). 0 if `USE_ADC` disabled |
| `vbatWarningCell` | `uint8_t` | 1 | 0.1V | Warning cell voltage / 10 (`currentBatteryProfile->voltage.cellWarning / 10`). 0 if `USE_ADC` disabled |

**Notes:** Superseded by `MSP2_INAV_MISC` and other specific commands which offer better precision and more fields.

## <a id="msp_boxnames"></a>`MSP_BOXNAMES (116 / 0x74)`
**Description:** Provides a semicolon-separated string containing the names of all available flight modes (boxes).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `boxNamesString` | `char[]` | array | String containing mode names separated by ';'. Null termination not guaranteed by MSP, relies on payload size. (`serializeBoxNamesReply()`) |

**Notes:** The exact set of names depends on compiled features and configuration. Due to the size of the payload, it is recommended that [`MSP_BOXIDS`](#msp_boxids-119--0x77) is used instead.

## <a id="msp_pidnames"></a>`MSP_PIDNAMES (117 / 0x75)`
**Description:** Provides a semicolon-separated string containing the names of the PID controllers.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `pidNamesString` | `char[]` | array | String "ROLL;PITCH;YAW;ALT;Pos;PosR;NavR;LEVEL;MAG;VEL;". Null termination not guaranteed by MSP |

## <a id="msp_wp"></a>`MSP_WP (118 / 0x76)`
**Description:** Get/Set a single waypoint from the mission plan.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `waypointIndex` | `uint8_t` | 1 | Index of the waypoint to retrieve (0 to `NAV_MAX_WAYPOINTS - 1`) |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `waypointIndex` | `uint8_t` | 1 | Index | Index of the returned waypoint |
| `action` | `uint8_t` | 1 | [navWaypointActions_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-navwaypointactions_e) | Enum `navWaypointActions_e` Waypoint action type |
| `latitude` | `int32_t` | 4 | deg * 1e7 | Latitude coordinate |
| `longitude` | `int32_t` | 4 | deg * 1e7 | Longitude coordinate |
| `altitude` | `int32_t` | 4 | cm | Altitude coordinate (relative to home or sea level, see flag) |
| `param1` | `int16_t` | 2 | Varies | Parameter 1 (meaning depends on action) |
| `param2` | `int16_t` | 2 | Varies | Parameter 2 (meaning depends on action) |
| `param3` | `int16_t` | 2 | Varies | Parameter 3 (meaning depends on action) |
| `flag` | `uint8_t` | 1 | Bitmask | Bitmask: Waypoint flags (`NAV_WP_FLAG_*`) |

**Notes:** See `navWaypoint_t` and `navWaypointActions_e`.

## <a id="msp_boxids"></a>`MSP_BOXIDS (119 / 0x77)`
**Description:** Provides a list of permanent IDs associated with the available flight modes (boxes).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `boxIds` | `uint8_t[]` | array | Array of permanent IDs for each configured box (`serializeBoxReply()`). Length depends on number of boxes |

**Notes:** Useful for mapping mode range configurations (`MSP_MODE_RANGES`) back to user-understandable modes via `MSP_BOXNAMES`.

## <a id="msp_servo_configurations"></a>`MSP_SERVO_CONFIGURATIONS (120 / 0x78)`
**Description:** Retrieves the configuration parameters for all supported servos (min, max, middle, rate). Legacy format with unused fields.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `min` | `int16_t` | 2 | PWM | Minimum servo endpoint (`servoParams(i)->min`) |
| `max` | `int16_t` | 2 | PWM | Maximum servo endpoint (`servoParams(i)->max`) |
| `middle` | `int16_t` | 2 | PWM | Middle/Neutral servo position (`servoParams(i)->middle`) |
| `rate` | `int8_t` | 1 | % (-100 to 100) | Servo rate/scaling (`servoParams(i)->rate`, -125..125). Encoded as two's complement |
| `reserved1` | `uint8_t` | 1 | - | Always 0 |
| `reserved2` | `uint8_t` | 1 | - | Always 0 |
| `legacyForwardChan` | `uint8_t` | 1 | - | Always 255 (Legacy) |
| `legacyReversedSources` | `uint32_t` | 4 | - | Always 0 (Legacy) |

**Notes:** Superseded by `MSP2_INAV_SERVO_CONFIG` which has a cleaner structure.

## <a id="msp_nav_status"></a>`MSP_NAV_STATUS (121 / 0x79)`
**Description:** Retrieves the current status of the navigation system.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `navMode` | `uint8_t` | 1 | [navSystemStatus_Mode_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-navsystemstatus_mode_e) | Enum (`navSystemStatus_Mode_e`): Current navigation mode (None, RTH, NAV, Hold, etc.) (`NAV_Status.mode`) |
| `navState` | `uint8_t` | 1 | [navSystemStatus_State_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-navsystemstatus_state_e) | Enum (`navSystemStatus_State_e`): Current navigation state (`NAV_Status.state`) |
| `activeWpAction` | `uint8_t` | 1 | [navWaypointActions_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-navwaypointactions_e) | Enum (`navWaypointActions_e`): Action of the currently executing waypoint (`NAV_Status.activeWpAction`) |
| `activeWpNumber` | `uint8_t` | 1 | - | Index: Index of the currently executing waypoint (`NAV_Status.activeWpNumber`) |
| `navError` | `uint8_t` | 1 | [navSystemStatus_Error_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-navsystemstatus_error_e) | Enum (`navSystemStatus_Error_e`): Current navigation error code (`NAV_Status.error`) |
| `targetHeading` | `int16_t` | 2 | degrees | Target heading for heading controller (`getHeadingHoldTarget()`) |

**Notes:** Requires `USE_GPS`.

## <a id="msp_nav_config"></a>`MSP_NAV_CONFIG (122 / 0x7a)`

**Request Payload:** **None**  

**Reply Payload:** **None**  

## <a id="msp_3d"></a>`MSP_3D (124 / 0x7c)`
**Description:** Retrieves settings related to 3D/reversible motor operation.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `deadbandLow` | `uint16_t` | 2 | PWM | Lower deadband limit for 3D mode (`reversibleMotorsConfig()->deadband_low`) |
| `deadbandHigh` | `uint16_t` | 2 | PWM | Upper deadband limit for 3D mode (`reversibleMotorsConfig()->deadband_high`) |
| `neutral` | `uint16_t` | 2 | PWM | Neutral throttle point for 3D mode (`reversibleMotorsConfig()->neutral`) |

**Notes:** Requires reversible motor support.

## <a id="msp_rc_deadband"></a>`MSP_RC_DEADBAND (125 / 0x7d)`
**Description:** Retrieves RC input deadband settings.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `deadband` | `uint8_t` | 1 | PWM | General RC deadband for Roll/Pitch (`rcControlsConfig()->deadband`) |
| `yawDeadband` | `uint8_t` | 1 | PWM | Specific deadband for Yaw (`rcControlsConfig()->yaw_deadband`) |
| `altHoldDeadband` | `uint8_t` | 1 | PWM | Deadband for altitude hold adjustments (`rcControlsConfig()->alt_hold_deadband`) |
| `throttleDeadband` | `uint16_t` | 2 | PWM | Deadband around throttle mid-stick (`rcControlsConfig()->mid_throttle_deadband`) |

## <a id="msp_sensor_alignment"></a>`MSP_SENSOR_ALIGNMENT (126 / 0x7e)`
**Description:** Retrieves sensor alignment settings (legacy format).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `gyroAlign` | `uint8_t` | 1 | Always 0 (Legacy alignment enum) |
| `accAlign` | `uint8_t` | 1 | Always 0 (Legacy alignment enum) |
| `magAlign` | `uint8_t` | 1 | Magnetometer alignment (`compassConfig()->mag_align`). 0 if `USE_MAG` disabled |
| `opflowAlign` | `uint8_t` | 1 | Optical flow alignment (`opticalFlowConfig()->opflow_align`). 0 if `USE_OPFLOW` disabled |

**Notes:** Board alignment is now typically handled by `MSP_BOARD_ALIGNMENT`. This returns legacy enum values where applicable.

## <a id="msp_led_strip_modecolor"></a>`MSP_LED_STRIP_MODECOLOR (127 / 0x7f)`
**Description:** Retrieves the color index assigned to each LED mode and function/direction combination, including special colors.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `modeIndex` | `uint8_t` | 1 | [ledModeIndex_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-ledmodeindex_e) | Index of the LED mode Enum (`ledModeIndex_e`). `LED_MODE_COUNT` for special colors |
| `directionOrSpecialIndex` | `uint8_t` | 1 | - | Index of the direction (`ledDirectionId_e`) or special color (`ledSpecialColorIds_e`) |
| `colorIndex` | `uint8_t` | 1 | - | Index of the color assigned from `ledStripConfig()->colors` |

**Notes:** Only available if `USE_LED_STRIP` is defined. Entries where `modeIndex == LED_MODE_COUNT` describe special colors.

## <a id="msp_battery_state"></a>`MSP_BATTERY_STATE (130 / 0x82)`
**Description:** Provides battery state information, formatted primarily for DJI FPV Goggles compatibility.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `cellCount` | `uint8_t` | 1 | Count | Number of battery cells (`getBatteryCellCount()`) |
| `capacity` | `uint16_t` | 2 | mAh | Battery capacity (`currentBatteryProfile->capacity.value`) |
| `vbatScaled` | `uint8_t` | 1 | 0.1V | Battery voltage / 10 (`getBatteryVoltage() / 10`) |
| `mAhDrawn` | `uint16_t` | 2 | mAh | Consumed capacity (`getMAhDrawn()`) |
| `amperage` | `int16_t` | 2 | 0.01A | Current draw (`getAmperage()`) |
| `batteryState` | `uint8_t` | 1 | [batteryState_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-batterystate_e) | Enum `batteryState_e` Current battery state (`getBatteryState()`, see `BATTERY_STATE_*`) |
| `vbatActual` | `uint16_t` | 2 | 0.01V | Actual battery voltage (`getBatteryVoltage()`) |

**Notes:** Only available if `USE_DJI_HD_OSD` or `USE_MSP_DISPLAYPORT` is defined. Some values are duplicated from `MSP_ANALOG` / `MSP2_INAV_ANALOG` but potentially with different scaling/types.

## <a id="msp_vtxtable_band"></a>`MSP_VTXTABLE_BAND (137 / 0x89)`
**Description:** Retrieves information about a specific VTX band from the VTX table. (Implementation missing in provided `fc_msp.c`)  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** The ID is defined, but no handler exists in the provided C code. Likely intended to query band names and frequencies.

## <a id="msp_vtxtable_powerlevel"></a>`MSP_VTXTABLE_POWERLEVEL (138 / 0x8a)`
**Description:** Retrieves information about a specific VTX power level from the VTX table.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `powerLevelIndex` | `uint8_t` | 1 | 1-based index of the power level to query |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `powerLevelIndex` | `uint8_t` | 1 | 1-based index of the returned power level |
| `powerValue` | `uint16_t` | 2 | Always 0 (Actual power value in mW is not stored/returned via MSP) |
| `labelLength` | `uint8_t` | 1 | Length of the power level label string that follows |
| `label` | `char[]` | array | Power level label string (e.g., "25", "200"). Length given by previous field |

**Notes:** Requires `USE_VTX_CONTROL`. Returns error if index is out of bounds. The `powerValue` field is unused.

## <a id="msp_status_ex"></a>`MSP_STATUS_EX (150 / 0x96)`
**Description:** Provides extended flight controller status, including CPU load, arming flags, and calibration status, in addition to `MSP_STATUS` fields.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `cycleTime` | `uint16_t` | 2 | µs | Main loop cycle time |
| `i2cErrors` | `uint16_t` | 2 | Count | I2C errors |
| `sensorStatus` | `uint16_t` | 2 | Bitmask | Bitmask: Sensor status |
| `activeModesLow` | `uint32_t` | 4 | Bitmask | Bitmask: First 32 active modes |
| `profile` | `uint8_t` | 1 | Index | Current config profile index |
| `cpuLoad` | `uint16_t` | 2 | % | Average system load percentage (`averageSystemLoadPercent`) |
| `armingFlags` | `uint16_t` | 2 | Bitmask | Bitmask: Flight controller arming flags (`armingFlags`). Note: Truncated to 16 bits |
| `accCalibAxisFlags` | `uint8_t` | 1 | Bitmask | Bitmask: Accelerometer calibrated axes flags (`accGetCalibrationAxisFlags()`) |

**Notes:** Superseded by `MSP2_INAV_STATUS` which provides the full 32-bit `armingFlags` and other enhancements.

## <a id="msp_sensor_status"></a>`MSP_SENSOR_STATUS (151 / 0x97)`
**Description:** Provides the hardware status for each individual sensor system.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `overallHealth` | `uint8_t` | 1 | Boolean | 1 if all essential hardware is healthy, 0 otherwise (`isHardwareHealthy()`) |
| `gyroStatus` | `uint8_t` | 1 | [hardwareSensorStatus_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-hardwaresensorstatus_e) | Enum `hardwareSensorStatus_e` Gyro hardware status (`getHwGyroStatus()`) |
| `accStatus` | `uint8_t` | 1 | [hardwareSensorStatus_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-hardwaresensorstatus_e) | Enum `hardwareSensorStatus_e` Accelerometer hardware status (`getHwAccelerometerStatus()`) |
| `magStatus` | `uint8_t` | 1 | [hardwareSensorStatus_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-hardwaresensorstatus_e) | Enum `hardwareSensorStatus_e` Compass hardware status (`getHwCompassStatus()`) |
| `baroStatus` | `uint8_t` | 1 | [hardwareSensorStatus_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-hardwaresensorstatus_e) | Enum `hardwareSensorStatus_e` Barometer hardware status (`getHwBarometerStatus()`) |
| `gpsStatus` | `uint8_t` | 1 | [hardwareSensorStatus_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-hardwaresensorstatus_e) | Enum `hardwareSensorStatus_e` GPS hardware status (`getHwGPSStatus()`) |
| `rangefinderStatus` | `uint8_t` | 1 | [hardwareSensorStatus_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-hardwaresensorstatus_e) | Enum `hardwareSensorStatus_e` Rangefinder hardware status (`getHwRangefinderStatus()`) |
| `pitotStatus` | `uint8_t` | 1 | [hardwareSensorStatus_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-hardwaresensorstatus_e) | Enum `hardwareSensorStatus_e` Pitot hardware status (`getHwPitotmeterStatus()`) |
| `opflowStatus` | `uint8_t` | 1 | [hardwareSensorStatus_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-hardwaresensorstatus_e) | Enum `hardwareSensorStatus_e` Optical Flow hardware status (`getHwOpticalFlowStatus()`) |

**Notes:** Status values map to the `hardwareSensorStatus_e` enum: `HW_SENSOR_NONE`, `HW_SENSOR_OK`, `HW_SENSOR_UNAVAILABLE`, `HW_SENSOR_UNHEALTHY`.

## <a id="msp_uid"></a>`MSP_UID (160 / 0xa0)`
**Description:** Provides the unique identifier of the microcontroller.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `uid0` | `uint32_t` | 4 | First 32 bits of the unique ID (`U_ID_0`) |
| `uid1` | `uint32_t` | 4 | Middle 32 bits of the unique ID (`U_ID_1`) |
| `uid2` | `uint32_t` | 4 | Last 32 bits of the unique ID (`U_ID_2`) |

**Notes:** Total 12 bytes, representing a 96-bit unique ID.

## <a id="msp_gpssvinfo"></a>`MSP_GPSSVINFO (164 / 0xa4)`
**Description:** Provides satellite signal strength information (legacy U-Blox compatibility stub).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `protocolVersion` | `uint8_t` | 1 | Always 1 (Stub version) |
| `numChannels` | `uint8_t` | 1 | Always 0 (Number of SV info channels reported) |
| `hdopHundredsDigit` | `uint8_t` | 1 | Hundreds digit of HDOP (stub always writes 0) |
| `hdopTensDigit` | `uint8_t` | 1 | Tens digit of HDOP (`gpsSol.hdop / 100`, truncated) |
| `hdopUnitsDigit` | `uint8_t` | 1 | Units digit of HDOP (`gpsSol.hdop / 100`, duplicated by stub) |

**Notes:** Requires `USE_GPS`. This is just a stub in INAV and does not provide actual per-satellite signal info. HDOP digits are not formatted correctly: tens and units both contain `gpsSol.hdop / 100`.

## <a id="msp_gpsstatistics"></a>`MSP_GPSSTATISTICS (166 / 0xa6)`
**Description:** Provides debugging statistics for the GPS communication link.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `lastMessageDt` | `uint16_t` | 2 | ms | Time since last valid GPS message (`gpsStats.lastMessageDt`) |
| `errors` | `uint32_t` | 4 | Count | Number of GPS communication errors (`gpsStats.errors`) |
| `timeouts` | `uint32_t` | 4 | Count | Number of GPS communication timeouts (`gpsStats.timeouts`) |
| `packetCount` | `uint32_t` | 4 | Count | Number of valid GPS packets received (`gpsStats.packetCount`) |
| `hdop` | `uint16_t` | 2 | HDOP * 100 | Horizontal Dilution of Precision (`gpsSol.hdop`) |
| `eph` | `uint16_t` | 2 | cm | Estimated Horizontal Position Accuracy (`gpsSol.eph`) |
| `epv` | `uint16_t` | 2 | cm | Estimated Vertical Position Accuracy (`gpsSol.epv`) |

**Notes:** Requires `USE_GPS`.

## <a id="msp_osd_video_config"></a>`MSP_OSD_VIDEO_CONFIG (180 / 0xb4)`

**Request Payload:** **None**  

**Reply Payload:** **None**  

## <a id="msp_set_osd_video_config"></a>`MSP_SET_OSD_VIDEO_CONFIG (181 / 0xb5)`

**Request Payload:** **None**  

**Reply Payload:** **None**  

## <a id="msp_displayport"></a>`MSP_DISPLAYPORT (182 / 0xb6)`

**Request Payload:** **None**  

**Reply Payload:** **None**  

## <a id="msp_set_tx_info"></a>`MSP_SET_TX_INFO (186 / 0xba)`
**Description:** Allows a transmitter LUA script (or similar) to send runtime information (currently only RSSI) to the firmware.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `rssi` | `uint8_t` | 1 | Raw | RSSI value (0-255) provided by the external source; firmware scales it to 10-bit (`value << 2`) |

**Reply Payload:** **None**  

**Notes:** Calls `setRSSIFromMSP()`. Expects 1 byte.

## <a id="msp_tx_info"></a>`MSP_TX_INFO (187 / 0xbb)`
**Description:** Provides information potentially useful for transmitter LUA scripts.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `rssiSource` | `uint8_t` | 1 | [rssiSource_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-rssisource_e) | Enum: Source of the RSSI value (`getRSSISource()`, see `rssiSource_e`) |
| `rtcDateTimeIsSet` | `uint8_t` | 1 | - | Boolean: 1 if the RTC has been set, 0 otherwise |

**Notes:** See `rssiSource_e`.

## <a id="msp_set_raw_rc"></a>`MSP_SET_RAW_RC (200 / 0xc8)`
**Description:** Provides raw RC channel data to the flight controller, typically used when the receiver is connected via MSP (e.g., MSP RX feature).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `rcChannels` | `uint16_t[]` | array | PWM | Array of RC channel values (typically 1000-2000). Number of channels determined by payload size |

**Reply Payload:** **None**  

**Notes:** Requires `USE_RX_MSP`. Maximum channels `MAX_SUPPORTED_RC_CHANNEL_COUNT`. Calls `rxMspFrameReceive()`.

## <a id="msp_set_raw_gps"></a>`MSP_SET_RAW_GPS (201 / 0xc9)`
**Description:** Provides raw GPS data to the flight controller, typically for simulation or external GPS injection.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `fixType` | `uint8_t` | 1 | [gpsFixType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-gpsfixtype_e) | Enum `gpsFixType_e` GPS fix type |
| `numSat` | `uint8_t` | 1 | Count | Number of satellites |
| `latitude` | `int32_t` | 4 | deg * 1e7 | Latitude |
| `longitude` | `int32_t` | 4 | deg * 1e7 | Longitude |
| `altitude` | `uint16_t` | 2 | m | Altitude in meters (converted to centimeters internally; limited to 0-65535 m) |
| `speed` | `uint16_t` | 2 | cm/s | Ground speed (`gpsSol.groundSpeed`) |

**Reply Payload:** **None**  

**Notes:** Requires `USE_GPS`. Expects 14 bytes. Updates `gpsSol` structure and calls `onNewGPSData()`. Note the altitude unit mismatch (meters in MSP, cm internal). Does not provide velocity components.

## <a id="msp_set_box"></a>`MSP_SET_BOX (203 / 0xcb)`
**Description:** Sets the state of flight modes (boxes). (Likely unused/obsolete in INAV).  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Not implemented in INAV `fc_msp.c`. Mode changes are typically handled via RC channels (`MSP_MODE_RANGES`).

## <a id="msp_set_rc_tuning"></a>`MSP_SET_RC_TUNING (204 / 0xcc)`
**Description:** Sets RC tuning parameters (rates, expos, TPA) for the current control rate profile.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `legacyRcRate` | `uint8_t` | 1 | Ignored |
| `rcExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile->stabilized.rcExpo8` |
| `rollRate` | `uint8_t` | 1 | Sets `currentControlRateProfile->stabilized.rates[FD_ROLL]` (constrained) |
| `pitchRate` | `uint8_t` | 1 | Sets `currentControlRateProfile->stabilized.rates[FD_PITCH]` (constrained) |
| `yawRate` | `uint8_t` | 1 | Sets `currentControlRateProfile->stabilized.rates[FD_YAW]` (constrained) |
| `dynamicThrottlePID` | `uint8_t` | 1 | Sets `currentControlRateProfile->throttle.dynPID` (constrained) |
| `throttleMid` | `uint8_t` | 1 | Sets `currentControlRateProfile->throttle.rcMid8` |
| `throttleExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile->throttle.rcExpo8` |
| `tpaBreakpoint` | `uint16_t` | 2 | Sets `currentControlRateProfile->throttle.pa_breakpoint` |
| `rcYawExpo` | `uint8_t` | 1 | (Optional) Sets `currentControlRateProfile->stabilized.rcYawExpo8` |

**Reply Payload:** **None**  

**Notes:** Expects 10 or 11 bytes. Calls `schedulePidGainsUpdate()`. Superseded by `MSP2_INAV_SET_RATE_PROFILE`.

## <a id="msp_acc_calibration"></a>`MSP_ACC_CALIBRATION (205 / 0xcd)`
**Description:** Starts the accelerometer calibration procedure.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Will fail if armed. Calls `accStartCalibration()`.

## <a id="msp_mag_calibration"></a>`MSP_MAG_CALIBRATION (206 / 0xce)`
**Description:** Starts the magnetometer calibration procedure.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Will fail if armed. Enables the `CALIBRATE_MAG` state flag.

## <a id="msp_set_misc"></a>`MSP_SET_MISC (207 / 0xcf)`
**Description:** Sets miscellaneous configuration settings (legacy formats/scaling).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `midRc` | `uint16_t` | 2 | PWM | Ignored |
| `legacyMinThrottle` | `uint16_t` | 2 | - | Ignored |
| `legacyMaxThrottle` | `uint16_t` | 2 | - | Ignored |
| `minCommand` | `uint16_t` | 2 | PWM | Sets `motorConfigMutable()->mincommand` (constrained 0-PWM_RANGE_MAX) |
| `failsafeThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->failsafe_throttle` (constrained PWM_RANGE_MIN/MAX) |
| `gpsType` | `uint8_t` | 1 | [gpsProvider_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-gpsprovider_e) | Enum `gpsProvider_e` (Sets `gpsConfigMutable()->provider`) |
| `legacyGpsBaud` | `uint8_t` | 1 | - | Ignored |
| `gpsSbasMode` | `uint8_t` | 1 | [sbasMode_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-sbasmode_e) | Enum `sbasMode_e` (Sets `gpsConfigMutable()->sbasMode`) |
| `legacyMwCurrentOut` | `uint8_t` | 1 | - | Ignored |
| `rssiChannel` | `uint8_t` | 1 | Index | Sets `rxConfigMutable()->rssi_channel` (constrained 0-MAX_SUPPORTED_RC_CHANNEL_COUNT). Updates source |
| `reserved1` | `uint8_t` | 1 | - | Ignored |
| `magDeclination` | `uint16_t` | 2 | 0.1 degrees | Sets `compassConfigMutable()->mag_declination = value * 10` (if `USE_MAG`) |
| `vbatScale` | `uint8_t` | 1 | Scale / 10 | Sets `batteryMetersConfigMutable()->voltage.scale = value * 10` (if `USE_ADC`) |
| `vbatMinCell` | `uint8_t` | 1 | 0.1V | Sets `currentBatteryProfileMutable->voltage.cellMin = value * 10` (if `USE_ADC`) |
| `vbatMaxCell` | `uint8_t` | 1 | 0.1V | Sets `currentBatteryProfileMutable->voltage.cellMax = value * 10` (if `USE_ADC`) |
| `vbatWarningCell` | `uint8_t` | 1 | 0.1V | Sets `currentBatteryProfileMutable->voltage.cellWarning = value * 10` (if `USE_ADC`) |

**Reply Payload:** **None**  

**Notes:** Expects 22 bytes. Superseded by `MSP2_INAV_SET_MISC`.

## <a id="msp_reset_conf"></a>`MSP_RESET_CONF (208 / 0xd0)`
**Description:** Resets all configuration settings to their default values and saves to EEPROM.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Will fail if armed. Suspends RX, calls `resetEEPROM()`, `writeEEPROM()`, `readEEPROM()`, resumes RX. Use with caution!

## <a id="msp_set_wp"></a>`MSP_SET_WP (209 / 0xd1)`
**Description:** Sets a single waypoint in the mission plan.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `waypointIndex` | `uint8_t` | 1 | Index | Index of the waypoint to set (0 to `NAV_MAX_WAYPOINTS - 1`) |
| `action` | `uint8_t` | 1 | [navWaypointActions_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-navwaypointactions_e) | Enum `navWaypointActions_e` Waypoint action type |
| `latitude` | `int32_t` | 4 | deg * 1e7 | Latitude coordinate |
| `longitude` | `int32_t` | 4 | deg * 1e7 | Longitude coordinate |
| `altitude` | `int32_t` | 4 | cm | Altitude coordinate |
| `param1` | `uint16_t` | 2 | Varies | Parameter 1 |
| `param2` | `uint16_t` | 2 | Varies | Parameter 2 |
| `param3` | `uint16_t` | 2 | Varies | Parameter 3 |
| `flag` | `uint8_t` | 1 | [navWaypointFlags_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-navwaypointflags_e) | Bitmask: Waypoint flags (`navWaypointFlags_e`) |

**Reply Payload:** **None**  

**Notes:** Expects 21 bytes. Calls `setWaypoint()`. If `USE_FW_AUTOLAND` is enabled, this also interacts with autoland approach settings based on waypoint index and flags.

## <a id="msp_select_setting"></a>`MSP_SELECT_SETTING (210 / 0xd2)`
**Description:** Selects the active configuration profile and saves it.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `profileIndex` | `uint8_t` | 1 | Index of the profile to activate (0-based) |

**Reply Payload:** **None**  

**Notes:** Will fail if armed. Calls `setConfigProfileAndWriteEEPROM()`.

## <a id="msp_set_head"></a>`MSP_SET_HEAD (211 / 0xd3)`
**Description:** Sets the target heading for the heading hold controller (e.g., during MAG mode).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `heading` | `uint16_t` | 2 | degrees | Target heading (0-359) |

**Reply Payload:** **None**  

**Notes:** Expects 2 bytes. Calls `updateHeadingHoldTarget()`.

## <a id="msp_set_servo_configuration"></a>`MSP_SET_SERVO_CONFIGURATION (212 / 0xd4)`
**Description:** Sets the configuration for a single servo (legacy format).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `servoIndex` | `uint8_t` | 1 | Index | Index of the servo to configure (0 to `MAX_SUPPORTED_SERVOS - 1`) |
| `min` | `uint16_t` | 2 | PWM | Minimum servo endpoint |
| `max` | `uint16_t` | 2 | PWM | Maximum servo endpoint |
| `middle` | `uint16_t` | 2 | PWM | Middle/Neutral servo position |
| `rate` | `uint8_t` | 1 | % | Servo rate/scaling |
| `reserved1` | `uint8_t` | 1 | - | Ignored |
| `reserved2` | `uint8_t` | 1 | - | Ignored |
| `legacyForwardChan` | `uint8_t` | 1 | - | Ignored |
| `legacyReversedSources` | `uint32_t` | 4 | - | Ignored |

**Reply Payload:** **None**  

**Notes:** Expects 15 bytes. Returns error if index is invalid. Calls `servoComputeScalingFactors()`. Superseded by `MSP2_INAV_SET_SERVO_CONFIG`.

## <a id="msp_set_motor"></a>`MSP_SET_MOTOR (214 / 0xd6)`
**Description:** Sets the disarmed motor values, typically used for motor testing or propeller balancing functions in a configurator.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `motorValues` | `uint16_t[8]` | 16 | PWM | Array of motor values to set when disarmed. Only affects first `MAX_SUPPORTED_MOTORS` entries |

**Reply Payload:** **None**  

**Notes:** Expects 16 bytes. Modifies the `motor_disarmed` array. These values are *not* saved persistently.

## <a id="msp_set_nav_config"></a>`MSP_SET_NAV_CONFIG (215 / 0xd7)`

**Request Payload:** **None**  

**Reply Payload:** **None**  

## <a id="msp_set_3d"></a>`MSP_SET_3D (217 / 0xd9)`
**Description:** Sets parameters related to 3D/reversible motor operation.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `deadbandLow` | `uint16_t` | 2 | PWM | Sets `reversibleMotorsConfigMutable()->deadband_low` |
| `deadbandHigh` | `uint16_t` | 2 | PWM | Sets `reversibleMotorsConfigMutable()->deadband_high` |
| `neutral` | `uint16_t` | 2 | PWM | Sets `reversibleMotorsConfigMutable()->neutral` |

**Reply Payload:** **None**  

**Notes:** Expects 6 bytes. Requires reversible motor support.

## <a id="msp_set_rc_deadband"></a>`MSP_SET_RC_DEADBAND (218 / 0xda)`
**Description:** Sets RC input deadband values.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `deadband` | `uint8_t` | 1 | PWM | Sets `rcControlsConfigMutable()->deadband` |
| `yawDeadband` | `uint8_t` | 1 | PWM | Sets `rcControlsConfigMutable()->yaw_deadband` |
| `altHoldDeadband` | `uint8_t` | 1 | PWM | Sets `rcControlsConfigMutable()->alt_hold_deadband` |
| `throttleDeadband` | `uint16_t` | 2 | PWM | Sets `rcControlsConfigMutable()->mid_throttle_deadband` |

**Reply Payload:** **None**  

**Notes:** Expects 5 bytes.

## <a id="msp_set_reset_curr_pid"></a>`MSP_SET_RESET_CURR_PID (219 / 0xdb)`
**Description:** Resets the PIDs of the *current* profile to their default values. Does not save.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Calls `PG_RESET_CURRENT(pidProfile)`. To save, follow with `MSP_EEPROM_WRITE`.

## <a id="msp_set_sensor_alignment"></a>`MSP_SET_SENSOR_ALIGNMENT (220 / 0xdc)`
**Description:** Sets sensor alignment (legacy format).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `gyroAlign` | `uint8_t` | 1 | Ignored |
| `accAlign` | `uint8_t` | 1 | Ignored |
| `magAlign` | `uint8_t` | 1 | Sets `compassConfigMutable()->mag_align` (if `USE_MAG`) |
| `opflowAlign` | `uint8_t` | 1 | Sets `opticalFlowConfigMutable()->opflow_align` (if `USE_OPFLOW`) |

**Reply Payload:** **None**  

**Notes:** Expects 4 bytes. Use `MSP_SET_BOARD_ALIGNMENT` for primary board orientation.

## <a id="msp_set_led_strip_modecolor"></a>`MSP_SET_LED_STRIP_MODECOLOR (221 / 0xdd)`
**Description:** Sets the color index for a specific LED mode/function combination.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `modeIndex` | `uint8_t` | 1 | Index of the LED mode (`ledModeIndex_e` or `LED_MODE_COUNT` for special) |
| `directionOrSpecialIndex` | `uint8_t` | 1 | Index of the direction (`ledDirectionId_e`) or special color (`ledSpecialColorIds_e`) |
| `colorIndex` | `uint8_t` | 1 | Index of the color to assign from `ledStripConfig()->colors` |

**Reply Payload:** **None**  

**Notes:** Only available if `USE_LED_STRIP` is defined. Expects 3 bytes. Returns error if setting fails (invalid index).

## <a id="msp_set_acc_trim"></a>`MSP_SET_ACC_TRIM (239 / 0xef)`
**Description:** Sets the accelerometer trim values (leveling calibration).  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Not implemented in INAV `fc_msp.c`. Use `MSP_ACC_CALIBRATION`.

## <a id="msp_acc_trim"></a>`MSP_ACC_TRIM (240 / 0xf0)`
**Description:** Gets the accelerometer trim values.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Not implemented in INAV `fc_msp.c`. Calibration data via `MSP_CALIBRATION_DATA`.

## <a id="msp_servo_mix_rules"></a>`MSP_SERVO_MIX_RULES (241 / 0xf1)`
**Description:** Retrieves the custom servo mixer rules (legacy format).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `targetChannel` | `uint8_t` | 1 | Index | Servo output channel index (0-based) |
| `inputSource` | `uint8_t` | 1 | [inputSource_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-inputsource_e) | Enum `inputSource_e` Input source for the mix (RC chan, Roll, Pitch...) |
| `rate` | `int16_t` | 2 | % | Mixing rate/weight (`-1000` to `+1000`, percent with sign) |
| `speed` | `uint8_t` | 1 | 0-255 | Speed/Slew rate limit (`0`=instant, higher slows response) |
| `reserved1` | `uint8_t` | 1 | - | Always 0 |
| `legacyMax` | `uint8_t` | 1 | - | Always 100 (Legacy) |
| `legacyBox` | `uint8_t` | 1 | - | Always 0 (Legacy) |

**Notes:** Superseded by `MSP2_INAV_SERVO_MIXER`.

## <a id="msp_set_servo_mix_rule"></a>`MSP_SET_SERVO_MIX_RULE (242 / 0xf2)`
**Description:** Sets a single custom servo mixer rule (legacy format).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `ruleIndex` | `uint8_t` | 1 | Index | Index of the rule to set (0 to `MAX_SERVO_RULES - 1`) |
| `targetChannel` | `uint8_t` | 1 | Index | Servo output channel index |
| `inputSource` | `uint8_t` | 1 | [inputSource_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-inputsource_e) | Enum `inputSource_e` Input source for the mix |
| `rate` | `int16_t` | 2 | % | Mixing rate/weight (`-1000` to `+1000`, percent with sign) |
| `speed` | `uint8_t` | 1 | 0-255 | Speed/Slew rate limit (`0`=instant, higher slows response) |
| `legacyMinMax` | `uint16_t` | 2 | - | Ignored |
| `legacyBox` | `uint8_t` | 1 | - | Ignored |

**Reply Payload:** **None**  

**Notes:** Expects 9 bytes. Returns error if index invalid. Calls `loadCustomServoMixer()`. Superseded by `MSP2_INAV_SET_SERVO_MIXER`.

## <a id="msp_set_passthrough"></a>`MSP_SET_PASSTHROUGH (245 / 0xf5)`
**Description:** Enables serial passthrough mode to peripherals like ESCs (BLHeli 4-way) or other serial devices.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `status` | `uint8_t` | 1 | 1 if passthrough started successfully, 0 on error (e.g., port not found). For 4way, returns number of ESCs found |

**Notes:** Accepts 0 bytes (defaults to ESC 4-way) or up to 2 bytes for mode/argument. If successful, sets `mspPostProcessFn` to the appropriate handler (`mspSerialPassthroughFn` or `esc4wayProcess`). This handler takes over the serial port after the reply is sent. Requires `USE_SERIAL_4WAY_BLHELI_INTERFACE` for ESC passthrough.

## <a id="msp_rtc"></a>`MSP_RTC (246 / 0xf6)`
**Description:** Retrieves the current Real-Time Clock time.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `seconds` | `int32_t` | 4 | Seconds | Seconds since epoch (or relative time if not set). 0 if RTC time unknown |
| `millis` | `uint16_t` | 2 | Milliseconds | Millisecond part of the time. 0 if RTC time unknown |

**Notes:** Requires RTC hardware/support. Returns (0, 0) if time is not available/set.

## <a id="msp_set_rtc"></a>`MSP_SET_RTC (247 / 0xf7)`
**Description:** Sets the Real-Time Clock time.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `seconds` | `int32_t` | 4 | Seconds | Seconds component of time to set |
| `millis` | `uint16_t` | 2 | Milliseconds | Millisecond component of time to set |

**Reply Payload:** **None**  

**Notes:** Requires RTC hardware/support. Expects 6 bytes. Uses `rtcSet()`.

## <a id="msp_eeprom_write"></a>`MSP_EEPROM_WRITE (250 / 0xfa)`
**Description:** Saves the current configuration from RAM to non-volatile memory (EEPROM/Flash).  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Will fail if armed. Suspends RX, calls `writeEEPROM()`, `readEEPROM()`, resumes RX.

## <a id="msp_reserve_1"></a>`MSP_RESERVE_1 (251 / 0xfb)`

**Request Payload:** **None**  

**Reply Payload:** **None**  

## <a id="msp_reserve_2"></a>`MSP_RESERVE_2 (252 / 0xfc)`

**Request Payload:** **None**  

**Reply Payload:** **None**  

## <a id="msp_debugmsg"></a>`MSP_DEBUGMSG (253 / 0xfd)`
**Description:** Retrieves debug ("serial printf") messages from the firmware.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `Message Text` | `char[]` | array | Debug message text (not NUL-terminated). See [serial printf debugging](https://github.com/iNavFlight/inav/blob/master/docs/development/serial_printf_debugging.md) |

**Notes:** Published via the LOG UART or shared MSP/LOG port using `mspSerialPushPort()`.

## <a id="msp_debug"></a>`MSP_DEBUG (254 / 0xfe)`
**Description:** Retrieves values from the firmware's `debug[]` array (legacy 16-bit version).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `debugValues` | `uint16_t[4]` | 8 | First 4 values from the `debug` array |

**Notes:** Useful for developers. Values are truncated to the lower 16 bits of each `debug[]` entry. See `MSP2_INAV_DEBUG` for full 32-bit values.

## <a id="msp_v2_frame"></a>`MSP_V2_FRAME (255 / 0xff)`
**Description:** This ID is used as a *payload indicator* within an MSPv1 message structure (`$M>`) to signify that the following payload conforms to the MSPv2 format. It's not a command itself.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** See MSPv2 documentation for the actual frame structure that follows this indicator.

## <a id="msp2_common_tz"></a>`MSP2_COMMON_TZ (4097 / 0x1001)`
**Description:** Gets the time zone offset configuration.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `tzOffsetMinutes` | `int16_t` | 2 | Minutes | Time zone offset from UTC (`timeConfig()->tz_offset`) |
| `tzAutoDst` | `uint8_t` | 1 | Boolean | Automatic daylight saving time enabled (`timeConfig()->tz_automatic_dst`) |

## <a id="msp2_common_set_tz"></a>`MSP2_COMMON_SET_TZ (4098 / 0x1002)`
**Description:** Sets the time zone offset configuration.  
#### Variant: `dataSize == 2`

**Description:** dataSize == 2  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `tz_offset` | `int16_t` | 2 | minutes | Timezone offset from UTC. |

**Reply Payload:** **None**  

#### Variant: `dataSize == 3`

**Description:** dataSize == 3  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `tz_offset` | `int16_t` | 2 | minutes | Timezone offset from UTC. |
| `tz_automatic_dst` | `uint8_t` | 1 | bool | Automatic DST enable (0/1). |

**Reply Payload:** **None**  


**Notes:** Accepts 2 or 3 bytes.

## <a id="msp2_common_setting"></a>`MSP2_COMMON_SETTING (4099 / 0x1003)`
**Description:** Gets the value of a specific configuration setting, identified by name or index.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `settingIdentifier` | `Varies` | - | Setting name (null-terminated string) OR index selector (`0x00` followed by `uint16_t` index) |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `settingValue` | `uint8_t[]` | array | Raw byte value of the setting. Size depends on the setting's type (`settingGetValueSize()`) |

**Notes:** Returns error if setting not found. Use `MSP2_COMMON_SETTING_INFO` to discover settings, types, and sizes.

## <a id="msp2_common_set_setting"></a>`MSP2_COMMON_SET_SETTING (4100 / 0x1004)`
**Description:** Sets the value of a specific configuration setting, identified by name or index.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `settingIdentifier` | `Varies` | - | Setting name (null-terminated string) OR Index (0x00 followed by `uint16_t` index) |
| `settingValue` | `uint8_t[]` | array | Raw byte value to set for the setting. Size must match the setting's type |

**Reply Payload:** **None**  

**Notes:** Performs type checking and range validation (min/max). Returns error if setting not found, value size mismatch, or value out of range. Handles different data types (`uint8`, `int16`, `float`, `string`, etc.) internally.

## <a id="msp2_common_motor_mixer"></a>`MSP2_COMMON_MOTOR_MIXER (4101 / 0x1005)`
**Description:** Retrieves the current motor mixer configuration (throttle, roll, pitch, yaw weights) for each motor.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `motorMix` | `uint16_t[4]` | 8 | Scaled (0-4000) | Weights for a single motor `[throttle, roll, pitch, yaw]`, each encoded as `(mix + 2.0) * 1000` (range 0-4000) |

**Notes:** Scaling is `(float_weight + 2.0) * 1000`. `primaryMotorMixer()` provides the data. If multiple mixer profiles are enabled (`MAX_MIXER_PROFILE_COUNT > 1`), an additional block of mixes for the next profile follows immediately.

## <a id="msp2_common_set_motor_mixer"></a>`MSP2_COMMON_SET_MOTOR_MIXER (4102 / 0x1006)`
**Description:** Sets the motor mixer weights for a single motor in the primary mixer profile.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `motorIndex` | `uint8_t` | 1 | Index | Index of the motor to configure (0 to `MAX_SUPPORTED_MOTORS - 1`) |
| `throttleWeight` | `uint16_t` | 2 | Scaled (0-4000) | Sets throttle weight from `(value / 1000.0) - 2.0 |
| `rollWeight` | `uint16_t` | 2 | Scaled (0-4000) | Sets roll weight from `(value / 1000.0) - 2.0 |
| `pitchWeight` | `uint16_t` | 2 | Scaled (0-4000) | Sets pitch weight from `(value / 1000.0) - 2.0 |
| `yawWeight` | `uint16_t` | 2 | Scaled (0-4000) | Sets yaw weight from `(value / 1000.0) - 2.0 |

**Reply Payload:** **None**  

**Notes:** Expects 9 bytes. Modifies `primaryMotorMixerMutable()`. Returns error if index is invalid.

## <a id="msp2_common_setting_info"></a>`MSP2_COMMON_SETTING_INFO (4103 / 0x1007)`
**Description:** Gets detailed information about a specific configuration setting (name, type, range, flags, current value, etc.).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `settingName` | `char[]` | array | Null-terminated setting name |
| `pgn` | `uint16_t` | 2 | Parameter Group Number (PGN) ID |
| `type` | `uint8_t` | 1 | Variable type (`VAR_UINT8`, `VAR_FLOAT`, etc.) |
| `section` | `uint8_t` | 1 | Setting section (`MASTER_VALUE`, `PROFILE_VALUE`, etc.) |
| `mode` | `uint8_t` | 1 | Setting mode (`MODE_NORMAL`, `MODE_LOOKUP`, etc.) |
| `minValue` | `int32_t` | 4 | Minimum allowed value (as signed 32-bit) |
| `maxValue` | `uint32_t` | 4 | Maximum allowed value (as unsigned 32-bit) |
| `settingIndex` | `uint16_t` | 2 | Absolute index of the setting |
| `profileIndex` | `uint8_t` | 1 | Current profile index (if applicable, else 0) |
| `profileCount` | `uint8_t` | 1 | Total number of profiles (if applicable, else 0) |
| `lookupNames` | `char[]` | array | (If `mode == MODE_LOOKUP`) Series of null-terminated strings for each possible value from min to max |
| `settingValue` | `uint8_t[]` | array | Current raw byte value of the setting |

## <a id="msp2_common_pg_list"></a>`MSP2_COMMON_PG_LIST (4104 / 0x1008)`
**Description:** Gets a list of Parameter Group Numbers (PGNs) used by settings, along with the start and end setting indexes for each group. Can request info for a single PGN.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `pgn` | `uint16_t` | 2 | (Optional) PGN ID to query. If omitted, returns all used PGNs |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `pgn` | `uint16_t` | 2 | Parameter Group Number (PGN) ID |
| `startIndex` | `uint16_t` | 2 | Absolute index of the first setting in this group |
| `endIndex` | `uint16_t` | 2 | Absolute index of the last setting in this group |

**Notes:** Allows efficient fetching of related settings by group.

## <a id="msp2_common_serial_config"></a>`MSP2_COMMON_SERIAL_CONFIG (4105 / 0x1009)`
**Description:** Retrieves the configuration for all available serial ports.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `identifier` | `uint8_t` | 1 | [serialPortIdentifier_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-serialportidentifier_e) | Port identifier Enum (`serialPortIdentifier_e`) |
| `functionMask` | `uint32_t` | 4 | Bitmask | Bitmask: enabled functions (`FUNCTION_*`) |
| `mspBaudIndex` | `uint8_t` | 1 | - | Baud rate index for MSP function |
| `gpsBaudIndex` | `uint8_t` | 1 | - | Baud rate index for GPS function |
| `telemetryBaudIndex` | `uint8_t` | 1 | - | Baud rate index for Telemetry function |
| `peripheralBaudIndex` | `uint8_t` | 1 | - | Baud rate index for other peripheral functions |

**Notes:** Baud rate indexes map to actual baud rates (e.g., 9600, 115200). See `baudRates` array.

## <a id="msp2_common_set_serial_config"></a>`MSP2_COMMON_SET_SERIAL_CONFIG (4106 / 0x100a)`
**Description:** Sets the configuration for one or more serial ports.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `identifier` | `uint8_t` | 1 | [serialPortIdentifier_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-serialportidentifier_e) | Port identifier Enum (`serialPortIdentifier_e`) |
| `functionMask` | `uint32_t` | 4 | Bitmask | Bitmask: functions to enable |
| `mspBaudIndex` | `uint8_t` | 1 | - | Baud rate index for MSP |
| `gpsBaudIndex` | `uint8_t` | 1 | - | Baud rate index for GPS |
| `telemetryBaudIndex` | `uint8_t` | 1 | - | Baud rate index for Telemetry |
| `peripheralBaudIndex` | `uint8_t` | 1 | - | Baud rate index for peripherals |

**Reply Payload:** **None**  

**Notes:** Payload size must be a multiple of the size of one port config entry (1 + 4 + 4 = 9 bytes). Returns error if identifier is invalid or size is incorrect. Baud rate indexes are constrained `BAUD_MIN` to `BAUD_MAX`.

## <a id="msp2_common_set_radar_pos"></a>`MSP2_COMMON_SET_RADAR_POS (4107 / 0x100b)`
**Description:** Sets the position and status information for a "radar" Point of Interest (POI). Used for displaying other craft/objects on the OSD map.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `poiIndex` | `uint8_t` | 1 | Index | Index of the POI slot (0 to `RADAR_MAX_POIS - 1`) |
| `state` | `uint8_t` | 1 | - | Status of the POI (0=undefined, 1=armed, 2=lost) |
| `latitude` | `int32_t` | 4 | deg * 1e7 | Latitude of the POI |
| `longitude` | `int32_t` | 4 | deg * 1e7 | Longitude of the POI |
| `altitude` | `int32_t` | 4 | cm | Altitude of the POI |
| `heading` | `uint16_t` | 2 | degrees | Heading of the POI |
| `speed` | `uint16_t` | 2 | cm/s | Speed of the POI |
| `linkQuality` | `uint8_t` | 1 | 0-4 | Link quality indicator |

**Reply Payload:** **None**  

**Notes:** Expects 19 bytes. POI index is clamped to `RADAR_MAX_POIS - 1`. Updates the `radar_pois` array.

## <a id="msp2_common_set_radar_itd"></a>`MSP2_COMMON_SET_RADAR_ITD (4108 / 0x100c)`
**Description:** Sets radar information to display (likely internal/unused).  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Not implemented in INAV `fc_msp.c`.

## <a id="msp2_common_set_msp_rc_link_stats"></a>`MSP2_COMMON_SET_MSP_RC_LINK_STATS (4109 / 0x100d)`
**Description:** Provides RC link statistics (RSSI, LQ) to the FC, typically from an MSP-based RC link (like ExpressLRS). Sent periodically by the RC link.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `sublinkID` | `uint8_t` | 1 | - | Sublink identifier (usually 0) |
| `validLink` | `uint8_t` | 1 | Boolean | Indicates if the link is currently valid (not in failsafe) |
| `rssiPercent` | `uint8_t` | 1 | % | Uplink RSSI percentage (0-100) |
| `uplinkRSSI_dBm` | `uint8_t` | 1 | -dBm | Uplink RSSI in dBm (sent as positive, e.g., 70 means -70dBm) |
| `downlinkLQ` | `uint8_t` | 1 | % | Downlink Link Quality (0-100) |
| `uplinkLQ` | `uint8_t` | 1 | % | Uplink Link Quality (0-100) |
| `uplinkSNR` | `int8_t` | 1 | dB | Uplink Signal-to-Noise Ratio |

**Reply Payload:** **None**  

**Notes:** Requires `USE_RX_MSP`. Expects at least 7 bytes. Updates `rxLinkStatistics` and sets RSSI via `setRSSIFromMSP_RC()` only if `sublinkID` is 0. This message expects **no reply** (`MSP_RESULT_NO_REPLY`).

## <a id="msp2_common_set_msp_rc_info"></a>`MSP2_COMMON_SET_MSP_RC_INFO (4110 / 0x100e)`
**Description:** Provides additional RC link information (power levels, band, mode) to the FC from an MSP-based RC link. Sent less frequently than link stats.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `sublinkID` | `uint8_t` | 1 | - | Sublink identifier (usually 0) |
| `uplinkTxPower` | `uint16_t` | 2 | mW | Uplink transmitter power level |
| `downlinkTxPower` | `uint16_t` | 2 | mW | Downlink transmitter power level |
| `band` | `char[4]` | 4 | - | Operating band string (e.g., "2G4", "900"), null-terminated/padded |
| `mode` | `char[6]` | 6 | - | Operating mode/rate string (e.g., "100HZ", "F1000"), null-terminated/padded |

**Reply Payload:** **None**  

**Notes:** Requires `USE_RX_MSP`. Expects at least 15 bytes. Updates `rxLinkStatistics` only if `sublinkID` is 0. Converts band/mode strings to uppercase. This message expects **no reply** (`MSP_RESULT_NO_REPLY`).

## <a id="msp2_common_get_radar_gps"></a>`MSP2_COMMON_GET_RADAR_GPS (4111 / 0x100f)`
**Description:** Provides the GPS positions (latitude, longitude, altitude) for each radar point of interest.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `poiLatitude` | `int32_t` | 4 | deg * 1e7 | Latitude of a radar POI |
| `poiLongitude` | `int32_t` | 4 | deg * 1e7 | Longitude of a radar POI |
| `poiAltitude` | `int32_t` | 4 | cm | Altitude of a radar POI |

**Notes:** Returns the stored GPS coordinates for all radar POIs (`radar_pois[i].gps`).

## <a id="msp2_sensor_rangefinder"></a>`MSP2_SENSOR_RANGEFINDER (7937 / 0x1f01)`
**Description:** Provides rangefinder data (distance, quality) from an external MSP-based sensor.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `quality` | `uint8_t` | 1 | 0-255 | Quality of the measurement |
| `distanceMm` | `int32_t` | 4 | mm | Measured distance. Negative value indicates out of range |

**Reply Payload:** **None**  

**Notes:** Requires `USE_RANGEFINDER_MSP`. Calls `mspRangefinderReceiveNewData()`.

## <a id="msp2_sensor_optic_flow"></a>`MSP2_SENSOR_OPTIC_FLOW (7938 / 0x1f02)`
**Description:** Provides optical flow data (motion, quality) from an external MSP-based sensor.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `quality` | `uint8_t` | 1 | Quality of the measurement (0-255) |
| `motionX` | `int32_t` | 4 | Raw integrated flow value X |
| `motionY` | `int32_t` | 4 | Raw integrated flow value Y |

**Reply Payload:** **None**  

**Notes:** Requires `USE_OPFLOW_MSP`. Calls `mspOpflowReceiveNewData()`.

## <a id="msp2_sensor_gps"></a>`MSP2_SENSOR_GPS (7939 / 0x1f03)`
**Description:** Provides detailed GPS data from an external MSP-based GPS module.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `instance` | `uint8_t` | 1 | - | Sensor instance number (for multi-GPS) |
| `gpsWeek` | `uint16_t` | 2 | - | GPS week number (0xFFFF if unavailable) |
| `msTOW` | `uint32_t` | 4 | ms | Milliseconds Time of Week |
| `fixType` | `uint8_t` | 1 | [gpsFixType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-gpsfixtype_e) | Enum `gpsFixType_e` Type of GPS fix |
| `satellitesInView` | `uint8_t` | 1 | Count | Number of satellites used in solution |
| `hPosAccuracy` | `uint16_t` | 2 | mm | Horizontal position accuracy estimate in milimeters |
| `vPosAccuracy` | `uint16_t` | 2 | mm | Vertical position accuracy estimate in milimeters |
| `hVelAccuracy` | `uint16_t` | 2 | cm/s | Horizontal velocity accuracy estimate |
| `hdop` | `uint16_t` | 2 | HDOP * 100 | Horizontal Dilution of Precision |
| `longitude` | `int32_t` | 4 | deg * 1e7 | Longitude |
| `latitude` | `int32_t` | 4 | deg * 1e7 | Latitude |
| `mslAltitude` | `int32_t` | 4 | cm | Altitude above Mean Sea Level |
| `nedVelNorth` | `int32_t` | 4 | cm/s | North velocity (NED frame) |
| `nedVelEast` | `int32_t` | 4 | cm/s | East velocity (NED frame) |
| `nedVelDown` | `int32_t` | 4 | cm/s | Down velocity (NED frame) |
| `groundCourse` | `uint16_t` | 2 | deg * 100 | Ground course (0-36000) |
| `trueYaw` | `uint16_t` | 2 | deg * 100 | True heading/yaw (0-36000, 65535 if unavailable) |
| `year` | `uint16_t` | 2 | - | Year (e.g., 2023) |
| `month` | `uint8_t` | 1 | - | Month (1-12) |
| `day` | `uint8_t` | 1 | - | Day of month (1-31) |
| `hour` | `uint8_t` | 1 | - | Hour (0-23) |
| `min` | `uint8_t` | 1 | - | Minute (0-59) |
| `sec` | `uint8_t` | 1 | - | Second (0-59) |

**Reply Payload:** **None**  

**Notes:** Requires `USE_GPS_PROTO_MSP`. Calls `mspGPSReceiveNewData()`.

## <a id="msp2_sensor_compass"></a>`MSP2_SENSOR_COMPASS (7940 / 0x1f04)`
**Description:** Provides magnetometer data from an external MSP-based compass module.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `instance` | `uint8_t` | 1 | - | Sensor instance number |
| `timeMs` | `uint32_t` | 4 | ms | Timestamp from the sensor |
| `magX` | `int16_t` | 2 | mGauss | Front component reading |
| `magY` | `int16_t` | 2 | mGauss | Right component reading |
| `magZ` | `int16_t` | 2 | mGauss | Down component reading |

**Reply Payload:** **None**  

**Notes:** Requires `USE_MAG_MSP`. Calls `mspMagReceiveNewData()`.

## <a id="msp2_sensor_barometer"></a>`MSP2_SENSOR_BAROMETER (7941 / 0x1f05)`
**Description:** Provides barometer data from an external MSP-based barometer module.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `instance` | `uint8_t` | 1 | - | Sensor instance number |
| `timeMs` | `uint32_t` | 4 | ms | Timestamp from the sensor |
| `pressurePa` | `float` | 4 | Pa | Absolute pressure |
| `temp` | `int16_t` | 2 | 0.01 deg C | Temperature |

**Reply Payload:** **None**  

**Notes:** Requires `USE_BARO_MSP`. Calls `mspBaroReceiveNewData()`.

## <a id="msp2_sensor_airspeed"></a>`MSP2_SENSOR_AIRSPEED (7942 / 0x1f06)`
**Description:** Provides airspeed data from an external MSP-based pitot sensor module.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `instance` | `uint8_t` | 1 | - | Sensor instance number |
| `timeMs` | `uint32_t` | 4 | ms | Timestamp from the sensor |
| `diffPressurePa` | `float` | 4 | Pa | Differential pressure |
| `temp` | `int16_t` | 2 | 0.01 deg C | Temperature |

**Reply Payload:** **None**  

**Notes:** Requires `USE_PITOT_MSP`. Calls `mspPitotmeterReceiveNewData()`.

## <a id="msp2_sensor_headtracker"></a>`MSP2_SENSOR_HEADTRACKER (7943 / 0x1f07)`
**Description:** Provides head tracker orientation data.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `...` | `Varies` | - | Head tracker angles (e.g., int16 Roll, Pitch, Yaw in deci-degrees) |  |

**Reply Payload:** **None**  

**Notes:** Requires `USE_HEADTRACKER` and `USE_HEADTRACKER_MSP`. Calls `mspHeadTrackerReceiverNewData()`. Payload structure needs verification from `mspHeadTrackerReceiverNewData` implementation.

## <a id="msp2_inav_status"></a>`MSP2_INAV_STATUS (8192 / 0x2000)`
**Description:** Provides comprehensive flight controller status, extending `MSP_STATUS_EX` with full arming flags, battery profile, and mixer profile.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `cycleTime` | `uint16_t` | 2 | µs | Main loop cycle time |
| `i2cErrors` | `uint16_t` | 2 | Count | I2C errors |
| `sensorStatus` | `uint16_t` | 2 | Bitmask | Bitmask: Sensor status |
| `cpuLoad` | `uint16_t` | 2 | % | Average system load percentage |
| `profileAndBattProfile` | `uint8_t` | 1 | Packed | Bits 0-3: Config profile index (`getConfigProfile()`), Bits 4-7: Battery profile index (`getConfigBatteryProfile()`) |
| `armingFlags` | `uint32_t` | 4 | Bitmask | Bitmask: Full 32-bit flight controller arming flags (`armingFlags`) |
| `activeModes` | `boxBitmask_t` | - | Bitmask | Bitmask words for active flight modes (`packBoxModeFlags()`) |
| `mixerProfile` | `uint8_t` | 1 | Index | Current mixer profile index (`getConfigMixerProfile()`) |

**Notes:** `sensorStatus` bits follow `packSensorStatus()` (bit 15 indicates hardware failure). `profileAndBattProfile` packs the current config profile in the low nibble and the battery profile in the high nibble. `activeModes` is emitted as a little-endian array of 32-bit words sized to `CHECKBOX_ITEM_COUNT`.

## <a id="msp2_inav_optical_flow"></a>`MSP2_INAV_OPTICAL_FLOW (8193 / 0x2001)`
**Description:** Provides data from the optical flow sensor.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `quality` | `uint8_t` | 1 | 0-255 | Raw quality indicator from the sensor (`opflow.rawQuality`). 0 if `USE_OPFLOW` disabled |
| `flowRateX` | `int16_t` | 2 | degrees/s | Optical flow rate X (roll axis) (`RADIANS_TO_DEGREES(opflow.flowRate[X])`). 0 if `USE_OPFLOW` disabled |
| `flowRateY` | `int16_t` | 2 | degrees/s | Optical flow rate Y (pitch axis) (`RADIANS_TO_DEGREES(opflow.flowRate[Y])`). 0 if `USE_OPFLOW` disabled |
| `bodyRateX` | `int16_t` | 2 | degrees/s | Compensated body rate X (roll axis) (`RADIANS_TO_DEGREES(opflow.bodyRate[X])`). 0 if `USE_OPFLOW` disabled |
| `bodyRateY` | `int16_t` | 2 | degrees/s | Compensated body rate Y (pitch axis) (`RADIANS_TO_DEGREES(opflow.bodyRate[Y])`). 0 if `USE_OPFLOW` disabled |

**Notes:** Requires `USE_OPFLOW`.

## <a id="msp2_inav_analog"></a>`MSP2_INAV_ANALOG (8194 / 0x2002)`
**Description:** Provides detailed analog sensor readings, superseding `MSP_ANALOG` with higher precision and additional fields.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `batteryFlags` | `uint8_t` | 1 | Bitmask | Bitmask: Bit0=Full on plug-in, Bit1=Use capacity thresholds, Bits2-3=`batteryState_e` (`getBatteryState()`), Bits4-7=Cell count (`getBatteryCellCount()`) |
| `vbat` | `uint16_t` | 2 | 0.01V | Battery voltage (`getBatteryVoltage()`) |
| `amperage` | `int16_t` | 2 | 0.01A | Current draw (`getAmperage()`) |
| `powerDraw` | `uint32_t` | 4 | 0.01W | Power draw (`getPower()`) |
| `mAhDrawn` | `uint32_t` | 4 | mAh | Consumed capacity (`getMAhDrawn()`) |
| `mWhDrawn` | `uint32_t` | 4 | mWh | Consumed energy (`getMWhDrawn()`) |
| `remainingCapacity` | `uint32_t` | 4 | Capacity unit (`batteryMetersConfig()->capacity_unit`) | Estimated remaining capacity (`getBatteryRemainingCapacity()`) |
| `percentageRemaining` | `uint8_t` | 1 | % | Estimated remaining capacity percentage (`calculateBatteryPercentage()`) |
| `rssi` | `uint16_t` | 2 | Raw (0-1023) | RSSI value (`getRSSI()`) |

**Notes:** Requires `USE_CURRENT_METER`/`USE_ADC` for current-related fields; values fall back to zero when unavailable. Capacity fields are reported in the units configured by `batteryMetersConfig()->capacity_unit` (mAh or mWh).

## <a id="msp2_inav_misc"></a>`MSP2_INAV_MISC (8195 / 0x2003)`
**Description:** Retrieves miscellaneous configuration settings, superseding `MSP_MISC` with higher precision and capacity fields.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `midRc` | `uint16_t` | 2 | PWM | Mid RC value (`PWM_RANGE_MIDDLE`) |
| `legacyMinThrottle` | `uint16_t` | 2 | - | Always 0 (Legacy) |
| `maxThrottle` | `uint16_t` | 2 | PWM | Maximum throttle command (`getMaxThrottle()`) |
| `minCommand` | `uint16_t` | 2 | PWM | Minimum motor command (`motorConfig()->mincommand`) |
| `failsafeThrottle` | `uint16_t` | 2 | PWM | Failsafe throttle level (`currentBatteryProfile->failsafe_throttle`) |
| `gpsType` | `uint8_t` | 1 | [gpsProvider_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-gpsprovider_e) | Enum `gpsProvider_e` GPS provider type (`gpsConfig()->provider`). 0 if `USE_GPS` disabled |
| `legacyGpsBaud` | `uint8_t` | 1 | - | Always 0 (Legacy) |
| `gpsSbasMode` | `uint8_t` | 1 | [sbasMode_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-sbasmode_e) | Enum `sbasMode_e` GPS SBAS mode (`gpsConfig()->sbasMode`). 0 if `USE_GPS` disabled |
| `rssiChannel` | `uint8_t` | 1 | Index | RSSI channel index (1-based, 0 disables) (`rxConfig()->rssi_channel`) |
| `magDeclination` | `int16_t` | 2 | 0.1 degrees | Magnetic declination / 10 (`compassConfig()->mag_declination / 10`). 0 if `USE_MAG` disabled |
| `vbatScale` | `uint16_t` | 2 | Scale | Voltage scale (`batteryMetersConfig()->voltage.scale`). 0 if `USE_ADC` disabled |
| `vbatSource` | `uint8_t` | 1 | [batVoltageSource_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-batvoltagesource_e) | Enum `batVoltageSource_e` Voltage source (`batteryMetersConfig()->voltageSource`). 0 if `USE_ADC` disabled |
| `cellCount` | `uint8_t` | 1 | Count | Configured cell count (`currentBatteryProfile->cells`). 0 if `USE_ADC` disabled |
| `vbatCellDetect` | `uint16_t` | 2 | 0.01V | Cell detection voltage (`currentBatteryProfile->voltage.cellDetect`). 0 if `USE_ADC` disabled |
| `vbatMinCell` | `uint16_t` | 2 | 0.01V | Min cell voltage (`currentBatteryProfile->voltage.cellMin`). 0 if `USE_ADC` disabled |
| `vbatMaxCell` | `uint16_t` | 2 | 0.01V | Max cell voltage (`currentBatteryProfile->voltage.cellMax`). 0 if `USE_ADC` disabled |
| `vbatWarningCell` | `uint16_t` | 2 | 0.01V | Warning cell voltage (`currentBatteryProfile->voltage.cellWarning`). 0 if `USE_ADC` disabled |
| `capacityValue` | `uint32_t` | 4 | mAh/mWh | Battery capacity (`currentBatteryProfile->capacity.value`) |
| `capacityWarning` | `uint32_t` | 4 | mAh/mWh | Capacity warning threshold (`currentBatteryProfile->capacity.warning`) |
| `capacityCritical` | `uint32_t` | 4 | mAh/mWh | Capacity critical threshold (`currentBatteryProfile->capacity.critical`) |
| `capacityUnit` | `uint8_t` | 1 | [batCapacityUnit_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-batcapacityunit_e) | Enum `batCapacityUnit_e` Capacity unit (`batteryMetersConfig()->capacity_unit`) |

## <a id="msp2_inav_set_misc"></a>`MSP2_INAV_SET_MISC (8196 / 0x2004)`
**Description:** Sets miscellaneous configuration settings, superseding `MSP_SET_MISC`.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `midRc` | `uint16_t` | 2 | PWM | Ignored |
| `legacyMinThrottle` | `uint16_t` | 2 | - | Ignored |
| `legacyMaxThrottle` | `uint16_t` | 2 | - | Ignored |
| `minCommand` | `uint16_t` | 2 | PWM | Sets `motorConfigMutable()->mincommand` (constrained) |
| `failsafeThrottle` | `uint16_t` | 2 | PWM | Sets `currentBatteryProfileMutable->failsafe_throttle` (constrained) |
| `gpsType` | `uint8_t` | 1 | [gpsProvider_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-gpsprovider_e) | Enum `gpsProvider_e` Sets `gpsConfigMutable()->provider` (if `USE_GPS`) |
| `legacyGpsBaud` | `uint8_t` | 1 | - | Ignored |
| `gpsSbasMode` | `uint8_t` | 1 | [sbasMode_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-sbasmode_e) | Enum `sbasMode_e` Sets `gpsConfigMutable()->sbasMode` (if `USE_GPS`) |
| `rssiChannel` | `uint8_t` | 1 | Index | Sets `rxConfigMutable()->rssi_channel` (1-based, 0 disables) when <= `MAX_SUPPORTED_RC_CHANNEL_COUNT` |
| `magDeclination` | `int16_t` | 2 | 0.1 degrees | Sets `compassConfigMutable()->mag_declination = value * 10` (if `USE_MAG`) |
| `vbatScale` | `uint16_t` | 2 | Scale | Sets `batteryMetersConfigMutable()->voltage.scale` (if `USE_ADC`) |
| `vbatSource` | `uint8_t` | 1 | [batVoltageSource_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-batvoltagesource_e) | Enum `batVoltageSource_e` Sets `batteryMetersConfigMutable()->voltageSource` (if `USE_ADC`, validated) |
| `cellCount` | `uint8_t` | 1 | Count | Sets `currentBatteryProfileMutable->cells` (if `USE_ADC`) |
| `vbatCellDetect` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellDetect` (if `USE_ADC`) |
| `vbatMinCell` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellMin` (if `USE_ADC`) |
| `vbatMaxCell` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellMax` (if `USE_ADC`) |
| `vbatWarningCell` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellWarning` (if `USE_ADC`) |
| `capacityValue` | `uint32_t` | 4 | mAh/mWh | Sets `currentBatteryProfileMutable->capacity.value` |
| `capacityWarning` | `uint32_t` | 4 | mAh/mWh | Sets `currentBatteryProfileMutable->capacity.warning` |
| `capacityCritical` | `uint32_t` | 4 | mAh/mWh | Sets `currentBatteryProfileMutable->capacity.critical` |
| `capacityUnit` | `uint8_t` | 1 | [batCapacityUnit_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-batcapacityunit_e) | Enum `batCapacityUnit_e` Sets `batteryMetersConfigMutable()->capacity_unit` (validated, updates OSD energy unit if changed) |

**Reply Payload:** **None**  

**Notes:** Expects 41 bytes. Performs validation on `vbatSource` and `capacityUnit`.

## <a id="msp2_inav_battery_config"></a>`MSP2_INAV_BATTERY_CONFIG (8197 / 0x2005)`
**Description:** Retrieves the configuration specific to the battery voltage and current sensors and capacity settings for the current battery profile.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `vbatScale` | `uint16_t` | 2 | Scale | Voltage scale (`batteryMetersConfig()->voltage.scale`) |
| `vbatSource` | `uint8_t` | 1 | [batVoltageSource_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-batvoltagesource_e) | Enum `batVoltageSource_e` Voltage source (`batteryMetersConfig()->voltageSource`) |
| `cellCount` | `uint8_t` | 1 | Count | Configured cell count (`currentBatteryProfile->cells`) |
| `vbatCellDetect` | `uint16_t` | 2 | 0.01V | Cell detection voltage (`currentBatteryProfile->voltage.cellDetect`) |
| `vbatMinCell` | `uint16_t` | 2 | 0.01V | Min cell voltage (`currentBatteryProfile->voltage.cellMin`) |
| `vbatMaxCell` | `uint16_t` | 2 | 0.01V | Max cell voltage (`currentBatteryProfile->voltage.cellMax`) |
| `vbatWarningCell` | `uint16_t` | 2 | 0.01V | Warning cell voltage (`currentBatteryProfile->voltage.cellWarning`) |
| `currentOffset` | `int16_t` | 2 | mV | Current sensor offset (`batteryMetersConfig()->current.offset`) |
| `currentScale` | `int16_t` | 2 | 0.1 mV/A | Current sensor scale (`batteryMetersConfig()->current.scale`) |
| `capacityValue` | `uint32_t` | 4 | mAh/mWh | Battery capacity (`currentBatteryProfile->capacity.value`) |
| `capacityWarning` | `uint32_t` | 4 | mAh/mWh | Capacity warning threshold (`currentBatteryProfile->capacity.warning`) |
| `capacityCritical` | `uint32_t` | 4 | mAh/mWh | Capacity critical threshold (`currentBatteryProfile->capacity.critical`) |
| `capacityUnit` | `uint8_t` | 1 | [batCapacityUnit_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-batcapacityunit_e) | Enum `batCapacityUnit_e` Capacity unit (`batteryMetersConfig()->capacity_unit`) |

**Notes:** Fields are 0 if `USE_ADC` is not defined.

## <a id="msp2_inav_set_battery_config"></a>`MSP2_INAV_SET_BATTERY_CONFIG (8198 / 0x2006)`
**Description:** Sets the battery voltage/current sensor configuration and capacity settings for the current battery profile.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `vbatScale` | `uint16_t` | 2 | Scale | Sets `batteryMetersConfigMutable()->voltage.scale` (if `USE_ADC`) |
| `vbatSource` | `uint8_t` | 1 | [batVoltageSource_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-batvoltagesource_e) | Enum `batVoltageSource_e` Sets `batteryMetersConfigMutable()->voltageSource` (if `USE_ADC`, validated) |
| `cellCount` | `uint8_t` | 1 | Count | Sets `currentBatteryProfileMutable->cells` (if `USE_ADC`) |
| `vbatCellDetect` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellDetect` (if `USE_ADC`) |
| `vbatMinCell` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellMin` (if `USE_ADC`) |
| `vbatMaxCell` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellMax` (if `USE_ADC`) |
| `vbatWarningCell` | `uint16_t` | 2 | 0.01V | Sets `currentBatteryProfileMutable->voltage.cellWarning` (if `USE_ADC`) |
| `currentOffset` | `int16_t` | 2 | mV | Sets `batteryMetersConfigMutable()->current.offset` |
| `currentScale` | `int16_t` | 2 | 0.1 mV/A | Sets `batteryMetersConfigMutable()->current.scale` |
| `capacityValue` | `uint32_t` | 4 | mAh/mWh | Sets `currentBatteryProfileMutable->capacity.value` |
| `capacityWarning` | `uint32_t` | 4 | mAh/mWh | Sets `currentBatteryProfileMutable->capacity.warning` |
| `capacityCritical` | `uint32_t` | 4 | mAh/mWh | Sets `currentBatteryProfileMutable->capacity.critical` |
| `capacityUnit` | `uint8_t` | 1 | [batCapacityUnit_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-batcapacityunit_e) | Enum `batCapacityUnit_e` Sets `batteryMetersConfigMutable()->capacity_unit` (validated, updates OSD energy unit if changed) |

**Reply Payload:** **None**  

**Notes:** Expects 29 bytes. Performs validation on `vbatSource` and `capacityUnit`.

## <a id="msp2_inav_rate_profile"></a>`MSP2_INAV_RATE_PROFILE (8199 / 0x2007)`
**Description:** Retrieves the rates and expos for the current control rate profile, including both stabilized and manual flight modes. Supersedes `MSP_RC_TUNING`.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `throttleMid` | `uint8_t` | 1 | Throttle Midpoint (`currentControlRateProfile->throttle.rcMid8`) |
| `throttleExpo` | `uint8_t` | 1 | Throttle Expo (`currentControlRateProfile->throttle.rcExpo8`) |
| `dynamicThrottlePID` | `uint8_t` | 1 | TPA value (`currentControlRateProfile->throttle.dynPID`) |
| `tpaBreakpoint` | `uint16_t` | 2 | TPA breakpoint (`currentControlRateProfile->throttle.pa_breakpoint`) |
| `stabRcExpo` | `uint8_t` | 1 | Stabilized Roll/Pitch Expo (`currentControlRateProfile->stabilized.rcExpo8`) |
| `stabRcYawExpo` | `uint8_t` | 1 | Stabilized Yaw Expo (`currentControlRateProfile->stabilized.rcYawExpo8`) |
| `stabRollRate` | `uint8_t` | 1 | Stabilized Roll Rate (`currentControlRateProfile->stabilized.rates[FD_ROLL]`) |
| `stabPitchRate` | `uint8_t` | 1 | Stabilized Pitch Rate (`currentControlRateProfile->stabilized.rates[FD_PITCH]`) |
| `stabYawRate` | `uint8_t` | 1 | Stabilized Yaw Rate (`currentControlRateProfile->stabilized.rates[FD_YAW]`) |
| `manualRcExpo` | `uint8_t` | 1 | Manual Roll/Pitch Expo (`currentControlRateProfile->manual.rcExpo8`) |
| `manualRcYawExpo` | `uint8_t` | 1 | Manual Yaw Expo (`currentControlRateProfile->manual.rcYawExpo8`) |
| `manualRollRate` | `uint8_t` | 1 | Manual Roll Rate (`currentControlRateProfile->manual.rates[FD_ROLL]`) |
| `manualPitchRate` | `uint8_t` | 1 | Manual Pitch Rate (`currentControlRateProfile->manual.rates[FD_PITCH]`) |
| `manualYawRate` | `uint8_t` | 1 | Manual Yaw Rate (`currentControlRateProfile->manual.rates[FD_YAW]`) |

## <a id="msp2_inav_set_rate_profile"></a>`MSP2_INAV_SET_RATE_PROFILE (8200 / 0x2008)`
**Description:** Sets the rates and expos for the current control rate profile (stabilized and manual). Supersedes `MSP_SET_RC_TUNING`.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `throttleMid` | `uint8_t` | 1 | Sets `currentControlRateProfile->throttle.rcMid8` |
| `throttleExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile->throttle.rcExpo8` |
| `dynamicThrottlePID` | `uint8_t` | 1 | Sets `currentControlRateProfile->throttle.dynPID` |
| `tpaBreakpoint` | `uint16_t` | 2 | Sets `currentControlRateProfile->throttle.pa_breakpoint` |
| `stabRcExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile->stabilized.rcExpo8` |
| `stabRcYawExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile->stabilized.rcYawExpo8` |
| `stabRollRate` | `uint8_t` | 1 | Sets `currentControlRateProfile->stabilized.rates[FD_ROLL]` (constrained) |
| `stabPitchRate` | `uint8_t` | 1 | Sets `currentControlRateProfile->stabilized.rates[FD_PITCH]` (constrained) |
| `stabYawRate` | `uint8_t` | 1 | Sets `currentControlRateProfile->stabilized.rates[FD_YAW]` (constrained) |
| `manualRcExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile->manual.rcExpo8` |
| `manualRcYawExpo` | `uint8_t` | 1 | Sets `currentControlRateProfile->manual.rcYawExpo8` |
| `manualRollRate` | `uint8_t` | 1 | Sets `currentControlRateProfile->manual.rates[FD_ROLL]` (constrained) |
| `manualPitchRate` | `uint8_t` | 1 | Sets `currentControlRateProfile->manual.rates[FD_PITCH]` (constrained) |
| `manualYawRate` | `uint8_t` | 1 | Sets `currentControlRateProfile->manual.rates[FD_YAW]` (constrained) |

**Reply Payload:** **None**  

**Notes:** Expects 15 bytes. Constraints applied to rates based on axis.

## <a id="msp2_inav_air_speed"></a>`MSP2_INAV_AIR_SPEED (8201 / 0x2009)`
**Description:** Retrieves the estimated or measured airspeed.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `airspeed` | `uint32_t` | 4 | cm/s | Estimated/measured airspeed (`getAirspeedEstimate()`, cm/s). 0 if unavailable |

**Notes:** Requires `USE_PITOT`; returns 0 when pitot functionality is not enabled or calibrated.

## <a id="msp2_inav_output_mapping"></a>`MSP2_INAV_OUTPUT_MAPPING (8202 / 0x200a)`
**Description:** Retrieves the output mapping configuration (identifies which timer outputs are used for Motors/Servos). Legacy version sending only 8-bit usage flags.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `usageFlags` | `uint8_t` | 1 | Timer usage flags (lower 8 bits of `timerHardware[i].usageFlags`, e.g. `TIM_USE_MOTOR`, `TIM_USE_SERVO`) |

**Notes:** Superseded by `MSP2_INAV_OUTPUT_MAPPING_EXT2`. Only includes timers *not* used for PPM/PWM input.

## <a id="msp2_inav_mc_braking"></a>`MSP2_INAV_MC_BRAKING (8203 / 0x200b)`
**Description:** Retrieves configuration parameters for the multirotor braking mode feature.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `brakingSpeedThreshold` | `uint16_t` | 2 | cm/s | Speed above which braking engages (`navConfig()->mc.braking_speed_threshold`) |
| `brakingDisengageSpeed` | `uint16_t` | 2 | cm/s | Speed below which braking disengages (`navConfig()->mc.braking_disengage_speed`) |
| `brakingTimeout` | `uint16_t` | 2 | ms | Timeout before braking force reduces (`navConfig()->mc.braking_timeout`) |
| `brakingBoostFactor` | `uint8_t` | 1 | % | Boost factor applied during braking (`navConfig()->mc.braking_boost_factor`) |
| `brakingBoostTimeout` | `uint16_t` | 2 | ms | Timeout for the boost factor (`navConfig()->mc.braking_boost_timeout`) |
| `brakingBoostSpeedThreshold` | `uint16_t` | 2 | cm/s | Speed threshold for boost engagement (`navConfig()->mc.braking_boost_speed_threshold`) |
| `brakingBoostDisengageSpeed` | `uint16_t` | 2 | cm/s | Speed threshold for boost disengagement (`navConfig()->mc.braking_boost_disengage_speed`) |
| `brakingBankAngle` | `uint8_t` | 1 | degrees | Maximum bank angle allowed during braking (`navConfig()->mc.braking_bank_angle`) |

**Notes:** Payload is empty if `USE_MR_BRAKING_MODE` is not defined.

## <a id="msp2_inav_set_mc_braking"></a>`MSP2_INAV_SET_MC_BRAKING (8204 / 0x200c)`
**Description:** Sets configuration parameters for the multirotor braking mode feature.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `brakingSpeedThreshold` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->mc.braking_speed_threshold` |
| `brakingDisengageSpeed` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->mc.braking_disengage_speed` |
| `brakingTimeout` | `uint16_t` | 2 | ms | Sets `navConfigMutable()->mc.braking_timeout` |
| `brakingBoostFactor` | `uint8_t` | 1 | % | Sets `navConfigMutable()->mc.braking_boost_factor` |
| `brakingBoostTimeout` | `uint16_t` | 2 | ms | Sets `navConfigMutable()->mc.braking_boost_timeout` |
| `brakingBoostSpeedThreshold` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->mc.braking_boost_speed_threshold` |
| `brakingBoostDisengageSpeed` | `uint16_t` | 2 | cm/s | Sets `navConfigMutable()->mc.braking_boost_disengage_speed` |
| `brakingBankAngle` | `uint8_t` | 1 | degrees | Sets `navConfigMutable()->mc.braking_bank_angle` |

**Reply Payload:** **None**  

**Notes:** Expects 14 bytes. Returns error if `USE_MR_BRAKING_MODE` is not defined.

## <a id="msp2_inav_output_mapping_ext"></a>`MSP2_INAV_OUTPUT_MAPPING_EXT (8205 / 0x200d)`
**Description:** Retrieves extended output mapping configuration (timer ID and usage flags). Obsolete, use `MSP2_INAV_OUTPUT_MAPPING_EXT2`.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `timerId` | `uint8_t` | 1 | Hardware timer identifier (e.g., `TIM1`, `TIM2`). Value depends on target |
| `usageFlags` | `uint8_t` | 1 | Timer usage flags (lower 8 bits of `timerHardware[i].usageFlags`, e.g. `TIM_USE_MOTOR`, `TIM_USE_SERVO`) |

**Notes:** Usage flags are truncated to 8 bits. `timerId` mapping is target-specific.

## <a id="msp2_inav_timer_output_mode"></a>`MSP2_INAV_TIMER_OUTPUT_MODE (8206 / 0x200e)`
**Description:** Reads timer output mode overrides.  
#### Variant: `dataSize == 0`

**Description:** List all timers  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `timerIndex` | `uint8_t` | 1 | - | Timer index |
| `outputMode` | `uint8_t` | 1 | [outputMode_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-outputmode_e) | OUTPUT_MODE_* |

#### Variant: `dataSize == 1`

**Description:** Query one timer  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `timerIndex` | `uint8_t` | 1 | 0..HARDWARE_TIMER_DEFINITION_COUNT-1 |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `timerIndex` | `uint8_t` | 1 | - | Echoed timer index |
| `outputMode` | `uint8_t` | 1 | [outputMode_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-outputmode_e) | OUTPUT_MODE_* |


**Notes:** Non-SITL only. HARDWARE_TIMER_DEFINITION_COUNT is target specific. Returns MSP_RESULT_ACK on success, MSP_RESULT_ERROR on invalid timer index.

## <a id="msp2_inav_set_timer_output_mode"></a>`MSP2_INAV_SET_TIMER_OUTPUT_MODE (8207 / 0x200f)`
**Description:** Set the output mode override for a specific hardware timer.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `timerIndex` | `uint8_t` | 1 | - | Index of the hardware timer definition |
| `outputMode` | `uint8_t` | 1 | [outputMode_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-outputmode_e) | Output mode override (`outputMode_e` enum) to set |

**Reply Payload:** **None**  

**Notes:** Only available on non-SITL builds. Expects 2 bytes. Returns error if `timerIndex` is invalid.

## <a id="msp2_inav_mixer"></a>`MSP2_INAV_MIXER (8208 / 0x2010)`
**Description:** Retrieves INAV-specific mixer configuration details.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `motorDirectionInverted` | `uint8_t` | 1 | - | Boolean: 1 if motor direction is reversed globally (`mixerConfig()->motorDirectionInverted`) |
| `reserved1` | `uint8_t` | 1 | - | Always 0 (Was yaw jump prevention limit) |
| `motorStopOnLow` | `uint8_t` | 1 | - | Boolean: 1 if motors stop at minimum throttle (`mixerConfig()->motorstopOnLow`) |
| `platformType` | `uint8_t` | 1 | [flyingPlatformType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-flyingplatformtype_e) | Enum (`mixerConfig()->platformType`) |
| `hasFlaps` | `uint8_t` | 1 | - | Boolean: 1 if the current mixer configuration includes flaps (`mixerConfig()->hasFlaps`) |
| `appliedMixerPreset` | `int16_t` | 2 | [mixerPreset_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-mixerpreset_e) | Enum (`mixerPreset_e`): Mixer preset currently applied (`mixerConfig()->appliedMixerPreset`) WARNING: cannot figure where this is |
| `maxMotors` | `uint8_t` | 1 | - | Constant: Maximum motors supported (`MAX_SUPPORTED_MOTORS`) |
| `maxServos` | `uint8_t` | 1 | - | Constant: Maximum servos supported (`MAX_SUPPORTED_SERVOS`) |

## <a id="msp2_inav_set_mixer"></a>`MSP2_INAV_SET_MIXER (8209 / 0x2011)`
**Description:** Sets INAV-specific mixer configuration details.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `motorDirectionInverted` | `uint8_t` | 1 | - | Sets `mixerConfigMutable()->motorDirectionInverted` |
| `reserved1` | `uint8_t` | 1 | - | Ignored |
| `motorStopOnLow` | `uint8_t` | 1 | - | Sets `mixerConfigMutable()->motorstopOnLow` |
| `platformType` | `uint8_t` | 1 | [flyingPlatformType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-flyingplatformtype_e) | Sets `mixerConfigMutable()->platformType` |
| `hasFlaps` | `uint8_t` | 1 | - | Sets `mixerConfigMutable()->hasFlaps` |
| `appliedMixerPreset` | `int16_t` | 2 | - | Sets `mixerConfigMutable()->appliedMixerPreset` |
| `maxMotors` | `uint8_t` | 1 | - | Ignored |
| `maxServos` | `uint8_t` | 1 | - | Ignored |

**Reply Payload:** **None**  

**Notes:** Expects 9 bytes. Calls `mixerUpdateStateFlags()`.

## <a id="msp2_inav_osd_layouts"></a>`MSP2_INAV_OSD_LAYOUTS (8210 / 0x2012)`
**Description:** Retrieves OSD layout metadata or item positions for specific layouts/items.  
#### Variant: `dataSize == 0`

**Description:** Query layout/item counts  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `layoutCount` | `uint8_t` | 1 | Number of OSD layouts (`OSD_LAYOUT_COUNT`) |
| `itemCount` | `uint8_t` | 1 | Number of OSD items per layout (`OSD_ITEM_COUNT`) |

#### Variant: `dataSize == 1`

**Description:** Fetch all item positions for a layout  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `layoutIndex` | `uint8_t` | 1 | Layout index (0 to `OSD_LAYOUT_COUNT - 1`) |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `itemPosition` | `uint16_t` | 2 | packed coords | Packed X/Y position (`osdLayoutsConfig()->item_pos[layoutIndex][item]`) |

#### Variant: `dataSize == 3`

**Description:** Fetch a single item position  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `layoutIndex` | `uint8_t` | 1 | Layout index (0 to `OSD_LAYOUT_COUNT - 1`) |
| `itemIndex` | `uint16_t` | 2 | OSD item index (0 to `OSD_ITEM_COUNT - 1`) |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `itemPosition` | `uint16_t` | 2 | packed coords | Packed X/Y position (`osdLayoutsConfig()->item_pos[layoutIndex][itemIndex]`) |


**Notes:** Requires `USE_OSD`. Returns `MSP_RESULT_ACK` on success, `MSP_RESULT_ERROR` if indexes are out of range.

## <a id="msp2_inav_osd_set_layout_item"></a>`MSP2_INAV_OSD_SET_LAYOUT_ITEM (8211 / 0x2013)`
**Description:** Sets the position of a single OSD item within a specific layout.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `layoutIndex` | `uint8_t` | 1 | Index | Index of the OSD layout (0 to `OSD_LAYOUT_COUNT - 1`) |
| `itemIndex` | `uint8_t` | 1 | Index | Index of the OSD item |
| `itemPosition` | `uint16_t` | 2 | Coordinates | Packed X/Y position using `OSD_POS(x, y)` with `OSD_VISIBLE_FLAG` bit |

**Reply Payload:** **None**  

**Notes:** Requires `USE_OSD`. Expects 4 bytes. Returns error if indexes are invalid. If the modified layout is not the currently active one, it temporarily overrides the active layout for 10 seconds to show the change. Otherwise, triggers a full OSD redraw.

## <a id="msp2_inav_osd_alarms"></a>`MSP2_INAV_OSD_ALARMS (8212 / 0x2014)`
**Description:** Retrieves OSD alarm threshold settings.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `rssiAlarm` | `uint8_t` | 1 | % | RSSI alarm threshold (`osdConfig()->rssi_alarm`) |
| `timerAlarm` | `uint16_t` | 2 | seconds | Timer alarm threshold (`osdConfig()->time_alarm`) |
| `altAlarm` | `uint16_t` | 2 | meters | Altitude alarm threshold (`osdConfig()->alt_alarm`) |
| `distAlarm` | `uint16_t` | 2 | meters | Distance alarm threshold (`osdConfig()->dist_alarm`) |
| `negAltAlarm` | `uint16_t` | 2 | meters | Negative altitude alarm threshold (`osdConfig()->neg_alt_alarm`) |
| `gForceAlarm` | `uint16_t` | 2 | G * 1000 | G-force alarm threshold (`osdConfig()->gforce_alarm * 1000`) |
| `gForceAxisMinAlarm` | `int16_t` | 2 | G * 1000 | Min G-force per-axis alarm (`osdConfig()->gforce_axis_alarm_min * 1000`) |
| `gForceAxisMaxAlarm` | `int16_t` | 2 | G * 1000 | Max G-force per-axis alarm (`osdConfig()->gforce_axis_alarm_max * 1000`) |
| `currentAlarm` | `uint8_t` | 1 | A | Current draw alarm threshold (`osdConfig()->current_alarm`) |
| `imuTempMinAlarm` | `int16_t` | 2 | degrees C | Min IMU temperature alarm (`osdConfig()->imu_temp_alarm_min`) |
| `imuTempMaxAlarm` | `int16_t` | 2 | degrees C | Max IMU temperature alarm (`osdConfig()->imu_temp_alarm_max`) |
| `baroTempMinAlarm` | `int16_t` | 2 | degrees C | Min Baro temperature alarm (`osdConfig()->baro_temp_alarm_min`). 0 if `USE_BARO` disabled |
| `baroTempMaxAlarm` | `int16_t` | 2 | degrees C | Max Baro temperature alarm (`osdConfig()->baro_temp_alarm_max`). 0 if `USE_BARO` disabled |
| `adsbWarnDistance` | `uint16_t` | 2 | meters | ADSB warning distance (`osdConfig()->adsb_distance_warning`). 0 if `USE_ADSB` disabled |
| `adsbAlertDistance` | `uint16_t` | 2 | meters | ADSB alert distance (`osdConfig()->adsb_distance_alert`). 0 if `USE_ADSB` disabled |

**Notes:** Requires `USE_OSD`.

## <a id="msp2_inav_osd_set_alarms"></a>`MSP2_INAV_OSD_SET_ALARMS (8213 / 0x2015)`
**Description:** Sets OSD alarm threshold settings.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `rssiAlarm` | `uint8_t` | 1 | % | Sets `osdConfigMutable()->rssi_alarm |
| `timerAlarm` | `uint16_t` | 2 | seconds | Sets `osdConfigMutable()->time_alarm |
| `altAlarm` | `uint16_t` | 2 | meters | Sets `osdConfigMutable()->alt_alarm |
| `distAlarm` | `uint16_t` | 2 | meters | Sets `osdConfigMutable()->dist_alarm |
| `negAltAlarm` | `uint16_t` | 2 | meters | Sets `osdConfigMutable()->neg_alt_alarm` |
| `gForceAlarm` | `uint16_t` | 2 | G * 1000 | Sets `osdConfigMutable()->gforce_alarm = value / 1000.0f` |
| `gForceAxisMinAlarm` | `int16_t` | 2 | G * 1000 | Sets `osdConfigMutable()->gforce_axis_alarm_min = value / 1000.0f` |
| `gForceAxisMaxAlarm` | `int16_t` | 2 | G * 1000 | Sets `osdConfigMutable()->gforce_axis_alarm_max = value / 1000.0f` |
| `currentAlarm` | `uint8_t` | 1 | A | Sets `osdConfigMutable()->current_alarm` |
| `imuTempMinAlarm` | `int16_t` | 2 | degrees C | Sets `osdConfigMutable()->imu_temp_alarm_min` |
| `imuTempMaxAlarm` | `int16_t` | 2 | degrees C | Sets `osdConfigMutable()->imu_temp_alarm_max` |
| `baroTempMinAlarm` | `int16_t` | 2 | degrees C | Sets `osdConfigMutable()->baro_temp_alarm_min` (if `USE_BARO`) |
| `baroTempMaxAlarm` | `int16_t` | 2 | degrees C | Sets `osdConfigMutable()->baro_temp_alarm_max` (if `USE_BARO`) |

**Reply Payload:** **None**  

**Notes:** Requires `USE_OSD`. Expects 24 bytes. ADSB alarms are not settable via this message.

## <a id="msp2_inav_osd_preferences"></a>`MSP2_INAV_OSD_PREFERENCES (8214 / 0x2016)`
**Description:** Retrieves OSD display preferences (video system, units, styles, etc.).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `videoSystem` | `uint8_t` | 1 | [videoSystem_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-videosystem_e) | Enum `videoSystem_e`: Video system (Auto/PAL/NTSC) (`osdConfig()->video_system`) |
| `mainVoltageDecimals` | `uint8_t` | 1 | - | Count: Decimal places for main voltage display (`osdConfig()->main_voltage_decimals`) |
| `ahiReverseRoll` | `uint8_t` | 1 | - | Boolean: Reverse roll direction on Artificial Horizon (`osdConfig()->ahi_reverse_roll`) |
| `crosshairsStyle` | `uint8_t` | 1 | [osd_crosshairs_style_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osd_crosshairs_style_e) | Enum `osd_crosshairs_style_e`: Style of the center crosshairs (`osdConfig()->crosshairs_style`) |
| `leftSidebarScroll` | `uint8_t` | 1 | [osd_sidebar_scroll_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osd_sidebar_scroll_e) | Enum `osd_sidebar_scroll_e`: Left sidebar scroll behavior (`osdConfig()->left_sidebar_scroll`) |
| `rightSidebarScroll` | `uint8_t` | 1 | [osd_sidebar_scroll_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osd_sidebar_scroll_e) | Enum `osd_sidebar_scroll_e`: Right sidebar scroll behavior (`osdConfig()->right_sidebar_scroll`) |
| `sidebarScrollArrows` | `uint8_t` | 1 | - | Boolean: Show arrows for scrollable sidebars (`osdConfig()->sidebar_scroll_arrows`) |
| `units` | `uint8_t` | 1 | [osd_unit_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osd_unit_e) | Enum: `osd_unit_e` Measurement units (Metric/Imperial) (`osdConfig()->units`) |
| `statsEnergyUnit` | `uint8_t` | 1 | [osd_stats_energy_unit_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osd_stats_energy_unit_e) | Enum `osd_stats_energy_unit_e`: Unit for energy display in post-flight stats (`osdConfig()->stats_energy_unit`) |

**Notes:** Requires `USE_OSD`.

## <a id="msp2_inav_osd_set_preferences"></a>`MSP2_INAV_OSD_SET_PREFERENCES (8215 / 0x2017)`
**Description:** Sets OSD display preferences.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `videoSystem` | `uint8_t` | 1 | [videoSystem_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-videosystem_e) | Sets `osdConfigMutable()->video_system` |
| `mainVoltageDecimals` | `uint8_t` | 1 | - | Sets `osdConfigMutable()->main_voltage_decimals` |
| `ahiReverseRoll` | `uint8_t` | 1 | - | Sets `osdConfigMutable()->ahi_reverse_roll` |
| `crosshairsStyle` | `uint8_t` | 1 | [osd_crosshairs_style_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osd_crosshairs_style_e) | Sets `osdConfigMutable()->crosshairs_style` |
| `leftSidebarScroll` | `uint8_t` | 1 | [osd_sidebar_scroll_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osd_sidebar_scroll_e) | Sets `osdConfigMutable()->left_sidebar_scroll` |
| `rightSidebarScroll` | `uint8_t` | 1 | [osd_sidebar_scroll_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osd_sidebar_scroll_e) | Sets `osdConfigMutable()->right_sidebar_scroll` |
| `sidebarScrollArrows` | `uint8_t` | 1 | - | Sets `osdConfigMutable()->sidebar_scroll_arrows` |
| `units` | `uint8_t` | 1 | [osd_unit_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osd_unit_e) | Sets `osdConfigMutable()->units` (enum `osd_unit_e`) |
| `statsEnergyUnit` | `uint8_t` | 1 | [osd_stats_energy_unit_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osd_stats_energy_unit_e) | Sets `osdConfigMutable()->stats_energy_unit` |

**Reply Payload:** **None**  

**Notes:** Requires `USE_OSD`. Expects 9 bytes. Triggers a full OSD redraw.

## <a id="msp2_inav_select_battery_profile"></a>`MSP2_INAV_SELECT_BATTERY_PROFILE (8216 / 0x2018)`
**Description:** Selects the active battery profile and saves configuration.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `batteryProfileIndex` | `uint8_t` | 1 | Index of the battery profile to activate (0-based) |

**Reply Payload:** **None**  

**Notes:** Expects 1 byte. Will fail if armed. Calls `setConfigBatteryProfileAndWriteEEPROM()`.

## <a id="msp2_inav_debug"></a>`MSP2_INAV_DEBUG (8217 / 0x2019)`
**Description:** Retrieves values from the firmware's 32-bit `debug[]` array. Supersedes `MSP_DEBUG`.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `debugValues` | `int32_t[DEBUG32_VALUE_COUNT]` | 32 (DEBUG32_VALUE_COUNT) | Values from the `debug` array (signed, typically 8 entries) |

**Notes:** `DEBUG32_VALUE_COUNT` is usually 8.

## <a id="msp2_blackbox_config"></a>`MSP2_BLACKBOX_CONFIG (8218 / 0x201a)`
**Description:** Retrieves the Blackbox configuration. Supersedes `MSP_BLACKBOX_CONFIG`.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `blackboxSupported` | `uint8_t` | 1 | - | Boolean: 1 if Blackbox is supported (`USE_BLACKBOX`), 0 otherwise |
| `blackboxDevice` | `uint8_t` | 1 | [BlackboxDevice](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-blackboxdevice) | Enum `BlackboxDevice`: Target device for logging (`blackboxConfig()->device`). 0 if not supported |
| `blackboxRateNum` | `uint16_t` | 2 | - | Numerator for logging rate divider (`blackboxConfig()->rate_num`). 0 if not supported |
| `blackboxRateDenom` | `uint16_t` | 2 | - | Denominator for logging rate divider (`blackboxConfig()->rate_denom`). 0 if not supported |
| `blackboxIncludeFlags` | `uint32_t` | 4 | - | Bitmask: Flags for fields included/excluded from logging (`blackboxConfig()->includeFlags`) |

**Notes:** If `USE_BLACKBOX` is disabled, only the first four fields are returned (all zero).

## <a id="msp2_set_blackbox_config"></a>`MSP2_SET_BLACKBOX_CONFIG (8219 / 0x201b)`
**Description:** Sets the Blackbox configuration. Supersedes `MSP_SET_BLACKBOX_CONFIG`.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `blackboxDevice` | `uint8_t` | 1 | [BlackboxDevice](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-blackboxdevice) | Sets `blackboxConfigMutable()->device` |
| `blackboxRateNum` | `uint16_t` | 2 | - | Sets `blackboxConfigMutable()->rate_num` |
| `blackboxRateDenom` | `uint16_t` | 2 | - | Sets `blackboxConfigMutable()->rate_denom` |
| `blackboxIncludeFlags` | `uint32_t` | 4 | - | Sets `blackboxConfigMutable()->includeFlags` |

**Reply Payload:** **None**  

**Notes:** Requires `USE_BLACKBOX`. Expects 9 bytes. Returns error if Blackbox is currently logging (`!blackboxMayEditConfig()`).

## <a id="msp2_inav_temp_sensor_config"></a>`MSP2_INAV_TEMP_SENSOR_CONFIG (8220 / 0x201c)`
**Description:** Retrieves the configuration for all onboard temperature sensors.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `type` | `uint8_t` | 1 | [tempSensorType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-tempsensortype_e) | Enum (`tempSensorType_e`): Type of the temperature sensor |
| `address` | `uint64_t` | 8 | - | Sensor address/ID (e.g., for 1-Wire sensors) |
| `alarmMin` | `int16_t` | 2 | 0.1°C | Min temperature alarm threshold (`sensorConfig->alarm_min`) |
| `alarmMax` | `int16_t` | 2 | 0.1°C | Max temperature alarm threshold (`sensorConfig->alarm_max`) |
| `osdSymbol` | `uint8_t` | 1 | - | Index: OSD symbol to use for this sensor (0 to `TEMP_SENSOR_SYM_COUNT`) |
| `label` | `char[TEMPERATURE_LABEL_LEN]` | 4 (TEMPERATURE_LABEL_LEN) | - | User-defined label for the sensor |

**Notes:** Requires `USE_TEMPERATURE_SENSOR`.

## <a id="msp2_inav_set_temp_sensor_config"></a>`MSP2_INAV_SET_TEMP_SENSOR_CONFIG (8221 / 0x201d)`
**Description:** Sets the configuration for all onboard temperature sensors.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `type` | `uint8_t` | 1 | [tempSensorType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-tempsensortype_e) | Sets sensor type (`tempSensorType_e`) |
| `address` | `uint64_t` | 8 | - | Sets sensor address/ID |
| `alarmMin` | `int16_t` | 2 | 0.1°C | Sets min alarm threshold (`tempSensorConfigMutable(index)->alarm_min`) |
| `alarmMax` | `int16_t` | 2 | 0.1°C | Sets max alarm threshold (`tempSensorConfigMutable(index)->alarm_max`) |
| `osdSymbol` | `uint8_t` | 1 | - | Sets OSD symbol index (validated) |
| `label` | `char[TEMPERATURE_LABEL_LEN]` | 4 (TEMPERATURE_LABEL_LEN) | - | Sets sensor label (converted to uppercase) |

**Reply Payload:** **None**  

**Notes:** Requires `USE_TEMPERATURE_SENSOR`. Payload must include `MAX_TEMP_SENSORS` consecutive `tempSensorConfig_t` structures (labels are uppercased).

## <a id="msp2_inav_temperatures"></a>`MSP2_INAV_TEMPERATURES (8222 / 0x201e)`
**Description:** Retrieves the current readings from all configured temperature sensors.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `temperature` | `int16_t` | 2 | 0.1°C | Current temperature reading. -1000 if sensor is invalid or reading failed |

**Notes:** Requires `USE_TEMPERATURE_SENSOR`.

## <a id="msp_simulator"></a>`MSP_SIMULATOR (8223 / 0x201f)`
**Description:** Handles Hardware-in-the-Loop (HITL) simulation data exchange. Receives simulated sensor data and options, sends back control outputs and debug info.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `simulatorVersion` | `uint8_t` | 1 | - | Version of the simulator protocol (`SIMULATOR_MSP_VERSION`) |
| `simulatorFlags_t` | `uint8_t` | 1 | Bitmask | Bitmask: Options for HITL (`HITL_*` flags) |
| `gpsFixType` | `uint8_t` | 1 | [gpsFixType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-gpsfixtype_e) | Enum `gpsFixType_e` Type of GPS fix (If `HITL_HAS_NEW_GPS_DATA`) |
| `gpsNumSat` | `uint8_t` | 1 | - | (If `HITL_HAS_NEW_GPS_DATA`) Simulated satellite count |
| `gpsLat` | `int32_t` | 4 | - | (If `HITL_HAS_NEW_GPS_DATA`) Simulated latitude (1e7 deg) |
| `gpsLon` | `int32_t` | 4 | - | (If `HITL_HAS_NEW_GPS_DATA`) Simulated longitude (1e7 deg) |
| `gpsAlt` | `int32_t` | 4 | - | (If `HITL_HAS_NEW_GPS_DATA`) Simulated altitude (cm) |
| `gpsSpeed` | `uint16_t` | 2 | - | (If `HITL_HAS_NEW_GPS_DATA`) Simulated ground speed (cm/s) |
| `gpsCourse` | `uint16_t` | 2 | - | (If `HITL_HAS_NEW_GPS_DATA`) Simulated ground course (deci-deg) |
| `gpsVelN` | `int16_t` | 2 | - | (If `HITL_HAS_NEW_GPS_DATA`) Simulated North velocity (cm/s) |
| `gpsVelE` | `int16_t` | 2 | - | (If `HITL_HAS_NEW_GPS_DATA`) Simulated East velocity (cm/s) |
| `gpsVelD` | `int16_t` | 2 | - | (If `HITL_HAS_NEW_GPS_DATA`) Simulated Down velocity (cm/s) |
| `imuRoll` | `int16_t` | 2 | - | (If NOT `HITL_USE_IMU`) Simulated Roll (deci-deg) |
| `imuPitch` | `int16_t` | 2 | - | (If NOT `HITL_USE_IMU`) Simulated Pitch (deci-deg) |
| `imuYaw` | `int16_t` | 2 | - | (If NOT `HITL_USE_IMU`) Simulated Yaw (deci-deg) |
| `accX` | `int16_t` | 2 | - | mG (G * 1000) |
| `accY` | `int16_t` | 2 | - | mG (G * 1000) |
| `accZ` | `int16_t` | 2 | - | mG (G * 1000) |
| `gyroX` | `int16_t` | 2 | - | dps * 16 |
| `gyroY` | `int16_t` | 2 | - | dps * 16 |
| `gyroZ` | `int16_t` | 2 | - | dps * 16 |
| `baroPressure` | `uint32_t` | 4 | - | Pa |
| `magX` | `int16_t` | 2 | - | Scaled |
| `magY` | `int16_t` | 2 | - | Scaled |
| `magZ` | `int16_t` | 2 | - | Scaled |
| `vbat` | `uint8_t` | 1 | - | (If `HITL_EXT_BATTERY_VOLTAGE`) Simulated battery voltage (0.1V units) |
| `airspeed` | `uint16_t` | 2 | - | (If `HITL_AIRSPEED`) Simulated airspeed (cm/s) |
| `extFlags` | `uint8_t` | 1 | - | (If `HITL_EXTENDED_FLAGS`) Additional flags (upper 8 bits) |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `stabilizedRoll` | `uint16_t` | 2 | Stabilized Roll command output (-500 to 500) |
| `stabilizedPitch` | `uint16_t` | 2 | Stabilized Pitch command output (-500 to 500) |
| `stabilizedYaw` | `uint16_t` | 2 | Stabilized Yaw command output (-500 to 500) |
| `stabilizedThrottle` | `uint16_t` | 2 | Stabilized Throttle command output (-500 to 500 if armed, else -500) |
| `debugFlags` | `uint8_t` | 1 | Packed flags: Debug index (0-7), Platform type, Armed state, OSD feature status |
| `debugValue` | `uint32_t` | 4 | Current debug value (`debug[simulatorData.debugIndex]`) |
| `attitudeRoll` | `int16_t` | 2 | Current estimated Roll (deci-deg) |
| `attitudePitch` | `int16_t` | 2 | Current estimated Pitch (deci-deg) |
| `attitudeYaw` | `int16_t` | 2 | Current estimated Yaw (deci-deg) |
| `osdHeader` | `uint8_t` | 1 | OSD RLE Header (255) |
| `osdRows` | `uint8_t` | 1 | (If OSD supported) Number of OSD rows |
| `osdCols` | `uint8_t` | 1 | (If OSD supported) Number of OSD columns |
| `osdStartY` | `uint8_t` | 1 | (If OSD supported) Starting row for RLE data |
| `osdStartX` | `uint8_t` | 1 | (If OSD supported) Starting column for RLE data |
| `osdRleData` | `uint8_t[]` | array | (If OSD supported) Run-length encoded OSD character data. Terminated by `[0, 0]` |

**Notes:** Requires `USE_SIMULATOR`. Complex message handling state changes for enabling/disabling HITL. Sensor data is injected directly. OSD data is sent using a custom RLE scheme. See `simulatorData` struct and associated code for details.

## <a id="msp2_inav_servo_mixer"></a>`MSP2_INAV_SERVO_MIXER (8224 / 0x2020)`
**Description:** Retrieves the custom servo mixer rules, including programming framework condition IDs, for primary and secondary mixer profiles. Supersedes `MSP_SERVO_MIX_RULES`.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `targetChannel` | `uint8_t` | 1 | - | Servo output channel index (0-based) |
| `inputSource` | `uint8_t` | 1 | [inputSource_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-inputsource_e) | Enum `inputSource_e` Input source |
| `rate` | `int16_t` | 2 | - | Mixing rate/weight |
| `speed` | `uint8_t` | 1 | - | Speed/Slew rate limit (0-100) |
| `conditionId` | `int8_t` | 1 | - | Logic Condition ID (0 to `MAX_LOGIC_CONDITIONS - 1`, or 255/-1 if none/disabled) |
| `p2TargetChannel` | `uint8_t` | 1 | - | (Optional) Profile 2 Target channel |
| `p2InputSource` | `uint8_t` | 1 | [inputSource_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-inputsource_e) | (Optional) Profile 2 Enum `inputSource_e` Input source |
| `p2Rate` | `int16_t` | 2 | - | (Optional) Profile 2 Rate |
| `p2Speed` | `uint8_t` | 1 | - | (Optional) Profile 2 Speed |
| `p2ConditionId` | `int8_t` | 1 | - | (Optional) Profile 2 Logic Condition ID |

**Notes:** `conditionId` requires `USE_PROGRAMMING_FRAMEWORK`.

## <a id="msp2_inav_set_servo_mixer"></a>`MSP2_INAV_SET_SERVO_MIXER (8225 / 0x2021)`
**Description:** Sets a single custom servo mixer rule, including programming framework condition ID. Supersedes `MSP_SET_SERVO_MIX_RULE`.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `ruleIndex` | `uint8_t` | 1 | - | Index of the rule to set (0 to `MAX_SERVO_RULES - 1`) |
| `targetChannel` | `uint8_t` | 1 | - | Servo output channel index |
| `inputSource` | `uint8_t` | 1 | [inputSource_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-inputsource_e) | Enum `inputSource_e` Input source |
| `rate` | `int16_t` | 2 | - | Mixing rate/weight |
| `speed` | `uint8_t` | 1 | - | Speed/Slew rate limit (0-100) |
| `conditionId` | `int8_t` | 1 | - | Logic Condition ID (255/-1 if none). Ignored if `USE_PROGRAMMING_FRAMEWORK` is disabled |

**Reply Payload:** **None**  

**Notes:** Expects 7 bytes. Returns error if index invalid. Calls `loadCustomServoMixer()`.

## <a id="msp2_inav_logic_conditions"></a>`MSP2_INAV_LOGIC_CONDITIONS (8226 / 0x2022)`
**Description:** Retrieves the configuration of all defined Logic Conditions. Requires `USE_PROGRAMMING_FRAMEWORK`. See `logicCondition_t` structure.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `enabled` | `uint8_t` | 1 | - | Boolean: 1 if the condition is enabled |
| `activatorId` | `int8_t` | 1 | - | Activator condition ID (-1/255 if none) |
| `operation` | `uint8_t` | 1 | [logicOperation_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-logicoperation_e) | Enum `logicOperation_e` Logical operation (AND, OR, XOR, etc.) |
| `operandAType` | `uint8_t` | 1 | [logicOperandType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-logicoperandtype_e) | Enum `logicOperandType_e` Type of the first operand (Flight Mode, GVAR, etc.) |
| `operandAValue` | `int32_t` | 4 | - | Value/ID of the first operand |
| `operandBType` | `uint8_t` | 1 | [logicOperandType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-logicoperandtype_e) | Enum `logicOperandType_e`: Type of the second operand |
| `operandBValue` | `int32_t` | 4 | - | Value/ID of the second operand |
| `flags` | `uint8_t` | 1 | Bitmask | Bitmask: Condition flags (`logicConditionFlags_e`) |

**Notes:** Deprecated, causes buffer overflow for 14*64 bytes

## <a id="msp2_inav_set_logic_conditions"></a>`MSP2_INAV_SET_LOGIC_CONDITIONS (8227 / 0x2023)`
**Description:** Sets the configuration for a single Logic Condition by its index.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `conditionIndex` | `uint8_t` | 1 | - | Index of the condition to set (0 to `MAX_LOGIC_CONDITIONS - 1`) |
| `enabled` | `uint8_t` | 1 | - | Boolean: 1 to enable the condition |
| `activatorId` | `int8_t` | 1 | - | Activator condition ID (-1/255 if none) |
| `operation` | `uint8_t` | 1 | [logicOperation_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-logicoperation_e) | Enum `logicOperation_e` Logical operation |
| `operandAType` | `uint8_t` | 1 | [logicOperandType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-logicoperandtype_e) | Enum `logicOperandType_e` Type of operand A |
| `operandAValue` | `int32_t` | 4 | - | Value/ID of operand A |
| `operandBType` | `uint8_t` | 1 | [logicOperandType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-logicoperandtype_e) | Enum `logicOperandType_e` Type of operand B |
| `operandBValue` | `int32_t` | 4 | - | Value/ID of operand B |
| `flags` | `uint8_t` | 1 | Bitmask | Bitmask: Condition flags (`logicConditionFlags_e`) |

**Reply Payload:** **None**  

**Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`. Expects 15 bytes. Returns error if index is invalid.

## <a id="msp2_inav_global_functions"></a>`MSP2_INAV_GLOBAL_FUNCTIONS (8228 / 0x2024)`

**Request Payload:** **None**  

**Reply Payload:** **None**  

## <a id="msp2_inav_set_global_functions"></a>`MSP2_INAV_SET_GLOBAL_FUNCTIONS (8229 / 0x2025)`

**Request Payload:** **None**  

**Reply Payload:** **None**  

## <a id="msp2_inav_logic_conditions_status"></a>`MSP2_INAV_LOGIC_CONDITIONS_STATUS (8230 / 0x2026)`
**Description:** Retrieves the current evaluated status (true/false or numerical value) of all logic conditions.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `conditionValues` | `int32_t[MAX_LOGIC_CONDITIONS]` | 256 (MAX_LOGIC_CONDITIONS) | Array of current values for each logic condition (`logicConditionGetValue(i)`). 1 for true, 0 for false, or numerical value depending on operation |

**Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`.

## <a id="msp2_inav_gvar_status"></a>`MSP2_INAV_GVAR_STATUS (8231 / 0x2027)`
**Description:** Retrieves the current values of all Global Variables (GVARS).  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `gvarValues` | `int32_t[MAX_GLOBAL_VARIABLES]` | 32 (MAX_GLOBAL_VARIABLES) | Array of current values for each global variable (`gvGet(i)`) |

**Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`.

## <a id="msp2_inav_programming_pid"></a>`MSP2_INAV_PROGRAMMING_PID (8232 / 0x2028)`
**Description:** Retrieves the configuration of all Programming PIDs.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `enabled` | `uint8_t` | 1 | - | Boolean: 1 if the PID is enabled |
| `setpointType` | `uint8_t` | 1 | [logicOperandType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-logicoperandtype_e) | Enum (`logicOperandType_e`) Type of the setpoint source |
| `setpointValue` | `int32_t` | 4 | - | Value/ID of the setpoint source |
| `measurementType` | `uint8_t` | 1 | [logicOperandType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-logicoperandtype_e) | Enum (`logicOperandType_e`) Type of the measurement source |
| `measurementValue` | `int32_t` | 4 | - | Value/ID of the measurement source |
| `gainP` | `uint16_t` | 2 | - | Proportional gain |
| `gainI` | `uint16_t` | 2 | - | Integral gain |
| `gainD` | `uint16_t` | 2 | - | Derivative gain |
| `gainFF` | `uint16_t` | 2 | - | Feed-forward gain |

**Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`. See `programmingPid_t` structure.

## <a id="msp2_inav_set_programming_pid"></a>`MSP2_INAV_SET_PROGRAMMING_PID (8233 / 0x2029)`
**Description:** Sets the configuration for a single Programming PID by its index.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `pidIndex` | `uint8_t` | 1 | - | Index of the Programming PID to set (0 to `MAX_PROGRAMMING_PID_COUNT - 1`) |
| `enabled` | `uint8_t` | 1 | - | Boolean: 1 to enable the PID |
| `setpointType` | `uint8_t` | 1 | [logicOperandType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-logicoperandtype_e) | Enum (`logicOperandType_e`) Type of the setpoint source |
| `setpointValue` | `int32_t` | 4 | - | Value/ID of the setpoint source |
| `measurementType` | `uint8_t` | 1 | [logicOperandType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-logicoperandtype_e) | Enum (`logicOperandType_e`) Type of the measurement source |
| `measurementValue` | `int32_t` | 4 | - | Value/ID of the measurement source |
| `gainP` | `uint16_t` | 2 | - | Proportional gain |
| `gainI` | `uint16_t` | 2 | - | Integral gain |
| `gainD` | `uint16_t` | 2 | - | Derivative gain |
| `gainFF` | `uint16_t` | 2 | - | Feed-forward gain |

**Reply Payload:** **None**  

**Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`. Expects 20 bytes. Returns error if index is invalid.

## <a id="msp2_inav_programming_pid_status"></a>`MSP2_INAV_PROGRAMMING_PID_STATUS (8234 / 0x202a)`
**Description:** Retrieves the current output value of all Programming PIDs.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `pidOutputs` | `int32_t[MAX_PROGRAMMING_PID_COUNT]` | 16 (MAX_PROGRAMMING_PID_COUNT) | Array of current output values for each Programming PID (`programmingPidGetOutput(i)`, signed) |

**Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`.

## <a id="msp2_pid"></a>`MSP2_PID (8240 / 0x2030)`
**Description:** Retrieves the standard PID controller gains (P, I, D, FF) for the current PID profile.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `P` | `uint8_t` | 1 | Proportional gain (`pidBank()->pid[i].P`), constrained 0-255 |
| `I` | `uint8_t` | 1 | Integral gain (`pidBank()->pid[i].I`), constrained 0-255 |
| `D` | `uint8_t` | 1 | Derivative gain (`pidBank()->pid[i].D`), constrained 0-255 |
| `FF` | `uint8_t` | 1 | Feed-forward gain (`pidBank()->pid[i].FF`), constrained 0-255 |

**Notes:** `PID_ITEM_COUNT` defines the number of standard PID controllers (Roll, Pitch, Yaw, Alt, Vel, etc.). Updates from EZ-Tune if enabled.

## <a id="msp2_set_pid"></a>`MSP2_SET_PID (8241 / 0x2031)`
**Description:** Sets the standard PID controller gains (P, I, D, FF) for the current PID profile.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `P` | `uint8_t` | 1 | Sets Proportional gain (`pidBankMutable()->pid[i].P`) |
| `I` | `uint8_t` | 1 | Sets Integral gain (`pidBankMutable()->pid[i].I`) |
| `D` | `uint8_t` | 1 | Sets Derivative gain (`pidBankMutable()->pid[i].D`) |
| `FF` | `uint8_t` | 1 | Sets Feed-forward gain (`pidBankMutable()->pid[i].FF`) |

**Reply Payload:** **None**  

**Notes:** Expects `PID_ITEM_COUNT * 4` bytes. Calls `schedulePidGainsUpdate()` and `navigationUsePIDs()`.

## <a id="msp2_inav_opflow_calibration"></a>`MSP2_INAV_OPFLOW_CALIBRATION (8242 / 0x2032)`
**Description:** Starts the optical flow sensor calibration procedure.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Requires `USE_OPFLOW`. Will fail if armed. Calls `opflowStartCalibration()`.

## <a id="msp2_inav_fwupdt_prepare"></a>`MSP2_INAV_FWUPDT_PREPARE (8243 / 0x2033)`
**Description:** Prepares the flight controller to receive a firmware update via MSP.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `firmwareSize` | `uint32_t` | 4 | Total size of the incoming firmware file in bytes |

**Reply Payload:** **None**  

**Notes:** Requires `MSP_FIRMWARE_UPDATE`. Expects 4 bytes. Returns error if preparation fails (e.g., no storage, invalid size). Calls `firmwareUpdatePrepare()`.

## <a id="msp2_inav_fwupdt_store"></a>`MSP2_INAV_FWUPDT_STORE (8244 / 0x2034)`
**Description:** Stores a chunk of firmware data received via MSP.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `firmwareChunk` | `uint8_t[]` | array | Chunk of firmware data |

**Reply Payload:** **None**  

**Notes:** Requires `MSP_FIRMWARE_UPDATE`. Returns error if storage fails (e.g., out of space, checksum error). Called repeatedly until the entire firmware is transferred. Calls `firmwareUpdateStore()`.

## <a id="msp2_inav_fwupdt_exec"></a>`MSP2_INAV_FWUPDT_EXEC (8245 / 0x2035)`
**Description:** Executes the firmware update process (flashes the stored firmware and reboots).  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `updateType` | `uint8_t` | 1 | Type of update (e.g., full flash, specific section - currently ignored/unused) |

**Reply Payload:** **None**  

**Notes:** Requires `MSP_FIRMWARE_UPDATE`. Expects 1 byte. Returns error if update cannot start (e.g., not fully received). Calls `firmwareUpdateExec()`. If successful, the device will reboot into the new firmware.

## <a id="msp2_inav_fwupdt_rollback_prepare"></a>`MSP2_INAV_FWUPDT_ROLLBACK_PREPARE (8246 / 0x2036)`
**Description:** Prepares the flight controller to perform a firmware rollback to the previously stored version.  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Requires `MSP_FIRMWARE_UPDATE`. Returns error if rollback preparation fails (e.g., no rollback image available). Calls `firmwareUpdateRollbackPrepare()`.

## <a id="msp2_inav_fwupdt_rollback_exec"></a>`MSP2_INAV_FWUPDT_ROLLBACK_EXEC (8247 / 0x2037)`
**Description:** Executes the firmware rollback process (flashes the stored backup firmware and reboots).  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Requires `MSP_FIRMWARE_UPDATE`. Returns error if rollback cannot start. Calls `firmwareUpdateRollbackExec()`. If successful, the device will reboot into the backup firmware.

## <a id="msp2_inav_safehome"></a>`MSP2_INAV_SAFEHOME (8248 / 0x2038)`
**Description:** Get or Set configuration for a specific Safe Home location.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `safehomeIndex` | `uint8_t` | 1 | Index of the safe home location (0 to `MAX_SAFE_HOMES - 1`) |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `safehomeIndex` | `uint8_t` | 1 | Index requested |
| `enabled` | `uint8_t` | 1 | Boolean: 1 if this safe home is enabled |
| `latitude` | `int32_t` | 4 | Latitude (1e7 deg) |
| `longitude` | `int32_t` | 4 | Longitude (1e7 deg) |

**Notes:** Requires `USE_SAFE_HOME`. Used by `mspFcSafeHomeOutCommand`. See `MSP2_INAV_SET_SAFEHOME` for setting.

## <a id="msp2_inav_set_safehome"></a>`MSP2_INAV_SET_SAFEHOME (8249 / 0x2039)`
**Description:** Sets the configuration for a specific Safe Home location.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `safehomeIndex` | `uint8_t` | 1 | Index of the safe home location (0 to `MAX_SAFE_HOMES - 1`) |
| `enabled` | `uint8_t` | 1 | Boolean: 1 to enable this safe home |
| `latitude` | `int32_t` | 4 | Latitude (1e7 deg) |
| `longitude` | `int32_t` | 4 | Longitude (1e7 deg) |

**Reply Payload:** **None**  

**Notes:** Requires `USE_SAFE_HOME`. Expects 10 bytes. Returns error if index invalid. Resets corresponding FW autoland approach if `USE_FW_AUTOLAND` is enabled.

## <a id="msp2_inav_misc2"></a>`MSP2_INAV_MISC2 (8250 / 0x203a)`
**Description:** Retrieves miscellaneous runtime information including timers and throttle status.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `uptimeSeconds` | `uint32_t` | 4 | Seconds | Time since boot (`micros() / 1000000`) |
| `flightTimeSeconds` | `uint32_t` | 4 | Seconds | Accumulated flight time (`getFlightTime()`) |
| `throttlePercent` | `uint8_t` | 1 | % | Current throttle output percentage (`getThrottlePercent(true)`) |
| `autoThrottleFlag` | `uint8_t` | 1 | Boolean | 1 if navigation is controlling throttle, 0 otherwise (`navigationIsControllingThrottle()`) |

## <a id="msp2_inav_logic_conditions_single"></a>`MSP2_INAV_LOGIC_CONDITIONS_SINGLE (8251 / 0x203b)`
**Description:** Gets the configuration for a single Logic Condition by its index.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `conditionIndex` | `uint8_t` | 1 | Index of the condition to retrieve (0 to `MAX_LOGIC_CONDITIONS - 1`) |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `enabled` | `uint8_t` | 1 | - | Boolean: 1 if enabled |
| `activatorId` | `int8_t` | 1 | - | Activator ID (-1/255 if none) |
| `operation` | `uint8_t` | 1 | [logicOperation_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-logicoperation_e) | Enum `logicOperation_e` Logical operation |
| `operandAType` | `uint8_t` | 1 | [logicOperandType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-logicoperandtype_e) | Enum `logicOperandType_e` Type of operand A |
| `operandAValue` | `int32_t` | 4 | - | Value/ID of operand A |
| `operandBType` | `uint8_t` | 1 | [logicOperandType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-logicoperandtype_e) | Enum `logicOperandType_e` Type of operand B |
| `operandBValue` | `int32_t` | 4 | - | Value/ID of operand B |
| `flags` | `uint8_t` | 1 | Bitmask | Bitmask: Condition flags (`logicConditionFlags_e`) |

**Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`. Used by `mspFcLogicConditionCommand`.

## <a id="msp2_inav_esc_rpm"></a>`MSP2_INAV_ESC_RPM (8256 / 0x2040)`
**Description:** Retrieves the RPM reported by each ESC via telemetry.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `escRpm` | `uint32_t` | 4 | RPM | RPM reported by the ESC |

**Notes:** Requires `USE_ESC_SENSOR`. Payload size depends on the number of detected motors with telemetry.

## <a id="msp2_inav_esc_telem"></a>`MSP2_INAV_ESC_TELEM (8257 / 0x2041)`
**Description:** Retrieves the full telemetry data structure reported by each ESC.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `motorCount` | `uint8_t` | 1 | Number of motors reporting telemetry (`getMotorCount()`) |
| `escData` | `escSensorData_t[]` | array | Array of `escSensorData_t` structures containing voltage, current, temp, RPM, errors etc. for each ESC |

**Notes:** Requires `USE_ESC_SENSOR`. See `escSensorData_t` in `sensors/esc_sensor.h` for the exact structure fields.

## <a id="msp2_inav_led_strip_config_ex"></a>`MSP2_INAV_LED_STRIP_CONFIG_EX (8264 / 0x2048)`
**Description:** Retrieves the full configuration for each LED on the strip using the `ledConfig_t` structure. Supersedes `MSP_LED_STRIP_CONFIG`.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `ledConfig` | `ledConfig_t` | - | Raw `ledConfig_t` structure (6 bytes) holding position, function, overlay, color, direction, and params bitfields (`io/ledstrip.h`). |

**Notes:** Requires `USE_LED_STRIP`. See `ledConfig_t` in `io/ledstrip.h` for structure fields (position, function, overlay, color, direction, params).

## <a id="msp2_inav_set_led_strip_config_ex"></a>`MSP2_INAV_SET_LED_STRIP_CONFIG_EX (8265 / 0x2049)`
**Description:** Sets the configuration for a single LED on the strip using the `ledConfig_t` structure. Supersedes `MSP_SET_LED_STRIP_CONFIG`.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `ledIndex` | `uint8_t` | 1 | Index of the LED to configure (0 to `LED_MAX_STRIP_LENGTH - 1`) |
| `ledConfig` | `ledConfig_t` | - | Raw `ledConfig_t` structure (6 bytes) mirroring the firmware layout. |

**Reply Payload:** **None**  

**Notes:** Requires `USE_LED_STRIP`. Expects `1 + sizeof(ledConfig_t)` bytes. Returns error if index invalid. Calls `reevaluateLedConfig()`.

## <a id="msp2_inav_fw_approach"></a>`MSP2_INAV_FW_APPROACH (8266 / 0x204a)`
**Description:** Get or Set configuration for a specific Fixed Wing Autoland approach.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `approachIndex` | `uint8_t` | 1 | Index of the approach setting (0 to `MAX_FW_LAND_APPOACH_SETTINGS - 1`) |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `approachIndex` | `uint8_t` | 1 | Index | Index requested |
| `approachAlt` | `int32_t` | 4 | cm | Signed altitude for the approach phase (`navFwAutolandApproach_t.approachAlt`) |
| `landAlt` | `int32_t` | 4 | cm | Signed altitude for the final landing phase (`navFwAutolandApproach_t.landAlt`) |
| `approachDirection` | `uint8_t` | 1 | [fwAutolandApproachDirection_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-fwautolandapproachdirection_e) | Enum `fwAutolandApproachDirection_e`: Direction of approach (From WP, Specific Heading) |
| `landHeading1` | `int16_t` | 2 | degrees | Primary landing heading (if approachDirection requires it) |
| `landHeading2` | `int16_t` | 2 | degrees | Secondary landing heading (if approachDirection requires it) |
| `isSeaLevelRef` | `uint8_t` | 1 | Boolean | 1 if altitudes are relative to sea level, 0 if relative to home |

**Notes:** Requires `USE_FW_AUTOLAND`. Used by `mspFwApproachOutCommand`. See `MSP2_INAV_SET_FW_APPROACH` for setting.

## <a id="msp2_inav_set_fw_approach"></a>`MSP2_INAV_SET_FW_APPROACH (8267 / 0x204b)`
**Description:** Sets the configuration for a specific Fixed Wing Autoland approach.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `approachIndex` | `uint8_t` | 1 | Index | Index of the approach setting (0 to `MAX_FW_LAND_APPOACH_SETTINGS - 1`) |
| `approachAlt` | `int32_t` | 4 | cm | Signed approach altitude (`navFwAutolandApproach_t.approachAlt`) |
| `landAlt` | `int32_t` | 4 | cm | Signed landing altitude (`navFwAutolandApproach_t.landAlt`) |
| `approachDirection` | `uint8_t` | 1 | [fwAutolandApproachDirection_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-fwautolandapproachdirection_e) | Enum `fwAutolandApproachDirection_e` Sets approach direction |
| `landHeading1` | `int16_t` | 2 | degrees | Sets primary landing heading |
| `landHeading2` | `int16_t` | 2 | degrees | Sets secondary landing heading |
| `isSeaLevelRef` | `uint8_t` | 1 | Boolean | Sets altitude reference |

**Reply Payload:** **None**  

**Notes:** Requires `USE_FW_AUTOLAND`. Expects 15 bytes. Returns error if index invalid.

## <a id="msp2_inav_gps_ublox_command"></a>`MSP2_INAV_GPS_UBLOX_COMMAND (8272 / 0x2050)`
**Description:** Sends a raw command directly to a U-Blox GPS module connected to the FC.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `ubxCommand` | `uint8_t[]` | array | Raw U-Blox UBX protocol command frame (including header, class, ID, length, payload, checksum) |

**Reply Payload:** **None**  

**Notes:** Requires GPS feature enabled (`FEATURE_GPS`) and the GPS driver to be U-Blox (`isGpsUblox()`). Payload must be at least 8 bytes (minimum UBX frame size). Use with extreme caution, incorrect commands can misconfigure the GPS module. Calls `gpsUbloxSendCommand()`.

## <a id="msp2_inav_rate_dynamics"></a>`MSP2_INAV_RATE_DYNAMICS (8288 / 0x2060)`
**Description:** Retrieves Rate Dynamics configuration parameters for the current control rate profile.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `sensitivityCenter` | `uint8_t` | 1 | % | Sensitivity at stick center (`currentControlRateProfile->rateDynamics.sensitivityCenter`) |
| `sensitivityEnd` | `uint8_t` | 1 | % | Sensitivity at stick ends (`currentControlRateProfile->rateDynamics.sensitivityEnd`) |
| `correctionCenter` | `uint8_t` | 1 | % | Correction strength at stick center (`currentControlRateProfile->rateDynamics.correctionCenter`) |
| `correctionEnd` | `uint8_t` | 1 | % | Correction strength at stick ends (`currentControlRateProfile->rateDynamics.correctionEnd`) |
| `weightCenter` | `uint8_t` | 1 | % | Transition weight at stick center (`currentControlRateProfile->rateDynamics.weightCenter`) |
| `weightEnd` | `uint8_t` | 1 | % | Transition weight at stick ends (`currentControlRateProfile->rateDynamics.weightEnd`) |

**Notes:** Requires `USE_RATE_DYNAMICS`.

## <a id="msp2_inav_set_rate_dynamics"></a>`MSP2_INAV_SET_RATE_DYNAMICS (8289 / 0x2061)`
**Description:** Sets Rate Dynamics configuration parameters for the current control rate profile.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `sensitivityCenter` | `uint8_t` | 1 | % | Sets sensitivity at center |
| `sensitivityEnd` | `uint8_t` | 1 | % | Sets sensitivity at ends |
| `correctionCenter` | `uint8_t` | 1 | % | Sets correction at center |
| `correctionEnd` | `uint8_t` | 1 | % | Sets correction at ends |
| `weightCenter` | `uint8_t` | 1 | % | Sets weight at center |
| `weightEnd` | `uint8_t` | 1 | % | Sets weight at ends |

**Reply Payload:** **None**  

**Notes:** Requires `USE_RATE_DYNAMICS`. Expects 6 bytes.

## <a id="msp2_inav_ez_tune"></a>`MSP2_INAV_EZ_TUNE (8304 / 0x2070)`
**Description:** Retrieves the current EZ-Tune parameters.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `enabled` | `uint8_t` | 1 | Boolean: 1 if EZ-Tune is enabled (`ezTune()->enabled`) |
| `filterHz` | `uint16_t` | 2 | Filter frequency used during tuning (`ezTune()->filterHz`) |
| `axisRatio` | `uint8_t` | 1 | Roll vs Pitch axis tuning ratio (`ezTune()->axisRatio`) |
| `response` | `uint8_t` | 1 | Desired response characteristic (`ezTune()->response`) |
| `damping` | `uint8_t` | 1 | Desired damping characteristic (`ezTune()->damping`) |
| `stability` | `uint8_t` | 1 | Stability preference (`ezTune()->stability`) |
| `aggressiveness` | `uint8_t` | 1 | Aggressiveness preference (`ezTune()->aggressiveness`) |
| `rate` | `uint8_t` | 1 | Resulting rate setting (`ezTune()->rate`) |
| `expo` | `uint8_t` | 1 | Resulting expo setting (`ezTune()->expo`) |
| `snappiness` | `uint8_t` | 1 | Snappiness preference (`ezTune()->snappiness`) |

**Notes:** Requires `USE_EZ_TUNE`. Calls `ezTuneUpdate()` before sending.

## <a id="msp2_inav_ez_tune_set"></a>`MSP2_INAV_EZ_TUNE_SET (8305 / 0x2071)`
**Description:** Sets the EZ-Tune parameters and triggers an update.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `enabled` | `uint8_t` | 1 | Sets enabled state |
| `filterHz` | `uint16_t` | 2 | Sets filter frequency |
| `axisRatio` | `uint8_t` | 1 | Sets axis ratio |
| `response` | `uint8_t` | 1 | Sets response characteristic |
| `damping` | `uint8_t` | 1 | Sets damping characteristic |
| `stability` | `uint8_t` | 1 | Sets stability preference |
| `aggressiveness` | `uint8_t` | 1 | Sets aggressiveness preference |
| `rate` | `uint8_t` | 1 | Sets rate setting |
| `expo` | `uint8_t` | 1 | Sets expo setting |
| `snappiness` | `uint8_t` | 1 | (Optional) Sets snappiness preference |

**Reply Payload:** **None**  

**Notes:** Requires `USE_EZ_TUNE`. Expects 10 or 11 bytes. Calls `ezTuneUpdate()` after setting parameters.

## <a id="msp2_inav_select_mixer_profile"></a>`MSP2_INAV_SELECT_MIXER_PROFILE (8320 / 0x2080)`
**Description:** Selects the active mixer profile and saves configuration.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `mixerProfileIndex` | `uint8_t` | 1 | Index of the mixer profile to activate (0-based) |

**Reply Payload:** **None**  

**Notes:** Expects 1 byte. Will fail if armed. Calls `setConfigMixerProfileAndWriteEEPROM()`. Only applicable if `MAX_MIXER_PROFILE_COUNT` > 1.

## <a id="msp2_adsb_vehicle_list"></a>`MSP2_ADSB_VEHICLE_LIST (8336 / 0x2090)`
**Description:** Retrieves the list of currently tracked ADSB (Automatic Dependent Surveillance–Broadcast) vehicles. See `adsbVehicle_t` and `adsbVehicleValues_t` in `io/adsb.h` for the exact structure fields.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Repeats|Size (Bytes)|Units|Description|
|---|---|---|---|---|---|
| `maxVehicles` | `uint8_t` | - | 1 | - | Maximum number of vehicles tracked (`MAX_ADSB_VEHICLES`). 0 if `USE_ADSB` disabled |
| `callsignLength` | `uint8_t` | - | 1 | - | Maximum length of callsign string (`ADSB_CALL_SIGN_MAX_LENGTH`). 0 if `USE_ADSB` disabled |
| `totalVehicleMsgs` | `uint32_t` | - | 4 | - | Total vehicle messages received (`getAdsbStatus()->vehiclesMessagesTotal`). 0 if `USE_ADSB` disabled |
| `totalHeartbeatMsgs` | `uint32_t` | - | 4 | - | Total heartbeat messages received (`getAdsbStatus()->heartbeatMessagesTotal`). 0 if `USE_ADSB` disabled |
| `callsign` | `char[ADSB_CALL_SIGN_MAX_LENGTH]` | maxVehicles | 9 (ADSB_CALL_SIGN_MAX_LENGTH) | - | Fixed-length callsign from `adsbVehicle->vehicleValues.callsign` (padded with NULs if shorter). |
| `icao` | `uint32_t` | maxVehicles | 4 | - | ICAO address (`adsbVehicle->vehicleValues.icao`). |
| `lat` | `int32_t` | maxVehicles | 4 | 1e-7 deg | Latitude in degrees * 1e7 (`adsbVehicle->vehicleValues.lat`). |
| `lon` | `int32_t` | maxVehicles | 4 | 1e-7 deg | Longitude in degrees * 1e7 (`adsbVehicle->vehicleValues.lon`). |
| `alt` | `int32_t` | maxVehicles | 4 | cm | Altitude above sea level (`adsbVehicle->vehicleValues.alt`). |
| `headingDeg` | `uint16_t` | maxVehicles | 2 | deg | Course over ground in whole degrees (`CENTIDEGREES_TO_DEGREES(vehicleValues.heading)`). |
| `tslc` | `uint8_t` | maxVehicles | 1 | s | Time since last communication (`adsbVehicle->vehicleValues.tslc`). |
| `emitterType` | `uint8_t` | maxVehicles | 1 | - | Emitter category (`adsbVehicle->vehicleValues.emitterType`) (refers to enum 'ADSB_EMITTER_TYPE', but none found) |
| `ttl` | `uint8_t` | maxVehicles | 1 | - | TTL counter used for list maintenance (`adsbVehicle->ttl`). |

**Notes:** Requires `USE_ADSB`. Only a subset of `adsbVehicle_t` is transmitted (callsign, core values, heading in whole degrees, TSLC, emitter type, TTL).

## <a id="msp2_inav_custom_osd_elements"></a>`MSP2_INAV_CUSTOM_OSD_ELEMENTS (8448 / 0x2100)`
**Description:** Retrieves counts related to custom OSD elements defined by the programming framework.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `maxElements` | `uint8_t` | 1 | Maximum number of custom elements (`MAX_CUSTOM_ELEMENTS`) |
| `maxTextLength` | `uint8_t` | 1 | Maximum length of the text part (`OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1`) |
| `maxParts` | `uint8_t` | 1 | Maximum number of parts per element (`CUSTOM_ELEMENTS_PARTS`) |

**Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`.

## <a id="msp2_inav_custom_osd_element"></a>`MSP2_INAV_CUSTOM_OSD_ELEMENT (8449 / 0x2101)`
**Description:** Gets the configuration of a single custom OSD element defined by the programming framework.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `elementIndex` | `uint8_t` | 1 | Index of the custom element (0 to `MAX_CUSTOM_ELEMENTS - 1`) |
  
**Reply Payload:**
|Field|C Type|Repeats|Size (Bytes)|Units|Description|
|---|---|---|---|---|---|
| `partType` | `uint8_t` | CUSTOM_ELEMENTS_PARTS | 1 | [osdCustomElementType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osdcustomelementtype_e) | Type of this part |
| `partValue` | `uint16_t` | CUSTOM_ELEMENTS_PARTS | 2 | - | Value/ID associated with this part |
| `visibilityType` | `uint8_t` | - | 1 | [osdCustomElementTypeVisibility_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osdcustomelementtypevisibility_e) | Visibility condition source |
| `visibilityValue` | `uint16_t` | - | 2 | - | Value/ID of the visibility condition source |
| `elementText` | `char[OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1]` | - | OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1 | - | Static text bytes |

**Notes:** Reply emitted only if idx < MAX_CUSTOM_ELEMENTS; otherwise no body is written.

## <a id="msp2_inav_set_custom_osd_elements"></a>`MSP2_INAV_SET_CUSTOM_OSD_ELEMENTS (8450 / 0x2102)`
**Description:** Sets the configuration of one custom OSD element.  
  
**Request Payload:**
|Field|C Type|Repeats|Size (Bytes)|Units|Description|
|---|---|---|---|---|---|
| `elementIndex` | `uint8_t` | - | 1 | - | Index of the custom element (0 to `MAX_CUSTOM_ELEMENTS - 1`) |
| `partType` | `uint8_t` | CUSTOM_ELEMENTS_PARTS | 1 | [osdCustomElementType_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osdcustomelementtype_e) | Type of this part |
| `partValue` | `uint16_t` | CUSTOM_ELEMENTS_PARTS | 2 | - | Value/ID associated with this part |
| `visibilityType` | `uint8_t` | - | 1 | [osdCustomElementTypeVisibility_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-osdcustomelementtypevisibility_e) | Visibility condition source |
| `visibilityValue` | `uint16_t` | - | 2 | - | Value/ID of the visibility condition source |
| `elementText` | `char[OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1]` | - | OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1 | - | Raw bytes |

**Reply Payload:** **None**  

**Notes:** Payload length must be (OSD_CUSTOM_ELEMENT_TEXT_SIZE - 1) + (CUSTOM_ELEMENTS_PARTS * 3) + 4 bytes including elementIndex. elementIndex must be < MAX_CUSTOM_ELEMENTS. Each partType must be < CUSTOM_ELEMENT_TYPE_END. Firmware NUL-terminates elementText internally.

## <a id="msp2_inav_output_mapping_ext2"></a>`MSP2_INAV_OUTPUT_MAPPING_EXT2 (8461 / 0x210d)`
**Description:** Retrieves the full extended output mapping configuration (timer ID, full 32-bit usage flags, and pin label). Supersedes `MSP2_INAV_OUTPUT_MAPPING_EXT`.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `timerId` | `uint8_t` | 1 | - | Hardware timer identifier (e.g., `TIM1`, `TIM2`). SITL uses index |
| `usageFlags` | `uint32_t` | 4 | - | Full 32-bit timer usage flags (`TIM_USE_*`) |
| `pinLabel` | `uint8_t` | 1 | [pinLabel_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-pinlabel_e) | Label for special pin usage (`PIN_LABEL_*` enum, e.g., `PIN_LABEL_LED`). 0 (`PIN_LABEL_NONE`) otherwise |

**Notes:** Provides complete usage flags and helps identify pins repurposed for functions like LED strip.

## <a id="msp2_inav_servo_config"></a>`MSP2_INAV_SERVO_CONFIG (8704 / 0x2200)`
**Description:** Retrieves the configuration parameters for all supported servos (min, max, middle, rate). Supersedes `MSP_SERVO_CONFIGURATIONS`.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `min` | `int16_t` | 2 | PWM | Minimum servo endpoint (`servoParams(i)->min`) |
| `max` | `int16_t` | 2 | PWM | Maximum servo endpoint (`servoParams(i)->max`) |
| `middle` | `int16_t` | 2 | PWM | Middle/Neutral servo position (`servoParams(i)->middle`) |
| `rate` | `int8_t` | 1 | % (-125 to 125) | Servo rate/scaling (`servoParams(i)->rate`) |

## <a id="msp2_inav_set_servo_config"></a>`MSP2_INAV_SET_SERVO_CONFIG (8705 / 0x2201)`
**Description:** Sets the configuration parameters for a single servo. Supersedes `MSP_SET_SERVO_CONFIGURATION`.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `servoIndex` | `uint8_t` | 1 | Index | Index of the servo to configure (0 to `MAX_SUPPORTED_SERVOS - 1`) |
| `min` | `int16_t` | 2 | PWM | Sets minimum servo endpoint |
| `max` | `int16_t` | 2 | PWM | Sets maximum servo endpoint |
| `middle` | `int16_t` | 2 | PWM | Sets middle/neutral servo position |
| `rate` | `int8_t` | 1 | % (-125 to 125) | Sets servo rate/scaling |

**Reply Payload:** **None**  

**Notes:** Expects 8 bytes. Returns error if index invalid. Calls `servoComputeScalingFactors()`.

## <a id="msp2_inav_geozone"></a>`MSP2_INAV_GEOZONE (8720 / 0x2210)`
**Description:** Get configuration for a specific Geozone.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `geozoneIndex` | `uint8_t` | 1 | Index of the geozone (0 to `MAX_GEOZONES_IN_CONFIG - 1`) |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `geozoneIndex` | `uint8_t` | 1 | - | Index requested |
| `type` | `uint8_t` | 1 | - | Define (`GEOZONE_TYPE_EXCLUSIVE/INCLUSIVE`): Zone type (Inclusion/Exclusion) |
| `shape` | `uint8_t` | 1 | - | Define (`GEOZONE_SHAPE_CIRCULAR/POLYGON`): Zone shape (Polygon/Circular) |
| `minAltitude` | `int32_t` | 4 | cm | Minimum allowed altitude within the zone (`geoZonesConfig(idx)->minAltitude`) |
| `maxAltitude` | `int32_t` | 4 | cm | Maximum allowed altitude within the zone (`geoZonesConfig(idx)->maxAltitude`) |
| `isSeaLevelRef` | `uint8_t` | 1 | - | Boolean: 1 if altitudes are relative to sea level, 0 if relative to home |
| `fenceAction` | `uint8_t` | 1 | [fenceAction_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-fenceaction_e) | Enum (`fenceAction_e`): Action to take upon boundary violation |
| `vertexCount` | `uint8_t` | 1 | - | Number of vertices defined for this zone |

**Notes:** Requires `USE_GEOZONE`. Used by `mspFcGeozoneOutCommand`.

## <a id="msp2_inav_set_geozone"></a>`MSP2_INAV_SET_GEOZONE (8721 / 0x2211)`
**Description:** Sets the main configuration for a specific Geozone (type, shape, altitude, action). **This command resets (clears) all vertices associated with the zone.**  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `geozoneIndex` | `uint8_t` | 1 | - | Index of the geozone (0 to `MAX_GEOZONES_IN_CONFIG - 1`) |
| `type` | `uint8_t` | 1 | - | Define (`GEOZONE_TYPE_EXCLUSIVE/INCLUSIVE`): Zone type (Inclusion/Exclusion) |
| `shape` | `uint8_t` | 1 | - | Define (`GEOZONE_SHAPE_CIRCULAR/POLYGON`): Zone shape (Polygon/Circular) |
| `minAltitude` | `int32_t` | 4 | cm | Minimum allowed altitude (`geoZonesConfigMutable()->minAltitude`) |
| `maxAltitude` | `int32_t` | 4 | cm | Maximum allowed altitude (`geoZonesConfigMutable()->maxAltitude`) |
| `isSeaLevelRef` | `uint8_t` | 1 | - | Boolean: Altitude reference |
| `fenceAction` | `uint8_t` | 1 | [fenceAction_e](https://github.com/iNavFlight/inav/wiki/Enums-reference#enum-fenceaction_e) | Enum (`fenceAction_e`): Action to take upon boundary violation |
| `vertexCount` | `uint8_t` | 1 | - | Number of vertices to be defined (used for validation later) |

**Reply Payload:** **None**  

**Notes:** Requires `USE_GEOZONE`. Expects 14 bytes. Returns error if index invalid. Calls `geozoneResetVertices()`. Vertices must be set subsequently using `MSP2_INAV_SET_GEOZONE_VERTEX`.

## <a id="msp2_inav_geozone_vertex"></a>`MSP2_INAV_GEOZONE_VERTEX (8722 / 0x2212)`
**Description:** Get a specific vertex (or center+radius for circular zones) of a Geozone.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Description|
|---|---|---|---|
| `geozoneIndex` | `uint8_t` | 1 | Index of the geozone |
| `vertexId` | `uint8_t` | 1 | Index of the vertex within the zone (0-based). For circles, 0 = center |
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `geozoneIndex` | `uint8_t` | 1 | Index | Geozone index requested |
| `vertexId` | `uint8_t` | 1 | Index | Vertex index requested |
| `latitude` | `int32_t` | 4 | deg * 1e7 | Vertex latitude |
| `longitude` | `int32_t` | 4 | deg * 1e7 | Vertex longitude |
| `radius` | `int32_t` | 4 | cm | If vertex is circle, Radius of the circular zone |

**Notes:** Requires `USE_GEOZONE`. Returns error if indexes are invalid or vertex doesn't exist. For circular zones, the radius is stored internally as the 'latitude' of the vertex with index 1.

## <a id="msp2_inav_set_geozone_vertex"></a>`MSP2_INAV_SET_GEOZONE_VERTEX (8723 / 0x2213)`
**Description:** Sets a specific vertex (or center+radius for circular zones) for a Geozone.  
#### Variant: `polygon`

**Description:** Polygonal Geozone  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `geozoneIndex` | `uint8_t` | 1 | Index | Geozone index requested |
| `vertexId` | `uint8_t` | 1 | Index | Vertex index requested |
| `latitude` | `int32_t` | 4 | deg * 1e7 | Vertex latitude |
| `longitude` | `int32_t` | 4 | deg * 1e7 | Vertex longitude |

**Reply Payload:** **None**  

#### Variant: `circle`

**Description:** Circular Geozone  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `geozoneIndex` | `uint8_t` | 1 | Index | Geozone index requested |
| `vertexId` | `uint8_t` | 1 | Index | Vertex index requested |
| `latitude` | `int32_t` | 4 | deg * 1e7 | Vertex/Center latitude |
| `longitude` | `int32_t` | 4 | deg * 1e7 | Vertex/Center longitude |
| `radius` | `int32_t` | 4 | cm | Radius of the circular zone |

**Reply Payload:** **None**  


**Notes:** Requires `USE_GEOZONE`. Expects 10 bytes (Polygon) or 14 bytes (Circular). Returns error if indexes invalid or if trying to set vertex beyond `vertexCount` defined in `MSP2_INAV_SET_GEOZONE`. Calls `geozoneSetVertex()`. For circular zones, sets center (vertex 0) and radius (vertex 1's latitude).

## <a id="msp2_inav_set_gvar"></a>`MSP2_INAV_SET_GVAR (8724 / 0x2214)`
**Description:** Sets the specified Global Variable (GVAR) to the provided value.  
  
**Request Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `gvarIndex` | `uint8_t` | 1 | Index | Index of the Global Variable to set |
| `value` | `int32_t` | 4 | - | New value to store (clamped to configured min/max by `gvSet()`) |

**Reply Payload:** **None**  

**Notes:** Requires `USE_PROGRAMMING_FRAMEWORK`. Expects 5 bytes. Returns error if index is outside `MAX_GLOBAL_VARIABLES`.

## <a id="msp2_inav_full_local_pose"></a>`MSP2_INAV_FULL_LOCAL_POSE (8736 / 0x2220)`
**Description:** Provides estimates of current attitude, local NEU position, and velocity.  

**Request Payload:** **None**  
  
**Reply Payload:**
|Field|C Type|Size (Bytes)|Units|Description|
|---|---|---|---|---|
| `roll` | `int16_t` | 2 | deci-degrees | Roll angle (`attitude.values.roll`) |
| `pitch` | `int16_t` | 2 | deci-degrees | Pitch angle (`attitude.values.pitch`) |
| `yaw` | `int16_t` | 2 | deci-degrees | Yaw/Heading angle (`attitude.values.yaw`) |
| `localPositionNorth` | `int32_t` | 4 | cm | Estimated North coordinate in local NEU frame (`posControl.actualState.abs.pos.x`) |
| `localVelocityNorth` | `int16_t` | 2 | cm/s | Estimated North component of velocity in local NEU frame (`posControl.actualState.abs.vel.x`) |
| `localPositionEast` | `int32_t` | 4 | cm | Estimated East coordinate in local NEU frame (`posControl.actualState.abs.pos.y`) |
| `localVelocityEast` | `int16_t` | 2 | cm/s | Estimated East component of velocity in local NEU frame (`posControl.actualState.abs.vel.y`) |
| `localPositionUp` | `int32_t` | 4 | cm | Estimated Up coordinate in local NEU frame (`posControl.actualState.abs.pos.z`) |
| `localVelocityUp` | `int16_t` | 2 | cm/s | Estimated Up component of velocity in local NEU frame (`posControl.actualState.abs.vel.z`) |

**Notes:** All attitude angles are in deci-degrees.

## <a id="msp2_betaflight_bind"></a>`MSP2_BETAFLIGHT_BIND (12288 / 0x3000)`
**Description:** Initiates the receiver binding procedure for supported serial protocols (CRSF, SRXL2).  

**Request Payload:** **None**  

**Reply Payload:** **None**  

**Notes:** Requires `rxConfig()->receiverType == RX_TYPE_SERIAL`. Requires `USE_SERIALRX_CRSF` or `USE_SERIALRX_SRXL2`. Calls `crsfBind()` or `srxl2Bind()` respectively. Returns error if receiver type or provider is not supported for binding.

## <a id="msp2_rx_bind"></a>`MSP2_RX_BIND (12289 / 0x3001)`
**Description:** Initiates binding for MSP receivers (mLRS).

**Request Payload:** **None**

**Reply Payload:** **None**

**Notes:** Requires a receiver using MSP as the protocol, sends MSP2_RX_BIND to the receiver.

