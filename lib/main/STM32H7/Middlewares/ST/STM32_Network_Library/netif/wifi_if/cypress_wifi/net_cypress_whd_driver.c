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

#include <string.h>
#include "net_connect.h"
#include "net_internals.h"
#include "net_ip_lwip.h"

/*cstat -MISRAC* -DEFINE-* -CERT-EXP19*  */
#include "whd.h"
#include "whd_debug.h"
#include "whd_resource_api.h"
#include "whd_wifi_api.h"
#include "whd_network_types.h"
#include "whd_types_int.h"
/*cstat +MISRAC* +DEFINE-* +CERT-EXP19*  */

#define NET_CYPRESS_MAX_INTERFACE       4

typedef struct net_cypress_key
{
  void   *key;
  void   *value;
} net_cypress_key_t;

static net_cypress_key_t                netif2ifp[NET_CYPRESS_MAX_INTERFACE];
static  uint32_t                        cypress_alive_interface_count = 0;
static whd_driver_t                     whd_driver;

/* create and boot WHD driver */
cy_rslt_t whd_boot(whd_driver_t *whd_driver);
cy_rslt_t whd_powerdown(whd_driver_t *whd_driver);


/* global constructor of the ethernet network interface */
int32_t cypress_whd_net_driver(net_if_handle_t *pnetif);



static err_t low_level_output(struct netif *netif, struct pbuf *p);
static  err_t low_level_init(struct netif *netif);
static int32_t net_cypress_whd_if_init(net_if_handle_t *pnetif);
static int32_t net_cypress_whd_if_deinit(net_if_handle_t *pnetif);
static int32_t net_cypress_whd_if_start(net_if_handle_t *pnetif);
static int32_t net_cypress_whd_if_stop(net_if_handle_t *pnetif);
static int32_t net_cypress_whd_if_connect(net_if_handle_t *pnetif);
static int32_t net_cypress_whd_if_disconnect(net_if_handle_t *pnetif);

static int32_t net_cypress_whd_scan(net_if_handle_t *pnetif, net_wifi_scan_mode_t mode, char *ssid);
static int32_t net_cypress_whd_scan_result(net_if_handle_t *pnetif, net_wifi_scan_results_t *results, uint8_t number);



/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */



int32_t cypress_whd_net_driver(net_if_handle_t *pnetif)
{
  int32_t ret;
  /* init lwip library here if not already done by another network interface */
  net_ip_init();

  ret = net_cypress_whd_if_init(pnetif);
  return ret;
}

