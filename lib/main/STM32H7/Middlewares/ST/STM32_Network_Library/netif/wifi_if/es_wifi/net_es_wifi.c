/**
  ******************************************************************************
  * @file    net_es_wifi.c
  * @author  MCD Application Team
  * @brief   ES Wi-Fi specific BSD-like socket wrapper
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

#include "net_connect.h"
#include "net_internals.h"
#include "es_wifi.h"

#define ESWIFI_MAX_CHANNEL_NBR           4
#define WIFI_FREE_SOCKET                 0U
#define WIFI_ALLOCATED_SOCKET            1U
#define WIFI_BIND_SOCKET                 2U
#define WIFI_SEND_OK                     4U
#define WIFI_RECV_OK                     8U
#define WIFI_CONNECTED_SOCKET            16U
#define WIFI_STARTED_CLIENT_SOCKET       32U
#define WIFI_STARTED_SERVER_SOCKET       64U
#define WIFI_CONNECTED_SOCKET_RW         (WIFI_CONNECTED_SOCKET | WIFI_SEND_OK | WIFI_RECV_OK)

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

#define NET_NTOAR(N,A)   A[3] = (uint8_t)((N & 0xff000000U) >> 24U);\
  A[2] = (uint8_t)((N & 0x00ff0000U) >> 16U);\
  A[1] = (uint8_t)((N & 0x0000ff00U) >> 8U); \
  A[0] = (uint8_t)((N & 0x000000ffU) >> 0U); \

#define NET_ARTON(A)     ((uint32_t)(((uint32_t)A[3] << 24U) |\
                                     ((uint32_t)A[2] << 16U) |\
                                     ((uint32_t)A[1] <<  8U) |\
                                     ((uint32_t)A[0] <<  0U)))

/* Declaration of generic class functions               */
static   ES_WIFIObject_t  *castcontext(void *context)
{
  /*cstat -MISRAC2012-Rule-11.5 */
  return (ES_WIFIObject_t *)context;
  /*cstat +MISRAC2012-Rule-11.5 */

}


static   sockaddr_in_t *cast2sockaddr_in(const net_sockaddr_t *addr)
{
  /*cstat -MISRAC2012-Rule-11.3 -MISRAC2012-Rule-11.8 */
  return (sockaddr_in_t *) addr;
  /*cstat +MISRAC2012-Rule-11.3 +MISRAC2012-Rule-11.8 */

}


int32_t es_wifi_driver(net_if_handle_t *pnetif);

static int32_t es_wifi_if_init(net_if_handle_t *pnetif);
static int32_t es_wifi_if_deinit(net_if_handle_t *pnetif);

static int32_t es_wifi_if_start(net_if_handle_t *pnetif);
static int32_t es_wifi_if_stop(net_if_handle_t *pnetif);

static int32_t es_wifi_if_connect(net_if_handle_t *pnetif);
static int32_t es_wifi_if_disconnect(net_if_handle_t *pnetif);

static int32_t es_wifi_socket(int32_t domain, int32_t type, int32_t protocol);
static int32_t es_wifi_bind(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen);
static int32_t es_wifi_listen(int32_t sock, int32_t backlog);
static int32_t es_wifi_accept(int32_t sock, net_sockaddr_t *addr, uint32_t *addrlen);
static int32_t es_wifi_connect(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen);
static int32_t es_wifi_send(int32_t sock, uint8_t *buf, int32_t len, int32_t flags);
static int32_t es_wifi_recv(int32_t sock, uint8_t *buf, int32_t len, int32_t flags);
static int32_t es_wifi_sendto(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *to,
                              uint32_t tolen);
static int32_t es_wifi_recvfrom(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *from,
                                uint32_t *fromlen);
static int32_t es_wifi_setsockopt(int32_t sock, int32_t level, int32_t optname, const void *optvalue, uint32_t optlen);
static int32_t es_wifi_getsockopt(int32_t sock, int32_t level, int32_t optname, void *optvalue, uint32_t *optlen);
static int32_t es_wifi_getsockname(int32_t sock, net_sockaddr_t *name, uint32_t *namelen);
static int32_t es_wifi_getpeername(int32_t sock, net_sockaddr_t *name, uint32_t *namelen);
static int32_t es_wifi_close(int32_t sock, bool isaclone);
static int32_t es_wifi_shutdown(int32_t sock, int32_t mode);

static int32_t es_wifi_gethostbyname(net_if_handle_t *pnetif, net_sockaddr_t *addr, char_t *name);
static int32_t es_wifi_ping(net_if_handle_t *pnetif, net_sockaddr_t *addr, int32_t count, int32_t delay,
                            int32_t response[]);

/* Declaration and definition of class-specific functions */
static void check_connection_lost(net_if_handle_t *pnetif, int32_t n);
static  int32_t es_wifi_scan(net_if_handle_t *pnetif, net_wifi_scan_mode_t mode, char *ssid);
static  int32_t es_wifi_get_scan_results(net_if_handle_t *pnetif, net_wifi_scan_results_t *results, uint8_t number);

/* internal structure to mabage es_wfi socket */
typedef struct eswifi_tls_data_s
{
  char_t *tls_ca_certs; /**< Socket option. */
  char_t *tls_ca_crl;   /**< Socket option. */
  char_t *tls_dev_cert; /**< Socket option. */
  char_t *tls_dev_key;  /**< Socket option. */
  uint8_t *tls_dev_pwd;        /**< Socket option. */
  bool tls_srv_verification;   /**< Socket option. */
  char_t *tls_srv_name;          /**< Socket option. */
} eswifi_tls_data_t;

typedef struct WIFI_Channel_s
{
  uint8_t  Number;
  uint8_t  status;
  uint8_t  protocol;
  uint8_t  type;
  uint16_t localport;
  uint32_t  localaddress;
  uint32_t maxpconnections;
  int32_t  blocking;
  int32_t  sendtimeout;
  int32_t  recvtimeout;
  net_if_notify_func notify_callback;
  void          *notify_context;
  net_if_handle_t  *pnetif;

#ifdef NET_MBEDTLS_WIFI_MODULE_SUPPORT
  eswifi_tls_data_t        tlsData;
#endif /* NET_MBEDTLS_WIFI_MODULE_SUPPORT */

} WIFI_Channel_t;

static WIFI_Channel_t WifiChannel[ESWIFI_MAX_CHANNEL_NBR] = {.0};




int32_t es_wifi_driver(net_if_handle_t *pnetif)
{
  return es_wifi_if_init(pnetif);
}


