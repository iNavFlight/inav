/**
  ******************************************************************************
  * @file    nt35510.c
  * @author  MCD Application Team
  * @brief   This file provides the LCD Driver for Frida Techshine 3K138 (WVGA)
  *          DSI LCD Display NT35510.
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
#include "nt35510.h"
#include <stddef.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @defgroup NT35510 NT35510
  * @brief     This file provides a set of functions needed to drive the
  *            NT35510 IC display driver.
  * @{
  */

/** @defgroup NT35510_Exported_Types NT35510 Exported Types
  * @{
  */
NT35510_LCD_Drv_t   NT35510_LCD_Driver = 
{
  NT35510_Init,
  NT35510_DeInit,  
  NT35510_ReadID,
  NT35510_DisplayOn,
  NT35510_DisplayOff,
  NT35510_SetBrightness,
  NT35510_GetBrightness,  
  NT35510_SetOrientation,
  NT35510_GetOrientation,
  NT35510_SetCursor,
  NT35510_DrawBitmap,
  NT35510_FillRGBRect,
  NT35510_DrawHLine,
  NT35510_DrawVLine,
  NT35510_FillRect,
  NT35510_GetPixel,
  NT35510_SetPixel,
  NT35510_GetXSize,
  NT35510_GetYSize,
};
  
static NT35510_LCD_Ctx_t NT35510Ctx;
/**
  * @}
  */

/** @defgroup NT35510_Private_Constants NT35510 Private Constants
  * @{
  */
  static const uint8_t nt35510_madctl_portrait[] = {NT35510_CMD_MADCTL ,0x00};
  static const uint8_t nt35510_caset_portrait[] = {0x00, 0x00, 0x01, 0xDF ,NT35510_CMD_CASET};
  static const uint8_t nt35510_raset_portrait[] = {0x00, 0x00, 0x03, 0x1F ,NT35510_CMD_RASET};
  static const uint8_t nt35510_madctl_landscape[] = {NT35510_CMD_MADCTL, 0x60};
  static const uint8_t nt35510_caset_landscape[] = {0x00, 0x00, 0x03, 0x1F ,NT35510_CMD_CASET};
  static const uint8_t nt35510_raset_landscape[] = {0x00, 0x00, 0x01, 0xDF ,NT35510_CMD_RASET};

/**
  * @}
  */

/* Private macros ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/** @defgroup NT35510_Private_FunctionsPrototyes NT35510 Private_Functions Prototyes
  * @{
  */
static int32_t NT35510_ReadRegWrap(void *Handle, uint16_t Reg, uint8_t* Data, uint16_t Length);
static int32_t NT35510_WriteRegWrap(void *Handle, uint16_t Reg, uint8_t *pData, uint16_t Length);
static int32_t NT35510_IO_Delay(NT35510_Object_t *pObj, uint32_t Delay);
/**
  * @}
  */

/** @defgroup NT35510_Exported_Variables
  * @{
  */

/**
  * @}
  */

/* Exported functions ---------------------------------------------------------*/
/** @defgroup NT35510_Exported_Functions NT35510 Exported Functions
  * @{
  */

/**
  * @brief  Register component IO bus
  * @param  Component object pointer
  * @retval Component status
  */
int32_t NT35510_RegisterBusIO (NT35510_Object_t *pObj, NT35510_IO_t *pIO)
{
  int32_t ret = NT35510_OK;

  if(pObj == NULL)
  {
    ret = NT35510_ERROR;
  }
  else
  {
    pObj->IO.WriteReg  = pIO->WriteReg;
    pObj->IO.ReadReg   = pIO->ReadReg;
    pObj->IO.GetTick   = pIO->GetTick;
    
    pObj->Ctx.ReadReg  = NT35510_ReadRegWrap;
    pObj->Ctx.WriteReg = NT35510_WriteRegWrap;
    pObj->Ctx.handle   = pObj;
  }
  
  return ret;
}