static int32_t  net_cypress_whd_if_init(net_if_handle_t *pnetif)
{
  int32_t ret = NET_OK;

  net_if_drv_t *pdrv = NET_MALLOC(sizeof(net_if_drv_t));
  if (pdrv != NULL)
  {
    pdrv->if_class = NET_INTERFACE_CLASS_WIFI;
    pdrv->if_init = net_cypress_whd_if_init;
    pdrv->if_deinit = net_cypress_whd_if_deinit;
    pdrv->if_start = net_cypress_whd_if_start;
    pdrv->if_stop = net_cypress_whd_if_stop;
    pdrv->if_connect = net_cypress_whd_if_connect;
    pdrv->if_disconnect = net_cypress_whd_if_disconnect;
    pdrv->pping = icmp_ping;
    pnetif->pdrv = pdrv;
    pdrv->extension.wifi = NET_MALLOC(sizeof(net_if_wifi_class_extension_t));
    if (NULL == pdrv->extension.wifi)
    {
      NET_DBG_ERROR("can't allocate memory for wifi extension\n");
      NET_FREE(pdrv);
      ret = NET_ERROR_NO_MEMORY;
    }
    else
    {
      (void) memset(pdrv->extension.wifi, 0, sizeof(net_if_wifi_class_extension_t));
      pdrv->extension.wifi->scan = net_cypress_whd_scan;
      pdrv->extension.wifi->get_scan_results = net_cypress_whd_scan_result;

      if (cypress_alive_interface_count == 0)
      {
        /* Boot cypress module and start whd driver for very first interface */
        if (WHD_SUCCESS != whd_boot(&whd_driver))
        {
          NET_DBG_ERROR("can't perform intialization of whd driver and module\n");
          ret = NET_ERROR_MODULE_INITIALIZATION;
        }

        NET_DBG_PRINT("WHD init driver done\n");
        (void) memset(netif2ifp, 0, sizeof(netif2ifp));

        if (NET_OK == ret)
        {

          if (WHD_SUCCESS != whd_wifi_on(whd_driver, (whd_interface_t *) &pdrv->extension.wifi->ifp))
          {
            NET_DBG_PRINT("Failed when creating WIFI default interface\n");
            NET_FREE(pdrv->extension.wifi);
            NET_FREE(pdrv);
            ret = NET_ERROR_MODULE_INITIALIZATION;
          }
          else
          {
            NET_DBG_PRINT("WHD init interface done\n");
            cypress_alive_interface_count++;
            ret = NET_OK;
          }
        }
      }
      else
      {
        whd_mac_t mac_addr = {0xA0, 0xC9, 0xA0, 0X3D, 0x43, 0x41};
        if (WHD_SUCCESS != whd_add_secondary_interface(whd_driver, &mac_addr, (whd_interface_t *) &pdrv->extension.wifi->ifp))
        {
          NET_DBG_PRINT("Failed when creating WIFI default interface\n");
          NET_FREE(pdrv->extension.wifi);
          NET_FREE(pdrv);
          ret = NET_ERROR_MODULE_INITIALIZATION;
        }
        else
        {
          NET_DBG_PRINT("WHD init interface done\n");
          cypress_alive_interface_count++;
          ret = NET_OK;
        }
      }
    }
  }
  else
  {
    NET_DBG_ERROR("can't allocate memory for WHD driver class\n");
    ret = NET_ERROR_NO_MEMORY;
  }
  return ret;
}


static void convert_credential(const net_wifi_credentials_t *credentials, whd_security_t *privacy, whd_ssid_t *myssid)
{
  *privacy = (whd_security_t) credentials->security_mode;
  strcpy((char *) myssid->value, (char *) credentials->ssid);
  myssid->length = strlen((char *)myssid->value);
}

static int32_t net_cypress_whd_if_start_sta(net_if_handle_t *pnetif)
{
  int32_t        ret = 0;
  whd_security_t privacy;
  whd_ssid_t     myssid;
  const net_wifi_credentials_t *credentials =  pnetif->pdrv->extension.wifi->credentials;

  convert_credential(credentials, &privacy, &myssid);

  NET_DBG_PRINT("Joinning ... %s\n", myssid.value);

  ret = whd_wifi_join((whd_interface_t)pnetif->pdrv->extension.wifi->ifp, (whd_ssid_t const *) &myssid, privacy,
                      (uint8_t const *) credentials->psk, strlen(credentials->psk));
  if (ret != 0)
  {
    NET_DBG_ERROR("can't join %s\n", myssid.value);
    ret = NET_ERROR_MODULE_INITIALIZATION;
  }
  else
  {
    NET_DBG_PRINT("Joined Access point %s\n", myssid.value);
    ret = net_ip_add_if(pnetif, low_level_init, NET_ETHERNET_FLAG_DEFAULT_IF);
    if (ret == NET_OK)
    {
      (void) strncpy(pnetif->DeviceName, "Wlan WHD murata 1LD", sizeof(pnetif->DeviceName));
      (void) strncpy(pnetif->DeviceID, "Unknown", sizeof(pnetif->DeviceID));
      (void) strncpy(pnetif->DeviceVer, "Unknown", sizeof(pnetif->DeviceVer));
      (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_READY);
      netif_set_link_up(pnetif->netif);
    }
    else
    {
      NET_DBG_ERROR("can't add interface (netif)\n");
    }
  }
  return ret;
}

