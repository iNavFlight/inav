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

#if defined(STM32F7) || defined(STM32H7)
#define USE_ITCM_RAM
#endif

#ifdef USE_ITCM_RAM
#define FAST_CODE                   __attribute__((section(".tcm_code")))
#define NOINLINE                    __attribute__((noinline))
#else
#define FAST_CODE
#define NOINLINE
#endif

#define DYNAMIC_HEAP_SIZE   2048

#ifndef MAX_MAVLINK_PORTS
#define MAX_MAVLINK_PORTS 4
#endif

#define I2C1_OVERCLOCK false
#define I2C2_OVERCLOCK false
#ifndef USE_I2C_PULLUP
#define USE_I2C_PULLUP          // Enable built-in pullups on all boards in case external ones are too week
#endif

#ifndef USE_SERIAL_RX
#define USE_SERIAL_RX
#endif
#ifndef USE_SERIALRX_SPEKTRUM
#define USE_SERIALRX_SPEKTRUM   // Cheap and fairly common protocol
#endif
#ifndef USE_SERIALRX_SBUS
#define USE_SERIALRX_SBUS       // Very common protocol
#endif
#ifndef USE_SERIALRX_IBUS
#define USE_SERIALRX_IBUS       // Cheap FlySky & Turnigy receivers
#endif
#ifndef USE_SERIALRX_FPORT
#define USE_SERIALRX_FPORT
#endif
#ifndef USE_SERIALRX_FPORT2
#define USE_SERIALRX_FPORT2
#endif

//#define USE_DEV_TOOLS           // tools for dev use only. Undefine for release builds.

#define COMMON_DEFAULT_FEATURES (FEATURE_TX_PROF_SEL)

#ifndef USE_SERVO_SBUS
#define USE_SERVO_SBUS
#endif

#ifndef USE_ADC_AVERAGING
#define USE_ADC_AVERAGING
#endif
#ifndef USE_64BIT_TIME
#define USE_64BIT_TIME
#endif
#ifndef USE_BLACKBOX
#define USE_BLACKBOX
#endif
#ifndef USE_GPS
#define USE_GPS
#endif
#ifndef USE_GPS_PROTO_UBLOX
#define USE_GPS_PROTO_UBLOX
#endif
#ifndef USE_GPS_PROTO_MSP
#define USE_GPS_PROTO_MSP
#endif
#ifndef USE_GPS_PROTO_DRONECAN
#define USE_GPS_PROTO_DRONECAN
#endif
#ifndef USE_TELEMETRY
#define USE_TELEMETRY
#endif
#ifndef USE_TELEMETRY_LTM
#define USE_TELEMETRY_LTM
#endif
#ifndef USE_GPS_FIX_ESTIMATION
#define USE_GPS_FIX_ESTIMATION
#endif

// This is the shortest period in microseconds that the scheduler will allow
#define SCHEDULER_DELAY_LIMIT           10

#if defined(MAG_I2C_BUS) || defined(VCM5883_I2C_BUS)
#ifndef USE_MAG_VCM5883
#define USE_MAG_VCM5883
#endif
#endif

#ifndef USE_MR_BRAKING_MODE
#define USE_MR_BRAKING_MODE
#endif
#ifndef USE_PITOT
#define USE_PITOT
#endif
#ifndef USE_PITOT_ADC
#define USE_PITOT_ADC
#endif

#ifndef USE_DYNAMIC_FILTERS
#define USE_DYNAMIC_FILTERS
#endif
#ifndef USE_GYRO_KALMAN
#define USE_GYRO_KALMAN
#endif
#ifndef USE_SMITH_PREDICTOR
#define USE_SMITH_PREDICTOR
#endif
#ifndef USE_RATE_DYNAMICS
#define USE_RATE_DYNAMICS
#endif
#ifndef USE_EXTENDED_CMS_MENUS
#define USE_EXTENDED_CMS_MENUS
#endif

// Allow default rangefinders
#ifndef USE_RANGEFINDER
#define USE_RANGEFINDER
#endif
#ifndef USE_RANGEFINDER_MSP
#define USE_RANGEFINDER_MSP
#endif
#ifndef USE_RANGEFINDER_BENEWAKE
#define USE_RANGEFINDER_BENEWAKE
#endif
#ifndef USE_RANGEFINDER_VL53L0X
#define USE_RANGEFINDER_VL53L0X
#endif
#ifndef USE_RANGEFINDER_VL53L1X
#define USE_RANGEFINDER_VL53L1X
#endif
#ifndef USE_RANGEFINDER_US42
#define USE_RANGEFINDER_US42
#endif
#ifndef USE_RANGEFINDER_TOF10120_I2C
#define USE_RANGEFINDER_TOF10120_I2C
#endif
#ifndef USE_RANGEFINDER_TERARANGER_EVO_I2C
#define USE_RANGEFINDER_TERARANGER_EVO_I2C
#endif
#ifndef USE_RANGEFINDER_USD1_V0
#define USE_RANGEFINDER_USD1_V0
#endif
#ifndef USE_RANGEFINDER_NANORADAR
#define USE_RANGEFINDER_NANORADAR
#endif

