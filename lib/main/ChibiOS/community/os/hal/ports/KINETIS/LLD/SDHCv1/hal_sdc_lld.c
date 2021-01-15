/*
    ChibiOS - Copyright (C) 2006..2016 Giovanni Di Sirio
              Copyright (C) 2017..2018 Wim Lewis

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
 * @file    SDHCv1/hal_sdc_lld.h
 * @brief   Kinetis SDC subsystem low level driver.
 *
 * This driver provides a single SDC driver based on the Kinetis
 * "Secured Digital Host Controller (SDHC)" peripheral.
 *
 * In order to use this driver, other peripherals must also be configured:
 *
 * The MPU must either be disabled (CESR=0), or it must be configured
 * to allow the SDHC peripheral DMA access to any data buffers (read
 * or write).
 *
 * The SDHC signals must be routed to the desired pins, and pullups/pulldowns
 * configured.
 *
 * @addtogroup SDC
 * @{
 */

#include "hal.h"

#if (HAL_USE_SDC == TRUE) || defined(__DOXYGEN__)

#include "hal_mmcsd.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

#if defined(MK66F18)
/* Configure SDHC block to use the IRC48M clock */
#define KINETIS_SDHC_PERIPHERAL_FREQUENCY 48000000UL
#else
/* We configure the SDHC block to use the system clock */
#define KINETIS_SDHC_PERIPHERAL_FREQUENCY KINETIS_SYSCLK_FREQUENCY
#endif

#ifndef KINETIS_SDHC_PRIORITY
#define KINETIS_SDHC_PRIORITY 12 /* TODO? Default IRQ priority for SDHC */
#endif

/* The DTOC value (data timeout counter) controls how long the SDHC
   will wait for a data transfer before indicating a timeout to
   us. The card can tell us how long that should be, but various SDHC
   documentation suggests that we should always allow around 500 msec
   even if the card says it will finish sooner. This only comes into
   play if there's a malfunction or something, so it's not critical to
   get it exactly right.

   It controls the ratio between the SDCLK frequency and the
   timeout, so we have a different DTOCV for each bus clock
   frequency.
*/
#define DTOCV_300ms_400kHz    4  /* 4  -> 2^17 -> 328 msec */
#define DTOCV_700ms_25MHz    11  /* 11 -> 2^24 -> 671 msec */
#define DTOCV_700ms_50MHz    12  /* 12 -> 2^25 -> 671 msec */

#if 0
#define TRACE(t, val)   chDbgWriteTrace ((void *)t, (void *)(uintptr_t)(val))
#define TRACEI(t, val)  chDbgWriteTraceI((void *)t, (void *)(uintptr_t)(val))
#else
#define TRACE(t, val)
#define TRACEI(t, val)
#endif

#define DIV_RND_UP(a, b) ( ((a)+(b)-1) / (b) )

/* Error bits from the SD / MMC Card Status response word. */
/* TODO: These really belong in a HLD, not here. */
#define MMC_ERR_OUT_OF_RANGE            (1U << 31)
#define MMC_ERR_ADDRESS                 (1U << 30)
#define MMC_ERR_BLOCK_LEN               (1U << 29)
#define MMC_ERR_ERASE_SEQ               (1U << 28)
#define MMC_ERR_ERASE_PARAM             (1U << 27)
#define MMC_ERR_WP                      (1U << 26)
#define MMC_ERR_CARD_IS_LOCKED          (1U << 25)
#define MMC_ERR_LOCK_UNLOCK_FAILED      (1U << 24)
#define MMC_ERR_COM_CRC_ERROR           (1U << 23)
#define MMC_ERR_ILLEGAL_COMMAND         (1U << 22)
#define MMC_ERR_CARD_ECC_FAILED         (1U << 21)
#define MMC_ERR_CARD_CONTROLLER         (1U << 20)
#define MMC_ERR_ERROR                   (1U << 19)
#define MMC_ERR_CSD_OVERWRITE           (1U << 16)
#define MMC_ERR_AKE_SEQ                 (1U << 3)

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/**
 * @brief   SDCD1 driver identifier.
 */
#if (PLATFORM_SDC_USE_SDC1 == TRUE) || defined(__DOXYGEN__)
SDCDriver SDCD1;
#else
#error HAL_USE_SDC is true but PLATFORM_SDC_USE_SDC1 is false
#endif

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

static void recover_after_botched_transfer(SDCDriver *);
static msg_t wait_interrupt(SDCDriver *, uint32_t);
static bool sdc_lld_transfer(SDCDriver *, uint32_t, uintptr_t, uint32_t, uint32_t);

