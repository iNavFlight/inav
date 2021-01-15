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
 * @file    common/ARMCMx/nvic.c
 * @brief   Cortex-Mx NVIC support code.
 *
 * @addtogroup COMMON_ARMCMx_NVIC
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
/* Driver local types.                                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Sets the priority of an interrupt handler and enables it.
 *
 * @param[in] n         the interrupt number
 * @param[in] prio      the interrupt priority
 */
void nvicEnableVector(uint32_t n, uint32_t prio) {

#if defined(__CORE_CM0_H_GENERIC)
  NVIC->IP[_IP_IDX(n)] = (NVIC->IP[_IP_IDX(n)] & ~(0xFFU << _BIT_SHIFT(n))) |
                         (NVIC_PRIORITY_MASK(prio) << _BIT_SHIFT(n));
#else
  NVIC->IP[n] = NVIC_PRIORITY_MASK(prio);
#endif
  NVIC->ICPR[n >> 5U] = 1U << (n & 0x1FU);
  NVIC->ISER[n >> 5U] = 1U << (n & 0x1FU);
}

/**
 * @brief   Disables an interrupt handler.
 *
 * @param[in] n         the interrupt number
 */
void nvicDisableVector(uint32_t n) {

  NVIC->ICER[n >> 5U] = 1U << (n & 0x1FU);
#if defined(__CORE_CM0_H_GENERIC)
  NVIC->IP[_IP_IDX(n)] = NVIC->IP[_IP_IDX(n)] & ~(0xFFU << _BIT_SHIFT(n));
#else
  NVIC->IP[n] = 0U;
#endif
}

/**
 * @brief   Changes the priority of a system handler.
 *
 * @param[in] handler   the system handler number
 * @param[in] prio      the system handler priority
 */
void nvicSetSystemHandlerPriority(uint32_t handler, uint32_t prio) {

  osalDbgCheck(handler < 12U);

#if defined(__CORE_CM0_H_GENERIC)
  SCB->SHP[_SHP_IDX(handler)] = (SCB->SHP[_SHP_IDX(handler)] & ~(0xFFU << _BIT_SHIFT(handler))) |
                                (NVIC_PRIORITY_MASK(prio) << _BIT_SHIFT(handler));
#elif defined(__CORE_CM7_H_GENERIC)
  SCB->SHPR[handler] = NVIC_PRIORITY_MASK(prio);
#else
  SCB->SHP[handler] = NVIC_PRIORITY_MASK(prio);
#endif
}

/**
 * @brief   Clears a pending interrupt source.
 *
 * @param[in] n         the interrupt number
 */
void nvicClearPending(uint32_t n) {

  NVIC->ICPR[n >> 5] = 1 << (n & 0x1F);
}

/** @} */
