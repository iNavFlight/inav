/*
    Copyright (C) 2016 Stephane D'Alu

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
 * @file    NRF5/NRF52832/hal_lld.c
 * @brief   NRF52832 HAL Driver subsystem low level driver source.
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
  /* High frequency clock initialization
   */
  NRF_CLOCK->TASKS_HFCLKSTOP = 1;

#if !defined(NRF5_XTAL_VALUE) && (NRF5_XTAL_VALUE != 32000000)
#error "A 32Mhz crystal is mandatory on nRF52 boards."
#endif

#if (NRF5_HFCLK_SOURCE == NRF5_HFCLK_HFXO)
  NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_HFCLKSTART = 1;
  while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
#endif
  
  /* Low frequency clock initialization
   */
#if (OSAL_ST_MODE != OSAL_ST_MODE_NONE)
#if (NRF5_ST_USE_RTC0 || NRF5_ST_USE_RTC1) && \
	(NRF5_LFCLK_SOURCE == NRF5_LFCLK_RC)
#error "A NRF5_SYSTEM_TICKS_AS_RTC requires LFCLK clock to be started."
#endif
#endif

  NRF_CLOCK->TASKS_LFCLKSTOP = 1;

#if (NRF5_LFCLK_SOURCE != NRF5_LFCLK_RC)
  NRF_CLOCK->LFCLKSRC = NRF5_LFCLK_SOURCE;

  NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
  NRF_CLOCK->TASKS_LFCLKSTART = 1;
  while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0);
#endif
  
  irqInit();
}

/**
 * @}
 */
