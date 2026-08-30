/**
  ******************************************************************************
  * @file    stm32h750b_discovery_mmc.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32h750b_discovery_mmc.c driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H750B_DK_MMC_H
#define STM32H750B_DK_MMC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h750b_discovery_conf.h"
#include "stm32h750b_discovery_errno.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H750B_DK
  * @{
  */

/** @addtogroup STM32H750B_DK_MMC
  * @{
  */

/** @defgroup STM32H750B_DK_MMC_Exported_Types Exported Types
  * @{
  */

/**
  * @brief SD Card information structure
  */
#define BSP_MMC_CardInfo HAL_MMC_CardInfoTypeDef

#if (USE_HAL_MMC_REGISTER_CALLBACKS == 1)
typedef struct
{
  pMMC_CallbackTypeDef  pMspInitCb;
  pMMC_CallbackTypeDef  pMspDeInitCb;
}BSP_MMC_Cb_t;
#endif /* (USE_HAL_MMC_REGISTER_CALLBACKS == 1) */
/**
  * @}
  */

/** @defgroup STM32H750B_DK_MMC_Exported_Constants Exported Constants
  * @{
  */
/**
  * @brief  MMC status structure definition
  */
#define MMC_INSTANCES_NBR             1UL

/**
  * @brief  MMC read/write timeout
  */
#ifndef MMC_WRITE_TIMEOUT
#define MMC_WRITE_TIMEOUT         100U
#endif

#ifndef MMC_READ_TIMEOUT
#define MMC_READ_TIMEOUT          100U
#endif

/**
  * @brief  MMC transfer state definition
  */
#define MMC_TRANSFER_OK               0U
#define MMC_TRANSFER_BUSY             1U

#define MMC_PRESENT                   1UL
#define MMC_NOT_PRESENT               0UL

/**
  * @}
  */

/** @addtogroup STM32H750B_DK_MMC_Exported_Variables
  * @{
  */
extern MMC_HandleTypeDef hsd_sdmmc[MMC_INSTANCES_NBR];

/**
  * @}
  */

/** @addtogroup STM32H750B_DK_MMC_Exported_Functions
  * @{
  */
int32_t BSP_MMC_Init(uint32_t Instance);
int32_t BSP_MMC_DeInit(uint32_t Instance);

int32_t BSP_MMC_ReadBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t BlocksNbr);
int32_t BSP_MMC_WriteBlocks(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t NbrOfBlocks);
int32_t BSP_MMC_ReadBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t NbrOfBlocks);
int32_t BSP_MMC_WriteBlocks_DMA(uint32_t Instance, uint32_t *pData, uint32_t BlockIdx, uint32_t NbrOfBlocks);
int32_t BSP_MMC_Erase(uint32_t Instance,uint32_t StartAddr, uint32_t EndAddr);
int32_t BSP_MMC_GetCardState(uint32_t Instance);
int32_t BSP_MMC_GetCardInfo(uint32_t Instance, BSP_MMC_CardInfo *CardInfo);
void    BSP_MMC_IRQHandler(uint32_t Instance);
HAL_StatusTypeDef MX_MMC_SD_Init(MMC_HandleTypeDef *hmmc);
/* These functions can be modified in case the current settings (e.g. DMA stream)
   need to be changed for specific application needs */
void    BSP_MMC_AbortCallback(uint32_t Instance);
void    BSP_MMC_WriteCpltCallback(uint32_t Instance);
void    BSP_MMC_ReadCpltCallback(uint32_t Instance);
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

#endif /* STM32H750B_DK_MMC_H */
