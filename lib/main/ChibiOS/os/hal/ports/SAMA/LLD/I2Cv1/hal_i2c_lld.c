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
 * @file    I2Cv1/hal_i2c_lld.c
 * @brief   SAMA I2C subsystem low level driver source.
 *
 * @addtogroup I2C
 * @{
 */

#include "hal.h"

#if HAL_USE_I2C || defined(__DOXYGEN__)

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/* SAMA5D2 Clock offset. */
#define TWI_CLK_OFFSET                      3

/* Mask for 10-bit address. */
#define TWI_ADDR_MASK_10                    0x380

/* Mask for 10-bit address case for DADR field. */
#define TWI_ADDR_10_DADR_MASK               0x78
#define TWI_ADDR_10_IADR_MASK               0xFF

/* Mask for internal address check. */
#define TWI_INTERNAL_ADDR_MASK              0xFF

/* Mask for TWI errors interrupt. */
#define TWI_ERROR_MASK                      (TWI_SR_OVRE | TWI_SR_UNRE |      \
                                             TWI_SR_NACK | TWI_SR_ARBLST |    \
                                             TWI_SR_TOUT | TWI_SR_PECERR)

/*===========================================================================*/
/* Driver local macros.                                                      */
/*===========================================================================*/
#if !defined ROUND_INT_DIV
#define ROUND_INT_DIV(n,d)                  (((n) + ((d)-1)) / (d))
#endif

/**
 * @brief   Enable write protection on TWI registers block.
 *
 * @param[in] i2cp   pointer to a TWI register block
 *
 * @notapi
 */
#define twiEnableWP(i2cp) {                                                   \
  i2cp->TWI_WPMR = TWI_WPMR_WPKEY_PASSWD | TWI_WPMR_WPEN;                     \
}

/**
 * @brief   Disable write protection on TWI registers block.
 *
 * @param[in] i2cp    pointer to a TWI register block
 *
 * @notapi
 */
#define twiDisableWP(i2cp) {                                                  \
  i2cp->TWI_WPMR = TWI_WPMR_WPKEY_PASSWD;                                     \
}

/*===========================================================================*/
/* Driver constants.                                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/** @brief I2C0 driver identifier.*/
#if SAMA_I2C_USE_TWIHS0 || defined(__DOXYGEN__)
I2CDriver I2CD0;
#endif

/** @brief I2C1 driver identifier.*/
#if SAMA_I2C_USE_TWIHS1 || defined(__DOXYGEN__)
I2CDriver I2CD1;
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/**
 * @brief   Read a byte from TWI_RHR register.
 *
 * @param[in] reg       pointer to the target register
 * @param[in] value     pointer to the transmit variable
 *
 * @notapi
 */
static inline void i2c_lld_read_byte(volatile const void* reg, uint8_t* value) {
  *value = *(volatile const uint8_t*)reg;
}

/**
 * @brief   Write a byte to TWI_THR register.
 *
 * @param[in] reg       pointer to the target register
 * @param[in] value     pointer to the receive variable
 *
 * @notapi
 */
static inline void i2c_lld_write_byte(volatile void* reg, uint8_t value) {
  *(volatile uint8_t*)reg = value;
}

/**
 * @brief   Read bytes.
 * @note    Disables WRITE PROTECTION before using
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_read_bytes(I2CDriver *i2cp) {

  if (i2cp->rxbytes == 1) {

    /* Starts and stops the operation for 1 byte read.*/
    i2cp->i2c->TWI_CR = TWI_CR_START | TWI_CR_STOP;
    i2cp->i2c->TWI_IER = TWI_IER_TXCOMP;
    while ((i2cp->i2c->TWI_SR & TWI_SR_RXRDY) == 0);

    i2c_lld_read_byte(&i2cp->i2c->TWI_RHR, i2cp->rxbuf);
  }

  if (i2cp->rxbytes == 2) {

    /* Starts the operation.*/
    i2cp->i2c->TWI_CR = TWI_CR_START;
    i2cp->i2c->TWI_IER = TWI_IER_TXCOMP;
    while ((i2cp->i2c->TWI_SR & TWI_SR_RXRDY) == 0);

    /* Stops the operation and read penultimate byte. */
    i2cp->i2c->TWI_CR = TWI_CR_STOP;

    i2c_lld_read_byte(&i2cp->i2c->TWI_RHR, &i2cp->rxbuf[0]);

    while ((i2cp->i2c->TWI_SR & TWI_SR_RXRDY) == 0);

    /* Read last byte. */
    i2c_lld_read_byte(&i2cp->i2c->TWI_RHR, &i2cp->rxbuf[1]);
  }

  if (i2cp->rxbytes > 2) {

    /* RX DMA setup.*/
    dmaChannelSetDestination(i2cp->dmarx, i2cp->rxbuf);
    dmaChannelSetTransactionSize(i2cp->dmarx, i2cp->rxbytes -2);

    /* Starts the operation.*/
    i2cp->i2c->TWI_CR = TWI_CR_START;

    /* Start the DMA. */
    dmaChannelEnable(i2cp->dmarx);
  }
}

