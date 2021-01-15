/*
    ChibiOS - Copyright (C) 2016 Stephane D'Alu

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
 * @file    WDTv1/hal_wdg_lld.c
 * @brief   NRF5 Watchdog Driver subsystem low level driver source template.
 *
 * @addtogroup WDG
 * @{
 */

#include "hal.h"

#if HAL_USE_WDG || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#define RELOAD_REQUEST_VALUE         0x6E524635

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

WDGDriver WDGD1;

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if WDG_USE_TIMEOUT_CALLBACK == TRUE
/**
 * @brief   Watchdog vector.
 * @details This interrupt is used when watchdog timeout.
 *
 * @note    Only 2 cycles at NRF5_LFCLK_FREQUENCY are available
 *          to they good bye.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector80) {

  OSAL_IRQ_PROLOGUE();
  osalSysLockFromISR();

  /* Notify */
  if (WDGD1.config->callback)
      WDGD1.config->callback();

  /* Wait for reboot */
  while (1) { /* */ }

  osalSysUnlockFromISR();
  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level WDG driver initialization.
 *
 * @notapi
 */
void wdg_lld_init(void) {
  WDGD1.state = WDG_STOP;
  WDGD1.wdt   = NRF_WDT;
}

/**
 * @brief   Configures and activates the WDG peripheral.
 *
 * @note    Once started there is no way out.
 *
 * @param[in] wdgp      pointer to the @p WDGDriver object
 *
 * @notapi
 */
void wdg_lld_start(WDGDriver *wdgp) {
  osalDbgAssert((wdgp->state == WDG_STOP),
		"This WDG driver cannot be restarted once activated");

  /* Generate interrupt on timeout */
#if WDG_USE_TIMEOUT_CALLBACK == TRUE
  wdgp->wdt->INTENSET = WDT_INTENSET_TIMEOUT_Msk;
#endif

  /* When to pause? (halt, sleep) */
  uint32_t config = 0;
  if (!wdgp->config->pause_on_sleep)
      config |= WDT_CONFIG_SLEEP_Msk;
  if (!wdgp->config->pause_on_halt)
      config |= WDT_CONFIG_HALT_Msk;
  wdgp->wdt->CONFIG = config;

  /* Timeout in milli-seconds */
  uint64_t tout = (NRF5_LFCLK_FREQUENCY * wdgp->config->timeout_ms / 1000) - 1;
  osalDbgAssert(tout <= 0xFFFFFFFF, "watchdog timout value exceeded");
  wdgp->wdt->CRV         = (uint32_t)tout;

  /* Reload request (using RR0) */
  wdgp->wdt->RREN        = WDT_RREN_RR0_Msk;

  /* Say your prayers, little one. */
  wdgp->wdt->TASKS_START = 1;
}

/**
 * @brief   Deactivates the WDG peripheral.
 *
 * @param[in] wdgp      pointer to the @p WDGDriver object
 *
 * @api
 */
void wdg_lld_stop(WDGDriver *wdgp) {
  (void)wdgp;
  osalDbgAssert(false, "This WDG driver cannot be stopped once activated");
}

/**
 * @brief   Reloads WDG's counter.
 *
 * @param[in] wdgp      pointer to the @p WDGDriver object
 *
 * @notapi
 */
void wdg_lld_reset(WDGDriver * wdgp) {
  wdgp->wdt->RR[0] = RELOAD_REQUEST_VALUE;
}

#endif /* HAL_USE_WDG */

/** @} */
