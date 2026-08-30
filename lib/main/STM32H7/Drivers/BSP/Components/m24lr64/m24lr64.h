/**
  ******************************************************************************
  * @file    m24lr64.h
  * @author  MCD Application Team
  * @brief   This file is the header of m24lr64.c
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
#ifndef M24LR64_H
#define M24LR64_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/  
#include <stdint.h>
#include <stddef.h>
   
/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @addtogroup M24LR64 
  * @{
  */
  
/** @defgroup M24LR64_Exported_Types   M24LR64 Exported Types
  * @{
  */
typedef int32_t (*M24LR64_Init_Func)    (void);
typedef int32_t (*M24LR64_DeInit_Func)  (void);
typedef int32_t (*M24LR64_GetTick_Func) (void);
typedef int32_t (*M24LR64_Delay_Func)   (uint32_t);
typedef int32_t (*M24LR64_WriteReg_Func)   (uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*M24LR64_ReadReg_Func)    (uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*M24LR64_IsDeviceReady_Func) (uint16_t, uint32_t);


typedef int32_t (*M24LR64_Write_Func)(void *, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*M24LR64_Read_Func) (void *, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*M24LR64_IsReady_Func) (void *, uint32_t);

typedef struct
{
  M24LR64_Write_Func     Write;
  M24LR64_Read_Func      Read;
  M24LR64_IsReady_Func   IsReady;
  void                   *handle;
} M24LR64_Ctx_t;

typedef struct
{
  M24LR64_Init_Func           Init;
  M24LR64_DeInit_Func         DeInit;
  uint16_t                    Address;  
  M24LR64_WriteReg_Func       Write;
  M24LR64_ReadReg_Func        Read;
  M24LR64_IsDeviceReady_Func  IsReady;
  M24LR64_GetTick_Func        GetTick;
} M24LR64_IO_t;

typedef struct
{
  M24LR64_IO_t         IO;
  M24LR64_Ctx_t        Ctx;   
  uint8_t              IsInitialized;
} M24LR64_Object_t;

typedef struct
{
  int32_t  (*Init     )(M24LR64_Object_t*);
  int32_t  (*DeInit   )(M24LR64_Object_t*);   
  int32_t  (*Write    )(M24LR64_Object_t*, uint16_t, uint8_t*, uint16_t);
  int32_t  (*Read     )(M24LR64_Object_t*, uint16_t, uint8_t*, uint16_t);
  int32_t  (*IsReady  )(M24LR64_Object_t*, uint32_t); 
}M24LR64_EEPROM_Drv_t;
/**
  * @}
  */

/** @defgroup M24LR64_Exported_Constants   M24LR64 Exported Constants
  * @{
  */
#define M24LR64_OK      0
#define M24LR64_ERROR  -1

/**
  * @}
  */

/** @defgroup M24LR64_Exported_FunctionsPrototypes   M24LR64 Exported FunctionsPrototypes
  * @{
  */  
/*  public function  --------------------------------------------------------------------------*/
int32_t M24LR64_RegisterBusIO (M24LR64_Object_t *pObj, M24LR64_IO_t *pIO);  
int32_t M24LR64_Init(M24LR64_Object_t *pObj);
int32_t M24LR64_DeInit(M24LR64_Object_t *pObj);
int32_t M24LR64_Write(M24LR64_Object_t *pObj, uint16_t Addr, uint8_t *Data, uint16_t Length);
int32_t M24LR64_Read(M24LR64_Object_t *pObj, uint16_t Addr, uint8_t *Data, uint16_t Length);
int32_t M24LR64_IsReady(M24LR64_Object_t *pObj, uint32_t Trials);


extern M24LR64_EEPROM_Drv_t   M24LR64_EEPROM_Driver;
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

#endif /* M24LR64_H */




/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
