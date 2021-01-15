/*
  Copyright 2012 Uladzimir Pylinski aka barthess.
  You may use this work without restrictions, as long as this notice is included.
  The work is provided "as is" without warranty of any kind, neither express nor implied.
*/

#ifndef HAL_EE25XX_H
#define HAL_EE25XX_H

#include "hal.h"

#if defined(HAL_USE_EEPROM) && HAL_USE_EEPROM && EEPROM_USE_EE25XX

#define EEPROM_DEV_25XX 25

/**
 * @extends EepromFileConfig
 */
typedef struct {
  _eeprom_file_config_data
  /**
   * Driver connected to IC.
   */
  SPIDriver       *spip;
  /**
   * Config associated with SPI driver.
   */
  const SPIConfig *spicfg;
} SPIEepromFileConfig;

/**
 * @brief   @p SPIEepromFileStream specific data.
 */
#define _eeprom_file_stream_data_spi                                       \
  _eeprom_file_stream_data

/**
 * @extends EepromFileStream
 *
 * @brief   EEPROM file stream driver class for SPI device.
 */
typedef struct {
  const struct EepromFileStreamVMT *vmt;
  _eeprom_file_stream_data_spi
  /* Overwritten parent data member. */
  const SPIEepromFileConfig *cfg;
} SPIEepromFileStream;

/**
 * Open SPI EEPROM IC as file and return pointer to the file stream object
 * @note      Fucntion allways successfully open file. All checking makes
 *            in read/write functions.
 */
EepromFileStream *SPIEepromFileOpen(SPIEepromFileStream *efs,
                                    const SPIEepromFileConfig *eepcfg,
                                    const EepromDevice *eepdev);

#define SPIEepromFileOpen(efs, eepcfg, eepdev) \
  EepromFileOpen((EepromFileStream *)efs, (EepromFileConfig *)eepcfg, eepdev);

#endif /* #if defined(EEPROM_USE_EE25XX) && EEPROM_USE_EE25XX */

#endif // HAL_EE25XX_H
