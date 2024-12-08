/**
  ******************************************************************************
  * @file    st7789h2.c
  * @author  MCD Application Team
  * @brief   This file includes the driver for ST7789H2 LCD.
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
#include "st7789h2.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @defgroup ST7789H2
  * @{
  */

/** @defgroup ST7789H2_Private_Variables ST7789H2 Private Variables
  * @{
  */
ST7789H2_Drv_t   ST7789H2_Driver =
{
  ST7789H2_Init,
  ST7789H2_DeInit,
  ST7789H2_ReadID,
  ST7789H2_DisplayOn,
  ST7789H2_DisplayOff,
  ST7789H2_SetBrightness,
  ST7789H2_GetBrightness,
  ST7789H2_SetOrientation,
  ST7789H2_GetOrientation,
  ST7789H2_SetCursor,
  ST7789H2_DrawBitmap,
  ST7789H2_FillRGBRect,
  ST7789H2_DrawHLine,
  ST7789H2_DrawVLine,
  ST7789H2_FillRect,
  ST7789H2_GetPixel,
  ST7789H2_SetPixel,
  ST7789H2_GetXSize,
  ST7789H2_GetYSize,
};
/**
  * @}
  */

/** @defgroup ST7789H2_Private_FunctionPrototypes ST7789H2 Private FunctionPrototypes
  * @{
  */
static int32_t ST7789H2_ReadRegWrap(const void *handle, uint16_t Reg, uint8_t *pData, uint32_t Length);
static int32_t ST7789H2_WriteRegWrap(const void *handle, uint16_t Reg, uint8_t *pData, uint32_t Length);
static int32_t ST7789H2_SendDataWrap(const void *handle, uint8_t *pData, uint32_t Length);
static void    ST7789H2_Delay(const ST7789H2_Object_t *pObj, uint32_t Delay);
/**
  * @}
  */

/** @addtogroup ST7789H2_Exported_Functions
  * @{
  */
/**
  * @brief  Function to register IO bus.
  * @param  pObj Component object pointer.
  * @param  pIO  Component IO pointer.
  * @retval Error status.
  */
int32_t ST7789H2_RegisterBusIO(ST7789H2_Object_t *pObj, ST7789H2_IO_t *pIO)
{
  int32_t ret;

  if (pObj == NULL)
  {
    ret = ST7789H2_ERROR;
  }
  else
  {
    pObj->IO.Init      = pIO->Init;
    pObj->IO.DeInit    = pIO->DeInit;
    pObj->IO.Address   = pIO->Address;
    pObj->IO.WriteReg  = pIO->WriteReg;
    pObj->IO.ReadReg   = pIO->ReadReg;
    pObj->IO.SendData  = pIO->SendData;
    pObj->IO.GetTick   = pIO->GetTick;

    pObj->Ctx.ReadReg   = ST7789H2_ReadRegWrap;
    pObj->Ctx.WriteReg  = ST7789H2_WriteRegWrap;
    pObj->Ctx.SendData  = ST7789H2_SendDataWrap;
    pObj->Ctx.handle    = pObj;

    if (pObj->IO.Init != NULL)
    {
      ret = pObj->IO.Init();
    }
    else
    {
      ret = ST7789H2_ERROR;
    }
  }

  return ret;
}

/**
  * @brief  Initialize the st7789h2 LCD component.
  * @param  pObj pointer to component object.
  * @param  ColorCoding Color coding.
  * @param  Orientation Orientation.
  * @retval Component status.
  */
