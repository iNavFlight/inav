/**
  ******************************************************************************
  * @file    ov5640.h
  * @author  MCD Application Team
  * @brief   This file contains all the functions prototypes for the ov5640.c
  *          driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
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
#ifndef __OV5640_H
#define __OV5640_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "../Common/camera.h"
   
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
     
/**
  * @}
  */ 

/** @defgroup OV5640_Exported_Constants
  * @{
  */
/** 
  * @brief  OV5640 ID  
  */  
#define  OV5640_ID    0x5640
/** 
  * @brief  OV5640 Registers  
  */


/** 
 * @brief  OV5640 Features Parameters  
 */
 
/* Brightness */ 
#define OV5640_BRIGHTNESS_LEVEL4P       0x00   /* Brightness level +4         */   
#define OV5640_BRIGHTNESS_LEVEL3P       0x01   /* Brightness level +3         */
#define OV5640_BRIGHTNESS_LEVEL2P       0x02   /* Brightness level +2         */   
#define OV5640_BRIGHTNESS_LEVEL1P       0x04   /* Brightness level +1         */
#define OV5640_BRIGHTNESS_LEVEL0        0x08   /* Brightness level 0          */
#define OV5640_BRIGHTNESS_LEVEL1N       0x10   /* Brightness level -1         */
#define OV5640_BRIGHTNESS_LEVEL2N       0x20   /* Brightness level -2         */
#define OV5640_BRIGHTNESS_LEVEL3N       0x40   /* Brightness level -3         */
#define OV5640_BRIGHTNESS_LEVEL4N       0x80   /* Brightness level -4         */

/* Saturation */ 
#define OV5640_SATURATION_LEVEL4P       0x00   /* Saturation level +4         */   
#define OV5640_SATURATION_LEVEL3P       0x01   /* Saturation level +3         */
#define OV5640_SATURATION_LEVEL2P       0x02   /* Saturation level +2         */   
#define OV5640_SATURATION_LEVEL1P       0x04   /* Saturation level +1         */
#define OV5640_SATURATION_LEVEL0        0x08   /* Saturation level 0          */
#define OV5640_SATURATION_LEVEL1N       0x10   /* Saturation level -1         */
#define OV5640_SATURATION_LEVEL2N       0x20   /* Saturation level -2         */
#define OV5640_SATURATION_LEVEL3N       0x40   /* Saturation level -3         */
#define OV5640_SATURATION_LEVEL4N       0x80   /* Saturation level -4         */

/* Contrast */ 
#define OV5640_CONTRAST_LEVEL4P         0x00   /* Contrast level +4           */   
#define OV5640_CONTRAST_LEVEL3P         0x01   /* Contrast level +3           */
#define OV5640_CONTRAST_LEVEL2P         0x02   /* Contrast level +2           */   
#define OV5640_CONTRAST_LEVEL1P         0x04   /* Contrast level +1           */
#define OV5640_CONTRAST_LEVEL0          0x08   /* Contrast level 0            */
#define OV5640_CONTRAST_LEVEL1N         0x10   /* Contrast level -1           */
#define OV5640_CONTRAST_LEVEL2N         0x20   /* Contrast level -2           */
#define OV5640_CONTRAST_LEVEL3N         0x40   /* Contrast level -3           */
#define OV5640_CONTRAST_LEVEL4N         0x80   /* Contrast level -4           */


/* Hue Control */   
#define OV5640_HUE_150P                 0x0001   /* Hue 150+ degree           */ 
#define OV5640_HUE_120P                 0x0002   /* Hue 120+ degree           */   
#define OV5640_HUE_90P                  0x0004   /* Hue 90+ degree            */
#define OV5640_HUE_60P                  0x0008   /* Hue 60+ degree            */   
#define OV5640_HUE_30P                  0x0010   /* Hue 30+ degree            */
#define OV5640_HUE_0                    0x0020   /* Hue 0 degree              */
#define OV5640_HUE_30N                  0x0040   /* Hue 30- degree            */
#define OV5640_HUE_60N                  0x0080   /* Hue 60- degree            */
#define OV5640_HUE_90N                  0x0100   /* Hue 90- degree            */
#define OV5640_HUE_120N                 0x0200   /* Hue 120- degree           */    
#define OV5640_HUE_150N                 0x0400   /* Hue 150- degree           */   
#define OV5640_HUE_180N                 0x0800   /* Hue 180- degree           */

