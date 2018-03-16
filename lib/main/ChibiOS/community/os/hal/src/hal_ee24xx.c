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

#include "hal_ee24xx.h"
#include <string.h>

#if (defined(HAL_USE_EEPROM) && HAL_USE_EEPROM && EEPROM_USE_EE24XX) || defined(__DOXYGEN__)

/*
 ******************************************************************************
 * DEFINES
 ******************************************************************************
 */
/*
#if defined(SAM7_PLATFORM)
#define EEPROM_I2C_CLOCK (MCK / (((i2cp->config->cwgr & 0xFF) + ((i2cp->config->cwgr >> 8) & 0xFF)) * (1 << ((i2cp->config->cwgr >> 16) & 7)) + 6))
#else
#define EEPROM_I2C_CLOCK (i2cp->config->clock_speed)
#endif
*/
#define EEPROM_I2C_CLOCK 400000

/*
 ******************************************************************************
 * EXTERNS
 ******************************************************************************
 */

/*
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
 */

/*
 *******************************************************************************
 * LOCAL FUNCTIONS
 *******************************************************************************
 */
/**
 * @brief   Split one uint16_t address to two uint8_t.
 *
 * @param[in] txbuf pointer to driver transmit buffer
 * @param[in] addr  uint16_t address
 */
#define eeprom_split_addr(txbuf, addr){                                       \
    (txbuf)[0] = ((uint8_t)((addr >> 8) & 0xFF));                              \
    (txbuf)[1] = ((uint8_t)(addr & 0xFF));                                     \
  }

/*
 *******************************************************************************
 * EXPORTED FUNCTIONS
 *******************************************************************************
 */

/**
 * @brief     Calculates requred timeout.
 */
static systime_t calc_timeout(I2CDriver *i2cp, size_t txbytes, size_t rxbytes) {
  (void)i2cp;
  const uint32_t bitsinbyte = 10;
  uint32_t tmo;
  tmo = ((txbytes + rxbytes + 1) * bitsinbyte * 1000);
  tmo /= EEPROM_I2C_CLOCK;
  tmo += 10; /* some additional milliseconds to be safer */
  return MS2ST(tmo);
}

/**
 * @brief   EEPROM read routine.
 *
 * @param[in] eepcfg    pointer to configuration structure of eeprom file
 * @param[in] offset    addres of 1-st byte to be read
 * @param[in] data      pointer to buffer with data to be written
 * @param[in] len       number of bytes to be red
 */
static msg_t eeprom_read(const I2CEepromFileConfig *eepcfg,
                         uint32_t offset, uint8_t *data, size_t len) {

  msg_t status = MSG_RESET;
  systime_t tmo = calc_timeout(eepcfg->i2cp, 2, len);

  osalDbgAssert(((len <= eepcfg->size) && ((offset + len) <= eepcfg->size)),
             "out of device bounds");

  eeprom_split_addr(eepcfg->write_buf, (offset + eepcfg->barrier_low));

#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(eepcfg->i2cp);
#endif

  status = i2cMasterTransmitTimeout(eepcfg->i2cp, eepcfg->addr,
                                    eepcfg->write_buf, 2, data, len, tmo);

#if I2C_USE_MUTUAL_EXCLUSION
  i2cReleaseBus(eepcfg->i2cp);
#endif

  return status;
}

/**
 * @brief   EEPROM write routine.
 * @details Function writes data to EEPROM.
 * @pre     Data must be fit to single EEPROM page.
 *
 * @param[in] eepcfg  pointer to configuration structure of eeprom file
 * @param[in] offset  addres of 1-st byte to be write
 * @param[in] data    pointer to buffer with data to be written
 * @param[in] len     number of bytes to be written
 */
