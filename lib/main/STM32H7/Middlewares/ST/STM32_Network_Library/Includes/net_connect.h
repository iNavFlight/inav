/**
  ******************************************************************************
  * @file    net_connect.h
  * @author  MCD Application Team
  * @brief   Provides the network interface APIs.
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

#ifndef NET_CONNECT_H
#define NET_CONNECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include "net_types.h"
#include "net_conf.h"
#include "net_mem.h"
#include "net_perf.h"
#include "net_address.h"
#include "net_errors.h"
#include "net_wifi.h"
#include "net_cellular.h"

/* flags */

#define NET_ETHERNET_FLAG_DEFAULT_IF  1



/* Socket family */
#define NET_AF_INET           2
#define NET_AF_UNSPEC         0
#define NET_AF_INET6          10
#define NET_IPADDR_ANY        ((u32_t)0x00000000UL)

/* Socket types */
#define NET_SOCK_STREAM         1
#define NET_SOCK_DGRAM          2
#define NET_SOCK_RAW            3

/* Socket protocol */
#define NET_IPPROTO_TCP         6
#define NET_IPPROTO_ICMP        1
#define NET_IPPROTO_UDP        17
#define NET_IPPROTO_TCP_TLS    36


#define NET_SHUTDOWN_R          0
#define NET_SHUTDOWN_W          1
#define NET_SHUTDOWN_RW         2

#define NET_SOL_SOCKET                  0xfff
/** @defgroup Socket
  * @{
  */


typedef enum
{

  NET_SO_RCVTIMEO           =  0x1006,/**< to set received timeout in ms, option type is an integer value */
  NET_SO_SNDTIMEO           =  0x1005,/**< to set send timeout in ms, option type is an integer value */
  NET_SO_BINDTODEVICE       =       3,/**< to map socket to a specific network interface, no implemented so far , normal bind should make the job */
  NET_SO_BLOCKING           =       4,/**< to set blocking or none blocking mode, option type is an boolean, true means blocking (default value), false non blocking */
  NET_SO_SECURE             =       5,/**< to set a socket a a secure socket, no option argument, setsock option must be done before any connection */
  NET_SO_TLS_CA_CERT        =       7,/**< to pass root ca to secure socket, option type is a pointer to a string , certificat is pem format string */
  NET_SO_TLS_CA_CRL         =       8,/**< to pass revocation certificat list, this is not supported */
  NET_SO_TLS_DEV_KEY        =       9,/**< to pass device key to secure socket, option type is a pointer to a string , key is pem format string */
  NET_SO_TLS_DEV_CERT       =      10,/**< to pass device certificat to secure socket, option type is a pointer to a string , certificat is pem format string */
  NET_SO_TLS_SERVER_VERIFICATION = 11,/**< to define verification mode for secure socket,option type is a boolean, True to check server name */
  NET_SO_TLS_SERVER_NAME    =      12,/**< to define server name to check again,option type is a point to a null terminated string */
  NET_SO_TLS_PASSWORD       =      13,/**< to define passwd (if any) used to encrypt the device key, option type is pointer to a null terminated string  */
  NET_SO_TLS_CERT_PROF      =      14,/**< to set the X509 security profile , option type is pointer to mbedtls_x509_crt_profile structure */
}
net_socketoption_t;


/** @defgroup Socket
  * @}
  */
#define NET_MSG_DONTWAIT      0x08U    /* Nonblocking i/o for this operation only */

typedef struct pbuf net_buf_t;


/**
  *  TOPPP transition are requested by application. "ING" state are transitioning state, meaning that application has required a transition and the transition is ongoing. "ED" states are stable state.
  */


typedef enum
{
  NET_EVENT_STATE_CHANGE,
  NET_EVENT,
  NET_EVENT_WIFI,
} net_evt_t;


/** @defgroup State
  * @{
  *  State transition are requested by application. "ING" state are transitioning state, meaning that application has required a transition and the transition is ongoing. "ED" states are stable state.

  */