/**
 * @brief   Write bytes.
 * @note    Disables WRITE PROTECTION before using
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_write_bytes(I2CDriver *i2cp) {

  size_t txsize = i2cp->txbytes;

  if (txsize == 1) {

    i2c_lld_write_byte(&i2cp->i2c->TWI_THR, i2cp->txbuf[0]);

    /* Enable TXCOMP interrupt. */
    i2cp->i2c->TWI_IER = TWI_IER_TXCOMP;

    i2cp->i2c->TWI_CR = TWI_CR_STOP;

    /* Starts and stops the operation for 1 byte write.*/
    while ((i2cp->i2c->TWI_SR & TWI_SR_TXRDY) == 0);
  }

  if (txsize > 1) {
    /* RX DMA setup.*/
    dmaChannelSetSource(i2cp->dmatx, i2cp->txbuf);
    dmaChannelSetTransactionSize(i2cp->dmatx, (txsize - 1));
    dmaChannelEnable(i2cp->dmatx);
  }
}

/**
 * @brief   Set operation mode of I2C hardware.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_set_opmode(I2CDriver *i2cp) {

  switch (i2cp->config->op_mode) {
  case OPMODE_I2C:
    i2cp->i2c->TWI_CR = TWI_CR_SMBDIS;
    break;
  case OPMODE_SMBUS:
    i2cp->i2c->TWI_CR = TWI_CR_SMBEN;
    break;
  }
}

/**
 * @brief   DMA RX end IRQ handler.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 * @param[in] flags     pre-shifted content of the ISR register
 *
 * @notapi
 */
static void i2c_lld_serve_rx_interrupt(I2CDriver *i2cp, uint32_t flags) {

  /* DMA errors handling.*/
#if defined(SAMA_I2C_DMA_ERROR_HOOK)
  if ((flags & (XDMAC_CIS_RBEIS | XDMAC_CIS_ROIS)) != 0) {
    SAMA_I2C_DMA_ERROR_HOOK(i2cp);
  }
#else
  (void)flags;
#endif

  /* DMA channel disable. */
  dmaChannelDisable(i2cp->dmarx);

  /* Cache is enabled */
  cacheInvalidateRegion(i2cp->rxbuf, i2cp->rxbytes - 1);

  /* Wait for RXRDY flag. */
  while ((i2cp->i2c->TWI_SR & TWI_SR_RXRDY) == 0);

  /* Stops the operation and read the last 2 bytes.*/
  i2cp->i2c->TWI_CR = TWI_CR_STOP;
  i2c_lld_read_byte(&i2cp->i2c->TWI_RHR, &i2cp->rxbuf[i2cp->rxbytes - 2]);

  /* Wait for the last byte. */
  while ((i2cp->i2c->TWI_SR & TWI_SR_RXRDY) == 0);

  /* Enable TXCOMP interrupt. */
  i2cp->i2c->TWI_IER = TWI_IER_TXCOMP;

  /* Read the last byte. */
  i2c_lld_read_byte(&i2cp->i2c->TWI_RHR, &i2cp->rxbuf[i2cp->rxbytes - 1]);

}


/**
 * @brief    DMA TX end IRQ handler.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_serve_tx_interrupt(I2CDriver *i2cp, uint32_t flags) {

  const uint8_t tx_last_byte = i2cp->txbuf[i2cp->txbytes - 1];
  /* DMA errors handling.*/
