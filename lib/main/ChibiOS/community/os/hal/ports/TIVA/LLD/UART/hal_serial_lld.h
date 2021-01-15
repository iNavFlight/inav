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
 * @file    UART/hal_serial_lld.h
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
 * @brief   Advanced buffering support switch.
 * @details This constants enables the advanced buffering support in the
 *          low level driver, the queue buffer is no more part of the
 *          @p SerialDriver structure, each driver can have a different
 *          queue size.
 */
#define SERIAL_ADVANCED_BUFFERING_SUPPORT   TRUE

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
 * @brief   Input buffer size for UART0.
 */
#if !defined(TIVA_SERIAL_UART0_IN_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART0_IN_BUF_SIZE   SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Output buffer size for UART0.
 */
#if !defined(TIVA_SERIAL_UART0_OUT_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART0_OUT_BUF_SIZE  SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Input buffer size for UART1.
 */
#if !defined(TIVA_SERIAL_UART1_IN_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART1_IN_BUF_SIZE   SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Output buffer size for UART1.
 */
#if !defined(TIVA_SERIAL_UART1_OUT_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART1_OUT_BUF_SIZE  SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Input buffer size for UART2.
 */
#if !defined(TIVA_SERIAL_UART2_IN_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART2_IN_BUF_SIZE   SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Output buffer size for UART2.
 */
#if !defined(TIVA_SERIAL_UART2_OUT_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART2_OUT_BUF_SIZE  SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Input buffer size for UART3.
 */
#if !defined(TIVA_SERIAL_UART3_IN_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART3_IN_BUF_SIZE   SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Output buffer size for UART3.
 */
#if !defined(TIVA_SERIAL_UART3_OUT_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART3_OUT_BUF_SIZE  SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Input buffer size for UART4.
 */
#if !defined(TIVA_SERIAL_UART4_IN_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART4_IN_BUF_SIZE   SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Output buffer size for UART4.
 */
#if !defined(TIVA_SERIAL_UART4_OUT_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART4_OUT_BUF_SIZE  SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Input buffer size for UART5.
 */
#if !defined(TIVA_SERIAL_UART5_IN_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART5_IN_BUF_SIZE   SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Output buffer size for UART5.
 */
#if !defined(TIVA_SERIAL_UART5_OUT_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART5_OUT_BUF_SIZE  SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Input buffer size for UART6.
 */
#if !defined(TIVA_SERIAL_UART6_IN_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART6_IN_BUF_SIZE   SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Output buffer size for UART6.
 */
#if !defined(TIVA_SERIAL_UART6_OUT_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART6_OUT_BUF_SIZE  SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Input buffer size for UART7.
 */
#if !defined(TIVA_SERIAL_UART7_IN_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART7_IN_BUF_SIZE   SERIAL_BUFFERS_SIZE
#endif

/**
 * @brief   Output buffer size for UART7.
 */
#if !defined(TIVA_SERIAL_UART7_OUT_BUF_SIZE) || defined(__DOXYGEN__)
#define TIVA_SERIAL_UART7_OUT_BUF_SIZE  SERIAL_BUFFERS_SIZE
#endif
/** @} */

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
 * @note    This structure content is architecture dependent, each driver
 *          implementation defines its own version and the custom static
 *          initializers.
 */
typedef struct {
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
  /* End of the mandatory fields.*/                                         \
  /* Pointer to the USART registers block.*/                                \
  uint32_t                  uart;

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
