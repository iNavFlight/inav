/**
  ******************************************************************************
  * @file    mx_wifi_ipc.c
  * @author  MCD Application Team
  * @brief   Host driver IPC protocol of MXCHIP Wi-Fi component.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
/*cstat -MISRAC2012-* */
#include <stdint.h>
#include <stdio.h>
/*cstat +MISRAC2012-* */

#include "mx_wifi.h"
#include "lib_ipc/lib_ipc.h"
#include "mx_wifi_ipc.h"
#include "mx_wifi_slip.h"

/*cstat -MISRAC2012-* */
#include "net_address.h"
/*cstat +MISRAC2012-* */

#ifdef MX_WIFI_IPC_DEBUG
#define DEBUG_LOG(M, ...)  printf(M, ##__VA_ARGS__);
#else
#define DEBUG_LOG(M, ...)
#endif

extern MX_WIFIObject_t *wifi_obj_get(void);
extern uint32_t      HAL_GetTick(void);

ipc_api_t ipc_event_tbl[] =
{
#define IPC_CMD_API(name) name,
#include "lib_ipc/ipc_event_list.h"
};

static uint32_t mipc_timeout_ms = 200;
int32_t recv_cnt;

// ipc low level output interface
int32_t ipc_output(uint8_t *data, int32_t len)
{
  int32_t ret;

  if (mx_wifi_hci_send(data, (uint16_t)len, mipc_timeout_ms) > 0)
  {
    ret = 0;
  }
  else
  {
    ret = -1;
  }

  return ret;
}

/************************************************
  * IPC API implementations for mx_wifi over HCI
  ***********************************************/
int32_t mipc_init(phy_tx_func_t io_send, phy_rx_func_t io_recv, uint32_t timeout_ms)
{
  mipc_timeout_ms = timeout_ms;

  return mx_wifi_hci_init(io_send, io_recv, NULL);
}

int32_t mipc_deinit(void)
{
  return mx_wifi_hci_deinit();
}

int32_t mipc_poll(int32_t *p_ret_flag, int32_t *p_ret, uint32_t timeout_ms)
{
  int32_t ret = IPC_RET_ERROR;
  uint8_t *p_out_data = NULL;
  int16_t recv_len;
  uint32_t time_start;

  time_start = HAL_GetTick();
  do
  {
    // poll new event
    recv_len = mx_wifi_hci_recv(&p_out_data, 20);
    if ((recv_len > 0) && (NULL != p_out_data))
    {
      (void)ipc_api_recv(p_out_data, recv_len);
    }

    if (NULL != p_out_data)
    {
      HCI_FREE(p_out_data);
      p_out_data = NULL;
    }

    // check cmd response
    if (NULL != p_ret_flag) // need check ret_flag
    {
      if (MIPC_RET_FLAG_SET == *p_ret_flag) // response flag set
      {
        if (NULL != p_ret) // need check return value
        {
          if (IPC_RET_OK == *p_ret)
          {
            ret = IPC_RET_OK;
          }
        }
        else
        {
          ret = IPC_RET_OK;
        }
        break;  // response flag set return
      }
    }
  } while ((HAL_GetTick() - time_start) < timeout_ms);

  return ret;
}


/* copy return data */
static void _mipc_ret_copy(uint8_t *buf, int32_t len)
{
  int32_t *p_ret = NULL;
  uint8_t *pos;

  if ((NULL != buf) && (len > 4))
  {
    (void)memcpy(&p_ret, buf, 4);
    pos = &(buf[4]); /* pos = buf + 4 */
    (void)memcpy(p_ret, pos, (size_t)len - (size_t)4);
  }
}

static void _mipc_ret_flag_set(int32_t *p_ret_flag)
{
  if (NULL != p_ret_flag)
  {
    *p_ret_flag = MIPC_RET_FLAG_SET;
  }
}

/************************************************
  * master cmd send (MASTER --> SLAVE)
  ***********************************************/
int32_t ipc_echo_cmd(void *arg1, int32_t len, int32_t p_ret_flag)
{
  uint8_t arg_type[4];
  int32_t ret;

  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_NONE;

  ret = ipc_api_send((uint16_t)IPC_CMD_ipc_echo_cmd, arg_type, arg1, len, p_ret_flag);

  return ret;
}

int32_t ipc_echo_event(void *arg1, int32_t len, int32_t p_ret_flag)
{
  int32_t *p_ret_flag_addr;
#ifdef MX_WIFI_IPC_DEBUG
  char_t *str = (char_t *)arg1;
  int32_t i;

  DEBUG_LOG("IPC EVENT: echo len(%d), \r", len);
  for (i = 0; i < len; i++)
  {
    DEBUG_LOG("%c", str[i]);
  }
  DEBUG_LOG("\r\n");
#endif

  (void)arg1;
  recv_cnt += len;  // echo count

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return IPC_RET_OK;
}

