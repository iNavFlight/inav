/**
  ******************************************************************************
  * @file    net_mx_wifi.c
  * @author  MCD Application Team
  * @brief   MXCHIP Wi-Fi specific BSD-like socket wrapper
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

/*cstat -MISRAC2012-* */
#include "net_connect.h"
#include "net_internals.h"
/*cstat +MISRAC2012-* */

#include "mx_wifi.h"


#ifdef MX_WIFI_API_DEBUG
#define DEBUG_LOG(M, ...)  printf(M, ##__VA_ARGS__);
#else
#define DEBUG_LOG(M, ...)
#endif /* MX_WIFI_API_DEBUG */

#define MXWIFI_MAX_CHANNEL_NBR           4

#define WIFI_FREE_SOCKET                 0U
#define WIFI_ALLOCATED_SOCKET            1U
#define WIFI_BIND_SOCKET                 2U
#define WIFI_SEND_OK                     4U
#define WIFI_RECV_OK                     8U
#define WIFI_CONNECTED_SOCKET            16U
#define WIFI_STARTED_CLIENT_SOCKET       32U
#define WIFI_STARTED_SERVER_SOCKET       64U
#define WIFI_CONNECTED_SOCKET_RW         (WIFI_CONNECTED_SOCKET | WIFI_SEND_OK | WIFI_RECV_OK)

#define NET_ARTON(A)     ((uint32_t)(((uint32_t)A[3] << 24U) |\
                                     ((uint32_t)A[2] << 16U) |\
                                     ((uint32_t)A[1] <<  8U) |\
                                     ((uint32_t)A[0] <<  0U)))

/* Declaration of generic class functions               */
void  HAL_Delay(uint32_t delay);

int32_t mx_wifi_driver(net_if_handle_t *pnetif);

static int32_t mx_wifi_if_init(net_if_handle_t *pnetif);
static int32_t mx_wifi_if_deinit(net_if_handle_t *pnetif);

static int32_t mx_wifi_if_start(net_if_handle_t *pnetif);
static int32_t mx_wifi_if_stop(net_if_handle_t *pnetif);
static int32_t mx_wifi_if_yield(net_if_handle_t *pnetif, uint32_t timeout);

static int32_t mx_wifi_if_connect(net_if_handle_t *pnetif);
static int32_t mx_wifi_if_disconnect(net_if_handle_t *pnetif);

static int32_t mx_wifi_socket(int32_t domain, int32_t type, int32_t protocol);
static int32_t mx_wifi_bind(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen);
static int32_t mx_wifi_listen(int32_t sock, int32_t backlog);
static int32_t mx_wifi_accept(int32_t sock, net_sockaddr_t *addr, uint32_t *addrlen);
static int32_t mx_wifi_connect(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen);
static int32_t mx_wifi_send(int32_t sock, uint8_t *buf, int32_t len, int32_t flags);
static int32_t mx_wifi_recv(int32_t sock, uint8_t *buf, int32_t len, int32_t flags);
static int32_t mx_wifi_sendto(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *to,
                              uint32_t tolen);
static int32_t mx_wifi_recvfrom(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *from,
                                uint32_t *fromlen);
static int32_t mx_wifi_setsockopt(int32_t sock, int32_t level, int32_t optname, const void *optvalue, uint32_t optlen);
static int32_t mx_wifi_getsockopt(int32_t sock, int32_t level, int32_t optname, void *optvalue, uint32_t *optlen);
static int32_t mx_wifi_getsockname(int32_t sock, net_sockaddr_t *name, uint32_t *namelen);
static int32_t mx_wifi_getpeername(int32_t sock, net_sockaddr_t *name, uint32_t *namelen);
static int32_t mx_wifi_close(int32_t sock, bool isaclone);
static int32_t mx_wifi_shutdown(int32_t sock, int32_t mode);

