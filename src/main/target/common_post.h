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

// Touch up configuration

#pragma once

// Config storage in memory-mapped flash
extern uint8_t __config_start;
extern uint8_t __config_end;

// Backward compatibility for I2C OLED display
#if !defined(USE_I2C)
# undef USE_DASHBOARD
# undef USE_OLED_UG2864
#endif


// Make sure DEFAULT_I2C_BUS is valid
#ifndef DEFAULT_I2C_BUS

#if defined(USE_I2C_DEVICE_1)
#define DEFAULT_I2C_BUS BUS_I2C1
#elif defined(USE_I2C_DEVICE_2)
#define DEFAULT_I2C_BUS BUS_I2C2
#elif defined(USE_I2C_DEVICE_3)
#define DEFAULT_I2C_BUS BUS_I2C3
#elif defined(USE_I2C_DEVICE_4)
#define DEFAULT_I2C_BUS BUS_I2C4
#endif

#endif

// Airspeed sensors
#if defined(USE_PITOT) && defined(DEFAULT_I2C_BUS)

#ifndef PITOT_I2C_BUS
#define PITOT_I2C_BUS DEFAULT_I2C_BUS
#endif

#endif

// Temperature sensors
#if !defined(TEMPERATURE_I2C_BUS) && defined(DEFAULT_I2C_BUS)
#define TEMPERATURE_I2C_BUS DEFAULT_I2C_BUS
#endif

// Rangefinder sensors
#if !defined(RANGEFINDER_I2C_BUS) && defined(DEFAULT_I2C_BUS)
#define RANGEFINDER_I2C_BUS DEFAULT_I2C_BUS
#endif

// Enable MSP_DISPLAYPORT for F3 targets without builtin OSD,
// since it's used to display CMS on MWOSD
#if !defined(USE_MSP_DISPLAYPORT) && !defined(USE_OSD)
#define USE_MSP_DISPLAYPORT
#endif

#if defined(USE_OSD)
#define USE_CANVAS
#endif

// Enable MSP BARO & MAG drivers if BARO and MAG sensors are compiled in
#if defined(USE_MAG)
#define USE_MAG_MSP

#if defined(USE_MAG_ALL)

#define USE_MAG_HMC5883
#define USE_MAG_IST8310
#define USE_MAG_LIS3MDL
#define USE_MAG_MAG3110
#define USE_MAG_QMC5883

//#if (MCU_FLASH_SIZE > 512)
#define USE_MAG_AK8963
#define USE_MAG_AK8975
#define USE_MAG_IST8308
#define USE_MAG_MLX90393

#if defined(USE_IMU_MPU9250)
#define USE_MAG_MPU9250
#endif

#define USE_MAG_RM3100
#define USE_MAG_VCM5883
//#endif // MCU_FLASH_SIZE

#endif // USE_MAG_ALL

#if defined(DEFAULT_I2C_BUS) && !defined(MAG_I2C_BUS)
#define MAG_I2C_BUS DEFAULT_I2C_BUS
#endif

#endif // USE_MAG

#if defined(USE_BARO)
#define USE_BARO_MSP

#if defined(USE_BARO_ALL)
#define USE_BARO_BMP085
#define USE_BARO_BMP280
#define USE_BARO_BMP388
#define USE_BARO_BMP390
#define USE_BARO_DPS310
#define USE_BARO_LPS25H
#define USE_BARO_MS5607
#define USE_BARO_MS5611
//#define USE_BARO_SPI_BMP280
#define USE_BARO_SPL06
#endif

#if defined(DEFAULT_I2C_BUS) && !defined(BARO_I2C_BUS)
#define BARO_I2C_BUS DEFAULT_I2C_BUS
#endif

#endif

#ifdef USE_ESC_SENSOR
    #define USE_RPM_FILTER
#endif

#ifndef BEEPER_PWM_FREQUENCY
#define BEEPER_PWM_FREQUENCY    2500
#endif

#define USE_ARM_MATH // try to use FPU functions

#if defined(SITL_BUILD) || defined(UNIT_TEST)
// This feature uses 'arm_math.h', which does not exist for x86.
#undef USE_DYNAMIC_FILTERS
#undef USE_ARM_MATH
#endif

#if defined(CONFIG_IN_RAM) || defined(CONFIG_IN_FILE) || defined(CONFIG_IN_EXTERNAL_FLASH)
#ifndef EEPROM_SIZE
#define EEPROM_SIZE     8192
#endif
extern uint8_t eepromData[EEPROM_SIZE];
#define __config_start (*eepromData)
#define __config_end (*ARRAYEND(eepromData))
#else
#ifndef CONFIG_IN_FLASH
#define CONFIG_IN_FLASH
#endif
extern uint8_t __config_start;   // configured via linker script when building binaries.
extern uint8_t __config_end;
#endif
