/**
  ******************************************************************************
  * @file    camera.h
  * @author  MCD Application Team
  * @brief   This header file contains the common defines and functions prototypes
  *          for the camera driver.   
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
#ifndef CAMERA_H
#define CAMERA_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

/** @addtogroup BSP
  * @{
  */

/** @addtogroup Components
  * @{
  */
    
/** @addtogroup CAMERA
  * @{
  */


/** @defgroup CAMERA_Exported_Types
  * @{
  */ 
/**
  * @}
  */

/** @defgroup CAMERA_Driver_structure  Camera Driver structure
  * @{
  */
typedef struct
{
  int32_t  (*Init              )(void*, uint32_t, uint32_t);
  int32_t  (*DeInit            )(void*);    
  int32_t  (*ReadID            )(void*, uint32_t*);
  int32_t  (*GetCapabilities   )(void*, void*);  
  int32_t  (*SetLightMode      )(void*, uint32_t);
  int32_t  (*SetColorEffect    )(void*, uint32_t);
  int32_t  (*SetBrightness     )(void*, int32_t);
  int32_t  (*SetSaturation     )(void*, int32_t);
  int32_t  (*SetContrast       )(void*, int32_t);
  int32_t  (*SetHueDegree      )(void*, int32_t);
  int32_t  (*MirrorFlipConfig  )(void*, uint32_t);
  int32_t  (*ZoomConfig        )(void*, uint32_t);
  int32_t  (*SetResolution     )(void*, uint32_t);  
  int32_t  (*GetResolution     )(void*, uint32_t*);
  int32_t  (*SetPixelFormat    )(void*, uint32_t);  
  int32_t  (*GetPixelFormat    )(void*, uint32_t*);  
  int32_t  (*NightModeConfig   )(void*, uint32_t);
}CAMERA_Drv_t;

/**
  * @}
  */

/** @defgroup CAMERA_Exported_Constants
  * @{
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

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* CAMERA_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
