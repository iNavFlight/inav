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
 * @file    I2Cv3/hal_i2c_lld.c
 * @brief   STM32 I2C subsystem low level driver source.
 *
 * @addtogroup I2C
 * @{
 */

#include "hal.h"

#if HAL_USE_I2C || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if STM32_I2C_USE_DMA == TRUE

#if defined(STM32_I2C_DMA_REQUIRED)
#define DMAMODE_COMMON                                                      \
  (STM32_DMA_CR_PSIZE_BYTE | STM32_DMA_CR_MSIZE_BYTE |                      \
   STM32_DMA_CR_MINC       | STM32_DMA_CR_DMEIE      |                      \
   STM32_DMA_CR_TEIE       | STM32_DMA_CR_TCIE)
#endif

#if defined(STM32_I2C_BDMA_REQUIRED)
#define BDMAMODE_COMMON                                                     \
  (STM32_BDMA_CR_PSIZE_BYTE | STM32_BDMA_CR_MSIZE_BYTE |                    \
   STM32_BDMA_CR_MINC       |                                               \
   STM32_BDMA_CR_TEIE       | STM32_BDMA_CR_TCIE)
#endif

#endif /* STM32_I2C_USE_DMA == TRUE */

#if 0
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  if(i2cp->is_bdma)
#endif
#if defined(STM32_I2C_BDMA_REQUIRED)
  {
  }
#endif
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  else
#endif
#if defined(STM32_I2C_DMA_REQUIRED)
  {
  }
#endif
#endif

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

#define I2C_ERROR_MASK                                                      \
  ((uint32_t)(I2C_ISR_BERR | I2C_ISR_ARLO | I2C_ISR_OVR | I2C_ISR_PECERR |  \
              I2C_ISR_TIMEOUT | I2C_ISR_ALERT))

#define I2C_INT_MASK                                                        \
  ((uint32_t)(I2C_ISR_TCR | I2C_ISR_TC | I2C_ISR_STOPF | I2C_ISR_NACKF |    \
              I2C_ISR_ADDR | I2C_ISR_RXNE | I2C_ISR_TXIS))

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief I2C1 driver identifier.*/
#if STM32_I2C_USE_I2C1 || defined(__DOXYGEN__)
I2CDriver I2CD1;
#endif

/** @brief I2C2 driver identifier.*/
#if STM32_I2C_USE_I2C2 || defined(__DOXYGEN__)
I2CDriver I2CD2;
#endif

/** @brief I2C3 driver identifier.*/
#if STM32_I2C_USE_I2C3 || defined(__DOXYGEN__)
I2CDriver I2CD3;
#endif

/** @brief I2C4 driver identifier.*/
#if STM32_I2C_USE_I2C4 || defined(__DOXYGEN__)
I2CDriver I2CD4;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

#if STM32_I2C_USE_DMA == TRUE
static inline void i2c_lld_start_rx_dma(I2CDriver *i2cp) {

#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  if(i2cp->is_bdma)
#endif
#if defined(STM32_I2C_BDMA_REQUIRED)
  {
    bdmaStreamEnable(i2cp->rx.bdma);
  }
#endif
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  else
#endif
#if defined(STM32_I2C_DMA_REQUIRED)
  {
    dmaStreamEnable(i2cp->rx.dma);
  }
#endif
}

static inline void i2c_lld_start_tx_dma(I2CDriver *i2cp) {

#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  if(i2cp->is_bdma)
#endif
#if defined(STM32_I2C_BDMA_REQUIRED)
  {
    bdmaStreamEnable(i2cp->tx.bdma);
  }
#endif
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  else
#endif
#if defined(STM32_I2C_DMA_REQUIRED)
  {
    dmaStreamEnable(i2cp->tx.dma);
  }
#endif
}

static inline void i2c_lld_stop_rx_dma(I2CDriver *i2cp) {

#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  if(i2cp->is_bdma)
#endif
#if defined(STM32_I2C_BDMA_REQUIRED)
  {
    bdmaStreamDisable(i2cp->rx.bdma);
  }
#endif
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  else
#endif
#if defined(STM32_I2C_DMA_REQUIRED)
  {
    dmaStreamDisable(i2cp->rx.dma);
  }
#endif
}

static inline void i2c_lld_stop_tx_dma(I2CDriver *i2cp) {

#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  if(i2cp->is_bdma)
#endif
#if defined(STM32_I2C_BDMA_REQUIRED)
  {
    bdmaStreamDisable(i2cp->tx.bdma);
  }
#endif
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  else
#endif
#if defined(STM32_I2C_DMA_REQUIRED)
  {
    dmaStreamDisable(i2cp->tx.dma);
  }
#endif
}
#endif /* STM32_I2C_USE_DMA == TRUE */

/**
 * @brief   Slave address setup.
 * @note    The RW bit is set to zero internally.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] addr      slave device address
 *
 * @notapi
 */
static void i2c_lld_set_address(I2CDriver *i2cp, i2caddr_t addr) {
  I2C_TypeDef *dp = i2cp->i2c;

  /* Address alignment depends on the addressing mode selected.*/
  if ((i2cp->config->cr2 & I2C_CR2_ADD10) == 0U)
    dp->CR2 = (uint32_t)addr << 1U;
  else
    dp->CR2 = (uint32_t)addr;
}

/**
 * @brief   I2C RX transfer setup.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_setup_rx_transfer(I2CDriver *i2cp) {
  I2C_TypeDef *dp = i2cp->i2c;
  uint32_t reload;
  size_t n;

  /* The unit can transfer 255 bytes maximum in a single operation.*/
  n = i2cp->rxbytes;
  if (n > 255U) {
    n = 255U;
    reload = I2C_CR2_RELOAD;
  }
  else {
    reload = 0U;
  }
  i2cp->rxbytes -= n;

  /* Configures the CR2 registers with both the calculated and static
     settings.*/
  dp->CR2 = (dp->CR2 & ~(I2C_CR2_NBYTES | I2C_CR2_RELOAD)) | i2cp->config->cr2 |
            I2C_CR2_RD_WRN | (n << 16U) | reload;
}

