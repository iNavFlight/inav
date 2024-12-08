/**
  ******************************************************************************
  * @file    ft6x06.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the
  *          ft6x06.c IO expander driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef FT6X06_H
#define FT6X06_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ft6x06_reg.h"
#include <stddef.h>
#include "ft6x06_conf.h"
  
/* Macros --------------------------------------------------------------------*/

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Component
 * @{
 */

/** @addtogroup FT6X06
 * @{
 */

/** @defgroup FT6X06_Exported_Constants FT6X06 Exported Constants
 * @{
 */
#define FT6X06_OK                      (0)
#define FT6X06_ERROR                   (-1)

/* Max detectable simultaneous touches */
#define FT6X06_MAX_NB_TOUCH             2U
  
/* Touch FT6XX6 IDs */
#define FT6X06_ID                        0x11U
#define FT6X36_ID                        0xCDU

/* Possible values of FT6X06_DEV_MODE_REG */
#define FT6X06_DEV_MODE_WORKING          0x00U
#define FT6X06_DEV_MODE_FACTORY          0x04U
     
/* Possible values of FT6X06_GEST_ID_REG */
#define FT6X06_GEST_ID_NO_GESTURE        0x00U
#define FT6X06_GEST_ID_MOVE_UP           0x10U
#define FT6X06_GEST_ID_MOVE_RIGHT        0x14U
#define FT6X06_GEST_ID_MOVE_DOWN         0x18U
#define FT6X06_GEST_ID_MOVE_LEFT         0x1CU
#define FT6X06_GEST_ID_ZOOM_IN           0x48U
#define FT6X06_GEST_ID_ZOOM_OUT          0x49U
     
/* Values Pn_XH and Pn_YH related */
#define FT6X06_TOUCH_EVT_FLAG_PRESS_DOWN 0x00U
#define FT6X06_TOUCH_EVT_FLAG_LIFT_UP    0x01U
#define FT6X06_TOUCH_EVT_FLAG_CONTACT    0x02U
#define FT6X06_TOUCH_EVT_FLAG_NO_EVENT   0x03U

/* Possible values of FT6X06_GMODE_REG */
#define FT6X06_G_MODE_INTERRUPT_POLLING  0x00U
#define FT6X06_G_MODE_INTERRUPT_TRIGGER  0x01U

/**
 * @}
 */

/* Exported types ------------------------------------------------------------*/

/** @defgroup FT6X06_Exported_Types FT6X06 Exported Types
 * @{
 */
typedef struct
{ 
  uint32_t  Radian;
  uint32_t  OffsetLeftRight;
  uint32_t  OffsetUpDown;
  uint32_t  DistanceLeftRight;
  uint32_t  DistanceUpDown;
  uint32_t  DistanceZoom;  
}FT6X06_Gesture_Init_t;

typedef int32_t (*FT6X06_Init_Func)    (void);
typedef int32_t (*FT6X06_DeInit_Func)  (void);
typedef int32_t (*FT6X06_GetTick_Func) (void);
typedef int32_t (*FT6X06_Delay_Func)   (uint32_t);
typedef int32_t (*FT6X06_WriteReg_Func)(uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*FT6X06_ReadReg_Func) (uint16_t, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  FT6X06_Init_Func          Init;
  FT6X06_DeInit_Func        DeInit;
  uint16_t                  Address;  
  FT6X06_WriteReg_Func      WriteReg;
  FT6X06_ReadReg_Func       ReadReg; 
  FT6X06_GetTick_Func       GetTick; 
} FT6X06_IO_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX;
  uint32_t  TouchY;
} FT6X06_State_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX[FT6X06_MAX_NB_TOUCH];
  uint32_t  TouchY[FT6X06_MAX_NB_TOUCH];
  uint32_t  TouchWeight[FT6X06_MAX_NB_TOUCH];
  uint32_t  TouchEvent[FT6X06_MAX_NB_TOUCH];
  uint32_t  TouchArea[FT6X06_MAX_NB_TOUCH];
} FT6X06_MultiTouch_State_t;

typedef struct
{
  FT6X06_IO_t         IO;
  ft6x06_ctx_t        Ctx;   
  uint8_t             IsInitialized;
} FT6X06_Object_t;

typedef struct 
{       
  uint8_t   MultiTouch;      
  uint8_t   Gesture;
  uint8_t   MaxTouch;
  uint32_t  MaxXl;
  uint32_t  MaxYl;
} FT6X06_Capabilities_t;

 typedef struct
{
  int32_t ( *Init                 ) ( FT6X06_Object_t *);
  int32_t ( *DeInit               ) ( FT6X06_Object_t * );
  int32_t ( *GestureConfig        ) ( FT6X06_Object_t *, FT6X06_Gesture_Init_t* );   
  int32_t ( *ReadID               ) ( FT6X06_Object_t *, uint32_t * ); 
  int32_t ( *GetState             ) ( FT6X06_Object_t *, FT6X06_State_t* );
  int32_t ( *GetMultiTouchState   ) ( FT6X06_Object_t *, FT6X06_MultiTouch_State_t* );
  int32_t ( *GetGesture           ) ( FT6X06_Object_t *, uint8_t* );
  int32_t ( *GetCapabilities      ) ( FT6X06_Object_t *, FT6X06_Capabilities_t * );
  int32_t ( *EnableIT             ) ( FT6X06_Object_t * );
  int32_t ( *DisableIT            ) ( FT6X06_Object_t * );
  int32_t ( *ClearIT              ) ( FT6X06_Object_t * );
  int32_t ( *ITStatus             ) ( FT6X06_Object_t * ); 
} FT6X06_TS_Drv_t;
/**
 * @}
 */

/** @addtogroup FT6X06_Exported_Variables
  * @{
  */
extern FT6X06_TS_Drv_t FT6X06_TS_Driver;
/**
 * @}
 */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup FT6X06_Exported_Functions
 * @{
 */

int32_t FT6X06_RegisterBusIO (FT6X06_Object_t *pObj, FT6X06_IO_t *pIO);
int32_t FT6X06_Init(FT6X06_Object_t *pObj);
int32_t FT6X06_DeInit(FT6X06_Object_t *pObj);
int32_t FT6X06_GestureConfig(FT6X06_Object_t *pObj, FT6X06_Gesture_Init_t *GestureInit);
int32_t FT6X06_ReadID(FT6X06_Object_t *pObj, uint32_t *Id);
int32_t FT6X06_GetState(FT6X06_Object_t *pObj, FT6X06_State_t *State);
int32_t FT6X06_GetMultiTouchState(FT6X06_Object_t *pObj, FT6X06_MultiTouch_State_t *State);
int32_t FT6X06_GetGesture(FT6X06_Object_t *pObj, uint8_t *GestureId);
int32_t FT6X06_EnableIT(FT6X06_Object_t *pObj);
int32_t FT6X06_DisableIT(FT6X06_Object_t *pObj);
int32_t FT6X06_ITStatus(FT6X06_Object_t *pObj);
int32_t FT6X06_ClearIT(FT6X06_Object_t *pObj);
int32_t FT6X06_GetCapabilities(FT6X06_Object_t *pObj, FT6X06_Capabilities_t *Capabilities);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* FT6X06_H */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
