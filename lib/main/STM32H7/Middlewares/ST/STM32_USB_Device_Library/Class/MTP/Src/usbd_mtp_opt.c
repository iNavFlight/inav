/**
  ******************************************************************************
  * @file    usbd_mtp_opt.c
  * @author  MCD Application Team
  * @brief   This file includes the PTP operations layer
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
#include "usbd_mtp_opt.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t  ObjInfo_buff[255];
static uint32_t objhandle;
static uint16_t obj_format;
static uint32_t storage_id;

static MTP_DeviceInfoTypedef     MTP_DeviceInfo;
static MTP_StorageIDSTypeDef     MTP_StorageIDS;
static MTP_StorageInfoTypedef    MTP_StorageInfo;
static MTP_ObjectHandleTypeDef   MTP_ObjectHandle;
static MTP_ObjectInfoTypeDef     MTP_ObjectInfo;
static MTP_ObjectPropSuppTypeDef MTP_ObjectPropSupp;
static MTP_ObjectPropDescTypeDef MTP_ObjectPropDesc;
static MTP_PropertiesListTypedef MTP_PropertiesList;
static MTP_RefTypeDef            MTP_Ref;
static MTP_PropertyValueTypedef  MTP_PropertyValue;
static MTP_FileNameTypeDef       MTP_FileName;
static MTP_DevicePropDescTypeDef MTP_DevicePropDesc;

/* Private function prototypes -----------------------------------------------*/
static void MTP_Get_DeviceInfo(void);
static void MTP_Get_StorageIDS(void);
static void MTP_Get_PayloadContent(USBD_HandleTypeDef *pdev);
static void MTP_Get_ObjectInfo(USBD_HandleTypeDef *pdev);
static void MTP_Get_StorageInfo(USBD_HandleTypeDef *pdev);
static void MTP_Get_ObjectHandle(USBD_HandleTypeDef *pdev);
static void MTP_Get_ObjectPropSupp(void);
static void MTP_Get_ObjectPropDesc(USBD_HandleTypeDef *pdev);
static void MTP_Get_ObjectPropList(USBD_HandleTypeDef *pdev);
static void MTP_Get_DevicePropDesc(void);
static uint8_t *MTP_Get_ObjectPropValue(USBD_HandleTypeDef *pdev);
static uint32_t MTP_build_data_propdesc(USBD_HandleTypeDef *pdev, MTP_ObjectPropDescTypeDef def);
static uint32_t MTP_build_data_ObjInfo(USBD_HandleTypeDef *pdev, MTP_ObjectInfoTypeDef objinfo);
static uint32_t MTP_build_data_proplist(USBD_HandleTypeDef *pdev,
                                        MTP_PropertiesListTypedef proplist, uint32_t idx);

/* Private functions ---------------------------------------------------------*/

/**
  * @}
  */


/**
  * @brief  USBD_MTP_OPT_CreateObjectHandle
  *         Open a new session
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_CreateObjectHandle(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (hmtp->OperationsContainer.Param1 == 0U)   /* Param1 == Session ID*/
  {
    hmtp->ResponseCode = MTP_RESPONSE_INVALID_PARAMETER;
  }
  /* driver supports single session */
  else if (hmtp->MTP_SessionState == MTP_SESSION_OPENED)
  {
    hmtp->ResponseCode = MTP_RESPONSE_SESSION_ALREADY_OPEN;
  }
  else
  {
    hmtp->ResponseCode =  MTP_RESPONSE_OK;
    hmtp->MTP_SessionState = MTP_SESSION_OPENED;
  }

  hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
  hmtp->GenericContainer.type = MTP_CONT_TYPE_RESPONSE;
  hmtp->ResponseLength = MTP_CONT_HEADER_SIZE;
  hmtp->GenericContainer.length =  hmtp->ResponseLength;
}

/**
  * @brief  USBD_MTP_OPT_GetDeviceInfo
  *         Get all device information
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_GetDeviceInfo(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  if (hmtp->MTP_SessionState == MTP_SESSION_NOT_OPENED)        /* no session opened */
  {
    /* if GetDevice Info called outside a session then SessionID and Transaction_ID shall be 0x00000000*/
    /* Param1 == session ID*/
    if ((hmtp->OperationsContainer.Param1 == 0U) && (hmtp->OperationsContainer.trans_id == 0U))
    {
      hmtp->ResponseCode = MTP_RESPONSE_OK;
    }
    else
    {
      hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
      hmtp->GenericContainer.type = MTP_CONT_TYPE_RESPONSE;
      hmtp->ResponseCode = MTP_RESPONSE_INVALID_PARAMETER;
      hmtp->GenericContainer.length =  MTP_CONT_HEADER_SIZE;
    }
  }
  else
  {
    hmtp->ResponseCode = MTP_RESPONSE_OK;
  }

  if (hmtp->ResponseCode == MTP_RESPONSE_OK)
  {
    hmtp->GenericContainer.code =  MTP_OP_GET_DEVICE_INFO;
    hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
    hmtp->GenericContainer.type = MTP_CONT_TYPE_DATA;

    (void)MTP_Get_PayloadContent(pdev);

    hmtp->ResponseLength = sizeof(MTP_DeviceInfo) + MTP_CONT_HEADER_SIZE;
    hmtp->GenericContainer.length =  hmtp->ResponseLength;
  }
}