/* Mirror/Flip */
#define OV5640_MIRROR                   0x00   /* Set camera mirror config    */
#define OV5640_FLIP                     0x01   /* Set camera flip config      */
#define OV5640_MIRROR_FLIP              0x02   /* Set camera mirror and flip  */
#define OV5640_MIRROR_FLIP_NORMAL       0x04   /* Set camera normal mode      */
   
/* Zoom */
#define OV5640_ZOOM_x8                  0x00
#define OV5640_ZOOM_x4                  0x11   
#define OV5640_ZOOM_x2                  0x22   
#define OV5640_ZOOM_x1                  0x44   

/* Special Effect */
#define OV5640_COLOR_EFFECT_NONE              0x00 /* No effect               */
#define OV5640_COLOR_EFFECT_BLUE              0x01 /* Blue effect             */
#define OV5640_COLOR_EFFECT_RED               0x02 /* Red effect              */
#define OV5640_COLOR_EFFECT_GREEN             0x04 /* Green effect            */
#define OV5640_COLOR_EFFECT_BW                0x08 /* Black and White effect  */
#define OV5640_COLOR_EFFECT_SEPIA             0x10 /* Sepia effect            */
#define OV5640_COLOR_EFFECT_NEGATIVE          0x20 /* Negative effect         */
#define OV5640_COLOR_EFFECT_BW_NEGATIVE       0x40 /* BW Negative effect      */
#define OV5640_COLOR_EFFECT_OVEREXPOSURE      0x80 /* Over exposure effect    */
#define OV5640_COLOR_EFFECT_SOLARIZE          0x100 /* Solarized effect       */ 
   
/* Light Mode */
#define OV5640_LIGHT_AUTO               0x00   /* Light Mode Auto             */
#define OV5640_LIGHT_SUNNY              0x01   /* Light Mode Sunny            */
#define OV5640_LIGHT_OFFICE             0x02   /* Light Mode Office           */
#define OV5640_LIGHT_HOME               0x04   /* Light Mode Home             */
#define OV5640_LIGHT_CLOUDY             0x08   /* Light Mode Claudy           */

/* Saturation */ 
#define OV5640_SATURATION_0             0x00   /* Color saturation 0          */
#define OV5640_SATURATION_1             0x01   /* Color saturation 1          */
#define OV5640_SATURATION_2             0x02   /* Color saturation 2          */
#define OV5640_SATURATION_3             0x04   /* Color saturation 3          */

/* Exposure */ 
#define OV5640_EXPOSURE_LEVEL_0         0x00   /* Exposure Level 0            */
#define OV5640_EXPOSURE_LEVEL_1         0x01   /* Exposure Level 1            */
#define OV5640_EXPOSURE_LEVEL_2         0x02   /* Exposure Level 2            */
#define OV5640_EXPOSURE_LEVEL_3         0x04   /* Exposure Level 3            */

/**
  * @}
  */
  
/** @defgroup OV5640_Exported_Functions
  * @{
  */ 
void     ov5640_Init(uint16_t DeviceAddr, uint32_t resolution);
void     ov5640_Config(uint16_t DeviceAddr, uint32_t feature, uint32_t value, uint32_t BR_value);
uint16_t ov5640_ReadID(uint16_t DeviceAddr);
void     OV5640_SetLightMode(uint16_t DeviceAddr, uint8_t LightMode);
void     OV5640_SetEffect(uint16_t DeviceAddr, uint32_t Effect);
void     OV5640_SetBrightness(uint16_t DeviceAddr, uint8_t Level);
void     OV5640_SetSaturation(uint16_t DeviceAddr, uint8_t Level);
void     OV5640_SetContrast(uint16_t DeviceAddr, uint8_t Level);
void     OV5640_SetHueDegree(uint16_t DeviceAddr, uint16_t Degree);
void     OV5640_MirrorFlipConfig(uint16_t DeviceAddr, uint8_t Config);
void     OV5640_ZoomConfig(uint16_t DeviceAddr, uint8_t Zoom);
int32_t  OV5640_GetResolution(uint16_t DeviceAddr);
void     CAMERA_IO_Init(void);
void     CAMERA_IO_Write(uint8_t addr, uint16_t reg, uint16_t value);
uint16_t CAMERA_IO_Read(uint8_t Addr, uint16_t Reg);
void     CAMERA_Delay(uint32_t delay);

/* CAMERA driver structure */
extern CAMERA_DrvTypeDef   ov5640_drv;
/**
  * @}
  */    
#ifdef __cplusplus
}
#endif

#endif /* __OV5640_H */
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
