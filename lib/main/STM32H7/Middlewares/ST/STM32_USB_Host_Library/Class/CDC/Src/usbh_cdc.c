/**
  ******************************************************************************
  * @file    usbh_cdc.c
  * @author  MCD Application Team
  * @brief   This file is the CDC Layer Handlers for USB Host CDC class.
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
  *  @verbatim
  *
  *          ===================================================================
  *                                CDC Class Driver Description
  *          ===================================================================
  *           This driver manages the "Universal Serial Bus Class Definitions for Communications Devices
  *           Revision 1.2 November 16, 2007" and the sub-protocol specification of "Universal Serial Bus
  *           Communications Class Subclass Specification for PSTN Devices Revision 1.2 February 9, 2007"
  *           This driver implements the following aspects of the specification:
  *             - Device descriptor management
  *             - Configuration descriptor management
  *             - Enumeration as CDC device with 2 data endpoints (IN and OUT) and 1 command endpoint (IN)
  *             - Requests management (as described in section 6.2 in specification)
  *             - Abstract Control Model compliant
  *             - Union Functional collection (using 1 IN endpoint for control)
  *             - Data interface class
  *
  *  @endverbatim
  *
  ******************************************************************************
  */

/* BSPDependencies
- "stm32xxxxx_{eval}{discovery}{nucleo_144}.c"
- "stm32xxxxx_{eval}{discovery}_io.c"
- "stm32xxxxx_{eval}{discovery}{adafruit}_sd.c"
- "stm32xxxxx_{eval}{discovery}{adafruit}_lcd.c"
- "stm32xxxxx_{eval}{discovery}_sdram.c"
EndBSPDependencies */

/* Includes ------------------------------------------------------------------*/
#include "usbh_cdc.h"

