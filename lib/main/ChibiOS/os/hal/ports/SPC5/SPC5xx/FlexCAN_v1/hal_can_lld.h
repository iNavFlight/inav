/*
    SPC5 HAL - Copyright (C) 2013 STMicroelectronics

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
 * @file    SPC5xx/FlexCAN_v1/hal_can_lld.h
 * @brief   SPC5xx CAN subsystem low level driver header.
 *
 * @addtogroup CAN
 * @{
 */

#ifndef HAL_CAN_LLD_H
#define HAL_CAN_LLD_H

#if HAL_USE_CAN || defined(__DOXYGEN__)

#include "spc5_flexcan.h"

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @brief   This switch defines whether the driver implementation supports
 *          a low power switch mode with an automatic wakeup feature.
 */
#define CAN_SUPPORTS_SLEEP          FALSE

/**
 * @brief   This implementation supports eight FIFO receive filters.
 */
#define SPC5_CAN_MAX_FILTERS        8

/**
 * @brief   Enable filters.
 */
#define SPC5_CAN_FILTER_ON          1

/**
 * @brief   Disable filters.
 */
#define SPC5_CAN_FILTER_OFF         0

/**
 * @name    CAN registers helper macros
 * @{
 */
#define CAN_MCR_MAXMB(n)            (n)
#define CAN_MCR_AEN                 (1 << 12)
#define CAN_MCR_LPRIO_EN            (1 << 13)
#define CAN_MCR_BCC                 (1 << 16)
#define CAN_MCR_SRX_DIS             (1 << 17)
#define CAN_MCR_LPM_ACK             (1 << 20)
#define CAN_MCR_WRN_EN              (1 << 21)
#define CAN_MCR_SUPV                (1 << 23)
#define CAN_MCR_FRZ_ACK             (1 << 24)
#define CAN_MCR_WAK_MSK             (1 << 26)
#define CAN_MCR_NOT_RDY             (1 << 27)
#define CAN_MCR_HALT                (1 << 28)
#define CAN_MCR_FEN                 (1 << 29)
#define CAN_MCR_FRZ                 (1 << 30)
#define CAN_MCR_MDIS                (1 << 31)

#define CAN_CTRL_PROPSEG(n)         (n)
#define CAN_CTRL_LOM                (1 << 3)
#define CAN_CTRL_TSYN               (1 << 5)
#define CAN_CTRL_BOFF_REC           (1 << 6)
#define CAN_CTRL_SMP                (1 << 7)
#define CAN_CTRL_RWRN_MSK           (1 << 10)
#define CAN_CTRL_TWRN_MSK           (1 << 11)
#define CAN_CTRL_LPB                (1 << 12)
#define CAN_CTRL_CLK_SRC            (1 << 13)
#define CAN_CTRL_ERR_MSK            (1 << 14)
#define CAN_CTRL_BOFF_MSK           (1 << 15)
#define CAN_CTRL_PSEG2(n)           ((n) << 16)
#define CAN_CTRL_PSEG1(n)           ((n) << 19)
#define CAN_CTRL_RJW(n)             ((n) << 22)
#define CAN_CTRL_PRESDIV(n)         ((n) << 24)

#define CAN_IDE_STD                 0           /**< @brief Standard id.    */
#define CAN_IDE_EXT                 1           /**< @brief Extended id.    */

#define CAN_RTR_DATA                0           /**< @brief Data frame.     */
#define CAN_RTR_REMOTE              1           /**< @brief Remote frame.   */

#define CAN_ESR_ERR_INT             (1 << 1)
#define CAN_ESR_BOFF_INT            (1 << 2)
#define CAN_ESR_TWRN_INT            (1 << 14)
#define CAN_ESR_RWRN_INT            (1 << 15)
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   CAN filters enable setting.
 */
#if !defined(SPC5_CAN_USE_FILTERS) || defined(__DOXYGEN__)
#define SPC5_CAN_USE_FILTERS                FALSE
#endif

/**
 * @brief   FlexCAN clock source selection.
 */
#if !defined(SPC5_CAN_FLEXCAN_USE_EXT_CLK) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN_USE_EXT_CLK        FALSE
#endif

