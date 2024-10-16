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

#include "fx_stm32_sd_driver.h"

extern SD_HandleTypeDef hsd1;
#if (FX_STM32_SD_INIT == 1)
extern void MX_SDMMC1_SD_Init(void);
#endif

/* USER CODE BEGIN  0 */

/* USER CODE END  0 */

__IO UINT sd_rx_cplt;
__IO UINT sd_tx_cplt;

/**
* @brief Initializes the SD IP instance
* @param UINT instance SD instance to initialize
* @retval 0 on success error value otherwise
*/
INT fx_stm32_sd_init(UINT instance)
{
  INT ret = 0;

  /* USER CODE BEGIN PRE_FX_SD_INIT */
  UNUSED(instance);
  /* USER CODE END PRE_FX_SD_INIT */

#if (FX_STM32_SD_INIT == 1)
	MX_SDMMC1_SD_Init();
#endif

  /* USER CODE BEGIN POST_FX_SD_INIT */

  /* USER CODE END POST_FX_SD_INIT */

  return ret;
}

/**
* @brief Deinitializes the SD IP instance
* @param UINT instance SD instance to deinitialize
* @retval 0 on success error value otherwise
*/
INT fx_stm32_sd_deinit(UINT instance)
{
  INT ret = 0;

  /* USER CODE BEGIN PRE_FX_SD_DEINIT */
  UNUSED(instance);
  /* USER CODE END PRE_FX_SD_DEINIT */

  if(HAL_SD_DeInit(&hsd1) != HAL_OK)
  {
    ret = 1;
  }

  /* USER CODE BEGIN POST_FX_SD_DEINIT */

  /* USER CODE END POST_FX_SD_DEINIT */

  return ret;
}

/**
* @brief Check the SD IP status.
* @param UINT instance SD instance to check
* @retval 0 when ready 1 when busy
*/
INT fx_stm32_sd_get_status(UINT instance)
{
  INT ret = 0;

  /* USER CODE BEGIN PRE_GET_STATUS */
  UNUSED(instance);
  /* USER CODE END PRE_GET_STATUS */

  if(HAL_SD_GetCardState(&hsd1) != HAL_SD_CARD_TRANSFER)
  {
    ret = 1;
  }

  /* USER CODE BEGIN POST_GET_STATUS */

  /* USER CODE END POST_GET_STATUS */

  return ret;
}

/**
* @brief Read Data from the SD device into a buffer.
* @param UINT instance SD IP instance to read from.
* @param UINT *buffer buffer into which the data is to be read.
* @param UINT start_block the first block to start reading from.
* @param UINT total_blocks total number of blocks to read.
* @retval 0 on success error code otherwise
*/
INT fx_stm32_sd_read_blocks(UINT instance, UINT *buffer, UINT start_block, UINT total_blocks)
{
  INT ret = 0;

  /* USER CODE BEGIN PRE_READ_BLOCKS */
  /* USER CODE END PRE_READ_BLOCKS */
  sd_rx_cplt = 0;

  if(HAL_SD_ReadBlocks_DMA(&hsd1, (uint8_t *)buffer, start_block, total_blocks) != HAL_OK)
  {
    ret = 1;
  }

  /* USER CODE BEGIN POST_READ_BLOCKS */

  /* USER CODE END POST_READ_BLOCKS */

  return ret;
}

/**
* @brief Write data buffer into the SD device.
* @param UINT instance SD IP instance to write into.
* @param UINT *buffer buffer to write into the SD device.
* @param UINT start_block the first block to start writing into.
* @param UINT total_blocks total number of blocks to write.
* @retval 0 on success error code otherwise
*/
INT fx_stm32_sd_write_blocks(UINT instance, UINT *buffer, UINT start_block, UINT total_blocks)
{
  INT ret = 0;

  /* USER CODE BEGIN PRE_WRITE_BLOCKS */
  /* USER CODE END PRE_WRITE_BLOCKS */

  sd_tx_cplt = 0;

  if(HAL_SD_WriteBlocks_DMA(&hsd1, (uint8_t *)buffer, start_block, total_blocks) != HAL_OK)
  {
    ret = 1;
  }

  /* USER CODE BEGIN POST_WRITE_BLOCKS */

  /* USER CODE END POST_WRITE_BLOCKS */

  return ret;
}

/**
* @brief SD DMA Tx Transfer completed callbacks
* @param SD_HandleTypeDef *hsd the SD_HandleTypeDef handle
* @retval None
*/
void HAL_SD_TxCpltCallback(SD_HandleTypeDef *hsd)
{
/* USER CODE BEGIN TX_COMPLETED_0 */

/* USER CODE END TX_COMPLETED_0 */

  sd_tx_cplt = 1;

/* USER CODE BEGIN TX_COMPLETED_1 */

/* USER CODE END TX_COMPLETED_1 */

}

/**
* @brief SD DMA Rx Transfer completed callbacks
* @param SD_HandleTypeDef *hsd the SD_HandleTypeDef handle
* @retval None
*/
void HAL_SD_RxCpltCallback(SD_HandleTypeDef *hsd)
{
  /* USER CODE BEGIN RX_COMPLETED_0 */

/* USER CODE END RX_COMPLETED_0 */
  sd_rx_cplt = 1;

/* USER CODE BEGIN RX_COMPLETED_1 */

/* USER CODE END RX_COMPLETED_1 */

}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