/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_CDC_CLASS
  * @{
  */

/** @defgroup USBH_CDC_CORE
  * @brief    This file includes CDC Layer Handlers for USB Host CDC class.
  * @{
  */

/** @defgroup USBH_CDC_CORE_Private_TypesDefinitions
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_CDC_CORE_Private_Defines
  * @{
  */
#define USBH_CDC_BUFFER_SIZE                 1024
/**
  * @}
  */


/** @defgroup USBH_CDC_CORE_Private_Macros
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_CDC_CORE_Private_Variables
  * @{
  */
/**
  * @}
  */


/** @defgroup USBH_CDC_CORE_Private_FunctionPrototypes
  * @{
  */

static USBH_StatusTypeDef USBH_CDC_InterfaceInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_CDC_InterfaceDeInit(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_CDC_Process(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_CDC_SOFProcess(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef USBH_CDC_ClassRequest(USBH_HandleTypeDef *phost);

static USBH_StatusTypeDef GetLineCoding(USBH_HandleTypeDef *phost,
                                        CDC_LineCodingTypeDef *linecoding);

static USBH_StatusTypeDef SetLineCoding(USBH_HandleTypeDef *phost,
                                        CDC_LineCodingTypeDef *linecoding);

static void CDC_ProcessTransmission(USBH_HandleTypeDef *phost);

static void CDC_ProcessReception(USBH_HandleTypeDef *phost);

USBH_ClassTypeDef  CDC_Class =
{
  "CDC",
  USB_CDC_CLASS,
  USBH_CDC_InterfaceInit,
  USBH_CDC_InterfaceDeInit,
  USBH_CDC_ClassRequest,
  USBH_CDC_Process,
  USBH_CDC_SOFProcess,
  NULL,
};
/**
  * @}
  */


/** @defgroup USBH_CDC_CORE_Private_Functions
  * @{
  */

/**
  * @brief  USBH_CDC_InterfaceInit
  *         The function init the CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_CDC_InterfaceInit(USBH_HandleTypeDef *phost)
{

  USBH_StatusTypeDef status;
  uint8_t interface;
  CDC_HandleTypeDef *CDC_Handle;

  interface = USBH_FindInterface(phost, COMMUNICATION_INTERFACE_CLASS_CODE,
                                   ABSTRACT_CONTROL_MODEL, COMMON_AT_COMMAND);

  if ((interface == 0xFFU) || (interface >= USBH_MAX_NUM_INTERFACES)) /* No Valid Interface */
  {
    USBH_DbgLog("Cannot Find the interface for Communication Interface Class.", phost->pActiveClass->Name);
    return USBH_FAIL;
  }

  status = USBH_SelectInterface(phost, interface);

  if (status != USBH_OK)
  {
    return USBH_FAIL;
  }

  phost->pActiveClass->pData = (CDC_HandleTypeDef *)USBH_malloc(sizeof(CDC_HandleTypeDef));
  CDC_Handle = (CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (CDC_Handle == NULL)
  {
    USBH_DbgLog("Cannot allocate memory for CDC Handle");
    return USBH_FAIL;
  }

  /* Initialize cdc handler */
  (void)USBH_memset(CDC_Handle, 0, sizeof(CDC_HandleTypeDef));

  /*Collect the notification endpoint address and length*/
  if ((phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U) != 0U)
  {
    CDC_Handle->CommItf.NotifEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
    CDC_Handle->CommItf.NotifEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
  }

  /*Allocate the length for host channel number in*/
  CDC_Handle->CommItf.NotifPipe = USBH_AllocPipe(phost, CDC_Handle->CommItf.NotifEp);

  /* Open pipe for Notification endpoint */
  (void)USBH_OpenPipe(phost, CDC_Handle->CommItf.NotifPipe, CDC_Handle->CommItf.NotifEp,
                      phost->device.address, phost->device.speed, USB_EP_TYPE_INTR,
                      CDC_Handle->CommItf.NotifEpSize);

  (void)USBH_LL_SetToggle(phost, CDC_Handle->CommItf.NotifPipe, 0U);

  interface = USBH_FindInterface(phost, DATA_INTERFACE_CLASS_CODE,
                                   RESERVED, NO_CLASS_SPECIFIC_PROTOCOL_CODE);

  if ((interface == 0xFFU) || (interface >= USBH_MAX_NUM_INTERFACES)) /* No Valid Interface */
  {
    USBH_DbgLog("Cannot Find the interface for Data Interface Class.", phost->pActiveClass->Name);
    return USBH_FAIL;
  }

  /*Collect the class specific endpoint address and length*/
  if ((phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress & 0x80U) != 0U)
  {
    CDC_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
    CDC_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
  }
  else
  {
    CDC_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress;
    CDC_Handle->DataItf.OutEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize;
  }

  if ((phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress & 0x80U) != 0U)
  {
    CDC_Handle->DataItf.InEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
    CDC_Handle->DataItf.InEpSize  = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
  }
  else
  {
    CDC_Handle->DataItf.OutEp = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].bEndpointAddress;
    CDC_Handle->DataItf.OutEpSize = phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[1].wMaxPacketSize;
  }

  /*Allocate the length for host channel number out*/
  CDC_Handle->DataItf.OutPipe = USBH_AllocPipe(phost, CDC_Handle->DataItf.OutEp);

  /*Allocate the length for host channel number in*/
  CDC_Handle->DataItf.InPipe = USBH_AllocPipe(phost, CDC_Handle->DataItf.InEp);

  /* Open channel for OUT endpoint */
  (void)USBH_OpenPipe(phost, CDC_Handle->DataItf.OutPipe, CDC_Handle->DataItf.OutEp,
                      phost->device.address, phost->device.speed, USB_EP_TYPE_BULK,
                      CDC_Handle->DataItf.OutEpSize);

  /* Open channel for IN endpoint */
  (void)USBH_OpenPipe(phost, CDC_Handle->DataItf.InPipe, CDC_Handle->DataItf.InEp,
                      phost->device.address, phost->device.speed, USB_EP_TYPE_BULK,
                      CDC_Handle->DataItf.InEpSize);

  CDC_Handle->state = CDC_IDLE_STATE;

  (void)USBH_LL_SetToggle(phost, CDC_Handle->DataItf.OutPipe, 0U);
  (void)USBH_LL_SetToggle(phost, CDC_Handle->DataItf.InPipe, 0U);

  return USBH_OK;
}



/**
  * @brief  USBH_CDC_InterfaceDeInit
  *         The function DeInit the Pipes used for the CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_CDC_InterfaceDeInit(USBH_HandleTypeDef *phost)
{
  CDC_HandleTypeDef *CDC_Handle = (CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if ((CDC_Handle->CommItf.NotifPipe) != 0U)
  {
    (void)USBH_ClosePipe(phost, CDC_Handle->CommItf.NotifPipe);
    (void)USBH_FreePipe(phost, CDC_Handle->CommItf.NotifPipe);
    CDC_Handle->CommItf.NotifPipe = 0U;     /* Reset the Channel as Free */
  }

  if ((CDC_Handle->DataItf.InPipe) != 0U)
  {
    (void)USBH_ClosePipe(phost, CDC_Handle->DataItf.InPipe);
    (void)USBH_FreePipe(phost, CDC_Handle->DataItf.InPipe);
    CDC_Handle->DataItf.InPipe = 0U;     /* Reset the Channel as Free */
  }

  if ((CDC_Handle->DataItf.OutPipe) != 0U)
  {
    (void)USBH_ClosePipe(phost, CDC_Handle->DataItf.OutPipe);
    (void)USBH_FreePipe(phost, CDC_Handle->DataItf.OutPipe);
    CDC_Handle->DataItf.OutPipe = 0U;    /* Reset the Channel as Free */
  }

  if ((phost->pActiveClass->pData) != NULL)
  {
    USBH_free(phost->pActiveClass->pData);
    phost->pActiveClass->pData = 0U;
  }

  return USBH_OK;
}

/**
  * @brief  USBH_CDC_ClassRequest
  *         The function is responsible for handling Standard requests
  *         for CDC class.
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_CDC_ClassRequest(USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef status;
  CDC_HandleTypeDef *CDC_Handle = (CDC_HandleTypeDef *) phost->pActiveClass->pData;

  /* Issue the get line coding request */
  status = GetLineCoding(phost, &CDC_Handle->LineCoding);
  if (status == USBH_OK)
  {
    phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
  }
  else if (status == USBH_NOT_SUPPORTED)
  {
    USBH_ErrLog("Control error: CDC: Device Get Line Coding configuration failed");
  }
  else
  {
    /* .. */
  }

  return status;
}