static int32_t mx_wifi_gethostbyname(net_if_handle_t *pnetif, net_sockaddr_t *addr, char_t *name);
static int32_t mx_wifi_ping(net_if_handle_t *pnetif, net_sockaddr_t *addr, int32_t count, int32_t delay,
                            int32_t response[]);

/* Declaration and definition of class-specific functions */
static int32_t mx_wifi_scan(net_if_handle_t *pnetif, net_wifi_scan_mode_t mode, char *ssid);
static int32_t mx_wifi_get_scan_result(net_if_handle_t *pnetif, net_wifi_scan_results_t *results, uint8_t number);

extern int32_t wifi_probe(void **ll_drv_context);
extern MX_WIFIObject_t *wifi_obj_get(void);

/* internal structure to mabage es_wfi socket */
typedef struct mxwifi_tls_data_s
{
  char *tls_ca_certs; /**< Socket option. */
  char *tls_ca_crl;   /**< Socket option. */
  char *tls_dev_cert; /**< Socket option. */
  char *tls_dev_key;  /**< Socket option. */
  uint8_t *tls_dev_pwd;        /**< Socket option. */
  bool tls_srv_verification;   /**< Socket option. */
  char *tls_srv_name;          /**< Socket option. */
} mxwifi_tls_data_t;


int32_t mx_wifi_driver(net_if_handle_t *pnetif)
{
  return mx_wifi_if_init(pnetif);
}

int32_t  mx_wifi_if_init(net_if_handle_t *pnetif)
{
  int32_t ret;
  net_if_drv_t *p;
  void *ptmp;

  ptmp = NET_MALLOC(sizeof(net_if_drv_t));
  (void)memcpy(&p, &ptmp, sizeof(p));
  if (p != NULL)
  {
    p->if_class = NET_INTERFACE_CLASS_WIFI;

    p->if_init = mx_wifi_if_init;
    p->if_deinit = mx_wifi_if_deinit;

    p->if_start = mx_wifi_if_start;
    p->if_stop = mx_wifi_if_stop;
    p->if_yield = mx_wifi_if_yield;

    p->if_connect = mx_wifi_if_connect;
    p->if_disconnect = mx_wifi_if_disconnect;

    p->psocket = mx_wifi_socket;
    p->pbind = mx_wifi_bind;
    p->plisten = mx_wifi_listen;
    p->paccept = mx_wifi_accept;
    p->pconnect = mx_wifi_connect;
    p->psend = mx_wifi_send;
    p->precv = mx_wifi_recv;
    p->psendto = mx_wifi_sendto;
    p->precvfrom = mx_wifi_recvfrom;
    p->psetsockopt = mx_wifi_setsockopt;
    p->pgetsockopt = mx_wifi_getsockopt;
    p->pgetsockname = mx_wifi_getsockname;
    p->pgetpeername = mx_wifi_getpeername;
    p->pclose = mx_wifi_close;
    p->pshutdown = mx_wifi_shutdown;

    p->pgethostbyname = mx_wifi_gethostbyname;
    p->pping = mx_wifi_ping;
    p->extension.wifi = NET_MALLOC(sizeof(net_if_wifi_class_extension_t));

    if (NULL == p->extension.wifi)
    {
      NET_DBG_ERROR("can't allocate memory for mx_wifi_driver class\n");
      NET_FREE(p);
      ret = NET_ERROR_NO_MEMORY;
    }
    else
    {
      (void) memset(p->extension.wifi, 0, sizeof(net_if_wifi_class_extension_t));
      pnetif->dhcp_mode = true;
      pnetif->pdrv = p;
      p->extension.wifi->scan = mx_wifi_scan;
      p->extension.wifi->get_scan_results = mx_wifi_get_scan_result;
      ret = NET_OK;
    }
  }
  else
  {
    NET_DBG_ERROR("can't allocate memory for mx_wifi_driver class\n");
    ret = NET_ERROR_NO_MEMORY;
  }
  return ret;
}