static int32_t net_cypress_whd_if_start_ap(net_if_handle_t *pnetif)
{
  int32_t        ret = 0;
  whd_security_t privacy;
  whd_ssid_t     myssid;
  const net_wifi_credentials_t *credentials =  pnetif->pdrv->extension.wifi->credentials;

  NET_DBG_PRINT("Init Access Point ... %s\n", myssid.value);
  convert_credential(credentials, &privacy, &myssid);

  ret = whd_wifi_init_ap((whd_interface_t)pnetif->pdrv->extension.wifi->ifp, &myssid, privacy,
                         (uint8_t const *) credentials->psk, strlen(credentials->psk),
                         pnetif->pdrv->extension.wifi->access_channel);
  if (ret != 0)
  {
    NET_DBG_ERROR("can't init access point %s\n", myssid.value);
    ret = NET_ERROR_MODULE_INITIALIZATION;
  }
  else
  {
    ret = whd_wifi_start_ap((whd_interface_t)pnetif->pdrv->extension.wifi->ifp);
    if (ret != 0)
    {
      NET_DBG_ERROR("can't start access point %s\n", myssid.value);
      ret = NET_ERROR_MODULE_INITIALIZATION;
    }
    else
    {
      NET_DBG_PRINT("Start Access point %s\n", myssid.value);
      ret = net_ip_add_if(pnetif, low_level_init, NET_ETHERNET_FLAG_DEFAULT_IF);
      if (ret == NET_OK)
      {
        (void) strncpy(pnetif->DeviceName, "Wlan WHD murata 1LD", sizeof(pnetif->DeviceName));
        (void) strncpy(pnetif->DeviceID, "Unknown", sizeof(pnetif->DeviceID));
        (void) strncpy(pnetif->DeviceVer, "Unknown", sizeof(pnetif->DeviceVer));
        (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_READY);
        netif_set_link_up(pnetif->netif);
      }
      else
      {
        NET_DBG_ERROR("can't add interface (netif)\n");
      }
    }
  }
  return ret;
}

int32_t net_cypress_whd_if_start(net_if_handle_t *pnetif)
{
  int32_t ret = 0;
  if (pnetif->pdrv->extension.wifi->mode == NET_WIFI_MODE_STA)
  {
    ret =  net_cypress_whd_if_start_sta(pnetif);
  }
  else
  {
    ret =  net_cypress_whd_if_start_ap(pnetif);
  }
  return ret;
}

static int32_t net_cypress_whd_if_connect(net_if_handle_t *pnetif)
{
  int32_t       ret;

  ret =  net_ip_connect(pnetif);
  if (ret != NET_OK)
  {
    NET_DBG_ERROR("Failed to connect\n");
    ret = NET_ERROR_NO_CONNECTION;
  }

  return ret;
}


static int32_t net_cypress_whd_if_disconnect(net_if_handle_t *pnetif)
{
  int32_t       ret;
  ret =  net_ip_disconnect(pnetif);
  if (ret == NET_OK)
  {
    ret = net_state_manage_event(pnetif, NET_EVENT_INTERFACE_READY);
  }
  return ret;
}

static int32_t net_cypress_whd_if_stop(net_if_handle_t *pnetif)
{
  int32_t ret;

  ret =  net_ip_remove_if(pnetif, NULL);
  if (ret == NET_OK)
  {
    ret =     net_state_manage_event(pnetif, NET_EVENT_INTERFACE_INITIALIZED);
  }
  if (pnetif->pdrv->extension.wifi->mode == NET_WIFI_MODE_STA)
  {
    whd_wifi_leave(pnetif->pdrv->extension.wifi->ifp);
  }
  else
  {
    whd_wifi_stop_ap((whd_interface_t)pnetif->pdrv->extension.wifi->ifp);
  }
  return ret;
}

