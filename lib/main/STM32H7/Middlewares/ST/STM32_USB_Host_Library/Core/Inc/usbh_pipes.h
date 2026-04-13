/**
  ******************************************************************************
  * @file    usbh_pipes.h
  * @author  MCD Application Team
  * @brief   Header file for usbh_pipes.c
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
#ifndef __USBH_PIPES_H
#define __USBH_PIPES_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"

/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_LIB_CORE
  * @{
  */

/** @defgroup USBH_PIPES
  * @brief This file is the header file for usbh_pipes.c
  * @{
  */

/** @defgroup USBH_PIPES_Exported_Defines
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_PIPES_Exported_Types
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_PIPES_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_PIPES_Exported_Variables
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_PIPES_Exported_FunctionsPrototype
  * @{
  */

USBH_StatusTypeDef USBH_OpenPipe(USBH_HandleTypeDef *phost,
                                 uint8_t pipe_num,
                                 uint8_t epnum,
                                 uint8_t dev_address,
                                 uint8_t speed,
                                 uint8_t ep_type,
                                 uint16_t mps);

USBH_StatusTypeDef USBH_ClosePipe(USBH_HandleTypeDef *phost,
                                  uint8_t pipe_num);

uint8_t USBH_AllocPipe(USBH_HandleTypeDef *phost,
                       uint8_t ep_addr);

USBH_StatusTypeDef USBH_FreePipe(USBH_HandleTypeDef *phost,
                                 uint8_t idx);

#if defined (USBH_IN_NAK_PROCESS) && (USBH_IN_NAK_PROCESS == 1U)
USBH_StatusTypeDef USBH_ActivatePipe(USBH_HandleTypeDef *phost,
                                     uint8_t pipe_num);
#endif /* defined (USBH_IN_NAK_PROCESS) && (USBH_IN_NAK_PROCESS == 1U) */

/**
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif /* __USBH_PIPES_H */


/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */


