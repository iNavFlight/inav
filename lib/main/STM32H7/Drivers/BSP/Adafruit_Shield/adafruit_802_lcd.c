/**
  ******************************************************************************
  * @file    adafruit_802_lcd.c
  * @author  MCD Application Team
  * @brief   This file includes the driver for Liquid Crystal Display (LCD) module
  *          mounted on the Adafruit 1.8" TFT LCD shield (reference ID 802),
  *          that is used with the STM32 Nucleo board through SPI interface.
  ******************************************************************************
  @verbatim
  How To use this driver:
  --------------------------
   - This driver is used to drive directly the LCD mounted on on the Adafruit 1.8" TFT LCD
     shield (reference ID 802)
     The following IP are implied : SPI

  Driver description:
  ---------------------
   + Initialization steps:
     o Initialize the LCD with required orientation using the ADAFRUIT_802_LCD_Init() function.
       - LCD_ORIENTATION_PORTRAIT
       - LCD_ORIENTATION_PORTRAIT_ROT180
       - LCD_ORIENTATION_LANDSCAPE
       - LCD_ORIENTATION_LANDSCAPE_ROT180

     o Enable the LCD display using the ADAFRUIT_802_LCD_DisplayOn() function.
     o Disable the LCD display using the ADAFRUIT_802_LCD_DisplayOff() function.
     o Write a pixel to the LCD memory using the ADAFRUIT_802_LCD_WritePixel() function.
     o Draw an horizontal line using the ADAFRUIT_802_LCD_DrawHLine() function.
     o Draw a vertical line using the ADAFRUIT_802_LCD_DrawVLine() function.
     o Draw a bitmap image using the ADAFRUIT_802_LCD_DrawBitmap() function.
     o Select the LCD layer to be used using the ADAFRUIT_802_LCD_SelectLayer() function.
     o Set the display brightness using the ADAFRUIT_802_LCD_SetBrightness() function.
     o Get the display brightness using the ADAFRUIT_802_LCD_GetBrightness() function.
     o Read a pixel from the LCD memory using the ADAFRUIT_802_LCD_ReadPixel() function.

   + Display on LCD
     o To draw and fill a basic shapes (dot, line, rectangle, circle, ellipse, .. bitmap)
       on LCD and display text, utility stm32_lcd.c/.h must be called. Once the LCD is initialized,
       user should call UTIL_LCD_SetFuncDriver() API to link board LCD drivers to UTIL LCD drivers.
       The basic lcd services, defined in lcd utility, are ready for use.

  Note:
  --------
    - Regarding the "Instance" parameter, needed for all functions, it is used to select
      an LCD instance. On the "Adafruit 1.8" TFT LCD shield", only one LCD instance is availble. Then, this
      parameter should be 0.
    - The LCD support only 1 layer. Thus ADAFRUIT_802_LCD_SelectLayer function set always
      the Active Layer to 0
    - SetBrightness, GetBrightness and ReadPixel features are not supported

  @endverbatim
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
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "adafruit_802_lcd.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup ADAFRUIT_802
  * @{
  */

/** @addtogroup ADAFRUIT_802_LCD
  * @{
  */


/** @defgroup ADAFRUIT_802_LCD_Private_Variables LCD Private Variables
  * @{
  */
static void                 *Lcd_CompObj = NULL;
static LCD_Drv_t            *Lcd_Drv = NULL;
ADAFRUIT_802_LCD_Ctx_t      Lcd_Ctx[LCD_INSTANCES_NBR];

/**
  * @}
  */

/** @addtogroup ADAFRUIT_802_LCD_Exported_Variables
  * @{
  */
