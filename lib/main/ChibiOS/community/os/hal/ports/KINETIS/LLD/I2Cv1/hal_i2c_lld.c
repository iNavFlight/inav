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
 * @file    I2Cv1/hal_i2c_lld.c
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
  const uint16_t icr_table[] = {
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
    divisor = KINETIS_BUSCLK_FREQUENCY / i2cp->config->clock;
  else
    divisor = KINETIS_BUSCLK_FREQUENCY / 100000;

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

  /* check if we're master or slave */
  if (i2c->C1 & I2Cx_C1_MST) {
    /* master */

    if (i2c->S & I2Cx_S_ARBL) {
      /* check if we lost arbitration */
      i2cp->errors |= I2C_ARBITRATION_LOST;
      i2c->S |= I2Cx_S_ARBL;
      /* TODO: may need to do more here, reset bus? */
      /* Perhaps clear MST? */
    }

#if defined(KL27Zxxx) || defined(KL27Zxx) /* KL27Z RST workaround */
    else if ((i2cp->rsta_workaround == RSTA_WORKAROUND_ON) && (i2cp->i2c->FLT & I2Cx_FLT_STARTF)) {
      i2cp->rsta_workaround = RSTA_WORKAROUND_OFF;
      /* clear+disable STARTF/STOPF interrupts and wake up the thread */
      i2cp->i2c->FLT |= I2Cx_FLT_STOPF|I2Cx_FLT_STARTF;
      i2cp->i2c->FLT &= ~I2Cx_FLT_SSIE;
      i2c->S |= I2Cx_S_IICIF;
      _i2c_wakeup_isr(i2cp);
    }
#endif /* KL27Z RST workaround */

    else if (i2c->S & I2Cx_S_TCF) {
      /* just completed byte transfer */
      if (i2c->C1 & I2Cx_C1_TX) {
        /* the byte was transmitted */

        if (state == STATE_SEND) {
          /* currently sending stuff */

          if (i2c->S & I2Cx_S_RXAK) {
            /* slave did not ACK */
            i2cp->errors |= I2C_ACK_FAILURE;
            /* the thread will be woken up at the end of ISR and release the bus */

          } else if (i2cp->txbuf != NULL && i2cp->txidx < i2cp->txbytes) {
            /* slave ACK'd and we want to send more */
            i2c->D = i2cp->txbuf[i2cp->txidx++];
          } else {
            /* slave ACK'd and we are done sending */
            i2cp->intstate = STATE_STOP;
            /* this wakes up the waiting thread at the end of ISR */
          }

        } else if (state == STATE_RECV) {
          /* should be receiving stuff, so we've just sent the address */

          if (i2c->S & I2Cx_S_RXAK) {
            /* slave did not ACK */
            i2cp->errors |= I2C_ACK_FAILURE;
            /* the thread will be woken up and release the bus */

          } else {
            /* slave ACK'd, we should be receiving next */
            i2c->C1 &= ~I2Cx_C1_TX;

            if (i2cp->rxbytes > 1) {
              /* multi-byte read, send ACK after next transfer */
              i2c->C1 &= ~I2Cx_C1_TXAK;
            } else {
              /* only 1 byte remaining, send NAK */
              i2c->C1 |= I2Cx_C1_TXAK;
            }

            (void) i2c->D; /* dummy read; triggers next receive */
          }

        } /* possibly check other states here - should not happen! */

      } else {
        /* the byte was received */

        if (state == STATE_RECV) {
          /* currently receiving stuff */
          /* the received byte is now in D */

          if (i2cp->rxbytes > 1) {
            /* expecting at least one byte after this one */
            if (i2cp->rxidx == (i2cp->rxbytes - 2)) {
              /* expecting exactly one byte after this one, NAK that one */
              i2c->C1 |= I2Cx_C1_TXAK;
            } else {
              /* expecting more than one after this one, respond with ACK */
              i2c->C1 &= ~I2Cx_C1_TXAK;
            }
          }

          if (i2cp->rxidx == i2cp->rxbytes - 1) {
            /* D is the last byte we're expecting */
            /* release bus: switch to RX mode, send STOP */
            /* need to do it now otherwise the I2C module will wait for another byte */
            // delayMicroseconds(1);
            i2c->C1 &= ~(I2Cx_C1_TX | I2Cx_C1_MST);
            i2cp->intstate = STATE_STOP;
            /* this wakes up the waiting thread at the end of ISR */
          }

          /* get the data from D; this triggers the next receive */
          i2cp->rxbuf[i2cp->rxidx++] = i2c->D;

          // if (i2cp->rxidx == i2cp->rxbytes) {
            /* done receiving */
          // }
        } /* possibly check other states here - should not happen! */
      }

    } /* possibly check other interrupt flags here */
  } else {
    /* slave */

    /* Not implemented yet */
  }

  /* Reset other interrupt sources */
#if defined(I2Cx_FLT_STOPF) /* extra flags on KL26Z and KL27Z */
  i2cp->i2c->FLT |= I2Cx_FLT_STOPF;
#endif
#if defined(I2Cx_FLT_STARTF) /* extra flags on KL27Z */
  i2cp->i2c->FLT |= I2Cx_FLT_STARTF;
#endif
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

OSAL_IRQ_HANDLER(KINETIS_I2C1_IRQ_VECTOR) {

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
  i2cp->i2c->C1 = I2Cx_C1_IICEN | I2Cx_C1_IICIE; // reset I2C, enable interrupts
  i2cp->i2c->S = I2Cx_S_IICIF | I2Cx_S_ARBL; // clear status flags just in case
  i2cp->intstate = STATE_STOP; // internal state
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

  msg_t msg;
  systime_t start, end;

  uint8_t op = (i2cp->intstate == STATE_SEND) ? 0 : 1;

  i2cp->errors = I2C_NO_ERROR;
  i2cp->addr = addr;

  i2cp->txbuf = txbuf;
  i2cp->txbytes = txbytes;
  i2cp->txidx = 0;

  i2cp->rxbuf = rxbuf;
  i2cp->rxbytes = rxbytes;
  i2cp->rxidx = 0;

#if defined(KL27Zxxx) || defined(KL27Zxx) /* KL27Z RST workaround */
  i2cp->rsta_workaround = RSTA_WORKAROUND_OFF;
#endif /* KL27Z RST workaround */

  /* clear status flags */
#if defined(I2Cx_FLT_STOPF) /* extra flags on KL26Z and KL27Z */
  i2cp->i2c->FLT |= I2Cx_FLT_STOPF;
#endif
#if defined(I2Cx_FLT_STARTF) /* extra flags on KL27Z */
  i2cp->i2c->FLT |= I2Cx_FLT_STARTF;
#endif
  i2cp->i2c->S = I2Cx_S_IICIF|I2Cx_S_ARBL;

  /* acquire the bus */
  /* check to see if we already have the bus */
  if(i2cp->i2c->C1 & I2Cx_C1_MST) {

#if defined(KL27Zxxx) || defined(KL27Zxx) /* KL27Z RST workaround */
    /* need to wait for STARTF interrupt after issuing repeated start,
     * otherwise the double buffering mechanism sends the last sent byte
     * instead of the slave address.
     * https://community.freescale.com/thread/377611
     */
    i2cp->rsta_workaround = RSTA_WORKAROUND_ON;
    /* clear any interrupt bits and enable STARTF/STOPF interrupts */
    i2cp->i2c->FLT |= I2Cx_FLT_STOPF|I2Cx_FLT_STARTF;
    i2cp->i2c->S |= I2Cx_S_IICIF|I2Cx_S_ARBL;
    i2cp->i2c->FLT |= I2Cx_FLT_SSIE;
#endif /* KL27Z RST workaround */

    /* send repeated start */
    i2cp->i2c->C1 |= I2Cx_C1_RSTA | I2Cx_C1_TX;

#if defined(KL27Zxxx) || defined(KL27Zxx) /* KL27Z RST workaround */
    /* wait for the STARTF interrupt */
    msg = osalThreadSuspendTimeoutS(&i2cp->thread, timeout);
    /* abort if this didn't go well (timed out) */
    if (msg != MSG_OK) {
      /* release bus - RX mode, send STOP */
      i2cp->i2c->C1 &= ~(I2Cx_C1_TX | I2Cx_C1_MST);
      return msg;
    }
#endif /* KL27Z RST workaround */

  } else {
    /* unlock during the wait, so that tasks with
     * higher priority can get attention */
    osalSysUnlock();

    /* wait until the bus is released */
    /* Calculating the time window for the timeout on the busy bus condition.*/
    start = osalOsGetSystemTimeX();
#if defined(OSAL_MS2I)
    end = start + OSAL_MS2I(KINETIS_I2C_BUSY_TIMEOUT);
#elif defined(OSAL_TIME_MS2I)
    end = start + OSAL_TIME_MS2I(KINETIS_I2C_BUSY_TIMEOUT);
#elif defined(OSAL_TIME_MS2ST)
    end = start + OSAL_TIME_MS2ST(KINETIS_I2C_BUSY_TIMEOUT);
#else
    end = start + OSAL_MS2ST(KINETIS_I2C_BUSY_TIMEOUT);
#endif

    while(true) {
      osalSysLock();
      /* If the bus is not busy then the operation can continue, note, the
         loop is exited in the locked state.*/
      if(!(i2cp->i2c->S & I2Cx_S_BUSY))
        break;
      /* If the system time went outside the allowed window then a timeout
         condition is returned.*/
      if (!osalTimeIsInRangeX(osalOsGetSystemTimeX(), start, end)) {
        return MSG_TIMEOUT;
      }
      osalSysUnlock();
    }

    /* send START */
    i2cp->i2c->C1 |= I2Cx_C1_MST|I2Cx_C1_TX;
  }

  /* send slave address */
  i2cp->i2c->D = addr << 1 | op;

  /* wait for the ISR to signal that the transmission (or receive if no transmission) phase is complete */
  msg = osalThreadSuspendTimeoutS(&i2cp->thread, timeout);

  /* FIXME */
  //if (i2cp->i2c->S & I2Cx_S_RXAK)
  //  i2cp->errors |= I2C_ACK_FAILURE;

  /* the transmitting (or receiving if no transmission) phase has finished,
   * do we expect to receive something? */
  if (msg == MSG_OK && rxbuf != NULL && rxbytes > 0 && i2cp->rxidx < rxbytes) {

#if defined(KL27Zxxx) || defined(KL27Zxx) /* KL27Z RST workaround */
    /* the same KL27Z RST workaround as above */
    i2cp->rsta_workaround = RSTA_WORKAROUND_ON;
    /* clear any interrupt bits and enable STARTF/STOPF interrupts */
    i2cp->i2c->FLT |= I2Cx_FLT_STOPF|I2Cx_FLT_STARTF;
    i2cp->i2c->S |= I2Cx_S_IICIF|I2Cx_S_ARBL;
    i2cp->i2c->FLT |= I2Cx_FLT_SSIE;
#endif /* KL27Z RST workaround */

    /* send repeated start */
    i2cp->i2c->C1 |= I2Cx_C1_RSTA;

#if defined(KL27Zxxx) || defined(KL27Zxx) /* KL27Z RST workaround */
    /* wait for the STARTF interrupt */
    msg = osalThreadSuspendTimeoutS(&i2cp->thread, timeout);
    /* abort if this didn't go well (timed out) */
    if (msg != MSG_OK) {
      /* release bus - RX mode, send STOP */
      i2cp->i2c->C1 &= ~(I2Cx_C1_TX | I2Cx_C1_MST);
      return msg;
    }
#endif /* KL27Z RST workaround */

    /* FIXME */
    // while (!(i2cp->i2c->S & I2Cx_S_BUSY));

    i2cp->intstate = STATE_RECV;
    i2cp->i2c->D = i2cp->addr << 1 | 1;

    msg = osalThreadSuspendTimeoutS(&i2cp->thread, timeout);
  }

  /* release bus - RX mode, send STOP */
  // other kinetis I2C drivers wait here for 1us. is this needed?
  i2cp->i2c->C1 &= ~(I2Cx_C1_TX | I2Cx_C1_MST);
  /* FIXME */
  // while (i2cp->i2c->S & I2Cx_S_BUSY);

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

  i2cp->intstate = STATE_RECV;
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