int32_t system_reboot_cmd(void)
{
  uint8_t arg_type[3];
  int32_t ret;

  arg_type[0] = (uint8_t)IPC_TYPE_NONE;

  ret = ipc_api_send((uint16_t)IPC_CMD_system_reboot_cmd, arg_type);

  return ret;
}

int32_t system_factory_cmd(void)
{
  uint8_t arg_type[1];
  int32_t ret;

  arg_type[0] = (uint8_t)IPC_TYPE_NONE;

  ret = ipc_api_send((uint16_t)IPC_CMD_system_factory_cmd, arg_type);

  return ret;
}

int32_t system_firmware_version_get_cmd(int32_t version_buf_addr, int32_t version_buf_len,
                                        int32_t ret_addr, int32_t p_ret_flag)
{
  uint8_t arg_type[5];
  int32_t ret;

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_NONE;

  ret = ipc_api_send((uint16_t)IPC_CMD_system_firmware_version_get_cmd, arg_type,
                     version_buf_addr, version_buf_len,
                     ret_addr, p_ret_flag);

  return ret;
}

int32_t system_firmware_version_get_event(uint8_t *ret_version, int32_t ret_version_len,
                                          uint8_t *ret, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t rc;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_version) && (ret_version_len > 4))
  {
    DEBUG_LOG("IPC EVENT: version len(%d), %.*s\r\n",
              ret_version_len - 4, ret_version_len - 4, ret_version + 4);

    // version
    _mipc_ret_copy(ret_version, ret_version_len);

    // return value
    _mipc_ret_copy(ret, ret_len);

    // set flag
    (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
    _mipc_ret_flag_set(p_ret_flag_addr);

    rc = IPC_RET_OK;
  }
  else
  {
    rc = IPC_RET_ERROR;
  }

  return rc;
}

/*
  * wifi cmd
  */
int32_t wifi_mac_get_cmd(int32_t mac_buf_addr, int32_t mac_buf_len,
                         int32_t ret_addr, int32_t p_ret_flag)
{
  uint8_t arg_type[5];
  int32_t ret;

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_NONE;

  ret = ipc_api_send((uint16_t)IPC_CMD_wifi_mac_get_cmd, arg_type,
                     mac_buf_addr, mac_buf_len,
                     ret_addr, p_ret_flag);

  return ret;
}