/**
 * @brief   I2C TX transfer setup.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_setup_tx_transfer(I2CDriver *i2cp) {
  I2C_TypeDef *dp = i2cp->i2c;
  uint32_t reload;
  size_t n;

  /* The unit can transfer 255 bytes maximum in a single operation.*/
  n = i2cp->txbytes;
  if (n > 255U) {
    n = 255U;
    reload = I2C_CR2_RELOAD;
  }
  else {
    reload = 0U;
  }
  i2cp->txbytes -= n;

  /* Configures the CR2 registers with both the calculated and static
     settings.*/
  dp->CR2 = (dp->CR2 & ~(I2C_CR2_NBYTES | I2C_CR2_RELOAD)) | i2cp->config->cr2 |
            (n << 16U) | reload;
}

/**
 * @brief   Aborts an I2C transaction.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_abort_operation(I2CDriver *i2cp) {
  I2C_TypeDef *dp = i2cp->i2c;

  if (dp->CR1 & I2C_CR1_PE) {
    /* Stops the I2C peripheral.*/
    dp->CR1 &= ~I2C_CR1_PE;
    while (dp->CR1 & I2C_CR1_PE)
      dp->CR1 &= ~I2C_CR1_PE;
    dp->CR1 |= I2C_CR1_PE;
  }

#if STM32_I2C_USE_DMA == TRUE
  /* Stops the associated DMA streams.*/
  i2c_lld_stop_rx_dma(i2cp);
  i2c_lld_stop_tx_dma(i2cp);
#else
  dp->CR1 &= ~(I2C_CR1_TXIE | I2C_CR1_RXIE);
#endif
}

/**
 * @brief   I2C shared ISR code.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] isr       content of the ISR register to be decoded
 *
 * @notapi
 */
static void i2c_lld_serve_interrupt(I2CDriver *i2cp, uint32_t isr) {
  I2C_TypeDef *dp = i2cp->i2c;

  /* Special case of a received NACK, the transfer is aborted.*/
  if ((isr & I2C_ISR_NACKF) != 0U) {
#if STM32_I2C_USE_DMA == TRUE
    /* Stops the associated DMA streams.*/
    i2c_lld_stop_rx_dma(i2cp);
    i2c_lld_stop_tx_dma(i2cp);
#endif

    /* Error flag.*/
    i2cp->errors |= I2C_ACK_FAILURE;

    /* Transaction finished sending the STOP.*/
    dp->CR2 |= I2C_CR2_STOP;

    /* Make sure no more interrupts.*/
    dp->CR1 &= ~(I2C_CR1_TCIE | I2C_CR1_TXIE | I2C_CR1_RXIE);

    /* Errors are signaled to the upper layer.*/
    _i2c_wakeup_error_isr(i2cp);

    return;
  }

#if STM32_I2C_USE_DMA == FALSE
  /* Handling of data transfer if the DMA mode is disabled.*/
  {
    uint32_t cr1 = dp->CR1;

    if (i2cp->state == I2C_ACTIVE_TX) {
      /* Transmission phase.*/
      if (((cr1 &I2C_CR1_TXIE) != 0U) && ((isr & I2C_ISR_TXIS) != 0U)) {
        dp->TXDR = (uint32_t)*i2cp->txptr;
        i2cp->txptr++;
        i2cp->txbytes--;
        if (i2cp->txbytes == 0U) {
          dp->CR1 &= ~I2C_CR1_TXIE;
        }
      }
    }
    else {
      /* Receive phase.*/
      if (((cr1 & I2C_CR1_RXIE) != 0U) && ((isr & I2C_ISR_RXNE) != 0U)) {
        *i2cp->rxptr = (uint8_t)dp->RXDR;
        i2cp->rxptr++;
        i2cp->rxbytes--;
        if (i2cp->rxbytes == 0U) {
          dp->CR1 &= ~I2C_CR1_RXIE;
        }
      }
    }
  }
#endif

  /* Partial transfer handling, restarting the transfer and returning.*/
  if ((isr & I2C_ISR_TCR) != 0U) {
    if (i2cp->state == I2C_ACTIVE_TX) {
      i2c_lld_setup_tx_transfer(i2cp);
    }
    else {
      i2c_lld_setup_rx_transfer(i2cp);
    }
    return;
  }

  /* The following condition is true if a transfer phase has been completed.*/
  if ((isr & I2C_ISR_TC) != 0U) {
    if (i2cp->state == I2C_ACTIVE_TX) {
      /* End of the transmit phase.*/

#if STM32_I2C_USE_DMA == TRUE
      /* Disabling TX DMA channel.*/
      i2c_lld_stop_tx_dma(i2cp);
#endif

      /* Starting receive phase if necessary.*/
      if (i2cp->rxbytes > 0U) {
        /* Setting up the peripheral.*/
        i2c_lld_setup_rx_transfer(i2cp);

#if STM32_I2C_USE_DMA == TRUE
        /* Enabling RX DMA.*/
        i2c_lld_start_rx_dma(i2cp);
#else
        /* RX interrupt enabled.*/
        dp->CR1 |= I2C_CR1_RXIE;
#endif

        /* Starts the read operation.*/
        dp->CR2 |= I2C_CR2_START;

        /* State change.*/
        i2cp->state = I2C_ACTIVE_RX;

        /* Note, returning because the transaction is not over yet.*/
        return;
      }
    }
    else {
      /* End of the receive phase.*/
#if STM32_I2C_USE_DMA == TRUE
      /* Disabling RX DMA channel.*/
      i2c_lld_stop_rx_dma(i2cp);
#endif
    }

    /* Transaction finished sending the STOP.*/
    dp->CR2 |= I2C_CR2_STOP;

    /* Make sure no more 'Transfer Complete' interrupts.*/
    dp->CR1 &= ~I2C_CR1_TCIE;

    /* Normal transaction end.*/
    _i2c_wakeup_isr(i2cp);
  }
}