/**
  * @brief  USBD_MTP_OPT_GetStorageIDS
  *         Get Storage IDs
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_GetStorageIDS(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  hmtp->GenericContainer.code =  MTP_OP_GET_STORAGE_IDS;
  hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
  hmtp->GenericContainer.type = MTP_CONT_TYPE_DATA;

  (void)MTP_Get_PayloadContent(pdev);

  hmtp->ResponseLength = sizeof(MTP_StorageIDS) + MTP_CONT_HEADER_SIZE;
  hmtp->GenericContainer.length =  hmtp->ResponseLength;

  hmtp->ResponseCode = MTP_RESPONSE_OK;
}

/**
  * @brief  USBD_MTP_OPT_GetStorageInfo
  *         Get Storage information
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_GetStorageInfo(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  hmtp->GenericContainer.code =  MTP_OP_GET_STORAGE_INFO;
  hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
  hmtp->GenericContainer.type = MTP_CONT_TYPE_DATA;

  (void)MTP_Get_PayloadContent(pdev);

  hmtp->ResponseLength = sizeof(MTP_StorageInfo) + MTP_CONT_HEADER_SIZE;
  hmtp->GenericContainer.length =  hmtp->ResponseLength;

  hmtp->ResponseCode = MTP_RESPONSE_OK;
}

/**
  * @brief  USBD_MTP_OPT_GetObjectHandle
  *         Get all object handles
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_GetObjectHandle(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  hmtp->GenericContainer.code =  MTP_OP_GET_OBJECT_HANDLES;
  hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
  hmtp->GenericContainer.type = MTP_CONT_TYPE_DATA;

  (void)MTP_Get_PayloadContent(pdev);

  hmtp->ResponseLength = hmtp->ResponseLength + MTP_CONT_HEADER_SIZE;
  hmtp->GenericContainer.length =  hmtp->ResponseLength;

  hmtp->ResponseCode = MTP_RESPONSE_OK;
}

/**
  * @brief  USBD_MTP_OPT_GetObjectInfo
  *         Get all information about the object
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_GetObjectInfo(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  hmtp->GenericContainer.code =  MTP_OP_GET_OBJECT_INFO;
  hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
  hmtp->GenericContainer.type = MTP_CONT_TYPE_DATA;

  (void)MTP_Get_PayloadContent(pdev);

  hmtp->ResponseLength = hmtp->ResponseLength + MTP_CONT_HEADER_SIZE;
  hmtp->GenericContainer.length =  hmtp->ResponseLength;

  hmtp->ResponseCode = MTP_RESPONSE_OK;
}

/**
  * @brief  USBD_MTP_OPT_GetObjectReferences
  *         Get object references
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_GetObjectReferences(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  hmtp->GenericContainer.code = MTP_OP_GET_OBJECT_PROP_REFERENCES;
  hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
  hmtp->GenericContainer.type = MTP_CONT_TYPE_DATA;

  (void)MTP_Get_PayloadContent(pdev);

  hmtp->ResponseLength = sizeof(MTP_Ref) + MTP_CONT_HEADER_SIZE;
  hmtp->GenericContainer.length = hmtp->ResponseLength;

  hmtp->ResponseCode = MTP_RESPONSE_OK;
}

/**
  * @brief  USBD_MTP_OPT_GetObjectPropSupp
  *         Get all object properties supported
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_GetObjectPropSupp(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  hmtp->GenericContainer.code = MTP_OP_GET_OBJECT_PROPS_SUPPORTED;
  hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
  hmtp->GenericContainer.type = MTP_CONT_TYPE_DATA;

  (void)MTP_Get_PayloadContent(pdev);

  hmtp->ResponseLength = sizeof(MTP_ObjectPropSupp) + MTP_CONT_HEADER_SIZE;
  hmtp->GenericContainer.length =  hmtp->ResponseLength;

  hmtp->ResponseCode = MTP_RESPONSE_OK;
}

/**
  * @brief  USBD_MTP_OPT_GetObjectPropDesc
  *         Get all descriptions about object properties
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_GetObjectPropDesc(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  hmtp->GenericContainer.code = MTP_OP_GET_OBJECT_PROP_DESC;
  hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
  hmtp->GenericContainer.type = MTP_CONT_TYPE_DATA;

  (void)MTP_Get_PayloadContent(pdev);

  hmtp->ResponseLength = hmtp->ResponseLength + MTP_CONT_HEADER_SIZE;
  hmtp->GenericContainer.length =  hmtp->ResponseLength;

  hmtp->ResponseCode = MTP_RESPONSE_OK;
}

/**
  * @brief  USBD_MTP_OPT_GetObjectPropList
  *         Get the list of object properties
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_GetObjectPropList(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  hmtp->GenericContainer.code = MTP_OP_GET_OBJECT_PROPLIST;
  hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
  hmtp->GenericContainer.type = MTP_CONT_TYPE_DATA;

  (void)MTP_Get_PayloadContent(pdev);

  hmtp->ResponseLength = hmtp->ResponseLength + MTP_CONT_HEADER_SIZE;
  hmtp->GenericContainer.length =  hmtp->ResponseLength;

  hmtp->ResponseCode = MTP_RESPONSE_OK;
}

/**
  * @brief  USBD_MTP_OPT_GetObjectPropValue
  *         Get current value of the object property
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_GetObjectPropValue(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  hmtp->GenericContainer.code = MTP_OP_GET_OBJECT_PROP_VALUE;
  hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
  hmtp->GenericContainer.type = MTP_CONT_TYPE_DATA;

  (void)MTP_Get_PayloadContent(pdev);

  hmtp->ResponseLength = hmtp->ResponseLength + MTP_CONT_HEADER_SIZE;
  hmtp->GenericContainer.length =  hmtp->ResponseLength;

  hmtp->ResponseCode = MTP_RESPONSE_OK;
}

/**
  * @brief  USBD_MTP_OPT_GetObject
  *         Get binary data from an object
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_GetObject(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  USBD_MTP_ItfTypeDef     *hmtpif = (USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId];

  hmtp->GenericContainer.length = hmtpif->GetContainerLength(hmtp->OperationsContainer.Param1);
  hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
  hmtp->GenericContainer.type = MTP_CONT_TYPE_DATA;

  hmtp->ResponseCode = MTP_RESPONSE_OK;
}

/**
  * @brief  USBD_MTP_OPT_GetDevicePropDesc
  *         Get The DevicePropDesc dataset
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_GetDevicePropDesc(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];

  hmtp->GenericContainer.code = MTP_OP_GET_DEVICE_PROP_DESC;
  hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
  hmtp->GenericContainer.type = MTP_CONT_TYPE_DATA;

  (void)MTP_Get_PayloadContent(pdev);

  hmtp->ResponseLength = sizeof(MTP_DevicePropDesc) + MTP_CONT_HEADER_SIZE;
  hmtp->GenericContainer.length =  hmtp->ResponseLength;

  hmtp->ResponseCode = MTP_RESPONSE_OK;
}

/**
  * @brief  USBD_MTP_OPT_SendObject
  *         Send object from host to MTP device
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_SendObject(USBD_HandleTypeDef  *pdev, uint8_t *buff, uint32_t len)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  USBD_MTP_ItfTypeDef     *hmtpif = (USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId];
  static uint32_t tmp = 0U;

  switch (hmtp->RECEIVE_DATA_STATUS)
  {
    case RECEIVE_IDLE_STATE:
      hmtp->RECEIVE_DATA_STATUS = RECEIVE_COMMAND_DATA;
      break;
    case RECEIVE_COMMAND_DATA:
      hmtp->RECEIVE_DATA_STATUS = RECEIVE_FIRST_DATA;
      break;
    case RECEIVE_FIRST_DATA:
      if ((uint16_t)len < (hmtp->MaxPcktLen - MTP_CONT_HEADER_SIZE))
      {
        hmtp->GenericContainer.code =  MTP_RESPONSE_OK;
        hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
        hmtp->GenericContainer.type = MTP_CONT_TYPE_RESPONSE;
        hmtp->ResponseLength = MTP_CONT_HEADER_SIZE;
        hmtp->GenericContainer.length =  hmtp->ResponseLength;

        hmtp->RECEIVE_DATA_STATUS = RECEIVE_IDLE_STATE;
      }
      else
      {
        hmtp->RECEIVE_DATA_STATUS = RECEIVE_REST_OF_DATA;
      }
      tmp = (uint32_t)buff;
      hmtpif->WriteData(len, (uint8_t *)(tmp + 12U));
      break;

    case RECEIVE_REST_OF_DATA:
      hmtpif->WriteData(len, buff);
      break;

    case SEND_RESPONSE:
      hmtpif->WriteData(0, buff); /* send 0 length to stop write process */
      hmtp->GenericContainer.code =  MTP_RESPONSE_OK;
      hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
      hmtp->GenericContainer.type = MTP_CONT_TYPE_RESPONSE;
      hmtp->ResponseLength = MTP_CONT_HEADER_SIZE;
      hmtp->GenericContainer.length =  hmtp->ResponseLength;

      hmtp->RECEIVE_DATA_STATUS = RECEIVE_IDLE_STATE;
      break;

    default:
      break;
  }

  hmtp->ResponseCode = MTP_RESPONSE_OK;
}

