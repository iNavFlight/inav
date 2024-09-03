/**
  ******************************************************************************
  * @file    usbd_mtp.c
  * @author  MCD Application Team
  * @brief   This file provides the high layer firmware functions to manage the
  *          following functionalities of the USB MTP Class:
  *           - Initialization and Configuration of high and low layer
  *           - Enumeration as MTP Device (and enumeration for each implemented memory interface)
  *           - OUT/IN data transfer
  *           - Command IN transfer (class requests management)
  *           - Error management
  *
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
  *  @verbatim
  *
  *          ===================================================================
  *                                MTP Class Driver Description
  *          ===================================================================
  *           This driver manages the "Universal Serial Bus Class Definitions for Media Transfer Protocol
  *           Revision 1.1 April 6, 2011"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as MTP device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management
  *
  *
  *  @endverbatim
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_mtp.h"
#include "usbd_mtp_storage.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */


/** @defgroup USBD_MTP
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_MTP_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_MTP_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_MTP_Private_Macros
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_MTP_Private_FunctionPrototypes
  * @{
  */
static uint8_t USBD_MTP_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_MTP_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t USBD_MTP_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t USBD_MTP_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t USBD_MTP_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);

#ifndef USE_USBD_COMPOSITE
static uint8_t *USBD_MTP_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_MTP_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_MTP_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_MTP_GetDeviceQualifierDescriptor(uint16_t *length);
#endif /* USE_USBD_COMPOSITE  */

/**
  * @}
  */

/** @defgroup USBD_MTP_Private_Variables
  * @{
  */


/* MTP interface class callbacks structure */
USBD_ClassTypeDef USBD_MTP =
{
  USBD_MTP_Init,
  USBD_MTP_DeInit,
  USBD_MTP_Setup,
  NULL, /*EP0_TxSent*/
  NULL, /*EP0_RxReady*/
  USBD_MTP_DataIn,
  USBD_MTP_DataOut,
  NULL, /*SOF */
  NULL, /*ISOIn*/
  NULL, /*ISOOut*/
#ifdef USE_USBD_COMPOSITE
  NULL,
  NULL,
  NULL,
  NULL,
#else
  USBD_MTP_GetHSCfgDesc,
  USBD_MTP_GetFSCfgDesc,
  USBD_MTP_GetOtherSpeedCfgDesc,
  USBD_MTP_GetDeviceQualifierDescriptor,
#endif /* USE_USBD_COMPOSITE  */
};

#ifndef USE_USBD_COMPOSITE

/* USB MTP device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MTP_CfgDesc[MTP_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration Descriptor */
  0x09,                                         /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                  /* bDescriptorType: Configuration */
  LOBYTE(MTP_CONFIG_DESC_SIZ),                  /* wTotalLength: Total size of the Config descriptor */
  HIBYTE(MTP_CONFIG_DESC_SIZ),
  0x01,                                         /* bNumInterfaces: 1 interface */
  0x01,                                         /* bConfigurationValue: Configuration value */
  0x00,                                         /* iConfiguration: Index of string descriptor
                                                   describing the configuration */
#if (USBD_SELF_POWERED == 1U)
  0xC0,                                         /* bmAttributes: Bus Powered according to user configuration */
#else
  0x80,                                         /* bmAttributes: Bus Powered according to user configuration */