/**
 * @brief   I2C error handler.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] isr       content of the ISR register to be decoded
 *
 * @notapi
 */
static void i2c_lld_serve_error_interrupt(I2CDriver *i2cp, uint32_t isr) {

#if STM32_I2C_USE_DMA == TRUE
  /* Clears DMA interrupt flags just to be safe.*/
  i2c_lld_stop_rx_dma(i2cp);
  i2c_lld_stop_tx_dma(i2cp);
#else
  /* Disabling RX and TX interrupts.*/
  i2cp->i2c->CR1 &= ~(I2C_CR1_TXIE | I2C_CR1_RXIE);
#endif

  if (isr & I2C_ISR_BERR)
    i2cp->errors |= I2C_BUS_ERROR;

  if (isr & I2C_ISR_ARLO)
    i2cp->errors |= I2C_ARBITRATION_LOST;

  if (isr & I2C_ISR_OVR)
    i2cp->errors |= I2C_OVERRUN;

  if (isr & I2C_ISR_TIMEOUT)
    i2cp->errors |= I2C_TIMEOUT;

  /* If some error has been identified then sends wakes the waiting thread.*/
  if (i2cp->errors != I2C_NO_ERROR)
    _i2c_wakeup_error_isr(i2cp);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if STM32_I2C_USE_I2C1 || defined(__DOXYGEN__)
#if defined(STM32_I2C1_GLOBAL_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   I2C1 event interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(STM32_I2C1_GLOBAL_HANDLER) {
  uint32_t isr = I2CD1.i2c->ISR;

  OSAL_IRQ_PROLOGUE();

  /* Clearing IRQ bits.*/
  I2CD1.i2c->ICR = isr;

  if (isr & I2C_ERROR_MASK)
    i2c_lld_serve_error_interrupt(&I2CD1, isr);
  else if (isr & I2C_INT_MASK)
    i2c_lld_serve_interrupt(&I2CD1, isr);

  OSAL_IRQ_EPILOGUE();
}

#elif defined(STM32_I2C1_EVENT_HANDLER) && defined(STM32_I2C1_ERROR_HANDLER)
OSAL_IRQ_HANDLER(STM32_I2C1_EVENT_HANDLER) {
  uint32_t isr = I2CD1.i2c->ISR;

  OSAL_IRQ_PROLOGUE();

  /* Clearing IRQ bits.*/
  I2CD1.i2c->ICR = isr & I2C_INT_MASK;

  i2c_lld_serve_interrupt(&I2CD1, isr);

  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(STM32_I2C1_ERROR_HANDLER) {
  uint32_t isr = I2CD1.i2c->ISR;

  OSAL_IRQ_PROLOGUE();

  /* Clearing IRQ bits.*/
  I2CD1.i2c->ICR = isr & I2C_ERROR_MASK;

  i2c_lld_serve_error_interrupt(&I2CD1, isr);

  OSAL_IRQ_EPILOGUE();
}

#else
#error "I2C1 interrupt handlers not defined"
#endif
#endif /* STM32_I2C_USE_I2C1 */

#if STM32_I2C_USE_I2C2 || defined(__DOXYGEN__)
#if defined(STM32_I2C2_GLOBAL_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   I2C2 event interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(STM32_I2C2_GLOBAL_HANDLER) {
  uint32_t isr = I2CD2.i2c->ISR;

  OSAL_IRQ_PROLOGUE();

  /* Clearing IRQ bits.*/
  I2CD2.i2c->ICR = isr;

  if (isr & I2C_ERROR_MASK)
    i2c_lld_serve_error_interrupt(&I2CD2, isr);
  else if (isr & I2C_INT_MASK)
    i2c_lld_serve_interrupt(&I2CD2, isr);

  OSAL_IRQ_EPILOGUE();
}

