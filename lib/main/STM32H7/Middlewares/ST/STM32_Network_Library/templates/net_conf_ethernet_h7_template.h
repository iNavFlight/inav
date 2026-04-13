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

#include <stdint.h>
void ethernetif_low_init(void (*event_callback)(void), void (*buffer_output_callback)(uint8_t *));
void  ethernetif_low_deinit(void);

int16_t ethernetif_low_inputsize(void);
int16_t ethernetif_low_input(uint8_t *payload, uint16_t len);
int16_t ethernetif_low_output(uint8_t *payload, uint16_t len);

void ethernetif_low_get_mac_addr(uint8_t *MACAddr_in);
uint8_t ethernetif_low_get_link_status(void);

#define NET_USE_RTOS
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
