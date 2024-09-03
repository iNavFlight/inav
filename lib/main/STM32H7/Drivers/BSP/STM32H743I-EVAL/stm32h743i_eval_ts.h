/**
  ******************************************************************************
  * @file    stm32h743i_eval_ts.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32h743i_eval_ts.c driver.
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
#ifndef STM32H743I_EVAL_TS_H
#define STM32H743I_EVAL_TS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h743i_eval_conf.h"
#include "stm32h743i_eval_errno.h"

#include "../Components/exc7200/exc7200.h"
#include "../Components/ts3510/ts3510.h"
#include "../Components/Common/ts.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32H743I_EVAL
  * @{
  */

/** @defgroup STM32H743I_EVAL_TS TS
  * @{
  */

/** @defgroup STM32H7B3I_EVAL_TS_Exported_Constants TS Exported Constants
  * @{
  */
#define TS_INSTANCES_NBR            1U

#ifndef USE_TS_MULTI_TOUCH
  #define USE_TS_MULTI_TOUCH        1U
#endif

#ifndef TS_TOUCH_NBR
  #define TS_TOUCH_NBR              2U
#endif

/* Maximum border values of the touchscreen pad */
#define TS_MAX_WIDTH                640U     /* Touchscreen pad max width   */
#define TS_MAX_HEIGHT               480U     /* Touchscreen pad max height  */

#define TS_SWAP_NONE                0x01U
#define TS_SWAP_X                   0x02U
#define TS_SWAP_Y                   0x04U
#define TS_SWAP_XY                  0x08U

/**
  * @brief TouchScreen Slave I2C address 1
  */
#define TS_EXC7200_I2C_ADDRESS      0x08U
#define TS_TS3510_I2C_ADDRESS       0x80U

/**
  * @brief Touch screen interrupt signal
  */
#if (USE_BSP_IO_CLASS > 0)
#define TS_INT_PIN                   IO_PIN_14
#endif /* ( USE_BSP_IO_CLASS > 0) */
#define TS_INT_EXTI_IRQn             EXTI9_5_IRQn
#define TS_INT_LINE                  EXTI_LINE_8
/**
  * @}
  */

/** @defgroup STM32H7B3I_EVAL_TS_Exported_Types  TS Exported Types
  * @{
  */
typedef struct
{
  uint32_t   Width;                  /* Screen Width */
  uint32_t   Height;                 /* Screen Height */
  uint32_t   Orientation;            /* Touch Screen orientation from the upper left position  */
  uint32_t   Accuracy;               /* Expressed in pixel and means the x or y difference vs old
                                        position to consider the new values valid */
}TS_Init_t;

typedef struct
{
  uint32_t   Width;
  uint32_t   Height;
  uint32_t   Orientation;
  uint32_t   Accuracy;
  uint32_t   MaxX;
  uint32_t   MaxY;
  uint32_t   PreviousX[TS_TOUCH_NBR];
  uint32_t   PreviousY[TS_TOUCH_NBR];
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

/**
  * @}
  */

/** @addtogroup STM32H7B3I_EVAL_TS_Exported_Variables
  * @{
  */
extern void               *Ts_CompObj[];
extern EXTI_HandleTypeDef hts_exti[];
extern TS_Ctx_t           Ts_Ctx[];
/**
  * @}
  */

/** @addtogroup STM32H7B3I_EVAL_TS_Exported_Functions
  * @{
  */
int32_t BSP_TS_Init(uint32_t Instance, TS_Init_t *TS_Init);
int32_t BSP_TS_DeInit(uint32_t Instance);
int32_t BSP_TS_EnableIT(uint32_t Instance);
int32_t BSP_TS_DisableIT(uint32_t Instance);
int32_t BSP_TS_GetState(uint32_t Instance, TS_State_t *TS_State);
int32_t BSP_TS_Set_Orientation(uint32_t Instance, uint32_t Orientation);
int32_t BSP_TS_Get_Orientation(uint32_t Instance, uint32_t *Orientation);
int32_t BSP_TS_GetCapabilities(uint32_t Instance, TS_Capabilities_t *Capabilities);
void    BSP_TS_Callback(uint32_t Instance);
void    BSP_TS_IRQHandler(uint32_t Instance);
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

#endif /* STM32H743I_EVAL_TS_H */
