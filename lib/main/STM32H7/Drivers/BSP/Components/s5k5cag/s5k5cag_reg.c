/**
  ******************************************************************************
  * @file    s5k5cag_reg.c
  * @author  MCD Application Team
  * @brief   This file provides unitary register function to control the S5K5CAG 
  *          Camera driver.   
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
#include "s5k5cag_reg.h"

/** @addtogroup BSP
  * @{
  */
  
/** @addtogroup Components
  * @{
  */ 

/** @addtogroup S5K5CAG
  * @brief     This file provides a set of functions needed to drive the 
  *            S5K5CAG Camera.
  * @{
  */
  
/** @addtogroup S5K5CAG_Exported_Functions
  * @{
  */
/*******************************************************************************
* Function Name : s5k5cag_read_reg
* Description   : Generic Reading function. It must be full-filled with either
*                 I2C or SPI reading functions
* Input         : Register Address, length of buffer
* Output        : Data Read
*******************************************************************************/
int32_t s5k5cag_read_reg(s5k5cag_ctx_t *ctx, uint16_t reg, uint16_t* data, uint16_t length)
{
  int32_t ret;
  
  ret = ctx->ReadReg(ctx->handle, reg, (uint8_t *)data, length);
  
  if(ret >= 0)
  {
     *data = (uint16_t)__REV16(*data);
  }
  return ret;
}

/*******************************************************************************
* Function Name : s5k5cag_write_reg
* Description   : Generic Writing function. It must be full-filled with either
*                 I2C or SPI writing function
* Input         : Register Address, Data to be written, length of buffer
* Output        : None
*******************************************************************************/
int32_t s5k5cag_write_reg(s5k5cag_ctx_t *ctx, uint16_t reg, uint16_t *data, uint16_t length)
{
  uint16_t tmp = (uint16_t)__REV16(*data);
  
  return ctx->WriteReg(ctx->handle, reg, (uint8_t *)&tmp, length);
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
