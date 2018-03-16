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
 * @file    hal_qei.c
 * @brief   QEI Driver code.
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

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   QEI Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void qeiInit(void) {

  qei_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p QEIDriver structure.
 *
 * @param[out] qeip     pointer to the @p QEIDriver object
 *
 * @init
 */
void qeiObjectInit(QEIDriver *qeip) {

  qeip->state = QEI_STOP;
  qeip->last = 0;
  qeip->config = NULL;
}

/**
 * @brief   Configures and activates the QEI peripheral.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 * @param[in] config    pointer to the @p QEIConfig object
 *
 * @api
 */
void qeiStart(QEIDriver *qeip, const QEIConfig *config) {

  osalDbgCheck((qeip != NULL) && (config != NULL));

  osalSysLock();
  osalDbgAssert((qeip->state == QEI_STOP) || (qeip->state == QEI_READY),
                "invalid state");
  qeip->config = config;
  qei_lld_start(qeip);
  qeip->state = QEI_READY;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the QEI peripheral.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 *
 * @api
 */
void qeiStop(QEIDriver *qeip) {

  osalDbgCheck(qeip != NULL);

  osalSysLock();
  osalDbgAssert((qeip->state == QEI_STOP) || (qeip->state == QEI_READY),
                "invalid state");
  qei_lld_stop(qeip);
  qeip->state = QEI_STOP;
  osalSysUnlock();
}

/**
 * @brief   Enables the quadrature encoder interface.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 *
 * @api
 */
void qeiEnable(QEIDriver *qeip) {

  osalDbgCheck(qeip != NULL);

  osalSysLock();
  osalDbgAssert(qeip->state == QEI_READY, "invalid state");
  qei_lld_enable(qeip);
  qeip->state = QEI_ACTIVE;
  osalSysUnlock();
}

/**
 * @brief   Disables the quadrature encoder interface.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 *
 * @api
 */
void qeiDisable(QEIDriver *qeip) {

  osalDbgCheck(qeip != NULL);

  osalSysLock();
  osalDbgAssert((qeip->state == QEI_READY) || (qeip->state == QEI_ACTIVE),
                "invalid state");
  qei_lld_disable(qeip);
  qeip->state = QEI_READY;
  osalSysUnlock();
}

/**
 * @brief   Returns the counter value.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 * @return              The current counter value.
 *
 * @api
 */
qeicnt_t qeiGetCount(QEIDriver *qeip) {
  qeicnt_t cnt;

  osalSysLock();
  cnt = qeiGetCountI(qeip);
  osalSysUnlock();

  return cnt;
}

/**
 * @brief   Returns the counter delta from last reading.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 * @return              The delta from last read.
 *
 * @api
 */
qeidelta_t qeiUpdate(QEIDriver *qeip) {
  qeidelta_t diff;

  osalSysLock();
  diff = qeiUpdateI(qeip);
  osalSysUnlock();

  return diff;
}

/**
 * @brief   Returns the counter delta from last reading.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 * @return              The delta from last read.
 *
 * @iclass
 */
qeidelta_t qeiUpdateI(QEIDriver *qeip) {
  qeicnt_t cnt;
  qeidelta_t delta;

  osalDbgCheckClassI();
  osalDbgCheck(qeip != NULL);
  osalDbgAssert((qeip->state == QEI_READY) || (qeip->state == QEI_ACTIVE),
                "invalid state");

  cnt = qei_lld_get_count(qeip);
  delta = cnt - qeip->last;
  qeip->last = cnt;

  return delta;
}

#endif /* HAL_USE_QEI == TRUE */

/** @} */
