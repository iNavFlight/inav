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
/*
   SDRAM routines added by Nick Klimov aka progfin.
 */

/**
 * @file    hal_fsmc_sdram.h
 * @brief   SDRAM Driver subsystem low level driver header.
 *
 * @addtogroup SDRAM
 * @{
 */

#ifndef HAL_FMC_SDRAM_H_
#define HAL_FMC_SDRAM_H_

#if (defined(STM32F427xx) || defined(STM32F437xx) || \
     defined(STM32F429xx) || defined(STM32F439xx) || \
     defined(STM32F745xx) || defined(STM32F746xx) || \
     defined(STM32F756xx) || defined(STM32F767xx) || \
     defined(STM32F769xx) || defined(STM32F777xx) || \
     defined(STM32F779xx))

#include "hal_fsmc.h"

#if (STM32_USE_FSMC_SDRAM == TRUE) || defined(__DOXYGEN__)

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
 * @brief   SDRAM driver enable switch.
 * @details If set to @p TRUE the support for SDRAM1 is included.
 */
#if !defined(STM32_SDRAM_USE_FSMC_SDRAM1) || defined(__DOXYGEN__)
#define STM32_SDRAM_USE_FSMC_SDRAM1                  FALSE
#else
#define STM32_SDRAM1_MAP_BASE                        FSMC_Bank5_MAP_BASE
#endif

/**
 * @brief   SDRAM driver enable switch.
 * @details If set to @p TRUE the support for SDRAM2 is included.
 */
#if !defined(STM32_SDRAM_USE_FSMC_SDRAM2) || defined(__DOXYGEN__)
#define STM32_SDRAM_USE_FSMC_SDRAM2                  FALSE
#else
#define STM32_SDRAM2_MAP_BASE                        FSMC_Bank6_MAP_BASE
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !STM32_SDRAM_USE_FSMC_SDRAM1 && !STM32_SDRAM_USE_FSMC_SDRAM2
#error "SDRAM driver activated but no SDRAM peripheral assigned"
#endif

#if (STM32_SDRAM_USE_FSMC_SDRAM1 || STM32_SDRAM_USE_FSMC_SDRAM2) && !STM32_HAS_FSMC
#error "FMC not present in the selected device"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  SDRAM_UNINIT = 0,                   /**< Not initialized.                */
  SDRAM_STOP = 1,                     /**< Stopped.                        */
  SDRAM_READY = 2,                    /**< Ready.                          */
} sdramstate_t;

/**
 * @brief   Type of a structure representing an SDRAM driver.
 */
typedef struct SDRAMDriver SDRAMDriver;

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief     SDRAM control register.
   * @note      Its value will be used for both banks.
   */
  uint32_t      sdcr;

  /**
   * @brief     SDRAM timing register.
   * @note      Its value will be used for both banks.
   */
  uint32_t      sdtr;

  /**
   * @brief     SDRAM command mode register.
   * @note      Only its MRD and NRFS bits will be used.
   */
  uint32_t      sdcmr;

  /**
   * @brief     SDRAM refresh timer register.
   * @note      Only its COUNT bits will be used.
   */
  uint32_t      sdrtr;
} SDRAMConfig;

/**
 * @brief   Structure representing an SDRAM driver.
 */
struct SDRAMDriver {
  /**
   * @brief     Driver state.
   */
  sdramstate_t              state;
  /**
   * @brief     Pointer to the FMC SDRAM registers block.
   */
  FSMC_SDRAM_TypeDef        *sdram;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

extern SDRAMDriver SDRAMD;

#ifdef __cplusplus
extern "C" {
#endif
  void fsmcSdramInit(void);
  void fsmcSdramStart(SDRAMDriver *sdramp, const SDRAMConfig *cfgp);
  void fsmcSdramStop(SDRAMDriver *sdramp);
#ifdef __cplusplus
}
#endif

#endif /* STM32_USE_FSMC_SDRAM */

#endif /* STM32F427xx / STM32F429xx / STM32F437xx / STM32F439xx */

#endif /* HAL_FMC_SDRAM_H_ */

/** @} */
