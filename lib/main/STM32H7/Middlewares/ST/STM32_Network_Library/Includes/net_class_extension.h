/**
  ******************************************************************************
  * @file    net_class_extension.h
  * @author  MCD Application Team
  * @brief   Header for the network class extensions
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef NET_CLASS_EXTENSION_H
#define NET_CLASS_EXTENSION_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct net_if_wifi_class_extension_s
{
  int32_t (*scan)(net_if_handle_t *pnetif, net_wifi_scan_mode_t mode, char *ssid);
  int32_t (*get_scan_results)(net_if_handle_t *pnetif, net_wifi_scan_results_t *results, uint8_t number);
  int32_t (*set_credentials)(const net_wifi_credentials_t *cred);
  int32_t (*get_system_info)(const net_wifi_system_info_t info, void *data);
  int32_t (*set_param)(const net_wifi_param_t info, void *data);
  const          net_wifi_credentials_t *credentials;
  net_wifi_mode_t       mode;
  /* Acces Point parameter */
  uint8_t               access_channel;
  uint8_t               max_connections;
  bool                AP_hidden;

  const          net_wifi_powersave_t *powersave;
  void           *ifp; /* Interface STA or AP handler */
} net_if_wifi_class_extension_t;


typedef struct net_if_ethernet_class_extension_s
{
  int32_t (*version)(void);
} net_if_ethernet_class_extension_t;

typedef struct net_if_cellular_class_extension_s
{
  int32_t (*get_radio_results)(net_cellular_radio_results_t *results);
  const          net_cellular_credentials_t *credentials;
} net_if_cellular_class_extension_t;

typedef struct net_if_custom_class_extension_s
{
  int32_t (*version)(void);
} net_if_custom_class_extension_t;


#ifdef __cplusplus
}
#endif


#endif /* NET_CLASS_EXTENSION_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
