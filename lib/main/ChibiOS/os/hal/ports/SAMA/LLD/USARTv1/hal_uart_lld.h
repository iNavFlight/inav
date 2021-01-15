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
 * @file    USARTv1/hal_uart_lld.h
 * @brief   SAMA low level UART driver header.
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
#if !defined(SAMA_UART_USE_UART0) || defined(__DOXYGEN__)
#define SAMA_UART_USE_UART0                 FALSE
#endif

/**
 * @brief   UART driver on UART1 enable switch.
 * @details If set to @p TRUE the support for UART1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SAMA_UART_USE_UART1) || defined(__DOXYGEN__)
#define SAMA_UART_USE_UART1                 FALSE
#endif

/**
 * @brief   UART driver on UART2 enable switch.
 * @details If set to @p TRUE the support for UART2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SAMA_UART_USE_UART2) || defined(__DOXYGEN__)
#define SAMA_UART_USE_UART2                 FALSE
#endif

/**
 * @brief   UART driver on UART3 enable switch.
 * @details If set to @p TRUE the support for UART4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SAMA_UART_USE_UART3) || defined(__DOXYGEN__)
#define SAMA_UART_USE_UART3                 FALSE
#endif

/**
 * @brief   UART driver on UART4 enable switch.
 * @details If set to @p TRUE the support for UART4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SAMA_UART_USE_UART4) || defined(__DOXYGEN__)
#define SAMA_UART_USE_UART4                FALSE
#endif

/**
 * @brief   UART driver on FLEXCOM0 enable switch.
 * @details If set to @p TRUE the support for FLEXCOM0 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SAMA_UART_USE_FLEXCOM0) || defined(__DOXYGEN__)
#define SAMA_UART_USE_FLEXCOM0              FALSE
#endif

/**
 * @brief   UART driver on FLEXCOM1 enable switch.
 * @details If set to @p TRUE the support for FLEXCOM1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SAMA_UART_USE_FLEXCOM1) || defined(__DOXYGEN__)
#define SAMA_UART_USE_FLEXCOM1              FALSE
#endif

/**
 * @brief   UART driver on FLEXCOM2 enable switch.
 * @details If set to @p TRUE the support for FLEXCOM2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SAMA_UART_USE_FLEXCOM2) || defined(__DOXYGEN__)
#define SAMA_UART_USE_FLEXCOM2              FALSE
#endif

/**
 * @brief   UART driver on FLEXCOM3 enable switch.
 * @details If set to @p TRUE the support for FLEXCOM3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SAMA_UART_USE_FLEXCOM3) || defined(__DOXYGEN__)
#define SAMA_UART_USE_FLEXCOM3              FALSE
#endif

/**
 * @brief   UART driver on FLEXCOM4 enable switch.
 * @details If set to @p TRUE the support for FLEXCOM4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SAMA_UART_USE_FLEXCOM4) || defined(__DOXYGEN__)
#define SAMA_UART_USE_FLEXCOM4              FALSE
#endif

/**
 * @brief   UART0 interrupt priority level setting.
 */
#if !defined(SAMA_UART_UART0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_UART0_IRQ_PRIORITY        4
#endif

/**
 * @brief   UART1 interrupt priority level setting.
 */
#if !defined(SAMA_UART_UART1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_UART1_IRQ_PRIORITY        4
#endif

/**
 * @brief   UART2 interrupt priority level setting.
 */
#if !defined(SAMA_UART_UART2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_UART2_IRQ_PRIORITY        4
#endif

/**
 * @brief   UART3 interrupt priority level setting.
 */
#if !defined(SAMA_UART_UART3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_UART3_IRQ_PRIORITY        4
#endif

/**
 * @brief   UART4 interrupt priority level setting.
 */
#if !defined(SAMA_UART_UART4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_UART4_IRQ_PRIORITY        4
#endif

/**
 * @brief   FLEXCOM0 interrupt priority level setting.
 */
#if !defined(SAMA_UART_FLEXCOM0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_FLEXCOM0_IRQ_PRIORITY     4
#endif

/**
 * @brief   FLEXCOM1 interrupt priority level setting.
 */
#if !defined(SAMA_UART_FLEXCOM1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_FLEXCOM1_IRQ_PRIORITY     4
#endif

/**
 * @brief   FLEXCOM2 interrupt priority level setting.
 */
#if !defined(SAMA_UART_FLEXCOM2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_FLEXCOM2_IRQ_PRIORITY     4
#endif

/**
 * @brief   FLEXCOM3 interrupt priority level setting.
 */
#if !defined(SAMA_UART_FLEXCOM3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_FLEXCOM3_IRQ_PRIORITY     4
#endif

/**
 * @brief   FLEXCOM4 interrupt priority level setting.
 */
#if !defined(SAMA_UART_FLEXCOM4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_FLEXCOM4_IRQ_PRIORITY     4
#endif

/**
 * @brief   UART0 DMA interrupt priority level setting.
 */
#if !defined(SAMA_UART_UART0_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_UART0_DMA_IRQ_PRIORITY    4
#endif

