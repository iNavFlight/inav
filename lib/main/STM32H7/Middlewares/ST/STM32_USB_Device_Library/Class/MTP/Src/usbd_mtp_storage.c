/**
  ******************************************************************************
  * @file    usbd_mtp_storage.c
  * @author  MCD Application Team
  * @brief   This file provides all the transfer command functions for MTP
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

/* Includes ------------------------------------------------------------------*/
#include "usbd_mtp_storage.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
extern uint8_t MTPInEpAdd;
extern uint8_t MTPOutEpAdd;

/* Private variables ---------------------------------------------------------*/
static MTP_DataLengthTypeDef MTP_DataLength;
static MTP_READ_DATA_STATUS ReadDataStatus;

/* Private function prototypes -----------------------------------------------*/
static uint8_t USBD_MTP_STORAGE_DecodeOperations(USBD_HandleTypeDef  *pdev);
static uint8_t USBD_MTP_STORAGE_ReceiveContainer(USBD_HandleTypeDef  *pdev, uint32_t *pDst, uint32_t len);
static uint8_t USBD_MTP_STORAGE_SendData(USBD_HandleTypeDef  *pdev, uint8_t *buf, uint32_t len);

/* Private functions ---------------------------------------------------------*/
/**
  * @}
  */

/**
  * @brief  USBD_MTP_STORAGE_Init
  *         Initialize the MTP USB Layer
  * @param  pdev: device instance
  * @retval status value
  */