#elif defined(STM32_I2C2_EVENT_HANDLER) && defined(STM32_I2C2_ERROR_HANDLER)
OSAL_IRQ_HANDLER(STM32_I2C2_EVENT_HANDLER) {
  uint32_t isr = I2CD2.i2c->ISR;

  OSAL_IRQ_PROLOGUE();

  /* Clearing IRQ bits.*/
  I2CD2.i2c->ICR = isr & I2C_INT_MASK;

  i2c_lld_serve_interrupt(&I2CD2, isr);

  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(STM32_I2C2_ERROR_HANDLER) {
  uint32_t isr = I2CD2.i2c->ISR;

  OSAL_IRQ_PROLOGUE();

  /* Clearing IRQ bits.*/
  I2CD2.i2c->ICR = isr & I2C_ERROR_MASK;

  i2c_lld_serve_error_interrupt(&I2CD2, isr);

  OSAL_IRQ_EPILOGUE();
}

#else
#error "I2C2 interrupt handlers not defined"
#endif
#endif /* STM32_I2C_USE_I2C2 */

#if STM32_I2C_USE_I2C3 || defined(__DOXYGEN__)
#if defined(STM32_I2C3_GLOBAL_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   I2C3 event interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(STM32_I2C3_GLOBAL_HANDLER) {
  uint32_t isr = I2CD3.i2c->ISR;

  OSAL_IRQ_PROLOGUE();

  /* Clearing IRQ bits.*/
  I2CD3.i2c->ICR = isr;

  if (isr & I2C_ERROR_MASK)
    i2c_lld_serve_error_interrupt(&I2CD3, isr);
  else if (isr & I2C_INT_MASK)
    i2c_lld_serve_interrupt(&I2CD3, isr);

  OSAL_IRQ_EPILOGUE();
}

#elif defined(STM32_I2C3_EVENT_HANDLER) && defined(STM32_I2C3_ERROR_HANDLER)
OSAL_IRQ_HANDLER(STM32_I2C3_EVENT_HANDLER) {
  uint32_t isr = I2CD3.i2c->ISR;

  OSAL_IRQ_PROLOGUE();

  /* Clearing IRQ bits.*/
  I2CD3.i2c->ICR = isr & I2C_INT_MASK;

  i2c_lld_serve_interrupt(&I2CD3, isr);

  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(STM32_I2C3_ERROR_HANDLER) {
  uint32_t isr = I2CD3.i2c->ISR;

  OSAL_IRQ_PROLOGUE();

  /* Clearing IRQ bits.*/
  I2CD3.i2c->ICR = isr & I2C_ERROR_MASK;

  i2c_lld_serve_error_interrupt(&I2CD3, isr);

  OSAL_IRQ_EPILOGUE();
}

#else
#error "I2C3 interrupt handlers not defined"
#endif
#endif /* STM32_I2C_USE_I2C3 */

#if STM32_I2C_USE_I2C4 || defined(__DOXYGEN__)
#if defined(STM32_I2C4_GLOBAL_HANDLER) || defined(__DOXYGEN__)
/**
 * @brief   I2C4 event interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(STM32_I2C4_GLOBAL_HANDLER) {
  uint32_t isr = I2CD4.i2c->ISR;

  OSAL_IRQ_PROLOGUE();

  /* Clearing IRQ bits.*/
  I2CD4.i2c->ICR = isr;

  if (isr & I2C_ERROR_MASK)
    i2c_lld_serve_error_interrupt(&I2CD4, isr);
  else if (isr & I2C_INT_MASK)
    i2c_lld_serve_interrupt(&I2CD4, isr);

  OSAL_IRQ_EPILOGUE();
}

#elif defined(STM32_I2C4_EVENT_HANDLER) && defined(STM32_I2C4_ERROR_HANDLER)
OSAL_IRQ_HANDLER(STM32_I2C4_EVENT_HANDLER) {
  uint32_t isr = I2CD4.i2c->ISR;

  OSAL_IRQ_PROLOGUE();

  /* Clearing IRQ bits.*/
  I2CD4.i2c->ICR = isr & I2C_INT_MASK;

  i2c_lld_serve_interrupt(&I2CD4, isr);

  OSAL_IRQ_EPILOGUE();
}

OSAL_IRQ_HANDLER(STM32_I2C4_ERROR_HANDLER) {
  uint32_t isr = I2CD4.i2c->ISR;

  OSAL_IRQ_PROLOGUE();

  /* Clearing IRQ bits.*/
  I2CD4.i2c->ICR = isr & I2C_ERROR_MASK;

  i2c_lld_serve_error_interrupt(&I2CD4, isr);

  OSAL_IRQ_EPILOGUE();
}

#else
#error "I2C4 interrupt handlers not defined"
#endif
#endif /* STM32_I2C_USE_I2C4 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level I2C driver initialization.
 *
 * @notapi
 */
