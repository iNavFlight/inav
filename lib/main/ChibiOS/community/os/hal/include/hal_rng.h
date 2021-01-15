/*
    RNG for ChibiOS - Copyright (C) 2016 Stephane D'Alu

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

#ifndef HAL_RNG_H_
#define HAL_RNG_H_

#if (HAL_USE_RNG == TRUE) || defined(__DOXYGEN__)

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
 * @brief   Enables the @p rngAcquireBus() and @p rngReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(RNG_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define RNG_USE_MUTUAL_EXCLUSION        TRUE
#endif

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
  RNG_UNINIT,                /* Not initialized.                           */
  RNG_STOP,                  /* Stopped.                                   */
  RNG_READY,                 /* Ready.                                     */
} rngstate_t;

#include "hal_rng_lld.h"


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
 * @param[in] rngp      pointer to the @p RNGDriver object
 *
 * @notapi
 */
#define _rng_wakeup_isr(rngp) {                                             \
  osalSysLockFromISR();                                                     \
  osalThreadResumeI(&(rngp)->thread, MSG_OK);                               \
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
 * @param[in] rngp      pointer to the @p RNGDriver object
 *
 * @notapi
 */
#define _rng_isr_code(rngp, rng) {                                          \
  if ((rngp)->config->end_cb) {                                             \
    (rngp)->state = RNG_COMPLETE;                                           \
    (rngp)->config->end_cb(rngp, rng);                                      \
    if ((rngp)->state == RNG_COMPLETE)                                      \
      (rngp)->state = RNG_READY;                                            \
  }                                                                         \
  else                                                                      \
    (rngp)->state = RNG_READY;                                              \
  _rng_wakeup_isr(rngp);                                                    \
}

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void rngInit(void);
  void rngObjectInit(RNGDriver *rngp);
  void rngStart(RNGDriver *rngp, const RNGConfig *config);
  void rngStop(RNGDriver *rngp);
  msg_t rngWriteI(RNGDriver *rngp, uint8_t *buf, size_t n, systime_t timeout);
  msg_t rngWrite(RNGDriver *rngp, uint8_t *buf, size_t n, systime_t timeout);
#if RNG_USE_MUTUAL_EXCLUSION == TRUE
  void rngAcquireUnit(RNGDriver *rngp);
  void rngReleaseUnit(RNGDriver *rngp);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_RNG */

#endif /* HAL_RNG_H_ */

/** @} */
