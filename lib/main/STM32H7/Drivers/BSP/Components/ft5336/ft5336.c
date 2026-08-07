/**
  ******************************************************************************
  * @file    ft5336.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the FT5336
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
#include "ft5336.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @defgroup FT5336 FT5336
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/** @defgroup FT5336_Exported_Variables FT5336 Exported Variables
  * @{
  */

/* Touch screen driver structure initialization */
FT5336_TS_Drv_t FT5336_TS_Driver =
{
  FT5336_Init,
  FT5336_DeInit,
  FT5336_GestureConfig,
  FT5336_ReadID,
  FT5336_GetState,
  FT5336_GetMultiTouchState,
  FT5336_GetGesture,
  FT5336_GetCapabilities,
  FT5336_EnableIT,
  FT5336_DisableIT,
  FT5336_ClearIT,
  FT5336_ITStatus
};
/**
  * @}
  */

/** @defgroup FT5336_Private_Function_Prototypes FT5336 Private Function Prototypes
  * @{
  */
#if (FT5336_AUTO_CALIBRATION_ENABLED == 1)
static int32_t FT5336_TS_Calibration(FT5336_Object_t *pObj);
static int32_t FT5336_Delay(FT5336_Object_t *pObj, uint32_t Delay);
#endif /* FT5336_AUTO_CALIBRATION_ENABLED == 1 */
static int32_t FT5336_DetectTouch(FT5336_Object_t *pObj);
static int32_t ReadRegWrap(void *handle, uint8_t Reg, uint8_t* Data, uint16_t Length);
static int32_t WriteRegWrap(void *handle, uint8_t Reg, uint8_t* Data, uint16_t Length);

/**
  * @}
  */

/** @defgroup FT5336_Exported_Functions FT5336 Exported Functions
  * @{
  */

/**
  * @brief  Register IO bus to component object
  * @param  Component object pointer
  * @retval error status
  */
int32_t FT5336_RegisterBusIO (FT5336_Object_t *pObj, FT5336_IO_t *pIO)
{
  int32_t ret;

  if (pObj == NULL)
  {
    ret = FT5336_ERROR;
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
      ret = FT5336_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Get FT5336 sensor capabilities
  * @param  pObj Component object pointer
  * @param  Capabilities pointer to FT5336 sensor capabilities
  * @retval Component status
  */
int32_t FT5336_GetCapabilities(FT5336_Object_t *pObj, FT5336_Capabilities_t *Capabilities)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);

  /* Store component's capabilities */
  Capabilities->MultiTouch = 1;
  Capabilities->Gesture    = 1;
  Capabilities->MaxTouch   = FT5336_MAX_NB_TOUCH;
  Capabilities->MaxXl      = FT5336_MAX_X_LENGTH;
  Capabilities->MaxYl      = FT5336_MAX_Y_LENGTH;

  return FT5336_OK;
}

/**
  * @brief  Initialize the FT5336 communication bus
  *         from MCU to FT5336 : ie I2C channel initialization (if required).
  * @param  pObj Component object pointer
  * @param  GestureInit: Gesture init structure
  * @retval FT5336_OK
  */
int32_t FT5336_Init(FT5336_Object_t *pObj)
{
  int32_t ret = FT5336_OK;

  if(pObj->IsInitialized == 0U)
  {
    /* Initialize IO BUS layer */
    pObj->IO.Init();

#if (FT5336_AUTO_CALIBRATION_ENABLED == 1)
    /* Hw Calibration sequence start : should be done once after each power up */
    /* This is called internal calibration of the touch screen                 */
    ret += FT5336_TS_Calibration(pObj);
#endif /* (FT5336_AUTO_CALIBRATION_ENABLED == 1) */
    /* By default set FT5336 IC in Polling mode : no INT generation on FT5336 for new touch available */
    /* Note TS_INT is active low                                                                      */
    ret += FT5336_DisableIT(pObj);

    pObj->IsInitialized = 1;
  }

  if(ret != FT5336_OK)
  {
    ret = FT5336_ERROR;
  }

  return ret;
}

