/*
    ChibiOS - Copyright (C) 2006..2019 Giovanni Di Sirio
              Copyright (C) 2019 Fabien Poussin (fabien.poussin (at) google's mail)

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
 * @file    hal_opamp.c
 * @brief   OPAMP Driver code.
 *
 * @addtogroup OPAMP
 * @{
 */

#include "hal_opamp.h"

#if HAL_USE_OPAMP || defined(__DOXYGEN__)


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
 * @brief   OPAMP Driver initialization.
 * @note    This function is implicitly invoked by @p halInit(), there is
 *          no need to explicitly initialize the driver.
 *
 * @init
 */
void opampInit(void) {

  opamp_lld_init();
}

/**
 * @brief   Initializes the standard part of a @p OPAMPDriver structure.
 *
 * @param[out] opampp     pointer to the @p OPAMPDriver object
 *
 * @init
 */
void opampObjectInit(OPAMPDriver *opampp) {

  opampp->state  = OPAMP_STOP;
  opampp->config = NULL;
}

/**
 * @brief   Configures and activates the OPAMP peripheral.
 *
 * @param[in] opampp      pointer to the @p OPAMPDriver object
 * @param[in] config    pointer to the @p OPAMPConfig object
 *
 * @api
 */
void opampStart(OPAMPDriver *opampp, const OPAMPConfig *config) {

  osalDbgCheck((opampp != NULL) && (config != NULL));

  osalSysLock();
  osalDbgAssert((opampp->state == OPAMP_STOP) || (opampp->state == OPAMP_ACTIVE),
              "invalid state");
  opampp->config = config;
  opamp_lld_start(opampp);
  opampp->state = OPAMP_ACTIVE;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the OPAMP peripheral.
 *
 * @param[in] opampp      pointer to the @p OPAMPDriver object
 *
 * @api
 */
void opampStop(OPAMPDriver *opampp) {

  osalDbgCheck(opampp != NULL);

  osalSysLock();
  osalDbgAssert((opampp->state == OPAMP_STOP) || (opampp->state == OPAMP_ACTIVE),
              "invalid state");
  opamp_lld_stop(opampp);
  opampp->state = OPAMP_STOP;
  osalSysUnlock();
}

/**
 * @brief   Activates the opamp.
 *
 * @param[in] opampp      pointer to the @p OPAMPDriver object
 *
 * @api
 */
void opampEnable(OPAMPDriver *opampp) {

  osalDbgCheck(opampp != NULL);

  osalSysLock();
  osalDbgAssert(opampp->state == OPAMP_ACTIVE, "invalid state");
  opamp_lld_enable(opampp);
  opampp->state = OPAMP_ACTIVE;
  osalSysUnlock();
}

/**
 * @brief   Deactivates the opamp.
 *
 * @param[in] opampp      pointer to the @p OPAMPDriver object
 *
 * @api
 */
void opampDisable(OPAMPDriver *opampp) {

  osalDbgCheck(opampp != NULL);

  osalSysLock();
  osalDbgAssert((opampp->state == OPAMP_ACTIVE),
             "invalid state");
  opamp_lld_disable(opampp);
  opampp->state = OPAMP_ACTIVE;
  osalSysUnlock();
}

#endif /* HAL_USE_OPAMP */

/** @} */