static int32_t mx_wifi_if_deinit(net_if_handle_t *pnetif)
{
  NET_FREE(pnetif->pdrv->extension.wifi);
  pnetif->pdrv->extension.wifi = NULL;
  NET_FREE(pnetif->pdrv);
  pnetif->pdrv = NULL;
  return NET_OK;
}



static int32_t mx_wifi_if_start(net_if_handle_t *pnetif)
{
  int32_t ret;

  MX_WIFIObject_t  *pMxWifiObj;

  if (wifi_probe(&pnetif->pdrv->context) == NET_OK)
  {
    DEBUG_LOG("MX_WIFI IO [OK]\r\n");
    pMxWifiObj = wifi_obj_get();

    /* wifi module hardware reboot */
    DEBUG_LOG("MX_WIFI REBOOT(HW) ...\r\n");
    if (MX_WIFI_STATUS_OK != MX_WIFI_HardResetModule(pMxWifiObj))
    {
      ret = NET_ERROR_DEVICE_ERROR;
    }
    else
    {
#ifdef NET_USE_RTOS
      (void)osDelay(500);
#else
      HAL_Delay(500);
#endif /* NET_USE_RTOS */

      /* Init the WiFi module */
      if (MX_WIFI_STATUS_OK != MX_WIFI_Init(pMxWifiObj))
      {
        ret = NET_ERROR_INTERFACE_FAILURE;
      }
      else
      {
        DEBUG_LOG("MX_WIFI_Init [OK]\r\n");
        /* Retrieve the WiFi module information */
        (void)strncpy(pnetif->DeviceName, (char_t *)pMxWifiObj->SysInfo.Product_Name, NET_DEVICE_NAME_LEN);
        (void)strncpy(pnetif->DeviceID, (char_t *)pMxWifiObj->SysInfo.Product_ID, NET_DEVICE_ID_LEN);
        (void)strncpy(pnetif->DeviceVer, (char_t *)pMxWifiObj->SysInfo.FW_Rev, NET_DEVICE_VER_LEN);

        (void)MX_WIFI_GetMACAddress(pMxWifiObj, pnetif->macaddr.mac);

        (void)net_state_manage_event(pnetif, NET_EVENT_INTERFACE_READY);

        ret = NET_OK;
      }
    }
  }
  else
  {
    ret = NET_ERROR_DEVICE_ERROR;
  }
  return ret;
}

static int32_t mx_wifi_if_stop(net_if_handle_t *pnetif)
{
  int32_t ret;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  if (MX_WIFI_STATUS_OK != MX_WIFI_DeInit(pMxWifiObj))
  {
    ret = NET_ERROR_GENERIC;
  }
  else
  {
    (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_INITIALIZED);
    ret = NET_OK;
  }
  return ret;
}

static int32_t mx_wifi_if_yield(net_if_handle_t *pnetif, uint32_t timeout)
{
  int32_t ret;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  (void)pnetif;
  ret = MX_WIFI_IO_YIELD(pMxWifiObj, timeout);

  return ret;
}

