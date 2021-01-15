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
 * @file    WDT/hal_wdg_lld.h
 * @brief   WDG Driver subsystem low level driver header.
 *
 * @addtogroup WDG
 * @{
 */

#ifndef _WDG_LLD_H_
#define _WDG_LLD_H_

#if HAL_USE_WDG || defined(__DOXYGEN__)

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
 * @brief   WDT driver enable switch.
 * @details If set to @p TRUE the support for WDT is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_WDG_USE_WDT) || defined(__DOXYGEN__)
#define TIVA_WDG_USE_WDT                  FALSE
#endif

/**
 * @brief   WDT interrupt priority level setting.
 */
#if !defined(TIVA_WDG_WDT_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_WDG_WDT_IRQ_PRIORITY          5
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if TIVA_WDG_USE_WDT0 && !TIVA_HAS_WDT0
#error "WDT0 not present in the selected device"
#endif

#if TIVA_WDG_USE_WDT1 && !TIVA_HAS_WDT1
#error "WDT1 not present in the selected device"
#endif

#if !TIVA_WDG_USE_WDT0 && !TIVA_WDG_USE_WDT1
#error "WDG driver activated but no WDT peripheral assigned"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an WDG driver.
 */
typedef struct WDGDriver WDGDriver;

/**
 * @brief   WDG timeout callback type.
 *
 * @param[in] wdgp      pointer to the @p WDGDriver object triggering the callback.
 */
typedef bool (*wdgcallback_t)(WDGDriver *wdgp);

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct
{
  /**
   * @brief   Interval value used by the WDT.
   */
  uint32_t                  load;
  /**
   * @brief   Timeout callback pointer.
   * @note    This callback is invoked on the first WDT timeout. If set to
   *          @p NULL then the callback is disabled.
   */
   wdgcallback_t            callback;
   /**
    * @brief   Test register configuration value.
    */
   uint16_t                 test;
} WDGConfig;

/**
 * @brief   Structure representing an WDG driver.
 */
struct WDGDriver
{
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
  uint32_t                  wdt;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

#if !TIVA_WDG_USE_WDT1
#define wdgTivaSyncWrite(wdt)
#endif

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if TIVA_WDG_USE_WDT0 && !defined(__DOXYGEN__)
extern WDGDriver WDGD1;
#endif

#if TIVA_WDG_USE_WDT1 && !defined(__DOXYGEN__)
extern WDGDriver WDGD2;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void wdg_lld_init(void);
  void wdg_lld_start(WDGDriver *wdgp);
  void wdg_lld_stop(WDGDriver *wdgp);
  void wdg_lld_reset(WDGDriver *wdgp);
#if TIVA_WDG_USE_WDT1
  void wdgTivaSyncWrite(WDGDriver *wdgp);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_WDG */

#endif /* _WDG_LLD_H_ */

/** @} */
