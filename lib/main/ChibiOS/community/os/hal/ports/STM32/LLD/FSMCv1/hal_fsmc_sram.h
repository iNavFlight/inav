/*
    ChibiOS/HAL - Copyright (C) 2014 Uladzimir Pylinsky aka barthess

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
 * @file    hal_fsmc_sram.h
 * @brief   SRAM Driver subsystem low level driver header.
 *
 * @addtogroup SRAM
 * @{
 */

#ifndef HAL_FSMC_SRAM_H_
#define HAL_FSMC_SRAM_H_

#include "hal_fsmc.h"

#if (STM32_USE_FSMC_SRAM == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/
/**
 * @name    Configuration options
 * @{
 */

/**
 * @brief   SRAM driver enable switch.
 * @details If set to @p TRUE the support for SRAM1 is included.
 */
#if !defined(STM32_SRAM_USE_FSMC_SRAM1) || defined(__DOXYGEN__)
#define STM32_SRAM_USE_FSMC_SRAM1                  FALSE
#endif

/**
 * @brief   SRAM driver enable switch.
 * @details If set to @p TRUE the support for SRAM2 is included.
 */
#if !defined(STM32_SRAM_USE_FSMC_SRAM2) || defined(__DOXYGEN__)
#define STM32_SRAM_USE_FSMC_SRAM2                  FALSE
#endif

/**
 * @brief   SRAM driver enable switch.
 * @details If set to @p TRUE the support for SRAM3 is included.
 */
#if !defined(STM32_SRAM_USE_FSMC_SRAM3) || defined(__DOXYGEN__)
#define STM32_SRAM_USE_FSMC_SRAM3                  FALSE
#endif

/**
 * @brief   SRAM driver enable switch.
 * @details If set to @p TRUE the support for SRAM4 is included.
 */
#if !defined(STM32_SRAM_USE_FSMC_SRAM4) || defined(__DOXYGEN__)
#define STM32_SRAM_USE_FSMC_SRAM4                  FALSE
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !STM32_SRAM_USE_FSMC_SRAM1 && !STM32_SRAM_USE_FSMC_SRAM2 && \
    !STM32_SRAM_USE_FSMC_SRAM3 && !STM32_SRAM_USE_FSMC_SRAM4
#error "SRAM driver activated but no SRAM peripheral assigned"
#endif

#if (STM32_SRAM_USE_FSMC_SRAM1 || STM32_SRAM_USE_FSMC_SRAM2 || \
    STM32_SRAM_USE_FSMC_SRAM3 || STM32_SRAM_USE_FSMC_SRAM4) && !STM32_HAS_FSMC
#error "FSMC not present in the selected device"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  SRAM_UNINIT = 0,                   /**< Not initialized.                */
  SRAM_STOP = 1,                     /**< Stopped.                        */
  SRAM_READY = 2,                    /**< Ready.                          */
} sramstate_t;

/**
 * @brief   Type of a structure representing an NAND driver.
 */
typedef struct SRAMDriver SRAMDriver;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 * @note    Some bits in BCR register will be forced by driver.
 */
typedef struct {
  uint32_t  bcr;
  uint32_t  btr;
  uint32_t  bwtr;
} SRAMConfig;

/**
 * @brief   Structure representing an NAND driver.
 */
struct SRAMDriver {
  /**
   * @brief   Driver state.
   */
  sramstate_t               state;
  /**
   * @brief     Pointer to the FSMC SRAM registers block.
   */
  FSMC_SRAM_NOR_TypeDef     *sram;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if STM32_SRAM_USE_FSMC_SRAM1 && !defined(__DOXYGEN__)
extern SRAMDriver SRAMD1;
#endif

#if STM32_SRAM_USE_FSMC_SRAM2 && !defined(__DOXYGEN__)
extern SRAMDriver SRAMD2;
#endif

#if STM32_SRAM_USE_FSMC_SRAM3 && !defined(__DOXYGEN__)
extern SRAMDriver SRAMD3;
#endif

#if STM32_SRAM_USE_FSMC_SRAM4 && !defined(__DOXYGEN__)
extern SRAMDriver SRAMD4;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void fsmcSramInit(void);
  void fsmcSramStart(SRAMDriver *sramp, const SRAMConfig *cfgp);
  void fsmcSramStop(SRAMDriver *sramp);
#ifdef __cplusplus
}
#endif

#endif /* STM32_USE_FSMC_SRAM */

#endif /* HAL_FSMC_SRAM_H_ */

/** @} */
