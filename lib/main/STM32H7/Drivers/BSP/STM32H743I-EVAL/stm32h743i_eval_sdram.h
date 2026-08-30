/**
  ******************************************************************************
  * @file    stm32h743i_eval_sdram.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32h743i_eval_sdram.c driver.
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
#ifndef STM32H743I_EVAL_SDRAM_H
#define STM32H743I_EVAL_SDRAM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h743i_eval_conf.h"
#include "stm32h743i_eval_errno.h"
#include "../Components/is42s32800g/is42s32800g.h"
/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H743I_EVAL
  * @{
  */

/** @defgroup STM32H743I_EVAL_SDRAM SDRAM
  * @{
  */

/** @defgroup STM32H743I_EVAL_SDRAM_Exported_Types Exported Types
  * @{
  */
#if (USE_HAL_SDRAM_REGISTER_CALLBACKS == 1)
typedef struct
{
  void (* pMspInitCb)(SDRAM_HandleTypeDef *);
  void (* pMspDeInitCb)(SDRAM_HandleTypeDef *);
}BSP_SDRAM_Cb_t;
#endif /* (USE_HAL_SDRAM_REGISTER_CALLBACKS == 1) */
/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_SDRAM_Exported_Constants Exported Constants
  * @{
  */
#define SDRAM_INSTANCES_NBR                1U
#define SDRAM_DEVICE_ADDR                  0xD0000000U
#define SDRAM_DEVICE_SIZE                  0x02000000U  /* 32 MBytes */
#define SDRAM_MEMORY_WIDTH                 FMC_SDRAM_MEM_BUS_WIDTH_32

/* DMA definitions for SDRAM DMA transfer */
#define SDRAM_MDMAx_CLK_ENABLE             __HAL_RCC_MDMA_CLK_ENABLE
#define SDRAM_MDMAx_CLK_DISABLE            __HAL_RCC_MDMA_CLK_DISABLE
#define SDRAM_MDMAx_CHANNEL                MDMA_Channel0
#define SDRAM_MDMAx_IRQn                   MDMA_IRQn
#define SDRAM_MDMA_IRQHandler              MDMA_IRQHandler
/**
  * @}
  */

/** @defgroup STM32H743I_EVAL_SDRAM_Exported_Macro SDRAM Exported Macro
  * @{
  */
/**
  * @}
  */

/** @addtogroup STM32H743I_EVAL_SDRAM_Exported_Variables
  * @{
  */
extern SDRAM_HandleTypeDef hsdram[];
/**
  * @}
  */

/** @addtogroup STM32H743I_EVAL_SDRAM_Exported_Functions
  * @{
  */
int32_t BSP_SDRAM_Init(uint32_t Instance);
int32_t BSP_SDRAM_DeInit(uint32_t Instance);
#if (USE_HAL_SDRAM_REGISTER_CALLBACKS == 1)
int32_t BSP_SDRAM_RegisterDefaultMspCallbacks (uint32_t Instance)
int32_t BSP_SDRAM_RegisterMspCallbacks (uint32_t Instance, BSP_SDRAM_Cb_t *CallBacks)
#endif /* (USE_HAL_SDRAM_REGISTER_CALLBACKS == 1)  */
int32_t BSP_SDRAM_SendCmd(uint32_t Instance, FMC_SDRAM_CommandTypeDef *SdramCmd);
void BSP_SDRAM_IRQHandler(uint32_t Instance);
/* These functions can be modified in case the current settings need to be
changed for specific application needs */
HAL_StatusTypeDef MX_SDRAM_BANK2_Init(SDRAM_HandleTypeDef *hsdram);

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

#endif /* STM32H743I_EVAL_SDRAM_H */