/**
  * @brief  Initializes the LCD KoD display part by communication in DSI mode in Video Mode
  *         with IC Display Driver NT35510 (see IC Driver specification for more information).
  * @param  pObj Component object
  * @param  ColorCoding   Color Code
  * @param  Orientation   Display orientation
  * @retval Component status
  */
int32_t NT35510_Init(NT35510_Object_t *pObj, uint32_t ColorCoding, uint32_t Orientation)
{
/* ************************************************************************** */
/* Proprietary Initialization                                                 */
/* ************************************************************************** */
  int32_t ret;
  static const uint8_t nt35510_reg[]   = {0x55, 0xAA, 0x52, 0x08, 0x01};
  static const uint8_t nt35510_reg1[]  = {0x03, 0x03, 0x03};
  static const uint8_t nt35510_reg2[]  = {0x46, 0x46, 0x46};
  static const uint8_t nt35510_reg3[]  = {0x03, 0x03, 0x03};
  static const uint8_t nt35510_reg4[]  = {0x36, 0x36, 0x36};
  static const uint8_t nt35510_reg5[]  = {0x00, 0x00, 0x02};
  static const uint8_t nt35510_reg6[]  = {0x26, 0x26, 0x26};
  static const uint8_t nt35510_reg7[]  = {0x01,0x01};
  static const uint8_t nt35510_reg8[]  = {0x09, 0x09, 0x09};
  static const uint8_t nt35510_reg9[]  = {0x36, 0x36, 0x36};
  static const uint8_t nt35510_reg10[] = {0x08, 0x08, 0x08};
  static const uint8_t nt35510_reg12[] = {0x26, 0x26, 0x26};
  static const uint8_t nt35510_reg13[] = {0x00, 0x80, 0x00};
  static const uint8_t nt35510_reg14[] = {0x00, 0x80, 0x00};
  static const uint8_t nt35510_reg15[] = {0x00, 0x50};
  static const uint8_t nt35510_reg16[] = {0x55, 0xAA, 0x52, 0x08, 0x00};
  static const uint8_t nt35510_reg17[] = {0xFC, 0x00};
  static const uint8_t nt35510_reg18[] = {0x03, 0x03};
  static const uint8_t nt35510_reg19[] = {0x50, 0x50};
  static const uint8_t nt35510_reg20[] = {0x00, 0x00};

  static const uint8_t nt35510_reg21[] = {0x01, 0x02, 0x02, 0x02};
  static const uint8_t nt35510_reg22[] = {0x00, 0x00, 0x00};
  static const uint8_t nt35510_reg23[] = {0x03, 0x00, 0x00};
  static const uint8_t nt35510_reg24[] = {0x01, 0x01};

  static const uint8_t nt35510_reg27[] = {NT35510_CMD_SLPOUT, 0x00}; /* Sleep out */
  static const uint8_t nt35510_reg30[] = {NT35510_CMD_DISPON, 0x00};

  static const uint8_t nt35510_reg31[] = {NT35510_CMD_WRDISBV, 0x7F};
  static const uint8_t nt35510_reg32[] = {NT35510_CMD_WRCTRLD, 0x2C};
  static const uint8_t nt35510_reg33[] = {NT35510_CMD_WRCABC, 0x02};
  static const uint8_t nt35510_reg34[] = {NT35510_CMD_WRCABCMB, 0xFF};
  static const uint8_t nt35510_reg35[] = {NT35510_CMD_RAMWR, 0x00};
  static const uint8_t nt35510_reg36[] = {NT35510_CMD_COLMOD, NT35510_COLMOD_RGB565};
  static const uint8_t nt35510_reg37[] = {NT35510_CMD_COLMOD, NT35510_COLMOD_RGB888};

  ret =  nt35510_write_reg(&pObj->Ctx, 0xF0, nt35510_reg, 5);/* LV2:  Page 1 enable */
  ret += nt35510_write_reg(&pObj->Ctx, 0xB0, nt35510_reg1, 3);/* AVDD: 5.2V */
  ret += nt35510_write_reg(&pObj->Ctx, 0xB6, nt35510_reg2, 3); /* AVDD: Ratio */
  ret += nt35510_write_reg(&pObj->Ctx, 0xB1, nt35510_reg3, 3);/* AVEE: -5.2V */
  ret += nt35510_write_reg(&pObj->Ctx, 0xB7, nt35510_reg4, 3);/* AVEE: Ratio */
  ret += nt35510_write_reg(&pObj->Ctx, 0xB2, nt35510_reg5, 3);/* VCL: -2.5V */
  ret += nt35510_write_reg(&pObj->Ctx, 0xB8, nt35510_reg6, 3);/* VCL: Ratio */
  ret += nt35510_write_reg(&pObj->Ctx, 0xBF, nt35510_reg7, 1);/* VGH: 15V (Free Pump) */
  ret += nt35510_write_reg(&pObj->Ctx, 0xB3, nt35510_reg8, 3);
  ret += nt35510_write_reg(&pObj->Ctx, 0xB9, nt35510_reg9, 3);/* VGH: Ratio */ 
  ret += nt35510_write_reg(&pObj->Ctx, 0xB5, nt35510_reg10, 3);/* VGL_REG: -10V */
  ret += nt35510_write_reg(&pObj->Ctx, 0xBA, nt35510_reg12, 3);/* VGLX: Ratio */
  ret += nt35510_write_reg(&pObj->Ctx, 0xBC, nt35510_reg13, 3);/* VGMP/VGSP: 4.5V/0V */
  ret += nt35510_write_reg(&pObj->Ctx, 0xBD, nt35510_reg14, 3);/* VGMN/VGSN:-4.5V/0V */
  ret += nt35510_write_reg(&pObj->Ctx, 0xBE, nt35510_reg15, 2);/* VCOM: -1.325V */

/* ************************************************************************** */
/* Proprietary DCS Initialization                                             */
/* ************************************************************************** */

  ret += nt35510_write_reg(&pObj->Ctx, 0xF0, nt35510_reg16, 5);/* LV2: Page 0 enable */
  ret += nt35510_write_reg(&pObj->Ctx, 0xB1, nt35510_reg17, 2);/* Display optional control */
  ret += nt35510_write_reg(&pObj->Ctx, 0xB6, nt35510_reg18, 1);/* Set source output data hold time */
  ret += nt35510_write_reg(&pObj->Ctx, 0xB5, nt35510_reg19, 1);/*Display resolution control */
  ret += nt35510_write_reg(&pObj->Ctx, 0xB7, nt35510_reg20, 2);/* Gate EQ control */
  ret += nt35510_write_reg(&pObj->Ctx, 0xB8, nt35510_reg21, 4);/* Src EQ control(Mode2) */
  ret += nt35510_write_reg(&pObj->Ctx, 0xBC, nt35510_reg22, 3);
  ret += nt35510_write_reg(&pObj->Ctx, 0xCC, nt35510_reg23, 3);
  ret += nt35510_write_reg(&pObj->Ctx, 0xBA, nt35510_reg24, 1);

  /* Add a delay, otherwise MADCTL not taken */
  (void)NT35510_IO_Delay(pObj, 200);

  /* Configure orientation */
  if(Orientation == NT35510_ORIENTATION_PORTRAIT)
  {
    ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_MADCTL, &nt35510_madctl_portrait[1], 0);
    ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_CASET, nt35510_caset_portrait, 4);
    ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_RASET, nt35510_raset_portrait, 4);
  }
  else
  {
    ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_MADCTL, &nt35510_madctl_landscape[1], 0);
    ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_CASET, nt35510_caset_landscape, 4);
    ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_RASET, nt35510_raset_landscape, 4);
  }

  ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_SLPOUT, &nt35510_reg27[1], 0);

  /* Wait for sleep out exit */
  (void)NT35510_IO_Delay(pObj, 20);

  switch(ColorCoding)
  {
    case NT35510_FORMAT_RBG565 :
      /* Set Pixel color format to RGB565 */
      ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_COLMOD, &nt35510_reg36[1], 0);
	  NT35510Ctx.ColorCode = NT35510_FORMAT_RBG565;
      break;
    case NT35510_FORMAT_RGB888 :
      /* Set Pixel color format to RGB888 */
      ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_COLMOD, &nt35510_reg37[1], 0);
	  NT35510Ctx.ColorCode = NT35510_FORMAT_RGB888;
      break;
    default :
      /* Set Pixel color format to RGB888 */
      ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_COLMOD, &nt35510_reg37[1], 0);
	  NT35510Ctx.ColorCode = NT35510_FORMAT_RGB888;
      break;
  }

  /** CABC : Content Adaptive Backlight Control section start >> */
  /* Note : defaut is 0 (lowest Brightness], 0xFF is highest Brightness, try 0x7F : intermediate value */
  ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_WRDISBV, &nt35510_reg31[1], 0);

  /* defaut is 0, try 0x2C - Brightness Control Block, Display Dimming & BackLight on */
  ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_WRCTRLD, &nt35510_reg32[1], 0);

  /* defaut is 0, try 0x02 - image Content based Adaptive Brightness [Still Picture] */
  ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_WRCABC, &nt35510_reg33[1], 0);

  /* defaut is 0 (lowest Brightness], 0xFF is highest Brightness */
  ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_WRCABCMB, &nt35510_reg34[1], 0);

  /** CABC : Content Adaptive Backlight Control section end << */

  /* Send Command Display On */
  ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_DISPON, &nt35510_reg30[1], 0);

  /* Send Command GRAM memory write (no parameters) : this initiates frame write via other DSI commands sent by */
  /* DSI host from LTDC incoming pixels in video mode */
  ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_RAMWR, &nt35510_reg35[1], 0);

  if(ret != NT35510_OK)
  {
    ret = NT35510_ERROR;
  }

  return ret;
}

