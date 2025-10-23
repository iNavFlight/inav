/**
  ******************************************************************************
  * @file    s5k5cag.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the s5k5cag.c
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
#ifndef S5K5CAG_H
#define S5K5CAG_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "s5k5cag_reg.h"
#include <stddef.h>
   
/** @addtogroup BSP
  * @{
  */ 

/** @addtogroup Components
  * @{
  */ 
  
/** @addtogroup S5K5CAG
  * @{
  */

/** @defgroup S5K5CAG_Exported_Types Exported Types
  * @{
  */
typedef int32_t (*S5K5CAG_Init_Func)    (void);
typedef int32_t (*S5K5CAG_DeInit_Func)  (void);
typedef int32_t (*S5K5CAG_GetTick_Func) (void);
typedef int32_t (*S5K5CAG_Delay_Func)   (uint32_t);
typedef int32_t (*S5K5CAG_WriteReg_Func)(uint16_t, uint16_t, uint8_t*, uint16_t);
typedef int32_t (*S5K5CAG_ReadReg_Func) (uint16_t, uint16_t, uint8_t*, uint16_t);

typedef struct
{
  S5K5CAG_Init_Func          Init;
  S5K5CAG_DeInit_Func        DeInit;
  uint16_t                   Address;  
  S5K5CAG_WriteReg_Func      WriteReg;
  S5K5CAG_ReadReg_Func       ReadReg; 
  S5K5CAG_GetTick_Func       GetTick; 
} S5K5CAG_IO_t;

 
typedef struct
{
  S5K5CAG_IO_t         IO;
  s5k5cag_ctx_t        Ctx;   
  uint32_t             IsInitialized;
} S5K5CAG_Object_t;

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
} S5K5CAG_Capabilities_t;

typedef struct
{
  int32_t  (*Init              )(S5K5CAG_Object_t*, uint32_t, uint32_t);
  int32_t  (*DeInit            )(S5K5CAG_Object_t*);    
  int32_t  (*ReadID            )(S5K5CAG_Object_t*, uint32_t*);
  int32_t  (*GetCapabilities   )(S5K5CAG_Object_t*, S5K5CAG_Capabilities_t*);   
  int32_t  (*SetLightMode      )(S5K5CAG_Object_t*, uint32_t);
  int32_t  (*SetEffect         )(S5K5CAG_Object_t*, uint32_t);
  int32_t  (*SetBrightness     )(S5K5CAG_Object_t*, int32_t);
  int32_t  (*SetSaturation     )(S5K5CAG_Object_t*, int32_t);
  int32_t  (*SetContrast       )(S5K5CAG_Object_t*, int32_t);
  int32_t  (*SetHueDegree      )(S5K5CAG_Object_t*, int32_t);
  int32_t  (*MirrorFlipConfig  )(S5K5CAG_Object_t*, uint32_t);
  int32_t  (*ZoomConfig        )(S5K5CAG_Object_t*, uint32_t);
  int32_t  (*SetResolution     )(S5K5CAG_Object_t*, uint32_t);  
  int32_t  (*GetResolution     )(S5K5CAG_Object_t*, uint32_t*);
  int32_t  (*SetPixelFormat    )(S5K5CAG_Object_t*, uint32_t);  
  int32_t  (*GetPixelFormat    )(S5K5CAG_Object_t*, uint32_t*);  
  int32_t  (*NightModeConfig   )(S5K5CAG_Object_t*, uint32_t);
}S5K5CAG_CAMERA_Drv_t;
     
/**
  * @}
  */ 

/** @defgroup S5K5CAG_Exported_Constants Exported Constants
  * @{
  */
  
#define S5K5CAG_OK                           (0)
#define S5K5CAG_ERROR                        (-1)

/** 
 * @brief  S5K5CAG Features Parameters
 */