uint8_t USBD_MTP_STORAGE_Init(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  MTPOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  /* Initialize the HW layyer of the file system */
  (void)((USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId])->Init();

  /* Prepare EP to Receive First Operation */
  (void)USBD_LL_PrepareReceive(pdev, MTPOutEpAdd, (uint8_t *)&hmtp->rx_buff,
                               hmtp->MaxPcktLen);

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MTP_STORAGE_DeInit
  *         Uninitialize the MTP Machine
  * @param  pdev: device instance
  * @retval status value
  */
uint8_t USBD_MTP_STORAGE_DeInit(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  /* DeInit  physical Interface components */
  hmtp->MTP_SessionState = MTP_SESSION_NOT_OPENED;

  /* Stop low layer file system operations if any */
  USBD_MTP_STORAGE_Cancel(pdev, MTP_PHASE_IDLE);

  /* Free low layer file system resources */
  (void)((USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId])->DeInit();

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MTP_STORAGE_ReadData
  *         Read data from device objects and send it to the host
  * @param  pdev: device instance
  * @retval status value
  */
uint8_t USBD_MTP_STORAGE_ReadData(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint32_t *data_buff;

  /* Get the data buffer pointer from the low layer interface */
  data_buff = ((USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId])->ScratchBuff;

  switch (ReadDataStatus)
  {
    case READ_FIRST_DATA:
      /* Reset the data length */
      MTP_DataLength.temp_length = 0U;

      /* Perform the low layer read operation on the scratch buffer */
      (void)((USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId])->ReadData(hmtp->OperationsContainer.Param1,
                                                                              (uint8_t *)data_buff, &MTP_DataLength);

      /* Add the container header to the data buffer */
      (void)USBD_memcpy((uint8_t *)data_buff, (uint8_t *)&hmtp->GenericContainer, MTP_CONT_HEADER_SIZE);

      /* Start USB data transmission to the host */
      (void)USBD_MTP_STORAGE_SendData(pdev, (uint8_t *)data_buff,
                                      MTP_DataLength.readbytes + MTP_CONT_HEADER_SIZE);

      /* Check if this will be the last packet to send ? */
      if (MTP_DataLength.readbytes < ((uint32_t)hmtp->MaxPcktLen - MTP_CONT_HEADER_SIZE))
      {
        /* Move to response phase */
        hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      }
      else
      {
        /* Continue to the next packets sending */
        ReadDataStatus = READ_REST_OF_DATA;
      }
      break;

    case READ_REST_OF_DATA:
      /* Perform the low layer read operation on the scratch buffer */
      (void)((USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId])->ReadData(hmtp->OperationsContainer.Param1,
                                                                              (uint8_t *)data_buff, &MTP_DataLength);

      /* Check if more data need to be sent */
      if (MTP_DataLength.temp_length == MTP_DataLength.totallen)
      {
        /* Start USB data transmission to the host */
        (void)USBD_MTP_STORAGE_SendData(pdev, (uint8_t *)data_buff, MTP_DataLength.readbytes);

        /* Move to response phase */
        hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;

        /* Reset the stat machine */
        ReadDataStatus = READ_FIRST_DATA;
      }
      else
      {
        /* Start USB data transmission to the host */
        (void)USBD_MTP_STORAGE_SendData(pdev, (uint8_t *)data_buff, MTP_DataLength.readbytes);

        /* Keep the state machine into sending next packet of data */
        ReadDataStatus = READ_REST_OF_DATA;
      }
      break;

    default:
      break;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MTP_STORAGE_SendContainer
  *         Send generic container to the host
  * @param  pdev: device instance
  * @retval status value
  */
uint8_t USBD_MTP_STORAGE_SendContainer(USBD_HandleTypeDef  *pdev, MTP_CONTAINER_TYPE CONT_TYPE)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  switch (CONT_TYPE)
  {
    case DATA_TYPE:
      /* send header + data : hmtp->ResponseLength = header size + data size */
      (void)USBD_MTP_STORAGE_SendData(pdev, (uint8_t *)&hmtp->GenericContainer, hmtp->ResponseLength);
      break;
    case REP_TYPE:
      /* send header without data */
      hmtp->GenericContainer.code = (uint16_t)hmtp->ResponseCode;
      hmtp->ResponseLength = MTP_CONT_HEADER_SIZE;
      hmtp->GenericContainer.length = hmtp->ResponseLength;
      hmtp->GenericContainer.type = MTP_CONT_TYPE_RESPONSE;

      (void)USBD_MTP_STORAGE_SendData(pdev, (uint8_t *)&hmtp->GenericContainer, hmtp->ResponseLength);
      break;
    default:
      break;
  }
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MTP_STORAGE_ReceiveOpt
  *         Data length Packet Received from host
  * @param  pdev: device instance
  * @retval status value
  */
uint8_t USBD_MTP_STORAGE_ReceiveOpt(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint32_t *pMsgBuffer;
#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  MTPOutEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_OUT, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */
  MTP_DataLength.rx_length = USBD_GetRxCount(pdev, MTPOutEpAdd);

  switch (hmtp->RECEIVE_DATA_STATUS)
  {
    case RECEIVE_REST_OF_DATA:
      /* we don't need to do anything here because we receive only data without operation header*/
      break;

    case RECEIVE_FIRST_DATA:
      /* Expected Data Length Packet Received */
      pMsgBuffer = (uint32_t *) &hmtp->OperationsContainer;

      /* Fill hmtp->OperationsContainer Data Buffer from USB Buffer */
      (void)USBD_MTP_STORAGE_ReceiveContainer(pdev, pMsgBuffer, MTP_DataLength.rx_length);
      break;

    default:
      /* Expected Data Length Packet Received */
      pMsgBuffer = (uint32_t *) &hmtp->OperationsContainer;

      /* Fill hmtp->OperationsContainer Data Buffer from USB Buffer */
      (void)USBD_MTP_STORAGE_ReceiveContainer(pdev, pMsgBuffer, MTP_DataLength.rx_length);
      (void)USBD_MTP_STORAGE_DecodeOperations(pdev);
      break;

  }
  return (uint8_t)USBD_OK;
}


/**
  * @brief  USBD_MTP_STORAGE_ReceiveData
  *         Receive objects or object info from host
  * @param  pdev: device instance
  * @retval status value
  */
uint8_t USBD_MTP_STORAGE_ReceiveData(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  switch (hmtp->RECEIVE_DATA_STATUS)
  {
    case RECEIVE_COMMAND_DATA :
      if (hmtp->OperationsContainer.type == MTP_CONT_TYPE_COMMAND)
      {
        MTP_DataLength.temp_length = 0;
        MTP_DataLength.prv_len = 0;
        (void)USBD_MTP_STORAGE_DecodeOperations(pdev);

      }
      break;

    case RECEIVE_FIRST_DATA :
      if (hmtp->OperationsContainer.type == MTP_CONT_TYPE_DATA)
      {
        MTP_DataLength.totallen = hmtp->OperationsContainer.length;
        MTP_DataLength.temp_length = MTP_DataLength.rx_length;
        MTP_DataLength.rx_length = MTP_DataLength.temp_length - MTP_CONT_HEADER_SIZE;
        (void)USBD_MTP_STORAGE_DecodeOperations(pdev);

        if (MTP_DataLength.temp_length < hmtp->MaxPcktLen) /* we received all data, we don't need to go to next state */
        {
          hmtp->RECEIVE_DATA_STATUS = SEND_RESPONSE;
          (void)USBD_MTP_STORAGE_DecodeOperations(pdev);

          /* send response header after receiving all data successfully */
          (void)USBD_MTP_STORAGE_SendContainer(pdev, DATA_TYPE);
        }
      }

      break;

    case RECEIVE_REST_OF_DATA :
      MTP_DataLength.prv_len = MTP_DataLength.temp_length - MTP_CONT_HEADER_SIZE;
      (void)USBD_MTP_STORAGE_DecodeOperations(pdev);
      MTP_DataLength.temp_length = MTP_DataLength.temp_length + MTP_DataLength.rx_length;

      if (MTP_DataLength.temp_length == MTP_DataLength.totallen) /* we received all data*/
      {
        hmtp->RECEIVE_DATA_STATUS = SEND_RESPONSE;
        (void)USBD_MTP_STORAGE_DecodeOperations(pdev);

        /* send response header after receiving all data successfully  */
        (void)USBD_MTP_STORAGE_SendContainer(pdev, DATA_TYPE);
      }
      break;

    default :
      break;
  }

  return (uint8_t)USBD_OK;
}


/**
  * @brief  USBD_MTP_STORAGE_DecodeOperations
  *         Parse the operations and Process operations
  * @param  pdev: device instance
  * @retval status value
  */
static uint8_t USBD_MTP_STORAGE_DecodeOperations(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  switch (hmtp->OperationsContainer.code)
  {
    case MTP_OP_GET_DEVICE_INFO:
      USBD_MTP_OPT_GetDeviceInfo(pdev);
      hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      break;

    case MTP_OP_OPEN_SESSION:
      USBD_MTP_OPT_CreateObjectHandle(pdev);
      hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      break;

    case MTP_OP_GET_STORAGE_IDS:
      USBD_MTP_OPT_GetStorageIDS(pdev);
      hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      break;

    case MTP_OP_GET_STORAGE_INFO:
      USBD_MTP_OPT_GetStorageInfo(pdev);
      hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      break;

    case MTP_OP_GET_OBJECT_HANDLES:
      USBD_MTP_OPT_GetObjectHandle(pdev);
      hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      break;

    case MTP_OP_GET_OBJECT_INFO:
      USBD_MTP_OPT_GetObjectInfo(pdev);
      hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      break;

    case MTP_OP_GET_OBJECT_PROP_REFERENCES:
      USBD_MTP_OPT_GetObjectReferences(pdev);
      hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      break;

    case MTP_OP_GET_OBJECT_PROPS_SUPPORTED:
      USBD_MTP_OPT_GetObjectPropSupp(pdev);
      hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      break;

    case MTP_OP_GET_OBJECT_PROP_DESC:
      USBD_MTP_OPT_GetObjectPropDesc(pdev);
      hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      break;

    case MTP_OP_GET_OBJECT_PROPLIST:
      USBD_MTP_OPT_GetObjectPropList(pdev);
      hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      break;

    case MTP_OP_GET_OBJECT_PROP_VALUE:
      USBD_MTP_OPT_GetObjectPropValue(pdev);
      hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      break;

    case MTP_OP_GET_DEVICE_PROP_DESC:
      USBD_MTP_OPT_GetDevicePropDesc(pdev);
      hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      break;

    case MTP_OP_GET_OBJECT:
      USBD_MTP_OPT_GetObject(pdev);
      hmtp->MTP_ResponsePhase = MTP_READ_DATA;
      break;

    case MTP_OP_SEND_OBJECT_INFO:
      USBD_MTP_OPT_SendObjectInfo(pdev, (uint8_t *)(hmtp->rx_buff), MTP_DataLength.prv_len);
      hmtp->MTP_ResponsePhase = MTP_RECEIVE_DATA;
      break;

    case MTP_OP_SEND_OBJECT:
      USBD_MTP_OPT_SendObject(pdev, (uint8_t *)(hmtp->rx_buff), MTP_DataLength.rx_length);
      hmtp->MTP_ResponsePhase = MTP_RECEIVE_DATA;
      break;

    case MTP_OP_DELETE_OBJECT:
      USBD_MTP_OPT_DeleteObject(pdev);
      hmtp->MTP_ResponsePhase = MTP_RESPONSE_PHASE;
      break;

    default:
      break;
  }

  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MTP_STORAGE_ReceiveContainer
  *         Receive the Data from USB BulkOut Buffer to Pointer
  * @param  pdev: device instance
  * @param  pDst: destination address to copy the buffer
  * @param  len: length of data to copy
  * @retval status value
  */
static uint8_t USBD_MTP_STORAGE_ReceiveContainer(USBD_HandleTypeDef  *pdev,
                                                 uint32_t *pDst, uint32_t len)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint32_t Counter;
  uint32_t *pdst = pDst;

  for (Counter = 0; Counter < len; Counter++)
  {
    *pdst = (hmtp->rx_buff[Counter]);
    pdst++;
  }
  return (uint8_t)USBD_OK;
}

/**
  * @brief  USBD_MTP_STORAGE_Cancel
  *         Reinitialize all states and cancel transfer through Bulk transfer
  * @param  pdev: device instance
  * @param  MTP_ResponsePhase: MTP current state
  * @retval None
  */
void USBD_MTP_STORAGE_Cancel(USBD_HandleTypeDef  *pdev,
                             MTP_ResponsePhaseTypeDef MTP_ResponsePhase)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  hmtp->MTP_ResponsePhase = MTP_PHASE_IDLE;
  ReadDataStatus = READ_FIRST_DATA;
  hmtp->RECEIVE_DATA_STATUS = RECEIVE_IDLE_STATE;

  if (MTP_ResponsePhase == MTP_RECEIVE_DATA)
  {
    ((USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId])->Cancel(1U);
  }
  else
  {
    ((USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId])->Cancel(0U);
  }
}

/**
  * @brief  USBD_MTP_STORAGE_SendData
  *         Send the data on bulk-in EP
  * @param  pdev: device instance
  * @param  buf: pointer to data buffer
  * @param  len: Data Length
  * @retval status value
  */
static uint8_t USBD_MTP_STORAGE_SendData(USBD_HandleTypeDef  *pdev, uint8_t *buf,
                                         uint32_t len)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint32_t length = MIN(hmtp->GenericContainer.length, len);
#ifdef USE_USBD_COMPOSITE
  /* Get the Endpoints addresses allocated for this class instance */
  MTPInEpAdd = USBD_CoreGetEPAdd(pdev, USBD_EP_IN, USBD_EP_TYPE_BULK, (uint8_t)pdev->classId);
#endif /* USE_USBD_COMPOSITE */

  (void)USBD_LL_Transmit(pdev, MTPInEpAdd, buf, length);

  return (uint8_t)USBD_OK;
}
