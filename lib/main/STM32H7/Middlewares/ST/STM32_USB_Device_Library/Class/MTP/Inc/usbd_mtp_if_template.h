/**
  ******************************************************************************
  * @file    usbd_mtp_if_template.h
  * @author  MCD Application Team
  * @brief   Header file for the usbd_mtp_if_template.c file.
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
#ifndef __USBD_MTP_IF_TEMPLATE_H
#define __USBD_MTP_IF_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_mtp.h"

/* Exported Define -----------------------------------------------------------*/
#define USBD_MTP_DEVICE_PROP_SUPPORTED                           1U
#define USBD_MTP_CAPTURE_FORMAT_SUPPORTED                        1U
#define USBD_MTP_VEND_EXT_DESC_SUPPORTED                         1U
#define USBD_MTP_EVENTS_SUPPORTED                                1U

#if USBD_MTP_EVENTS_SUPPORTED  == 1
#define SUPP_EVENTS_LEN                                        (uint8_t)((uint8_t)sizeof(SuppEvents) / 2U)
#else
#define SUPP_EVENTS_LEN                                         0U
#endif /* USBD_MTP_EVENTS_SUPPORTED */

#if USBD_MTP_VEND_EXT_DESC_SUPPORTED == 1
#define VEND_EXT_DESC_LEN                                      (sizeof(VendExtDesc) / 2U)
#else
#define VEND_EXT_DESC_LEN                                       0U
#endif /* USBD_MTP_VEND_EXT_DESC_SUPPORTED */

#if USBD_MTP_CAPTURE_FORMAT_SUPPORTED  == 1
#define SUPP_CAPT_FORMAT_LEN                                   (uint8_t)((uint8_t)sizeof(SuppCaptFormat) / 2U)
#else
#define SUPP_CAPT_FORMAT_LEN                                    0U
#endif /* USBD_MTP_CAPTURE_FORMAT_SUPPORTED */

#if USBD_MTP_DEVICE_PROP_SUPPORTED == 1
#define SUPP_DEVICE_PROP_LEN                                   (uint8_t)((uint8_t)sizeof(DevicePropSupp) / 2U)
#else
#define SUPP_DEVICE_PROP_LEN                                    0U
#endif /* USBD_MTP_DEVICE_PROP_SUPPORTED */

#define MTP_IF_SCRATCH_BUFF_SZE                                1024U

/* Exported types ------------------------------------------------------------*/
extern USBD_MTP_ItfTypeDef USBD_MTP_fops;

/* Exported macros -----------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

static const uint16_t Manuf[] = {'S', 'T', 'M', 0}; /* last 2 bytes must be 0*/
static const uint16_t Model[] = {'S', 'T', 'M', '3', '2', 0}; /* last 2 bytes must be 0*/
static const uint16_t VendExtDesc[] = {'m', 'i', 'c', 'r', 'o', 's', 'o', 'f', 't', '.',
                                       'c', 'o', 'm', ':', ' ', '1', '.', '0', ';', ' ', 0
                                      };  /* last 2 bytes must be 0*/
/*SerialNbr shall be 32 character hexadecimal string for legacy compatibility reasons */
static const uint16_t SerialNbr[] = {'0', '0', '0', '0', '1', '0', '0', '0', '0', '1', '0', '0', '0', '0',
                                     '1', '0', '0', '0', '0', '1', '0', '0', '0', '0', '1', '0', '0', '0',
                                     '0', '1', '0', '0', 0
                                    };  /* last 2 bytes must be 0*/
static const uint16_t DeviceVers[] = {'V', '1', '.', '0', '0', 0}; /* last 2 bytes must be 0*/

static const uint16_t DefaultFileName[] = {'N', 'e', 'w', ' ', 'F', 'o', 'l', 'd', 'e', 'r', 0};

static const uint16_t DevicePropDefVal[] = {'S', 'T', 'M', '3', '2', 0}; /* last 2 bytes must be 0*/
static const uint16_t DevicePropCurDefVal[] = {'S', 'T', 'M', '3', '2', ' ', 'V', '1', '.', '0', 0};


/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBD_MTP_IF_TEMPLATE_H */

