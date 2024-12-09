#include <stdbool.h>
#include <string.h>

#ifdef NX_DEBUG
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#endif /* NX_DEBUG */

#include "nx_api.h"
#include "mx_wifi.h"
#include "core/mx_address.h"


#ifndef NX_DRIVER_DEFERRED_PROCESSING
#error The symbol NX_DRIVER_DEFERRED_PROCESSING should be defined
#endif /* NX_DRIVER_DEFERRED_PROCESSING */

#if !defined(WIFI_SSID)
#error The symbol WIFI_SSID should be defined
#endif /* WIFI_SSID  */

#if !defined(WIFI_PASSWORD)
#error The symbol WIFI_PASSWORD should be defined
#endif /* WIFI_PASSWORD  */

#define NX_DRIVER_ENABLE_DEFERRED

/* Indicate that driver source is being compiled. */
#define NX_DRIVER_SOURCE

#include "nx_driver_emw3080.h"
#include "nx_driver_framework.c"
#include "io_pattern/mx_wifi_io.h"

/* The station mode is the default. */
uint8_t WifiMode = MC_STATION;

static void _nx_netlink_input_callback(mx_buf_t *pbuf, void *user_args);
static void _nx_mx_wifi_status_changed(uint8_t cate, uint8_t status, void *arg);

static UINT _nx_driver_emw3080_initialize(NX_IP_DRIVER *driver_req_ptr);
static UINT _nx_driver_emw3080_enable(NX_IP_DRIVER *driver_req_ptr);
static UINT _nx_driver_emw3080_disable(NX_IP_DRIVER *driver_req_ptr);
static UINT _nx_driver_emw3080_packet_send(NX_PACKET *packet_ptr);
static UINT _nx_driver_emw3080_interface_status(NX_IP_DRIVER *driver_req_ptr);
static VOID _nx_driver_emw3080_packet_received(VOID);

#if defined(NX_DEBUG)
static const char *nx_driver_mx_wifi_status_to_string(uint8_t status);
#endif /* NX_DEBUG */

static volatile bool nx_driver_interface_up = false;
static volatile bool nx_driver_ip_acquired = false;


void nx_driver_emw3080_entry(NX_IP_DRIVER *driver_req_ptr)
{
  static bool started = false;
  if (!started)
  {
    nx_driver_hardware_initialize         = _nx_driver_emw3080_initialize;
    nx_driver_hardware_enable             = _nx_driver_emw3080_enable;
    nx_driver_hardware_disable            = _nx_driver_emw3080_disable;
    nx_driver_hardware_packet_send        = _nx_driver_emw3080_packet_send;
    nx_driver_hardware_get_status         = _nx_driver_emw3080_interface_status;
    nx_driver_hardware_packet_received    = _nx_driver_emw3080_packet_received;

    started = true;
  }

  nx_driver_framework_entry_default(driver_req_ptr);
}

void nx_driver_emw3080_interrupt(void)
{
  ULONG deffered_events;

  if ((!nx_driver_interface_up) || (!nx_driver_ip_acquired))
  {
    return; /* not yet running */
  }

  deffered_events = nx_driver_information.nx_driver_information_deferred_events;

  nx_driver_information.nx_driver_information_deferred_events |= NX_DRIVER_DEFERRED_PACKET_RECEIVED;

  if (!deffered_events)
  {
    /* Call NetX deferred driver processing.  */
    _nx_ip_driver_deferred_processing(nx_driver_information.nx_driver_information_ip_ptr);
  }
}

static UINT _nx_driver_emw3080_initialize(NX_IP_DRIVER *driver_req_ptr)
{
  if (mx_wifi_alloc_init())
  {
    return NX_DRIVER_ERROR;
  }

  if (mxwifi_probe(NULL))
  {
    return NX_DRIVER_ERROR;
  }

  if (MX_WIFI_HardResetModule(wifi_obj_get()))
  {
    return NX_DRIVER_ERROR;
  }

  if (MX_WIFI_Init(wifi_obj_get()))
  {
    return NX_DRIVER_ERROR;
  }
  /* Register MAC address that corresponds to STA mode by default. */
  nx_driver_update_hardware_address(wifi_obj_get()->SysInfo.MAC);

  if (MX_WIFI_RegisterStatusCallback_if(wifi_obj_get(),
                                        _nx_mx_wifi_status_changed,
                                        NULL /* void * arg */,
                                        (mwifi_if_t)WifiMode))
  {
    return NX_DRIVER_ERROR;
  }

  return NX_SUCCESS;
}

