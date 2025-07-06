/**
  ******************************************************************************
  * @file    exc80w32.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the EXC80W32
  *          devices.
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

/* Includes ------------------------------------------------------------------*/
#include "exc80w32.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup EXC80W32 EXC80W32
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/** @defgroup EXC80W32_Exported_Variables EXC80W32 Exported Variables
  * @{
  */

/* Touch screen driver structure initialization */
EXC80W32_TS_Drv_t EXC80W32_TS_Driver =
{
  EXC80W32_Init,
  EXC80W32_DeInit,
  EXC80W32_GestureConfig,
  EXC80W32_ReadID, 
  EXC80W32_GetState,
  EXC80W32_GetMultiTouchState,
  EXC80W32_GetGesture,
  EXC80W32_GetCapabilities,
  EXC80W32_EnableIT,
  EXC80W32_DisableIT,  
  EXC80W32_ClearIT,
  EXC80W32_ITStatus
};

/**
  * @}
  */

/** @defgroup EXC80W32_Private_Function_Prototypes EXC80W32 Private Function Prototypes
  * @{
  */
static int32_t EXC80W32_DetectTouch(EXC80W32_Object_t *pObj, uint8_t *pData, uint32_t Length);
static int32_t ReadRegWrap(void *handle, uint8_t Reg, uint8_t* Data, uint16_t Length);
static int32_t WriteRegWrap(void *handle, uint8_t Reg, uint8_t* Data, uint16_t Length);

/**
  * @}
  */

/** @defgroup EXC80W32_Exported_Functions EXC80W32 Exported Functions
  * @{
  */

/**
  * @brief  Register IO bus to component object
  * @param  Component object pointer
  * @retval error status
  */
int32_t EXC80W32_RegisterBusIO (EXC80W32_Object_t *pObj, EXC80W32_IO_t *pIO)
{
  int32_t ret;
  
  if (pObj == NULL)
  {
    ret = EXC80W32_ERROR;
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
      ret = EXC80W32_ERROR;
    }
  }    
  
  return ret;
}

/**
  * @brief  Get EXC80W32 sensor capabilities
  * @param  pObj Component object pointer
  * @param  Capabilities pointer to EXC80W32 sensor capabilities
  * @retval Component status
  */
int32_t EXC80W32_GetCapabilities(EXC80W32_Object_t *pObj, EXC80W32_Capabilities_t *Capabilities)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  /* Store component's capabilities */
  Capabilities->MultiTouch = 0;
  Capabilities->Gesture    = 0;
  Capabilities->MaxTouch   = EXC80W32_MAX_NB_TOUCH;
  Capabilities->MaxXl      = EXC80W32_MAX_X_LENGTH;
  Capabilities->MaxYl      = EXC80W32_MAX_Y_LENGTH;
  
  return EXC80W32_OK;
}

/**
  * @brief  Initialize the EXC80W32 communication bus
  *         from MCU to EXC80W32 : ie I2C channel initialization (if required).
  * @param  pObj Component object pointer
  * @retval Component status
  */
int32_t EXC80W32_Init(EXC80W32_Object_t *pObj)
{
  int32_t ret = EXC80W32_OK;
  
  if(pObj->IsInitialized == 0U)
  {    
    /* Initialize IO BUS layer */
    if(pObj->IO.Init() != EXC80W32_OK)
    {
      ret = EXC80W32_ERROR;
    }
    else
    {
      pObj->IsInitialized = 1;
    }
  }
  
  return ret;
}

/**
  * @brief  De-Initialize the EXC80W32 communication bus
  *         from MCU to EXC80W32 : ie I2C channel de-initialization.
  * @param  pObj Component object pointer
  * @retval Component status
  */
int32_t EXC80W32_DeInit(EXC80W32_Object_t *pObj)
{
  int32_t ret = EXC80W32_OK;
  
  if(pObj->IsInitialized == 1U)
  {
    if(pObj->IO.DeInit() != EXC80W32_OK)
    {
      ret = EXC80W32_ERROR;
    }
    else
    {
      pObj->IsInitialized = 0;
    }
  }
  
  return ret;
}