/**
  * @brief  USBD_MTP_OPT_SendObjectInfo
  *         Send the object information from host to MTP device
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_SendObjectInfo(USBD_HandleTypeDef  *pdev, uint8_t *buff, uint32_t len)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  USBD_MTP_ItfTypeDef     *hmtpif = (USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId];
  MTP_ObjectInfoTypeDef ObjectInfo;
  uint8_t dataLength = offsetof(MTP_ObjectInfoTypeDef, Filename);
  uint8_t *tmp;

  switch (hmtp->RECEIVE_DATA_STATUS)
  {
    case RECEIVE_IDLE_STATE:
      hmtp->RECEIVE_DATA_STATUS = RECEIVE_COMMAND_DATA;
      break;

    case RECEIVE_COMMAND_DATA:
      /* store object handle and storage id for future use */
      if (hmtp->OperationsContainer.Param2  == 0xFFFFFFFFU)
      {
        objhandle = 0U;
      }
      else
      {
        objhandle = hmtp->OperationsContainer.Param2;
      }
      storage_id = hmtp->OperationsContainer.Param1;
      hmtp->RECEIVE_DATA_STATUS = RECEIVE_FIRST_DATA;
      break;

    case RECEIVE_FIRST_DATA:
      tmp = buff;

      (void)USBD_memcpy(ObjInfo_buff, tmp + 12U,
                        (uint16_t)(hmtp->MaxPcktLen - MTP_CONT_HEADER_SIZE));

      hmtp->RECEIVE_DATA_STATUS = RECEIVE_REST_OF_DATA;
      break;

    case RECEIVE_REST_OF_DATA:

      (void)USBD_memcpy(ObjInfo_buff + len, buff, hmtp->MaxPcktLen);

      break;

    case SEND_RESPONSE:
      (void)USBD_memcpy((uint8_t *)&ObjectInfo, ObjInfo_buff, dataLength);
      (void)USBD_memcpy((uint8_t *)&ObjectInfo.Filename, (ObjInfo_buff + dataLength),
                        ((uint32_t)(ObjectInfo.Filename_len) * 2U));

      obj_format = ObjectInfo.ObjectFormat;

      hmtp->ResponseCode = hmtpif->Create_NewObject(ObjectInfo, objhandle);
      hmtp->GenericContainer.code = (uint16_t)hmtp->ResponseCode;
      hmtp->ResponseLength = MTP_CONT_HEADER_SIZE + (sizeof(uint32_t) * 3U); /* Header + 3 Param */
      hmtp->GenericContainer.length =  hmtp->ResponseLength;
      hmtp->GenericContainer.type = MTP_CONT_TYPE_RESPONSE;
      hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;

      (void)MTP_Get_PayloadContent(pdev);

      hmtp->RECEIVE_DATA_STATUS = RECEIVE_IDLE_STATE;
      break;

    default:
      break;
  }
}

/**
  * @brief  USBD_MTP_OPT_DeleteObject
  *         Delete the object from the device
  * @param  pdev: device instance
  * @retval None
  */
void USBD_MTP_OPT_DeleteObject(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  USBD_MTP_ItfTypeDef     *hmtpif = (USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId];

  hmtp->GenericContainer.trans_id = hmtp->OperationsContainer.trans_id;
  hmtp->GenericContainer.type = MTP_CONT_TYPE_RESPONSE;
  hmtp->ResponseLength = MTP_CONT_HEADER_SIZE;
  hmtp->GenericContainer.length =  hmtp->ResponseLength;
  hmtp->ResponseCode = hmtpif->DeleteObject(hmtp->OperationsContainer.Param1);
}

/**
  * @brief  MTP_Get_PayloadContent
  *         Get the payload data of generic container
  * @param  pdev: device instance
  * @retval None
  */
