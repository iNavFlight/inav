/**
  ******************************************************************************
  * @file    stm32h573i_discovery_ts.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32h573i_discovery_ts.c driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef STM32H573I_DK_TS_H
#define STM32H573I_DK_TS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h573i_discovery_conf.h"
#include "stm32h573i_discovery_errno.h"
#include "../Components/Common/ts.h"
#include "../Components/ft6x06/ft6x06.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H573I_DK
  * @{
  */

/** @addtogroup STM32H573I_DK_TS
  * @{
  */

/** @defgroup STM32H573I_DK_TS_Exported_Constants TS Exported Constants
  * @{
  */

#ifndef USE_TS_MULTI_TOUCH
#define USE_TS_MULTI_TOUCH          1U
#endif
#ifndef USE_TS_GESTURE
#define USE_TS_GESTURE              1U
#endif

#ifndef TS_TOUCH_NBR
#define TS_TOUCH_NBR                2U
#endif

/* TS instances */
#define TS_INSTANCES_NBR 1U

/* Maximum border values of the touchscreen pad */
#define TS_MAX_WIDTH                FT6X06_MAX_X_LENGTH     /* Touchscreen pad max width   */
#define TS_MAX_HEIGHT               FT6X06_MAX_Y_LENGTH     /* Touchscreen pad max height  */

#define TS_SWAP_NONE                0x01U
#define TS_SWAP_X                   0x02U
#define TS_SWAP_Y                   0x04U
#define TS_SWAP_XY                  0x08U

/* TS orientations */
#define TS_ORIENTATION_PORTRAIT_ROT180   0U
#define TS_ORIENTATION_LANDSCAPE_ROT180  1U
#define TS_ORIENTATION_PORTRAIT          2U
#define TS_ORIENTATION_LANDSCAPE         3U
    
/* TS I2C address */
#define TS_I2C_ADDRESS              0x70U

/**
  * @brief Touch screen interrupt signal
  */
#define TS_INT_PIN                   GPIO_PIN_7
#define TS_INT_GPIO_PORT             GPIOG
#define TS_INT_GPIO_CLK_ENABLE()     __HAL_RCC_GPIOG_CLK_ENABLE()
#define TS_INT_GPIO_CLK_DISABLE()    __HAL_RCC_GPIOG_CLK_DISABLE()
#define TS_INT_EXTI_IRQn             EXTI7_IRQn
#define TS_EXTI_LINE                 EXTI_LINE_7

/**
  * @brief Touch screen Reset signal
  */
#define TS_RESET_GPIO_PORT           GPIOG
#define TS_RESET_GPIO_PIN            GPIO_PIN_3
#define TS_RESET_GPIO_CLK_ENABLE() __HAL_RCC_GPIOG_CLK_ENABLE()

/**
  * @}
  */


/** @defgroup STM32H573I_DK_TS_Exported_Types TS Exported Types
  * @{
  */
typedef struct
{
  uint32_t   Width;                  /* Screen width */
  uint32_t   Height;                 /* Screen height */
  uint32_t   Orientation;            /* Touch screen orientation */
  uint32_t   Accuracy;               /* Expressed in pixel and means the x or y difference vs old
                                        position to consider the new values valid */
} TS_Init_t;

typedef struct
{
  uint32_t   Width;
  uint32_t   Height;
  uint32_t   Orientation;
  uint32_t   Accuracy;
  uint32_t   MaxX;
  uint32_t   MaxY;
  uint32_t   PrevX[TS_TOUCH_NBR];    /* Previous X positions */
  uint32_t   PrevY[TS_TOUCH_NBR];    /* Previous Y positions */
} TS_Ctx_t;

typedef struct
{
  uint8_t   MultiTouch;
  uint8_t   Gesture;
  uint8_t   MaxTouch;
  uint32_t  MaxXl;
  uint32_t  MaxYl;
} TS_Capabilities_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX;
  uint32_t  TouchY;
} TS_State_t;

#if (USE_TS_MULTI_TOUCH > 0)
typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX[TS_TOUCH_NBR];
  uint32_t  TouchY[TS_TOUCH_NBR];
  uint32_t  TouchWeight[TS_TOUCH_NBR];
  uint32_t  TouchEvent[TS_TOUCH_NBR];
  uint32_t  TouchArea[TS_TOUCH_NBR];
} TS_MultiTouch_State_t;
#endif /* USE_TS_MULTI_TOUCH > 0 */

#if (USE_TS_GESTURE > 0)
typedef struct
{
  uint32_t  Radian;
  uint32_t  OffsetLeftRight;
  uint32_t  OffsetUpDown;
  uint32_t  DistanceLeftRight;
  uint32_t  DistanceUpDown;
  uint32_t  DistanceZoom;
} TS_Gesture_Config_t;
#endif /* (USE_TS_GESTURE > 0) */

/**
  * @}
  */

/** @defgroup STM32H573I_DK_TS_Exported_Variables TS Exported Variables
  * @{
  */
extern void               *Ts_CompObj[TS_INSTANCES_NBR];
extern EXTI_HandleTypeDef hts_exti[TS_INSTANCES_NBR];
extern TS_Ctx_t           Ts_Ctx[TS_INSTANCES_NBR];
/**
  * @}
  */

/** @defgroup STM32H573I_DK_TS_Exported_Functions TS Exported Functions
  * @{
  */
int32_t  BSP_TS_Init(uint32_t Instance, TS_Init_t *TS_Init);
int32_t  BSP_TS_DeInit(uint32_t Instance);
int32_t  BSP_TS_EnableIT(uint32_t Instance);
int32_t  BSP_TS_DisableIT(uint32_t Instance);
int32_t  BSP_TS_Set_Orientation(uint32_t Instance, uint32_t Orientation);
int32_t  BSP_TS_Get_Orientation(uint32_t Instance, uint32_t *Orientation);
int32_t  BSP_TS_GetState(uint32_t Instance, TS_State_t *TS_State);
#if (USE_TS_MULTI_TOUCH > 0)
int32_t  BSP_TS_Get_MultiTouchState(uint32_t Instance, TS_MultiTouch_State_t *TS_State);
#endif /* USE_TS_MULTI_TOUCH > 0 */
#if (USE_TS_GESTURE > 0)
int32_t  BSP_TS_GestureConfig(uint32_t Instance, const TS_Gesture_Config_t *GestureConfig);
int32_t  BSP_TS_GetGestureId(uint32_t Instance, const uint32_t *GestureId);
#endif /* (USE_TS_GESTURE > 0) */
int32_t  BSP_TS_GetCapabilities(uint32_t Instance, TS_Capabilities_t *Capabilities);
void     BSP_TS_Callback(uint32_t Instance);
void     BSP_TS_IRQHandler(uint32_t Instance);
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

#endif /* STM32H573I_DK_TS_H */