/**
  * @brief  USBH_CDC_Process
  *         The function is for managing state machine for CDC data transfers
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_CDC_Process(USBH_HandleTypeDef *phost)
{
  USBH_StatusTypeDef status = USBH_BUSY;
  USBH_StatusTypeDef req_status = USBH_OK;
  CDC_HandleTypeDef *CDC_Handle = (CDC_HandleTypeDef *) phost->pActiveClass->pData;

  switch (CDC_Handle->state)
  {

    case CDC_IDLE_STATE:
      status = USBH_OK;
      break;

    case CDC_SET_LINE_CODING_STATE:
      req_status = SetLineCoding(phost, CDC_Handle->pUserLineCoding);

      if (req_status == USBH_OK)
      {
        CDC_Handle->state = CDC_GET_LAST_LINE_CODING_STATE;
      }

      else
      {
        if (req_status != USBH_BUSY)
        {
          CDC_Handle->state = CDC_ERROR_STATE;
        }
      }
      break;


    case CDC_GET_LAST_LINE_CODING_STATE:
      req_status = GetLineCoding(phost, &(CDC_Handle->LineCoding));

      if (req_status == USBH_OK)
      {
        CDC_Handle->state = CDC_IDLE_STATE;

        if ((CDC_Handle->LineCoding.b.bCharFormat == CDC_Handle->pUserLineCoding->b.bCharFormat) &&
            (CDC_Handle->LineCoding.b.bDataBits == CDC_Handle->pUserLineCoding->b.bDataBits) &&
            (CDC_Handle->LineCoding.b.bParityType == CDC_Handle->pUserLineCoding->b.bParityType) &&
            (CDC_Handle->LineCoding.b.dwDTERate == CDC_Handle->pUserLineCoding->b.dwDTERate))
        {
          USBH_CDC_LineCodingChanged(phost);
        }
      }
      else
      {
        if (req_status != USBH_BUSY)
        {
          CDC_Handle->state = CDC_ERROR_STATE;
        }
      }
      break;

    case CDC_TRANSFER_DATA:
      CDC_ProcessTransmission(phost);
      CDC_ProcessReception(phost);
      break;

    case CDC_ERROR_STATE:
      req_status = USBH_ClrFeature(phost, 0x00U);

      if (req_status == USBH_OK)
      {
        /*Change the state to waiting*/
        CDC_Handle->state = CDC_IDLE_STATE;
      }
      break;

    default:
      break;

  }

  return status;
}