/**
  * @brief  Configure the EXC80W32 gesture
  *         from MCU to EXC80W32 : ie I2C channel initialization (if required).
  * @param  pObj  Component object pointer
  * @param  GestureInit Gesture init structure
  * @retval Component status
  */
int32_t EXC80W32_GestureConfig(EXC80W32_Object_t *pObj, EXC80W32_Gesture_Init_t *GestureInit)
{
  /* Feature not supported */
  return EXC80W32_ERROR;  
}

/**
  * @brief  Read the EXC80W32 device ID, pre initialize I2C in case of need to be
  *         able to read the EXC80W32 device ID, and verify this is a EXC80W32.
  * @param  pObj Component object pointer
  * @param  Id Pointer to component ID value
  * @retval Component status
  */
int32_t EXC80W32_ReadID(EXC80W32_Object_t *pObj, uint32_t *Id)
{
  int32_t ret = EXC80W32_ERROR;
  uint8_t data;
  
  if(exc80w32_read_reg(&pObj->Ctx, EXC80W32_READ_REG, &data, 1) == EXC80W32_OK)
  {
    *Id = EXC80W32_ID; 
    ret = EXC80W32_OK;
  }  
  
  return ret;
}

/**
  * @brief  Get the touch screen X and Y positions values
  * @param  pObj Component object pointer
  * @param  State Single Touch stucture pointer
  * @retval Component status.
  */
int32_t EXC80W32_GetState(EXC80W32_Object_t *pObj, EXC80W32_State_t *State)
{
  uint8_t  data[10];
  
  State->TouchDetected = (uint32_t)EXC80W32_DetectTouch(pObj, data, (uint32_t)sizeof(data));
  if (State->TouchDetected == 1)
  {
    /* Send back first ready X position to caller */
    State->TouchX = (((data[6] & 0x00FFU) << 0U) | ((data[7] & 0x00FFU) << 8U)) >>2;
    /* Send back first ready Y position to caller */
    State->TouchY = (((data[8] & 0x00FFU) << 0U) | ((data[9] & 0x00FFU) << 8U)) >>2;
  }
  return EXC80W32_OK;
}

/**
  * @brief  Get the touch screen Xn and Yn positions values in multi-touch mode
  * @param  pObj Component object pointer
  * @param  State Multi Touch stucture pointer
  * @retval Component status.
  */
int32_t EXC80W32_GetMultiTouchState(EXC80W32_Object_t *pObj, EXC80W32_MultiTouch_State_t *State)
{
  uint8_t data[54];
  uint8_t TouchNb;
  State->TouchDetected = (uint32_t)EXC80W32_DetectTouch(pObj, data, (uint32_t)sizeof(data));
  if (State->TouchDetected == 1)
  {
    if (data[2] == EXC80W32_MULTITOUCH_ID)
    {
      if (data[3] <= EXC80W32_MAX_NB_TOUCH)
      {
        for (TouchNb=0;TouchNb<=data[3];TouchNb++)
        {
          /* Send back X positions to caller */
          State->TouchX[TouchNb] = (((data[6 + 10 * TouchNb] & 0x00FFU) << 0U) |
                                   ((data[7 + 10 * TouchNb] & 0x00FFU) << 8U)) >>2;
          /* Send back Y positions to caller */
          State->TouchY[TouchNb] = (((data[8 + 10 * TouchNb] & 0x00FFU) << 0U) |
                                   ((data[9 + 10 * TouchNb] & 0x00FFU) << 8U)) >>2;
        }
      }
      else
      {
        for (TouchNb=0;TouchNb<EXC80W32_MAX_NB_TOUCH;TouchNb++)
        {
          /* Send back X positions to caller */
          State->TouchX[TouchNb] = (((data[6 + 10 * TouchNb] & 0x00FFU) << 0U) |
                                   ((data[7 + 10 * TouchNb] & 0x00FFU) << 8U)) >>2;
          /* Send back Y positions to caller */
          State->TouchY[TouchNb] = (((data[8 + 10 * TouchNb] & 0x00FFU) << 0U) |
                                   ((data[9 + 10 * TouchNb] & 0x00FFU) << 8U)) >>2;
        }
      }
    }
  }
  return EXC80W32_OK;
}

