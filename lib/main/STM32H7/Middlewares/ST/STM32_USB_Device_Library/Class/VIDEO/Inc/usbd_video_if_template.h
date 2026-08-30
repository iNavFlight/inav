/**
  ******************************************************************************
  * @file    usbd_video_if_template.h
  * @author  MCD Application Team
  * @brief   Template Header file for the video Interface application layer functions
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_VIDEO_IF_H__
#define __USBD_VIDEO_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_video.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief For Usb device.
  * @{
  */

/** @defgroup USBD_VIDEO_IF
  * @brief Usb VIDEO interface device module.
  * @{
  */

/** @defgroup USBD_VIDEO_IF_Exported_Defines
  * @brief Defines.
  * @{
  */

/* USER CODE BEGIN EXPORTED_DEFINES */

/* USER CODE END EXPORTED_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_VIDEO_IF_Exported_Types
  * @brief Types.
  * @{
  */

/* USER CODE BEGIN EXPORTED_TYPES */

/* USER CODE END EXPORTED_TYPES */

/**
  * @}
  */

/** @defgroup USBD_VIDEO_IF_Exported_Macros
  * @brief Aliases.
  * @{
  */

/* USER CODE BEGIN EXPORTED_MACRO */

/* USER CODE END EXPORTED_MACRO */

/**
  * @}
  */

/** @defgroup USBD_VIDEO_IF_Exported_Variables
  * @brief Public variables.
  * @{
  */

/** VIDEO_IF Interface callback. */
extern USBD_VIDEO_ItfTypeDef USBD_VIDEO_fops_FS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_VIDEO_IF_Exported_FunctionsPrototype
  * @brief Public functions declaration.
  * @{
  */

/**
  * @brief  Manages the DMA full transfer complete event.
  * @retval None
  */
void TransferComplete_CallBack_FS(void);

/**
  * @brief  Manages the DMA half transfer complete event.
  * @retval None
  */
void HalfTransfer_CallBack_FS(void);



#define IMG_NBR                   1U
#define IMAGE_SIZE              0x1U

const  uint8_t image[] = {0x00};
const  uint8_t *tImagesList[] = {image};
uint16_t tImagesSizes[] = {IMAGE_SIZE};

/* Time laps between video frames in ms.
   Please adjust this value depending on required speed.
   Please note that this define uses the system HAL_Delay() which uses the systick.
   In case of changes on HAL_Delay, please ensure the values in ms correspond. */
#ifdef USE_USB_HS
#define USBD_VIDEO_IMAGE_LAPS                     160U
#else
#define USBD_VIDEO_IMAGE_LAPS                     80U
#endif /* USE_USB_HS */

/* USER CODE BEGIN EXPORTED_FUNCTIONS */

/* USER CODE END EXPORTED_FUNCTIONS */

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

#endif /* USBD_VIDEO_IF_H_ */

