/**
  ******************************************************************************
  * @file    usbd_mtp_if.c
  * @author  MCD Application Team
  * @brief   Source file for USBD MTP file list_files.
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
#include "usbd_mtp_if_template.h"

/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*
static FILE MyFile;
static FATFS SDFatFs;
static char SDPath[4];
static FolderLevel Fold_Lvl;
static FOLD_INFTypeDef FoldStruct;
static FILE_INFTypeDef FileStruct;
static SD_Object_TypeDef sd_object;
*/
extern USBD_HandleTypeDef USBD_Device;

uint32_t idx[200];
uint32_t parent;
/* static char path[255]; */
uint32_t sc_buff[MTP_IF_SCRATCH_BUFF_SZE / 4U];
uint32_t sc_len = 0U;
uint32_t pckt_cnt = 1U;
uint32_t foldsize;

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static uint8_t  USBD_MTP_Itf_Init(void);
static uint8_t  USBD_MTP_Itf_DeInit(void);
static uint32_t USBD_MTP_Itf_ReadData(uint32_t Param1, uint8_t *buff, MTP_DataLengthTypeDef *data_length);
static uint16_t USBD_MTP_Itf_Create_NewObject(MTP_ObjectInfoTypeDef ObjectInfo, uint32_t objhandle);

static uint32_t USBD_MTP_Itf_GetIdx(uint32_t Param3, uint32_t *obj_handle);
static uint32_t USBD_MTP_Itf_GetParentObject(uint32_t Param);
static uint16_t USBD_MTP_Itf_GetObjectFormat(uint32_t Param);
static uint8_t  USBD_MTP_Itf_GetObjectName_len(uint32_t Param);
static void     USBD_MTP_Itf_GetObjectName(uint32_t Param, uint8_t obj_len, uint16_t *buf);
static uint32_t USBD_MTP_Itf_GetObjectSize(uint32_t Param);
static uint64_t USBD_MTP_Itf_GetMaxCapability(void);
static uint64_t USBD_MTP_Itf_GetFreeSpaceInBytes(void);
static uint32_t USBD_MTP_Itf_GetNewIndex(uint16_t objformat);
static void     USBD_MTP_Itf_WriteData(uint16_t len, uint8_t *buff);
static uint32_t USBD_MTP_Itf_GetContainerLength(uint32_t Param1);
static uint16_t USBD_MTP_Itf_DeleteObject(uint32_t Param1);

static void     USBD_MTP_Itf_Cancel(uint32_t Phase);
/* static uint32_t USBD_MTP_Get_idx_to_delete(uint32_t Param, uint8_t *tab); */

