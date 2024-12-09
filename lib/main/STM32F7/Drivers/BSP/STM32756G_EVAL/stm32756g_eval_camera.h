/**
  ******************************************************************************
  * @file    stm32756g_eval_camera.h
  * @author  MCD Application Team
  * @brief   This file contains the common defines and functions prototypes for
  *          the stm32756g_eval_camera.c driver.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32756G_EVAL_CAMERA_H
#define __STM32756G_EVAL_CAMERA_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
/* Include Camera component Driver */
#include "../Components/s5k5cag/s5k5cag.h"
#include "../Components/ov5640/ov5640.h"
   
/* Include IO Driver */
#include "stm32756g_eval_io.h"

/** @addtogroup BSP
  * @{
  */

/** @addtogroup STM32756G_EVAL
  * @{
  */
    
/** @addtogroup STM32756G_EVAL_CAMERA
  * @{
  */ 
   
/** @defgroup STM32756G_EVAL_CAMERA_Exported_Types CAMERA Exported Types
  * @{
  */
  
/** 
  * @brief  Camera State structures definition  
  */  
typedef enum 
{
  CAMERA_OK            = 0x00,
  CAMERA_ERROR         = 0x01,
  CAMERA_TIMEOUT       = 0x02,
  CAMERA_NOT_DETECTED  = 0x03,
  CAMERA_NOT_SUPPORTED = 0x04

} Camera_StatusTypeDef;

#define RESOLUTION_R160x120      CAMERA_R160x120      /* QQVGA Resolution     */
#define RESOLUTION_R320x240      CAMERA_R320x240      /* QVGA Resolution      */
#define RESOLUTION_R480x272      CAMERA_R480x272      /* 480x272 Resolution   */
#define RESOLUTION_R640x480      CAMERA_R640x480      /* VGA Resolution       */  
/**
  * @}
  */ 
 
/** @defgroup STM32756G_EVAL_CAMERA_Exported_Constants CAMERA Exported Constants
  * @{
  */
#define BSP_CAMERA_IRQHandler      DCMI_IRQHandler
#define BSP_CAMERA_DMA_IRQHandler  DMA2_Stream1_IRQHandler 
/**
  * @}
  */

/** @defgroup STM32756G_EVAL_CAMERA_Exported_Functions CAMERA Exported Functions
  * @{
  */    
uint8_t BSP_CAMERA_Init(uint32_t Resolution);  
uint8_t BSP_CAMERA_DeInit(void);
void    BSP_CAMERA_ContinuousStart(uint8_t *buff);
void    BSP_CAMERA_SnapshotStart(uint8_t *buff);
void    BSP_CAMERA_Suspend(void);
void    BSP_CAMERA_Resume(void);
uint8_t BSP_CAMERA_Stop(void); 
void    BSP_CAMERA_HwReset(void);
void    BSP_CAMERA_PwrDown(void);
void    BSP_CAMERA_LineEventCallback(void);
void    BSP_CAMERA_VsyncEventCallback(void);
void    BSP_CAMERA_FrameEventCallback(void);
void    BSP_CAMERA_ErrorCallback(void);

/* Camera features functions prototype */
void    BSP_CAMERA_ContrastBrightnessConfig(uint32_t contrast_level, uint32_t brightness_level);
void    BSP_CAMERA_BlackWhiteConfig(uint32_t Mode);
void    BSP_CAMERA_ColorEffectConfig(uint32_t Effect);

/* To be called in DCMI_IRQHandler function */
void    BSP_CAMERA_IRQHandler(void);
/* To be called in DMA2_Stream1_IRQHandler function */
void    BSP_CAMERA_DMA_IRQHandler(void);
   
/* These functions can be modified in case the current settings (e.g. DMA stream)
   need to be changed for specific application needs */
void BSP_CAMERA_MspInit(DCMI_HandleTypeDef *hdcmi, void *Params);
void BSP_CAMERA_MspDeInit(DCMI_HandleTypeDef *hdcmi, void *Params);


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

#endif /* __STM32756G_EVAL_CAMERA_H */

