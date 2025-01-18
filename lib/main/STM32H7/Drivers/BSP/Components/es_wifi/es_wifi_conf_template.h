/**
  ******************************************************************************
  * @file    es_wifi_conf.h
  * @author  MCD Application Team
  * @brief   ES_WIFI configuration.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
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
#ifndef ES_WIFI_CONF_H
#define ES_WIFI_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

//#define WIFI_USE_CMSIS_OS

#ifdef WIFI_USE_CMSIS_OS
#include "cmsis_os.h"
extern osMutexId es_wifi_mutex;

#define LOCK_SPI()              osMutexWait(spi_mutex, 0)
#define UNLOCK_SPI()            osMutexRelease(spi_mutex)

#define LOCK_WIFI()             osMutexWait(es_wifi_mutex, 0)
#define UNLOCK_WIFI()           osMutexRelease(es_wifi_mutex)

#define SEM_SIGNAL(a)           osSemaphoreRelease(a)
#define SEM_WAIT(a,timeout)     osSemaphoreWait(a,timeout)

#else /* WIFI_USE_CMSIS_OS */

#define LOCK_SPI()
#define LOCK_WIFI()
#define UNLOCK_SPI()
#define UNLOCK_WIFI()
#define SEM_SIGNAL(a)
#endif /* WIFI_USE_CMSIS_OS */


#define ES_WIFI_MAX_SSID_NAME_SIZE                  32
#define ES_WIFI_MAX_PSWD_NAME_SIZE                  32
#define ES_WIFI_PRODUCT_ID_SIZE                     32
#define ES_WIFI_PRODUCT_NAME_SIZE                   32
#define ES_WIFI_FW_REV_SIZE                         24
#define ES_WIFI_API_REV_SIZE                        16
#define ES_WIFI_STACK_REV_SIZE                      16
#define ES_WIFI_RTOS_REV_SIZE                       16

#define ES_WIFI_DATA_SIZE                           1400
#define ES_WIFI_MAX_DETECTED_AP                     10

#define ES_WIFI_TIMEOUT                             0xFFFF

#define ES_WIFI_USE_PING                            1
#define ES_WIFI_USE_AWS                             0
#define ES_WIFI_USE_FIRMWAREUPDATE                  0
#define ES_WIFI_USE_WPS                             0

#define ES_WIFI_USE_SPI                             0
#define ES_WIFI_USE_UART                            (!ES_WIFI_USE_SPI)

/* Definitions of Country codes for "CN" AT command */
/* Allows to change WiFi channels range */
#define ES_WIFI_COUNTRY_FR "FR/0" /* allows WiFi channels 1-13 */
#define ES_WIFI_COUNTRY_JP "JP/0" /* allows WiFi channels 1-14 (channel 14 low power) */
#define ES_WIFI_COUNTRY_CA "CA/0" /* allows WiFi channels 1-12 (channel 12 low power) */
#define ES_WIFI_COUNTRY_US "US/0" /* allows WiFi channels 1-11 (set by default) */
/* define ES_WIFI_COUNTRY to one of ES_WIFI_COUNTRY_FR, ES_WIFI_COUNTRY_JP, ... */
/* define it only if country is not US (US is the default setting) */
/* #define ES_WIFI_COUNTRY    ES_WIFI_COUNTRY_FR */

#ifdef __cplusplus
}
#endif

#endif /* ES_WIFI_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
