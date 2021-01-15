/*
  Copyright (c) 2013 Timon Wong

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

/*
  Copyright 2012 Uladzimir Pylinski aka barthess.
  You may use this work without restrictions, as long as this notice is included.
  The work is provided "as is" without warranty of any kind, neither express nor implied.
*/

/*****************************************************************************
 * DATASHEET NOTES
 *****************************************************************************
Write cycle time (byte or page) - 5 ms

Note:
  Page write operations are limited to writing bytes within a single physical
  page, regardless of the number of bytes actually being written. Physical page
  boundaries start at addresses that are integer multiples of the page buffer
  size (or page size and end at addresses that are integer multiples of
  [page size]. If a Page Write command attempts to write across a physical
  page boundary, the result is that the data wraps around to the beginning of
  the current page (overwriting data previously stored there), instead of
  being written to the next page as might be expected.
*********************************************************************/

#include "hal_ee25xx.h"
#include <string.h>

#if (defined(HAL_USE_EEPROM) && HAL_USE_EEPROM && EEPROM_USE_EE25XX) || defined(__DOXYGEN__)

/**
 * @name Commands of 25XX chip.
 * @{
 */
#define CMD_READ    0x03  /**< @brief Read data from memory array beginning at
                               selected address. */
#define CMD_WRITE   0x02  /**< @brief Write data to memory array beginning at
                               selected address. */
#define CMD_WRDI    0x04  /**< Reset the write enable latch (disable write
                               operations). */
#define CMD_WREN    0x06  /**< Set the write enable latch (enable write
                               operations). */
#define CMD_RDSR    0x05  /**< Read STATUS register. */
#define CMD_WRSR    0x01  /**< Write STATUS register. */

/** @} */

/**
 * @name Status of 25XX chip.
 * @{}
 */
#define STAT_BP1    0x08  /**< @brief Block protection (high). */
#define STAT_BP0    0x04  /**< @brief Block protection (low). */
#define STAT_WEL    0x02  /**< @brief Write enable latch. */
#define STAT_WIP    0x01  /**< @brief Write-In-Progress. */

/** @} */

/**
 * @brief 25XX low level write then read rountine.
 *
 * @param[in]  eepcfg pointer to configuration structure of eeprom file.
 * @param[in]  txbuf  pointer to buffer to be transfered.
 * @param[in]  txlen  number of bytes to be transfered.
 * @param[out] rxbuf  pointer to buffer to be received.
 * @param[in]  rxlen  number of bytes to be received.
 */
static void ll_25xx_transmit_receive(const SPIEepromFileConfig *eepcfg,
                                     const uint8_t *txbuf, size_t txlen,
                                     uint8_t *rxbuf, size_t rxlen) {

#if SPI_USE_MUTUAL_EXCLUSION
  spiAcquireBus(eepcfg->spip);
#endif
  spiSelect(eepcfg->spip);
  spiSend(eepcfg->spip, txlen, txbuf);
  if (rxlen) /* Check if receive is needed. */
      spiReceive(eepcfg->spip, rxlen, rxbuf);
  spiUnselect(eepcfg->spip);

#if SPI_USE_MUTUAL_EXCLUSION
  spiReleaseBus(eepcfg->spip);
#endif
}

/**
 * @brief Check whether the device is busy (writing in progress).
 *
 * @param[in] eepcfg   pointer to configuration structure of eeprom file.
 * @return @p true on busy.
 */
static bool ll_eeprom_is_busy(const SPIEepromFileConfig *eepcfg) {

  uint8_t cmd = CMD_RDSR;
  uint8_t stat;
  ll_25xx_transmit_receive(eepcfg, &cmd, 1, &stat, 1);
  if (stat & STAT_WIP)
    return TRUE;
  return FALSE;
}

/**
 * @brief Lock device.
 *
 * @param[in] eepcfg  pointer to configuration structure of eeprom file.
 */
static void ll_eeprom_lock(const SPIEepromFileConfig *eepcfg) {

  uint8_t cmd = CMD_WRDI;
  ll_25xx_transmit_receive(eepcfg, &cmd, 1, NULL, 0);
}

/**
 * @brief Unlock device.
 *
 * @param[in] eepcfg  pointer to configuration structure of eeprom file.
 */