static void MTP_Get_PayloadContent(USBD_HandleTypeDef *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  USBD_MTP_ItfTypeDef     *hmtpif = (USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId];
  uint8_t *buffer = hmtp->GenericContainer.data;
  uint32_t i;
  uint32_t n_idx;

  switch (hmtp->OperationsContainer.code)
  {
    case MTP_OP_GET_DEVICE_INFO:
      (void)MTP_Get_DeviceInfo();
      (void)USBD_memcpy(buffer, (const uint8_t *)&MTP_DeviceInfo, sizeof(MTP_DeviceInfo));

      for (i = 0U; i < sizeof(MTP_StorageIDS); i++)
      {
        hmtp->GenericContainer.data[i] = buffer[i];
      }
      break;

    case MTP_OP_GET_STORAGE_IDS:
      (void)MTP_Get_StorageIDS();
      (void)USBD_memcpy(buffer, (const uint8_t *)&MTP_StorageIDS, sizeof(MTP_StorageIDS));

      for (i = 0U; i < sizeof(MTP_StorageIDS); i++)
      {
        hmtp->GenericContainer.data[i] = buffer[i];
      }
      break;

    case MTP_OP_GET_STORAGE_INFO:
      (void)MTP_Get_StorageInfo(pdev);
      (void)USBD_memcpy(buffer, (const uint8_t *)&MTP_StorageInfo, sizeof(MTP_StorageInfo));

      for (i = 0U; i < sizeof(MTP_StorageInfo); i++)
      {
        hmtp->GenericContainer.data[i] = buffer[i];
      }
      break;

    case MTP_OP_GET_OBJECT_HANDLES:
      (void)MTP_Get_ObjectHandle(pdev);
      (void)USBD_memcpy(buffer, (const uint8_t *)&MTP_ObjectHandle, hmtp->ResponseLength);

      for (i = 0U; i < hmtp->ResponseLength; i++)
      {
        hmtp->GenericContainer.data[i] = buffer[i];
      }
      break;

    case MTP_OP_GET_OBJECT_INFO:
      (void)MTP_Get_ObjectInfo(pdev);
      break;

    case MTP_OP_GET_OBJECT_PROPS_SUPPORTED:
      (void)MTP_Get_ObjectPropSupp();
      (void)USBD_memcpy(buffer, (const uint8_t *)&MTP_ObjectPropSupp, sizeof(MTP_ObjectPropSupp));

      for (i = 0U; i < sizeof(MTP_ObjectPropSupp); i++)
      {
        hmtp->GenericContainer.data[i] = buffer[i];
      }
      break;

    case MTP_OP_GET_OBJECT_PROP_DESC:
      (void)MTP_Get_ObjectPropDesc(pdev);
      hmtp->ResponseLength = MTP_build_data_propdesc(pdev, MTP_ObjectPropDesc);
      break;

    case MTP_OP_GET_OBJECT_PROP_REFERENCES:
      MTP_Ref.ref_len = 0U;
      (void)USBD_memcpy(buffer, (const uint8_t *)&MTP_Ref.ref_len, sizeof(MTP_Ref.ref_len));

      for (i = 0U; i < sizeof(MTP_Ref.ref_len); i++)
      {
        hmtp->GenericContainer.data[i] = buffer[i];
      }
      break;

    case MTP_OP_GET_OBJECT_PROPLIST:
      (void)MTP_Get_ObjectPropList(pdev);
      break;

    case MTP_OP_GET_OBJECT_PROP_VALUE:
      buffer = MTP_Get_ObjectPropValue(pdev);
      for (i = 0U; i < hmtp->ResponseLength; i++)
      {
        hmtp->GenericContainer.data[i] = buffer[i];
      }

      break;

    case MTP_OP_GET_DEVICE_PROP_DESC:
      (void)MTP_Get_DevicePropDesc();
      (void)USBD_memcpy(buffer, (const uint8_t *)&MTP_DevicePropDesc, sizeof(MTP_DevicePropDesc));
      for (i = 0U; i < sizeof(MTP_DevicePropDesc); i++)
      {
        hmtp->GenericContainer.data[i] = buffer[i];
      }
      break;

    case MTP_OP_SEND_OBJECT_INFO:
      n_idx = hmtpif->GetNewIndex(obj_format);
      (void)USBD_memcpy(hmtp->GenericContainer.data, (const uint8_t *)&storage_id, sizeof(uint32_t));
      (void)USBD_memcpy(hmtp->GenericContainer.data + 4U, (const uint8_t *)&objhandle, sizeof(uint32_t));
      (void)USBD_memcpy(hmtp->GenericContainer.data + 8U, (const uint8_t *)&n_idx, sizeof(uint32_t));
      break;

    case MTP_OP_GET_OBJECT:
      break;

    default:
      break;
  }
}

/**
  * @brief  MTP_Get_DeviceInfo
  *         Fill the MTP_DeviceInfo struct
  * @param  pdev: device instance
  * @retval None
  */
static void MTP_Get_DeviceInfo(void)
{
  MTP_DeviceInfo.StandardVersion = STANDARD_VERSION;
  MTP_DeviceInfo.VendorExtensionID = VEND_EXT_ID;
  MTP_DeviceInfo.VendorExtensionVersion = VEND_EXT_VERSION;
  MTP_DeviceInfo.VendorExtensionDesc_len = (uint8_t)VEND_EXT_DESC_LEN;
  uint32_t i;

#if USBD_MTP_VEND_EXT_DESC_SUPPORTED == 1
  for (i = 0U; i < VEND_EXT_DESC_LEN; i++)
  {
    MTP_DeviceInfo.VendorExtensionDesc[i] = VendExtDesc[i];
  }
#endif /* USBD_MTP_VEND_EXT_DESC_SUPPORTED */

  MTP_DeviceInfo.FunctionalMode = FUNCTIONAL_MODE; /* device supports one mode , standard mode */

  /* All supported operation */
  MTP_DeviceInfo.OperationsSupported_len = SUPP_OP_LEN;
  for (i = 0U; i < SUPP_OP_LEN; i++)
  {
    MTP_DeviceInfo.OperationsSupported[i] = SuppOP[i];
  }

  MTP_DeviceInfo.EventsSupported_len = SUPP_EVENTS_LEN; /* event that are currently generated by the device*/

#if USBD_MTP_EVENTS_SUPPORTED == 1
  for (i = 0U; i < SUPP_EVENTS_LEN; i++)
  {
    MTP_DeviceInfo.EventsSupported[i] = SuppEvents[i];
  }
#endif /* USBD_MTP_EVENTS_SUPPORTED */

  MTP_DeviceInfo.DevicePropertiesSupported_len = SUPP_DEVICE_PROP_LEN;

#if USBD_MTP_DEVICE_PROP_SUPPORTED == 1
  for (i = 0U; i < SUPP_DEVICE_PROP_LEN; i++)
  {
    MTP_DeviceInfo.DevicePropertiesSupported[i] = DevicePropSupp[i];
  }
#endif /* USBD_MTP_DEVICE_PROP_SUPPORTED */

  MTP_DeviceInfo.CaptureFormats_len = SUPP_CAPT_FORMAT_LEN;

#if USBD_MTP_CAPTURE_FORMAT_SUPPORTED == 1
  for (i = 0U; i < SUPP_CAPT_FORMAT_LEN; i++)
  {
    MTP_DeviceInfo.CaptureFormats[i] = SuppCaptFormat[i];
  }
#endif /* USBD_MTP_CAPTURE_FORMAT_SUPPORTED */

  MTP_DeviceInfo.ImageFormats_len = SUPP_IMG_FORMAT_LEN; /* number of image formats that are supported by the device*/
  for (i = 0U; i < SUPP_IMG_FORMAT_LEN; i++)
  {
    MTP_DeviceInfo.ImageFormats[i] = SuppImgFormat[i];
  }

  MTP_DeviceInfo.Manufacturer_len = (uint8_t)MANUF_LEN;
  for (i = 0U; i < MANUF_LEN; i++)
  {
    MTP_DeviceInfo.Manufacturer[i] = Manuf[i];
  }

  MTP_DeviceInfo.Model_len = (uint8_t)MODEL_LEN;
  for (i = 0U; i < MODEL_LEN; i++)
  {
    MTP_DeviceInfo.Model[i] = Model[i];
  }

  MTP_DeviceInfo.DeviceVersion_len = (uint8_t)DEVICE_VERSION_LEN;
  for (i = 0U; i < DEVICE_VERSION_LEN; i++)
  {
    MTP_DeviceInfo.DeviceVersion[i] = DeviceVers[i];
  }

  MTP_DeviceInfo.SerialNumber_len = (uint8_t)SERIAL_NBR_LEN;
  for (i = 0U; i < SERIAL_NBR_LEN; i++)
  {
    MTP_DeviceInfo.SerialNumber[i] = SerialNbr[i];
  }
}