/**
  * @brief  De-Initializes the component
  * @param  pObj Component object
  * @retval Component status
  */
int32_t NT35510_DeInit(NT35510_Object_t *pObj)
{
  return NT35510_ERROR;
}

/**
  * @brief  Read the component ID.
  * @param  pObj Component object
  * @param  Id   Component ID
  * @retval Component status
  */
int32_t NT35510_ReadID(NT35510_Object_t *pObj, uint32_t *Id)
{ 
  int32_t ret;

  if(nt35510_read_reg(&pObj->Ctx, NT35510_CMD_RDID2, (uint8_t *)Id, 1)!= NT35510_OK)
  {
    ret = NT35510_ERROR;
  }
  else
  {
    ret = NT35510_OK;
  }

  return ret;  
}  

/**
  * @brief  Set the display brightness.
  * @param  pObj Component object
  * @param  Brightness   display brightness to be set
  * @retval Component status
  */
int32_t NT35510_SetBrightness(NT35510_Object_t *pObj, uint32_t Brightness)
{
  int32_t ret;
  uint8_t brightness = (uint8_t)((Brightness * 255U)/100U);
  
  /* Send Display on DCS command to display */
  if(nt35510_write_reg(&pObj->Ctx, NT35510_CMD_WRDISBV, &brightness, 0) != NT35510_OK)
  {
    ret = NT35510_ERROR;
  }
  else
  {
    NT35510Ctx.Brightness = Brightness;
    ret = NT35510_OK;
  }
  
  return ret;  
}