#endif /* USBD_SELF_POWERED */
  USBD_MAX_POWER,                               /* MaxPower (mA) */

  /********************  MTP **** interface ********************/
  MTP_INTERFACE_DESC_SIZE,                      /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                      /* bDescriptorType: Interface descriptor type */
  MTP_CMD_ITF_NBR,                              /* bInterfaceNumber: Number of Interface */
  0x00,                                         /* bAlternateSetting: Alternate setting */
  0x03,                                         /* bNumEndpoints:  */
  USB_MTP_INTRERFACE_CLASS,                     /* bInterfaceClass: bInterfaceClass: user's interface for MTP */
  USB_MTP_INTRERFACE_SUB_CLASS,                 /* bInterfaceSubClass:Abstract Control Model */
  USB_MTP_INTRERFACE_PROTOCOL,                  /* bInterfaceProtocol: Common AT commands */
  0x00,                                         /* iInterface: */

  /********************  MTP   Endpoints ********************/
  MTP_ENDPOINT_DESC_SIZE,                       /* Endpoint descriptor length = 7 */
  USB_DESC_TYPE_ENDPOINT,                       /* Endpoint descriptor type */
  MTP_IN_EP,                                    /* Endpoint address (IN, address 1) */
  USBD_EP_TYPE_BULK,                            /* Bulk endpoint type */
  LOBYTE(MTP_DATA_MAX_FS_PACKET_SIZE),
  HIBYTE(MTP_DATA_MAX_FS_PACKET_SIZE),
  0x00,                                         /* Polling interval in milliseconds */

  MTP_ENDPOINT_DESC_SIZE,                       /* Endpoint descriptor length = 7 */
  USB_DESC_TYPE_ENDPOINT,                       /* Endpoint descriptor type */
  MTP_OUT_EP,                                   /* Endpoint address (OUT, address 1) */
  USBD_EP_TYPE_BULK,                            /* Bulk endpoint type */
  LOBYTE(MTP_DATA_MAX_FS_PACKET_SIZE),
  HIBYTE(MTP_DATA_MAX_FS_PACKET_SIZE),
  0x00,                                         /* Polling interval in milliseconds */

  MTP_ENDPOINT_DESC_SIZE,                       /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                       /* bDescriptorType:*/
  MTP_CMD_EP,                                   /* bEndpointAddress: Endpoint Address (IN) */
  USBD_EP_TYPE_INTR,                            /* bmAttributes: Interrupt endpoint */
  LOBYTE(MTP_CMD_PACKET_SIZE),
  HIBYTE(MTP_CMD_PACKET_SIZE),
  MTP_FS_BINTERVAL                              /* Polling interval in milliseconds */
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_MTP_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
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
#endif /* USE_USBD_COMPOSITE  */

uint8_t MTPInEpAdd = MTP_IN_EP;
uint8_t MTPOutEpAdd = MTP_OUT_EP;
uint8_t MTPCmdEpAdd = MTP_CMD_EP;

/**
  * @}
  */

/** @defgroup USBD_MTP_Private_Functions
  * @{
  */

