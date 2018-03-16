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
 * @file    spi.h
 * @brief   SPI Driver macros and structures.
 *
 * @addtogroup SPI
 * @{
 */

#ifndef _SPI_H_
#define _SPI_H_

#if (HAL_USE_SPI == TRUE) || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver pre-compile time settings.                                         */
/*===========================================================================*/

/**
 * @name    SPI configuration options
 * @{
 */
/**
 * @brief   Enables synchronous APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(SPI_USE_WAIT) || defined(__DOXYGEN__)
#define SPI_USE_WAIT                TRUE
#endif

/**
 * @brief   Enables the @p spiAcquireBus() and @p spiReleaseBus() APIs.
 * @note    Disabling this option saves both code and data space.
 */
#if !defined(SPI_USE_MUTUAL_EXCLUSION) || defined(__DOXYGEN__)
#define SPI_USE_MUTUAL_EXCLUSION    TRUE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Driver state machine possible states.
 */
typedef enum {
  SPI_UNINIT = 0,                   /**< Not initialized.                   */
  SPI_STOP = 1,                     /**< Stopped.                           */
  SPI_READY = 2,                    /**< Ready.                             */
  SPI_ACTIVE = 3,                   /**< Exchanging data.                   */
  SPI_COMPLETE = 4                  /**< Asynchronous operation complete.   */
} spistate_t;

#include "spi_lld.h"

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @name    Macro Functions
 * @{
 */
/**
 * @brief   Asserts the slave select signal and prepares for transfers.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @iclass
 */
#define spiSelectI(spip) {                                                  \
  spi_lld_select(spip);                                                     \
}

/**
 * @brief   Deasserts the slave select signal.
 * @details The previously selected peripheral is unselected.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @iclass
 */
#define spiUnselectI(spip) {                                                \
  spi_lld_unselect(spip);                                                   \
}

/**
 * @brief   Ignores data on the SPI bus.
 * @details This asynchronous function starts the transmission of a series of
 *          idle words on the SPI bus and ignores the received data.
 * @pre     A slave must have been selected using @p spiSelect() or
 *          @p spiSelectI().
 * @post    At the end of the operation the configured callback is invoked.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to be ignored
 *
 * @iclass
 */
#define spiStartIgnoreI(spip, n) {                                          \
  (spip)->state = SPI_ACTIVE;                                               \
  spi_lld_ignore(spip, n);                                                  \
}

/**
 * @brief   Exchanges data on the SPI bus.
 * @details This asynchronous function starts a simultaneous transmit/receive
 *          operation.
 * @pre     A slave must have been selected using @p spiSelect() or
 *          @p spiSelectI().
 * @post    At the end of the operation the configured callback is invoked.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to be exchanged
 * @param[in] txbuf     the pointer to the transmit buffer
 * @param[out] rxbuf    the pointer to the receive buffer
 *
 * @iclass
 */
#define spiStartExchangeI(spip, n, txbuf, rxbuf) {                          \
  (spip)->state = SPI_ACTIVE;                                               \
  spi_lld_exchange(spip, n, txbuf, rxbuf);                                  \
}

/**
 * @brief   Sends data over the SPI bus.
 * @details This asynchronous function starts a transmit operation.
 * @pre     A slave must have been selected using @p spiSelect() or
 *          @p spiSelectI().
 * @post    At the end of the operation the configured callback is invoked.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to send
 * @param[in] txbuf     the pointer to the transmit buffer
 *
 * @iclass
 */
#define spiStartSendI(spip, n, txbuf) {                                     \
  (spip)->state = SPI_ACTIVE;                                               \
  spi_lld_send(spip, n, txbuf);                                             \
}