static void ll_eeprom_unlock(const SPIEepromFileConfig *eepcfg) {

  uint8_t cmd = CMD_WREN;
  ll_25xx_transmit_receive(eepcfg, &cmd, 1, NULL, 0);
}

/**
 * @brief   Prepare byte sequence for command and address
 *
 * @param[in] seq   pointer to first 3byte sequence
 * @param[in] size  size of the eeprom device
 * @param[in] cmd   command
 * @param[in] addr  address
 * @return number of bytes of this sequence
 */
static uint8_t ll_eeprom_prepare_seq(uint8_t *seq, uint32_t size, uint8_t cmd,
                                     uint32_t addr) {

  seq[0] = ((uint8_t)cmd & 0xff);

  if (size > 0xffffUL) {
    /* High density, 24bit address. */
    seq[1] = (uint8_t)((addr >> 16) & 0xff);
    seq[2] = (uint8_t)((addr >> 8) & 0xff);
    seq[3] = (uint8_t)(addr & 0xff);
    return 4;
  }
  else if (size > 0x00ffUL) {
    /* Medium density, 16bit address. */
    seq[1] = (uint8_t)((addr >> 8) & 0xff);
    seq[2] = (uint8_t)(addr & 0xff);
    return 3;
  }

  /* Low density, 8bit address. */
  seq[1] = (uint8_t)(addr & 0xff);
  return 2;
}

/**
 * @brief   EEPROM read routine.
 *
 * @param[in]  eepcfg   pointer to configuration structure of eeprom file.
 * @param[in]  offset   addres of 1-st byte to be read.
 * @param[out] data     pointer to buffer with data to be written.
 * @param[in]  len      number of bytes to be red.
 */
static msg_t ll_eeprom_read(const SPIEepromFileConfig *eepcfg, uint32_t offset,
                            uint8_t *data, size_t len) {

  uint8_t txbuff[4];
  uint8_t txlen;

  osalDbgAssert(((len <= eepcfg->size) && ((offset + len) <= eepcfg->size)),
             "out of device bounds");

  if (eepcfg->spip->state != SPI_READY)
      return MSG_RESET;

  txlen = ll_eeprom_prepare_seq(txbuff, eepcfg->size, CMD_READ,
                                (offset + eepcfg->barrier_low));
  ll_25xx_transmit_receive(eepcfg, txbuff, txlen, data, len);

  return MSG_OK;
}

/**
 * @brief   EEPROM write routine.
 * @details Function writes data to EEPROM.
 * @pre     Data must be fit to single EEPROM page.
 *
 * @param[in] eepcfg  pointer to configuration structure of eeprom file.
 * @param[in] offset  addres of 1-st byte to be writen.
 * @param[in] data    pointer to buffer with data to be written.
 * @param[in] len     number of bytes to be written.
 */
static msg_t ll_eeprom_write(const SPIEepromFileConfig *eepcfg, uint32_t offset,
                             const uint8_t *data, size_t len) {

  uint8_t txbuff[4];
  uint8_t txlen;
  systime_t now;

  osalDbgAssert(((len <= eepcfg->size) && ((offset + len) <= eepcfg->size)),
             "out of device bounds");
  osalDbgAssert((((offset + eepcfg->barrier_low) / eepcfg->pagesize) ==
              (((offset + eepcfg->barrier_low) + len - 1) / eepcfg->pagesize)),
             "data can not be fitted in single page");

  if (eepcfg->spip->state != SPI_READY)
      return MSG_RESET;

  /* Unlock array for writting. */
  ll_eeprom_unlock(eepcfg);

#if SPI_USE_MUTUAL_EXCLUSION
  spiAcquireBus(eepcfg->spip);
#endif

  spiSelect(eepcfg->spip);
  txlen = ll_eeprom_prepare_seq(txbuff, eepcfg->size, CMD_WRITE,
                                (offset + eepcfg->barrier_low));
  spiSend(eepcfg->spip, txlen, txbuff);
  spiSend(eepcfg->spip, len, data);
  spiUnselect(eepcfg->spip);

#if SPI_USE_MUTUAL_EXCLUSION
  spiReleaseBus(eepcfg->spip);
#endif

  /* Wait until EEPROM process data. */
  now = chVTGetSystemTimeX();
  while (ll_eeprom_is_busy(eepcfg)) {
    if ((chVTGetSystemTimeX() - now) > eepcfg->write_time) {
      return MSG_TIMEOUT;
    }

    chThdYield();
  }

  /* Lock array preventing unexpected access */
  ll_eeprom_lock(eepcfg);
  return MSG_OK;
}

