/**
  ******************************************************************************
  * @file    ov9655.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the ov9655.c
  *          driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2015 STMicroelectronics.
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
#ifndef OV9655_H
#define OV9655_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "ov9655_reg.h"
#include <stddef.h>
   
/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Components
  * @{
  */ 
  
/** @addtogroup ov9655
  * @{
  */

/** @defgroup OV9655_Exported_Types Exported Types
  * @{
  */

typedef int32_t (*OV9655_Init_Func)    (void);
typedef int32_t (*OV9655_DeInit_Func)  (void);
typedef int32_t (*OV9655_GetTick_Func) (void);
typedef int32_t (*OV9655_Delay_Func)   (uint32_t);
typedef int32_t (*OV9655_WriteReg_Func)(uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*OV9655_ReadReg_Func) (uint16_t, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  OV9655_Init_Func          Init;
  OV9655_DeInit_Func        DeInit;
  uint16_t                  Address;  
  OV9655_WriteReg_Func      WriteReg;
  OV9655_ReadReg_Func       ReadReg; 
  OV9655_GetTick_Func       GetTick; 
} OV9655_IO_t;

 
typedef struct
{
  OV9655_IO_t         IO;
  ov9655_ctx_t        Ctx;   
  uint8_t             IsInitialized;
} OV9655_Object_t;

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
} OV9655_Capabilities_t;

typedef struct
{
  int32_t  (*Init              )(OV9655_Object_t*, uint32_t, uint32_t);
  int32_t  (*DeInit            )(OV9655_Object_t*);    
  int32_t  (*ReadID            )(OV9655_Object_t*, uint32_t*); 
  int32_t  (*GetCapabilities   )(OV9655_Object_t*, OV9655_Capabilities_t*);  
  int32_t  (*SetLightMode      )(OV9655_Object_t*, uint32_t);
  int32_t  (*SetColorEffect    )(OV9655_Object_t*, uint32_t);
  int32_t  (*SetBrightness     )(OV9655_Object_t*, int32_t);
  int32_t  (*SetSaturation     )(OV9655_Object_t*, int32_t);
  int32_t  (*SetContrast       )(OV9655_Object_t*, int32_t);
  int32_t  (*SetHueDegree      )(OV9655_Object_t*, int32_t);
  int32_t  (*MirrorFlipConfig  )(OV9655_Object_t*, uint32_t);
  int32_t  (*ZoomConfig        )(OV9655_Object_t*, uint32_t);
  int32_t  (*SetResolution     )(OV9655_Object_t*, uint32_t);  
  int32_t  (*GetResolution     )(OV9655_Object_t*, uint32_t*);
  int32_t  (*SetPixelFormat    )(OV9655_Object_t*, uint32_t);  
  int32_t  (*GetPixelFormat    )(OV9655_Object_t*, uint32_t*);  
  int32_t  (*NightModeConfig   )(OV9655_Object_t*, uint32_t);
}OV9655_CAMERA_Drv_t;

/**
  * @}
  */ 

/** @defgroup OV9655_Exported_Constants Exported Constants
  * @{
  */
#define OV9655_OK                      (0)
#define OV9655_ERROR                   (-1)
/** 
 * @brief  OV9655 Features Parameters  
 */
/* Camera resolutions */ 
#define OV9655_R160x120                 0x00U   /* QQVGA Resolution           */
#define OV9655_R320x240                 0x01U   /* QVGA Resolution            */
#define OV9655_R480x272                 0x02U   /* 480x272 Resolution: Not 
                                                   supported                  */
#define OV9655_R640x480                 0x03U   /* VGA Resolution             */

/* Camera Pixel Format */ 
#define OV9655_RGB565                   0x00U   /* Pixel Format RGB565        */
#define OV9655_RGB888                   0x01U   /* Pixel Format RGB888:  
                                                   Not supported              */
#define OV9655_YUV422                   0x02U   /* Pixel Format YUV422        */

/* Mirror/Flip */
#define OV9655_MIRROR_FLIP_NONE         0x00U   /* Set camera normal mode     */
#define OV9655_FLIP                     0x01U   /* Set camera flip config     */
#define OV9655_MIRROR                   0x02U   /* Set camera mirror config   */
#define OV9655_MIRROR_FLIP              0x03U   /* Set camera mirror and flip */
 
