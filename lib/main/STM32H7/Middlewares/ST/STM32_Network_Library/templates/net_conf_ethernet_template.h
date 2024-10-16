/**
  ******************************************************************************
  * @file    net_conf.h
  * @author  MCD Application Team
  * @brief   This file provides the configuration for net
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#ifndef NET_CONF_H
#define NET_CONF_H

#ifdef __cplusplus
extern "C" {
#endif


#define NET_USE_RTOS
#define NET_USE_HARDWARE_CHKSUM
#define ETHERNET_MAC_GENERATION_FROM_SHA256


#ifdef GENERATOR_WAKAAMACLIENT_CLOUD
#define USE_TINY_DTLS
#else
#define NET_MBEDTLS_HOST_SUPPORT
#endif /* GENERATOR_WAKAAMACLIENT_CLOUD */


#include "net_conf_template.h"


#ifdef __cplusplus
}
#endif

#endif /* NET_CONF_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