/**
 * @brief   CAN1 driver enable switch.
 * @details If set to @p TRUE the support for CAN1 is included.
 */
#if !defined(SPC5_CAN_USE_FLEXCAN0) || defined(__DOXYGEN__)
#define SPC5_CAN_USE_FLEXCAN0               FALSE
#endif

/**
 * @brief   CAN2 driver enable switch.
 * @details If set to @p TRUE the support for CAN2 is included.
 */
#if !defined(SPC5_CAN_USE_FLEXCAN1) || defined(__DOXYGEN__)
#define SPC5_CAN_USE_FLEXCAN1               FALSE
#endif

/**
 * @brief   CAN3 driver enable switch.
 * @details If set to @p TRUE the support for CAN3 is included.
 */
#if !defined(SPC5_CAN_USE_FLEXCAN2) || defined(__DOXYGEN__)
#define SPC5_CAN_USE_FLEXCAN2               FALSE
#endif

/**
 * @brief   CAN4 driver enable switch.
 * @details If set to @p TRUE the support for CAN4 is included.
 */
#if !defined(SPC5_CAN_USE_FLEXCAN3) || defined(__DOXYGEN__)
#define SPC5_CAN_USE_FLEXCAN3               FALSE
#endif

/**
 * @brief   CAN5 driver enable switch.
 * @details If set to @p TRUE the support for CAN5 is included.
 */
#if !defined(SPC5_CAN_USE_FLEXCAN4) || defined(__DOXYGEN__)
#define SPC5_CAN_USE_FLEXCAN4               FALSE
#endif

/**
 * @brief   CAN6 driver enable switch.
 * @details If set to @p TRUE the support for CAN6 is included.
 */
#if !defined(SPC5_CAN_USE_FLEXCAN5) || defined(__DOXYGEN__)
#define SPC5_CAN_USE_FLEXCAN5               FALSE
#endif

/**
 * @brief   Number of RX mailboxes.
 */
#if !defined(SPC5_CAN_NUM_RX_MAILBOXES) || defined(__DOXYGEN__)
#define SPC5_CAN_NUM_RX_MAILBOXES           8
#endif

/**
 * @brief   Number of TX mailboxes.
 */
#if !defined(SPC5_CAN_NUM_TX_MAILBOXES) || defined(__DOXYGEN__)
#define SPC5_CAN_NUM_TX_MAILBOXES           24
#endif

/**
 * @brief   CAN1 interrupt priority level setting.
 */
#if !defined(SPC5_CAN_FLEXCAN0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN0_IRQ_PRIORITY      11
#endif

/**
 * @brief   CAN2 interrupt priority level setting.
 */
#if !defined(SPC5_CAN_FLEXCAN1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN1_IRQ_PRIORITY      11
#endif

/**
 * @brief   CAN3 interrupt priority level setting.
 */
#if !defined(SPC5_CAN_FLEXCAN2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN2_IRQ_PRIORITY      11
#endif

/**
 * @brief   CAN4 interrupt priority level setting.
 */
#if !defined(SPC5_CAN_FLEXCAN3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN3_IRQ_PRIORITY      11
#endif

/**
 * @brief   CAN5 interrupt priority level setting.
 */
#if !defined(SPC5_CAN_FLEXCAN4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN4_IRQ_PRIORITY      11
#endif

/**
 * @brief   CAN6 interrupt priority level setting.
 */
#if !defined(SPC5_CAN_FLEXCAN5_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN5_IRQ_PRIORITY      11
#endif

