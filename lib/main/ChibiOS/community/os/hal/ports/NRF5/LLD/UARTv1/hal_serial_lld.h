/*
    Copyright (C) 2015 Fabio Utzig

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
 * @file    UARTv1/hal_serial_lld.h
 * @brief   NRF5 serial subsystem low level driver header.
 *
 * @addtogroup SERIAL
 * @{
 */

#ifndef HAL_SERIAL_LLD_H
#define HAL_SERIAL_LLD_H

#if (HAL_USE_SERIAL == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    PLATFORM configuration options
 * @{
 */
/**
 * @brief   SD flow control enable switch.
 * @details If set to @p TRUE the support for hardware flow control
 *          is included.
 * @note    The default is @p FALSE.
 */
#if !defined(NRF5_SERIAL_USE_HWFLOWCTRL) || defined(__DOXYGEN__)
#define NRF5_SERIAL_USE_HWFLOWCTRL        FALSE
#endif

/**
 * @brief   SD1 driver enable switch.
 * @details If set to @p TRUE the support for SD1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(NRF5_SERIAL_USE_UART0) || defined(__DOXYGEN__)
#define NRF5_SERIAL_USE_UART0             FALSE
#endif

/**
 * @brief   UART0 interrupt priority level setting.
 */
#if !defined(NRF5_SERIAL_UART0_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_SERIAL_UART0_PRIORITY        3
#endif

/* Value indicating that no pad is connected to this UART register. */
#define  NRF5_SERIAL_PAD_DISCONNECTED 0xFFFFFFFFU
#define  NRF5_SERIAL_INVALID_BAUDRATE 0xFFFFFFFFU

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if NRF5_SERIAL_USE_UART0 &&					\
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_SERIAL_UART0_PRIORITY)
#error "Invalid IRQ priority assigned to UART0"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   NRF51 Serial Driver configuration structure.
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
  /* End of the mandatory fields.*/
  uint32_t                  tx_pad;
  uint32_t                  rx_pad;
#if (NRF5_SERIAL_USE_HWFLOWCTRL == TRUE)
  uint32_t                  rts_pad;
  uint32_t                  cts_pad;
#endif
} SerialConfig;

/**
 * @brief   @p SerialDriver specific data.
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
  /* 1 if port is busy transmitting, 0 otherwise. */                        \
  uint8_t                   tx_busy;                                        \
  /* End of the mandatory fields.*/                                         \
  thread_t                  *thread;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (NRF5_SERIAL_USE_UART0 == TRUE) && !defined(__DOXYGEN__)
extern SerialDriver SD1;
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

#endif /* HAL_USE_SERIAL == TRUE */

#endif /* HAL_SERIAL_LLD_H */

/** @} */