// Allow default optic flow boards
#ifndef USE_OPFLOW
#define USE_OPFLOW
#endif
#ifndef USE_OPFLOW_CXOF
#define USE_OPFLOW_CXOF
#endif
#ifndef USE_OPFLOW_MSP
#define USE_OPFLOW_MSP
#endif

// Allow default airspeed sensors
#ifndef USE_PITOT
#define USE_PITOT
#endif
#ifndef USE_PITOT_MS4525
#define USE_PITOT_MS4525
#endif
#ifndef USE_PITOT_MS5525
#define USE_PITOT_MS5525
#endif
#ifndef USE_PITOT_MSP
#define USE_PITOT_MSP
#endif
#ifndef USE_PITOT_DLVR
#define USE_PITOT_DLVR
#endif

#ifndef USE_1WIRE
#define USE_1WIRE
#endif
#ifndef USE_1WIRE_DS2482
#define USE_1WIRE_DS2482
#endif

#ifndef USE_TEMPERATURE_SENSOR
#define USE_TEMPERATURE_SENSOR
#endif
#ifndef USE_TEMPERATURE_LM75
#define USE_TEMPERATURE_LM75
#endif
#ifndef USE_TEMPERATURE_DS18B20
#define USE_TEMPERATURE_DS18B20
#endif

#ifndef USE_MSP_DISPLAYPORT
#define USE_MSP_DISPLAYPORT
#endif
#ifndef USE_DASHBOARD
#define USE_DASHBOARD
#endif
#ifndef DASHBOARD_ARMED_BITMAP
#define DASHBOARD_ARMED_BITMAP
#endif
#ifndef USE_OLED_UG2864
#define USE_OLED_UG2864
#endif

#ifndef USE_OSD
#define USE_OSD
#endif
#ifndef USE_FRSKYOSD
#define USE_FRSKYOSD
#endif
#ifndef USE_DJI_HD_OSD
#define USE_DJI_HD_OSD
#endif
#ifndef USE_MSP_OSD
#define USE_MSP_OSD
#endif

#ifndef NAV_NON_VOLATILE_WAYPOINT_CLI
#define NAV_NON_VOLATILE_WAYPOINT_CLI
#endif

#ifndef USE_D_BOOST
#define USE_D_BOOST
#endif
#ifndef USE_ANTIGRAVITY
#define USE_ANTIGRAVITY
#endif

#ifndef USE_I2C_IO_EXPANDER
#define USE_I2C_IO_EXPANDER
#endif

#ifndef USE_MSP_OVER_TELEMETRY
#define USE_MSP_OVER_TELEMETRY
#endif

#ifndef USE_SERIALRX_SRXL2
#define USE_SERIALRX_SRXL2     // Spektrum SRXL2 protocol
#endif
#ifndef USE_SERIALRX_JETIEXBUS
#define USE_SERIALRX_JETIEXBUS
#endif
#ifndef USE_TELEMETRY_SRXL
#define USE_TELEMETRY_SRXL
#endif
#ifndef USE_SPEKTRUM_CMS_TELEMETRY
#define USE_SPEKTRUM_CMS_TELEMETRY
#endif
//#define USE_SPEKTRUM_VTX_CONTROL //Some functions from betaflight still not implemented
#ifndef USE_SPEKTRUM_VTX_TELEMETRY
#define USE_SPEKTRUM_VTX_TELEMETRY
#endif

#ifndef USE_VTX_COMMON
#define USE_VTX_COMMON
#endif

#ifndef USE_SERIALRX_GHST
#define USE_SERIALRX_GHST
#endif
#ifndef USE_TELEMETRY_GHST
#define USE_TELEMETRY_GHST
#endif

#ifndef USE_POWER_LIMITS
#define USE_POWER_LIMITS
#endif

