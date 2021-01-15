/*
    Copyright (C) 2018	andru

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
 * @file    NRF5/NRF52832/hal_i2c_lld.h
 * @brief   NRF52 I2C subsystem low level driver header.
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
 * @details If set to @p TRUE the support for I2C0 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(NRF5_I2C_USE_I2C0) || defined(__DOXYGEN__)
#define NRF5_I2C_USE_I2C0               FALSE
#endif

/**
 * @brief   I2C1 driver enable switch.
 * @details If set to @p TRUE the support for I2C1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(NRF5_I2C_USE_I2C1) || defined(__DOXYGEN__)
#define NRF5_I2C_USE_I2C1               FALSE
#endif

/**
 * @brief   I2C0 interrupt priority level setting.
 */
#if !defined(NRF5_I2C_I2C0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_I2C_I2C0_IRQ_PRIORITY         3
#endif

/**
 * @brief   I2C1 interrupt priority level setting.
 */
#if !defined(NRF5_I2C_I2C1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define NRF5_I2C_I2C1_IRQ_PRIORITY         3
#endif
/** @} */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

#if NRF5_I2C_USE_I2C0 &&						    \
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_I2C_I2C0_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2C0"
#endif

#if NRF5_I2C_USE_I2C1 &&						    \
    !OSAL_IRQ_IS_VALID_PRIORITY(NRF5_I2C_I2C1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2C1"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/* @brief Type representing I2C address. */
typedef uint8_t i2caddr_t;

/* @brief Type of I2C Driver condition flags. */
typedef uint32_t i2cflags_t;

/**
 * @brief   Driver configuration structure.
 * @note    Implementations may extend this structure to contain more,
 *          architecture dependent, fields.
 */

/**
 * @brief Driver configuration structure.
 */
typedef struct {

  /* @brief Clock to be used for the I2C bus. */
  uint32_t             clock;
  /* @brief Pad number for SCL */
  uint8_t              scl_pad;
  /* @brief Pad number for SDA */
  uint8_t              sda_pad;

} I2CConfig;

/**
 * @brief   Type of a structure representing an I2C driver.
 */
typedef struct I2CDriver I2CDriver;

/**
 * @brief Structure representing an I2C driver.
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
  /* @brief Thread waiting for I/O completion. */
  thread_reference_t        thread;
  /* @brief     Current slave address without R/W bit. */
  i2caddr_t                 addr;

  /* End of the mandatory fields.*/

  /* @brief Low-level register access. */
  NRF_TWIM_Type             *i2c;
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

#if NRF5_I2C_USE_I2C0
extern I2CDriver I2CD1;
#endif

#if NRF5_I2C_USE_I2C1
extern I2CDriver I2CD2;
#endif

#endif

#ifdef __cplusplus
extern "C" {
#endif
  void i2c_lld_init(void);
  void i2c_lld_start(I2CDriver *i2cp);
  void i2c_lld_stop(I2CDriver *i2cp);
  msg_t i2c_lld_master_transmit_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                        const uint8_t *txbuf, size_t txbytes,
                                        uint8_t *rxbuf, size_t rxbytes,
                                        systime_t timeout);
  msg_t i2c_lld_master_receive_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                       uint8_t *rxbuf, size_t rxbytes,
                                       systime_t timeout);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_I2C */

#endif /* HAL_I2C_LLD_H */

/** @} */
