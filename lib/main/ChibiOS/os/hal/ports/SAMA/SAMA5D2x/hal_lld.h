/*
    ChibiOS - Copyright (C) 2006..2018 Giovanni Di Sirio

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
 * @file    SAMA5D2x/hal_lld.h
 * @brief   SAMA5D2x HAL subsystem low level driver header.
 * @pre     This module requires the following macros to be defined in the
 *          @p board.h file:
 *          - SAMA_MOSCXTCLK.
 *          - SAMA_OSCXTCLK
 *          .
 *          One of the following macros must also be defined:
 *          - SAMA5D21, SAMA5D22, SAMA5D23, SAMA5D24, SAMA5D25, SAMA5D26,
 *            SAMA5D27, SAMA5D28.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef _HAL_LLD_H_
#define _HAL_LLD_H_

#include "sama_registry.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Platform identification macros
 * @{
 */
#if defined(SAMA5D21) || defined(__DOXYGEN__)
#define PLATFORM_NAME           "500Mhz processor with TrustZone, 16-bit DDR, BGA196"

#elif defined(SAMA5D22)
#define PLATFORM_NAME           "500Mhz processor with TrustZone, 16-bit DDR, CAN, BGA196"

#elif defined(SAMA5D23)
#define PLATFORM_NAME           "500Mhz processor with TrustZone, 16-bit DDR, CAN, Enhanced Security, BGA196"

#elif defined(SAMA5D24)
#define PLATFORM_NAME           "A500Mhz processor with TrustZone, 16/32-bit DDR, USB HSIC, BGA256"

#elif defined(SAMA5D25)
#define PLATFORM_NAME           "500Mhz processor with TrustZone, 16/32-bit DDR, BGA289"

#elif defined(SAMA5D26)
#define PLATFORM_NAME           "500Mhz processor with TrustZone, 16/32-bit DDR, CAN, BGA289"

#elif defined(SAMA5D27)
#define PLATFORM_NAME           "500Mhz processor with TrustZone, 16/32-bit DDR, CAN, Enhanced Security, BGA289"

#elif defined(SAMA5D28)
#define PLATFORM_NAME           "500Mhz processor with TrustZone, 16/32-bit DDR, CAN, Enhanced Security, BGA289, 'internal DDR"

#else
#error "SAMA5D2x device unsupported or not specified"
#endif
/** @} */

/**
 * @name    Absolute Maximum Ratings
 * @{
 */
/**
 * @brief   Maximum processor clock frequency.
 */
#define SAMA_PCK_MAX            504000000

/**
 * @brief   Minimum processor clock frequency.
 */
#define SAMA_PCK_MIN            250000000

/**
 * @brief   Maximum master clock frequency.
 */
#define SAMA_MCK_MAX            168000000

/**
 * @brief   Minimum master clock frequency.
 */
#define SAMA_MCK_MIN            125000000

/**
 * @brief   Maximum Main Crystal Oscillator clock frequency.
 */
#define SAMA_MOSCXTCLK_MAX      24000000

/**
 * @brief   Minimum Main Crystal Oscillator clock frequency.
 */
#define SAMA_MOSCXTCLK_MIN      8000000

/**
 * @brief   Maximum PLLs input clock frequency.
 */
#define SAMA_PLLIN_MAX          24000000

/**
 * @brief   Minimum PLLs input clock frequency.
 */
#define SAMA_PLLIN_MIN          800000

/**
 * @brief   Maximum PLL output clock frequency.
 */
#define SAMA_PLLOUT_MAX         1200000000

/**
 * @brief   Minimum PLL output clock frequency.
 */
#define SAMA_PLLOUT_MIN         600000000
/** @} */

/**
 * @name    Internal clock sources
 * @{
 */
#define SAMA_MOSCRCCLK          12000000    /**< RC Main oscillator clock. */
#define SAMA_OSCRCCLK           32000       /**< RC Slow oscillator clock. */
/** @} */

/**
 * @name    SCK_CR register bits definitions
 * @{
 */                                           
#define SAMA_OSC_OSCRC          (0 << 3)    /**< Slow Clock source MOSCRC. */
#define SAMA_OSC_OSCXT          (1 << 3)    /**< Slow Clock source MOSCXT. */
/** @} */