/**
 * Compute the SDCLKFS and DVS values for a given SDCLK divisor.
 *
 * Note that in the current code, this function is always called with
 * a constant argument (there are only a handful of valid SDCLK
 * frequencies), and so GCC computes the results at compile time and
 * does not actually emit this function into the output at all unless
 * you're compiling with optimizations turned off.
 *
 * However if someone compiles with a KINETIS_SDHC_PERIPHERAL_FREQUENCY
 * that is not a compile-time constant, this function would get emitted.
 */
static uint32_t divisor_settings(unsigned divisor)
{
  /* First, handle all the special cases */
  if (divisor <= 1) {
    /* Pass through */
    return SDHC_SYSCTL_SDCLKFS(0) | SDHC_SYSCTL_DVS(0);
  }
  if (divisor <= 16 && (divisor & 0x01)) {
    /* Disable the prescaler, just use the divider. */
    return SDHC_SYSCTL_SDCLKFS(0) | SDHC_SYSCTL_DVS(divisor - 1);
  }
  if (divisor <= 32 && !(divisor & 0x01)) {
    /* Prescale by 2, but do the rest with the divider */
    return SDHC_SYSCTL_SDCLKFS(0x01) | SDHC_SYSCTL_DVS((divisor >> 1) - 1);
  }
  if (divisor >= 0x1000) {
    /* It's not possible to divide by more than 2^12. If we're asked to,
       just do the best we can. */
    return SDHC_SYSCTL_SDCLKFS(0x80) | SDHC_SYSCTL_DVS(0xF);
  }

  /* The bit position in SDCLKFS provides a power-of-two prescale
     factor, and the four bits in DVS allow division by up to 16
     (division by DVS+1). We want to approximate `divisor` using these
     terms, but we want to round up --- it's OK to run the card a
     little bit too slow, but not OK to run it a little bit too
     fast. */

  unsigned shift = (8 * sizeof(unsigned int) - 4) - __builtin_clz(divisor);

  /* Shift the divisor value right so that it only occupies the four
     lowest bits. Subtract one because that's how the DVS circuit
     works. Add one if we shifted any 1-bits off the bottom, so that
     we always round up. */
  unsigned dvs = (divisor >> shift) - ((divisor & ((1 << shift)-1))? 0 : 1);

  return SDHC_SYSCTL_SDCLKFS(1 << (shift-1)) | SDHC_SYSCTL_DVS(dvs);
}

/**
 * @brief Enable the SDHC clock when stable.
 *
 * Waits for the clock divider in the SDHC block to stabilize, then
 * enables the SD clock.
 */
static void enable_clock_when_stable(uint32_t new_sysctl)
{
  SDHC->SYSCTL = new_sysctl;
  
  /* Wait for clock divider to stabilize */
  while(!(SDHC->PRSSTAT & SDHC_PRSSTAT_SDSTB)) {
    osalThreadSleepMilliseconds(1);
  }

  /* Restart the clock */
  SDHC->SYSCTL = new_sysctl | SDHC_SYSCTL_SDCLKEN;

  /* Wait for clock to stabilize again */
  while(!(SDHC->PRSSTAT & SDHC_PRSSTAT_SDSTB)) {
    osalThreadSleepMilliseconds(1);
  }
}

/**
 * Translate error bits from a CMD transaction to the HAL's error flag set.
 */
static sdcflags_t translate_cmd_error(uint32_t status) {
  /* Translate the failure into the flags understood by the top half */

  sdcflags_t errors = 0;

  if (status & SDHC_IRQSTAT_CTOE || !(status & SDHC_IRQSTAT_CC)) {
    errors |= SDC_COMMAND_TIMEOUT;
  }
  if (status & SDHC_IRQSTAT_CCE) {
    errors |= SDC_CMD_CRC_ERROR;
  }

  /* If CTOE and CCE are both set, this indicates that the Kinetis
     SDHC peripheral has detected a CMD line conflict in a
     multi-master scenario. There's no specific code for that, so just
     pass it through as a combined timeout+CRC failure. */

  /* Translate any other framing and protocol errors into CRC errors. */
  if (status & ~(SDHC_IRQSTAT_CCE|SDHC_IRQSTAT_CTOE|SDHC_IRQSTAT_CC)) {
    errors |= SDC_CMD_CRC_ERROR;
  }

  return errors;
}

/**
 * Translate error bits from a card's R1 response word into the HAL's
 * error flag set.
 *
 * This function should probably be in the HLD, not here.
 */