USBD_MTP_ItfTypeDef USBD_MTP_fops =
{
  USBD_MTP_Itf_Init,
  USBD_MTP_Itf_DeInit,
  USBD_MTP_Itf_ReadData,
  USBD_MTP_Itf_Create_NewObject,
  USBD_MTP_Itf_GetIdx,
  USBD_MTP_Itf_GetParentObject,
  USBD_MTP_Itf_GetObjectFormat,
  USBD_MTP_Itf_GetObjectName_len,
  USBD_MTP_Itf_GetObjectName,
  USBD_MTP_Itf_GetObjectSize,
  USBD_MTP_Itf_GetMaxCapability,
  USBD_MTP_Itf_GetFreeSpaceInBytes,
  USBD_MTP_Itf_GetNewIndex,
  USBD_MTP_Itf_WriteData,
  USBD_MTP_Itf_GetContainerLength,
  USBD_MTP_Itf_DeleteObject,
  USBD_MTP_Itf_Cancel,
  sc_buff,
  MTP_IF_SCRATCH_BUFF_SZE,
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  USBD_MTP_Itf_Init
  *         Initialize the file system Layer
  * @param  None
  * @retval status value
  */
static uint8_t USBD_MTP_Itf_Init(void)
{
  return 0;
}

/**
  * @brief  USBD_MTP_Itf_DeInit
  *         Uninitialize the file system Layer
  * @param  None
  * @retval status value
  */
static uint8_t USBD_MTP_Itf_DeInit(void)
{
  return 0;
}

/**
  * @brief  USBD_MTP_Itf_GetIdx
  *         Get all object handle
  * @param  Param3: current object handle
  * @param  obj_handle: all objects handle (subfolders/files) in current object
  * @retval number of object handle in current object
  */
static uint32_t USBD_MTP_Itf_GetIdx(uint32_t Param3, uint32_t *obj_handle)
{
  uint32_t count = 0U;
  UNUSED(Param3);
  UNUSED(obj_handle);

  return count;
}

/**
  * @brief  USBD_MTP_Itf_GetParentObject
  *         Get parent object
  * @param  Param: object handle (object index)
  * @retval parent object
  */
static uint32_t USBD_MTP_Itf_GetParentObject(uint32_t Param)
{
  uint32_t parentobj = 0U;
  UNUSED(Param);

  return parentobj;
}

/**
  * @brief  USBD_MTP_Itf_GetObjectFormat
  *         Get object format
  * @param  Param: object handle (object index)
  * @retval object format
  */
static uint16_t USBD_MTP_Itf_GetObjectFormat(uint32_t Param)
{
  uint16_t objformat = 0U;
  UNUSED(Param);

  return objformat;
}

/**
  * @brief  USBD_MTP_Itf_GetObjectName_len
  *         Get object name length
  * @param  Param: object handle (object index)
  * @retval object name length
  */
static uint8_t USBD_MTP_Itf_GetObjectName_len(uint32_t Param)
{
  uint8_t obj_len = 0U;
  UNUSED(Param);

  return obj_len;
}

/**
  * @brief  USBD_MTP_Itf_GetObjectName
  *         Get object name
  * @param  Param: object handle (object index)
  * @param  obj_len: length of object name
  * @param  buf: pointer to object name
  * @retval object size in SD card
  */
static void USBD_MTP_Itf_GetObjectName(uint32_t Param, uint8_t obj_len, uint16_t *buf)
{
  UNUSED(Param);
  UNUSED(obj_len);
  UNUSED(buf);

  return;
}

/**
  * @brief  USBD_MTP_Itf_GetObjectSize
  *         Get size of current object
  * @param  Param: object handle (object index)
  * @retval object size in SD card
  */
static uint32_t USBD_MTP_Itf_GetObjectSize(uint32_t Param)
{
  uint32_t ObjCompSize = 0U;
  UNUSED(Param);

  return ObjCompSize;
}

/**
  * @brief  USBD_MTP_Itf_Create_NewObject
  *         Create new object in SD card and store necessary information for future use
  * @param  ObjectInfo: object information to use
  * @param  objhandle: object handle (object index)
  * @retval None
  */
static uint16_t USBD_MTP_Itf_Create_NewObject(MTP_ObjectInfoTypeDef ObjectInfo, uint32_t objhandle)
{
  uint16_t rep_code = 0U;
  UNUSED(ObjectInfo);
  UNUSED(objhandle);

  return rep_code;
}

/**
  * @brief  USBD_MTP_Itf_GetMaxCapability
  *         Get max capability in SD card
  * @param  None
  * @retval max capability
  */
static uint64_t USBD_MTP_Itf_GetMaxCapability(void)
{
  uint64_t max_cap = 0U;

  return max_cap;
}

/**
  * @brief  USBD_MTP_Itf_GetFreeSpaceInBytes
  *         Get free space in bytes in SD card
  * @param  None
  * @retval free space in bytes
  */
static uint64_t USBD_MTP_Itf_GetFreeSpaceInBytes(void)
{
  uint64_t f_space_inbytes = 0U;

  return f_space_inbytes;
}

/**
  * @brief  USBD_MTP_Itf_GetNewIndex
  *         Create new object handle
  * @param  objformat: object format
  * @retval object handle
  */
static uint32_t USBD_MTP_Itf_GetNewIndex(uint16_t objformat)
{
  uint32_t n_index = 0U;
  UNUSED(objformat);

  return n_index;
}

/**
  * @brief  USBD_MTP_Itf_WriteData
  *         Write file data to SD card
  * @param  len: size of data to write
  * @param  buff: data to write in SD card
  * @retval None
  */
static void USBD_MTP_Itf_WriteData(uint16_t len, uint8_t *buff)
{
  UNUSED(len);
  UNUSED(buff);

  return;
}

/**
  * @brief  USBD_MTP_Itf_GetContainerLength
  *         Get length of generic container
  * @param  Param1: object handle
  * @retval length of generic container
  */
static uint32_t USBD_MTP_Itf_GetContainerLength(uint32_t Param1)
{
  uint32_t length = 0U;
  UNUSED(Param1);

  return length;
}

/**
  * @brief  USBD_MTP_Itf_DeleteObject
  *         delete object from SD card
  * @param  Param1: object handle (file/folder index)
  * @retval response code
  */
static uint16_t USBD_MTP_Itf_DeleteObject(uint32_t Param1)
{
  uint16_t rep_code = 0U;
  UNUSED(Param1);

  return rep_code;
}

/**
  * @brief  USBD_MTP_Get_idx_to_delete
  *         Get all files/foldres index to delete with descending order ( max depth)
  * @param  Param: object handle (file/folder index)
  * @param  tab: pointer to list of files/folders to delete
  * @retval Number of files/folders to delete
  */
/* static uint32_t USBD_MTP_Get_idx_to_delete(uint32_t Param, uint8_t *tab)
{
  uint32_t cnt = 0U;

  return cnt;
}
*/

/**
  * @brief  USBD_MTP_Itf_ReadData
  *         Read data from SD card
  * @param  Param1: object handle
  * @param  buff: pointer to data to be read
  * @param  temp_length: current data size read
  * @retval necessary information for next read/finish reading
  */
static uint32_t USBD_MTP_Itf_ReadData(uint32_t Param1, uint8_t *buff, MTP_DataLengthTypeDef *data_length)
{
  UNUSED(Param1);
  UNUSED(buff);
  UNUSED(data_length);

  return 0U;
}

/**
  * @brief  USBD_MTP_Itf_Cancel
  *         Close opened folder/file while cancelling transaction
  * @param  MTP_ResponsePhase: MTP current state
  * @retval None
  */
static void USBD_MTP_Itf_Cancel(uint32_t Phase)
{
  UNUSED(Phase);

  /* Make sure to close open file while canceling transaction */

  return;
}