/**
  * @brief  Get Gesture ID
  * @param  pObj Component object pointer
  * @param  GestureId gesture ID
  * @retval Component status.
  */
int32_t EXC80W32_GetGesture(EXC80W32_Object_t *pObj, uint8_t *GestureId)
{  
  /* Feature not supported */
  return EXC80W32_ERROR;
}

/**
  * @brief  Configure the EXC80W32 device to generate IT on given INT pin
  *         connected to MCU as EXTI.
  * @param  pObj Component object pointer
  * @retval Component status.
  */
int32_t EXC80W32_EnableIT(EXC80W32_Object_t *pObj)
{
  /* Feature not supported */
  return EXC80W32_ERROR;
}

/**
  * @brief  Configure the EXC80W32 device to stop generating IT on the given INT pin
  *         connected to MCU as EXTI.
  * @param  pObj Component object pointer
  * @retval Component status.
  */
int32_t EXC80W32_DisableIT(EXC80W32_Object_t *pObj)
{
  /* Feature not supported */
  return EXC80W32_ERROR;
}

/**
  * @brief  Get IT status from EXC80W32 interrupt status registers
  *         Should be called Following an EXTI coming to the MCU to know the detailed
  *         reason of the interrupt.
  *         @note : This feature is not applicable to EXC80W32.
  * @param  pObj Component object pointer
  * @retval Component status.
  */
int32_t EXC80W32_ITStatus(EXC80W32_Object_t *pObj)
{
  /* Feature not supported */
  return EXC80W32_ERROR;
}

/**
  * @brief  Clear IT status in EXC80W32 interrupt status clear registers
  *         Should be called Following an EXTI coming to the MCU.
  *         @note : This feature is not applicable to EXC80W32.
  * @param  pObj Component object pointer
  * @retval Component status.
  */
int32_t EXC80W32_ClearIT(EXC80W32_Object_t *pObj)
{
  /* Feature not supported */
  return EXC80W32_ERROR;
}
/**
  * @}
  */

/** @defgroup EXC80W32_Private_Functions EXC80W32 Private Functions
  * @{
  */

/**
  * @brief  Return if there is touches detected or not.
  *         Get the touch screen X and Y positions values 
  * @param  pObj Component object pointer
  * @param  pData Pointer to data buffer
  * @param  Length Number of data to read
  * @retval Number of active touches detected (can be 0 or 1) or EXC80W32_ERROR
  *         in case of error
  */
static int32_t EXC80W32_DetectTouch(EXC80W32_Object_t *pObj, uint8_t *pData, uint32_t Length)
{
  int32_t ret;
  
  /* Read TS data */
  if(exc80w32_read_reg(&pObj->Ctx, EXC80W32_READ_REG, pData, Length) != EXC80W32_OK)
  {
    ret = EXC80W32_ERROR;
  }
  else
  {
    if(pData[4] != 0xFFU)
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
  * @brief  Wrap IO bus read function to component register read function
  * @param  handle Component object handle
  * @param  Reg The target register address to read
  * @param  pData The target register value to be read
  * @param  Length buffer size to be read
  * @retval Component status.
  */
static int32_t ReadRegWrap(void *handle, uint8_t Reg, uint8_t* pData, uint16_t Length)
{
  EXC80W32_Object_t *pObj = (EXC80W32_Object_t *)handle;

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
  EXC80W32_Object_t *pObj = (EXC80W32_Object_t *)handle;

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

