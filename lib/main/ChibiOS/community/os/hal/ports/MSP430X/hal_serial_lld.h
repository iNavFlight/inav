/*
    ChibiOS - Copyright (C) 2016 Andrew Wygle aka awygle

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
 * @file    MSP430X/hal_serial_lld.h
 * @brief   MSP430X serial subsystem low level driver header.
 *
 * @addtogroup SERIAL
 * @{
 */

#ifndef _SERIAL_LLD_H_
#define _SERIAL_LLD_H_

#if (HAL_USE_SERIAL == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define NONE 0
#define ODD  2
#define EVEN 3

#define MSB 1
#define LSB 0

#define SEVEN 1
#define EIGHT 0

#define ONE 0
#define TWO 1

#define MSP430X_SERIAL_SMCLK UCSSEL__SMCLK
#define MSP430X_SERIAL_UCLK UCSSEL__UCLK
#define MSP430X_SERIAL_ACLK UCSSEL__ACLK

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    USART0 configuration options
 * @{
 */
/**
 * @brief   USART0 driver enable switch.
 * @details If set to @p TRUE the support for USART1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_SERIAL_USE_USART0) || defined(__DOXYGEN__)
#define MSP430X_SERIAL_USE_USART0             FALSE
#endif
/** @} */

/**
 * @name    USART1 configuration options
 * @{
 */
/**
 * @brief   USART1 driver enable switch.
 * @details If set to @p TRUE the support for USART1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_SERIAL_USE_USART1) || defined(__DOXYGEN__)
#define MSP430X_SERIAL_USE_USART1             FALSE
#endif
/** @} */

/**
 * @name    USART2 configuration options
 * @{
 */
/**
 * @brief   USART2 driver enable switch.
 * @details If set to @p TRUE the support for USART1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_SERIAL_USE_USART2) || defined(__DOXYGEN__)
#define MSP430X_SERIAL_USE_USART2             FALSE
#endif
/** @} */

/**
 * @name    USART3 configuration options
 * @{
 */
/**
 * @brief   USART3 driver enable switch.
 * @details If set to @p TRUE the support for USART1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(MSP430X_SERIAL_USE_USART3) || defined(__DOXYGEN__)
#define MSP430X_SERIAL_USE_USART3             FALSE
#endif

#if MSP430X_SERIAL_USE_USART0
  #if !defined(MSP430X_USART0_PARITY)
    #define MSP430X_USART0_PARITY NONE
  #endif
  #if !defined(MSP430X_USART0_ORDER)
    #define MSP430X_USART0_ORDER LSB
  #endif
  #if !defined(MSP430X_USART0_SIZE)
    #define MSP430X_USART0_SIZE EIGHT
  #endif
  #if !defined(MSP430X_USART0_STOP)
    #define MSP430X_USART0_STOP ONE
  #endif
  #if !defined(MSP430X_USART0_CLK_SRC)
    #define MSP430X_USART0_CLK_SRC MSP430X_UCLK_SRC
    #ifndef MSP430X_USART0_CLK_FREQ
      #error "Requested external UART0 clock but no frequency given"
    #endif
    #define MSP430X_USART0_UCSSEL UCSSEL__UCLK
  #elif MSP430X_USART0_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_USART0_CLK_SRC MSP430X_ACLK_SRC
    #define MSP430X_USART0_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_USART0_UCSSEL UCSSEL__ACLK
  #elif MSP430X_USART0_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_USART0_CLK_SRC MSP430X_SMCLK_SRC
    #define MSP430X_USART0_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_USART0_UCSSEL UCSSEL__SMCLK
  #else
    #error "MSP430X_USART0_CLK_SRC invalid"
  #endif
#endif

#if MSP430X_SERIAL_USE_USART1
  #if !defined(MSP430X_USART1_PARITY)
    #define MSP430X_USART1_PARITY NONE
  #endif
  #if !defined(MSP430X_USART1_ORDER)
    #define MSP430X_USART1_ORDER LSB
  #endif
  #if !defined(MSP430X_USART1_SIZE)
    #define MSP430X_USART1_SIZE EIGHT
  #endif
  #if !defined(MSP430X_USART1_STOP)
    #define MSP430X_USART1_STOP ONE
  #endif
  #if !defined(MSP430X_USART1_CLK_SRC)
    #define MSP430X_USART1_CLK_SRC MSP430X_UCLK_SRC
    #ifndef MSP430X_USART1_CLK_FREQ
      #error "Requested external UART0 clock but no frequency given"
    #endif
    #define MSP430X_USART1_UCSSEL UCSSEL__UCLK
  #elif MSP430X_USART1_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_USART1_CLK_SRC MSP430X_ACLK_SRC
    #define MSP430X_USART1_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_USART1_UCSSEL UCSSEL__ACLK
  #elif MSP430X_USART1_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_USART1_CLK_SRC MSP430X_SMCLK_SRC
    #define MSP430X_USART1_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_USART1_UCSSEL UCSSEL__SMCLK
  #else
    #error "MSP430X_USART1_CLK_SRC invalid"
  #endif
#endif

#if MSP430X_SERIAL_USE_USART2
  #if !defined(MSP430X_USART2_PARITY)
    #define MSP430X_USART2_PARITY NONE
  #endif
  #if !defined(MSP430X_USART2_ORDER)
    #define MSP430X_USART2_ORDER LSB
  #endif
  #if !defined(MSP430X_USART2_SIZE)
    #define MSP430X_USART2_SIZE EIGHT
  #endif
  #if !defined(MSP430X_USART2_STOP)
    #define MSP430X_USART2_STOP ONE
  #endif
  #if !defined(MSP430X_USART2_CLK_SRC)
    #define MSP430X_USART2_CLK_SRC MSP430X_UCLK_SRC
    #ifndef MSP430X_USART2_CLK_FREQ
      #error "Requested external UART0 clock but no frequency given"
    #endif
    #define MSP430X_USART2_UCSSEL UCSSEL__UCLK
  #elif MSP430X_USART2_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_USART2_CLK_SRC MSP430X_ACLK_SRC
    #define MSP430X_USART2_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_USART2_UCSSEL UCSSEL__ACLK
  #elif MSP430X_USART2_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_USART2_CLK_SRC MSP430X_SMCLK_SRC
    #define MSP430X_USART2_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_USART2_UCSSEL UCSSEL__SMCLK
  #else
    #error "MSP430X_USART2_CLK_SRC invalid"
  #endif
#endif

#if MSP430X_SERIAL_USE_USART3
  #if !defined(MSP430X_USART3_PARITY)
    #define MSP430X_USART3_PARITY NONE
  #endif
  #if !defined(MSP430X_USART3_ORDER)
    #define MSP430X_USART3_ORDER LSB
  #endif
  #if !defined(MSP430X_USART3_SIZE)
    #define MSP430X_USART3_SIZE EIGHT
  #endif
  #if !defined(MSP430X_USART3_STOP)
    #define MSP430X_USART3_STOP ONE
  #endif
  #if !defined(MSP430X_USART3_CLK_SRC)
    #define MSP430X_USART3_CLK_SRC MSP430X_UCLK_SRC
    #ifndef MSP430X_USART3_CLK_FREQ
      #error "Requested external UART0 clock but no frequency given"
    #endif
    #define MSP430X_USART3_UCSSEL UCSSEL__UCLK
  #elif MSP430X_USART3_CLK_SRC == MSP430X_ACLK_SRC
    #define MSP430X_USART3_CLK_SRC MSP430X_ACLK_SRC
    #define MSP430X_USART3_CLK_FREQ MSP430X_ACLK_FREQ
    #define MSP430X_USART3_UCSSEL UCSSEL__ACLK
  #elif MSP430X_USART3_CLK_SRC == MSP430X_SMCLK_SRC
    #define MSP430X_USART3_CLK_SRC MSP430X_SMCLK_SRC
    #define MSP430X_USART3_CLK_FREQ MSP430X_SMCLK_FREQ
    #define MSP430X_USART3_UCSSEL UCSSEL__SMCLK
  #else
    #error "MSP430X_USART3_CLK_SRC invalid"
  #endif
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   MSP430X Serial Driver configuration structure.
 * @details An insance of this structure must be passed to @p sdStart()
 *          in order to configure and start a serial driver operations.
 * @note    This structure content is architecture dependent, each driver
 *          implementation defines its own version and the custom static
 *          initializers.
 */
typedef struct {
  /**
   * @brief Bit rate.
   */
  uint32_t                  sc_bitrate;
    
  /* End of the mandatory fields.*/
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
  /* End of the mandatory fields.*/

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (MSP430X_SERIAL_USE_USART0 == TRUE) && !defined(__DOXYGEN__)
extern SerialDriver SD0;
#endif

#if (MSP430X_SERIAL_USE_USART1 == TRUE) && !defined(__DOXYGEN__)
extern SerialDriver SD1;
#endif

#if (MSP430X_SERIAL_USE_USART2 == TRUE) && !defined(__DOXYGEN__)
extern SerialDriver SD2;
#endif

#if (MSP430X_SERIAL_USE_USART3 == TRUE) && !defined(__DOXYGEN__)
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

#endif /* HAL_USE_SERIAL == TRUE */

#endif /* _SERIAL_LLD_H_ */

/** @} */