typedef enum enum_state
{

  NET_STATE_DEINITIALIZED = 0,
  NET_STATE_INITIALIZED, /**< basic memory allocation for driver and network interface have been performed  */

  NET_STATE_STARTING, /**< Network interface interface is starting, application waits for event from network interface to signal transition is performed to NET_STATE_STARTED */
  NET_STATE_READY,    /**< Network interface interface is started, MAC address can be retrieved */

  NET_STATE_CONNECTING,/**< Network interface interface is connecting */
  NET_STATE_CONNECTED,/**< Network interface interface is connected, IP address can be retrieved , socket operation can be performed*/

  NET_STATE_STOPPING, /**< Network interface interface is stopping */

  NET_STATE_DISCONNECTING, /**< Network interface interface is disconnecting */
  NET_STATE_CONNECTION_LOST,     /**< Network interface connection is lost , this can be a transient state , it can return to connected state without application specific action*/
} net_state_t;


/** @defgroup State
  * @}
  */

/** Network state events. */
typedef enum
{
  NET_EVENT_CMD_INIT = 0,
  NET_EVENT_CMD_START,
  NET_EVENT_CMD_CONNECT,
  NET_EVENT_CMD_DISCONNECT,
  NET_EVENT_CMD_STOP,
  NET_EVENT_CMD_DEINIT,
  NET_EVENT_INTERFACE_INITIALIZED,
  NET_EVENT_INTERFACE_READY,
  NET_EVENT_LINK_UP,
  NET_EVENT_LINK_DOWN,
  NET_EVENT_IPADDR,
} net_state_event_t;




/** Network events. */
typedef enum
{
  NET_EVENT_POWERSAVE_ENABLED = 0,
} net_event_t;



typedef struct net_if_drv_s             net_if_drv_t;
typedef struct net_ip_if_s              net_ip_if_t;

typedef void(*  net_if_notify_func)(void *context, uint32_t event_class, uint32_t event_id, void  *event_data);

typedef struct
{
  net_if_notify_func callback;
  void *context;
} net_event_handler_t;

struct net_if_handle_s
{
  struct net_if_handle_s *next;
  net_ip_addr_t       ipaddr;
  net_ip_addr_t       gateway;
  net_ip_addr_t       netmask;
  net_ip_addr_t       static_ipaddr;
  net_ip_addr_t       static_gateway;
  net_ip_addr_t       static_netmask;
  net_ip_addr_t       static_dnserver;
  bool_t           dhcp_mode;
  bool_t           dhcp_inform_flag;
  bool_t           dhcp_enabled;
  bool_t           dhcp_release_on_link_lost;
  char_t  DeviceName[NET_DEVICE_NAME_LEN];
  char_t  DeviceID  [NET_DEVICE_ID_LEN];
  char_t  DeviceVer [NET_DEVICE_VER_LEN];
  macaddr_t  macaddr;
  net_state_t    state;
  net_if_drv_t   *pdrv;
  struct netif   *netif;
  const net_event_handler_t *event_handler;
} ;





typedef int32_t(* net_if_driver_init_func)(net_if_handle_t *pnetif);



/* network state control functions */
int32_t net_if_init(net_if_handle_t *pnetif, net_if_driver_init_func driver_init,
                    const net_event_handler_t *event_handler);
int32_t net_if_deinit(net_if_handle_t *pnetif);

int32_t net_if_start(net_if_handle_t *pnetif);
int32_t net_if_stop(net_if_handle_t *pnetif);

/* network io data receive process, called in main loop */
int32_t net_if_yield(net_if_handle_t *pnetif, uint32_t timeout);

int32_t net_if_connect(net_if_handle_t *pnetif);
int32_t net_if_disconnect(net_if_handle_t *pnetif);


int32_t net_if_getState(net_if_handle_t *pnetif, net_state_t *state);
int32_t net_if_wait_state(net_if_handle_t *pnetif, net_state_t state, uint32_t timeout);

/* network event management */
void net_if_notify(net_if_handle_t *pnetif, net_evt_t event_class, uint32_t event_if, void  *event_data);

