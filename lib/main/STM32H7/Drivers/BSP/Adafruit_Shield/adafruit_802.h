/**
  ******************************************************************************
  * @file    adafruit_802.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for:
  *          - Joystick available on Adafruit 1.8" TFT LCD shield (reference ID 802)
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ADAFRUIT_802_H
#define ADAFRUIT_802_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "adafruit_802_conf.h"

/** @addtogroup BSP
  * @{
  */

/** @defgroup ADAFRUIT_802 ADAFRUIT_802
  * @{
  */

/** @defgroup ADAFRUIT_802_LOW_LEVEL STM32 ADAFRUIT_802 LOW LEVEL
  * @brief This file provides set of firmware functions to manage joystick
  *        available on Adafruit shield.
  * @{
  */

/** @defgroup ADAFRUIT_802_LOW_LEVEL_Exported_Types LOW LEVEL Exported Types
  * @{
  */
typedef enum
{
 JOY1 = 0U,
 JOYn
}JOY_TypeDef;

typedef enum
{
  JOY_MODE_GPIO = 0U,
  JOY_MODE_EXTI = 1U,
}JOYMode_TypeDef;

typedef enum
{
 JOY_NONE  = 0x00U,
 JOY_SEL   = 0x01U,
 JOY_DOWN  = 0x02U,
 JOY_LEFT  = 0x04U,
 JOY_RIGHT = 0x08U,
 JOY_UP    = 0x10U,
 JOY_ALL   = 0x1FU
}JOYPin_TypeDef;

#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
typedef struct
{
  pADC_CallbackTypeDef  pMspInitCb;
  pADC_CallbackTypeDef  pMspDeInitCb;
}ADAFRUIT_802_JOY_Cb_t;
#endif /* (USE_HAL_ADC_REGISTER_CALLBACKS == 1) */

/**
  * @}
  */

/** @defgroup ADAFRUIT_802_LOW_LEVEL_Exported_Functions LOW LEVEL Exported_Functions
  * @{
  */
int32_t ADAFRUIT_802_JOY_Init(JOY_TypeDef JOY, JOYMode_TypeDef JoyMode, JOYPin_TypeDef JoyPins);
int32_t ADAFRUIT_802_JOY_DeInit(JOY_TypeDef JOY, JOYPin_TypeDef JoyPins);
int32_t ADAFRUIT_802_JOY_GetState(JOY_TypeDef JOY);
#if (USE_HAL_ADC_REGISTER_CALLBACKS == 1)
int32_t ADAFRUIT_802_JOY_RegisterDefaultMspCallbacks(JOY_TypeDef JOY);
int32_t ADAFRUIT_802_JOY_RegisterMspCallbacks(JOY_TypeDef JOY, ADAFRUIT_802_JOY_Cb_t *Callback);
#endif /* (USE_HAL_ADC_REGISTER_CALLBACKS == 1) */
HAL_StatusTypeDef MX_ADAFRUIT_802_ADC_Init(ADC_HandleTypeDef *hadc);

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

#endif /* ADAFRUIT_802_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