/* Camera resolutions */ 
#define S5K5CAG_R160x120                      0x00U   /* QQVGA Resolution     */
#define S5K5CAG_R320x240                      0x01U   /* QVGA Resolution      */
#define S5K5CAG_R480x272                      0x02U   /* 480x272 Resolution   */
#define S5K5CAG_R640x480                      0x03U   /* VGA Resolution       */
#define S5K5CAG_R800x480                      0x04U   /* WVGA Resolution      */

/* Camera Pixel Format */ 
#define S5K5CAG_RGB565                        0x00U   /* Pixel Format RGB565  */
#define S5K5CAG_RGB888                        0x01U   /* Pixel Format RGB888  */
#define S5K5CAG_YUV422                        0x02U   /* Pixel Format YUV422  */


/* Special Effect */
#define S5K5CAG_COLOR_EFFECT_NONE             0x00U /* No effect              */
#define S5K5CAG_COLOR_EFFECT_BLUE             0x01U /* Blue effect            */
#define S5K5CAG_COLOR_EFFECT_RED              0x02U /* Red effect             */
#define S5K5CAG_COLOR_EFFECT_GREEN            0x04U /* Green effect           */   
#define S5K5CAG_COLOR_EFFECT_BW               0x08U /* Black and White effect */ 
#define S5K5CAG_COLOR_EFFECT_SEPIA            0x10U /* Sepia effect           */ 
#define S5K5CAG_COLOR_EFFECT_NEGATIVE         0x20U /* Negative effect        */ 
   
/**
  * @}
  */

  
/** @addtogroup S5K5CAG_Exported_Functions
  * @{
  */ 
int32_t S5K5CAG_RegisterBusIO (S5K5CAG_Object_t *pObj, S5K5CAG_IO_t *pIO);  
int32_t S5K5CAG_Init(S5K5CAG_Object_t *pObj, uint32_t Resolution, uint32_t PixelFormat);
int32_t S5K5CAG_DeInit(S5K5CAG_Object_t *pObj);
int32_t S5K5CAG_ReadID(S5K5CAG_Object_t *pObj, uint32_t *Id);
int32_t S5K5CAG_GetCapabilities(S5K5CAG_Object_t *pObj, S5K5CAG_Capabilities_t *Capabilities);
int32_t S5K5CAG_SetLightMode(S5K5CAG_Object_t *pObj, uint32_t LightMode);
int32_t S5K5CAG_SetColorEffect(S5K5CAG_Object_t *pObj, uint32_t Effect);
int32_t S5K5CAG_SetBrightness(S5K5CAG_Object_t *pObj, int32_t Level);
int32_t S5K5CAG_SetSaturation(S5K5CAG_Object_t *pObj, int32_t Level);
int32_t S5K5CAG_SetContrast(S5K5CAG_Object_t *pObj, int32_t Level);
int32_t S5K5CAG_SetHueDegree(S5K5CAG_Object_t *pObj, int32_t Degree);
int32_t S5K5CAG_MirrorFlipConfig(S5K5CAG_Object_t *pObj, uint32_t Config);
int32_t S5K5CAG_ZoomConfig(S5K5CAG_Object_t *pObj, uint32_t Zoom);
int32_t S5K5CAG_SetPixelFormat(S5K5CAG_Object_t *pObj, uint32_t PixelFormat);
int32_t S5K5CAG_GetPixelFormat(S5K5CAG_Object_t *pObj, uint32_t *PixelFormat);
int32_t S5K5CAG_SetResolution(S5K5CAG_Object_t *pObj, uint32_t Resolution);
int32_t S5K5CAG_GetResolution(S5K5CAG_Object_t *pObj, uint32_t *Resolution);
int32_t S5K5CAG_NightModeConfig(S5K5CAG_Object_t *pObj, uint32_t Cmd);

/* CAMERA driver structure */
extern S5K5CAG_CAMERA_Drv_t   S5K5CAG_CAMERA_Driver;
/**
  * @}
  */    
#ifdef __cplusplus
}
#endif

#endif /* S5K5CAG_H */
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