#ifndef USE_SAFE_HOME
#define USE_SAFE_HOME
#endif
#ifndef USE_FW_AUTOLAND
#define USE_FW_AUTOLAND
#endif
#ifndef USE_AUTOTUNE_FIXED_WING
#define USE_AUTOTUNE_FIXED_WING
#endif
#ifndef USE_LOG
#define USE_LOG
#endif
#define USE_BOOTLOG 2048
#ifndef USE_STATS
#define USE_STATS
#endif
#ifndef USE_CMS
#define USE_CMS
#endif
#ifndef CMS_MENU_OSD
#define CMS_MENU_OSD
#endif
#ifndef NAV_NON_VOLATILE_WAYPOINT_STORAGE
#define NAV_NON_VOLATILE_WAYPOINT_STORAGE
#endif
#ifndef USE_TELEMETRY_IBUS
#define USE_TELEMETRY_IBUS
#endif
#ifndef USE_TELEMETRY_SMARTPORT
#define USE_TELEMETRY_SMARTPORT
#endif
#ifndef USE_TELEMETRY_CRSF
#define USE_TELEMETRY_CRSF
#endif
#ifndef USE_TELEMETRY_JETIEXBUS
#define USE_TELEMETRY_JETIEXBUS
#endif
// These are rather exotic serial protocols
#ifndef USE_RX_MSP
#define USE_RX_MSP
#endif
#ifndef USE_MSP_RC_OVERRIDE
#define USE_MSP_RC_OVERRIDE
#endif
#ifndef USE_SERIALRX_CRSF
#define USE_SERIALRX_CRSF
#endif
#ifndef USE_SERIAL_PASSTHROUGH
#define USE_SERIAL_PASSTHROUGH
#endif
#define NAV_MAX_WAYPOINTS       120
#ifndef USE_RCDEVICE
#define USE_RCDEVICE
#endif
#ifndef USE_MULTI_MISSION
#define USE_MULTI_MISSION
#endif
#ifndef USE_MULTI_FUNCTIONS
#define USE_MULTI_FUNCTIONS  // defines functions only, warnings always defined
#endif

//Enable VTX control
#ifndef USE_VTX_CONTROL
#define USE_VTX_CONTROL
#endif
#ifndef USE_VTX_SMARTAUDIO
#define USE_VTX_SMARTAUDIO
#endif
#ifndef USE_VTX_TRAMP
#define USE_VTX_TRAMP
#endif
#ifndef USE_VTX_MSP
#define USE_VTX_MSP
#endif

#ifndef USE_PROGRAMMING_FRAMEWORK
#define USE_PROGRAMMING_FRAMEWORK
#endif
#ifndef USE_CLI_BATCH
#define USE_CLI_BATCH
#endif

//Enable DST calculations
#ifndef RTC_AUTOMATIC_DST
#define RTC_AUTOMATIC_DST
#endif
// Wind estimator
#ifndef USE_WIND_ESTIMATOR
#define USE_WIND_ESTIMATOR
#endif

#ifndef USE_SIMULATOR
#define USE_SIMULATOR
#endif
#ifndef USE_PITOT_VIRTUAL
#define USE_PITOT_VIRTUAL
#endif
#ifndef USE_FAKE_BATT_SENSOR
#define USE_FAKE_BATT_SENSOR
#endif
#ifndef USE_RANGEFINDER_FAKE
#define USE_RANGEFINDER_FAKE
#endif
#ifndef USE_RX_SIM
#define USE_RX_SIM
#endif

#ifndef USE_CMS_FONT_PREVIEW
#define USE_CMS_FONT_PREVIEW
#endif

//ADSB RECEIVER
#ifdef USE_GPS
#ifndef USE_ADSB
#define USE_ADSB
#endif
#define MAX_ADSB_VEHICLES               5
#define ADSB_LIMIT_CM                   6400000
#endif

#ifndef USE_SERIAL_GIMBAL
#define USE_SERIAL_GIMBAL
#endif
#ifndef USE_HEADTRACKER
#define USE_HEADTRACKER
#endif
#ifndef USE_HEADTRACKER_SERIAL
#define USE_HEADTRACKER_SERIAL
#endif
#ifndef USE_HEADTRACKER_MSP
#define USE_HEADTRACKER_MSP
#endif

#if defined(STM32F7) || defined(STM32H7)
// needs bi-direction inverter, not available on F4 hardware.
#define USE_TELEMETRY_SBUS2
#endif

// Keep larger optional features off 512 KB targets to preserve flash space.
#if (MCU_FLASH_SIZE > 512)
#define USE_AUTO_TRANSITION
#define USE_TELEMETRY_MAVLINK
#define USE_SERIALRX_MAVLINK
#define USE_TELEMETRY_SIM
#define USE_VTX_FFPV
#define USE_SERIALRX_SUMD
#define USE_TELEMETRY_HOTT
#define USE_HOTT_TEXTMODE
#define USE_34CHANNELS
#define USE_MARKER_GUIDANCE
#define MAX_MIXER_PROFILE_COUNT 2
#define USE_SMARTPORT_MASTER
#ifdef USE_GPS
#define USE_GEOZONE
#define MAX_GEOZONES_IN_CONFIG 63
#define MAX_VERTICES_IN_CONFIG 126
#endif
#elif !defined(STM32F7)
#define MAX_MIXER_PROFILE_COUNT 1
#endif

#if (MCU_FLASH_SIZE <= 512)
    #define SKIP_CLI_COMMAND_HELP
    #undef USE_SERIALRX_SPEKTRUM
    #undef USE_TELEMETRY_SRXL
#endif

#ifndef USE_EZ_TUNE
#define USE_EZ_TUNE
#endif
#ifndef USE_ADAPTIVE_FILTER
#define USE_ADAPTIVE_FILTER
#endif