static sdcflags_t translate_mmcsd_error(uint32_t cardstatus) {
  sdcflags_t errors = 0;

  cardstatus &= MMCSD_R1_ERROR_MASK;

  if (cardstatus & MMC_ERR_COM_CRC_ERROR)
    errors |= SDC_CMD_CRC_ERROR;

  if (cardstatus & MMC_ERR_CARD_ECC_FAILED)
    errors |= SDC_DATA_CRC_ERROR;

  /* TODO: Extend the HLD error codes at least enough to distinguish
     between invalid command/parameter errors (card is OK, but
     retrying w/o change won't help) and other errors */
  if (cardstatus & ~(MMC_ERR_COM_CRC_ERROR|MMC_ERR_CARD_ECC_FAILED))
    errors |= SDC_UNHANDLED_ERROR;

  return errors;
}

/**
 * @brief Perform one CMD transaction on the SD bus.
 */
static bool send_and_wait_cmd(SDCDriver *sdcp, uint32_t cmd) {
  /* SDCLKEN (CMD clock enabled) should be true;
   * SDSTB (clock stable) should be true;
   * CIHB (command inhibit / busy) should be false */
  osalDbgAssert((SDHC->PRSSTAT & (SDHC_PRSSTAT_SDSTB|SDHC_PRSSTAT_CIHB)) == SDHC_PRSSTAT_SDSTB, "Not in expected state");
  osalDbgAssert(SDHC->SYSCTL & SDHC_SYSCTL_SDCLKEN, "Clock disabled");
  osalDbgCheck((cmd & SDHC_XFERTYP_DPSEL) == 0);
  osalDbgCheck((SDHC->IRQSTAT & (SDHC_IRQSTAT_CIE | SDHC_IRQSTAT_CEBE | SDHC_IRQSTAT_CCE |
                                 SDHC_IRQSTAT_CTOE | SDHC_IRQSTAT_CC)) == 0);

  /* This initiates the CMD transaction */
  TRACE(1, cmd);
  SDHC->XFERTYP = cmd;

  uint32_t events =
    SDHC_IRQSTAT_CIE | SDHC_IRQSTAT_CEBE | SDHC_IRQSTAT_CCE |
    SDHC_IRQSTAT_CTOE | /* SDHC_IRQSTAT_CRM | */ SDHC_IRQSTAT_CC;
  wait_interrupt(sdcp, SDHC_IRQSTAT_CTOE | SDHC_IRQSTAT_CC);
  uint32_t status = SDHC->IRQSTAT & events;

  /* These bits are write-1-to-clear (w1c) */
  SDHC->IRQSTAT = status;

  /* In the normal case, the CC (command complete) bit is set but none
     of the others are */
  if (status == SDHC_IRQSTAT_CC)
    return HAL_SUCCESS;

  /* Translate the failure into the flags understood by the top half */
  sdcp->errors |= translate_cmd_error(status);

  TRACE(9, SDHC->PRSSTAT);

  /* Issue a reset to the CMD portion of the SDHC peripheral to clear the
     error bits and enable subsequent commands */
  SDHC->SYSCTL |= SDHC_SYSCTL_RSTC;

  return HAL_FAILED;
}

/**
 * @brief Perform one data transaction on the SD bus.
 */
