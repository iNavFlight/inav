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

// Backward compatibility for I2C OLED display
#if !defined(USE_I2C)
# undef USE_DASHBOARD
# undef USE_OLED_UG2864
#endif

// Enable MSP_DISPLAYPORT for F3 targets without builtin OSD,
// since it's used to display CMS on MWOSD
#if !defined(USE_MSP_DISPLAYPORT) && (FLASH_SIZE > 128) && !defined(USE_OSD)
#define USE_MSP_DISPLAYPORT
#endif

#if defined(USE_OSD) && (FLASH_SIZE > 256)
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

#ifdef USE_ITCM_RAM
#define FAST_CODE                   __attribute__((section(".tcm_code")))
#define NOINLINE                    __NOINLINE
#else
#define FAST_CODE
#define NOINLINE
#endif

#ifdef STM32F3
#undef USE_WIND_ESTIMATOR
#undef USE_SERIALRX_SUMD
#undef USE_SERIALRX_SUMH
#undef USE_SERIALRX_XBUS
#undef USE_SERIALRX_JETIEXBUS
#undef USE_PWM_SERVO_DRIVER
#endif

#if defined(SIMULATOR_BUILD) || defined(UNIT_TEST)
// This feature uses 'arm_math.h', which does not exist for x86.
#undef USE_DYNAMIC_FILTERS
#endif

//Defines for compiler optimizations
#ifndef STM32F3
#define FUNCTION_COMPILE_FOR_SIZE __attribute__((optimize("-Os")))
#define FUNCTION_COMPILE_NORMAL __attribute__((optimize("-O2")))
#define FUNCTION_COMPILE_FOR_SPEED __attribute__((optimize("-Ofast")))
#define FILE_COMPILE_FOR_SIZE _Pragma("GCC optimize(\"Os\")")
#define FILE_COMPILE_NORMAL _Pragma("GCC optimize(\"O2\")")
#define FILE_COMPILE_FOR_SPEED _Pragma("GCC optimize(\"Ofast\")")
#else
#define FUNCTION_COMPILE_FOR_SIZE
#define FUNCTION_COMPILE_NORMAL
#define FUNCTION_COMPILE_FOR_SPEED
#define FILE_COMPILE_FOR_SIZE
#define FILE_COMPILE_NORMAL
#define FILE_COMPILE_FOR_SPEED
#endif