/**
 * @name    PCM_MOR register bits definitions
 * @{
 */                                           
#define SAMA_MOSC_MOSCRC        (0 << 24)   /**< Main Clock source MOSCRC. */
#define SAMA_MOSC_MOSCXT        (1 << 24)   /**< Main Clock source MOSCXT. */
/** @} */

/**
 * @name    PCM_MCR register bits definitions
 * @{
 */                                           
#define SAMA_MCK_SLOW_CLK       (0 << 0)    /**< MCK source is Slow Clock. */
#define SAMA_MCK_MAIN_CLK       (1 << 0)    /**< MCK source is Main Clock. */
#define SAMA_MCK_PLLA_CLK       (2 << 0)    /**< MCK source is PLLA Clock. */
#define SAMA_MCK_UPLL_CLK       (3 << 0)    /**< MCK source is UPLL Clock. */

#define SAMA_MCK_PRE_DIV1       (0 << 4)    /**< MCK not prescaled.        */
#define SAMA_MCK_PRE_DIV2       (1 << 4)    /**< MCK prescaled by 2.       */
#define SAMA_MCK_PRE_DIV4       (2 << 4)    /**< MCK prescaled by 4.       */
#define SAMA_MCK_PRE_DIV8       (3 << 4)    /**< MCK prescaled by 8.       */
#define SAMA_MCK_PRE_DIV16      (4 << 4)    /**< MCK prescaled by 16.      */
#define SAMA_MCK_PRE_DIV32      (5 << 4)    /**< MCK prescaled by 32.      */
#define SAMA_MCK_PRE_DIV64      (6 << 4)    /**< MCK prescaled by 64.      */

#define SAMA_MCK_MDIV_DIV1      (0 << 8)    /**< MCK is not divided.       */
#define SAMA_MCK_MDIV_DIV2      (1 << 8)    /**< MCK is divided by 2.      */
#define SAMA_MCK_MDIV_DIV3      (3 << 8)    /**< MCK is divided by 3.      */
#define SAMA_MCK_MDIV_DIV4      (2 << 8)    /**< MCK is divided by 4.      */

#define SAMA_MCK_PLLADIV2       (1 << 12)   /**< PLLA is divided by 2.     */

/**
 * @name    PCM_PCR register bits definitions
 * @{
 */
#define   SAMA_GCLK_SLOW_CLK    (0x0u << 8) /**< GCLK Slow clock is selected      */
#define   SAMA_GCLK_MAIN_CLK    (0x1u << 8) /**< GCLK GMain clock is selected     */
#define   SAMA_GCLK_PLLA_CLK    (0x2u << 8) /**< GCLK  PLLACK is selected         */
#define   SAMA_GCLKUPLL_CLK     (0x3u << 8) /**< GCLK UPLL Clock is selected      */
#define   SAMA_GCLK_MCK_CLK     (0x4u << 8) /**< GCLK Master Clock is selected    */
#define   SAMA_GCLK_AUDIO_CLK   (0x5u << 8) /**< GCLK Audio PLL clock is selected */

/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   Defines if the code is running in the secure side of the ARM Trust
 *          Zone. It must be @p TRUE whenever the code is compiled for the
 *          secure side.
 */
#if !defined(SAMA_HAL_IS_SECURE) || defined(__DOXYGEN__)
#define SAMA_HAL_IS_SECURE                  TRUE
#endif

/**
 * @brief   Disables the PMC initialization in the HAL.
 */
#if !defined(SAMA_NO_INIT) || defined(__DOXYGEN__)
#define SAMA_NO_INIT                        FALSE
#endif

/**
 * @brief   Enables or disables the MOSCRC clock source.
 */
#if !defined(SAMA_MOSCRC_ENABLED) || defined(__DOXYGEN__)
#define SAMA_MOSCRC_ENABLED                 TRUE
#endif

/**
 * @brief   Enables or disables the MOSCXT clock source.
 */
#if !defined(SAMA_MOSCXT_ENABLED) || defined(__DOXYGEN__)
#define SAMA_MOSCXT_ENABLED                 FALSE
#endif

/**
 * @brief   Main clock source selection.
 */