int32_t wifi_mac_get_event(uint8_t *ret_mac, int32_t ret_mac_len,
                           uint8_t *ret, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t rc = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

#ifdef MX_WIFI_IPC_DEBUG
  uint8_t *mac = NULL;
#endif

  if ((NULL != ret_mac) && (ret_mac_len > 4))
  {
#ifdef MX_WIFI_IPC_DEBUG
    mac = ret_mac + 4;
    DEBUG_LOG("IPC EVENT: MAC len(%d), %02X%02X%02X%02X%02X%02X\r\n",
              ret_mac_len - 4, mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
#endif
    // version
    _mipc_ret_copy(ret_mac, ret_mac_len);

    // return value
    _mipc_ret_copy(ret, ret_len);

    // set flag
    (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
    _mipc_ret_flag_set(p_ret_flag_addr);

    rc = IPC_RET_OK;
  }

  return rc;
}

int32_t wifi_scan_cmd(int32_t mode, void *ssid, int32_t ssid_len, int32_t p_ret_flag)
{
  uint8_t arg_type[5];
  int32_t ret;

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_NONE;

  ret = ipc_api_send((uint16_t)IPC_CMD_wifi_scan_cmd, arg_type, mode, ssid, ssid_len, p_ret_flag);

  return ret;
}

int32_t wifi_scan_event(int32_t num, uint8_t *ap_list, int32_t ap_list_len, int32_t p_ret_flag)
{
  int32_t *p_ret_flag_addr;
  int32_t ret_num;

#ifdef MX_WIFI_IPC_DEBUG
  mc_wifi_ap_info_t ap_info;
  uint8_t *list_pos = ap_list;

  DEBUG_LOG("IPC EVENT: scan result: num=%d\r\n", num);
  for (int32_t i = 0; i < num; i++)
  {
    memcpy(&ap_info, list_pos, sizeof(mc_wifi_ap_info_t));
    list_pos += sizeof(mc_wifi_ap_info_t);
    DEBUG_LOG("\t ssid %s,  bssid %02X%02X%02X%02X%02X%02X,  rssi %d,  security %d,  channel %d\r\n",
              ap_info.ssid, ap_info.bssid[0], ap_info.bssid[1], ap_info.bssid[2],
              ap_info.bssid[3], ap_info.bssid[4], ap_info.bssid[5],
              ap_info.rssi, ap_info.security, ap_info.channel);
  }
#endif
  /* save raw scan data */
  (void)ap_list_len;
  ret_num = MIN(num, (((int32_t)(MX_WIFI_SCAN_BUF_SIZE)) / ((int32_t)sizeof(mc_wifi_ap_info_t))));
  (void)memset(&(wifi_obj_get()->Runtime.scan_result), 0, (size_t)(MX_WIFI_SCAN_BUF_SIZE));
  (void)memcpy(&(wifi_obj_get()->Runtime.scan_result), ap_list, (size_t)ret_num * sizeof(mc_wifi_ap_info_t));
  wifi_obj_get()->Runtime.scan_number = (uint8_t)ret_num;

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return IPC_RET_OK;
}

int32_t wifi_link_info_get_cmd(int32_t p_ret_flag)
{
  uint8_t arg_type[2];
  int32_t ret;

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_NONE;

  ret = ipc_api_send((uint16_t)IPC_CMD_wifi_link_info_get_cmd, arg_type, p_ret_flag);

  return ret;
}

int32_t wifi_link_info_get_event(uint8_t *info, int32_t len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;
  mc_wifi_link_info_t mc_link;

  if ((NULL != info) && (len == (int32_t)sizeof(mc_wifi_link_info_t)))
  {
    if (MX_WIFI_LOCK((wifi_obj_get())))
    {
      (void)memcpy(&mc_link, info, sizeof(mc_link));
      wifi_obj_get()->NetSettings.IsConnected = (int8_t)(mc_link.is_connected);
      (void)strncpy((char_t *)wifi_obj_get()->NetSettings.SSID, mc_link.ssid, mxMaxSsidLen);
      (void)memcpy(&(wifi_obj_get()->NetSettings.Security), &(mc_link.security), 1);
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((wifi_obj_get()));
#endif
    }

    // set flag
    (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
    _mipc_ret_flag_set(p_ret_flag_addr);

    ret = IPC_RET_OK;
  }

  return ret;
}

int wifi_eap_set_rootca_cmd(void *rootca, int rootca_len)
{
  uint8_t arg_type[3];
  
  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_NONE;
  
  return ipc_api_send((uint16_t)IPC_CMD_wifi_eap_set_rootca_cmd, arg_type,
                      rootca, rootca_len);
}

int wifi_eap_set_client_cert_cmd(void *client_cert, int client_cert_len)
{
  uint8_t arg_type[3];
  
  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_NONE;
  
  return ipc_api_send((uint16_t)IPC_CMD_wifi_eap_set_client_cert_cmd, arg_type,
                      client_cert, client_cert_len);
}

int wifi_eap_set_client_key_cmd(void *client_key, int client_key_len)
{
  uint8_t arg_type[3];
  
  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_NONE;
  
  return ipc_api_send((uint16_t)IPC_CMD_wifi_eap_set_client_key_cmd, arg_type,
                      client_key, client_key_len);
}

int wifi_eap_connect_cmd(void *ssid, int ssid_len, void *identity, int identity_len, 
                        void *password, int password_len, void *attr, int attr_len,
                        void *ip, int ip_len)
{
  uint8_t arg_type[11];

  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[5] = (uint8_t)IPC_TYPE_DATA;
  arg_type[6] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[7] = (uint8_t)IPC_TYPE_DATA;
  arg_type[8] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[9] = (uint8_t)IPC_TYPE_DATA;
  arg_type[10] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_wifi_eap_connect_cmd, arg_type,
                      ssid, ssid_len, identity, identity_len, password, password_len,
                      attr, attr_len, ip, ip_len);
}

int32_t wifi_connect_cmd(const void *ssid, int32_t ssid_len, const void *key, int32_t key_len,
                         void *connect_attr, int32_t connect_attr_len,
                         void *ip_attr, int32_t ip_attr_len)
{
  uint8_t arg_type[9];
  int32_t ret;

  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;

  arg_type[2] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;

  arg_type[4] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[5] = (uint8_t)IPC_TYPE_DATA;

  arg_type[6] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[7] = (uint8_t)IPC_TYPE_DATA;

  arg_type[8] = (uint8_t)IPC_TYPE_NONE;

  ret = ipc_api_send((uint16_t)IPC_CMD_wifi_connect_cmd, arg_type, ssid, ssid_len, key, key_len,
                     connect_attr, connect_attr_len, ip_attr, ip_attr_len);

  return ret;
}

int32_t wifi_disconnect_cmd(void)
{
  uint8_t arg_type[1];
  int32_t ret;

  arg_type[0] = (uint8_t)IPC_TYPE_NONE;

  ret = ipc_api_send((uint16_t)IPC_CMD_wifi_disconnect_cmd, arg_type);

  return ret;
}

int32_t wifi_status_event(int32_t mode, int32_t state)
{
  if ((int32_t)MC_SOFTAP == mode)
  {
    switch (state)
    {
      case MC_AP_DOWN:
        DEBUG_LOG("IPC EVENT: AP DOWN!\r\n");
        wifi_obj_get()->NetSettings.IsConnected = (int8_t)MC_AP_DOWN;
        if (NULL != wifi_obj_get()->Runtime.status_cb)
        {
          wifi_obj_get()->Runtime.status_cb(MC_SOFTAP, MC_AP_DOWN, wifi_obj_get()->Runtime.callback_arg);
        }
        break;

      case MC_AP_UP:
        DEBUG_LOG("IPC EVENT: AP UP!\r\n");
        wifi_obj_get()->NetSettings.IsConnected = (int8_t)MC_AP_UP;
        if (NULL != wifi_obj_get()->Runtime.status_cb)
        {
          wifi_obj_get()->Runtime.status_cb(MC_SOFTAP, MC_AP_UP, wifi_obj_get()->Runtime.callback_arg);
        }
        break;

      default:
        break;
    }

  }
  else if ((int32_t)MC_STATION == mode)
  {
    switch (state)
    {
      case MC_STA_DOWN:
        DEBUG_LOG("IPC EVENT: STATION DOWN!\r\n");
        wifi_obj_get()->NetSettings.IsConnected = (int8_t)MC_STA_DOWN;
        if (NULL != wifi_obj_get()->Runtime.status_cb)
        {
          wifi_obj_get()->Runtime.status_cb(MC_STATION, MC_STA_DOWN,
                                            wifi_obj_get()->Runtime.callback_arg);
        }
        break;

      case MC_STA_UP:
        DEBUG_LOG("IPC EVENT: STATION UP!\r\n");
        wifi_obj_get()->NetSettings.IsConnected = (int8_t)MC_STA_UP;
        (void)wifi_ip_get_cmd((int32_t)MC_STATION, 0);
        if (NULL != wifi_obj_get()->Runtime.status_cb)
        {
          wifi_obj_get()->Runtime.status_cb(MC_STATION, MC_STA_UP,
                                            wifi_obj_get()->Runtime.callback_arg);
        }
        break;

      default:
        break;
    }
  }
  else {}

  return 0;
}

int32_t wifi_ip_get_cmd(int32_t mode, int32_t p_ret_flag)
{
  uint8_t arg_type[3] = {0};

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_wifi_ip_get_cmd, arg_type, mode, p_ret_flag);
}

int32_t wifi_ip_get_event(uint8_t *ip, int32_t len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;
  mwifi_ip_attr_t mc_wifi_ip;
  int32_t addr;

  if ((NULL != ip) && (len > 0))
  {
    (void)memcpy(&mc_wifi_ip, ip, sizeof(mc_wifi_ip));
    DEBUG_LOG("IPC EVENT: IP %s, mask %s, gw %s, dns %s\r\n",
              mc_wifi_ip.localip, mc_wifi_ip.netmask, mc_wifi_ip.gateway,
              mc_wifi_ip.dnserver);

    if (MX_WIFI_LOCK((wifi_obj_get())))
    {
      addr = net_aton_r((const char_t *) & (mc_wifi_ip.localip));
      (void)memcpy(wifi_obj_get()->NetSettings.IP_Addr, &addr, 4);

      addr = net_aton_r((const char_t *) & (mc_wifi_ip.netmask));
      (void)memcpy(wifi_obj_get()->NetSettings.IP_Mask, &addr, 4);

      addr = net_aton_r((const char_t *) & (mc_wifi_ip.gateway));
      (void)memcpy(wifi_obj_get()->NetSettings.Gateway_Addr, &addr, 4);

      addr = net_aton_r((const char_t *) & (mc_wifi_ip.dnserver));
      (void)memcpy(wifi_obj_get()->NetSettings.DNS1, &addr, 4);

      if (NULL != wifi_obj_get()->Runtime.status_cb)
      {
        wifi_obj_get()->Runtime.status_cb(MC_STATION, MC_STA_GOT_IP,
                                          wifi_obj_get()->Runtime.callback_arg);
      }
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((wifi_obj_get()));
#endif
    }

    // set response flag

    (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
    _mipc_ret_flag_set(p_ret_flag_addr);

    ret = IPC_RET_OK;
  }

  return ret;
}


// softAP
int32_t wifi_softap_start_cmd(void *ssid, int32_t ssid_len, void *key, int32_t key_len,
                              int32_t channel, void *ip_attr, int32_t ip_attr_len,
                              int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[10];

  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[6] = (uint8_t)IPC_TYPE_DATA;
  arg_type[7] = (uint8_t)IPC_TYPE_DATA;
  arg_type[8] = (uint8_t)IPC_TYPE_DATA;
  arg_type[9] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_wifi_softap_start_cmd, arg_type, ssid, ssid_len,
                      key, key_len, channel, ip_attr, ip_attr_len,
                      p_ret, p_ret_flag);
}

int32_t wifi_softap_start_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    // set flag
    (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
    _mipc_ret_flag_set(p_ret_flag_addr);

    ret = IPC_RET_OK;
  }

  return ret;
}