#if defined(SAMA_I2C_DMA_ERROR_HOOK)
  if ((flags & (XDMAC_CIS_WBEIS | XDMAC_CIS_ROIS)) != 0) {
    SAMA_I2C_DMA_ERROR_HOOK(i2cp);
  }
#else
  (void)flags;
#endif

  dmaChannelDisable(i2cp->dmatx);

  /* Wait for the TX ready flag. */
  while ((i2cp->i2c->TWI_SR & TWI_SR_TXRDY) == 0);

  /* Stops the operation and transmit the last byte.*/
  i2cp->i2c->TWI_CR = TWI_CR_STOP;

  i2cp->i2c->TWI_IER = TWI_IER_TXCOMP;

  i2c_lld_write_byte(&i2cp->i2c->TWI_THR, tx_last_byte);
}

/**
 * @brief   I2C interrupts handler.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
static void i2c_lld_serve_interrupt(I2CDriver *i2cp, uint32_t flags) {

  /* Used only in 1/2 bytes transmissions in order to wake up the thread. */
  if (flags & TWI_SR_TXCOMP) {
    _i2c_wakeup_isr(i2cp);
  }

  i2cp->errors = I2C_NO_ERROR;

  if (flags & TWI_SR_OVRE)                             /* Overrun error. */
    i2cp->errors |= I2C_OVERRUN;

  if (flags & TWI_SR_UNRE)                             /* Underrun error. */
    i2cp->errors |= I2C_OVERRUN;

  if (flags & TWI_SR_NACK) {                           /* Acknowledge fail. */
    i2cp->i2c->TWI_CR = TWI_CR_STOP;                   /* Setting stop bit. */
    i2cp->errors |= I2C_ACK_FAILURE;
  }

  if (flags & TWI_SR_ARBLST)                           /* Arbitration lost. */
    i2cp->errors |= I2C_ARBITRATION_LOST;

  if (flags & TWI_SR_TOUT)                             /* SMBus Timeout. */
    i2cp->errors |= I2C_TIMEOUT;

  if (flags & TWI_SR_PECERR)                           /* PEC error. */
    i2cp->errors |= I2C_PEC_ERROR;

  /* If some error has been identified then sends wakes the waiting thread.*/
  if (i2cp->errors != I2C_NO_ERROR)
    _i2c_wakeup_error_isr(i2cp);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if SAMA_I2C_USE_TWIHS0 || defined(__DOXYGEN__)
/**
 * @brief   TWIHS0 interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(SAMA_TWIHS0_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t sr = I2CD0.i2c->TWI_SR;

  I2CD0.i2c->TWI_IDR = TWI_IDR_TXCOMP;

  i2c_lld_serve_interrupt(&I2CD0, sr);
  aicAckInt();

  OSAL_IRQ_EPILOGUE();
}
#endif /* SAMA_I2C_USE_TWIHS0 */

#if SAMA_I2C_USE_TWIHS1 || defined(__DOXYGEN__)
/**
 * @brief   TWIHS1 interrupt handler.
 *
 * @notapi
 */
OSAL_IRQ_HANDLER(SAMA_TWIHS1_HANDLER) {

  OSAL_IRQ_PROLOGUE();

  uint32_t sr = I2CD1.i2c->TWI_SR;

  I2CD1.i2c->TWI_IDR = TWI_IDR_TXCOMP;

  i2c_lld_serve_interrupt(&I2CD1, sr);
  aicAckInt();

  OSAL_IRQ_EPILOGUE();
}

#endif /* SAMA_I2C_USE_TWIHS1 */

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level I2C driver initialization.
 *
 * @notapi
 */