/**
  * @brief  De-Initialize the FT5336 communication bus
  *         from MCU to FT5336 : ie I2C channel initialization (if required).
  * @param  pObj Component object pointer
  * @retval FT5336_OK
  */
int32_t FT5336_DeInit(FT5336_Object_t *pObj)
{
  int32_t ret = FT5336_OK;

  if(pObj->IsInitialized == 1U)
  {
    pObj->IsInitialized = 0;
  }

  return ret;
}

/**
  * @brief  Configure the FT5336 gesture
  * @param  pObj  Component object pointer
  * @param  GestureInit Gesture init structure
  * @retval FT5336_OK
  */
int32_t FT5336_GestureConfig(FT5336_Object_t *pObj, FT5336_Gesture_Init_t *GestureInit)
{
  int32_t ret;

  ret = ft5336_radian_value(&pObj->Ctx, (uint8_t)GestureInit->Radian);
  ret += ft5336_offset_left_right(&pObj->Ctx, (uint8_t)GestureInit->OffsetLeftRight);
  ret += ft5336_offset_up_down(&pObj->Ctx, (uint8_t)GestureInit->OffsetUpDown);
  ret += ft5336_disatnce_left_right(&pObj->Ctx, (uint8_t)GestureInit->DistanceLeftRight);
  ret += ft5336_distance_up_down(&pObj->Ctx, (uint8_t)GestureInit->DistanceUpDown);
  ret += ft5336_distance_zoom(&pObj->Ctx, (uint8_t)GestureInit->DistanceZoom);

  if(ret != FT5336_OK)
  {
    ret = FT5336_ERROR;
  }

  return ret;
}

/**
  * @brief  Read the FT5336 device ID, pre initialize I2C in case of need to be
  *         able to read the FT5336 device ID, and verify this is a FT5336.
  * @param  pObj Component object pointer
  * @retval The Device ID (two bytes).
  */
int32_t FT5336_ReadID(FT5336_Object_t *pObj, uint32_t *Id)
{
  return ft5336_chip_id(&pObj->Ctx, (uint8_t *)Id);
}

/**
  * @brief  Get the touch screen X and Y positions values
  * @param  pObj Component object pointer
  * @param  State: Single Touch stucture pointer
  * @retval FT5336_OK.
  */
int32_t FT5336_GetState(FT5336_Object_t *pObj, FT5336_State_t *State)
{
  int32_t ret = FT5336_OK;
  uint8_t  data[4];

  State->TouchDetected = (uint32_t)FT5336_DetectTouch(pObj);
  if(ft5336_read_reg(&pObj->Ctx, FT5336_P1_XH_REG, data, (uint16_t)sizeof(data)) != FT5336_OK)
  {
    ret = FT5336_ERROR;
  }
  else
  {
    /* Send back first ready X position to caller */
    State->TouchX = (((uint32_t)data[0] & FT5336_P1_XH_TP_BIT_MASK) << 8) | ((uint32_t)data[1] & FT5336_P1_XL_TP_BIT_MASK);
    /* Send back first ready Y position to caller */
    State->TouchY = (((uint32_t)data[2] & FT5336_P1_YH_TP_BIT_MASK) << 8) | ((uint32_t)data[3] & FT5336_P1_YL_TP_BIT_MASK);
  }

  return ret;
}

/**
  * @brief  Get the touch screen Xn and Yn positions values in multi-touch mode
  * @param  pObj Component object pointer
  * @param  State Multi Touch structure pointer
  * @retval FT5336_OK.
  */