/**
 * @brief   UART1 DMA interrupt priority level setting.
 */
#if !defined(SAMA_UART_UART1_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_UART1_DMA_IRQ_PRIORITY    4
#endif

/**
 * @brief   UART2 DMA interrupt priority level setting.
 */
#if !defined(SAMA_UART_UART2_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_UART2_DMA_IRQ_PRIORITY    4
#endif

/**
 * @brief   UART3 DMA interrupt priority level setting.
 */
#if !defined(SAMA_UART_UART3_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_UART3_DMA_IRQ_PRIORITY    4
#endif

/**
 * @brief   UART4 DMA interrupt priority level setting.
 */
#if !defined(SAMA_UART_UART4_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_UART4_DMA_IRQ_PRIORITY    4
#endif

/**
 * @brief   FLEXCOM0 DMA interrupt priority level setting.
 */
#if !defined(SAMA_UART_FLEXCOM0_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_FLEXCOM0_DMA_IRQ_PRIORITY 4
#endif

/**
 * @brief   FLEXCOM1 DMA interrupt priority level setting.
 */
#if !defined(SAMA_UART_FLEXCOM1_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_FLEXCOM1_DMA_IRQ_PRIORITY 4
#endif

/**
 * @brief   FLEXCOM2 DMA interrupt priority level setting.
 */
#if !defined(SAMA_UART_FLEXCOM2_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_FLEXCOM2_DMA_IRQ_PRIORITY 4
#endif

/**
 * @brief   FLEXCOM3 DMA interrupt priority level setting.
 */
#if !defined(SAMA_UART_FLEXCOM3_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_FLEXCOM3_DMA_IRQ_PRIORITY 4
#endif

/**
 * @brief   FLEXCOM4 DMA interrupt priority level setting.
 */
#if !defined(SAMA_UART_FLEXCOM4_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_UART_FLEXCOM4_DMA_IRQ_PRIORITY 4
#endif

/**
 * @brief   UART DMA error hook.
 * @note    The default action for DMA errors is a system halt because DMA
 *          error can only happen because programming errors.
 */
#if !defined(SAMA_UART_DMA_ERROR_HOOK) || defined(__DOXYGEN__)
#define SAMA_UART_DMA_ERROR_HOOK(uartp)    osalSysHalt("DMA failure")
#endif

/**
 * @brief   UART cache managing.
 */
#if !defined(SAMA_UART_CACHE_USER_MANAGED) || defined(__DOXYGEN__)
#define SAMA_UART_CACHE_USER_MANAGED       FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/**
 * @brief   At least an UART unit is in use.
 */
#define SAMA_UART_USE_UART (SAMA_UART_USE_UART0 ||                          \
                            SAMA_UART_USE_UART1 ||                          \
                            SAMA_UART_USE_UART2 ||                          \
                            SAMA_UART_USE_UART3 ||                          \
                            SAMA_UART_USE_UART4)

/**
 * @brief   At least an FLEXCOM unit is in use.
 */
#define SAMA_UART_USE_FLEXCOM (SAMA_UART_USE_FLEXCOM0 ||                    \
                               SAMA_UART_USE_FLEXCOM1 ||                    \
                               SAMA_UART_USE_FLEXCOM2 ||                    \
                               SAMA_UART_USE_FLEXCOM3 ||                    \
                               SAMA_UART_USE_FLEXCOM4)


#if !SAMA_UART_USE_UART0 && !SAMA_UART_USE_UART1 &&                         \
    !SAMA_UART_USE_UART2 && !SAMA_UART_USE_UART3 &&                         \
    !SAMA_UART_USE_UART4 &&                                                 \
    !SAMA_UART_USE_FLEXCOM0 && !SAMA_UART_USE_FLEXCOM1 &&                   \
    !SAMA_UART_USE_FLEXCOM2 && !SAMA_UART_USE_FLEXCOM3 &&                   \
    !SAMA_UART_USE_FLEXCOM4
#error "UART driver activated but no USART/UART peripheral assigned"
#endif

/* Checks on allocation of UARTx units.*/
#if SAMA_UART_USE_UART0
#if defined(SAMA_UART0_IS_USED)
#error "UARTD0 requires UART0 but the peripheral is already used"
#else
#define SAMA_UART0_IS_USED
#endif
#endif

#if SAMA_UART_USE_UART1
#if defined(SAMA_UART1_IS_USED)
#error "UARTD1 requires UART1 but the peripheral is already used"
#else
#define SAMA_UART1_IS_USED
#endif
#endif

#if SAMA_UART_USE_UART2
#if defined(SAMA_UART2_IS_USED)
#error "UARTD2 requires UART2 but the peripheral is already used"
#else
#define SAMA_UART2_IS_USED
#endif
#endif

#if SAMA_UART_USE_UART3
#if defined(SAMA_UART3_IS_USED)
#error "UARTD3 requires UART3 but the peripheral is already used"
#else
#define SAMA_UART3_IS_USED
#endif
#endif

#if SAMA_UART_USE_UART4
#if defined(SAMA_UART4_IS_USED)
#error "UARTD4 requires UART4 but the peripheral is already used"
#else
#define SAMA_UART4_IS_USED
#endif
#endif