static int32_t net_cypress_whd_if_deinit(net_if_handle_t *pnetif)
{
  int32_t ret = NET_OK;
  uint32_t      i;

  /*Switch off Wifi*/
  whd_wifi_off(pnetif->pdrv->extension.wifi->ifp);

  for (i = 0; i < NET_CYPRESS_MAX_INTERFACE; i++)
  {
    if (netif2ifp[i].key == pnetif->pdrv->extension.wifi->ifp)
    {
      break;
    }
  }

  if (i == NET_CYPRESS_MAX_INTERFACE)
  {
    WPRINT_WHD_DEBUG(("Couldn't find the interface \n"));
    return ERR_VAL;
  }
  else
  {
    netif2ifp[i].key = NULL;
    netif2ifp[i].value = NULL;

    if (cypress_alive_interface_count == 1)
    {
      /*Deletes all the interface and De-init the whd, free whd_driver memory */
      whd_deinit(pnetif->pdrv->extension.wifi->ifp);
      whd_powerdown(&whd_driver);

    }
    cypress_alive_interface_count--;
    NET_FREE(pnetif->pdrv->extension.wifi);
    NET_FREE(pnetif->pdrv);
    pnetif->pdrv = NULL;
  }
  return ret;
}

static  err_t low_level_init(struct netif *netif)
{
  err_t ret = ERR_VAL;
  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  net_if_handle_t *pnetif = netif->state;
  whd_interface_t ifp = pnetif->pdrv->extension.wifi->ifp;

  /* to retrieve back netif from ifp */
  for (int32_t i = 0; i < NET_CYPRESS_MAX_INTERFACE; i++)
  {
    if (netif2ifp[i].key == NULL)
    {
      netif2ifp[i].key = ifp;
      netif2ifp[i].value = netif;
      ret = (err_t) ERR_OK;
      break;
    }
  }



  netif->name[0] = 'c';
  netif->name[1] = 'y';

  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...)
   */
  netif->output = etharp_output;
#if LWIP_IPV6
  netif->output_ip6 = ethip6_output;
#endif /* LWIP_IPV6 */
  netif->linkoutput = low_level_output;


  /* Set MAC hardware address length ( 6)*/
  netif->hwaddr_len = (u8_t) ETHARP_HWADDR_LEN;

  /* Setup the physical address of this IP instance. */
  if (whd_wifi_get_mac_address(ifp, (whd_mac_t *)  netif->hwaddr) != WHD_SUCCESS)
  {
    WPRINT_WHD_DEBUG(("Couldn't get MAC address\n"));
    return ERR_VAL;
  }
  WPRINT_WHD_DEBUG((" MAC address %x.%x.%x.%x.%x.%x\n", netif->hwaddr[0], netif->hwaddr[1], netif->hwaddr[2],
                    netif->hwaddr[3], netif->hwaddr[4], netif->hwaddr[5]));

  /* Set Maximum Transfer Unit */
  netif->mtu = (u16_t) WHD_PAYLOAD_MTU;

  /* Set device capabilities. Don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = (u8_t)(NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET);

  /* Do whatever else is needed to initialize interface. */
#if LWIP_IGMP
  netif->flags |= NETIF_FLAG_IGMP;
  netif_set_igmp_mac_filter(netif, lwip_igmp_mac_filter);
#endif /* LWIP_IGMP */

  /* Register a handler for any address changes  and when interface goes up or down*/
  netif_set_status_callback(netif, net_ip_status_cb);
  netif_set_link_callback(netif, net_ip_status_cb);

  return ret;

}



/* This function should do the actual transmission of the packet. The packet is
* contained in the pbuf that is passed to the function. This pbuf
* might be chained.
*
* @param netif the lwip network interface structure for this ethernetif
* @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
* @return ERR_OK if the packet could be sent
*         an err_t value if the packet couldn't be sent
*
* @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
*       strange results. You might consider waiting for space in the DMA queue
*       to become availale since the stack doesn't retry to send a packet
*       dropped because of memory failure (except for the TCP timers).
*/
static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  net_if_handle_t *pnetif = netif->state;
  whd_interface_t ifp = pnetif->pdrv->extension.wifi->ifp;

  if (whd_wifi_is_ready_to_transceive(ifp) == WHD_SUCCESS)
  {
    /* Take a reference to this packet */
    pbuf_ref(p);
#if 0
    NET_DBG_PRINT("Transmit buffer 1 %x next=%p  tot-len=%d len=%d\n", p, p->next, p->tot_len, p->len);
#endif /* for debug */
    LWIP_ASSERT("No chained buffers", ((p->next == NULL) && ((p->tot_len == p->len))));
#ifdef NET_PERF
    stat.whd_send_cycle -= net_get_cycle();
#endif /* NET_PERF */
    whd_network_send_ethernet_data((whd_interface_t) ifp, p);
#ifdef NET_PERF
    /*stat.whd_send_cycle += net_get_cycle();*/
#endif /* NET_PERF */
    LINK_STATS_INC(link.xmit);
    return (err_t) ERR_OK;
  }
  else
  {
    NET_DBG_PRINT("cannot transmit wifi not rdy\n");

    /* Stop lint warning about packet not being freed - it is not being referenced */ /*@-mustfree@*/
    return (err_t) ERR_INPROGRESS; /* Note that signalling ERR_CLSD or ERR_CONN causes loss of connectivity on a roam */
    /*@+mustfree@*/
  }
}

uint32_t        cy_callback_tcpip = 0;
uint32_t        cy_callback = 0;

/**
  * This function should be called when a packet is ready to be read
  * from the interface. It uses the function low_level_input() that
  * should handle the actual reception of bytes from the network
  * interface. Then the type of the received packet is determined and
  * the appropriate input function is called.
  *
  * @param p : the incoming ethernet packet
  */
void cy_network_process_ethernet_data(whd_interface_t interface, whd_buffer_t buff)
{

  struct eth_hdr *ethernet_header;
  struct netif   *tmp_netif;
  u8_t            result;
  uint16_t        ethertype;
  struct pbuf *buffer = (struct pbuf *) buff;

  if (buffer == NULL)
  {
    return;
  }

  /* points to packet payload, which starts with an Ethernet header */
  ethernet_header = (struct eth_hdr *) buffer->payload;

  ethertype = lwip_htons(ethernet_header->type);

#ifdef FILTER
  if (filter_ethernet_packet_callback != NULL && filter_ethertype == ethertype && filter_interface == interface)
  {
    filter_ethernet_packet_callback(buffer->payload, filter_userdata);
  }
#endif /* FILTER */
  /* Check if this is an 802.1Q VLAN tagged packet */
  if (ethertype == WHD_ETHERTYPE_8021Q)
  {
    /* Need to remove the 4 octet VLAN Tag, by moving src and dest addresses 4 octets to the right,
      * and then read the actual ethertype. The VLAN ID and priority fields are currently ignored. */
    uint8_t temp_buffer[ 12 ];
    memcpy(temp_buffer, buffer->payload, 12);
    memcpy(((uint8_t *) buffer->payload) + 4, temp_buffer, 12);

    buffer->payload = ((uint8_t *) buffer->payload) + 4;
    buffer->len = (u16_t)(buffer->len - 4);

    ethernet_header = (struct eth_hdr *) buffer->payload;
    ethertype = lwip_htons(ethernet_header->type);
  }
#ifdef DEEP_SLEEP
  if (WHD_DEEP_SLEEP_IS_ENABLED() && (WHD_DEEP_SLEEP_SAVE_PACKETS_NUM != 0))
  {
    if (wiced_deep_sleep_save_packet(buffer, interface))
    {
      return;
    }
  }
#endif /* DEEP_SLEEP */
  cy_callback++;
  switch (ethertype)
  {
    case WHD_ETHERTYPE_IPv4:
    case WHD_ETHERTYPE_ARP:
#if PPPOE_SUPPORT
    /* PPPoE packet? */
    case ETHTYPE_PPPOEDISC:
    case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */

      /* Find the netif object matching the provided interface */
      tmp_netif = NULL;
      for (int32_t i = 0; i < NET_CYPRESS_MAX_INTERFACE; i++)
      {
        if (netif2ifp[i].key == interface)
        {
          tmp_netif = netif2ifp[i].value;
          break;
        }
      }

      /*NET_DBG_PRINT("Recv %d for netif %x  ifp %x\n",buffer->len,tmp_netif,interface);*/

      if (tmp_netif == NULL)
      {
        NET_DBG_PRINT("This buffer is not for this interface !\n");

        /* Received a packet for a network interface is not initialised Cannot do anything with packet
        - just drop it. */
        result = pbuf_free(buffer);
        LWIP_ASSERT("Failed to release packet buffer", (result != (u8_t)0));
        buffer = NULL;
        return;
      }
#if 0
      NET_DBG_PRINT("process input packet ethertype %x size %d\n", ethertype, buffer->len);
#endif /* debug */

      /* Send to packet to tcpip_thread to process */
      cy_callback_tcpip++;

      if (tcpip_input(buffer, tmp_netif) != ERR_OK)
      {
        LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));

        /* Stop lint warning - packet has not been released in this case */ /*@-usereleased@*/
        result = pbuf_free(buffer);
        /*@+usereleased@*/
        LWIP_ASSERT("Failed to release packet buffer", (result != (u8_t)0));
        buffer = NULL;
      }
      break;