const LCD_UTILS_Drv_t LCD_Driver =
{
  ADAFRUIT_802_LCD_DrawBitmap,
  ADAFRUIT_802_LCD_FillRGBRect,
  ADAFRUIT_802_LCD_DrawHLine,
  ADAFRUIT_802_LCD_DrawVLine,
  ADAFRUIT_802_LCD_FillRect,
  ADAFRUIT_802_LCD_ReadPixel,
  ADAFRUIT_802_LCD_WritePixel,
  ADAFRUIT_802_LCD_GetXSize,
  ADAFRUIT_802_LCD_GetYSize,
  NULL,
  ADAFRUIT_802_LCD_GetPixelFormat
};
/**
  * @}
  */

/** @defgroup ADAFRUIT_802_LCD_Private_FunctionPrototypes LCD Private Function Prototypes
  * @{
  */
static int32_t ST7735_Probe(uint32_t Orientation);
static int32_t LCD_IO_Init(void);
static int32_t LCD_IO_DeInit(void);
static int32_t LCD_IO_WriteReg(uint8_t Reg, uint8_t* pData, uint32_t Length);
static int32_t LCD_IO_SendData(uint8_t *pData, uint32_t Length);
/**
  * @}
  */

/** @addtogroup ADAFRUIT_802_LCD_Exported_Functions
  * @{
  */

/**
  * @brief  Initializes the LCD with a given orientation.
  * @param  Instance    LCD Instance
  * @param  orientation Select display orientation:
  *         - LCD_ORIENTATION_PORTRAIT
  *         - LCD_ORIENTATION_LANDSCAPE
  *         - LCD_ORIENTATION_PORTRAIT_ROT180
  *         - LCD_ORIENTATION_LANDSCAPE_ROT180
  * @retval Error status
  */
int32_t ADAFRUIT_802_LCD_Init(uint32_t Instance, uint32_t Orientation)
{
  int32_t ret;

  if((Orientation > LCD_ORIENTATION_LANDSCAPE_ROT180) || (Instance >= LCD_INSTANCES_NBR))
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(ST7735_Probe(Orientation) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_UNKNOWN_COMPONENT;
    }
    else if(ADAFRUIT_802_LCD_GetXSize(Instance, &Lcd_Ctx[Instance].XSize) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else if(ADAFRUIT_802_LCD_GetYSize(Instance, &Lcd_Ctx[Instance].YSize) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
    else
    {
      /* Clear the LCD screen with white color */
      ret = ADAFRUIT_802_LCD_FillRect(Instance, 0U, 0U, Lcd_Ctx[Instance].XSize, Lcd_Ctx[Instance].YSize, 0xFFFFFFFFU);
    }
  }
  return ret;
}

/**
  * @brief  DeInitializes the LCD.
  * @param  Instance    LCD Instance
  * @retval Error status
  */
int32_t ADAFRUIT_802_LCD_DeInit(uint32_t Instance)
{
  int32_t ret;

  if(Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
   ret = LCD_IO_DeInit();
  }

  return ret;
}

/**
  * @brief  Gets the LCD Active LCD Pixel Format.
  * @param  Instance    LCD Instance
  * @param  PixelFormat Active LCD Pixel Format
  * @retval BSP status
  */
int32_t ADAFRUIT_802_LCD_GetPixelFormat(uint32_t Instance, uint32_t *PixelFormat)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    /* Only RGB565 format is supported */
    *PixelFormat = LCD_PIXEL_FORMAT_RGB565;
  }

  return ret;
}

/**
  * @brief  Gets the LCD X size.
  * @param  Instance LCD Instance
  * @param  XSize    LCD width
  * @retval Error status
  */
