/**
  ******************************************************************************
  * @file   stm32h747i_eval_nor.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          thestm32h747i_eval_nor.c driver.
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
#ifndef STM32H747I_EVAL_NOR_H
#define STM32H747I_EVAL_NOR_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h747i_eval_conf.h"
#include "stm32h747i_eval_errno.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H747I_EVAL
  * @{
  */

/** @addtogroup STM32H747I_EVAL_NOR
  * @{
  */

/** @defgroup STM32H747I_EVAL_NOR_Exported_Types Exported Types
  * @{
  */
#if (USE_HAL_NOR_REGISTER_CALLBACKS == 1)
typedef struct
{
  void (* pMspInitCb)(NOR_HandleTypeDef *);
  void (* pMspDeInitCb)(NOR_HandleTypeDef *);
}BSP_NOR_Cb_t;
#endif /* (USE_HAL_NOR_REGISTER_CALLBACKS == 1) */
/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_NOR_Exported_Constants Exported Constants
  * @{
  */
#define NOR_INSTANCES_NBR  1U

#define NOR_DEVICE_ADDR  0x60000000U
#define NOR_DEVICE_SIZE  0x4000000U

/* NOR operations Timeout definitions */
#define BLOCKERASE_TIMEOUT   0x00A00000U  /* NOR block erase timeout */
#define CHIPERASE_TIMEOUT    0x30000000U  /* NOR chip erase timeout  */
#define PROGRAM_TIMEOUT      0x00004400U  /* NOR program timeout     */

/* NOR Ready/Busy signal GPIO definitions */
#define NOR_READY_BUSY_PIN    GPIO_PIN_6
#define NOR_READY_BUSY_GPIO   GPIOC
#define NOR_READY_STATE       GPIO_PIN_SET
#define NOR_BUSY_STATE        GPIO_PIN_RESET

/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_NOR_Exported_Macro Exported Macro
  * @{
  */
extern NOR_HandleTypeDef hnor[];
/**
  * @}
  */

/** @defgroup STM32H747I_EVAL_NOR_Exported_Functions Exported Functions
  * @{
  */
int32_t BSP_NOR_Init(uint32_t Instance);
int32_t BSP_NOR_DeInit(uint32_t Instance);
#if (USE_HAL_NOR_REGISTER_CALLBACKS == 1)
int32_t BSP_NOR_RegisterDefaultMspCallbacks (uint32_t Instance);
int32_t BSP_NOR_RegisterMspCallbacks (uint32_t Instance, BSP_NOR_Cb_t *Callbacks);
#endif /* (USE_HAL_NOR_REGISTER_CALLBACKS == 1) */
int32_t BSP_NOR_ReadData(uint32_t Instance, uint32_t uwStartAddress, uint16_t *pData, uint32_t uwDataSize);
int32_t BSP_NOR_WriteData(uint32_t Instance, uint32_t uwStartAddress, uint16_t *pData, uint32_t uwDataSize);
int32_t BSP_NOR_ProgramData(uint32_t Instance, uint32_t uwStartAddress, uint16_t *pData, uint32_t uwDataSize);
int32_t BSP_NOR_EraseBlock(uint32_t Instance, uint32_t BlockAddress);
int32_t BSP_NOR_EraseChip(uint32_t Instance);
int32_t BSP_NOR_ReadID(uint32_t Instance, NOR_IDTypeDef *pNOR_ID);
int32_t BSP_NOR_ReturnToReadMode(uint32_t Instance);

/* These functions can be modified in case the current settings
   need to be changed for specific application needs */
HAL_StatusTypeDef MX_NOR_Init(NOR_HandleTypeDef *hnor);

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

#endif /* STM32H747I_EVAL_NOR_H */
