/**
  ******************************************************************************
  * @file    stmpe811.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the STMPE811
  *          IO Expander component core drivers.
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
#include "stmpe811.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */ 
  
/** @defgroup STMPE811 STMPE811
  * @{
  */   

/** @defgroup STMPE811_Exported_Variables Exported Variables
  * @{
  */ 

/* Touch screen driver structure initialization */  
STMPE811_TS_Mode_t STMPE811_TS_Driver = 
{
  STMPE811_TS_Init,
  STMPE811_DeInit,
  STMPE811_TS_GestureConfig,
  STMPE811_ReadID, 
  STMPE811_TS_GetState,
  STMPE811_TS_GetMultiTouchState,
  STMPE811_TS_GetGesture,
  STMPE811_GetCapabilities,
  STMPE811_TS_EnableIT,
  STMPE811_TS_DisableIT,  
  STMPE811_TS_ClearIT,
  STMPE811_TS_ITStatus
};

/* IO driver structure initialization */ 
STMPE811_IO_Mode_t STMPE811_IO_Driver = 
{
  STMPE811_IO_Init,
  STMPE811_DeInit,  
  STMPE811_ReadID,
  STMPE811_Reset,  
  STMPE811_IO_Start,
  STMPE811_IO_WritePin,
  STMPE811_IO_ReadPin,
  STMPE811_IO_EnableIT,
  STMPE811_IO_DisableIT,
  STMPE811_IO_ITStatus,
  STMPE811_IO_ClearIT,
};

/**
  * @}
  */ 

/** @defgroup STMPE811_Private_Function_Prototypes Private Function Prototypes
  * @{
  */
static int32_t STMPE811_EnableGlobalIT(STMPE811_Object_t *pObj);
static int32_t STMPE811_DisableGlobalIT(STMPE811_Object_t *pObj);
static int32_t STMPE811_TS_DetectTouch(STMPE811_Object_t *pObj);
static int32_t STMPE811_EnableITSource(STMPE811_Object_t *pObj, uint8_t Source);
static int32_t STMPE811_DisableITSource(STMPE811_Object_t *pObj, uint8_t Source);
static int32_t STMPE811_ClearGlobalIT(STMPE811_Object_t *pObj, uint8_t Source);
static void    STMPE811_Delay(STMPE811_Object_t *pObj, uint32_t Delay);
static int32_t STMPE811_ReadRegWrap(void *handle, uint16_t Reg, uint8_t* pData, uint16_t Length);
static int32_t STMPE811_WriteRegWrap(void *handle, uint16_t Reg, uint8_t* pData, uint16_t Length);
/**
  * @}
  */ 

/** @defgroup STMPE811_Exported_Functions Exported Functions
  * @{
  */

/**
  * @brief  Register IO bus to component object
  * @param  Component object pointer
  * @retval Component status
  */
int32_t STMPE811_RegisterBusIO (STMPE811_Object_t *pObj, STMPE811_IO_t *pIO)
{
  int32_t ret;
  
  if (pObj == NULL)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    pObj->IO.Init      = pIO->Init;
    pObj->IO.DeInit    = pIO->DeInit;
    pObj->IO.Address   = pIO->Address;
    pObj->IO.WriteReg  = pIO->WriteReg;
    pObj->IO.ReadReg   = pIO->ReadReg;
    pObj->IO.GetTick   = pIO->GetTick;
    
    pObj->Ctx.ReadReg  = STMPE811_ReadRegWrap;
    pObj->Ctx.WriteReg = STMPE811_WriteRegWrap;
    pObj->Ctx.handle   = pObj;
    
    if(pObj->IO.Init != NULL)
    {
      ret = pObj->IO.Init();
    }
    else
    {
      ret = STMPE811_ERROR;
    }
  }    
  
  return ret;
}

/**
  * @brief  Initialize the stmpe811 and configure the needed hardware resources
  * @param  pObj  Pointer to component object.
  * @retval Component status
  */
int32_t STMPE811_Init(STMPE811_Object_t *pObj)
{
  int32_t ret = STMPE811_OK;
  
  if(pObj->IsInitialized == 0U)
  {    
    /* Initialize IO BUS layer */
    pObj->IO.Init();
    
    /* Generate stmpe811 Software reset */
    if(STMPE811_Reset(pObj) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    }
    else
    {
      pObj->IsInitialized = 1U;
    }
  }
  
  return ret;
}

/**
  * @brief  DeInitialize the stmpe811 and un-configure the needed hardware resources
  * @param  pObj  Pointer to component object.
  * @retval Component status
  */
int32_t STMPE811_DeInit(STMPE811_Object_t *pObj)
{
  if(pObj->IsInitialized == 1U)
  {
    /* Generate stmpe811 Software reset */
    if(STMPE811_Reset(pObj) != STMPE811_OK)
    {
      return STMPE811_ERROR;
    }
    else 
    {	
      /* De-Initialize IO BUS layer */
      pObj->IO.DeInit();
      
      pObj->IsInitialized = 0U; 
    }
  }
  
  return STMPE811_OK;  
}
 
