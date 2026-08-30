/**
  ******************************************************************************
  * @file    nt35510.h
  * @author  MCD Application Team
  * @brief   This file contains all the constants parameters for the NT35510
  *          which is the LCD Driver for Frida Techshine 3K138 (WVGA)
  *          DSI LCD Display.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NT35510_H
#define __NT35510_H

#ifdef __cplusplus
 extern "C" {
#endif

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

/** @addtogroup NT35510_Exported_Variables
  * @{
  */

typedef int32_t (*NT35510_GetTick_Func) (void);
typedef int32_t (*NT35510_Delay_Func)   (uint32_t);
typedef int32_t (*NT35510_WriteReg_Func)(uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*NT35510_ReadReg_Func) (uint16_t, uint16_t, uint8_t*, uint16_t);

/** @addtogroup NT35510_Exported_Types
  * @{
  */

typedef struct
{ 
  uint32_t  Orientation;
  uint32_t  ColorCode;
  uint32_t  Brightness;
} NT35510_LCD_Ctx_t;

typedef struct
{
  uint16_t                  Address;  
  NT35510_WriteReg_Func    WriteReg;
  NT35510_ReadReg_Func     ReadReg;  
  NT35510_GetTick_Func     GetTick; 
} NT35510_IO_t;

typedef struct
{
  NT35510_IO_t         IO;
  nt35510_ctx_t        Ctx; 
  uint8_t               IsInitialized;
} NT35510_Object_t;

