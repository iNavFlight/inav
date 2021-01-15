/*
    Copyright (C) 2015 Fabio Utzig

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
 * @file    NRF5/NRF51822/hal_lld.h
 * @brief   NRF51822 HAL subsystem low level driver header.
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
#define PLATFORM_NAME           "Nordic Semiconductor nRF51822"

/**
 * @name    Chip series
 */
#define NRF_SERIES 51

/**
 * @brief  Frequency value for the Low Frequency Clock
 */
#define NRF5_LFCLK_FREQUENCY       32768

/**
 * @brief  Frequency value for the High Frequency Clock
 */
#define NRF5_HFCLK_FREQUENCY       16000000

/**
 * @}
 */



/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Select source of Low Frequency Clock (LFCLK)
 * @details Possible values for source are:
 *            0 : RC oscillator
 *            1 : External cristal
 *            2 : Synthetized clock from High Frequency Clock (HFCLK)
 *          When cristal is not available it's preferable to use the
 *          internal RC oscillator that synthezing the clock.
 */
#if !defined(NRF5_LFCLK_SOURCE) || defined(__DOXYGEN__)
#define NRF5_LFCLK_SOURCE             0
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

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

#include "nvic.h"
#include "nrf51_isr.h"

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
