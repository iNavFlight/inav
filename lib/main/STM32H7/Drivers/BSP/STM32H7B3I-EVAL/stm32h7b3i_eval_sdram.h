/**
  ******************************************************************************
  * @file    stm32h7b3i_eval_sdram.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32h7b3i_eval_sdram.c driver.
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
#ifndef STM32H7B3I_EVAL_SDRAM_H
#define STM32H7B3I_EVAL_SDRAM_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7b3i_eval_conf.h"
#include "stm32h7b3i_eval_errno.h"
#include "../Components/is42s32800j/is42s32800j.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H7B3I_EVAL
  * @{
  */

/** @defgroup STM32H7B3I_EVAL_SDRAM SDRAM
  * @{
  */

/** @defgroup STM32H7B3I_EVAL_SDRAM_Exported_Types SDRAM Exported Types
  * @{
  */
typedef enum
{
  SDRAM_DATA_LENGTH_16B,
  SDRAM_DATA_LENGTH_32B
} SDRAM_DataLength_t;

#if (USE_HAL_SDRAM_REGISTER_CALLBACKS == 1)
typedef struct
{
  void (* pMspInitCb)(SDRAM_HandleTypeDef *);
  void (* pMspDeInitCb)(SDRAM_HandleTypeDef *);
} BSP_SDRAM_Cb_t;
#endif /* (USE_HAL_SDRAM_REGISTER_CALLBACKS == 1) */
/**
  * @}
  */


/** @defgroup STM32H7B3I_EVAL_SDRAM_Exported_Constants SDRAM Exported Constants
  * @{
  */
#define SDRAM_INSTANCES_NBR       1U
#define SDRAM_DEVICE_ADDR         0xD0000000U
#define SDRAM_DEVICE_SIZE         0x02000000U  /* 32 MBytes */

/* DMA definitions for SDRAM DMA transfer */
#define SDRAM_MDMAx_CLK_ENABLE             __HAL_RCC_MDMA_CLK_ENABLE
#define SDRAM_MDMAx_CLK_DISABLE            __HAL_RCC_MDMA_CLK_DISABLE
#define SDRAM_MDMAx_CHANNEL                MDMA_Channel0
#define SDRAM_MDMAx_IRQn                   MDMA_IRQn
#define SDRAM_MDMA_IRQHandler              MDMA_IRQHandler


/**
  * @brief  FMC SDRAM Mode definition register defines
  */
#define SDRAM_MODEREG_BURST_LENGTH_1             0x0000U
#define SDRAM_MODEREG_BURST_LENGTH_2             0x0001U
#define SDRAM_MODEREG_BURST_LENGTH_4             0x0002U
#define SDRAM_MODEREG_BURST_LENGTH_8             0x0004U
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      0x0000U
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     0x0008U
#define SDRAM_MODEREG_CAS_LATENCY_2              0x0020U
#define SDRAM_MODEREG_CAS_LATENCY_3              0x0030U
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    0x0000U
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED 0x0000U
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     0x0200U
/**
  * @}
  */

/** @addtogroup STM32H7B3I_EVAL_SDRAM_Exported_Variables
  * @{
  */
extern SDRAM_HandleTypeDef hsdram[];
/**
  * @}
  */
/** @addtogroup STM32H7B3I_EVAL_SDRAM_Exported_Functions
  * @{
  */
int32_t BSP_SDRAM_Init(uint32_t Instance);
int32_t BSP_SDRAM_InitEx(uint32_t Instance, SDRAM_DataLength_t DataLength);
int32_t BSP_SDRAM_DeInit(uint32_t Instance);
#if (USE_HAL_SDRAM_REGISTER_CALLBACKS == 1)
int32_t BSP_SDRAM_RegisterDefaultMspCallbacks(uint32_t Instance);
int32_t BSP_SDRAM_RegisterMspCallbacks(uint32_t Instance, BSP_SDRAM_Cb_t *CallBacks);
#endif /* (USE_HAL_SDRAM_REGISTER_CALLBACKS == 1)  */
int32_t BSP_SDRAM_SendCmd(uint32_t Instance, FMC_SDRAM_CommandTypeDef *SdramCmd);
void BSP_SDRAM_IRQHandler(uint32_t Instance);

/* These functions can be modified in case the current settings need to be
   changed for specific application needs */
HAL_StatusTypeDef MX_SDRAM_BANK2_Init(SDRAM_HandleTypeDef *hSdram, uint32_t RowBitsNumber, uint32_t MemoryDataWidth);

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

#endif /* STM32H7B3I_EVAL_SDRAM_H */