#if !defined(SAMA_MOSC_SEL) || defined(__DOXYGEN__)
#define SAMA_MOSC_SEL                       SAMA_MOSC_MOSCRC
#endif

/**
 * @brief   Slow clock source selection.
 */
#if !defined(SAMA_OSC_SEL) || defined(__DOXYGEN__)
#define SAMA_OSC_SEL                        SAMA_OSC_OSCRC
#endif

/**
 * @brief   Master clock source selection.
 */
#if !defined(SAMA_MCK_SEL) || defined(__DOXYGEN__)
#define SAMA_MCK_SEL                        SAMA_MCK_PLLA_CLK
#endif

/**
 * @brief   Master clock prescaler.
 */
#if !defined(SAMA_MCK_PRES_VALUE) || defined(__DOXYGEN__)
#define SAMA_MCK_PRES_VALUE                 1
#endif

/**
 * @brief   Master clock divider.
 */
#if !defined(SAMA_MCK_MDIV_VALUE) || defined(__DOXYGEN__)
#define SAMA_MCK_MDIV_VALUE                 3
#endif

/**
 * @brief   PLLA clock multiplier.
 */
#if !defined(SAMA_PLLA_MUL_VALUE) || defined(__DOXYGEN__)
#define SAMA_PLLA_MUL_VALUE                 83
#endif

/**
 * @brief   PLLADIV2 clock divider.
 */
#if !defined(SAMA_PLLADIV2_EN) || defined(__DOXYGEN__)
#define SAMA_PLLADIV2_EN                    TRUE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*
 * Configuration-related checks.
 */
#if !defined(SAMA5D2x_MCUCONF)
#error "Using a wrong mcuconf.h file, SAMA5D2x_MCUCONF not defined"
#endif

/**
 * @brief   Slow clock value.
 */
/* Main oscillator is fed by internal RC. */
#if (SAMA_OSC_SEL == SAMA_OSC_OSCRC) || defined(__DOXYGEN__)
#define SAMA_SLOW_CLK                       SAMA_OSCRCCLK
#elif (SAMA_OSC_SEL == SAMA_OSC_OSCXT)
#define SAMA_SLOW_CLK                       SAMA_OSCXTCLK
#else
#error "Wrong SAMA_OSC_SEL value."
#endif

/**
 * @brief   MAIN clock value.
 */
/* Main oscillator is fed by internal RC. */
#if (SAMA_MOSC_SEL == SAMA_MOSC_MOSCRC) || defined(__DOXYGEN__)
#if !SAMA_MOSCRC_ENABLED
#error "Internal RC oscillator disabled (required by SAMA_MOSC_SEL)."
#endif
#define SAMA_MAIN_CLK                       SAMA_MOSCRCCLK

/* Main oscillator is fed by external XTAL. */  
#elif (SAMA_MOSC_SEL == SAMA_MOSC_MOSCXT)
#if !SAMA_MOSCXT_ENABLED
#error "External crystal oscillator disabled (required by SAMA_MOSC_SEL)."
#endif
#define SAMA_MAIN_CLK                       SAMA_MOSCXTCLK
/* Checks on external crystal range. */
#if (SAMA_MOSCXTCLK > SAMA_MOSCXTCLK_MAX) ||                                \
    (SAMA_MOSCXTCLK < SAMA_MOSCXTCLK_MIN)
#error "External crystal oscillator out of range."
#endif

/* Unallowed condition. */ 
#else
#error "Wrong SAMA_MOSC_SEL value."  
#endif /* SAMA_MOSCXTCLK */

#if (SAMA_MCK_SEL == SAMA_MCK_PLLA_CLK) || defined(__DOXYGEN__)
#define SAMA_ACTIVATE_PLLA                  TRUE
#else
#define SAMA_ACTIVATE_PLLA                  FALSE
#endif

/**
 * @brief   PLLAMUL field.
 */
#if ((SAMA_PLLA_MUL_VALUE >= 1) && (SAMA_PLLA_MUL_VALUE <= 127)) ||          \
    defined(__DOXYGEN__)
#define SAMA_PLLA_MUL                       ((SAMA_PLLA_MUL_VALUE - 1) << 18)
#else
#error "invalid SAMA_PLLA_MUL_VALUE value specified"
#endif

