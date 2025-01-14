/**
  ******************************************************************************
  * @file    usbd_mtp.h
  * @author  MCD Application Team
  * @brief   header file for the usbd_mtp.c file.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USB_MTP_H
#define __USB_MTP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_ioreq.h"
#include "usbd_ctlreq.h"

/** @addtogroup STM32_USB_DEVICE_LIBRARY
  * @{
  */

/** @defgroup USBD_MTP
  * @brief This file is the header file for usbd_mtp.c
  * @{
  */


/** @defgroup USBD_MTP_Exported_Defines
  * @{
  */
#ifndef MTP_IN_EP
#define MTP_IN_EP                                                   0x81U       /* EP1 for data IN */
#endif /* MTP_IN_EP */
#ifndef MTP_OUT_EP
#define MTP_OUT_EP                                                  0x01U       /* EP1 for data OUT */
#endif /* MTP_OUT_EP */
#ifndef MTP_CMD_EP
#define MTP_CMD_EP                                                  0x82U       /* EP2 for MTP commands */
#endif /* MTP_CMD_EP */

#ifndef MTP_CMD_ITF_NBR
#define MTP_CMD_ITF_NBR                                             0x00U       /* Command Interface Number 0 */
#endif /* MTP_CMD_ITF_NBR */

#ifndef MTP_COM_ITF_NBR
#define MTP_COM_ITF_NBR                                             0x01U       /* Communication Interface Number 0 */
#endif /* MTP_CMD_ITF_NBR */

#ifndef MTP_HS_BINTERVAL
#define MTP_HS_BINTERVAL                                            0x10U
#endif /* MTP_HS_BINTERVAL */

#ifndef MTP_FS_BINTERVAL
#define MTP_FS_BINTERVAL                                            0x10U
#endif /* MTP_FS_BINTERVAL */

#define MTP_DATA_MAX_HS_PACKET_SIZE                                 512U
#define MTP_DATA_MAX_FS_PACKET_SIZE                                 64U         /* Endpoint IN & OUT Packet size */
#define MTP_CMD_PACKET_SIZE                                         8U          /* Control Endpoint Packet size */

#define MTP_MEDIA_PACKET                                            512U
#define MTP_CONT_HEADER_SIZE                                        12U

#define MTP_CONFIG_DESC_SIZ                                         39U
#define MTP_INTERFACE_DESC_SIZE                                     0x09U
#define USB_MTP_INTRERFACE_CLASS                                    0x06U
#define USB_MTP_INTRERFACE_SUB_CLASS                                0x01U
#define USB_MTP_INTRERFACE_PROTOCOL                                 0x01U
#define MTP_ENDPOINT_DESC_SIZE                                      0x07U

/*---------------------------------------------------------------------*/
/*  MTP definitions                                                    */
/*---------------------------------------------------------------------*/

/* MTP class requests */
#define MTP_REQ_CANCEL                                              0x64U
#define MTP_REQ_GET_EXT_EVENT_DATA                                  0x65U
#define MTP_REQ_RESET                                               0x66U
#define MTP_REQ_GET_DEVICE_STATUS                                   0x67U


/* Max info items size */
#define MTP_SUPPORTED_OPERATIONS_NBR                                100U
#define MTP_SUPPORTED_EVENTS_NBR                                    100U
#define MTP_SUPPORTED_PROPRIETIES_NBR                               100U
#define MTP_CAPTURE_FORMATS_NBR                                     100U
#define MTP_IMAGE_FORMATS_NBR                                       100U
#define MTP_MAX_STR_SIZE                                            255U

/* MTP response code */
#define MTP_RESPONSE_OK                                             0x2001U
#define MTP_RESPONSE_GENERAL_ERROR                                  0x2002U
#define MTP_RESPONSE_PARAMETER_NOT_SUPPORTED                        0x2006U
#define MTP_RESPONSE_INCOMPLETE_TRANSFER                            0x2007U
#define MTP_RESPONSE_INVALID_STORAGE_ID                             0x2008U
#define MTP_RESPONSE_INVALID_OBJECT_HANDLE                          0x2009U
#define MTP_RESPONSE_DEVICEPROP_NOT_SUPPORTED                       0x200AU
#define MTP_RESPONSE_STORE_FULL                                     0x200CU
#define MTP_RESPONSE_ACCESS_DENIED                                  0x200FU
#define MTP_RESPONSE_STORE_NOT_AVAILABLE                            0x2013U
#define MTP_RESPONSE_SPECIFICATION_BY_FORMAT_NOT_SUPPORTED          0x2014U
#define MTP_RESPONSE_NO_VALID_OBJECT_INFO                           0x2015U
#define MTP_RESPONSE_DEVICE_BUSY                                    0x2019U
#define MTP_RESPONSE_INVALID_PARENT_OBJECT                          0x201AU
#define MTP_RESPONSE_INVALID_PARAMETER                              0x201DU
#define MTP_RESPONSE_SESSION_ALREADY_OPEN                           0x201EU
#define MTP_RESPONSE_TRANSACTION_CANCELLED                          0x201FU
#define MTP_RESPONSE_INVALID_OBJECT_PROP_CODE                       0xA801U
#define MTP_RESPONSE_SPECIFICATION_BY_GROUP_UNSUPPORTED             0xA807U
#define MTP_RESPONSE_OBJECT_PROP_NOT_SUPPORTED                      0xA80AU

