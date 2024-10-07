/**
  ******************************************************************************
  * @file    usbh_template.h
  * @author  MCD Application Team
  * @brief   This file contains all the prototypes for the usbh_template.c
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2015 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive  ----------------------------------------------*/
#ifndef __USBH_TEMPLATE_H
#define __USBH_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"


/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_TEMPLATE_CLASS
  * @{
  */

/** @defgroup USBH_TEMPLATE_CLASS
  * @brief This file is the Header file for usbh_template.c
  * @{
  */


/**
  * @}
  */

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Types
  * @{
  */

/* States for TEMPLATE State Machine */


/**
  * @}
  */

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Defines
  * @{
  */

/**
  * @}
  */

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_TEMPLATE_CLASS_Exported_Variables
  * @{
  */
extern USBH_ClassTypeDef  TEMPLATE_Class;
#define USBH_TEMPLATE_CLASS    &TEMPLATE_Class

/**
  * @}
  */

/** @defgroup USBH_TEMPLATE_CLASS_Exported_FunctionsPrototype
  * @{
  */
USBH_StatusTypeDef USBH_TEMPLATE_IOProcess(USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_TEMPLATE_Init(USBH_HandleTypeDef *phost);
/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBH_TEMPLATE_H */

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

