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
#endif

#if defined(USE_BARO)
#define USE_BARO_MSP
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

//Defines for compiler optimizations
#define FUNCTION_COMPILE_FOR_SIZE __attribute__((optimize("-Os")))
#define FUNCTION_COMPILE_NORMAL __attribute__((optimize("-O2")))
#define FUNCTION_COMPILE_FOR_SPEED __attribute__((optimize("-Ofast")))
#define FILE_COMPILE_FOR_SIZE _Pragma("GCC optimize(\"Os\")")
#define FILE_COMPILE_NORMAL _Pragma("GCC optimize(\"O2\")")
#define FILE_COMPILE_FOR_SPEED _Pragma("GCC optimize(\"Ofast\")")

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