void i2c_lld_init(void) {

#if STM32_I2C_USE_I2C1
  i2cObjectInit(&I2CD1);
  I2CD1.thread  = NULL;
  I2CD1.i2c     = I2C1;
#if STM32_I2C_USE_DMA == TRUE
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  I2CD1.is_bdma = false;
#endif
  I2CD1.rx.dma  = NULL;
  I2CD1.tx.dma  = NULL;
#endif
#if defined(STM32_I2C1_GLOBAL_NUMBER) || defined(__DOXYGEN__)
      nvicEnableVector(STM32_I2C1_GLOBAL_NUMBER, STM32_I2C_I2C1_IRQ_PRIORITY);
#elif defined(STM32_I2C1_EVENT_NUMBER) && defined(STM32_I2C1_ERROR_NUMBER)
      nvicEnableVector(STM32_I2C1_EVENT_NUMBER, STM32_I2C_I2C1_IRQ_PRIORITY);
      nvicEnableVector(STM32_I2C1_ERROR_NUMBER, STM32_I2C_I2C1_IRQ_PRIORITY);
#else
#error "I2C1 interrupt numbers not defined"
#endif
#endif /* STM32_I2C_USE_I2C1 */

#if STM32_I2C_USE_I2C2
  i2cObjectInit(&I2CD2);
  I2CD2.thread  = NULL;
  I2CD2.i2c     = I2C2;
#if STM32_I2C_USE_DMA == TRUE
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  I2CD2.is_bdma = false;
#endif
  I2CD2.rx.dma  = NULL;
  I2CD2.tx.dma  = NULL;
#endif
#if defined(STM32_I2C2_GLOBAL_NUMBER) || defined(__DOXYGEN__)
      nvicEnableVector(STM32_I2C2_GLOBAL_NUMBER, STM32_I2C_I2C2_IRQ_PRIORITY);
#elif defined(STM32_I2C2_EVENT_NUMBER) && defined(STM32_I2C2_ERROR_NUMBER)
      nvicEnableVector(STM32_I2C2_EVENT_NUMBER, STM32_I2C_I2C2_IRQ_PRIORITY);
      nvicEnableVector(STM32_I2C2_ERROR_NUMBER, STM32_I2C_I2C2_IRQ_PRIORITY);
#else
#error "I2C2 interrupt numbers not defined"
#endif
#endif /* STM32_I2C_USE_I2C2 */

#if STM32_I2C_USE_I2C3
  i2cObjectInit(&I2CD3);
  I2CD3.thread  = NULL;
  I2CD3.i2c     = I2C3;
#if STM32_I2C_USE_DMA == TRUE
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  I2CD3.is_bdma = false;
#endif
  I2CD3.rx.dma  = NULL;
  I2CD3.tx.dma  = NULL;
#endif
#if defined(STM32_I2C3_GLOBAL_NUMBER) || defined(__DOXYGEN__)
      nvicEnableVector(STM32_I2C3_GLOBAL_NUMBER, STM32_I2C_I2C3_IRQ_PRIORITY);
#elif defined(STM32_I2C3_EVENT_NUMBER) && defined(STM32_I2C3_ERROR_NUMBER)
      nvicEnableVector(STM32_I2C3_EVENT_NUMBER, STM32_I2C_I2C3_IRQ_PRIORITY);
      nvicEnableVector(STM32_I2C3_ERROR_NUMBER, STM32_I2C_I2C3_IRQ_PRIORITY);
#else
#error "I2C3 interrupt numbers not defined"
#endif
#endif /* STM32_I2C_USE_I2C3 */

#if STM32_I2C_USE_I2C4
  i2cObjectInit(&I2CD4);
  I2CD4.thread  = NULL;
  I2CD4.i2c     = I2C4;
#if STM32_I2C_USE_DMA == TRUE
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  I2CD4.is_bdma = true;
#endif
  I2CD4.rx.bdma = NULL;
  I2CD4.tx.bdma = NULL;
#endif
#if defined(STM32_I2C4_GLOBAL_NUMBER) || defined(__DOXYGEN__)
      nvicEnableVector(STM32_I2C4_GLOBAL_NUMBER, STM32_I2C_I2C4_IRQ_PRIORITY);
#elif defined(STM32_I2C4_EVENT_NUMBER) && defined(STM32_I2C4_ERROR_NUMBER)
      nvicEnableVector(STM32_I2C4_EVENT_NUMBER, STM32_I2C_I2C4_IRQ_PRIORITY);
      nvicEnableVector(STM32_I2C4_ERROR_NUMBER, STM32_I2C_I2C4_IRQ_PRIORITY);
#else
#error "I2C4 interrupt numbers not defined"
#endif
#endif /* STM32_I2C_USE_I2C4 */
}

