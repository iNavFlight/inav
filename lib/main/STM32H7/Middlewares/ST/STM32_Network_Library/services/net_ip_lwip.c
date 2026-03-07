/**
  ******************************************************************************
  * @file    net_ip_lwip.c
  * @author  MCD Application Team
  * @brief   lwIP network interface functions
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
#include "net_ip_lwip.h"
#include "net_buffers.h"

#define YES (u8_t)1
#define NO (u8_t)0

static void link_socket_to_lwip(net_if_drv_t *drv);
#ifndef NET_BYPASS_NET_SOCKET
static int32_t net_lwip_socket(int32_t domain, int32_t type, int32_t protocol);
static int32_t net_lwip_bind(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen);
static int32_t net_lwip_listen(int32_t sock, int32_t backlog);
static int32_t net_lwip_accept(int32_t sock, net_sockaddr_t *addr, uint32_t *addrlen);
static int32_t net_lwip_connect(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen);
static int32_t net_lwip_send(int32_t sock, uint8_t *buf, int32_t len, int32_t flags);
static int32_t net_lwip_recv(int32_t sock, uint8_t *buf, int32_t len, int32_t flags);
static int32_t net_lwip_sendto(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *to,
                               uint32_t tolen);
static int32_t net_lwip_recvfrom(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *from,
                                 uint32_t *fromlen);
static int32_t net_lwip_setsockopt(int32_t sock, int32_t level, int32_t optname, const void *optvalue, uint32_t optlen);
static int32_t net_lwip_getsockopt(int32_t sock, int32_t level, int32_t optname,  void *optvalue, uint32_t *optlen);
static int32_t net_lwip_getsockname(int32_t sock, net_sockaddr_t *name, uint32_t *namelen);
static int32_t net_lwip_getpeername(int32_t sock, net_sockaddr_t *name, uint32_t *namelen);
static int32_t net_lwip_close(int32_t sock, bool clone);
static int32_t net_lwip_shutdown(int32_t sock, int32_t mode);
#endif /* NET_BYPASS_NET_SOCKET */

static int32_t net_lwip_gethostbyname(net_if_handle_t *pnetif, net_sockaddr_t *addr, char_t *name);

#define NETIF_IS_LINK_UP(netif) (((netif)->flags & NETIF_FLAG_LINK_UP)!=0U)
/* Manage init of LWIP libraryu as a singleton */
void net_ip_init(void)
{
  static  bool tcpip_init_done = 0;
  if (false == tcpip_init_done)
  {
    tcpip_init(NULL, NULL);
    tcpip_init_done = true;
  }
}


/*    Add of a new network interface       */
int32_t net_ip_add_if(net_if_handle_t *pnetif, err_t (*if_init)(struct netif *netif), uint32_t flag)
{
  int32_t      ret;
  struct netif *netif;
  /*cstat -MISRAC2012-Rule-11.5 Malloc*/
  netif = (struct netif *)(void *) NET_MALLOC(sizeof(struct netif));
  /*cstat +MISRAC2012-Rule-11.5 */
  if (NULL != netif)
  {
    pnetif->netif = netif;
    (void) memset(netif, 0x00, sizeof(struct netif));
    netif = netif_add(netif, NULL, NULL, NULL, pnetif, if_init, tcpip_input);
    if ((flag & NET_IP_FLAG_DEFAULT_INTERFACE) != 0U)
    {
      netif_set_default(netif);
    }

    /* Get  MAC hardware address from netif */
    (void) memcpy(pnetif->macaddr.mac, netif->hwaddr, sizeof(netif->hwaddr));

    /* link current network interface to LWIP library */
    link_socket_to_lwip(pnetif->pdrv);
#if 0 /*FIXME*/
#if LWIP_IPV6
    /* Set the IPv6 linklocal address using our MAC */
    NET_DBG_PRINT("Setting IPv6 link-local address\n");

    netif_create_ip6_linklocal_address(netif, 1);
    IP_HANDLE(interface).ip6_autoconfig_enabled = 1;
#endif  /* LWIP_IPV6 */
#endif /* FIXME */
    ret = NET_OK;
  }
  else
  {
    ret = NET_ERROR_GENERIC;
  }
  return ret;
}

static ip4_addr_t const          *ip4_addr(net_ip_addr_t    *ipaddr)
{
  /*cstat -MISRAC2012-Rule-11.3 Const casting*/
  return (ip4_addr_t const *) ipaddr;
  /*cstat +MISRAC2012-Rule-11.3*/

}