/**
 * @brief   Receives data from the SPI bus.
 * @details This asynchronous function starts a receive operation.
 * @pre     A slave must have been selected using @p spiSelect() or
 *          @p spiSelectI().
 * @post    At the end of the operation the configured callback is invoked.
 * @note    The buffers are organized as uint8_t arrays for data sizes below
 *          or equal to 8 bits else it is organized as uint16_t arrays.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] n         number of words to receive
 * @param[out] rxbuf    the pointer to the receive buffer
 *
 * @iclass
 */
#define spiStartReceiveI(spip, n, rxbuf) {                                  \
  (spip)->state = SPI_ACTIVE;                                               \
  spi_lld_receive(spip, n, rxbuf);                                          \
}

/**
 * @brief   Exchanges one frame using a polled wait.
 * @details This synchronous function exchanges one frame using a polled
 *          synchronization method. This function is useful when exchanging
 *          small amount of data on high speed channels, usually in this
 *          situation is much more efficient just wait for completion using
 *          polling than suspending the thread waiting for an interrupt.
 * @note    This API is implemented as a macro in order to minimize latency.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 * @param[in] frame     the data frame to send over the SPI bus
 * @return              The received data frame from the SPI bus.
 */
#define spiPolledExchange(spip, frame) spi_lld_polled_exchange(spip, frame)
/** @} */

/**
 * @name    Low level driver helper macros
 * @{
 */
#if (SPI_USE_WAIT == TRUE) || defined(__DOXYGEN__)
/**
 * @brief   Wakes up the waiting thread.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
#define _spi_wakeup_isr(spip) {                                             \
  osalSysLockFromISR();                                                     \
  osalThreadResumeI(&(spip)->thread, MSG_OK);                               \
  osalSysUnlockFromISR();                                                   \
}
#else /* !SPI_USE_WAIT */
#define _spi_wakeup_isr(spip)
#endif /* !SPI_USE_WAIT */

/**
 * @brief   Common ISR code.
 * @details This code handles the portable part of the ISR code:
 *          - Callback invocation.
 *          - Waiting thread wakeup, if any.
 *          - Driver state transitions.
 *          .
 * @note    This macro is meant to be used in the low level drivers
 *          implementation only.
 *
 * @param[in] spip      pointer to the @p SPIDriver object
 *
 * @notapi
 */
#define _spi_isr_code(spip) {                                               \
  if ((spip)->config->end_cb) {                                             \
    (spip)->state = SPI_COMPLETE;                                           \
    (spip)->config->end_cb(spip);                                           \
    if ((spip)->state == SPI_COMPLETE)                                      \
      (spip)->state = SPI_READY;                                            \
  }                                                                         \
  else                                                                      \
    (spip)->state = SPI_READY;                                              \
  _spi_wakeup_isr(spip);                                                    \
}
/** @} */

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#ifdef __cplusplus
extern "C" {
#endif
  void spiInit(void);
  void spiObjectInit(SPIDriver *spip);
  void spiStart(SPIDriver *spip, const SPIConfig *config);
  void spiStop(SPIDriver *spip);
  void spiSelect(SPIDriver *spip);
  void spiUnselect(SPIDriver *spip);
  void spiStartIgnore(SPIDriver *spip, size_t n);
  void spiStartExchange(SPIDriver *spip, size_t n,
                        const void *txbuf, void *rxbuf);
  void spiStartSend(SPIDriver *spip, size_t n, const void *txbuf);
  void spiStartReceive(SPIDriver *spip, size_t n, void *rxbuf);
#if SPI_USE_WAIT == TRUE
  void spiIgnore(SPIDriver *spip, size_t n);
  void spiExchange(SPIDriver *spip, size_t n, const void *txbuf, void *rxbuf);
  void spiSend(SPIDriver *spip, size_t n, const void *txbuf);
  void spiReceive(SPIDriver *spip, size_t n, void *rxbuf);
#endif
#if SPI_USE_MUTUAL_EXCLUSION == TRUE
  void spiAcquireBus(SPIDriver *spip);
  void spiReleaseBus(SPIDriver *spip);
#endif
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SPI == TRUE */

#endif /* _SPI_H_ */

/** @} */