int32_t ST7789H2_Init(ST7789H2_Object_t *pObj, uint32_t ColorCoding, uint32_t Orientation)
{
  int32_t ret = ST7789H2_OK;
  uint8_t parameter[28];

  if (pObj->IsInitialized == 0U)
  {
    /* Sleep In Command */
    parameter[1] = 0;
    parameter[0] = ST7789H2_SLEEP_IN;
    ret += st7789h2_send_data(&pObj->Ctx, parameter, 1);
    /* Wait for 10ms */
    ST7789H2_Delay(pObj, 10);

    /* SW Reset Command */
    parameter[0] = ST7789H2_SW_RESET;
    ret += st7789h2_send_data(&pObj->Ctx, parameter, 1);
    /* Wait for 200ms */
    ST7789H2_Delay(pObj, 200);

    /* Sleep Out Command */
    parameter[0] = ST7789H2_SLEEP_OUT;
    ret += st7789h2_send_data(&pObj->Ctx, parameter, 1);
    /* Wait for 120ms */
    ST7789H2_Delay(pObj, 120);

    /* Memory access control */
    if (Orientation == ST7789H2_ORIENTATION_PORTRAIT)
    {
      parameter[0] = 0x00; /* MY = 0, MX = 0, MV = 0 */
    }
    else if (Orientation == ST7789H2_ORIENTATION_LANDSCAPE)
    {
      parameter[0] = 0xA0; /* MY = 1, MX = 0, MV = 1 */
    }
    else if (Orientation == ST7789H2_ORIENTATION_PORTRAIT_ROT180)
    {
      parameter[0] = 0xC0; /* MY = 1, MX = 1, MV = 0 */
    }
    else /* Orientation == ST7789H2_ORIENTATION_LANDSCAPE_ROT180 */
    {
      parameter[0] = 0x60; /* MY = 0, MX = 1, MV = 1 */
    }
    parameter[1] = 0x00;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_MADCTL, parameter, 1);

    /* Color mode 16bits/pixel */
    parameter[0] = (uint8_t) ColorCoding;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_COLOR_MODE, parameter, 1);

    /* Display inversion On */
    parameter[0] = ST7789H2_DISPLAY_INVERSION_ON;
    ret += st7789h2_send_data(&pObj->Ctx, parameter, 1);

    /* Set Column address CASET */
    if (Orientation == ST7789H2_ORIENTATION_LANDSCAPE)
    {
      parameter[0] = 0x00; /* XS[15:8] */
      parameter[1] = 0x00;
      parameter[2] = 0x50; /* XS[7:0] */
      parameter[3] = 0x00;
      parameter[4] = 0x01; /* XE[15:8] */
      parameter[5] = 0x00;
      parameter[6] = 0x3F; /* XE[7:0] */
      parameter[7] = 0x00;
    }
    else
    {
      parameter[0] = 0x00; /* XS[15:8] */
      parameter[1] = 0x00;
      parameter[2] = 0x00; /* XS[7:0] */
      parameter[3] = 0x00;
      parameter[4] = 0x00; /* XE[15:8] */
      parameter[5] = 0x00;
      parameter[6] = 0xEF; /* XE[7:0] */
      parameter[7] = 0x00;
    }
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_CASET, parameter, 4);
    /* Set Row address RASET */
    if (Orientation == ST7789H2_ORIENTATION_PORTRAIT_ROT180)
    {
      parameter[0] = 0x00; /* YS[15:8] */
      parameter[1] = 0x00;
      parameter[2] = 0x50; /* YS[7:0] */
      parameter[3] = 0x00;
      parameter[4] = 0x01; /* YE[15:8] */
      parameter[5] = 0x00;
      parameter[6] = 0x3F; /* YE[7:0] */
      parameter[7] = 0x00;
    }
    else
    {
      parameter[0] = 0x00; /* YS[15:8] */
      parameter[1] = 0x00;
      parameter[2] = 0x00; /* YS[7:0] */
      parameter[3] = 0x00;
      parameter[4] = 0x00; /* YE[15:8] */
      parameter[5] = 0x00;
      parameter[6] = 0xEF; /* YE[7:0] */
      parameter[7] = 0x00;
    }
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_RASET, parameter, 4);

    /*--------------- ST7789H2 Frame rate setting ----------------------------*/
    /* PORCH control setting */
    parameter[0] = 0x0C;
    parameter[1] = 0x00;
    parameter[2] = 0x0C;
    parameter[3] = 0x00;
    parameter[4] = 0x00;
    parameter[5] = 0x00;
    parameter[6] = 0x33;
    parameter[7] = 0x00;
    parameter[8] = 0x33;
    parameter[9] = 0x00;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_PORCH_CTRL, parameter, 5);

    /* GATE control setting */
    parameter[0] = 0x35;
    parameter[1] = 0x00;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_GATE_CTRL, parameter, 1);

    /*--------------- ST7789H2 Power setting ---------------------------------*/
    /* VCOM setting */
    parameter[0] = 0x1F;
    parameter[1] = 0x00;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_VCOM_SET, parameter, 1);

    /* LCM Control setting */
    parameter[0] = 0x2C;
    parameter[1] = 0x00;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_LCM_CTRL, parameter, 1);

    /* VDV and VRH Command Enable */
    parameter[0] = 0x01;
    parameter[1] = 0x00;
    parameter[2] = 0xC3;
    parameter[3] = 0x00;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_VDV_VRH_EN, parameter, 2);

    /* VDV Set */
    parameter[0] = 0x20;
    parameter[1] = 0x00;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_VDV_SET, parameter, 1);

    /* Frame Rate Control in normal mode */
    parameter[0] = 0x0F;
    parameter[1] = 0x00;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_FR_CTRL, parameter, 1);

    /* Power Control */
    parameter[0] = 0xA4;
    parameter[1] = 0x00;
    parameter[2] = 0xA1;
    parameter[3] = 0x00;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_POWER_CTRL, parameter, 2);

    /*--------------- ST7789H2 Gamma setting ---------------------------------*/
    /* Positive Voltage Gamma Control */
    parameter[0]  = 0xD0;
    parameter[1]  = 0x00;
    parameter[2]  = 0x08;
    parameter[3]  = 0x00;
    parameter[4]  = 0x11;
    parameter[5]  = 0x00;
    parameter[6]  = 0x08;
    parameter[7]  = 0x00;
    parameter[8]  = 0x0C;
    parameter[9]  = 0x00;
    parameter[10] = 0x15;
    parameter[11] = 0x00;
    parameter[12] = 0x39;
    parameter[13] = 0x00;
    parameter[14] = 0x33;
    parameter[15] = 0x00;
    parameter[16] = 0x50;
    parameter[17] = 0x00;
    parameter[18] = 0x36;
    parameter[19] = 0x00;
    parameter[20] = 0x13;
    parameter[21] = 0x00;
    parameter[22] = 0x14;
    parameter[23] = 0x00;
    parameter[24] = 0x29;
    parameter[25] = 0x00;
    parameter[26] = 0x2D;
    parameter[27] = 0x00;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_PV_GAMMA_CTRL, parameter, 14);

    /* Negative Voltage Gamma Control */
    parameter[0]  = 0xD0;
    parameter[1]  = 0x00;
    parameter[2]  = 0x08;
    parameter[3]  = 0x00;
    parameter[4]  = 0x10;
    parameter[5]  = 0x00;
    parameter[6]  = 0x08;
    parameter[7]  = 0x00;
    parameter[8]  = 0x06;
    parameter[9]  = 0x00;
    parameter[10] = 0x06;
    parameter[11] = 0x00;
    parameter[12] = 0x39;
    parameter[13] = 0x00;
    parameter[14] = 0x44;
    parameter[15] = 0x00;
    parameter[16] = 0x51;
    parameter[17] = 0x00;
    parameter[18] = 0x0B;
    parameter[19] = 0x00;
    parameter[20] = 0x16;
    parameter[21] = 0x00;
    parameter[22] = 0x14;
    parameter[23] = 0x00;
    parameter[24] = 0x2F;
    parameter[25] = 0x00;
    parameter[26] = 0x31;
    parameter[27] = 0x00;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_NV_GAMMA_CTRL, parameter, 14);

    /* Tearing Effect Line On: Option (00h:VSYNC Interface OFF, 01h:VSYNC Interface ON) */
    parameter[0] = 0x01;
    parameter[1] = 0x00;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_TE_LINE_ON, parameter, 1);

    pObj->IsInitialized = 1U;
    pObj->Orientation   = Orientation;
  }

  if (ret != ST7789H2_OK)
  {
    ret = ST7789H2_ERROR;
  }

  return ret;
}