/**
  * @brief  MTP_Get_StorageInfo
  *         Fill the MTP_StorageInfo struct
  * @param  pdev: device instance
  * @retval None
  */
static void MTP_Get_StorageInfo(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_ItfTypeDef *hmtpif = (USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId];

  MTP_StorageInfo.StorageType = MTP_STORAGE_REMOVABLE_RAM;
  MTP_StorageInfo.FilesystemType = MTP_FILESYSTEM_GENERIC_FLAT;
  MTP_StorageInfo.AccessCapability = MTP_ACCESS_CAP_RW;
  MTP_StorageInfo.MaxCapability = hmtpif->GetMaxCapability();
  MTP_StorageInfo.FreeSpaceInBytes = hmtpif->GetFreeSpaceInBytes();
  MTP_StorageInfo.FreeSpaceInObjects = FREE_SPACE_IN_OBJ_NOT_USED; /* not used */
  MTP_StorageInfo.StorageDescription = 0U;
  MTP_StorageInfo.VolumeLabel = 0U;
}

/**
  * @brief  MTP_Get_ObjectHandle
  *         Fill the MTP_ObjectHandle struct
  * @param  pdev: device instance
  * @retval None
  */
static void MTP_Get_ObjectHandle(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  USBD_MTP_ItfTypeDef     *hmtpif = (USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId];

  MTP_ObjectHandle.ObjectHandle_len = (uint32_t)(hmtpif->GetIdx(hmtp->OperationsContainer.Param3,
                                                                MTP_ObjectHandle.ObjectHandle));

  hmtp->ResponseLength = (MTP_ObjectHandle.ObjectHandle_len * sizeof(uint32_t)) + sizeof(uint32_t);
}

/**
  * @brief  MTP_Get_ObjectPropSupp
  *         Fill the MTP_ObjectPropSupp struct
  * @param  pdev: device instance
  * @retval None
  */
static void MTP_Get_ObjectPropSupp(void)
{
  uint32_t i;

  MTP_ObjectPropSupp.ObjectPropSupp_len = SUPP_OBJ_PROP_LEN;

  for (i = 0U; i < SUPP_OBJ_PROP_LEN; i++)
  {
    MTP_ObjectPropSupp.ObjectPropSupp[i] = ObjectPropSupp[i];
  }
}

/**
  * @brief  MTP_Get_ObjectPropDesc
  *         Fill the MTP_ObjectPropDesc struct
  * @param  pdev: device instance
  * @retval None
  */
static void MTP_Get_ObjectPropDesc(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint16_t undef_format = MTP_OBJ_FORMAT_UNDEFINED;
  uint32_t storageid = MTP_STORAGE_ID;

  switch (hmtp->OperationsContainer.Param1) /* switch obj prop code */
  {
    case MTP_OB_PROP_OBJECT_FORMAT :
      MTP_ObjectPropDesc.ObjectPropertyCode = (uint16_t)(hmtp->OperationsContainer.Param1);
      MTP_ObjectPropDesc.DataType = MTP_DATATYPE_UINT16;
      MTP_ObjectPropDesc.GetSet = MTP_PROP_GET;
      MTP_ObjectPropDesc.DefValue = (uint8_t *)&undef_format;
      MTP_ObjectPropDesc.GroupCode = 0U;
      MTP_ObjectPropDesc.FormFlag = 0U;
      break;

    case MTP_OB_PROP_STORAGE_ID :
      MTP_ObjectPropDesc.ObjectPropertyCode = (uint16_t)(hmtp->OperationsContainer.Param1);
      MTP_ObjectPropDesc.DataType = MTP_DATATYPE_UINT32;
      MTP_ObjectPropDesc.GetSet = MTP_PROP_GET;
      MTP_ObjectPropDesc.DefValue = (uint8_t *)&storageid;
      MTP_ObjectPropDesc.GroupCode = 0U;
      MTP_ObjectPropDesc.FormFlag = 0U;
      break;

    case MTP_OB_PROP_OBJ_FILE_NAME :
      MTP_ObjectPropDesc.ObjectPropertyCode = (uint16_t)(hmtp->OperationsContainer.Param1);
      MTP_ObjectPropDesc.DataType = MTP_DATATYPE_STR;
      MTP_ObjectPropDesc.GetSet = MTP_PROP_GET;
      MTP_FileName.FileName_len = DEFAULT_FILE_NAME_LEN;
      (void)USBD_memcpy((void *) & (MTP_FileName.FileName), (const void *)DefaultFileName, sizeof(DefaultFileName));
      MTP_ObjectPropDesc.DefValue = (uint8_t *)&MTP_FileName;
      MTP_ObjectPropDesc.GroupCode = 0U;
      MTP_ObjectPropDesc.FormFlag = 0U;
      break;

    case MTP_OB_PROP_PARENT_OBJECT :
      MTP_ObjectPropDesc.ObjectPropertyCode = (uint16_t)(hmtp->OperationsContainer.Param1);
      MTP_ObjectPropDesc.DataType = MTP_DATATYPE_STR;
      MTP_ObjectPropDesc.GetSet = MTP_PROP_GET;
      MTP_ObjectPropDesc.DefValue = 0U;
      MTP_ObjectPropDesc.GroupCode = 0U;
      MTP_ObjectPropDesc.FormFlag = 0U;
      break;

    case MTP_OB_PROP_OBJECT_SIZE :
      MTP_ObjectPropDesc.ObjectPropertyCode = (uint16_t)(hmtp->OperationsContainer.Param1);
      MTP_ObjectPropDesc.DataType = MTP_DATATYPE_UINT64;
      MTP_ObjectPropDesc.GetSet = MTP_PROP_GET;
      MTP_ObjectPropDesc.DefValue = 0U;
      MTP_ObjectPropDesc.GroupCode = 0U;
      MTP_ObjectPropDesc.FormFlag = 0U;
      break;

    case MTP_OB_PROP_NAME :
      MTP_ObjectPropDesc.ObjectPropertyCode = (uint16_t)(hmtp->OperationsContainer.Param1);
      MTP_ObjectPropDesc.DataType = MTP_DATATYPE_STR;
      MTP_ObjectPropDesc.GetSet = MTP_PROP_GET;
      MTP_FileName.FileName_len = DEFAULT_FILE_NAME_LEN;
      (void)USBD_memcpy((void *) & (MTP_FileName.FileName),
                        (const void *)DefaultFileName, sizeof(DefaultFileName));

      MTP_ObjectPropDesc.DefValue = (uint8_t *)&MTP_FileName;
      MTP_ObjectPropDesc.GroupCode = 0U;
      MTP_ObjectPropDesc.FormFlag = 0U;
      break;

    case MTP_OB_PROP_PERS_UNIQ_OBJ_IDEN :
      MTP_ObjectPropDesc.ObjectPropertyCode = (uint16_t)(hmtp->OperationsContainer.Param1);
      MTP_ObjectPropDesc.DataType = MTP_DATATYPE_UINT128;
      MTP_ObjectPropDesc.GetSet = MTP_PROP_GET;
      MTP_ObjectPropDesc.DefValue = 0U;
      MTP_ObjectPropDesc.GroupCode = 0U;
      MTP_ObjectPropDesc.FormFlag = 0U;
      break;

    case MTP_OB_PROP_PROTECTION_STATUS :
      MTP_ObjectPropDesc.ObjectPropertyCode = (uint16_t)(hmtp->OperationsContainer.Param1);
      MTP_ObjectPropDesc.DataType = MTP_DATATYPE_UINT16;
      MTP_ObjectPropDesc.GetSet = MTP_PROP_GET_SET;
      MTP_ObjectPropDesc.DefValue = 0U;
      MTP_ObjectPropDesc.GroupCode = 0U;
      MTP_ObjectPropDesc.FormFlag = 0U;
      break;

    default:
      break;
  }
}

