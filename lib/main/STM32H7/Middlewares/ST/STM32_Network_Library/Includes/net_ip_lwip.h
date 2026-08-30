/**
  ******************************************************************************
  * @file    net_ip_lwip.h
  * @author  MCD Application Team
  * @brief   Header for the network IP functions.
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

#ifndef NET_IP_LWIP_H
#define NET_IP_LWIP_H

#include "net_connect.h"
#include "net_internals.h"

/*cstat -MISRAC* -DEFINE-* -CERT-EXP19*  */
#include "lwip/netdb.h"
#include "lwip/dhcp.h"
#include "lwip/tcpip.h"
#include "lwip/etharp.h"
/*cstat +MISRAC* +DEFINE-* -CERT-EXP19*  */


#define NET_IP_IF_TIMEOUT              1000
#define NET_IP_INPUT_QUEUE_TIMEOUT     1000
#define NET_IP_INPUT_QUEUE_SIZE        128
#define NET_IP_THREAD_SIZE             1024

#define NET_IP_FLAG_DEFAULT_INTERFACE        (1U<<0)
#define NET_IP_FLAG_TCPIP_STARTED_EXTERNALLY (1U<<1)



void net_ip_init(void);

int32_t net_ip_add_if(net_if_handle_t *pnetif, err_t (*if_init)(struct netif *netif), uint32_t flag);
int32_t net_ip_remove_if(net_if_handle_t *pnetif, err_t (*if_deinit)(struct netif *netif));
int32_t net_ip_connect(net_if_handle_t *pnetif);
int32_t net_ip_disconnect(net_if_handle_t *pnetif);
void net_ip_status_cb(struct netif *netif);
void net_ip_link_status(struct netif *netif, uint8_t status);
int32_t returncode_lwip2net(int32_t     ret);



#endif /* NET_IP_LWIP_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