/**
  * @brief  De-Initialize the st7789h2 LCD Component.
  * @param  pObj pointer to component object.
  * @retval Component status.
  */
int32_t ST7789H2_DeInit(ST7789H2_Object_t *pObj)
{
  int32_t ret = ST7789H2_OK;
  uint8_t parameter[2];

  if (pObj->IsInitialized != 0U)
  {
    ret += ST7789H2_DisplayOff(pObj);

    /* Power Off sequence ----------------------------------------------------*/
    /* Sleep In Command */
    parameter[1] = 0;
    parameter[0] = ST7789H2_SLEEP_IN;
    ret += st7789h2_send_data(&pObj->Ctx, parameter, 1);
    /* Wait for 10ms */
    ST7789H2_Delay(pObj, 10);

    pObj->IsInitialized = 0;
  }

  if (ret != ST7789H2_OK)
  {
    ret = ST7789H2_ERROR;
  }

  return ret;
}

/**
  * @brief  Get the st7789h2 ID.
  * @param  pObj pointer to component object.
  * @param  Id   pointer to component id.
  * @retval Component status.
  */
int32_t ST7789H2_ReadID(ST7789H2_Object_t *pObj, uint32_t *Id)
{
  int32_t ret;
  uint8_t st7789h2_id[4] = {0};

  /* Get ID from component */
  ret = st7789h2_read_reg(&pObj->Ctx, ST7789H2_READ_ID1, st7789h2_id, 2);

  *Id = (uint32_t)st7789h2_id[2] | ((uint32_t)st7789h2_id[3] << 8U);

  if (ret != ST7789H2_OK)
  {
    ret = ST7789H2_ERROR;
  }

  return ret;
}