static void mx_wifi_status_changed(uint8_t cate, uint8_t status, void *arg)
{
  net_if_handle_t *pnetif;
  uint8_t addr[4];
  net_state_t net_state;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  (void)memcpy(&pnetif, &arg, sizeof(pnetif));

  if ((uint8_t)MC_STATION == cate)
  {
    switch (status)
    {
      case MC_STA_DOWN:
        DEBUG_LOG("MC_STA_DOWN\r\n");

        (void) net_if_getState(pnetif, &net_state);
        if (NET_STATE_CONNECTED == net_state)
        {
          (void)net_state_manage_event(pnetif, NET_EVENT_LINK_DOWN);
        }
        else
        {
          (void)net_state_manage_event(pnetif, NET_EVENT_INTERFACE_READY);
        }
        break;

      case MC_STA_UP:
        DEBUG_LOG("MC_STA_UP\r\n");
        (void)net_state_manage_event(pnetif, NET_EVENT_LINK_UP);
        break;

      case MC_STA_GOT_IP:
        DEBUG_LOG("MC_STA_GOT_IP\r\n");
        (void) memcpy(addr, pMxWifiObj->NetSettings.IP_Addr, 4);
        pnetif->ipaddr.addr = NET_ARTON(pMxWifiObj->NetSettings.IP_Addr);
        (void) memcpy(addr, pMxWifiObj->NetSettings.IP_Mask, 4);
        pnetif->netmask.addr = NET_ARTON(pMxWifiObj->NetSettings.IP_Mask);
        (void) memcpy(addr, pMxWifiObj->NetSettings.Gateway_Addr, 4);
        pnetif->gateway.addr = NET_ARTON(pMxWifiObj->NetSettings.Gateway_Addr);
        (void) net_state_manage_event(pnetif, NET_EVENT_IPADDR);
        break;

      default:
        break;
    }
  }
  else if ((uint8_t)MC_SOFTAP == cate)
  {
    switch (status)
    {
      case MC_AP_DOWN:
        DEBUG_LOG("MC_AP_DOWN\r\n");
        net_if_notify(pnetif, NET_EVENT_STATE_CHANGE, (uint32_t) NET_STATE_CONNECTION_LOST, NULL);
        break;

      case MC_AP_UP:
        DEBUG_LOG("MC_AP_UP\r\n");
        pnetif->ipaddr.addr = pnetif->static_ipaddr.addr;
        pnetif->gateway.addr = pnetif->static_gateway.addr;
        pnetif->netmask.addr = pnetif->static_netmask.addr;
        (void) net_state_manage_event(pnetif, NET_EVENT_IPADDR);
        break;

      default:
        break;
    }
  }
  else
  {
    /* nothing */
  }
}

static   MX_WIFI_SecurityType_t  convert(int    security_mode)
{
#if 0
  MX_WIFI_SEC_NONE,       /**< Open system. */
  MX_WIFI_SEC_WEP,        /**< Wired Equivalent Privacy. WEP security. */
  MX_WIFI_SEC_WPA_TKIP,   /**< WPA /w TKIP */
  MX_WIFI_SEC_WPA_AES,    /**< WPA /w AES */
  MX_WIFI_SEC_WPA2_TKIP,  /**< WPA2 /w TKIP */
  MX_WIFI_SEC_WPA2_AES,   /**< WPA2 /w AES */
  MX_WIFI_SEC_WPA2_MIXED, /**< WPA2 /w AES or TKIP */
#endif
  return MX_WIFI_SEC_AUTO;
}

static int32_t mx_wifi_if_connect_sta(net_if_handle_t *pnetif)
{
  int32_t ret;
  MX_WIFI_SecurityType_t secure_type;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();
  const net_wifi_credentials_t *credentials =  pnetif->pdrv->extension.wifi->credentials;

  if (false == pnetif->dhcp_mode)
  {
    pMxWifiObj->NetSettings.DHCP_IsEnabled = 0;
    (void)memcpy(pMxWifiObj->NetSettings.IP_Addr, &(pnetif->ipaddr), 4);
    (void)memcpy(pMxWifiObj->NetSettings.IP_Mask, &(pnetif->netmask), 4);
    (void)memcpy(pMxWifiObj->NetSettings.Gateway_Addr, &(pnetif->gateway), 4);
  }
  else
  {
    pMxWifiObj->NetSettings.DHCP_IsEnabled = 1;
  }

  (void)MX_WIFI_RegisterStatusCallback(pMxWifiObj, mx_wifi_status_changed, pnetif);

  secure_type = convert(credentials->security_mode);
  ret = MX_WIFI_Connect(pMxWifiObj, credentials->ssid, credentials->psk, secure_type);
  return ret;
}