/**
 * @brief   Configures and activates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
void i2c_lld_start(I2CDriver *i2cp) {
  I2C_TypeDef *dp = i2cp->i2c;

  /* Make sure I2C peripheral is disabled */
  dp->CR1 &= ~I2C_CR1_PE;

  /* If in stopped state then enables the I2C and DMA clocks.*/
  if (i2cp->state == I2C_STOP) {

#if STM32_I2C_USE_DMA == TRUE
    /* Common DMA modes.*/
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  if(i2cp->is_bdma)
#endif
#if defined(STM32_I2C_BDMA_REQUIRED)
  {
    i2cp->txdmamode = BDMAMODE_COMMON | STM32_DMA_CR_DIR_M2P;
    i2cp->rxdmamode = BDMAMODE_COMMON | STM32_DMA_CR_DIR_P2M;
  }
#endif
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  else
#endif
#if defined(STM32_I2C_DMA_REQUIRED)
  {
    i2cp->txdmamode = DMAMODE_COMMON | STM32_DMA_CR_DIR_M2P;
    i2cp->rxdmamode = DMAMODE_COMMON | STM32_DMA_CR_DIR_P2M;
  }
#endif
#endif /* STM32_I2C_USE_DMA == TRUE */

#if STM32_I2C_USE_I2C1
    if (&I2CD1 == i2cp) {

      rccResetI2C1();
      rccEnableI2C1(true);
#if STM32_I2C_USE_DMA == TRUE
      {
        i2cp->rx.dma = dmaStreamAllocI(STM32_I2C_I2C1_RX_DMA_STREAM,
                                       STM32_I2C_I2C1_IRQ_PRIORITY,
                                       NULL,
                                       NULL);
        osalDbgAssert(i2cp->rx.dma != NULL, "unable to allocate stream");
        i2cp->tx.dma = dmaStreamAllocI(STM32_I2C_I2C1_TX_DMA_STREAM,
                                       STM32_I2C_I2C1_IRQ_PRIORITY,
                                       NULL,
                                       NULL);
        osalDbgAssert(i2cp->tx.dma != NULL, "unable to allocate stream");

        i2cp->rxdmamode |= STM32_DMA_CR_PL(STM32_I2C_I2C1_DMA_PRIORITY);
        i2cp->txdmamode |= STM32_DMA_CR_PL(STM32_I2C_I2C1_DMA_PRIORITY);
        dmaSetRequestSource(i2cp->rx.dma, STM32_DMAMUX1_I2C1_RX);
        dmaSetRequestSource(i2cp->tx.dma, STM32_DMAMUX1_I2C1_TX);
      }
#endif /* STM32_I2C_USE_DMA == TRUE */
    }
#endif /* STM32_I2C_USE_I2C1 */

#if STM32_I2C_USE_I2C2
    if (&I2CD2 == i2cp) {

      rccResetI2C2();
      rccEnableI2C2(true);
#if STM32_I2C_USE_DMA == TRUE
      {
        i2cp->rx.dma = dmaStreamAllocI(STM32_I2C_I2C2_RX_DMA_STREAM,
                                       STM32_I2C_I2C2_IRQ_PRIORITY,
                                       NULL,
                                       NULL);
        osalDbgAssert(i2cp->rx.dma != NULL, "unable to allocate stream");
        i2cp->tx.dma = dmaStreamAllocI(STM32_I2C_I2C2_TX_DMA_STREAM,
                                       STM32_I2C_I2C2_IRQ_PRIORITY,
                                       NULL,
                                       NULL);
        osalDbgAssert(i2cp->tx.dma != NULL, "unable to allocate stream");

        i2cp->rxdmamode |= STM32_DMA_CR_PL(STM32_I2C_I2C2_DMA_PRIORITY);
        i2cp->txdmamode |= STM32_DMA_CR_PL(STM32_I2C_I2C2_DMA_PRIORITY);
        dmaSetRequestSource(i2cp->rx.dma, STM32_DMAMUX1_I2C2_RX);
        dmaSetRequestSource(i2cp->tx.dma, STM32_DMAMUX1_I2C2_TX);
      }
#endif /*STM32_I2C_USE_DMA == TRUE */
    }
#endif /* STM32_I2C_USE_I2C2 */

#if STM32_I2C_USE_I2C3
    if (&I2CD3 == i2cp) {

      rccResetI2C3();
      rccEnableI2C3(true);
#if STM32_I2C_USE_DMA == TRUE
      {
        i2cp->rx.dma = dmaStreamAllocI(STM32_I2C_I2C3_RX_DMA_STREAM,
                                       STM32_I2C_I2C3_IRQ_PRIORITY,
                                       NULL,
                                       NULL);
        osalDbgAssert(i2cp->rx.dma != NULL, "unable to allocate stream");
        i2cp->tx.dma = dmaStreamAllocI(STM32_I2C_I2C3_TX_DMA_STREAM,
                                       STM32_I2C_I2C3_IRQ_PRIORITY,
                                       NULL,
                                       NULL);
        osalDbgAssert(i2cp->tx.dma != NULL, "unable to allocate stream");

        i2cp->rxdmamode |= STM32_DMA_CR_PL(STM32_I2C_I2C3_DMA_PRIORITY);
        i2cp->txdmamode |= STM32_DMA_CR_PL(STM32_I2C_I2C3_DMA_PRIORITY);
        dmaSetRequestSource(i2cp->rx.dma, STM32_DMAMUX1_I2C3_RX);
        dmaSetRequestSource(i2cp->tx.dma, STM32_DMAMUX1_I2C3_TX);
      }
#endif /*STM32_I2C_USE_DMA == TRUE */
    }
#endif /* STM32_I2C_USE_I2C3 */

#if STM32_I2C_USE_I2C4
    if (&I2CD4 == i2cp) {

      rccResetI2C4();
      rccEnableI2C4(true);
#if STM32_I2C_USE_DMA == TRUE
      {
        i2cp->rx.bdma = bdmaStreamAllocI(STM32_I2C_I2C4_RX_BDMA_STREAM,
                                         STM32_I2C_I2C4_IRQ_PRIORITY,
                                         NULL,
                                         NULL);
        osalDbgAssert(i2cp->rx.bdma != NULL, "unable to allocate stream");
        i2cp->tx.bdma = bdmaStreamAllocI(STM32_I2C_I2C4_TX_BDMA_STREAM,
                                         STM32_I2C_I2C4_IRQ_PRIORITY,
                                         NULL,
                                         NULL);
        osalDbgAssert(i2cp->tx.bdma != NULL, "unable to allocate stream");

        i2cp->rxdmamode |= STM32_BDMA_CR_PL(STM32_I2C_I2C4_DMA_PRIORITY);
        i2cp->txdmamode |= STM32_BDMA_CR_PL(STM32_I2C_I2C4_DMA_PRIORITY);
        bdmaSetRequestSource(i2cp->rx.bdma, STM32_DMAMUX2_I2C4_RX);
        bdmaSetRequestSource(i2cp->tx.bdma, STM32_DMAMUX2_I2C4_TX);
      }
#endif /*STM32_I2C_USE_DMA == TRUE */
    }
#endif /* STM32_I2C_USE_I2C4 */
  }

