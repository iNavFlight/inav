# On Screen Display

The On Screen Display, or OSD, is a feature that overlays flight data over the video image. This can be done on the flight controller, using the analogue MAX7456 chip. Digital systems take the OSD data, via MSP DisplayPort, send it to the video receiver; which combines the data with the image. You can specify what elements are displayed, and their locations on the image. Most systems are character based, and use the MAX7456 analogue setup, or MSP DisplayPort. However, there are some different systems which are also supported. Such as the canvas based FrSKY PixelOSD on analogue. Canvas OSDs draw shapes on the image. Whereas character based OSDs use font characters to display the data.

## Features and Limitations
Not all OSDs are created equally. This table shows the differences between the different systems available.

| OSD System    | Character grid | Character | Canvas | MSP DisplayPort | All elements supported  |
|---------------|----------------|-----------|--------|-----------------|-------------------------|
| Analogue PAL  | 30 x 16        | X         |        |                 | YES                     |
| Analogue NTSC | 30 x 13        | X         |        |                 | YES                     |
| PixelOSD      | As PAL or NTSC |           | X      |                 | YES                     |
| DJI OSD       | 30 x 16        | X         |        |                 | NO - BF Characters only |
| DJI WTFOS     | 60 x 22        | X         |        | X               | YES                     |
| HDZero        | 50 x 18        | X         |        | X               | YES                     |
| Avatar        | 53 x 20        | X         |        | X               | YES                     |
| DJI O3        | 53 x 20 (HD)   | X         |        | X	(partial)     | NO - BF Characters only |

## OSD Elements
Here are the OSD Elements provided by INAV.

