/**
  ******************************************************************************
  * @file    usbd_composite_builder.h
  * @author  MCD Application Team
  * @brief   Header for the usbd_composite_builder.c file
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
#ifndef __USBD_COMPOSITE_BUILDER_H__
#define __USBD_COMPOSITE_BUILDER_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include  "usbd_ioreq.h"

#if USBD_CMPSIT_ACTIVATE_HID == 1U
#include "usbd_hid.h"
#endif /* USBD_CMPSIT_ACTIVATE_HID */

#if USBD_CMPSIT_ACTIVATE_MSC == 1U
#include "usbd_msc.h"
#endif /* USBD_CMPSIT_ACTIVATE_MSC */

#if USBD_CMPSIT_ACTIVATE_CDC == 1U
#include "usbd_cdc.h"
#endif /* USBD_CMPSIT_ACTIVATE_CDC */

#if USBD_CMPSIT_ACTIVATE_DFU == 1U
#include "usbd_dfu.h"
#endif /* USBD_CMPSIT_ACTIVATE_DFU */

#if USBD_CMPSIT_ACTIVATE_RNDIS == 1U
#include "usbd_cdc_rndis.h"
#endif /* USBD_CMPSIT_ACTIVATE_RNDIS */

#if USBD_CMPSIT_ACTIVATE_CDC_ECM == 1U
#include "usbd_cdc_ecm.h"

#ifndef __USBD_CDC_ECM_IF_H
#include "usbd_cdc_ecm_if_template.h"
#endif /* __USBD_CDC_ECM_IF_H */
#endif /* USBD_CMPSIT_ACTIVATE_CDC_ECM */

#if USBD_CMPSIT_ACTIVATE_AUDIO == 1
#include "usbd_audio.h"
#endif /* USBD_CMPSIT_ACTIVATE_AUDIO */

#if USBD_CMPSIT_ACTIVATE_CUSTOMHID == 1
#include "usbd_customhid.h"
#endif /* USBD_CMPSIT_ACTIVATE_CUSTOMHID */

#if USBD_CMPSIT_ACTIVATE_VIDEO == 1
#include "usbd_video.h"
#endif /* USBD_CMPSIT_ACTIVATE_VIDEO */

#if USBD_CMPSIT_ACTIVATE_PRINTER == 1
#include "usbd_printer.h"
#endif /* USBD_CMPSIT_ACTIVATE_PRINTER */

#if USBD_CMPSIT_ACTIVATE_CCID == 1U
#include "usbd_ccid.h"
#endif /* USBD_CMPSIT_ACTIVATE_CCID */

#if USBD_CMPSIT_ACTIVATE_MTP == 1U
#include "usbd_mtp.h"
#endif /* USBD_CMPSIT_ACTIVATE_MTP */

/* Private defines -----------------------------------------------------------*/
/* By default all classes are deactivated, in order to activate a class
   define its value to zero  */
#ifndef USBD_CMPSIT_ACTIVATE_HID
#define USBD_CMPSIT_ACTIVATE_HID                           0U
#endif /* USBD_CMPSIT_ACTIVATE_HID */

#ifndef USBD_CMPSIT_ACTIVATE_MSC
#define USBD_CMPSIT_ACTIVATE_MSC                           0U
#endif /* USBD_CMPSIT_ACTIVATE_MSC */

#ifndef USBD_CMPSIT_ACTIVATE_DFU
#define USBD_CMPSIT_ACTIVATE_DFU                           0U
#endif /* USBD_CMPSIT_ACTIVATE_DFU */

#ifndef USBD_CMPSIT_ACTIVATE_CDC
#define USBD_CMPSIT_ACTIVATE_CDC                           0U
#endif /* USBD_CMPSIT_ACTIVATE_CDC */

#ifndef USBD_CMPSIT_ACTIVATE_CDC_ECM
#define USBD_CMPSIT_ACTIVATE_CDC_ECM                       0U
#endif /* USBD_CMPSIT_ACTIVATE_CDC_ECM */

