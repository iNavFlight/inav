/*
    Copyright (C) 2014..2017 Marco Veeneman

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
 * @file    UART/hal_uart_lld.h
 * @brief   Tiva low level UART driver header.
 *
 * @addtogroup UART
 * @{
 */

#ifndef HAL_UART_LLD_H
#define HAL_UART_LLD_H

#if HAL_USE_UART || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */
/**
 * @brief   UART driver on UART0 enable switch.
 * @details If set to @p TRUE the support for UART0 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_UART_USE_UART0) || defined(__DOXYGEN__)
#define TIVA_UART_USE_UART0               FALSE
#endif

/**
 * @brief   UART driver on UART1 enable switch.
 * @details If set to @p TRUE the support for UART1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_UART_USE_UART1) || defined(__DOXYGEN__)
#define TIVA_UART_USE_UART1               FALSE
#endif

/**
 * @brief   UART driver on UART2 enable switch.
 * @details If set to @p TRUE the support for UART2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_UART_USE_UART2) || defined(__DOXYGEN__)
#define TIVA_UART_USE_UART2               FALSE
#endif

/**
 * @brief   UART driver on UART3 enable switch.
 * @details If set to @p TRUE the support for UART3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_UART_USE_UART3) || defined(__DOXYGEN__)
#define TIVA_UART_USE_UART3               FALSE
#endif

/**
 * @brief   UART driver on UART4 enable switch.
 * @details If set to @p TRUE the support for UART4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_UART_USE_UART4) || defined(__DOXYGEN__)
#define TIVA_UART_USE_UART4               FALSE
#endif

/**
 * @brief   UART driver on UART5 enable switch.
 * @details If set to @p TRUE the support for UART5 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_UART_USE_UART5) || defined(__DOXYGEN__)
#define TIVA_UART_USE_UART5               FALSE
#endif

/**
 * @brief   UART driver on UART6 enable switch.
 * @details If set to @p TRUE the support for UART6 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_UART_USE_UART6) || defined(__DOXYGEN__)
#define TIVA_UART_USE_UART6               FALSE
#endif

/**
 * @brief   UART driver on UART7 enable switch.
 * @details If set to @p TRUE the support for UART7 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_UART_USE_UART7) || defined(__DOXYGEN__)
#define TIVA_UART_USE_UART7               FALSE
#endif

/**
 * @brief   UART0 interrupt priority level setting.
 */
#if !defined(TIVA_UART_UART0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_UART_UART0_IRQ_PRIORITY      5
#endif

/**
 * @brief   UART1 interrupt priority level setting.
 */
#if !defined(TIVA_UART_UART1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_UART_UART1_IRQ_PRIORITY      5
#endif

/**
 * @brief   UART2 interrupt priority level setting.
 */
#if !defined(TIVA_UART_UART2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_UART_UART2_IRQ_PRIORITY      5
#endif

/**
 * @brief   UART3 interrupt priority level setting.
 */
#if !defined(TIVA_UART_UART3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_UART_UART3_IRQ_PRIORITY      5
#endif

/**
 * @brief   UART4 interrupt priority level setting.
 */
#if !defined(TIVA_UART_UART4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_UART_UART4_IRQ_PRIORITY      5
#endif

/**
 * @brief   UART5 interrupt priority level setting.
 */
#if !defined(TIVA_UART_UART5_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_UART_UART5_IRQ_PRIORITY      5
#endif

/**
 * @brief   UART6 interrupt priority level setting.
 */
#if !defined(TIVA_UART_UART6_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_UART_UART6_IRQ_PRIORITY      5
#endif

/**
 * @brief   UART7 interrupt priority level setting.
 */
#if !defined(TIVA_UART_UART7_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_UART_UART7_IRQ_PRIORITY      5
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if TIVA_UART_USE_UART0 && !TIVA_HAS_UART0
#error "UART0 not present in the selected device"
#endif

#if TIVA_UART_USE_UART1 && !TIVA_HAS_UART1
#error "UART1 not present in the selected device"
#endif

#if TIVA_UART_USE_UART2 && !TIVA_HAS_UART2
#error "UART2 not present in the selected device"
#endif

#if TIVA_UART_USE_UART3 && !TIVA_HAS_UART3
#error "UART3 not present in the selected device"
#endif

#if TIVA_UART_USE_UART4 && !TIVA_HAS_UART4
#error "UART4 not present in the selected device"
#endif

#if TIVA_UART_USE_UART5 && !TIVA_HAS_UART5
#error "UART5 not present in the selected device"
#endif

#if TIVA_UART_USE_UART6 && !TIVA_HAS_UART6
#error "UART6 not present in the selected device"
#endif

#if TIVA_UART_USE_UART7 && !TIVA_HAS_UART7
#error "UART7 not present in the selected device"
#endif

#if !TIVA_UART_USE_UART0 && !TIVA_UART_USE_UART1 && !TIVA_UART_USE_UART2 && \
    !TIVA_UART_USE_UART3 && !TIVA_UART_USE_UART4 && !TIVA_UART_USE_UART5 && \
    !TIVA_UART_USE_UART6 && !TIVA_UART_USE_UART7
#error "UART driver activated but no UART peripheral assigned"
#endif

#if TIVA_UART_USE_UART0 &&                                                \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_UART_UART0_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to UART0"
#endif

#if TIVA_UART_USE_UART1 &&                                                \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_UART_UART1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to UART1"
#endif

#if TIVA_UART_USE_UART2 &&                                                \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_UART_UART2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to UART2"
#endif

#if TIVA_UART_USE_UART3 &&                                                \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_UART_UART3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to UART3"
#endif

