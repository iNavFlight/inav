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
 * @file    serial.h
 * @brief   Serial Driver macros and structures.
 *
 * @addtogroup SERIAL
 * @{
 */

#ifndef _SERIAL_H_
#define _SERIAL_H_

#if (HAL_USE_SERIAL == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    Serial status flags
 * @{
 */
#define SD_PARITY_ERROR         (eventflags_t)32    /**< @brief Parity.     */
#define SD_FRAMING_ERROR        (eventflags_t)64    /**< @brief Framing.    */
#define SD_OVERRUN_ERROR        (eventflags_t)128   /**< @brief Overflow.   */
#define SD_NOISE_ERROR          (eventflags_t)256   /**< @brief Line noise. */
#define SD_BREAK_DETECTED       (eventflags_t)512   /**< @brief LIN Break.  */
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    Serial configuration options
 * @{
 */
/**
 * @brief   Default bit rate.
 * @details Configuration parameter, this is the baud rate selected for the
 *          default configuration.
 */
#if !defined(SERIAL_DEFAULT_BITRATE) || defined(__DOXYGEN__)
#define SERIAL_DEFAULT_BITRATE      38400
#endif

/**
 * @brief   Serial buffers size.
 * @details Configuration parameter, you can change the depth of the queue
 *          buffers depending on the requirements of your application.
 * @note    The default is 16 bytes for both the transmission and receive
 *          buffers.
 */
#if !defined(SERIAL_BUFFERS_SIZE) || defined(__DOXYGEN__)
#define SERIAL_BUFFERS_SIZE         16
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief Driver state machine possible states.
 */
typedef enum {
  SD_UNINIT = 0,                    /**< Not initialized.                   */
  SD_STOP = 1,                      /**< Stopped.                           */
  SD_READY = 2                      /**< Ready.                             */
} sdstate_t;

/**
 * @brief   Structure representing a serial driver.
 */
typedef struct SerialDriver SerialDriver;

#include "serial_lld.h"

/**
 * @brief   @p SerialDriver specific methods.
 */
#define _serial_driver_methods                                              \
  _base_asynchronous_channel_methods

/**
 * @extends BaseAsynchronousChannelVMT
 *
 * @brief   @p SerialDriver virtual methods table.
 */
struct SerialDriverVMT {
  _serial_driver_methods
};

/**
 * @extends BaseAsynchronousChannel
 *
 * @brief   Full duplex serial driver class.
 * @details This class extends @p BaseAsynchronousChannel by adding physical
 *          I/O queues.
 */
struct SerialDriver {
  /** @brief Virtual Methods Table.*/
  const struct SerialDriverVMT *vmt;
  _serial_driver_data
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Direct write to a @p SerialDriver.
 * @note    This function bypasses the indirect access to the channel and
 *          writes directly on the output queue. This is faster but cannot
 *          be used to write to different channels implementations.
 *
 * @see     chnPutTimeout()
 *
 * @api
 */
#define sdPut(sdp, b) oqPut(&(sdp)->oqueue, b)

/**
 * @brief   Direct write to a @p SerialDriver with timeout specification.
 * @note    This function bypasses the indirect access to the channel and
 *          writes directly on the output queue. This is faster but cannot
 *          be used to write to different channels implementations.
 *
 * @see     chnPutTimeout()
 *
 * @api
 */
#define sdPutTimeout(sdp, b, t) oqPutTimeout(&(sdp)->oqueue, b, t)

/**
 * @brief   Direct read from a @p SerialDriver.
 * @note    This function bypasses the indirect access to the channel and
 *          reads directly from the input queue. This is faster but cannot
 *          be used to read from different channels implementations.
 *
 * @see     chnGetTimeout()
 *
 * @api
 */
#define sdGet(sdp) iqGet(&(sdp)->iqueue)

/**
 * @brief   Direct read from a @p SerialDriver with timeout specification.
 * @note    This function bypasses the indirect access to the channel and
 *          reads directly from the input queue. This is faster but cannot
 *          be used to read from different channels implementations.
 *
 * @see     chnGetTimeout()
 *
 * @api
 */
#define sdGetTimeout(sdp, t) iqGetTimeout(&(sdp)->iqueue, t)

/**
 * @brief   Direct blocking write to a @p SerialDriver.
 * @note    This function bypasses the indirect access to the channel and
 *          writes directly to the output queue. This is faster but cannot
 *          be used to write from different channels implementations.
 *
 * @see     chnWrite()
 *
 * @api
 */
#define sdWrite(sdp, b, n)                                                  \
  oqWriteTimeout(&(sdp)->oqueue, b, n, TIME_INFINITE)

/**
 * @brief   Direct blocking write to a @p SerialDriver with timeout
 *          specification.
 * @note    This function bypasses the indirect access to the channel and
 *          writes directly to the output queue. This is faster but cannot
 *          be used to write to different channels implementations.
 *
 * @see     chnWriteTimeout()
 *
 * @api
 */
#define sdWriteTimeout(sdp, b, n, t)                                        \
  oqWriteTimeout(&(sdp)->oqueue, b, n, t)

/**
 * @brief   Direct non-blocking write to a @p SerialDriver.
 * @note    This function bypasses the indirect access to the channel and
 *          writes directly to the output queue. This is faster but cannot
 *          be used to write to different channels implementations.
 *
 * @see     chnWriteTimeout()
 *
 * @api
 */
#define sdAsynchronousWrite(sdp, b, n)                                      \
  oqWriteTimeout(&(sdp)->oqueue, b, n, TIME_IMMEDIATE)

/**
 * @brief   Direct blocking read from a @p SerialDriver.
 * @note    This function bypasses the indirect access to the channel and
 *          reads directly from the input queue. This is faster but cannot
 *          be used to read from different channels implementations.
 *
 * @see     chnRead()
 *
 * @api
 */
#define sdRead(sdp, b, n)                                                   \
  iqReadTimeout(&(sdp)->iqueue, b, n, TIME_INFINITE)

/**
 * @brief   Direct blocking read from a @p SerialDriver with timeout
 *          specification.
 * @note    This function bypasses the indirect access to the channel and
 *          reads directly from the input queue. This is faster but cannot
 *          be used to read from different channels implementations.
 *
 * @see     chnReadTimeout()
 *
 * @api
 */
#define sdReadTimeout(sdp, b, n, t)                                         \
  iqReadTimeout(&(sdp)->iqueue, b, n, t)

/**
 * @brief   Direct non-blocking read from a @p SerialDriver.
 * @note    This function bypasses the indirect access to the channel and
 *          reads directly from the input queue. This is faster but cannot
 *          be used to read from different channels implementations.
 *
 * @see     chnReadTimeout()
 *
 * @api
 */
#define sdAsynchronousRead(sdp, b, n)                                       \
  iqReadTimeout(&(sdp)->iqueue, b, n, TIME_IMMEDIATE)
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void sdInit(void);
  void sdObjectInit(SerialDriver *sdp, qnotify_t inotify, qnotify_t onotify);
  void sdStart(SerialDriver *sdp, const SerialConfig *config);
  void sdStop(SerialDriver *sdp);
  void sdIncomingDataI(SerialDriver *sdp, uint8_t b);
  msg_t sdRequestDataI(SerialDriver *sdp);
  bool sdPutWouldBlock(SerialDriver *sdp);
  bool sdGetWouldBlock(SerialDriver *sdp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SERIAL == TRUE */

#endif /* _SERIAL_H_ */

/** @} */