/**
  * @brief  MTP_Get_ObjectPropValue
  *         Get the property value
  * @param  pdev: device instance
  * @retval None
  */
static uint8_t *MTP_Get_ObjectPropValue(USBD_HandleTypeDef *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  USBD_MTP_ItfTypeDef     *hmtpif = (USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId];
  static uint8_t buf[512];

  /* Add all other supported object properties */
  switch (hmtp->OperationsContainer.Param2)
  {
    case MTP_OB_PROP_STORAGE_ID:
      MTP_PropertyValue.u32 = MTP_STORAGE_ID;
      (void)USBD_memcpy(buf, (const uint8_t *)&MTP_PropertyValue, sizeof(uint32_t));
      hmtp->ResponseLength = sizeof(uint32_t);
      break;

    case MTP_OB_PROP_OBJECT_FORMAT:
      MTP_PropertyValue.u16 = hmtpif->GetObjectFormat(hmtp->OperationsContainer.Param1);
      (void)USBD_memcpy(buf, (const uint8_t *)&MTP_PropertyValue, sizeof(uint16_t));
      hmtp->ResponseLength = sizeof(uint16_t);
      break;

    case MTP_OB_PROP_OBJ_FILE_NAME:
      MTP_FileName.FileName_len = hmtpif->GetObjectName_len(hmtp->OperationsContainer.Param1);
      hmtpif->GetObjectName(hmtp->OperationsContainer.Param1, MTP_FileName.FileName_len, (uint16_t *)buf);
      (void)USBD_memcpy(MTP_FileName.FileName, (uint16_t *)buf, ((uint32_t)MTP_FileName.FileName_len * 2U) + 1U);

      hmtp->ResponseLength = ((uint32_t)MTP_FileName.FileName_len * 2U) + 1U;
      break;

    case MTP_OB_PROP_PARENT_OBJECT :
      MTP_PropertyValue.u32 = hmtpif->GetParentObject(hmtp->OperationsContainer.Param1);
      (void)USBD_memcpy(buf, (const uint8_t *)&MTP_PropertyValue, sizeof(uint32_t));
      hmtp->ResponseLength = sizeof(uint32_t);
      break;

    case MTP_OB_PROP_OBJECT_SIZE :
      MTP_PropertyValue.u64 = hmtpif->GetObjectSize(hmtp->OperationsContainer.Param1);
      (void)USBD_memcpy(buf, (const uint8_t *)&MTP_PropertyValue, sizeof(uint64_t));
      hmtp->ResponseLength = sizeof(uint64_t);
      break;

    default:
      break;
  }

  return buf;
}

/**
  * @brief  MTP_Get_ObjectPropList
  *         Get the object property list data to be transmitted
  * @param  pdev: device instance
  * @retval None
  */
