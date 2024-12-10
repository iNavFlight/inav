/**
  ******************************************************************************
  * @file    ft6x06.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the FT6X06
  *          IO Expander devices.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015-2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "ft6x06.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup FT6X06 FT6X06
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/** @defgroup FT6X06_Exported_Variables FT6X06 Exported Variables
  * @{
  */

/* Touch screen driver structure initialization */
FT6X06_TS_Drv_t FT6X06_TS_Driver =
{
  FT6X06_Init,
  FT6X06_DeInit,
  FT6X06_GestureConfig,
  FT6X06_ReadID, 
  FT6X06_GetState,
  FT6X06_GetMultiTouchState,
  FT6X06_GetGesture,
  FT6X06_GetCapabilities,
  FT6X06_EnableIT,
  FT6X06_DisableIT,  
  FT6X06_ClearIT,
  FT6X06_ITStatus
};
/**
  * @}
  */

/** @defgroup FT6X06_Private_Function_Prototypes FT6X06 Private Function Prototypes
  * @{
  */
#if (FT6X06_AUTO_CALIBRATION_ENABLED == 1)
static int32_t FT6X06_TS_Calibration(FT6X06_Object_t *pObj);
static int32_t FT6X06_Delay(FT6X06_Object_t *pObj, uint32_t Delay);
#endif /* FT6X06_AUTO_CALIBRATION_ENABLED == 1 */
static int32_t FT6X06_DetectTouch(FT6X06_Object_t *pObj);
static int32_t ReadRegWrap(const void *handle, uint8_t Reg, uint8_t* Data, uint16_t Length);
static int32_t WriteRegWrap(const void *handle, uint8_t Reg, uint8_t* Data, uint16_t Length);

/**
  * @}
  */

/** @defgroup FT6X06_Exported_Functions FT6X06 Exported Functions
  * @{
  */

/**
  * @brief  Register IO bus to component object
  * @param  Component object pointer
  * @retval error status
  */
int32_t FT6X06_RegisterBusIO (FT6X06_Object_t *pObj, FT6X06_IO_t *pIO)
{
  int32_t ret;
  
  if (pObj == NULL)
  {
    ret = FT6X06_ERROR;
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
      ret = FT6X06_ERROR;
    }
  }    
  
  return ret;
}

/**
  * @brief  Get FT6X06 sensor capabilities
  * @param  pObj Component object pointer
  * @param  Capabilities pointer to FT6X06 sensor capabilities
  * @retval Component status
  */
int32_t FT6X06_GetCapabilities(const FT6X06_Object_t *pObj, FT6X06_Capabilities_t *Capabilities)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  /* Store component's capabilities */
  Capabilities->MultiTouch = 1;
  Capabilities->Gesture    = 0;  /* Gesture feature is currently not activated on FW chipset */
  Capabilities->MaxTouch   = FT6X06_MAX_NB_TOUCH;
  Capabilities->MaxXl      = FT6X06_MAX_X_LENGTH;
  Capabilities->MaxYl      = FT6X06_MAX_Y_LENGTH;
  
  return FT6X06_OK;
}

/**
  * @brief  Initialize the FT6X06 communication bus
  *         from MCU to FT6X06 : ie I2C channel initialization (if required).
  * @param  pObj Component object pointer
  * @retval Component status
  */
int32_t FT6X06_Init(FT6X06_Object_t *pObj)
{
  int32_t ret = FT6X06_OK;
  
  if(pObj->IsInitialized == 0U)
  {    
    /* Initialize IO BUS layer */
    pObj->IO.Init();
    
#if (FT6X06_AUTO_CALIBRATION_ENABLED == 1)
    /* Hw Calibration sequence start : should be done once after each power up */
    /* This is called internal calibration of the touch screen                 */
    ret += FT6X06_TS_Calibration(pObj);
#endif /* (FT6X06_AUTO_CALIBRATION_ENABLED == 1) */    
    /* By default set FT6X06 IC in Polling mode : no INT generation on FT6X06 for new touch available */
    /* Note TS_INT is active low                                                                      */
    ret += FT6X06_DisableIT(pObj);
    
    pObj->IsInitialized = 1;
  }
  
  if(ret != FT6X06_OK)
  {
    ret = FT6X06_ERROR;
  }
  
  return ret;
}