static bool send_and_wait_transfer(SDCDriver *sdcp, uint32_t cmd) {

  osalDbgCheck(cmd & SDHC_XFERTYP_DPSEL);
  osalDbgCheck(cmd & SDHC_XFERTYP_DMAEN);

  const uint32_t cmd_end_bits =
    SDHC_IRQSTAT_CIE | SDHC_IRQSTAT_CEBE | SDHC_IRQSTAT_CCE |
    SDHC_IRQSTAT_CTOE | /* SDHC_IRQSTAT_CRM | */ SDHC_IRQSTAT_CC;

  const uint32_t transfer_end_bits =
    SDHC_IRQSTAT_DMAE | SDHC_IRQSTAT_AC12E | SDHC_IRQSTAT_DEBE |
    SDHC_IRQSTAT_DCE | SDHC_IRQSTAT_DTOE | SDHC_IRQSTAT_TC;

  TRACE(3, cmd);

  osalSysLock();
  osalDbgCheck(sdcp->thread == NULL);

  /* Clear anything pending from an earlier transfer */
  SDHC->IRQSTAT = cmd_end_bits | transfer_end_bits | SDHC_IRQSTAT_DINT;

  /* Enable interrupts on completions or failures */
  uint32_t old_staten = SDHC->IRQSTATEN;
  SDHC->IRQSTATEN = (old_staten & ~(SDHC_IRQSTAT_BRR|SDHC_IRQSTAT_BWR)) | (cmd_end_bits | transfer_end_bits | SDHC_IRQSTAT_DINT);
  SDHC->IRQSIGEN = SDHC_IRQSTAT_CTOE | SDHC_IRQSTAT_CC;

  /* Start the transfer */
  SDHC->XFERTYP = cmd;

  /* Await an interrupt */
  osalThreadSuspendS(&sdcp->thread);
  osalSysUnlock();

  /* Retrieve the flags and clear them */
  uint32_t cmdstat = SDHC->IRQSTAT & cmd_end_bits;
  SDHC->IRQSTAT = cmdstat;
  TRACE(2, cmdstat);

  /* If the command failed, the transfer won't happen */
  if (cmdstat != SDHC_IRQSTAT_CC) {
    /* The command couldn't be sent, or wasn't acknowledged */
    sdcp->errors |= translate_cmd_error(cmdstat);

    /* Clear the error status */
    SDHC->SYSCTL |= SDHC_SYSCTL_RSTC;

    if (cmdstat == (SDHC_IRQSTAT_CCE|SDHC_IRQSTAT_CTOE)) {
      /* A CMD-line conflict is unlikely, but doesn't require further recovery */
    } else {
      /* For most error situations, we don't know whether the command
	 failed to send or we got line noise while receiving. Make sure
	 we're in a sane state by resetting the connection. */
      recover_after_botched_transfer(sdcp);
    }

    return HAL_FAILED;
  }

  uint32_t cmdresp = SDHC->CMDRSP[0];
  TRACE(11, cmdresp);
  if (cmdresp & MMCSD_R1_ERROR_MASK) {
    /* The command was sent, and the card responded with an error indication */
    sdcp->errors |= translate_mmcsd_error(cmdresp);
    return HAL_FAILED;
  }

  /* Check for end of data transfer phase */
  uint32_t datastat;
  for (;;) {
    datastat = SDHC->IRQSTAT & (transfer_end_bits | SDHC_IRQSTAT_DINT);
    if (datastat & transfer_end_bits)
      break;
    wait_interrupt(sdcp, transfer_end_bits);
  }
  TRACE(6, datastat);
  SDHC->IRQSTAT = datastat;

  /* Handle data transfer errors */
  if ((datastat & ~(SDHC_IRQSTAT_DINT)) != SDHC_IRQSTAT_TC) {
    bool should_cancel = false;

    /* Data phase errors */
    if (datastat & (SDHC_IRQSTAT_DCE|SDHC_IRQSTAT_DEBE)) {
      sdcp->errors |= SDC_DATA_CRC_ERROR;
      should_cancel = true;
    }
    if (datastat & SDHC_IRQSTAT_DTOE) {
      sdcp->errors |= SDC_DATA_TIMEOUT;
      should_cancel = true;
    }

    /* Internal DMA error */
    if (datastat & SDHC_IRQSTAT_DMAE) {
      sdcp->errors |= SDC_UNHANDLED_ERROR;
      if (!(datastat & SDHC_IRQSTAT_TC))
	should_cancel = true;
    }

    if (datastat & SDHC_IRQSTAT_AC12E) {
      uint32_t cmd12error = SDHC->AC12ERR;

      /* We don't know if CMD12 was successfully executed */
      should_cancel = true;

      if (cmd12error & SDHC_AC12ERR_AC12NE) {
	sdcp->errors |= SDC_UNHANDLED_ERROR;
      } else {
	if (cmd12error & SDHC_AC12ERR_AC12TOE)
	  sdcp->errors |= SDC_COMMAND_TIMEOUT;
	if (cmd12error & (SDHC_AC12ERR_AC12CE|SDHC_AC12ERR_AC12EBE))
	  sdcp->errors |= SDC_CMD_CRC_ERROR;
      }
    }

    if (should_cancel) {
      recover_after_botched_transfer(sdcp);
    }

    return HAL_FAILED;
  }

  /* For a read transfer, make sure the DMA has finished transferring
   * to host memory. (For a write transfer, the DMA necessarily finishes
   * before the transfer does, so we don't need to wait for it
   * specially.) */
  if (!(datastat & SDHC_IRQSTAT_DINT)) {
    for(;;) {
      datastat = SDHC->IRQSTAT & (SDHC_IRQSTAT_DINT|SDHC_IRQSTAT_DMAE);
      if (datastat) {
	SDHC->IRQSTAT = datastat;
	TRACE(7, datastat);
	break;
      }
      /* ...?? */
    }
  }

  SDHC->IRQSTATEN = old_staten;

  return HAL_SUCCESS;
}

/**
 * @brief Wait for an interrupt from the SDHC peripheral.
 *
 * @param[in] mask    Bits to enable in IRQSIGEN.
 *
 * @return            MSG_OK
 */