/**
  * @brief  USBD_MTP_Init
  *         Initialize the MTP interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_MTP_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);
  USBD_MTP_HandleTypeDef *hmtp;

  hmtp = (USBD_MTP_HandleTypeDef *)USBD_malloc(sizeof(USBD_MTP_HandleTypeDef));

  if (hmtp == NULL)
  {
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    return (uint8_t)USBD_EMEM;
  }

  /* Setup the pClassData pointer */
  pdev->pClassDataCmsit[pdev->classId] = (void *)hmtp;
  pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  MTPInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  MTPOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  MTPCmdEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */


  /* Initialize all variables */
  (void)USBD_memset(hmtp, 0, sizeof(USBD_MTP_HandleTypeDef));

  /* Setup the max packet size according to selected speed */
  if (pdev->dev_speed == USBD_SPEED_HIGH)
  {
    hmtp->MaxPcktLen = MTP_DATA_MAX_HS_PACKET_SIZE;
  }
  else
  {
    hmtp->MaxPcktLen = MTP_DATA_MAX_FS_PACKET_SIZE;
  }

  /* Open EP IN */
  (void)USBD_LL_OpenEP(pdev, MTPInEpAdd, USBD_EP_TYPE_BULK, hmtp->MaxPcktLen);
  pdev->ep_in[MTPInEpAdd & 0xFU].is_used = 1U;

  /* Open EP OUT */
  (void)USBD_LL_OpenEP(pdev, MTPOutEpAdd, USBD_EP_TYPE_BULK, hmtp->MaxPcktLen);
  pdev->ep_out[MTPOutEpAdd & 0xFU].is_used = 1U;

  /* Open INTR EP IN */
  (void)USBD_LL_OpenEP(pdev, MTPCmdEpAdd, USBD_EP_TYPE_INTR, MTP_CMD_PACKET_SIZE);
  pdev->ep_in[MTPCmdEpAdd & 0xFU].is_used = 1U;

  /* Init the MTP  layer */
  (void)USBD_MTP_STORAGE_Init(pdev);

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MTP_DeInit
  *         DeInitialize the MTP layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_MTP_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this MTP class instance */
  MTPInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  MTPOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  MTPCmdEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  /* Close EP IN */
  (void)USBD_LL_CloseEP(pdev, MTPInEpAdd);
  pdev->ep_in[MTPInEpAdd & 0xFU].is_used = 0U;

  /* Close EP OUT */
  (void)USBD_LL_CloseEP(pdev, MTPOutEpAdd);
  pdev->ep_out[MTPOutEpAdd & 0xFU].is_used = 0U;

  /* Close EP Command */
  (void)USBD_LL_CloseEP(pdev, MTPCmdEpAdd);
  pdev->ep_in[MTPCmdEpAdd & 0xFU].is_used = 0U;

  /* Free MTP Class Resources */
  if (pdev->pClassDataCmsit[pdev->classId] != NULL)
  {
    /* De-Init the MTP layer */
    (void)USBD_MTP_STORAGE_DeInit(pdev);

    (void)USBD_free(pdev->pClassDataCmsit[pdev->classId]);
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MTP_Setup
  *         Handle the MTP specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_MTP_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_MTP_HandleTypeDef *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  USBD_StatusTypeDef ret = USBD_OK;
  uint16_t len = 0U;

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this MTP class instance */
  MTPOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  if (hmtp == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    /* Class request */
    case USB_REQ_TYPE_CLASS :
      switch (req->bRequest)
      {
        case  MTP_REQ_CANCEL:
          len = MIN(hmtp->MaxPcktLen, req->wLength);
          (void)USBD_CtlPrepareRx(pdev, (uint8_t *)(hmtp->rx_buff), len);
          break;

        case MTP_REQ_GET_EXT_EVENT_DATA:
          break;

        case MTP_REQ_RESET:
          /* Stop low layer file system operations if any */
          USBD_MTP_STORAGE_Cancel(pdev, MTP_PHASE_IDLE);

          (void)USBD_LL_PrepareReceive(pdev, MTPOutEpAdd, (uint8_t *)&hmtp->rx_buff, hmtp->MaxPcktLen);
          break;

        case MTP_REQ_GET_DEVICE_STATUS:
          switch (hmtp->MTP_ResponsePhase)
          {
            case MTP_READ_DATA :
              len = 4U;
              hmtp->dev_status = ((uint32_t)MTP_RESPONSE_DEVICE_BUSY << 16) | len;
              break;

            case MTP_RECEIVE_DATA :
              len = 4U;
              hmtp->dev_status = ((uint32_t)MTP_RESPONSE_TRANSACTION_CANCELLED << 16) | len;
              break;

            case MTP_PHASE_IDLE :
              len = 4U;
              hmtp->dev_status = ((uint32_t)MTP_RESPONSE_OK << 16) | len;
              break;

            default:
              break;
          }
          (void)USBD_CtlSendData(pdev, (uint8_t *)&hmtp->dev_status, len);
          break;

        default:
          USBD_CtlError(pdev, req);
          ret = USBD_FAIL;
          break;
      }
      break;

    /* Interface & Endpoint request */
    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_INTERFACE :

          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            hmtp->alt_setting = 0U;
            (void)USBD_CtlSendData(pdev, (uint8_t *)&hmtp->alt_setting, 1U);
          }
          break;

        case USB_REQ_SET_INTERFACE :
          if (pdev->dev_state != USBD_STATE_CONFIGURED)
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:

          /* Re-activate the EP */
          (void)USBD_LL_CloseEP(pdev, (uint8_t)req->wIndex);

          if ((((uint8_t)req->wIndex) & 0x80U) == 0x80U)
          {
            (void)USBD_LL_OpenEP(pdev, ((uint8_t)req->wIndex), USBD_EP_TYPE_BULK, hmtp->MaxPcktLen);
          }
          else
          {
            (void)USBD_LL_OpenEP(pdev, ((uint8_t)req->wIndex), USBD_EP_TYPE_BULK, hmtp->MaxPcktLen);
          }
          break;

        default:
          break;
      }
      break;

    default:
      break;
  }
  return (uint8_t)ret;
}