#if STM32_I2C_USE_DMA == TRUE
  /* I2C registers pointed by the DMA.*/
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  if(i2cp->is_bdma)
#endif
#if defined(STM32_I2C_BDMA_REQUIRED)
  {
    bdmaStreamSetPeripheral(i2cp->rx.bdma, &dp->RXDR);
    bdmaStreamSetPeripheral(i2cp->tx.bdma, &dp->TXDR);
  }
#endif
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  else
#endif
#if defined(STM32_I2C_DMA_REQUIRED)
  {
    dmaStreamSetPeripheral(i2cp->rx.dma, &dp->RXDR);
    dmaStreamSetPeripheral(i2cp->tx.dma, &dp->TXDR);
  }
#endif
#endif /* STM32_I2C_USE_DMA == TRUE */

  /* Reset i2c peripheral, the TCIE bit will be handled separately.*/
  dp->CR1 = i2cp->config->cr1 |
#if STM32_I2C_USE_DMA == TRUE
            I2C_CR1_TXDMAEN | I2C_CR1_RXDMAEN | /* Enable only if using DMA */
#endif
            I2C_CR1_ERRIE | I2C_CR1_NACKIE;

  /* Setup I2C parameters.*/
  dp->TIMINGR = i2cp->config->timingr;

  /* Ready to go.*/
  dp->CR1 |= I2C_CR1_PE;
}

