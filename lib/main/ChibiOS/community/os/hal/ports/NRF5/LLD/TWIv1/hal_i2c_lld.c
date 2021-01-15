/*
    Copyright (C) 2015 Stephen Caudle

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
 * @file    TWIv1/hal_i2c_lld.c
 * @brief   NRF5 I2C subsystem low level driver source.
 *
 * @addtogroup I2C
 * @{
 */

#include "osal.h"
#include "hal.h"
#include "nrf_delay.h"

#if HAL_USE_I2C || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/* These macros are needed to see if the slave is stuck and we as master send dummy clock cycles to end its wait */
#define I2C_HIGH(p)   do { IOPORT1->OUTSET = (1UL << (p)); } while(0)   /*!< Pulls I2C line high */
#define I2C_LOW(p)    do { IOPORT1->OUTCLR = (1UL << (p)); } while(0)   /*!< Pulls I2C line low  */
#define I2C_INPUT(p)  do { IOPORT1->DIRCLR = (1UL << (p)); } while(0)   /*!< Configures I2C pin as input  */
#define I2C_OUTPUT(p) do { IOPORT1->DIRSET = (1UL << (p)); } while(0)   /*!< Configures I2C pin as output */

#define I2C_PIN_CNF \
      ((GPIO_PIN_CNF_SENSE_Disabled  << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
      | (GPIO_PIN_CNF_PULL_Disabled  << GPIO_PIN_CNF_PULL_Pos)  \
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
      | (GPIO_PIN_CNF_DIR_Input      << GPIO_PIN_CNF_DIR_Pos))

#define I2C_PIN_CNF_CLR \
      ((GPIO_PIN_CNF_SENSE_Disabled  << GPIO_PIN_CNF_SENSE_Pos) \
      | (GPIO_PIN_CNF_DRIVE_S0D1     << GPIO_PIN_CNF_DRIVE_Pos) \
      | (GPIO_PIN_CNF_PULL_Disabled  << GPIO_PIN_CNF_PULL_Pos)  \
      | (GPIO_PIN_CNF_INPUT_Connect  << GPIO_PIN_CNF_INPUT_Pos) \
      | (GPIO_PIN_CNF_DIR_Output     << GPIO_PIN_CNF_DIR_Pos))

#if NRF5_I2C_USE_I2C0
#define I2C_IRQ_NUM     SPI0_TWI0_IRQn
#define I2C_IRQ_PRI     NRF5_I2C_I2C0_IRQ_PRIORITY
#elif NRF5_I2C_USE_I2C1
#define I2C_IRQ_NUM     SPI1_TWI1_IRQn
#define I2C_IRQ_PRI     NRF5_I2C_I2C1_IRQ_PRIORITY
#endif

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   I2C0 driver identifier.
 */
#if NRF5_I2C_USE_I2C0 || defined(__DOXYGEN__)
I2CDriver I2CD1;
#endif

/**
 * @brief   I2C1 driver identifier.
 */
#if NRF5_I2C_USE_I2C1 || defined(__DOXYGEN__)
I2CDriver I2CD2;
#endif

uint8_t tx_resume_count;
uint8_t rx_resume_count;
uint8_t stop_count;

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief Function for detecting stuck slaves (SDA = 0 and SCL = 1) and tries to clear the bus.
 *
 * @return
 * @retval false Bus is stuck.
 * @retval true Bus is clear.
 */
static void i2c_clear_bus(I2CDriver *i2cp)
{
  const I2CConfig *cfg = i2cp->config;
  int i;

  IOPORT1->PIN_CNF[cfg->scl_pad] = I2C_PIN_CNF;
  IOPORT1->PIN_CNF[cfg->sda_pad] = I2C_PIN_CNF;

  I2C_HIGH(cfg->sda_pad);
  I2C_HIGH(cfg->scl_pad);

  IOPORT1->PIN_CNF[cfg->scl_pad] = I2C_PIN_CNF_CLR;
  IOPORT1->PIN_CNF[cfg->sda_pad] = I2C_PIN_CNF_CLR;

  nrf_delay_us(4);

  for(i = 0; i < 9; i++) {
    if (palReadPad(IOPORT1, cfg->sda_pad)) {
      if(i > 0)
        break;
      else
        return;
    }

    I2C_LOW(cfg->scl_pad);
    nrf_delay_us(4);
    I2C_HIGH(cfg->scl_pad);
    nrf_delay_us(4);
  }

  I2C_LOW(cfg->sda_pad);
  nrf_delay_us(4);
  I2C_HIGH(cfg->sda_pad);
}

static inline void i2c_setup_shortcut(I2CDriver *i2cp)
{
  uint32_t rxbytes = i2cp->rxbytes;
  uint32_t txbytes = i2cp->txbytes;

  osalDbgAssert(rxbytes + txbytes, "transfer must be greater than zero");

  if (txbytes > 1 || (!txbytes && rxbytes > 1))
    i2cp->i2c->SHORTS = TWI_SHORTS_BB_SUSPEND_Enabled << TWI_SHORTS_BB_SUSPEND_Pos;
  else if (((txbytes == 1)  && !rxbytes) || ((rxbytes == 1) && !txbytes))
    i2cp->i2c->SHORTS = TWI_SHORTS_BB_STOP_Enabled << TWI_SHORTS_BB_STOP_Pos;
  else
    i2cp->i2c->SHORTS = 0;
}

#if defined(__GNUC__)
__attribute__((noinline))
#endif
/**
 * @brief   Common IRQ handler.
 * @note    Tries hard to clear all the pending interrupt sources, we don't
 *          want to go through the whole ISR and have another interrupt soon
 *          after.
 *
 * @param[in] i2cp         pointer to an I2CDriver
 */
static void serve_interrupt(I2CDriver *i2cp) {

  NRF_TWI_Type *i2c = i2cp->i2c;

  if(i2c->EVENTS_TXDSENT) {

    i2c->EVENTS_TXDSENT = 0;
#if CORTEX_MODEL >= 4
    (void)i2c->EVENTS_TXDSENT;
#endif
    
    if(--i2cp->txbytes) {

      i2c->TXD = *i2cp->txptr++;
      i2c_setup_shortcut(i2cp);
      i2c->TASKS_RESUME = 1;
      tx_resume_count++;
    }
    else if (i2cp->rxbytes) {

      i2c_setup_shortcut(i2cp);
      i2c->TASKS_STARTRX = 1;
    }
  }
  if(i2c->EVENTS_RXDREADY) {

    i2c->EVENTS_RXDREADY = 0;
#if CORTEX_MODEL >= 4
    (void)i2c->EVENTS_RXDREADY;
#endif
    
    *i2cp->rxptr++ = i2c->RXD;

    if(--i2cp->rxbytes) {
      i2c_setup_shortcut(i2cp);
      i2c->TASKS_RESUME = 1;
      rx_resume_count++;
    }
  }
  if(i2c->EVENTS_ERROR) {

    uint32_t err = i2c->ERRORSRC;
    i2c->EVENTS_ERROR = 0;
#if CORTEX_MODEL >= 4
    (void)i2c->EVENTS_ERROR;
#endif
    if (err & TWI_ERRORSRC_OVERRUN_Msk)
      i2cp->errors |= I2C_OVERRUN;
    if (err & (TWI_ERRORSRC_ANACK_Msk | TWI_ERRORSRC_DNACK_Msk))
      i2cp->errors |= I2C_ACK_FAILURE;

    i2c->TASKS_STOP = 1;
    _i2c_wakeup_error_isr(i2cp);
  } else if(i2c->EVENTS_STOPPED) {

    stop_count++;
    i2c->EVENTS_STOPPED = 0;
#if CORTEX_MODEL >= 4
    (void)i2c->EVENTS_STOPPED;
#endif
    _i2c_wakeup_isr(i2cp);
  }
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if NRF5_I2C_USE_I2C0 || defined(__DOXYGEN__)

OSAL_IRQ_HANDLER(Vector4C) {

  OSAL_IRQ_PROLOGUE();
  serve_interrupt(&I2CD1);
  OSAL_IRQ_EPILOGUE();
}

#endif

#if NRF5_I2C_USE_I2C1 || defined(__DOXYGEN__)

OSAL_IRQ_HANDLER(Vector50) {

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

#if NRF5_I2C_USE_I2C0
  i2cObjectInit(&I2CD1);
  I2CD1.thread = NULL;
  I2CD1.i2c = NRF_TWI0;
#endif

#if NRF5_I2C_USE_I2C1
  i2cObjectInit(&I2CD2);
  I2CD2.thread = NULL;
  I2CD2.i2c = NRF_TWI1;
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

  NRF_TWI_Type *i2c = i2cp->i2c;
  const I2CConfig *cfg = i2cp->config;

  if (i2cp->state != I2C_STOP)
    return;

  i2c_clear_bus(i2cp);

  IOPORT1->PIN_CNF[cfg->scl_pad] = I2C_PIN_CNF;
  IOPORT1->PIN_CNF[cfg->sda_pad] = I2C_PIN_CNF;

  i2c->EVENTS_RXDREADY = 0;
  i2c->EVENTS_TXDSENT = 0;
#if CORTEX_MODEL >= 4
  (void)i2c->EVENTS_RXDREADY;
  (void)i2c->EVENTS_TXDSENT;
#endif
#if NRF_SERIES == 51
  i2c->PSELSCL = cfg->scl_pad;
  i2c->PSELSDA = cfg->sda_pad;
#else
  i2c->PSEL.SCL = cfg->scl_pad;
  i2c->PSEL.SDA = cfg->sda_pad;
#endif
  
  switch (cfg->clock) {
    case 100000:
      i2c->FREQUENCY = TWI_FREQUENCY_FREQUENCY_K100 << TWI_FREQUENCY_FREQUENCY_Pos;
      break;
    case 250000:
      i2c->FREQUENCY = TWI_FREQUENCY_FREQUENCY_K250 << TWI_FREQUENCY_FREQUENCY_Pos;
      break;
    case 400000:
      i2c->FREQUENCY = TWI_FREQUENCY_FREQUENCY_K400 << TWI_FREQUENCY_FREQUENCY_Pos;
      break;
    default:
      osalDbgAssert(0, "invalid I2C frequency");
      break;
  };

  nvicEnableVector(I2C_IRQ_NUM, I2C_IRQ_PRI);

  i2c->INTENSET = TWI_INTENSET_TXDSENT_Msk | TWI_INTENSET_STOPPED_Msk |
    TWI_INTENSET_ERROR_Msk | TWI_INTENSET_RXDREADY_Msk;

  i2c->ENABLE = TWI_ENABLE_ENABLE_Enabled << TWI_ENABLE_ENABLE_Pos;
}

/**
 * @brief   Deactivates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
void i2c_lld_stop(I2CDriver *i2cp) {

  NRF_TWI_Type *i2c = i2cp->i2c;
  const I2CConfig *cfg = i2cp->config;

  if (i2cp->state != I2C_STOP) {

    i2c->ENABLE = TWI_ENABLE_ENABLE_Disabled << TWI_ENABLE_ENABLE_Pos;

    i2c->INTENCLR = TWI_INTENSET_TXDSENT_Msk | TWI_INTENSET_STOPPED_Msk |
      TWI_INTENSET_ERROR_Msk | TWI_INTENSET_RXDREADY_Msk;

    nvicDisableVector(I2C_IRQ_NUM);

    IOPORT1->PIN_CNF[cfg->scl_pad] = I2C_PIN_CNF_CLR;
    IOPORT1->PIN_CNF[cfg->sda_pad] = I2C_PIN_CNF_CLR;
  }
}

static inline msg_t _i2c_txrx_timeout(I2CDriver *i2cp, i2caddr_t addr,
                                      const uint8_t *txbuf, size_t txbytes,
                                      uint8_t *rxbuf, size_t rxbytes,
                                      systime_t timeout) {

  NRF_TWI_Type *i2c = i2cp->i2c;

  (void)timeout;
  msg_t msg;

  i2cp->errors = I2C_NO_ERROR;
  i2cp->addr = addr;

  i2cp->txptr = txbuf;
  i2cp->txbytes = txbytes;

  i2cp->rxptr = rxbuf;
  i2cp->rxbytes = rxbytes;

  i2c->ADDRESS = addr;

  tx_resume_count = 0;
  rx_resume_count = 0;
  stop_count = 0;

  if (i2cp->txbytes) {

    i2c->TXD = *i2cp->txptr++;
    i2c_setup_shortcut(i2cp);
    i2c->TASKS_STARTTX = 1;
  } else if (i2cp->rxbytes) {

    i2c_setup_shortcut(i2cp);
    i2c->TASKS_STARTRX = 1;
  } else {

    osalDbgAssert(0, "no bytes to transfer");
  }

  msg = osalThreadSuspendTimeoutS(&i2cp->thread, timeout);

  if (msg == MSG_TIMEOUT)
    i2c->TASKS_STOP = 1;

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

  return _i2c_txrx_timeout(i2cp, addr, txbuf, txbytes, rxbuf, rxbytes, timeout);
}

#endif /* HAL_USE_I2C */

/** @} */
