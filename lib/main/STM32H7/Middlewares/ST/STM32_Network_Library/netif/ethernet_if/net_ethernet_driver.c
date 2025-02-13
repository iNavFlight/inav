/**
  ******************************************************************************
  * @file    net_ethernet_driver.c
  * @author  MCD Application Team
  * @brief   Ethernet specific BSD-like socket wrapper
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
#include "net_ip_ethernet.h"
#include "net_ip_lwip.h"

/* global constructor of the ethernet network interface */
int32_t ethernet_net_driver(net_if_handle_t *pnetif);


static int32_t net_ethernet_if_init(net_if_handle_t *pnetif);
static int32_t net_ethernet_if_deinit(net_if_handle_t *pnetif);
static int32_t net_ethernet_if_start(net_if_handle_t *pnetif);
static int32_t net_ethernet_if_stop(net_if_handle_t *pnetif);
static int32_t net_ethernet_if_connect(net_if_handle_t *pnetif);
static int32_t net_ethernet_if_disconnect(net_if_handle_t *pnetif);

err_t ethernetif_init(struct netif *netif);
err_t ethernetif_deinit(struct netif *netif);

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */

int32_t ethernet_net_driver(net_if_handle_t *pnetif)
{
  int32_t ret;
  net_ip_init();
  ret = net_ethernet_if_init(pnetif);
  return ret;
}

static int32_t  net_ethernet_if_init(net_if_handle_t *pnetif)
{
  int32_t ret;
  /*cstat -MISRAC2012-Rule-11.5 malloc cast*/
  net_if_drv_t *pdrv = NET_MALLOC(sizeof(net_if_drv_t));
  /*cstat +MISRAC2012-Rule-11.5 */

  if (pdrv != NULL)
  {
    pdrv->if_class = NET_INTERFACE_CLASS_ETHERNET;
    pdrv->if_init = net_ethernet_if_init;
    pdrv->if_deinit = net_ethernet_if_deinit;
    pdrv->if_start = net_ethernet_if_start;
    pdrv->if_stop = net_ethernet_if_stop;
    pdrv->if_connect = net_ethernet_if_connect;
    pdrv->if_disconnect = net_ethernet_if_disconnect;
    pdrv->pping = icmp_ping;
    pnetif->pdrv = pdrv;
    (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_INITIALIZED);
    ret = NET_OK;
  }
  else
  {
    NET_DBG_ERROR("can't allocate memory for es_wifi_driver class\n");
    ret = NET_ERROR_NO_MEMORY;
  }
  return ret;
}

int32_t net_ethernet_if_start(net_if_handle_t *pnetif)
{
  int32_t ret;

  ret = net_ip_add_if(pnetif, ethernetif_init, NET_ETHERNET_FLAG_DEFAULT_IF);
  if (ret == NET_OK)
  {
    (void) strncpy(pnetif->DeviceName, "Ethernet IF", sizeof(pnetif->DeviceName));
    (void) strncpy(pnetif->DeviceID, "Unknown", sizeof(pnetif->DeviceID));
    (void) strncpy(pnetif->DeviceVer, "Unknown", sizeof(pnetif->DeviceVer));
    /* set call back , here to not loose first linkup when if_init is performed */
    netif_set_down(pnetif->netif);
    netif_set_link_down(pnetif->netif);

    netif_set_status_callback(pnetif->netif, net_ip_status_cb);
    netif_set_link_callback(pnetif->netif, net_ip_status_cb);
    netif_set_link_up(pnetif->netif);
    netif_set_up(pnetif->netif);

    (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_READY);
  }
  return ret;
}

static int32_t net_ethernet_if_connect(net_if_handle_t *pnetif)
{
  int32_t       ret;
  ret =  net_ip_connect(pnetif);
  return ret;
}

static int32_t net_ethernet_if_disconnect(net_if_handle_t *pnetif)
{
  int32_t       ret;
  (void) net_ip_disconnect(pnetif);
  ret = net_state_manage_event(pnetif, NET_EVENT_INTERFACE_READY);
  return ret;
}

static int32_t net_ethernet_if_stop(net_if_handle_t *pnetif)
{
  int32_t ret;
  (void) net_ip_remove_if(pnetif, ethernetif_deinit);
  ret = net_state_manage_event(pnetif, NET_EVENT_INTERFACE_INITIALIZED);
  return ret;
}

static int32_t net_ethernet_if_deinit(net_if_handle_t *pnetif)
{
  int32_t ret = NET_OK;

  NET_FREE(pnetif->pdrv);
  pnetif->pdrv = NULL;
  return ret;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/





