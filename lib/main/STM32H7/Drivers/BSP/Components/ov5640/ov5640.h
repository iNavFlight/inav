/**
  ******************************************************************************
  * @file    ov5640.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the ov5640.c
  *          driver.
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
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef OV5640_H
#define OV5640_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "ov5640_reg.h"
#include <stddef.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */

/** @addtogroup ov5640
  * @{
  */

/** @defgroup OV5640_Exported_Types
  * @{
  */

typedef int32_t (*OV5640_Init_Func)(void);
typedef int32_t (*OV5640_DeInit_Func)(void);
typedef int32_t (*OV5640_GetTick_Func)(void);
typedef int32_t (*OV5640_Delay_Func)(uint32_t);
typedef int32_t (*OV5640_WriteReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);
typedef int32_t (*OV5640_ReadReg_Func)(uint16_t, uint16_t, uint8_t *, uint16_t);

typedef struct
{
  OV5640_Init_Func          Init;
  OV5640_DeInit_Func        DeInit;
  uint16_t                  Address;
  OV5640_WriteReg_Func      WriteReg;
  OV5640_ReadReg_Func       ReadReg;
  OV5640_GetTick_Func       GetTick;
} OV5640_IO_t;


typedef struct
{
  OV5640_IO_t         IO;
  ov5640_ctx_t        Ctx;
  uint8_t             IsInitialized;
} OV5640_Object_t;

typedef struct
{
  uint8_t FrameStartCode; /*!< Specifies the code of the frame start delimiter. */
  uint8_t LineStartCode;  /*!< Specifies the code of the line start delimiter.  */
  uint8_t LineEndCode;    /*!< Specifies the code of the line end delimiter.    */
  uint8_t FrameEndCode;   /*!< Specifies the code of the frame end delimiter.   */

} OV5640_SyncCodes_t;

typedef struct
{
  uint32_t Config_Resolution;
  uint32_t Config_LightMode;
  uint32_t Config_SpecialEffect;
  uint32_t Config_Brightness;
  uint32_t Config_Saturation;
  uint32_t Config_Contrast;
  uint32_t Config_HueDegree;
  uint32_t Config_MirrorFlip;
  uint32_t Config_Zoom;
  uint32_t Config_NightMode;
} OV5640_Capabilities_t;

typedef struct
{
  int32_t (*Init)(OV5640_Object_t *, uint32_t, uint32_t);
  int32_t (*DeInit)(OV5640_Object_t *);
  int32_t (*ReadID)(OV5640_Object_t *, uint32_t *);
  int32_t (*GetCapabilities)(OV5640_Object_t *, OV5640_Capabilities_t *);
  int32_t (*SetLightMode)(OV5640_Object_t *, uint32_t);
  int32_t (*SetColorEffect)(OV5640_Object_t *, uint32_t);
  int32_t (*SetBrightness)(OV5640_Object_t *, int32_t);
  int32_t (*SetSaturation)(OV5640_Object_t *, int32_t);
  int32_t (*SetContrast)(OV5640_Object_t *, int32_t);
  int32_t (*SetHueDegree)(OV5640_Object_t *, int32_t);
  int32_t (*MirrorFlipConfig)(OV5640_Object_t *, uint32_t);
  int32_t (*ZoomConfig)(OV5640_Object_t *, uint32_t);
  int32_t (*SetResolution)(OV5640_Object_t *, uint32_t);
  int32_t (*GetResolution)(OV5640_Object_t *, uint32_t *);
  int32_t (*SetPixelFormat)(OV5640_Object_t *, uint32_t);
  int32_t (*GetPixelFormat)(OV5640_Object_t *, uint32_t *);
  int32_t (*NightModeConfig)(OV5640_Object_t *, uint32_t);
} OV5640_CAMERA_Drv_t;
/**
  * @}
  */

/** @defgroup OV5640_Exported_Constants
  * @{
  */
#define OV5640_OK                      (0)
#define OV5640_ERROR                   (-1)
/**
  * @brief  OV5640 Features Parameters
  */
/* Camera resolutions */
#define OV5640_R160x120                 0x00U   /* QQVGA Resolution           */
#define OV5640_R320x240                 0x01U   /* QVGA Resolution            */
#define OV5640_R480x272                 0x02U   /* 480x272 Resolution         */
#define OV5640_R640x480                 0x03U   /* VGA Resolution             */
#define OV5640_R800x480                 0x04U   /* WVGA Resolution            */

/* Camera Pixel Format */
#define OV5640_RGB565                   0x00U   /* Pixel Format RGB565        */
#define OV5640_RGB888                   0x01U   /* Pixel Format RGB888        */
#define OV5640_YUV422                   0x02U   /* Pixel Format YUV422        */
#define OV5640_Y8                       0x07U   /* Pixel Format Y8            */
#define OV5640_JPEG                     0x08U   /* Compressed format JPEG          */

/* Polarity */
#define OV5640_POLARITY_PCLK_LOW  0x00U /* Signal Active Low          */
#define OV5640_POLARITY_PCLK_HIGH 0x01U /* Signal Active High         */
#define OV5640_POLARITY_HREF_LOW  0x00U /* Signal Active Low          */
#define OV5640_POLARITY_HREF_HIGH 0x01U /* Signal Active High         */
#define OV5640_POLARITY_VSYNC_LOW 0x01U /* Signal Active Low          */
#define OV5640_POLARITY_VSYNC_HIGH  0x00U /* Signal Active High         */

/* Mirror/Flip */
#define OV5640_MIRROR_FLIP_NONE         0x00U   /* Set camera normal mode     */
#define OV5640_FLIP                     0x01U   /* Set camera flip config     */
#define OV5640_MIRROR                   0x02U   /* Set camera mirror config   */
#define OV5640_MIRROR_FLIP              0x03U   /* Set camera mirror and flip */

