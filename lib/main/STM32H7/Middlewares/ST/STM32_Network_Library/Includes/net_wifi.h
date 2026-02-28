/**
  ******************************************************************************
  * @file    net_wifi.h
  * @author  MCD Application Team
  * @brief   Header for the network Wi-Fi class.
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
#ifndef NET_WIFI_H
#define NET_WIFI_H

#ifdef __cplusplus
extern "C" {
#endif

#define NET_WIFI_MAC_ADDRESS_SIZE  6
#define NET_WIFI_MAX_SSID_SIZE     32
#define NET_WEP_ENABLED            0x0001U  /**< Flag to enable WEP Security        */
#define NET_TKIP_ENABLED           0x0002U  /**< Flag to enable TKIP Encryption     */
#define NET_AES_ENABLED            0x0004U  /**< Flag to enable AES Encryption      */
#define NET_SHARED_ENABLED     0x00008000U  /**< Flag to enable Shared key Security */
#define NET_WPA_SECURITY       0x00200000U  /**< Flag to enable WPA Security        */
#define NET_WPA2_SECURITY      0x00400000U  /**< Flag to enable WPA2 Security       */
#define NET_WPA3_SECURITY      0x01000000U  /**< Flag to enable WPA3 PSK Security   */

#define NET_ENTERPRISE_ENABLED 0x02000000U  /**< Flag to enable Enterprise Security */
#define NET_WPS_ENABLED        0x10000000U  /**< Flag to enable WPS Security        */
#define NET_IBSS_ENABLED       0x20000000U  /**< Flag to enable IBSS mode           */
#define NET_FBT_ENABLED        0x40000000U  /**< Flag to enable FBT                 */

#define    NET_WIFI_SM_OPEN             0U
/**< Open security                                         */
#define    NET_WIFI_SM_WEP_PSK           NET_WEP_ENABLED
/**< WEP PSK Security with open authentication             */
#define    NET_WIFI_SM_WEP_SHARED        (NET_WEP_ENABLED | NET_SHARED_ENABLED)
/**< WEP PSK Security with shared authentication           */
#define    NET_WIFI_SM_WPA_TKIP_PSK      (NET_WPA_SECURITY | NET_TKIP_ENABLED)
/**< WPA PSK Security with TKIP                            */
#define    NET_WIFI_SM_WPA_AES_PSK       (NET_WPA_SECURITY | NET_AES_ENABLED)
/**< WPA PSK Security with AES                             */
#define    NET_WIFI_SM_WPA_MIXED_PSK     (NET_WPA_SECURITY | NET_AES_ENABLED | NET_TKIP_ENABLED)
/**< WPA PSK Security with AES & TKIP                      */
#define    NET_WIFI_SM_WPA2_AES_PSK      (NET_WPA2_SECURITY | NET_AES_ENABLED)
/**< WPA2 PSK Security with AES                            */
#define    NET_WIFI_SM_WPA2_WPA_PSK      (NET_WPA2_SECURITY | NET_WPA_SECURITY)
/**< WPA2 PSK Security with AES                            */
#define    NET_WIFI_SM_WPA2_TKIP_PSK     (NET_WPA2_SECURITY | NET_TKIP_ENABLED)
/**< WPA2 PSK Security with TKIP                           */
#define    NET_WIFI_SM_WPA2_MIXED_PSK    (NET_WPA2_SECURITY | NET_AES_ENABLED | NET_TKIP_ENABLED)
/**< WPA2 PSK Security with AES & TKIP                     */
#define    NET_WIFI_SM_WPA2_FBT_PSK      (NET_WPA2_SECURITY | NET_AES_ENABLED | NET_FBT_ENABLED)
/**< WPA2 FBT PSK Security with AES & TKIP */
#define    NET_WIFI_SM_WPA3_SAE          (NET_WPA3_SECURITY | NET_AES_ENABLED)
/**< WPA3 Security with AES */
#define    NET_WIFI_SM_WPA3_WPA2_PSK     (NET_WPA3_SECURITY | NET_WPA2_SECURITY | NET_AES_ENABLED)
/**< WPA3 WPA2 PSK Security with AES */

#define    NET_WIFI_SM_WPA_TKIP_ENT      (NET_ENTERPRISE_ENABLED | NET_WPA_SECURITY | NET_TKIP_ENABLED)
/**< WPA Enterprise Security with TKIP                     */
#define    NET_WIFI_SM_WPA_AES_ENT       (NET_ENTERPRISE_ENABLED | NET_WPA_SECURITY | NET_AES_ENABLED)
/**< WPA Enterprise Security with AES                      */
#define    NET_WIFI_SM_WPA_MIXED_ENT     (NET_ENTERPRISE_ENABLED\
                                          | NET_WPA_SECURITY | NET_AES_ENABLED | NET_TKIP_ENABLED)
