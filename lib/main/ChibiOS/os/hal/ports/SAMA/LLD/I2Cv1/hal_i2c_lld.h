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
 * @file    I2Cv1/hal_i2c_lld.h
 * @brief   SAMA I2C subsystem low level driver header.
 *
 * @addtogroup I2C
 * @{
 */

#ifndef HAL_I2C_LLD_H
#define HAL_I2C_LLD_H

#if HAL_USE_I2C || defined(__DOXYGEN__)

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
 * @brief   I2C0 driver enable switch.
 * @details If set to @p TRUE the support for TWIHS0 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SAMA_I2C_USE_TWIHS0) || defined(__DOXYGEN__)
#define SAMA_I2C_USE_TWIHS0                 FALSE
#endif

/**
 * @brief   I2C1 driver enable switch.
 * @details If set to @p TRUE the support for TWIHS1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(SAMA_I2C_USE_TWIHS1) || defined(__DOXYGEN__)
#define SAMA_I2C_USE_TWIHS1                 FALSE
#endif

/**
 * @brief   I2C timeout on busy condition in milliseconds.
 */
#if !defined(SAMA_I2C_BUSY_TIMEOUT) || defined(__DOXYGEN__)
#define SAMA_I2C_BUSY_TIMEOUT               50
#endif

/**
 * @brief   I2C0 interrupt priority level setting.
 */
#if !defined(SAMA_I2C_TWIHS0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_I2C_TWIHS0_IRQ_PRIORITY        6
#endif

/**
 * @brief   I2C1 interrupt priority level setting.
 */
#if !defined(SAMA_I2C_TWIHS1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_I2C_TWIHS1_IRQ_PRIORITY        6
#endif

/**
* @brief   I2C0 DMA IRQ priority (0..7|lowest..highest).
* @note    The priority level is used for both the TX and RX DMA channels.
*/
#if !defined(SAMA_I2C_TWIHS0_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_I2C_TWIHS0_DMA_IRQ_PRIORITY    6
#endif

/**
* @brief   I2C1 DMA IRQ priority (0..7|lowest..highest).
* @note    The priority level is used for both the TX and RX DMA streams.
*/
#if !defined(SAMA_I2C_TWIHS1_DMA_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define SAMA_I2C_TWIHS1_DMA_IRQ_PRIORITY    6
#endif

/**
 * @brief   I2C DMA error hook.
 * @note    The default action for DMA errors is a system halt because DMA
 *          error can only happen because programming errors.
 */
#if !defined(SAMA_I2C_DMA_ERROR_HOOK) || defined(__DOXYGEN__)
#define SAMA_I2C_DMA_ERROR_HOOK(i2cp)      osalSysHalt("DMA failure")
#endif

/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/** @brief  error checks */

#if !SAMA_I2C_USE_TWIHS0 && !SAMA_I2C_USE_TWIHS1
#error "I2C driver activated but no TWIHS peripheral assigned"
#endif

#if !defined(SAMA_DMA_REQUIRED)
#define SAMA_DMA_REQUIRED
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type representing an I2C address.
 */
typedef uint32_t i2caddr_t;

/**
 * @brief   Type of I2C driver condition flags.
 */
typedef uint32_t i2cflags_t;

/**
 * @brief   Supported modes for the I2C bus.
 */
typedef enum {
  OPMODE_I2C = 1,
  OPMODE_SMBUS = 2,
} i2copmode_t;

/**
 * @brief   Type of I2C driver configuration structure.
 */
typedef struct {
  /* End of the mandatory fields.*/
  i2copmode_t     op_mode;       /**< @brief Specifies the I2C mode.        */
  uint32_t        clock_speed;   /**< @brief Specifies the clock frequency.
                                      @note Must be set to a value lower
                                      than 400kHz.                          */
} I2CConfig;

/**
 * @brief   Type of a structure representing an I2C driver.
 */
typedef struct I2CDriver I2CDriver;

/**
 * @brief   Structure representing an I2C driver.
 */
struct I2CDriver {
  /**
   * @brief   Driver state.
   */
  i2cstate_t                state;
  /**
   * @brief   Current configuration data.
   */
  const I2CConfig           *config;
  /**
   * @brief   Error flags.
   */
  i2cflags_t                errors;
#if I2C_USE_MUTUAL_EXCLUSION || defined(__DOXYGEN__)
  /**
   * @brief   Mutex protecting the bus.
   */
  mutex_t                   mutex;
#endif /* I2C_USE_MUTUAL_EXCLUSION */
#if defined(I2C_DRIVER_EXT_FIELDS)
  I2C_DRIVER_EXT_FIELDS
#endif
  /* End of the mandatory fields.*/
  /**
   * @brief   Thread waiting for I/O completion.
   */
  thread_reference_t        thread;
  /**
   * @brief     Number of bytes in TX phase.
   */
  size_t                    txbytes;
  /**
   * @brief     Number of bytes in RX phase.
   */
  size_t                    rxbytes;
  /**
   * @brief     Pointer to the TX buffer location.
   */
  const uint8_t             *txbuf;
  /**
   * @brief     Pointer to the RX buffer location.
   */
  uint8_t                   *rxbuf;
  /**
   * @brief   Receive DMA stream.
   */
  sama_dma_channel_t       *dmarx;
  /**
   * @brief   Transmit DMA stream.
   */
  sama_dma_channel_t       *dmatx;
  /**
    * @brief   RX DMA mode bit mask.
    */
  uint32_t                 rxdmamode;
  /**
    * @brief   TX DMA mode bit mask.
    */
  uint32_t                 txdmamode;
  /**
   * @brief     Pointer to the TWIHSx registers block.
   */
  Twi                      *i2c;
};

/*===========================================================================*/
/* Driver macros.                                                            */
/*===========================================================================*/

/**
 * @brief   Get errors from I2C driver.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
#define i2c_lld_get_errors(i2cp) ((i2cp)->errors)

/*===========================================================================*/
/* External declarations.                                                    */
/*===========================================================================*/

#if !defined(__DOXYGEN__)
#if SAMA_I2C_USE_TWIHS0
extern I2CDriver I2CD0;
#endif

#if SAMA_I2C_USE_TWIHS1
extern I2CDriver I2CD1;
#endif

#endif /* !defined(__DOXYGEN__) */

#ifdef __cplusplus
extern "C" {
#endif
  void i2c_lld_init(void);
  void i2c_lld_start(I2CDriver *i2cp);
  void i2c_lld_stop(I2CDriver *i2cp);
  msg_t i2c_lld_master_transmit_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                        const uint8_t *txbuf, size_t txbytes,
                                        uint8_t *rxbuf, size_t rxbytes,
                                        sysinterval_t timeout);
  msg_t i2c_lld_master_receive_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                       uint8_t *rxbuf, size_t rxbytes,
                                       sysinterval_t timeout);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_I2C  */

#endif /* HAL_I2C_LLD_H */

/** @} */