| ID  | Element                                          | Added  | Notes |
|-----|--------------------------------------------------|--------|-------|
| 0   | OSD_RSSI_VALUE                                   | 1.0.0  |       |
| 1   | OSD_MAIN_BATT_VOLTAGE                            | 1.0.0  |       |
| 2   | OSD_CROSSHAIRS                                   | 1.0.0  |       |
| 3   | OSD_ARTIFICIAL_HORIZON                           | 1.0.0  |       |
| 4   | OSD_HORIZON_SIDEBARS                             | 1.0.0  |       |
| 5   | OSD_ONTIME                                       | 1.0.0  |       |
| 6   | OSD_FLYTIME                                      | 1.0.0  |       |
| 7   | OSD_FLYMODE                                      | 1.0.0  |       |
| 8   | OSD_CRAFT_NAME                                   | 1.0.0  |       |
| 9   | OSD_THROTTLE_POS                                 | 1.0.0  |       |
| 10  | OSD_VTX_CHANNEL                                  | 1.0.0  |       |
| 11  | OSD_CURRENT_DRAW                                 | 1.0.0  |       |
| 12  | OSD_MAH_DRAWN                                    | 1.0.0  |       |
| 13  | OSD_GPS_SPEED                                    | 1.0.0  |       |
| 14  | OSD_GPS_SATS                                     | 1.0.0  |       |
| 15  | OSD_ALTITUDE                                     | 1.0.0  |       |
| 16  | OSD_ROLL_PIDS                                    | 1.6.0  |       |
| 17  | OSD_PITCH_PIDS                                   | 1.6.0  |       |
| 18  | OSD_YAW_PIDS                                     | 1.6.0  |       |
| 19  | OSD_POWER                                        | 1.6.0  |       |
| 20  | OSD_GPS_LON                                      | 1.6.0  |       |
| 21  | OSD_GPS_LAT                                      | 1.6.0  |       |
| 22  | OSD_HOME_DIR                                     | 1.6.0  |       |
| 23  | OSD_HOME_DIST                                    | 1.6.0  |       |
| 24  | OSD_HEADING                                      | 1.6.0  |       |
| 25  | OSD_VARIO                                        | 1.6.0  |       |
| 26  | OSD_VARIO_NUM                                    | 1.6.0  |       |
| 27  | OSD_AIR_SPEED                                    | 1.7.3  |       |
| 28  | OSD_ONTIME_FLYTIME                               | 1.8.0  |       |
| 29  | OSD_RTC_TIME                                     | 1.8.0  |       |
| 30  | OSD_MESSAGES                                     | 1.8.0  |       |
| 31  | OSD_GPS_HDOP                                     | 1.8.0  |       |
| 32  | OSD_MAIN_BATT_CELL_VOLTAGE                       | 1.8.0  |       |
| 33  | OSD_SCALED_THROTTLE_POS                          | 1.8.0  |       |
| 34  | OSD_HEADING_GRAPH                                | 1.8.0  |       |
| 35  | OSD_EFFICIENCY_MAH_PER_KM                        | 1.9.0  |       |
| 36  | OSD_WH_DRAWN                                     | 1.9.0  |       |
| 37  | OSD_BATTERY_REMAINING_CAPACITY                   | 1.9.0  |       |
| 38  | OSD_BATTERY_REMAINING_PERCENT                    | 1.9.0  |       |
| 39  | OSD_EFFICIENCY_WH_PER_KM                         | 1.9.0  |       |
| 40  | OSD_TRIP_DIST                                    | 1.9.1  |       |
| 41  | OSD_ATTITUDE_PITCH                               | 2.0.0  |       |
| 42  | OSD_ATTITUDE_ROLL                                | 2.0.0  |       |
| 43  | OSD_MAP_NORTH                                    | 2.0.0  |       |
| 44  | OSD_MAP_TAKEOFF                                  | 2.0.0  |       |
| 45  | OSD_RADAR                                        | 2.0.0  |       |
| 46  | OSD_WIND_SPEED_HORIZONTAL                        | 2.0.0  |       |
| 47  | OSD_WIND_SPEED_VERTICAL                          | 2.0.0  |       |
| 48  | OSD_REMAINING_FLIGHT_TIME_BEFORE_RTH             | 2.0.0  |       |
| 49  | OSD_REMAINING_DISTANCE_BEFORE_RTH                | 2.0.0  |       |
| 50  | OSD_HOME_HEADING_ERROR                           | 2.0.0  |       |
| 51  | OSD_COURSE_HOLD_ERROR                            | 2.0.0  |       |
| 52  | OSD_COURSE_HOLD_ADJUSTMENT                       | 2.0.0  |       |
| 53  | OSD_SAG_COMPENSATED_MAIN_BATT_VOLTAGE            | 2.0.0  |       |
| 54  | OSD_MAIN_BATT_SAG_COMPENSATED_CELL_VOLTAGE       | 2.0.0  |       |
| 55  | OSD_POWER_SUPPLY_IMPEDANCE                       | 2.0.0  |       |
| 56  | OSD_LEVEL_PIDS                                   | 2.0.0  |       |
| 57  | OSD_POS_XY_PIDS                                  | 2.0.0  |       |
| 58  | OSD_POS_Z_PIDS                                   | 2.0.0  |       |
| 59  | OSD_VEL_XY_PIDS                                  | 2.0.0  |       |
| 60  | OSD_VEL_Z_PIDS                                   | 2.0.0  |       |
| 61  | OSD_HEADING_P                                    | 2.0.0  |       |
| 62  | OSD_BOARD_ALIGN_ROLL                             | 2.0.0  |       |
| 63  | OSD_BOARD_ALIGN_PITCH                            | 2.0.0  |       |
| 64  | OSD_RC_EXPO                                      | 2.0.0  |       |
| 65  | OSD_RC_YAW_EXPO                                  | 2.0.0  |       |
| 66  | OSD_THROTTLE_EXPO                                | 2.0.0  |       |
| 67  | OSD_PITCH_RATE                                   | 2.0.0  |       |
| 68  | OSD_ROLL_RATE                                    | 2.0.0  |       |
| 69  | OSD_YAW_RATE                                     | 2.0.0  |       |
| 70  | OSD_MANUAL_RC_EXPO                               | 2.0.0  |       |
| 71  | OSD_MANUAL_RC_YAW_EXPO                           | 2.0.0  |       |
| 72  | OSD_MANUAL_PITCH_RATE                            | 2.0.0  |       |
| 73  | OSD_MANUAL_ROLL_RATE                             | 2.0.0  |       |
| 74  | OSD_MANUAL_YAW_RATE                              | 2.0.0  |       |
| 75  | OSD_NAV_FW_CRUISE_THR                            | 2.0.0  |       |
| 76  | OSD_NAV_FW_PITCH2THR                             | 2.0.0  |       |
| 77  | OSD_FW_MIN_THROTTLE_DOWN_PITCH_ANGLE             | 2.0.0  |       |
| 78  | OSD_DEBUG                                        | 2.0.0  |       |
| 79  | OSD_FW_ALT_PID_OUTPUTS                           | 2.0.0  |       |
| 80  | OSD_FW_POS_PID_OUTPUTS                           | 2.0.0  |       |
| 81  | OSD_MC_VEL_X_PID_OUTPUTS                         | 2.0.0  |       |
| 82  | OSD_MC_VEL_Y_PID_OUTPUTS                         | 2.0.0  |       |
| 83  | OSD_MC_VEL_Z_PID_OUTPUTS                         | 2.0.0  |       |
| 84  | OSD_MC_POS_XYZ_P_OUTPUTS                         | 2.0.0  |       |
| 85  | OSD_3D_SPEED                                     | 2.1.0  |       |
| 86  | OSD_IMU_TEMPERATURE                              | 2.1.0  |       |
| 87  | OSD_BARO_TEMPERATURE                             | 2.1.0  |       |
| 88  | OSD_TEMP_SENSOR_0_TEMPERATURE                    | 2.1.0  |       |
| 89  | OSD_TEMP_SENSOR_1_TEMPERATURE                    | 2.1.0  |       |
| 90  | OSD_TEMP_SENSOR_2_TEMPERATURE                    | 2.1.0  |       |
| 91  | OSD_TEMP_SENSOR_3_TEMPERATURE                    | 2.1.0  |       |
| 92  | OSD_TEMP_SENSOR_4_TEMPERATURE                    | 2.1.0  |       |
| 93  | OSD_TEMP_SENSOR_5_TEMPERATURE                    | 2.1.0  |       |
| 94  | OSD_TEMP_SENSOR_6_TEMPERATURE                    | 2.1.0  |       |
| 95  | OSD_TEMP_SENSOR_7_TEMPERATURE                    | 2.1.0  |       |
| 96  | OSD_ALTITUDE_MSL                                 | 2.1.0  |       |
| 97  | OSD_PLUS_CODE                                    | 2.1.0  |       |
| 98  | OSD_MAP_SCALE                                    | 2.2.0  |       |
| 99  | OSD_MAP_REFERENCE                                | 2.2.0  |       |
| 100 | OSD_GFORCE                                       | 2.2.0  |       |
| 101 | OSD_GFORCE_X                                     | 2.2.0  |       |
| 102 | OSD_GFORCE_Y                                     | 2.2.0  |       |
| 103 | OSD_GFORCE_Z                                     | 2.2.0  |       |
| 104 | OSD_RC_SOURCE                                    | 2.2.0  |       |
| 105 | OSD_VTX_POWER                                    | 2.2.0  |       |
| 106 | OSD_ESC_RPM                                      | 2.3.0  |       |
| 107 | OSD_ESC_TEMPERATURE                              | 2.5.0  |       |
| 108 | OSD_AZIMUTH                                      | 2.6.0  |       |
| 109 | OSD_CRSF_RSSI_DBM                                | 2.6.0  |       |
| 110 | OSD_CRSF_LQ                                      | 2.6.0  |       |
| 111 | OSD_CRSF_SNR_DB                                  | 2.6.0  |       |
| 112 | OSD_CRSF_TX_POWER                                | 2.6.0  |       |
| 113 | OSD_GVAR_0                                       | 2.6.0  |       |
| 114 | OSD_GVAR_1                                       | 2.6.0  |       |
| 115 | OSD_GVAR_2                                       | 2.6.0  |       |
| 116 | OSD_GVAR_3                                       | 2.6.0  |       |
| 117 | OSD_TPA                                          | 2.6.0  |       |
| 118 | OSD_NAV_FW_CONTROL_SMOOTHNESS                    | 2.6.0  |       |
| 119 | OSD_VERSION                                      | 3.0.0  |       |
| 120 | OSD_RANGEFINDER                                  | 3.0.0  |       |
| 121 | OSD_PLIMIT_REMAINING_BURST_TIME                  | 3.0.0  |       |
| 122 | OSD_PLIMIT_ACTIVE_CURRENT_LIMIT                  | 3.0.0  |       |
| 123 | OSD_PLIMIT_ACTIVE_POWER_LIMIT                    | 3.0.0  |       |
| 124 | OSD_GLIDESLOPE                                   | 3.0.1  |       |
| 125 | OSD_GPS_MAX_SPEED                                | 4.0.0  |       |
| 126 | OSD_3D_MAX_SPEED                                 | 4.0.0  |       |
| 127 | OSD_AIR_MAX_SPEED                                | 4.0.0  |       |
| 128 | OSD_ACTIVE_PROFILE                               | 4.0.0  |       |
| 129 | OSD_MISSION                                      | 4.0.0  |       |
| 130 | OSD_SWITCH_INDICATOR_0                           | 5.0.0  |       |
| 131 | OSD_SWITCH_INDICATOR_1                           | 5.0.0  |       |
| 132 | OSD_SWITCH_INDICATOR_2                           | 5.0.0  |       |
| 133 | OSD_SWITCH_INDICATOR_3                           | 5.0.0  |       |
| 134 | OSD_TPA_TIME_CONSTANT                            | 5.0.0  |       |
| 135 | OSD_FW_LEVEL_TRIM                                | 5.0.0  |       |
| 136 | OSD_GLIDE_TIME_REMAINING                         | 6.0.0  |       |
| 137 | OSD_GLIDE_RANGE                                  | 6.0.0  |       |
| 138 | OSD_CLIMB_EFFICIENCY                             | 6.0.0  |       |
| 139 | OSD_NAV_WP_MULTI_MISSION_INDEX                   | 6.0.0  |       |
| 140 | OSD_GROUND_COURSE                                | 6.0.0  |       |
| 141 | OSD_CROSS_TRACK_ERROR                            | 6.0.0  |       |
| 142 | OSD_PILOT_NAME                                   | 6.0.0  |       |
| 143 | OSD_PAN_SERVO_CENTRED                            | 6.0.0  |       |
| 144 | OSD_MULTI_FUNCTION                               | 7.0.0  |       |
| 145 | OSD_ODOMETER                                     | 7.0.0  | For this to work correctly, stats must be enabled (`set stats=ON`). Otherwise, this will show the total flight distance. |
| 146 | OSD_PILOT_LOGO                                   | 7.0.0  |       |
| 147 | OSD_BLACKBOX                                     | 7.1.0  | The element will be hidden unless blackbox recording is attempted. |

