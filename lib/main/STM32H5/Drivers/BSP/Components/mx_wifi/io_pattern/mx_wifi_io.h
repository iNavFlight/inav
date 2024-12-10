/**
  ******************************************************************************
  * @file    mx_wifi_io.h
  * @author  MCD Application Team
  * @brief   Header for mx_wifi IO module
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_WIFI_IO_H
#define MX_WIFI_IO_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "mx_wifi.h"


/**
  * @brief  Create internal connections to the had hoc bus.
  * @param  ll_drv_context
  * @retval 0 in case of success
  */
int32_t mxwifi_probe(void **ll_drv_context);

/* Retrieve the handle of the abstract object previously connected to the bus in use. */
MX_WIFIObject_t *wifi_obj_get(void);

/* Protocol functions for communication with the WiFi module. */
void process_txrx_poll(uint32_t timeout);

/* The handler for the WiFi module SPI interrupts (NOTIFY, FLOW). */
void mxchip_WIFI_ISR(uint16_t isr_source);

/* The handler of SPI transfer with the WiFi module. */
void HAL_SPI_TransferCallback(void *hspi);

/* The handler for the WiFi module UART interrupts (byte received). */
void mxchip_WIFI_ISR_UART(void *huart);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* MX_WIFI_IO_H */
