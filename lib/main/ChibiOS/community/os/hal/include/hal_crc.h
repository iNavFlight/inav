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

#ifndef HAL_CRC_H_
#define HAL_CRC_H_

#if (HAL_USE_CRC == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */

/**
 * @brief   Enable DMA CRC
 * @note    Enables DMA when doing CRC calculations.  This may be less 
 *          efficient with smaller CRC calculations.
 */
#if !defined(CRC_USE_DMA) || defined(__DOXYGEN__)
#define CRC_USE_DMA                     FALSE
#endif

/**
 * @brief   Enables the @p crcAcquireBus() and @p crcReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(CRC_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define CRC_USE_MUTUAL_EXCLUSION        TRUE
#endif

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if STM32_CRC_USE_CRC1 != TRUE && CRCSW_USE_CRC1 != TRUE
#error "CRC requires at least one LLD driver."
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  CRC_UNINIT,                /* Not initialized.                           */
  CRC_STOP,                  /* Stopped.                                   */
  CRC_READY,                 /* Ready.                                     */
  CRC_ACTIVE,                /* Calculating CRC.                           */
  CRC_COMPLETE               /* Asynchronous operation complete.           */
} crcstate_t;

#include "hal_crc_lld.h"
#include "crcsw.h" /* Include software LL driver */


/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Low level driver helper macros
 * @{
 */

/**
 * @brief   Wakes up the waiting thread.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @notapi
 */
#define _crc_wakeup_isr(crcp) {                                             \
  osalSysLockFromISR();                                                     \
  osalThreadResumeI(&(crcp)->thread, MSG_OK);                               \
  osalSysUnlockFromISR();                                                   \
}

/**
 * @brief   Common ISR code.
 * @details This code handles the portable part of the ISR code:
 *          - Callback invocation.
 *          - Waiting thread wakeup, if any.
 *          - Driver state transitions.
 *          .
 * @note    This macro is meant to be used in the low level drivers
 *          implementation only.
 *
 * @param[in] crcp      pointer to the @p CRCDriver object
 *
 * @notapi
 */
#define _crc_isr_code(crcp, crc) {                                          \
  if ((crcp)->config->end_cb) {                                             \
    (crcp)->state = CRC_COMPLETE;                                           \
    (crcp)->config->end_cb(crcp, crc);                                      \
    if ((crcp)->state == CRC_COMPLETE)                                      \
      (crcp)->state = CRC_READY;                                            \
  }                                                                         \
  else                                                                      \
    (crcp)->state = CRC_READY;                                              \
  _crc_wakeup_isr(crcp);                                                    \
}

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void crcInit(void);
  void crcObjectInit(CRCDriver *crcp);
  void crcStart(CRCDriver *crcp, const CRCConfig *config);
  void crcStop(CRCDriver *crcp);
  void crcReset(CRCDriver *crcp);
  void crcResetI(CRCDriver *crcp);
  uint32_t crcCalc(CRCDriver *crcp, size_t n, const void *buf);
  uint32_t crcCalcI(CRCDriver *crcp, size_t n, const void *buf);
#if CRC_USE_DMA == TRUE
  void crcStartCalc(CRCDriver *crcp, size_t n, const void *buf);
  void crcStartCalcI(CRCDriver *crcp, size_t n, const void *buf);
#endif
#if CRC_USE_MUTUAL_EXCLUSION == TRUE
  void crcAcquireUnit(CRCDriver *crcp);
  void crcReleaseUnit(CRCDriver *crcp);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_CRC */

#endif /* HAL_CRC_H_ */

/** @} */
