/**
  ******************************************************************************
  * @file    net_core.h
  * @author  MCD Application Team
  * @brief   Provides the network interface driver APIs.
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
#ifndef NET_CORE_H
#define NET_CORE_H

#ifdef __cplusplus
extern "C" {
#endif
#include "net_state.h"
#include "net_wifi.h"
#include "net_class_extension.h"
/*cstat -MISRAC* -DEFINE-* -CERT-EXP19*  */
/* #include "lwip/err.h" */
/*cstat +MISRAC* +DEFINE-* +CERT-EXP19*  */




int32_t icmp_ping(net_if_handle_t *netif, net_sockaddr_t *addr, int32_t count, int32_t timeout, int32_t response[]);

#ifdef NET_USE_RTOS



void net_init_locks(void);
void net_destroy_locks(void);

void net_lock(int32_t idx, uint32_t to);
void net_unlock(int32_t idx);

void net_lock_nochk(int32_t idx, uint32_t to);
void net_unlock_nochk(int32_t idx);

/* OS */
#define NET_OS_WAIT_FOREVER     0xffffffffU


#define NET_LOCK_SOCKET_ARRAY   NET_MAX_SOCKETS_NBR
#define NET_LOCK_NETIF_LIST     NET_MAX_SOCKETS_NBR+1
#define NET_LOCK_STATE_EVENT    NET_MAX_SOCKETS_NBR+2

#define NET_LOCK_NUMBER          (NET_LOCK_STATE_EVENT+1)

#define  LOCK_SOCK(s)           net_lock((int32_t)s,NET_OS_WAIT_FOREVER)
#define  UNLOCK_SOCK(s)         net_unlock(s)

#define  LOCK_SOCK_ARRAY()      net_lock(NET_LOCK_SOCKET_ARRAY,NET_OS_WAIT_FOREVER)
#define  UNLOCK_SOCK_ARRAY()    net_unlock(NET_LOCK_SOCKET_ARRAY )

#define  LOCK_NETIF_LIST()      net_lock(NET_LOCK_NETIF_LIST,NET_OS_WAIT_FOREVER )
#define  UNLOCK_NETIF_LIST()    net_unlock(NET_LOCK_NETIF_LIST )

#define  WAIT_STATE_CHANGE(to)  net_lock_nochk(NET_LOCK_STATE_EVENT,to )
#define  SIGNAL_STATE_CHANGE()  net_unlock_nochk(NET_LOCK_STATE_EVENT )

#else

#define  LOCK_SOCK(s)
#define  UNLOCK_SOCK(s)
#define  LOCK_SOCK_ARRAY()
#define  UNLOCK_SOCK_ARRAY()
#define  LOCK_NETIF_LIST()
#define  UNLOCK_NETIF_LIST()
#define  WAIT_STATE_CHANGE(to)  pnetif->pdrv->if_yield(pnetif, 10)
#define  SIGNAL_STATE_CHANGE()



#endif /* NET_USE_RTOS */



typedef enum
{
  NET_INTERFACE_CLASS_WIFI,
  NET_INTERFACE_CLASS_CELLULAR,
  NET_INTERFACE_CLASS_ETHERNET,
  NET_INTERFACE_CLASS_CUSTOM
}
net_interface_class_t;


typedef enum
{
  NET_ACCESS_SOCKET,
  NET_ACCESS_BIND,
  NET_ACCESS_LISTEN,
  NET_ACCESS_CONNECT,
  NET_ACCESS_SEND,
  NET_ACCESS_SENDTO,
  NET_ACCESS_RECV,
  NET_ACCESS_RECVFROM,
  NET_ACCESS_CLOSE,
  NET_ACCESS_SETSOCKOPT,
}
net_access_t;

