/**
  ******************************************************************************
  * @file    gt911.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the
  *          gt911.c driver.
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
#ifndef GT911_H
#define GT911_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "gt911_reg.h"
#include <stddef.h>
#include "gt911_conf.h"

/* Macros --------------------------------------------------------------------*/

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @addtogroup GT911
  * @{
  */

/** @defgroup GT911_Exported_Constants GT911 Exported Constants
  * @{
  */
#define GT911_OK                      (0)
#define GT911_ERROR                   (-1)

/* Max detectable simultaneous touches */
#define GT911_MAX_NB_TOUCH             5U

/* Touch GT911 IDs: "911" */
#define GT911_ID                        0x00313139U
#define GT911_ID1                       0x39U
#define GT911_ID2                       0x31U
#define GT911_ID3                       0x31U
/* Possible values of GT911_DEV_MODE_REG */
#define GT911_DEV_MODE_WORKING          0x00U
#define GT911_DEV_MODE_FACTORY          0x04U

/* Possible values of GT911_GEST_ID_REG */
#define GT911_GEST_ID_NO_GESTURE        0x00U
#define GT911_GEST_ID_SWIPE_RIGHT       0xAAU
#define GT911_GEST_ID_SWIPE_LEFT        0xBBU
#define GT911_GEST_ID_SWIPE_DOWN        0xABU
#define GT911_GEST_ID_SWIPE_UP          0xBAU
#define GT911_GEST_ID_DOUBLE_TAP        0xCCU

/* Values Pn_XH and Pn_YH related */
#define GT911_TOUCH_EVT_FLAG_PRESS_DOWN 0x00U
#define GT911_TOUCH_EVT_FLAG_LIFT_UP    0x01U
#define GT911_TOUCH_EVT_FLAG_CONTACT    0x02U
#define GT911_TOUCH_EVT_FLAG_NO_EVENT   0x03U

/* Possible values of GT911_MSW1_REG(Module_Switch1) */
#define GT911_M_SW1_INTERRUPT_RISING    0x00U
#define GT911_M_SW1_INTERRUPT_FALLING   0x01U
#define GT911_M_SW1_INTERRUPT_LOW       0x02U
#define GT911_M_SW1_INTERRUPT_HIGH      0x03U

/* Mask for reading the MSW1 register without INT trigger */
#define GT911_M_SW1_DATA_MASK           0xFCU

/**
  * @}
  */

/* Exported types ------------------------------------------------------------*/

/** @defgroup GT911_Exported_Types GT911 Exported Types
  * @{
  */
typedef struct
{
  uint32_t  DistanceLeftRight;
  uint32_t  DistanceUpDown;
  uint8_t   RefreshRate;
  uint8_t   GestureThreshold;
  uint8_t   Gain;
} GT911_Gesture_Init_t;

typedef int32_t (*GT911_Init_Func)(void);
typedef int32_t (*GT911_DeInit_Func)(void);
typedef int32_t (*GT911_GetTick_Func)(void);
typedef int32_t (*GT911_WriteReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);
typedef int32_t (*GT911_ReadReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);

typedef struct
{
  GT911_Init_Func          Init;
  GT911_DeInit_Func        DeInit;
  uint16_t                 Address;
  GT911_WriteReg_Func      WriteReg;
  GT911_ReadReg_Func       ReadReg;
  GT911_GetTick_Func       GetTick;
} GT911_IO_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX;
  uint32_t  TouchY;
} GT911_State_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX[GT911_MAX_NB_TOUCH];
  uint32_t  TouchY[GT911_MAX_NB_TOUCH];
  uint32_t  TouchWeight[GT911_MAX_NB_TOUCH];
  uint32_t  TouchTrackID[GT911_MAX_NB_TOUCH];
} GT911_MultiTouch_State_t;

typedef struct
{
  GT911_IO_t          IO;
  gt911_ctx_t         Ctx;
  uint8_t             IsInitialized;
  uint8_t             Trigger;
} GT911_Object_t;
typedef struct
{
  uint8_t   MultiTouch;
  uint8_t   Gesture;
  uint8_t   MaxTouch;
  uint32_t  MaxXl;
  uint32_t  MaxYl;
} GT911_Capabilities_t;

typedef struct
{
  int32_t (*Init)(GT911_Object_t *);
  int32_t (*DeInit)(GT911_Object_t *);
  int32_t (*GestureConfig)(GT911_Object_t *, GT911_Gesture_Init_t *);
  int32_t (*ReadID)(GT911_Object_t *, uint32_t *);
  int32_t (*GetState)(GT911_Object_t *, GT911_State_t *);
  int32_t (*GetMultiTouchState)(GT911_Object_t *, GT911_MultiTouch_State_t *);
  int32_t (*GetGesture)(GT911_Object_t *, uint8_t *);
  int32_t (*GetCapabilities)(GT911_Object_t *, GT911_Capabilities_t *);
  int32_t (*EnableIT)(GT911_Object_t *);
  int32_t (*DisableIT)(GT911_Object_t *);
  int32_t (*ClearIT)(GT911_Object_t *);
  int32_t (*ITStatus)(GT911_Object_t *);
} GT911_TS_Drv_t;
/**
  * @}
  */

/** @addtogroup GT911_Exported_Variables
  * @{
  */
extern GT911_TS_Drv_t GT911_TS_Driver; /* Derogation MISRAC2012-Rule-8.9_b */
/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup GT911_Exported_Functions
  * @{
  */

int32_t GT911_RegisterBusIO(GT911_Object_t *pObj, GT911_IO_t *pIO);
int32_t GT911_Init(GT911_Object_t *pObj);
int32_t GT911_DeInit(GT911_Object_t *pObj);
int32_t GT911_GestureConfig(GT911_Object_t *pObj, GT911_Gesture_Init_t *pGestureInit);
int32_t GT911_ReadID(GT911_Object_t *pObj, uint32_t *pId);
int32_t GT911_GetState(GT911_Object_t *pObj, GT911_State_t *pState);
int32_t GT911_GetMultiTouchState(GT911_Object_t *pObj, GT911_MultiTouch_State_t *pState);
int32_t GT911_GetGesture(GT911_Object_t *pObj, uint8_t *pGestureId);
int32_t GT911_SetTriggerMode(GT911_Object_t *pObj, uint8_t Trigger);
int32_t GT911_GetTriggerMode(GT911_Object_t *pObj, uint8_t *pTrigger);
int32_t GT911_EnableIT(GT911_Object_t *pObj);
int32_t GT911_DisableIT(GT911_Object_t *pObj);
int32_t GT911_ITStatus(GT911_Object_t *pObj);
int32_t GT911_ClearIT(GT911_Object_t *pObj);
int32_t GT911_GetCapabilities(GT911_Object_t *pObj, GT911_Capabilities_t *pCapabilities);

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
#endif /* GT911_H */
