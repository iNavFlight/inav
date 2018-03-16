/*
    ChibiOS - Copyright (C) 2006..2015 Giovanni Di Sirio

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
 * @file    can.h
 * @brief   CAN Driver macros and structures.
 *
 * @addtogroup CAN
 * @{
 */

#ifndef _CAN_H_
#define _CAN_H_

#if (HAL_USE_CAN == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    CAN status flags
 * @{
 */
/**
 * @brief   Errors rate warning.
 */
#define CAN_LIMIT_WARNING           1U
/**
 * @brief   Errors rate error.
 */
#define CAN_LIMIT_ERROR             2U
/**
 * @brief   Bus off condition reached.
 */
#define CAN_BUS_OFF_ERROR           4U
/**
 * @brief   Framing error of some kind on the CAN bus.
 */
#define CAN_FRAMING_ERROR           8U
/**
 * @brief   Overflow in receive queue.
 */
#define CAN_OVERFLOW_ERROR          16U
/** @} */

/**
 * @brief   Special mailbox identifier.
 */
#define CAN_ANY_MAILBOX             0

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    CAN configuration options
 * @{
 */
/**
 * @brief   Sleep mode related APIs inclusion switch.
 * @details This option can only be enabled if the CAN implementation supports
 *          the sleep mode, see the macro @p CAN_SUPPORTS_SLEEP exported by
 *          the underlying implementation.
 */
#if !defined(CAN_USE_SLEEP_MODE) || defined(__DOXYGEN__)
#define CAN_USE_SLEEP_MODE          TRUE
#endif
/** @} */

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
  CAN_UNINIT = 0,                           /**< Not initialized.           */
  CAN_STOP = 1,                             /**< Stopped.                   */
  CAN_STARTING = 2,                         /**< Starting.                  */
  CAN_READY = 3,                            /**< Ready.                     */
  CAN_SLEEP = 4                             /**< Sleep state.               */
} canstate_t;

#include "can_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Converts a mailbox index to a bit mask.
 */
#define CAN_MAILBOX_TO_MASK(mbx) (1U << ((mbx) - 1U))
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void canInit(void);
  void canObjectInit(CANDriver *canp);
  void canStart(CANDriver *canp, const CANConfig *config);
  void canStop(CANDriver *canp);
  bool canTryTransmitI(CANDriver *canp,
                       canmbx_t mailbox,
                       const CANTxFrame *ctfp);
  bool canTryReceiveI(CANDriver *canp,
                       canmbx_t mailbox,
                       CANRxFrame *crfp);
  msg_t canTransmit(CANDriver *canp,
                    canmbx_t mailbox,
                    const CANTxFrame *ctfp,
                    systime_t timeout);
  msg_t canReceive(CANDriver *canp,
                   canmbx_t mailbox,
                   CANRxFrame *crfp,
                   systime_t timeout);
#if CAN_USE_SLEEP_MODE
  void canSleep(CANDriver *canp);
  void canWakeup(CANDriver *canp);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_CAN == TRUE */

#endif /* _CAN_H_ */

/** @} */
