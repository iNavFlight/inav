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
 * @file    hal_flash.c
 * @brief   Generic flash driver class code.
 *
 * @addtogroup HAL_FLASH
 * @{
 */

#include "hal.h"

#include "hal_flash.h"

/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Waits until the current erase operation is finished.
 *
 * @param[in] devp      pointer to a @p BaseFlash object
 * 
 * @return              An error code.
 * @retval FLASH_NO_ERROR if there is no erase operation in progress.
 * @retval FLASH_ERROR_ERASE if the erase operation failed.
 * @retval FLASH_ERROR_HW_FAILURE if access to the memory failed.
 */
flash_error_t flashWaitErase(BaseFlash *devp) {

  while (true) {
    flash_error_t err;
    uint32_t msec;

    /* Checking operation state.*/
    err = flashQueryErase(devp, &msec);
    if (err != FLASH_BUSY_ERASING) {
      return err;
    }

    /* Interval because nice waiting.*/
    osalThreadSleepMilliseconds(msec);
  }
}

/**
 * @brief   Returns the offset of a sector.
 *
 * @param[in] devp      pointer to a @p BaseFlash object
 * @param[in] sector    flash sector number
 *
 * @return the offset of the sector
 */
flash_offset_t flashGetSectorOffset(BaseFlash *devp,
                                    flash_sector_t sector) {
  flash_offset_t offset;
  const flash_descriptor_t *descriptor = flashGetDescriptor(devp);

  osalDbgAssert(sector < descriptor->sectors_count, "invalid sector");

  if (descriptor->sectors != NULL) {
    offset = descriptor->sectors[sector].offset;
  }
  else {
    offset = (flash_offset_t)sector * (flash_offset_t)descriptor->sectors_size;
  }

  return offset;
}

/**
 * @brief   Returns the size of a sector.
 *
 * @param[in] devp      pointer to a @p BaseFlash object
 * @param[in] sector    flash sector number
 *
 * @return the size of the sector
 */
uint32_t flashGetSectorSize(BaseFlash *devp,
                            flash_sector_t sector) {
  uint32_t size;
  const flash_descriptor_t *descriptor = flashGetDescriptor(devp);

  osalDbgAssert(sector < descriptor->sectors_count, "invalid sector");

  if (descriptor->sectors != NULL) {
    size = descriptor->sectors[sector].size;
  }
  else {
    size = descriptor->sectors_size;
  }

  return size;
}
/** @} */
