/*
    Copyright (C) 2014..2017 Marco Veeneman

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
 * @file    I2C/hal_i2c_lld.h
 * @brief   TM4C123x/TM4C129x I2C subsystem low level driver header.
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
#if !defined(TIVA_I2C_USE_I2C0) || defined(__DOXYGEN__)
#define TIVA_I2C_USE_I2C0           FALSE
#endif

/**
 * @brief   I2C1 driver enable switch.
 * @details If set to @p TRUE the support for I2C1 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_I2C_USE_I2C1) || defined(__DOXYGEN__)
#define TIVA_I2C_USE_I2C1           FALSE
#endif

/**
 * @brief   I2C2 driver enable switch.
 * @details If set to @p TRUE the support for I2C2 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_I2C_USE_I2C2) || defined(__DOXYGEN__)
#define TIVA_I2C_USE_I2C2           FALSE
#endif

/**
 * @brief   I2C3 driver enable switch.
 * @details If set to @p TRUE the support for I2C3 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_I2C_USE_I2C3) || defined(__DOXYGEN__)
#define TIVA_I2C_USE_I2C3           FALSE
#endif

/**
 * @brief   I2C4 driver enable switch.
 * @details If set to @p TRUE the support for I2C4 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_I2C_USE_I2C4) || defined(__DOXYGEN__)
#define TIVA_I2C_USE_I2C4           FALSE
#endif

/**
 * @brief   I2C5 driver enable switch.
 * @details If set to @p TRUE the support for I2C5 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_I2C_USE_I2C5) || defined(__DOXYGEN__)
#define TIVA_I2C_USE_I2C5           FALSE
#endif

/**
 * @brief   I2C6 driver enable switch.
 * @details If set to @p TRUE the support for I2C6 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_I2C_USE_I2C6) || defined(__DOXYGEN__)
#define TIVA_I2C_USE_I2C6           FALSE
#endif

/**
 * @brief   I2C7 driver enable switch.
 * @details If set to @p TRUE the support for I2C7 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_I2C_USE_I2C7) || defined(__DOXYGEN__)
#define TIVA_I2C_USE_I2C7           FALSE
#endif

/**
 * @brief   I2C8 driver enable switch.
 * @details If set to @p TRUE the support for I2C8 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_I2C_USE_I2C8) || defined(__DOXYGEN__)
#define TIVA_I2C_USE_I2C8           FALSE
#endif

/**
 * @brief   I2C9 driver enable switch.
 * @details If set to @p TRUE the support for I2C9 is included.
 * @note    The default is @p FALSE.
 */
#if !defined(TIVA_I2C_USE_I2C9) || defined(__DOXYGEN__)
#define TIVA_I2C_USE_I2C9           FALSE
#endif

/**
 * @brief   I2C timeout on busy condition in milliseconds.
 */
#if !defined(TIVA_I2C_BUSY_TIMEOUT) || defined(__DOXYGEN__)
#define TIVA_I2C_BUSY_TIMEOUT       50
#endif

/**
 * @brief   I2C0 interrupt priority level setting.
 */
#if !defined(TIVA_I2C_I2C0_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_I2C_I2C0_IRQ_PRIORITY  4
#endif

/**
 * @brief   I2C1 interrupt priority level setting.
 */
#if !defined(TIVA_I2C_I2C1_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_I2C_I2C1_IRQ_PRIORITY  4
#endif

/**
 * @brief   I2C2 interrupt priority level setting.
 */
#if !defined(TIVA_I2C_I2C2_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_I2C_I2C2_IRQ_PRIORITY  4
#endif

/**
 * @brief   I2C3 interrupt priority level setting.
 */
#if !defined(TIVA_I2C_I2C3_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_I2C_I2C3_IRQ_PRIORITY  4
#endif

/**
 * @brief   I2C4 interrupt priority level setting.
 */
#if !defined(TIVA_I2C_I2C4_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_I2C_I2C4_IRQ_PRIORITY  4
#endif

/**
 * @brief   I2C5 interrupt priority level setting.
 */
#if !defined(TIVA_I2C_I2C5_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_I2C_I2C5_IRQ_PRIORITY  4
#endif

/**
 * @brief   I2C6 interrupt priority level setting.
 */
#if !defined(TIVA_I2C_I2C6_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_I2C_I2C6_IRQ_PRIORITY  4
#endif

/**
 * @brief   I2C7 interrupt priority level setting.
 */
#if !defined(TIVA_I2C_I2C7_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_I2C_I2C7_IRQ_PRIORITY  4
#endif

/**
 * @brief   I2C8 interrupt priority level setting.
 */
#if !defined(TIVA_I2C_I2C8_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_I2C_I2C8_IRQ_PRIORITY  4
#endif

/**
 * @brief   I2C9 interrupt priority level setting.
 */