#ifndef USBD_CMPSIT_ACTIVATE_RNDIS
#define USBD_CMPSIT_ACTIVATE_RNDIS                         0U
#endif /* USBD_CMPSIT_ACTIVATE_RNDIS */

#ifndef USBD_CMPSIT_ACTIVATE_AUDIO
#define USBD_CMPSIT_ACTIVATE_AUDIO                         0U
#endif /* USBD_CMPSIT_ACTIVATE_AUDIO */

#ifndef USBD_CMPSIT_ACTIVATE_CUSTOMHID
#define USBD_CMPSIT_ACTIVATE_CUSTOMHID                     0U
#endif /* USBD_CMPSIT_ACTIVATE_CUSTOMHID */

#ifndef USBD_CMPSIT_ACTIVATE_VIDEO
#define USBD_CMPSIT_ACTIVATE_VIDEO                         0U
#endif /* USBD_CMPSIT_ACTIVATE_VIDEO */

#ifndef USBD_CMPSIT_ACTIVATE_PRINTER
#define USBD_CMPSIT_ACTIVATE_PRINTER                       0U
#endif /* USBD_CMPSIT_ACTIVATE_PRINTER */

#ifndef USBD_CMPSIT_ACTIVATE_CCID
#define USBD_CMPSIT_ACTIVATE_CCID                          0U
#endif /* USBD_CMPSIT_ACTIVATE_CCID */

#ifndef USBD_CMPSIT_ACTIVATE_MTP
#define USBD_CMPSIT_ACTIVATE_MTP                           0U
#endif /* USBD_CMPSIT_ACTIVATE_MTP */


/* This is the maximum supported configuration descriptor size
   User may define this value in usbd_conf.h in order to optimize footprint */
#ifndef USBD_CMPST_MAX_CONFDESC_SZ
#define USBD_CMPST_MAX_CONFDESC_SZ                         300U
#endif /* USBD_CMPST_MAX_CONFDESC_SZ */

#ifndef USBD_CONFIG_STR_DESC_IDX
#define USBD_CONFIG_STR_DESC_IDX                           4U
#endif /* USBD_CONFIG_STR_DESC_IDX */

/* Exported types ------------------------------------------------------------*/
/* USB Iad descriptors structure */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bFirstInterface;
  uint8_t           bInterfaceCount;
  uint8_t           bFunctionClass;
  uint8_t           bFunctionSubClass;
  uint8_t           bFunctionProtocol;
  uint8_t           iFunction;
} USBD_IadDescTypeDef;

/* USB interface descriptors structure */
typedef struct
{
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bInterfaceNumber;
  uint8_t           bAlternateSetting;
  uint8_t           bNumEndpoints;
  uint8_t           bInterfaceClass;
  uint8_t           bInterfaceSubClass;
  uint8_t           bInterfaceProtocol;
  uint8_t           iInterface;
} USBD_IfDescTypeDef;

#if (USBD_CMPSIT_ACTIVATE_CDC == 1) || (USBD_CMPSIT_ACTIVATE_RNDIS == 1) || (USBD_CMPSIT_ACTIVATE_CDC_ECM == 1)
typedef struct
{
  /*
   * CDC Class specification revision 1.2
   * Table 15: Class-Specific Descriptor Header Format
   */
  /* Header Functional Descriptor */
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bDescriptorSubtype;
  uint16_t          bcdCDC;
} __PACKED USBD_CDCHeaderFuncDescTypeDef;

typedef struct
{
  /* Call Management Functional Descriptor */
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bDescriptorSubtype;
  uint8_t           bmCapabilities;
  uint8_t           bDataInterface;
} USBD_CDCCallMgmFuncDescTypeDef;

typedef struct
{
  /* ACM Functional Descriptor */
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bDescriptorSubtype;
  uint8_t           bmCapabilities;
} USBD_CDCACMFuncDescTypeDef;

