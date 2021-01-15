/*
    Copyright (C) 2014..2017 Marco Veeneman

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

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */

#if !defined(TIVA_OSCSRC)
#define TIVA_OSCSRC                     SYSCTL_RCC2_OSCSRC2_MO
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
#if !(TIVA_OSCSRC == SYSCTL_RCC2_OSCSRC2_MO) &&                                 \
    !(TIVA_OSCSRC == SYSCTL_RCC2_OSCSRC2_IO) &&                                \
    !(TIVA_OSCSRC == SYSCTL_RCC2_OSCSRC2_IO4) &&                              \
    !(TIVA_OSCSRC == SYSCTL_RCC2_OSCSRC2_30) &&                               \
    !(TIVA_OSCSRC == SYSCTL_RCC2_OSCSRC2_32)
#error "Invalid value for TIVA_OSCSRC defined"
#endif

#if TIVA_XTAL_VALUE == 4000000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_4MHZ
#elif TIVA_XTAL_VALUE == 4096000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_4_09MHZ
#elif TIVA_XTAL_VALUE == 4915200
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_4_91MHZ
#elif TIVA_XTAL_VALUE == 5000000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_5MHZ
#elif TIVA_XTAL_VALUE == 5120000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_5_12MHZ
#elif TIVA_XTAL_VALUE == 6000000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_6MHZ
#elif TIVA_XTAL_VALUE == 6144000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_6_14MHZ
#elif TIVA_XTAL_VALUE == 7372800
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_7_37MHZ
#elif TIVA_XTAL_VALUE == 8000000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_8MHZ
#elif TIVA_XTAL_VALUE == 8192000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_8_19MHZ
#elif TIVA_XTAL_VALUE == 10000000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_10MHZ
#elif TIVA_XTAL_VALUE == 12000000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_12MHZ
#elif TIVA_XTAL_VALUE == 12288000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_12_2MHZ
#elif TIVA_XTAL_VALUE == 13560000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_13_5MHZ
#elif TIVA_XTAL_VALUE == 14318180
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_14_3MHZ
#elif TIVA_XTAL_VALUE == 16000000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_16MHZ
#elif TIVA_XTAL_VALUE == 16384000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_16_3MHZ
#elif TIVA_XTAL_VALUE == 18000000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_18MHZ
#elif TIVA_XTAL_VALUE == 20000000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_20MHZ
#elif TIVA_XTAL_VALUE == 24000000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_24MHZ
#elif TIVA_XTAL_VALUE == 25000000
#define TIVA_XTAL_              SYSCTL_RCC_XTAL_25MHZ
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

#if (TIVA_OSCSRC == SYSCTL_RCC2_OSCSRC2_MO) && (TIVA_MOSC_ENABLE == FALSE)
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