# Pilot Logos

From INAV 7.0.0, pilots can add their own logos to the OSD. These can appear in 2 places: the power on/arming screens or as an element on the standard OSD. Please note that the power on/arming screen large pilot logos are only available on HD systems.

To use the pilot logos, you will need to make a custom font for your OSD system. Base fonts and information can be found in the [OSD folder](https://github.com/iNavFlight/inav-configurator/tree/master/resources/osd) in the Configurator resources. Each system will need a specific method to create the font image files. So they will not be covered here. There are two pilot logos.

<img alt="Default small INAV Pilot logo" src="https://github.com/iNavFlight/inav-configurator/raw/master/resources/osd/digital/default/24x36/469_471.png" align="right" />The small pilot logo appears on standard OSD layouts, when you add the elemement to the OSD screen. This is a 3 character wide symbol (characters 469-471).

<img alt="Default large INAV Pilot logo" src="https://github.com/iNavFlight/inav-configurator/raw/master/resources/osd/digital/default/24x36/472_511.png" align="right" />The large pilot logo appears on the power on and arming screens, when you enable the feature in the CLI. To do this, set the `osd_use_pilot_logo` parameter to `on`. This is a 10 character wide, 4 character high symbol (characters 472-511).

## Settings

* `osd_arm_screen_display_time` The amount of time the arming screen is displayed.
* `osd_inav_to_pilot_logo_spacing` The spacing between two logos. This can be set to `0`, so the original INAV logo and Pilot Logo can be combined in to a larger logo. Any non-0 value will be adjusted to best align the logos. For example, the Avatar system has an odd number of columns. If you set the spacing to 8, the logos would look misaligned. So the even number will be changed to an odd number for better alignment.
* `osd_use_pilot_logo` Enable to use the large pilot logo.

## Examples

This is an example of the arming screen with the pilot logo enabled. This is using the default settings.
![Arming screen example using default settings with osd_use_pilot_logo set to on](https://user-images.githubusercontent.com/17590174/271817487-eb18da4d-0911-44b2-b670-ea5940f79176.png)

This is an example of setting the `osd_inav_to_pilot_logo_spacing` to 0. This will allow a larger, single logo.
![Power on screen example with 0 spacing between logos](https://user-images.githubusercontent.com/17590174/271817352-6206402c-9da4-4682-9d83-790cc2396b00.png)

# Post Flight Statistics
The post flight statistcs are set in the firmware. Statistics are only hidden if the supporting hardware is not present. Due to size contraints. The post flight statistics are spread over 2 pages on analogue systems.

## Statistics shown
| Statistic                     | Requirement           | Page  | |
|-------------------------------|-----------------------|-------|-|
| Flight Time                   |                       | 1     | The total time from arm to disarm. |
| Flight Distance               |                       | 1     |  |
| Maximum Distance From Home    | GPS                   | 1     |  |
| Maximum Speed                 | GPS                   | 1     |  |
| Average Speed                 | GPS                   | 1     |  |
| Maximum Altitude              | Baro/GPS              | 1     |  |
| Minimum Average Cell Voltage  |                       | 1     |  |
| Minimum Pack Voltage          |                       | 1     |  |
| Maximum Current               | Current Sensor        | 1     |  |
| Maximum Power                 | Current Sensor        | 1     |  |
| Energy Used (Flight)          | Current Sensor        | 1     |  |
| Energy Used (Battery Total)   | Current Sensor        | 1     | This data is not reset on arming. |
| Average Efficiency            | Current Sensor & GPS  | 1     |  |
| Minimum RSSI                  |                       | 2     |  |
| Minimum LQ                    | CRSF                  | 2     |  |
| Minmum dBm                    | CRSF                  | 2     |  |
| Minimum Satellites            | GPS                   | 2     |  |
| Maximum Satellites            | GPS                   | 2     |  |
| Minimum ESC Temperature       | ESC Telemetry         | 2     |  |
| Maximum ESC Temperature       | ESC Telemetry         | 2     |  |
| Maximum G-Force               |                       | 2     |  |
| Minimum Z axis G-Force        |                       | 2     |  |
| Maximum Z axis G-Force        |                       | 2     |  |
| Blackbox file number          | Blackbox recording    | 2     |  |
| Disarm method                 |                       | 1 & 2 |  |
| Settings save status          |                       | 1 & 2 | Shows a message if the settings are being saved or have been saved on disarm. |

## Configuration
There are a couple of settings that allow you to adjust parts of the post flights statistics.

- `osd_stats_page_auto_swap_time` allows you to specify how long each stats page is displayed [seconds]. Reverts to manual control when Roll stick used to change pages. Disabled when set to 0.
- `osd_stats_energy_unit` allows you to choose the unit used for the drawn energy in the OSD stats [MAH/WH] (milliAmpere hour/ Watt hour). Default is MAH.
- `osd_stats_show_metric_efficiency` if you use non-metric units on your OSD. Enabling this option will also show the efficiency in metric.