/*
  Copyright 2012 Uladzimir Pylinski aka barthess.
  You may use this work without restrictions, as long as this notice is included.
  The work is provided "as is" without warranty of any kind, neither express nor implied.
*/

#ifndef HAL_EE24XX_H
#define HAL_EE24XX_H

#include "hal.h"

#if defined(HAL_USE_EEPROM) && HAL_USE_EEPROM && EEPROM_USE_EE24XX

#define EEPROM_DEV_24XX 24

/**
 * @extends EepromFileConfig
 */
typedef struct {
  _eeprom_file_config_data
  /**
   * Driver connected to IC.
   */
  I2CDriver     *i2cp;
  /**
   * Address of IC on I2C bus.
   */
  i2caddr_t     addr;
  /**
   * Pointer to write buffer. The safest size is (pagesize + 2)
   */
  uint8_t       *write_buf;
} I2CEepromFileConfig;

/**
 * @brief   @p I2CEepromFileStream specific data.
 */
#define _eeprom_file_stream_data_i2c                                       \
  _eeprom_file_stream_data

/**
 * @extends EepromFileStream
 *
 * @brief   EEPROM file stream driver class for I2C device.
 */
typedef struct {
  const struct EepromFileStreamVMT *vmt;
  _eeprom_file_stream_data_i2c
  /* Overwritten parent data member. */
  const I2CEepromFileConfig *cfg;
} I2CEepromFileStream;


/**
 * Open I2C EEPROM IC as file and return pointer to the file stream object
 * @note      Fucntion allways successfully open file. All checking makes
 *            in read/write functions.
 */
#define I2CEepromFileOpen(efs, eepcfg, eepdev) \
  EepromFileOpen((EepromFileStream *)efs, (EepromFileConfig *)eepcfg, eepdev);

#endif /* #if defined(EEPROM_USE_EE24XX) && EEPROM_USE_EE24XX */

#endif // HAL_EE24XX_H
