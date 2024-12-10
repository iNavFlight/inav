/**
  ******************************************************************************
  * @file    st7789h2_reg.c
  * @author  MCD Application Team
  * @brief   This file provides unitary register function to control the ST7789H2
  *          LCD driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017-2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "st7789h2_reg.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup ST7789H2
  * @{
  */

/** @addtogroup ST7789H2_REG_Exported_Functions
  * @{
  */
/*******************************************************************************
* Function Name : st7789h2_read_reg
* Description   : Generic Reading function.
* Input         : Driver context, register Address, length of buffer
* Output        : Status.
*******************************************************************************/
int32_t st7789h2_read_reg(const ST7789H2_ctx_t *ctx, uint16_t reg, uint8_t *pdata, uint32_t length)
{
  return ctx->ReadReg(ctx->handle, reg, pdata, length);
}

/*******************************************************************************
* Function Name : st7789h2_write_reg
* Description   : Generic Writing function.
* Input         : Driver context, Register Address, data to be written,
*                 length of buffer.
* Output        : Status.
*******************************************************************************/
int32_t st7789h2_write_reg(const ST7789H2_ctx_t *ctx, uint16_t reg, uint8_t *pdata, uint32_t length)
{
  return ctx->WriteReg(ctx->handle, reg, pdata, length);
}

/*******************************************************************************
* Function Name : st7789h2_send_data
* Description   : Generic Send function.
* Input         : Driver context, data to be written, length of buffer.
* Output        : Status.
*******************************************************************************/
int32_t st7789h2_send_data(const ST7789H2_ctx_t *ctx, uint8_t *pdata, uint32_t length)
{
  return ctx->SendData(ctx->handle, pdata, length);
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
