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
 * @file    i2s_lld.h
 * @brief   PLATFORM I2S subsystem low level driver header.
 *
 * @addtogroup I2S
 * @{
 */

#ifndef _I2S_LLD_H_
#define _I2S_LLD_H_

#if (HAL_USE_I2S == TRUE) || defined(__DOXYGEN__)

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
 * @brief   I2SD1 driver enable switch.
 * @details If set to @p TRUE the support for I2S1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(PLATFORM_I2S_USE_I2S1) || defined(__DOXYGEN__)
#define PLATFORM_I2S_USE_I2S1                  FALSE
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type of a structure representing an I2S driver.
 */
typedef struct I2SDriver I2SDriver;

/**
 * @brief   I2S notification callback type.
 *
 * @param[in] i2sp      pointer to the @p I2SDriver object
 * @param[in] offset    offset in buffers of the data to read/write
 * @param[in] n         number of samples to read/write
 */
typedef void (*i2scallback_t)(I2SDriver *i2sp, size_t offset, size_t n);

/**
 * @brief   Driver configuration structure.
 * @note    It could be empty on some architectures.
 */
typedef struct {
  /**
   * @brief   Transmission buffer pointer.
   * @note    Can be @p NULL if TX is not required.
   */
  const void                *tx_buffer;
  /**
   * @brief   Receive buffer pointer.
   * @note    Can be @p NULL if RX is not required.
   */
  void                      *rx_buffer;
  /**
   * @brief   TX and RX buffers size as number of samples.
   */
  size_t                    size;
  /**
   * @brief   Callback function called during streaming.
   */
  i2scallback_t             end_cb;
  /* End of the mandatory fields.*/
} I2SConfig;

/**
 * @brief   Structure representing an I2S driver.
 */
struct I2SDriver {
  /**
   * @brief   Driver state.
   */
  i2sstate_t                state;
  /**
   * @brief   Current configuration data.
   */
  const I2SConfig           *config;
  /* End of the mandatory fields.*/
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if (PLATFORM_I2S_USE_I2S1 == TRUE) && !defined(__DOXYGEN__)
extern I2SDriver I2SD1;
#endif

#ifdef __cplusplus
extern "C" {
#endif
  void i2s_lld_init(void);
  void i2s_lld_start(I2SDriver *i2sp);
  void i2s_lld_stop(I2SDriver *i2sp);
  void i2s_lld_start_exchange(I2SDriver *i2sp);
  void i2s_lld_stop_exchange(I2SDriver *i2sp);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_I2S == TRUE */

#endif /* _I2S_LLD_H_ */

/** @} */
