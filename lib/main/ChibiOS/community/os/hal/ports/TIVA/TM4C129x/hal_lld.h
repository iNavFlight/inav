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
 * @file    TIVA/TM4C129x/hal_lld.h
 * @brief   TM4C129x HAL subsystem low level driver header.
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
 
#define PLATFORM_NAME           "Tiva C Series TM4C129x"

/**
 * @}
 */

/**
 * @name    RIS register bits definitions
 * @{
 */

#define SYSCTL_RIS_PLLLRIS	            (1 << 6)
#define SYSCTL_RIS_MOSCPUPRIS           (1 << 8)

/**
 * @}
 */

/**
 * @name    MOSCCTL register bits definitions
 * @{
 */

#define MOSCCTL_CVAL                (1 << 0)
#define MOSCCTL_MOSCIM              (1 << 1)
#define MOSCCTL_NOXTAL              (1 << 2)
#define MOSCCTL_PWRDN               (1 << 3)
#define MOSCCTL_OSCRNG              (1 << 4)

/**
 * @}
 */

/**
 * @name    RSCLKCFG register bits definitions
 * @{
 */

#define RSCLKCFG_PSYSDIV_bm         (0xfffff << 0)
#define RSCLKCFG_OSYSDIV_bm         (0xfffff << 10

#define RSCLKCFG_OSCSRC_bm          (0xff << 20)
#define RSCLKCFG_OSCSRC_PIOSC       (0 << 20)
#define RSCLKCFG_OSCSRC_LFIOSC      (0x02 << 20)
#define RSCLKCFG_OSCSRC_MOSC        (0x03 << 20)
#define RSCLKCFG_OSCSRC_RTCOSC      (0x04 << 20)

#define RSCLKCFG_PLLSRC_bm          (0xff << 24)
#define RSCLKCFG_PLLSRC_PIOSC       (0 << 24)
#define RSCLKCFG_PLLSRC_MOSC        (0x03 << 24)

#define RSCLKCFG_USEPLL             (1 << 28)

#define RSCLKCFG_ACG                (1 << 29)

#define RSCLKCFG_NEWFREQ            (1 << 30)

#define RSCLKCFG_MEMTIMU            (1 << 31)

/**
 * @}
 */

/**
 * @name    PLLFREQ0 register bits definitions
 * The PLL frequency can be calculated using the following equation:
 * fVCO = (fIN * MDIV)
 * where
 * fIN = fXTAL/(Q+1)(N+1) or fPIOSC/(Q+1)(N+1)
 * MDIV = MINT + (MFRAC / 1024)
 * The Q and N values are programmed in the PLLFREQ1 register. Note that to reduce jitter, MFRAC
 * should be programmed to 0x0.
 * @{
 */

#define PLLFREQ0_MINT_bm            (0xfffff << 0)
#define PLLFREQ0_MFRAC_bm           (0xfffff << 10)
#define PLLFREQ0_PLLPWR             (1 << 23)

/**
 * @}
 */

/**
 * @name    PLLFREQ1 register bits definitions
 * @{
 */

#define PLLFREQ1_N_bm               (0x7ff << 0)
#define PLLFREQ1_Q_bm               (0x7ff << 8)

/**
 * @}
 */

/**
 * @name    MEMTIM0 register bits definitions
 * @{
 */

#define MEMTIM0_FWS_bm              (0xff << 0)
#define MEMTIM0_FWS_0               (0x00 << 0)
#define MEMTIM0_FWS_1               (0x01 << 0)
#define MEMTIM0_FWS_2               (0x02 << 0)
#define MEMTIM0_FWS_3               (0x03 << 0)
#define MEMTIM0_FWS_4               (0x04 << 0)
#define MEMTIM0_FWS_5               (0x05 << 0)
#define MEMTIM0_FWS_6               (0x06 << 0)
#define MEMTIM0_FWS_7               (0x07 << 0)

#define MEMTIM0_FBCE                (1 << 5)

#define MEMTIM0_FBCHT_bm            (0xff << 6)
#define MEMTIM0_FBCHT_0_5           (0x00 << 6)
#define MEMTIM0_FBCHT_1             (0x01 << 6)
#define MEMTIM0_FBCHT_1_5           (0x02 << 6)
#define MEMTIM0_FBCHT_2             (0x03 << 6)
#define MEMTIM0_FBCHT_2_5           (0x04 << 6)
#define MEMTIM0_FBCHT_3             (0x05 << 6)
#define MEMTIM0_FBCHT_3_5           (0x06 << 6)
#define MEMTIM0_FBCHT_4             (0x07 << 6)
#define MEMTIM0_FBCHT_4_5           (0x08 << 6)

