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
 * @file    KINETIS/i2c_lld.c
 * @brief   KINETIS I2C subsystem low level driver source.
 *
 * @addtogroup I2C
 * @{
 */

#include "osal.h"
#include "hal.h"

#if HAL_USE_I2C || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   I2C0 driver identifier.
 */
#if KINETIS_I2C_USE_I2C0 || defined(__DOXYGEN__)
I2CDriver I2CD1;
#endif

/**
 * @brief   I2C1 driver identifier.
 */
#if KINETIS_I2C_USE_I2C1 || defined(__DOXYGEN__)
I2CDriver I2CD2;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

void config_frequency(I2CDriver *i2cp) {

  /* Each index in the table corresponds to a a frequency
   * divider used to generate the SCL clock from the main
   * system clock.
   */
  uint16_t icr_table[] = {
    /* 0x00 - 0x0F */
    20,22,24,26,28,30,34,40,28,32,36,40,44,48,56,68,
    /* 0x10 - 0x1F */
    48,56,64,72,80,88,104,128,80,96,112,128,144,160,192,240,
    /* 0x20 - 0x2F */
    160,192,224,256,288,320,384,480,320,384,448,512,576,640,768,960,
    /* 0x30 - 0x3F */
    640,768,896,1024,1152,1280,1536,1920,1280,1536,1792,2048,2304,2560,3072,3840,
  };

  int length = sizeof(icr_table) / sizeof(icr_table[0]);
  uint16_t divisor;
  uint8_t i = 0, index = 0;
  uint16_t best, diff;

  if (i2cp->config != NULL)
    divisor = KINETIS_SYSCLK_FREQUENCY / i2cp->config->clock;
  else
    divisor = KINETIS_SYSCLK_FREQUENCY / 100000;

  best = ~0;
  index = 0;
  /* Tries to find the SCL clock which is the closest
   * approximation to the clock passed in config. To
   * stay on the safe side, only values that generate
   * lower frequency are used.
   */
  for (i = 0; i < length; i++) {
    if (icr_table[i] >= divisor) {
      diff = icr_table[i] - divisor;
      if (diff < best) {
        best = diff;
        index = i;
      }
    }
  }

  i2cp->i2c->F = index;
}

/**
 * @brief   Common IRQ handler.
 * @note    Tries hard to clear all the pending interrupt sources, we don't
 *          want to go through the whole ISR and have another interrupt soon
 *          after.
 *
 * @param[in] i2cp         pointer to an I2CDriver
 */
static void serve_interrupt(I2CDriver *i2cp) {

  I2C_TypeDef *i2c = i2cp->i2c;
  intstate_t state = i2cp->intstate;

  if (i2c->S & I2Cx_S_ARBL) {

    i2cp->errors |= I2C_ARBITRATION_LOST;
    i2c->S |= I2Cx_S_ARBL;

  } else if (state == STATE_SEND) {

    if (i2c->S & I2Cx_S_RXAK)
      i2cp->errors |= I2C_ACK_FAILURE;
    else if (i2cp->txbuf != NULL && i2cp->txidx < i2cp->txbytes)
      i2c->D = i2cp->txbuf[i2cp->txidx++];
    else
      i2cp->intstate = STATE_STOP;

  } else if (state == STATE_DUMMY) {

    if (i2c->S & I2Cx_S_RXAK)
      i2cp->errors |= I2C_ACK_FAILURE;
    else {
      i2c->C1 &= ~I2Cx_C1_TX;

      if (i2cp->rxbytes > 1)
        i2c->C1 &= ~I2Cx_C1_TXAK;
      else
        i2c->C1 |= I2Cx_C1_TXAK;
      (void) i2c->D;
      i2cp->intstate = STATE_RECV;
    }

  } else if (state == STATE_RECV) {

    if (i2cp->rxbytes > 1) {
      if (i2cp->rxidx == (i2cp->rxbytes - 2))
        i2c->C1 |= I2Cx_C1_TXAK;
      else
        i2c->C1 &= ~I2Cx_C1_TXAK;
    }

    if (i2cp->rxidx == i2cp->rxbytes - 1)
      i2c->C1 &= ~(I2Cx_C1_TX | I2Cx_C1_MST);

    i2cp->rxbuf[i2cp->rxidx++] = i2c->D;

    if (i2cp->rxidx == i2cp->rxbytes)
      i2cp->intstate = STATE_STOP;
  }

  /* Reset interrupt flag */
  i2c->S |= I2Cx_S_IICIF;

  if (i2cp->errors != I2C_NO_ERROR)
    _i2c_wakeup_error_isr(i2cp);

  if (i2cp->intstate == STATE_STOP)
    _i2c_wakeup_isr(i2cp);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if KINETIS_I2C_USE_I2C0 || defined(__DOXYGEN__)

OSAL_IRQ_HANDLER(KINETIS_I2C0_IRQ_VECTOR) {

  OSAL_IRQ_PROLOGUE();
  serve_interrupt(&I2CD1);
  OSAL_IRQ_EPILOGUE();
}

#endif

#if KINETIS_I2C_USE_I2C1 || defined(__DOXYGEN__)

/* FIXME: KL2x has I2C1 on Vector64; K2x don't have I2C1! */
OSAL_IRQ_HANDLER(Vector64) {

  OSAL_IRQ_PROLOGUE();
  serve_interrupt(&I2CD2);
  OSAL_IRQ_EPILOGUE();
}

#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level I2C driver initialization.
 *
 * @notapi
 */
void i2c_lld_init(void) {

#if KINETIS_I2C_USE_I2C0
  i2cObjectInit(&I2CD1);
  I2CD1.thread = NULL;
  I2CD1.i2c = I2C0;
#endif

#if KINETIS_I2C_USE_I2C1
  i2cObjectInit(&I2CD2);
  I2CD2.thread = NULL;
  I2CD2.i2c = I2C1;
#endif

}

/**
 * @brief   Configures and activates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
void i2c_lld_start(I2CDriver *i2cp) {

  if (i2cp->state == I2C_STOP) {

  /* TODO:
   *   The PORT must be enabled somewhere. The PIN multiplexer
   *   will map the I2C functionality to some PORT which must
   *   than be enabled. The easier way is enabling all PORTs at
   *   startup, which is currently being done in __early_init.
   */

#if KINETIS_I2C_USE_I2C0
    if (&I2CD1 == i2cp) {
      SIM->SCGC4 |= SIM_SCGC4_I2C0;
      nvicEnableVector(I2C0_IRQn, KINETIS_I2C_I2C0_PRIORITY);
    }
#endif

#if KINETIS_I2C_USE_I2C1
    if (&I2CD2 == i2cp) {
      SIM->SCGC4 |= SIM_SCGC4_I2C1;
      nvicEnableVector(I2C1_IRQn, KINETIS_I2C_I2C1_PRIORITY);
    }
#endif

  }

  config_frequency(i2cp);
  i2cp->i2c->C1 |= I2Cx_C1_IICEN | I2Cx_C1_IICIE;
  i2cp->intstate = STATE_STOP;
}