typedef struct
{
  /*
   * CDC Class specification revision 1.2
   * Table 16: Union Interface Functional Descriptor
   */
  /* Union Functional Descriptor */
  uint8_t           bLength;
  uint8_t           bDescriptorType;
  uint8_t           bDescriptorSubtype;
  uint8_t           bMasterInterface;
  uint8_t           bSlaveInterface;
} USBD_CDCUnionFuncDescTypeDef;

#endif /* (USBD_CMPSIT_ACTIVATE_CDC == 1) || (USBD_CMPSIT_ACTIVATE_RNDIS == 1)  || (USBD_CMPSIT_ACTIVATE_CDC_ECM == 1)*/

extern USBD_ClassTypeDef  USBD_CMPSIT;

/* Exported functions prototypes ---------------------------------------------*/
uint8_t  USBD_CMPSIT_AddToConfDesc(USBD_HandleTypeDef *pdev);

#ifdef USE_USBD_COMPOSITE
uint8_t  USBD_CMPSIT_AddClass(USBD_HandleTypeDef *pdev,
                              USBD_ClassTypeDef *pclass,
                              USBD_CompositeClassTypeDef class,
                              uint8_t cfgidx);

uint32_t  USBD_CMPSIT_SetClassID(USBD_HandleTypeDef *pdev,
                                 USBD_CompositeClassTypeDef Class,
                                 uint32_t Instance);

uint32_t  USBD_CMPSIT_GetClassID(USBD_HandleTypeDef *pdev,
                                 USBD_CompositeClassTypeDef Class,
                                 uint32_t Instance);
#endif /* USE_USBD_COMPOSITE */

uint8_t USBD_CMPST_ClearConfDesc(USBD_HandleTypeDef *pdev);

/* Private macro -----------------------------------------------------------*/
#define __USBD_CMPSIT_SET_EP(epadd, eptype, epsize, HSinterval, FSinterval) \
  do { \
    /* Append Endpoint descriptor to Configuration descriptor */ \
    pEpDesc = ((USBD_EpDescTypeDef*)((uint32_t)pConf + *Sze)); \
    pEpDesc->bLength            = (uint8_t)sizeof(USBD_EpDescTypeDef); \
    pEpDesc->bDescriptorType    = USB_DESC_TYPE_ENDPOINT; \
    pEpDesc->bEndpointAddress   = (epadd); \
    pEpDesc->bmAttributes       = (eptype); \
    pEpDesc->wMaxPacketSize     = (uint16_t)(epsize); \
    if(speed == (uint8_t)USBD_SPEED_HIGH) \
    { \
      pEpDesc->bInterval        = HSinterval; \
    } \
    else \
    { \
      pEpDesc->bInterval        = FSinterval;   \
    } \
    *Sze += (uint32_t)sizeof(USBD_EpDescTypeDef); \
  } while(0)

#define __USBD_CMPSIT_SET_IF(ifnum, alt, eps, class, subclass, protocol, istring) \
  do { \
    /* Interface Descriptor */ \
    pIfDesc = ((USBD_IfDescTypeDef*)((uint32_t)pConf + *Sze)); \
    pIfDesc->bLength = (uint8_t)sizeof(USBD_IfDescTypeDef); \
    pIfDesc->bDescriptorType = USB_DESC_TYPE_INTERFACE; \
    pIfDesc->bInterfaceNumber = ifnum; \
    pIfDesc->bAlternateSetting = alt; \
    pIfDesc->bNumEndpoints = eps; \
    pIfDesc->bInterfaceClass = class; \
    pIfDesc->bInterfaceSubClass = subclass; \
    pIfDesc->bInterfaceProtocol = protocol; \
    pIfDesc->iInterface = istring; \
    *Sze += (uint32_t)sizeof(USBD_IfDescTypeDef); \
  } while(0)

#ifdef __cplusplus
}
#endif

#endif  /* __USBD_COMPOSITE_BUILDER_H__ */

/**
  * @}
  */