typedef struct
{
  /* Control functions */
  int32_t (*Init             )(NT35510_Object_t*, uint32_t, uint32_t);
  int32_t (*DeInit           )(NT35510_Object_t*);
  int32_t (*ReadID           )(NT35510_Object_t*, uint32_t*);
  int32_t (*DisplayOn        )(NT35510_Object_t*);
  int32_t (*DisplayOff       )(NT35510_Object_t*);
  int32_t (*SetBrightness    )(NT35510_Object_t*, uint32_t);
  int32_t (*GetBrightness    )(NT35510_Object_t*, uint32_t*);
  int32_t (*SetOrientation   )(NT35510_Object_t*, uint32_t);
  int32_t (*GetOrientation   )(NT35510_Object_t*, uint32_t*);

  /* Drawing functions*/
  int32_t (*SetCursor        )(NT35510_Object_t*, uint32_t, uint32_t);
  int32_t (*DrawBitmap       )(NT35510_Object_t*, uint32_t, uint32_t, uint8_t*);
  int32_t (*FillRGBRect      )(NT35510_Object_t*, uint32_t, uint32_t, uint8_t*, uint32_t, uint32_t);
  int32_t (*DrawHLine        )(NT35510_Object_t*, uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t (*DrawVLine        )(NT35510_Object_t*, uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t (*FillRect         )(NT35510_Object_t*, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t);
  int32_t (*GetPixel         )(NT35510_Object_t*, uint32_t, uint32_t, uint32_t*);
  int32_t (*SetPixel         )(NT35510_Object_t*, uint32_t, uint32_t, uint32_t);
  int32_t (*GetXSize         )(NT35510_Object_t*, uint32_t *);
  int32_t (*GetYSize         )(NT35510_Object_t*, uint32_t *);

}NT35510_LCD_Drv_t;

/**
  * @}
  */

#define NT35510_OK                (0)
#define NT35510_ERROR             (-1)

/* NT35510 ID */
#define NT35510_ID                 0x80U

/**
 *  @brief LCD_OrientationTypeDef
 *  Possible values of Display Orientation
 */
#define NT35510_ORIENTATION_PORTRAIT    ((uint32_t)0x00) /* Portrait orientation choice of LCD screen  */
#define NT35510_ORIENTATION_LANDSCAPE   ((uint32_t)0x01) /* Landscape orientation choice of LCD screen */

/**
 *  @brief  Possible values of
 *  pixel data format (ie color coding) transmitted on DSI Data lane in DSI packets
 */
#define NT35510_FORMAT_RGB888    ((uint32_t)0x00) /* Pixel format chosen is RGB888 : 24 bpp */
#define NT35510_FORMAT_RBG565    ((uint32_t)0x02) /* Pixel format chosen is RGB565 : 16 bpp */

/**
  * @brief  nt35510_480x800 Size
  */

/* Width and Height in Portrait mode */
#define  NT35510_480X800_WIDTH             ((uint16_t)480)     /* LCD PIXEL WIDTH   */
#define  NT35510_480X800_HEIGHT            ((uint16_t)800)     /* LCD PIXEL HEIGHT  */

/* Width and Height in Landscape mode */
#define  NT35510_800X480_WIDTH             ((uint16_t)800)     /* LCD PIXEL WIDTH   */
#define  NT35510_800X480_HEIGHT            ((uint16_t)480)     /* LCD PIXEL HEIGHT  */

/**
  * @brief  NT35510_480X800 Timing parameters for Portrait orientation mode
  */
#define  NT35510_480X800_HSYNC             ((uint16_t)2)      /* Horizontal synchronization */
#define  NT35510_480X800_HBP               ((uint16_t)34)     /* Horizontal back porch      */
#define  NT35510_480X800_HFP               ((uint16_t)34)     /* Horizontal front porch     */

#define  NT35510_480X800_VSYNC             ((uint16_t)120)      /* Vertical synchronization   */
#define  NT35510_480X800_VBP               ((uint16_t)150)     /* Vertical back porch        */
#define  NT35510_480X800_VFP               ((uint16_t)150)     /* Vertical front porch       */

/**
  * @brief  NT35510_800X480 Timing parameters for Landscape orientation mode
  *         Same values as for Portrait mode in fact.
  */
#define  NT35510_800X480_HSYNC             NT35510_480X800_VSYNC  /* Horizontal synchronization */
#define  NT35510_800X480_HBP               NT35510_480X800_VBP    /* Horizontal back porch      */
#define  NT35510_800X480_HFP               NT35510_480X800_VFP    /* Horizontal front porch     */
#define  NT35510_800X480_VSYNC             NT35510_480X800_HSYNC  /* Vertical synchronization   */
#define  NT35510_800X480_VBP               NT35510_480X800_HBP    /* Vertical back porch        */
#define  NT35510_800X480_VFP               NT35510_480X800_HFP    /* Vertical front porch       */

/**
  * @brief  NT35510_480X800 frequency divider
  */
#define NT35510_480X800_FREQUENCY_DIVIDER  2   /* LCD Frequency divider      */

/**
  * @}
  */

/* Exported macro ------------------------------------------------------------*/

/** @defgroup NT35510_Exported_Macros NT35510 Exported Macros
  * @{
  */

/**
  * @}
  */

/* Exported functions --------------------------------------------------------*/

/** @addtogroup NT35510_Exported_Functions
  * @{
  */
int32_t NT35510_RegisterBusIO (NT35510_Object_t *pObj, NT35510_IO_t *pIO);
int32_t NT35510_Init(NT35510_Object_t *pObj, uint32_t ColorCoding, uint32_t Orientation);
int32_t NT35510_DeInit(NT35510_Object_t *pObj);
int32_t NT35510_ReadID(NT35510_Object_t *pObj, uint32_t *Id);
int32_t NT35510_DisplayOn(NT35510_Object_t *pObj);
int32_t NT35510_DisplayOff(NT35510_Object_t *pObj);
int32_t NT35510_SetBrightness(NT35510_Object_t *pObj, uint32_t Brightness);
int32_t NT35510_GetBrightness(NT35510_Object_t *pObj, uint32_t *Brightness);
int32_t NT35510_SetOrientation(NT35510_Object_t *pObj, uint32_t Orientation);
int32_t NT35510_GetOrientation(NT35510_Object_t *pObj, uint32_t *Orientation);


int32_t NT35510_SetCursor(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos);
int32_t NT35510_DrawBitmap(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint8_t *pBmp);
int32_t NT35510_FillRGBRect(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint8_t *pData, uint32_t Width, uint32_t Height);
int32_t NT35510_DrawHLine(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
int32_t NT35510_DrawVLine(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Length, uint32_t Color);
int32_t NT35510_FillRect(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Width, uint32_t Height, uint32_t Color);
int32_t NT35510_SetPixel(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t Color);
int32_t NT35510_GetPixel(NT35510_Object_t *pObj, uint32_t Xpos, uint32_t Ypos, uint32_t *Color);
int32_t NT35510_GetXSize(NT35510_Object_t *pObj, uint32_t *XSize);
int32_t NT35510_GetYSize(NT35510_Object_t *pObj, uint32_t *YSize);

extern NT35510_LCD_Drv_t   NT35510_LCD_Driver;

/**
  * @}
  */
#ifdef __cplusplus
}
#endif

#endif /* __NT35510_H */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */
