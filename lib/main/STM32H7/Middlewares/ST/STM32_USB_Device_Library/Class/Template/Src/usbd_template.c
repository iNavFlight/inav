/**
  ******************************************************************************
  * @file    usbd_template.c
  * @author  MCD Application Team
  * @brief   This file provides the HID core functions.
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
  * @verbatim
  *
  *          ===================================================================
  *                                TEMPLATE Class  Description
  *          ===================================================================
  *
  *
  *
  *
  *
  *
  * @note     In HS mode and when the DMA is used, all variables and data structures
  *           dealing with the DMA during the transaction process should be 32-bit aligned.
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_template.h"
#include "usbd_ctlreq.h"


/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_TEMPLATE
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_TEMPLATE_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_TEMPLATE_Private_Defines
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_TEMPLATE_Private_Macros
  * @{
  */

/**
  * @}
  */




/** @defgroup USBD_TEMPLATE_Private_FunctionPrototypes
  * @{
  */


static uint8_t USBD_TEMPLATE_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_TEMPLATE_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_TEMPLATE_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_TEMPLATE_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_TEMPLATE_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_TEMPLATE_EP0_RxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_TEMPLATE_EP0_TxReady(USBD_HandleTypeDef *pdev);
static uint8_t USBD_TEMPLATE_SOF(USBD_HandleTypeDef *pdev);
static uint8_t USBD_TEMPLATE_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_TEMPLATE_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t *USBD_TEMPLATE_GetCfgDesc(uint16_t *length);
static uint8_t *USBD_TEMPLATE_GetDeviceQualifierDesc(uint16_t *length);
/**
  * @}
  */

/** @defgroup USBD_TEMPLATE_Private_Variables
  * @{
  */

USBD_ClassTypeDef USBD_TEMPLATE_ClassDriver =
{
  USBD_TEMPLATE_Init,
  USBD_TEMPLATE_DeInit,
  USBD_TEMPLATE_Setup,
  USBD_TEMPLATE_EP0_TxReady,
  USBD_TEMPLATE_EP0_RxReady,
  USBD_TEMPLATE_DataIn,
  USBD_TEMPLATE_DataOut,
  USBD_TEMPLATE_SOF,
  USBD_TEMPLATE_IsoINIncomplete,
  USBD_TEMPLATE_IsoOutIncomplete,
  USBD_TEMPLATE_GetCfgDesc,
  USBD_TEMPLATE_GetCfgDesc,
  USBD_TEMPLATE_GetCfgDesc,
  USBD_TEMPLATE_GetDeviceQualifierDesc,
};

#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma data_alignment=4
#endif /* __ICCARM__ */
/* USB TEMPLATE device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_TEMPLATE_CfgDesc[USB_TEMPLATE_CONFIG_DESC_SIZ] __ALIGN_END =
{
  0x09, /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_TEMPLATE_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x02,         /*iConfiguration: Index of string descriptor describing the configuration*/
  0xC0,         /*bmAttributes: bus powered and Supports Remote Wakeup */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/
  /* 09 */

  /**********  Descriptor of TEMPLATE interface 0 Alternate setting 0 **************/

};

#if defined ( __ICCARM__ ) /*!< IAR Compiler */
#pragma data_alignment=4
#endif /* __ICCARM__ */
/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_TEMPLATE_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40,
  0x01,
  0x00,
};

/**
  * @}
  */

/** @defgroup USBD_TEMPLATE_Private_Functions
  * @{
  */

/**
  * @brief  USBD_TEMPLATE_Init
  *         Initialize the TEMPLATE interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_TEMPLATE_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_TEMPLATE_Init
  *         DeInitialize the TEMPLATE layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_TEMPLATE_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_TEMPLATE_Setup
  *         Handle the TEMPLATE specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_TEMPLATE_Setup(USBD_HandleTypeDef *pdev,
                                   USBD_SetupReqTypedef *req)
{
  USBD_StatusTypeDef ret = USBD_OK;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    case USB_REQ_TYPE_CLASS :
      switch (req->bRequest)
      {
        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    default:
      USBD_CtlError(pdev, req);
      ret = USBD_FAIL;
      break;
  }

  return (uint8_t)ret;
}

/**
  * @brief  USBD_TEMPLATE_GetCfgDesc
  *         return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_TEMPLATE_GetCfgDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_TEMPLATE_CfgDesc);
  return USBD_TEMPLATE_CfgDesc;
}

/**
  * @brief  USBD_TEMPLATE_GetDeviceQualifierDesc
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
uint8_t *USBD_TEMPLATE_GetDeviceQualifierDesc(uint16_t *length)
{
  *length = (uint16_t)sizeof(USBD_TEMPLATE_DeviceQualifierDesc);
  return USBD_TEMPLATE_DeviceQualifierDesc;
}

/**
  * @brief  USBD_TEMPLATE_DataIn
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_TEMPLATE_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_TEMPLATE_EP0_RxReady
  *         handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_TEMPLATE_EP0_RxReady(USBD_HandleTypeDef *pdev)
{

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_TEMPLATE_EP0_TxReady
  *         handle EP0 TRx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_TEMPLATE_EP0_TxReady(USBD_HandleTypeDef *pdev)
{

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_TEMPLATE_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t USBD_TEMPLATE_SOF(USBD_HandleTypeDef *pdev)
{

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_TEMPLATE_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_TEMPLATE_IsoINIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_TEMPLATE_IsoOutIncomplete
  *         handle data ISO OUT Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_TEMPLATE_IsoOutIncomplete(USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  return (uint8_t)USBD_OK;
}
/**
  * @brief  USBD_TEMPLATE_DataOut
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t USBD_TEMPLATE_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  return (uint8_t)USBD_OK;
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