#if SAMA_UART_USE_FLEXCOM0
#if defined(SAMA_FLEXCOM0_IS_USED)
#error "FUARTD0 requires FLEXCOM0 but the peripheral is already used"
#else
#define SAMA_FLEXCOM0_IS_USED
#endif
#endif

#if SAMA_UART_USE_FLEXCOM1
#if defined(SAMA_FLEXCOM1_IS_USED)
#error "FUARTD1 requires FLEXCOM1 but the peripheral is already used"
#else
#define SAMA_FLEXCOM1_IS_USED
#endif
#endif

#if SAMA_UART_USE_FLEXCOM2
#if defined(SAMA_FLEXCOM2_IS_USED)
#error "FUARTD2 requires FLEXCOM2 but the peripheral is already used"
#else
#define SAMA_FLEXCOM2_IS_USED
#endif
#endif

#if SAMA_UART_USE_FLEXCOM3
#if defined(SAMA_FLEXCOM3_IS_USED)
#error "FUARTD3 requires FLEXCOM3 but the peripheral is already used"
#else
#define SAMA_FLEXCOM3_IS_USED
#endif
#endif

#if SAMA_UART_USE_FLEXCOM4
#if defined(SAMA_FLEXCOM4_IS_USED)
#error "FUARTD4 requires FLEXCOM4 but the peripheral is already used"
#else
#define SAMA_FLEXCOM4_IS_USED
#endif
#endif

#if !defined(SAMA_DMA_REQUIRED)
#define SAMA_DMA_REQUIRED
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
   * @brief   Receiver timeout callback.
   * @details Handles both idle and timeout interrupts depending on configured
   *          flags in CR registers and supported hardware features.
   */
  uartcb_t                  timeout_cb;
  /**
   * @brief   Receiver timeout value in terms of number of bit duration.
   * @details Set it to 0 when you want to handle idle interrupt instead of
   *          hardware timeout.
   */
  uint32_t                  timeout;
  /**
   * @brief   Bit rate.
   */
  uint32_t                  speed;
  /**
   * @brief   Initialization value for the CR register.
   */
  uint32_t                  cr;
  /**
   * @brief   Initialization value for the MR register.
   */
  uint32_t                  mr;
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
#if SAMA_UART_USE_UART
  /**
   * @brief   Pointer to the UART registers block.
   */
  Uart                      *uart;
#endif
#if SAMA_UART_USE_FLEXCOM
  /* Pointer to the FLEXCOM registers block.*/
  Flexcom                   *flexcom;
  /* Pointer to the USART registers block.*/
  Usart                     *usart;
#endif
  /**
   * @brief   Clock frequency for the associated USART/UART.
   */
  uint32_t                  clock;
  /**
   * @brief   DMA mode bit mask.
   */
  uint32_t                  rxdmamode;
  /**
    * @brief   DMA mode bit mask.
    */
  uint32_t                  txdmamode;
  /**
   * @brief   Receive DMA channel.
   */
  sama_dma_channel_t        *dmarx;
  /**
   * @brief   Transmit DMA channel.
   */
  sama_dma_channel_t        *dmatx;
  /**
   * @brief   Default receive buffer while into @p UART_RX_IDLE state.
   */
  CACHE_ALIGNED
  volatile uint16_t         rxbuf;
  /**
   * @brief     Pointer to the TX buffer location.
   */
  const uint8_t            *txbufp;
  /**
   * @brief     Pointer to the RX buffer location.
   */
  uint8_t                  *rxbufp;
  /**
   * @brief     Number of bytes in RX phase.
   */
  size_t                   rxbytes;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if SAMA_UART_USE_UART0 && !defined(__DOXYGEN__)
extern UARTDriver UARTD0;
#endif

#if SAMA_UART_USE_UART1 && !defined(__DOXYGEN__)
extern UARTDriver UARTD1;
#endif

#if SAMA_UART_USE_UART2 && !defined(__DOXYGEN__)
extern UARTDriver UARTD2;
#endif

#if SAMA_UART_USE_UART3 && !defined(__DOXYGEN__)
extern UARTDriver UARTD3;
#endif

#if SAMA_UART_USE_UART4 && !defined(__DOXYGEN__)
extern UARTDriver UARTD4;
#endif

#if SAMA_UART_USE_FLEXCOM0 && !defined(__DOXYGEN__)
extern UARTDriver FUARTD0;
#endif

#if SAMA_UART_USE_FLEXCOM1 && !defined(__DOXYGEN__)
extern UARTDriver FUARTD1;
#endif

#if SAMA_UART_USE_FLEXCOM2 && !defined(__DOXYGEN__)
extern UARTDriver FUARTD2;
#endif

#if SAMA_UART_USE_FLEXCOM3 && !defined(__DOXYGEN__)
extern UARTDriver FUARTD3;
#endif

#if SAMA_UART_USE_FLEXCOM4 && !defined(__DOXYGEN__)
extern UARTDriver FUARTD4;
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