/**
  * @brief  Set the display on.
  * @param  pObj pointer to component object.
  * @retval Component status.
  */
int32_t ST7789H2_DisplayOn(ST7789H2_Object_t *pObj)
{
  int32_t ret = ST7789H2_OK;
  uint8_t parameter[2];

  parameter[1] = 0x00U;

  /* Display ON command */
  parameter[0] = ST7789H2_DISPLAY_ON;
  ret += st7789h2_send_data(&pObj->Ctx, parameter, 1);

  /* Sleep Out command */
  parameter[0] = ST7789H2_SLEEP_OUT;
  ret += st7789h2_send_data(&pObj->Ctx, parameter, 1);

  if (ret != ST7789H2_OK)
  {
    ret = ST7789H2_ERROR;
  }

  return ret;
}

/**
  * @brief  Set the display off.
  * @param  pObj pointer to component object.
  * @retval Component status.
  */
int32_t ST7789H2_DisplayOff(ST7789H2_Object_t *pObj)
{
  int32_t ret = ST7789H2_OK;
  uint8_t parameter[2];

  /* Display OFF command */
  parameter[0] = 0xFEU;
  parameter[1] = 0x00U;
  ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_DISPLAY_OFF_PWR_SAVE, parameter, 1);

  /* Sleep In Command */
  parameter[0] = ST7789H2_SLEEP_IN;
  ret += st7789h2_send_data(&pObj->Ctx, parameter, 1);

  /* Wait for 10ms */
  ST7789H2_Delay(pObj, 10);

  if (ret != ST7789H2_OK)
  {
    ret = ST7789H2_ERROR;
  }

  return ret;
}

/**
  * @brief  Set the display brightness.
  * @param  pObj Pointer to component object.
  * @param  Brightness Display brightness to be set.
  * @retval Component status.
  */
int32_t ST7789H2_SetBrightness(const ST7789H2_Object_t *pObj, uint32_t Brightness)
{
  /* Feature not supported */
  (void)pObj;
  (void)Brightness;
  return ST7789H2_ERROR;
}

/**
  * @brief  Get the display brightness.
  * @param  pObj Pointer to component object.
  * @param  Brightness Current display brightness.
  * @retval Component status.
  */
int32_t ST7789H2_GetBrightness(const ST7789H2_Object_t *pObj, const uint32_t *Brightness)
{
  /* Feature not supported */
  (void)pObj;
  (void)Brightness;
  return ST7789H2_ERROR;
}

/**
  * @brief  Set the display orientation.
  * @param  pObj Pointer to component object.
  * @param  Orientation Display orientation to be set.
  * @retval Component status.
  */
int32_t ST7789H2_SetOrientation(ST7789H2_Object_t *pObj, uint32_t Orientation)
{
  int32_t ret = ST7789H2_OK;
  uint8_t   parameter[2];

  /* Memory access control */
  if (Orientation == ST7789H2_ORIENTATION_PORTRAIT)
  {
    parameter[0] = 0x00U; /* MY = 0, MX = 0, MV = 0 */
  }
  else if (Orientation == ST7789H2_ORIENTATION_LANDSCAPE)
  {
    parameter[0] = 0xA0U; /* MY = 1, MX = 0, MV = 1 */
  }
  else if (Orientation == ST7789H2_ORIENTATION_PORTRAIT_ROT180)
  {
    parameter[0] = 0xC0U; /* MY = 1, MX = 1, MV = 0 */
  }
  else /* Orientation == ST7789H2_ORIENTATION_LANDSCAPE_ROT180 */
  {
    parameter[0] = 0x60U; /* MY = 0, MX = 1, MV = 1 */
  }
  parameter[1] = 0x00U;
  ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_MADCTL, parameter, 1);

  pObj->Orientation = Orientation;

  return ret;
}

/**
  * @brief  Get the display orientation.
  * @param  pObj Pointer to component object.
  * @param  Orientation Current display orientation.
  * @retval Component status.
  */
int32_t ST7789H2_GetOrientation(ST7789H2_Object_t *pObj, uint32_t *Orientation)
{
  int32_t ret = ST7789H2_OK;

  *Orientation = pObj->Orientation;

  return ret;
}

/**
  * @brief  Set cursor.
  * @param  pObj Pointer to component object.
  * @param  Xpos X position on LCD.
  * @param  Ypos Y position on LCD.
  * @retval Component status.
  */