int32_t FT5336_GetMultiTouchState(FT5336_Object_t *pObj, FT5336_MultiTouch_State_t *State)
{
  int32_t ret = FT5336_OK;
  uint8_t  data[30];
  uint32_t i;

  State->TouchDetected = (uint32_t)FT5336_DetectTouch(pObj);

  if(ft5336_read_reg(&pObj->Ctx, FT5336_P1_XH_REG, data, (uint16_t)sizeof(data)) != FT5336_OK)
  {
    ret = FT5336_ERROR;
  }
  else
  {
    for(i = 0; i < FT5336_MAX_NB_TOUCH; i++)
    {
    /* Send back first ready X position to caller */
    State->TouchX[i] = (((uint32_t)data[i*6U] & FT5336_P1_XH_TP_BIT_MASK) << 8U) | ((uint32_t)data[(i*6U) + 1U] & FT5336_P1_XL_TP_BIT_MASK);
    /* Send back first ready Y position to caller */
    State->TouchY[i] = (((uint32_t)data[(i*6U) + 2U] & FT5336_P1_YH_TP_BIT_MASK) << 8U) | ((uint32_t)data[(i*6U) + 3U] & FT5336_P1_YL_TP_BIT_MASK);
    /* Send back first ready Event to caller */
    State->TouchEvent[i] = (((uint32_t)data[i*6U] & FT5336_P1_XH_EF_BIT_MASK) >> FT5336_P1_XH_EF_BIT_POSITION);
    /* Send back first ready Weight to caller */
    State->TouchWeight[i] = ((uint32_t)data[(i*6U) + 4U] & FT5336_P1_WEIGHT_BIT_MASK);
    /* Send back first ready Area to caller */
    State->TouchArea[i] = ((uint32_t)data[(i*6U) + 5U] & FT5336_P1_MISC_BIT_MASK) >> FT5336_P1_MISC_BIT_POSITION;
    }
  }

  return ret;
}

/**
  * @brief  Get Gesture ID
  * @param  pObj Component object pointer
  * @param  GestureId: gesture ID
  * @retval Gesture ID.
  */
int32_t FT5336_GetGesture(FT5336_Object_t *pObj, uint8_t *GestureId)
{
  return ft5336_gest_id(&pObj->Ctx, GestureId);
}

/**
  * @brief  Configure the FT5336 device to generate IT on given INT pin
  *         connected to MCU as EXTI.
  * @param  pObj Component object pointer
  * @retval None
  */
int32_t FT5336_EnableIT(FT5336_Object_t *pObj)
{
  return ft5336_g_mode(&pObj->Ctx, FT5336_G_MODE_INTERRUPT_TRIGGER);
}

/**
  * @brief  Configure the FT5336 device to stop generating IT on the given INT pin
  *         connected to MCU as EXTI.
  * @param  pObj Component object pointer
  * @retval None
  */
int32_t FT5336_DisableIT(FT5336_Object_t *pObj)
{
  return ft5336_g_mode(&pObj->Ctx, FT5336_G_MODE_INTERRUPT_POLLING);
}

/**
  * @brief  Get IT status from FT5336 interrupt status registers
  *         Should be called Following an EXTI coming to the MCU to know the detailed
  *         reason of the interrupt.
  *         @note : This feature is not applicable to FT5336.
  * @param  pObj Component object pointer
  * @retval TS interrupts status : always return 0 here
  */
int32_t FT5336_ITStatus(FT5336_Object_t *pObj)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);

  /* Always return FT5336_OK as feature not applicable to FT5336 */
  return FT5336_OK;
}

/**
  * @brief  Clear IT status in FT5336 interrupt status clear registers
  *         Should be called Following an EXTI coming to the MCU.
  *         @note : This feature is not applicable to FT5336.
  * @param  pObj Component object pointer
  * @retval None
  */
int32_t FT5336_ClearIT(FT5336_Object_t *pObj)
{
  /* Prevent unused argument(s) compilation warning */
  (void)(pObj);

  /* Always return FT5336_OK as feature not applicable to FT5336 */
  return FT5336_OK;
}

/******************** Static functions ****************************************/
#if (FT5336_AUTO_CALIBRATION_ENABLED == 1)
/**
  * @brief This function provides accurate delay (in milliseconds)
  * @param pObj pointer to component object
  * @param Delay: specifies the delay time length, in milliseconds
  * @retval WM8994_OK
  */
static int32_t FT5336_Delay(FT5336_Object_t *pObj, uint32_t Delay)
{
  uint32_t tickstart;
  tickstart = pObj->IO.GetTick();
  while((pObj->IO.GetTick() - tickstart) < Delay)
  {
  }
  return FT5336_OK;
}