#if TIVA_UART_USE_UART4 &&                                                \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_UART_UART4_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to UART4"
#endif

#if TIVA_UART_USE_UART5 &&                                                \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_UART_UART5_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to UART5"
#endif

#if TIVA_UART_USE_UART6 &&                                                \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_UART_UART6_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to UART6"
#endif

#if TIVA_UART_USE_UART7 &&                                                \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_UART_UART7_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to UART7"
#endif

#if !defined(TIVA_UDMA_REQUIRED)
#define TIVA_UDMA_REQUIRED
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   UART driver condition flags type.
 */
typedef uint32_t uartflags_t;

/**
 * @brief   Structure representing an UART driver.
 */
typedef struct UARTDriver UARTDriver;

/**
 * @brief   Generic UART notification callback type.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 */
typedef void (*uartcb_t)(UARTDriver *uartp);

/**
 * @brief   Character received UART notification callback type.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 * @param[in] c         received character
 */
typedef void (*uartccb_t)(UARTDriver *uartp, uint16_t c);

/**
 * @brief   Receive error UART notification callback type.
 *
 * @param[in] uartp     pointer to the @p UARTDriver object
 * @param[in] e         receive error mask
 */
typedef void (*uartecb_t)(UARTDriver *uartp, uartflags_t e);

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief   End of transmission buffer callback.
   */
  uartcb_t                  txend1_cb;
  /**
   * @brief   Physical end of transmission callback.
   */
  uartcb_t                  txend2_cb;
  /**
   * @brief   Receive buffer filled callback.
   */
  uartcb_t                  rxend_cb;
  /**
   * @brief   Character received while out if the @p UART_RECEIVE state.
   */
  uartccb_t                 rxchar_cb;
  /**
   * @brief   Receive error callback.
   */
  uartecb_t                 rxerr_cb;
  /* End of the mandatory fields.*/
  /**
   * @brief Bit rate.
   */
  uint32_t                  speed;
  /* End of the mandatory fields. */
  /**
   * @brief Initialization value for the CTL register.
   */
  uint16_t                  ctl;
  /**
   * @brief Initialization value for the LCRH register.
   */
  uint8_t                   lcrh;
  /**
   * @brief Initialization value for the IFLS register.
   */
  uint8_t                   ifls;
  /**
   * @brief Initialization value for the CC register.
   */
  uint8_t                   cc;
} UARTConfig;

/**
 * @brief   Structure representing an UART driver.
 */
struct UARTDriver {
  /**
   * @brief   Driver state.
   */
  uartstate_t               state;
  /**
   * @brief   Transmitter state.
   */
  uarttxstate_t             txstate;
  /**
   * @brief   Receiver state.
   */
  uartrxstate_t             rxstate;
  /**
   * @brief   Current configuration data.
   */
  const UARTConfig          *config;
#if (UART_USE_WAIT == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Synchronization flag for transmit operations.
   */
  bool                      early;
  /**
   * @brief   Waiting thread on RX.
   */
  thread_reference_t        threadrx;
  /**
   * @brief   Waiting thread on TX.
   */
  thread_reference_t        threadtx;
#endif /* UART_USE_WAIT */
#if (UART_USE_MUTUAL_EXCLUSION == TRUE) || defined(__DOXYGEN__)
  /**
   * @brief   Mutex protecting the peripheral.
   */
  mutex_t                   mutex;
#endif /* UART_USE_MUTUAL_EXCLUSION */
#if defined(UART_DRIVER_EXT_FIELDS)
  UART_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief Pointer to the UART registers block.
   */
  uint32_t                  uart;
  /**
   * @brief Receive DMA channel number.
   */
  uint8_t                   dmarxnr;
  /**
   * @brief Transmit DMA channel number.
   */
  uint8_t                   dmatxnr;
  /**
   * @brief Receive DMA channel map.
   */
  uint8_t                   rxchnmap;
  /**
   * @brief Transmit DMA channel map.
   */
  uint8_t                   txchnmap;
  /**
   * @brief   Default receive buffer while into @p UART_RX_IDLE state.
   */
  volatile uint16_t         rxbuf;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if TIVA_UART_USE_UART0 && !defined(__DOXYGEN__)
extern UARTDriver UARTD1;
#endif

#if TIVA_UART_USE_UART1 && !defined(__DOXYGEN__)
extern UARTDriver UARTD2;
#endif

#if TIVA_UART_USE_UART2 && !defined(__DOXYGEN__)
extern UARTDriver UARTD3;
#endif

#if TIVA_UART_USE_UART3 && !defined(__DOXYGEN__)
extern UARTDriver UARTD4;
#endif

#if TIVA_UART_USE_UART4 && !defined(__DOXYGEN__)
extern UARTDriver UARTD5;
#endif

#if TIVA_UART_USE_UART5 && !defined(__DOXYGEN__)
extern UARTDriver UARTD6;
#endif

#if TIVA_UART_USE_UART6 && !defined(__DOXYGEN__)
extern UARTDriver UARTD7;
#endif

#if TIVA_UART_USE_UART7 && !defined(__DOXYGEN__)
extern UARTDriver UARTD8;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void uart_lld_init(void);
  void uart_lld_start(UARTDriver *uartp);
  void uart_lld_stop(UARTDriver *uartp);
  void uart_lld_start_send(UARTDriver *uartp, size_t n, const void *txbuf);
  size_t uart_lld_stop_send(UARTDriver *uartp);
  void uart_lld_start_receive(UARTDriver *uartp, size_t n, void *rxbuf);
  size_t uart_lld_stop_receive(UARTDriver *uartp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_UART */

#endif /* HAL_UART_LLD_H */

/** @} */
