/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/

#include "lx_stm32_qspi_driver.h"
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

ULONG  qspi_sector_buffer[LX_STM32_QSPI_SECTOR_SIZE/sizeof(ULONG)];

/**
* @brief Initializes the QSPI IP instance
* @param UINT Instance QSPI instance to initialize
* @retval 0 on success error value otherwise
*/
INT lx_stm32_qspi_lowlevel_init(UINT instance)
{
  INT status = 0;
  /* USER CODE BEGIN OSPI_Init */

  /* USER CODE END OSPI_Init */

  return status;
}

/**
* @brief Get the status of the QSPI instance
* @param UINT Instance QSPI instance
* @retval 0 if the QSPI is ready 1 otherwise
*/
INT lx_stm32_qspi_get_status(UINT instance)
{
  INT status = 0;
  /* USER CODE BEGIN OSPI_Get_Status */

  /* USER CODE END OSPI_Get_Status */

  return status;
}


/**
* @brief Get size info of the flash meomory
* @param UINT Instance QSPI instance
* @param UINT * block_size pointer to be filled with Flash block size
* @param UINT * total_blocks pointer to be filled with Flash total number of blocks
* @retval 0 on Success and block_size and total_blocks are correctly filled
          1 on Failure, block_size = 0, total_blocks = 0
*/
INT lx_stm32_qspi_get_info(UINT instance, ULONG *block_size, ULONG *total_blocks)
{
  INT status = 0;
  /* USER CODE BEGIN OSPI_Get_Info */
  
  /* USER CODE END OSPI_Get_Info */

  return status;
}

/**
* @brief Read data from the QSPI memory into a buffer
* @param UINT Instance QSPI instance
* @param ULONG * address the start address to read from
* @param ULONG * buffer the destination buffer
* @param ULONG words the total number of words to be read
* @retval 0 on Success
1 on Failure
*/

INT lx_stm32_qspi_read(UINT instance, ULONG *address, ULONG *buffer, ULONG words)
{
  INT status = 0;
  /* USER CODE BEGIN OSPI_Read */
  
  /* USER CODE END OSPI_Read */

  return status;
}

/**
* @brief write a data buffer into the QSPI memory
* @param UINT Instance QSPI instance
* @param ULONG * address the start address to write into
* @param ULONG * buffer the data source buffer
* @param ULONG words the total number of words to be written
* @retval 0 on Success
1 on Failure
*/

INT lx_stm32_qspi_write(UINT instance, ULONG *address, ULONG *buffer, ULONG words)
{
  INT status = 0;
  /* USER CODE BEGIN OSPI_Write */

  /* USER CODE END OSPI_Write */

  return status;
}

/**
* @brief Erase the whole flash or a single block
* @param UINT Instance QSPI instance
* @param ULONG  block the block to be erased
* @param ULONG  erase_count the number of times the block was erased
* @param UINT full_chip_erase if set to 0 a single block is erased otherwise the whole flash is
* @retval 0 on Success
1 on Failure
*/
INT lx_stm32_qspi_erase(UINT instance, ULONG block, ULONG erase_count, UINT full_chip_erase)
{
  INT status = 0;
  /* USER CODE BEGIN OSPI_Erase_Block */

  /* USER CODE END OSPI_Erase_Block */

  return status;
}

/**
* @brief Check that a block was actually erased
* @param UINT Instance QSPI instance
* @param ULONG  block the block to be checked
* @retval 0 on Success
1 on Failure
*/
INT lx_stm32_qspi_is_block_erased(UINT instance, ULONG block)
{
  INT status = 0;
  /* USER CODE BEGIN OSPI_Block_Erased */

  /* USER CODE END OSPI_Block_Erased */

  return status;
}

UINT  lx_ospi_driver_system_error(UINT error_code)
{
  UINT status = LX_ERROR;
  /* USER CODE BEGIN OSPI_Block_Erased */

  /* USER CODE END OSPI_Block_Erased */

  return status;
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