void i2c_lld_init(void) {

#if SAMA_I2C_USE_TWIHS0
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_TWIHS0, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  /* Driver initialization.*/
  i2cObjectInit(&I2CD0);
  I2CD0.thread    = NULL;
  I2CD0.i2c       = TWIHS0;
  I2CD0.dmarx     = NULL;
  I2CD0.dmatx     = NULL;
  I2CD0.rxdmamode = XDMAC_CC_TYPE_PER_TRAN |
                    XDMAC_CC_MBSIZE_SIXTEEN |
                    XDMAC_CC_DSYNC_PER2MEM |
                    XDMAC_CC_PROT_SEC |
                    XDMAC_CC_CSIZE_CHK_1 |
                    XDMAC_CC_DWIDTH_BYTE |
                    XDMAC_CC_SIF_AHB_IF1 |
                    XDMAC_CC_DIF_AHB_IF0 |
                    XDMAC_CC_SAM_FIXED_AM |
                    XDMAC_CC_DAM_INCREMENTED_AM |
                    XDMAC_CC_PERID(PERID_TWIHS0_RX);
  I2CD0.txdmamode = XDMAC_CC_TYPE_PER_TRAN |
                    XDMAC_CC_MBSIZE_SIXTEEN |
                    XDMAC_CC_DSYNC_MEM2PER |
                    XDMAC_CC_PROT_SEC |
                    XDMAC_CC_CSIZE_CHK_1 |
                    XDMAC_CC_DWIDTH_BYTE |
                    XDMAC_CC_SIF_AHB_IF0 |
                    XDMAC_CC_DIF_AHB_IF1 |
                    XDMAC_CC_SAM_INCREMENTED_AM |
                    XDMAC_CC_DAM_FIXED_AM |
                    XDMAC_CC_PERID(PERID_TWIHS0_TX);
#endif /* SAMA_I2C_USE_TWIHS0 */

#if SAMA_I2C_USE_TWIHS1
#if SAMA_HAL_IS_SECURE
  mtxConfigPeriphSecurity(MATRIX1, ID_TWIHS1, SECURE_PER);
#endif /* SAMA_HAL_IS_SECURE */
  /* Driver initialization.*/
  i2cObjectInit(&I2CD1);
  I2CD1.thread    = NULL;
  I2CD1.i2c       = TWIHS1;
  I2CD1.dmarx     = NULL;
  I2CD1.dmatx     = NULL;
  I2CD1.rxdmamode = XDMAC_CC_TYPE_PER_TRAN |
                    XDMAC_CC_MBSIZE_SIXTEEN |
                    XDMAC_CC_DSYNC_PER2MEM |
                    XDMAC_CC_PROT_SEC |
                    XDMAC_CC_CSIZE_CHK_1 |
                    XDMAC_CC_DWIDTH_BYTE |
                    XDMAC_CC_SIF_AHB_IF1 |
                    XDMAC_CC_DIF_AHB_IF0 |
                    XDMAC_CC_SAM_FIXED_AM |
                    XDMAC_CC_DAM_INCREMENTED_AM |
                    XDMAC_CC_PERID(PERID_TWIHS1_RX);
  I2CD1.txdmamode = XDMAC_CC_TYPE_PER_TRAN |
                    XDMAC_CC_MBSIZE_SIXTEEN |
                    XDMAC_CC_DSYNC_MEM2PER |
                    XDMAC_CC_PROT_SEC |
                    XDMAC_CC_CSIZE_CHK_1 |
                    XDMAC_CC_DWIDTH_BYTE |
                    XDMAC_CC_SIF_AHB_IF0 |
                    XDMAC_CC_DIF_AHB_IF1 |
                    XDMAC_CC_SAM_INCREMENTED_AM |
                    XDMAC_CC_DAM_FIXED_AM |
                    XDMAC_CC_PERID(PERID_TWIHS1_TX);
#endif /* SAMA_I2C_USE_TWIHS1 */
}

/**
 * @brief   Configures and activates the I2C peripheral.
 *
 * @param[in] i2cp      pointer to the @p I2CDriver object
 *
 * @notapi
 */
