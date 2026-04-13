/**
  ******************************************************************************
  * @file    exc7200.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the EXC7200
  *          IO Expander devices.
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

/* Includes ------------------------------------------------------------------*/
#include "exc7200.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup EXC7200 EXC7200
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/** @defgroup EXC7200_Exported_Variables EXC7200 Exported Variables
  * @{
  */

/* Touch screen driver structure initialization */
EXC7200_TS_Drv_t EXC7200_TS_Driver =
{
  EXC7200_Init,
  EXC7200_DeInit,
  EXC7200_GestureConfig,
  EXC7200_ReadID, 
  EXC7200_GetState,
  EXC7200_GetMultiTouchState,
  EXC7200_GetGesture,
  EXC7200_GetCapabilities,
  EXC7200_EnableIT,
  EXC7200_DisableIT,  
  EXC7200_ClearIT,
  EXC7200_ITStatus
};
/**
  * @}
  */

/** @defgroup EXC7200_Private_Function_Prototypes EXC7200 Private Function Prototypes
  * @{
  */
static int32_t EXC7200_DetectTouch(EXC7200_Object_t *pObj, uint8_t *pData, uint32_t Length);
static int32_t ReadRegWrap(void *handle, uint8_t Reg, uint8_t* Data, uint16_t Length);
static int32_t WriteRegWrap(void *handle, uint8_t Reg, uint8_t* Data, uint16_t Length);

/**
  * @}
  */

/** @defgroup EXC7200_Exported_Functions EXC7200 Exported Functions
  * @{
  */

/**
  * @brief  Register IO bus to component object
  * @param  Component object pointer
  * @retval error status
  */
int32_t EXC7200_RegisterBusIO (EXC7200_Object_t *pObj, EXC7200_IO_t *pIO)
{
  int32_t ret;
  
  if (pObj == NULL)
  {
    ret = EXC7200_ERROR;
  }
  else
  {
    pObj->IO.Init      = pIO->Init;
    pObj->IO.DeInit    = pIO->DeInit;
    pObj->IO.Address   = pIO->Address;
    pObj->IO.WriteReg  = pIO->WriteReg;
    pObj->IO.ReadReg   = pIO->ReadReg;
    pObj->IO.GetTick   = pIO->GetTick;
    
    pObj->Ctx.ReadReg  = ReadRegWrap;
    pObj->Ctx.WriteReg = WriteRegWrap;
    pObj->Ctx.handle   = pObj;
    
    if(pObj->IO.Init != NULL)
    {
      ret = pObj->IO.Init();
    }
    else
    {
      ret = EXC7200_ERROR;
    }
  }    
  
  return ret;
}

/**
  * @brief  Get EXC7200 sensor capabilities
  * @param  pObj Component object pointer
  * @param  Capabilities pointer to EXC7200 sensor capabilities
  * @retval Component status
  */
int32_t EXC7200_GetCapabilities(EXC7200_Object_t *pObj, EXC7200_Capabilities_t *Capabilities)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  /* Store component's capabilities */
  Capabilities->MultiTouch = 0;
  Capabilities->Gesture    = 0;
  Capabilities->MaxTouch   = EXC7200_MAX_NB_TOUCH;
  Capabilities->MaxXl      = EXC7200_MAX_X_LENGTH;
  Capabilities->MaxYl      = EXC7200_MAX_Y_LENGTH;
  
  return EXC7200_OK;
}

/**
  * @brief  Initialize the EXC7200 communication bus
  *         from MCU to EXC7200 : ie I2C channel initialization (if required).
  * @param  pObj Component object pointer
  * @retval Component status
  */
int32_t EXC7200_Init(EXC7200_Object_t *pObj)
{
  int32_t ret = EXC7200_OK;
  
  if(pObj->IsInitialized == 0U)
  {    
    /* Initialize IO BUS layer */
    if(pObj->IO.Init() != EXC7200_OK)
    {
      ret = EXC7200_ERROR;
    }
    else
    {
      pObj->IsInitialized = 1;
    }
  }
  
  return ret;
}

/**
  * @brief  De-Initialize the EXC7200 communication bus
  *         from MCU to EXC7200 : ie I2C channel initialization (if required).
  * @param  pObj Component object pointer
  * @retval Component status
  */
int32_t EXC7200_DeInit(EXC7200_Object_t *pObj)
{
  int32_t ret = EXC7200_OK;
  
  if(pObj->IsInitialized == 1U)
  {
    if(pObj->IO.DeInit() != EXC7200_OK)
    {
      ret = EXC7200_ERROR;
    }
    else
    {
      pObj->IsInitialized = 0;
    }
  }
  
  return ret;
}

/**
  * @brief  Configure the EXC7200 gesture
  *         from MCU to EXC7200 : ie I2C channel initialization (if required).
  * @param  pObj  Component object pointer
  * @param  GestureInit Gesture init structure
  * @retval Component status
  */
int32_t EXC7200_GestureConfig(EXC7200_Object_t *pObj, EXC7200_Gesture_Init_t *GestureInit)
{
  /* Feature not supported */
  return EXC7200_ERROR;  
}

/**
  * @brief  Read the EXC7200 device ID, pre initialize I2C in case of need to be
  *         able to read the EXC7200 device ID, and verify this is a EXC7200.
  * @param  pObj Component object pointer
  * @param  Id Pointer to component ID value
  * @retval Component status
  */