#define BYTEN(A,n) ((A)>>(8u*(n))) & 0xffu
#define BYTE3(A)  BYTEN((A),3u)
#define BYTE2(A)  BYTEN((A),2u)
#define BYTE1(A)  BYTEN((A),1u)
#define BYTE0(A)  BYTEN((A),0u)
#define ADDR(a) BYTE0(a),BYTE1(a),BYTE2(a),BYTE3(a)

static int32_t mx_wifi_if_connect_ap(net_if_handle_t *pnetif)
{
  int32_t ret = NET_ERROR_GENERIC;

  MX_WIFI_APSettings_t ap_cfg;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();
  const net_wifi_credentials_t *credentials =  pnetif->pdrv->extension.wifi->credentials;

  (void) memset(&ap_cfg, 0, sizeof(ap_cfg));
  (void) strcpy(ap_cfg.SSID, credentials->ssid);
  (void) strcpy(ap_cfg.pswd, credentials->psk);

  ap_cfg.channel = pnetif->pdrv->extension.wifi->access_channel;

  (void) sprintf(ap_cfg.ip.localip, "%ld.%ld.%ld.%ld", ADDR(pnetif->static_ipaddr.addr));
  (void) sprintf(ap_cfg.ip.netmask, "%ld.%ld.%ld.%ld", ADDR(pnetif->static_gateway.addr));
  (void) sprintf(ap_cfg.ip.gateway, "%ld.%ld.%ld.%ld", ADDR(pnetif->static_netmask.addr));
  (void) sprintf(ap_cfg.ip.dnserver, "%ld.%ld.%ld.%ld", ADDR(pnetif->static_dnserver.addr));

  (void) MX_WIFI_RegisterStatusCallback(pMxWifiObj, mx_wifi_status_changed, pnetif);

  if (MX_WIFI_STATUS_OK == MX_WIFI_StartAP(pMxWifiObj, &ap_cfg))
  {
    ret = NET_OK;
  }

  return ret;
}





static int32_t mx_wifi_if_disconnect(net_if_handle_t *pnetif)
{
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();
  if (pnetif->pdrv->extension.wifi->mode == NET_WIFI_MODE_STA)
  {
    (void)MX_WIFI_Disconnect(pMxWifiObj);
  }
  else
  {
    (void) MX_WIFI_StopAP(pMxWifiObj);
  }

  (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_READY);
  return NET_OK;
}




static int32_t mx_wifi_if_connect(net_if_handle_t *pnetif)
{
  int32_t ret;
  if (pnetif->pdrv->extension.wifi->mode == NET_WIFI_MODE_STA)
  {
    ret =  mx_wifi_if_connect_sta(pnetif);
  }
  else
  {
    ret =  mx_wifi_if_connect_ap(pnetif);
  }
  return ret;
}


static int32_t mx_wifi_scan(net_if_handle_t *pnetif, net_wifi_scan_mode_t mode, char_t *ssid)
{
  int32_t ret;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();
  uint32_t        len = 0u;

  (void) pnetif;

  if (ssid != NULL)
  {
    len = strlen(ssid);
  }
  ret = MX_WIFI_Scan(pMxWifiObj, (uint8_t)mode, ssid, (int32_t) len);
  return ret;
}