/**
  * @brief  USBH_CDC_SOFProcess
  *         The function is for managing SOF callback
  * @param  phost: Host handle
  * @retval USBH Status
  */
static USBH_StatusTypeDef USBH_CDC_SOFProcess(USBH_HandleTypeDef *phost)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(phost);

  return USBH_OK;
}


/**
  * @brief  USBH_CDC_Stop
  *         Stop current CDC Transmission
  * @param  phost: Host handle
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_CDC_Stop(USBH_HandleTypeDef *phost)
{
  CDC_HandleTypeDef *CDC_Handle = (CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (phost->gState == HOST_CLASS)
  {
    CDC_Handle->state = CDC_IDLE_STATE;

    (void)USBH_ClosePipe(phost, CDC_Handle->CommItf.NotifPipe);
    (void)USBH_ClosePipe(phost, CDC_Handle->DataItf.InPipe);
    (void)USBH_ClosePipe(phost, CDC_Handle->DataItf.OutPipe);
  }
  return USBH_OK;
}
/**
  * @brief  This request allows the host to find out the currently
  *         configured line coding.
  * @param  pdev: Selected device
  * @retval USBH_StatusTypeDef : USB ctl xfer status
  */
static USBH_StatusTypeDef GetLineCoding(USBH_HandleTypeDef *phost, CDC_LineCodingTypeDef *linecoding)
{

  phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_TYPE_CLASS | \
                                         USB_REQ_RECIPIENT_INTERFACE;

  phost->Control.setup.b.bRequest = CDC_GET_LINE_CODING;
  phost->Control.setup.b.wValue.w = 0U;
  phost->Control.setup.b.wIndex.w = 0U;
  phost->Control.setup.b.wLength.w = LINE_CODING_STRUCTURE_SIZE;

  return USBH_CtlReq(phost, linecoding->Array, LINE_CODING_STRUCTURE_SIZE);
}


/**
  * @brief  This request allows the host to specify typical asynchronous
  * line-character formatting properties
  * This request applies to asynchronous byte stream data class interfaces
  * and endpoints
  * @param  pdev: Selected device
  * @retval USBH_StatusTypeDef : USB ctl xfer status
  */
static USBH_StatusTypeDef SetLineCoding(USBH_HandleTypeDef *phost,
                                        CDC_LineCodingTypeDef *linecoding)
{
  phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_TYPE_CLASS |
                                         USB_REQ_RECIPIENT_INTERFACE;

  phost->Control.setup.b.bRequest = CDC_SET_LINE_CODING;
  phost->Control.setup.b.wValue.w = 0U;

  phost->Control.setup.b.wIndex.w = 0U;

  phost->Control.setup.b.wLength.w = LINE_CODING_STRUCTURE_SIZE;

  return USBH_CtlReq(phost, linecoding->Array, LINE_CODING_STRUCTURE_SIZE);
}

/**
  * @brief  This function prepares the state before issuing the class specific commands
  * @param  None
  * @retval None
  */