int32_t wifi_softap_stop_cmd(int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[3];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_wifi_softap_stop_cmd, arg_type, p_ret, p_ret_flag);
}

int32_t wifi_softap_stop_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    // set flag
    (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
    _mipc_ret_flag_set(p_ret_flag_addr);

    ret = IPC_RET_OK;
  }

  return ret;
}

// socket
int32_t socket_gethostbyname_cmd(const void *hostname, int32_t namelen,
                                 int32_t p_host_addr, int32_t host_addr_len,
                                 int32_t p_ret_flag)
{
  uint8_t arg_type[6];

  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_gethostbyname_cmd, arg_type, hostname, namelen,
                      p_host_addr, host_addr_len,
                      p_ret_flag);
}

int32_t socket_gethostbyname_event(uint8_t *ret_host_addr, int32_t ret_host_addr_len,
                                   int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_host_addr) && (ret_host_addr_len > 0))
  {
    // address
    _mipc_ret_copy(ret_host_addr, ret_host_addr_len);

    // set flag
    (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
    _mipc_ret_flag_set(p_ret_flag_addr);

    ret = IPC_RET_OK;
  }

  return ret;
}

int32_t socket_ping_cmd(const void *hostname, int32_t namelen, int32_t count, int32_t delay_ms,
                        int32_t p_ret_buf, int32_t p_ret_flag)
{
  uint8_t arg_type[7];

  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_DATA;
  arg_type[6] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_ping_cmd, arg_type, hostname, namelen, count, delay_ms,
                      p_ret_buf, p_ret_flag);
}