int32_t EXC7200_ReadID(EXC7200_Object_t *pObj, uint32_t *Id)
{
  int32_t ret = EXC7200_ERROR;
  uint8_t data;
  
  if(exc7200_read_reg(&pObj->Ctx, EXC7200_READ_REG, &data, 1) == EXC7200_OK)
  {
    *Id = EXC7200_ID; 
    ret = EXC7200_OK;
  }  
  
  return ret;
}

/**
  * @brief  Get the touch screen X and Y positions values
  * @param  pObj Component object pointer
  * @param  State Single Touch stucture pointer
  * @retval Component status.
  */
int32_t EXC7200_GetState(EXC7200_Object_t *pObj, EXC7200_State_t *State)
{
  uint8_t  data[10];
  
  State->TouchDetected = (uint32_t)EXC7200_DetectTouch(pObj, data, (uint32_t)sizeof(data));
  
  /* Send back first ready X position to caller */
  State->TouchX = (((data[3]&0x00ff) << 4) | ((data[2]&0x00f0) >> 4)) << 1;
  /* Send back first ready Y position to caller */
  State->TouchY = (((data[5]&0x00ff) << 4) | ((data[4]&0x00f0) >> 4)) << 1;
  
  /* Dummy Read to deactivate read mode */
  (void)EXC7200_DetectTouch(pObj, data, (uint32_t)sizeof(data));
  
  return EXC7200_OK;
}

/**
  * @brief  Get the touch screen Xn and Yn positions values in multi-touch mode
  * @param  pObj Component object pointer
  * @param  State Multi Touch stucture pointer
  * @retval Component status.
  */
int32_t EXC7200_GetMultiTouchState(EXC7200_Object_t *pObj, EXC7200_MultiTouch_State_t *State)
{
  /* Feature not supported */
  return EXC7200_ERROR; 
}

/**
  * @brief  Get Gesture ID
  * @param  pObj Component object pointer
  * @param  GestureId gesture ID
  * @retval Component status.
  */
int32_t EXC7200_GetGesture(EXC7200_Object_t *pObj, uint8_t *GestureId)
{  
  /* Feature not supported */
  return EXC7200_ERROR;
}

/**
  * @brief  Configure the EXC7200 device to generate IT on given INT pin
  *         connected to MCU as EXTI.
  * @param  pObj Component object pointer
  * @retval Component status.
  */
int32_t EXC7200_EnableIT(EXC7200_Object_t *pObj)
{
  /* Feature not supported */
  return EXC7200_ERROR;
}

/**
  * @brief  Configure the EXC7200 device to stop generating IT on the given INT pin
  *         connected to MCU as EXTI.
  * @param  pObj Component object pointer
  * @retval Component status.
  */
int32_t EXC7200_DisableIT(EXC7200_Object_t *pObj)
{
  /* Feature not supported */
  return EXC7200_ERROR;
}

/**
  * @brief  Get IT status from EXC7200 interrupt status registers
  *         Should be called Following an EXTI coming to the MCU to know the detailed
  *         reason of the interrupt.
  *         @note : This feature is not applicable to EXC7200.
  * @param  pObj Component object pointer
  * @retval Component status.
  */
int32_t EXC7200_ITStatus(EXC7200_Object_t *pObj)
{
  /* Feature not supported */
  return EXC7200_ERROR;
}

/**
  * @brief  Clear IT status in EXC7200 interrupt status clear registers
  *         Should be called Following an EXTI coming to the MCU.
  *         @note : This feature is not applicable to EXC7200.
  * @param  pObj Component object pointer
  * @retval Component status.
  */
int32_t EXC7200_ClearIT(EXC7200_Object_t *pObj)
{
  /* Feature not supported */
  return EXC7200_ERROR;
}
/**
  * @}
  */

/** @defgroup EXC7200_Private_Functions EXC7200 Private Functions
  * @{
  */

/**
  * @brief  Return if there is touches detected or not.
  *         Get the touch screen X and Y positions values 
  * @param  pObj Component object pointer
  * @param  pData Pointer to data buffer
  * @param  Length Number of data to read
  * @retval Number of active touches detected (can be 0 or 1) or EXC7200_ERROR
  *         in case of error
  */
static int32_t EXC7200_DetectTouch(EXC7200_Object_t *pObj, uint8_t *pData, uint32_t Length)
{
  int32_t ret;
  
  /* Read TS data */
  if(exc7200_read_reg(&pObj->Ctx, EXC7200_READ_REG, pData, Length) != EXC7200_OK)
  {
    ret = EXC7200_ERROR;
  }
  else
  {
    if(pData[1] == 0x83U)
    {
      /* Touch detected */
      ret = 1;
    }
    else
    {
      /* If invalid number of touch detected, set it to zero */
      ret = 0;
    }
  }
  
  return ret;
}

/**
  * @brief  Wrap IO bus read function to component register red function
  * @param  handle Component object handle
  * @param  Reg The target register address to read
  * @param  pData The target register value to be read
  * @param  Length buffer size to be read
  * @retval Component status.
  */
static int32_t ReadRegWrap(void *handle, uint8_t Reg, uint8_t* pData, uint16_t Length)
{
  EXC7200_Object_t *pObj = (EXC7200_Object_t *)handle;

  return pObj->IO.ReadReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  Wrap IO bus write function to component register write function
  * @param  handle Component object handle
  * @param  Reg The target register address to write
  * @param  pData The target register value to be written
  * @param  Length buffer size to be written
  * @retval Component status.
  */
static int32_t WriteRegWrap(void *handle, uint8_t Reg, uint8_t* pData, uint16_t Length)
{
  EXC7200_Object_t *pObj = (EXC7200_Object_t *)handle;

  return pObj->IO.WriteReg(pObj->IO.Address, Reg, pData, Length);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
