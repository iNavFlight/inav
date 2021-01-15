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

#ifndef HAL_EEPROM_H_
#define HAL_EEPROM_H_

#include "hal.h"

#ifndef EEPROM_USE_EE25XX
#define EEPROM_USE_EE25XX FALSE
#endif

#ifndef EEPROM_USE_EE24XX
#define EEPROM_USE_EE24XX FALSE
#endif

#if (HAL_USE_EEPROM == TRUE) || defined(__DOXYGEN__)

#if EEPROM_USE_EE25XX && EEPROM_USE_EE24XX
#define EEPROM_TABLE_SIZE 2
#elif EEPROM_USE_EE25XX || EEPROM_USE_EE24XX
#define EEPROM_TABLE_SIZE 1
#else
#error "No EEPROM device selected!"
#endif

#if EEPROM_USE_EE25XX && !HAL_USE_SPI
#error "25xx enabled but SPI driver is disabled!"
#endif

#if EEPROM_USE_EE24XX && !HAL_USE_I2C
#error "24xx enabled but I2C driver is disabled!"
#endif

#define _eeprom_file_config_data                                            \
  /* Lower barrier of file in EEPROM memory array. */                       \
  uint32_t        barrier_low;                                              \
  /* Higher barrier of file in EEPROM memory array. */                      \
  uint32_t        barrier_hi;                                               \
  /* Size of memory array in bytes. */                                      \
  uint32_t        size;                                                     \
  /* Size of single page in bytes. */                                       \
  uint16_t        pagesize;                                                 \
  /* Time needed by IC for single byte/page writing. */                     \
  systime_t       write_time;
  
typedef uint32_t fileoffset_t;

typedef struct {
  _eeprom_file_config_data
} EepromFileConfig;

/**
 * @brief   @p EepromFileStream specific data.
 */
#define _eeprom_file_stream_data                                            \
  _base_sequential_stream_data                                                    \
  uint32_t                    errors;                                       \
  uint32_t                    position;                                     \

/**
 * @extends BaseFileStreamVMT
 *
 * @brief   @p EepromFileStream virtual methods table.
 */
struct EepromFileStreamVMT {
  _file_stream_methods
};

/**
 * @extends BaseFileStream
 *
 * @brief   EEPROM file stream driver class.
 * @details This class extends @p BaseFileStream by adding some fields.
 */
typedef struct {
  /** @brief Virtual Methods Table.*/
  const struct EepromFileStreamVMT *vmt;
  _eeprom_file_stream_data
  /** pointer to config object, must be overwritten by all derived classes.*/
  const EepromFileConfig *cfg;
} EepromFileStream;

/**
 * @brief   Low level device descriptor.
 */
typedef struct {
  const uint8_t                       id;
  const struct EepromFileStreamVMT   *efsvmt;
} EepromDevice;

const EepromDevice *EepromFindDevice(uint8_t id);

EepromFileStream *EepromFileOpen(EepromFileStream *efs,
                                 const EepromFileConfig *eepcfg,
                                 const EepromDevice *eepdev);

uint8_t  EepromReadByte(EepromFileStream *efs);
uint16_t EepromReadHalfword(EepromFileStream *efs);
uint32_t EepromReadWord(EepromFileStream *efs);
size_t EepromWriteByte(EepromFileStream *efs, uint8_t data);
size_t EepromWriteHalfword(EepromFileStream *efs, uint16_t data);
size_t EepromWriteWord(EepromFileStream *efs, uint32_t data);

msg_t eepfs_getsize(void *ip);
msg_t eepfs_getposition(void *ip);
msg_t eepfs_lseek(void *ip, fileoffset_t offset);
msg_t eepfs_close(void *ip);
msg_t eepfs_geterror(void *ip);
msg_t eepfs_put(void *ip, uint8_t b);
msg_t eepfs_get(void *ip);

#include "hal_ee24xx.h"
#include "hal_ee25xx.h"

#endif /* #if defined(HAL_USE_EEPROM) && HAL_USE_EEPROM */
#endif /* HAL_EEPROM_H_ */