static msg_t wait_interrupt(SDCDriver *sdcp, uint32_t mask) {
  osalSysLock();
  SDHC->IRQSIGEN = mask;
  msg_t wakeup = osalThreadSuspendS(&sdcp->thread);
  osalSysUnlock();
  return wakeup;
}

static void recover_after_botched_transfer(SDCDriver *sdcp) {

  /* Query the card state */
  uint32_t cardstatus;
  if (sdc_lld_send_cmd_short_crc(sdcp,
                                 MMCSD_CMD_SEND_STATUS,
                                 sdcp->rca, &cardstatus) == HAL_SUCCESS) {
    sdcp->errors |= translate_mmcsd_error(cardstatus);
    uint32_t state = MMCSD_R1_STS(cardstatus);
    if (state == MMCSD_STS_DATA) {

      /* Send a CMD12 to make sure the card isn't still transferring anything */
      SDHC->CMDARG = 0;
      send_and_wait_cmd(sdcp,
                        SDHC_XFERTYP_CMDINX(MMCSD_CMD_STOP_TRANSMISSION) |
                        SDHC_XFERTYP_CMDTYP_ABORT |
                        /* TODO: Should we set CICEN and CCCEN here? */
                        SDHC_XFERTYP_CICEN | SDHC_XFERTYP_CCCEN |
                        SDHC_XFERTYP_RSPTYP_48b);
    }
  }

  /* And reset the data block of the SDHC peripheral */
  SDHC->SYSCTL |= SDHC_SYSCTL_RSTD;
}

/**
 * @brief Perform one data transfer command
 *
 * Sends a command to the card and waits for the corresponding data transfer
 * (either a read or write) to complete.
 */
static bool sdc_lld_transfer(SDCDriver *sdcp, uint32_t startblk,
			     uintptr_t buf, uint32_t n,
			     uint32_t cmdx) {

  osalDbgCheck(n > 0);
  osalDbgCheck((buf & 0x03) == 0);  /* Must be 32-bit aligned */

  osalDbgAssert((SDHC->PRSSTAT & (SDHC_PRSSTAT_DLA|SDHC_PRSSTAT_CDIHB|SDHC_PRSSTAT_CIHB)) == 0,
		"SDHC interface not ready");

  /* We always operate in terms of 512-byte blocks; the upper-layer
     driver doesn't change the block size. The SDHC spec suggests that
     only low-capacity cards support block sizes other than 512 bytes
     anyway (SDHC "Physical Layer Simplified Specification" ver 6.0) */

  if (sdcp->cardmode & SDC_MODE_HIGH_CAPACITY) {
    SDHC->CMDARG = startblk;
  } else {
    SDHC->CMDARG = startblk * MMCSD_BLOCK_SIZE;
  }

  /* Store the DMA start address */
  SDHC->DSADDR = buf;

  uint32_t xfer;
  /* For data transfers, we need to set some extra bits in XFERTYP according to the
     transfer we're starting:
     DPSEL -> enable data transfer
     DTDSEL -> 1 for a read (card-to-host) transfer
     MSBSEL, BCEN -> multiple block transfer using BLKATTR_BLKCNT
     AC12EN -> Automatically issue MMCSD_CMD_STOP_TRANSMISSION at end of transfer

     Setting BLKCOUNT to 1 seems to be necessary even if MSBSEL+BCEN
     is not set, despite the datasheet suggesting otherwise. I'm not
     sure if this is a silicon bug or if I'm misunderstanding the
     datasheet.
  */
  SDHC->BLKATTR =
    SDHC_BLKATTR_BLKCNT(n) |
    SDHC_BLKATTR_BLKSIZE(MMCSD_BLOCK_SIZE);
  if (n == 1) {
    xfer =
      cmdx |
      SDHC_XFERTYP_CMDTYP_NORMAL |
      SDHC_XFERTYP_CICEN | SDHC_XFERTYP_CCCEN |
      SDHC_XFERTYP_RSPTYP_48b |
      SDHC_XFERTYP_DPSEL | SDHC_XFERTYP_DMAEN;
  } else {
    xfer =
      cmdx |
      SDHC_XFERTYP_CMDTYP_NORMAL |
      SDHC_XFERTYP_CICEN | SDHC_XFERTYP_CCCEN |
      SDHC_XFERTYP_RSPTYP_48b |
      SDHC_XFERTYP_MSBSEL | SDHC_XFERTYP_BCEN | SDHC_XFERTYP_AC12EN |
      SDHC_XFERTYP_DPSEL | SDHC_XFERTYP_DMAEN;
  }

  return send_and_wait_transfer(sdcp, xfer);
}

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