/**
 * @brief   Deactivates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
void i2c_lld_stop(I2CDriver *i2cp) {

  if (i2cp->state != I2C_STOP) {

    i2cp->i2c->C1 &= ~(I2Cx_C1_IICEN | I2Cx_C1_IICIE);

#if KINETIS_I2C_USE_I2C0
    if (&I2CD1 == i2cp) {
      SIM->SCGC4 &= ~SIM_SCGC4_I2C0;
      nvicDisableVector(I2C0_IRQn);
    }
#endif

#if KINETIS_I2C_USE_I2C1
    if (&I2CD2 == i2cp) {
      SIM->SCGC4 &= ~SIM_SCGC4_I2C1;
      nvicDisableVector(I2C1_IRQn);
    }
#endif

  }
}

static inline msg_t _i2c_txrx_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                      const uint8_t *txbuf, size_t txbytes,
                                      uint8_t *rxbuf, size_t rxbytes,
                                      systime_t timeout) {

  (void)timeout;
  msg_t msg;

  uint8_t op = (i2cp->intstate == STATE_SEND) ? 0 : 1;

  i2cp->errors = I2C_NO_ERROR;
  i2cp->addr = addr;

  i2cp->txbuf = txbuf;
  i2cp->txbytes = txbytes;
  i2cp->txidx = 0;

  i2cp->rxbuf = rxbuf;
  i2cp->rxbytes = rxbytes;
  i2cp->rxidx = 0;

  /* send START */
  i2cp->i2c->C1 |= I2Cx_C1_MST;
  i2cp->i2c->C1 |= I2Cx_C1_TX;

  /* FIXME: should not use busy waiting! */
  while (!(i2cp->i2c->S & I2Cx_S_BUSY));

  i2cp->i2c->D = addr << 1 | op;

  msg = osalThreadSuspendTimeoutS(&i2cp->thread, TIME_INFINITE);

  /* FIXME */
  //if (i2cp->i2c->S & I2Cx_S_RXAK)
  //  i2cp->errors |= I2C_ACK_FAILURE;

  if (msg == MSG_OK && txbuf != NULL && rxbuf != NULL) {
    i2cp->i2c->C1 |= I2Cx_C1_RSTA;
    /* FIXME */
    while (!(i2cp->i2c->S & I2Cx_S_BUSY));

    i2cp->intstate = STATE_DUMMY;
    i2cp->i2c->D = i2cp->addr << 1 | 1;

    msg = osalThreadSuspendTimeoutS(&i2cp->thread, TIME_INFINITE);
  }

  i2cp->i2c->C1 &= ~(I2Cx_C1_TX | I2Cx_C1_MST);
  /* FIXME */
  while (i2cp->i2c->S & I2Cx_S_BUSY);

  return msg;
}