/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
int32_t  es_wifi_if_init(net_if_handle_t *pnetif)
{
  int32_t ret;
  /*cstat -MISRAC2012-Rule-11.5 -MISRAC2012-Rule-21.3 */
  net_if_drv_t *p = NET_MALLOC(sizeof(net_if_drv_t));
  /*cstat +MISRAC2012-Rule-11.5 +MISRAC2012-Rule-21.3 */
  if (p != NULL)
  {
    p->if_class = NET_INTERFACE_CLASS_WIFI;

    p->if_init = es_wifi_if_init;
    p->if_deinit = es_wifi_if_deinit;

    p->if_start = es_wifi_if_start;
    p->if_stop = es_wifi_if_stop;

    p->if_connect = es_wifi_if_connect;
    p->if_disconnect = es_wifi_if_disconnect;

    p->psocket = es_wifi_socket;
    p->pbind = es_wifi_bind;
    p->plisten = es_wifi_listen;
    p->paccept = es_wifi_accept;
    p->pconnect = es_wifi_connect;
    p->psend = es_wifi_send;
    p->precv = es_wifi_recv;
    p->psendto = es_wifi_sendto;
    p->precvfrom = es_wifi_recvfrom;
    p->psetsockopt = es_wifi_setsockopt;
    p->pgetsockopt = es_wifi_getsockopt;
    p->pgetsockname = es_wifi_getsockname;
    p->pgetpeername = es_wifi_getpeername;
    p->pclose = es_wifi_close;
    p->pshutdown = es_wifi_shutdown;

    p->pgethostbyname = es_wifi_gethostbyname;
    p->pping = es_wifi_ping;
    /*cstat -MISRAC2012-Rule-11.5 -MISRAC2012-Rule-21.3 -MISRAC2012-Dir-4.12  malloc */
    net_if_wifi_class_extension_t       *pextwifi;
    pextwifi = NET_MALLOC(sizeof(net_if_wifi_class_extension_t));
    /*cstat +MISRAC2012-Rule-11.5 +MISRAC2012-Rule-21.3 +MISRAC2012-Dir-4.12*/
    if (NULL == pextwifi)
    {
      NET_DBG_ERROR("can't allocate memory for es_wifi_driver class\n");
      /*cstat -MISRAC2012-Rule-21.3 */
      NET_FREE(p);
      /*cstat +MISRAC2012-Rule-21.3 */
      ret = NET_ERROR_NO_MEMORY;
    }
    else
    {
      (void) memset(pextwifi, 0, sizeof(net_if_wifi_class_extension_t));
      pnetif->pdrv = p;
      p->extension.wifi = pextwifi;
      p->extension.wifi->scan = es_wifi_scan;
      p->extension.wifi->get_scan_results = es_wifi_get_scan_results;
      ;

      (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_INITIALIZED);

      ret = NET_OK;
    }
  }
  else
  {
    NET_DBG_ERROR("can't allocate memory for es_wifi_driver class\n");
    ret = NET_ERROR_NO_MEMORY;
  }
  return ret;
}