int32_t ADAFRUIT_802_LCD_GetXSize(uint32_t Instance, uint32_t *XSize)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(Lcd_Drv->GetXSize(Lcd_CompObj, XSize) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Gets the LCD Y size.
  * @param  Instance LCD Instance
  * @param  YSize    LCD hieght
  * @retval Error status
  */
int32_t ADAFRUIT_802_LCD_GetYSize(uint32_t Instance, uint32_t *YSize)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else
  {
    if(Lcd_Drv->GetYSize(Lcd_CompObj, YSize) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }

  return ret;
}

/**
  * @brief  Reads an LCD pixel.
  * @param  Instance    LCD Instance
  * @param  Xpos X position
  * @param  Ypos Y position
  * @param  Color RGB color
  * @retval Error status
  */
int32_t ADAFRUIT_802_LCD_ReadPixel(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t *Color)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(Instance);
  UNUSED(Xpos);
  UNUSED(Ypos);
  UNUSED(Color);
  return BSP_ERROR_FEATURE_NOT_SUPPORTED;
}

/**
  * @brief  Draws a pixel on LCD.
  * @param  Instance    LCD Instance
  * @param  Xpos X position
  * @param  Ypos Y position
  * @param  Color RGB color
  * @retval Error status
  */
int32_t ADAFRUIT_802_LCD_WritePixel(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Color)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Lcd_Drv->SetPixel != NULL)
  {
    if(Lcd_Drv->SetPixel(Lcd_CompObj, Xpos, Ypos, Color) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  return ret;
}

/**
  * @brief  Draws an horizontal line.
  * @param  Instance    LCD Instance
  * @param  Xpos X position
  * @param  Ypos Y position
  * @param  Length Line length
  * @param  Color RGB color
  * @retval Error status
  */
int32_t ADAFRUIT_802_LCD_DrawHLine(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Lcd_Drv->DrawHLine != NULL)
  {
    if(Lcd_Drv->DrawHLine(Lcd_CompObj, Xpos, Ypos, Length, Color) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  return ret;
}

/**
  * @brief  Draws a vertical line.
  * @param  Instance    LCD Instance
  * @param  Xpos X position
  * @param  Ypos Y position
  * @param  Length Line length
  * @param  Color RGB color
  * @retval Error status
  */
int32_t ADAFRUIT_802_LCD_DrawVLine(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Lcd_Drv->DrawVLine != NULL)
  {
    if(Lcd_Drv->DrawVLine(Lcd_CompObj, Xpos, Ypos, Length, Color) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  return ret;
}

/**
  * @brief  Draws a bitmap picture (16 bpp).
  * @param  Instance    LCD Instance
  * @param  Xpos Bmp X position in the LCD
  * @param  Ypos Bmp Y position in the LCD
  * @param  pBmp Pointer to Bmp picture address.
  * @retval Error status
  */
int32_t ADAFRUIT_802_LCD_DrawBitmap(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Lcd_Drv->DrawBitmap != NULL)
  {
    if(Lcd_Drv->DrawBitmap(Lcd_CompObj, Xpos, Ypos, pBmp) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  return ret;
}

/**
  * @brief  Draws a full RGB rectangle
  * @param  Instance LCD Instance.
  * @param  Xpos   specifies the X position.
  * @param  Ypos   specifies the Y position.
  * @param  pData  pointer to RGB data
  * @param  Width  specifies the rectangle width.
  * @param  Height Specifies the rectangle height
  * @retval BSP status
  */
int32_t ADAFRUIT_802_LCD_FillRGBRect(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint8_t *pData, uint32_t Width, uint32_t Height)
{
  int32_t ret = BSP_ERROR_NONE;

  if (Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Lcd_Drv->FillRGBRect != NULL)
  {
    /* Draw the horizontal line on LCD */
    if (Lcd_Drv->FillRGBRect(Lcd_CompObj, Xpos, Ypos, pData, Width, Height) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  return ret;
}

/**
  * @brief  Draws a full rectangle.
  * @param  Instance LCD instance
  * @param  Xpos X position
  * @param  Ypos Y position
  * @param  Width Rectangle width
  * @param  Height Rectangle height
  * @param  Color RGB color
  * @retval BSP status
  */
int32_t ADAFRUIT_802_LCD_FillRect(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Lcd_Drv->FillRect != NULL)
  {
    if(Lcd_Drv->FillRect(Lcd_CompObj, Xpos, Ypos, Width, Height, Color) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  return ret;
}

/**
  * @brief  Enables the display.
  * @param  Instance    LCD Instance
  * @retval Error status
  */
int32_t ADAFRUIT_802_LCD_DisplayOn(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Lcd_Drv->DisplayOn != NULL)
  {
    if(Lcd_Drv->DisplayOn(Lcd_CompObj) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  return ret;
}

/**
  * @brief  Disables the display.
  * @param  Instance    LCD Instance
  * @retval Error status
  */
int32_t ADAFRUIT_802_LCD_DisplayOff(uint32_t Instance)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Lcd_Drv->DisplayOff != NULL)
  {
    if(Lcd_Drv->DisplayOff(Lcd_CompObj) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  return ret;
}


/**
  * @brief  Set the brightness value
  * @param  Instance    LCD Instance
  * @param  Brightness [00: Min (black), 100 Max]
  * @retval Error status
  */
int32_t ADAFRUIT_802_LCD_SetBrightness(uint32_t Instance, uint32_t Brightness)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Lcd_Drv->SetBrightness != NULL)
  {
    if(Lcd_Drv->SetBrightness(Lcd_CompObj, Brightness) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  return ret;
}

/**
  * @brief  Get the brightness value
  * @param  Instance    LCD Instance
  * @param  Brightness [00: Min (black), 100 Max]
  * @retval Error status
  */
int32_t ADAFRUIT_802_LCD_GetBrightness(uint32_t Instance, uint32_t *Brightness)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Instance >= LCD_INSTANCES_NBR)
  {
    ret = BSP_ERROR_WRONG_PARAM;
  }
  else if(Lcd_Drv->GetBrightness != NULL)
  {
    if(Lcd_Drv->GetBrightness(Lcd_CompObj, Brightness) < 0)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  else
  {
    ret = BSP_ERROR_FEATURE_NOT_SUPPORTED;
  }

  return ret;
}

/**
  * @}
  */

/** @defgroup ADAFRUIT_802_LCD_Private_Functions LCD Private Functions
  * @{
  */

/**
  * @brief  Register Bus IOs if component ID is OK
  * @param  Orientation Display orientation
  * @retval Error status
  */
static int32_t ST7735_Probe(uint32_t Orientation)
{
  int32_t ret = BSP_ERROR_NONE;
  ST7735_IO_t              IOCtx;
  static ST7735_Object_t   ST7735Obj;

  /* Configure the audio driver */
  IOCtx.Address     = 0;
  IOCtx.Init        = LCD_IO_Init;
  IOCtx.DeInit      = LCD_IO_DeInit;
  IOCtx.GetTick     = BSP_GetTick;
  IOCtx.WriteReg    = LCD_IO_WriteReg;
  IOCtx.SendData    = LCD_IO_SendData;

  if(ST7735_RegisterBusIO(&ST7735Obj, &IOCtx) != ST7735_OK)
  {
    ret = BSP_ERROR_BUS_FAILURE;
  }
  else
  {
    Lcd_CompObj = &ST7735Obj;

    Lcd_Drv = (LCD_Drv_t *) &ST7735_LCD_Driver;
    if(Lcd_Drv->Init(Lcd_CompObj, ST7735_FORMAT_DEFAULT, Orientation) != ST7735_OK)
    {
      ret = BSP_ERROR_COMPONENT_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Initializes the LCD
  * @retval None
  */
static int32_t LCD_IO_Init(void)
{
  int32_t ret = BSP_ERROR_NONE;

  GPIO_InitTypeDef  GPIO_InitStruct;

  /* LCD_CS_GPIO and LCD_DC_GPIO Periph clock enable */
  ADAFRUIT_802_LCD_CS_GPIO_CLK_ENABLE();
  ADAFRUIT_802_LCD_DC_GPIO_CLK_ENABLE();

  /* Configure LCD_CS_PIN pin: LCD Card CS pin */
  GPIO_InitStruct.Pin   = ADAFRUIT_802_LCD_CS_PIN;
  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull  = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(ADAFRUIT_802_LCD_CS_GPIO_PORT, &GPIO_InitStruct);

  /* Configure LCD_DC_PIN pin: LCD Card DC pin */
  GPIO_InitStruct.Pin = ADAFRUIT_802_LCD_DC_PIN;
  HAL_GPIO_Init(ADAFRUIT_802_LCD_DC_GPIO_PORT, &GPIO_InitStruct);

  /* LCD chip select high */
  ADAFRUIT_802_LCD_CS_HIGH();

  /* LCD SPI Config */
  if(BUS_SPIx_Init() != BSP_ERROR_NONE)
  {
    ret = BSP_ERROR_NO_INIT;
  }

  return ret;
}

/**
  * @brief  De-Initializes the LCD
  * @retval None
  */
static int32_t LCD_IO_DeInit(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct;

  /* LCD chip de-select high */
  ADAFRUIT_802_LCD_CS_LOW();

  /* Configure LCD_CS_PIN pin: LCD Card CS pin */
  GPIO_InitStruct.Pin = ADAFRUIT_802_LCD_CS_PIN;
  HAL_GPIO_DeInit(ADAFRUIT_802_LCD_CS_GPIO_PORT, GPIO_InitStruct.Pin);

  /* Configure LCD_DC_PIN pin: LCD Card DC pin */
  GPIO_InitStruct.Pin = ADAFRUIT_802_LCD_DC_PIN;
  HAL_GPIO_DeInit(ADAFRUIT_802_LCD_DC_GPIO_PORT, GPIO_InitStruct.Pin);

  /* LCD_CS_GPIO and LCD_DC_GPIO Periph clock disable */
  ADAFRUIT_802_LCD_CS_GPIO_CLK_DISABLE();
  ADAFRUIT_802_LCD_DC_GPIO_CLK_DISABLE();

  return BSP_ERROR_NONE;
}

/**
  * @brief  Writes command to select the LCD register.
  * @param  Reg Address of the selected register.
  * @param  pData pointer to data to write to the register
  * @param  Length length of data to write to the register
  * @retval Error status
  */
static int32_t LCD_IO_WriteReg(uint8_t Reg, uint8_t* pData, uint32_t Length)
{
  int32_t ret;

  /* Reset LCD control line CS */
  ADAFRUIT_802_LCD_CS_LOW();

  /* Set LCD data/command line DC to Low */
  ADAFRUIT_802_LCD_DC_LOW();

  /* Send Command */
  if(BUS_SPIx_Send(&Reg, 1) != BSP_ERROR_NONE)
  {
    ret = BSP_ERROR_PERIPH_FAILURE;
  }
  else
  {
    /* Deselect : Chip Select high */
    ADAFRUIT_802_LCD_CS_HIGH();

    /* Send Data */
    ret = LCD_IO_SendData(pData, Length);
  }

  return ret;
}

/**
  * @brief  Send data to select the LCD GRAM.
  * @param  pData pointer to data to write to LCD GRAM.
  * @param  Length length of data to write to LCD GRAM
  * @retval Error status
  */
static int32_t LCD_IO_SendData(uint8_t *pData, uint32_t Length)
{
  int32_t ret = BSP_ERROR_NONE;

  if(Length != 0U)
  {
    /* Reset LCD control line CS */
    ADAFRUIT_802_LCD_CS_LOW();

    /* Set LCD data/command line DC to High */
    ADAFRUIT_802_LCD_DC_HIGH();

    if(BUS_SPIx_Send(pData, Length) != BSP_ERROR_NONE)
    {
      ret = BSP_ERROR_PERIPH_FAILURE;
    }

    /* Deselect : Chip Select high */
    ADAFRUIT_802_LCD_CS_HIGH();
  }

  return ret;
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