/**
  * @brief  Reset the stmpe811 by Software.
  * @param  pObj  Pointer to component object.
  * @retval Component status
  */
int32_t STMPE811_Reset(STMPE811_Object_t *pObj)
{
  int32_t ret = STMPE811_OK;
  uint8_t tmp = 2;
  
  /* Power Down the stmpe811 */  
  if(stmpe811_write_reg(&pObj->Ctx, STMPE811_SYS_CTRL1_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    /* Wait for a delay to ensure registers erasing */
    STMPE811_Delay(pObj, 10); 
    tmp = 0;
    /* Power On the stmpe811 after the power off => all registers are reinitialized */
    if(stmpe811_write_reg(&pObj->Ctx, STMPE811_SYS_CTRL1_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    }
    else
    {
      /* Wait for a delay to ensure registers erasing */
      STMPE811_Delay(pObj, 2);
    }
  }
  
  return ret;  
}

/**
  * @brief  Read the STMPE811 IO Expander device ID.
  * @param  pObj  Pointer to component object.  
  * @retval The Device ID (two bytes).
  */
int32_t STMPE811_ReadID(STMPE811_Object_t *pObj, uint32_t *Id)
{
  int32_t ret = STMPE811_OK;  
  uint8_t id_lsb, id_msb;
  
  /* Initialize IO BUS layer */
  pObj->IO.Init();
  
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_CHP_ID_LSB_REG, &id_lsb, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else if(stmpe811_read_reg(&pObj->Ctx, STMPE811_CHP_ID_MSB_REG, &id_msb, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }  
  else
  {
    /* Store the device ID value */
    *Id = ((uint32_t)id_lsb << 8) | (uint32_t)id_msb;
  }
  
  return ret;
}


/**
  * @brief  Start the IO functionality use and disable the AF for selected IO pin(s).
  * @param  pObj  Pointer to component object. 
  * @param  IO_Pin  The IO pin(s) to put in AF. This parameter can be one 
  *         of the following values:
  *   @arg  STMPE811_PIN_x: where x can be from 0 to 7.
  * @retval Component status
  */
int32_t STMPE811_IO_Start(STMPE811_Object_t *pObj, uint32_t IO_Pin)
{
  int32_t ret = STMPE811_OK; 
  uint8_t tmp;
  
  /* Get the current register value */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_SYS_CTRL2_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }  
  else
  {
    /* Set the Functionalities to be Disabled */    
    tmp &= ~(STMPE811_IO_FCT | STMPE811_ADC_FCT);  
    
    /* Write the new register value */  
    if(stmpe811_write_reg(&pObj->Ctx, STMPE811_SYS_CTRL2_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    }   
    /* Disable AF for the selected IO pin(s) */   
    /* Get the current state of the IO_AF register */
    else if(stmpe811_read_reg(&pObj->Ctx, STMPE811_IO_AF_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    }   
    else
    {
      /* Enable the selected pins alternate function */
      tmp |= (uint8_t)IO_Pin;
      
      /* Write back the new value in IO AF register */
      if(stmpe811_write_reg(&pObj->Ctx, STMPE811_IO_AF_REG, &tmp, 1) != STMPE811_OK)
      {
        ret = STMPE811_ERROR;
      }   
    }
  }
  
  return ret;
}

/**
  * @brief  Configures the IO pin(s) according to IO mode structure value.
  * @param  pObj  Pointer to component object. 
  * @param  IO_Pin  The output pin to be set or reset. This parameter can be one 
  *         of the following values:   
  *   @arg  STMPE811_PIN_x: where x can be from 0 to 7.
  * @param  IO_Mode: The IO pin mode to configure, could be one of the following values:
  *   @arg  IO_MODE_INPUT
  *   @arg  IO_MODE_OUTPUT
  *   @arg  IO_MODE_IT_RISING_EDGE
  *   @arg  IO_MODE_IT_FALLING_EDGE
  *   @arg  IO_MODE_IT_LOW_LEVEL
  *   @arg  IO_MODE_IT_HIGH_LEVEL            
  * @retval 0 if no error, IO_Mode if error
  */
int32_t STMPE811_IO_Init(STMPE811_Object_t *pObj, STMPE811_IO_Init_t *IoInit)
{
  int32_t ret = STMPE811_OK; 
  uint8_t tmp;
  
  /* IT enable/disable */
  switch(IoInit->Mode)
  {
  case STMPE811_GPIO_MODE_OFF:
  case STMPE811_GPIO_MODE_ANALOG:  
  case STMPE811_GPIO_MODE_INPUT:   
  case STMPE811_GPIO_MODE_OUTPUT_OD:
  case STMPE811_GPIO_MODE_OUTPUT_PP:
    ret += STMPE811_IO_DisablePinIT(pObj, IoInit->Pin); /* first disable IT */
    break;
    
  case STMPE811_GPIO_MODE_IT_RISING_EDGE:
  case STMPE811_GPIO_MODE_IT_FALLING_EDGE:
  case STMPE811_GPIO_MODE_IT_LOW_LEVEL:  
  case STMPE811_GPIO_MODE_IT_HIGH_LEVEL: 
    ret += STMPE811_IO_EnableIT(pObj); /* first enable IT */
    break;
  default:
    break;
  }
  
  /* Set direction IN/OUT */
  if((IoInit->Mode == STMPE811_GPIO_MODE_OUTPUT_PP) || (IoInit->Mode == STMPE811_GPIO_MODE_OUTPUT_OD))
  {
    ret += STMPE811_IO_InitPin(pObj, IoInit->Pin, STMPE811_GPIO_DIR_OUT);
  }
  else
  {
    ret += STMPE811_IO_InitPin(pObj, IoInit->Pin, STMPE811_GPIO_DIR_IN);
  }
  
  /* Configure IT mode */
  if(IoInit->Mode >= STMPE811_GPIO_MODE_IT_RISING_EDGE)
  {
    if(STMPE811_IO_EnableIT(pObj) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    }
    else if(STMPE811_IO_EnablePinIT(pObj, IoInit->Pin) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    }/* Get the current register value */ 
    else if(stmpe811_read_reg(&pObj->Ctx, STMPE811_INT_CTRL_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    }
    else
    {
      /* Mask the polarity and type bits */
      tmp &= ~(uint8_t)0x06U;
      
      /* Modify the Interrupt Output line configuration */
      tmp |= (uint8_t)((IoInit->Mode - STMPE811_GPIO_MODE_IT_RISING_EDGE) << 1);
      
      /* Set the new register value */
      if(stmpe811_write_reg(&pObj->Ctx, STMPE811_INT_CTRL_REG, &tmp, 1) != STMPE811_OK)
      {
        ret = STMPE811_ERROR;
      }
    }    
  }
  
  return ret;
}

/**
  * @brief  Initialize the selected IO pin direction.
  * @param  pObj  Pointer to component object. 
  * @param  IO_Pin  The IO pin to be configured. This parameter could be any 
  *         combination of the following values:
  *   @arg  STMPE811_PIN_x: Where x can be from 0 to 7.   
  * @param  Direction could be STMPE811_GPIO_DIR_IN or STMPE811_GPIO_DIR_OUT.      
  * @retval Component status
  */
int32_t STMPE811_IO_InitPin(STMPE811_Object_t *pObj, uint32_t IO_Pin, uint8_t Direction)
{
  int32_t ret = STMPE811_OK; 
  uint8_t tmp;   
  
  /* Get all the Pins direction */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_IO_DIR_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    /* Set the selected pin direction */
    if (Direction != STMPE811_GPIO_DIR_IN)
    {
      tmp |= (uint8_t)IO_Pin;
    }  
    else 
    {
      tmp &= ~(uint8_t)IO_Pin;
    }
    
    /* Write the register new value */
    if(stmpe811_write_reg(&pObj->Ctx, STMPE811_IO_DIR_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    } 
  }
  
  return ret;  
}

/**
  * @brief  Write a new IO pin state.
  * @param  pObj  Pointer to component object. 
  * @param IO_Pin The output pin to be set or reset. This parameter can be one 
  *        of the following values:
  *   @arg  STMPE811_GPIO_PIN_x: where x can be from 0 to 7. 
  * @param PinState The new IO pin state.
  * @retval Component status
  */
int32_t STMPE811_IO_WritePin(STMPE811_Object_t *pObj, uint32_t IO_Pin, uint8_t PinState)
{
  int32_t ret = STMPE811_OK; 
  uint8_t tmp = (uint8_t)IO_Pin; 

  /* Apply the bit value to the selected pin */
  if (PinState != 0U)
  {
    /* Set the register */
    if(stmpe811_write_reg(&pObj->Ctx, STMPE811_IO_SET_PIN_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    }
  }
  else
  {
    /* Set the register */
    if(stmpe811_write_reg(&pObj->Ctx, STMPE811_IO_CLR_PIN_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    }
  }
  
  return ret;
}

/**
  * @brief  Return the state of the selected IO pin(s).
  * @param  pObj  Pointer to component object. 
  * @param IO_Pin The output pin to read its state. This parameter can be one 
  *        of the following values:
  *   @arg  STMPE811_PIN_x: where x can be from 0 to 7. 
  * @retval STMPE811_ERROR in case of error else IO pin(s) state.
  */
int32_t STMPE811_IO_ReadPin(STMPE811_Object_t *pObj, uint32_t IO_Pin)
{
  uint8_t tmp;

  /* Get the current register value */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_IO_MP_STA_REG, &tmp, 1) != STMPE811_OK)
  {
    return STMPE811_ERROR;
  } 
  tmp &= (uint8_t)IO_Pin;
  
  return (int32_t)tmp;
}

/**
  * @brief  Enable the AF for the selected IO pin(s).
  * @param  pObj  Pointer to component object. 
  * @param  IO_Pin  The IO pin to be configured. This parameter could be any 
  *         combination of the following values:
  *   @arg  STMPE811_PIN_x: Where x can be from 0 to 7.       
  * @retval Component status
  */
int32_t STMPE811_IO_EnableAF(STMPE811_Object_t *pObj, uint32_t IO_Pin)
{
  int32_t ret = STMPE811_OK;
  uint8_t tmp;
  
  /* Get the current register value */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_IO_AF_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  } 
  else
  {
    /* Enable the selected pins alternate function */ 
    tmp &= ~(uint8_t)IO_Pin; 
    
    /* Write back the new register value */
    if(stmpe811_write_reg(&pObj->Ctx, STMPE811_IO_AF_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    } 
  }
  
  return ret;
}

/**
  * @brief  Disable the AF for the selected IO pin(s).
  * @param  pObj  Pointer to component object. 
  * @param  IO_Pin  The IO pin to be configured. This parameter could be any 
  *         combination of the following values:
  *   @arg  STMPE811_PIN_x: Where x can be from 0 to 7.       
  * @retval Component status
  */
int32_t STMPE811_IO_DisableAF(STMPE811_Object_t *pObj, uint32_t IO_Pin)
{
  int32_t ret = STMPE811_OK;
  uint8_t tmp;
  
  /* Get the current register value */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_IO_AF_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  } 
  else
  {
    /* Disable the selected pins alternate function */   
    tmp |= (uint8_t)IO_Pin;    
    
    /* Write back the new register value */
    if(stmpe811_write_reg(&pObj->Ctx, STMPE811_IO_AF_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    } 
  }
  
  return ret;
}

/**
  * @brief  Enable the global IO interrupt source.
  * @param  pObj  Pointer to component object. 
  * @retval Component status
  */
int32_t STMPE811_IO_EnableIT(STMPE811_Object_t *pObj)
{ 
  int32_t ret;
  
  /* Enable global IO IT source */
  if(STMPE811_EnableITSource(pObj, STMPE811_GIT_IO) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  } /* Enable global interrupt */
  else if(STMPE811_EnableGlobalIT(pObj) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    ret = STMPE811_OK;
  }
  
  return ret;
}

/**
  * @brief  Disable the global IO interrupt source.
  * @param  pObj  Pointer to component object.  
  * @retval Component status
  */
int32_t STMPE811_IO_DisableIT(STMPE811_Object_t *pObj)
{
  int32_t ret;
  
  /* Disable the global interrupt */
  if(STMPE811_DisableGlobalIT(pObj) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }/* Disable global IO IT source */
  else if(STMPE811_DisableITSource(pObj, STMPE811_GIT_IO) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    ret = STMPE811_OK;
  }
  
  return ret;   
}

/**
  * @brief  Enable interrupt mode for the selected IO pin(s).
  * @param  pObj  Pointer to component object. 
  * @param  IO_Pin  The IO interrupt to be enabled. This parameter could be any 
  *         combination of the following values:
  *   @arg  STMPE811_PIN_x: where x can be from 0 to 7.
  * @retval Component status
  */
int32_t STMPE811_IO_EnablePinIT(STMPE811_Object_t *pObj, uint32_t IO_Pin)
{
  int32_t ret = STMPE811_OK;
  uint8_t tmp;
  
  /* Get the IO interrupt state */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_IO_INT_EN_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    /* Set the interrupts to be enabled */    
    tmp |= (uint8_t)IO_Pin;
    
    /* Write the register new value */
    if(stmpe811_write_reg(&pObj->Ctx, STMPE811_IO_INT_EN_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    } 
  }
  
  return ret;
}

/**
  * @brief  Disable interrupt mode for the selected IO pin(s).
  * @param  pObj  Pointer to component object. 
  * @param  IO_Pin  The IO interrupt to be disabled. This parameter could be any 
  *         combination of the following values:
  *   @arg  STMPE811_PIN_x: where x can be from 0 to 7.
  * @retval Component status
  */
int32_t STMPE811_IO_DisablePinIT(STMPE811_Object_t *pObj, uint32_t IO_Pin)
{
  int32_t ret = STMPE811_OK;
  uint8_t tmp;
  
  /* Get the IO interrupt state */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_IO_INT_EN_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    /* Set the interrupts to be enabled */    
    tmp &= ~(uint8_t)IO_Pin;
    
    /* Write the register new value */
    if(stmpe811_write_reg(&pObj->Ctx, STMPE811_IO_INT_EN_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    } 
  }
  
  return ret; 
}

/**
  * @brief  Check the status of the selected IO interrupt pending bit
  * @param  pObj  Pointer to component object. 
  * @param  IO_Pin  The IO interrupt to be checked could be:
  *   @arg  STMPE811_PIN_x Where x can be from 0 to 7.             
  * @retval Status of the checked IO pin(s).
  */
int32_t STMPE811_IO_ITStatus(STMPE811_Object_t *pObj, uint32_t IO_Pin)
{
  uint8_t tmp;

  /* Get the Interrupt status  */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_IO_INT_STA_REG, &tmp, 1) != STMPE811_OK)
  {
    return STMPE811_ERROR;
  } 
  tmp &= (uint8_t)IO_Pin;
  
  return (int32_t)tmp;
}

/**
  * @brief  Clear the selected IO interrupt pending bit(s).
  * @param  pObj  Pointer to component object. 
  * @param  IO_Pin  the IO interrupt to be cleared, could be:
  *   @arg  STMPE811_PIN_x: Where x can be from 0 to 7.            
  * @retval Component status
  */
int32_t STMPE811_IO_ClearIT(STMPE811_Object_t *pObj, uint32_t IO_Pin)
{
  int32_t ret;
  uint8_t tmp = (uint8_t)IO_Pin;
  
  /* Clear the global IO IT pending bit */
  if(STMPE811_ClearGlobalIT(pObj, STMPE811_GIT_IO) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  } /* Clear the IO IT pending bit(s) */
  else if(stmpe811_write_reg(&pObj->Ctx, STMPE811_IO_INT_STA_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }/* Clear the Edge detection pending bit*/
  else if(stmpe811_write_reg(&pObj->Ctx, STMPE811_IO_ED_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }/* Clear the Rising edge pending bit */
  else if(stmpe811_write_reg(&pObj->Ctx, STMPE811_IO_RE_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }/* Clear the Falling edge pending bit */
  else if(stmpe811_write_reg(&pObj->Ctx, STMPE811_IO_FE_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    ret = STMPE811_OK;
  }
  
  return ret;
}

/**
  * @brief  Configures the touch Screen Controller (Single point detection)
  * @param  pObj  Pointer to component object. 
  * @retval Component status.
  */
int32_t STMPE811_TS_Init(STMPE811_Object_t *pObj)
{
  int32_t ret;
  uint8_t tmp;
  
  ret = STMPE811_Reset(pObj);
  
  /* Get the current register value */
  ret += stmpe811_read_reg(&pObj->Ctx, STMPE811_SYS_CTRL2_REG, &tmp, 1);
  
  /* Set the Functionalities to be Enabled */    
  tmp &= ~(STMPE811_IO_FCT);  
  
  /* Write the new register value */  
  ret += stmpe811_write_reg(&pObj->Ctx, STMPE811_SYS_CTRL2_REG, &tmp, 1); 
  
  /* Select TSC pins in TSC alternate mode */  
  ret += STMPE811_IO_EnableAF(pObj, STMPE811_TOUCH_IO_ALL);
  
  /* Set the Functionalities to be Enabled */    
  tmp &= ~(STMPE811_TS_FCT | STMPE811_ADC_FCT);  
  
  /* Set the new register value */  
  ret += stmpe811_write_reg(&pObj->Ctx, STMPE811_SYS_CTRL2_REG, &tmp, 1); 
  
  /* Select Sample Time, bit number and ADC Reference */
  tmp = 0x48U;
  ret += stmpe811_write_reg(&pObj->Ctx, STMPE811_ADC_CTRL1_REG, &tmp, 1);
  
  /* Wait for 2 ms */
  STMPE811_Delay(pObj, 2); 
  
  /* Select the ADC clock speed: 3.25 MHz */
  tmp = 0x01U;
  ret += stmpe811_write_reg(&pObj->Ctx, STMPE811_ADC_CTRL2_REG, &tmp, 1);
  
  /* Select 2 nF filter capacitor */
  /* Configuration: 
  - Touch average control    : 4 samples
  - Touch delay time         : 500 uS
  - Panel driver setting time: 500 uS 
  */
  tmp = 0x9AU;
  ret += stmpe811_write_reg(&pObj->Ctx, STMPE811_TSC_CFG_REG, &tmp, 1); 
  
  /* Configure the Touch FIFO threshold: single point reading */
  tmp = 0x01U;
  ret += stmpe811_write_reg(&pObj->Ctx, STMPE811_FIFO_TH_REG, &tmp, 1);
  
  /* Clear the FIFO memory content. */
  ret += stmpe811_write_reg(&pObj->Ctx, STMPE811_FIFO_STA_REG, &tmp, 1);
  
  /* Put the FIFO back into operation mode  */
  tmp = 0x00U;
  ret += stmpe811_write_reg(&pObj->Ctx, STMPE811_FIFO_STA_REG, &tmp, 1);
  
  /* Set the range and accuracy pf the pressure measurement (Z) : 
  - Fractional part :7 
  - Whole part      :1 
  */
  tmp = 0x01U;
  ret += stmpe811_write_reg(&pObj->Ctx, STMPE811_TSC_FRACT_XYZ_REG, &tmp, 1);
  
  /* Set the driving capability (limit) of the device for TSC pins: 50mA */
  ret += stmpe811_write_reg(&pObj->Ctx, STMPE811_TSC_I_DRIVE_REG, &tmp, 1);
  
  /* Touch screen control configuration (enable TSC):
  - Window tracking index at 127
  - X, Y only acquisition mode
  */
  tmp = 0x73U;
  ret += stmpe811_write_reg(&pObj->Ctx, STMPE811_TSC_CTRL_REG, &tmp, 1);
  
  /*  Clear all the status pending bits if any */
  tmp = 0xFFU;
  ret += stmpe811_write_reg(&pObj->Ctx, STMPE811_INT_STA_REG, &tmp, 1);
  
  /* Wait for 2 ms delay */
  STMPE811_Delay(pObj, 2);
  
  if(ret != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  
  return ret;
}

/**
  * @brief  Configure the STMPE811 gesture
  * @param  pObj  Component object pointer
  * @param  GestureInit Gesture init structure
  * @retval STMPE811_OK
  */
int32_t STMPE811_TS_GestureConfig(STMPE811_Object_t *pObj, STMPE811_Gesture_Init_t *GestureInit)
{
  /* Feature not supported */
  (void)pObj;
  (void)GestureInit;
  return STMPE811_ERROR;  
}

/**
  * @brief  Get the touch screen X and Y positions values
  * @param  pObj  Pointer to component object. 
  * @param  State Single Touch stucture pointer  
  * @retval Component status.
  */
int32_t STMPE811_TS_GetState(STMPE811_Object_t *pObj, STMPE811_State_t *State)
{
  int32_t  ret;
  int32_t  touchDetected;
  uint8_t  tmp, data_xy[3];
  uint32_t uldata_xy;
  
  touchDetected = STMPE811_TS_DetectTouch(pObj);
  
  if(touchDetected > 0)
  {
    if(stmpe811_read_reg(&pObj->Ctx, STMPE811_TSC_DATA_NON_INC_REG, data_xy, (uint16_t) sizeof(data_xy)) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    }
    else
    {
      State->TouchDetected = 1U;
      /* Calculate positions values */
      uldata_xy = ((uint32_t)data_xy[0] << 16)|((uint32_t)data_xy[1] << 8)| (uint32_t)data_xy[2];
      State->TouchX = (uldata_xy >> 12U) & 0x00000FFFU;
      State->TouchY = uldata_xy & 0x00000FFFU;
      
      /* Reset FIFO */
      tmp = 0x01U;
      if(stmpe811_write_reg(&pObj->Ctx, STMPE811_FIFO_STA_REG, &tmp, 1) != STMPE811_OK)
      {
        ret = STMPE811_ERROR;
      }
      else
      {
        /* Enable the FIFO again */
        tmp = 0x00U;
        if(stmpe811_write_reg(&pObj->Ctx, STMPE811_FIFO_STA_REG, &tmp, 1) != STMPE811_OK)
        {
          ret = STMPE811_ERROR;
        }
        else
        {
          ret = STMPE811_OK;
        }
      }
    }
  }
  else if (touchDetected == 0)
  {
    State->TouchDetected = 0U;
    ret = STMPE811_OK;
  }
  else
  {
    ret = STMPE811_ERROR;
  }
  
  return ret;
}

/**
  * @brief  Get the touch screen Xn and Yn positions values in multi-touch mode
  * @param  pObj Component object pointer
  * @param  State Multi Touch structure pointer
  * @retval STMPE811_OK.
  */
int32_t STMPE811_TS_GetMultiTouchState(STMPE811_Object_t *pObj, STMPE811_MultiTouch_State_t *State)
{
  /* Feature not supported */
  (void)pObj;
  (void)State;
  return STMPE811_ERROR;  
}

/**
  * @brief  Get Gesture ID
  * @param  pObj Component object pointer
  * @param  GestureId: gesture ID
  * @retval Gesture ID.
  */
int32_t STMPE811_TS_GetGesture(STMPE811_Object_t *pObj, uint8_t *GestureId)
{  
  /* Feature not supported */  
  (void)pObj;
  (void)GestureId;
  return STMPE811_ERROR;
}

/**
  * @brief  Configure the selected source to generate a global interrupt or not
  * @param  pObj  Pointer to component object. 
  * @retval Component status
  */
int32_t STMPE811_TS_EnableIT(STMPE811_Object_t *pObj)
{
  int32_t ret;
  
  /* Enable TS fifo threshold IT */
  if(STMPE811_EnableITSource(pObj, STMPE811_GIT_FTH) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }/* Enable global interrupt */
  else if(STMPE811_EnableGlobalIT(pObj) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    ret = STMPE811_OK;
  }
  
  return ret;
}

/**
  * @brief  Configure the selected source to generate a global interrupt or not
  * @param  pObj  Pointer to component object.   
  * @retval Component status
  */
int32_t STMPE811_TS_DisableIT(STMPE811_Object_t *pObj)
{
  int32_t ret;
  
  /* Disable global interrupt */
  if(STMPE811_DisableGlobalIT(pObj) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }/* Disable TS fifo threshold IT */
  else if(STMPE811_DisableITSource(pObj, STMPE811_GIT_FTH) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    ret = STMPE811_OK;
  }
  
  return ret;
}

/**
  * @brief  Configure the selected source to generate a global interrupt or not
  * @param  pObj  Pointer to component object.   
  * @retval TS interrupts status
  */
int32_t STMPE811_TS_ITStatus(STMPE811_Object_t *pObj)
{
  uint8_t tmp;
  
  /* Get the Interrupt status  */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_INT_STA_REG, &tmp, 1) != STMPE811_OK)
  {
    return STMPE811_ERROR;
  } 
  tmp &= STMPE811_TS_IT;
  
  /* Return TS interrupts status */
  return (int32_t)tmp;
}

/**
  * @brief  Configure the selected source to generate a global interrupt or not
  * @param  pObj  Pointer to component object. 
  * @retval Component status
  */
int32_t STMPE811_TS_ClearIT(STMPE811_Object_t *pObj)
{
  /* Clear the global TS IT source */
  return STMPE811_ClearGlobalIT(pObj, STMPE811_TS_IT);
}

/**
  * @brief  Get STMPE811 TouchScreen capabilities
  * @param  pObj Component object pointer
  * @param  Capabilities pointer to STMPE811 TouchScreen capabilities
  * @retval Component status
  */
int32_t STMPE811_GetCapabilities(STMPE811_Object_t *pObj, STMPE811_Capabilities_t *Capabilities)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  /* Store component's capabilities */
  Capabilities->MultiTouch = 0;
  Capabilities->Gesture    = 0;
  Capabilities->MaxTouch   = STMPE811_MAX_NB_TOUCH;
  Capabilities->MaxXl      = STMPE811_MAX_X_LENGTH;
  Capabilities->MaxYl      = STMPE811_MAX_Y_LENGTH;
  
  return STMPE811_OK;
}

/**
  * @}
  */ 

/** @defgroup STMPE811_Private_Functions Private Functions
  * @{
  */

/**
  * @brief  Enable the Global interrupt.
  * @param  pObj  Pointer to component object.      
  * @retval Component status
  */
static int32_t STMPE811_EnableGlobalIT(STMPE811_Object_t *pObj)
{
  int32_t ret = STMPE811_OK;
  uint8_t tmp;
  
  /* Read the Interrupt Control register  */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_INT_CTRL_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    /* Set the global interrupts to be Enabled */    
    tmp |= (uint8_t)STMPE811_GIT_EN;
    
    /* Write Back the Interrupt Control register */
    if(stmpe811_write_reg(&pObj->Ctx, STMPE811_INT_CTRL_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    }
  }
  return ret;
}

/**
  * @brief  Disable the Global interrupt.
  * @param  pObj  Pointer to component object.     
  * @retval Component status
  */
static int32_t STMPE811_DisableGlobalIT(STMPE811_Object_t *pObj)
{
  int32_t ret = STMPE811_OK;
  uint8_t tmp;
  
  /* Read the Interrupt Control register  */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_INT_CTRL_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    /* Set the global interrupts to be Disabled */    
    tmp &= ~(uint8_t)STMPE811_GIT_EN;
    
    /* Write Back the Interrupt Control register */
    if(stmpe811_write_reg(&pObj->Ctx, STMPE811_INT_CTRL_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    }
  }
  
  return ret;
}

/**
  * @brief  Enable the interrupt mode for the selected IT source
  * @param  pObj  Pointer to component object. 
  * @param Source: The interrupt source to be configured, could be:
  *   @arg  STMPE811_GIT_IO: IO interrupt 
  *   @arg  STMPE811_GIT_ADC : ADC interrupt    
  *   @arg  STMPE811_GIT_FE : Touch Screen Controller FIFO Error interrupt
  *   @arg  STMPE811_GIT_FF : Touch Screen Controller FIFO Full interrupt      
  *   @arg  STMPE811_GIT_FOV : Touch Screen Controller FIFO Overrun interrupt     
  *   @arg  STMPE811_GIT_FTH : Touch Screen Controller FIFO Threshold interrupt   
  *   @arg  STMPE811_GIT_TOUCH : Touch Screen Controller Touch Detected interrupt  
  * @retval Component status
  */
static int32_t STMPE811_EnableITSource(STMPE811_Object_t *pObj, uint8_t Source)
{
  int32_t ret = STMPE811_OK;
  uint8_t tmp;
  
  /* Get the current value of the INT_EN register */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_INT_EN_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    /* Set the interrupts to be Enabled */    
    tmp |= Source; 
    
    /* Set the register */
    if(stmpe811_write_reg(&pObj->Ctx, STMPE811_INT_EN_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    }
  }
  
  return ret;
}

/**
  * @brief  Disable the interrupt mode for the selected IT source
  * @param  pObj  Pointer to component object. 
  * @param  Source: The interrupt source to be configured, could be:
  *   @arg  STMPE811_GIT_IO: IO interrupt 
  *   @arg  STMPE811_GIT_ADC : ADC interrupt    
  *   @arg  STMPE811_GIT_FE : Touch Screen Controller FIFO Error interrupt
  *   @arg  STMPE811_GIT_FF : Touch Screen Controller FIFO Full interrupt      
  *   @arg  STMPE811_GIT_FOV : Touch Screen Controller FIFO Overrun interrupt     
  *   @arg  STMPE811_GIT_FTH : Touch Screen Controller FIFO Threshold interrupt   
  *   @arg  STMPE811_GIT_TOUCH : Touch Screen Controller Touch Detected interrupt  
  * @retval Component status
  */
static int32_t STMPE811_DisableITSource(STMPE811_Object_t *pObj, uint8_t Source)
{
  int32_t ret = STMPE811_OK;
  uint8_t tmp;
  
  /* Get the current value of the INT_EN register */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_INT_EN_REG, &tmp, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    /* Set the interrupts to be Enabled */    
    tmp &= ~Source; 
    
    /* Set the register */
    if(stmpe811_write_reg(&pObj->Ctx, STMPE811_INT_EN_REG, &tmp, 1) != STMPE811_OK)
    {
      ret = STMPE811_ERROR;
    } 
  }
  
  return ret;  
}

/**
  * @brief  Clear the selected Global interrupt pending bit(s)
  * @param  pObj  Pointer to component object.
  * @param  Source: the Global interrupt source to be cleared, could be any combination
  *         of the following values:        
  *   @arg  STMPE811_GIT_IO: IO interrupt 
  *   @arg  STMPE811_GIT_ADC : ADC interrupt    
  *   @arg  STMPE811_GIT_FE : Touch Screen Controller FIFO Error interrupt
  *   @arg  STMPE811_GIT_FF : Touch Screen Controller FIFO Full interrupt      
  *   @arg  STMPE811_GIT_FOV : Touch Screen Controller FIFO Overrun interrupt     
  *   @arg  STMPE811_GIT_FTH : Touch Screen Controller FIFO Threshold interrupt   
  *   @arg  STMPE811_GIT_TOUCH : Touch Screen Controller Touch Detected interrupt 
  * @retval Component status
  */
static int32_t STMPE811_ClearGlobalIT(STMPE811_Object_t *pObj, uint8_t Source)
{
  /* Write 1 to the bits that have to be cleared */
  return stmpe811_write_reg(&pObj->Ctx, STMPE811_INT_STA_REG, &Source, 1);
}

/**
  * @brief  Return if there is touch detected or not.
  * @param pObj pointer to component object
  * @retval Touch detected state.
  */
static int32_t STMPE811_TS_DetectTouch(STMPE811_Object_t *pObj)
{
  int32_t ret;
  uint8_t fifo_level;

  /* Read fifo level */
  if(stmpe811_read_reg(&pObj->Ctx, STMPE811_FIFO_SIZE_REG, &fifo_level, 1) != STMPE811_OK)
  {
    ret = STMPE811_ERROR;
  }
  else
  {
    if(fifo_level > 0U)
    {
      ret = 1;
    }
    else
    {
      ret = 0;
    }
  }

  return ret;
}

/**
  * @brief This function provides accurate delay (in milliseconds)
  * @param pObj pointer to component object
  * @param Delay specifies the delay time length, in milliseconds
  * @retval STMPE811_OK
  */
static void STMPE811_Delay(STMPE811_Object_t *pObj, uint32_t Delay)
{  
  uint32_t tickstart;
  tickstart = pObj->IO.GetTick();
  while((pObj->IO.GetTick() - tickstart) < Delay)
  {
  }
}

/**
  * @brief  Wrap STMPE811 read function to Bus IO function
  * @param  handle Component object handle
  * @param  Reg The target register address to write
  * @param  pData The target register value to be written
  * @param  Length buffer size to be written
  * @retval Component status
  */
static int32_t STMPE811_ReadRegWrap(void *handle, uint16_t Reg, uint8_t* pData, uint16_t Length)
{
  STMPE811_Object_t *pObj = (STMPE811_Object_t *)handle;

  return pObj->IO.ReadReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  Wrap STMPE811 write function to Bus IO function
  * @param  handle Component object handle
  * @param  Reg The target register address to write
  * @param  pData The target register value to be written
  * @param  Length buffer size to be written
  * @retval Component status
  */
static int32_t STMPE811_WriteRegWrap(void *handle, uint16_t Reg, uint8_t* pData, uint16_t Length)
{
  STMPE811_Object_t *pObj = (STMPE811_Object_t *)handle;

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