/**
  * @brief  De-Initialize the FT6X06 communication bus
  *         from MCU to FT6X06 : ie I2C channel initialization (if required).
  * @param  pObj Component object pointer
  * @retval Component status
  */
int32_t FT6X06_DeInit(FT6X06_Object_t *pObj)
{
  if(pObj->IsInitialized == 1U)
  {
    pObj->IsInitialized = 0;
  }
  
  return FT6X06_OK;
}

/**
  * @brief  Configure the FT6X06 gesture
  *         from MCU to FT6X06 : ie I2C channel initialization (if required).
  * @param  pObj  Component object pointer
  * @param  GestureInit Gesture init structure
  * @retval Component status
  */
int32_t FT6X06_GestureConfig(FT6X06_Object_t *pObj, FT6X06_Gesture_Init_t *GestureInit)
{
  int32_t ret;
  
  ret = ft6x06_radian_value(&pObj->Ctx, (uint8_t)GestureInit->Radian);
  ret += ft6x06_offset_left_right(&pObj->Ctx, (uint8_t)GestureInit->OffsetLeftRight);
  ret += ft6x06_offset_up_down(&pObj->Ctx, (uint8_t)GestureInit->OffsetUpDown);
  ret += ft6x06_disatnce_left_right(&pObj->Ctx, (uint8_t)GestureInit->DistanceLeftRight);
  ret += ft6x06_distance_up_down(&pObj->Ctx, (uint8_t)GestureInit->DistanceUpDown);
  ret += ft6x06_distance_zoom(&pObj->Ctx, (uint8_t)GestureInit->DistanceZoom);
  
  if(ret != FT6X06_OK)
  {
    ret = FT6X06_ERROR;
  }
  
  return ret;   
}

/**
  * @brief  Read the FT6X06 device ID, pre initialize I2C in case of need to be
  *         able to read the FT6X06 device ID, and verify this is a FT6X06.
  * @param  pObj Component object pointer
  * @param  Id Pointer to component's ID
  * @retval Component status
  */
int32_t FT6X06_ReadID(FT6X06_Object_t *pObj, uint32_t *Id)
{
  int32_t ret;
  uint8_t ft6x06_id;

  ret = ft6x06_chip_id(&pObj->Ctx, &ft6x06_id);
  *Id = (uint32_t) ft6x06_id;

  return ret;
}

/**
  * @brief  Get the touch screen X and Y positions values
  * @param  pObj Component object pointer
  * @param  State Single Touch structure pointer
  * @retval Component status.
  */
int32_t FT6X06_GetState(FT6X06_Object_t *pObj, FT6X06_State_t *State)
{
  int32_t ret = FT6X06_OK;
  uint8_t  data[4];
  
  State->TouchDetected = (uint32_t)FT6X06_DetectTouch(pObj);
  if(ft6x06_read_reg(&pObj->Ctx, FT6X06_P1_XH_REG, data, (uint16_t)sizeof(data)) != FT6X06_OK)
  {
    ret = FT6X06_ERROR;
  }
  else
  {
    /* Send back first ready X position to caller */
    State->TouchX = (((uint32_t)data[0] & FT6X06_P1_XH_TP_BIT_MASK) << 8) | ((uint32_t)data[1] & FT6X06_P1_XL_TP_BIT_MASK);
    /* Send back first ready Y position to caller */
    State->TouchY = (((uint32_t)data[2] & FT6X06_P1_YH_TP_BIT_MASK) << 8) | ((uint32_t)data[3] & FT6X06_P1_YL_TP_BIT_MASK);
  }
  
  return ret;
}

/**
  * @brief  Get the touch screen Xn and Yn positions values in multi-touch mode
  * @param  pObj Component object pointer
  * @param  State Multi Touch structure pointer
  * @retval Component status.
  */