/**
 * @brief   Deactivates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
void i2c_lld_stop(I2CDriver *i2cp) {

  /* If not in stopped state then disables the I2C clock.*/
  if (i2cp->state != I2C_STOP) {

    /* I2C disable.*/
    i2c_lld_abort_operation(i2cp);

#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
    if(i2cp->is_bdma)
#endif
#if defined(STM32_I2C_BDMA_REQUIRED)
    {
      bdmaStreamFreeI(i2cp->rx.bdma);
      bdmaStreamFreeI(i2cp->tx.bdma);
    }
#endif
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
    else
#endif
#if defined(STM32_I2C_DMA_REQUIRED)
    {
      dmaStreamFreeI(i2cp->rx.dma);
      dmaStreamFreeI(i2cp->tx.dma);
    }
#endif

#if STM32_I2C_USE_I2C1
    if (&I2CD1 == i2cp) {
      rccDisableI2C1();
    }
#endif

#if STM32_I2C_USE_I2C2
    if (&I2CD2 == i2cp) {
      rccDisableI2C2();
    }
#endif

#if STM32_I2C_USE_I2C3
    if (&I2CD3 == i2cp) {
      rccDisableI2C3();
    }
#endif

#if STM32_I2C_USE_I2C4
    if (&I2CD4 == i2cp) {
      rccDisableI2C4();
    }
#endif
  }
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
                                     sysinterval_t timeout) {
  msg_t msg;
  I2C_TypeDef *dp = i2cp->i2c;
  systime_t start, end;

  /* Resetting error flags for this transfer.*/
  i2cp->errors = I2C_NO_ERROR;

  /* Releases the lock from high level driver.*/
  osalSysUnlock();

  /* Sizes of transfer phases.*/
  i2cp->txbytes = 0U;
  i2cp->rxbytes = rxbytes;

#if STM32_I2C_USE_DMA == TRUE
  /* RX DMA setup.*/
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  if(i2cp->is_bdma)
#endif
#if defined(STM32_I2C_BDMA_REQUIRED)
  {
    bdmaStreamSetMode(i2cp->rx.bdma, i2cp->rxdmamode);
    bdmaStreamSetMemory(i2cp->rx.bdma, rxbuf);
    bdmaStreamSetTransactionSize(i2cp->rx.bdma, rxbytes);
  }
#endif
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  else
#endif
#if defined(STM32_I2C_DMA_REQUIRED)
  {
    dmaStreamSetMode(i2cp->rx.dma, i2cp->rxdmamode);
    dmaStreamSetMemory0(i2cp->rx.dma, rxbuf);
    dmaStreamSetTransactionSize(i2cp->rx.dma, rxbytes);
  }
#endif
#else
  i2cp->rxptr = rxbuf;
#endif

  /* Calculating the time window for the timeout on the busy bus condition.*/
  start = osalOsGetSystemTimeX();
  end = osalTimeAddX(start, OSAL_MS2I(STM32_I2C_BUSY_TIMEOUT));

  /* Waits until BUSY flag is reset or, alternatively, for a timeout
     condition.*/
  while (true) {
    osalSysLock();

    /* If the bus is not busy then the operation can continue, note, the
       loop is exited in the locked state.*/
    if ((dp->ISR & I2C_ISR_BUSY) == 0)
      break;

    /* If the system time went outside the allowed window then a timeout
       condition is returned.*/
    if (!osalTimeIsInRangeX(osalOsGetSystemTimeX(), start, end)) {
      return MSG_TIMEOUT;
    }

    osalSysUnlock();
  }

  /* Setting up the slave address.*/
  i2c_lld_set_address(i2cp, addr);

  /* Setting up the peripheral.*/
  i2c_lld_setup_rx_transfer(i2cp);

#if STM32_I2C_USE_DMA == TRUE
  /* Enabling RX DMA.*/
  i2c_lld_start_rx_dma(i2cp);

  /* Transfer complete interrupt enabled.*/
  dp->CR1 |= I2C_CR1_TCIE;
#else

  /* Transfer complete and RX interrupts enabled.*/
  dp->CR1 |= I2C_CR1_TCIE | I2C_CR1_RXIE;
#endif

  /* Starts the operation.*/
  dp->CR2 |= I2C_CR2_START;

  /* Waits for the operation completion or a timeout.*/
  msg = osalThreadSuspendTimeoutS(&i2cp->thread, timeout);

  /* In case of a software timeout a STOP is sent as an extreme attempt
     to release the bus.*/
  if (msg == MSG_TIMEOUT) {
    dp->CR2 |= I2C_CR2_STOP;
  }

  return msg;
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
                                      sysinterval_t timeout) {
  msg_t msg;
  I2C_TypeDef *dp = i2cp->i2c;
  systime_t start, end;

  /* Resetting error flags for this transfer.*/
  i2cp->errors = I2C_NO_ERROR;

  /* Releases the lock from high level driver.*/
  osalSysUnlock();

  /* Sizes of transfer phases.*/
  i2cp->txbytes = txbytes;
  i2cp->rxbytes = rxbytes;

#if STM32_I2C_USE_DMA == TRUE
  /* TX and RX DMA setup.*/
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  if(i2cp->is_bdma)
#endif
#if defined(STM32_I2C_BDMA_REQUIRED)
  {
    bdmaStreamSetMode(i2cp->tx.bdma, i2cp->txdmamode);
    bdmaStreamSetMemory(i2cp->tx.bdma, txbuf);
    bdmaStreamSetTransactionSize(i2cp->tx.bdma, txbytes);

    bdmaStreamSetMode(i2cp->rx.bdma, i2cp->rxdmamode);
    bdmaStreamSetMemory(i2cp->rx.bdma, rxbuf);
    bdmaStreamSetTransactionSize(i2cp->rx.bdma, rxbytes);
  }
#endif
#if defined(STM32_I2C_DMA_REQUIRED) && defined(STM32_I2C_BDMA_REQUIRED)
  else
#endif
#if defined(STM32_I2C_DMA_REQUIRED)
  {
    dmaStreamSetMode(i2cp->tx.dma, i2cp->txdmamode);
    dmaStreamSetMemory0(i2cp->tx.dma, txbuf);
    dmaStreamSetTransactionSize(i2cp->tx.dma, txbytes);

    dmaStreamSetMode(i2cp->rx.dma, i2cp->rxdmamode);
    dmaStreamSetMemory0(i2cp->rx.dma, rxbuf);
    dmaStreamSetTransactionSize(i2cp->rx.dma, rxbytes);
  }
#endif
#else
  i2cp->txptr = txbuf;
  i2cp->rxptr = rxbuf;
#endif

  /* Calculating the time window for the timeout on the busy bus condition.*/
  start = osalOsGetSystemTimeX();
  end = osalTimeAddX(start, OSAL_MS2I(STM32_I2C_BUSY_TIMEOUT));

  /* Waits until BUSY flag is reset or, alternatively, for a timeout
     condition.*/
  while (true) {
    osalSysLock();

    /* If the bus is not busy then the operation can continue, note, the
       loop is exited in the locked state.*/
    if ((dp->ISR & I2C_ISR_BUSY) == 0)
      break;

    /* If the system time went outside the allowed window then a timeout
       condition is returned.*/
    if (!osalTimeIsInRangeX(osalOsGetSystemTimeX(), start, end)) {
      return MSG_TIMEOUT;
    }

    osalSysUnlock();
  }

  /* Setting up the slave address.*/
  i2c_lld_set_address(i2cp, addr);

  /* Preparing the transfer.*/
  i2c_lld_setup_tx_transfer(i2cp);

#if STM32_I2C_USE_DMA == TRUE
  /* Enabling TX DMA.*/
  i2c_lld_start_tx_dma(i2cp);

  /* Transfer complete interrupt enabled.*/
  dp->CR1 |= I2C_CR1_TCIE;
#else
  /* Transfer complete and TX interrupts enabled.*/
  dp->CR1 |= I2C_CR1_TCIE | I2C_CR1_TXIE;
#endif

  /* Starts the operation.*/
  dp->CR2 |= I2C_CR2_START;

  /* Waits for the operation completion or a timeout.*/
  msg = osalThreadSuspendTimeoutS(&i2cp->thread, timeout);

  /* In case of a software timeout a STOP is sent as an extreme attempt
     to release the bus.*/
  if (msg == MSG_TIMEOUT) {
    dp->CR2 |= I2C_CR2_STOP;
  }

  return msg;
}

#endif /* HAL_USE_I2C */

/** @} */
