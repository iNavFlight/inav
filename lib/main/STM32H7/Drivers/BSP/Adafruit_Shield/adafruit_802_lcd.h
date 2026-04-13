/**
  ******************************************************************************
  * @file    adafruit_802_lcd.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the adafruit_802_lcd.c driver.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ADAFRUIT_802_LCD_H
#define ADAFRUIT_802_LCD_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "adafruit_802.h"
#include "adafruit_802_conf.h"
#include "lcd.h"
#include "../Components/st7735/st7735.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup ADAFRUIT_802
  * @{
  */

/** @defgroup ADAFRUIT_802_LCD LCD
  * @{
  */
/** @defgroup ADAFRUIT_802_LCD_Exported_Variables LCD Exported Variables
  * @{
  */
extern const LCD_UTILS_Drv_t LCD_Driver;
/**
  * @}
  */

/** @defgroup ADAFRUIT_802_LCD_Exported_Constants LCD Exported Constants
  * @{
  */
#define LCD_INSTANCES_NBR                  1U

#define LCD_ORIENTATION_PORTRAIT         0x00U /* Portrait orientation choice of LCD screen               */
#define LCD_ORIENTATION_PORTRAIT_ROT180  0x01U /* Portrait rotated 180° orientation choice of LCD screen  */
#define LCD_ORIENTATION_LANDSCAPE        0x02U /* Landscape orientation choice of LCD screen              */
#define LCD_ORIENTATION_LANDSCAPE_ROT180 0x03U /* Landscape rotated 180° orientation choice of LCD screen */

/**
  * @}
  */

/** @defgroup ADAFRUIT_802_LCD_Exported_Types LCD Exported Types
  * @{
  */
typedef struct
{
  uint32_t XSize;
  uint32_t YSize;
  uint32_t PixelFormat;
  uint32_t IsMspCallbacksValid;
} ADAFRUIT_802_LCD_Ctx_t;

/**
  * @}
  */

/** @addtogroup ADAFRUIT_802_LCD_Exported_Variables
  * @{
  */
extern ADAFRUIT_802_LCD_Ctx_t      Lcd_Ctx[LCD_INSTANCES_NBR];

/**
  * @}
  */

/** @defgroup ADAFRUIT_802_LCD_Exported_Functions LCD Exported Functions
  * @{
  */
int32_t ADAFRUIT_802_LCD_Init(uint32_t Instance, uint32_t Orientation);
int32_t ADAFRUIT_802_LCD_DeInit(uint32_t Instance);

/* LCD generic APIs: Display control */
int32_t ADAFRUIT_802_LCD_DisplayOn(uint32_t Instance);
int32_t ADAFRUIT_802_LCD_DisplayOff(uint32_t Instance);
int32_t ADAFRUIT_802_LCD_SetBrightness(uint32_t Instance, uint32_t Brightness);
int32_t ADAFRUIT_802_LCD_GetBrightness(uint32_t Instance, uint32_t *Brightness);
int32_t ADAFRUIT_802_LCD_GetXSize(uint32_t Instance, uint32_t *XSize);
int32_t ADAFRUIT_802_LCD_GetYSize(uint32_t Instance, uint32_t *YSize);

/* LCD generic APIs: Draw operations. This list of APIs is required for
   lcd gfx utilities */
int32_t ADAFRUIT_802_LCD_GetPixelFormat(uint32_t Instance, uint32_t *PixelFormat);
int32_t ADAFRUIT_802_LCD_DrawBitmap(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp);
int32_t ADAFRUIT_802_LCD_FillRGBRect(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint8_t *pData, uint32_t Width, uint32_t Height);
int32_t ADAFRUIT_802_LCD_DrawHLine(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
int32_t ADAFRUIT_802_LCD_DrawVLine(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
int32_t ADAFRUIT_802_LCD_FillRect(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color);
int32_t ADAFRUIT_802_LCD_ReadPixel(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t *Color);
int32_t ADAFRUIT_802_LCD_WritePixel(uint32_t Instance, uint32_t Xpos, uint32_t Ypos, uint32_t Color);

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

#ifdef __cplusplus
}
#endif

#endif /* ADAFRUIT_802_LCD_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
