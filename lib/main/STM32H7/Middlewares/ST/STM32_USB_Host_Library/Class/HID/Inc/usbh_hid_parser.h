/**
  ******************************************************************************
  * @file    usbh_hid_parser.c
  * @author  MCD Application Team
  * @brief   This file is the header file of the usbh_hid_parser.c
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

/* Define to prevent recursive -----------------------------------------------*/
#ifndef __USBH_HID_PARSER_H
#define __USBH_HID_PARSER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbh_hid.h"
#include "usbh_hid_usage.h"

/** @addtogroup USBH_LIB
  * @{
  */

/** @addtogroup USBH_CLASS
  * @{
  */

/** @addtogroup USBH_HID_CLASS
  * @{
  */

/** @defgroup USBH_HID_PARSER
  * @brief This file is the Header file for usbh_hid_parser.c
  * @{
  */


/** @defgroup USBH_HID_PARSER_Exported_Types
  * @{
  */
typedef struct
{
  uint8_t  *data;
  uint32_t size;
  uint8_t  shift;
  uint8_t  count;
  uint8_t  sign;
  uint32_t logical_min;  /*min value device can return*/
  uint32_t logical_max;  /*max value device can return*/
  uint32_t physical_min; /*min vale read can report*/
  uint32_t physical_max; /*max value read can report*/
  uint32_t resolution;
}
HID_Report_ItemTypedef;


uint32_t HID_ReadItem(HID_Report_ItemTypedef *ri, uint8_t ndx);
uint32_t HID_WriteItem(HID_Report_ItemTypedef *ri, uint32_t value, uint8_t ndx);


/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif /* __USBH_HID_PARSER_H */

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