int32_t FT6X06_GetMultiTouchState(FT6X06_Object_t *pObj, FT6X06_MultiTouch_State_t *State)
{
  int32_t ret = FT6X06_OK;  
  uint8_t  data[12];
  
  State->TouchDetected = (uint32_t)FT6X06_DetectTouch(pObj);
  
  if(ft6x06_read_reg(&pObj->Ctx, FT6X06_P1_XH_REG, data, (uint16_t)sizeof(data)) != FT6X06_OK)
  {
    ret = FT6X06_ERROR;
  }
  else
  {  
    /* Send back first ready X position to caller */
    State->TouchX[0] = (((uint32_t)data[0] & FT6X06_P1_XH_TP_BIT_MASK) << 8) | ((uint32_t)data[1] & FT6X06_P1_XL_TP_BIT_MASK);
    /* Send back first ready Y position to caller */
    State->TouchY[0] = (((uint32_t)data[2] & FT6X06_P1_YH_TP_BIT_MASK) << 8) | ((uint32_t)data[3] & FT6X06_P1_YL_TP_BIT_MASK);
    /* Send back first ready Event to caller */  
    State->TouchEvent[0] = (((uint32_t)data[0] & FT6X06_P1_XH_EF_BIT_MASK) >> FT6X06_P1_XH_EF_BIT_POSITION);
    /* Send back first ready Weight to caller */  
    State->TouchWeight[0] = ((uint32_t)data[4] & FT6X06_P1_WEIGHT_BIT_MASK);
    /* Send back first ready Area to caller */  
    State->TouchArea[0] = ((uint32_t)data[5] & FT6X06_P1_MISC_BIT_MASK) >> FT6X06_P1_MISC_BIT_POSITION;
    
    /* Send back first ready X position to caller */
    State->TouchX[1] = (((uint32_t)data[6] & FT6X06_P2_XH_TP_BIT_MASK) << 8) | ((uint32_t)data[7] & FT6X06_P2_XL_TP_BIT_MASK);
    /* Send back first ready Y position to caller */
    State->TouchY[1] = (((uint32_t)data[8] & FT6X06_P2_YH_TP_BIT_MASK) << 8) | ((uint32_t)data[9] & FT6X06_P2_YL_TP_BIT_MASK);
    /* Send back first ready Event to caller */  
    State->TouchEvent[1] = (((uint32_t)data[6] & FT6X06_P2_XH_EF_BIT_MASK) >> FT6X06_P2_XH_EF_BIT_POSITION);
    /* Send back first ready Weight to caller */  
    State->TouchWeight[1] = ((uint32_t)data[10] & FT6X06_P2_WEIGHT_BIT_MASK);
    /* Send back first ready Area to caller */  
    State->TouchArea[1] = ((uint32_t)data[11] & FT6X06_P2_MISC_BIT_MASK) >> FT6X06_P2_MISC_BIT_POSITION;
  }
  
  return ret;  
}

/**
  * @brief  Get Gesture ID
  * @param  pObj Component object pointer
  * @param  GestureId gesture ID
  * @retval Component status
  */
int32_t FT6X06_GetGesture(FT6X06_Object_t *pObj, uint8_t *GestureId)
{  
  return ft6x06_gest_id(&pObj->Ctx, GestureId);
}

/**
  * @brief  Configure the FT6X06 device to generate IT on given INT pin
  *         connected to MCU as EXTI.
  * @param  pObj Component object pointer
  * @retval Component status
  */
int32_t FT6X06_EnableIT(FT6X06_Object_t *pObj)
{
  return ft6x06_g_mode(&pObj->Ctx, FT6X06_G_MODE_INTERRUPT_TRIGGER);
}

/**
  * @brief  Configure the FT6X06 device to stop generating IT on the given INT pin
  *         connected to MCU as EXTI.
  * @param  pObj Component object pointer
  * @retval Component status
  */
int32_t FT6X06_DisableIT(FT6X06_Object_t *pObj)
{
  return ft6x06_g_mode(&pObj->Ctx, FT6X06_G_MODE_INTERRUPT_POLLING);
}

/**
  * @brief  Get IT status from FT6X06 interrupt status registers
  *         Should be called Following an EXTI coming to the MCU to know the detailed
  *         reason of the interrupt.
  *         @note : This feature is not supported by FT6X06.
  * @param  pObj Component object pointer
  * @retval Component status
  */
int32_t FT6X06_ITStatus(const FT6X06_Object_t *pObj)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  /* Always return FT6X06_OK as feature not supported by FT6X06 */
  return FT6X06_OK;
}

/**
  * @brief  Clear IT status in FT6X06 interrupt status clear registers
  *         Should be called Following an EXTI coming to the MCU.
  *         @note : This feature is not supported by FT6X06.
  * @param  pObj Component object pointer
  * @retval Component status
  */
int32_t FT6X06_ClearIT(const FT6X06_Object_t *pObj)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  /* Always return FT6X06_OK as feature not supported by FT6X06 */
  return FT6X06_OK;
}

/**
  * @}
  */