/**
 * @brief   Determines and returns size of data that can be processed
 */
static size_t __clamp_size(void *ip, size_t n) {

  if (((size_t)eepfs_getposition(ip) + n) > (size_t)eepfs_getsize(ip))
    return eepfs_getsize(ip) - eepfs_getposition(ip);
  else
    return n;
}

/**
 * @brief   Write data that can be fitted in one page boundary
 */
static msg_t __fitted_write(void *ip, const uint8_t *data, size_t len, uint32_t *written) {

  msg_t status = MSG_RESET;

  osalDbgAssert(len > 0, "len must be greater than 0");

  status = ll_eeprom_write(((SPIEepromFileStream *)ip)->cfg,
                           eepfs_getposition(ip), data, len);
  if (status == MSG_OK) {
    *written += len;
    eepfs_lseek(ip, eepfs_getposition(ip) + len);
  }
  return status;
}

/**
 * @brief     Write data to EEPROM.
 * @details   Only one EEPROM page can be written at once. So function
 *            splits large data chunks in small EEPROM transactions if needed.
 * @note      To achieve the maximum efficiency use write operations
 *            aligned to EEPROM page boundaries.
 */
static size_t write(void *ip, const uint8_t *bp, size_t n) {

  size_t   len = 0;      /* bytes to be written per transaction */
  uint32_t written = 0;  /* total bytes successfully written */
  uint16_t pagesize;
  uint32_t firstpage;
  uint32_t lastpage;

  volatile const SPIEepromFileConfig *cfg = ((SPIEepromFileStream *)ip)->cfg;

  osalDbgCheck((ip != NULL) && (((SPIEepromFileStream *)ip)->vmt != NULL));

  if (n == 0)
    return 0;

  n = __clamp_size(ip, n);
  if (n == 0)
    return 0;

  pagesize  = cfg->pagesize;
  firstpage = (cfg->barrier_low + eepfs_getposition(ip)) / pagesize;
  lastpage  = ((cfg->barrier_low + eepfs_getposition(ip) + n) - 1) / pagesize;

  /* data fits in single page */
  if (firstpage == lastpage) {
    len = n;
    __fitted_write(ip, bp, len, &written);
    return written;
  }
  
  else {
    /* write first piece of data to first page boundary */
    len =  ((firstpage + 1) * pagesize) - eepfs_getposition(ip);
    len -= cfg->barrier_low;
    if (__fitted_write(ip, bp, len, &written) != MSG_OK)
      return written;
    bp += len;

    /* now write page sized blocks (zero or more) */
    while ((n - written) > pagesize) {
      len = pagesize;
      if (__fitted_write(ip, bp, len, &written) != MSG_OK)
        return written;
      bp += len;
    }

    /* write tail */
    len = n - written;
    if (len == 0)
      return written;
    else {
      __fitted_write(ip, bp, len, &written);
    }
  }

  return written;
}

/**
 * Read some bytes from current position in file. After successful
 * read operation the position pointer will be increased by the number
 * of read bytes.
 */
static size_t read(void *ip, uint8_t *bp, size_t n) {

  msg_t status = MSG_OK;

  osalDbgCheck((ip != NULL) && (((EepromFileStream *)ip)->vmt != NULL));

  if (n == 0)
    return 0;

  n = __clamp_size(ip, n);
  if (n == 0)
    return 0;

  /* call low level function */
  status = ll_eeprom_read(((SPIEepromFileStream *)ip)->cfg,
                          eepfs_getposition(ip), bp, n);
  if (status != MSG_OK)
    return 0;
  else {
    eepfs_lseek(ip, (eepfs_getposition(ip) + n));
    return n;
  }
}

static const struct EepromFileStreamVMT vmt = {
  (size_t)0,
  write,
  read,
  eepfs_put,
  eepfs_get,
  eepfs_close,
  eepfs_geterror,
  eepfs_getsize,
  eepfs_getposition,
  eepfs_lseek,
};

EepromDevice eepdev_25xx = {
  EEPROM_DEV_25XX,
  &vmt
};

#endif /* EEPROM_USE_EE25XX */
