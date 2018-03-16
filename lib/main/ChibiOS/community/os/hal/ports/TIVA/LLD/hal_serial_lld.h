/*
    Copyright (C) 2014..2016 Marco Veeneman

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
 * @file    TIVA/LLD/serial_lld.h
 * @brief   Tiva low level serial driver header.
 *
 * @addtogroup SERIAL
 * @{
 */

#ifndef HAL_SERIAL_LLD_H
#define HAL_SERIAL_LLD_H

#if HAL_USE_SERIAL || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    FR register bits definitions
 * @{
 */

#define TIVA_FR_CTS         (1 << 0)

#define TIVA_FR_BUSY        (1 << 3)

#define TIVA_FR_RXFE        (1 << 4)

#define TIVA_FR_TXFF        (1 << 5)

#define TIVA_FR_RXFF        (1 << 6)

#define TIVA_FR_TXFE        (1 << 7)

/**
 * @}
 */

/**
 * @name    LCRH register bits definitions
 * @{
 */

#define TIVA_LCRH_BRK       (1 << 0)

#define TIVA_LCRH_PEN       (1 << 1)

#define TIVA_LCRH_EPS       (1 << 2)

#define TIVA_LCRH_STP2      (1 << 3)

#define TIVA_LCRH_FEN       (1 << 4)

#define TIVA_LCRH_WLEN_MASK (3 << 5)
#define TIVA_LCRH_WLEN_5    (0 << 5)
#define TIVA_LCRH_WLEN_6    (1 << 5)
#define TIVA_LCRH_WLEN_7    (2 << 5)
#define TIVA_LCRH_WLEN_8    (3 << 5)

#define TIVA_LCRH_SPS       (1 << 7)

/**
 * @}
 */
 
/**
 * @name    CTL register bits definitions
 * @{
 */

#define TIVA_CTL_UARTEN     (1 << 0)

#define TIVA_CTL_SIREN      (1 << 1)

#define TIVA_CTL_SIRLP      (1 << 2)

#define TIVA_CTL_SMART      (1 << 3)

#define TIVA_CTL_EOT        (1 << 4)

#define TIVA_CTL_HSE        (1 << 5)

#define TIVA_CTL_LBE        (1 << 7)

#define TIVA_CTL_TXE        (1 << 8)

#define TIVA_CTL_RXE        (1 << 9)

#define TIVA_CTL_RTS        (1 << 11)

#define TIVA_CTL_RTSEN      (1 << 14)

#define TIVA_CTL_CTSEN      (1 << 15)

/**
 * @}
 */

/**
 * @name    IFLS register bits definitions
 * @{
 */

#define TIVA_IFLS_TXIFLSEL_MASK     (7 << 0)
#define TIVA_IFLS_TXIFLSEL_1_8_F    (0 << 0)
#define TIVA_IFLS_TXIFLSEL_1_4_F    (1 << 0)
#define TIVA_IFLS_TXIFLSEL_1_2_F    (2 << 0)
#define TIVA_IFLS_TXIFLSEL_3_4_F    (3 << 0)
#define TIVA_IFLS_TXIFLSEL_7_8_F    (4 << 0)

#define TIVA_IFLS_RXIFLSEL_MASK     (7 << 3)
#define TIVA_IFLS_RXIFLSEL_7_8_E    (0 << 3)
#define TIVA_IFLS_RXIFLSEL_3_4_E    (1 << 3)
#define TIVA_IFLS_RXIFLSEL_1_2_E    (2 << 3)
#define TIVA_IFLS_RXIFLSEL_1_4_E    (3 << 3)
#define TIVA_IFLS_RXIFLSEL_1_8_E    (4 << 3)

/**
 * @}
 */

/**
 * @name    MIS register bits definitions
 * @{
 */

#define TIVA_MIS_CTSMIS             (1 << 1)

#define TIVA_MIS_RXMIS              (1 << 4)

#define TIVA_MIS_TXMIS              (1 << 5)

#define TIVA_MIS_RTMIS              (1 << 6)

#define TIVA_MIS_FEMIS              (1 << 7)

#define TIVA_MIS_PEMIS              (1 << 8)

#define TIVA_MIS_BEMIS              (1 << 9)

#define TIVA_MIS_OEMIS              (1 << 10)

#define TIVA_MIS_9BITMIS            (1 << 12)

/**
 * @}
 */

/**
 * @name    IM register bits definitions
 * @{
 */

#define TIVA_IM_CTSIM               (1 << 1)

#define TIVA_IM_RXIM                (1 << 4)

#define TIVA_IM_TXIM                (1 << 5)

#define TIVA_IM_RTIM                (1 << 6)

#define TIVA_IM_FEIM                (1 << 7)

#define TIVA_IM_PEIM                (1 << 8)

#define TIVA_IM_BEIM                (1 << 9)

#define TIVA_IM_OEIM                (1 << 10)

#define TIVA_IM_9BITIM              (1 << 12)

/**
 * @}
 */
/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Configuration options
 * @{
 */

/**
 * @brief   UART0 driver enable switch.
 * @details If set to @p TRUE the support for UART0 is included.
 * @note    The default is @p TRUE.
 */
#if !defined(TIVA_SERIAL_USE_UART0) || defined(__DOXYGEN__)
#define TIVA_SERIAL_USE_UART0           FALSE
#endif

/**
 * @brief   UART1 driver enable switch.
 * @details If set to @p TRUE the support for UART1 is included.
 * @note    The default is @p FALSE .
 */
#if !defined(TIVA_SERIAL_USE_UART1) || defined(__DOXYGEN__)
#define TIVA_SERIAL_USE_UART1           FALSE
#endif

/**
 * @brief   UART2 driver enable switch.
 * @details If set to @p TRUE the support for UART2 is included.
 * @note    The default is @p FALSE .
 */
#if !defined(TIVA_SERIAL_USE_UART2) || defined(__DOXYGEN__)
#define TIVA_SERIAL_USE_UART2           FALSE
#endif

/**
 * @brief   UART3 driver enable switch.
 * @details If set to @p TRUE the support for UART3 is included.
 * @note    The default is @p FALSE .
 */
#if !defined(TIVA_SERIAL_USE_UART3) || defined(__DOXYGEN__)
#define TIVA_SERIAL_USE_UART3           FALSE
#endif

/**
 * @brief   UART4 driver enable switch.
 * @details If set to @p TRUE the support for UART4 is included.
 * @note    The default is @p FALSE .
 */
#if !defined(TIVA_SERIAL_USE_UART4) || defined(__DOXYGEN__)
#define TIVA_SERIAL_USE_UART4           FALSE
#endif

/**
 * @brief   UART5 driver enable switch.
 * @details If set to @p TRUE the support for UART5 is included.
 * @note    The default is @p FALSE .
 */
#if !defined(TIVA_SERIAL_USE_UART5) || defined(__DOXYGEN__)
#define TIVA_SERIAL_USE_UART5           FALSE
#endif

/**
 * @brief   UART6 driver enable switch.
 * @details If set to @p TRUE the support for UART6 is included.
 * @note    The default is @p FALSE .
 */
#if !defined(TIVA_SERIAL_USE_UART6) || defined(__DOXYGEN__)
#define TIVA_SERIAL_USE_UART6           FALSE
#endif

/**
 * @brief   UART7 driver enable switch.
 * @details If set to @p TRUE the support for UART7 is included.
 * @note    The default is @p FALSE .
 */
#if !defined(TIVA_SERIAL_USE_UART7) || defined(__DOXYGEN__)
#define TIVA_SERIAL_USE_UART7           FALSE
#endif

/**
 * @brief   UART0 interrupt priority level setting.
 */
#if !defined(TIVA_SERIAL_UART0_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART0_PRIORITY      5
#endif

/**
 * @brief   UART1 interrupt priority level setting.
 */
#if !defined(TIVA_SERIAL_UART1_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART1_PRIORITY      5
#endif

/**
 * @brief   UART2 interrupt priority level setting.
 */
#if !defined(TIVA_SERIAL_UART2_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART2_PRIORITY      5
#endif

/**
 * @brief   UART3 interrupt priority level setting.
 */
#if !defined(TIVA_SERIAL_UART3_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART3_PRIORITY      5
#endif

/**
 * @brief   UART4 interrupt priority level setting.
 */
#if !defined(TIVA_SERIAL_UART4_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART4_PRIORITY      5
#endif

/**
 * @brief   UART5 interrupt priority level setting.
 */
#if !defined(TIVA_SERIAL_UART5_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART5_PRIORITY      5
#endif

/**
 * @brief   UART6 interrupt priority level setting.
 */
#if !defined(TIVA_SERIAL_UART6_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART6_PRIORITY      5
#endif

/**
 * @brief   UART7 interrupt priority level setting.
 */
#if !defined(TIVA_SERIAL_UART7_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART7_PRIORITY      5
#endif

/**
 * @}
 */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if !TIVA_SERIAL_USE_UART0 && !TIVA_SERIAL_USE_UART1 && \
    !TIVA_SERIAL_USE_UART2 && !TIVA_SERIAL_USE_UART3 && \
	!TIVA_SERIAL_USE_UART4 && !TIVA_SERIAL_USE_UART5 && \
    !TIVA_SERIAL_USE_UART6 && !TIVA_SERIAL_USE_UART7
#error "SERIAL driver activated but no UART peripheral assigned"
#endif

#if TIVA_SERIAL_USE_UART0 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_SERIAL_UART0_PRIORITY)
#error "Invalid IRQ priority assigned to UART0"
#endif

#if TIVA_SERIAL_USE_UART1 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_SERIAL_UART1_PRIORITY)
#error "Invalid IRQ priority assigned to UART1"
#endif

#if TIVA_SERIAL_USE_UART2 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_SERIAL_UART2_PRIORITY)
#error "Invalid IRQ priority assigned to UART2"
#endif

#if TIVA_SERIAL_USE_UART3 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_SERIAL_UART3_PRIORITY)
#error "Invalid IRQ priority assigned to UART3"
#endif

#if TIVA_SERIAL_USE_UART4 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_SERIAL_UART4_PRIORITY)
#error "Invalid IRQ priority assigned to UART4"
#endif

#if TIVA_SERIAL_USE_UART5 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_SERIAL_UART5_PRIORITY)
#error "Invalid IRQ priority assigned to UART5"
#endif

#if TIVA_SERIAL_USE_UART6 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_SERIAL_UART6_PRIORITY)
#error "Invalid IRQ priority assigned to UART6"
#endif

#if TIVA_SERIAL_USE_UART7 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_SERIAL_UART7_PRIORITY)
#error "Invalid IRQ priority assigned to UART7"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Tiva Serial Driver configuration structure.
 * @details An instance of this structure must be passed to @p sdStart()
 *          in order to configure and start a serial driver operations.
 */
typedef struct {
  /**
   * @brief Bit rate.
   */
  uint32_t                  sc_speed;
  /* End of the mandatory fields. */
  /**
   * @brief Initialization value for the LCRH (Line Control) register.
   */
  uint32_t                  sc_lcrh;
  /**
   * @brief Initialization value for the IFLS (Interrupt FIFO Level Select)
   * register.
   */
  uint32_t                  sc_ifls;
} SerialConfig;

/**
 * @brief @p SerialDriver specific data.
 */
#define _serial_driver_data                                                 \
  _base_asynchronous_channel_data                                           \
  /* Driver state.*/                                                        \
  sdstate_t                 state;                                          \
  /* Input queue.*/                                                         \
  input_queue_t             iqueue;                                         \
  /* Output queue.*/                                                        \
  output_queue_t            oqueue;                                         \
  /* Input circular buffer.*/                                               \
  uint8_t                   ib[SERIAL_BUFFERS_SIZE];                        \
  /* Output circular buffer.*/                                              \
  uint8_t                   ob[SERIAL_BUFFERS_SIZE];                        \
  /* End of the mandatory fields.*/                                         \
  /* Pointer to the USART registers block.*/                                \
  UART_TypeDef              *uart;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if TIVA_SERIAL_USE_UART0 && !defined(__DOXYGEN__)
extern SerialDriver SD1;
#endif

#if TIVA_SERIAL_USE_UART1 && !defined(__DOXYGEN__)
extern SerialDriver SD2;
#endif

#if TIVA_SERIAL_USE_UART2 && !defined(__DOXYGEN__)
extern SerialDriver SD3;
#endif

#if TIVA_SERIAL_USE_UART3 && !defined(__DOXYGEN__)
extern SerialDriver SD4;
#endif

#if TIVA_SERIAL_USE_UART4 && !defined(__DOXYGEN__)
extern SerialDriver SD5;
#endif

#if TIVA_SERIAL_USE_UART5 && !defined(__DOXYGEN__)
extern SerialDriver SD6;
#endif

#if TIVA_SERIAL_USE_UART6 && !defined(__DOXYGEN__)
extern SerialDriver SD7;
#endif

#if TIVA_SERIAL_USE_UART7 && !defined(__DOXYGEN__)
extern SerialDriver SD8;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void sd_lld_init(void);
  void sd_lld_start(SerialDriver *sdp, const SerialConfig *config);
  void sd_lld_stop(SerialDriver *sdp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SERIAL */

#endif /* HAL_SERIAL_LLD_H */

/** @} */