int32_t socket_ping_event(uint8_t *ping_result, int32_t result_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ping_result) && (result_len > 0))
  {
    // ping results
    _mipc_ret_copy((uint8_t *)ping_result, result_len);

    // set flag
    (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
    _mipc_ret_flag_set(p_ret_flag_addr);

    ret = IPC_RET_OK;
  }

  return ret;
}

/*
  * socket cmd
  */
int32_t socket_create_cmd(int32_t domain, int32_t type, int32_t protocol, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[6];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_create_cmd, arg_type, domain, type, protocol,
                      p_ret, p_ret_flag);
}

int32_t socket_create_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    // set flag
    (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
    _mipc_ret_flag_set(p_ret_flag_addr);

    ret = IPC_RET_OK;
  }

  return ret;
}

int32_t socket_setsockopt_cmd(int32_t sockfd, int32_t level, int32_t optname,
                              const void *optval, int32_t optlen, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[8];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_DATA;
  arg_type[6] = (uint8_t)IPC_TYPE_DATA;
  arg_type[7] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_setsockopt_cmd, arg_type, sockfd, level,
                      optname, optval, optlen,
                      p_ret, p_ret_flag);
}

int32_t socket_setsockopt_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    // set flag
    (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
    _mipc_ret_flag_set(p_ret_flag_addr);

    ret = IPC_RET_OK;
  }

  return ret;
}

int32_t socket_getsockopt_cmd(int32_t sockfd, int32_t level, int32_t optname,
                              int32_t p_optbuf, int32_t p_optlen, int32_t optlen,
                              int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[9];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_DATA;
  arg_type[6] = (uint8_t)IPC_TYPE_DATA;
  arg_type[7] = (uint8_t)IPC_TYPE_DATA;
  arg_type[8] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_getsockopt_cmd, arg_type, sockfd, level,
                      optname, p_optbuf, p_optlen, optlen,
                      p_ret, p_ret_flag);
}