/**
 * @brief   FlexCAN-0 peripheral configuration when started.
 * @note    The default configuration is 1 (always run) in run mode and
 *          2 (only halt) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_CAN_FLEXCAN0_START_PCTL) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN0_START_PCTL        (SPC5_ME_PCTL_RUN(1) |          \
                                             SPC5_ME_PCTL_LP(2))
#endif

/**
 * @brief   FlexCAN-0 peripheral configuration when stopped.
 * @note    The default configuration is 0 (never run) in run mode and
 *          0 (never run) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_CAN_FLEXCAN0_STOP_PCTL) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN0_STOP_PCTL         (SPC5_ME_PCTL_RUN(0) |          \
                                             SPC5_ME_PCTL_LP(0))
#endif

/**
 * @brief   FlexCAN-1 peripheral configuration when started.
 * @note    The default configuration is 1 (always run) in run mode and
 *          2 (only halt) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_CAN_FLEXCAN1_START_PCTL) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN1_START_PCTL        (SPC5_ME_PCTL_RUN(1) |          \
                                             SPC5_ME_PCTL_LP(2))
#endif

/**
 * @brief   FlexCAN-1 peripheral configuration when stopped.
 * @note    The default configuration is 0 (never run) in run mode and
 *          0 (never run) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_CAN_FLEXCAN1_STOP_PCTL) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN2_STOP_PCTL         (SPC5_ME_PCTL_RUN(0) |          \
                                             SPC5_ME_PCTL_LP(0))
#endif

/**
 * @brief   FlexCAN-2 peripheral configuration when started.
 * @note    The default configuration is 1 (always run) in run mode and
 *          2 (only halt) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_CAN_FLEXCAN2_START_PCTL) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN2_START_PCTL        (SPC5_ME_PCTL_RUN(1) |          \
                                             SPC5_ME_PCTL_LP(2))
#endif

/**
 * @brief   FlexCAN-2 peripheral configuration when stopped.
 * @note    The default configuration is 0 (never run) in run mode and
 *          0 (never run) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_CAN_FLEXCAN2_STOP_PCTL) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN2_STOP_PCTL         (SPC5_ME_PCTL_RUN(0) |          \
                                             SPC5_ME_PCTL_LP(0))
#endif

/**
 * @brief   FlexCAN-3 peripheral configuration when started.
 * @note    The default configuration is 1 (always run) in run mode and
 *          2 (only halt) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_CAN_FLEXCAN3_START_PCTL) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN3_START_PCTL        (SPC5_ME_PCTL_RUN(1) |          \
                                             SPC5_ME_PCTL_LP(2))
#endif

/**
 * @brief   FlexCAN-3 peripheral configuration when stopped.
 * @note    The default configuration is 0 (never run) in run mode and
 *          0 (never run) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_CAN_FLEXCAN3_STOP_PCTL) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN3_STOP_PCTL         (SPC5_ME_PCTL_RUN(0) |          \
                                             SPC5_ME_PCTL_LP(0))
#endif

/**
 * @brief   FlexCAN-4 peripheral configuration when started.
 * @note    The default configuration is 1 (always run) in run mode and
 *          2 (only halt) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_CAN_FLEXCAN4_START_PCTL) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN4_START_PCTL        (SPC5_ME_PCTL_RUN(1) |          \
                                             SPC5_ME_PCTL_LP(2))
#endif

/**
 * @brief   FlexCAN-4 peripheral configuration when stopped.
 * @note    The default configuration is 0 (never run) in run mode and
 *          0 (never run) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_CAN_FLEXCAN4_STOP_PCTL) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN4_STOP_PCTL         (SPC5_ME_PCTL_RUN(0) |          \
                                             SPC5_ME_PCTL_LP(0))
#endif

/**
 * @brief   FlexCAN-5 peripheral configuration when started.
 * @note    The default configuration is 1 (always run) in run mode and
 *          2 (only halt) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_CAN_FLEXCAN5_START_PCTL) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN5_START_PCTL        (SPC5_ME_PCTL_RUN(1) |          \
                                             SPC5_ME_PCTL_LP(2))
#endif

/**
 * @brief   FlexCAN-5 peripheral configuration when stopped.
 * @note    The default configuration is 0 (never run) in run mode and
 *          0 (never run) in low power mode. The defaults of the run modes
 *          are defined in @p hal_lld.h.
 */
#if !defined(SPC5_CAN_FLEXCAN5_STOP_PCTL) || defined(__DOXYGEN__)
#define SPC5_CAN_FLEXCAN5_STOP_PCTL         (SPC5_ME_PCTL_RUN(0) |          \
                                             SPC5_ME_PCTL_LP(0))
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if SPC5_CAN_USE_FLEXCAN0 && !SPC5_HAS_FLEXCAN0
#error "CAN1 not present in the selected device"
#endif