#if (PLATFORM_SDC_USE_SDC1 == TRUE) || defined(__DOXYGEN__)
OSAL_IRQ_HANDLER(KINETIS_SDHC_IRQ_VECTOR) {
  OSAL_IRQ_PROLOGUE();
  osalSysLockFromISR();

  TRACEI(4, SDHC->IRQSTAT);

  /* We disable the interrupts, and wake up the usermode task to read
   * the flags from IRQSTAT.
   */
  SDHC->IRQSIGEN = 0;

  osalThreadResumeI(&SDCD1.thread, MSG_OK);

  osalSysUnlockFromISR();
  OSAL_IRQ_EPILOGUE();
}
#endif

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level SDC driver initialization.
 *
 * @notapi
 */
void sdc_lld_init(void) {
#if PLATFORM_SDC_USE_SDC1 == TRUE
  sdcObjectInit(&SDCD1);
#endif
}


/**
 * @brief   Configures and activates the SDC peripheral.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_start(SDCDriver *sdcp) {

  if (sdcp->state == BLK_STOP) {
#if defined(MK66F18)
    /* Use IRC48M clock for SDHC */
    SIM->SOPT2 |= SIM_SOPT2_SDHCSRC(1);
    SIM->SOPT2 |= SIM_SOPT2_PLLFLLSEL_SET(3);
#else
    SIM->SOPT2 =
      (SIM->SOPT2 & ~SIM_SOPT2_SDHCSRC_MASK) |
      SIM_SOPT2_SDHCSRC(0);  /* SDHC clock source 0: Core/system clock. */
#endif
    SIM->SCGC3 |= SIM_SCGC3_SDHC; /* Enable clock to SDHC peripheral */

    /* Reset the SDHC block */
    SDHC->SYSCTL |= SDHC_SYSCTL_RSTA;
    while(SDHC->SYSCTL & SDHC_SYSCTL_RSTA) {
      osalThreadSleepMilliseconds(1);
    }

    SDHC->IRQSIGEN = 0;
    nvicEnableVector(SDHC_IRQn, KINETIS_SDHC_PRIORITY);
  }
}

/**
 * @brief   Deactivates the SDC peripheral.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_stop(SDCDriver *sdcp) {

  if (sdcp->state != BLK_STOP) {
    /* TODO: Should we perform a reset (RSTA) before putting the
       peripheral to sleep? */

    /* Disable the card clock */
    SDHC->SYSCTL &= ~( SDHC_SYSCTL_SDCLKEN );

    /* Turn off interrupts */
    nvicDisableVector(SDHC_IRQn);
    SDHC->IRQSIGEN = 0;
    SDHC->IRQSTATEN &= ~( SDHC_IRQSTATEN_CINTSEN |
			  SDHC_IRQSTATEN_CINSEN |
			  SDHC_IRQSTATEN_CRMSEN );

    /* Disable the clock to the SDHC peripheral block */
    SIM->SCGC3 &= ~( SIM_SCGC3_SDHC );
  }
}

/**
 * @brief   Starts the SDIO clock and sets it to init mode (400kHz or less).
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_start_clk(SDCDriver *sdcp) {

  (void)sdcp;

  /* Stop the card clock (it should already be stopped) */
  SDHC->SYSCTL &= ~( SDHC_SYSCTL_SDCLKEN );

  /* Change the divisor and DTOCV for a 400kHz card closk */
  uint32_t sysctl =
    SDHC_SYSCTL_DTOCV(DTOCV_300ms_400kHz) |
    divisor_settings(DIV_RND_UP(KINETIS_SDHC_PERIPHERAL_FREQUENCY, 400000));

  /* Restart the clock */
  enable_clock_when_stable(sysctl);

  /* Reset any protocol machinery; this also runs the clock for 80
     cycles without any data bits to help initalize the card's state
     (the Kinetis peripheral docs say that this is required after card
     insertion or power-on, but the abridged SDHC specifications I
     have don't seem to mention it) */
  SDHC->SYSCTL |= SDHC_SYSCTL_INITA;

  TRACE(9, SDHC->PRSSTAT);
}

/**
 * @brief   Sets the SDIO clock to data mode (25MHz or less).
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] clk       the clock mode
 *
 * @notapi
 */
void sdc_lld_set_data_clk(SDCDriver *sdcp, sdcbusclk_t clk) {

  (void)sdcp;

  /* Stop the card clock */
  SDHC->SYSCTL &= ~( SDHC_SYSCTL_SDCLKEN );

  /* Change the divisor */
  uint32_t ctl;
  switch (clk) {
  default:
  case SDC_CLK_25MHz:
    ctl =
      SDHC_SYSCTL_DTOCV(DTOCV_700ms_25MHz) |
      divisor_settings(DIV_RND_UP(KINETIS_SDHC_PERIPHERAL_FREQUENCY, 25000000));
    break;
  case SDC_CLK_50MHz:
    ctl =
      SDHC_SYSCTL_DTOCV(DTOCV_700ms_50MHz) |
      divisor_settings(DIV_RND_UP(KINETIS_SDHC_PERIPHERAL_FREQUENCY, 50000000));
    break;
  }

  /* Restart the clock */
  enable_clock_when_stable(ctl);
}