/** @defgroup FT6X06_Private_Functions FT6X06 Private Functions
  * @{
  */
#if (FT6X06_AUTO_CALIBRATION_ENABLED == 1)
/**
  * @brief This function provides accurate delay (in milliseconds)
  * @param pObj pointer to component object
  * @param Delay specifies the delay time length, in milliseconds
  * @retval Component status
  */
static int32_t FT6X06_Delay(FT6X06_Object_t *pObj, uint32_t Delay)
{  
  uint32_t tickstart;
  tickstart = pObj->IO.GetTick();
  while((pObj->IO.GetTick() - tickstart) < Delay)
  {
  }
  return FT6X06_OK;
}

/**
  * @brief  Start TouchScreen calibration phase
  * @param pObj pointer to component object
  * @retval Component status
  */
static int32_t FT6X06_TS_Calibration(FT6X06_Object_t *pObj)
{
  int32_t ret = FT6X06_OK;
  uint32_t nbr_attempt;
  uint8_t read_data;
  uint8_t end_calibration = 0;
  
  /* Switch FT6X06 back to factory mode to calibrate */
  if(ft6x06_dev_mode_w(&pObj->Ctx, FT6X06_DEV_MODE_FACTORY) != FT6X06_OK)
  {
    ret = FT6X06_ERROR;
  }/* Read back the same register FT6X06_DEV_MODE_REG */
  else if(ft6x06_dev_mode_r(&pObj->Ctx, &read_data) != FT6X06_OK)
  {
    ret = FT6X06_ERROR;
  }
  else
  {
    (void)FT6X06_Delay(pObj, 300); /* Wait 300 ms */
    
    if(read_data != FT6X06_DEV_MODE_FACTORY )
    {
      /* Return error to caller */
      ret = FT6X06_ERROR;
    }
    else
    {
      /* Start calibration command */
      read_data= 0x04;
      if(ft6x06_write_reg(&pObj->Ctx, FT6X06_TD_STAT_REG, &read_data, 1) != FT6X06_OK)
      {
        ret = FT6X06_ERROR;
      }
      else
      {
        (void)FT6X06_Delay(pObj, 300); /* Wait 300 ms */
        
        /* 100 attempts to wait switch from factory mode (calibration) to working mode */
        for (nbr_attempt=0; ((nbr_attempt < 100U) && (end_calibration == 0U)) ; nbr_attempt++)
        {
          if(ft6x06_dev_mode_r(&pObj->Ctx, &read_data) != FT6X06_OK)
          {
            ret = FT6X06_ERROR;
            break;
          }
          if(read_data == FT6X06_DEV_MODE_WORKING)
          {
            /* Auto Switch to FT6X06_DEV_MODE_WORKING : means calibration have ended */
            end_calibration = 1; /* exit for loop */
          }
          
          (void)FT6X06_Delay(pObj, 200); /* Wait 200 ms */
        } 
      }
    }
  }
  
  return ret;
}
#endif /* FT6X06_AUTO_CALIBRATION_ENABLED == 1 */

/**
  * @brief  Return if there is touches detected or not.
  *         Try to detect new touches and forget the old ones (reset internal global
  *         variables).
  * @param  pObj Component object pointer
  * @retval Number of active touches detected (can be 0, 1 or 2) or FT6X06_ERROR
  *         in case of error
  */
static int32_t FT6X06_DetectTouch(FT6X06_Object_t *pObj)
{
  int32_t ret;
  uint8_t nb_touch;
  
  /* Read register FT6X06_TD_STAT_REG to check number of touches detection */
  if(ft6x06_td_status(&pObj->Ctx, &nb_touch) != FT6X06_OK)
  {
    ret = FT6X06_ERROR;
  }
  else
  {
    if(nb_touch > FT6X06_MAX_NB_TOUCH)
    {
      /* If invalid number of touch detected, set it to zero */
      ret = 0;
    }
    else
    {
      ret = (int32_t)nb_touch;
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
static int32_t ReadRegWrap(const void *handle, uint8_t Reg, uint8_t* pData, uint16_t Length)
{
  const FT6X06_Object_t *pObj = (const FT6X06_Object_t *)handle;

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
static int32_t WriteRegWrap(const void *handle, uint8_t Reg, uint8_t* pData, uint16_t Length)
{
  const FT6X06_Object_t *pObj = (const FT6X06_Object_t *)handle;

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
