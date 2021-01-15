/*
    ChibiOS - Copyright (C) 2006..2016 Martino Migliavacca

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
 * @file    TIMv1/hal_qei_lld.c
 * @brief   STM32 QEI subsystem low level driver header.
 *
 * @addtogroup QEI
 * @{
 */

#include "hal.h"

#if (HAL_USE_QEI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   QEID1 driver identifier.
 * @note    The driver QEID1 allocates the complex timer TIM1 when enabled.
 */
#if STM32_QEI_USE_TIM1 || defined(__DOXYGEN__)
QEIDriver QEID1;
#endif

/**
 * @brief   QEID2 driver identifier.
 * @note    The driver QEID1 allocates the timer TIM2 when enabled.
 */
#if STM32_QEI_USE_TIM2 || defined(__DOXYGEN__)
QEIDriver QEID2;
#endif

/**
 * @brief   QEID3 driver identifier.
 * @note    The driver QEID1 allocates the timer TIM3 when enabled.
 */
#if STM32_QEI_USE_TIM3 || defined(__DOXYGEN__)
QEIDriver QEID3;
#endif

/**
 * @brief   QEID4 driver identifier.
 * @note    The driver QEID4 allocates the timer TIM4 when enabled.
 */
#if STM32_QEI_USE_TIM4 || defined(__DOXYGEN__)
QEIDriver QEID4;
#endif

/**
 * @brief   QEID5 driver identifier.
 * @note    The driver QEID5 allocates the timer TIM5 when enabled.
 */
#if STM32_QEI_USE_TIM5 || defined(__DOXYGEN__)
QEIDriver QEID5;
#endif

/**
 * @brief   QEID8 driver identifier.
 * @note    The driver QEID8 allocates the timer TIM8 when enabled.
 */
#if STM32_QEI_USE_TIM8 || defined(__DOXYGEN__)
QEIDriver QEID8;
#endif

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
 * @brief   Low level QEI driver initialization.
 *
 * @notapi
 */
void qei_lld_init(void) {

#if STM32_QEI_USE_TIM1
  /* Driver initialization.*/
  qeiObjectInit(&QEID1);
  QEID1.tim = STM32_TIM1;
#endif

#if STM32_QEI_USE_TIM2
  /* Driver initialization.*/
  qeiObjectInit(&QEID2);
  QEID2.tim = STM32_TIM2;
#endif

#if STM32_QEI_USE_TIM3
  /* Driver initialization.*/
  qeiObjectInit(&QEID3);
  QEID3.tim = STM32_TIM3;
#endif

#if STM32_QEI_USE_TIM4
  /* Driver initialization.*/
  qeiObjectInit(&QEID4);
  QEID4.tim = STM32_TIM4;
#endif

#if STM32_QEI_USE_TIM5
  /* Driver initialization.*/
  qeiObjectInit(&QEID5);
  QEID5.tim = STM32_TIM5;
#endif

#if STM32_QEI_USE_TIM8
  /* Driver initialization.*/
  qeiObjectInit(&QEID8);
  QEID8.tim = STM32_TIM8;
#endif
}

/**
 * @brief   Configures and activates the QEI peripheral.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 *
 * @notapi
 */
void qei_lld_start(QEIDriver *qeip) {
  osalDbgAssert((qeip->config->min == 0) || (qeip->config->max == 0),
		"only min/max set to 0 is supported");

  if (qeip->state == QEI_STOP) {
    /* Clock activation and timer reset.*/
#if STM32_QEI_USE_TIM1
    if (&QEID1 == qeip) {
      rccEnableTIM1(FALSE);
      rccResetTIM1();
    }
#endif
#if STM32_QEI_USE_TIM2
    if (&QEID2 == qeip) {
      rccEnableTIM2(FALSE);
      rccResetTIM2();
    }
#endif
#if STM32_QEI_USE_TIM3
    if (&QEID3 == qeip) {
      rccEnableTIM3(FALSE);
      rccResetTIM3();
    }
#endif
#if STM32_QEI_USE_TIM4
    if (&QEID4 == qeip) {
      rccEnableTIM4(FALSE);
      rccResetTIM4();
    }
#endif

#if STM32_QEI_USE_TIM5
    if (&QEID5 == qeip) {
      rccEnableTIM5(FALSE);
      rccResetTIM5();
    }
#endif
#if STM32_QEI_USE_TIM8
    if (&QEID8 == qeip) {
      rccEnableTIM8(FALSE);
      rccResetTIM8();
    }
#endif
  }
   /* Timer configuration.*/
  qeip->tim->CR1  = 0;                      /* Initially stopped. */
  qeip->tim->CR2  = 0;
  qeip->tim->PSC  = 0;
  qeip->tim->DIER = 0;
  qeip->tim->ARR  = 0xFFFF;

  /* Set Capture Compare 1 and Capture Compare 2 as input. */
   qeip->tim->CCMR1 |= TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0;

  if (qeip->config->mode == QEI_MODE_QUADRATURE) {
    if (qeip->config->resolution == QEI_BOTH_EDGES)
      qeip->tim->SMCR = TIM_SMCR_SMS_1 | TIM_SMCR_SMS_0;
    else
      qeip->tim->SMCR = TIM_SMCR_SMS_0;
  } else {
    /* Direction/Clock mode.
     * Direction input on TI1, Clock input on TI2. */
    qeip->tim->SMCR = TIM_SMCR_SMS_0;
  }

  if (qeip->config->dirinv == QEI_DIRINV_TRUE)
    qeip->tim->CCER = TIM_CCER_CC1E | TIM_CCER_CC1P | TIM_CCER_CC2E;
  else
    qeip->tim->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E;
}

/**
 * @brief   Deactivates the QEI peripheral.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 *
 * @notapi
 */
void qei_lld_stop(QEIDriver *qeip) {

  if (qeip->state == QEI_READY) {
    qeip->tim->CR1 = 0;                    /* Timer disabled. */

    /* Clock deactivation.*/
#if STM32_QEI_USE_TIM1
    if (&QEID1 == qeip) {
      rccDisableTIM1();
    }
#endif
#if STM32_QEI_USE_TIM2
    if (&QEID2 == qeip) {
      rccDisableTIM2();
    }
#endif
#if STM32_QEI_USE_TIM3
    if (&QEID3 == qeip) {
      rccDisableTIM3();
    }
#endif
#if STM32_QEI_USE_TIM4
    if (&QEID4 == qeip) {
      rccDisableTIM4();
    }
#endif
#if STM32_QEI_USE_TIM5
    if (&QEID5 == qeip) {
      rccDisableTIM5();
    }
#endif
  }
#if STM32_QEI_USE_TIM8
    if (&QEID8 == qeip) {
      rccDisableTIM8();
    }
#endif
}

/**
 * @brief   Enables the input capture.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 *
 * @notapi
 */
void qei_lld_enable(QEIDriver *qeip) {

  qeip->tim->CR1 = TIM_CR1_CEN;            /* Timer enabled. */
}

/**
 * @brief   Disables the input capture.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 *
 * @notapi
 */
void qei_lld_disable(QEIDriver *qeip) {

  qeip->tim->CR1 = 0;                    /* Timer disabled. */
}

#endif /* HAL_USE_QEI */

/** @} */