/**
 * @brief   Receives data via the I2C bus as master.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address
 * @param[out] rxbuf    pointer to the receive buffer
 * @param[in] rxbytes   number of bytes to be received
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end. <b>After a
 *                      timeout the driver must be stopped and restarted
 *                      because the bus is in an uncertain state</b>.
 *
 * @notapi
 */
msg_t i2c_lld_master_receive_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                     uint8_t *rxbuf, size_t rxbytes,
                                     systime_t timeout) {

  i2cp->intstate = STATE_DUMMY;
  return _i2c_txrx_timeout(i2cp, addr, NULL, 0, rxbuf, rxbytes, timeout);
}

/**
 * @brief   Transmits data via the I2C bus as master.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address
 * @param[in] txbuf     pointer to the transmit buffer
 * @param[in] txbytes   number of bytes to be transmitted
 * @param[out] rxbuf    pointer to the receive buffer
 * @param[in] rxbytes   number of bytes to be received
 * @param[in] timeout   the number of ticks before the operation timeouts,
 *                      the following special values are allowed:
 *                      - @a TIME_INFINITE no timeout.
 *                      .
 * @return              The operation status.
 * @retval MSG_OK       if the function succeeded.
 * @retval MSG_RESET    if one or more I2C errors occurred, the errors can
 *                      be retrieved using @p i2cGetErrors().
 * @retval MSG_TIMEOUT  if a timeout occurred before operation end. <b>After a
 *                      timeout the driver must be stopped and restarted
 *                      because the bus is in an uncertain state</b>.
 *
 * @notapi
 */
msg_t i2c_lld_master_transmit_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                      const uint8_t *txbuf, size_t txbytes,
                                      uint8_t *rxbuf, size_t rxbytes,
                                      systime_t timeout) {

  i2cp->intstate = STATE_SEND;
  return _i2c_txrx_timeout(i2cp, addr, txbuf, txbytes, rxbuf, rxbytes, timeout);
}

#endif /* HAL_USE_I2C */

/** @} */
