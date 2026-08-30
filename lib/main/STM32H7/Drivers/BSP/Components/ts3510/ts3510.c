/**
  ******************************************************************************
  * @file    ts3510.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the TS3510
  *          Touch Screen.
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

/* Includes ------------------------------------------------------------------*/
#include "ts3510.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup TS3510 TS3510
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/** @defgroup TS3510_Exported_Variables TS3510 Exported Variables
  * @{
  */

/* Touch screen driver structure initialization */
TS3510_TS_Drv_t TS3510_TS_Driver =
{
  TS3510_Init,
  TS3510_DeInit,
  TS3510_GestureConfig,
  TS3510_ReadID, 
  TS3510_GetState,
  TS3510_GetMultiTouchState,
  TS3510_GetGesture,
  TS3510_GetCapabilities,
  TS3510_EnableIT,
  TS3510_DisableIT,  
  TS3510_ClearIT,
  TS3510_ITStatus
};
/**
  * @}
  */

/** @defgroup TS3510_Private_Function_Prototypes TS3510 Private Function Prototypes
  * @{
  */
static int32_t TS3510_DetectTouch(TS3510_Object_t *pObj);
static int32_t ReadRegWrap(void *handle, uint8_t Reg, uint8_t* Data, uint16_t Length);
static int32_t WriteRegWrap(void *handle, uint8_t Reg, uint8_t* Data, uint16_t Length);

/**
  * @}
  */

/** @defgroup TS3510_Exported_Functions TS3510 Exported Functions
  * @{
  */

/**
  * @brief  Register IO bus to component object
  * @param  Component object pointer
  * @retval error status
  */
int32_t TS3510_RegisterBusIO (TS3510_Object_t *pObj, TS3510_IO_t *pIO)
{
  int32_t ret;
  
  if (pObj == NULL)
  {
    ret = TS3510_ERROR;
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
      ret = TS3510_ERROR;
    }
  }    
  
  return ret;
}

/**
  * @brief  Get TS3510 sensor capabilities
  * @param  pObj Component object pointer
  * @param  Capabilities pointer to TS3510 sensor capabilities
  * @retval Component status
  */
int32_t TS3510_GetCapabilities(TS3510_Object_t *pObj, TS3510_Capabilities_t *Capabilities)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  /* Store component's capabilities */
  Capabilities->MultiTouch = 0;
  Capabilities->Gesture    = 0;
  Capabilities->MaxTouch   = TS3510_MAX_NB_TOUCH;
  Capabilities->MaxXl      = TS3510_MAX_X_LENGTH;
  Capabilities->MaxYl      = TS3510_MAX_Y_LENGTH;
  
  return TS3510_OK;
}

/**
  * @brief  Initialize the TS3510 communication bus
  *         from MCU to TS3510 : ie I2C channel initialization (if required).
  * @param  pObj Component object pointer
  * @retval Component status
  */
int32_t TS3510_Init(TS3510_Object_t *pObj)
{
  int32_t ret = TS3510_OK;
  
  if(pObj->IsInitialized == 0U)
  {    
    /* Initialize IO BUS layer */
    if(pObj->IO.Init() != TS3510_OK)
    {
      ret = TS3510_ERROR;
    }
    else
    {
      pObj->IsInitialized = 1;
    }
  }
  
  return ret;
}

/**
  * @brief  De-Initialize the TS3510 communication bus
  *         from MCU to TS3510 : ie I2C channel initialization (if required).
  * @param  pObj Component object pointer
  * @retval Component status
  */
int32_t TS3510_DeInit(TS3510_Object_t *pObj)
{
  int32_t ret = TS3510_OK;
  
  if(pObj->IsInitialized == 1U)
  {
    if(pObj->IO.DeInit() != TS3510_OK)
    {
      ret = TS3510_ERROR;
    }
    else
    {
      pObj->IsInitialized = 0;
    }
  }
  
  return ret;
}

/**
  * @brief  Configure the TS3510 gesture
  *         from MCU to TS3510 : ie I2C channel initialization (if required).
  * @param  pObj  Component object pointer
  * @param  GestureInit Gesture init structure
  * @retval Component status
  */
int32_t TS3510_GestureConfig(TS3510_Object_t *pObj, TS3510_Gesture_Init_t *GestureInit)
{
  /* Feature not supported */
  return TS3510_ERROR;  
}

/**
  * @brief  Read the TS3510 device ID, pre initialize I2C in case of need to be
  *         able to read the TS3510 device ID, and verify this is a TS3510.
  * @param  pObj Component object pointer
  * @param  Id Pointer to component ID value
  * @retval Component status
  */
int32_t TS3510_ReadID(TS3510_Object_t *pObj, uint32_t *Id)
{
  int32_t ret = TS3510_OK;
  uint8_t aTmpBuffer[2] = {TS3510_READ_CMD, TS3510_WRITE_CMD};
  uint8_t  data;
  
  /* Prepare for LCD read data */
  if(ts3510_write_reg(&pObj->Ctx, TS3510_READ_BLOCK_REG, aTmpBuffer, 2) != TS3510_OK)
  {
    ret = TS3510_ERROR;
  }/* Read TS data from LCD */
  else if(ts3510_read_reg(&pObj->Ctx, TS3510_READ_BLOCK_REG, &data, 1) != TS3510_OK)
  {
    ret = TS3510_ERROR;
  }
  else
  {
    *Id = TS3510_ID;
  }
  
  return ret;     
}

