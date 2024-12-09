/**
  ******************************************************************************
  * @file    st7789h2.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the st7789h2.c
  *          driver.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ST7789H2_H
#define ST7789H2_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "st7789h2_reg.h"
#include <stddef.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup ST7789H2
  * @{
  */

/** @defgroup ST7789H2_Exported_Types ST7789H2 Exported Types
  * @{
  */
typedef int32_t (*ST7789H2_Init_Func)(void);
typedef int32_t (*ST7789H2_DeInit_Func)(void);
typedef int32_t (*ST7789H2_GetTick_Func)(void);
typedef int32_t (*ST7789H2_WriteReg_Func)(uint16_t, uint16_t, uint8_t *, uint32_t);
typedef int32_t (*ST7789H2_ReadReg_Func)(uint16_t, uint16_t, uint8_t *, uint32_t);
typedef int32_t (*ST7789H2_SendData_Func)(uint8_t *, uint32_t);

typedef struct
{
  ST7789H2_Init_Func          Init;
  ST7789H2_DeInit_Func        DeInit;
  uint16_t                    Address;
  ST7789H2_WriteReg_Func      WriteReg;
  ST7789H2_ReadReg_Func       ReadReg;
  ST7789H2_SendData_Func      SendData;
  ST7789H2_GetTick_Func       GetTick;
} ST7789H2_IO_t;

typedef struct
{
  ST7789H2_IO_t         IO;
  ST7789H2_ctx_t        Ctx;
  uint8_t               IsInitialized;
  uint32_t              Orientation;
} ST7789H2_Object_t;

typedef struct
{
  /* Control functions */
  int32_t (*Init)(ST7789H2_Object_t *, uint32_t, uint32_t);
  int32_t (*DeInit)(ST7789H2_Object_t *);
  int32_t (*ReadID)(ST7789H2_Object_t *, uint32_t *);
  int32_t (*DisplayOn)(ST7789H2_Object_t *);
  int32_t (*DisplayOff)(ST7789H2_Object_t *);
  int32_t (*SetBrightness)(const ST7789H2_Object_t *, uint32_t);
  int32_t (*GetBrightness)(const ST7789H2_Object_t *, const uint32_t *);
  int32_t (*SetOrientation)(ST7789H2_Object_t *, uint32_t);
  int32_t (*GetOrientation)(ST7789H2_Object_t *, uint32_t *);

  /* Drawing functions*/
  int32_t (*SetCursor)(ST7789H2_Object_t *, uint32_t, uint32_t);
  int32_t (*DrawBitmap)(ST7789H2_Object_t *, uint32_t, uint32_t, uint8_t *);
  int32_t (*FillRGBRect)(ST7789H2_Object_t *, uint32_t, uint32_t, uint8_t *, uint32_t, uint32_t);
  int32_t (*DrawHLine)(ST7789H2_Object_t *, uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t (*DrawVLine)(ST7789H2_Object_t *, uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t (*FillRect)(ST7789H2_Object_t *, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t (*GetPixel)(ST7789H2_Object_t *, uint32_t, uint32_t, uint32_t *);
  int32_t (*SetPixel)(ST7789H2_Object_t *, uint32_t, uint32_t, uint32_t);
  int32_t (*GetXSize)(const ST7789H2_Object_t *, uint32_t *);
  int32_t (*GetYSize)(const ST7789H2_Object_t *, uint32_t *);

} ST7789H2_Drv_t;
/**
  * @}
  */

/** @defgroup ST7789H2_Exported_Constants ST7789H2 Exported Constants
  * @{
  */
#define ST7789H2_OK      (0)
#define ST7789H2_ERROR   (-1)

#define ST7789H2_ID      0x85U

#define ST7789H2_ORIENTATION_PORTRAIT         0x00U /* Portrait orientation choice of LCD screen  */
#define ST7789H2_ORIENTATION_LANDSCAPE        0x01U /* Landscape orientation choice of LCD screen */
#define ST7789H2_ORIENTATION_PORTRAIT_ROT180  0x02U /* Portrait with 180 degrees rotation orientation choice of LCD screen */
#define ST7789H2_ORIENTATION_LANDSCAPE_ROT180 0x03U /* Landscape with 180 degrees rotation orientation choice of LCD screen */

#define ST7789H2_FORMAT_RBG444                0x03U /* Pixel format chosen is RGB444 : 12 bpp (currently not supported)  */
#define ST7789H2_FORMAT_RBG565                0x05U /* Pixel format chosen is RGB565 : 16 bpp */
#define ST7789H2_FORMAT_RBG666                0x06U /* Pixel format chosen is RGB666 : 18 bpp (currently not supported)  */
/**
  * @}
  */

/** @defgroup ST7789H2_Exported_Variables ST7789H2 Exported Variables
  * @{
  */
extern ST7789H2_Drv_t ST7789H2_Driver;
/**
  * @}
  */

/** @defgroup ST7789H2_Exported_Functions ST7789H2 Exported Functions
  * @{
  */
int32_t ST7789H2_RegisterBusIO(ST7789H2_Object_t *pObj, ST7789H2_IO_t *pIO);
int32_t ST7789H2_Init(ST7789H2_Object_t *pObj, uint32_t ColorCoding, uint32_t Orientation);
int32_t ST7789H2_DeInit(ST7789H2_Object_t *pObj);
int32_t ST7789H2_ReadID(ST7789H2_Object_t *pObj, uint32_t *Id);
int32_t ST7789H2_DisplayOn(ST7789H2_Object_t *pObj);
int32_t ST7789H2_DisplayOff(ST7789H2_Object_t *pObj);
int32_t ST7789H2_SetBrightness(const ST7789H2_Object_t *pObj, uint32_t Brightness);
int32_t ST7789H2_GetBrightness(const ST7789H2_Object_t *pObj, const uint32_t *Brightness);
int32_t ST7789H2_SetOrientation(ST7789H2_Object_t *pObj, uint32_t Orientation);
int32_t ST7789H2_GetOrientation(ST7789H2_Object_t *pObj, uint32_t *Orientation);
int32_t ST7789H2_SetCursor(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos);
int32_t ST7789H2_DrawBitmap(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp);
int32_t ST7789H2_FillRGBRect(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint8_t *pData, uint32_t Width, uint32_t Height);
int32_t ST7789H2_DrawHLine(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
int32_t ST7789H2_DrawVLine(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
int32_t ST7789H2_FillRect(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color);
int32_t ST7789H2_SetPixel(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Color);
int32_t ST7789H2_GetPixel(ST7789H2_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t *Color);
int32_t ST7789H2_GetXSize(const ST7789H2_Object_t *pObj, uint32_t *XSize);
int32_t ST7789H2_GetYSize(const ST7789H2_Object_t *pObj, uint32_t *YSize);
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

#endif /* ST7789H2_H */