int32_t socket_getsockopt_event(uint8_t *optval_buf, int32_t optval_len,
                                uint8_t *ret_optlen_buf, int32_t ret_optlen,
                                uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != optval_buf) && (optval_len > 0))
  {
    // optval
    _mipc_ret_copy((uint8_t *)optval_buf, optval_len);

    // optval len
    _mipc_ret_copy((uint8_t *)ret_optlen_buf, ret_optlen);

    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    // set flag
    (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
    _mipc_ret_flag_set(p_ret_flag_addr);

    ret = IPC_RET_OK;
  }

  return ret;
}

int32_t socket_bind_cmd(int32_t sockfd, const void *addr, int32_t addrlen, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[6];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_bind_cmd, arg_type, sockfd, addr, addrlen,
                      p_ret, p_ret_flag);
}

int32_t socket_bind_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    // set flag
    (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
    _mipc_ret_flag_set(p_ret_flag_addr);

    ret = IPC_RET_OK;
  }

  return ret;
}

int32_t socket_listen_cmd(int32_t sockfd, int32_t n, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[5];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_listen_cmd, arg_type, sockfd, n,
                      p_ret, p_ret_flag);
}

int32_t socket_listen_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t *p_ret_flag_addr;
  int32_t ret = IPC_RET_ERROR;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}


int32_t socket_accept_cmd(int32_t sockfd, int32_t p_addr, int32_t p_addrlen, int32_t addrlen,
                          int32_t p_ret_fd, int32_t p_ret_flag)
{
  uint8_t arg_type[7];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_DATA;
  arg_type[6] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_accept_cmd, arg_type, sockfd, p_addr,
                      p_addrlen, addrlen,
                      p_ret_fd, p_ret_flag);
}

int32_t socket_accept_event(uint8_t *addr_buf, int32_t addr_buf_len,
                            uint8_t *p_addrlen, int32_t addrlen,
                            uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != addr_buf) && (addr_buf_len > 0))
  {
    // addr
    _mipc_ret_copy((uint8_t *)addr_buf, addr_buf_len);

    // addr len
    _mipc_ret_copy((uint8_t *)p_addrlen, addrlen);

    // ret fd
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t socket_connect_cmd(int32_t sockfd, const void *addr, int32_t addrlen, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[6];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_connect_cmd, arg_type, sockfd, addr, addrlen,
                      p_ret, p_ret_flag);
}

int32_t socket_connect_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t socket_send_cmd(int32_t sockfd, const uint8_t *buffer, size_t size, int32_t flags,
                        int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[7];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_DATA;
  arg_type[6] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_send_cmd, arg_type, sockfd, buffer, size, flags,
                      p_ret, p_ret_flag);
}

int32_t socket_send_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t socket_sendto_cmd(int32_t sockfd, const uint8_t *buffer, size_t size, int32_t flags,
                          const void *addr, int32_t addrlen,
                          int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[9];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[5] = (uint8_t)IPC_TYPE_DATA;
  arg_type[6] = (uint8_t)IPC_TYPE_DATA;
  arg_type[7] = (uint8_t)IPC_TYPE_DATA;
  arg_type[8] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_sendto_cmd, arg_type, sockfd, buffer, size,
                      flags, addr, addrlen,
                      p_ret, p_ret_flag);
}

int32_t socket_sendto_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t socket_recv_cmd(int32_t sockfd, int32_t p_buf, size_t size, int32_t flags,
                        int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[7];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_DATA;
  arg_type[6] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_recv_cmd, arg_type, sockfd, p_buf, size, flags,
                      p_ret, p_ret_flag);
}

int32_t socket_recv_event(uint8_t *recv_buf, size_t size,
                          uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != recv_buf) && (size > (size_t)0))
  {
    // recv data
    _mipc_ret_copy((uint8_t *)recv_buf, (int32_t)size);

    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t socket_recvfrom_cmd(int32_t sockfd, int32_t p_buf, size_t size, int32_t flags,
                            int32_t p_fromaddr, int32_t p_fromaddr_len, int32_t fromaddr_len,
                            int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[10];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_DATA;
  arg_type[6] = (uint8_t)IPC_TYPE_DATA;
  arg_type[7] = (uint8_t)IPC_TYPE_DATA;
  arg_type[8] = (uint8_t)IPC_TYPE_DATA;
  arg_type[9] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_recvfrom_cmd, arg_type, sockfd, p_buf, size, flags,
                      p_fromaddr, p_fromaddr_len, fromaddr_len,
                      p_ret, p_ret_flag);
}

