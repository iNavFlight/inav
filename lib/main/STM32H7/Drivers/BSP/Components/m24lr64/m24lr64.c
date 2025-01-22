/**
  ******************************************************************************
  * @file    m24lr64.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions to interface with the M24LR64
  *          device.
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

/* Includes ------------------------------------------------------------------*/
#include "m24lr64.h"

/** @addtogroup BSP
  * @{
  */
  
/** @addtogroup Components
  * @{
  */ 

/** @defgroup M24LR64 M24LR64
  * @{
  */

/** @defgroup M24LR64_Private_Types M24LR64 Private Types
  * @{
  */
M24LR64_EEPROM_Drv_t   M24LR64_EEPROM_Driver = 
{
  M24LR64_Init,
  M24LR64_DeInit,
  M24LR64_Write,  
  M24LR64_Read,
  M24LR64_IsReady
};    
/**
  * @}
  */    
/** @defgroup M24LR64_Private_FunctionPrototypes M24LR64 Private Function Prototypes
  * @{
  */
static int32_t M24LR64_ReadWrap(void *handle, uint16_t Addr, uint8_t* Data, uint16_t Length);
static int32_t M24LR64_WriteWrap(void *handle, uint16_t Addr, uint8_t* Data, uint16_t Length);
static int32_t M24LR64_IsReadyWrap(void *handle, uint32_t Trials);

/**
  * @}
  */ 

/** @defgroup M24LR64_Private_Functions M24LR64 Private Functions
  * @{
  */
/**
  * @brief  Function
  * @param  Component object pointer
  * @retval Component status
  */
int32_t M24LR64_RegisterBusIO (M24LR64_Object_t *pObj, M24LR64_IO_t *pIO)
{
  int32_t ret;
    
    if(pObj == NULL)
    {
      ret = M24LR64_ERROR;
    }
    else
    {
      pObj->IO.Init      = pIO->Init;
      pObj->IO.DeInit    = pIO->DeInit;
      pObj->IO.Address   = pIO->Address;
      pObj->IO.Write     = pIO->Write;
      pObj->IO.Read      = pIO->Read;
      pObj->IO.IsReady   = pIO->IsReady;
      
      pObj->Ctx.Read     = M24LR64_ReadWrap;
      pObj->Ctx.Write    = M24LR64_WriteWrap;
      pObj->Ctx.IsReady  = M24LR64_IsReadyWrap;
      pObj->Ctx.handle   = pObj;
      
      if(pObj->IO.Init != NULL)
      {
        ret = pObj->IO.Init();
      }
      else
      {
        ret = M24LR64_ERROR;
      }
    }
  
  return ret;
}

 
/**
  * @brief  Initializes the M24LR64 EEPROM component.
  * @param  pObj  M24LR64 object
  * @retval Component status
  */
int32_t M24LR64_Init(M24LR64_Object_t *pObj)
{
  if(pObj->IsInitialized != 1U)
  {
    pObj->IO.Init();
    pObj->IsInitialized = 0;
  }
  return M24LR64_OK;
} 

/**
  * @brief  DeInitializes the M24LR64 EEPROM component.
  * @param  pObj  M24LR64 object
  * @retval Component status
  */
int32_t M24LR64_DeInit(M24LR64_Object_t *pObj)
{
  if(pObj->IsInitialized != 0U)
  {
    pObj->IsInitialized = 1;
  }
  return M24LR64_OK;
}

/**
  * @brief  DeInitializes the M24LR64 EEPROM component.
  * @param  pObj  M24LR64 object
  * @retval Component status
  */
int32_t M24LR64_Write(M24LR64_Object_t *pObj, uint16_t Addr, uint8_t *Data, uint16_t Length)
{
  return pObj->Ctx.Write(pObj->Ctx.handle, Addr, Data, Length);
}

/**
  * @brief  DeInitializes the M24LR64 EEPROM component.
  * @param  pObj  M24LR64 object
  * @retval Component status
  */
int32_t M24LR64_Read(M24LR64_Object_t *pObj, uint16_t Addr, uint8_t *Data, uint16_t Length)
{
  return pObj->Ctx.Read(pObj->Ctx.handle, Addr, Data, Length);
}

/**
  * @brief  Check if the M24LR64 EEPROM component is ready.
  * @param  pObj  M24LR64 object
  * @param  Trials  The number of trials before returning a timeout error
  * @retval Component status
  */
int32_t M24LR64_IsReady(M24LR64_Object_t *pObj, uint32_t Trials)
{
  return pObj->Ctx.IsReady(pObj->Ctx.handle, Trials);
}
/**
  * @}
  */ 

/** @addtogroup M24LR64_Private_FunctionPrototypes
  * @{
  */
/**
  * @brief  Wrap component ReadReg to Bus Read function
  * @param  handle Component object handle
  * @param  Addr   The target address to read
  * @param  pData  The target register value to be written
  * @param  Length Buffer size to be written
  * @retval Component status
  */
static int32_t M24LR64_ReadWrap(void *handle, uint16_t Addr, uint8_t* pData, uint16_t Length)
{
  M24LR64_Object_t *pObj = (M24LR64_Object_t *)handle;

  return pObj->IO.Read(pObj->IO.Address, Addr, pData, Length);
}

/**
  * @brief  Wrap component WriteReg to Bus Write function
  * @param  handle  Component object handle
  * @param  Addr    The target address to write
  * @param  pData   The target register value to be written
  * @param  Length  Buffer size to be written
  * @retval Component status
  */
static int32_t M24LR64_WriteWrap(void *handle, uint16_t Addr, uint8_t* pData, uint16_t Length)
{
  M24LR64_Object_t *pObj = (M24LR64_Object_t *)handle;

  return pObj->IO.Write(pObj->IO.Address, Addr, pData, Length);
}

/**
  * @brief  Wrap component IsReady to Bus IsReady function
  * @param  handle  Component object handle
  * @param  Trials  The number of trials before returning a timeout error
  * @retval Component status
  */
static int32_t M24LR64_IsReadyWrap(void *handle, uint32_t Trials)
{
  M24LR64_Object_t *pObj = (M24LR64_Object_t *)handle;
  
  return pObj->IO.IsReady(pObj->IO.Address, Trials);
}

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

/******************* (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
