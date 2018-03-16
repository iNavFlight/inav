/*
    ChibiOS - Copyright (C) 2015 Michael D. Spradling

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

/*
 * Hardware Abstraction Layer for CRC Unit
 */
#include "hal.h"

#if (HAL_USE_CRC == TRUE) || defined(__DOXYGEN__)

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
 * @brief   CRC Driver initialization.
 *
 * @init
 */
void crcInit(void) {
  crc_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p CRCDriver structure.
 *
 * @param[out] crcp    Pointer to the @p CRCDriver object
 *
 * @init
 */
void crcObjectInit(CRCDriver *crcp) {
  crcp->state  = CRC_STOP;
  crcp->config = NULL;
#if CRC_USE_DMA == TRUE
  crcp->thread = NULL;
#endif
#if CRC_USE_MUTUAL_EXCLUSION == TRUE
  osalMutexObjectInit(&crcp->mutex);
#endif
#if defined(CRC_DRIVER_EXT_INIT_HOOK)
  CRC_DRIVER_EXT_INIT_HOOK(crcp);
#endif
}

/**
 * @brief   Configures and activates the CRC peripheral.
 *
 * @param[in] crcp      Pointer to the @p CRCDriver object
 * @param[in] config    Pointer to the @p CRCConfig object
 *                      @p NULL if the low level driver implementation
 *                      supports a default configuration
 *
 * @api
 */
void crcStart(CRCDriver *crcp, const CRCConfig *config) {
  osalDbgCheck(crcp != NULL);

  osalSysLock();
  osalDbgAssert((crcp->state == CRC_STOP) || (crcp->state == CRC_READY),
                "invalid state");
  crcp->config = config;
  crc_lld_start(crcp);
  crcp->state = CRC_READY;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the CRC peripheral.
 *
 * @param[in] crcp     Pointer to the @p CRCDriver object
 *
 * @api
 */
void crcStop(CRCDriver *crcp) {
  osalDbgCheck(crcp != NULL);

  osalSysLock();
  osalDbgAssert((crcp->state == CRC_STOP) || (crcp->state == CRC_READY),
                "invalid state");
  crc_lld_stop(crcp);
  crcp->state = CRC_STOP;
  osalSysUnlock();
}

/**
 * @brief   Resets the CRC calculation
 *
 * @param[in] crcp     Pointer to the @p CRCDriver object
 *
 * @api
 */
void crcReset(CRCDriver *crcp) {
  osalSysLock();
  crcResetI(crcp);
  osalSysUnlock();
}

/**
 * @brief   Resets the current CRC calculation
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @iclass
 */
void crcResetI(CRCDriver *crcp) {
  osalDbgCheck(crcp != NULL);
  osalDbgAssert(crcp->state == CRC_READY, "Not ready");
  crc_lld_reset(crcp);
}

/**
 * @brief   Performs a CRC calculation.
 * @details This synchronous function performs a crc calculation operation.
 * @pre     In order to use this function the driver must have been configured
 *          without callbacks (@p end_cb = @p NULL).
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 * @param[in] n         number of bytes to send
 * @param[in] buf       the pointer to the buffer
 *
 * @api
 */
uint32_t crcCalc(CRCDriver *crcp, size_t n, const void *buf) {
  uint32_t crc;
#if CRC_USE_DMA
  osalSysLock();
#endif
  crc = crcCalcI(crcp, n, buf);
#if CRC_USE_DMA
  osalSysUnlock();
#endif
  return crc;
}

/**
 * @brief   Performs a CRC calculation.
 * @details This synchronous function performs a crc calcuation operation.
 * @pre     In order to use this function the driver must have been configured
 *          without callbacks (@p end_cb = @p NULL).
 * @post    At the end of the operation the configured callback is invoked.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 * @param[in] n         number of bytes to send
 * @param[in] buf       the pointer to the buffer
 *
 * @iclass
 */
uint32_t crcCalcI(CRCDriver *crcp, size_t n, const void *buf) {
  osalDbgCheck((crcp != NULL) && (n > 0U) && (buf != NULL));
  osalDbgAssert(crcp->state == CRC_READY, "not ready");
#if CRC_USE_DMA
  osalDbgAssert(crcp->config->end_cb == NULL, "callback defined");
  (crcp)->state = CRC_ACTIVE;
#endif
  return crc_lld_calc(crcp, n, buf);
}

#if CRC_USE_DMA == TRUE
/**
 * @brief   Performs a CRC calculation.
 * @details This asynchronous function starts a crc calcuation operation.
 * @pre     In order to use this function the driver must have been configured
 *          with callbacks (@p end_cb != @p NULL).
 * @post    At the end of the operation the configured callback is invoked.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 * @param[in] n         number of bytes to send
 * @param[in] buf       the pointer to the buffer
 *
 * @api
 */
void crcStartCalc(CRCDriver *crcp, size_t n, const void *buf) {
  osalSysLock();
  crcStartCalcI(crcp, n, buf);
  osalSysUnlock();
}

/**
 * @brief   Performs a CRC calculation.
 * @details This asynchronous function starts a crc calcuation operation.
 * @pre     In order to use this function the driver must have been configured
 *          without callbacks (@p end_cb = @p NULL).
 * @post    At the end of the operation the configured callback is invoked.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 * @param[in] n         number of bytes to send
 * @param[in] buf       the pointer to the buffer
 *
 *
 * @iclass
 */
void crcStartCalcI(CRCDriver *crcp, size_t n, const void *buf) {
  osalDbgCheck((crcp != NULL) && (n > 0U) && (buf != NULL));
  osalDbgAssert(crcp->state == CRC_READY, "not ready");
  osalDbgAssert(crcp->config->end_cb != NULL, "callback not defined");
  (crcp)->state = CRC_ACTIVE;
  crc_lld_start_calc(crcp, n, buf);
}
#endif

#if (CRC_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Gains exclusive access to the CRC unit.
 * @details This function tries to gain ownership to the CRC, if the CRC is
 *          already being used then the invoking thread is queued.
 * @pre     In order to use this function the option @p CRC_USE_MUTUAL_EXCLUSION
 *          must be enabled.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @api
 */
void crcAcquireUnit(CRCDriver *crcp) {
  osalDbgCheck(crcp != NULL);

  osalMutexLock(&crcp->mutex);
}

/**
 * @brief   Releases exclusive access to the CRC unit.
 * @pre     In order to use this function the option @p CRC_USE_MUTUAL_EXCLUSION
 *          must be enabled.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @api
 */
void crcReleaseUnit(CRCDriver *crcp) {
  osalDbgCheck(crcp != NULL);

  osalMutexUnlock(&crcp->mutex);
}
#endif /* CRC_USE_MUTUAL_EXCLUSION == TRUE */

#endif /* HAL_USE_CRC */
