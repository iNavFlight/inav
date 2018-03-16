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

/*
 * Hardware Abstraction Layer for RNG Unit
 */
#include "hal.h"

#if (HAL_USE_RNG == TRUE) || defined(__DOXYGEN__)

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
 * @brief   RNG Driver initialization.
 *
 * @init
 */
void rngInit(void) {
  rng_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p RNGDriver structure.
 *
 * @param[out] rngp    Pointer to the @p RNGDriver object
 *
 * @init
 */
void rngObjectInit(RNGDriver *rngp) {
  rngp->state  = RNG_STOP;
  rngp->config = NULL;
#if RNG_USE_MUTUAL_EXCLUSION == TRUE
  osalMutexObjectInit(&rngp->mutex);
#endif
#if defined(RNG_DRIVER_EXT_INIT_HOOK)
  RNG_DRIVER_EXT_INIT_HOOK(rngp);
#endif
}

/**
 * @brief   Configures and activates the RNG peripheral.
 *
 * @param[in] rngp      Pointer to the @p RNGDriver object
 * @param[in] config    Pointer to the @p RNGConfig object
 *                      @p NULL if the low level driver implementation
 *                      supports a default configuration
 *
 * @api
 */
void rngStart(RNGDriver *rngp, const RNGConfig *config) {
  osalDbgCheck(rngp != NULL);

  osalSysLock();
  osalDbgAssert((rngp->state == RNG_STOP) || (rngp->state == RNG_READY),
                "invalid state");
  rngp->config = config;
  rng_lld_start(rngp);
  rngp->state = RNG_READY;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the RNG peripheral.
 *
 * @param[in] rngp     Pointer to the @p RNGDriver object
 *
 * @api
 */
void rngStop(RNGDriver *rngp) {
  osalDbgCheck(rngp != NULL);

  osalSysLock();
  osalDbgAssert((rngp->state == RNG_STOP) || (rngp->state == RNG_READY),
                "invalid state");
  rng_lld_stop(rngp);
  rngp->state = RNG_STOP;
  osalSysUnlock();
}

/**
 * @brief   Write random bytes
 * @details Write the request number of bytes..
 *
 * @param[in] rngp      pointer to the @p RNGDriver object
 * @param[in] buf       the pointer to the buffer
 * @param[in] n         number of bytes to send
 * @param[in] timeout   timeout value
 *
 * @api
 */
msg_t rngWrite(RNGDriver *rngp, uint8_t *buf, size_t n, systime_t timeout) {
  msg_t msg;
  osalSysLock();
  msg = rngWriteI(rngp, buf, n, timeout);
  osalSysUnlock();
  return msg;
}

/**
 * @brief   Write random bytes
 * @details Write the request number of bytes..
 *
 * @param[in] rngp      pointer to the @p RNGDriver object
 * @param[in] buf       the pointer to the buffer
 * @param[in] n         number of bytes to send
 * @param[in] timeout   timeout value
 *
 * @iclass
 */
msg_t rngWriteI(RNGDriver *rngp, uint8_t *buf, size_t n, systime_t timeout) {
  osalDbgCheck((rngp != NULL) && (n > 0U) && (buf != NULL));
  osalDbgAssert(rngp->state == RNG_READY, "not ready");
  return rng_lld_write(rngp, buf, n, timeout);
}


#if (RNG_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Gains exclusive access to the RNG unit.
 * @details This function tries to gain ownership to the RNG, if the RNG is
 *          already being used then the invoking thread is queued.
 * @pre     In order to use this function the option @p RNG_USE_MUTUAL_EXCLUSION
 *          must be enabled.
 *
 * @param[in] rngp      pointer to the @p RNGDriver object
 *
 * @api
 */
void rngAcquireUnit(RNGDriver *rngp) {
  osalDbgCheck(rngp != NULL);

  osalMutexLock(&rngp->mutex);
}

/**
 * @brief   Releases exclusive access to the RNG unit.
 * @pre     In order to use this function the option @p RNG_USE_MUTUAL_EXCLUSION
 *          must be enabled.
 *
 * @param[in] rngp      pointer to the @p RNGDriver object
 *
 * @api
 */
void rngReleaseUnit(RNGDriver *rngp) {
  osalDbgCheck(rngp != NULL);

  osalMutexUnlock(&rngp->mutex);
}
#endif /* RNG_USE_MUTUAL_EXCLUSION == TRUE */

#endif /* HAL_USE_RNG */
