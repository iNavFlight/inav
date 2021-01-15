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
 * @file    WDGv1/hal_wdg_lld.h
 * @brief   WDG Driver subsystem low level driver header.
 *
 * @addtogroup WDG
 * @{
 */

#ifndef HAL_WDG_LLD_H
#define HAL_WDG_LLD_H

#if (HAL_USE_WDG == TRUE) || defined(__DOXYGEN__)

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
/*
 * WDG driver system settings.
 */
#define SAMA_WDG_IRQ_PRIORITY       4
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/
/**
  * @brief   Type of an WDG event.
  */
typedef enum {
  WDG_ERROR = 0,                          /** Watchdog fault error.          */
  WDG_UNDERFLOW = 1                       /** Watchdog underflow error.      */
} wdgevent_t;

/**
 * @brief   Type of a structure representing an WDG driver.
 */
typedef struct WDGDriver WDGDriver;

/**
  * @brief   Type of a generic WDG callback.
  */
typedef void (*wdgcb_t)(WDGDriver *wdgp, wdgevent_t event);

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief   Callback pointer.
   */
  wdgcb_t                   callback;
  /**
   * @brief   Configuration of the WDT modes.
   */
  uint32_t                  mode;
  /**
   * @brief   Configuration of the WDT counter.
   */
  uint32_t                  counter;
  /**
   * @brief   Configuration of the WDT delta.
   */
  uint32_t                  delta;
} WDGConfig;

/**
 * @brief   Structure representing an WDG driver.
 */
struct WDGDriver {
  /**
   * @brief   Driver state.
   */
  wdgstate_t                state;
  /**
   * @brief   Current configuration data.
   */
  const WDGConfig           *config;
  /* End of the mandatory fields.*/
  /**
   * @brief   Pointer to the WDT registers block.
   */
  Wdt                       *wdg;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
extern WDGDriver WDGD0;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void wdg_lld_init(void);
  void wdg_lld_start(WDGDriver *wdgp);
  void wdg_lld_stop(WDGDriver *wdgp);
  void wdg_lld_reset(WDGDriver *wdgp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_WDG == TRUE */

#endif /* HAL_WDG_LLD_H */

/** @} */
