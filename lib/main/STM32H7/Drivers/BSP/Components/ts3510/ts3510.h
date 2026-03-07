/**
 ******************************************************************************
 * @file    ts3510.h
 * @author  MCD Application Team
 * @brief   This file contains all the functions prototypes for the
 *          ts3510.c Touch Screen driver.
 ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2014 STMicroelectronics.
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
#ifndef TS3510_H
#define TS3510_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ts3510_reg.h"
#include <stddef.h>
#include "ts3510_conf.h"
  
/* Macros --------------------------------------------------------------------*/

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Component
 * @{
 */

/** @addtogroup TS3510
 * @{
 */

/** @addtogroup TS3510_Exported_Constants
 * @{
 */
#define TS3510_OK                      (0)
#define TS3510_ERROR                   (-1)

/* Max detectable simultaneous touches */
#define TS3510_MAX_NB_TOUCH             1U
  
/* Touch TS3510 IDs */
#define TS3510_ID                       0x3510U

#define TS3510_READ_CMD                 0x81U  
#define TS3510_WRITE_CMD                0x08U 
/**
 * @}
 */

/** @defgroup TS3510_Exported_Types TS3510 Exported Types
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
}TS3510_Gesture_Init_t;

typedef int32_t (*TS3510_Init_Func)    (void);
typedef int32_t (*TS3510_DeInit_Func)  (void);
typedef int32_t (*TS3510_GetTick_Func) (void);
typedef int32_t (*TS3510_Delay_Func)   (uint32_t);
typedef int32_t (*TS3510_WriteReg_Func)(uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*TS3510_ReadReg_Func) (uint16_t, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  TS3510_Init_Func          Init;
  TS3510_DeInit_Func        DeInit;
  uint16_t                   Address;  
  TS3510_WriteReg_Func      WriteReg;
  TS3510_ReadReg_Func       ReadReg; 
  TS3510_GetTick_Func       GetTick; 
} TS3510_IO_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX;
  uint32_t  TouchY;
} TS3510_State_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX[TS3510_MAX_NB_TOUCH];
  uint32_t  TouchY[TS3510_MAX_NB_TOUCH];
  uint32_t  TouchWeight[TS3510_MAX_NB_TOUCH];
  uint32_t  TouchEvent[TS3510_MAX_NB_TOUCH];
  uint32_t  TouchArea[TS3510_MAX_NB_TOUCH];
} TS3510_MultiTouch_State_t;

typedef struct
{
  TS3510_IO_t         IO;
  ts3510_ctx_t        Ctx;   
  uint8_t             IsInitialized;
} TS3510_Object_t;

typedef struct 
{       
  uint8_t   MultiTouch;      
  uint8_t   Gesture;
  uint8_t   MaxTouch;
  uint32_t  MaxXl;
  uint32_t  MaxYl;
} TS3510_Capabilities_t;

 typedef struct
{
  int32_t ( *Init                 ) ( TS3510_Object_t *);
  int32_t ( *DeInit               ) ( TS3510_Object_t * );
  int32_t ( *GestureConfig        ) ( TS3510_Object_t *, TS3510_Gesture_Init_t* );   
  int32_t ( *ReadID               ) ( TS3510_Object_t *, uint32_t * ); 
  int32_t ( *GetState             ) ( TS3510_Object_t *, TS3510_State_t* );
  int32_t ( *GetMultiTouchState   ) ( TS3510_Object_t *, TS3510_MultiTouch_State_t* );
  int32_t ( *GetGesture           ) ( TS3510_Object_t *, uint8_t* );
  int32_t ( *GetCapabilities      ) ( TS3510_Object_t *, TS3510_Capabilities_t * );
  int32_t ( *EnableIT             ) ( TS3510_Object_t * );
  int32_t ( *DisableIT            ) ( TS3510_Object_t * );
  int32_t ( *ClearIT              ) ( TS3510_Object_t * );
  int32_t ( *ITStatus             ) ( TS3510_Object_t * ); 
} TS3510_TS_Drv_t;
/**
 * @}
 */

/** @addtogroup TS3510_Exported_Variables
  * @{
  */
extern TS3510_TS_Drv_t TS3510_TS_Driver;
/**
 * @}
 */

/** @addtogroup TS3510_Exported_Functions
 * @{
 */
int32_t TS3510_RegisterBusIO (TS3510_Object_t *pObj, TS3510_IO_t *pIO);
int32_t TS3510_Init(TS3510_Object_t *pObj);
int32_t TS3510_DeInit(TS3510_Object_t *pObj);
int32_t TS3510_GestureConfig(TS3510_Object_t *pObj, TS3510_Gesture_Init_t *GestureInit);
int32_t TS3510_ReadID(TS3510_Object_t *pObj, uint32_t *Id);
int32_t TS3510_GetState(TS3510_Object_t *pObj, TS3510_State_t *State);
int32_t TS3510_GetMultiTouchState(TS3510_Object_t *pObj, TS3510_MultiTouch_State_t *State);
int32_t TS3510_GetGesture(TS3510_Object_t *pObj, uint8_t *GestureId);
int32_t TS3510_EnableIT(TS3510_Object_t *pObj);
int32_t TS3510_DisableIT(TS3510_Object_t *pObj);
int32_t TS3510_ITStatus(TS3510_Object_t *pObj);
int32_t TS3510_ClearIT(TS3510_Object_t *pObj);
int32_t TS3510_GetCapabilities(TS3510_Object_t *pObj, TS3510_Capabilities_t *Capabilities);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* TS3510_H */

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