static msg_t eeprom_write(const I2CEepromFileConfig *eepcfg, uint32_t offset,
                          const uint8_t *data, size_t len) {
  msg_t status = MSG_RESET;
  systime_t tmo = calc_timeout(eepcfg->i2cp, (len + 2), 0);

  osalDbgAssert(((len <= eepcfg->size) && ((offset + len) <= eepcfg->size)),
             "out of device bounds");
  osalDbgAssert((((offset + eepcfg->barrier_low) / eepcfg->pagesize) ==
              (((offset + eepcfg->barrier_low) + len - 1) / eepcfg->pagesize)),
             "data can not be fitted in single page");

  /* write address bytes */
  eeprom_split_addr(eepcfg->write_buf, (offset + eepcfg->barrier_low));
  /* write data bytes */
  memcpy(&(eepcfg->write_buf[2]), data, len);

#if I2C_USE_MUTUAL_EXCLUSION
  i2cAcquireBus(eepcfg->i2cp);
#endif

  status = i2cMasterTransmitTimeout(eepcfg->i2cp, eepcfg->addr,
                                    eepcfg->write_buf, (len + 2), NULL, 0, tmo);

#if I2C_USE_MUTUAL_EXCLUSION
  i2cReleaseBus(eepcfg->i2cp);
#endif

  /* wait until EEPROM process data */
  chThdSleep(eepcfg->write_time);

  return status;
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
static void __fitted_write(void *ip, const uint8_t *data, size_t len, uint32_t *written) {

  msg_t status = MSG_RESET;

  osalDbgAssert(len != 0, "something broken in hi level part");

  status = eeprom_write(((I2CEepromFileStream *)ip)->cfg,
                        eepfs_getposition(ip), data, len);
  if (status == MSG_OK) {
    *written += len;
    eepfs_lseek(ip, eepfs_getposition(ip) + len);
  }
}

/**
 * @brief     Write data to EEPROM.
 * @details   Only one EEPROM page can be written at once. So fucntion
 *            splits large data chunks in small EEPROM transactions if needed.
 * @note      To achieve the maximum effectivity use write operations
 *            aligned to EEPROM page boundaries.
 */
static size_t write(void *ip, const uint8_t *bp, size_t n) {

  size_t   len = 0;     /* bytes to be written at one trasaction */
  uint32_t written; /* total bytes successfully written */
  uint16_t pagesize;
  uint32_t firstpage;
  uint32_t lastpage;

  osalDbgCheck((ip != NULL) && (((EepromFileStream *)ip)->vmt != NULL));

  if (n == 0)
    return 0;

  n = __clamp_size(ip, n);
  if (n == 0)
    return 0;

  pagesize  =  ((EepromFileStream *)ip)->cfg->pagesize;
  firstpage = (((EepromFileStream *)ip)->cfg->barrier_low +
               eepfs_getposition(ip)) / pagesize;
  lastpage  = (((EepromFileStream *)ip)->cfg->barrier_low +
               eepfs_getposition(ip) + n - 1) / pagesize;

  written = 0;
  /* data fitted in single page */
  if (firstpage == lastpage) {
    len = n;
    __fitted_write(ip, bp, len, &written);
    bp += len;
    return written;
  }

  else {
    /* write first piece of data to first page boundary */
    len =  ((firstpage + 1) * pagesize) - eepfs_getposition(ip);
    len -= ((EepromFileStream *)ip)->cfg->barrier_low;
    __fitted_write(ip, bp, len, &written);
    bp += len;

    /* now writes blocks at a size of pages (may be no one) */
    while ((n - written) > pagesize) {
      len = pagesize;
      __fitted_write(ip, bp, len, &written);
      bp += len;
    }

    /* wrtie tail */
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

  /* Stupid I2C cell in STM32F1x does not allow to read single byte.
     So we must read 2 bytes and return needed one. */
#if defined(STM32F1XX_I2C)
  if (n == 1) {
    uint8_t __buf[2];
    /* if NOT last byte of file requested */
    if ((eepfs_getposition(ip) + 1) < eepfs_getsize(ip)) {
      if (read(ip, __buf, 2) == 2) {
        eepfs_lseek(ip, (eepfs_getposition(ip) + 1));
        bp[0] = __buf[0];
        return 1;
      }
      else
        return 0;
    }
    else {
      eepfs_lseek(ip, (eepfs_getposition(ip) - 1));
      if (read(ip, __buf, 2) == 2) {
        eepfs_lseek(ip, (eepfs_getposition(ip) + 2));
        bp[0] = __buf[1];
        return 1;
      }
      else
        return 0;
    }
  }
#endif /* defined(STM32F1XX_I2C) */

  /* call low level function */
  status  = eeprom_read(((I2CEepromFileStream *)ip)->cfg,
                        eepfs_getposition(ip), bp, n);
  if (status != MSG_OK)
    return 0;
  else {
    eepfs_lseek(ip, (eepfs_getposition(ip) + n));
    return n;
  }
}

static const struct EepromFileStreamVMT vmt = {
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

EepromDevice eepdev_24xx = {
  EEPROM_DEV_24XX,
  &vmt
};

#endif /* EEPROM_USE_EE24XX */
