/**
  ******************************************************************************
  * @file    usbd_ccid.c
  * @author  MCD Application Team
  * @brief   This file provides the high layer firmware functions to manage
  *          all the functionalities of the USB CCID Class:
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
  *                                CCID Class Driver Description
  *          ===================================================================
  *           This module manages the Specification for Integrated Circuit(s)
  *             Cards Interface Revision 1.1
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as CCID device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *               and enumeration for each implemented memory interface
  *             - Bulk OUT/IN data Transfers
  *             - Requests management
  *
  *  @endverbatim
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usbd_ccid.h"
#include "usbd_ccid_cmd.h"
#include "usbd_ctlreq.h"
/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_CCID
  * @brief usbd core module
  * @{
  */

/** @defgroup USBD_CCID_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CCID_Private_Defines
  * @{
  */
/**
  * @}
  */


/** @defgroup USBD_CCID_Private_Macros
  * @{
  */

/**
  * @}
  */


/** @defgroup USBD_CCID_Private_FunctionPrototypes
  * @{
  */
static uint8_t  USBD_CCID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_CCID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx);
static uint8_t  USBD_CCID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req);
static uint8_t  USBD_CCID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_CCID_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum);
static uint8_t  USBD_CCID_DispatchCommand(USBD_HandleTypeDef *pdev);
static uint8_t  USBD_CCID_ReceiveCmdHeader(USBD_HandleTypeDef *pdev,
                                           uint8_t *pDst, uint16_t u8length);
#ifndef USE_USBD_COMPOSITE
static uint8_t *USBD_CCID_GetHSCfgDesc(uint16_t *length);
static uint8_t *USBD_CCID_GetFSCfgDesc(uint16_t *length);
static uint8_t *USBD_CCID_GetOtherSpeedCfgDesc(uint16_t *length);
static uint8_t *USBD_CCID_GetDeviceQualifierDescriptor(uint16_t *length);
#endif /* USE_USBD_COMPOSITE  */

/**
  * @}
  */

/** @defgroup USBD_CCID_Private_Variables
  * @{
  */

static uint8_t CCIDInEpAdd = CCID_IN_EP;
static uint8_t CCIDOutEpAdd = CCID_OUT_EP;
static uint8_t CCIDCmdEpAdd = CCID_CMD_EP;


/* CCID interface class callbacks structure */
USBD_ClassTypeDef USBD_CCID =
{
  USBD_CCID_Init,
  USBD_CCID_DeInit,
  USBD_CCID_Setup,
  NULL, /*EP0_TxSent*/
  NULL, /*EP0_RxReady*/
  USBD_CCID_DataIn,
  USBD_CCID_DataOut,
  NULL, /*SOF */
  NULL, /*ISOIn*/
  NULL, /*ISOOut*/
#ifdef USE_USBD_COMPOSITE
  NULL,
  NULL,
  NULL,
  NULL,
#else
  USBD_CCID_GetHSCfgDesc,
  USBD_CCID_GetFSCfgDesc,
  USBD_CCID_GetOtherSpeedCfgDesc,
  USBD_CCID_GetDeviceQualifierDescriptor,
#endif /* USE_USBD_COMPOSITE  */
};

#ifndef USE_USBD_COMPOSITE

/* USB CCID device Configuration Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CCID_CfgDesc[USB_CCID_CONFIG_DESC_SIZ] __ALIGN_END =
{
  /* Configuration Descriptor */
  0x09,                                                  /* bLength: Configuration Descriptor size */
  USB_DESC_TYPE_CONFIGURATION,                           /* bDescriptorType: Configuration */
  USB_CCID_CONFIG_DESC_SIZ,                              /* wTotalLength:no of returned bytes */
  0x00,
  0x01,                                                  /* bNumInterfaces: 1 interface */
  0x01,                                                  /* bConfigurationValue: */
  0x00,                                                  /* iConfiguration: */
#if (USBD_SELF_POWERED == 1U)
  0xC0,                                                  /* bmAttributes: Bus Powered according to user configuration */
#else
  0x80,                                                  /* bmAttributes: Bus Powered according to user configuration */
#endif /* USBD_SELF_POWERED */
  USBD_MAX_POWER,                                        /* MaxPower (mA) */

  /********************  CCID **** interface ********************/
  CCID_INTERFACE_DESC_SIZE,                              /* bLength: Interface Descriptor size */
  USB_DESC_TYPE_INTERFACE,                               /* bDescriptorType: */
  0x00,                                                  /* bInterfaceNumber: Number of Interface */
  0x00,                                                  /* bAlternateSetting: Alternate setting */
  0x03,                                                  /* bNumEndpoints: 3 endpoints used */
  USB_DEVICE_CLASS_CCID,                                 /* bInterfaceClass: user's interface for CCID */
  0x00,                                                  /* bInterfaceSubClass : No subclass,
                                                            can be changed but no description in USB 2.0 Spec */
  0x00,                                                  /* nInterfaceProtocol : None */
  0x00,                                                  /* iInterface */

  /*******************  CCID class descriptor ********************/
  CCID_CLASS_DESC_SIZE,                                  /* bLength: CCID Descriptor size */
  CCID_DESC_TYPE,                                        /* bDescriptorType: Functional Descriptor type. */
  0x10,                                                  /* bcdCCID(LSB): CCID Class Spec release number (1.1) */
  0x01,                                                  /* bcdCCID(MSB) */

  0x00,                                                  /* bMaxSlotIndex :highest available slot on this device */
  CCID_VOLTAGE_SUPP,                                     /* bVoltageSupport:  bVoltageSupport: 5v, 3v and 1.8v */
  LOBYTE(USBD_CCID_PROTOCOL),                            /* dwProtocols: supports T=0 and T=1 */
  HIBYTE(USBD_CCID_PROTOCOL),
  0x00,
  0x00,
  LOBYTE(USBD_CCID_DEFAULT_CLOCK_FREQ),                  /* dwDefaultClock: 3.6Mhz */
  HIBYTE(USBD_CCID_DEFAULT_CLOCK_FREQ),
  0x00,
  0x00,
  LOBYTE(USBD_CCID_MAX_CLOCK_FREQ),                      /* dwMaximumClock */
  HIBYTE(USBD_CCID_MAX_CLOCK_FREQ),
  0x00,
  0x00,
  0x00,                                                  /* bNumClockSupported */
  LOBYTE(USBD_CCID_DEFAULT_DATA_RATE),                   /* dwDataRate: 9677 bps */
  HIBYTE(USBD_CCID_DEFAULT_DATA_RATE),
  0x00,
  0x00,

  LOBYTE(USBD_CCID_MAX_DATA_RATE),                       /* dwMaxDataRate */
  HIBYTE(USBD_CCID_MAX_DATA_RATE),
  0x00,
  0x00,
  0x35,                                                  /* bNumDataRatesSupported */

  LOBYTE(USBD_CCID_MAX_INF_FIELD_SIZE),                  /* dwMaxIFSD: maximum IFSD supported for T=1 */
  HIBYTE(USBD_CCID_MAX_INF_FIELD_SIZE),
  0x00,
  0x00,
  0x00, 0x00, 0x00, 0x00,                                /* dwSynchProtocols */
  0x00, 0x00, 0x00, 0x00,                                /* dwMechanical: no special characteristics */

  0xBA, 0x04, EXCHANGE_LEVEL_FEATURE, 0x00,              /* dwFeatures */
  LOBYTE(CCID_MAX_BLOCK_SIZE_HEADER),                    /* dwMaxCCIDMessageLength: Maximum block size + header*/
  HIBYTE(CCID_MAX_BLOCK_SIZE_HEADER),
  0x00,
  0x00,
  0x00,                                                  /* bClassGetResponse*/
  0x00,                                                  /* bClassEnvelope */
  0x00, 0x00,                                            /* wLcdLayout : 0000h no LCD. */
  0x03,                                                  /* bPINSupport : PIN verification and PIN modification */
  0x01,                                                  /* bMaxCCIDBusySlots  */

  /********************  CCID   Endpoints ********************/
  CCID_ENDPOINT_DESC_SIZE,                               /* Endpoint descriptor length = 7 */
  USB_DESC_TYPE_ENDPOINT,                                /* Endpoint descriptor type */
  CCID_IN_EP,                                            /* Endpoint address (IN, address 1) */
  USBD_EP_TYPE_BULK,                                     /* Bulk endpoint type */

  LOBYTE(CCID_DATA_FS_MAX_PACKET_SIZE),
  HIBYTE(CCID_DATA_FS_MAX_PACKET_SIZE),
  0x00,                                                  /* Polling interval in milliseconds */
  CCID_ENDPOINT_DESC_SIZE,                               /* Endpoint descriptor length = 7 */
  USB_DESC_TYPE_ENDPOINT,                                /* Endpoint descriptor type */
  CCID_OUT_EP,                                           /* Endpoint address (OUT, address 1) */
  USBD_EP_TYPE_BULK,                                     /* Bulk endpoint type */

  LOBYTE(CCID_DATA_FS_MAX_PACKET_SIZE),
  HIBYTE(CCID_DATA_FS_MAX_PACKET_SIZE),
  0x00,                                                  /* Polling interval in milliseconds */
  CCID_ENDPOINT_DESC_SIZE,                               /* bLength: Endpoint Descriptor size */
  USB_DESC_TYPE_ENDPOINT,                                /* bDescriptorType:*/
  CCID_CMD_EP,                                           /* bEndpointAddress: Endpoint Address (IN) */
  USBD_EP_TYPE_INTR,                                     /* bmAttributes: Interrupt endpoint */
  LOBYTE(CCID_CMD_PACKET_SIZE),
  HIBYTE(CCID_CMD_PACKET_SIZE),
  CCID_CMD_FS_BINTERVAL                                  /* Polling interval in milliseconds */
};