void i2c_lld_start(I2CDriver *i2cp) {

  uint32_t ck_div, clh_div, hold;

  /* If in stopped state then enables the I2C and DMA clocks.*/
  if (i2cp->state == I2C_STOP) {

#if SAMA_I2C_USE_TWIHS0
    if (&I2CD0 == i2cp) {

      i2cp->dmarx = dmaChannelAllocate(SAMA_I2C_TWIHS0_DMA_IRQ_PRIORITY,
                                       (sama_dmaisr_t)i2c_lld_serve_rx_interrupt,
                                       (void *)i2cp);
      osalDbgAssert(i2cp->dmarx != NULL, "no channel allocated");

      i2cp->dmatx = dmaChannelAllocate(SAMA_I2C_TWIHS0_DMA_IRQ_PRIORITY,
                                       (sama_dmaisr_t)i2c_lld_serve_tx_interrupt,
                                       (void *)i2cp);
      osalDbgAssert(i2cp->dmatx != NULL, "no channel allocated");

      pmcEnableTWIHS0();
      /* To prevent spurious interrupt */
      aicSetIntSourceType(ID_TWIHS0, EXT_NEGATIVE_EDGE);
      aicSetSourcePriority(ID_TWIHS0, SAMA_I2C_TWIHS0_IRQ_PRIORITY);
      aicSetSourceHandler(ID_TWIHS0, SAMA_TWIHS0_HANDLER);
      aicEnableInt(ID_TWIHS0);
    }
#endif /* SAMA_I2C_USE_TWIHS0 */

#if SAMA_I2C_USE_TWIHS1
    if (&I2CD1 == i2cp) {

      i2cp->dmarx = dmaChannelAllocate(SAMA_I2C_TWIHS1_DMA_IRQ_PRIORITY,
                                       (sama_dmaisr_t)i2c_lld_serve_rx_interrupt,
                                       (void *)i2cp);
      osalDbgAssert(i2cp->dmarx != NULL, "no channel allocated");

      i2cp->dmatx = dmaChannelAllocate(SAMA_I2C_TWIHS1_DMA_IRQ_PRIORITY,
                                       (sama_dmaisr_t)i2c_lld_serve_tx_interrupt,
                                       (void *)i2cp);
      osalDbgAssert(i2cp->dmatx != NULL, "no channel allocated");

      pmcEnableTWIHS1();
      /* To prevent spurious interrupt */
      aicSetIntSourceType(ID_TWIHS1, EXT_NEGATIVE_EDGE);
      aicSetSourcePriority(ID_TWIHS1, SAMA_I2C_TWIHS1_IRQ_PRIORITY);
      aicSetSourceHandler(ID_TWIHS1, SAMA_TWIHS1_HANDLER);
      aicEnableInt(ID_TWIHS1);
    }
#endif /* SAMA_I2C_USE_TWIHS1 */
  }

  /* Set mode */
  dmaChannelSetMode(i2cp->dmarx, i2cp->rxdmamode);
  dmaChannelSetSource(i2cp->dmarx, &i2cp->i2c->TWI_RHR);

  dmaChannelSetMode(i2cp->dmatx, i2cp->txdmamode);
  dmaChannelSetDestination(i2cp->dmatx, &i2cp->i2c->TWI_THR);

  /* Disable write protection. */
  twiDisableWP(i2cp->i2c);

  /* TWI software reset */
  i2cp->i2c->TWI_CR = TWI_CR_SWRST;

  /* TWI set operation mode. */
  i2c_lld_set_opmode(i2cp);

  /* Configure dummy slave address */
  i2cp->i2c->TWI_MMR = 0;

  /* Compute clock */
  for (ck_div = 0; ck_div < 7; ck_div++) {
    clh_div = ((SAMA_TWIHSxCLK / i2cp->config->clock_speed) - 2 * (TWI_CLK_OFFSET)) >> ck_div;
    if (clh_div <= 511) {
      break;
    }
  }

  /* Compute holding time (I2C spec requires 300ns) */
  hold = TWI_CWGR_HOLD(ROUND_INT_DIV((uint32_t)(0.3 * SAMA_TWIHSxCLK), 1000000) - 3);

  /* Configure clock */
  i2cp->i2c->TWI_CWGR = TWI_CWGR_CKDIV(ck_div) |
                        TWI_CWGR_CHDIV(clh_div >> 1) |
                        TWI_CWGR_CLDIV(clh_div >> 1) |
                        hold;

  /* Clear status flag */
  i2cp->i2c->TWI_SR;

  /* Enable Interrupt. */
  i2cp->i2c->TWI_IER = TWI_ERROR_MASK;

  /* Set master mode */
  i2cp->i2c->TWI_CR = TWI_CR_SVDIS;
  i2cp->i2c->TWI_CR = TWI_CR_MSEN;

  /* Enable write protection. */
  twiEnableWP(i2cp->i2c);
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

    /* Disable write protection. */
    twiDisableWP(i2cp->i2c);

    /* I2C disable.*/

    /* Disable interrupts. */
    i2cp->i2c->TWI_IDR = TWI_ERROR_MASK;

    /* TWI software reset. */
    i2cp->i2c->TWI_CR = TWI_CR_SWRST;
    i2cp->i2c->TWI_MMR = 0;

    /* DMA channel release. */
    dmaChannelRelease(i2cp->dmatx);
    dmaChannelRelease(i2cp->dmarx);

#if SAMA_I2C_USE_TWIHS0
    if (&I2CD0 == i2cp) {
      pmcDisableTWIHS0();
    }
#endif

#if SAMA_I2C_USE_TWIHS1
    if (&I2CD1 == i2cp) {
      pmcDisableTWIHS1();
    }
#endif

    /* Enable write protection. */
    twiEnableWP(i2cp->i2c);
  }

  i2cp->txbuf = NULL;
  i2cp->rxbuf = NULL;
  i2cp->txbytes = 0;
  i2cp->rxbytes = 0;
}

