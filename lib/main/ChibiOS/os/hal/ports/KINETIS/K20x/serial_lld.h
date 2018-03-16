/*
    ChibiOS - Copyright (C) 2014-2015 Fabio Utzig

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
 * @file    K20x/serial_lld.h
 * @brief   Kinetis K20x Serial Driver subsystem low level driver header.
 *
 * @addtogroup SERIAL
 * @{
 */

#ifndef _SERIAL_LLD_H_
#define _SERIAL_LLD_H_

#if HAL_USE_SERIAL || defined(__DOXYGEN__)

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
 * @brief   SD1 driver enable switch.
 * @details If set to @p TRUE the support for SD1 is included.
 */
#if !defined(KINETIS_SERIAL_USE_UART0) || defined(__DOXYGEN__)
#define KINETIS_SERIAL_USE_UART0             FALSE
#endif
/**
 * @brief   SD2 driver enable switch.
 * @details If set to @p TRUE the support for SD2 is included.
 */
#if !defined(KINETIS_SERIAL_USE_UART1) || defined(__DOXYGEN__)
#define KINETIS_SERIAL_USE_UART1             FALSE
#endif
/**
 * @brief   SD3 driver enable switch.
 * @details If set to @p TRUE the support for SD3 is included.
 */
#if !defined(KINETIS_SERIAL_USE_UART2) || defined(__DOXYGEN__)
#define KINETIS_SERIAL_USE_UART2             FALSE
#endif

/**
 * @brief   UART0 interrupt priority level setting.
 */
#if !defined(KINETIS_SERIAL_UART0_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_SERIAL_UART0_PRIORITY        12
#endif

/**
 * @brief   UART1 interrupt priority level setting.
 */
#if !defined(KINETIS_SERIAL_UART1_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_SERIAL_UART1_PRIORITY        12
#endif

/**
 * @brief   UART2 interrupt priority level setting.
 */
#if !defined(KINETIS_SERIAL_UART2_PRIORITY) || defined(__DOXYGEN__)
#define KINETIS_SERIAL_UART2_PRIORITY        12
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Generic Serial Driver configuration structure.
 * @details An instance of this structure must be passed to @p sdStart()
 *          in order to configure and start a serial driver operations.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
typedef struct {
  /**
   * @brief Bit rate.
   */
  uint32_t                  sc_speed;
  /* End of the mandatory fields.*/
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
  /* Pointer to the UART registers block.*/                                 \
  UART_TypeDef            *uart;

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if KINETIS_SERIAL_USE_UART0 && !defined(__DOXYGEN__)
extern SerialDriver SD1;
#endif

#if KINETIS_SERIAL_USE_UART1 && !defined(__DOXYGEN__)
extern SerialDriver SD2;
#endif

#if KINETIS_SERIAL_USE_UART2 && !defined(__DOXYGEN__)
extern SerialDriver SD3;
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

#endif /* _SERIAL_LLD_H_ */

/** @} */
