/**
  ******************************************************************************
  * @file    stm32756g_eval_sd.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32756g_eval_sd.c driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32756G_EVAL_SD_H
#define __STM32756G_EVAL_SD_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "stm32756g_eval_io.h"

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup STM32756G_EVAL
  * @{
  */
    
/** @addtogroup STM32756G_EVAL_SD
  * @{
  */    

/** @defgroup STM32756G_EVAL_SD_Exported_Types SD Exported Types
  * @{
  */

/** 
  * @brief SD Card information structure 
  */
#define BSP_SD_CardInfo HAL_SD_CardInfoTypeDef
/**
  * @}
  */
   
/** 
  * @brief  SD status structure definition  
  */     
#define   MSD_OK                        ((uint8_t)0x00)
#define   MSD_ERROR                     ((uint8_t)0x01)
#define   MSD_ERROR_SD_NOT_PRESENT      ((uint8_t)0x02)

/** 
  * @brief  SD transfer state definition  
  */     
#define   SD_TRANSFER_OK                ((uint8_t)0x00)
#define   SD_TRANSFER_BUSY              ((uint8_t)0x01)
   
/** @defgroup STM32756G_EVAL_SD_Exported_Constants SD Exported Constants
  * @{
  */ 
#define SD_PRESENT               ((uint8_t)0x01)
#define SD_NOT_PRESENT           ((uint8_t)0x00)

#define SD_DATATIMEOUT           ((uint32_t)100000000)
    
/* DMA definitions for SD DMA transfer */
#define __DMAx_TxRx_CLK_ENABLE            __HAL_RCC_DMA2_CLK_ENABLE
#define SD_DMAx_Tx_CHANNEL                DMA_CHANNEL_4
#define SD_DMAx_Rx_CHANNEL                DMA_CHANNEL_4
#define SD_DMAx_Tx_STREAM                 DMA2_Stream6  
#define SD_DMAx_Rx_STREAM                 DMA2_Stream3  
#define SD_DMAx_Tx_IRQn                   DMA2_Stream6_IRQn
#define SD_DMAx_Rx_IRQn                   DMA2_Stream3_IRQn
#define BSP_SDMMC_IRQHandler              SDMMC1_IRQHandler   
#define BSP_SDMMC_DMA_Tx_IRQHandler       DMA2_Stream6_IRQHandler   
#define BSP_SDMMC_DMA_Rx_IRQHandler       DMA2_Stream3_IRQHandler 
#define SD_DetectIRQHandler()             HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8)
/**
  * @}
  */
  
/** @defgroup STM32756G_EVAL_SD_Exported_Macro SD Exported Macro
  * @{
  */ 
/**
  * @}
  */
   
/** @defgroup STM32756G_EVAL_SD_Exported_Functions SD Exported Functions
  * @{
  */   
uint8_t BSP_SD_Init(void);
uint8_t BSP_SD_DeInit(void);
uint8_t BSP_SD_ITConfig(void);
void    BSP_SD_DetectIT(void);
void    BSP_SD_DetectCallback(void);
uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout);
uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout);
uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks);
uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks);
uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr);
uint8_t BSP_SD_GetCardState(void);
void    BSP_SD_GetCardInfo(BSP_SD_CardInfo *CardInfo);
uint8_t BSP_SD_IsDetected(void);

/* These functions can be modified in case the current settings (e.g. DMA stream)
   need to be changed for specific application needs */
void    BSP_SD_MspInit(SD_HandleTypeDef *hsd, void *Params);
void    BSP_SD_MspDeInit(SD_HandleTypeDef *hsd, void *Params);
void    BSP_SD_AbortCallback(void);
void    BSP_SD_WriteCpltCallback(void);
void    BSP_SD_ReadCpltCallback(void);
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

#endif /* __STM32756G_EVAL_SD_H */

