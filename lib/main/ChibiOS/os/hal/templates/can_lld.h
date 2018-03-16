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
 * @file    can_lld.h
 * @brief   PLATFORM CAN subsystem low level driver header.
 *
 * @addtogroup CAN
 * @{
 */

#ifndef _CAN_LLD_H_
#define _CAN_LLD_H_

#if (HAL_USE_CAN == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   Number of transmit mailboxes.
 */
#define CAN_TX_MAILBOXES            1

/**
 * @brief   Number of receive mailboxes.
 */
#define CAN_RX_MAILBOXES            1

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    PLATFORM configuration options
 * @{
 */
/**
 * @brief   CAN1 driver enable switch.
 * @details If set to @p TRUE the support for CAN1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(PLATFORM_CAN_USE_CAN1) || defined(__DOXYGEN__)
#define PLATFORM_CAN_USE_CAN1                  FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a transmission mailbox index.
 */
typedef uint32_t canmbx_t;

/**
 * @brief   CAN transmission frame.
 * @note    Accessing the frame data as word16 or word32 is not portable because
 *          machine data endianness, it can be still useful for a quick filling.
 */
typedef struct {
  /*lint -save -e46 [6.1] Standard types are fine too.*/
  uint8_t                   DLC:4;          /**< @brief Data length.        */
  uint8_t                   RTR:1;          /**< @brief Frame type.         */
  uint8_t                   IDE:1;          /**< @brief Identifier type.    */
  union {
    uint32_t                SID:11;         /**< @brief Standard identifier.*/
    uint32_t                EID:29;         /**< @brief Extended identifier.*/
    uint32_t                _align1;
  };
  /*lint -restore*/
  union {
    uint8_t                 data8[8];       /**< @brief Frame data.         */
    uint16_t                data16[4];      /**< @brief Frame data.         */
    uint32_t                data32[2];      /**< @brief Frame data.         */
  };
} CANTxFrame;

/**
 * @brief   CAN received frame.
 * @note    Accessing the frame data as word16 or word32 is not portable because
 *          machine data endianness, it can be still useful for a quick filling.
 */
typedef struct {
  /*lint -save -e46 [6.1] Standard types are fine too.*/
  uint8_t                   FMI;            /**< @brief Filter id.          */
  uint16_t                  TIME;           /**< @brief Time stamp.         */
  uint8_t                   DLC:4;          /**< @brief Data length.        */
  uint8_t                   RTR:1;          /**< @brief Frame type.         */
  uint8_t                   IDE:1;          /**< @brief Identifier type.    */
  union {
    uint32_t                SID:11;         /**< @brief Standard identifier.*/
    uint32_t                EID:29;         /**< @brief Extended identifier.*/
    uint32_t                _align1;
  };
  /*lint -restore*/
  union {
    uint8_t                 data8[8];       /**< @brief Frame data.         */
    uint16_t                data16[4];      /**< @brief Frame data.         */
    uint32_t                data32[2];      /**< @brief Frame data.         */
  };
} CANRxFrame;

/**
 * @brief   Driver configuration structure.
 */
typedef struct {
  /* End of the mandatory fields.*/
  uint32_t                  dummy;
} CANConfig;

/**
 * @brief   Structure representing an CAN driver.
 */
typedef struct {
  /**
   * @brief   Driver state.
   */
  canstate_t                state;
  /**
   * @brief   Current configuration data.
   */
  const CANConfig           *config;
  /**
   * @brief   Transmission threads queue.
   */
  threads_queue_t           txqueue;
  /**
   * @brief   Receive threads queue.
   */
  threads_queue_t           rxqueue;
  /**
   * @brief   One or more frames become available.
   * @note    After broadcasting this event it will not be broadcasted again
   *          until the received frames queue has been completely emptied. It
   *          is <b>not</b> broadcasted for each received frame. It is
   *          responsibility of the application to empty the queue by
   *          repeatedly invoking @p chReceive() when listening to this event.
   *          This behavior minimizes the interrupt served by the system
   *          because CAN traffic.
   * @note    The flags associated to the listeners will indicate which
   *          receive mailboxes become non-empty.
   */
  event_source_t            rxfull_event;
  /**
   * @brief   One or more transmission mailbox become available.
   * @note    The flags associated to the listeners will indicate which
   *          transmit mailboxes become empty.
   *
   */
  event_source_t            txempty_event;
  /**
   * @brief   A CAN bus error happened.
   * @note    The flags associated to the listeners will indicate the
   *          error(s) that have occurred.
   */
  event_source_t            error_event;
#if (CAN_USE_SLEEP_MODE == TRUE) || defined (__DOXYGEN__)
  /**
   * @brief   Entering sleep state event.
   */
  event_source_t            sleep_event;
  /**
   * @brief   Exiting sleep state event.
   */
  event_source_t            wakeup_event;
#endif
  /* End of the mandatory fields.*/
} CANDriver;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (PLATFORM_CAN_USE_CAN1 == TRUE) && !defined(__DOXYGEN__)
extern CANDriver CAND1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void can_lld_init(void);
  void can_lld_start(CANDriver *canp);
  void can_lld_stop(CANDriver *canp);
  bool can_lld_is_tx_empty(CANDriver *canp, canmbx_t mailbox);
  void can_lld_transmit(CANDriver *canp,
                        canmbx_t mailbox,
                        const CANTxFrame *ctfp);
  bool can_lld_is_rx_nonempty(CANDriver *canp, canmbx_t mailbox);
  void can_lld_receive(CANDriver *canp,
                       canmbx_t mailbox,
                       CANRxFrame *crfp);
#if CAN_USE_SLEEP_MODE == TRUE
  void can_lld_sleep(CANDriver *canp);
  void can_lld_wakeup(CANDriver *canp);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_CAN == TRUE */

#endif /* _CAN_LLD_H_ */

/** @} */