static void MTP_Get_ObjectPropList(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_HandleTypeDef  *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  USBD_MTP_ItfTypeDef     *hmtpif = (USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId];
  uint16_t filename[255];
  uint32_t storageid = MTP_STORAGE_ID;
  uint32_t default_val = 0U;
  uint32_t i;
  uint16_t format;
  uint64_t objsize;
  uint32_t parent_proval;

  MTP_PropertiesList.MTP_Properties_len = SUPP_OBJ_PROP_LEN;
  hmtp->ResponseLength = 4U;    /* size of MTP_PropertiesList.MTP_Properties_len */
  (void)USBD_memcpy(hmtp->GenericContainer.data,
                    (const uint8_t *)&MTP_PropertiesList.MTP_Properties_len, hmtp->ResponseLength);

  for (i = 0U; i < SUPP_OBJ_PROP_LEN; i++)
  {
    MTP_PropertiesList.MTP_Properties[i].ObjectHandle = hmtp->OperationsContainer.Param1;

    switch (ObjectPropSupp[i])
    {
      case MTP_OB_PROP_STORAGE_ID :
        MTP_PropertiesList.MTP_Properties[i].PropertyCode = MTP_OB_PROP_STORAGE_ID;
        MTP_PropertiesList.MTP_Properties[i].Datatype = MTP_DATATYPE_UINT32;
        MTP_PropertiesList.MTP_Properties[i].propval = (uint8_t *)&storageid;
        break;

      case MTP_OB_PROP_OBJECT_FORMAT :
        MTP_PropertiesList.MTP_Properties[i].PropertyCode = MTP_OB_PROP_OBJECT_FORMAT;
        MTP_PropertiesList.MTP_Properties[i].Datatype = MTP_DATATYPE_UINT16;
        format = hmtpif->GetObjectFormat(hmtp->OperationsContainer.Param1);
        MTP_PropertiesList.MTP_Properties[i].propval = (uint8_t *)&format;
        break;

      case MTP_OB_PROP_OBJ_FILE_NAME:
        MTP_PropertiesList.MTP_Properties[i].PropertyCode = MTP_OB_PROP_OBJ_FILE_NAME;
        MTP_PropertiesList.MTP_Properties[i].Datatype = MTP_DATATYPE_STR;
        /* MTP_FileName.FileName_len value shall be set before USBD_MTP_FS_GetObjectName */
        MTP_FileName.FileName_len = hmtpif->GetObjectName_len(hmtp->OperationsContainer.Param1);
        hmtpif->GetObjectName(hmtp->OperationsContainer.Param1, MTP_FileName.FileName_len, filename);
        (void)USBD_memcpy(MTP_FileName.FileName, filename, ((uint32_t)MTP_FileName.FileName_len * 2U) + 1U);
        MTP_PropertiesList.MTP_Properties[i].propval = (uint8_t *)&MTP_FileName;
        break;

      case MTP_OB_PROP_PARENT_OBJECT :
        MTP_PropertiesList.MTP_Properties[i].PropertyCode = MTP_OB_PROP_PARENT_OBJECT;
        MTP_PropertiesList.MTP_Properties[i].Datatype = MTP_DATATYPE_UINT32;
        parent_proval = hmtpif->GetParentObject(hmtp->OperationsContainer.Param1);
        MTP_PropertiesList.MTP_Properties[i].propval = (uint8_t *)&parent_proval;
        break;

      case MTP_OB_PROP_OBJECT_SIZE :
        MTP_PropertiesList.MTP_Properties[i].PropertyCode = MTP_OB_PROP_OBJECT_SIZE;
        MTP_PropertiesList.MTP_Properties[i].Datatype = MTP_DATATYPE_UINT64;
        objsize = hmtpif->GetObjectSize(hmtp->OperationsContainer.Param1);
        MTP_PropertiesList.MTP_Properties[i].propval = (uint8_t *)&objsize;
        break;

      case MTP_OB_PROP_NAME :
        MTP_PropertiesList.MTP_Properties[i].PropertyCode = MTP_OB_PROP_NAME;
        MTP_PropertiesList.MTP_Properties[i].Datatype = MTP_DATATYPE_STR;
        /* MTP_FileName.FileName_len value shall be set before USBD_MTP_FS_GetObjectName */
        MTP_FileName.FileName_len = hmtpif->GetObjectName_len(hmtp->OperationsContainer.Param1);
        hmtpif->GetObjectName(hmtp->OperationsContainer.Param1, MTP_FileName.FileName_len, filename);
        (void)USBD_memcpy(MTP_FileName.FileName, filename, ((uint32_t)MTP_FileName.FileName_len * 2U) + 1U);
        MTP_PropertiesList.MTP_Properties[i].propval = (uint8_t *)&MTP_FileName;
        break;

      case MTP_OB_PROP_PERS_UNIQ_OBJ_IDEN :
        MTP_PropertiesList.MTP_Properties[i].PropertyCode = MTP_OB_PROP_PERS_UNIQ_OBJ_IDEN;
        MTP_PropertiesList.MTP_Properties[i].Datatype = MTP_DATATYPE_UINT128;
        MTP_PropertiesList.MTP_Properties[i].propval = (uint8_t *)&hmtp->OperationsContainer.Param1;
        break;

      case MTP_OB_PROP_PROTECTION_STATUS :
        MTP_PropertiesList.MTP_Properties[i].PropertyCode = MTP_OB_PROP_PROTECTION_STATUS;
        MTP_PropertiesList.MTP_Properties[i].Datatype = MTP_DATATYPE_UINT16;
        MTP_PropertiesList.MTP_Properties[i].propval = (uint8_t *)&default_val;
        break;

      default:
        break;
    }

    hmtp->ResponseLength = MTP_build_data_proplist(pdev, MTP_PropertiesList, i);
  }
}

/**
  * @brief  MTP_Get_DevicePropDesc
  *         Fill the MTP_DevicePropDesc struct
  * @param  pdev: device instance
  * @retval None
  */
static void MTP_Get_DevicePropDesc(void)
{
  MTP_DevicePropDesc.DevicePropertyCode = MTP_DEV_PROP_DEVICE_FRIENDLY_NAME;
  MTP_DevicePropDesc.DataType = MTP_DATATYPE_STR;
  MTP_DevicePropDesc.GetSet = MTP_PROP_GET_SET;
  MTP_DevicePropDesc.DefValue_len = DEVICE_PROP_DESC_DEF_LEN;
  uint32_t i;

  for (i = 0U; i < (sizeof(DevicePropDefVal) / 2U); i++)
  {
    MTP_DevicePropDesc.DefValue[i] = DevicePropDefVal[i];
  }

  MTP_DevicePropDesc.curDefValue_len = DEVICE_PROP_DESC_CUR_LEN;

  for (i = 0U; i < (sizeof(DevicePropCurDefVal) / 2U); i++)
  {
    MTP_DevicePropDesc.curDefValue[i] = DevicePropCurDefVal[i];
  }

  MTP_DevicePropDesc.FormFlag = 0U;
}

/**
  * @brief  MTP_Get_ObjectInfo
  *         Fill the MTP_ObjectInfo struct
  * @param  pdev: device instance
  * @retval None
  */
static void MTP_Get_ObjectInfo(USBD_HandleTypeDef  *pdev)
{
  USBD_MTP_ItfTypeDef *hmtpif = (USBD_MTP_ItfTypeDef *)pdev->pUserData[pdev->classId];
  USBD_MTP_HandleTypeDef *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint16_t filename[255];

  MTP_ObjectInfo.Storage_id = MTP_STORAGE_ID;
  MTP_ObjectInfo.ObjectFormat = hmtpif->GetObjectFormat(hmtp->OperationsContainer.Param1);
  MTP_ObjectInfo.ObjectCompressedSize = hmtpif->GetObjectSize(hmtp->OperationsContainer.Param1);
  MTP_ObjectInfo.ProtectionStatus = 0U;
  MTP_ObjectInfo.ThumbFormat = MTP_OBJ_FORMAT_UNDEFINED;
  MTP_ObjectInfo.ThumbCompressedSize  = 0U;
  MTP_ObjectInfo.ThumbPixWidth = 0U; /* not supported or not an image */
  MTP_ObjectInfo.ThumbPixHeight = 0U;
  MTP_ObjectInfo.ImagePixWidth = 0U;
  MTP_ObjectInfo.ImagePixHeight = 0U;
  MTP_ObjectInfo.ImageBitDepth = 0U;
  MTP_ObjectInfo.ParentObject = hmtpif->GetParentObject(hmtp->OperationsContainer.Param1);
  MTP_ObjectInfo.AssociationType = 0U;
  MTP_ObjectInfo.AssociationDesc = 0U;
  MTP_ObjectInfo.SequenceNumber = 0U;

  /* we have to get this value before MTP_ObjectInfo.Filename */
  MTP_ObjectInfo.Filename_len = hmtpif->GetObjectName_len(hmtp->OperationsContainer.Param1);
  hmtpif->GetObjectName(hmtp->OperationsContainer.Param1, MTP_ObjectInfo.Filename_len, filename);
  (void)USBD_memcpy(MTP_ObjectInfo.Filename, filename, ((uint32_t)MTP_FileName.FileName_len * 2U) + 1U);

  MTP_ObjectInfo.CaptureDate = 0U;
  MTP_ObjectInfo.ModificationDate = 0U;
  MTP_ObjectInfo.Keywords = 0U;
  hmtp->ResponseLength = MTP_build_data_ObjInfo(pdev, MTP_ObjectInfo);
}