static int32_t es_wifi_if_deinit(net_if_handle_t *pnetif)
{
  /*cstat -MISRAC2012-Rule-21.3 */
  NET_FREE(pnetif->pdrv->extension.wifi);
  /*cstat -MISRAC2012-Rule-21.3 */
  pnetif->pdrv->extension.wifi = NULL;
  NET_FREE(pnetif->pdrv);
  pnetif->pdrv = NULL;
  return NET_OK;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
int32_t es_wifi_if_start(net_if_handle_t *pnetif)
{
  int32_t ret;

  ES_WIFIObject_t  *pEsWifiObj;

  if (wifi_probe(&pnetif->pdrv->context) == NET_OK)
  {
    pEsWifiObj = castcontext(pnetif->pdrv->context);
    /* Init the WiFi module */
    if (ES_WIFI_STATUS_OK != ES_WIFI_Init(pEsWifiObj))
    {
      ret = NET_ERROR_INTERFACE_FAILURE;
    }
    else
    {
      /* Retrieve the WiFi module information */

      (void)strncpy(pnetif->DeviceName, (char_t *)pEsWifiObj->Product_Name, NET_DEVICE_NAME_LEN);
      (void) strncpy(pnetif->DeviceID, (char_t *)pEsWifiObj->Product_ID, NET_DEVICE_ID_LEN);
      (void) strncpy(pnetif->DeviceVer, (char_t *)pEsWifiObj->FW_Rev, NET_DEVICE_VER_LEN);
      (void) ES_WIFI_GetMACAddress(pEsWifiObj, pnetif->macaddr.mac);


      /* Initialise the Channels*/
      for (int32_t i = 0; i < ESWIFI_MAX_CHANNEL_NBR; i++)
      {
        WifiChannel[i].status          = WIFI_FREE_SOCKET;
        WifiChannel[i].Number          = (uint8_t) i;
        WifiChannel[i].recvtimeout     = MIN((NET_SOCK_DEFAULT_RECEIVE_TO), (ES_WIFI_MAX_SO_TIMEOUT));
        WifiChannel[i].sendtimeout     = MIN((NET_SOCK_DEFAULT_SEND_TO), (ES_WIFI_MAX_SO_TIMEOUT));
        WifiChannel[i].blocking        = 1; /* default blocking */
        WifiChannel[i].pnetif          = pnetif;
      }
      (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_READY);
      ret = NET_OK;
    }
  }
  else
  {
    ret = NET_ERROR_DEVICE_ERROR;
  }
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t es_wifi_if_stop(net_if_handle_t *pnetif)
{
  (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_INITIALIZED);
  return NET_OK;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */

#define MATCH(a,b)      (((a) & (b)) == (b))
ES_WIFI_SecurityType_t  GetESWifiSecurityType(int32_t    security_in);
ES_WIFI_SecurityType_t  GetESWifiSecurityType(int32_t    security_in)
{
  ES_WIFI_SecurityType_t        ret = ES_WIFI_SEC_UNKNOWN;
  uint32_t    security = (uint32_t)     security_in;

  if (security == NET_WIFI_SM_OPEN)
  {
    ret = ES_WIFI_SEC_OPEN;
  }

  if (MATCH(security, NET_WEP_ENABLED))
  {
    ret = ES_WIFI_SEC_WEP;
  }
  if (MATCH(security, NET_WIFI_SM_WPA_MIXED_PSK))
  {
    ret = ES_WIFI_SEC_WPA;
  }
  if (MATCH(security, NET_WIFI_SM_WPA_TKIP_PSK))
  {
    ret = ES_WIFI_SEC_WPA;
  }
  if (MATCH(security, NET_WIFI_SM_WPA_AES_PSK))
  {
    ret = ES_WIFI_SEC_WPA;
  }

  if (MATCH(security, NET_WIFI_SM_WPA2_WPA_PSK))
  {
    ret = ES_WIFI_SEC_WPA_WPA2;
  }
  if (MATCH(security, NET_WIFI_SM_WPA2_TKIP_PSK))
  {
    ret = ES_WIFI_SEC_WPA_WPA2;
  }
  if (MATCH(security, NET_WIFI_SM_WPA2_AES_PSK))
  {
    ret = ES_WIFI_SEC_WPA_WPA2;
  }
  if (MATCH(security, NET_WIFI_SM_WPA2_MIXED_PSK))
  {
    ret = ES_WIFI_SEC_WPA_WPA2;
  }

  return ret;
}

static int32_t es_wifi_if_connect_sta(net_if_handle_t *pnetif)
{
  ES_WIFI_Status_t      ret;
  uint8_t addr[4];
  ES_WIFIObject_t  *pEsWifiObj = castcontext(pnetif->pdrv->context);

  const net_wifi_credentials_t *credentials =  pnetif->pdrv->extension.wifi->credentials;

  ret = ES_WIFI_Connect(pEsWifiObj, credentials->ssid, credentials->psk,
                        GetESWifiSecurityType(credentials->security_mode));
  if (ret == ES_WIFI_STATUS_OK)
  {
    ret = ES_WIFI_GetNetworkSettings(pEsWifiObj);
  }

  if (ES_WIFI_STATUS_OK == ret)
  {
    if (pEsWifiObj->NetSettings.IsConnected != 0U)
    {
      (void) memcpy(addr, pEsWifiObj->NetSettings.IP_Addr, 4);
      pnetif->ipaddr.addr = NET_ARTON(addr);
      (void) net_state_manage_event(pnetif, NET_EVENT_IPADDR);
      ret = ES_WIFI_STATUS_OK;

    }
    else
    {
      ret = ES_WIFI_STATUS_ERROR;
    }
  }
  return (int32_t)ret;
}



static int32_t es_wifi_if_connect_ap(net_if_handle_t *pnetif)
{
  ES_WIFI_Status_t      ret;
  ES_WIFI_APConfig_t     ApConfig;
  ES_WIFIObject_t  *pEsWifiObj = castcontext(pnetif->pdrv->context);
  const net_wifi_credentials_t *credentials =  pnetif->pdrv->extension.wifi->credentials;


  (void) strncpy((char_t *) ApConfig.SSID, credentials->ssid, sizeof(ApConfig.SSID));
  (void) strncpy((char_t *) ApConfig.Pass, credentials->psk, sizeof(ApConfig.SSID));
  ApConfig.Security = GetESWifiSecurityType(credentials->security_mode);

  ApConfig.Channel = pnetif->pdrv->extension.wifi->access_channel;
  ApConfig.MaxConnections = pnetif->pdrv->extension.wifi->max_connections;
  if (pnetif->pdrv->extension.wifi->AP_hidden)
  {
    ApConfig.Hidden = 1;
  }
  else
  {
    ApConfig.Hidden = 0;
  }

  ret = ES_WIFI_ActivateAP(pEsWifiObj, &ApConfig);

  if (ret == ES_WIFI_STATUS_OK)
  {
    /*FIX ME ? gateway/blabla  Address by default is 192.168.10.1 */
    (void) NET_ATON("192.168.10.1", &(pnetif->ipaddr));
    (void) net_state_manage_event(pnetif, NET_EVENT_IPADDR);
    ret = ES_WIFI_STATUS_OK;
  }
  return (int32_t)ret;
}


static int32_t es_wifi_if_connect(net_if_handle_t *pnetif)
{
  int32_t      ret;
  if (pnetif->pdrv->extension.wifi->mode == NET_WIFI_MODE_STA)
  {
    ret =  es_wifi_if_connect_sta(pnetif);
  }
  else
  {
    ret =  es_wifi_if_connect_ap(pnetif);
  }
  return ret;
}



/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t es_wifi_if_disconnect(net_if_handle_t *pnetif)
{
  ES_WIFIObject_t  *pEsWifiObj = castcontext(pnetif->pdrv->context);
  (void) ES_WIFI_Disconnect(pEsWifiObj);
  (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_READY);
  return NET_OK;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t es_wifi_socket(int32_t domain, int32_t type, int32_t protocol_in)
{
  int32_t ret = NET_OK;
  int32_t protocol = protocol_in;

  if (domain != NET_AF_INET)
  {
    ret = NET_ERROR_UNSUPPORTED;
  }
  else
  {
    /* currently only SOCK_DGRAM and SOCK_STREAM are supported */
    if ((type != (int32_t) NET_SOCK_STREAM) && (type != (int32_t) NET_SOCK_DGRAM))
    {
      ret = NET_ERROR_UNSUPPORTED;
    }
    else
    {
      /* Only support PROT_IP/TCP/UDP/IPV4 are supported */
      if ((protocol != NET_IPPROTO_TCP) && (protocol != NET_IPPROTO_UDP))
      {
        if (type == (int32_t) NET_SOCK_STREAM)
        {
          protocol = NET_IPPROTO_TCP;
        }
        /*cstat -MISRAC2012-Rule-14.3_a */
        else if (type == (int32_t) NET_SOCK_DGRAM)
          /*cstat +MISRAC2012-Rule-14.3_a */
        {
          protocol = NET_IPPROTO_UDP;
        }
        else
        {
          ret = NET_ERROR_UNSUPPORTED;
        }
      }
    }
  }

  if (ret == NET_OK)
  {
    ret = NET_ERROR_INVALID_SOCKET;

    for (int32_t i = 0; i < ESWIFI_MAX_CHANNEL_NBR; i++)
    {
      if (WifiChannel[i].status == WIFI_FREE_SOCKET)
      {
        /* Initialise the socket*/
        WifiChannel[i].status = WIFI_ALLOCATED_SOCKET;
        if (type == (int32_t) NET_SOCK_DGRAM)
        {
          WifiChannel[i].status |= WIFI_SEND_OK | WIFI_RECV_OK;
        }
        WifiChannel[i].protocol        = (uint8_t) protocol;
        WifiChannel[i].type            = (uint8_t) type;
        ret = i;
        break;
      }
    }
  }

  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t es_wifi_bind(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen)
{
  int32_t ret;
  sockaddr_in_t *source;

  if ((sock < 0) || (sock >= ESWIFI_MAX_CHANNEL_NBR))
  {
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    /* STREAM sockets cannot be bound after connection. */
    if ((WifiChannel [sock].status == WIFI_ALLOCATED_SOCKET)
        || (((WifiChannel [sock].status & WIFI_ALLOCATED_SOCKET) != 0U)
            && ((uint8_t) WifiChannel [sock].type == (uint8_t) NET_SOCK_DGRAM)))
    {
      if (addrlen == sizeof(sockaddr_in_t))
      {
        source = cast2sockaddr_in(addr);
        WifiChannel [sock].localport = NET_NTOHS(source->sin_port);
        WifiChannel [sock].localaddress = source->sin_addr.s_addr;
        WifiChannel [sock].status |= WIFI_BIND_SOCKET;
        ret = NET_OK;
      }
      else
      {
        ret = NET_ERROR_PARAMETER;
      }
    }
    else
    {
      ret = NET_ERROR_INVALID_SOCKET;
    }
  }
  return ret;
}


/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t es_wifi_listen(int32_t sock, int32_t backlog)
{
  int32_t ret;

  if ((sock < 0) || (sock >= ESWIFI_MAX_CHANNEL_NBR))
  {
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    ES_WIFIObject_t  *pEsWifiObj = castcontext(WifiChannel[sock].pnetif->pdrv->context);
    ES_WIFI_Conn_t conn;

    conn.Number = (uint8_t) sock;
    conn.LocalPort = WifiChannel[sock].localport;
    conn.Type = ES_WIFI_UDP_CONNECTION;
    if (WifiChannel[sock].protocol == (uint8_t) NET_IPPROTO_TCP)
    {
      conn.Type = ES_WIFI_TCP_CONNECTION;
    }
    conn.Backlog = (uint8_t) backlog;
    ret = (int32_t) ES_WIFI_StartServerSingleConn(pEsWifiObj, &conn);
  }

  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t es_wifi_accept(int32_t sock, net_sockaddr_t *addr, uint32_t *addrlen)
{
  int32_t ret;
  ES_WIFI_Status_t      status;

  (void) addrlen;
  sockaddr_in_t *addrin = cast2sockaddr_in(addr);

  if ((sock < 0) || (sock >= ESWIFI_MAX_CHANNEL_NBR))
  {
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    ES_WIFI_Conn_t conn;

    conn.Number = (uint8_t) sock;
    /* 0 value timeout means forever here */
    status = ES_WIFI_WaitServerConnection(castcontext(WifiChannel[sock].pnetif->pdrv->context), 0, &conn);

    if (ES_WIFI_STATUS_OK == status)
    {
      addrin->sin_port = NET_NTOHS(conn.RemotePort);
      addrin->sin_addr.s_addr = NET_ARTON(conn.RemoteIP);
      WifiChannel [sock].status |= WIFI_CONNECTED_SOCKET_RW | WIFI_STARTED_SERVER_SOCKET ;
      ret = sock;
    }
    else
    {
      ret =  NET_ERROR_SOCKET_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t es_wifi_connect(int32_t sock,  const net_sockaddr_t *addr, uint32_t addrlen)
{
  int32_t  ret;
  sockaddr_in_t *dest;

  if ((sock < 0) || (sock >= ESWIFI_MAX_CHANNEL_NBR))
  {
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    ES_WIFI_Conn_t conn;
#ifdef NET_MBEDTLS_WIFI_MODULE_SUPPORT
    eswifi_tls_data_t *tlsdata;
    uint32_t      casize;
    uint32_t      keysize;
    uint32_t      devsize;
#endif /* NET_MBEDTLS_WIFI_MODULE_SUPPORT */

    /*addr is an outbound address */
    if (addrlen == sizeof(sockaddr_in_t))
    {
      dest = cast2sockaddr_in(addr);
      conn.Number = (uint8_t) sock;
      conn.LocalPort = WifiChannel[sock].localport;
      conn.RemotePort = NET_NTOHS(dest->sin_port);
      NET_NTOAR((dest->sin_addr.s_addr), (conn.RemoteIP));

      switch (WifiChannel[sock].protocol)
      {
        case NET_IPPROTO_TCP :
          conn.Type = ES_WIFI_TCP_CONNECTION;
          ret = NET_OK;
          break;

        case NET_IPPROTO_UDP :
          ret = NET_ERROR_UNSUPPORTED;
          break;

        case NET_IPPROTO_TCP_TLS :
#ifdef NET_MBEDTLS_WIFI_MODULE_SUPPORT
          tlsdata = &WifiChannel[sock].tlsData;
          conn.Type = ES_WIFI_TCP_SSL_CONNECTION;

          casize  = (tlsdata->tls_ca_certs == 0) ? 0 : strlen(tlsdata->tls_ca_certs) + 1;
          keysize = (tlsdata->tls_dev_key == 0) ? 0 : strlen(tlsdata->tls_dev_key) + 1;
          devsize = (tlsdata->tls_dev_cert == 0) ? 0 : strlen(tlsdata->tls_dev_cert) + 1;
          conn.TLScheckMode = ES_WIFI_TLS_CHECK_NOTHING;
          if (casize)
          {
            conn.TLScheckMode = ES_WIFI_TLS_CHECK_ROOTCA;
          }
          if (keysize)
          {
            conn.TLScheckMode = ES_WIFI_TLS_CHECK_DEVICE_CERTS;
          }
          ret = ES_WIFI_StoreCreds(WifiChannel[sock].pnetif->pdrv->context, ES_WIFI_FUNCTION_TLS,
                                   ES_WIFI_TLS_MULTIPLE_WRITE_SLOT,
                                   (uint8_t *) tlsdata->tls_ca_certs, casize,
                                   (uint8_t *) tlsdata->tls_dev_cert, devsize,
                                   (uint8_t *) tlsdata->tls_dev_key, keysize);

#else
          ret = NET_ERROR_UNSUPPORTED;
#endif /* NET_MBEDTLS_WIFI_MODULE_SUPPORT */
          break;

        default:
          ret = NET_ERROR_PARAMETER;
          break;
      }

      if (ret == NET_OK)
      {
        if (ES_WIFI_STATUS_OK ==
            ES_WIFI_StartClientConnection(castcontext(WifiChannel[sock].pnetif->pdrv->context), &conn))
        {
          WifiChannel[sock].status |= WIFI_CONNECTED_SOCKET_RW;
        }
        else
        {
          ret = NET_ERROR_SOCKET_FAILURE;
        }
      }
    }
    else
    {
      ret = NET_ERROR_PARAMETER;
    }
  }
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t es_wifi_shutdown(int32_t sock, int32_t mode)
{
  int32_t       ret;
  if ((sock < 0) || (sock >= ESWIFI_MAX_CHANNEL_NBR))
  {
    ret =  NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if ((WifiChannel[sock].status &  WIFI_CONNECTED_SOCKET) != 0U)
    {
      if (mode == NET_SHUTDOWN_R)
      {
        WifiChannel[sock].status &=  ~WIFI_RECV_OK;
      }
      if (mode == NET_SHUTDOWN_W)
      {
        WifiChannel[sock].status &=  ~WIFI_SEND_OK;
      }
      if (mode == NET_SHUTDOWN_RW)
      {
        WifiChannel[sock].status &=  ~(WIFI_SEND_OK | WIFI_RECV_OK);
      }
    }
    ret = NET_OK;
  }
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t es_wifi_close(int32_t sock, bool isaclone)
{
  int32_t ret;
  if ((sock < 0) || (sock >= ESWIFI_MAX_CHANNEL_NBR))
  {
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if ((WifiChannel [sock].status & WIFI_STARTED_SERVER_SOCKET) != 0U)
    {
      if (isaclone)
      {
        if (ES_WIFI_CloseServerConnection(castcontext(WifiChannel[sock].pnetif->pdrv->context), sock)
            != ES_WIFI_STATUS_OK)
        {
          WifiChannel[sock].status = WIFI_FREE_SOCKET;
          ret = NET_ERROR_SOCKET_FAILURE;
        }
        else
        {
          WifiChannel[sock].status = WIFI_FREE_SOCKET;
          ret = NET_OK;
        }
      }
      else
      {
        if (ES_WIFI_StopServerSingleConn(castcontext(WifiChannel[sock].pnetif->pdrv->context), sock)
            != ES_WIFI_STATUS_OK)
        {
          WifiChannel[sock].status = WIFI_FREE_SOCKET;
          ret = NET_ERROR_SOCKET_FAILURE;
        }
        else
        {
          WifiChannel[sock].status = WIFI_FREE_SOCKET;
          ret = NET_OK;
        }
      }
    }
    else
    {
      ES_WIFI_Conn_t conn;
      conn.Number = (uint8_t) sock;
      if (ES_WIFI_StopClientConnection(castcontext(WifiChannel[sock].pnetif->pdrv->context), &conn)
          != ES_WIFI_STATUS_OK)
      {
        WifiChannel[sock].status = WIFI_FREE_SOCKET;
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      else
      {
        WifiChannel[sock].status = WIFI_FREE_SOCKET;
        ret = NET_OK;
      }
    }
  }
  return ret;
}


static void check_connection_lost(net_if_handle_t *pnetif, int32_t n)
{
  if (n <= 0)
  {
    if (NET_STATE_CONNECTED == pnetif->state)
    {
      if (ES_WIFI_IsConnected(castcontext(pnetif->pdrv->context)) == 0u)
      {
        (void) net_if_notify(pnetif, NET_EVENT_STATE_CHANGE, (uint32_t) NET_STATE_CONNECTION_LOST, NULL);
      }
    }
  }
  else
  {
    if (NET_STATE_CONNECTION_LOST == pnetif->state)
    {
      (void) net_if_notify(pnetif, NET_EVENT_STATE_CHANGE, (uint32_t) NET_STATE_CONNECTED, NULL);
    }
  }
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t es_wifi_send(int32_t sock, uint8_t *buf, int32_t length, int32_t flags)
{
  int32_t ret;
  int32_t len = length;
  uint16_t SentDatalen;

  if (flags != 0)
  {
    ret = NET_ERROR_UNSUPPORTED;
  }
  else if ((sock < 0) || (sock >= ESWIFI_MAX_CHANNEL_NBR))
  {
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else if ((WifiChannel [sock].status & WIFI_SEND_OK) == 0U)
  {
    ret = NET_ERROR_SOCKET_FAILURE;
  }
  else
  {

    if (len > ES_WIFI_PAYLOAD_SIZE)
    {
      len = ES_WIFI_PAYLOAD_SIZE;
    }

    if (ES_WIFI_STATUS_OK == ES_WIFI_SendData(castcontext(WifiChannel[sock].pnetif->pdrv->context),
                                              (uint8_t) sock,
                                              buf,
                                              (uint16_t) len,
                                              &SentDatalen,
                                              (uint32_t) WifiChannel[sock].sendtimeout))
    {
      ret = (int32_t) SentDatalen;
    }
    else
    {
      ret = NET_ERROR_SOCKET_FAILURE;
    }
    check_connection_lost(WifiChannel[sock].pnetif, ret);
  }
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t es_wifi_recv(int32_t sock, uint8_t *buf, int32_t length, int32_t flags)
{
  int32_t ret;
  int32_t len = length;
  uint16_t ReceivedDatalen;
  uint32_t     timeout;


  if ((sock < 0) || (sock >= ESWIFI_MAX_CHANNEL_NBR))
  {
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else if ((WifiChannel [sock].status & WIFI_RECV_OK) == 0U)
  {
    ret = NET_ERROR_SOCKET_FAILURE;
  }
  else
  {
    if (flags == (int32_t) NET_MSG_DONTWAIT)
    {
      timeout = 0;
    }
    else
    {
      timeout = (uint16_t) WifiChannel[sock].recvtimeout;
    }

    if (len > ES_WIFI_PAYLOAD_SIZE)
    {
      len = ES_WIFI_PAYLOAD_SIZE;
    }

    if (ES_WIFI_STATUS_OK == ES_WIFI_ReceiveData(castcontext(WifiChannel[sock].pnetif->pdrv->context),
                                                 (uint8_t) sock,
                                                 buf,
                                                 (uint16_t) len,
                                                 &ReceivedDatalen,
                                                 (uint32_t) timeout))
    {
      ret = (int32_t) ReceivedDatalen;
    }
    else
    {
      ret = NET_ERROR_SOCKET_FAILURE;
    }

    check_connection_lost(WifiChannel[sock].pnetif, ret);
  }
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t es_wifi_sendto(int32_t sock, uint8_t *buf, int32_t length, int32_t flags, net_sockaddr_t *to,
                              uint32_t tolen)
{
  int32_t ret = NET_OK;
  int32_t len = length;
  /*cstat -MISRAC2012-Rule-14.3_b */
  if ((flags != 0) || (to->sa_family != (uint8_t) NET_AF_INET))
    /*cstat +MISRAC2012-Rule-14.3_b */
  {
    ret = NET_ERROR_UNSUPPORTED;
  }
  /*cstat -MISRAC2012-Rule-14.3_b */
  else if ((sock < 0) || (sock >= ESWIFI_MAX_CHANNEL_NBR))
    /*cstat +MISRAC2012-Rule-14.3_b */
  {
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else if (WifiChannel[sock].protocol != (uint8_t) NET_IPPROTO_UDP)
  {
    ret = NET_ERROR_SOCKET_FAILURE;
  }
  else if ((WifiChannel [sock].status & WIFI_SEND_OK) == 0U)
  {
    ret =  NET_ERROR_SOCKET_FAILURE;
  }
  else
  {
    if ((WifiChannel [sock].status & WIFI_STARTED_CLIENT_SOCKET) == 0U)
    {
      sockaddr_in_t *dest;
      ES_WIFI_Conn_t conn;

      if (tolen == sizeof(sockaddr_in_t))
      {
        dest = cast2sockaddr_in(to);
        conn.Number = (uint8_t) sock;
        conn.LocalPort = WifiChannel[sock].localport;
        conn.RemotePort = NET_NTOHS(dest->sin_port);
        NET_NTOAR((dest->sin_addr.s_addr), (conn.RemoteIP));
        conn.Type = ES_WIFI_UDP_CONNECTION;

        if (ES_WIFI_STATUS_OK ==
            ES_WIFI_StartClientConnection(castcontext(WifiChannel[sock].pnetif->pdrv->context), &conn))
        {
          WifiChannel[sock].status |= WIFI_STARTED_CLIENT_SOCKET;
        }
        else
        {
          ret = NET_ERROR_SOCKET_FAILURE;
        }
      }
    }

    if (ret == NET_OK)
    {
      uint16_t SentDatalen;
      /*cstat -MISRAC2012-Rule-14.3_b */
      if (len > ES_WIFI_PAYLOAD_SIZE)
        /*cstat +MISRAC2012-Rule-14.3_b */
      {
        len = ES_WIFI_PAYLOAD_SIZE;
      }

      uint32_t remoteaddr ;
      sockaddr_in_t *saddr = cast2sockaddr_in(to);
      remoteaddr = NET_NTOHL(saddr->sin_addr.s_addr);

      uint32_t IPaddr = NET_NTOHL(remoteaddr);
      uint16_t Port = NET_NTOHS(saddr->sin_port);

      if (ES_WIFI_STATUS_OK == ES_WIFI_SendDataTo(castcontext(WifiChannel[sock].pnetif->pdrv->context),
                                                  (uint8_t) sock,
                                                  buf,
                                                  (uint16_t) len,
                                                  &SentDatalen,
                                                  (uint32_t) WifiChannel[sock].sendtimeout,
                                                  (uint8_t *) &IPaddr,
                                                  Port))
      {
        ret = (int32_t) SentDatalen;
      }
      else
      {
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      check_connection_lost(WifiChannel[sock].pnetif, ret);
    }
  }
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
/*cstat -MISRAC2012-Rule-2.2_b */
static int32_t es_wifi_recvfrom(int32_t sock, uint8_t *buf, int32_t length, int32_t flagsparam, net_sockaddr_t *from,
                                uint32_t *fromlen)
/*cstat +MISRAC2012-Rule-2.2_b */
{
  int32_t ret = NET_OK;
  uint32_t timeout = 0;
  int32_t flags = flagsparam;
  int32_t len = length;

  if ((sock < 0) || (sock >= ESWIFI_MAX_CHANNEL_NBR))
  {
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else if (WifiChannel[sock].protocol != (uint8_t) NET_IPPROTO_UDP)
  {
    ret = NET_ERROR_SOCKET_FAILURE;
  }
  else if ((WifiChannel [sock].status & WIFI_RECV_OK) == 0U)
  {
    ret = NET_ERROR_SOCKET_FAILURE;
  }
  else
  {
    if (((uint8_t) flags & (uint8_t) NET_MSG_DONTWAIT) == 0U)
    {
      timeout = (uint32_t) WifiChannel[sock].recvtimeout;
    }

    if (timeout > (uint32_t) ES_WIFI_MAX_SO_TIMEOUT)
    {
      ret = NET_ERROR_UNSUPPORTED;
    }
    else
    {
      if ((WifiChannel [sock].status & WIFI_STARTED_CLIENT_SOCKET) == 0U)
      {
        /* TODO: If starting by reading, would mean that es_wifi "server mode" is needed?       */
        /* Workaround sending a fake packet, so that further recvfrom() is possible without error.  */
        /* If no better idea, could send an mdns advertisement.                   */

        flags = 0;
        net_sockaddr_t saddr;
        net_ip_addr_t ipaddr;

        (void) NET_ATON((const char_t *)"224.0.0.251", &ipaddr);
        /*cstat -MISRAC2012-Rule-11.5 -MISRAC2012-Rule-17.7 */
        NET_IPADDR_PORT_TO_SOCKADDR(&saddr, &ipaddr, 5353);
        /*cstat +MISRAC2012-Rule-11.5 +MISRAC2012-Rule-17.7 */
        ret = es_wifi_sendto(sock, NULL, 0, flags, &saddr, sizeof(sockaddr_in_t));
      }

      if (ret == NET_OK)
      {
        uint16_t ReceivedDatalen;
        uint32_t IPaddr = 0;
        uint16_t port = 0;

        if (len > ES_WIFI_PAYLOAD_SIZE)
        {
          len = ES_WIFI_PAYLOAD_SIZE;
        }

        if (ES_WIFI_STATUS_OK == ES_WIFI_ReceiveDataFrom(castcontext(WifiChannel[sock].pnetif->pdrv->context),
                                                         (uint8_t) sock,
                                                         buf,
                                                         (uint16_t) len,
                                                         &ReceivedDatalen,
                                                         timeout,
                                                         (uint8_t *) &IPaddr,
                                                         &port))
        {
          sockaddr_in_t saddr;
          (void) memset(&saddr, 0, sizeof(sockaddr_in_t));
          saddr.sin_family = NET_AF_INET;
          saddr.sin_addr.s_addr = IPaddr;
          saddr.sin_port = NET_HTONS(port);
          saddr.sin_len = (uint8_t) sizeof(sockaddr_in_t);
          (void)  memset(from, 0, *fromlen);
          (void)  memcpy(from, &saddr, MIN((*fromlen), (saddr.sin_len)));
          *fromlen = sizeof(sockaddr_in_t);
          ret = (int32_t) ReceivedDatalen;
        }
        else
        {
          ret = NET_ERROR_SOCKET_FAILURE;
        }
      }

      check_connection_lost(WifiChannel[sock].pnetif, ret);
    }
  }
  return ret;

}


static uint32_t conv_to_net_security(ES_WIFI_SecurityType_t     sec)
{
  uint32_t ret;
  switch (sec)
  {
    case ES_WIFI_SEC_OPEN:
      ret = NET_WIFI_SM_OPEN;
      break;
    case ES_WIFI_SEC_WEP:
      ret = NET_WIFI_SM_WEP_PSK;
      break;
    case ES_WIFI_SEC_WPA:
      ret = NET_WIFI_SM_WPA_MIXED_PSK;
      break;
    case ES_WIFI_SEC_WPA2:
      ret = NET_WIFI_SM_WPA2_MIXED_PSK;
      break;
    case ES_WIFI_SEC_WPA_WPA2:
      ret = NET_WIFI_SM_WPA2_MIXED_PSK;
      break;
    case ES_WIFI_SEC_WPA2_TKIP:
      ret = NET_WIFI_SM_WPA2_TKIP_PSK;
      break;
    default :
      ret = NET_WIFI_SM_UNKNOWN;
      break;

  }
  return ret;
}

/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */

#define OPTCHECKTYPE(type,optlen)       if (sizeof(type)!= optlen) return NET_ERROR_PARAMETER
#define OPTCHECKSTRING(opt,optlen)       if (strlen(opt)!= (optlen-1)) return NET_ERROR_PARAMETER

static int32_t es_wifi_setsockopt(int32_t sock, int32_t level, int32_t optname, const void *optvalue, uint32_t optlen)
{
  int32_t ret = NET_ERROR_PARAMETER;
  /*cstat -MISRAC2012-Rule-11.8 -MISRAC2012-Rule-11.5 */
  int32_t       *optint32 = (int32_t *)optvalue;
  bool          *optbool = (bool *)optvalue;
  /*cstat +MISRAC2012-Rule-11.8 +MISRAC2012-Rule-11.5  */

  (void) level;

  if ((sock < 0) || (sock >= ESWIFI_MAX_CHANNEL_NBR))
  {
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    switch (optname)
    {
      case NET_SO_RCVTIMEO:
        /* Receive time out. */
        WifiChannel[sock].recvtimeout = * optint32;
        ret = NET_OK;
        break;

      case NET_SO_SNDTIMEO:
        WifiChannel[sock].sendtimeout = *optint32;

        if (WifiChannel[sock].protocol == (uint8_t) NET_IPPROTO_UDP)
        {
          if (WifiChannel[sock].sendtimeout > NET_UDP_MAX_SEND_BLOCK_TO)
          {
            WifiChannel[sock].sendtimeout = NET_UDP_MAX_SEND_BLOCK_TO;
          }
        }
        ret = NET_OK;
        break;
        /*
            case NET_SO_EVENT_CALLBACK:
               WifiChannel[sock].notify_callback = ((net_event_handler_t*)optvalue)->callback;
               WifiChannel[sock].notify_context = ((net_event_handler_t*)optvalue)->context;
              ret = NET_OK;
              break;
        */

#ifdef NET_MBEDTLS_WIFI_MODULE_SUPPORT
      case NET_SO_SECURE:

        if (WifiChannel[sock].protocol == (uint8_t) NET_IPPROTO_TCP)
        {
          WifiChannel[sock].protocol = (uint8_t) NET_IPPROTO_TCP_TLS;
          ret =  NET_OK;
        }
        else
        {
          NET_DBG_ERROR("Only TCP socket supports TLS protocol\n");
        }
        break;

      case NET_SO_TLS_DEV_CERT:

        OPTCHECKSTRING(optvalue, optlen);
        if (WifiChannel[sock].protocol != (uint8_t) NET_IPPROTO_TCP_TLS)
        {
          NET_DBG_ERROR("Failed to set tls device certificats, please set socket as secure before\n");
          ret = NET_ERROR_IS_NOT_SECURE;
        }
        WifiChannel[sock].tlsData.tls_dev_cert = (char_t *) optvalue;
        ret =  NET_OK;
        break;

      case NET_SO_TLS_DEV_KEY:

        OPTCHECKSTRING(optvalue, optlen);
        if (WifiChannel[sock].protocol != (uint8_t) NET_IPPROTO_TCP_TLS)
        {
          NET_DBG_ERROR("Failed to set tls device key, please set socket as secure before\n");
          return NET_ERROR_IS_NOT_SECURE;
        }
        WifiChannel[sock].tlsData.tls_dev_key = (char_t *) optvalue;
        ret =  NET_OK;
        break;

      case NET_SO_TLS_CA_CERT:

        if (WifiChannel[sock].protocol != (uint8_t) NET_IPPROTO_TCP_TLS)
        {
          NET_DBG_ERROR("Failed to set tls root ca, please set socket as secure before\n");
          return NET_ERROR_IS_NOT_SECURE;
        }
        /* FIXME OPTCHECKSTRING(optvalue,optlen); */
        WifiChannel[sock].tlsData.tls_ca_certs = (char_t *) optvalue;
        ret =  NET_OK;
        break;

      case NET_SO_TLS_CA_CRL:

        OPTCHECKSTRING(optvalue, optlen);
        if (WifiChannel[sock].protocol != (uint8_t) NET_IPPROTO_TCP_TLS)
        {
          NET_DBG_ERROR("Failed to set tls root ca, please set socket as secure before\n");
          return NET_ERROR_IS_NOT_SECURE;
        }
        WifiChannel[sock].tlsData.tls_ca_crl = (char_t *) optvalue;
        ret =  NET_OK;
        break;

      case NET_SO_TLS_PASSWORD:

        OPTCHECKSTRING(optvalue, optlen);
        if (WifiChannel[sock].protocol != (uint8_t) NET_IPPROTO_TCP_TLS)
        {
          NET_DBG_ERROR("Failed to set tls password, please set socket as secure before\n");
          return NET_ERROR_IS_NOT_SECURE;
        }
        WifiChannel[sock].tlsData.tls_dev_pwd = (unsigned char_t *) optvalue;
        ret =  NET_OK;
        break;


      case NET_SO_TLS_SERVER_VERIFICATION:

        if (sizeof(bool) != optlen)
        {
          ret = NET_ERROR_PARAMETER;
        }
        else
        {
          if (WifiChannel[sock].protocol != (uint8_t) NET_IPPROTO_TCP_TLS)
          {
            NET_DBG_ERROR("Failed to set tls server verification, please set socket as secure before\n");
            ret = NET_ERROR_IS_NOT_SECURE;
          }
          else
          {
            WifiChannel[sock].tlsData.tls_srv_verification = (*(bool *)optvalue > 0) ? true : false;
            ret = NET_OK;
          }
        }
        break;

      case NET_SO_TLS_SERVER_NAME:

        OPTCHECKSTRING(optvalue, optlen);
        if (WifiChannel[sock].protocol != (uint8_t) NET_IPPROTO_TCP_TLS)
        {
          NET_DBG_ERROR("Failed to set tls server name, please set socket as secure before\n");
          return NET_ERROR_IS_NOT_SECURE;
        }
        WifiChannel[sock].tlsData.tls_srv_name = (char_t *)optvalue;
        ret = NET_OK;
        break;
#endif /* NET_MBEDTLS_WIFI_MODULE_SUPPORT */
      case  NET_SO_BLOCKING:
        if (sizeof(bool) != optlen)
        {
          ret = NET_ERROR_PARAMETER;
        }
        else
        {
          if (*optbool == false)
          {
            /* Workaroud , non blocking socket are not supported so force timeout to min value */
            WifiChannel[sock].sendtimeout = 1;
            WifiChannel[sock].recvtimeout = 1;
          }
          ret =  NET_OK;
        }
        break;

      case  NET_SO_TLS_CERT_PROF:
        NET_DBG_ERROR("Ignore this option because it embedded inside Inventek Wifi module and can not be changed\n");
        ret =  NET_OK;
        break;


      default :
        break;
    }
  }
  return ret;
}

static int32_t es_wifi_getsockopt(int32_t sock, int32_t level, int32_t optname, void *optvalue, uint32_t *optlen)
{
  (void) sock;
  (void) level;
  (void) optname;
  (void) optvalue;
  (void) optlen;

  return NET_ERROR_UNSUPPORTED;
}

static int32_t es_wifi_getsockname(int32_t sock, net_sockaddr_t *name, uint32_t *namelen)
{
  int32_t  ret;
  uint32_t  IPaddr;
  uint16_t port;

  if (*namelen < sizeof(sockaddr_in_t))
  {
    ret = NET_ERROR_PARAMETER;
  }
  else
  {
    if ((sock < 0) || (sock >= ESWIFI_MAX_CHANNEL_NBR))
    {
      ret = NET_ERROR_INVALID_SOCKET;
    }
    else
    {
      if (ES_WIFI_STATUS_OK == ES_WIFI_SockInfo(castcontext(WifiChannel[sock].pnetif->pdrv->context),
                                                WifiChannel[sock].Number, (uint8_t *) &IPaddr, &port))
      {
        sockaddr_in_t saddr;
        ret = NET_OK;
        (void) memset(&saddr, 0, sizeof(sockaddr_in_t));
        saddr.sin_family = NET_AF_INET;
        saddr.sin_addr.s_addr = IPaddr;
        saddr.sin_port = NET_HTONS(port);
        saddr.sin_len = (uint8_t) sizeof(sockaddr_in_t);
        (void)  memset(name, 0, *namelen);
        (void)  memcpy(name, &saddr, MIN((*namelen), (saddr.sin_len)));
        *namelen = sizeof(sockaddr_in_t);
      }
      else
      {
        ret = NET_ERROR_NO_CONNECTION;
      }
    }
  }
  return ret;
}


static int32_t es_wifi_getpeername(int32_t sock, net_sockaddr_t *name, uint32_t *namelen)
{
  int32_t  ret;
  uint32_t  IPaddr;
  uint16_t port;

  if (*namelen < sizeof(sockaddr_in_t))
  {
    ret = NET_ERROR_PARAMETER;
  }
  else
  {
    if ((sock < 0) || (sock >= (int32_t) ESWIFI_MAX_CHANNEL_NBR))
    {
      ret = NET_ERROR_INVALID_SOCKET;
    }
    else
    {
      if (ES_WIFI_STATUS_OK == ES_WIFI_PeerInfo(castcontext(WifiChannel[sock].pnetif->pdrv->context),
                                                WifiChannel[sock].Number, (uint8_t *) &IPaddr, &port))
      {
        sockaddr_in_t saddr;
        ret = NET_OK;
        (void) memset(&saddr, 0, sizeof(sockaddr_in_t));
        saddr.sin_family = NET_AF_INET;
        saddr.sin_addr.s_addr = IPaddr;
        saddr.sin_port = NET_HTONS(port);
        saddr.sin_len = (uint8_t) sizeof(sockaddr_in_t);
        (void)  memset(name, 0, *namelen);
        (void)  memcpy(name, &saddr, MIN((*namelen), (saddr.sin_len)));
        *namelen = sizeof(sockaddr_in_t);
      }
      else
      {
        ret = NET_ERROR_NO_CONNECTION;
      }
    }
  }
  return ret;
}

static int32_t es_wifi_gethostbyname(net_if_handle_t *pnetif, net_sockaddr_t *addr, char_t *name)
{
  int32_t ret;
  uint8_t ipaddr[6];

  if (addr->sa_len < sizeof(sockaddr_in_t))
  {
    ret = NET_ERROR_PARAMETER;
  }
  else
  {
    if (ES_WIFI_STATUS_OK == ES_WIFI_DNS_LookUp(castcontext(pnetif->pdrv->context), (char_t *)name, ipaddr))
    {
      uint8_t len = addr->sa_len;
      sockaddr_in_t *saddr = cast2sockaddr_in(addr);

      (void)  memset(saddr, 0, len);
      saddr->sin_len = len;
      saddr->sin_family = NET_AF_INET;
      (void) memcpy(&(saddr->sin_addr), ipaddr, 4);
      ret = NET_OK;
    }
    else
    {
      ret = NET_ERROR_DNS_FAILURE;
    }
  }
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */


int32_t es_wifi_ping(net_if_handle_t *pnetif, net_sockaddr_t *addr, int32_t count, int32_t delay, int32_t response[])
{
  int32_t ret;
  uint8_t     ipaddr[6];

  (void) memcpy(ipaddr, &addr->sa_data[2], 6);

  if (ES_WIFI_STATUS_OK == ES_WIFI_Ping(castcontext(pnetif->pdrv->context), ipaddr, (uint16_t) count, (uint16_t) delay,
                                        response))
  {
    ret = NET_OK;
  }
  else
  {
    ret = NET_ERROR_NO_CONNECTION;
  }
  return ret;
}

static  int32_t es_wifi_scan(net_if_handle_t *pnetif, net_wifi_scan_mode_t mode, char_t *ssid)
{
  (void) mode;
  (void) pnetif;
  (void) ssid;
  return 0;
}

static  int32_t es_wifi_get_scan_results(net_if_handle_t *pnetif, net_wifi_scan_results_t *results, uint8_t number)
{
  int32_t ret;
  ES_WIFI_APs_t APs;
  ES_WIFI_AP_t *AP = &APs.AP[0];

  if (ES_WIFI_STATUS_OK == ES_WIFI_ListAccessPoints(castcontext(pnetif->pdrv->context), &APs))
  {
    if (APs.nbr > number)
    {
      APs.nbr = number;
    }
    for (uint32_t i = 0; i < APs.nbr ; i++)
    {
      (void) memset((void *) &results[i], 0, sizeof(net_wifi_scan_bss_t));
      (void) memcpy(results[i].ssid.value, AP->SSID, NET_WIFI_MAX_SSID_SIZE);
      results[i].ssid.length = (uint8_t) strlen((char_t *) AP->SSID);
      results[i].security = conv_to_net_security(AP->Security);
      (void)  memcpy(&results[i].bssid, AP->MAC, NET_WIFI_MAC_ADDRESS_SIZE);
      results[i].rssi = (int8_t)AP->RSSI;
      results[i].channel = AP->Channel;
      (void) memcpy(results[i].country, ".CN", 4);  /* NOT SUPPORT for MX_WIFI */
      AP++;
    }
    ret = (int32_t) APs.nbr;
  }
  else
  {
    ret = NET_ERROR_NO_CONNECTION;
  }
  return ret;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

