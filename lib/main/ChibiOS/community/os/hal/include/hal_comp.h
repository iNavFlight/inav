/*
    ChibiOS - Copyright (C) 2006..2017 Giovanni Di Sirio
              Copyright (C) 2017 Fabien Poussin (fabien.poussin (at) google's mail)

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

#ifndef HAL_COMP_H_
#define HAL_COMP_H_

#include "hal.h"


#if (HAL_USE_COMP == TRUE) || defined(__DOXYGEN__)

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
  COMP_UNINIT = 0,                   /**< Not initialized.                   */
  COMP_STOP = 1,                     /**< Stopped.                           */
  COMP_READY = 2,                    /**< Ready.                             */
  COMP_ACTIVE = 3,                   /**< Active cycle phase.                */
} compstate_t;

/**
 * @brief   Type of a structure representing an COMP driver.
 */
typedef struct COMPDriver COMPDriver;

/**
 * @brief   COMP notification callback type.
 *
 * @param[in] comp      pointer to a @p COMPDriver object
 */
typedef void (*compcallback_t)(COMPDriver *comp);

#include "hal_comp_lld.h"

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
 * @param[in] comp      pointer to the @p COMPDriver object
 *
 * @iclass
 */
#define compEnableI(comp) comp_lld_enable(comp)

/**
 * @brief   Disables the input capture.
 *
 * @param[in] comp      pointer to the @p COMPDriver object
 *
 * @iclass
 */
#define compDisableI(comp) comp_lld_disable(comp)
/** @} */


/**
 * @name    Low Level driver helper macros
 * @{
 */

/**
 * @brief   Common ISR code, main event.
 *
 * @param[in] comp      pointer to the @p COMPDriver object
 *
 * @notapi
 */
#define _comp_isr_invoke_cb(comp) {                                 \
  (comp)->config->cb(comp);                                        \
}
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void compInit(void);
  void compObjectInit(COMPDriver *comp);
  void compStart(COMPDriver *comp, const COMPConfig *config);
  void compStop(COMPDriver *comp);
  void compEnable(COMPDriver *comp);
  void compDisable(COMPDriver *comp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_COMP */


#endif /* HAL_COMP_H_ */