/* network parameter and status functions */
int32_t net_if_set_dhcp_mode(net_if_handle_t *pnetif, bool_t mode);
int32_t net_if_set_ipaddr(net_if_handle_t *pnetif, net_ip_addr_t ipaddr, net_ip_addr_t gateway, net_ip_addr_t netmask);
int32_t net_if_get_mac_address(net_if_handle_t *pnetif, macaddr_t *mac);
int32_t net_if_get_ip_address(net_if_handle_t *pnetif, net_ip_addr_t *ip);
int32_t net_if_gethostbyname(net_if_handle_t *pnetif, net_sockaddr_t *addr, char_t *name);
int32_t net_if_ping(net_if_handle_t *pnetif, net_sockaddr_t *addr, int32_t count, int32_t delay, int32_t reponse[]);

/* networtk interface power management */
int32_t net_if_powersave_enable(net_if_handle_t *pnetif);
int32_t net_if_powersave_disable(net_if_handle_t *pnetif);

#if 0
int32_t net_if_sleep(net_if_handle_t *pnetif);
int32_t net_if_wakeup(net_if_handle_t *pnetif);
#endif /* 0 */



/* network socket API */
#ifdef  NET_BYPASS_NET_SOCKET

/*cstat -MISRAC* -DEFINE-* -CERT-EXP19*  */
#include "lwip/netdb.h"
#include "lwip/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/etharp.h"
/*cstat +MISRAC* +DEFINE-* -CERT-EXP19*  */

#define net_socket              lwip_socket
#define net_bind                lwip_bind
#define net_accept              lwip_accept
#define net_closesocket         lwip_close
#define net_shutdown            lwip_shutdown
#define net_setsockopt          lwip_setsockopt
#define net_getsockopt          lwip_getsockopt
#define net_connect             lwip_connect
#define net_listen              lwip_listen
#define net_send                lwip_send
#define net_recv                lwip_recv
#define net_sendto              lwip_sendto
#define net_recvfrom            lwip_recvfrom
#define net_getsockname         lwip_getsockname
#define net_getpeername         lwip_getpeeername

#else

int32_t net_socket(int32_t Domain, int32_t Type, int32_t Protocol);
int32_t net_bind(int32_t sock, net_sockaddr_t *addr, uint32_t addrlen);
int32_t net_accept(int32_t sock, net_sockaddr_t *addr, uint32_t *addrlen);
int32_t net_closesocket(int32_t sock);
int32_t net_shutdown(int32_t sock, int32_t mode);
int32_t net_setsockopt(int32_t sock, int32_t level, net_socketoption_t optname, const void *optvalue, uint32_t optlen);
int32_t net_getsockopt(int32_t sock, int32_t level, net_socketoption_t optname, void *optvalue, uint32_t *optlen);
int32_t net_connect(int32_t sock, net_sockaddr_t *addr, uint32_t addrlen);
int32_t net_listen(int32_t sock, int32_t backlog);
int32_t net_send(int32_t sock, uint8_t *buf, uint32_t len, int32_t flags);
int32_t net_recv(int32_t sock, uint8_t *buf, uint32_t len, int32_t flags);
int32_t net_sendto(int32_t sock, uint8_t *buf, uint32_t len, int32_t flags, net_sockaddr_t *to, uint32_t tolen);
int32_t net_recvfrom(int32_t sock, uint8_t *buf, uint32_t len, int32_t flags, net_sockaddr_t *from, uint32_t *fromlen);
int32_t net_getsockname(int32_t sock, net_sockaddr_t *name, uint32_t *namelen);
int32_t net_getpeername(int32_t sock, net_sockaddr_t *name, uint32_t *namelen);

#endif /* NET_BYPASS_NET_SOCKET */

extern  const int32_t net_tls_sizeof_suite_structure;
extern  const void    *net_tls_user_suite0;
extern  const void    *net_tls_user_suite1;
extern  const void    *net_tls_user_suite2;
extern  const void    *net_tls_user_suite3;
extern  const void    *net_tls_user_suite4;



#ifdef __cplusplus
}
#endif


#endif /* NET_CONNECT_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