USBH_StatusTypeDef USBH_CDC_SetLineCoding(USBH_HandleTypeDef *phost,
                                          CDC_LineCodingTypeDef *linecoding)
{
  CDC_HandleTypeDef *CDC_Handle = (CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (phost->gState == HOST_CLASS)
  {
    CDC_Handle->state = CDC_SET_LINE_CODING_STATE;
    CDC_Handle->pUserLineCoding = linecoding;

#if (USBH_USE_OS == 1U)
    phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
    (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
    (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
  }

  return USBH_OK;
}

/**
  * @brief  This function prepares the state before issuing the class specific commands
  * @param  None
  * @retval None
  */
USBH_StatusTypeDef USBH_CDC_GetLineCoding(USBH_HandleTypeDef *phost,
                                          CDC_LineCodingTypeDef *linecoding)
{
  CDC_HandleTypeDef *CDC_Handle = (CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if ((phost->gState == HOST_CLASS) || (phost->gState == HOST_CLASS_REQUEST))
  {
    *linecoding = CDC_Handle->LineCoding;
    return USBH_OK;
  }
  else
  {
    return USBH_FAIL;
  }
}

/**
  * @brief  This function return last received data size
  * @param  None
  * @retval None
  */
uint16_t USBH_CDC_GetLastReceivedDataSize(USBH_HandleTypeDef *phost)
{
  uint32_t dataSize;
  CDC_HandleTypeDef *CDC_Handle = (CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if (phost->gState == HOST_CLASS)
  {
    dataSize = USBH_LL_GetLastXferSize(phost, CDC_Handle->DataItf.InPipe);
  }
  else
  {
    dataSize =  0U;
  }

  return (uint16_t)dataSize;
}

/**
  * @brief  This function prepares the state before issuing the class specific commands
  * @param  None
  * @retval None
  */
USBH_StatusTypeDef USBH_CDC_Transmit(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
  CDC_HandleTypeDef *CDC_Handle = (CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if ((CDC_Handle->state == CDC_IDLE_STATE) || (CDC_Handle->state == CDC_TRANSFER_DATA))
  {
    CDC_Handle->pTxData = pbuff;
    CDC_Handle->TxDataLength = length;
    CDC_Handle->state = CDC_TRANSFER_DATA;
    CDC_Handle->data_tx_state = CDC_SEND_DATA;
    Status = USBH_OK;

#if (USBH_USE_OS == 1U)
    phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
    (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
    (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
  }
  return Status;
}


/**
  * @brief  This function prepares the state before issuing the class specific commands
  * @param  None
  * @retval None
  */
USBH_StatusTypeDef USBH_CDC_Receive(USBH_HandleTypeDef *phost, uint8_t *pbuff, uint32_t length)
{
  USBH_StatusTypeDef Status = USBH_BUSY;
  CDC_HandleTypeDef *CDC_Handle = (CDC_HandleTypeDef *) phost->pActiveClass->pData;

  if ((CDC_Handle->state == CDC_IDLE_STATE) || (CDC_Handle->state == CDC_TRANSFER_DATA))
  {
    CDC_Handle->pRxData = pbuff;
    CDC_Handle->RxDataLength = length;
    CDC_Handle->state = CDC_TRANSFER_DATA;
    CDC_Handle->data_rx_state = CDC_RECEIVE_DATA;
    Status = USBH_OK;

#if (USBH_USE_OS == 1U)
    phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
    (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
    (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
  }
  return Status;
}

/**
  * @brief  The function is responsible for sending data to the device
  *  @param  pdev: Selected device
  * @retval None
  */
static void CDC_ProcessTransmission(USBH_HandleTypeDef *phost)
{
  CDC_HandleTypeDef *CDC_Handle = (CDC_HandleTypeDef *) phost->pActiveClass->pData;
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;

  switch (CDC_Handle->data_tx_state)
  {
    case CDC_SEND_DATA:
      if (CDC_Handle->TxDataLength > CDC_Handle->DataItf.OutEpSize)
      {
        (void)USBH_BulkSendData(phost,
                                CDC_Handle->pTxData,
                                CDC_Handle->DataItf.OutEpSize,
                                CDC_Handle->DataItf.OutPipe,
                                1U);
      }
      else
      {
        (void)USBH_BulkSendData(phost,
                                CDC_Handle->pTxData,
                                (uint16_t)CDC_Handle->TxDataLength,
                                CDC_Handle->DataItf.OutPipe,
                                1U);
      }

      CDC_Handle->data_tx_state = CDC_SEND_DATA_WAIT;
      break;

    case CDC_SEND_DATA_WAIT:

      URB_Status = USBH_LL_GetURBState(phost, CDC_Handle->DataItf.OutPipe);

      /* Check the status done for transmission */
      if (URB_Status == USBH_URB_DONE)
      {
        if (CDC_Handle->TxDataLength > CDC_Handle->DataItf.OutEpSize)
        {
          CDC_Handle->TxDataLength -= CDC_Handle->DataItf.OutEpSize;
          CDC_Handle->pTxData += CDC_Handle->DataItf.OutEpSize;
        }
        else
        {
          CDC_Handle->TxDataLength = 0U;
        }

        if (CDC_Handle->TxDataLength > 0U)
        {
          CDC_Handle->data_tx_state = CDC_SEND_DATA;
        }
        else
        {
          CDC_Handle->data_tx_state = CDC_IDLE;
          USBH_CDC_TransmitCallback(phost);
        }

#if (USBH_USE_OS == 1U)
        phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
        (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
        (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
      }
      else
      {
        if (URB_Status == USBH_URB_NOTREADY)
        {
          CDC_Handle->data_tx_state = CDC_SEND_DATA;

#if (USBH_USE_OS == 1U)
          phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
          (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
          (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
        }
      }
      break;

    default:
      break;
  }
}
/**
  * @brief  This function responsible for reception of data from the device
  *  @param  pdev: Selected device
  * @retval None
  */

static void CDC_ProcessReception(USBH_HandleTypeDef *phost)
{
  CDC_HandleTypeDef *CDC_Handle = (CDC_HandleTypeDef *) phost->pActiveClass->pData;
  USBH_URBStateTypeDef URB_Status = USBH_URB_IDLE;
  uint32_t length;

  switch (CDC_Handle->data_rx_state)
  {

    case CDC_RECEIVE_DATA:

      (void)USBH_BulkReceiveData(phost,
                                 CDC_Handle->pRxData,
                                 CDC_Handle->DataItf.InEpSize,
                                 CDC_Handle->DataItf.InPipe);

      CDC_Handle->data_rx_state = CDC_RECEIVE_DATA_WAIT;

      break;

    case CDC_RECEIVE_DATA_WAIT:

      URB_Status = USBH_LL_GetURBState(phost, CDC_Handle->DataItf.InPipe);

      /*Check the status done for reception*/
      if (URB_Status == USBH_URB_DONE)
      {
        length = USBH_LL_GetLastXferSize(phost, CDC_Handle->DataItf.InPipe);

        if (((CDC_Handle->RxDataLength - length) > 0U) && (length == CDC_Handle->DataItf.InEpSize))
        {
          CDC_Handle->RxDataLength -= length;
          CDC_Handle->pRxData += length;
          CDC_Handle->data_rx_state = CDC_RECEIVE_DATA;
        }
        else
        {
          CDC_Handle->data_rx_state = CDC_IDLE;
          USBH_CDC_ReceiveCallback(phost);
        }

#if (USBH_USE_OS == 1U)
        phost->os_msg = (uint32_t)USBH_CLASS_EVENT;
#if (osCMSIS < 0x20000U)
        (void)osMessagePut(phost->os_event, phost->os_msg, 0U);
#else
        (void)osMessageQueuePut(phost->os_event, &phost->os_msg, 0U, 0U);
#endif
#endif
      }
      break;

    default:
      break;
  }
}

/**
  * @brief  The function informs user that data have been received
  *  @param  pdev: Selected device
  * @retval None
  */
__weak void USBH_CDC_TransmitCallback(USBH_HandleTypeDef *phost)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(phost);
}

/**
  * @brief  The function informs user that data have been sent
  *  @param  pdev: Selected device
  * @retval None
  */
__weak void USBH_CDC_ReceiveCallback(USBH_HandleTypeDef *phost)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(phost);
}

/**
  * @brief  The function informs user that Settings have been changed
  *  @param  pdev: Selected device
  * @retval None
  */
__weak void USBH_CDC_LineCodingChanged(USBH_HandleTypeDef *phost)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(phost);
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

