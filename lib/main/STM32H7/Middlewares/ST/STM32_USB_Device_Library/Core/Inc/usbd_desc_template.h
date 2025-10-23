/**
  ******************************************************************************
  * @file    usbd_desc_template.h
  * @author  MCD Application Team
  * @brief   Header for usbd_desc_template.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_DESC_TEMPLATE_H
#define __USBD_DESC_TEMPLATE_H

/* Includes ------------------------------------------------------------------*/
#include "usbd_def.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define         DEVICE_ID1          (UID_BASE)
#define         DEVICE_ID2          (UID_BASE + 0x4U)
#define         DEVICE_ID3          (UID_BASE + 0x8U)

/*
 * USB Billboard Class USER string desc Defines Template
 * index should start form 0x10 to avoid using the reserved device string desc indexes
 */
#if (USBD_CLASS_USER_STRING_DESC == 1)
#define USBD_BB_IF_STRING_INDEX         0x10U
#define USBD_BB_URL_STRING_INDEX        0x11U
#define USBD_BB_ALTMODE0_STRING_INDEX   0x12U
#define USBD_BB_ALTMODE1_STRING_INDEX   0x13U
/* Add Specific USER string Desc */
#define USBD_BB_IF_STR_DESC           (uint8_t *)"STM32 BillBoard Interface"
#define USBD_BB_URL_STR_DESC          (uint8_t *)"www.st.com"
#define USBD_BB_ALTMODE0_STR_DESC     (uint8_t *)"STM32 Alternate0 Mode"
#define USBD_BB_ALTMODE1_STR_DESC     (uint8_t *)"STM32 Alternate1 Mode"
#endif /* USBD_CLASS_USER_STRING_DESC  */

#define  USB_SIZ_STRING_SERIAL       0x1AU

#if (USBD_LPM_ENABLED == 1)
#define  USB_SIZ_BOS_DESC            0x0CU
#elif (USBD_CLASS_BOS_ENABLED == 1)
#define  USB_SIZ_BOS_DESC            0x5DU
#endif /* USBD_LPM_ENABLED  */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
extern USBD_DescriptorsTypeDef XXX_Desc; /* Replace 'XXX_Desc' with your active USB device class, ex: HID_Desc */

#endif /* __USBD_DESC_TEMPLATE_H*/