#define MEMTIM0_EWS_bm              (0xff << 16)
#define MEMTIM0_EWS_0               (0x00 << 16)
#define MEMTIM0_EWS_1               (0x01 << 16)
#define MEMTIM0_EWS_2               (0x02 << 16)
#define MEMTIM0_EWS_3               (0x03 << 16)
#define MEMTIM0_EWS_4               (0x04 << 16)
#define MEMTIM0_EWS_5               (0x05 << 16)
#define MEMTIM0_EWS_6               (0x06 << 16)
#define MEMTIM0_EWS_7               (0x07 << 16)

#define MEMTIM0_EBCE                (1 << 21)

#define MEMTIM0_EBCHT_bm            (0xff << 22)
#define MEMTIM0_EBCHT_0_5           (0x00 << 22)
#define MEMTIM0_EBCHT_1             (0x01 << 22)
#define MEMTIM0_EBCHT_1_5           (0x02 << 22)
#define MEMTIM0_EBCHT_2             (0x03 << 22)
#define MEMTIM0_EBCHT_2_5           (0x04 << 22)
#define MEMTIM0_EBCHT_3             (0x05 << 22)
#define MEMTIM0_EBCHT_3_5           (0x06 << 22)
#define MEMTIM0_EBCHT_4             (0x07 << 22)
#define MEMTIM0_EBCHT_4_5           (0x08 << 22)

// XXX: what is this?
#define MEMTIM0_MB1          0x00100010  // MB1 = Must be one

/**
 * @}
 */

/**
 * @name    PLLSTAT register bits definitions
 * @{
 */

#define PLLSTAT_LOCK                (1 << 0)

/**
 * @}
 */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

#if !defined(TIVA_MOSC_SINGLE_ENDED)
#define TIVA_MOSC_SINGLE_ENDED          FALSE
#endif

#if !defined(TIVA_RSCLKCFG_OSCSRC)
#define TIVA_RSCLKCFG_OSCSRC            RSCLKCFG_OSCSRC_MOSC
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*
 * Configuration-related checks.
 */
#if !defined(TM4C129x_MCUCONF)
#error "Using a wrong mcuconf.h file, TM4C129x_MCUCONF not defined"
#endif

/*
 * Oscillator-related checks.
 */
#if !(TIVA_RSCLKCFG_OSCSRC == RSCLKCFG_OSCSRC_PIOSC) &&                       \
    !(TIVA_RSCLKCFG_OSCSRC == RSCLKCFG_OSCSRC_LFIOSC) &&                      \
    !(TIVA_RSCLKCFG_OSCSRC == RSCLKCFG_OSCSRC_MOSC) &&                        \
    !(TIVA_RSCLKCFG_OSCSRC == RSCLKCFG_OSCSRC_RTCOSC)
#error "Invalid value for TIVA_RSCLKCFG_OSCSRC defined"
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

/*
#if TIVA_MOSC_ENABLE == TRUE
#define TIVA_MOSCDIS            (0 << 0)
#define TIVA_XTAL               TIVA_XTAL_
#elif TIVA_MOSC_ENABLE == FALSE
#define TIVA_MOSCDIS            (1 << 0)
#define TIVA_XTAL               0
#else
#error "Invalid value for TIVA_MOSC_ENABLE defined"
#endif

#if TIVA_DIV400_ENABLE == TRUE
#define TIVA_DIV400             (1 << 30)
#elif TIVA_DIV400_ENABLE == FALSE
#define TIVA_DIV400             (0 << 30)
#else
#error "Invalid value for TIVA_DIV400_ENABLE defined"
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

#if TIVA_BYPASS_ENABLE == TRUE
#define TIVA_SRC                16000000
#define TIVA_BYPASS             (1 << 11)
#elif TIVA_BYPASS_ENABLE == FALSE
#define TIVA_SRC                (200000000 + ((TIVA_DIV400 >> 30) * 200000000))
#define TIVA_BYPASS             (0 << 11)
#else
#error "Invalid value for TIVA_BYPASS_ENABLE defined"
#endif

#if (TIVA_OSCSRC == TIVA_RCC_OSCSRC_MOSC) && (TIVA_MOSC_ENABLE == FALSE)
#error "Main Oscillator selected but not enabled"
#endif
*/
/*
 * System Clock calculation
 * TODO: dynamic TIVA_SYSCLK value
 */
#define TIVA_SYSCLK             120000000

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