/* USB Standard Device Descriptor */
__ALIGN_BEGIN static uint8_t USBD_CCID_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] __ALIGN_END =
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

/**
  * @}
  */

/** @defgroup USBD_CCID_Private_Functions
  * @{
  */

/**
  * @brief  USBD_CCID_Init
  *         Initialize the CCID interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_CCID_Init(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  USBD_CCID_HandleTypeDef  *hccid;
  UNUSED(cfgidx);

  /* Allocate CCID structure */
  hccid = (USBD_CCID_HandleTypeDef *)USBD_malloc(sizeof(USBD_CCID_HandleTypeDef));

  if (hccid == NULL)
  {
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    return (uint8_t)USBD_EMEM;
  }

  pdev->pClassDataCmsit[pdev->classId] = (void *)hccid;
  pdev->pClassData = pdev->pClassDataCmsit[pdev->classId];

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  CCIDInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  CCIDOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  CCIDCmdEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  /* Init the CCID parameters into a state where it can receive a new command message */
  hccid->USBD_CCID_Param.bAbortRequestFlag = 0U;
  hccid->USBD_CCID_Param.bSeq = 0U;
  hccid->USBD_CCID_Param.bSlot = 0U;
  hccid->MaxPcktLen = (pdev->dev_speed == USBD_SPEED_HIGH) ? \
                      CCID_DATA_HS_MAX_PACKET_SIZE : CCID_DATA_FS_MAX_PACKET_SIZE;

  /* Open EP IN */
  (void)USBD_LL_OpenEP(pdev, CCIDInEpAdd, USBD_EP_TYPE_BULK, (uint16_t)hccid->MaxPcktLen);
  pdev->ep_in[CCIDInEpAdd & 0xFU].is_used = 1U;

  /* Open EP OUT */
  (void)USBD_LL_OpenEP(pdev, CCIDOutEpAdd, USBD_EP_TYPE_BULK, (uint16_t)hccid->MaxPcktLen);
  pdev->ep_out[CCIDOutEpAdd & 0xFU].is_used = 1U;

  /* Open INTR EP IN */
  (void)USBD_LL_OpenEP(pdev, CCIDCmdEpAdd,
                       USBD_EP_TYPE_INTR, CCID_CMD_PACKET_SIZE);
  pdev->ep_in[CCIDCmdEpAdd & 0xFU].is_used = 1U;

  /* Init  physical Interface components */
  ((USBD_CCID_ItfTypeDef *)pdev->pUserData[pdev->classId])->Init(pdev);

  /* Prepare Out endpoint to receive next packet */
  (void)USBD_LL_PrepareReceive(pdev, CCIDOutEpAdd,
                               hccid->data, hccid->MaxPcktLen);

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CCID_DeInit
  *         DeInitialize the CCID layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t USBD_CCID_DeInit(USBD_HandleTypeDef *pdev, uint8_t cfgidx)
{
  UNUSED(cfgidx);

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  CCIDInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  CCIDOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  CCIDCmdEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  /* Close EP IN */
  (void)USBD_LL_CloseEP(pdev, CCIDInEpAdd);
  pdev->ep_in[CCIDInEpAdd & 0xFU].is_used = 0U;

  /* Close EP OUT */
  (void)USBD_LL_CloseEP(pdev, CCIDOutEpAdd);
  pdev->ep_out[CCIDOutEpAdd & 0xFU].is_used = 0U;

  /* Close EP Command */
  (void)USBD_LL_CloseEP(pdev, CCIDCmdEpAdd);
  pdev->ep_in[CCIDCmdEpAdd & 0xFU].is_used = 0U;

  /* DeInit  physical Interface components */
  if (pdev->pClassDataCmsit[pdev->classId] != NULL)
  {
    ((USBD_CCID_ItfTypeDef *)pdev->pUserData[pdev->classId])->DeInit(pdev);
    (void)USBD_free(pdev->pClassDataCmsit[pdev->classId]);
    pdev->pClassDataCmsit[pdev->classId] = NULL;
    pdev->pClassData = NULL;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CCID_Setup
  *         Handle the CCID specific requests
  * @param  pdev: instance
  * @param  req: usb requests
  * @retval status
  */
static uint8_t USBD_CCID_Setup(USBD_HandleTypeDef *pdev, USBD_SetupReqTypedef *req)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  USBD_CCID_ItfTypeDef     *hCCIDitf = (USBD_CCID_ItfTypeDef *)pdev->pUserData[pdev->classId];
  USBD_StatusTypeDef ret = USBD_OK;
  uint8_t ifalt = 0U;
  uint16_t status_info = 0U;
  uint16_t len;

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
    /* Class request */
    case USB_REQ_TYPE_CLASS :
      if (req->wLength != 0U)
      {
        len = MIN(CCID_EP0_BUFF_SIZ, req->wLength);
        if ((req->bmRequest & 0x80U) != 0U)
        {
          hCCIDitf->Control(req->bRequest, hccid->data, &len);
          (void)USBD_CtlSendData(pdev, hccid->data, len);
        }
        else
        {
          (void)USBD_CtlPrepareRx(pdev, hccid->data, len);
        }
      }
      else
      {
        len = 0U;
        hCCIDitf->Control(req->bRequest, (uint8_t *)&req->wValue, &len);
      }
      break;

    /* Interface & Endpoint request */
    case USB_REQ_TYPE_STANDARD:
      switch (req->bRequest)
      {
        case USB_REQ_GET_STATUS:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, (uint8_t *)&status_info, 2U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_GET_INTERFACE:
          if (pdev->dev_state == USBD_STATE_CONFIGURED)
          {
            (void)USBD_CtlSendData(pdev, &ifalt, 1U);
          }
          else
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_SET_INTERFACE:
          if (pdev->dev_state != USBD_STATE_CONFIGURED)
          {
            USBD_CtlError(pdev, req);
            ret = USBD_FAIL;
          }
          break;

        case USB_REQ_CLEAR_FEATURE:
          break;

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
  * @brief  USBD_CCID_DataIn
  *         Data sent on non-control IN endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_CCID_DataIn(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_CCID_HandleTypeDef *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  CCIDInEpAdd  = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
  CCIDCmdEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  if (epnum == (CCIDInEpAdd & 0x7FU))
  {
    /* Filter the epnum by masking with 0x7f (mask of IN Direction)  */

    /*************** Handle Bulk Transfer IN data completion  *****************/

    switch (hccid->blkt_state)
    {
      case CCID_STATE_SEND_RESP:

        /* won't wait ack to avoid missing a command */
        hccid->blkt_state = CCID_STATE_IDLE;

        /* Prepare EP to Receive  Cmd */
        (void)USBD_LL_PrepareReceive(pdev, CCID_OUT_EP,
                                     hccid->data, hccid->MaxPcktLen);
        break;

      default:
        break;
    }
  }
  else if (epnum == (CCIDCmdEpAdd & 0x7FU))
  {
    /* Filter the epnum by masking with 0x7f (mask of IN Direction)  */

    /*************** Handle Interrupt Transfer IN data completion  *****************/

    (void)USBD_CCID_IntMessage(pdev);
  }
  else
  {
    return (uint8_t)USBD_FAIL;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CCID_DataOut
  *         Data received on non-control Out endpoint
  * @param  pdev: device instance
  * @param  epnum: endpoint number
  * @retval status
  */
static uint8_t USBD_CCID_DataOut(USBD_HandleTypeDef *pdev, uint8_t epnum)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint16_t CurrPcktLen;

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  CCIDOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  if (hccid == NULL)
  {
    return (uint8_t)USBD_EMEM;
  }

  if (epnum == CCIDOutEpAdd)
  {
    CurrPcktLen = (uint16_t)USBD_GetRxCount(pdev, epnum);

    switch (hccid->blkt_state)
    {
      case CCID_STATE_IDLE:

        if (CurrPcktLen >= (uint16_t)CCID_CMD_HEADER_SIZE)
        {
          hccid->UsbMessageLength = CurrPcktLen; /* Store for future use */

          /* Fill CCID_BulkOut Data Buffer from USB Buffer */
          (void)USBD_CCID_ReceiveCmdHeader(pdev, (uint8_t *)&hccid->UsbBlkOutData.bMessageType,
                                           (uint16_t)CurrPcktLen);

          /*
          Refer : 6 CCID Messages
          The response messages always contain the exact same slot number,
          and sequence number fields from the header that was contained in
          the Bulk-OUT command message.
          */
          hccid->UsbBlkInData.bSlot = hccid->UsbBlkOutData.bSlot;
          hccid->UsbBlkInData.bSeq = hccid->UsbBlkOutData.bSeq;

          if (CurrPcktLen < hccid->MaxPcktLen)
          {
            /* Short message, less than the EP Out Size, execute the command,
             if parameter like dwLength is too big, the appropriate command will
             give an error */
            (void)USBD_CCID_DispatchCommand(pdev);
          }
          else
          {
            /* Check if length of data to be sent by host is > buffer size */
            if (hccid->UsbBlkOutData.dwLength > (uint32_t)ABDATA_SIZE)
            {
              /* Too long data received.... Error ! */
              hccid->blkt_state = CCID_STATE_UNCORRECT_LENGTH;
            }

            else
            {
              /* Expect more data on OUT EP */
              hccid->blkt_state = CCID_STATE_RECEIVE_DATA;

              /* Prepare EP to Receive next Cmd */
              (void)USBD_LL_PrepareReceive(pdev, CCID_OUT_EP,
                                           hccid->data, hccid->MaxPcktLen);

            } /* if (CurrPcktLen == CCID_DATA_MAX_PACKET_SIZE) ends */
          } /*  if (CurrPcktLen >= CCID_DATA_MAX_PACKET_SIZE) ends */
        } /* if (CurrPcktLen >= CCID_CMD_HEADER_SIZE) ends */
        else
        {
          if (CurrPcktLen == 0x00U) /* Zero Length Packet Received */
          {
            hccid->blkt_state = CCID_STATE_IDLE;
          }
        }

        break;

      case CCID_STATE_RECEIVE_DATA:
        hccid->UsbMessageLength += CurrPcktLen;

        if (CurrPcktLen < hccid->MaxPcktLen)
        {
          /* Short message, less than the EP Out Size, execute the command,
           if parameter like dwLength is too big, the appropriate command will
           give an error */

          /* Full command is received, process the Command */
          (void)USBD_CCID_ReceiveCmdHeader(pdev, (uint8_t *)&hccid->UsbBlkOutData.bMessageType,
                                           (uint16_t)CurrPcktLen);

          (void)USBD_CCID_DispatchCommand(pdev);
        }
        else if (CurrPcktLen == hccid->MaxPcktLen)
        {
          if (hccid->UsbMessageLength < (hccid->UsbBlkOutData.dwLength + (uint32_t)CCID_CMD_HEADER_SIZE))
          {
            (void)USBD_CCID_ReceiveCmdHeader(pdev, (uint8_t *)&hccid->UsbBlkOutData.bMessageType,
                                             (uint16_t)CurrPcktLen); /* Copy data */

            /* Prepare EP to Receive next Cmd */
            (void)USBD_LL_PrepareReceive(pdev, CCID_OUT_EP,
                                         hccid->data, hccid->MaxPcktLen);
          }
          else if (hccid->UsbMessageLength == (hccid->UsbBlkOutData.dwLength + (uint32_t)CCID_CMD_HEADER_SIZE))
          {
            /* Full command is received, process the Command */
            (void)USBD_CCID_ReceiveCmdHeader(pdev, (uint8_t *)&hccid->UsbBlkOutData.bMessageType,
                                             (uint16_t)CurrPcktLen);

            (void)USBD_CCID_DispatchCommand(pdev);
          }
          else
          {
            /* Too long data received.... Error ! */
            hccid->blkt_state = CCID_STATE_UNCORRECT_LENGTH;
          }
        }
        else
        {
          /* Too long data received.... Error ! */
          hccid->blkt_state = CCID_STATE_UNCORRECT_LENGTH;
        }
        break;

      case CCID_STATE_UNCORRECT_LENGTH:
        hccid->blkt_state = CCID_STATE_IDLE;
        break;

      default:
        break;
    }
  }
  else
  {
    return (uint8_t)USBD_FAIL;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CCID_DispatchCommand
  *         Parse the commands and Process command
  * @param  pdev: device instance
  * @retval status value
  */
static uint8_t USBD_CCID_DispatchCommand(USBD_HandleTypeDef *pdev)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t errorCode;

  switch (hccid->UsbBlkOutData.bMessageType)
  {
    case PC_TO_RDR_ICCPOWERON:
      errorCode = PC_to_RDR_IccPowerOn(pdev);
      RDR_to_PC_DataBlock(errorCode, pdev);
      break;

    case PC_TO_RDR_ICCPOWEROFF:
      errorCode = PC_to_RDR_IccPowerOff(pdev);
      RDR_to_PC_SlotStatus(errorCode, pdev);
      break;

    case PC_TO_RDR_GETSLOTSTATUS:
      errorCode = PC_to_RDR_GetSlotStatus(pdev);
      RDR_to_PC_SlotStatus(errorCode, pdev);
      break;

    case PC_TO_RDR_XFRBLOCK:
      errorCode = PC_to_RDR_XfrBlock(pdev);
      RDR_to_PC_DataBlock(errorCode, pdev);
      break;

    case PC_TO_RDR_GETPARAMETERS:
      errorCode = PC_to_RDR_GetParameters(pdev);
      RDR_to_PC_Parameters(errorCode, pdev);
      break;

    case PC_TO_RDR_RESETPARAMETERS:
      errorCode = PC_to_RDR_ResetParameters(pdev);
      RDR_to_PC_Parameters(errorCode, pdev);
      break;

    case PC_TO_RDR_SETPARAMETERS:
      errorCode = PC_to_RDR_SetParameters(pdev);
      RDR_to_PC_Parameters(errorCode, pdev);
      break;

    case PC_TO_RDR_ESCAPE:
      errorCode = PC_to_RDR_Escape(pdev);
      RDR_to_PC_Escape(errorCode, pdev);
      break;

    case PC_TO_RDR_ICCCLOCK:
      errorCode = PC_to_RDR_IccClock(pdev);
      RDR_to_PC_SlotStatus(errorCode, pdev);
      break;

    case PC_TO_RDR_ABORT:
      errorCode = PC_to_RDR_Abort(pdev);
      RDR_to_PC_SlotStatus(errorCode, pdev);
      break;

    case PC_TO_RDR_T0APDU:
      errorCode = PC_TO_RDR_T0Apdu(pdev);
      RDR_to_PC_SlotStatus(errorCode, pdev);
      break;

    case PC_TO_RDR_MECHANICAL:
      errorCode = PC_TO_RDR_Mechanical(pdev);
      RDR_to_PC_SlotStatus(errorCode, pdev);
      break;

    case PC_TO_RDR_SETDATARATEANDCLOCKFREQUENCY:
      errorCode = PC_TO_RDR_SetDataRateAndClockFrequency(pdev);
      RDR_to_PC_DataRateAndClockFrequency(errorCode, pdev);
      break;

    case PC_TO_RDR_SECURE:
      errorCode = PC_TO_RDR_Secure(pdev);
      RDR_to_PC_DataBlock(errorCode, pdev);
      break;

    default:
      RDR_to_PC_SlotStatus(SLOTERROR_CMD_NOT_SUPPORTED, pdev);
      break;
  }
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CCID_Transfer_Data_Request
  *         Prepare the request response to be sent to the host
  * @param  pdev: device instance
  * @param  dataPointer: Pointer to the data buffer to send
  * @param  dataLen : number of bytes to send
  * @retval status value
  */
uint8_t USBD_CCID_Transfer_Data_Request(USBD_HandleTypeDef *pdev,
                                        uint8_t *dataPointer, uint16_t dataLen)
{
  USBD_CCID_HandleTypeDef  *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  USBD_CCID_ItfTypeDef     *hCCIDitf = (USBD_CCID_ItfTypeDef *)pdev->pUserData[pdev->classId];

  UNUSED(dataPointer);

  hccid->blkt_state = CCID_STATE_SEND_RESP;
  hccid->UsbMessageLength = (uint32_t)dataLen; /* Store for future use */

  /* use the header declared size packet must be well formed */
  hCCIDitf->Response_SendData(pdev, (uint8_t *)&hccid->UsbBlkInData,
                              (uint16_t)MIN(CCID_DATA_FS_MAX_PACKET_SIZE, hccid->UsbMessageLength));

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CCID_ReceiveCmdHeader
  *         Receive the Data from USB BulkOut Buffer to Pointer
  * @param  pdev: device instance
  * @param  pDst: destination address to copy the buffer
  * @param  u8length: length of data to copy
  * @retval status
  */
static uint8_t USBD_CCID_ReceiveCmdHeader(USBD_HandleTypeDef  *pdev,
                                          uint8_t *pDst, uint16_t u8length)
{
  USBD_CCID_HandleTypeDef *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t *pdst = pDst;
  uint32_t Counter;

  for (Counter = 0U; Counter < u8length; Counter++)
  {
    *pdst = hccid->data[Counter];
    pdst++;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_CCID_IntMessage
  *         Send the Interrupt-IN data to the host
  * @param  pdev: device instance
  * @retval None
  */
uint8_t USBD_CCID_IntMessage(USBD_HandleTypeDef  *pdev)
{
  USBD_CCID_HandleTypeDef *hccid = (USBD_CCID_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  CCIDCmdEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_INTR, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  /* Check if there is change in Smartcard Slot status */
  if (CCID_IsSlotStatusChange(pdev) != 0U)
  {
    /* Check Slot Status is changed. Card is Removed/Fitted  */
    RDR_to_PC_NotifySlotChange(pdev);

    /* Set the Slot status */
    ((USBD_CCID_ItfTypeDef *)pdev->pUserData[pdev->classId])->SetSlotStatus(pdev);

    (void)USBD_LL_Transmit(pdev, CCIDCmdEpAdd, hccid->UsbIntData, 2U);
  }
  else
  {
    /* Set the Slot status */
    ((USBD_CCID_ItfTypeDef *)pdev->pUserData[pdev->classId])->SetSlotStatus(pdev);
  }

  return (uint8_t)USBD_OK;
}

#ifndef USE_USBD_COMPOSITE
/**
  * @brief  USBD_CCID_GetHSCfgDesc
  *         Return configuration descriptor
  * @param  length pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CCID_GetHSCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpInDesc = USBD_GetEpDesc(USBD_CCID_CfgDesc, CCID_IN_EP);
  USBD_EpDescTypeDef *pEpOutDesc = USBD_GetEpDesc(USBD_CCID_CfgDesc, CCID_OUT_EP);
  USBD_EpDescTypeDef *pEpCmdDesc = USBD_GetEpDesc(USBD_CCID_CfgDesc, CCID_CMD_EP);

  if (pEpInDesc != NULL)
  {
    pEpInDesc->wMaxPacketSize = CCID_DATA_HS_MAX_PACKET_SIZE;
  }

  if (pEpOutDesc != NULL)
  {
    pEpOutDesc->wMaxPacketSize = CCID_DATA_HS_MAX_PACKET_SIZE;
  }

  if (pEpCmdDesc != NULL)
  {
    pEpCmdDesc->bInterval = CCID_CMD_HS_BINTERVAL;
  }

  *length = (uint16_t)sizeof(USBD_CCID_CfgDesc);
  return USBD_CCID_CfgDesc;
}
/**
  * @brief  USBD_CCID_GetFSCfgDesc
  *         Return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_CCID_GetFSCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpInDesc = USBD_GetEpDesc(USBD_CCID_CfgDesc, CCID_IN_EP);
  USBD_EpDescTypeDef *pEpOutDesc = USBD_GetEpDesc(USBD_CCID_CfgDesc, CCID_OUT_EP);
  USBD_EpDescTypeDef *pEpCmdDesc = USBD_GetEpDesc(USBD_CCID_CfgDesc, CCID_CMD_EP);

  if (pEpInDesc != NULL)
  {
    pEpInDesc->wMaxPacketSize = CCID_DATA_FS_MAX_PACKET_SIZE;
  }

  if (pEpOutDesc != NULL)
  {
    pEpOutDesc->wMaxPacketSize = CCID_DATA_FS_MAX_PACKET_SIZE;
  }

  if (pEpCmdDesc != NULL)
  {
    pEpCmdDesc->bInterval = CCID_CMD_FS_BINTERVAL;
  }

  *length = (uint16_t)sizeof(USBD_CCID_CfgDesc);
  return USBD_CCID_CfgDesc;
}

/**
  * @brief  USBD_CCID_GetOtherSpeedCfgDesc
  *         Return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_CCID_GetOtherSpeedCfgDesc(uint16_t *length)
{
  USBD_EpDescTypeDef *pEpInDesc = USBD_GetEpDesc(USBD_CCID_CfgDesc, CCID_IN_EP);
  USBD_EpDescTypeDef *pEpOutDesc = USBD_GetEpDesc(USBD_CCID_CfgDesc, CCID_OUT_EP);
  USBD_EpDescTypeDef *pEpCmdDesc = USBD_GetEpDesc(USBD_CCID_CfgDesc, CCID_CMD_EP);

  if (pEpInDesc != NULL)
  {
    pEpInDesc->wMaxPacketSize = CCID_DATA_FS_MAX_PACKET_SIZE;
  }

  if (pEpOutDesc != NULL)
  {
    pEpOutDesc->wMaxPacketSize = CCID_DATA_FS_MAX_PACKET_SIZE;
  }

  if (pEpCmdDesc != NULL)
  {
    pEpCmdDesc->bInterval = CCID_CMD_FS_BINTERVAL;
  }

  *length = (uint16_t)sizeof(USBD_CCID_CfgDesc);
  return USBD_CCID_CfgDesc;
}

/**
  * @brief  USBD_CCID_GetDeviceQualifierDescriptor
  *         return Device Qualifier descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t *USBD_CCID_GetDeviceQualifierDescriptor(uint16_t *length)
{
  *length = (uint16_t)(sizeof(USBD_CCID_DeviceQualifierDesc));
  return USBD_CCID_DeviceQualifierDesc;
}
#endif /* USE_USBD_COMPOSITE  */

/**
  * @brief  USBD_CCID_RegisterInterface
  * @param  pdev: device instance
  * @param  fops: CD  Interface callback
  * @retval status
  */
uint8_t USBD_CCID_RegisterInterface(USBD_HandleTypeDef *pdev,
                                    USBD_CCID_ItfTypeDef *fops)
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
