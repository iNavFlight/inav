/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

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
 * @file    SPC57EMxx_HSM/hal_lld.c
 * @brief   SPC57EMxx_HSM HAL subsystem low level driver source.
 *
 * @addtogroup HAL
 * @{
 */

#include "hal.h"

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

/**
 * @brief   PIT channel 0 interrupt handler.
 *
 * @isr
 */
OSAL_IRQ_HANDLER(vector8) {

  OSAL_IRQ_PROLOGUE();

  osalSysLockFromISR();
  osalOsTimerHandlerI();
  osalSysUnlockFromISR();

  /* Resets the PIT channel 0 IRQ flag.*/
  PIT_HSM.TIMER[0].TFLG.R = 1;

  OSAL_IRQ_EPILOGUE();
}

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level HAL driver initialization.
 *
 * @notapi
 */
void hal_lld_init(void) {
  uint32_t n;

  /* HSM PIT channel 0 initialization for Kernel ticks.*/
  n = 100000000 / OSAL_ST_FREQUENCY - 1;
  PIT_HSM.MCR.R            = 1;
  PIT_HSM.TIMER[0].LDVAL.R = n;
  PIT_HSM.TIMER[0].CVAL.R  = n;
  PIT_HSM.TIMER[0].TFLG.R  = 1;         /* Interrupt flag cleared.          */
  PIT_HSM.TIMER[0].TCTRL.R = 3;         /* Timer active, interrupt enabled. */

  /* HSM PIT interrupt vector enabled.*/
  INTC_PSR(8) = SPC5_PIT0_IRQ_PRIORITY;
}

/** @} */
