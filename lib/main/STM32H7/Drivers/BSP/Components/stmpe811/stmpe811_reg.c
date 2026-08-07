/**
  ******************************************************************************
  * @file    stmpe811_reg.c
  * @author  MCD Application Team
  * @brief   This file provides a set of functions needed to manage the STMPE811
  *          IO Expander component registers drivers.
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
#include "stmpe811_reg.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */ 
  
/** @addtogroup STMPE811
  * @{
  */   

/** @addtogroup STMPE811_Exported_Functions
  * @{
  */
/*******************************************************************************
* Function Name : stmpe811_read_reg
* Description   : Generic Reading function. It must be fullfilled with either
*                 I2C or SPI reading functions
* Input         : Register Address, length of buffer
* Output        : Data Read
*******************************************************************************/
int32_t stmpe811_read_reg(stmpe811_ctx_t *ctx, uint16_t reg, uint8_t* data, uint16_t length)
{
  return ctx->ReadReg(ctx->handle, reg, data, length);
}

/*******************************************************************************
* Function Name : stmpe811_write_reg
* Description   : Generic Writing function. It must be fullfilled with either
*                 I2C or SPI writing function
* Input         : Register Address, Data to be written, length of buffer
* Output        : None
*******************************************************************************/
int32_t stmpe811_write_reg(stmpe811_ctx_t *ctx, uint16_t reg, uint8_t* data, uint16_t length)
{
  return ctx->WriteReg(ctx->handle, reg, data, length);
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
