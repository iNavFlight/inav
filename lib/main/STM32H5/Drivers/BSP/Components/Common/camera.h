/**
  ******************************************************************************
  * @file    camera.h
  * @author  MCD Application Team
  * @brief   This header file contains the common defines and functions prototypes
  *          for the camera driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2018 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
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
  int32_t (*Init)(void *, uint32_t, uint32_t);
  int32_t (*DeInit)(void *);
  int32_t (*ReadID)(void *, uint32_t *);
  int32_t (*GetCapabilities)(void *, void *);
  int32_t (*SetLightMode)(void *, uint32_t);
  int32_t (*SetColorEffect)(void *, uint32_t);
  int32_t (*SetBrightness)(void *, int32_t);
  int32_t (*SetSaturation)(void *, int32_t);
  int32_t (*SetContrast)(void *, int32_t);
  int32_t (*SetHueDegree)(void *, int32_t);
  int32_t (*MirrorFlipConfig)(void *, uint32_t);
  int32_t (*ZoomConfig)(void *, uint32_t);
  int32_t (*SetResolution)(void *, uint32_t);
  int32_t (*GetResolution)(void *, uint32_t *);
  int32_t (*SetPixelFormat)(void *, uint32_t);
  int32_t (*GetPixelFormat)(void *, uint32_t *);
  int32_t (*NightModeConfig)(void *, uint32_t);
} CAMERA_Drv_t;

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