/**< WPA Enterprise Security with AES & TKIP               */
#define    NET_WIFI_SM_WPA2_TKIP_ENT     (NET_ENTERPRISE_ENABLED | NET_WPA2_SECURITY | NET_TKIP_ENABLED)
/**< WPA2 Enterprise Security with TKIP                    */
#define    NET_WIFI_SM_WPA2_AES_ENT      (NET_ENTERPRISE_ENABLED | NET_WPA2_SECURITY | NET_AES_ENABLED)
/**< WPA2 Enterprise Security with AES                     */
#define    NET_WIFI_SM_WPA2_MIXED_ENT    (NET_ENTERPRISE_ENABLED\
                                          | NET_WPA2_SECURITY | NET_AES_ENABLED | NET_TKIP_ENABLED)
/**< WPA2 Enterprise Security with AES & TKIP              */
#define    NET_WIFI_SM_WPA2_FBT_ENT      (NET_ENTERPRISE_ENABLED\
                                          | NET_WPA2_SECURITY | NET_AES_ENABLED | NET_FBT_ENABLED)
/**< WPA2 Enterprise Security with AES & FBT               */

#define    NET_WIFI_SM_IBSS_OPEN         (NET_IBSS_ENABLED)
/**< Open security on IBSS ad-hoc network                  */
#define    NET_WIFI_SM_WPS_OPEN          (NET_WPS_ENABLED)
/**< WPS with open security                                */
#define    NET_WIFI_SM_WPS_SECURE        (NET_WPS_ENABLED | NET_AES_ENABLED)
/**< WPS with AES security                                 */
#define    NET_WIFI_SM_UNKNOWN           0xFFFFFFFFU
/**< UNKNOWN security                                      */
#define    NET_WIFI_SM_AUTO              0xFFFFFFF0U
/**< Auto Mode                                         */


/* Wi-Fi events */
typedef enum
{
  NET_WIFI_SCAN_RESULTS_READY
} net_wifi_event_t;


/* Mode */
typedef enum
{
  NET_WIFI_MODE_STA,
  NET_WIFI_MODE_AP
} net_wifi_mode_t;

/* MAC address */
typedef uint8_t net_wifi_mac_t[NET_WIFI_MAC_ADDRESS_SIZE];

/* SSID , max 32 alpha string chain*/
typedef struct
{
  uint8_t length;                /**< SSID length */
  uint8_t value[NET_WIFI_MAX_SSID_SIZE]; /**< SSID name (AP name)  */
} net_wifi_ssid_t;


/* Scan */
typedef enum
{
  NET_WIFI_SCAN_PASSIVE,
  NET_WIFI_SCAN_ACTIVE,
  NET_WIFI_SCAN_AUTO
} net_wifi_scan_mode_t;

typedef struct net_wifi_scan_bss_s
{
  net_wifi_ssid_t  ssid;
  net_wifi_mac_t   bssid;
  uint32_t          security;
  uint8_t          channel;
  uint8_t          country[4]; /* one more char for null terminated string */
  int8_t          rssi;
} net_wifi_scan_bss_t;
#if 0
typedef struct net_wifi_scan_results_s
{
  uint16_t              number;
  net_wifi_scan_bss_t  *bss;
} net_wifi_scan_results_t;
#else
typedef net_wifi_scan_bss_t net_wifi_scan_results_t;
#endif /* old definiton */

/* Param */
typedef enum
{
  NET_WIFI_MODE,
} net_wifi_param_t;

/* System info */
typedef enum
{
  NET_WIFI_SCAN_RESULTS_NUMBER,
} net_wifi_system_info_t;

/* Credential configuration */
typedef struct net_wifi_credentials_s
{
  const char_t *ssid;
  const char_t *psk;
  int32_t security_mode;
} net_wifi_credentials_t;

/* Powersave */
typedef enum
{
  WIFI_POWERSAVE_ACTIVE,
  WIFI_POWERSAVE_LIGHT_SLEEP,
  WIFI_POWERSAVE_DEEP_SLEEP
}
net_wifi_powersave_t;

const char_t *net_wifi_security_to_string(uint32_t sec);
uint32_t net_wifi_string_to_security(char *sec);

int32_t net_wifi_scan(net_if_handle_t *pnetif, net_wifi_scan_mode_t mode, char *ssid);

int32_t net_wifi_get_scan_results(net_if_handle_t *pnetif, net_wifi_scan_results_t *results, uint8_t number);
int32_t net_wifi_set_credentials(net_if_handle_t *pnetif, const net_wifi_credentials_t *credentials);
int32_t net_wifi_set_access_mode(net_if_handle_t *pnetif,  net_wifi_mode_t mode);
int32_t net_wifi_set_access_channel(net_if_handle_t *pnetif,  uint8_t channel);
int32_t net_wifi_set_powersave(net_if_handle_t *pnetif, const net_wifi_powersave_t *powersave);
int32_t net_wifi_set_param(net_if_handle_t *pnetif, const net_wifi_param_t param, void *data);

#ifdef __cplusplus
}
#endif

#endif /* NET_WIFI_H */
