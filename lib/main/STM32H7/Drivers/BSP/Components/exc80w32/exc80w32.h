/**
 ******************************************************************************
 * @file    exc80w32.h
 * @author  MCD Application Team
 * @brief   This file contains all the functions prototypes for the
 *          exc80w32.c driver.
 ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef EXC80W32_H
#define EXC80W32_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "exc80w32_reg.h"
#include <stddef.h>
#include "exc80w32_conf.h"
  
/* Macros --------------------------------------------------------------------*/

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Component
 * @{
 */

/** @addtogroup EXC80W32
 * @{
 */

/** @addtogroup EXC80W32_Exported_Constants
 * @{
 */
#define EXC80W32_OK                      (0)
#define EXC80W32_ERROR                   (-1)

/* Max detectable simultaneous touches */
#define EXC80W32_MAX_NB_TOUCH             1U
  
/* Touch EXC80W32 IDs */
#define EXC80W32_ID                       0x8032U
   
/* EXC80W32 Multitouch state Report ID*/
#define EXC80W32_MULTITOUCH_ID              0x18U

/**
 * @}
 */

/** @defgroup EXC80W32_Exported_Types EXC80W32 Exported Types
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
}EXC80W32_Gesture_Init_t;

typedef int32_t (*EXC80W32_Init_Func)    (void);
typedef int32_t (*EXC80W32_DeInit_Func)  (void);
typedef int32_t (*EXC80W32_GetTick_Func) (void);
typedef int32_t (*EXC80W32_Delay_Func)   (uint32_t);
typedef int32_t (*EXC80W32_WriteReg_Func)(uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*EXC80W32_ReadReg_Func) (uint16_t, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  EXC80W32_Init_Func          Init;
  EXC80W32_DeInit_Func        DeInit;
  uint16_t                   Address;  
  EXC80W32_WriteReg_Func      WriteReg;
  EXC80W32_ReadReg_Func       ReadReg; 
  EXC80W32_GetTick_Func       GetTick; 
} EXC80W32_IO_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX;
  uint32_t  TouchY;
} EXC80W32_State_t;

typedef struct
{
  uint32_t  TouchDetected;
  uint32_t  TouchX[EXC80W32_MAX_NB_TOUCH];
  uint32_t  TouchY[EXC80W32_MAX_NB_TOUCH];
  uint32_t  TouchWeight[EXC80W32_MAX_NB_TOUCH];
  uint32_t  TouchEvent[EXC80W32_MAX_NB_TOUCH];
  uint32_t  TouchArea[EXC80W32_MAX_NB_TOUCH];
} EXC80W32_MultiTouch_State_t;

typedef struct
{
  EXC80W32_IO_t         IO;
  exc80w32_ctx_t        Ctx;   
  uint8_t             IsInitialized;
} EXC80W32_Object_t;

typedef struct 
{       
  uint8_t   MultiTouch;      
  uint8_t   Gesture;
  uint8_t   MaxTouch;
  uint32_t  MaxXl;
  uint32_t  MaxYl;
} EXC80W32_Capabilities_t;

 typedef struct
{
  int32_t ( *Init                 ) ( EXC80W32_Object_t *);
  int32_t ( *DeInit               ) ( EXC80W32_Object_t * );
  int32_t ( *GestureConfig        ) ( EXC80W32_Object_t *, EXC80W32_Gesture_Init_t* );   
  int32_t ( *ReadID               ) ( EXC80W32_Object_t *, uint32_t * ); 
  int32_t ( *GetState             ) ( EXC80W32_Object_t *, EXC80W32_State_t* );
  int32_t ( *GetMultiTouchState   ) ( EXC80W32_Object_t *, EXC80W32_MultiTouch_State_t* );
  int32_t ( *GetGesture           ) ( EXC80W32_Object_t *, uint8_t* );
  int32_t ( *GetCapabilities      ) ( EXC80W32_Object_t *, EXC80W32_Capabilities_t * );
  int32_t ( *EnableIT             ) ( EXC80W32_Object_t * );
  int32_t ( *DisableIT            ) ( EXC80W32_Object_t * );
  int32_t ( *ClearIT              ) ( EXC80W32_Object_t * );
  int32_t ( *ITStatus             ) ( EXC80W32_Object_t * ); 
} EXC80W32_TS_Drv_t;
/**
 * @}
 */

/** @addtogroup EXC80W32_Exported_Variables
  * @{
  */
extern EXC80W32_TS_Drv_t EXC80W32_TS_Driver;
/**
 * @}
 */

/** @addtogroup EXC80W32_Exported_Functions
 * @{
 */
int32_t EXC80W32_RegisterBusIO (EXC80W32_Object_t *pObj, EXC80W32_IO_t *pIO);
int32_t EXC80W32_Init(EXC80W32_Object_t *pObj);
int32_t EXC80W32_DeInit(EXC80W32_Object_t *pObj);
int32_t EXC80W32_GestureConfig(EXC80W32_Object_t *pObj, EXC80W32_Gesture_Init_t *GestureInit);
int32_t EXC80W32_ReadID(EXC80W32_Object_t *pObj, uint32_t *Id);
int32_t EXC80W32_GetState(EXC80W32_Object_t *pObj, EXC80W32_State_t *State);
int32_t EXC80W32_GetMultiTouchState(EXC80W32_Object_t *pObj, EXC80W32_MultiTouch_State_t *State);
int32_t EXC80W32_GetGesture(EXC80W32_Object_t *pObj, uint8_t *GestureId);
int32_t EXC80W32_EnableIT(EXC80W32_Object_t *pObj);
int32_t EXC80W32_DisableIT(EXC80W32_Object_t *pObj);
int32_t EXC80W32_ITStatus(EXC80W32_Object_t *pObj);
int32_t EXC80W32_ClearIT(EXC80W32_Object_t *pObj);
int32_t EXC80W32_GetCapabilities(EXC80W32_Object_t *pObj, EXC80W32_Capabilities_t *Capabilities);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif
#endif /* EXC80W32_H */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
