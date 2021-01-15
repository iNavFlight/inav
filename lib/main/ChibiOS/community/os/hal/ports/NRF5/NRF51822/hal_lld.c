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
 * @file    NRF51/NRF51822/hal_lld.c
 * @brief   NRF51822 HAL Driver subsystem low level driver source.
 *
 * @addtogroup HAL
 * @{
 */

#include "hal.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level HAL driver initialization.
 *
 * @notapi
 */
void hal_lld_init(void)
{
  /* High frequency clock initialisation
   *  (If NRF5_XTAL_VALUE is not defined assume its an 16Mhz RC oscillator)
   */
  NRF_CLOCK->TASKS_HFCLKSTOP = 1;
#if defined(NRF5_XTAL_VALUE)
#if   NRF5_XTAL_VALUE == 16000000
  NRF_CLOCK->XTALFREQ = 0xFF;
#elif NRF5_XTAL_VALUE == 32000000
  NRF_CLOCK->XTALFREQ = 0x00;
#else
#error "Unsupported XTAL value"
#endif
#endif

  
  /* Low frequency clock initialisation
   * Clock is only started if st driver requires it
   */
  NRF_CLOCK->TASKS_LFCLKSTOP = 1;
  NRF_CLOCK->LFCLKSRC = NRF5_LFCLK_SOURCE;
  
#if (OSAL_ST_MODE != OSAL_ST_MODE_NONE) &&			\
    (NRF5_SYSTEM_TICKS == NRF5_SYSTEM_TICKS_AS_RTC)
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
#endif
  
  irqInit();
}

/**
 * @}
 */