/**
 * @brief   Stops the SDIO clock.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @notapi
 */
void sdc_lld_stop_clk(SDCDriver *sdcp) {
  (void)sdcp;
  SDHC->SYSCTL &= ~( SDHC_SYSCTL_SDCLKEN );
}

/**
 * @brief   Switches the bus to 4 bit or 8 bit mode.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] mode      bus mode
 *
 * @notapi
 */
void sdc_lld_set_bus_mode(SDCDriver *sdcp, sdcbusmode_t mode) {
  (void)sdcp;
  uint32_t proctl = SDHC->PROCTL & ~( SDHC_PROCTL_DTW_MASK );

  switch (mode) {
  case SDC_MODE_1BIT:
    proctl |= SDHC_PROCTL_DTW_1BIT;
    break;
  case SDC_MODE_4BIT:
    proctl |= SDHC_PROCTL_DTW_4BIT;
    break;
  case SDC_MODE_8BIT:
    proctl |= SDHC_PROCTL_DTW_8BIT;
    break;
  default:
    osalDbgAssert(false, "invalid bus mode");
    break;
  }

  SDHC->PROCTL = proctl;
}

/**
 * @brief   Sends an SDIO command with no response expected.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] cmd       card command
 * @param[in] arg       command argument
 *
 * @notapi
 */
void sdc_lld_send_cmd_none(SDCDriver *sdcp, uint8_t cmd, uint32_t arg) {
  SDHC->CMDARG = arg;
  uint32_t xfer =
    SDHC_XFERTYP_CMDINX(cmd) |
    SDHC_XFERTYP_CMDTYP_NORMAL |
    /* DPSEL=0, CICEN=0, CCCEN=0 */
    SDHC_XFERTYP_RSPTYP_NONE;

  send_and_wait_cmd(sdcp, xfer);
}

/**
 * @brief   Sends an SDIO command with a short response expected.
 * @note    The CRC is not verified.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] cmd       card command
 * @param[in] arg       command argument
 * @param[out] resp     pointer to the response buffer (one word)
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */
bool sdc_lld_send_cmd_short(SDCDriver *sdcp, uint8_t cmd, uint32_t arg,
                            uint32_t *resp) {
  SDHC->CMDARG = arg;
  uint32_t xfer =
    SDHC_XFERTYP_CMDINX(cmd) |
    SDHC_XFERTYP_CMDTYP_NORMAL |
    /* DPSEL=0, CICEN=0, CCCEN=0 */
    SDHC_XFERTYP_RSPTYP_48;

  bool waited = send_and_wait_cmd(sdcp, xfer);

  *resp = SDHC->CMDRSP[0];

  return waited;
}

/**
 * @brief   Sends an SDIO command with a short response expected and CRC.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] cmd       card command
 * @param[in] arg       command argument
 * @param[out] resp     pointer to the response buffer (one word)
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */
bool sdc_lld_send_cmd_short_crc(SDCDriver *sdcp, uint8_t cmd, uint32_t arg,
                                uint32_t *resp) {
  SDHC->CMDARG = arg;
  uint32_t xfer =
    SDHC_XFERTYP_CMDINX(cmd) |
    SDHC_XFERTYP_CMDTYP_NORMAL |
    /* DPSEL=0, CICEN=1, CCCEN=1 */
    SDHC_XFERTYP_CICEN | SDHC_XFERTYP_CCCEN |
    SDHC_XFERTYP_RSPTYP_48;

  bool waited = send_and_wait_cmd(sdcp, xfer);

  *resp = SDHC->CMDRSP[0];
  TRACE(11, *resp);

  return waited;
}