/*
 * MTP Class specification Revision 1.1
 * Appendix A. Object Formats
 */

/* MTP Object format codes */
#define MTP_OBJ_FORMAT_UNDEFINED                                    0x3000U
#define MTP_OBJ_FORMAT_ASSOCIATION                                  0x3001U
#define MTP_OBJ_FORMAT_SCRIPT                                       0x3002U
#define MTP_OBJ_FORMAT_EXECUTABLE                                   0x3003U
#define MTP_OBJ_FORMAT_TEXT                                         0x3004U
#define MTP_OBJ_FORMAT_HTML                                         0x3005U
#define MTP_OBJ_FORMAT_DPOF                                         0x3006U
#define MTP_OBJ_FORMAT_AIFF                                         0x3007U
#define MTP_OBJ_FORMAT_WAV                                          0x3008U
#define MTP_OBJ_FORMAT_MP3                                          0x3009U
#define MTP_OBJ_FORMAT_AVI                                          0x300AU
#define MTP_OBJ_FORMAT_MPEG                                         0x300BU
#define MTP_OBJ_FORMAT_ASF                                          0x300CU
#define MTP_OBJ_FORMAT_DEFINED                                      0x3800U
#define MTP_OBJ_FORMAT_EXIF_JPEG                                    0x3801U
#define MTP_OBJ_FORMAT_TIFF_EP                                      0x3802U
#define MTP_OBJ_FORMAT_FLASHPIX                                     0x3803U
#define MTP_OBJ_FORMAT_BMP                                          0x3804U
#define MTP_OBJ_FORMAT_CIFF                                         0x3805U
#define MTP_OBJ_FORMAT_UNDEFINED_RESERVED0                          0x3806U
#define MTP_OBJ_FORMAT_GIF                                          0x3807U
#define MTP_OBJ_FORMAT_JFIF                                         0x3808U
#define MTP_OBJ_FORMAT_CD                                           0x3809U
#define MTP_OBJ_FORMAT_PICT                                         0x380AU
#define MTP_OBJ_FORMAT_PNG                                          0x380BU
#define MTP_OBJ_FORMAT_UNDEFINED_RESERVED1                          0x380CU
#define MTP_OBJ_FORMAT_TIFF                                         0x380DU
#define MTP_OBJ_FORMAT_TIFF_IT                                      0x380EU
#define MTP_OBJ_FORMAT_JP2                                          0x380FU
#define MTP_OBJ_FORMAT_JPX                                          0x3810U
#define MTP_OBJ_FORMAT_UNDEFINED_FIRMWARE                           0xB802U
#define MTP_OBJ_FORMAT_WINDOWS_IMAGE_FORMAT                         0xB881U
#define MTP_OBJ_FORMAT_UNDEFINED_AUDIO                              0xB900U
#define MTP_OBJ_FORMAT_WMA                                          0xB901U
#define MTP_OBJ_FORMAT_OGG                                          0xB902U
#define MTP_OBJ_FORMAT_AAC                                          0xB903U
#define MTP_OBJ_FORMAT_AUDIBLE                                      0xB904U
#define MTP_OBJ_FORMAT_FLAC                                         0xB906U
#define MTP_OBJ_FORMAT_UNDEFINED_VIDEO                              0xB980U
#define MTP_OBJ_FORMAT_WMV                                          0xB981U
#define MTP_OBJ_FORMAT_MP4_CONTAINER                                0xB982U
#define MTP_OBJ_FORMAT_MP2                                          0xB983U
#define MTP_OBJ_FORMAT_3GP_CONTAINER                                0xB984U
/**
  * @}
  */


/** @defgroup USBD_CORE_Exported_TypesDefinitions
  * @{
  */

/**
  * @}
  */


/* MTP Session state    */
typedef enum
{
  MTP_SESSION_NOT_OPENED = 0x00,
  MTP_SESSION_OPENED     = 0x01,
} MTP_SessionStateTypeDef;

/* MTP response phases    */
typedef enum
{
  MTP_PHASE_IDLE       = 0x00,
  MTP_RESPONSE_PHASE   = 0x01,
  MTP_READ_DATA        = 0x02,
  MTP_RECEIVE_DATA     = 0x03,
} MTP_ResponsePhaseTypeDef;

typedef struct
{
  uint32_t temp_length;
  uint32_t prv_len;
  uint32_t totallen;
  uint32_t rx_length;
  uint32_t readbytes;  /* File write/read counts */
} MTP_DataLengthTypeDef;