static int32_t mx_wifi_get_scan_result(net_if_handle_t *pnetif, net_wifi_scan_results_t *scan_bss_array,
                                       uint8_t scan_bss_array_size)
{
  int32_t       ret;
  uint8_t       number = scan_bss_array_size;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();
  mc_wifi_ap_info_t *ap_list_head;
  net_wifi_scan_results_t *scan_bss = scan_bss_array;

  (void)pnetif;
  static  uint32_t mxsec[] =
  {
    NET_WIFI_SM_OPEN,
    NET_WIFI_SM_WEP_PSK,        /**< Wired Equivalent Privacy. WEP security. */
    NET_WIFI_SM_WPA_TKIP_PSK,   /**< WPA /w TKIP */
    NET_WIFI_SM_WPA_AES_PSK,    /**< WPA /w AES */
    NET_WIFI_SM_WPA2_TKIP_PSK,  /**< WPA2 /w TKIP */
    NET_WIFI_SM_WPA2_AES_PSK,   /**< WPA2 /w AES */
    NET_WIFI_SM_WPA2_MIXED_PSK, /**< WPA2 /w AES or TKIP */
  };

  if ((NULL == scan_bss_array) || (0u == number))
  {
    ret = NET_ERROR_PARAMETER;
  }
  else
  {
    /* create buff for mc_wifi results */
    void *ptmp = NET_MALLOC(sizeof(mc_wifi_ap_info_t) * number);
    (void) memcpy(&ap_list_head, &ptmp, sizeof(ap_list_head));

    if (NULL == ap_list_head)
    {
      ret = NET_ERROR_NO_MEMORY;
    }
    else
    {
      mc_wifi_ap_info_t *ap_info = ap_list_head;

      (void) memset(ap_list_head, 0, sizeof(mc_wifi_ap_info_t) * number);

      /* get real mc_wifi scan results data */
      number = (uint8_t) MX_WIFI_Get_scan_result(pMxWifiObj, (uint8_t *) ap_info, number);

      for (uint32_t i = 0U; i < number; i++)
      {
        (void) memset(scan_bss, 0, sizeof(net_wifi_scan_bss_t));
        (void) memcpy(scan_bss->ssid.value, ap_info->ssid, NET_WIFI_MAX_SSID_SIZE);
        scan_bss->ssid.length = (uint8_t) strlen(ap_info->ssid);
        scan_bss->security = mxsec[ap_info->security];
        (void) memcpy(&scan_bss->bssid, ap_info->bssid, NET_WIFI_MAC_ADDRESS_SIZE);
        scan_bss->rssi = (int8_t)ap_info->rssi;
        scan_bss->channel = ap_info->channel;
        (void) memcpy(scan_bss->country, ".CN", 4);  /* NOT SUPPORT for MX_WIFI */

        scan_bss++;
        ap_info++;
      }
      ret = (int32_t) number;
      NET_FREE((void *) ap_list_head);
    }
  }
  return ret;
}

static int32_t mx_wifi_socket(int32_t domain, int32_t type, int32_t protocol)
{
  int32_t ret;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  ret = MX_WIFI_Socket_create(pMxWifiObj, domain, type, protocol);
  return ret;
}

static int32_t mx_wifi_setsockopt(int32_t sock, int32_t level, int32_t optname, const void *optvalue, uint32_t optlen)
{
  int32_t ret;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  ret = MX_WIFI_Socket_setsockopt(pMxWifiObj, sock, level, optname, optvalue, (int32_t)optlen);
  return ret;
}

static int32_t mx_wifi_getsockopt(int32_t sock, int32_t level, int32_t optname, void *optvalue, uint32_t *optlen)
{
  int32_t ret;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  ret = MX_WIFI_Socket_getsockopt(pMxWifiObj, sock, level, optname, optvalue, optlen);
  return ret;
}

static int32_t mx_wifi_bind(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen)
{
  int32_t ret;
  struct sockaddr mx_addr;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  (void)memcpy(&mx_addr, addr, sizeof(mx_addr));
  ret = MX_WIFI_Socket_bind(pMxWifiObj, sock, &mx_addr, (int32_t)addrlen);
  return ret;
}

static int32_t mx_wifi_listen(int32_t sock, int32_t backlog)
{
  int32_t ret;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  ret = MX_WIFI_Socket_listen(pMxWifiObj, sock, backlog);
  return ret;
}

static int32_t mx_wifi_accept(int32_t sock, net_sockaddr_t *addr, uint32_t *addrlen)
{
  int32_t ret;
  struct sockaddr mx_addr;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  (void)memcpy(&mx_addr, addr, sizeof(mx_addr));
  ret = MX_WIFI_Socket_accept(pMxWifiObj, sock, &mx_addr, addrlen);
  return ret;
}