/**
  * @brief  Start TouchScreen calibration phase
  * @param  DeviceAddr: FT5336 Device address for communication on I2C Bus.
  * @retval Status FT5336_OK or FT5336_ERROR.
  */
static int32_t FT5336_TS_Calibration(FT5336_Object_t *pObj)
{
  int32_t ret = FT5336_OK;
  uint32_t nbr_attempt;
  uint8_t read_data;
  uint8_t end_calibration = 0;

  /* Switch FT5336 back to factory mode to calibrate */
  if(ft5336_dev_mode_w(&pObj->Ctx, FT5336_DEV_MODE_FACTORY) != FT5336_OK)
  {
    ret = FT5336_ERROR;
  }/* Read back the same register FT5336_DEV_MODE_REG */
  else if(ft5336_dev_mode_r(&pObj->Ctx, &read_data) != FT5336_OK)
  {
    ret = FT5336_ERROR;
  }
  else
  {
    (void)FT5336_Delay(pObj, 300); /* Wait 300 ms */

    if(read_data != FT5336_DEV_MODE_FACTORY )
    {
      /* Return error to caller */
      ret = FT5336_ERROR;
    }
    else
    {
      /* Start calibration command */
      read_data= 0x04;
      if(ft5336_write_reg(&pObj->Ctx, FT5336_TD_STAT_REG, &read_data, 1) != FT5336_OK)
      {
        ret = FT5336_ERROR;
      }
      else
      {
        (void)FT5336_Delay(pObj, 300); /* Wait 300 ms */

        /* 100 attempts to wait switch from factory mode (calibration) to working mode */
        for (nbr_attempt=0; ((nbr_attempt < 100U) && (end_calibration == 0U)) ; nbr_attempt++)
        {
          if(ft5336_dev_mode_r(&pObj->Ctx, &read_data) != FT5336_OK)
          {
            ret = FT5336_ERROR;
            break;
          }
          if(read_data == FT5336_DEV_MODE_WORKING)
          {
            /* Auto Switch to FT5336_DEV_MODE_WORKING : means calibration have ended */
            end_calibration = 1; /* exit for loop */
          }

          (void)FT5336_Delay(pObj, 200); /* Wait 200 ms */
        }
      }
    }
  }

  return ret;
}
#endif /* FT5336_AUTO_CALIBRATION_ENABLED == 1 */

/**
  * @brief  Return if there is touches detected or not.
  *         Try to detect new touches and forget the old ones (reset internal global
  *         variables).
  * @param  pObj Component object pointer
  * @retval Number of active touches detected (can be 0, 1 or 2) or FT5336_ERROR
  *         in case of error
  */
static int32_t FT5336_DetectTouch(FT5336_Object_t *pObj)
{
  int32_t ret;
  uint8_t nb_touch;

  /* Read register FT5336_TD_STAT_REG to check number of touches detection */
  if(ft5336_td_status(&pObj->Ctx, &nb_touch) != FT5336_OK)
  {
    ret = FT5336_ERROR;
  }
  else
  {
    if(nb_touch > FT5336_MAX_NB_TOUCH)
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
  * @brief  Function
  * @param  handle: Component object handle
  * @param  Reg: The target register address to write
  * @param  pData: The target register value to be written
  * @param  Length: buffer size to be written
  * @retval error status
  */
static int32_t ReadRegWrap(void *handle, uint8_t Reg, uint8_t* pData, uint16_t Length)
{
  FT5336_Object_t *pObj = (FT5336_Object_t *)handle;

  return pObj->IO.ReadReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  Function
  * @param  handle: Component object handle
  * @param  Reg: The target register address to write
  * @param  pData: The target register value to be written
  * @param  Length: buffer size to be written
  * @retval error status
  */
static int32_t WriteRegWrap(void *handle, uint8_t Reg, uint8_t* pData, uint16_t Length)
{
  FT5336_Object_t *pObj = (FT5336_Object_t *)handle;

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
