/**
  ******************************************************************************
  * @file    nt35510_reg.c
  * @author  MCD Application Team
  * @brief   This file provides generic register APIs to read/write the NT35510
  *          controller's registers
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "nt35510_reg.h"

/** @addtogroup BSP
  * @{
  */
  
/** @addtogroup Components
  * @{
  */ 

/** @addtogroup NT35510
  * @{
  */
    
/* Private types -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/** @defgroup NT35510_Private_Constants NT35510 Private Constants
  * @{
  */

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/************** Generic Function  *******************/
/*******************************************************************************
* Function Name : nt35510_read_reg
* Description   : Generic Reading function. It must be fullfilled with either
*                 I2C or SPI reading functions
* Input         : Register Address, length of buffer
* Output        : Data Read
*******************************************************************************/
int32_t nt35510_read_reg(nt35510_ctx_t *ctx, uint16_t reg, uint8_t *pdata, uint16_t length)
{
  return ctx->ReadReg(ctx->handle, reg, pdata, length);
}

/*******************************************************************************
* Function Name : nt35510_write_reg
* Description   : Generic Writing function. It must be fullfilled with either
*                 I2C or SPI writing function
* Input         : Register Address, Data to be written, length of buffer
* Output        : None
*******************************************************************************/
int32_t nt35510_write_reg(nt35510_ctx_t *ctx, uint16_t reg, const uint8_t *pdata, uint16_t length)
{
  return ctx->WriteReg(ctx->handle, reg, (uint8_t *)pdata, length);
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