#if SPC5_CAN_USE_FLEXCAN1 && !SPC5_HAS_FLEXCAN1
#error "CAN2 not present in the selected device"
#endif

#if SPC5_CAN_USE_FLEXCAN2 && !SPC5_HAS_FLEXCAN2
#error "CAN3 not present in the selected device"
#endif

#if SPC5_CAN_USE_FLEXCAN3 && !SPC5_HAS_FLEXCAN3
#error "CAN4 not present in the selected device"
#endif

#if SPC5_CAN_USE_FLEXCAN4 && !SPC5_HAS_FLEXCAN4
#error "CAN5 not present in the selected device"
#endif

#if SPC5_CAN_USE_FLEXCAN5 && !SPC5_HAS_FLEXCAN5
#error "CAN6 not present in the selected device"
#endif

#if !SPC5_CAN_USE_FLEXCAN0 && !SPC5_CAN_USE_FLEXCAN1                        \
    && !SPC5_CAN_USE_FLEXCAN2 && !SPC5_CAN_USE_FLEXCAN3                     \
    && !SPC5_CAN_USE_FLEXCAN4 && !SPC5_CAN_USE_FLEXCAN5
#error "CAN driver activated but no CAN peripheral assigned"
#endif

#if CAN_USE_SLEEP_MODE && !CAN_SUPPORTS_SLEEP
#error "CAN sleep mode not supported in this architecture"
#endif

#if (SPC5_CAN_NUM_RX_MAILBOXES < 1) || (SPC5_CAN_NUM_RX_MAILBOXES > SPC5_FLEXCAN0_MB)
#error "invalid number of RX mailboxes"
#endif

#if (SPC5_CAN_NUM_TX_MAILBOXES < 1) || (SPC5_CAN_NUM_TX_MAILBOXES > SPC5_FLEXCAN0_MB)
#error "invalid number of TX mailboxes"
#endif

#if (SPC5_CAN_NUM_RX_MAILBOXES + SPC5_CAN_NUM_TX_MAILBOXES) > SPC5_FLEXCAN0_MB
#error "invalid amount of RX and TX mailboxes"
#endif

/**
 * @brief   Number of RX mailboxes to be allocated.
 */
#define CAN_TX_MAILBOXES            SPC5_CAN_NUM_TX_MAILBOXES

/**
 * @brief   Number of TX mailboxes to be allocated.
 */
#define CAN_RX_MAILBOXES            SPC5_CAN_NUM_RX_MAILBOXES

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a transmission mailbox index.
 */
typedef uint32_t canmbx_t;

/**
 * @brief   CAN TX MB structure.
 */
typedef struct {
  union {
    vuint32_t R;
    struct {
      vuint8_t:4;
      vuint8_t CODE:4;
      vuint8_t:1;
      vuint8_t SRR:1;
      vuint8_t IDE:1;
      vuint8_t RTR:1;
      vuint8_t LENGTH:4;
      vuint16_t TIMESTAMP:16;
    } B;
  } CS;

  union {
    vuint32_t R;
    struct {
      vuint8_t PRIO:3;
      vuint32_t ID:29;
    } B;
  } ID;
  vuint32_t DATA[2];     /* Data buffer in words (32 bits) */
} CAN_TxMailBox_TypeDef;

/**
 * @brief   CAN transmission frame.
 * @note    Accessing the frame data as word16 or word32 is not portable because
 *          machine data endianness, it can be still useful for a quick filling.
 */
