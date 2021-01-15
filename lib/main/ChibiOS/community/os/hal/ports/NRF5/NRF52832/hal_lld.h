/*
    Copyright (C) 2016 Stephane D'Alu

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
 * @file    NRF5/NRF52832/hal_lld.h
 * @brief   NRF52832 HAL subsystem low level driver header.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef HAL_LLD_H
#define HAL_LLD_H

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Platform identification
 * @{
 */
#define PLATFORM_NAME           "Nordic Semiconductor nRF52832"

/**
 * @name    Chip series
 */
#define NRF_SERIES              52

/**
 * @brief  Frequency value for the Low Frequency Clock
 */
#define NRF5_LFCLK_FREQUENCY    32768

/**
 * @brief  Frequency value for the High Frequency Clock
 */
#define NRF5_HFCLK_FREQUENCY    64000000

/**
 * @}
 */


/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Select source of High Frequency Clock (HFCLK)
 * @details Possible values for source are:
 *            0 : 64 MHz internal oscillator (HFINT)
 *            1 : 32 MHz external crystal oscillator (HFXO)
 */
#if !defined(NRF5_HFCLK_SOURCE) || defined(__DOXYGEN__)
#define NRF5_HFCLK_SOURCE             NRF5_HFCLK_HFINT
#endif

/**
 * @brief   Select source of Low Frequency Clock (LFCLK)
 * @details Possible values for source are:
 *            0 : RC oscillator
 *            1 : External crystal
 *            2 : Synthesized clock from High Frequency Clock (HFCLK)
 *          When crystal is not available it's preferable to use the
 *          internal RC oscillator that synthesizing the clock.
 */
#if !defined(NRF5_LFCLK_SOURCE) || defined(__DOXYGEN__)
#define NRF5_LFCLK_SOURCE             NRF5_LFCLK_RC
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if (NRF5_HFCLK_SOURCE < 0) || (NRF5_HFCLK_SOURCE > 1)
#error "Possible value for NRF5_HFCLK_SOURCE are HFINT=0, HFXO=1"
#endif

#if (NRF5_LFCLK_SOURCE < 0) || (NRF5_LFCLK_SOURCE > 2)
#error "Possible value for NRF5_LFCLK_SOURCE are 0=RC, 1=XTAL, 2=Synth"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/
#if 0 // moved to board.h
#define NRF5_HFCLK_HFINT             0
#define NRF5_HFCLK_HFXO              1

#define NRF5_LFCLK_RC                0
#define NRF5_LFCLK_XTAL              1
#define NRF5_LFCLK_SYNTH             2
#endif

#include "nvic.h"
#include "nrf52_isr.h"


#ifdef __cplusplus
extern "C" {
#endif
  void hal_lld_init(void);
#ifdef __cplusplus
}
#endif

#endif /* HAL_LLD_H */

/**
 * @}
 */