/**
  * @brief  MTP_Get_StorageIDS
  *         Fill the MTP_StorageIDS struct
  * @param  pdev: device instance
  * @retval None
  */
static void MTP_Get_StorageIDS(void)
{
  MTP_StorageIDS.StorageIDS_len = MTP_NBR_STORAGE_ID;
  MTP_StorageIDS.StorageIDS[0] = MTP_STORAGE_ID;
}

/**
  * @brief  MTP_build_data_propdesc
  *         Copy the MTP_ObjectPropDesc dataset to the payload data
  * @param  pdev: device instance
  * @retval None
  */
static uint32_t MTP_build_data_propdesc(USBD_HandleTypeDef  *pdev, MTP_ObjectPropDescTypeDef def)
{
  USBD_MTP_HandleTypeDef *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t DefValue_size = (MTP_FileName.FileName_len * 2U) + 1U;
  uint32_t dataLength = offsetof(MTP_ObjectPropDescTypeDef, DefValue);

  (void)USBD_memcpy(hmtp->GenericContainer.data, (const uint8_t *)&def, dataLength);

  switch (def.DataType)
  {
    case MTP_DATATYPE_UINT16:
      (void)USBD_memcpy(hmtp->GenericContainer.data + dataLength, def.DefValue, sizeof(uint16_t));
      dataLength += sizeof(uint16_t);
      break;

    case MTP_DATATYPE_UINT32:
      (void)USBD_memcpy(hmtp->GenericContainer.data + dataLength, def.DefValue,  sizeof(uint32_t));
      dataLength += sizeof(uint32_t);
      break;

    case MTP_DATATYPE_UINT64:
      (void)USBD_memcpy(hmtp->GenericContainer.data + dataLength, def.DefValue,  sizeof(uint64_t));
      dataLength += sizeof(uint64_t);
      break;

    case MTP_DATATYPE_STR:
      (void)USBD_memcpy(hmtp->GenericContainer.data + dataLength, def.DefValue,  DefValue_size);
      dataLength += DefValue_size;
      break;

    case MTP_DATATYPE_UINT128:
      (void)USBD_memcpy(hmtp->GenericContainer.data + dataLength, def.DefValue, (sizeof(uint64_t) * 2U));
      dataLength += (sizeof(uint64_t) * 2U);
      break;

    default:
      break;
  }

  (void)USBD_memcpy(hmtp->GenericContainer.data + dataLength,
                    (const uint8_t *)&MTP_ObjectPropDesc.GroupCode, sizeof(MTP_ObjectPropDesc.GroupCode));

  dataLength += sizeof(MTP_ObjectPropDesc.GroupCode);

  (void)USBD_memcpy(hmtp->GenericContainer.data + dataLength,
                    (const uint8_t *)&MTP_ObjectPropDesc.FormFlag, sizeof(MTP_ObjectPropDesc.FormFlag));

  dataLength += sizeof(MTP_ObjectPropDesc.FormFlag);

  return dataLength;
}

/**
  * @brief  MTP_build_data_proplist
  *         Copy the MTP_PropertiesList dataset to the payload data
  * @param  pdev: device instance
  * @retval None
  */
static uint32_t MTP_build_data_proplist(USBD_HandleTypeDef  *pdev,
                                        MTP_PropertiesListTypedef proplist, uint32_t idx)
{
  USBD_MTP_HandleTypeDef *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint8_t propval_size = (MTP_FileName.FileName_len * 2U) + 1U;
  uint32_t dataLength;

  dataLength = offsetof(MTP_PropertiesTypedef, propval);

  (void)USBD_memcpy(hmtp->GenericContainer.data + hmtp->ResponseLength,
                    (const uint8_t *)&proplist.MTP_Properties[idx], dataLength);

  dataLength += hmtp->ResponseLength;

  switch (proplist.MTP_Properties[idx].Datatype)
  {
    case MTP_DATATYPE_UINT16:
      (void)USBD_memcpy(hmtp->GenericContainer.data + dataLength,
                        proplist.MTP_Properties[idx].propval, sizeof(uint16_t));

      dataLength += sizeof(uint16_t);
      break;

    case MTP_DATATYPE_UINT32:
      (void)USBD_memcpy(hmtp->GenericContainer.data + dataLength,
                        proplist.MTP_Properties[idx].propval, sizeof(uint32_t));

      dataLength += sizeof(uint32_t);
      break;

    case MTP_DATATYPE_STR:
      (void)USBD_memcpy(hmtp->GenericContainer.data + dataLength,
                        proplist.MTP_Properties[idx].propval, propval_size);

      dataLength += propval_size;
      break;

    case MTP_DATATYPE_UINT64:
      (void)USBD_memcpy(hmtp->GenericContainer.data + dataLength,
                        proplist.MTP_Properties[idx].propval, sizeof(uint64_t));

      dataLength += sizeof(uint64_t);
      break;

    case MTP_DATATYPE_UINT128:
      (void)USBD_memcpy(hmtp->GenericContainer.data + dataLength,
                        proplist.MTP_Properties[idx].propval, (sizeof(uint64_t) * 2U));

      dataLength += (sizeof(uint64_t) * 2U);
      break;

    default:
      break;
  }

  return dataLength;
}

/**
  * @brief  MTP_build_data_ObjInfo
  *         Copy the MTP_ObjectInfo dataset to the payload data
  * @param  pdev: device instance
  * @retval None
  */
static uint32_t MTP_build_data_ObjInfo(USBD_HandleTypeDef *pdev, MTP_ObjectInfoTypeDef objinfo)
{
  USBD_MTP_HandleTypeDef *hmtp = (USBD_MTP_HandleTypeDef *)pdev->pClassDataCmsit[pdev->classId];
  uint32_t ObjInfo_len = offsetof(MTP_ObjectInfoTypeDef, Filename);

  (void)USBD_memcpy(hmtp->GenericContainer.data, (const uint8_t *)&objinfo, ObjInfo_len);
  (void)USBD_memcpy(hmtp->GenericContainer.data + ObjInfo_len,
                    (const uint8_t *)&objinfo.Filename, objinfo.Filename_len * sizeof(uint16_t));

  ObjInfo_len = ObjInfo_len + (objinfo.Filename_len * sizeof(uint16_t));

  (void)USBD_memcpy(hmtp->GenericContainer.data + ObjInfo_len,
                    (const uint8_t *)&objinfo.CaptureDate, sizeof(objinfo.CaptureDate));

  ObjInfo_len = ObjInfo_len + sizeof(objinfo.CaptureDate);

  return ObjInfo_len;
}