/**
  * @brief  USBD_MTP_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_MTP_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  UNUSED(epnum);
  USBD_MTP_HandleTypeDef *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint16_t len;

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this MTP class instance */
  MTPInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  MTPOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  if (epnum == (MTPInEpAdd & 0x7FU))
  {
    switch (hmtp->MTP_ResponsePhase)
    {
      case MTP_RESPONSE_PHASE :
        (void)USBD_MTP_STORAGE_SendContainer(pdev, REP_TYPE);

        /* prepare to receive next operation */
        len = MIN(hmtp->MaxPcktLen, pdev->request.wLength);

        (void)USBD_LL_PrepareReceive(pdev, MTPOutEpAdd, (uint8_t *)&hmtp->rx_buff, len);
        hmtp->MTP_ResponsePhase = MTP_PHASE_IDLE;
        break;

      case MTP_READ_DATA :
        (void)USBD_MTP_STORAGE_ReadData(pdev);

        /* prepare to receive next operation */
        len = MIN(hmtp->MaxPcktLen, pdev->request.wLength);

        (void)USBD_LL_PrepareReceive(pdev, MTPInEpAdd, (uint8_t *)&hmtp->rx_buff, len);
        break;

      case MTP_PHASE_IDLE :
        /* prepare to receive next operation */
        len = MIN(hmtp->MaxPcktLen, pdev->request.wLength);

        (void)USBD_LL_PrepareReceive(pdev, MTPOutEpAdd, (uint8_t *)&hmtp->rx_buff, len);

        break;
      default:
        break;
    }
  }
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MTP_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_MTP_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  UNUSED(epnum);
  USBD_MTP_HandleTypeDef *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint16_t len;

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this MTP class instance */
  MTPOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  (void)USBD_MTP_STORAGE_ReceiveOpt(pdev);

  switch (hmtp->MTP_ResponsePhase)
  {
    case MTP_RESPONSE_PHASE :

      if (hmtp->ResponseLength == MTP_CONT_HEADER_SIZE)
      {
        (void)USBD_MTP_STORAGE_SendContainer(pdev, REP_TYPE);
        hmtp->MTP_ResponsePhase = MTP_PHASE_IDLE;
      }
      else
      {
        (void)USBD_MTP_STORAGE_SendContainer(pdev, DATA_TYPE);
      }
      break;

    case MTP_READ_DATA :
      (void)USBD_MTP_STORAGE_ReadData(pdev);
      break;

    case MTP_RECEIVE_DATA :
      (void)USBD_MTP_STORAGE_ReceiveData(pdev);

      /* prepare endpoint to receive operations */
      len = MIN(hmtp->MaxPcktLen, pdev->request.wLength);

      (void)USBD_LL_PrepareReceive(pdev, MTPOutEpAdd, (uint8_t *)&hmtp->rx_buff, len);
      break;

    case MTP_PHASE_IDLE :
      /* prepare to receive next operation */
      len = MIN(hmtp->MaxPcktLen, pdev->request.wLength);

      (void)USBD_LL_PrepareReceive(pdev, MTPOutEpAdd, (uint8_t *)&hmtp->rx_buff, len);
      break;

    default:
      break;
  }

  return (uint8_t)USBD_OK;
}

