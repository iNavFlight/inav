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
 * @file    hal_sio.c
 * @brief   SIO Driver code.
 *
 * @addtogroup SIO
 * @{
 */

#include "hal.h"

#if (HAL_USE_SIO == TRUE) || defined(__DOXYGEN__)

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
 * @brief   SIO Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void sioInit(void) {

  sio_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p SIODriver structure.
 *
 * @param[out] siop     pointer to the @p SIODriver object
 *
 * @init
 */
void sioObjectInit(SIODriver *siop) {

  siop->state      = SIO_STOP;
  siop->config     = NULL;

  /* Optional, user-defined initializer.*/
#if defined(SIO_DRIVER_EXT_INIT_HOOK)
  SIO_DRIVER_EXT_INIT_HOOK(siop);
#endif
}

/**
 * @brief   Configures and activates the SIO peripheral.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @param[in] config    pointer to the @p SIOConfig object
 *
 * @api
 */
void sioStart(SIODriver *siop, const SIOConfig *config) {

  osalDbgCheck((siop != NULL) && (config != NULL));

  osalSysLock();
  osalDbgAssert((siop->state == SIO_STOP) || (siop->state == SIO_READY),
                "invalid state");

  siop->config = config;
  sio_lld_start(siop);
  siop->state = SIO_READY;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the SIO peripheral.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 *
 * @api
 */
void sioStop(SIODriver *siop) {

  osalDbgCheck(siop != NULL);

  osalSysLock();

  osalDbgAssert((siop->state == SIO_STOP) || (siop->state == SIO_READY),
                "invalid state");

  sio_lld_stop(siop);
  siop->config  = NULL;
  siop->state   = SIO_STOP;

  osalSysUnlock();
}

#endif /* HAL_USE_SIO == TRUE */

/** @} */