#if !defined(TIVA_I2C_I2C9_IRQ_PRIORITY) || defined(__DOXYGEN__)
#define TIVA_I2C_I2C9_IRQ_PRIORITY  4
#endif

/**
 * @}
 */

/*===========================================================================*/
/* Derived constants and error checks.                                       */
/*===========================================================================*/

/**
 * @brief  error checks
 */
#if !TIVA_I2C_USE_I2C0 && !TIVA_I2C_USE_I2C1 && !TIVA_I2C_USE_I2C2 && \
    !TIVA_I2C_USE_I2C3 && !TIVA_I2C_USE_I2C4 && !TIVA_I2C_USE_I2C5 && \
    !TIVA_I2C_USE_I2C6 && !TIVA_I2C_USE_I2C7 && !TIVA_I2C_USE_I2C8 && \
    !TIVA_I2C_USE_I2C9
#error "I2C driver activated but no I2C peripheral assigned"
#endif

#if TIVA_I2C_USE_I2C0 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_I2C_I2C0_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2C0"
#endif

#if TIVA_I2C_USE_I2C1 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_I2C_I2C1_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2C1"
#endif

#if TIVA_I2C_USE_I2C2 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_I2C_I2C2_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2C2"
#endif

#if TIVA_I2C_USE_I2C3 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_I2C_I2C3_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2C3"
#endif

#if TIVA_I2C_USE_I2C4 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_I2C_I2C4_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2C4"
#endif

#if TIVA_I2C_USE_I2C5 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_I2C_I2C5_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2C5"
#endif

#if TIVA_I2C_USE_I2C6 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_I2C_I2C6_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2C6"
#endif

#if TIVA_I2C_USE_I2C7 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_I2C_I2C7_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2C7"
#endif

#if TIVA_I2C_USE_I2C8 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_I2C_I2C8_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2C8"
#endif

#if TIVA_I2C_USE_I2C9 && \
    !OSAL_IRQ_IS_VALID_PRIORITY(TIVA_I2C_I2C9_IRQ_PRIORITY)
#error "Invalid IRQ priority assigned to I2C9"
#endif

/*===========================================================================*/
/* Driver data structures and types.                                         */
/*===========================================================================*/

/**
 * @brief   Type representing I2C address.
 */
typedef uint16_t i2caddr_t;

/**
 * @brief   I2C Driver condition flags type.
 */
typedef uint32_t i2cflags_t;

/**
 * @brief Driver configuration structure.
 */
typedef struct
{
  /**
   * @brief Specifies the clock frequency.
   * @note  Must be set to a value lower than 3.33Mbps.
   * TODO: high-speed mode: 3333 kHz. setup is 100-400-1000 kHz then switched to 3333 kHz
   */
  uint32_t        clock_speed;
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
  /* End of the mandatory fields.*/
  /**
   * @brief     Thread waiting for I/O completion.
   */
  thread_reference_t        thread;
  /**
   * @brief     Current slave address without R/W bit.
   */
  i2caddr_t                 addr;
  /**
   * @brief     Pointer to the buffer with data to send.
   */
  const uint8_t             *txbuf;
  /**
   * @brief     Number of bytes of data to send.
   */
  size_t                    txbytes;
  /**
   * @brief     Pointer to the buffer to put received data.
   */
  uint8_t                   *rxbuf;
    /**
   * @brief     Number of bytes of data to receive.
   */
  size_t                    rxbytes;
  /**
   * @brief     State of the interrupt state machine.
   *
   * TODO is it possible to remove the interrupt state?
   */
  uint8_t                   intstate;
  /**
   * @brief     Pointer to the I2Cx registers block.
   */
  uint32_t                  i2c;
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
#if TIVA_I2C_USE_I2C0
extern I2CDriver I2CD1;
#endif

#if TIVA_I2C_USE_I2C1
extern I2CDriver I2CD2;
#endif

#if TIVA_I2C_USE_I2C2
extern I2CDriver I2CD3;
#endif

#if TIVA_I2C_USE_I2C3
extern I2CDriver I2CD4;
#endif

#if TIVA_I2C_USE_I2C4
extern I2CDriver I2CD5;
#endif

#if TIVA_I2C_USE_I2C5
extern I2CDriver I2CD6;
#endif

#if TIVA_I2C_USE_I2C6
extern I2CDriver I2CD7;
#endif

#if TIVA_I2C_USE_I2C7
extern I2CDriver I2CD8;
#endif

#if TIVA_I2C_USE_I2C8
extern I2CDriver I2CD9;
#endif

#if TIVA_I2C_USE_I2C9
extern I2CDriver I2CD10;
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
                                        systime_t timeout);
  msg_t i2c_lld_master_receive_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                       uint8_t *rxbuf, size_t rxbytes,
                                       systime_t timeout);
#ifdef __cplusplus
}
#endif

#endif /* HAL_USE_I2C  */

#endif /* HAL_I2C_LLD_H */

/** @} */
