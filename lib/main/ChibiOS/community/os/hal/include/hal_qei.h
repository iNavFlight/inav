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
 * @file    hal_qei.h
 * @brief   QEI Driver macros and structures.
 *
 * @addtogroup QEI
 * @{
 */

#ifndef HAL_QEI_H
#define HAL_QEI_H

#if (HAL_USE_QEI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  QEI_UNINIT = 0,                   /**< Not initialized.                   */
  QEI_STOP = 1,                     /**< Stopped.                           */
  QEI_READY = 2,                    /**< Ready.                             */
  QEI_ACTIVE = 3,                   /**< Active.                            */
} qeistate_t;

/**
 * @brief   Type of a structure representing an QEI driver.
 */
typedef struct QEIDriver QEIDriver;

/**
 * @brief   QEI notification callback type.
 *
 * @param[in] qeip      pointer to a @p QEIDriver object
 */
typedef void (*qeicallback_t)(QEIDriver *qeip);

#include "hal_qei_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Enables the input capture.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 *
 * @iclass
 */
#define qeiEnableI(qeip) qei_lld_enable(qeip)

/**
 * @brief   Disables the input capture.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 *
 * @iclass
 */
#define qeiDisableI(qeip) qei_lld_disable(qeip)

/**
 * @brief   Returns the counter value.
 *
 * @param[in] qeip      pointer to the @p QEIDriver object
 * @return              The current counter value.
 *
 * @iclass
 */
#define qeiGetCountI(qeip) qei_lld_get_count(qeip)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void qeiInit(void);
  void qeiObjectInit(QEIDriver *qeip);
  void qeiStart(QEIDriver *qeip, const QEIConfig *config);
  void qeiStop(QEIDriver *qeip);
  void qeiEnable(QEIDriver *qeip);
  void qeiDisable(QEIDriver *qeip);
  qeicnt_t qeiGetCount(QEIDriver *qeip);
  qeidelta_t qeiUpdate(QEIDriver *qeip);
  qeidelta_t qeiUpdateI(QEIDriver *qeip);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_QEI  == TRUE */

#endif /* HAL_QEI_H */

/** @} */
