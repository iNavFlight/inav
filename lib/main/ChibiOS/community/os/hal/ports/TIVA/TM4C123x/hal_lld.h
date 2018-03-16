/*
    Copyright (C) 2014..2016 Marco Veeneman

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    Tiva/TM4C123x/hal_lld.h
 * @brief   TM4C123x HAL subsystem low level driver header.
 * @pre     This module requires the following macros to be defined in the
 *          @p board.h file:
 *          - TODO: add required macros
 *
 * @addtogroup HAL
 * @{
 */

#ifndef HAL_LLD_H
#define HAL_LLD_H

#include "tiva_registry.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Platform identification
 * @{
 */
 
#define PLATFORM_NAME           "Tiva C Series TM4C123x"

/**
 * @}
 */

/**
 * @name    RCC register bits definitions
 * @{
 */

#define TIVA_RCC_MOSCDIS            (0x01 << 0)

#define TIVA_RCC_OSCSRC_MASK        (0x03 << 4)
#define TIVA_RCC_OSCSRC_MOSC        (0x00 << 4)
#define TIVA_RCC_OSCSRC_PIOSC       (0x01 << 4)
#define TIVA_RCC_OSCSRC_PIOSC_4     (0x02 << 4)
#define TIVA_RCC_OSCSRC_LFIOSC      (0x03 << 4)

#define TIVA_RCC_XTAL_MASK          (0x1f << 6)
#define TIVA_RCC_XTAL_4000000       (0x06 << 6)
#define TIVA_RCC_XTAL_4096000       (0x07 << 6)
#define TIVA_RCC_XTAL_4915200       (0x08 << 6)
#define TIVA_RCC_XTAL_5000000       (0x09 << 6)
#define TIVA_RCC_XTAL_5120000       (0x0a << 6)
#define TIVA_RCC_XTAL_6000000       (0x0b << 6)
#define TIVA_RCC_XTAL_6144000       (0x0c << 6)
#define TIVA_RCC_XTAL_7372800       (0x0d << 6)
#define TIVA_RCC_XTAL_8000000       (0x0e << 6)
#define TIVA_RCC_XTAL_8192000       (0x0f << 6)
#define TIVA_RCC_XTAL_10000000      (0x10 << 6)
#define TIVA_RCC_XTAL_12000000      (0x11 << 6)
#define TIVA_RCC_XTAL_12288000      (0x12 << 6)
#define TIVA_RCC_XTAL_13560000      (0x13 << 6)
#define TIVA_RCC_XTAL_14318180      (0x14 << 6)
#define TIVA_RCC_XTAL_16000000      (0x15 << 6)
#define TIVA_RCC_XTAL_16384000      (0x16 << 6)
#define TIVA_RCC_XTAL_18000000      (0x17 << 6)
#define TIVA_RCC_XTAL_20000000      (0x18 << 6)
#define TIVA_RCC_XTAL_24000000      (0x19 << 6)
#define TIVA_RCC_XTAL_25000000      (0x1a << 6)

#define TIVA_RCC_BYPASS             (1 << 11)

#define TIVA_RCC_PWRDN              (1 << 13)

#define TIVA_RCC_PWMDIV_MASK        (0x07 << 17)
#define TIVA_RCC_PWMDIV_2           (0x00 << 17)
#define TIVA_RCC_PWMDIV_4           (0x01 << 17)
#define TIVA_RCC_PWMDIV_8           (0x02 << 17)
#define TIVA_RCC_PWMDIV_16          (0x03 << 17)
#define TIVA_RCC_PWMDIV_32          (0x04 << 17)
#define TIVA_RCC_PWMDIV_64          (0x07 << 17)

#define TIVA_RCC_USEPWMDIV          (1 << 20)

#define TIVA_RCC_USESYSDIV	        (1 << 22)

#define TIVA_RCC_SYSDIV_MASK	    (0x0f << 23)
#define TIVA_RCC_SYSDIV_1           (0x00 << 23)
#define TIVA_RCC_SYSDIV_2           (0x01 << 23)
#define TIVA_RCC_SYSDIV_3           (0x02 << 23)
#define TIVA_RCC_SYSDIV_4           (0x03 << 23)
#define TIVA_RCC_SYSDIV_5           (0x04 << 23)
#define TIVA_RCC_SYSDIV_6           (0x05 << 23)
#define TIVA_RCC_SYSDIV_7           (0x06 << 23)
#define TIVA_RCC_SYSDIV_8           (0x07 << 23)
#define TIVA_RCC_SYSDIV_9           (0x08 << 23)
#define TIVA_RCC_SYSDIV_10          (0x09 << 23)
#define TIVA_RCC_SYSDIV_11          (0x0a << 23)
#define TIVA_RCC_SYSDIV_12          (0x0b << 23)
#define TIVA_RCC_SYSDIV_13          (0x0c << 23)
#define TIVA_RCC_SYSDIV_14          (0x0d << 23)
#define TIVA_RCC_SYSDIV_15          (0x0e << 23)
#define TIVA_RCC_SYSDIV_16          (0x0f << 23)

#define TIVA_RCC_ACG		        (1 << 27)

/**
 * @}
 */

/**
 * @name    RCC2 register bits definitions
 * @{
 */

#define TIVA_RCC2_OSCSRC2_MASK      (0x07 << 4)
#define TIVA_RCC2_OSCSRC2_MOSC      (0x00 << 4)
#define TIVA_RCC2_OSCSRC2_PIOSC     (0x01 << 4)
#define TIVA_RCC2_OSCSRC2_PIOSC_4   (0x02 << 4)
#define TIVA_RCC2_OSCSRC2_LFIOSC    (0x03 << 4)
#define TIVA_RCC2_OSCSRC2_32768     (0x07 << 4)

#define TIVA_RCC2_BYPASS2           (1 << 11)

#define TIVA_RCC2_PWRDN2            (1 << 13)

#define TIVA_RCC2_USBPWRDN          (1 << 14)

#define TIVA_RCC2_SYSDIV2LSB        (1 << 22)

#define TIVA_RCC2_SYSDIV2_MASK      (0x3f << 23)

#define TIVA_RCC2_DIV400            (1 << 30)

#define TIVA_RCC2_USERCC2           (1 << 31)

/**
 * @}
 */

/**
 * @name    RIS register bits definitions
 * @{
 */

#define SYSCTL_RIS_PLLLRIS	            (1 << 6)

/**
 * @}
 */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */

#if !defined(TIVA_OSCSRC)
#define TIVA_OSCSRC                     TIVA_RCC2_OSCSRC2_MOSC
#endif

#if !defined(TIVA_MOSC_ENABLE)
#define TIVA_MOSC_ENABLE                TRUE
#endif

#if !defined(TIVA_DIV400_VALUE)
#define TIVA_DIV400_VALUE               1
#endif

#if !defined(TIVA_SYSDIV_VALUE)
#define TIVA_SYSDIV_VALUE               2
#endif

#if !defined(TIVA_USESYSDIV_ENABLE)
#define TIVA_USESYSDIV_ENABLE           FALSE
#endif

#if !defined(TIVA_SYSDIV2LSB_ENABLE)
#define TIVA_SYSDIV2LSB_ENABLE          FALSE
#endif

#if !defined(TIVA_BYPASS_VALUE)
#define TIVA_BYPASS_VALUE               0
#endif

/**
 * @}
 */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*
 * Configuration-related checks.
 */
#if !defined(TM4C123x_MCUCONF)
#error "Using a wrong mcuconf.h file, TM4C123x_MCUCONF not defined"
#endif

/*
 * Oscillator-related checks.
 */
#if !(TIVA_OSCSRC == TIVA_RCC2_OSCSRC2_MOSC) &&                                 \
    !(TIVA_OSCSRC == TIVA_RCC2_OSCSRC2_PIOSC) &&                                \
    !(TIVA_OSCSRC == TIVA_RCC2_OSCSRC2_PIOSC_4) &&                              \
    !(TIVA_OSCSRC == TIVA_RCC2_OSCSRC2_LFIOSC) &&                               \
    !(TIVA_OSCSRC == TIVA_RCC2_OSCSRC2_32768)
#error "Invalid value for TIVA_OSCSRC defined"
#endif

#if TIVA_XTAL_VALUE == 4000000
#define TIVA_XTAL_              (0x06 << 6)
#elif TIVA_XTAL_VALUE == 4096000
#define TIVA_XTAL_              (0x07 << 6)
#elif TIVA_XTAL_VALUE == 4915200
#define TIVA_XTAL_              (0x08 << 6)
#elif TIVA_XTAL_VALUE == 5000000
#define TIVA_XTAL_              (0x09 << 6)
#elif TIVA_XTAL_VALUE == 5120000
#define TIVA_XTAL_              (0x0a << 6)
#elif TIVA_XTAL_VALUE == 6000000
#define TIVA_XTAL_              (0x0b << 6)
#elif TIVA_XTAL_VALUE == 6144000
#define TIVA_XTAL_              (0x0c << 6)
#elif TIVA_XTAL_VALUE == 7372800
#define TIVA_XTAL_              (0x0d << 6)
#elif TIVA_XTAL_VALUE == 8000000
#define TIVA_XTAL_              (0x0e << 6)
#elif TIVA_XTAL_VALUE == 8192000
#define TIVA_XTAL_              (0x0f << 6)
#elif TIVA_XTAL_VALUE == 10000000
#define TIVA_XTAL_              (0x10 << 6)
#elif TIVA_XTAL_VALUE == 12000000
#define TIVA_XTAL_              (0x11 << 6)
#elif TIVA_XTAL_VALUE == 12288000
#define TIVA_XTAL_              (0x12 << 6)
#elif TIVA_XTAL_VALUE == 13560000
#define TIVA_XTAL_              (0x13 << 6)
#elif TIVA_XTAL_VALUE == 14318180
#define TIVA_XTAL_              (0x14 << 6)
#elif TIVA_XTAL_VALUE == 16000000
#define TIVA_XTAL_              (0x15 << 6)
#elif TIVA_XTAL_VALUE == 16384000
#define TIVA_XTAL_              (0x16 << 6)
#elif TIVA_XTAL_VALUE == 18000000
#define TIVA_XTAL_              (0x17 << 6)
#elif TIVA_XTAL_VALUE == 20000000
#define TIVA_XTAL_              (0x18 << 6)
#elif TIVA_XTAL_VALUE == 24000000
#define TIVA_XTAL_              (0x19 << 6)
#elif TIVA_XTAL_VALUE == 25000000
#define TIVA_XTAL_              (0x1a << 6)
#else
#error "Invalid value for TIVA_XTAL_VALUE defined"
#endif

#if TIVA_MOSC_ENABLE == TRUE
#define TIVA_MOSCDIS            (0 << 0)
#define TIVA_XTAL               TIVA_XTAL_
#elif TIVA_MOSC_ENABLE == FALSE
#define TIVA_MOSCDIS            (1 << 0)
#define TIVA_XTAL               0
#else
#error "Invalid value for TIVA_MOSC_ENABLE defined"
#endif

#if TIVA_DIV400_VALUE == 1
#define TIVA_DIV400             (1 << 30)
#elif TIVA_DIV400_VALUE == 0
#define TIVA_DIV400             (0 << 30)
#else
#error "Invalid value for TIVA_DIV400_VALUE defined"
#endif

#if (TIVA_SYSDIV_VALUE >= 0x02) && (TIVA_SYSDIV_VALUE <= 0x3f)
#define TIVA_SYSDIV             (TIVA_SYSDIV_VALUE << 23)
#define TIVA_SYSDIV2            (TIVA_SYSDIV_VALUE << 23)
#else
#error "Invalid value for TIVA_SYSDIV_VALUE defined"
#endif

#if TIVA_USESYSDIV_ENABLE == TRUE
#define TIVA_USESYSDIV          (1 << 22)
#elif TIVA_USESYSDIV_ENABLE == FALSE
#define TIVA_USESYSDIV          (0 << 22)
#else
#error "Invalid value for TIVA_USESYSDIV_ENABLE defined"
#endif

#if TIVA_SYSDIV2LSB_ENABLE == TRUE
#define TIVA_SYSDIV2LSB         (1 << 22)
#elif TIVA_SYSDIV2LSB_ENABLE == FALSE
#define TIVA_SYSDIV2LSB         (0 << 22)
#else
#error "Invalid value for TIVA_SYSDIV2LSB_ENABLE defined"
#endif

#if TIVA_BYPASS_VALUE == 1
#define TIVA_SRC                16000000
#elif TIVA_BYPASS_VALUE == 0
#define TIVA_SRC                (200000000 + (TIVA_DIV400_VALUE * 200000000))
#else
#error "Invalid value for TIVA_BYPASS_VALUE defined"
#endif

#if (TIVA_OSCSRC == TIVA_RCC_OSCSRC_MOSC) && (TIVA_MOSC_ENABLE == FALSE)
#error "Main Oscillator selected but not enabled"
#endif

/*
 * System Clock calculation 
 */
#define TIVA_SYSCLK             (TIVA_SRC / (((TIVA_SYSDIV_VALUE << TIVA_DIV400_VALUE /*& TIVA_BYPASS_VALUE*/) | (TIVA_SYSDIV2LSB >> 22)) + 1))

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

/* Various helpers.*/
#include "nvic.h"
#include "tiva_isr.h"
#include "tiva_udma.h"

#ifdef __cplusplus
extern "C" {
#endif
  void hal_lld_init(void);
  void tiva_clock_init(void);
#ifdef __cplusplus
}
#endif

#endif /* HAL_LLD_H */

/**
 * @}
 */
