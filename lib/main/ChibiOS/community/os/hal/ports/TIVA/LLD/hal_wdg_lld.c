/*
    Copyright (C) 2014..2016 Marco Veeneman

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
 * @file    TIVA/wdg_lld.c
 * @brief   WDG Driver subsystem low level driver source.
 *
 * @addtogroup WDG
 * @{
 */

#include "hal.h"

#if HAL_USE_WDG || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

#if TIVA_WDG_USE_WDT0 || defined(__DOXYGEN__)
WDGDriver WDGD1;
#endif /* TIVA_WDG_USE_WDT0 */

#if TIVA_WDG_USE_WDT1 || defined(__DOXYGEN__)
WDGDriver WDGD2;
#endif /* TIVA_WDG_USE_WDT1 */

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Shared IRQ handler.
 *
 * @param[in] wdgp      pointer to @p WDGDriver object.
 */
static void serve_interrupt(WDGDriver *wdgp)
{
  uint32_t mis;

  mis = wdgp->wdt->MIS;

  if (mis & MIS_WDTMIS) {
    /* Invoke callback, if any */
    if (wdgp->config->callback) {
      if (wdgp->config->callback(wdgp)) {
        /* Clear interrupt */
        wdgp->wdt->ICR = 0;
        wdgTivaSyncWrite(wdgp);
      }
    }
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if TIVA_WDG_USE_WDT0 || TIVA_WDG_USE_WDT1
/**
 * @brief   WDT0/WDT1 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(TIVA_WDT_HANDLER)
{
  OSAL_IRQ_PROLOGUE();

#if TIVA_WDG_USE_WDT0
  serve_interrupt(&WDGD1);
#endif

#if TIVA_WDG_USE_WDT1
  serve_interrupt(&WDGD2);
#endif

  OSAL_IRQ_EPILOGUE();
}
#endif /* TIVA_WDG_USE_WDT0 || TIVA_WDG_USE_WDT1 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level WDG driver initialization.
 *
 * @notapi
 */
void wdg_lld_init(void)
{
#if TIVA_WDG_USE_WDT0
  WDGD1.state = WDG_STOP;
  WDGD1.wdt = WDT0;
#endif /* TIVA_WDG_USE_WDT0 */

#if TIVA_WDG_USE_WDT1
  WDGD2.state = WDG_STOP;
  WDGD2.wdt = WDT1;
#endif /* TIVA_WDG_USE_WDT1 */

  /* The shared vector is initialized on driver initialization and never
     disabled because it is shared between the Watchdog Timers.*/
  nvicEnableVector(TIVA_WDT_NUMBER, TIVA_WDG_WDT_IRQ_PRIORITY);
}

/**
 * @brief   Configures and activates the WDG peripheral.
 *
 * @param[in] wdgp      pointer to the @p WDGDriver object
 *
 * @notapi
 */
void wdg_lld_start(WDGDriver *wdgp)
{
#if TIVA_WDG_USE_WDT0
  if (&WDGD1 == wdgp) {
    SYSCTL->RCGCWD |= (1 << 0);

    while (!(SYSCTL->PRWD & (1 << 0)))
      ;
  }
#endif /* TIVA_WDG_USE_WDT0 */

#if TIVA_WDG_USE_WDT1
  if (&WDGD2 == wdgp) {
    SYSCTL->RCGCWD |= (1 << 1);

    while (!(SYSCTL->PRWD & (1 << 1)))
      ;
  }
#endif /* TIVA_WDG_USE_WDT1 */

  wdgp->wdt->LOAD = wdgp->config->load;
  wdgTivaSyncWrite(wdgp);

  wdgp->wdt->TEST = wdgp->config->test;
  wdgTivaSyncWrite(wdgp);

  wdgp->wdt->CTL |= CTL_RESEN;
  wdgTivaSyncWrite(wdgp);

  wdgp->wdt->CTL |= CTL_INTEN;
  wdgTivaSyncWrite(wdgp);
}

/**
 * @brief   Deactivates the WDG peripheral.
 *
 * @param[in] wdgp      pointer to the @p WDGDriver object
 *
 * @api
 */
void wdg_lld_stop(WDGDriver *wdgp)
{
#if TIVA_WDG_USE_WDT0
  if (&WDGD1 == wdgp) {
    SYSCTL->SRWD |= (1 << 0);
    SYSCTL->SRWD &= ~(1 << 0);
  }
#endif /* TIVA_WDG_USE_WDT0 */

#if TIVA_WDG_USE_WDT1
  if (&WDGD2 == wdgp) {
    SYSCTL->SRWD |= (1 << 1);
    SYSCTL->SRWD &= ~(1 << 1);
  }
#endif /* TIVA_WDG_USE_WDT1 */
}

/**
 * @brief   Reloads WDG's counter.
 *
 * @param[in] wdgp      pointer to the @p WDGDriver object
 *
 * @notapi
 */
void wdg_lld_reset(WDGDriver *wdgp)
{
#if defined(TM4C123_USE_REVISION_6_FIX) || defined(TM4C123_USE_REVISION_7_FIX)

#if TIVA_WDG_USE_WDT1
  if (&WDGD2 == wdgp) {
    /* Number:      WDT#02
     * Description: Periodically reloading the count value into the Watchdog
     *              Timer Load (WDTLOAD) register of the Watchdog Timer 1
     *              module will not restart the count, as specified in the data
     *              sheet.
     * Workaround:  Disable the Watchdog Timer 1 module by setting the
     *              appropriate bit in the Watchdog Timer Software Reset (SRWD)
     *              register before reprogramming the counter.*/
    wdg_lld_stop(wdgp);
    wdg_lld_start(wdgp);
    return;
  }
#endif /* TIVA_WDG_USE_WDT1 */

#endif /* defined(TM4C123_USE_REVISION_6_FIX) ||
          defined(TM4C123_USE_REVISION_7_FIX) */
  wdgp->wdt->LOAD = wdgp->config->load;
  wdgTivaSyncWrite(wdgp);
}

#endif /* HAL_USE_WDG */

#if TIVA_WDG_USE_WDT1
/**
 * @brief   synchronize after a write to a watchdog register.
 *
 * @param[in] wdgp      pointer to the @p WDGDriver object.
 */
void wdgTivaSyncWrite(WDGDriver *wdgp)
{
  if (&WDGD2 == wdgp) {
    while (!(wdgp->wdt->CTL & CTL_WRC)) {
      ;
    }
  }
}
#endif /* TIVA_WDG_USE_WDT1 */

/** @} */
