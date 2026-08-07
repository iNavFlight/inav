/**
  ******************************************************************************
  * @file    stm32h7b3i_eval_sram.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32h7b3i_eval_sram.c driver.
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
#ifndef STM32H7B3I_EVAL_SRAM_H
#define STM32H7B3I_EVAL_SRAM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7b3i_eval_conf.h"
#include "stm32h7b3i_eval_errno.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H7B3I_EVAL
  * @{
  */

/** @defgroup STM32H7B3I_EVAL_SRAM SRAM
  * @{
  */

/** @defgroup STM32H7B3I_EVAL_SRAM_Exported_Types SRAM Exported Types
  * @{
  */
#if (USE_HAL_SRAM_REGISTER_CALLBACKS == 1)
typedef struct
{
  void (* pMspInitCb)(SRAM_HandleTypeDef *);
  void (* pMspDeInitCb)(SRAM_HandleTypeDef *);
} BSP_SRAM_Cb_t;
#endif /* (USE_HAL_SRAM_REGISTER_CALLBACKS == 1) */
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_SRAM_Exported_Constants SRAM Exported Constants
  * @{
  */
#define SRAM_INSTANCES_NBR              1U

/**
  * @brief  SRAM status structure definition
  */
#define SRAM_DEVICE_ADDR  0x68000000U
#define SRAM_DEVICE_SIZE  0x200000U

/* DMA definitions for SRAM DMA transfer */
#define SRAM_MDMAx_CLK_ENABLE              __HAL_RCC_MDMA_CLK_ENABLE
#define SRAM_MDMAx_CLK_DISABLE             __HAL_RCC_MDMA_CLK_DISABLE
#define SRAM_MDMAx_CHANNEL                 MDMA_Channel0
#define SRAM_MDMAx_IRQn                    MDMA_IRQn
#define SRAM_MDMA_IRQHandler               MDMA_IRQHandler

/**
  * @}
  */

/** @addtogroup STM32H7B3I_EVAL_SRAM_Exported_Variables
  * @{
  */
extern SRAM_HandleTypeDef hsram[SRAM_INSTANCES_NBR];
/**
  * @}
  */

/** @addtogroup STM32H7B3I_EVAL_SRAM_Exported_Functions
  * @{
  */
int32_t BSP_SRAM_Init(uint32_t Instance);
int32_t BSP_SRAM_DeInit(uint32_t Instance);
#if (USE_HAL_SRAM_REGISTER_CALLBACKS == 1)
int32_t BSP_SRAM_RegisterDefaultMspCallbacks(uint32_t Instance);
int32_t BSP_SRAM_RegisterMspCallbacks(uint32_t Instance, BSP_SRAM_Cb_t *CallBacks);
#endif /* (USE_HAL_SRAM_REGISTER_CALLBACKS == 1)  */

void BSP_SRAM_IRQHandler(uint32_t Instance);

/* These functions can be modified in case the current settings
   need to be changed for specific application needs */
HAL_StatusTypeDef MX_SRAM_BANK3_Init(SRAM_HandleTypeDef *hSram);

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

#endif /*STM32H7B3I_EVAL_SRAM_H */