/**
 * @brief   Sends an SDIO command with a long response expected and CRC.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] cmd       card command
 * @param[in] arg       command argument
 * @param[out] resp     pointer to the response buffer (four words)
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */
bool sdc_lld_send_cmd_long_crc(SDCDriver *sdcp, uint8_t cmd, uint32_t arg,
                               uint32_t *resp) {

  /* In response format R2 (the 136-bit or "long" response) the CRC7
     field is valid, but the command index field is set to all 1s, so
     we need to disable the command index check function (CICEN=0). */
  
  SDHC->CMDARG = arg;
  uint32_t xfer =
    SDHC_XFERTYP_CMDINX(cmd) |
    SDHC_XFERTYP_CMDTYP_NORMAL |
    /* DPSEL=0, CICEN=0, CCCEN=1 */
    SDHC_XFERTYP_CCCEN |
    SDHC_XFERTYP_RSPTYP_136;

  bool waited = send_and_wait_cmd(sdcp, xfer);

  resp[0] = SDHC->CMDRSP[0];
  resp[1] = SDHC->CMDRSP[1];
  resp[2] = SDHC->CMDRSP[2];
  resp[3] = SDHC->CMDRSP[3];

  return waited;
}

/**
 * @brief   Reads one or more blocks.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to read
 * @param[out] buf      pointer to the read buffer
 * @param[in] n         number of blocks to read
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */

bool sdc_lld_read(SDCDriver *sdcp, uint32_t startblk,
		  uint8_t *buf, uint32_t n) {
  uint32_t cmdx = (n == 1)?
    SDHC_XFERTYP_CMDINX(MMCSD_CMD_READ_SINGLE_BLOCK) :
    SDHC_XFERTYP_CMDINX(MMCSD_CMD_READ_MULTIPLE_BLOCK);
  cmdx |= SDHC_XFERTYP_DTDSEL;

  return sdc_lld_transfer(sdcp, startblk, (uintptr_t)buf, n, cmdx);
}

/**
 * @brief   Writes one or more blocks.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 * @param[in] startblk  first block to write
 * @param[out] buf      pointer to the write buffer
 * @param[in] n         number of blocks to write
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS operation succeeded.
 * @retval HAL_FAILED   operation failed.
 *
 * @notapi
 */
bool sdc_lld_write(SDCDriver *sdcp, uint32_t startblk,
                   const uint8_t *buf, uint32_t n) {
  uint32_t cmdx = (n == 1)?
    SDHC_XFERTYP_CMDINX(MMCSD_CMD_WRITE_BLOCK) :
    SDHC_XFERTYP_CMDINX(MMCSD_CMD_WRITE_MULTIPLE_BLOCK);

  return sdc_lld_transfer(sdcp, startblk, (uintptr_t)buf, n, cmdx);
}

/**
 * @brief   Waits for card idle condition.
 *
 * @param[in] sdcp      pointer to the @p SDCDriver object
 *
 * @return              The operation status.
 * @retval HAL_SUCCESS  the operation succeeded.
 * @retval HAL_FAILED   the operation failed.
 *
 * @api
 */
bool sdc_lld_sync(SDCDriver *sdcp) {

  (void)sdcp;

  return HAL_SUCCESS;
}

bool sdc_lld_read_special(SDCDriver *sdcp, uint8_t *buf, size_t bytes,
			  uint8_t cmd, uint32_t argument) {
  uintptr_t bufaddr = (uintptr_t)buf;

  osalDbgCheck((bufaddr & 0x03) == 0);  /* Must be 32-bit aligned */
  osalDbgCheck(bytes > 0);
  osalDbgCheck(bytes < 4096);

  osalDbgAssert((SDHC->PRSSTAT & (SDHC_PRSSTAT_DLA|SDHC_PRSSTAT_CDIHB|SDHC_PRSSTAT_CIHB)) == 0,
		"SDHC interface not ready");

  TRACE(5, argument);

  /* Store the cmd argument and DMA start address */
  SDHC->CMDARG = argument;
  SDHC->DSADDR = bufaddr;

  /* We're reading one block, of a (possibly) nonstandard size */
  SDHC->BLKATTR = SDHC_BLKATTR_BLKSIZE(bytes);

  uint32_t xfer =
    SDHC_XFERTYP_CMDINX(cmd) |    /* the command */
    SDHC_XFERTYP_DTDSEL |         /* read transfer (card -> host) */
    SDHC_XFERTYP_CMDTYP_NORMAL |
    SDHC_XFERTYP_CICEN | SDHC_XFERTYP_CCCEN |
    SDHC_XFERTYP_RSPTYP_48 |
    SDHC_XFERTYP_DPSEL | SDHC_XFERTYP_DMAEN;  /* DMA-assisted data transfer */

  return send_and_wait_transfer(sdcp, xfer);
}

bool sdc_lld_is_card_inserted(SDCDriver *sdcp) {
  (void)sdcp;

  return ( SDHC->PRSSTAT & SDHC_PRSSTAT_CLSL )? true : false;
}

bool sdc_lld_is_write_protected(SDCDriver *sdcp) {
  (void)sdcp;
  return false;
}

#endif /* HAL_USE_SDC == TRUE */

/** @} */
