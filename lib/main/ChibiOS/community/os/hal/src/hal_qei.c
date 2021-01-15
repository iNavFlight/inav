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

/**
 * @brief   Helper for correclty handling overflow/underflow
 *
 * @details Underflow/overflow will be handled according to mode:
 *          QEI_OVERFLOW_WRAP:    counter value will wrap around.
 *          QEI_OVERFLOW_DISCARD: counter will not change
 *          QEI_OVERFLOW_MINMAX:  counter will be updated upto min or max.
 *
 * @note    This function is for use by low level driver.
 *
 * @param[in,out] count counter value
 * @param[in,out] delta adjustment value
 * @param[in]     min   minimum allowed value for counter
 * @param[in]     max   maximum allowed value for counter
 * @param[in]     mode  how to handle overflow
 *
 * @return        true if counter underflow/overflow occured or
 *                was due to occur
 *
 */
static inline
bool qei_adjust_count(qeicnt_t *count, qeidelta_t *delta,
		      qeicnt_t min, qeicnt_t max, qeioverflow_t mode) {
  /* For information on signed integer overflow see:
   * https://www.securecoding.cert.org/confluence/x/RgE
   */

  /* Get values */
  const qeicnt_t   _count = *count;
  const qeidelta_t _delta = *delta;

  /* Overflow operation
   */
  if ((_delta > 0) && (_count > (max - _delta))) {
    switch(mode) {
    case QEI_OVERFLOW_WRAP:
      *delta = 0;
      *count = (min + (_count - (max - _delta))) - 1;
      break;
#if QEI_USE_OVERFLOW_DISCARD == TRUE
    case QEI_OVERFLOW_DISCARD:
      *delta = _delta;
      *count = _count;
      break;
#endif
#if QEI_USE_OVERFLOW_MINMAX == TRUE
    case QEI_OVERFLOW_MINMAX:
      *delta = _count - (max - _delta);
      *count = max;
      break;
#endif
    }
    return true;
    
 /* Underflow operation
  */
  } else if ((_delta < 0) && (_count < (min - _delta))) {
    switch(mode) {
    case QEI_OVERFLOW_WRAP:
      *delta = 0;
      *count = (max + (_count - (min - _delta))) + 1;
    break;
#if QEI_USE_OVERFLOW_DISCARD == TRUE
    case QEI_OVERFLOW_DISCARD:
      *delta = _delta;
      *count = _count;
      break;
#endif
#if QEI_USE_OVERFLOW_MINMAX == TRUE
    case QEI_OVERFLOW_MINMAX:
      *delta = _count - (min - _delta);
      *count = min;
      break;
#endif
    }
    return true;

  /* Normal operation
   */
  } else {
    *delta = 0;
    *count = _count + _delta;
    return false;
  }
}

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
 * @brief   Set counter value.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object.
 * @param[in] value     the new counter value.
 *
 * @api
 */
void qeiSetCount(QEIDriver *qeip, qeicnt_t value) {
  osalDbgCheck(qeip != NULL);
  osalDbgAssert((qeip->state == QEI_READY) || (qeip->state == QEI_ACTIVE),
		"invalid state");

  osalSysLock();
  qei_lld_set_count(qeip, value);
  osalSysUnlock();
}

/**
 * @brief   Adjust the counter by delta.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object.
 * @param[in] delta     the adjustement value.
 * @return              the remaining delta (can occur during overflow).
 *
 * @api
 */
qeidelta_t qeiAdjust(QEIDriver *qeip, qeidelta_t delta) {
  osalDbgCheck(qeip != NULL);
  osalDbgAssert((qeip->state == QEI_ACTIVE), "invalid state");

  osalSysLock();
  delta = qeiAdjustI(qeip, delta);
  osalSysUnlock();

  return delta;
}

/**
 * @brief   Adjust the counter by delta.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object.
 * @param[in] delta     the adjustement value.
 * @return              the remaining delta (can occur during overflow).
 *
 * @api
 */
qeidelta_t qeiAdjustI(QEIDriver *qeip, qeidelta_t delta) {
  /* Get boundaries */
  qeicnt_t min = QEI_COUNT_MIN;
  qeicnt_t max = QEI_COUNT_MAX;
  if (qeip->config->min != qeip->config->max) {
    min = qeip->config->min;
    max = qeip->config->max;
  }

  /* Get counter */
  qeicnt_t count = qei_lld_get_count(qeip);
  
  /* Adjust counter value */
  bool overflowed = qei_adjust_count(&count, &delta,
				     min, max, qeip->config->overflow);

  /* Notify for value change */
  qei_lld_set_count(qeip, count);

  /* Notify for overflow (passing the remaining delta) */
  if (overflowed && qeip->config->overflow_cb)
    qeip->config->overflow_cb(qeip, delta);

  /* Remaining delta */
  return delta;
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
  delta = (qeicnt_t)(cnt - qeip->last);
  qeip->last = cnt;

  return delta;
}

#endif /* HAL_USE_QEI == TRUE */

/** @} */
