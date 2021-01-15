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
 * @file    WDGv1/hal_wdg_lld.c
 * @brief   WDG Driver subsystem low level driver source.
 *
 * @addtogroup WDG
 * @{
 */

#include "hal.h"

#if (HAL_USE_WDG == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/
/**
 * @brief WDG driver identifier.
 */
WDGDriver WDGD0;

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/
/**
 * @brief   Local function that computes the period
 *          for WDT_MR_WDV and WDT_MR_VDD registers.
 *
 * @param[in] period    period to be computed.
 *
 * @notapi
 */
static uint32_t wdt_compute_period(uint32_t period) {

  uint32_t value;
  value = period * (SAMA_SLOW_CLK >> 7) / 1000;
  if (value > 0xfff)
    value = 0xfff;

  return value;
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/
/**
 * @brief   WDG IRQ handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(SAMA_WDG_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  /* Read status register. */
  uint32_t sr = WDT->WDT_SR;

  if (WDGD0.config->callback != NULL) {
    if (sr & WDT_SR_WDERR) {
      WDGD0.config->callback(&WDGD0, WDG_ERROR);
    }

    else
      WDGD0.config->callback(&WDGD0, WDG_UNDERFLOW);
  }

  aicAckInt();
  OSAL_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level WDG driver initialization.
 *
 * @notapi
 */
void wdg_lld_init(void) {

#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_WDT, SECURE_PER);
#endif

  WDGD0.state = WDG_STOP;
  WDGD0.wdg   = WDT;
}

/**
 * @brief   Configures and activates the WDT peripheral.
 *
 * @param[in] wdgp      pointer to the @p WDGDriver object
 *
 * @notapi
 */
void wdg_lld_start(WDGDriver *wdgp) {

  (void) wdgp;

  /* Read status register. */
  WDT->WDT_SR;

  /* Write configuration */
  WDT->WDT_MR = (wdgp->config->mode & ~(WDT_MR_WDDIS | WDT_MR_WDD_Msk | WDT_MR_WDV_Msk)) |
                WDT_MR_WDV(wdt_compute_period(wdgp->config->counter)) |
                WDT_MR_WDD(wdt_compute_period(wdgp->config->delta));

  aicSetSourcePriority(ID_WDT, SAMA_WDG_IRQ_PRIORITY);
  aicSetSourceHandler(ID_WDT, SAMA_WDG_HANDLER);
  aicEnableInt(ID_WDT);
}

/**
 * @brief   Deactivates the WDG peripheral.
 *
 * @param[in] wdgp      pointer to the @p WDGDriver object
 *
 * @notapi
 */
void wdg_lld_stop(WDGDriver *wdgp) {

  (void) wdgp;
  WDT->WDT_MR = WDT_MR_WDDIS;
}

/**
 * @brief   Reloads the WDG counter.
 *
 * @param[in] wdgp      pointer to the @p WDGDriver object
 *
 * @notapi
 */
void wdg_lld_reset(WDGDriver * wdgp) {

  (void) wdgp;
  WDT->WDT_CR = WDT_CR_KEY_PASSWD | WDT_CR_WDRSTT;
}

#endif /* HAL_USE_WDG == TRUE */

/** @} */