typedef enum
{
  RECEIVE_IDLE_STATE     = 0x00U,
  RECEIVE_COMMAND_DATA   = 0x01U,
  RECEIVE_FIRST_DATA     = 0x02U,
  RECEIVE_REST_OF_DATA   = 0x03U,
  SEND_RESPONSE          = 0x04U,
} MTP_RECEIVE_DATA_STATUS;

typedef struct
{
  uint32_t length;
  uint16_t type;
  uint16_t code;
  uint32_t trans_id;
  uint32_t Param1;
  uint32_t Param2;
  uint32_t Param3;
  uint32_t Param4;
  uint32_t Param5;
} MTP_OperationsTypeDef;

typedef struct
{
  uint32_t length;
  uint16_t type;
  uint16_t code;
  uint32_t trans_id;
  uint8_t  data[MTP_MEDIA_PACKET];
} MTP_GenericContainerTypeDef;

#if defined ( __GNUC__ )
typedef __PACKED_STRUCT
#else
__packed typedef struct
#endif /* __GNUC__ */
{
  uint32_t Storage_id;
  uint16_t ObjectFormat;
  uint16_t ProtectionStatus;
  uint32_t ObjectCompressedSize;
  uint16_t ThumbFormat;
  uint32_t ThumbCompressedSize;
  uint32_t ThumbPixWidth;
  uint32_t ThumbPixHeight;
  uint32_t ImagePixWidth;
  uint32_t ImagePixHeight;
  uint32_t ImageBitDepth;
  uint32_t ParentObject;
  uint16_t AssociationType;
  uint32_t AssociationDesc;
  uint32_t SequenceNumber;
  uint8_t  Filename_len;
  uint16_t Filename[255];
  uint32_t CaptureDate;
  uint32_t ModificationDate;
  uint8_t  Keywords;
} MTP_ObjectInfoTypeDef;

typedef struct
{
  uint32_t                     alt_setting;
  uint32_t                     dev_status;
  uint32_t                     ResponseLength;
  uint32_t                     ResponseCode;
  __IO uint16_t                MaxPcktLen;
  uint32_t                     rx_buff[MTP_MEDIA_PACKET / 4U];               /* Force 32-bit alignment */
  MTP_ResponsePhaseTypeDef     MTP_ResponsePhase;
  MTP_SessionStateTypeDef      MTP_SessionState;
  MTP_RECEIVE_DATA_STATUS      RECEIVE_DATA_STATUS;
  MTP_OperationsTypeDef        OperationsContainer;
  MTP_GenericContainerTypeDef  GenericContainer;
} USBD_MTP_HandleTypeDef;

/** @defgroup USBD_CORE_Exported_Macros
  * @{
  */

/**
  * @}
  */

/** @defgroup USBD_CORE_Exported_Variables
  * @{
  */

extern USBD_ClassTypeDef  USBD_MTP;
#define USBD_MTP_CLASS    &USBD_MTP

/**
  * @}
  */

/** @defgroup USB_CORE_Exported_Functions
  * @{
  */

typedef struct _USBD_MTP_ItfTypeDef
{
  uint8_t (*Init)(void);
  uint8_t (*DeInit)(void);
  uint32_t (*ReadData)(uint32_t Param1, uint8_t *buff, MTP_DataLengthTypeDef *data_length);
  uint16_t (*Create_NewObject)(MTP_ObjectInfoTypeDef ObjectInfo, uint32_t objhandle);

  uint32_t (*GetIdx)(uint32_t Param3, uint32_t *obj_handle);
  uint32_t (*GetParentObject)(uint32_t Param);
  uint16_t (*GetObjectFormat)(uint32_t Param);
  uint8_t (*GetObjectName_len)(uint32_t Param);
  void (*GetObjectName)(uint32_t Param, uint8_t obj_len, uint16_t *buf);
  uint32_t (*GetObjectSize)(uint32_t Param);
  uint64_t (*GetMaxCapability)(void);
  uint64_t (*GetFreeSpaceInBytes)(void);
  uint32_t (*GetNewIndex)(uint16_t objformat);
  void (*WriteData)(uint16_t len, uint8_t buff[]);
  uint32_t (*GetContainerLength)(uint32_t Param1);
  uint16_t (*DeleteObject)(uint32_t Param1);
  void (*Cancel)(uint32_t Phase);
  uint32_t                      *ScratchBuff;
  uint32_t                       ScratchBuffSze;
} USBD_MTP_ItfTypeDef;

/**
  * @}
  */
/** @defgroup USB_CORE_Exported_Functions
  * @{
  */
uint8_t  USBD_MTP_RegisterInterface(USBD_HandleTypeDef *pdev, USBD_MTP_ItfTypeDef *fops);


/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif  /* __USB_MTP_H */
/**
  * @}
  */

/**
  * @}
  */
