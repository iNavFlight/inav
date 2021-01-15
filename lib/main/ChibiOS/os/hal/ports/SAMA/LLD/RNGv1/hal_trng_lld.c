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
 * @file    hal_trng_lld.c
 * @brief   STM32 TRNG subsystem low level driver source.
 *
 * @addtogroup TRNG
 * @{
 */

#include "hal.h"

#if (HAL_USE_TRNG == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   TRNGD0 driver identifier.
 */
#if (SAMA_TRNG_USE_TRNG0 == TRUE) || defined(__DOXYGEN__)
TRNGDriver TRNGD0;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

static const TRNGConfig default_cfg = {0};

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
 * @brief   Low level TRNG driver initialization.
 *
 * @notapi
 */
void trng_lld_init(void) {

#if SAMA_TRNG_USE_TRNG0 == TRUE

#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_TRNG, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */

  /* Driver initialization.*/
  trngObjectInit(&TRNGD0);
  TRNGD0.trng = TRNG;
#endif
}

/**
 * @brief   Configures and activates the TRNG peripheral.
 *
 * @param[in] trngp      pointer to the @p TRNGDriver object
 *
 * @notapi
 */
void trng_lld_start(TRNGDriver *trngp) {

  /* There is no real configuration but setting up a valid pointer anyway.*/
  if (trngp->config == NULL) {
    trngp->config = &default_cfg;
  }

  if (trngp->state == TRNG_STOP) {
    /* Enables the peripheral.*/
#if SAMA_TRNG_USE_TRNG0 == TRUE
    if (&TRNGD0 == trngp) {
      pmcEnableTRNG0();
    }
#endif
  }
  /* Configures the peripheral.*/
  trngp->trng->TRNG_CR = TRNG_CR_ENABLE | TRNG_CR_KEY_PASSWD;
}

/**
 * @brief   Deactivates the TRNG peripheral.
 *
 * @param[in] trngp      pointer to the @p TRNGDriver object
 *
 * @notapi
 */
void trng_lld_stop(TRNGDriver *trngp) {

  if (trngp->state == TRNG_READY) {
    /* Resets the peripheral.*/
    trngp->trng->TRNG_CR = TRNG_CR_KEY_PASSWD;

    /* Disables the peripheral.*/
#if SAMA_TRNG_USE_TRNG0 == TRUE
    if (&TRNGD0 == trngp) {
      pmcDisableTRNG0();
    }
#endif
  }
}

/**
 * @brief   True random numbers generator.
 * @note    The function is blocking and likely performs polled waiting
 *          inside the low level implementation.
 *
 * @param[in] trngp             pointer to the @p TRNGDriver object
 * @param[in] size              size of output buffer
 * @param[out] out              output buffer
 * @return                      The operation status.
 * @retval false                if a random number has been generated.
 * @retval true                 if an HW error occurred.
 *
 * @api
 */
bool trng_lld_generate(TRNGDriver *trngp, size_t size, uint8_t *out) {

  while (true) {
    uint32_t r, tmo;
    size_t i;

    /* Waiting for a random number in data register.*/
    tmo = SAMA_DATA_FETCH_ATTEMPTS;
    while ((tmo > 0) && ((trngp->trng->TRNG_ISR & TRNG_ISR_DATRDY) == 0)) {
      tmo--;
      if (tmo == 0) {
        return true;
      }
    }

    /* Getting the generated random number.*/
    r = trngp->trng->TRNG_ODATA;

    /* Writing in the output buffer.*/
    for (i = 0; i < sizeof (uint32_t) / sizeof (uint8_t); i++) {
      *out++ = (uint8_t)r;
      r = r >> 8;
      size--;
      if (size == 0) {
        return false;
      }
    }
  }
}

#endif /* HAL_USE_TRNG == TRUE */

/** @} */
