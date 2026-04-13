/**
  ******************************************************************************
  * @file    mx_wifi_conf_template.h
  * @author  MCD Application Team
  * @brief   Header for mx_wifi_conf_template module
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_WIFI_CONF_TEMPLATE_H
#define MX_WIFI_CONF_TEMPLATE_H

#ifdef __cplusplus
extern "C" {
#endif

/* DEBUG LOG */
//#define MX_WIFI_API_DEBUG
//#define MX_WIFI_IPC_DEBUG
//#define MX_WIFI_HCI_DEBUG
//#define MX_WIFI_SLIP_DEBUG
//#define MX_WIFI_IO_DEBUG


#define MX_WIFI_PRODUCT_NAME                        ("MXCHIP-WIFI")
#define MX_WIFI_PRODUCT_ID                          ("EMW3080B")

#ifndef MX_WIFI_USE_SPI
#define MX_WIFI_USE_SPI                             (0)
#endif

#ifndef MX_WIFI_USE_CMSIS_OS
#define MX_WIFI_USE_CMSIS_OS                        (0)
#endif

#ifndef MX_WIFI_UART_BAUDRATE
#define MX_WIFI_UART_BAUDRATE                       (115200*2)
#endif

#ifndef MX_WIFI_DATA_SIZE
#define MX_WIFI_DATA_SIZE                           (1024*2)
#endif

#define MX_WIFI_TIMEOUT                             (10000)
#define MX_WIFI_MAX_SOCKET_NBR                      (8)
#define MX_WIFI_MAX_DETECTED_AP                     (10)

#define MX_WIFI_MAX_SSID_NAME_SIZE                  32
#define MX_WIFI_MAX_PSWD_NAME_SIZE                  64

#define MX_WIFI_PRODUCT_NAME_SIZE                   32
#define MX_WIFI_PRODUCT_ID_SIZE                     32

#define MX_WIFI_FW_REV_SIZE                         24
#define MX_WIFI_API_REV_SIZE                        16
#define MX_WIFI_STACK_REV_SIZE                      16
#define MX_WIFI_RTOS_REV_SIZE                       16


#ifdef __cplusplus
}
#endif

#endif /* MX_WIFI_CONF_TEMPLATE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