int32_t socket_recvfrom_event(uint8_t *recv_buf, size_t size, uint8_t *fromaddr, int32_t fromaddr_size,
                              uint8_t *fromaddr_len_buf, int32_t fromaddr_len_buf_size,
                              uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != recv_buf) && (size > (size_t)0) && (NULL != fromaddr) && (fromaddr_size > 0) && \
      (NULL != fromaddr_len_buf) && (fromaddr_len_buf_size > 0))
  {
    // recv data
    _mipc_ret_copy((uint8_t *)recv_buf, (int32_t)size);

    // recv from addr
    _mipc_ret_copy((uint8_t *)fromaddr, fromaddr_size);

    // recv from addr len
    _mipc_ret_copy((uint8_t *)fromaddr_len_buf, fromaddr_len_buf_size);

    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}


int32_t socket_shutdown_cmd(int32_t filedes, int32_t how, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[5];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_shutdown_cmd, arg_type, filedes, how,
                      p_ret, p_ret_flag);
}

int32_t socket_shutdown_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t socket_close_cmd(int32_t filedes, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[4];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_socket_close_cmd, arg_type, filedes,
                      p_ret, p_ret_flag);
}

int32_t socket_close_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}


/*
  * mDNS
  */
int32_t mdns_start_cmd(const char_t *domain, char_t *hostname, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[5];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_mdns_start_cmd, arg_type, domain, hostname,
                      p_ret, p_ret_flag);
}

int32_t mdns_start_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t mdns_stop_cmd(int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[3];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_mdns_stop_cmd, arg_type,
                      p_ret, p_ret_flag);
}

int32_t mdns_stop_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t mdns_announce_service_cmd(void *service, int32_t service_len, int32_t iface,
                                  int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[6];

  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_mdns_announce_service_cmd, arg_type,
                      service, service_len, iface, p_ret, p_ret_flag);
}

int32_t mdns_announce_service_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t mdns_deannounce_service_cmd(void *service, int32_t service_len, int32_t iface,
                                    int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[6];

  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_mdns_deannounce_service_cmd, arg_type,
                      service, service_len, iface, p_ret, p_ret_flag);
}

int32_t mdns_deannounce_service_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t mdns_deannounce_service_all_cmd(int32_t iface, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[4];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_mdns_deannounce_service_all_cmd, arg_type,
                      iface, p_ret, p_ret_flag);
}

int32_t mdns_deannounce_service_all_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t mdns_iface_state_change_cmd(int32_t iface, int32_t state, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[5];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_mdns_iface_state_change_cmd, arg_type,
                      iface, state, p_ret, p_ret_flag);
}

int32_t mdns_iface_state_change_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t mdns_set_hostname_cmd(void *hostname, int32_t len, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[5];

  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_mdns_set_hostname_cmd, arg_type,
                      hostname, len, p_ret, p_ret_flag);
}

int32_t mdns_set_hostname_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t mdns_set_txt_rec_cmd(void *service, int32_t service_len, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[5];

  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_mdns_set_txt_rec_cmd, arg_type,
                      service, service_len, p_ret, p_ret_flag);
}

int32_t mdns_set_txt_rec_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

/*
  * TLS
  */
int32_t tls_set_ver_cmd(int32_t version, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[4];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_tls_set_ver_cmd, arg_type,
                      version, p_ret, p_ret_flag);
}

int32_t tls_set_ver_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t tls_set_client_certificate_cmd(uint8_t *cert, int32_t len,  int32_t p_ret, int32_t p_ret_flag)
{
	  uint8_t arg_type[5];

	  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
	  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
	  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
	  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
	  arg_type[4] = (uint8_t)IPC_TYPE_NONE;

	  return ipc_api_send((uint16_t)IPC_CMD_tls_set_client_certificate_cmd, arg_type,
	                      cert, len, p_ret, p_ret_flag);
}

int32_t tls_set_client_certificate_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t tls_set_client_private_key_cmd(uint8_t *key, int32_t len,  int32_t p_ret, int32_t p_ret_flag)
{
	  uint8_t arg_type[5];

	  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
	  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
	  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
	  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
	  arg_type[4] = (uint8_t)IPC_TYPE_NONE;

	  return ipc_api_send((uint16_t)IPC_CMD_tls_set_client_private_key_cmd, arg_type,
	                      key, len, p_ret, p_ret_flag);
}

int32_t tls_set_client_private_key_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t tls_connect_cmd(int32_t domain, int32_t type, int32_t protocol,
                        const void *addr, int32_t addrlen,
                        void *ca, int32_t calen, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[10];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[6] = (uint8_t)IPC_TYPE_DATA;
  arg_type[7] = (uint8_t)IPC_TYPE_DATA;
  arg_type[8] = (uint8_t)IPC_TYPE_DATA;
  arg_type[9] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_tls_connect_cmd, arg_type,
                      domain, type, protocol, addr, addrlen, ca, calen,
                      p_ret, p_ret_flag);
}

