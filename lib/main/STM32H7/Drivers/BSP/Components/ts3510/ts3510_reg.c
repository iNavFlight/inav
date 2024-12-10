/**
  ******************************************************************************
  * @file    ts3510_reg.c
  * @author  MCD Application Team
  * @brief   This file provides unitary register function to control the ts3510 Touch
  *          
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
#include "ts3510_reg.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Component
  * @{
  */

/** @addtogroup TS3510
  * @{
  */

/** @addtogroup TS3510_Exported_Functions
 * @{
 */

/**
  * @brief  Read TS3510 component registers
  * @param  ctx component contex
  * @param  reg Register to read from
  * @param  pdata Pointer to data buffer
  * @param  length Number of data to read  
  * @retval Component status
  */
int32_t ts3510_read_reg(ts3510_ctx_t *ctx, uint8_t reg, uint8_t* pdata, uint16_t length)
{
  return ctx->ReadReg(ctx->handle, reg, pdata, length);
}

/**
  * @brief  Write TS3510 component registers
  * @param  ctx component contex
  * @param  reg Register to write to
  * @param  pdata Pointer to data buffer
  * @param  length Number of data to write  
  * @retval Component status
  */
int32_t ts3510_write_reg(ts3510_ctx_t *ctx, uint8_t reg, uint8_t *pdata, uint16_t length)
{
  return ctx->WriteReg(ctx->handle, reg, pdata, length);
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