/* Zoom */
#define OV9655_ZOOM_x8                  0x00U   /* Set zoom to x8             */
#define OV9655_ZOOM_x4                  0x11U   /* Set zoom to x4             */
#define OV9655_ZOOM_x2                  0x22U   /* Set zoom to x2             */
#define OV9655_ZOOM_x1                  0x44U   /* Set zoom to x1             */

/* Special Effect */
#define OV9655_COLOR_EFFECT_NONE        0x00U   /* No effect                  */
#define OV9655_COLOR_EFFECT_BLUE        0x01U   /* Blue effect                */
#define OV9655_COLOR_EFFECT_RED         0x02U   /* Red effect                 */
#define OV9655_COLOR_EFFECT_GREEN       0x04U   /* Green effect               */   
#define OV9655_COLOR_EFFECT_BW          0x08U   /* Black and White effect     */ 
#define OV9655_COLOR_EFFECT_SEPIA       0x10U   /* Sepia effect               */ 
#define OV9655_COLOR_EFFECT_NEGATIVE    0x20U   /* Negative effect            */ 

   
/* Light Mode */
#define OV9655_LIGHT_AUTO               0x00U   /* Light Mode Auto            */
#define OV9655_LIGHT_SUNNY              0x01U   /* Light Mode Sunny           */
#define OV9655_LIGHT_OFFICE             0x02U   /* Light Mode Office          */
#define OV9655_LIGHT_HOME               0x04U   /* Light Mode Home            */
#define OV9655_LIGHT_CLOUDY             0x08U   /* Light Mode Claudy          */

/* Night Mode */
#define NIGHT_MODE_DISABLE              0x00U   /* Disable night mode         */
#define NIGHT_MODE_ENABLE               0x01U   /* Enable night mode          */   
    
/**
  * @}
  */
  
/** @defgroup OV9655_Exported_Functions OV9655 Exported Functions
  * @{
  */
int32_t OV9655_RegisterBusIO (OV9655_Object_t *pObj, OV9655_IO_t *pIO);
int32_t OV9655_Init(OV9655_Object_t *pObj, uint32_t Resolution, uint32_t PixelFormat);
int32_t OV9655_DeInit(OV9655_Object_t *pObj);
int32_t OV9655_ReadID(OV9655_Object_t *pObj, uint32_t *Id);
int32_t OV9655_GetCapabilities(OV9655_Object_t *pObj, OV9655_Capabilities_t *Capabilities);
int32_t OV9655_SetLightMode(OV9655_Object_t *pObj, uint32_t LightMode);
int32_t OV9655_SetColorEffect(OV9655_Object_t *pObj, uint32_t Effect);
int32_t OV9655_SetBrightness(OV9655_Object_t *pObj, int32_t Level);
int32_t OV9655_SetSaturation(OV9655_Object_t *pObj, int32_t Level);
int32_t OV9655_SetContrast(OV9655_Object_t *pObj, int32_t Level);
int32_t OV9655_SetHueDegree(OV9655_Object_t *pObj, int32_t Degree);
int32_t OV9655_MirrorFlipConfig(OV9655_Object_t *pObj, uint32_t Config);
int32_t OV9655_ZoomConfig(OV9655_Object_t *pObj, uint32_t Zoom);
int32_t OV9655_SetResolution(OV9655_Object_t *pObj, uint32_t Resolution);
int32_t OV9655_GetResolution(OV9655_Object_t *pObj, uint32_t *Resolution);
int32_t OV9655_SetPixelFormat(OV9655_Object_t *pObj, uint32_t PixelFormat);
int32_t OV9655_GetPixelFormat(OV9655_Object_t *pObj, uint32_t *PixelFormat);
int32_t OV9655_NightModeConfig(OV9655_Object_t *pObj, uint32_t Cmd);

/* CAMERA driver structure */
extern OV9655_CAMERA_Drv_t   OV9655_CAMERA_Driver;
/**
  * @}
  */    
#ifdef __cplusplus
}
#endif

#endif /* OV9655_H */
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