#if 0
    /*FIXME , not supported todays */
    case WHD_ETHERTYPE_EAPOL:
      whd_eapol_receive_eapol_packet(buffer, interface);
      break;
#endif /* 0 */
    default:
      result = pbuf_free(buffer);
      LWIP_ASSERT("Failed to release packet buffer", (result != (u8_t)0));
      buffer = NULL;
      break;
  }
}


static int32_t net_cypress_whd_scan(net_if_handle_t *pnetif, net_wifi_scan_mode_t mode, char *ssid)
{
  (void) pnetif;
  (void) mode;
  (void) ssid;
  return NET_OK;
}


static int32_t net_cypress_whd_scan_result(net_if_handle_t *pnetif, net_wifi_scan_results_t *scan_bss, uint8_t number)
{
  int32_t ret = NET_ERROR_GENERIC;
  whd_sync_scan_result_t        *scan_result;

  if ((NULL == scan_bss) || (0 == number))
  {
    return NET_ERROR_PARAMETER;
  }

  scan_result = (whd_sync_scan_result_t *)NET_MALLOC(sizeof(whd_sync_scan_result_t) * number);
  if (NULL == scan_result)
  {
    return NET_ERROR_NO_MEMORY;
  }
  else
  {
    uint32_t    apcount;
    whd_sync_scan_result_t     *scan_result_info = scan_result;

    memset(scan_result, 0, sizeof(whd_sync_scan_result_t) * number);
    apcount = whd_wifi_scan_synch((whd_interface_t) pnetif->pdrv->extension.wifi->ifp, scan_result, number);

    for (uint32_t i = 0; i < apcount; i++)
    {
      memset(scan_bss, 0, sizeof(net_wifi_scan_bss_t));
      memcpy(scan_bss->ssid.value, (void *) scan_result_info->SSID.value, scan_result_info->SSID.length);
      scan_bss->ssid.value[scan_result_info->SSID.length] = 0;
      scan_bss->ssid.length = scan_result_info->SSID.length;

      scan_bss->security = scan_result_info->security;
      memcpy(&scan_bss->bssid, scan_result_info->BSSID.octet, NET_WIFI_MAC_ADDRESS_SIZE);
      scan_bss->rssi = (int8_t)scan_result_info->signal_strength;
      scan_bss->channel = scan_result_info->channel;
      memcpy(scan_bss->country, ".CN", 4);  /* NOT SUPPORT for MX_WIFI */

      scan_bss++;
      scan_result_info++;
    }
    ret = apcount;
  }

  free((void *) scan_result);
  return ret;
}



/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
