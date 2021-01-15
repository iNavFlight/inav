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

#ifndef HAL_OPAMP_H_
#define HAL_OPAMP_H_

#include "hal.h"

#if (HAL_USE_OPAMP == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define OPAMP_P_BELOW_M (0U)
#define OPAMP_M_BELOW_P (1U)

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
  OPAMP_UNINIT = 0,                   /**< Not initialized.                   */
  OPAMP_STOP = 1,                     /**< Stopped.                           */
  OPAMP_ACTIVE = 2,                   /**< Active.                            */
  OPAMP_CALIBRATING = 3,              /**< Calibration in progress.           */
} opampstate_t;

/**
 * @brief   Type of a structure representing an OPAMP driver.
 */
typedef struct OPAMPDriver OPAMPDriver;

#include "hal_opamp_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Calibrate opamps
 * *
 * @iclass
 */
#define opampCalibrate() opamp_lld_calibrate()
/** @} */

/**
 * @name    Low Level driver helper macros
 * @{
 */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void opampInit(void);
  void opampObjectInit(OPAMPDriver *opamp);
  void opampStart(OPAMPDriver *opamp, const OPAMPConfig *config);
  void opampStop(OPAMPDriver *opamp);
  void opampEnable(OPAMPDriver *opamp);
  void opampDisable(OPAMPDriver *opamp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_OPAMP */

#endif /* HAL_OPAMP_H_ */