int32_t ST7789H2_SetCursor(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos)
{
  int32_t ret = ST7789H2_OK;
  uint8_t parameter[8];

  /* CASET: Column Address Set */
  if (pObj->Orientation == ST7789H2_ORIENTATION_LANDSCAPE)
  {
    parameter[0] = (uint8_t)((Xpos + 0x50U) >> 8);  /* XS[15:8] */
    parameter[1] = 0x00;
    parameter[2] = (uint8_t)(Xpos + 0x50U);         /* XS[7:0] */
    parameter[3] = 0x00;
    parameter[4] = 0x01;                            /* XE[15:8] */
    parameter[5] = 0x00;
    parameter[6] = 0x3F;                            /* XE[7:0] */
    parameter[7] = 0x00;
  }
  else
  {
    parameter[0] = (uint8_t)(Xpos >> 8);  /* XS[15:8] */
    parameter[1] = 0x00;
    parameter[2] = (uint8_t) Xpos;        /* XS[7:0] */
    parameter[3] = 0x00;
    parameter[4] = 0x00;                  /* XE[15:8] */
    parameter[5] = 0x00;
    parameter[6] = 0xEF;                  /* XE[7:0] */
    parameter[7] = 0x00;
  }
  ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_CASET, parameter, 4);

  /* RASET: Row Address Set */
  if (pObj->Orientation == ST7789H2_ORIENTATION_PORTRAIT_ROT180)
  {
    parameter[0] = (uint8_t)((Ypos + 0x50U) >> 8);  /* YS[15:8] */
    parameter[1] = 0x00;
    parameter[2] = (uint8_t)(Ypos + 0x50U);         /* YS[7:0] */
    parameter[3] = 0x00;
    parameter[4] = 0x01;                            /* YE[15:8] */
    parameter[5] = 0x00;
    parameter[6] = 0x3F;                            /* YE[7:0] */
    parameter[7] = 0x00;
  }
  else
  {
    parameter[0] = (uint8_t)(Ypos >> 8);  /* YS[15:8] */
    parameter[1] = 0x00;
    parameter[2] = (uint8_t) Ypos;        /* YS[7:0] */
    parameter[3] = 0x00;
    parameter[4] = 0x00;                  /* YE[15:8] */
    parameter[5] = 0x00;
    parameter[6] = 0xEF;                  /* YE[7:0] */
    parameter[7] = 0x00;
  }
  ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_RASET, parameter, 4);

  if (ret != ST7789H2_OK)
  {
    ret = ST7789H2_ERROR;
  }

  return ret;
}

/**
  * @brief  Display a bitmap picture.
  * @param  pObj Pointer to component object.
  * @param  Xpos X position on LCD.
  * @param  Ypos Y position on LCD.
  * @param  pBmp Pointer to bitmap.
  * @retval Component status.
  */
