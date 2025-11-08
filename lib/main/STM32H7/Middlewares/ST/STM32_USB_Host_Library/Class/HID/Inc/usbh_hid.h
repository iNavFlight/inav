/**
  ******************************************************************************
  * @file    usbh_hid.h
  * @author  MCD Application Team
  * @brief   This file contains all the prototypes for the usbh_hid.c
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
#ifndef __USBH_HID_H
#define __USBH_HID_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_core.h"
#include "usbh_hid_mouse.h"
#include "usbh_hid_keybd.h"

/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_HID_CLASS
  * @{
  */

/** @defgroup USBH_HID_CORE
  * @brief This file is the Header file for usbh_hid.c
  * @{
  */


/** @defgroup USBH_HID_CORE_Exported_Types
  * @{
  */

#define HID_MIN_POLL                                10U
#define HID_REPORT_SIZE                             16U
#define HID_MAX_USAGE                               10U
#define HID_MAX_NBR_REPORT_FMT                      10U
#define HID_QUEUE_SIZE                              10U

#define  HID_ITEM_LONG                              0xFEU

#define  HID_ITEM_TYPE_MAIN                         0x00U
#define  HID_ITEM_TYPE_GLOBAL                       0x01U
#define  HID_ITEM_TYPE_LOCAL                        0x02U
#define  HID_ITEM_TYPE_RESERVED                     0x03U


#define  HID_MAIN_ITEM_TAG_INPUT                    0x08U
#define  HID_MAIN_ITEM_TAG_OUTPUT                   0x09U
#define  HID_MAIN_ITEM_TAG_COLLECTION               0x0AU
#define  HID_MAIN_ITEM_TAG_FEATURE                  0x0BU
#define  HID_MAIN_ITEM_TAG_ENDCOLLECTION            0x0CU


#define  HID_GLOBAL_ITEM_TAG_USAGE_PAGE             0x00U
#define  HID_GLOBAL_ITEM_TAG_LOG_MIN                0x01U
#define  HID_GLOBAL_ITEM_TAG_LOG_MAX                0x02U
#define  HID_GLOBAL_ITEM_TAG_PHY_MIN                0x03U
#define  HID_GLOBAL_ITEM_TAG_PHY_MAX                0x04U
#define  HID_GLOBAL_ITEM_TAG_UNIT_EXPONENT          0x05U
#define  HID_GLOBAL_ITEM_TAG_UNIT                   0x06U
#define  HID_GLOBAL_ITEM_TAG_REPORT_SIZE            0x07U
#define  HID_GLOBAL_ITEM_TAG_REPORT_ID              0x08U
#define  HID_GLOBAL_ITEM_TAG_REPORT_COUNT           0x09U
#define  HID_GLOBAL_ITEM_TAG_PUSH                   0x0AU
#define  HID_GLOBAL_ITEM_TAG_POP                    0x0BU


#define  HID_LOCAL_ITEM_TAG_USAGE                   0x00U
#define  HID_LOCAL_ITEM_TAG_USAGE_MIN               0x01U
#define  HID_LOCAL_ITEM_TAG_USAGE_MAX               0x02U
#define  HID_LOCAL_ITEM_TAG_DESIGNATOR_INDEX        0x03U
#define  HID_LOCAL_ITEM_TAG_DESIGNATOR_MIN          0x04U
#define  HID_LOCAL_ITEM_TAG_DESIGNATOR_MAX          0x05U
#define  HID_LOCAL_ITEM_TAG_STRING_INDEX            0x07U
#define  HID_LOCAL_ITEM_TAG_STRING_MIN              0x08U
#define  HID_LOCAL_ITEM_TAG_STRING_MAX              0x09U
#define  HID_LOCAL_ITEM_TAG_DELIMITER               0x0AU


/* States for HID State Machine */
typedef enum
{
  USBH_HID_INIT = 0,
  USBH_HID_IDLE,
  USBH_HID_SEND_DATA,
  USBH_HID_BUSY,
  USBH_HID_GET_DATA,
  USBH_HID_SYNC,
  USBH_HID_POLL,
  USBH_HID_ERROR,
}
USBH_HID_StateTypeDef;

typedef enum
{
  USBH_HID_REQ_INIT = 0,
  USBH_HID_REQ_IDLE,
  USBH_HID_REQ_GET_REPORT_DESC,
  USBH_HID_REQ_GET_HID_DESC,
  USBH_HID_REQ_SET_IDLE,
  USBH_HID_REQ_SET_PROTOCOL,
  USBH_HID_REQ_SET_REPORT,

}
HID_CtlStateTypeDef;

typedef enum
{
  HID_MOUSE    = 0x01,
  HID_KEYBOARD = 0x02,
  HID_UNKNOWN = 0xFF,
}
HID_TypeTypeDef;


typedef  struct  _HID_ReportData
{
  uint8_t   ReportID;
  uint8_t   ReportType;
  uint16_t  UsagePage;
  uint32_t  Usage[HID_MAX_USAGE];
  uint32_t  NbrUsage;
  uint32_t  UsageMin;
  uint32_t  UsageMax;
  int32_t   LogMin;
  int32_t   LogMax;
  int32_t   PhyMin;
  int32_t   PhyMax;
  int32_t   UnitExp;
  uint32_t  Unit;
  uint32_t  ReportSize;
  uint32_t  ReportCnt;
  uint32_t  Flag;
  uint32_t  PhyUsage;
  uint32_t  AppUsage;
  uint32_t  LogUsage;
}
HID_ReportDataTypeDef;