/* Zoom */
#define OV5640_ZOOM_x8                  0x00U   /* Set zoom to x8             */
#define OV5640_ZOOM_x4                  0x11U   /* Set zoom to x4             */
#define OV5640_ZOOM_x2                  0x22U   /* Set zoom to x2             */
#define OV5640_ZOOM_x1                  0x44U   /* Set zoom to x1             */

/* Special Effect */
#define OV5640_COLOR_EFFECT_NONE        0x00U   /* No effect                  */
#define OV5640_COLOR_EFFECT_BLUE        0x01U   /* Blue effect                */
#define OV5640_COLOR_EFFECT_RED         0x02U   /* Red effect                 */
#define OV5640_COLOR_EFFECT_GREEN       0x04U   /* Green effect               */
#define OV5640_COLOR_EFFECT_BW          0x08U   /* Black and White effect     */
#define OV5640_COLOR_EFFECT_SEPIA       0x10U   /* Sepia effect               */
#define OV5640_COLOR_EFFECT_NEGATIVE    0x20U   /* Negative effect            */


/* Light Mode */
#define OV5640_LIGHT_AUTO               0x00U   /* Light Mode Auto            */
#define OV5640_LIGHT_SUNNY              0x01U   /* Light Mode Sunny           */
#define OV5640_LIGHT_OFFICE             0x02U   /* Light Mode Office          */
#define OV5640_LIGHT_HOME               0x04U   /* Light Mode Home            */
#define OV5640_LIGHT_CLOUDY             0x08U   /* Light Mode Claudy          */

/* Night Mode */
#define NIGHT_MODE_DISABLE              0x00U   /* Disable night mode         */
#define NIGHT_MODE_ENABLE               0x01U   /* Enable night mode          */

/* Colorbar Mode */
#define COLORBAR_MODE_DISABLE           0x00U   /* Disable colorbar mode      */
#define COLORBAR_MODE_ENABLE            0x01U   /* 8 bars W/Y/C/G/M/R/B/Bl    */
#define COLORBAR_MODE_GRADUALV          0x02U   /* Gradual vertical colorbar  */

/* Pixel Clock */
#define OV5640_PCLK_7M                  0x00U   /* Pixel Clock set to 7Mhz    */
#define OV5640_PCLK_8M                  0x01U   /* Pixel Clock set to 8Mhz    */
#define OV5640_PCLK_9M                  0x02U   /* Pixel Clock set to 9Mhz    */
#define OV5640_PCLK_12M                 0x04U   /* Pixel Clock set to 12Mhz   */
#define OV5640_PCLK_24M                 0x08U   /* Pixel Clock set to 24Mhz   */

/**
  * @}
  */

/** @defgroup OV5640_Exported_Functions OV5640 Exported Functions
  * @{
  */
int32_t OV5640_RegisterBusIO(OV5640_Object_t *pObj, OV5640_IO_t *pIO);
int32_t OV5640_Init(OV5640_Object_t *pObj, uint32_t Resolution, uint32_t PixelFormat);
int32_t OV5640_DeInit(OV5640_Object_t *pObj);
int32_t OV5640_ReadID(OV5640_Object_t *pObj, uint32_t *Id);
int32_t OV5640_GetCapabilities(OV5640_Object_t *pObj, OV5640_Capabilities_t *Capabilities);
int32_t OV5640_SetLightMode(OV5640_Object_t *pObj, uint32_t LightMode);
int32_t OV5640_SetColorEffect(OV5640_Object_t *pObj, uint32_t Effect);
int32_t OV5640_SetBrightness(OV5640_Object_t *pObj, int32_t Level);
int32_t OV5640_SetSaturation(OV5640_Object_t *pObj, int32_t Level);
int32_t OV5640_SetContrast(OV5640_Object_t *pObj, int32_t Level);
int32_t OV5640_SetHueDegree(OV5640_Object_t *pObj, int32_t Degree);
int32_t OV5640_MirrorFlipConfig(OV5640_Object_t *pObj, uint32_t Config);
int32_t OV5640_ZoomConfig(OV5640_Object_t *pObj, uint32_t Zoom);
int32_t OV5640_SetResolution(OV5640_Object_t *pObj, uint32_t Resolution);
int32_t OV5640_GetResolution(OV5640_Object_t *pObj, uint32_t *Resolution);
int32_t OV5640_SetPixelFormat(OV5640_Object_t *pObj, uint32_t PixelFormat);
int32_t OV5640_GetPixelFormat(OV5640_Object_t *pObj, uint32_t *PixelFormat);
int32_t OV5640_SetPolarities(OV5640_Object_t *pObj, uint32_t PclkPolarity, uint32_t HrefPolarity,
                             uint32_t VsyncPolarity);
int32_t OV5640_GetPolarities(OV5640_Object_t *pObj, uint32_t *PclkPolarity, uint32_t *HrefPolarity,
                             uint32_t *VsyncPolarity);
int32_t OV5640_NightModeConfig(OV5640_Object_t *pObj, uint32_t Cmd);
int32_t OV5640_ColorbarModeConfig(OV5640_Object_t *pObj, uint32_t Cmd);
int32_t OV5640_EmbeddedSynchroConfig(OV5640_Object_t *pObj, OV5640_SyncCodes_t *pSyncCodes);
int32_t OV5640_SetPCLK(OV5640_Object_t *pObj, uint32_t ClockValue);

/* CAMERA driver structure */
extern OV5640_CAMERA_Drv_t   OV5640_CAMERA_Driver;
/**
  * @}
  */
#ifdef __cplusplus
}
#endif

#endif /* OV5640_H */
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