int32_t ST7789H2_DrawBitmap(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp)
{
  int32_t  ret = ST7789H2_OK;
  uint8_t  parameter[8];
  uint32_t index, size;
  uint32_t width, height;
  uint32_t Ystart, Ystop;

  /* Read file size */
  size = ((uint32_t)pBmp[5] << 24) | ((uint32_t)pBmp[4] << 16) | ((uint32_t)pBmp[3] << 8) | (uint32_t)pBmp[2];
  /* Get bitmap data address offset */
  index = ((uint32_t)pBmp[13] << 24) | ((uint32_t)pBmp[12] << 16) | ((uint32_t)pBmp[11] << 8) | (uint32_t)pBmp[10];
  /* Get image width */
  width = ((uint32_t)pBmp[21] << 24) | ((uint32_t)pBmp[20] << 16) | ((uint32_t)pBmp[19] << 8) | (uint32_t)pBmp[18];
  width--;
  /* Get image height */
  height = ((uint32_t)pBmp[25] << 24) | ((uint32_t)pBmp[24] << 16) | ((uint32_t)pBmp[23] << 8) | (uint32_t)pBmp[22];
  height--;
  /* Get size of data */
  size = size - index;
  size = size / 2U;

  /* Compute new Y start and stop values */
  if (pObj->Orientation == ST7789H2_ORIENTATION_PORTRAIT)
  {
    Ystart = 319U - (Ypos + height);
    Ystop  = 319U - Ypos;
  }
  else if (pObj->Orientation == ST7789H2_ORIENTATION_PORTRAIT_ROT180)
  {
    Ystart = 319U - (Ypos + 0x50U + height);
    Ystop  = 319U - (Ypos + 0x50U);
  }
  else
  {
    Ystart = 239U - (Ypos + height);
    Ystop  = 239U - Ypos;
  }

  /* Set GRAM Area - Partial Display Control */
  if (pObj->Orientation == ST7789H2_ORIENTATION_LANDSCAPE)
  {
    parameter[0] = (uint8_t)((Xpos + 0x50U) >> 8);          /* XS[15:8] */
    parameter[1] = 0x00;
    parameter[2] = (uint8_t)(Xpos + 0x50U);                 /* XS[7:0] */
    parameter[3] = 0x00;
    parameter[4] = (uint8_t)((Xpos + width + 0x50U) >> 8);  /* XE[15:8] */
    parameter[5] = 0x00;
    parameter[6] = (uint8_t)(Xpos + width + 0x50U);         /* XE[7:0] */
    parameter[7] = 0x00;
  }
  else
  {
    parameter[0] = (uint8_t)(Xpos >> 8);            /* XS[15:8] */
    parameter[1] = 0x00;
    parameter[2] = (uint8_t) Xpos;                  /* XS[7:0] */
    parameter[3] = 0x00;
    parameter[4] = (uint8_t)((Xpos + width) >> 8);  /* XE[15:8] */
    parameter[5] = 0x00;
    parameter[6] = (uint8_t)(Xpos + width);         /* XE[7:0] */
    parameter[7] = 0x00;
  }
  ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_CASET, parameter, 4);
  parameter[0] = (uint8_t)(Ystart >> 8);  /* YS[15:8] */
  parameter[1] = 0x00;
  parameter[2] = (uint8_t) Ystart;        /* YS[7:0] */
  parameter[3] = 0x00;
  parameter[4] = (uint8_t)(Ystop >> 8);   /* YE[15:8] */
  parameter[5] = 0x00;
  parameter[6] = (uint8_t) Ystop;         /* YE[7:0] */
  parameter[7] = 0x00;
  ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_RASET, parameter, 4);
  if ((pObj->Orientation == ST7789H2_ORIENTATION_PORTRAIT) || (pObj->Orientation == ST7789H2_ORIENTATION_PORTRAIT_ROT180))
  {
    /* Memory access control: Invert MY */
    parameter[0] = (pObj->Orientation == ST7789H2_ORIENTATION_PORTRAIT) ? 0x80U : 0x40U;
    parameter[1] = 0x00U;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_MADCTL, parameter, 1);
  }
  else
  {
    /* Memory access control: Invert MX */
    parameter[0] = (pObj->Orientation == ST7789H2_ORIENTATION_LANDSCAPE) ? 0xE0U : 0x20U;
    parameter[1] = 0x00U;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_MADCTL, parameter, 1);
  }

  /* Write GRAM */
  ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_WRITE_RAM, &pBmp[index], size);

  /* Restore GRAM Area - Partial Display Control */
  if (pObj->Orientation == ST7789H2_ORIENTATION_LANDSCAPE)
  {
    parameter[0] = 0x00; /* XS[15:8] */
    parameter[1] = 0x00;
    parameter[2] = 0x50; /* XS[7:0] */
    parameter[3] = 0x00;
    parameter[4] = 0x01; /* XE[15:8] */
    parameter[5] = 0x00;
    parameter[6] = 0x3F; /* XE[7:0] */
    parameter[7] = 0x00;
  }
  else
  {
    parameter[0] = 0x00; /* XS[15:8] */
    parameter[1] = 0x00;
    parameter[2] = 0x00; /* XS[7:0] */
    parameter[3] = 0x00;
    parameter[4] = 0x00; /* XE[15:8] */
    parameter[5] = 0x00;
    parameter[6] = 0xEF; /* XE[7:0] */
    parameter[7] = 0x00;
  }
  ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_CASET, parameter, 4);
  if (pObj->Orientation == ST7789H2_ORIENTATION_PORTRAIT_ROT180)
  {
    parameter[0] = 0x00; /* YS[15:8] */
    parameter[1] = 0x00;
    parameter[2] = 0x50; /* YS[7:0] */
    parameter[3] = 0x00;
    parameter[4] = 0x01; /* YE[15:8] */
    parameter[5] = 0x00;
    parameter[6] = 0x3F; /* YE[7:0] */
    parameter[7] = 0x00;
  }
  else
  {
    parameter[0] = 0x00; /* YS[15:8] */
    parameter[1] = 0x00;
    parameter[2] = 0x00; /* YS[7:0] */
    parameter[3] = 0x00;
    parameter[4] = 0x00; /* YE[15:8] */
    parameter[5] = 0x00;
    parameter[6] = 0xEF; /* YE[7:0] */
    parameter[7] = 0x00;
  }
  ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_RASET, parameter, 4);
  if ((pObj->Orientation == ST7789H2_ORIENTATION_PORTRAIT) || (pObj->Orientation == ST7789H2_ORIENTATION_PORTRAIT_ROT180))
  {
    /* Memory access control: Re-invert MY */
    parameter[0] = (pObj->Orientation == ST7789H2_ORIENTATION_PORTRAIT) ? 0x00U : 0xC0U;
    parameter[1] = 0x00U;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_MADCTL, parameter, 1);
  }
  else
  {
    /* Memory access control: Re-invert MX */
    parameter[0] = (pObj->Orientation == ST7789H2_ORIENTATION_LANDSCAPE) ? 0xA0U : 0x60U;
    parameter[1] = 0x00U;
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_MADCTL, parameter, 1);
  }

  if (ret != ST7789H2_OK)
  {
    ret = ST7789H2_ERROR;
  }

  return ret;
}

/**
  * @brief  Fill rectangle with RGB buffer.
  * @param  pObj Pointer to component object.
  * @param  Xpos X position on LCD.
  * @param  Ypos Y position on LCD.
  * @param  pData Pointer on RGB pixels buffer.
  * @param  Width Width of the rectangle.
  * @param  Height Height of the rectangle.
  * @retval Component status.
  */
int32_t ST7789H2_FillRGBRect(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint8_t *pData, uint32_t Width, uint32_t Height)
{
  int32_t  ret = ST7789H2_OK;
  uint8_t  buffer[480];
  uint8_t *rect; 
  uint32_t i, j;

  rect = pData;
  
  for(i = 0; i < Height; i++)
  {
    /* Set Cursor */
    ret += ST7789H2_SetCursor(pObj, Xpos, Ypos + i);

    /* Sent a complete line */
    for(j = 0; j < Width; j++)
    {
      buffer[2U*j]      = *rect;
      rect++;
      buffer[(2U*j)+1U] = *rect;
      rect++;
    }
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_WRITE_RAM, buffer, Width);
  }

  if(ret != ST7789H2_OK)
  {
    ret = ST7789H2_ERROR;
  }

  return ret;
}

/**
  * @brief  Display a horizontal line.
  * @param  pObj Pointer to component object.
  * @param  Xpos X position on LCD.
  * @param  Ypos Y position on LCD.
  * @param  Length Length of the line.
  * @param  Color  Color of the line.
  * @retval Component status.
  */
int32_t ST7789H2_DrawHLine(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color)
{
  int32_t  ret = ST7789H2_OK;
  uint8_t  parameter[2];
  uint32_t i;

  /* Set Cursor */
  ret += ST7789H2_SetCursor(pObj, Xpos, Ypos);

  /* Sent a complete line */
  parameter[0] = (uint8_t)(Color & 0xFFU);
  parameter[1] = (uint8_t)((Color >> 8) & 0xFFU);
  ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_WRITE_RAM, parameter, 1);
  for (i = 1; i < Length; i++)
  {
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_WRITE_RAM_CONTINUE, parameter, 1);
  }

  /* Workaround for last pixel */
  if ((Xpos + Length) == 240U)
  {
    /* Write last pixel */
    ret += ST7789H2_SetCursor(pObj, (Xpos + Length - 1U), Ypos);
    ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_WRITE_RAM, parameter, 1);
  }
  
  if (ret != ST7789H2_OK)
  {
    ret = ST7789H2_ERROR;
  }

  return ret;
}

/**
  * @brief  Display a vertical line.
  * @param  pObj Pointer to component object.
  * @param  Xpos X position on LCD.
  * @param  Ypos Y position on LCD.
  * @param  Length Length of the line.
  * @param  Color  Color of the line.
  * @retval Component status.
  */
int32_t ST7789H2_DrawVLine(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color)
{
  int32_t  ret = ST7789H2_OK;
  uint32_t i;

  /* Sent a complete line */
  for (i = 0; i < Length; i++)
  {
    ret += ST7789H2_SetPixel(pObj, Xpos, (Ypos + i), Color);
  }

  if (ret != ST7789H2_OK)
  {
    ret = ST7789H2_ERROR;
  }

  return ret;
}

/**
  * @brief  Fill rectangle.
  * @param  pObj Pointer to component object.
  * @param  Xpos X position on LCD.
  * @param  Ypos Y position on LCD.
  * @param  Width Width of the rectangle.
  * @param  Height Height of the rectangle.
  * @param  Color  Color of the rectangle.
  * @retval Component status.
  */
int32_t ST7789H2_FillRect(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color)
{
  int32_t  ret = ST7789H2_OK;
  uint32_t i;

  for (i = 0U; i < Height; i++)
  {
    if (ST7789H2_DrawHLine(pObj, Xpos, (i + Ypos), Width, Color) != ST7789H2_OK)
    {
      ret = ST7789H2_ERROR;
      break;
    }
  }

  return ret;
}

/**
  * @brief  Set pixel.
  * @param  pObj Pointer to component object.
  * @param  Xpos X position on LCD.
  * @param  Ypos Y position on LCD.
  * @param  Color Color of the pixel.
  * @retval Component status.
  */
int32_t ST7789H2_SetPixel(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Color)
{
  int32_t ret = ST7789H2_OK;

  /* Set Cursor */
  ret += ST7789H2_SetCursor(pObj, Xpos, Ypos);

  /* write pixel */
  ret += st7789h2_write_reg(&pObj->Ctx, ST7789H2_WRITE_RAM, (uint8_t *) &Color, 1);

  if (ret != ST7789H2_OK)
  {
    ret = ST7789H2_ERROR;
  }

  return ret;
}

/**
  * @brief  Get pixel.
  * @param  pObj Pointer to component object.
  * @param  Xpos X position on LCD.
  * @param  Ypos Y position on LCD.
  * @param  Color Color of the pixel.
  * @retval Component status.
  */
int32_t ST7789H2_GetPixel(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t *Color)
{
  int32_t ret = ST7789H2_OK;
  uint8_t parameter[6] = {0};

  /* Set Cursor */
  ret += ST7789H2_SetCursor(pObj, Xpos, Ypos);

  /* read pixel */
  ret += st7789h2_read_reg(&pObj->Ctx, ST7789H2_READ_RAM, parameter, 3);

  /* Set color in RGB565 format */
  *Color = ((((uint32_t)parameter[3] << 8) & 0xF800U) |
            (((uint32_t)parameter[2] << 3) & 0x07E0U) |
            (((uint32_t)parameter[5] >> 3) & 0x001FU));

  if (ret != ST7789H2_OK)
  {
    ret = ST7789H2_ERROR;
  }

  return ret;
}

/**
  * @brief  Get X size.
  * @param  pObj Pointer to component object.
  * @param  Xsize X size of LCD.
  * @retval Component status.
  */
int32_t ST7789H2_GetXSize(const ST7789H2_Object_t *pObj, uint32_t *XSize)
{
  (void)pObj;

  *XSize = 240;

  return ST7789H2_OK;
}

/**
  * @brief  Get Y size.
  * @param  pObj Pointer to component object.
  * @param  Ysize Y size of LCD.
  * @retval Component status.
  */
int32_t ST7789H2_GetYSize(const ST7789H2_Object_t *pObj, uint32_t *YSize)
{
  (void)pObj;

  *YSize = 240;

  return ST7789H2_OK;
}
/**
  * @}
  */

/** @defgroup ST7789H2_Private_Functions ST7789H2 Private Functions
  * @{
  */
/**
  * @brief  Read register wrapped function.
  * @param  handle  Component object handle.
  * @param  Reg     The target register address to read.
  * @param  pData   The target register value to be red.
  * @param  Length  Buffer size to be red.
  * @retval error status.
  */
static int32_t ST7789H2_ReadRegWrap(const void *handle, uint16_t Reg, uint8_t *pData, uint32_t Length)
{
  const ST7789H2_Object_t *pObj = (const ST7789H2_Object_t *)handle;

  return pObj->IO.ReadReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  Write register wrapped function.
  * @param  handle Component object handle.
  * @param  Reg    The target register address to write.
  * @param  pData  The target register value to be written.
  * @param  Length Buffer size to be written.
  * @retval error status.
  */
static int32_t ST7789H2_WriteRegWrap(const void *handle, uint16_t Reg, uint8_t *pData, uint32_t Length)
{
  const ST7789H2_Object_t *pObj = (const ST7789H2_Object_t *)handle;

  return pObj->IO.WriteReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  Send data wrapped function.
  * @param  handle Component object handle.
  * @param  pData  The value to be written.
  * @param  Length Buffer size to be written.
  * @retval error status.
  */
static int32_t ST7789H2_SendDataWrap(const void *handle, uint8_t *pData, uint32_t Length)
{
  const ST7789H2_Object_t *pObj = (const ST7789H2_Object_t *)handle;

  return pObj->IO.SendData(pData, Length);
}

/**
  * @brief  ST7789H2 delay
  * @param  Delay Delay in ms
  * @retval Component error status
  */
static void ST7789H2_Delay(const ST7789H2_Object_t *pObj, uint32_t Delay)
{
  uint32_t tickstart;
  tickstart = pObj->IO.GetTick();
  while ((pObj->IO.GetTick() - tickstart) < Delay)
  {
  }
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