/**
  * @brief  Get the display brightness.
  * @param  pObj Component object
  * @param  Brightness   display brightness to be returned
  * @retval Component status
  */
int32_t NT35510_GetBrightness(NT35510_Object_t *pObj, uint32_t *Brightness)
{
  *Brightness = NT35510Ctx.Brightness;
  return NT35510_OK;  
}

/**
  * @brief  Set the display On.
  * @param  pObj Component object
  * @retval Component status
  */
int32_t NT35510_DisplayOn(NT35510_Object_t *pObj)
{
  int32_t ret;
  uint8_t display = 0;
  
  /* Send Display on DCS command to display */
  if(nt35510_write_reg(&pObj->Ctx, NT35510_CMD_DISPON, &display, 0) != NT35510_OK)
  {
    ret = NT35510_ERROR;
  }
  else
  {
    ret = NT35510_OK;
  }
  
  return ret;
}

/**
  * @brief  Set the display Off.
  * @param  pObj Component object
  * @retval Component status
  */
int32_t NT35510_DisplayOff(NT35510_Object_t *pObj)
{
  int32_t ret;
  uint8_t display = 0;
  
  /* Send Display on DCS command to display */
  if(nt35510_write_reg(&pObj->Ctx, NT35510_CMD_DISPOFF, &display, 0) != NT35510_OK)
  {
    ret = NT35510_ERROR;
  }
  else
  {
    ret = NT35510_OK;
  }
  
  return ret;
}

