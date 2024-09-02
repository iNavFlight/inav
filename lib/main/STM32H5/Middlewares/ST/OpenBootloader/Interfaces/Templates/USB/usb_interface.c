/**
  ******************************************************************************
  * @file    usb_interface.c
  * @author  MCD Application Team
  * @brief   Contains USB protocol commands
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2019-2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "usb_interface.h"
#include "app_usbx_device.h"
#include "app_azure_rtos.h"
#include "openbl_core.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t UsbDetected = 0U;

/* Exported variables --------------------------------------------------------*/
uint8_t UsbSofDetected = 0U;

PCD_HandleTypeDef hpcd_USB_OTG_FS;

/* External variables --------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Exported functions --------------------------------------------------------*/

/**
  * @brief  This function is used to configure USB pins and then initialize the used USB instance.
  * @retval None.
  */
void OPENBL_USB_Configuration(void)
{
}

/**
  * @brief  This function is used to De-initialize the USB pins and instance.
  * @retval None.
  */
void OPENBL_USB_DeInit(void)
{
}

/**
  * @brief  This function is used to detect if there is any activity on USB protocol.
  * @retval Returns 1 if interface is detected else 0.
  */
uint8_t OPENBL_USB_ProtocolDetection(void)
{
  return UsbDetected;
}

/**
  * @brief  Gets the page of a given address.
  * @param  Address Address of the FLASH Memory.
  * @retval The page of a given address.
  */
uint32_t OPENBL_USB_GetPage(uint32_t Address)
{
  uint32_t page;

  return page;
}