/*          CONNECTION      */
int32_t net_ip_connect(net_if_handle_t *pnetif)
{
  int32_t ret;
  struct netif     *netif = (struct netif *)pnetif->netif;

  if (NULL != netif)
  {
    if (!pnetif->dhcp_mode && (NET_IP_ADDR_ISANY_VAL(pnetif->static_ipaddr)))
    {
      ret = NET_ERROR_PARAMETER;
    }
    else
    {
      netif_set_up(netif);
      if (pnetif->dhcp_mode)
      {
        (void) dhcp_start(netif);
      }
      else
      {
        netif_set_addr(netif, ip4_addr(&pnetif->static_ipaddr), ip4_addr(&pnetif->static_netmask),
                       ip4_addr(&pnetif->static_gateway));
        dhcp_inform(netif);
      }
      ret = NET_OK;
    }
  }
  else
  {
    ret = NET_ERROR_PARAMETER;
  }
  return ret;
}

int32_t net_ip_disconnect(net_if_handle_t *pnetif)
{
  struct netif *netif = pnetif->netif;
  /*cstat -MISRAC2012-Rule-11.5 LWIP function*/
  if (netif_dhcp_data(netif) != NULL)
    /*cstat +MISRAC2012-Rule-11.5 */
  {
    (void) dhcp_release(netif);
    dhcp_stop(netif);
    dhcp_cleanup(netif);
  }
  else
  {
    netif_set_addr(netif, NULL, NULL, NULL);
    pnetif->dhcp_inform_flag = false;
    dhcp_inform(netif);
  }
  return (int32_t) ERR_OK;
}

int32_t net_ip_remove_if(net_if_handle_t *pnetif, err_t (*if_deinit)(struct netif *netif))
{
  if (pnetif != NULL)
  {
    struct netif   *netif = pnetif->netif;
    if (netif != NULL)
    {
      netif_set_down(netif);
      netif_set_link_down(netif);
      netif_remove(netif);
      if (NULL != if_deinit)
      {
        (*if_deinit)(netif);
      }
      NET_FREE(netif);
      pnetif->netif = NULL;
    }
  }
  return NET_OK;
}

void net_ip_status_cb(struct netif *netif)
{
  net_ip_addr_t   ipaddr_zero;
  /*cstat -MISRAC2012-Rule-11.5 casting state pointer by defintion from LWIP */
  net_if_handle_t *pnetif = (net_if_handle_t *) netif->state;
  /*cstat +MISRAC2012-Rule-11.5 */

  NET_ZERO(ipaddr_zero);

  if (pnetif->dhcp_enabled)
  {
    /* lost connection */
    if (NET_DIFF(netif->ip_addr, ipaddr_zero) && (!NETIF_IS_LINK_UP(netif)) && pnetif->dhcp_release_on_link_lost)
    {
      NET_DBG_PRINT("Callback lost connection so release connection\n");
      (void) dhcp_release(netif);
      (void) dhcp_start(netif);
    }

    /* up connection , so need to inform other at first time */
    if (pnetif->dhcp_inform_flag && NETIF_IS_LINK_UP(netif))
    {
      NET_DBG_PRINT("Callback get connection\n");
      dhcp_inform(netif);
      pnetif->dhcp_inform_flag = false;
    }
  }

  if (!NET_IP_ADDR_CMP(&pnetif->ipaddr, &netif->ip_addr))
  {
    NET_IP_ADDR_COPY(pnetif->ipaddr, netif->ip_addr);
    NET_IP_ADDR_COPY(pnetif->netmask, netif->netmask);
    NET_IP_ADDR_COPY(pnetif->gateway, netif->gw);
    /* FIXME for IPV6 support */
    if (!NET_IP_ADDR_ISANY_VAL(pnetif->ipaddr))
    {
      (void) net_state_manage_event(pnetif, NET_EVENT_IPADDR);
    }
  }

  if (NETIF_IS_LINK_UP(netif))
  {
    (void) net_state_manage_event(pnetif, NET_EVENT_LINK_UP);
  }
  else
  {
    (void) net_state_manage_event(pnetif, NET_EVENT_LINK_DOWN);
  }
}




/*       UTILITIES          */

void link_socket_to_lwip(net_if_drv_t *drv)
{
#ifndef NET_BYPASS_NET_SOCKET
  drv->psocket = net_lwip_socket;
  drv->pbind = net_lwip_bind;
  drv->plisten = net_lwip_listen;
  drv->paccept = net_lwip_accept;
  drv->pconnect = net_lwip_connect;
  drv->psend = net_lwip_send;
  drv->precv = net_lwip_recv;
  drv->psendto = net_lwip_sendto;
  drv->precvfrom = net_lwip_recvfrom;
  drv->psetsockopt = net_lwip_setsockopt;
  drv->pgetsockopt = net_lwip_getsockopt;
  drv->pgetsockname = net_lwip_getsockname;
  drv->pgetpeername = net_lwip_getpeername;
  drv->pclose = net_lwip_close;
  drv->pshutdown = net_lwip_shutdown;
#endif /* NET_BYPASS_NET_SOCKET */
  /* Service */
  drv->pgethostbyname = net_lwip_gethostbyname;
}