/**
  * @brief  Set the display Orientation.
  * @param  pObj Component object
  * @param  Orientation   display Orientation to be set
  * @retval Component status
  */
int32_t NT35510_SetOrientation(NT35510_Object_t *pObj, uint32_t Orientation)
{
  int32_t ret;
  uint8_t tmp = NT35510_MADCTR_MODE_LANDSCAPE;
  uint8_t tmp1 = NT35510_MADCTR_MODE_PORTRAIT;
  
  if((Orientation != NT35510_ORIENTATION_LANDSCAPE) && (Orientation != NT35510_ORIENTATION_PORTRAIT))
  {
    ret = NT35510_ERROR;
  }/* Send command to configure display orientation mode  */
  else if(Orientation == NT35510_ORIENTATION_LANDSCAPE)
  {
    ret = nt35510_write_reg(&pObj->Ctx, NT35510_CMD_RDDMADCTL, &tmp, 0);
    ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_CASET, nt35510_caset_landscape, 4);
    ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_RASET, nt35510_raset_landscape, 4);
    
    NT35510Ctx.Orientation = NT35510_ORIENTATION_LANDSCAPE;
  }
  else
  {
    ret = nt35510_write_reg(&pObj->Ctx, NT35510_CMD_MADCTL, &tmp1, 0);
    ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_CASET, nt35510_caset_portrait, 4);
    ret += nt35510_write_reg(&pObj->Ctx, NT35510_CMD_RASET, nt35510_raset_portrait, 4);
    
    NT35510Ctx.Orientation = NT35510_ORIENTATION_PORTRAIT;
  }
  
  if(ret != NT35510_OK)
  {
    ret = NT35510_ERROR;
  }
  
  return ret;
}

/**
  * @brief  Set the display Orientation.
  * @param  pObj Component object
  * @param  Orientation   display Orientation to be returned
  * @retval Component status
  */
int32_t NT35510_GetOrientation(NT35510_Object_t *pObj, uint32_t *Orientation)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  *Orientation = NT35510Ctx.Orientation;
  
  return NT35510_OK;
}

/**
  * @brief  Set the display Width.
  * @param  pObj Component object
  * @param  Xsize   display Width to be set
  * @retval Component status
  */
int32_t NT35510_GetXSize(NT35510_Object_t *pObj, uint32_t *Xsize)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  switch(NT35510Ctx.Orientation)
  {
  case NT35510_ORIENTATION_PORTRAIT:
    *Xsize = NT35510_480X800_WIDTH;
    break;
  case NT35510_ORIENTATION_LANDSCAPE:
    *Xsize = NT35510_800X480_WIDTH;
    break;
  default:
    *Xsize = NT35510_800X480_WIDTH;
    break;
  }
  
  return NT35510_OK;
}

/**
  * @brief  Set the display Height.
  * @param  pObj Component object
  * @param  Ysize   display Height to be set
  * @retval Component status
  */
int32_t NT35510_GetYSize(NT35510_Object_t *pObj, uint32_t *Ysize)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  switch(NT35510Ctx.Orientation)
  {
  case NT35510_ORIENTATION_PORTRAIT:
    *Ysize = NT35510_480X800_HEIGHT;
    break;
  case NT35510_ORIENTATION_LANDSCAPE:
    *Ysize = NT35510_800X480_HEIGHT;
    break;
  default:
    *Ysize = NT35510_800X480_HEIGHT;
    break;
  }
  
  return NT35510_OK;
}

