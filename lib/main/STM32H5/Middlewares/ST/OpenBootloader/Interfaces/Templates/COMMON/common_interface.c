/**
  ******************************************************************************
  * @file    common_interface.c
  * @author  MCD Application Team
  * @brief   Contains common functions used by different interfaces
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "flash_interface.h"
#include "openbootloader_conf.h"
#include "common_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static Function_Pointer ResetCallback;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
  * @brief  Assigns the given value to the Main Stack Pointer (MSP).
  * @param  TopOfMainStack  Main Stack Pointer value to set.
  * @retval None.
  */
void Common_SetMsp(uint32_t TopOfMainStack)
{
}

/**
  * @brief  Enable IRQ Interrupts.
  * @retval None.
  */
void Common_EnableIrq(void)
{
}

/**
  * @brief  Disable IRQ Interrupts.
  * @retval None.
  */
void Common_DisableIrq(void)
{
}

/**
  * @brief  Checks whether the target Protection Status is set or not.
  * @retval Returns SET if protection is enabled else return RESET.
  */
FlagStatus Common_GetProtectionStatus(void)
{
  FlagStatus status;

  return status;
}

/**
  * @brief  Register a callback function to be called at the end of commands processing.
  * @retval None.
  */
void Common_SetPostProcessingCallback(Function_Pointer Callback)
{
  ResetCallback = Callback;
}

/**
  * @brief  Start post processing task.
  * @retval None.
  */
void Common_StartPostProcessing()
{
}