static int32_t mx_wifi_connect(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen)
{
  int32_t ret;
  struct sockaddr mx_addr;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  (void)memcpy(&mx_addr, addr, sizeof(mx_addr));
  ret = MX_WIFI_Socket_connect(pMxWifiObj, sock, &mx_addr, (int32_t)addrlen);
  return ret;
}

static int32_t mx_wifi_shutdown(int32_t sock, int32_t mode)
{
  int32_t ret;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  ret = MX_WIFI_Socket_shutdown(pMxWifiObj, sock, mode);
  return ret;
}

static int32_t mx_wifi_close(int32_t sock, bool isaclone)
{
  int32_t ret;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  (void)isaclone;
  ret = MX_WIFI_Socket_close(pMxWifiObj, sock);
  return ret;
}

static int32_t mx_wifi_send(int32_t sock, uint8_t *buf, int32_t len, int32_t flags)
{
  int32_t ret;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  ret = MX_WIFI_Socket_send(pMxWifiObj, sock, buf, len, flags);
  return ret;
}

static int32_t mx_wifi_recv(int32_t sock, uint8_t *buf, int32_t len, int32_t flags)
{
  int32_t ret;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  ret = MX_WIFI_Socket_recv(pMxWifiObj, sock, buf, len, flags);
  return ret;
}

static int32_t mx_wifi_sendto(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *to,
                              uint32_t tolen)
{
  int32_t ret;
  struct sockaddr mx_addr;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  (void)memcpy(&mx_addr, to, sizeof(mx_addr));
  ret = MX_WIFI_Socket_sendto(pMxWifiObj, sock, buf, len, flags, &mx_addr, (int32_t)tolen);
  return ret;
}

static int32_t mx_wifi_recvfrom(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *from,
                                uint32_t *fromlen)
{
  int32_t ret;
  struct sockaddr mx_addr;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  (void)memcpy(&mx_addr, from, sizeof(mx_addr));
  ret = MX_WIFI_Socket_recvfrom(pMxWifiObj, sock, buf, len, flags, &mx_addr, fromlen);
  return ret;
}

static int32_t mx_wifi_gethostbyname(net_if_handle_t *pnetif, net_sockaddr_t *addr, char_t *name)
{
  int32_t ret;
  struct sockaddr mx_addr;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();

  (void)pnetif;
  ret = MX_WIFI_Socket_gethostbyname(pMxWifiObj, &mx_addr, (char_t *)name);
  (void)memcpy(addr, &mx_addr, sizeof(mx_addr));
  return ret;
}

int32_t mx_wifi_ping(net_if_handle_t *pnetif, net_sockaddr_t *addr, int32_t count, int32_t delay, int32_t response[])
{
  int32_t ret;
  MX_WIFIObject_t  *pMxWifiObj = wifi_obj_get();
  net_sockaddr_in_t addr_in;
  net_ip_addr_t ip_addr;

  (void)pnetif;
  (void)memcpy(&addr_in, addr, sizeof(addr_in));
  ip_addr.addr = addr_in.sin_addr.s_addr;
  ret = MX_WIFI_Socket_ping(pMxWifiObj, (char_t *)net_ntoa(&ip_addr), count, delay, response);
  return ret;
}

static int32_t mx_wifi_getsockname(int32_t sock, net_sockaddr_t *name, uint32_t *namelen)
{
  DEBUG_LOG("mx_wifi_getsockname UNSUPPORTED!");
  (void)sock;
  (void)name;
  (void)namelen;
  return NET_ERROR_UNSUPPORTED;
}

static int32_t mx_wifi_getpeername(int32_t sock, net_sockaddr_t *name, uint32_t *namelen)
{
  DEBUG_LOG("mx_wifi_getpeername UNSUPPORTED!");
  (void)sock;
  (void)name;
  (void)namelen;
  return NET_ERROR_UNSUPPORTED;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