/**
  * @brief  Get the touch screen X and Y positions values
  * @param  pObj Component object pointer
  * @param  State Single Touch stucture pointer
  * @retval Component status.
  */
int32_t TS3510_GetState(TS3510_Object_t *pObj, TS3510_State_t *State)
{
  int32_t ret = TS3510_OK;
  uint8_t aTmpBuffer[2] = {TS3510_READ_CMD, TS3510_WRITE_CMD};
  uint8_t pData[11];
  
  /* Check if a touch is detected */
  State->TouchDetected = (uint32_t)TS3510_DetectTouch(pObj);
  
  if(State->TouchDetected == 1U)
  {  
    /* Prepare for LCD read data */
    if(ts3510_write_reg(&pObj->Ctx, TS3510_SEND_CMD_REG, aTmpBuffer, 2) != TS3510_OK)
    {
      ret = TS3510_ERROR;
    }/* Read TS data from LCD */
    else if(ts3510_read_reg(&pObj->Ctx, TS3510_READ_BLOCK_REG, pData, 11) != TS3510_OK)
    {
      ret = TS3510_ERROR;
    }  
    else
    {
      /* Send back first ready X position to caller */
      State->TouchX = (((pData[1] << 8) | pData[2]) << 12) / 640;
      /* Send back first ready Y position to caller */
      State->TouchY = (((pData[3] << 8) | pData[4]) << 12) / 480;
    }
  }
  
  return ret;
}

/**
  * @brief  Get the touch screen Xn and Yn positions values in multi-touch mode
  * @param  pObj Component object pointer
  * @param  State Multi Touch stucture pointer
  * @retval Component status.
  */
int32_t TS3510_GetMultiTouchState(TS3510_Object_t *pObj, TS3510_MultiTouch_State_t *State)
{
  /* Feature not supported */
  return TS3510_ERROR; 
}

/**
  * @brief  Get Gesture ID
  * @param  pObj Component object pointer
  * @param  GestureId gesture ID
  * @retval Component status.
  */
int32_t TS3510_GetGesture(TS3510_Object_t *pObj, uint8_t *GestureId)
{  
  /* Feature not supported */
  return TS3510_ERROR;
}

/**
  * @brief  Configure the TS3510 device to generate IT on given INT pin
  *         connected to MCU as EXTI.
  * @param  pObj Component object pointer
  * @retval Component status.
  */
int32_t TS3510_EnableIT(TS3510_Object_t *pObj)
{
  /* Feature not supported */
  return TS3510_ERROR;
}

/**
  * @brief  Configure the TS3510 device to stop generating IT on the given INT pin
  *         connected to MCU as EXTI.
  * @param  pObj Component object pointer
  * @retval Component status.
  */
int32_t TS3510_DisableIT(TS3510_Object_t *pObj)
{
  /* Feature not supported */
  return TS3510_ERROR;
}

/**
  * @brief  Get IT status from TS3510 interrupt status registers
  *         Should be called Following an EXTI coming to the MCU to know the detailed
  *         reason of the interrupt.
  *         @note : This feature is not applicable to TS3510.
  * @param  pObj Component object pointer
  * @retval Component status.
  */
int32_t TS3510_ITStatus(TS3510_Object_t *pObj)
{
  /* Feature not supported */
  return TS3510_ERROR;
}

/**
  * @brief  Clear IT status in TS3510 interrupt status clear registers
  *         Should be called Following an EXTI coming to the MCU.
  *         @note : This feature is not applicable to TS3510.
  * @param  pObj Component object pointer
  * @retval Component status.
  */
int32_t TS3510_ClearIT(TS3510_Object_t *pObj)
{
  /* Feature not supported */
  return TS3510_ERROR;
}
/**
  * @}
  */

/** @defgroup TS3510_Private_Functions TS3510 Private Functions
  * @{
  */

/**
  * @brief  Return if there is touches detected or not.
  *         Get the touch screen X and Y positions values 
  * @param  pObj Component object pointer
  * @retval Number of active touches detected (can be 0 or 1) or TS3510_ERROR
  *         in case of error
  */
static int32_t TS3510_DetectTouch(TS3510_Object_t *pObj)
{
  int32_t ret;
  uint8_t aTmpBuffer[2] = {TS3510_READ_CMD, TS3510_WRITE_CMD};
  uint8_t pData[11];
  
  /* Prepare for LCD read data */
  if(ts3510_write_reg(&pObj->Ctx, TS3510_SEND_CMD_REG, aTmpBuffer, 2) != TS3510_OK)
  {
    ret = TS3510_ERROR;
  }/* Read TS data from LCD */
  else if(ts3510_read_reg(&pObj->Ctx, TS3510_READ_BLOCK_REG, pData, 11) != TS3510_OK)
  {
    ret = TS3510_ERROR;
  }
  else
  {
    if((pData[1] == 0xFF) && (pData[2] == 0xFF) && (pData[3] == 0xFF) && (pData[4] == 0xFF))
    {
      /* If invalid number of touch detected, set it to zero */
      ret = 0;
    }
    else
    {
      /* Touch detected */
      ret = 1;
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
  TS3510_Object_t *pObj = (TS3510_Object_t *)handle;

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
  TS3510_Object_t *pObj = (TS3510_Object_t *)handle;

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