typedef struct {
  struct {
    uint8_t                 LENGTH:4;       /**< @brief Data length.        */
    uint8_t                 RTR:1;          /**< @brief Frame type.         */
    uint8_t                 IDE:1;          /**< @brief Identifier type.    */
  };
  union {
    struct {
      uint32_t              SID:11;         /**< @brief Standard identifier.*/
    };
    struct {
      uint32_t              EID:29;         /**< @brief Extended identifier.*/
    };
  };
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
  struct {
    uint16_t                TIME;           /**< @brief Time stamp.         */
  };
  struct {
    uint8_t                 LENGTH:4;       /**< @brief Data length.        */
    uint8_t                 RTR:1;          /**< @brief Frame type.         */
    uint8_t                 IDE:1;          /**< @brief Identifier type.    */
  };
  union {
    struct {
      uint32_t              SID:11;         /**< @brief Standard identifier.*/
    };
    struct {
      uint32_t              EID:29;         /**< @brief Extended identifier.*/
    };
  };
  union {
    uint8_t                 data8[8];       /**< @brief Frame data.         */
    uint16_t                data16[4];      /**< @brief Frame data.         */
    uint32_t                data32[2];      /**< @brief Frame data.         */
  };
} CANRxFrame;

/**
 * @brief   CAN filter.
 * @note    Refer to the SPC5 reference manual for info about filters.
 */
typedef struct {
  /**
   * @brief   Filter scale.
   * @note    This bit represents the EXT bit associated to this
   *          filter (0=standard ID mode, 1=extended ID mode).
   */
  uint32_t                  scale:1;
  /**
   * @brief   Filter register (identifier).
   */
  uint32_t                  register1;
} CANFilter;

/**
 * @brief   Driver configuration structure.
 */
typedef struct {
 /**
   * @brief   CAN MCR register initialization data.
   * @note    Some bits in this register are enforced by the driver regardless
   *          their status in this field.
   */
  uint32_t                  mcr;
  /**
   * @brief   CAN CTRL register initialization data.
   * @note    Some bits in this register are enforced by the driver regardless
   *          their status in this field.
   */
  uint32_t                  ctrl;
#if SPC5_CAN_USE_FILTERS
  /**
   * @brief   CAN filters structure.
   */
  CANFilter                 RxFilter[CAN_RX_MAILBOXES];
#endif
} CANConfig;

/**
 * @brief   Structure representing an CAN driver.
 */
typedef struct {
  /**
   * @brief   Driver state.
   */
  volatile canstate_t       state;
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
#if CAN_USE_SLEEP_MODE || defined (__DOXYGEN__)
  /**
   * @brief   Entering sleep state event.
   */
  event_source_t            sleep_event;
  /**
   * @brief   Exiting sleep state event.
   */
  event_source_t            wakeup_event;
#endif /* CAN_USE_SLEEP_MODE */
  /* End of the mandatory fields.*/
  /**
   * @brief   Pointer to the CAN registers.
   */
  volatile struct spc5_flexcan *flexcan;
} CANDriver;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if SPC5_CAN_USE_FLEXCAN0 && !defined(__DOXYGEN__)
extern CANDriver CAND1;
#endif

#if SPC5_CAN_USE_FLEXCAN1 && !defined(__DOXYGEN__)
extern CANDriver CAND2;
#endif

#if SPC5_CAN_USE_FLEXCAN2 && !defined(__DOXYGEN__)
extern CANDriver CAND3;
#endif

#if SPC5_CAN_USE_FLEXCAN3 && !defined(__DOXYGEN__)
extern CANDriver CAND4;
#endif

#if SPC5_CAN_USE_FLEXCAN4 && !defined(__DOXYGEN__)
extern CANDriver CAND5;
#endif

#if SPC5_CAN_USE_FLEXCAN5 && !defined(__DOXYGEN__)
extern CANDriver CAND6;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void can_lld_init(void);
  void can_lld_start(CANDriver *canp);
  void can_lld_stop(CANDriver *canp);
  bool can_lld_is_tx_empty(CANDriver *canp,
                           canmbx_t mailbox);
  void can_lld_transmit(CANDriver *canp,
                        canmbx_t mailbox,
                        const CANTxFrame *crfp);
  bool can_lld_is_rx_nonempty(CANDriver *canp,
                              canmbx_t mailbox);
  void can_lld_receive(CANDriver *canp,
                       canmbx_t mailbox,
                       CANRxFrame *ctfp);
#if CAN_USE_SLEEP_MODE
  void can_lld_sleep(CANDriver *canp);
  void can_lld_wakeup(CANDriver *canp);
#endif /* CAN_USE_SLEEP_MODE */
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_CAN */

#endif /* HAL_CAN_LLD_H */

/** @} */
