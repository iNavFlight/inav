/**
  ******************************************************************************
  * @file    iwdg_interface.c
  * @author  MCD Application Team
  * @brief   Contains IWDG HW configuration
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
#include "main.h"
#include "iwdg_interface.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static IWDG_HandleTypeDef IWDGHandle;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
  * @brief  This function is used to configure the watchdog.
  * @retval None.
  */
void OPENBL_IWDG_Configuration(void)
{
  IWDGHandle.Instance       = IWDG;
  IWDGHandle.Init.Prescaler = IWDG_PRESCALER_256;
  IWDGHandle.Init.Window    = IWDG_WINDOW_DISABLE;
  IWDGHandle.Init.Reload    = IWDG_KEY_RELOAD;

  /* In case the user has enabled the IWDG through HW before entering the Open Bootloader */
  IWDG->KR = IWDG_KEY_WRITE_ACCESS_ENABLE;
  IWDG->PR = IWDG_PRESCALER_256;
  IWDG->KR = IWDG_KEY_RELOAD;
}

/**
  * @brief  This function is used to refresh the watchdog.
  * @retval None.
  */
void OPENBL_IWDG_Refresh(void)
{
  /* Refresh IWDG: reload counter */
  if (HAL_IWDG_Refresh(&IWDGHandle) != HAL_OK)
  {
    /* Refresh Error */
    Error_Handler();
  }
}