struct net_if_drv_s
{
  net_interface_class_t     if_class;
  /* Interface APIs */
  int32_t (* if_init)(net_if_handle_t *pnetif);
  int32_t (* if_deinit)(net_if_handle_t *pnetif);
  int32_t (* if_start)(net_if_handle_t *pnetif);
  int32_t (* if_stop)(net_if_handle_t *pnetif);
  int32_t (* if_yield)(net_if_handle_t *pnetif, uint32_t timeout);
  int32_t (* if_connect)(net_if_handle_t *pnetif);
  int32_t (* if_disconnect)(net_if_handle_t *pnetif);
  int32_t (* if_powersave_enable)(net_if_handle_t *pnetif);
  int32_t (* if_powersave_disable)(net_if_handle_t *pnetif);
  void    *netif;
  void    *context;
#ifndef NET_BYPASS_NET_SOCKET
  /* Socket BSD Like APIs */
  int32_t (* psocket)(int32_t domain, int32_t type, int32_t protocol);
  int32_t (* pbind)(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen);
  int32_t (* plisten)(int32_t sock, int32_t backlog);
  int32_t (* paccept)(int32_t sock, net_sockaddr_t *addr, uint32_t *addrlen);
  int32_t (* pconnect)(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen);
  int32_t (* psend)(int32_t sock, uint8_t *buf, int32_t len, int32_t flags);
  int32_t (* precv)(int32_t sock, uint8_t *buf, int32_t len, int32_t flags);
  int32_t (* psendto)(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *to, uint32_t tolen);
  int32_t (* precvfrom)(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *from, uint32_t *flen);
  int32_t (* psetsockopt)(int32_t sock, int32_t level, int32_t optname, const void *optvalue, uint32_t optlen);
  int32_t (* pgetsockopt)(int32_t sock, int32_t level, int32_t optname, void *optvalue, uint32_t *optlen);
  int32_t (* pgetsockname)(int32_t sock, net_sockaddr_t *name, uint32_t *namelen);
  int32_t (* pgetpeername)(int32_t sock, net_sockaddr_t *name, uint32_t *namelen);
  int32_t (* pclose)(int32_t sock, bool Clone);
  int32_t (* pshutdown)(int32_t sock, int32_t mode);
#endif /* NET_BYPASS_NET_SOCKET */
  /* Service */
  int32_t (* pgethostbyname)(net_if_handle_t *, net_sockaddr_t *addr, char_t *name);
  int32_t (* pping)(net_if_handle_t *, net_sockaddr_t *addr, int32_t count, int32_t delay, int32_t reponse[]);
  /* class extension */
  struct
  {
    net_if_wifi_class_extension_t         *wifi;
    net_if_ethernet_class_extension_t     *ethernet;
    net_if_cellular_class_extension_t     *cellular;
    net_if_custom_class_extension_t       *custom;
  } extension;
};





net_if_handle_t *net_if_find(net_sockaddr_t  *addr);
net_if_handle_t *netif_check(net_if_handle_t *pnetif);


bool    net_access_control(net_if_handle_t *pnetif, net_access_t access, int32_t *l);

typedef   void (*sock_notify_func)(int32_t, int32_t, const uint8_t *, uint32_t);

typedef struct net_tls_data net_tls_data_t;
typedef int32_t net_ulsock_t;

typedef enum { SOCKET_NOT_ALIVE = 0, SOCKET_ALLOCATED, SOCKET_CONNECTED  } socket_state_t;


typedef struct net_socket_s
{
  net_if_handle_t  *pnetif;
  net_ulsock_t     ulsocket;
  socket_state_t   status;
  int32_t          domain;
  int32_t          type;
  int32_t          protocol;
  bool             cloneserver;
  bool             connected;
#ifdef NET_MBEDTLS_HOST_SUPPORT
  bool             is_secure;
  net_tls_data_t   *tlsData;
  bool             tls_started;
#endif /* NET_MBEDTLS_HOST_SUPPORT */
  int32_t          read_timeout;
  int32_t          write_timeout;
  bool             blocking;
  int32_t         idx;
} net_socket_t;



#ifdef  NET_MBEDTLS_HOST_SUPPORT
/*cstat -MISRAC* -DEFINE-* -CERT-EXP19*  */
#include "net_mbedtls.h"
/*cstat +MISRAC* +DEFINE-* +CERT-EXP19*  */
#endif /* NET_MBEDTLS_HOST_SUPPORT */

#ifdef __cplusplus
}
#endif


#endif /* NET_CORE_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
