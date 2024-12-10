/**
  ******************************************************************************
  * @file    cs42l51_reg.c
  * @author  MCD Application Team
  * @brief   This file provides unitary register function to control the CS42L51
  *          Audio Codec driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017-2019 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
  * Description   : Generic Reading function. It must be fulfilled with either
  *                 I2C or SPI reading functions
  * Input         : Register Address, length of buffer
  * Output        : data Read
  *******************************************************************************/
int32_t cs42l51_read_reg(const cs42l51_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length)
{
  return ctx->ReadReg(ctx->handle, reg, data, length);
}

/*******************************************************************************
  * Function Name : cs42l51_write_reg
  * Description   : Generic Writing function. It must be fulfilled with either
  *                 I2C or SPI writing function
  * Input         : Register Address, data to be written, length of buffer
  * Output        : None
  *******************************************************************************/
int32_t cs42l51_write_reg(const cs42l51_ctx_t *ctx, uint16_t reg, uint8_t *data, uint16_t length)
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