/**
 * @brief   PLLA input clock frequency.
 * @todo    Consider to add DIVA to this. On SAMA5D27 DIVA is a nonsense since
 *          it could be only 1 or 0 whereas 0 means PLLA disabled. This could
 *          be useful for other chip belonging to this family
 */
#define SAMA_PLLACLKIN                      SAMA_MAIN_CLK

/* PLLA input frequency range check.*/
#if (SAMA_PLLACLKIN < SAMA_PLLIN_MIN) || (SAMA_PLLACLKIN > SAMA_PLLIN_MAX)
#error "SAMA_PLLACLKIN out of range"
#endif

/**
 * @brief   PLLA output clock frequency.
 */
#define SAMA_PLLACLKOUT                     (SAMA_MAIN_CLK * SAMA_PLLA_MUL_VALUE)

/* PLLA output frequency range check.*/
#if (SAMA_PLLACLKOUT < SAMA_PLLOUT_MIN) || (SAMA_PLLACLKOUT > SAMA_PLLOUT_MAX)
#error "SAMA_PLLACLKOUT out of range"
#endif

/**
 * @brief   PLLADIV2 divider value.
 */
#if SAMA_PLLADIV2_EN || defined(__DOXYGEN__)
#define SAMA_PLLADIV2                       SAMA_MCK_PLLADIV2
#else
#define SAMA_PLLADIV                        0
#endif

/**
 * @brief   Master Clock prescaler.
 */
#if (SAMA_MCK_PRES_VALUE == 1) || defined(__DOXYGEN__)
#define SAMA_MCK_PRES                       SAMA_MCK_PRE_DIV1
#elif (SAMA_MCK_PRES_VALUE == 2)
#define SAMA_MCK_PRES                       SAMA_MCK_PRE_DIV2
#elif (SAMA_MCK_PRES_VALUE == 4)
#define SAMA_MCK_PRES                       SAMA_MCK_PRE_DIV4
#elif (SAMA_MCK_PRES_VALUE == 8)
#define SAMA_MCK_PRES                       SAMA_MCK_PRE_DIV8
#elif (SAMA_MCK_PRES_VALUE == 16)
#define SAMA_MCK_PRES                       SAMA_MCK_PRE_DIV16
#elif (SAMA_MCK_PRES_VALUE == 32)
#define SAMA_MCK_PRES                       SAMA_MCK_PRE_DIV32
#elif (SAMA_MCK_PRES_VALUE == 64)
#define SAMA_MCK_PRES                       SAMA_MCK_PRE_DIV64
#else
#error "Wrong SAMA_MCK_PRES_VALUE."
#endif

/**
 * @brief   Master Clock divider.
 */
#if (SAMA_MCK_MDIV_VALUE == 1) || defined(__DOXYGEN__)
#define SAMA_MCK_MDIV                       SAMA_MCK_MDIV_DIV1
#elif (SAMA_MCK_MDIV_VALUE == 2)
#define SAMA_MCK_MDIV                       SAMA_MCK_MDIV_DIV2
#elif (SAMA_MCK_MDIV_VALUE == 3)
#define SAMA_MCK_MDIV                       SAMA_MCK_MDIV_DIV3
#elif (SAMA_MCK_MDIV_VALUE == 4)
#define SAMA_MCK_MDIV                       SAMA_MCK_MDIV_DIV4
#else
#error "Wrong SAMA_MCK_MDIV_VALUE."
#endif

/* Check on MDIV and PLLADIV2 value. */
#if (SAMA_MCK_MDIV == SAMA_MCK_MDIV_DIV3) && !SAMA_PLLADIV2_EN
#error "PLLADIV2 must be always enabled when Main Clock Divider is 3"
#endif

/**
 * @brief   Processor Clock frequency.
 */
