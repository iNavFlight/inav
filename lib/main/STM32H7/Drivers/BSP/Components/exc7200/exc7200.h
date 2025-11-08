/**
 ******************************************************************************
 * @file    exc7200.h
 * @author  MCD Application Team
 * @brief   This file contains all the functions prototypes for the
 *          exc7200.c IO expander driver.
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
#ifndef EXC7200_H
#define EXC7200_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "exc7200_reg.h"
#include <stddef.h>
#include "exc7200_conf.h"
  
/* Macros --------------------------------------------------------------------*/

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Component
 * @{
 */

/** @addtogroup EXC7200
 * @{
 */

/** @addtogroup EXC7200_Exported_Constants
 * @{
 */
#define EXC7200_OK                      (0)
#define EXC7200_ERROR                   (-1)

/* Max detectable simultaneous touches */
#define EXC7200_MAX_NB_TOUCH             1U
  
/* Touch EXC7200 IDs */
#define EXC7200_ID                       0x7200U

/**
 * @}
 */

/** @defgroup EXC7200_Exported_Types EXC7200 Exported Types
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
}EXC7200_Gesture_Init_t;

typedef int32_t (*EXC7200_Init_Func)    (void);
typedef int32_t (*EXC7200_DeInit_Func)  (void);
typedef int32_t (*EXC7200_GetTick_Func) (void);
typedef int32_t (*EXC7200_Delay_Func)   (uint32_t);
typedef int32_t (*EXC7200_WriteReg_Func)(uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*EXC7200_ReadReg_Func) (uint16_t, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  EXC7200_Init_Func          Init;
  EXC7200_DeInit_Func        DeInit;
  uint16_t                   Address;  
  EXC7200_WriteReg_Func      WriteReg;
  EXC7200_ReadReg_Func       ReadReg; 
  EXC7200_GetTick_Func       GetTick; 
} EXC7200_IO_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX;
  uint32_t  TouchY;
} EXC7200_State_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX[EXC7200_MAX_NB_TOUCH];
  uint32_t  TouchY[EXC7200_MAX_NB_TOUCH];
  uint32_t  TouchWeight[EXC7200_MAX_NB_TOUCH];
  uint32_t  TouchEvent[EXC7200_MAX_NB_TOUCH];
  uint32_t  TouchArea[EXC7200_MAX_NB_TOUCH];
} EXC7200_MultiTouch_State_t;

typedef struct
{
  EXC7200_IO_t         IO;
  exc7200_ctx_t        Ctx;   
  uint8_t             IsInitialized;
} EXC7200_Object_t;

typedef struct 
{       
  uint8_t   MultiTouch;      
  uint8_t   Gesture;
  uint8_t   MaxTouch;
  uint32_t  MaxXl;
  uint32_t  MaxYl;
} EXC7200_Capabilities_t;

 typedef struct
{
  int32_t ( *Init                 ) ( EXC7200_Object_t *);
  int32_t ( *DeInit               ) ( EXC7200_Object_t * );
  int32_t ( *GestureConfig        ) ( EXC7200_Object_t *, EXC7200_Gesture_Init_t* );   
  int32_t ( *ReadID               ) ( EXC7200_Object_t *, uint32_t * ); 
  int32_t ( *GetState             ) ( EXC7200_Object_t *, EXC7200_State_t* );
  int32_t ( *GetMultiTouchState   ) ( EXC7200_Object_t *, EXC7200_MultiTouch_State_t* );
  int32_t ( *GetGesture           ) ( EXC7200_Object_t *, uint8_t* );
  int32_t ( *GetCapabilities      ) ( EXC7200_Object_t *, EXC7200_Capabilities_t * );
  int32_t ( *EnableIT             ) ( EXC7200_Object_t * );
  int32_t ( *DisableIT            ) ( EXC7200_Object_t * );
  int32_t ( *ClearIT              ) ( EXC7200_Object_t * );
  int32_t ( *ITStatus             ) ( EXC7200_Object_t * ); 
} EXC7200_TS_Drv_t;
/**
 * @}
 */

/** @addtogroup EXC7200_Exported_Variables
  * @{
  */
extern EXC7200_TS_Drv_t EXC7200_TS_Driver;
/**
 * @}
 */

/** @addtogroup EXC7200_Exported_Functions
 * @{
 */
int32_t EXC7200_RegisterBusIO (EXC7200_Object_t *pObj, EXC7200_IO_t *pIO);
int32_t EXC7200_Init(EXC7200_Object_t *pObj);
int32_t EXC7200_DeInit(EXC7200_Object_t *pObj);
int32_t EXC7200_GestureConfig(EXC7200_Object_t *pObj, EXC7200_Gesture_Init_t *GestureInit);
int32_t EXC7200_ReadID(EXC7200_Object_t *pObj, uint32_t *Id);
int32_t EXC7200_GetState(EXC7200_Object_t *pObj, EXC7200_State_t *State);
int32_t EXC7200_GetMultiTouchState(EXC7200_Object_t *pObj, EXC7200_MultiTouch_State_t *State);
int32_t EXC7200_GetGesture(EXC7200_Object_t *pObj, uint8_t *GestureId);
int32_t EXC7200_EnableIT(EXC7200_Object_t *pObj);
int32_t EXC7200_DisableIT(EXC7200_Object_t *pObj);
int32_t EXC7200_ITStatus(EXC7200_Object_t *pObj);
int32_t EXC7200_ClearIT(EXC7200_Object_t *pObj);
int32_t EXC7200_GetCapabilities(EXC7200_Object_t *pObj, EXC7200_Capabilities_t *Capabilities);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* EXC7200_H */

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
