/**
  ******************************************************************************
  * @file    usbd_mtp_storage.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_mtp_storage.c file.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_MTP_STORAGE_H__
#define __USBD_MTP_STORAGE_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_ctlreq.h"
#include "usbd_mtp_opt.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_MTP_STORAGE
  * @brief This file is the header file for usbd_template_core.c
  * @{
  */


/** @defgroup USBD_MTP_STORAGE_Exported_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_MTP_STORAGE_Exported_TypesDefinitions
  * @{
  */

typedef enum
{
  DATA_TYPE = 0x00,
  REP_TYPE = 0x01,
} MTP_CONTAINER_TYPE;


typedef enum
{
  READ_FIRST_DATA = 0x00,
  READ_REST_OF_DATA = 0x01,
} MTP_READ_DATA_STATUS;


/**
  * @}
  */



/** @defgroup USBD_MTP_STORAGE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_MTP_STORAGE_Exported_Variables
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_MTP_STORAGE_Exported_Functions
  * @{
  */

uint8_t USBD_MTP_STORAGE_Init(USBD_HandleTypeDef  *pdev);
uint8_t USBD_MTP_STORAGE_DeInit(USBD_HandleTypeDef  *pdev);
void USBD_MTP_STORAGE_Cancel(USBD_HandleTypeDef  *pdev, MTP_ResponsePhaseTypeDef MTP_ResponsePhase);
uint8_t USBD_MTP_STORAGE_ReadData(USBD_HandleTypeDef  *pdev);
uint8_t USBD_MTP_STORAGE_SendContainer(USBD_HandleTypeDef  *pdev, MTP_CONTAINER_TYPE CONT_TYPE);
uint8_t USBD_MTP_STORAGE_ReceiveOpt(USBD_HandleTypeDef  *pdev);
uint8_t USBD_MTP_STORAGE_ReceiveData(USBD_HandleTypeDef  *pdev);


/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_MTP_STORAGE_H */
/**
  * @}
  */

/**
  * @}
  */
