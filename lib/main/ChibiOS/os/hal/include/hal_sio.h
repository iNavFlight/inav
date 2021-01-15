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
 * @file    hal_sio.h
 * @brief   SIO Driver macros and structures.
 *
 * @addtogroup SIO
 * @{
 */

#ifndef HAL_SIO_H
#define HAL_SIO_H

#if (HAL_USE_SIO == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/**
 * @name    SIO status flags
 * @{
 */
#define SIO_NO_ERROR           0    /**< @brief No pending conditions.      */
#define SIO_PARITY_ERROR       4    /**< @brief Parity error happened.      */
#define SIO_FRAMING_ERROR      8    /**< @brief Framing error happened.     */
#define SIO_OVERRUN_ERROR      16   /**< @brief Overflow happened.          */
#define SIO_NOISE_ERROR        32   /**< @brief Noise on the line.          */
#define SIO_BREAK_DETECTED     64   /**< @brief Break detected.             */
/** @} */

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    SIO configuration options
 * @{
 */

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of structure representing a SIO driver.
 */
typedef struct hal_sio_driver SIODriver;

/**
 * @brief   Type of structure representing a SIO configuration.
 */
typedef struct hal_sio_config SIOConfig;

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  SIO_UNINIT = 0,                   /**< Not initialized.                   */
  SIO_STOP = 1,                     /**< Stopped.                           */
  SIO_READY = 2                     /**< Ready.                             */
} siostate_t;

#include "hal_sio_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Returns the current set of flags and clears it.
 */
#define sioGetFlagsX(siop) sio_lld_get_flags(siop)

/**
 * @brief   Determines the state of the RX FIFO.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The RX FIFO state.
 * @retval false        if RX FIFO is not empty
 * @retval true         if RX FIFO is empty
 *
 * @xclass
 */
#define sioRXIsEmptyX(siop) sio_lld_rx_is_empty(siop)

/**
 * @brief   Determines the state of the TX FIFO.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The TX FIFO state.
 * @retval false        if TX FIFO is not full
 * @retval true         if TX FIFO is full
 *
 * @xclass
 */
#define sioTXIsFullX(siop) sio_lld_tx_is_full(siop)

/**
 * @brief   Returns one frame from the RX FIFO.
 * @note    If the FIFO is empty then the returned value is unpredictable.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The frame from RX FIFO.
 *
 * @xclass
 */
#define sioRXGetX(siop) sio_lld_rx_get(siop)

/**
 * @brief   Pushes one frame into the TX FIFO.
 * @note    If the FIFO is full then the behavior is unpredictable.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @param[in] data      frame to be written
 *
 * @xclass
 */
#define sioTXPutX(siop, data) sio_lld_tx_put(siop, data)

/**
 * @brief   Reads data from the RX FIFO.
 * @details This function is non-blocking, data is read if present and the
 *          effective amount is returned.
 * @note    This function can be called from any context but it is meant to
 *          be called from the @p rxne_cb callback handler.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @param[in] buffer    buffer for the received data
 * @param[in] size      maximum number of frames to read
 * @return              The number of received frames.
 *
 * @xclass
 */
#define sioReadX(siop, buffer, size) sio_lld_read(siop, buffer, size)

/**
 * @brief   Writes data into the TX FIFO.
 * @details This function is non-blocking, data is written if there is space
 *          in the FIFO and the effective amount is returned.
 * @note    This function can be called from any context but it is meant to
 *          be called from the @p txnf_cb callback handler.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @param[out] buffer   buffer containing the data to be transmitted
 * @param[in] size      maximum number of frames to read
 * @return              The number of transmitted frames.
 *
 * @xclass
 */
#define sioWriteX(siop, buffer, size) sio_lld_write(siop, buffer, size)

/**
 * @brief   Control operation on a serial port.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @param[in] operation control operation code
 * @param[in,out] arg   operation argument
 *
 * @return              The control operation status.
 * @retval MSG_OK       in case of success.
 * @retval MSG_TIMEOUT  in case of operation timeout.
 * @retval MSG_RESET    in case of operation reset.
 *
 * @xclass
 */
#define sioControlX(siop, operation, arg) sio_lld_control(siop, operation, arg)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void sioInit(void);
  void sioObjectInit(SIODriver *siop);
  void sioStart(SIODriver *siop, const SIOConfig *config);
  void sioStop(SIODriver *siop);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SIO == TRUE */

#endif /* HAL_SIO_H */

/** @} */
