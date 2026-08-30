/**
  ******************************************************************************
  * @file    usbh_mtp.c
  * @author  MCD Application Team
  * @brief   This file is the MTP Layer Handlers for USB Host MTP class.
  *
  *
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

/* Includes ------------------------------------------------------------------*/
#include "usbh_template.h"

/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_TEMPLATE_CLASS
  * @{
  */

/** @defgroup USBH_TEMPLATE_CORE
  * @brief    This file includes TEMPLATE Layer Handlers for USB Host TEMPLATE class.
  * @{
  */

/** @defgroup USBH_TEMPLATE_CORE_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_TEMPLATE_CORE_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_TEMPLATE_CORE_Private_Macros
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_TEMPLATE_CORE_Private_Variables
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_TEMPLATE_CORE_Private_FunctionPrototypes
  * @{
  */

static USBH_StatusTypeDef USBH_TEMPLATE_InterfaceInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_TEMPLATE_InterfaceDeInit(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_TEMPLATE_Process(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_TEMPLATE_ClassRequest(USBH_HandleTypeDef *phost);


USBH_ClassTypeDef  TEMPLATE_Class =
{
  "TEMPLATE",
  USB_TEMPLATE_CLASS,
  USBH_TEMPLATE_InterfaceInit,
  USBH_TEMPLATE_InterfaceDeInit,
  USBH_TEMPLATE_ClassRequest,
  USBH_TEMPLATE_Process
};
/**
  * @}
  */


/** @defgroup USBH_TEMPLATE_CORE_Private_Functions
  * @{
  */

/**
  * @brief  USBH_TEMPLATE_InterfaceInit
  *         The function init the TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_TEMPLATE_InterfaceInit(USBH_HandleTypeDef *phost)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(phost);

  return USBH_OK;
}


/**
  * @brief  USBH_TEMPLATE_InterfaceDeInit
  *         The function DeInit the Pipes used for the TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_TEMPLATE_InterfaceDeInit(USBH_HandleTypeDef *phost)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(phost);

  return USBH_OK;
}


/**
  * @brief  USBH_TEMPLATE_ClassRequest
  *         The function is responsible for handling Standard requests
  *         for TEMPLATE class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_TEMPLATE_ClassRequest(USBH_HandleTypeDef *phost)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(phost);

  return USBH_OK;
}


/**
  * @brief  USBH_TEMPLATE_Process
  *         The function is for managing state machine for TEMPLATE data transfers
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_TEMPLATE_Process(USBH_HandleTypeDef *phost)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(phost);

  return USBH_OK;
}


/**
  * @brief  USBH_TEMPLATE_Init
  *         The function Initialize the TEMPLATE function
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_TEMPLATE_Init(USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef Status = USBH_BUSY;

#if (USBH_USE_OS == 1U)
  osEvent event;

  event = osMessageGet(phost->class_ready_event, osWaitForever);

  if (event.status == osEventMessage)
  {
    if (event.value.v == USBH_CLASS_EVENT)
    {
#else
  while ((Status == USBH_BUSY) || (Status == USBH_FAIL))
  {
    /* Host background process */
    USBH_Process(phost);

    if (phost->gState == HOST_CLASS)
    {
#endif
      Status = USBH_OK;
    }
  }
  return Status;
}

/**
  * @brief  USBH_TEMPLATE_IOProcess
  *         TEMPLATE TEMPLATE process
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_TEMPLATE_IOProcess(USBH_HandleTypeDef *phost)
{
  if (phost->device.is_connected == 1U)
  {
    if (phost->gState == HOST_CLASS)
    {
      USBH_TEMPLATE_Process(phost);
    }
  }

  return USBH_OK;
}

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


/**
  * @}
  */