UINT _nx_driver_emw3080_enable(NX_IP_DRIVER *driver_req_ptr)
{
  if (WifiMode == MC_STATION)
  {
#if defined(NX_DEBUG)
    printf("Joining ... \"%s\"\n", WIFI_SSID);
#endif /* NX_DEBUG */

    if (MX_WIFI_STATUS_OK != MX_WIFI_Connect(wifi_obj_get(), WIFI_SSID, WIFI_PASSWORD, MX_WIFI_SEC_AUTO))
    {
      return NX_DRIVER_ERROR;
    }
  }
  else
  {
    MX_WIFI_APSettings_t ApSettings = {0};

    strncpy(ApSettings.SSID, WIFI_SSID, sizeof(ApSettings.SSID));
    strncpy(ApSettings.pswd, WIFI_PASSWORD, sizeof(ApSettings.pswd));
    ApSettings.channel = 8;

    {
      const mx_ip_addr_t ip_addr = { htonl(IP_ADDRESS(0, 0, 0, 0)) };
      strncpy(ApSettings.ip.localip, mx_ntoa(&ip_addr), sizeof(ApSettings.ip.localip));
    }
    {
      const mx_ip_addr_t network_mask = { htonl(IP_ADDRESS(0, 0, 0, 0)) };
      strncpy(ApSettings.ip.netmask, mx_ntoa(&network_mask), sizeof(ApSettings.ip.netmask));
    }
    {
      const mx_ip_addr_t dns_server_addr = { htonl(IP_ADDRESS(0, 0, 0, 0)) };
      strncpy(ApSettings.ip.dnserver, mx_ntoa(&dns_server_addr), sizeof(ApSettings.ip.dnserver));
    }
    {
      const mx_ip_addr_t gateway_addr = { htonl(IP_ADDRESS(0, 0, 0, 0)) };
      strncpy(ApSettings.ip.gateway, mx_ntoa(&gateway_addr), sizeof(ApSettings.ip.gateway));
    }

#if defined(NX_DEBUG)
    printf("Init Access Point ... \"%s\" with %s\n", ApSettings.SSID, ApSettings.ip.localip);
#endif /* NX_DEBUG */

    if (MX_WIFI_STATUS_OK != MX_WIFI_StartAP(wifi_obj_get(), &ApSettings))
    {
      return NX_DRIVER_ERROR;
    }
    else
    {
      uint8_t ap_mac[MX_WIFI_MAC_SIZE] = {0};

      (void)MX_WIFI_GetsoftapMACAddress(wifi_obj_get(), ap_mac);

      /* Register MAC address that corresponds to AP mode. */
      nx_driver_update_hardware_address(ap_mac);
    }
  }

  while (!nx_driver_interface_up)
  {
    MX_WIFI_IO_YIELD(wifi_obj_get(), 10 /* timeout */);
  }

  if (MX_WIFI_STATUS_OK != MX_WIFI_Network_bypass_mode_set(wifi_obj_get(),
                                                           1 /* enable */,
                                                           _nx_netlink_input_callback,
                                                           NULL /* user arg */))
  {
    return NX_DRIVER_ERROR;
  }

  return NX_SUCCESS;
}


UINT _nx_driver_emw3080_disable(NX_IP_DRIVER *driver_req_ptr)
{
  MX_WIFI_Network_bypass_mode_set(wifi_obj_get(), 0 /* disable */, NULL, NULL);

  if (MX_WIFI_Disconnect(wifi_obj_get()))
  {
    return NX_DRIVER_ERROR;
  }

  if (MX_WIFI_DeInit(wifi_obj_get()))
  {
    return NX_DRIVER_ERROR;
  }

  return NX_SUCCESS;
}


UINT _nx_driver_emw3080_packet_send(NX_PACKET *packet_ptr)
{
  static int errors = 0;

  /* Verify that the length matches the size between the pointers. */
  if (packet_ptr->nx_packet_length != (packet_ptr->nx_packet_append_ptr - packet_ptr->nx_packet_prepend_ptr))
  {
    return NX_DRIVER_ERROR;
  }

  if (packet_ptr->nx_packet_next)
  {
    NX_DRIVER_PHYSICAL_HEADER_REMOVE(packet_ptr);
    nx_packet_transmit_release(packet_ptr);
    return NX_DRIVER_ERROR;
  }

#if defined(NX_DEBUG)
  if ((packet_ptr->nx_packet_prepend_ptr - packet_ptr->nx_packet_data_start) < 28)
  {
    printf("Incorrect NX packet, need at least 28 byte in front of payload, got %d\n",
           packet_ptr->nx_packet_prepend_ptr - packet_ptr->nx_packet_data_start);
  }
#endif /* NX_DEBUG */

  {
    int32_t interface = (WifiMode == MC_STATION) ? STATION_IDX : SOFTAP_IDX;

    if (MX_WIFI_Network_bypass_netlink_output(wifi_obj_get(),
                                              packet_ptr->nx_packet_prepend_ptr, packet_ptr->nx_packet_length,
                                              interface))
    {
      errors++;
    }
  }

  NX_DRIVER_PHYSICAL_HEADER_REMOVE(packet_ptr);
  nx_packet_transmit_release(packet_ptr);

  return NX_SUCCESS;
}


static void _nx_netlink_input_callback(mx_buf_t *buffer, void *user_args)
{
  NX_PACKET *packet_ptr = buffer;

  /* Avoid starving. */
  if (packet_ptr -> nx_packet_pool_owner -> nx_packet_pool_available == 0)
  {
    nx_packet_release(packet_ptr);
    return;
  }

  /* Everything is OK, transfer the packet to NetX. */
  nx_driver_transfer_to_netx(nx_driver_information.nx_driver_information_ip_ptr, packet_ptr);
}


static VOID _nx_driver_emw3080_packet_received(VOID)
{
  MX_WIFI_IO_YIELD(wifi_obj_get(), 100 /* timeout */);
}


static UINT _nx_driver_emw3080_interface_status(NX_IP_DRIVER *driver_req_ptr)
{
  UINT status = NX_PTR_ERROR;

  if (NULL != driver_req_ptr -> nx_ip_driver_return_ptr)
  {
    *driver_req_ptr -> nx_ip_driver_return_ptr = nx_driver_interface_up ? NX_TRUE : NX_FALSE;
    status = NX_SUCCESS;
  }

  return status;
}


static void _nx_mx_wifi_status_changed(uint8_t cate, uint8_t status, void *arg)
{

#if defined(NX_DEBUG)
  printf("\n[%06"PRIu32"] > %s\n", HAL_GetTick(), nx_driver_mx_wifi_status_to_string(status));
#endif /* NX_DEBUG */

  if ((uint8_t)MC_STATION == cate)
  {
    switch (status)
    {
      case MWIFI_EVENT_STA_DOWN:
        nx_driver_interface_up = false;
        break;

      case MWIFI_EVENT_STA_UP:
        nx_driver_interface_up = true;
        break;

      case MWIFI_EVENT_STA_GOT_IP:
        nx_driver_ip_acquired = true;
        break;

      default:
        break;
    }
  }
  else if ((uint8_t)MC_SOFTAP == cate)
  {
    switch (status)
    {
      case MWIFI_EVENT_AP_DOWN:
        nx_driver_interface_up = false;
        break;

      case MWIFI_EVENT_AP_UP:
        nx_driver_interface_up = true;
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


#if defined(NX_DEBUG)
#define CASE(x) case x: return #x
#define DEFAULT default: return "UNKNOWN"
static const char *nx_driver_mx_wifi_status_to_string(uint8_t status)
{
  switch (status)
  {
      CASE(MWIFI_EVENT_NONE);
      CASE(MWIFI_EVENT_STA_DOWN);
      CASE(MWIFI_EVENT_STA_UP);
      CASE(MWIFI_EVENT_STA_GOT_IP);
      CASE(MWIFI_EVENT_AP_DOWN);
      CASE(MWIFI_EVENT_AP_UP);
      DEFAULT;
  }
}
#endif /* NX_DEBUG */

