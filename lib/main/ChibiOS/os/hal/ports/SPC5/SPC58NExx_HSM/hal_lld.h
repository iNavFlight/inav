/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

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
 * @file    SPC58NExx_HSM/hal_lld.h
 * @brief   SPC58NExx_HSM HAL subsystem low level driver header.
 *
 * @addtogroup HAL
 * @{
 */

#ifndef HAL_LLD_H
#define HAL_LLD_H

#include "registers.h"
#include "spc5_registry.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Defines the support for realtime counters in the HAL.
 */
#define HAL_IMPLEMENTS_COUNTERS             FALSE

/**
 * @name    Platform identification
 * @{
 */
#define PLATFORM_NAME                       "SPC58NExx HSM"
/** @} */

/**
 * @name    Absolute Maximum Ratings
 * @{
 */
/**
 * @brief   Maximum XOSC clock frequency.
 */
#define SPC5_HSM_CLK_MAX                    100000000
/** @} */

/**
 * @name    Internal clock sources
 * @{
 */
#define SPC5_IRC_CLK                        16000000
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @brief   Disables the clocks initialization in the HAL.
 */
#if !defined(SPC5_NO_INIT) || defined(__DOXYGEN__)
#define SPC5_NO_INIT                        FALSE
#endif

/**
 * @brief   HSM PIT channel 0 IRQ priority.
 * @note    This PIT channel is allocated permanently for system tick
 *          generation.
 */
#if !defined(SPC5_PIT0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_PIT0_IRQ_PRIORITY              INTC_PSR_ENABLE(INTC_PSR_CORE4, 8)
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*-----------------------------------------*
 * Configuration-related checks.           *
 *-----------------------------------------*/

#if !defined(SPC58NExx_HSM_MCUCONF)
#error "Using a wrong mcuconf.h file, SPC58NExx_HSM_MCUCONF not defined"
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

#ifdef __cplusplus
extern "C" {
#endif
  void hal_lld_init(void);
#ifdef __cplusplus
}
#endif

#endif /* HAL_LLD_H */

/** @} */
