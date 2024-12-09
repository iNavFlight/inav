/**
  ******************************************************************************
  * @file    stm32f769i_eval_sram.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32f769i_eval_sram.c driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F769I_EVAL_SRAM_H
#define __STM32F769I_EVAL_SRAM_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup STM32F769I_EVAL
  * @{
  */
    
/** @addtogroup STM32F769I_EVAL_SRAM
  * @{
  */    

/** @defgroup STM32F769I_EVAL_SRAM_Exported_Types SRAM Exported Types
  * @{
  */
/**
  * @}
  */

/** @defgroup STM32F769I_EVAL_SRAM_Exported_Constants SRAM Exported Constants
  * @{
  */ 

/** 
  * @brief  SRAM status structure definition  
  */     
#define   SRAM_OK         ((uint8_t)0x00)
#define   SRAM_ERROR      ((uint8_t)0x01)

#define SRAM_DEVICE_ADDR  ((uint32_t)0x68000000)
#define SRAM_DEVICE_SIZE  ((uint32_t)0x200000)  /* SRAM device size in MBytes */  
  
/* #define SRAM_MEMORY_WIDTH    FMC_NORSRAM_MEM_BUS_WIDTH_8*/
#define SRAM_MEMORY_WIDTH    FMC_NORSRAM_MEM_BUS_WIDTH_16

#define SRAM_BURSTACCESS     FMC_BURST_ACCESS_MODE_DISABLE
/* #define SRAM_BURSTACCESS     FMC_BURST_ACCESS_MODE_ENABLE*/
  
#define SRAM_WRITEBURST      FMC_WRITE_BURST_DISABLE
/* #define SRAM_WRITEBURST     FMC_WRITE_BURST_ENABLE */
 
#define CONTINUOUSCLOCK_FEATURE    FMC_CONTINUOUS_CLOCK_SYNC_ONLY 
/* #define CONTINUOUSCLOCK_FEATURE     FMC_CONTINUOUS_CLOCK_SYNC_ASYNC */ 

/* DMA definitions for SRAM DMA transfer */
#define __SRAM_DMAx_CLK_ENABLE            __HAL_RCC_DMA2_CLK_ENABLE
#define __SRAM_DMAx_CLK_DISABLE           __HAL_RCC_DMA2_CLK_DISABLE
#define SRAM_DMAx_CHANNEL                 DMA_CHANNEL_0
#define SRAM_DMAx_STREAM                  DMA2_Stream4  
#define SRAM_DMAx_IRQn                    DMA2_Stream4_IRQn
#define BSP_SRAM_DMA_IRQHandler           DMA2_Stream4_IRQHandler  
/**
  * @}
  */ 
  
/** @defgroup STM32F769I_EVAL_SRAM_Exported_Macro SRAM Exported Macro
  * @{
  */  
/**
  * @}
  */ 
   
/** @defgroup STM32F769I_EVAL_SRAM_Exported_Functions SRAM Exported Functions
  * @{
  */    
uint8_t BSP_SRAM_Init(void);
uint8_t BSP_SRAM_DeInit(void);
uint8_t BSP_SRAM_ReadData(uint32_t uwStartAddress, uint16_t *pData, uint32_t uwDataSize);
uint8_t BSP_SRAM_ReadData_DMA(uint32_t uwStartAddress, uint16_t *pData, uint32_t uwDataSize);
uint8_t BSP_SRAM_WriteData(uint32_t uwStartAddress, uint16_t *pData, uint32_t uwDataSize);
uint8_t BSP_SRAM_WriteData_DMA(uint32_t uwStartAddress, uint16_t *pData, uint32_t uwDataSize);
   
/* These functions can be modified in case the current settings (e.g. DMA stream)
   need to be changed for specific application needs */
void    BSP_SRAM_MspInit(SRAM_HandleTypeDef  *hsram, void *Params);
void    BSP_SRAM_MspDeInit(SRAM_HandleTypeDef  *hsram, void *Params);

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

#endif /* __STM32F769I_EVAL_SRAM_H */