#ifndef USE_USBD_COMPOSITE
/**
  * @brief  USBD_MTP_GetHSCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_MTP_GetHSCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpInDesc = USBD_GetEpDesc(USBD_MTP_CfgDesc, MTP_IN_EP);
  USBD_EpDescTypeDef *pEpOutDesc = USBD_GetEpDesc(USBD_MTP_CfgDesc, MTP_OUT_EP);
  USBD_EpDescTypeDef *pEpCmdDesc = USBD_GetEpDesc(USBD_MTP_CfgDesc, MTP_CMD_EP);

  if (pEpInDesc != NULL)
  {
    pEpInDesc->wMaxPacketSize = MTP_DATA_MAX_HS_PACKET_SIZE;
  }

  if (pEpOutDesc != NULL)
  {
    pEpOutDesc->wMaxPacketSize = MTP_DATA_MAX_HS_PACKET_SIZE;
  }

  if (pEpCmdDesc != NULL)
  {
    pEpCmdDesc->bInterval = MTP_HS_BINTERVAL;
  }

  *length = (uint16_t)sizeof(USBD_MTP_CfgDesc);
  return USBD_MTP_CfgDesc;
}

/**
  * @brief  USBD_MTP_GetFSCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_MTP_GetFSCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpInDesc = USBD_GetEpDesc(USBD_MTP_CfgDesc, MTP_IN_EP);
  USBD_EpDescTypeDef *pEpOutDesc = USBD_GetEpDesc(USBD_MTP_CfgDesc, MTP_OUT_EP);
  USBD_EpDescTypeDef *pEpCmdDesc = USBD_GetEpDesc(USBD_MTP_CfgDesc, MTP_CMD_EP);

  if (pEpInDesc != NULL)
  {
    pEpInDesc->wMaxPacketSize = MTP_DATA_MAX_FS_PACKET_SIZE;
  }

  if (pEpOutDesc != NULL)
  {
    pEpOutDesc->wMaxPacketSize = MTP_DATA_MAX_FS_PACKET_SIZE;
  }

  if (pEpCmdDesc != NULL)
  {
    pEpCmdDesc->bInterval = MTP_FS_BINTERVAL;
  }

  *length = (uint16_t)sizeof(USBD_MTP_CfgDesc);
  return USBD_MTP_CfgDesc;
}

/**
  * @brief  USBD_MTP_GetOtherSpeedCfgDesc
  *         Return configuration descriptor
  * @param  speed : current device speed
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_MTP_GetOtherSpeedCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpInDesc = USBD_GetEpDesc(USBD_MTP_CfgDesc, MTP_IN_EP);
  USBD_EpDescTypeDef *pEpOutDesc = USBD_GetEpDesc(USBD_MTP_CfgDesc, MTP_OUT_EP);
  USBD_EpDescTypeDef *pEpCmdDesc = USBD_GetEpDesc(USBD_MTP_CfgDesc, MTP_CMD_EP);

  if (pEpInDesc != NULL)
  {
    pEpInDesc->wMaxPacketSize = MTP_DATA_MAX_FS_PACKET_SIZE;
  }

  if (pEpOutDesc != NULL)
  {
    pEpOutDesc->wMaxPacketSize = MTP_DATA_MAX_FS_PACKET_SIZE;
  }

  if (pEpCmdDesc != NULL)
  {
    pEpCmdDesc->bInterval = MTP_FS_BINTERVAL;
  }

  *length = (uint16_t)sizeof(USBD_MTP_CfgDesc);
  return USBD_MTP_CfgDesc;
}

/**
  * @brief  USBD_MTP_GetDeviceQualifierDescriptor
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_MTP_GetDeviceQualifierDescriptor(uint16_t *length)
{
  *length = (uint16_t)(sizeof(USBD_MTP_DeviceQualifierDesc));
  return USBD_MTP_DeviceQualifierDesc;
}
#endif /* USE_USBD_COMPOSITE  */

/**
  * @brief  USBD_MTP_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @retval status
  */
uint8_t USBD_MTP_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MTP_ItfTypeDef *fops)
{
  if (fops == NULL)
  {
    return (uint8_t)USBD_FAIL;
  }

  pdev->pUserData[pdev->classId] = fops;

  return (uint8_t)USBD_OK;
}

/**
  * @}
  */

/**
  * @}
  */

