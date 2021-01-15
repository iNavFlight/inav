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
 * @file    hal_sio_lld.h
 * @brief   PLATFORM SIO subsystem low level driver header.
 *
 * @addtogroup SIO
 * @{
 */

#ifndef HAL_SIO_LLD_H
#define HAL_SIO_LLD_H

#if (HAL_USE_SIO == TRUE) || defined(__DOXYGEN__)

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
 * @brief   SIO driver enable switch.
 * @details If set to @p TRUE the support for SIO1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(PLATFORM_SIO_USE_SIO1) || defined(__DOXYGEN__)
#define PLATFORM_SIO_USE_SIO1             FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   SIO driver condition flags type.
 */
typedef uint32_t sioflags_t;

/**
 * @brief   Generic SIO notification callback type.
 *
 * @param[in] siop     pointer to the @p SIODriver object
 */
typedef void (*siocb_t)(SIODriver *siop);

/**
 * @brief   Receive error SIO notification callback type.
 *
 * @param[in] siop     pointer to the @p SIODriver object triggering the
 *                      callback
 * @param[in] e         receive error mask
 */
typedef void (*sioecb_t)(SIODriver *siop, sioflags_t e);

/**
 * @brief   Driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
struct hal_sio_config {
  /**
   * @brief   Receive buffer filled callback.
   * @note    Can be @p NULL.
   */
  siocb_t                   rxne_cb;
  /**
   * @brief   End of transmission buffer callback.
   * @note    Can be @p NULL.
   */
  siocb_t                   txnf_cb;
  /**
   * @brief   Physical end of transmission callback.
   * @note    Can be @p NULL.
   */
  siocb_t                   txend_cb;
  /**
   * @brief   Receive event callback.
   * @note    Can be @p NULL.
   */
  sioecb_t                  rxevt_cb;
  /* End of the mandatory fields.*/
};

/**
 * @brief   Structure representing a SIO driver.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */
struct hal_sio_driver {
  /**
   * @brief Driver state.
   */
  siostate_t               state;
  /**
   * @brief Current configuration data.
   */
  const SIOConfig          *config;
#if defined(SIO_DRIVER_EXT_FIELDS)
  SIO_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Determines the state of the RX FIFO.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The RX FIFO state.
 * @retval false        if RX FIFO is not empty
 * @retval true         if RX FIFO is empty
 *
 * @notapi
 */
#define sio_lld_rx_is_empty(siop) true

/**
 * @brief   Determines the state of the TX FIFO.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The TX FIFO state.
 * @retval false        if TX FIFO is not full
 * @retval true         if TX FIFO is full
 *
 * @notapi
 */
#define sio_lld_tx_is_full(siop) true

/**
 * @brief   Returns one frame from the RX FIFO.
 * @note    If the FIFO is empty then the returned value is unpredictable.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @return              The frame from RX FIFO.
 *
 * @notapi
 */
#define sio_lld_rx_get(siop)

/**
 * @brief   Pushes one frame into the TX FIFO.
 * @note    If the FIFO is full then the behavior is unpredictable.
 *
 * @param[in] siop      pointer to the @p SIODriver object
 * @param[in] data      frame to be written
 *
 * @notapi
 */
#define sio_lld_tx_put(siop, data)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (PLATFORM_SIO_USE_SIO1 == TRUE) && !defined(__DOXYGEN__)
extern SIODriver SIOD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void sio_lld_init(void);
  void sio_lld_start(SIODriver *siop);
  void sio_lld_stop(SIODriver *siop);
  size_t sio_lld_read(SIODriver *siop, void *buffer, size_t size);
  size_t sio_lld_write(SIODriver *siop, const void *buffer, size_t size);
  msg_t sio_lld_control(SIODriver *siop, unsigned int operation, void *arg);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_SIO == TRUE */

#endif /* HAL_SIO_LLD_H */

/** @} */