/**
  * @brief  Set the display cursor.
  * @param  pObj Component object
  * @param  Xpos X position
  * @param  Ypos Y position
  * @retval Component status
  */
int32_t NT35510_SetCursor(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  return NT35510_ERROR;
}

/**
  * @brief  Draw Bitmap image
  * @param  pObj Component object
  * @param  Xpos X position
  * @param  Ypos Y position
  * @param  pBmp pointer to bmp data
  * @retval Component status
  */
int32_t NT35510_DrawBitmap(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  return NT35510_ERROR;
}

int32_t NT35510_FillRGBRect(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint8_t *pData, uint32_t Width, uint32_t Height)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  return NT35510_ERROR;
}

/**
  * @brief  Draw Horizontal Line
  * @param  pObj Component object
  * @param  Xpos X position
  * @param  Ypos Y position
  * @param  Length Line length
  * @param  Color Draw color
  * @retval Component status
  */
int32_t NT35510_DrawHLine(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  return NT35510_ERROR;
}

/**
  * @brief  Draw Vertical line
  * @param  pObj Component object
  * @param  Xpos X position
  * @param  Ypos Y position
  * @param  Length Line length
  * @param  Color Draw color
  * @retval Component status
  */
int32_t NT35510_DrawVLine(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  return NT35510_ERROR;
}

/**
  * @brief  Fill rectangle
  * @param  pObj Component object
  * @param  Xpos X position
  * @param  Ypos Y position
  * @param  Width Rectangle width
  * @param  Height Rectangle height
  * @param  Color Draw color
  * @retval Component status
  */
int32_t NT35510_FillRect(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  return NT35510_ERROR;
}

/**
  * @brief  Get pixel color
  * @param  pObj Component object
  * @param  Xpos X position
  * @param  Ypos Y position
  * @param  Color Color to be returned
  * @retval Component status
  */
int32_t NT35510_GetPixel(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t *Color)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  return NT35510_ERROR;
}

/**
  * @brief  Set pixel color
  * @param  pObj Component object
  * @param  Xpos X position
  * @param  Ypos Y position
  * @param  Color Color to be returned
  * @retval Component status
  */
int32_t NT35510_SetPixel(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Color)
{
  /* Prevent unused argument(s) compilation warning */  
  (void)(pObj);
  
  return NT35510_ERROR;
}

/**
  * @}
  */

/** @addtogroup NT35510_Private_FunctionsPrototyes
  * @{
  */

/**
  * @brief  Wrap component ReadReg to Bus Read function
  * @param  Handle  Component object handle
  * @param  Reg  The target register address to write
  * @param  pData  The target register value to be written
  * @param  Length  buffer size to be written
  * @retval error status
  */
static int32_t NT35510_ReadRegWrap(void *Handle, uint16_t Reg, uint8_t* pData, uint16_t Length)
{
  NT35510_Object_t *pObj = (NT35510_Object_t *)Handle;

  return pObj->IO.ReadReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  Wrap component WriteReg to Bus Write function
  * @param  handle  Component object handle
  * @param  Reg  The target register address to write
  * @param  pData  The target register value to be written
  * @param  Length  buffer size to be written
  * @retval error status
  */
static int32_t NT35510_WriteRegWrap(void *Handle, uint16_t Reg, uint8_t *pData, uint16_t Length)
{
  NT35510_Object_t *pObj = (NT35510_Object_t *)Handle;

  return pObj->IO.WriteReg(pObj->IO.Address, Reg, pData, Length);
}

/**
  * @brief  NT35510 delay
  * @param  Delay  Delay in ms
  */
static int32_t NT35510_IO_Delay(NT35510_Object_t *pObj, uint32_t Delay)
{
  uint32_t tickstart;
  tickstart = pObj->IO.GetTick();
  while((pObj->IO.GetTick() - tickstart) < Delay)
  {
  }
  return NT35510_OK;
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
