/**
  ******************************************************************************
  * @file    stm32h7b3i_discovery_sd.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32h7b3i_discovery_sd.c driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H7B3I_DK_SD_H
#define STM32H7B3I_DK_SD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7b3i_discovery_conf.h"
#include "stm32h7b3i_discovery_errno.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H7B3I_DK
  * @{
  */

/** @defgroup STM32H7B3I_DK_SD SD
  * @{
  */

/** @defgroup STM32H7B3I_DK_SD_Exported_Types Exported Types
  * @{
  */

/**
  * @brief SD Card information structure
  */
#define BSP_SD_CardInfo HAL_SD_CardInfoTypeDef

#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
typedef struct
{
  pSD_CallbackTypeDef  pMspInitCb;
  pSD_CallbackTypeDef  pMspDeInitCb;
} BSP_SD_Cb_t;
#endif /* (USE_HAL_SD_REGISTER_CALLBACKS == 1) */
/**
  * @}
  */

/** @defgroup STM32H7B3I_DK_SD_Exported_Constants Exported Constants
  * @{
  */
#define SD_INSTANCES_NBR         1UL

#ifndef SD_WRITE_TIMEOUT
#define SD_WRITE_TIMEOUT         500U
#endif

#ifndef SD_READ_TIMEOUT
#define SD_READ_TIMEOUT          100U
#endif

/**
  * @brief  SD transfer state definition
  */
#define   SD_TRANSFER_OK         0U
#define   SD_TRANSFER_BUSY       1U

/**
  * @brief SD-detect signal
  */
#define SD_PRESENT               1UL
#define SD_NOT_PRESENT           0UL

#define SD_DETECT_PIN                        GPIO_PIN_8
#define SD_DETECT_GPIO_PORT                  GPIOI
#define SD_DETECT_GPIO_CLK_ENABLE()          __HAL_RCC_GPIOI_CLK_ENABLE()
#define SD_DETECT_GPIO_CLK_DISABLE()         __HAL_RCC_GPIOI_CLK_DISABLE()
#define SD_DETECT_EXTI_IRQn                  EXTI9_5_IRQn
#define SD_DETECT_EXTI_LINE                  EXTI_LINE_8

/* DMA definitions for SD DMA transfer */
#define SD1_DMAx_TxRx_CLK_ENABLE           __HAL_RCC_DMA2_CLK_ENABLE
#define SD1_SDMMC1_DMAx_Tx_CHANNEL         DMA_CHANNEL_4
#define SD1_SDMMC1_DMAx_Rx_CHANNEL         DMA_CHANNEL_4
#define SD1_SDMMC1_DMAx_Tx_STREAM          DMA2_Stream6
#define SD1_SDMMC1_DMAx_Rx_STREAM          DMA2_Stream3
#define SD1_SDMMC1_DMAx_Tx_IRQn            DMA2_Stream6_IRQn
#define SD1_SDMMC1_DMAx_Rx_IRQn            DMA2_Stream3_IRQn
#define SD1_SDMMC1_DMA_Tx_IRQHandler       DMA2_Stream6_IRQHandler
#define SD1_SDMMC1_DMA_Rx_IRQHandler       DMA2_Stream3_IRQHandler
#define SD1_SDMMC1_IRQHandler              SDMMC1_IRQHandler

/**
  * @}
  */

/** @addtogroup STM32H7B3I_DK_SD_Exported_Variables
  * @{
  */
extern SD_HandleTypeDef   hsd_sdmmc[SD_INSTANCES_NBR];
extern EXTI_HandleTypeDef hsd_exti[SD_INSTANCES_NBR];

/**
  * @}
  */

/** @defgroup STM32H7B3I_DK_SD_Exported_FunctionsPrototypes Exported Functions Prototypes
  * @{
  */
int32_t BSP_SD_Init(uint32_t Instance);
int32_t BSP_SD_DeInit(uint32_t Instance);
#if (USE_HAL_SD_REGISTER_CALLBACKS == 1)
int32_t BSP_SD_RegisterMspCallbacks(uint32_t Instance, BSP_SD_Cb_t *CallBacks);
int32_t BSP_SD_RegisterDefaultMspCallbacks(uint32_t Instance);
#endif /* (USE_HAL_SD_REGISTER_CALLBACKS == 1) */
int32_t BSP_SD_DetectITConfig(uint32_t Instance);
int32_t BSP_SD_ReadBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t BSP_SD_WriteBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t NbrOfBlocks);
int32_t BSP_SD_ReadBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t NbrOfBlocks);
int32_t BSP_SD_WriteBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t NbrOfBlocks);
int32_t BSP_SD_ReadBlocks_IT(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t NbrOfBlocks);
int32_t BSP_SD_WriteBlocks_IT(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t NbrOfBlocks);
int32_t BSP_SD_Erase(uint32_t Instance, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t BSP_SD_GetCardState(uint32_t Instance);
int32_t BSP_SD_GetCardInfo(uint32_t Instance, BSP_SD_CardInfo *CardInfo);
int32_t BSP_SD_IsDetected(uint32_t Instance);

/* These functions can be modified in case the current settings (e.g. DMA stream or IT)
   need to be changed for specific application needs */
void BSP_SD_AbortCallback(uint32_t Instance);
void BSP_SD_WriteCpltCallback(uint32_t Instance);
void BSP_SD_ReadCpltCallback(uint32_t Instance);

HAL_StatusTypeDef MX_SDMMC1_SD_Init(SD_HandleTypeDef *hsd);
void BSP_SD_DetectCallback(uint32_t Instance, uint32_t Status);

void BSP_SD_DETECT_IRQHandler(uint32_t Instance);
void BSP_SD_IRQHandler(uint32_t Instance);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* STM32H7B3I_DK_SD_H */