int tls_connect_sni_cmd(const char* sni_servername, int sni_servername_len,
                    const void *addr, int32_t addrlen,
                    void* ca, int calen, int p_ret, int p_ret_flag)
{
  uint8_t arg_type[9];

  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[5] = (uint8_t)IPC_TYPE_DATA;
  arg_type[6] = (uint8_t)IPC_TYPE_DATA;
  arg_type[7] = (uint8_t)IPC_TYPE_DATA;
  arg_type[8] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_tls_connect_sni_cmd, arg_type,
                      sni_servername, sni_servername_len, addr, addrlen, ca, calen,
                      p_ret, p_ret_flag);
}

int32_t tls_connect_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t tls_send_cmd(int32_t tls, void *data, int32_t len,  int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[6];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_tls_send_cmd, arg_type,
                      tls, data, len, p_ret, p_ret_flag);
}

int32_t tls_send_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t tls_recv_cmd(int32_t tls, int32_t p_buf, int32_t len,  int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[6];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_tls_recv_cmd, arg_type,
                      tls, p_buf, len, p_ret, p_ret_flag);
}

int32_t tls_recv_event(uint8_t *recv_buf, size_t size,
                       uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != recv_buf) && (size > (size_t)0))
  {
    // recv data
    _mipc_ret_copy((uint8_t *)recv_buf, (int32_t)size);

    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t tls_close_cmd(int32_t tls, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[4];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_tls_close_cmd, arg_type,
                      tls, p_ret, p_ret_flag);
}

int32_t tls_close_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t tls_set_nonblock_cmd(int32_t tls, int32_t nonblock, int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[5];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_DATA;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_tls_set_nonblock_cmd, arg_type,
                      tls, nonblock, p_ret, p_ret_flag);
}

int32_t tls_set_nonblock_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

/*
  * Webserver
  */
int32_t web_start_cmd(int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[3];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_web_start_cmd, arg_type,
                      p_ret, p_ret_flag);
}

int32_t web_start_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t web_stop_cmd(int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[3];

  arg_type[0] = (uint8_t)IPC_TYPE_DATA;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_web_stop_cmd, arg_type,
                      p_ret, p_ret_flag);
}
int32_t web_stop_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

/*
  * FOTA
  */
int32_t fota_start_cmd(const char_t *url, int32_t url_len, const char_t *md5, int32_t md5_len,
                       int32_t p_ret, int32_t p_ret_flag)
{
  uint8_t arg_type[7];

  arg_type[0] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[1] = (uint8_t)IPC_TYPE_DATA;
  arg_type[2] = (uint8_t)IPC_TYPE_POINTER;
  arg_type[3] = (uint8_t)IPC_TYPE_DATA;
  arg_type[4] = (uint8_t)IPC_TYPE_DATA;
  arg_type[5] = (uint8_t)IPC_TYPE_DATA;
  arg_type[6] = (uint8_t)IPC_TYPE_NONE;

  return ipc_api_send((uint16_t)IPC_CMD_fota_start_cmd, arg_type,
                      url, url_len, md5, md5_len,
                      p_ret, p_ret_flag);
}

int32_t fota_start_event(uint8_t *ret_buf, int32_t ret_len, int32_t p_ret_flag)
{
  int32_t ret = IPC_RET_ERROR;
  int32_t *p_ret_flag_addr;

  if ((NULL != ret_buf) && (ret_len > 0))
  {
    // ret
    _mipc_ret_copy((uint8_t *)ret_buf, ret_len);

    ret = IPC_RET_OK;
  }

  // set flag
  (void)memcpy(&p_ret_flag_addr, &p_ret_flag, sizeof(p_ret_flag_addr));
  _mipc_ret_flag_set(p_ret_flag_addr);

  return ret;
}

int32_t fota_status_event(int32_t status)
{
  mx_wifi_fota_status_e ota_status;
  if (NULL != wifi_obj_get()->Runtime.fota_status_cb)
  {
    if (0 == status)
    {
      ota_status = MX_WIFI_FOTA_SUCCESS;
    }
    else
    {
      ota_status = MX_WIFI_FOTA_FAILED;
    }
    wifi_obj_get()->Runtime.fota_status_cb(ota_status,
                                           wifi_obj_get()->Runtime.fota_user_args);
  }

  return IPC_RET_OK;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
