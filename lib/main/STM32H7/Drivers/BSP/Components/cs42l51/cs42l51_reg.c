/**
  ******************************************************************************
  * @file    cs42l51_reg.c
  * @author  MCD Application Team
  * @brief   This file provides unitary register function to control the CS42L51 
  *          Audio Codec driver.   
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2018 STMicroelectronics.
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
#include "cs42l51_reg.h"

/** @addtogroup BSP
  * @{
  */
  
/** @addtogroup Components
  * @{
  */ 

/** @addtogroup cs42l51
  * @brief     This file provides a set of functions needed to drive the 
  *            CS42L51 audio codec.
  * @{
  */

/************** Generic Function  *******************/
/*******************************************************************************
* Function Name : cs42l51_read_reg
* Description   : Generic Reading function. It must be fullfilled with either
*                 I2C or SPI reading functions
* Input         : Register Address, length of buffer
* Output        : data Read
*******************************************************************************/
int32_t cs42l51_read_reg(cs42l51_ctx_t *ctx, uint16_t reg, uint8_t* data, uint16_t length)
{
  return ctx->ReadReg(ctx->handle, reg, data, length);
}

/*******************************************************************************
* Function Name : cs42l51_write_reg
* Description   : Generic Writing function. It must be fullfilled with either
*                 I2C or SPI writing function
* Input         : Register Address, data to be written, length of buffer
* Output        : None
*******************************************************************************/
int32_t cs42l51_write_reg(cs42l51_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length)
{ 
  return ctx->WriteReg(ctx->handle, reg, data, length);
}

/******************************************************************************/
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
