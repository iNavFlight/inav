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

/**
 * @file    NRF51/RNGv1/rng_lld.c
 * @brief   NRF51 RNG subsystem low level driver source.
 *
 * @addtogroup RNG
 * @{
 */

#include "hal.h"

#if (HAL_USE_RNG == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/**
 * @brief   RNG default configuration.
 */
static const RNGConfig default_config = {
  .digital_error_correction = 1,
};

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief RNG1 driver identifier.*/
#if NRF51_RNG_USE_RNG1 || defined(__DOXYGEN__)
RNGDriver RNGD1;
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
 * @brief   Low level RNG driver initialization.
 *
 * @notapi
 */
void rng_lld_init(void) {
  rngObjectInit(&RNGD1);
  RNGD1.rng    = NRF_RNG;
}

/**
 * @brief   Configures and activates the RNG peripheral.
 *
 * @param[in] rngp      pointer to the @p RNGDriver object
 *
 * @notapi
 */
void rng_lld_start(RNGDriver *rngp) {
  if (rngp->config == NULL)
    rngp->config = &default_config;

  rngp->rng->POWER         = 1;

  if (rngp->config->digital_error_correction) 
    rngp->rng->CONFIG |=  RNG_CONFIG_DERCEN_Msk;
  else
    rngp->rng->CONFIG &= ~RNG_CONFIG_DERCEN_Msk;

  rngp->rng->EVENTS_VALRDY = 0;
  rngp->rng->INTENSET      = RNG_INTENSET_VALRDY_Msk;
  rngp->rng->TASKS_START   = 1;
}


/**
 * @brief   Deactivates the RNG peripheral.
 *
 * @param[in] rngp      pointer to the @p RNGDriver object
 *
 * @notapi
 */
void rng_lld_stop(RNGDriver *rngp) {
  rngp->rng->TASKS_STOP = 1;
  rngp->rng->POWER      = 0;
}


/**
 * @brief   Write random bytes;
 *
 * @param[in] rngp      pointer to the @p RNGDriver object
 * @param[in] n         size of buf in bytes
 * @param[in] buf       @p buffer location
 *
 * @notapi
 */
msg_t rng_lld_write(RNGDriver *rngp, uint8_t *buf, size_t n,
                    systime_t timeout) {
  size_t i;

  for (i = 0 ; i < n ; i++) {
    /* Wait for byte ready
     * It take about 677µs to generate a new byte, not sure if
     * forcing a context switch will be a benefit
     */
    while (NRF_RNG->EVENTS_VALRDY == 0) {
      /* Sleep and wakeup on ARM event (interrupt) */
      SCB->SCR |= SCB_SCR_SEVONPEND_Msk;
      __SEV();
      __WFE();
      __WFE();
    }

    /* Read byte */
    buf[i] = (char)NRF_RNG->VALUE;

    /* Mark as read */
    NRF_RNG->EVENTS_VALRDY = 0;

    /* Clear interrupt so we can wake up again */
    nvicClearPending(RNG_IRQn);
  }
  return MSG_OK;
}

#endif /* HAL_USE_RNG */

/** @} */