typedef  struct  _HID_ReportIDTypeDef
{
  uint8_t  Size;         /* Report size return by the device id            */
  uint8_t  ReportID;     /* Report Id                                      */
  uint8_t  Type;         /* Report Type (INPUT/OUTPUT/FEATURE)             */
} HID_ReportIDTypeDef;

typedef struct  _HID_CollectionTypeDef
{
  uint32_t                       Usage;
  uint8_t                        Type;
  struct _HID_CollectionTypeDef  *NextPtr;
} HID_CollectionTypeDef;


typedef  struct  _HID_AppCollectionTypeDef
{
  uint32_t               Usage;
  uint8_t                Type;
  uint8_t                NbrReportFmt;
  HID_ReportDataTypeDef  ReportData[HID_MAX_NBR_REPORT_FMT];
} HID_AppCollectionTypeDef;


typedef struct _HIDDescriptor
{
  uint8_t   bLength;
  uint8_t   bDescriptorType;
  uint16_t  bcdHID;               /* indicates what endpoint this descriptor is describing */
  uint8_t   bCountryCode;        /* specifies the transfer type. */
  uint8_t   bNumDescriptors;     /* specifies the transfer type. */
  uint8_t   bReportDescriptorType;    /* Maximum Packet Size this endpoint is capable of sending or receiving */
  uint16_t  wItemLength;          /* is used to specify the polling interval of certain transfers. */
}
HID_DescTypeDef;


typedef struct
{
  uint8_t  *buf;
  uint16_t  head;
  uint16_t tail;
  uint16_t size;
  uint8_t  lock;
} FIFO_TypeDef;


/* Structure for HID process */
typedef struct _HID_Process
{
  uint8_t              OutPipe;
  uint8_t              InPipe;
  USBH_HID_StateTypeDef     state;
  uint8_t              OutEp;
  uint8_t              InEp;
  HID_CtlStateTypeDef  ctl_state;
  FIFO_TypeDef         fifo;
  uint8_t              *pData;
  uint16_t             length;
  uint8_t              ep_addr;
  uint16_t             poll;
  uint32_t             timer;
  uint8_t              DataReady;
  HID_DescTypeDef      HID_Desc;
  USBH_StatusTypeDef(* Init)(USBH_HandleTypeDef *phost);
}
HID_HandleTypeDef;

/**
  * @}
  */

/** @defgroup USBH_HID_CORE_Exported_Defines
  * @{
  */

#define USB_HID_GET_REPORT                            0x01U
#define USB_HID_GET_IDLE                              0x02U
#define USB_HID_GET_PROTOCOL                          0x03U
#define USB_HID_SET_REPORT                            0x09U
#define USB_HID_SET_IDLE                              0x0AU
#define USB_HID_SET_PROTOCOL                          0x0BU




/* HID Class Codes */
#define USB_HID_CLASS                                 0x03U

/* Interface Descriptor field values for HID Boot Protocol */
#define HID_BOOT_CODE                                 0x01U
#define HID_KEYBRD_BOOT_CODE                          0x01U
#define HID_MOUSE_BOOT_CODE                           0x02U


/**
  * @}
  */

/** @defgroup USBH_HID_CORE_Exported_Macros
  * @{
  */
/**
  * @}
  */

/** @defgroup USBH_HID_CORE_Exported_Variables
  * @{
  */
extern USBH_ClassTypeDef  HID_Class;
#define USBH_HID_CLASS    &HID_Class
/**
  * @}
  */

/** @defgroup USBH_HID_CORE_Exported_FunctionsPrototype
  * @{
  */

USBH_StatusTypeDef USBH_HID_SetReport(USBH_HandleTypeDef *phost,
                                      uint8_t reportType,
                                      uint8_t reportId,
                                      uint8_t *reportBuff,
                                      uint8_t reportLen);

USBH_StatusTypeDef USBH_HID_GetReport(USBH_HandleTypeDef *phost,
                                      uint8_t reportType,
                                      uint8_t reportId,
                                      uint8_t *reportBuff,
                                      uint8_t reportLen);

USBH_StatusTypeDef USBH_HID_GetHIDReportDescriptor(USBH_HandleTypeDef *phost,
                                                   uint16_t length);

USBH_StatusTypeDef USBH_HID_GetHIDDescriptor(USBH_HandleTypeDef *phost,
                                             uint16_t length);

USBH_StatusTypeDef USBH_HID_SetIdle(USBH_HandleTypeDef *phost,
                                    uint8_t duration,
                                    uint8_t reportId);

USBH_StatusTypeDef USBH_HID_SetProtocol(USBH_HandleTypeDef *phost,
                                        uint8_t protocol);

void USBH_HID_EventCallback(USBH_HandleTypeDef *phost);

HID_TypeTypeDef USBH_HID_GetDeviceType(USBH_HandleTypeDef *phost);

uint8_t USBH_HID_GetPollInterval(USBH_HandleTypeDef *phost);

void USBH_HID_FifoInit(FIFO_TypeDef *f, uint8_t *buf, uint16_t size);

uint16_t  USBH_HID_FifoRead(FIFO_TypeDef *f, void *buf, uint16_t  nbytes);

uint16_t  USBH_HID_FifoWrite(FIFO_TypeDef *f, void *buf, uint16_t nbytes);

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBH_HID_H */

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

