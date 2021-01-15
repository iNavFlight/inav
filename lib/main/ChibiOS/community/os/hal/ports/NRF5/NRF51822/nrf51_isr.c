/*
    Copyright (C) 2018 Konstantin Oblaukhov
    Copyright (C) 2015 Stephen Caudle

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
 * @file    NRF51822/nrf51_isr.c
 * @brief   NRF51822 ISR handler code.
 *
 * @addtogroup NRF51822_ISR
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
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/**
 * @brief   GPIOTE interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(Vector58) {

  OSAL_IRQ_PROLOGUE();
  
  for (int ch = 0; ch < NRF5_GPIOTE_NUM_CHANNELS; ch++)
  {
    if (NRF_GPIOTE->EVENTS_IN[ch])
    {
        NRF_GPIOTE->EVENTS_IN[ch] = 0;
        _pal_isr_code(ch);
    }
  }

  OSAL_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/
/**
 * @brief   Enables IRQ sources.
 *
 * @notapi
 */
void irqInit(void) {

#if HAL_USE_PAL
  nvicEnableVector(GPIOTE_IRQn, NRF5_IRQ_GPIOTE_PRIORITY);
#endif
}

/**
 * @brief   Disables IRQ sources.
 *
 * @notapi
 */
void irqDeinit(void) {

#if HAL_USE_PAL
  nvicDisableVector(GPIOTE_IRQn);
#endif
}

/** @} */