/**
 * @brief   Receives data via the I2C bus as master.
 * @details
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

  osalDbgAssert(!((uint32_t) rxbuf & (L1_CACHE_BYTES - 1)), "rxbuf address not cache aligned");

#if 0
  osalDbgAssert(!(rxbytes & (L1_CACHE_BYTES - 1)), "size not multiple of cache line");
#endif

  i2cp->txbuf = NULL;
  i2cp->rxbuf = rxbuf;
  i2cp->txbytes = 0;
  i2cp->rxbytes = rxbytes;

  /*
   * If size is not multiple of cache line, clean cache region is required.
   * TODO: remove when size assert works
   */
  if (rxbytes & (L1_CACHE_BYTES - 1)) {
    cacheCleanRegion((uint8_t *) rxbuf, rxbytes);
  }

  systime_t start, end;

  /* Resetting error flags for this transfer.*/
  i2cp->errors = I2C_NO_ERROR;

  /* Disable write protection. */
  twiDisableWP(i2cp->i2c);

  /* Compute device address and/or internal address. */
  i2cp->i2c->TWI_MMR = 0;

  if ((addr & TWI_ADDR_MASK_10) != 0) {
    /* 10-bit address. */
    if (i2cp->config->op_mode == OPMODE_I2C) {

      /* Store 2 slave device address MSB bits in MMR_DADR with 11110 mask. Configure number of internal slave address bytes in MMR_IADRSZ as 1. */
      i2cp->i2c->TWI_MMR = TWI_MMR_DADR((addr >> 8) | TWI_ADDR_10_DADR_MASK) |
                           TWI_MMR_IADRSZ_1_BYTE | TWI_MMR_MREAD;
      i2cp->i2c->TWI_IADR = TWI_ADDR_10_IADR_MASK & addr;

    } else if (i2cp->config->op_mode == OPMODE_SMBUS)
      osalDbgAssert((addr & TWI_ADDR_MASK_10) != 0, "10-bit address not supported in SMBus mode");

  } else {
    /* 7-bit address. */
    /* Store slave device address in MMR_DADR. */
    i2cp->i2c->TWI_MMR |= TWI_MMR_DADR(addr);

    /* Configure read direction. */
    i2cp->i2c->TWI_MMR |= TWI_MMR_MREAD;
  }
  /* Releases the lock from high level driver.*/
  osalSysUnlock();

  /* Calculating the time window for the timeout on the busy bus condition.*/
  start = osalOsGetSystemTimeX();
  end = osalTimeAddX(start, OSAL_MS2I(SAMA_I2C_BUSY_TIMEOUT));

  /* Waits until BUSY flag is reset or, alternatively, for a timeout
     condition.*/
  while (true) {
    osalSysLock();

    /* If the bus is not busy then the operation can continue, note, the
       loop is exited in the locked state.*/
    if (i2cp->i2c->TWI_SR & (TWI_SR_TXCOMP | TWI_SR_RXRDY))
      break;

    /* If the system time went outside the allowed window then a timeout
       condition is returned.*/
    if (!osalTimeIsInRangeX(osalOsGetSystemTimeX(), start, end))
      return MSG_TIMEOUT;

    osalSysUnlock();
  }

  i2c_lld_read_bytes(i2cp);

  /* Enable write protection. */
  twiEnableWP(i2cp->i2c);

  /* Waits for the operation completion or a timeout.*/
  return osalThreadSuspendTimeoutS(&i2cp->thread, timeout);
}