int32_t returncode_lwip2net(int32_t     ret_in)
{
  int32_t     ret = ret_in;
  if (ret_in == -1)
  {
    if (errno == EWOULDBLOCK)
    {
      ret = 0;
    }
  }
  else if (ret_in == 0)
  {
    /* connection close */
    ret = NET_ERROR_DISCONNECTED;
  }
  else
  {
    /* do not catch ret valie otherwise */
    ret = ret_in;
  }
  return ret;
}

#ifndef NET_BYPASS_NET_SOCKET
/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_socket(int32_t domain, int32_t type, int32_t protocol)
{
  int32_t sock;
  sock = (int32_t) lwip_socket(domain, type, protocol);
  return (sock);
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */

static struct sockaddr   *getsockaddr(const net_sockaddr_t *addr)
{
  /*cstat -MISRAC2012-Rule-11.8 const*/
  return (struct sockaddr *) addr;
  /*cstat +MISRAC2012-Rule-11.8 const*/

}

static int32_t net_lwip_bind(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen)
{
  int32_t   ret = lwip_bind(sock, getsockaddr(addr),  addrlen);
  return ret;
}


/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_listen(int32_t sock, int32_t backlog)
{
  int32_t ret;
  ret = lwip_listen(sock, backlog);
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_accept(int32_t sock, net_sockaddr_t *addr, uint32_t *addrlen)
{
  int32_t ret;
  ret = lwip_accept(sock, getsockaddr(addr), addrlen);
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_connect(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen)
{
  int32_t ret;
  ret = lwip_connect(sock, getsockaddr(addr), addrlen);
  return ret;
}


/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_send(int32_t sock, uint8_t *buf, int32_t len, int32_t flags)
{
  int32_t ret;

  ret = lwip_send(sock, buf, (uint32_t) len, flags);
  return returncode_lwip2net(ret);
}



/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_recv(int32_t sock, uint8_t *buf, int32_t len, int32_t flags)
{
  int32_t ret;
  ret = lwip_recv(sock, buf, (uint32_t) len, flags);
  return returncode_lwip2net(ret);
}


/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_sendto(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *to,
                               uint32_t tolen)
{
  int32_t ret = lwip_sendto(sock, buf, (uint32_t) len, flags, getsockaddr(to),  tolen);
  return returncode_lwip2net(ret);
}


/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_recvfrom(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *from,
                                 uint32_t *fromlen)
{
  int32_t ret = lwip_recvfrom(sock, buf, (uint32_t) len, flags, getsockaddr(from), fromlen);
  return returncode_lwip2net(ret);
}



/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_setsockopt(int32_t sock, int32_t level, int32_t optname, const void *optvalue, uint32_t optlen)
{
  int32_t   ret = lwip_setsockopt(sock, level, optname, optvalue, optlen);
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */

static int32_t net_lwip_getsockopt(int32_t sock, int32_t level, int32_t optname, void *optvalue, uint32_t *optlen)
{
  int32_t ret = lwip_getsockopt(sock, level, optname, optvalue, optlen);
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_getsockname(int32_t sock, net_sockaddr_t *name, uint32_t *namelen)
{
  int32_t ret = lwip_getsockname(sock, getsockaddr(name),  namelen);
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_getpeername(int32_t sock, net_sockaddr_t *name, uint32_t *namelen)
{
  int32_t ret = lwip_getpeername(sock, getsockaddr(name), namelen);
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_close(int32_t sock, bool clone)
{
  (void) clone;
  int32_t   ret = lwip_close(sock);
  return ret;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_shutdown(int32_t sock, int32_t mode)
{
  int32_t   ret = lwip_shutdown(sock, mode);
  return ret;
}

#endif /* NET_BYPASS_NET_SOCKET */
/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_lwip_gethostbyname(net_if_handle_t *pnetif, net_sockaddr_t *addr, char_t *name)
{
  int32_t ret = NET_ERROR_DNS_FAILURE;
  struct addrinfo *hostinfo;
  struct addrinfo hints;

  (void) pnetif;

  if (addr->sa_len < sizeof(net_sockaddr_in_t))
  {
    ret = NET_ERROR_PARAMETER;
  }
  else
  {
    (void) memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    if (0 == lwip_getaddrinfo((char_t *)name, NULL, &hints, &hostinfo))
    {
      if (hostinfo->ai_family == AF_INET)
      {
        uint8_t len = addr->sa_len;
        net_sockaddr_in_t *saddr = (net_sockaddr_in_t *) addr;

        (void) memset(saddr, 0, len);
        saddr->sin_len = len;
        saddr->sin_family = NET_AF_INET;
        (void) memcpy(&saddr->sin_addr, &((net_sockaddr_in_t *)(hostinfo->ai_addr))->sin_addr, 4);
        ret = NET_OK;
      }
      lwip_freeaddrinfo(hostinfo);
    }
  }
  return ret;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
