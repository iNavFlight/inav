/**
 ******************************************************************************
 * @file    ft5336.h
 * @author  MCD Application Team
 * @brief   This file contains all the functions prototypes for the
 *          ft5336.c IO expander driver.
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
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
#ifndef FT5336_H
#define FT5336_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ft5336_reg.h"
#include <stddef.h>
#include "ft5336_conf.h"
  
/* Macros --------------------------------------------------------------------*/

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Component
 * @{
 */

/** @addtogroup FT5336
 * @{
 */

/** @defgroup FT5336_Exported_Constants FT5336 Exported Constants
 * @{
 */
#define FT5336_OK                      (0)
#define FT5336_ERROR                   (-1)

/* Max detectable simultaneous touches */
#define FT5336_MAX_NB_TOUCH             5U
  
/* Touch FT5336 IDs */
#define FT5336_ID                        0x51U

/* Possible values of FT5336_DEV_MODE_REG */
#define FT5336_DEV_MODE_WORKING          0x00U
#define FT5336_DEV_MODE_FACTORY          0x04U
     
/* Possible values of FT5336_GEST_ID_REG */
#define FT5336_GEST_ID_NO_GESTURE        0x00U
#define FT5336_GEST_ID_MOVE_UP           0x10U
#define FT5336_GEST_ID_MOVE_RIGHT        0x14U
#define FT5336_GEST_ID_MOVE_DOWN         0x18U
#define FT5336_GEST_ID_MOVE_LEFT         0x1CU
#define FT5336_GEST_ID_ZOOM_IN           0x48U
#define FT5336_GEST_ID_ZOOM_OUT          0x49U
     
/* Values Pn_XH and Pn_YH related */
#define FT5336_TOUCH_EVT_FLAG_PRESS_DOWN 0x00U
#define FT5336_TOUCH_EVT_FLAG_LIFT_UP    0x01U
#define FT5336_TOUCH_EVT_FLAG_CONTACT    0x02U
#define FT5336_TOUCH_EVT_FLAG_NO_EVENT   0x03U

/* Possible values of FT5336_GMODE_REG */
#define FT5336_G_MODE_INTERRUPT_POLLING  0x00U
#define FT5336_G_MODE_INTERRUPT_TRIGGER  0x01U

/**
 * @}
 */

/* Exported types ------------------------------------------------------------*/

/** @defgroup FT5336_Exported_Types FT5336 Exported Types
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
}FT5336_Gesture_Init_t;

typedef int32_t (*FT5336_Init_Func)    (void);
typedef int32_t (*FT5336_DeInit_Func)  (void);
typedef int32_t (*FT5336_GetTick_Func) (void);
typedef int32_t (*FT5336_Delay_Func)   (uint32_t);
typedef int32_t (*FT5336_WriteReg_Func)(uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*FT5336_ReadReg_Func) (uint16_t, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  FT5336_Init_Func          Init;
  FT5336_DeInit_Func        DeInit;
  uint16_t                  Address;  
  FT5336_WriteReg_Func      WriteReg;
  FT5336_ReadReg_Func       ReadReg; 
  FT5336_GetTick_Func       GetTick; 
} FT5336_IO_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX;
  uint32_t  TouchY;
} FT5336_State_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX[FT5336_MAX_NB_TOUCH];
  uint32_t  TouchY[FT5336_MAX_NB_TOUCH];
  uint32_t  TouchWeight[FT5336_MAX_NB_TOUCH];
  uint32_t  TouchEvent[FT5336_MAX_NB_TOUCH];
  uint32_t  TouchArea[FT5336_MAX_NB_TOUCH];
} FT5336_MultiTouch_State_t;

typedef struct
{
  FT5336_IO_t         IO;
  ft5336_ctx_t        Ctx;   
  uint8_t             IsInitialized;
} FT5336_Object_t;

typedef struct 
{       
  uint8_t   MultiTouch;      
  uint8_t   Gesture;
  uint8_t   MaxTouch;
  uint32_t  MaxXl;
  uint32_t  MaxYl;
} FT5336_Capabilities_t;

 typedef struct
{
  int32_t ( *Init                 ) ( FT5336_Object_t *);
  int32_t ( *DeInit               ) ( FT5336_Object_t * );
  int32_t ( *GestureConfig        ) ( FT5336_Object_t *, FT5336_Gesture_Init_t* );   
  int32_t ( *ReadID               ) ( FT5336_Object_t *, uint32_t * ); 
  int32_t ( *GetState             ) ( FT5336_Object_t *, FT5336_State_t* );
  int32_t ( *GetMultiTouchState   ) ( FT5336_Object_t *, FT5336_MultiTouch_State_t* );
  int32_t ( *GetGesture           ) ( FT5336_Object_t *, uint8_t* );
  int32_t ( *GetCapabilities      ) ( FT5336_Object_t *, FT5336_Capabilities_t * );
  int32_t ( *EnableIT             ) ( FT5336_Object_t * );
  int32_t ( *DisableIT            ) ( FT5336_Object_t * );
  int32_t ( *ClearIT              ) ( FT5336_Object_t * );
  int32_t ( *ITStatus             ) ( FT5336_Object_t * ); 
} FT5336_TS_Drv_t;
/**
 * @}
 */

/** @addtogroup FT5336_Exported_Variables
  * @{
  */
extern FT5336_TS_Drv_t FT5336_TS_Driver;
/**
 * @}
 */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/** @addtogroup FT5336_Exported_Functions
 * @{
 */

int32_t FT5336_RegisterBusIO (FT5336_Object_t *pObj, FT5336_IO_t *pIO);
int32_t FT5336_Init(FT5336_Object_t *pObj);
int32_t FT5336_DeInit(FT5336_Object_t *pObj);
int32_t FT5336_GestureConfig(FT5336_Object_t *pObj, FT5336_Gesture_Init_t *GestureInit);
int32_t FT5336_ReadID(FT5336_Object_t *pObj, uint32_t *Id);
int32_t FT5336_GetState(FT5336_Object_t *pObj, FT5336_State_t *State);
int32_t FT5336_GetMultiTouchState(FT5336_Object_t *pObj, FT5336_MultiTouch_State_t *State);
int32_t FT5336_GetGesture(FT5336_Object_t *pObj, uint8_t *GestureId);
int32_t FT5336_EnableIT(FT5336_Object_t *pObj);
int32_t FT5336_DisableIT(FT5336_Object_t *pObj);
int32_t FT5336_ITStatus(FT5336_Object_t *pObj);
int32_t FT5336_ClearIT(FT5336_Object_t *pObj);
int32_t FT5336_GetCapabilities(FT5336_Object_t *pObj, FT5336_Capabilities_t *Capabilities);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* FT5336_H */

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