/**
 * @brief   Transmits data via the I2C bus as master.
 * @details When performing reading through write you can not write more than
 *          3 bytes of data to I2C slave. This is SAMA platform limitation.
 *          Internal address bytes must be set in txbuf from LSB (position 0 of the buffer) to MSB
 *          (position 1 or 2 of the buffer depending from the number of internal address bytes.
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

  osalDbgAssert(!((uint32_t) txbuf & (L1_CACHE_BYTES - 1)), "txbuf address not cache aligned");
  osalDbgAssert(!((uint32_t) rxbuf & (L1_CACHE_BYTES - 1)), "rxbuf address not cache aligned");

#if 0
  osalDbgAssert(!(rxbytes & (L1_CACHE_BYTES - 1)), "size not multiple of cache line");
#endif

  i2cp->txbuf = txbuf;
  i2cp->rxbuf = rxbuf;
  i2cp->txbytes = txbytes;
  i2cp->rxbytes = rxbytes;

  /* Cache is enabled */
  cacheCleanRegion((uint8_t *)i2cp->txbuf, txbytes);

  /*
   * If size is not multiple of cache line, clean cache region is required.
   * TODO: remove when size assert works
   */
  if (rxbytes & (L1_CACHE_BYTES - 1)) {
    cacheCleanRegion((uint8_t *) rxbuf, rxbytes);
  }

  systime_t start, end;

  /* Resetting error flags for this transfer.*/
  i2cp->errors = I2C_NO_ERROR;

  /* Disable write protection. */
  twiDisableWP(i2cp->i2c);

  /* prepare to read through write operation */
  if (rxbytes > 0){

    osalDbgAssert(txbytes <= 3, "Number of internal address bytes not supported. Max number of internal address bytes is 3.");

    i2cp->i2c->TWI_MMR  = 0;

    /* Compute slave address and internal addresses. */

    /* Internal address of I2C slave was set in special Atmel registers.
     * Now we must call read function. The I2C cell automatically sends
     * bytes from IADR register to bus and issues repeated start. */

    if ((addr & TWI_ADDR_MASK_10) != 0) {
      /* 10-bit address. */
      if (i2cp->config->op_mode == OPMODE_I2C) {

        uint16_t mem_addr = 0;

        osalDbgAssert(txbytes <= 2, "Number of internal address bytes not supported");

        /* Store 2 slave device address MSB bits in MMR_DADR with 11110 mask. Configure number of internal slave address bytes in
         * MMR_IADRSZ as 1 + slave internal addresses. */
        i2cp->i2c->TWI_MMR = TWI_MMR_DADR((addr >> 8) | TWI_ADDR_10_DADR_MASK) |
                             TWI_MMR_IADRSZ(txbytes + 1);

        if(txbytes == 1)
          mem_addr = i2cp->txbuf[0];

        else if(txbytes == 2)
          mem_addr = i2cp->txbuf[0] | (i2cp->txbuf[1] << 8);

        /* Store the rest of the 10-bit address in IADR register. Also store the internal slave address bytes. */
        i2cp->i2c->TWI_IADR = (TWI_ADDR_10_IADR_MASK & addr) | (mem_addr << 8);

      } else if (i2cp->config->op_mode == OPMODE_SMBUS)
        osalDbgAssert((addr & TWI_ADDR_MASK_10) != 0, "10-bit address not supported in SMBus mode");

    } else {
      /* 7-bit address. */
      i2cp->i2c->TWI_MMR |= txbytes << 8;

      /* Store internal slave address in TWI_IADR registers */
      i2cp->i2c->TWI_IADR = 0;
      while (txbytes > 0){
        i2cp->i2c->TWI_IADR = (i2cp->i2c->TWI_IADR << 8);
        i2cp->i2c->TWI_IADR |= *(txbuf++);
        txbytes--;
      }
      /* Store slave device address in MMR_DADR. */
      i2cp->i2c->TWI_MMR |= TWI_MMR_DADR(addr);
    }

  #if 0
    osalDbgAssert(!(rxbytes & (L1_CACHE_BYTES - 1)), "size not multiple of cache line");
  #endif

    /* Configure read direction. */
    i2cp->i2c->TWI_MMR |= TWI_MMR_MREAD;

    /* Releases the lock from high level driver.*/
    osalSysUnlock();

    /* Calculating the time window for the timeout on the busy bus condition.*/
    start = osalOsGetSystemTimeX();
    end = osalTimeAddX(start, OSAL_MS2I(SAMA_I2C_BUSY_TIMEOUT));

    /* Waits until BUSY flag is reset or, alternatively, for a timeout
       condition.*/
    while (true) {
      osalSysLock();

      /* If the bus is not busy then the operation can continue, note, the
         loop is exited in the locked state.*/
      if (i2cp->i2c->TWI_SR & (TWI_SR_TXCOMP | TWI_SR_RXRDY))
        break;

      /* If the system time went outside the allowed window then a timeout
         condition is returned.*/
      if (!osalTimeIsInRangeX(osalOsGetSystemTimeX(), start, end))
        return MSG_TIMEOUT;

      osalSysUnlock();
    }

    i2c_lld_read_bytes(i2cp);

    /* Enable write protection. */
    twiEnableWP(i2cp->i2c);

    /* Waits for the operation completion or a timeout.*/
    return osalThreadSuspendTimeoutS(&i2cp->thread, timeout);

  } else {
    /* Compute device slave address. Internal slave address are sent as data. */

    i2cp->i2c->TWI_MMR = 0;

    if ((addr & TWI_ADDR_MASK_10) != 0) {
      /* 10-bit address. */
      if (i2cp->config->op_mode == OPMODE_I2C) {

        /* Store 2 slave device address MSB bits in MMR_DADR with 11110 mask. Configure number of internal slave address bytes in MMR_IADRSZ as 1. */
        i2cp->i2c->TWI_MMR = TWI_MMR_DADR((addr >> 8) | TWI_ADDR_10_DADR_MASK) |
                             TWI_MMR_IADRSZ_1_BYTE;
        i2cp->i2c->TWI_IADR = TWI_ADDR_10_IADR_MASK & addr;

      } else if (i2cp->config->op_mode == OPMODE_SMBUS)
        osalDbgAssert((addr & TWI_ADDR_MASK_10) != 0, "10-bit address not supported in SMBus mode");

    } else {
      /* 7-bit address. */
      /* Store slave device address in MMR_DADR. */
      i2cp->i2c->TWI_MMR |= TWI_MMR_DADR(addr);
    }

    /* Releases the lock from high level driver.*/
    osalSysUnlock();

    /* Calculating the time window for the timeout on the busy bus condition.*/
    start = osalOsGetSystemTimeX();
    end = osalTimeAddX(start, OSAL_MS2I(SAMA_I2C_BUSY_TIMEOUT));

    /* Waits until BUSY flag is reset or, alternatively, for a timeout
       condition.*/
    while (true) {
      osalSysLock();

      /* If the bus is not busy then the operation can continue, note, the
         loop is exited in the locked state.*/
      if (i2cp->i2c->TWI_SR & (TWI_SR_TXCOMP | TWI_SR_RXRDY))
        break;

      /* If the system time went outside the allowed window then a timeout
         condition is returned.*/
      if (!osalTimeIsInRangeX(osalOsGetSystemTimeX(), start, end))
        return MSG_TIMEOUT;

      osalSysUnlock();
    }

    i2c_lld_write_bytes(i2cp);

    /* Enable write protection. */
    twiEnableWP(i2cp->i2c);

    /* Waits for the operation completion or a timeout.*/
    return osalThreadSuspendTimeoutS(&i2cp->thread, timeout);
  }
}

#endif /* HAL_USE_I2C */

/** @} */