#if (SAMA_MCK_SEL == SAMA_MCK_SLOW_CLK) || defined(__DOXYGEN__)
#define SAMA_PCK                            (SAMA_SLOW_CLK / SAMA_MCK_PRES_VALUE)
#elif (SAMA_MCK_SEL == SAMA_MCK_MAIN_CLK)
#define SAMA_PCK                            (SAMA_MAIN_CLK / SAMA_MCK_PRES_VALUE)
#elif (SAMA_MCK_SEL == SAMA_MCK_PLLA_CLK)
#if SAMA_PLLADIV2_EN
#define SAMA_PCK                            (SAMA_PLLACLKOUT / SAMA_MCK_PRES_VALUE / 2)
#else
#define SAMA_PCK                            (SAMA_PLLACLKOUT / SAMA_MCK_PRES_VALUE)
#endif
#elif (SAMA_MCK_SEL == SAMA_MCK_UPLL_CLK)
#error "UPLL still unsupported"
#else
#error "Wrong SAMA_MCK_SEL."
#endif

/**
 * @brief   Master Clock frequency.
 */
#define SAMA_MCK                            (SAMA_PCK / SAMA_MCK_MDIV_VALUE)

/* Checks on Processor Clock crystal range. */
#if (SAMA_PCK > SAMA_PCK_MAX) || (SAMA_PCK < SAMA_PCK_MIN)
#error "Processor clock frequency out of range."
#endif

/* Checks on Master Clock crystal range. */
#if (SAMA_MCK > SAMA_MCK_MAX) || (SAMA_MCK < SAMA_MCK_MIN)
#error "Master clock frequency out of range."
#define VALUE(x) #x
#define VAR_NAME_VALUE(var) #var "="  VALUE(var)
#pragma message(VAR_NAME_VALUE(SAMA_MCK))

#endif

/**
 * @brief   Matrix H64H32 clock ratio.
 */
#if ((SAMA_H64MX_H32MX_RATIO == 2) || defined(__DOXYGEN__))
#define SAMA_H64MX_H32MX_DIV  PMC_MCKR_H32MXDIV_H32MXDIV2
#elif (SAMA_H64MX_H32MX_RATIO == 1)
#define SAMA_H64MX_H32MX_DIV  PMC_MCKR_H32MXDIV_H32MXDIV1
#if (SAMA_MCK > 83000000)
#error "Invalid H32MXCLK. MCK > 83MHz wants SAMA_H64MX_H32MX_RATIO == 2"
#endif
#else
#error "H64MX H32MX clock ratio out of range."
#endif

/**
 * @brief   UARTx clock.
 * TODO: Work only with PERIPH CLOCK
 */
#define SAMA_UART0CLK                       (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)
#define SAMA_UART1CLK                       (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)
#define SAMA_UART2CLK                       (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)
#define SAMA_UART3CLK                       (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)
#define SAMA_UART4CLK                       (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)

/**
 * @brief   FLEXCOMx clock.
 * TODO: Work only with PERIPH CLOCK
 */
#define SAMA_FLEXCOM0CLK                    (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)
#define SAMA_FLEXCOM1CLK                    (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)
#define SAMA_FLEXCOM2CLK                    (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)
#define SAMA_FLEXCOM3CLK                    (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)
#define SAMA_FLEXCOM4CLK                    (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)

/**
 * @brief   TCx clock.
 * TODO: Work only with PERIPH CLOCK
 */
#define SAMA_TC0CLK                         (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)
#define SAMA_TC1CLK                         (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)

/**
 * @brief   GMAC0 clock.
 * TODO: Work only with PERIPH CLOCK
 */
#define SAMA_GMAC0CLK                       (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)

/**
 * @brief   TWIHSx clock.
 * TODO: Work only with PERIPH CLOCK
 */
#define SAMA_TWIHSxCLK                      (SAMA_MCK / SAMA_H64MX_H32MX_RATIO)

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
#include "sama_pmc.h"
#include "sama_aic.h"
#include "sama_matrix.h"
#include "sama_xdmac.h" 
#include "sama_cache.h"
#include "sama_tc_lld.h"
#include "sama_lcdc.h"
#include "sama_secumod.h"
#include "sama_onewire.h"
#include "sama_classd.h"
#include "sama_rstc.h"

#ifdef __cplusplus
extern "C" {
#endif
  void hal_lld_init(void);
  void sama_clock_init(void);
#ifdef __cplusplus
}
#endif

#endif /* _HAL_LLD_H_ */

/** @} */
