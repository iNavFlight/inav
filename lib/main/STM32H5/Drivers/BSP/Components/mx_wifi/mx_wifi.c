/**
  ******************************************************************************
  * @file    mx_wifi.c
  * @author  MCD Application Team
  * @brief   Host driver API of MXCHIP Wi-Fi component.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <inttypes.h>

#include "mx_wifi.h"
#include "mx_wifi_conf.h"
#include "core/mx_wifi_ipc.h"
#include "io_pattern/mx_wifi_io.h"


#ifdef MX_WIFI_API_DEBUG
#define DEBUG_LOG(...)       (void)printf(__VA_ARGS__) /*;*/
#else
#define DEBUG_LOG(...)
#endif /* MX_WIFI_API_DEBUG */

#define DEBUG_ERROR(...)     (void)printf(__VA_ARGS__) /*;*/

/* Private defines -----------------------------------------------------------*/
static void void_strncpy(char *Destination, const char *Source, size_t Num);

/* Ensures that the last character of DEST is not overwritten, implying that it is already set to '\0'. */
#define MX_WIFI_STRNCPY(DEST, SRC)                                         \
  do                                                                         \
  {                                                                        \
    void_strncpy((char *)(DEST), (const char *)(SRC), sizeof((DEST)) - 1); \
  } while(false) /* ; */

#define MX_WIFI_MEMSET(DEST, VAL)  memset((void *)(DEST), (int)(VAL), sizeof((DEST))) /*;*/

MX_STAT_DECLARE();

#ifndef MX_WIFI_BARE_OS_H
static __IO bool RecvThreadQuitFlag;
static THREAD_DECLARE(MX_WIFI_RecvThreadId);
#endif /* MX_WIFI_BARE_OS_H */


/* Private functions ---------------------------------------------------------*/
#ifndef MX_WIFI_BARE_OS_H
static void _MX_WIFI_RecvThread(THREAD_CONTEXT_TYPE context);
#endif /* MX_WIFI_BARE_OS_H */

#if (MX_WIFI_NETWORK_BYPASS_MODE == 0)
/* Manage un-packed to packed according the given IP v4 socket address structure. */
static struct mx_sockaddr_storage mx_s_addr_in_to_packed(const struct mx_sockaddr *Addr);
static struct mx_sockaddr_in mx_s_addr_in_from_packed(const struct mx_sockaddr_storage *Addr);

/* Manage un-packed to packed according the given IP v6 socket address structure. */
static struct mx_sockaddr_storage mx_s_addr_in6_to_packed(const struct mx_sockaddr *Addr);
static struct mx_sockaddr_in6 mx_s_addr_in6_from_packed(const struct mx_sockaddr_storage *Addr);
#endif /* (MX_WIFI_NETWORK_BYPASS_MODE == 0) */

static MX_WIFI_STATUS_T _mx_wifi_set_eap_cert(uint8_t cert_type, const mx_char_t *cert, uint32_t len);


MX_WIFI_STATUS_T MX_WIFI_RegisterBusIO(MX_WIFIObject_t *Obj,
                                       IO_Init_Func IO_Init,
                                       IO_DeInit_Func IO_DeInit,
                                       IO_Delay_Func IO_Delay,
                                       IO_Send_Func IO_Send,
                                       IO_Receive_Func IO_Receive)
{
  MX_WIFI_STATUS_T rc;

  if ((NULL == Obj) || (NULL == IO_Init) || (NULL == IO_DeInit) || (NULL == IO_Send) || \
      (NULL == IO_Receive) || (NULL == IO_Delay))
  {
    rc = MX_WIFI_STATUS_ERROR;
  }
  else
  {
    Obj->fops.IO_Init = IO_Init;
    Obj->fops.IO_DeInit = IO_DeInit;
    Obj->fops.IO_Send = IO_Send;
    Obj->fops.IO_Receive = IO_Receive;
    Obj->fops.IO_Delay = IO_Delay;
    rc = MX_WIFI_STATUS_OK;
  }
  return rc;
}


MX_WIFI_STATUS_T MX_WIFI_HardResetModule(MX_WIFIObject_t *Obj)
{
  MX_WIFI_STATUS_T rc = MX_WIFI_STATUS_ERROR;

  MX_STAT_INIT();

  if (NULL != Obj)
  {
    /* reset Wi-Fi by reset pin */
    const int8_t ret = Obj->fops.IO_Init(MX_WIFI_RESET);
    if ((int8_t)0 == ret)
    {
      rc = MX_WIFI_STATUS_OK;
    }
    else
    {
      rc = MX_WIFI_STATUS_ERROR;
    }
  }

  return rc;
}


MX_WIFI_STATUS_T MX_WIFI_SetTimeout(MX_WIFIObject_t *Obj, uint32_t Timeout)
{
  MX_WIFI_STATUS_T rc = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    Obj->Runtime.Timeout = Timeout;
    rc = MX_WIFI_STATUS_OK;
  }

  return rc;
}

/*
 * run with RTOS
 */


#ifndef MX_WIFI_BARE_OS_H
/**
  * @brief                  WiFi receive thread for RTOS
  * @param  context         thread arguments
  */
static void _MX_WIFI_RecvThread(THREAD_CONTEXT_TYPE context)
{
  (void)context;

  RecvThreadQuitFlag = false;

  while (RecvThreadQuitFlag != true)
  {
    (void)MX_WIFI_IO_YIELD(wifi_obj_get(), 500);
  }

  RecvThreadQuitFlag = false;

  /* Prepare deletion (depends on implementation). */
  THREAD_TERMINATE();

  /* Delete the Thread. */
  THREAD_DEINIT(MX_WIFI_RecvThreadId);
}
#endif /* MX_WIFI_BARE_OS_H */


MX_WIFI_STATUS_T MX_WIFI_Init(MX_WIFIObject_t *Obj)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    if (Obj->Runtime.interfaces == 0u)
    {
      LOCK_INIT(Obj->lockcmd);

      /* 0. Set command timeout. */
      Obj->Runtime.Timeout = MX_WIFI_CMD_TIMEOUT;

      /* 1. Initialize the WiFi low level IO (UART/SPI). */
      (void)(Obj->fops.IO_Init(MX_WIFI_INIT));
      {
        /* 2. Initialize the WiFi IPC. */
        if (MIPC_CODE_SUCCESS == mipc_init(Obj->fops.IO_Send))
        {
          /* 2a. Start the thread for RTOS implementation. */
          if (THREAD_OK == THREAD_INIT(MX_WIFI_RecvThreadId, _MX_WIFI_RecvThread, NULL,
                                       MX_WIFI_RECEIVED_THREAD_STACK_SIZE,
                                       MX_WIFI_RECEIVED_THREAD_PRIORITY))
          {
            uint16_t rparams_size;

            /* 3. Get the version of the WiFi module firmware. */
            (void)MX_WIFI_MEMSET(Obj->SysInfo.FW_Rev, 0);
            rparams_size = (uint16_t)sizeof(Obj->SysInfo.FW_Rev);

            if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SYS_VERSION_CMD,
                                                  NULL, 0,
                                                  Obj->SysInfo.FW_Rev, &rparams_size,
                                                  MX_WIFI_CMD_TIMEOUT))
            {
              /* Check if WiFi module firmware is correctly managed by the current version of the host driver. */
              {
                uint32_t firmware_rev[3] = {0};
                const uint32_t firmware_rev_required[3] = {2, 3, 4};

                int status = sscanf((const char *)Obj->SysInfo.FW_Rev,
                                    "V%" PRIu32 ".%" PRIu32 ".%" PRIu32 "", &firmware_rev[0], &firmware_rev[1], &firmware_rev[2]);
                if (status <= 0)
                {
                  DEBUG_ERROR("ERROR: Unable to decode WiFi firmware version\n");
                  MX_ASSERT(false);
                }

                for (uint8_t i = 0; i < sizeof(firmware_rev) / sizeof(firmware_rev[0]); ++i)
                {
                  if (firmware_rev[i] > firmware_rev_required[i])
                  {
                    break;
                  }
                  else if (firmware_rev[i] < firmware_rev_required[i])
                  {
                    DEBUG_ERROR("ERROR: The WiFi firmware is out of date\n");
                    MX_ASSERT(false);
                  }
                  else
                  {
                    /* Going on with the next revision digit. */
                  }
                }
              }

              MX_WIFI_STRNCPY(Obj->SysInfo.Product_Name, MX_WIFI_PRODUCT_NAME);
              MX_WIFI_STRNCPY(Obj->SysInfo.Product_ID, MX_WIFI_PRODUCT_ID);

              /* 4. Get MAC address. */
              (void)MX_WIFI_MEMSET(Obj->SysInfo.MAC, 0);
              rparams_size = (uint16_t)sizeof(Obj->SysInfo.MAC);
              if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_GET_MAC_CMD,
                                                    NULL, 0,
                                                    Obj->SysInfo.MAC, &rparams_size,
                                                    MX_WIFI_CMD_TIMEOUT))
              {
                ret = MX_WIFI_STATUS_OK;
                Obj->Runtime.interfaces++;
              }
            }
          }
        }
      }
    }
    else
    {
      Obj->Runtime.interfaces++;
      ret = MX_WIFI_STATUS_OK;
    }
  }

  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_DeInit(MX_WIFIObject_t *Obj)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if (NULL != Obj)
  {
    if (Obj->Runtime.interfaces == 1u)
    {
#ifndef MX_WIFI_BARE_OS_H
      /* Set the thread quit flag to TRUE. */
      RecvThreadQuitFlag = true;

      /* Wait for the thread to terminate. */
      while (RecvThreadQuitFlag == true)
      {
        DELAY_MS(50);
      }
#endif /* MX_WIFI_BARE_OS_H */

      /* Delete the thread (depends on implementation). */
      THREAD_DEINIT(MX_WIFI_RecvThreadId);

      (void)mipc_deinit();
      Obj->fops.IO_DeInit();
      ret = MX_WIFI_STATUS_OK;
      Obj->Runtime.interfaces--;
    }
    else
    {
      ret = MX_WIFI_STATUS_OK;
      if (Obj->Runtime.interfaces > 0u)
      {
        Obj->Runtime.interfaces--;
      }
    }
  }

  MX_STAT_LOG();

  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_IO_YIELD(MX_WIFIObject_t *Obj, uint32_t timeout)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_OK;
  if (NULL != Obj)
  {
    mipc_poll(timeout);
  }
  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_ResetModule(MX_WIFIObject_t *Obj)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    uint16_t rparam_size = 0;
    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SYS_REBOOT_CMD, NULL, 0,
                                          NULL, &rparam_size, MX_WIFI_CMD_TIMEOUT))
    {
      ret = MX_WIFI_STATUS_OK;
    }
  }

  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_ResetToFactoryDefault(MX_WIFIObject_t *Obj)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    uint16_t rparam_size = 0;
    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SYS_RESET_CMD, NULL, 0,
                                          NULL, &rparam_size, MX_WIFI_CMD_TIMEOUT))
    {
      ret = MX_WIFI_STATUS_OK;
    }
  }

  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_GetVersion(MX_WIFIObject_t *Obj, uint8_t *version, uint32_t size)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if ((NULL != Obj) && (NULL != version) && \
      (size <= (uint32_t)MX_WIFI_FW_REV_SIZE) && (0U < size))
  {
    uint16_t rparams_size = (uint16_t)sizeof(Obj->SysInfo.FW_Rev);

    (void)MX_WIFI_MEMSET(Obj->SysInfo.FW_Rev, 0);

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SYS_VERSION_CMD, NULL, 0,
                                          &Obj->SysInfo.FW_Rev[0],
                                          &rparams_size, MX_WIFI_CMD_TIMEOUT))
    {
      (void)memcpy(version, &Obj->SysInfo.FW_Rev[0], size);
      *(version + size - 1) = 0;
      ret = MX_WIFI_STATUS_OK;
    }
  }

  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_GetMACAddress(MX_WIFIObject_t *Obj, uint8_t *Mac)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if ((NULL != Obj) && (NULL != Mac))
  {
    (void)memcpy(Mac, Obj->SysInfo.MAC, MX_WIFI_MAC_SIZE);
    ret = MX_WIFI_STATUS_OK;
  }

  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_GetsoftapMACAddress(MX_WIFIObject_t *Obj, uint8_t *Mac)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if ((NULL != Obj) && (NULL != Mac))
  {
    uint16_t rparams_size = MX_WIFI_MAC_SIZE;

    (void)memset(&Obj->SysInfo.apMAC[0], 0, MX_WIFI_MAC_SIZE);

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_GET_SOFT_MAC_CMD,
                                          NULL, 0,
                                          &Obj->SysInfo.apMAC[0],
                                          &rparams_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      (void)memcpy(Mac, &Obj->SysInfo.apMAC[0], MX_WIFI_MAC_SIZE);
      ret = MX_WIFI_STATUS_OK;
    }
  }
  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_Scan(MX_WIFIObject_t *Obj, mc_wifi_scan_mode_t ScanMode,
                              char *SSID, int32_t Len)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if ((NULL == Obj) ||
      (((mc_wifi_scan_mode_t)MC_SCAN_ACTIVE == ScanMode) && ((NULL == SSID) || (Len <= 0) || (Len > 32))))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    wifi_scan_cparams_t cparams = {0};
    const uint16_t cparams_size = (uint16_t)(sizeof(cparams));
    wifi_scan_rparams_t *rparams_p = (wifi_scan_rparams_t *)&Obj->Runtime.scan_result[0];
    uint16_t rparams_p_size = (uint16_t)sizeof(Obj->Runtime.scan_result);

    (void)memcpy(&cparams.ssid[0], SSID, (size_t)MIN(Len, (int32_t)(sizeof(cparams.ssid) - 1)));

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_SCAN_CMD,
                                          (uint8_t *)&cparams, cparams_size,
                                          (uint8_t *)rparams_p, &rparams_p_size,
                                          MX_WIFI_SCAN_TIMEOUT))
    {
      const uint8_t ap_count = rparams_p->num;
      memmove(Obj->Runtime.scan_result, &rparams_p->ap[0],
              ap_count * sizeof(mwifi_ap_info_t));

      Obj->Runtime.scan_number = ap_count;

      ret = MX_WIFI_STATUS_OK;
    }
  }

  return ret;
}


int8_t MX_WIFI_Get_scan_result(MX_WIFIObject_t *Obj, uint8_t *Results, uint8_t Number)
{
  uint8_t copy_number = 0;

  if ((NULL != Obj) && (NULL != Results) && (0U != Number))
  {
    copy_number = MIN(Obj->Runtime.scan_number, Number);
    (void)memcpy(Results, Obj->Runtime.scan_result, (size_t)copy_number * sizeof(mwifi_ap_info_t));
  }

  return (int8_t)copy_number;
}


MX_WIFI_STATUS_T MX_WIFI_RegisterStatusCallback(MX_WIFIObject_t *Obj,
                                                mx_wifi_status_callback_t Cb,
                                                void *arg)
{
  MX_WIFI_STATUS_T rc = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    Obj->Runtime.status_cb[0] = Cb;
    Obj->Runtime.callback_arg[0] = arg;
    rc = MX_WIFI_STATUS_OK;
  }

  return rc;
}


MX_WIFI_STATUS_T MX_WIFI_UnRegisterStatusCallback(MX_WIFIObject_t *Obj)
{
  MX_WIFI_STATUS_T rc = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    Obj->Runtime.status_cb[0] = NULL;
    Obj->Runtime.callback_arg[0] = NULL;
    rc = MX_WIFI_STATUS_OK;
  }

  return rc;
}


MX_WIFI_STATUS_T MX_WIFI_RegisterStatusCallback_if(MX_WIFIObject_t *Obj,
                                                   mx_wifi_status_callback_t Cb,
                                                   void *Arg,
                                                   mwifi_if_t Interface)
{
  MX_WIFI_STATUS_T rc = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    const uint8_t interface_num = ((mwifi_if_t)MC_SOFTAP == Interface) ? 1 : 0;

    Obj->Runtime.status_cb[interface_num] = Cb;
    Obj->Runtime.callback_arg[interface_num] = Arg;
    rc = MX_WIFI_STATUS_OK;
  }

  return rc;
}


MX_WIFI_STATUS_T MX_WIFI_UnRegisterStatusCallback_if(MX_WIFIObject_t *Obj, mwifi_if_t Interface)
{
  MX_WIFI_STATUS_T rc = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    const uint8_t interface_num = ((mwifi_if_t)MC_SOFTAP == Interface) ? 1 : 0;

    Obj->Runtime.status_cb[interface_num] = NULL;
    Obj->Runtime.callback_arg[interface_num] = NULL;
    rc = MX_WIFI_STATUS_OK;
  }

  return rc;
}


MX_WIFI_STATUS_T MX_WIFI_Connect(MX_WIFIObject_t *Obj, const mx_char_t *SSID,
                                 const mx_char_t *Password, MX_WIFI_SecurityType_t SecType)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  (void)SecType;

  if ((NULL != Obj) && (NULL != SSID) && (NULL != Password))
  {
    const size_t ssid_len = strlen(SSID);
    const size_t password_len = strlen(Password);

    if ((ssid_len > (uint32_t)MX_MAX_SSID_LEN) || (password_len > (uint32_t)MX_MAX_KEY_LEN))
    {
      ret = MX_WIFI_STATUS_PARAM_ERROR;
    }
    else
    {
      wifi_connect_cparams_t cp = {0};
      const uint16_t cp_size = (uint16_t)(sizeof(cp));
      int32_t status = MIPC_CODE_ERROR;
      uint16_t status_size = (uint16_t)sizeof(status);

      MX_WIFI_STRNCPY(cp.ssid, SSID);
      MX_WIFI_STRNCPY(cp.key, Password);
      cp.key_len = (int32_t)password_len;

      if ((uint8_t)0 == Obj->NetSettings.DHCP_IsEnabled)
      {
        mwifi_ip_attr_t ip_attr = {0};
        {
          mx_ip_addr_t ip_addr = {0};
          (void)memcpy(&ip_addr, Obj->NetSettings.IP_Addr, sizeof(ip_addr));
          MX_WIFI_STRNCPY(ip_attr.localip, mx_ntoa(&ip_addr));
        }
        {
          mx_ip_addr_t ip_mask = {0};
          (void)memcpy(&ip_mask, Obj->NetSettings.IP_Mask, sizeof(ip_mask));
          MX_WIFI_STRNCPY(ip_attr.netmask, mx_ntoa(&ip_mask));
        }
        {
          mx_ip_addr_t gateway_ip_addr = {0};
          (void)memcpy(&gateway_ip_addr, Obj->NetSettings.Gateway_Addr, sizeof(gateway_ip_addr));
          MX_WIFI_STRNCPY(ip_attr.gateway, mx_ntoa(&gateway_ip_addr));
        }
        {
          mx_ip_addr_t dns_ip_addr = {0};
          (void)memcpy(&dns_ip_addr, Obj->NetSettings.DNS1, sizeof(dns_ip_addr));
          MX_WIFI_STRNCPY(ip_attr.dnserver, mx_ntoa(&dns_ip_addr));
        }

        cp.use_ip = 1;
        cp.ip = ip_attr;
      }

      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_CONNECT_CMD,
                                            (uint8_t *)&cp, cp_size,
                                            (uint8_t *)&status, &status_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        if (MIPC_CODE_SUCCESS == status)
        {
          ret = MX_WIFI_STATUS_OK;
        }
      }
    }
  }

  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_Connect_Adv(MX_WIFIObject_t *Obj, const mx_char_t *SSID, const mx_char_t *Password,
                                     mwifi_connect_attr_t *Attr, mwifi_ip_attr_t *IP)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if ((NULL != Obj) && (NULL != SSID) && (NULL != Password))
  {
    const size_t ssid_len = strlen(SSID);
    const size_t password_len = strlen(Password);

    if ((ssid_len > (uint32_t)MX_MAX_SSID_LEN) || (password_len > (uint32_t)MX_MAX_KEY_LEN))
    {
      ret = MX_WIFI_STATUS_PARAM_ERROR;
    }
    else
    {
      wifi_connect_cparams_t cp = {0};
      const uint16_t cp_size = (uint16_t)(sizeof(cp));
      int32_t status = MIPC_CODE_ERROR;
      uint16_t status_size = (uint16_t)sizeof(status);

      MX_WIFI_STRNCPY(cp.ssid, SSID);
      MX_WIFI_STRNCPY(cp.key, Password);
      cp.key_len = (int32_t)password_len;

      if (Attr != NULL)
      {
        cp.use_attr = 1;
        (void)memcpy(&cp.attr.bssid, &Attr->bssid, sizeof(cp.attr.bssid));
        cp.attr.channel = Attr->channel;
        cp.attr.security = Attr->security;
      }

      if (IP != NULL)
      {
        cp.use_ip = 1;
        cp.ip = *IP;
      }

      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_CONNECT_CMD,
                                            (uint8_t *)&cp, cp_size,
                                            (uint8_t *)&status, &status_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        if (MIPC_CODE_SUCCESS == status)
        {
          ret = MX_WIFI_STATUS_OK;
        }
      }

    }
  }

  return ret;
}


/**
  * @brief  Set EAP certificates for WAP-E connect.
  * @param  cert_type: cert type, EAP_ROOTCA/EAP_CLIENT_CERT/EAP_CLIENT_KEY
  * @param  cert: cert string
  * @param  len: size of the cert string
  * @retval operation status, error code @ref MX_WIFI_STATUS_T
  */
static MX_WIFI_STATUS_T _mx_wifi_set_eap_cert(uint8_t cert_type, const mx_char_t *cert, uint32_t len)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  (void)len;

  if (NULL != cert)
  {
    const uint16_t cert_len = (uint16_t)strlen(cert);
    const uint16_t set_cert_cp_size = (uint16_t)sizeof(wifi_eap_set_cert_cparams_t) + cert_len; /* len + 1 */

    wifi_eap_set_cert_cparams_t *set_cert_cp = (wifi_eap_set_cert_cparams_t *)MX_WIFI_MALLOC(set_cert_cp_size);

    if (NULL != set_cert_cp)
    {
      int32_t status = MIPC_CODE_ERROR;
      uint16_t status_size = (uint16_t)sizeof(status);

      (void)memset(set_cert_cp, 0, set_cert_cp_size);
      set_cert_cp->type = cert_type;
      set_cert_cp->len = cert_len;
      (void)memcpy(set_cert_cp->cert, cert, cert_len);

      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_EAP_SET_CERT_CMD,
                                            (uint8_t *)set_cert_cp, set_cert_cp_size,
                                            (uint8_t *)&status, &status_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        if (MIPC_CODE_SUCCESS == status)
        {
          ret = MX_WIFI_STATUS_OK;
        }
      }

      MX_WIFI_FREE(set_cert_cp);
    }
  }

  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_EAP_Connect(MX_WIFIObject_t *Obj, const char *SSID,
                                     const char *Identity, const char *Password,
                                     mwifi_eap_attr_t *Attr, mwifi_ip_attr_t *IP)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (NULL != SSID) && (NULL != Identity) && (NULL != Password))
  {
    const size_t ssid_len = strlen(SSID);
    const size_t identity_len = strlen(Identity);
    const size_t password_len = strlen(Password);

    if ((ssid_len > (uint32_t)MX_MAX_SSID_LEN) || (identity_len > (uint32_t)MX_MAX_IDENTITY_LEN) || \
        (password_len > (uint32_t)MX_MAX_KEY_LEN))
    {
      ret = MX_WIFI_STATUS_PARAM_ERROR;
    }
    else
    {
      wifi_eap_connect_cparams_t connect_cp = {0};
      const uint16_t connect_cp_size = (uint16_t)(sizeof(connect_cp));

      ret = MX_WIFI_STATUS_OK;  /* ret set ERROR if any step error */

      /* Set eap_attr. */
      if (NULL != Attr)
      {
        /* Just set EAP type here, rootca / cert / key set by specifical API */
        if (((uint8_t)EAP_TYPE_TLS != Attr->eap_type) && ((uint8_t)EAP_TYPE_TTLS != Attr->eap_type) &&
            ((uint8_t)EAP_TYPE_PEAP != Attr->eap_type))
        {
          ret = MX_WIFI_STATUS_PARAM_ERROR;
        }
        else
        {
          connect_cp.attr.eap_type = Attr->eap_type;

          if (NULL != Attr->rootca)
          {
            ret = _mx_wifi_set_eap_cert((uint8_t)EAP_ROOTCA, Attr->rootca, strlen(Attr->rootca));
            if (MX_WIFI_STATUS_OK == ret)
            {
              connect_cp.attr_used = 1;
            }
          }

          if ((MX_WIFI_STATUS_OK == ret) && (NULL != Attr->client_cert))
          {
            ret = _mx_wifi_set_eap_cert((uint8_t)EAP_CLIENT_CERT, Attr->client_cert, strlen(Attr->client_cert));
            if (MX_WIFI_STATUS_OK == ret)
            {
              connect_cp.attr_used = 1;
            }
          }

          if ((MX_WIFI_STATUS_OK == ret) && (NULL != Attr->client_key))
          {
            ret = _mx_wifi_set_eap_cert((uint8_t)EAP_CLIENT_KEY, Attr->client_key, strlen(Attr->client_key));
            if (MX_WIFI_STATUS_OK == ret)
            {
              connect_cp.attr_used = 1;
            }
          }
        }
      }
      else
      {
        connect_cp.attr.eap_type = (uint8_t)EAP_TYPE_PEAP;  /* default eap type */
      }

      if (MX_WIFI_STATUS_OK == ret)
      {
        int32_t status = MIPC_CODE_ERROR;
        uint16_t status_size = (uint16_t)sizeof(status);

        /* Set EAP IP. */
        if (NULL != IP)
        {
          connect_cp.ip = *IP;
          connect_cp.ip_used = 1;  /* IP set */
        }

        /* Start EAP connect. */
        ret = MX_WIFI_STATUS_ERROR;
        MX_WIFI_STRNCPY(connect_cp.ssid, SSID);
        MX_WIFI_STRNCPY(connect_cp.identity, Identity);
        MX_WIFI_STRNCPY(connect_cp.password, Password);

        if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_EAP_CONNECT_CMD,
                                              (uint8_t *)&connect_cp, connect_cp_size,
                                              (uint8_t *)&status, &status_size,
                                              MX_WIFI_CMD_TIMEOUT))
        {
          if (MIPC_CODE_SUCCESS == status)
          {
            ret = MX_WIFI_STATUS_OK;
          }
        }
      }
    }
  }

  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_Disconnect(MX_WIFIObject_t *Obj)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);
    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_DISCONNECT_CMD, NULL, 0,
                                          (uint8_t *)&status, &status_size,
                                          15000))  /* disconnect max timeout 15s */
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_WPS_Connect(MX_WIFIObject_t *Obj)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_WPS_CONNECT_CMD, NULL, 0,
                                          (uint8_t *)&status, &status_size,
                                          15000))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_WPS_Stop(MX_WIFIObject_t *Obj)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);
    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_WPS_STOP_CMD, NULL, 0,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return ret;
}


int8_t MX_WIFI_IsConnected(MX_WIFIObject_t *Obj)
{
  int8_t ret = 0;

  if (NULL != Obj)
  {
    wifi_get_linkinof_rparams_t rparams = {0};
    uint16_t rparams_size = (uint16_t)sizeof(rparams);

    rparams.status = MIPC_CODE_ERROR;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_GET_LINKINFO_CMD, NULL, 0,
                                          (uint8_t *)&rparams, &rparams_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == rparams.status)
      {
        Obj->NetSettings.IsConnected = (int8_t)rparams.info.is_connected;
        if (Obj->NetSettings.IsConnected > 0)
        {
          ret = 1;
        }
      }
    }
  }

  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_GetIPAddress(MX_WIFIObject_t *Obj, uint8_t *IpAddr, mwifi_if_t WifiMode)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    wifi_get_ip_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);
    uint8_t interface_num = ((mwifi_if_t)MC_SOFTAP == WifiMode) ? 0 : 1;
    const uint16_t interface_num_size = (uint16_t)(sizeof(interface_num));

    rp.status = MIPC_CODE_ERROR;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_GET_IP_CMD,
                                          &interface_num, interface_num_size,
                                          (uint8_t *)&rp, &rp_size,
                                          1000))
    {
      if (MIPC_CODE_SUCCESS == rp.status)
      {
        {
          int32_t ip = mx_aton_r(&rp.ip.localip[0]);
          (void)memcpy(&Obj->NetSettings.IP_Addr[0], &ip, sizeof(Obj->NetSettings.IP_Addr));
        }
        {
          int32_t netmask = mx_aton_r(&rp.ip.netmask[0]);
          (void)memcpy(&Obj->NetSettings.IP_Mask[0], &netmask, sizeof(Obj->NetSettings.IP_Mask));
        }
        {
          int32_t gw = mx_aton_r(&rp.ip.gateway[0]);
          (void)memcpy(&Obj->NetSettings.Gateway_Addr[0], &gw, sizeof(Obj->NetSettings.Gateway_Addr));
        }
        {
          int32_t dns = mx_aton_r(&rp.ip.dnserver[0]);
          (void)memcpy(&Obj->NetSettings.DNS1[0], &dns, sizeof(Obj->NetSettings.DNS1));
        }
        (void)memcpy(IpAddr, Obj->NetSettings.IP_Addr, sizeof(Obj->NetSettings.IP_Addr));
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_GetIP6Address(MX_WIFIObject_t *Obj, uint8_t *IpAddr6, int32_t AddrSlot, mwifi_if_t WifiMode)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if ((NULL != Obj) && (NULL != IpAddr6) && (AddrSlot < 3) && (AddrSlot >= 0))
  {
    wifi_get_ip6_addr_cprams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    wifi_get_ip6_addr_rprams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    cp.addr_num = (uint8_t)AddrSlot;
    cp.iface = WifiMode;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_GET_IP6_ADDR_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          1000))
    {
      if (rp.status == MIPC_CODE_SUCCESS)
      {
        (void)memcpy(&Obj->NetSettings.IP6_Addr[AddrSlot][0], rp.ip6, sizeof(Obj->NetSettings.IP6_Addr[AddrSlot][0]));
        (void)memcpy(IpAddr6, rp.ip6, sizeof(rp.ip6));
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return ret;
}


int32_t MX_WIFI_GetIP6AddressState(MX_WIFIObject_t *Obj, int32_t AddrSlot, mwifi_if_t WifiMode)
{
  int32_t state = (int32_t)MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    wifi_get_ip6_state_cprams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    wifi_get_ip6_state_rprams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    cp.addr_num = (uint8_t)AddrSlot;
    cp.iface = WifiMode;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_GET_IP6_STATE_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      state = (int32_t)rp.state;
    }
  }

  return state;
}


/**
  * SoftAP
  */


MX_WIFI_STATUS_T MX_WIFI_StartAP(MX_WIFIObject_t *Obj, MX_WIFI_APSettings_t *ApSettings)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if ((NULL != Obj) && (NULL != ApSettings))
  {
    wifi_softap_start_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    (void)memcpy((char *)cp.ssid, ApSettings->SSID, sizeof(cp.ssid));
    (void)memcpy((char *)cp.key, ApSettings->pswd, sizeof(cp.key));
    cp.channel = (int32_t)ApSettings->channel;
    (void)memcpy(&cp.ip, &ApSettings->ip, sizeof(cp.ip));

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_SOFTAP_START_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&status, &status_size,
                                          3000))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_StopAP(MX_WIFIObject_t *Obj)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_SOFTAP_STOP_CMD, NULL, 0,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return ret;
}


#if (MX_WIFI_NETWORK_BYPASS_MODE == 0)

int32_t MX_WIFI_Socket_create(MX_WIFIObject_t *Obj, int32_t Domain, int32_t Type, int32_t Protocol)
{
  int32_t ret_fd = -1;

  if (NULL != Obj)
  {
    socket_create_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    socket_create_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    cp.domain = Domain;
    cp.type = Type;
    cp.protocol = Protocol;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_CREATE_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      ret_fd = rp.fd;
    }
  }

  return ret_fd;
}


int32_t MX_WIFI_Socket_setsockopt(MX_WIFIObject_t *Obj, int32_t SockFd, int32_t Level,
                                  int32_t OptName, const void *OptValue, int32_t OptLen)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if ((NULL != Obj) && (0 <= SockFd) && (NULL != OptValue) && (0 < OptLen))
  {
    socket_setsockopt_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    socket_setsockopt_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    rp.status = MIPC_CODE_ERROR;

    cp.socket = SockFd;
    cp.level = Level;
    cp.optname = OptName;
    cp.optlen = OptLen > (int32_t)sizeof(cp.optval) ? (mx_socklen_t)sizeof(cp.optval) : (mx_socklen_t)OptLen;

    (void)memcpy(&cp.optval[0], OptValue, cp.optlen);

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_SETSOCKOPT_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (rp.status == MIPC_CODE_SUCCESS)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_Socket_getsockopt(MX_WIFIObject_t *Obj, int32_t SockFd, int32_t Level,
                                  int32_t OptName, void *OptValue, uint32_t *OptLen)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if ((NULL != Obj) && (0 <= SockFd) && (NULL != OptValue) && (NULL != OptLen))
  {
    socket_getsockopt_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    socket_getsockopt_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    rp.status = MIPC_CODE_ERROR;

    cp.socket = SockFd;
    cp.level = Level;
    cp.optname = OptName;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_GETSOCKOPT_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (rp.status == MIPC_CODE_SUCCESS)
      {
        *OptLen = (rp.optlen > *OptLen) ? *OptLen : rp.optlen;
        (void)memcpy(OptValue, &rp.optval[0], *OptLen);
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_Socket_bind(MX_WIFIObject_t *Obj, int32_t SockFd,
                            const struct mx_sockaddr *Addr, int32_t AddrLen)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (0 <= SockFd) && (NULL != Addr) && (0 < AddrLen))
  {
    socket_bind_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    bool is_to_do_mipc_request = true;

    ret = MX_WIFI_STATUS_ERROR;

    if ((Addr->sa_family == MX_AF_INET) && (AddrLen == sizeof(struct mx_sockaddr_in)))
    {
      cp.addr = mx_s_addr_in_to_packed(Addr);
    }
    else if ((Addr->sa_family == MX_AF_INET6) && (AddrLen == sizeof(struct mx_sockaddr_in6)))
    {
      cp.addr = mx_s_addr_in6_to_packed(Addr);
    }
    else
    {
      is_to_do_mipc_request = false;
    }

    if (is_to_do_mipc_request)
    {
      socket_bind_rparams_t rp = {0};
      uint16_t rp_size = (uint16_t)sizeof(rp);

      rp.status =  MIPC_CODE_ERROR;

      cp.socket = SockFd;
      cp.length = (mx_socklen_t)AddrLen;

      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_BIND_CMD,
                                            (uint8_t *)&cp, cp_size,
                                            (uint8_t *)&rp, &rp_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        if (rp.status == MIPC_CODE_SUCCESS)
        {
          ret = MX_WIFI_STATUS_OK;
        }
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_Socket_listen(MX_WIFIObject_t *Obj, int32_t sockfd, int32_t backlog)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if ((NULL == Obj) || (sockfd < 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    socket_listen_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    socket_listen_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    rp.status =  MIPC_CODE_ERROR;

    cp.socket = sockfd;
    cp.backlog = backlog;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_LISTEN_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (rp.status == MIPC_CODE_SUCCESS)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_Socket_accept(MX_WIFIObject_t *Obj, int32_t SockFd,
                              struct mx_sockaddr *Addr, uint32_t *AddrLen)
{
  int32_t ret_fd = (int32_t)MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (0 <= SockFd) && (NULL != Addr) && (NULL != AddrLen))
  {
    socket_accept_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    socket_accept_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    rp.socket = -1;
    ret_fd = -1;
    cp.socket = SockFd;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_ACCEPT_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (rp.socket >= 0)
      {
        ret_fd = rp.socket;

        if ((rp.addr.ss_family == MX_AF_INET) && (rp.addr.s2_len == 16) && (*AddrLen == sizeof(struct mx_sockaddr_in)))
        {
          *((struct mx_sockaddr_in *)((void *)Addr)) = mx_s_addr_in_from_packed(&rp.addr);
        }
        else if ((rp.addr.ss_family == MX_AF_INET6) && (rp.addr.s2_len == sizeof(struct mx_sockaddr_storage)) && \
                 (*AddrLen == sizeof(struct mx_sockaddr_in6)))
        {
          *((struct mx_sockaddr_in6 *)((void *)Addr)) = mx_s_addr_in6_from_packed(&rp.addr);
        }
        else
        {
          ret_fd = -1;
        }
      }
    }
  }

  return ret_fd;
}


int32_t MX_WIFI_Socket_connect(MX_WIFIObject_t *Obj, int32_t SockFd,
                               const struct mx_sockaddr *Addr, int32_t AddrLen)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (0 <= SockFd) && (NULL != Addr) && (0 < AddrLen))
  {
    socket_connect_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    bool is_to_do_mipc_request = true;

    if ((Addr->sa_family == MX_AF_INET) && (AddrLen == sizeof(struct mx_sockaddr_in)))
    {
      cp.addr = mx_s_addr_in_to_packed(Addr);
    }
    else if ((Addr->sa_family == MX_AF_INET6) && (AddrLen == sizeof(struct mx_sockaddr_in6)))
    {
      cp.addr = mx_s_addr_in6_to_packed(Addr);
    }
    else
    {
      is_to_do_mipc_request = false;
    }

    if (is_to_do_mipc_request)
    {
      socket_connect_rparams_t rp = {0};
      uint16_t rp_size = (uint16_t)sizeof(rp);

      rp.status = MIPC_CODE_ERROR;

      cp.socket = SockFd;
      cp.length = (mx_socklen_t)AddrLen;

      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_CONNECT_CMD,
                                            (uint8_t *)&cp, cp_size,
                                            (uint8_t *)&rp, &rp_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        if (rp.status == MIPC_CODE_SUCCESS)
        {
          ret = MX_WIFI_STATUS_OK;
        }
        else
        {
          DEBUG_LOG("%-15s(): %" PRIi32 "\n", __FUNCTION__, (int32_t)rp.status);
        }
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_Socket_shutdown(MX_WIFIObject_t *Obj, int32_t SockFd, int32_t Mode)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (0 <= SockFd))
  {
    socket_shutdown_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    socket_shutdown_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    rp.status = MIPC_CODE_ERROR;
    cp.filedes = SockFd;
    cp.how = Mode;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_SHUTDOWN_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (rp.status == MIPC_CODE_SUCCESS)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_Socket_close(MX_WIFIObject_t *Obj, int32_t SockFd)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (0 <= SockFd))
  {
    socket_close_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    socket_close_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    rp.status = MIPC_CODE_ERROR;
    cp.filedes = SockFd;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_CLOSE_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (rp.status == MIPC_CODE_SUCCESS)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_Socket_send(MX_WIFIObject_t *Obj, int32_t SockFd, const uint8_t *Buf,
                            int32_t Len, int32_t flags)
{
  int32_t ret = (int32_t)MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (0 <= SockFd) && (NULL != Buf) && (0 < Len))
  {
    socket_send_cparams_t *cp = NULL;
    socket_send_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);
    size_t data_len = (size_t)Len;

    ret = (int32_t)MX_WIFI_STATUS_ERROR;

    if ((data_len + sizeof(socket_send_cparams_t) - 1) > MX_WIFI_IPC_PAYLOAD_SIZE)
    {
      /* Restrict to the length which corresponds to the maximum size of the IPC transfer. */
      data_len = MX_WIFI_IPC_PAYLOAD_SIZE - (sizeof(socket_send_cparams_t) - 1);
    }

    /* useless: rp.sent = 0; */

    const uint16_t cp_size = (uint16_t)(sizeof(socket_send_cparams_t) - 1 + data_len);
    cp = (socket_send_cparams_t *)MX_WIFI_MALLOC(cp_size);
    if (NULL != cp)
    {
      cp->socket = SockFd;
      (void)memcpy(&cp->buffer[0], Buf, data_len);
      cp->size = data_len;
      cp->flags = flags;
      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_SEND_CMD,
                                            (uint8_t *)cp, cp_size,
                                            (uint8_t *)&rp, &rp_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        ret = rp.sent;
      }
      MX_WIFI_FREE(cp);
    }
  }

  return ret;
}


int32_t MX_WIFI_Socket_sendto(MX_WIFIObject_t *Obj, int32_t SockFd, const uint8_t *Buf,
                              int32_t Len, int32_t Flags,
                              struct mx_sockaddr *ToAddr, int32_t ToAddrLen)
{
  int32_t ret = (int32_t)MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (0 <= SockFd) && (NULL != Buf) && (0 < Len) && (NULL != ToAddr) && (0 < ToAddrLen))
  {
    socket_sendto_cparams_t *cp = NULL;
    size_t data_len = (size_t)Len;

    ret = (int32_t)MX_WIFI_STATUS_ERROR;

    if ((data_len + sizeof(socket_sendto_cparams_t) - 1) > MX_WIFI_IPC_PAYLOAD_SIZE)
    {
      /* Restrict to the length which corresponds to the maximum size of the IPC transfer. */
      data_len = MX_WIFI_IPC_PAYLOAD_SIZE - (sizeof(socket_sendto_cparams_t) - 1);
    }

    const uint16_t cp_size = (uint16_t)(sizeof(socket_sendto_cparams_t) - 1 + data_len);

    cp = (socket_sendto_cparams_t *)MX_WIFI_MALLOC(cp_size);

    if (NULL != cp)
    {
      bool is_to_do_mipc_request = true;
      socket_sendto_rparams_t rp = {0};
      uint16_t rp_size = (uint16_t)sizeof(rp);

      /* useless: rp.sent = 0; */
      cp->socket = SockFd;
      (void)memcpy(&cp->buffer[0], Buf, data_len);
      cp->size = data_len;
      cp->flags = Flags;

      if ((ToAddr->sa_family == MX_AF_INET) && (ToAddrLen == sizeof(struct mx_sockaddr_in)))
      {
        cp->addr = mx_s_addr_in_to_packed(ToAddr);
      }
      else if ((ToAddr->sa_family == MX_AF_INET6) && (ToAddrLen == sizeof(struct mx_sockaddr_in6)))
      {
        cp->addr = mx_s_addr_in6_to_packed(ToAddr);
      }
      else
      {
        is_to_do_mipc_request = false;
      }

      if (is_to_do_mipc_request)
      {
        cp->length = (mx_socklen_t)ToAddrLen;

        if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_SENDTO_CMD,
                                              (uint8_t *)cp, cp_size,
                                              (uint8_t *)&rp, &rp_size,
                                              MX_WIFI_CMD_TIMEOUT))
        {
          ret = rp.sent;
        }
      }
      MX_WIFI_FREE(cp);
    }
  }

  return ret;
}


int32_t MX_WIFI_Socket_recv(MX_WIFIObject_t *Obj, int32_t SockFd, uint8_t *Buf,
                            int32_t Len, int32_t flags)
{
  int32_t ret = (int32_t)MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (0 <= SockFd) && (NULL != Buf) && (0 < Len))
  {
    socket_recv_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    socket_recv_rparams_t *rp = NULL;
    size_t data_len = (size_t)Len;
    uint16_t rp_size;

    ret = (int32_t)MX_WIFI_STATUS_ERROR;

    if ((data_len + sizeof(socket_recv_rparams_t) - 1) > MX_WIFI_IPC_PAYLOAD_SIZE)
    {
      /* Restrict to the length which corresponds to the maximum size of the IPC transfer. */
      data_len = MX_WIFI_IPC_PAYLOAD_SIZE - (sizeof(socket_recv_rparams_t) - 1);
    }

    rp_size = (uint16_t)(sizeof(socket_recv_rparams_t) - 1 + data_len);
    rp = (socket_recv_rparams_t *)MX_WIFI_MALLOC(rp_size);
    if (NULL != rp)
    {
      rp->received = 0;
      cp.socket = SockFd;
      cp.size = data_len;
      cp.flags = flags;
      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_RECV_CMD,
                                            (uint8_t *)&cp, cp_size,
                                            (uint8_t *)rp, &rp_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        if (rp->received > 0)
        {
          const size_t received_len = (size_t)rp->received;
          if (received_len <= data_len)
          {
            (void)memcpy(Buf, &rp->buffer[0], received_len);
          }
        }
        ret = rp->received;
      }
      MX_WIFI_FREE(rp);
    }
  }

  return ret;
}


int32_t MX_WIFI_Socket_recvfrom(MX_WIFIObject_t *Obj, int32_t SockFd, uint8_t *Buf,
                                int32_t Len, int32_t Flags,
                                struct mx_sockaddr *FromAddr, uint32_t *FromAddrLen)
{
  int32_t ret = (int32_t)MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (0 <= SockFd) && (NULL != Buf) && (0 < Len) && (NULL != FromAddr) && (NULL != FromAddrLen))
  {
    socket_recvfrom_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    socket_recvfrom_rparams_t *rp = NULL;
    size_t data_len = (size_t)Len;

    ret = (int32_t)MX_WIFI_STATUS_OK;

    if ((data_len + sizeof(socket_recvfrom_rparams_t) - 1) > MX_WIFI_IPC_PAYLOAD_SIZE)
    {
      /* Restrict to the length which corresponds to the maximum size of the IPC transfer. */
      data_len = MX_WIFI_IPC_PAYLOAD_SIZE - (sizeof(socket_recvfrom_rparams_t) - 1);
    }

    uint16_t rp_size = (uint16_t)(sizeof(socket_recvfrom_rparams_t) - 1 + data_len);

    rp = (socket_recvfrom_rparams_t *)MX_WIFI_MALLOC(rp_size);

    if (NULL != rp)
    {
      rp->received = 0;
      cp.socket = SockFd;
      cp.size = data_len;
      cp.flags = Flags;
      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_RECVFROM_CMD,
                                            (uint8_t *)&cp, cp_size,
                                            (uint8_t *)rp, &rp_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        if (rp->received > 0)
        {
          const size_t received_len = (size_t)rp->received;

          if (received_len <= data_len)
          {
            const int32_t buf_size = MIN(Len, rp->received);
            const size_t rp_addr_size = MIN(sizeof(rp->addr), *FromAddrLen);

            (void)memcpy(Buf, rp->buffer, (size_t)buf_size);

            if ((rp->addr.ss_family == MX_AF_INET) && (rp->addr.s2_len == 16) && (*FromAddrLen == sizeof(struct mx_sockaddr_in)))
            {
              *((struct mx_sockaddr_in *)((void *)FromAddr)) = mx_s_addr_in_from_packed(&rp->addr);
            }
            else if ((rp->addr.ss_family == MX_AF_INET6) && (rp->addr.s2_len == sizeof(struct mx_sockaddr_storage)) && \
                     (*FromAddrLen == sizeof(struct mx_sockaddr_in6)))
            {
              *((struct mx_sockaddr_in6 *)((void *)FromAddr)) = mx_s_addr_in6_from_packed(&rp->addr);
            }

            *FromAddrLen = rp_addr_size;
            ret = buf_size;
          }
        }
      }
      MX_WIFI_FREE(rp);
    }
  }

  return ret;
}


int32_t MX_WIFI_Socket_gethostbyname(MX_WIFIObject_t *Obj, struct mx_sockaddr *Addr, const mx_char_t *Name)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (NULL != Addr) && (NULL != Name) && (strlen(Name) < (size_t)MX_MAX_DNSNAME_LEN))
  {
    socket_gethostbyname_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    socket_gethostbyname_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    const size_t msize = MIN(sizeof(cp.name), strlen(Name) + 1);

    ret = MX_WIFI_STATUS_ERROR;
    rp.status =  MIPC_CODE_ERROR;

    (void)memcpy(&cp.name[0], Name, msize);

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_GETHOSTBYNAME_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (rp.status == MIPC_CODE_SUCCESS)
      {
        /* Only for IPv4 address. */
        ((mx_sockaddr_in_t *)Addr)->sin_family = MX_AF_INET;
        ((mx_sockaddr_in_t *)Addr)->sin_addr.s_addr = rp.s_addr;
        ((mx_sockaddr_in_t *)Addr)->sin_len = sizeof(mx_sockaddr_in_t);
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_Socket_ping(MX_WIFIObject_t *Obj, const char *hostname,
                            int32_t count, int32_t delay, int32_t response[])
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (NULL != hostname) && (0 < count))
  {
    wifi_ping_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    int32_t ping_resp[1 + MX_WIFI_PING_MAX] = {0};
    wifi_ping_rparams_t *rp = NULL;
    uint16_t rp_size = sizeof(ping_resp);

    ret = MX_WIFI_STATUS_ERROR;

    MX_WIFI_STRNCPY(cp.hostname, hostname);
    cp.count = count;
    cp.delay_ms = delay;

    rp = (wifi_ping_rparams_t *)&ping_resp;
    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_PING_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (rp->num > 0)
      {
        for (int32_t i = 0; i < rp->num; i++)
        {
          response[i] = rp->delay_ms[i];
        }
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_Socket_ping6(MX_WIFIObject_t *Obj, const mx_char_t *hostname,
                             int32_t count, int32_t delay, int32_t response[])
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (NULL != hostname) && (0 < count))
  {
    wifi_ping_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    int32_t ping_resp[1 + MX_WIFI_PING_MAX] = {0};
    wifi_ping_rparams_t *rp = NULL;
    uint16_t rp_size = (uint16_t)sizeof(ping_resp);

    ret = MX_WIFI_STATUS_ERROR;

    MX_WIFI_STRNCPY(cp.hostname, hostname);
    cp.count = count;
    cp.delay_ms = delay;

    rp = (wifi_ping_rparams_t *)&ping_resp;
    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_PING6_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (rp->num > 0)
      {
        for (int32_t i = 0; i < rp->num; i++)
        {
          response[i] = rp->delay_ms[i];
        }
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_Socket_getaddrinfo(MX_WIFIObject_t *Obj, const char *NodeName, const char *ServerName,
                                   const struct mx_addrinfo *Hints, struct mx_addrinfo *Res)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (NULL != NodeName) && (NULL != Hints) && (NULL != Res))
  {
    socket_getaddrinfo_cparam_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    socket_getaddrinfo_rparam_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    ret = MX_WIFI_STATUS_ERROR;
    MX_WIFI_STRNCPY(cp.nodename, NodeName);

    if (NULL != ServerName)
    {
      MX_WIFI_STRNCPY(cp.servname, ServerName);
    }
    cp.hints = *Hints;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_GETADDRINFO_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (rp.status == MIPC_CODE_SUCCESS)
      {
        Res->ai_flags = rp.res.ai_flags;
        Res->ai_family = rp.res.ai_family;
        Res->ai_socktype = rp.res.ai_socktype;
        Res->ai_protocol = rp.res.ai_protocol;
        Res->ai_addrlen = rp.res.ai_addrlen;
        Res->ai_addr.s2_len = rp.res.ai_addr.s2_len;
        Res->ai_addr.ss_family = rp.res.ai_addr.ss_family;
        /* Keep port possibly set as it is. */
        Res->ai_addr.s2_data2[0] = rp.res.ai_addr.s2_data2[0];
        Res->ai_addr.s2_data2[1] = rp.res.ai_addr.s2_data2[1];
        Res->ai_addr.s2_data2[2] = rp.res.ai_addr.s2_data2[2];
        Res->ai_addr.s2_data3[0] = rp.res.ai_addr.s2_data3[0];
        Res->ai_addr.s2_data3[1] = rp.res.ai_addr.s2_data3[1];
        Res->ai_addr.s2_data3[2] = rp.res.ai_addr.s2_data3[2];
        (void)memcpy(Res->ai_canonname, rp.res.ai_canonname, sizeof(Res->ai_canonname));

        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_Socket_select(MX_WIFIObject_t *Obj, int32_t nfds,
                              mx_fd_set *readfds, mx_fd_set *writefds,
                              mx_fd_set *exceptfds,
                              struct mx_timeval *timeout)
{
  int32_t ret = (int32_t)MX_WIFI_STATUS_ERROR; /* error */
  (void)Obj;
  (void)nfds;
  (void)readfds;
  (void)writefds;
  (void)exceptfds;
  (void)timeout;

  return ret;
}


int32_t MX_WIFI_Socket_getpeername(MX_WIFIObject_t *Obj, int32_t SockFd, struct mx_sockaddr *Addr, uint32_t *AddrLen)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (0 <= SockFd) && (NULL != Addr) && (NULL != AddrLen) && (0 < *AddrLen))
  {
    socket_getpeername_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    socket_getpeername_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    ret = MX_WIFI_STATUS_ERROR;
    cp.sockfd = SockFd;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_GETPEERNAME_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == rp.status)
      {
        const size_t msize = MIN(rp.namelen, *AddrLen);

        (void)memcpy(Addr, &rp.name, msize);
        *AddrLen = msize;
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_Socket_getsockname(MX_WIFIObject_t *Obj, int32_t SockFd, struct mx_sockaddr *Addr, uint32_t *AddrLen)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (0 <= SockFd) && (NULL != Addr) && (NULL != AddrLen) && (0 < *AddrLen))
  {
    socket_getsockname_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    socket_getsockname_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    ret = MX_WIFI_STATUS_ERROR;
    rp.status = MIPC_CODE_ERROR;
    cp.sockfd = SockFd;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SOCKET_GETSOCKNAME_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == rp.status)
      {
        const size_t msize = MIN(rp.namelen, *AddrLen);

        (void)memcpy(Addr, &rp.name, msize);
        *AddrLen = msize;
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return (int32_t)ret;
}


/* mDNS */


int32_t MX_WIFI_MDNS_start(MX_WIFIObject_t *Obj, const mx_char_t *domain, mx_char_t *hostname)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if (NULL != Obj)
  {
    mdns_start_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    int32_t status =  MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    ret = MX_WIFI_STATUS_ERROR;

    (void)memcpy(&cp.domain[0], domain, sizeof(cp.domain));
    (void)memcpy(&cp.hostname[0], hostname, sizeof(cp.hostname));

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_MDNS_START_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_MDNS_stop(MX_WIFIObject_t *Obj)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if (NULL != Obj)
  {
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    ret = MX_WIFI_STATUS_ERROR;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_MDNS_STOP_CMD, NULL, 0,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_MDNS_announce_service(MX_WIFIObject_t *Obj,
                                      struct mc_mdns_service *service, mwifi_if_t interface)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (NULL != service))
  {
    mdns_announce_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    ret = MX_WIFI_STATUS_ERROR;

    cp.iface = interface;
    (void)memcpy(&cp.service_data, service, sizeof(cp.service_data));


    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_MDNS_ANNOUNCE_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_MDNS_deannounce_service(MX_WIFIObject_t *Obj,
                                        struct mc_mdns_service *service,
                                        mwifi_if_t interface)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (NULL != service))
  {
    mdns_deannounce_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    ret = MX_WIFI_STATUS_ERROR;

    cp.iface = interface;
    (void)memcpy(&cp.service_data, service, sizeof(cp.service_data));

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_MDNS_DEANNOUNCE_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_MDNS_deannounce_service_all(MX_WIFIObject_t *Obj, mwifi_if_t interface)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if (NULL != Obj)
  {
    mdns_deannounce_all_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    ret = MX_WIFI_STATUS_ERROR;

    cp.iface = interface;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_MDNS_DEANNOUNCE_ALL_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_MDNS_iface_state_change(MX_WIFIObject_t *Obj, mwifi_if_t interface,
                                        enum iface_state state)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if (NULL != Obj)
  {
    mdns_iface_state_change_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    ret = MX_WIFI_STATUS_ERROR;

    cp.iface = interface;
    cp.state = state;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_MDNS_IFACE_STATE_CHANGE_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_MDNS_set_hostname(MX_WIFIObject_t *Obj, char *hostname)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if (NULL != Obj)
  {
    mdns_set_hostname_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    ret = MX_WIFI_STATUS_ERROR;

    MX_WIFI_STRNCPY(cp.hostname, hostname);

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_MDNS_SET_HOSTNAME_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_MDNS_set_txt_rec(MX_WIFIObject_t *Obj, struct mc_mdns_service *Service, mx_char_t *KeyVals,
                                 mx_char_t separator)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (NULL != Service) && (NULL != KeyVals))
  {
    mdns_set_txt_rec_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    ret = MX_WIFI_STATUS_ERROR;

    cp.service_data = *Service;
    MX_WIFI_STRNCPY(cp.keyvals, KeyVals);
    cp.separator = separator;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_MDNS_SET_TXT_REC_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


/* TLS */


int32_t MX_WIFI_TLS_set_ver(MX_WIFIObject_t *Obj, mtls_ver_t version)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if (NULL != Obj)
  {
    tls_set_ver_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    tls_set_ver_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    ret = MX_WIFI_STATUS_ERROR;

    cp.version = version;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_TLS_SET_VER_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == rp.ret)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_TLS_set_clientCertificate(MX_WIFIObject_t *Obj, uint8_t *client_cert, uint16_t cert_len)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;
  tls_set_client_cert_cparams_t *cp = NULL;
  const uint16_t cp_size = sizeof(tls_set_client_cert_cparams_t) -1 + cert_len;

  if ((NULL == Obj) || (NULL == client_cert) || (cert_len == (uint16_t)0) ||
      (cp_size > (uint16_t)(MX_WIFI_IPC_PAYLOAD_SIZE)))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    tls_set_client_cert_rparams_t rp = {0};
    uint16_t rp_size = sizeof(rp);

    cp = (tls_set_client_cert_cparams_t *)MX_WIFI_MALLOC(cp_size);
    if (NULL != cp)
    {
      memset(cp, 0, cp_size);
      cp->cert_pem_size = cert_len;
      /* useless: cp->private_key_pem_size = 0; */
      (void)memcpy(&cp->cert_data[0], client_cert, cert_len);

      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_TLS_SET_CLIENT_CERT_CMD,
                                            (uint8_t *)cp, cp_size,
                                            (uint8_t *)&rp, &rp_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        if (MIPC_CODE_SUCCESS == rp.ret)
        {
          ret = MX_WIFI_STATUS_OK;
        }
      }
      MX_WIFI_FREE(cp);
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_TLS_set_clientPrivateKey(MX_WIFIObject_t *Obj, uint8_t *client_private_key, uint16_t key_len)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;
  tls_set_client_cert_cparams_t *cp = NULL;
  const uint16_t cp_size = sizeof(tls_set_client_cert_cparams_t) -1 + key_len;

  if ((NULL == Obj) || (NULL == client_private_key) || (key_len == (uint16_t)0)
      || (cp_size > (uint16_t)(MX_WIFI_IPC_PAYLOAD_SIZE)))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    tls_set_client_cert_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    cp = (tls_set_client_cert_cparams_t *)MX_WIFI_MALLOC(cp_size);
    if (NULL != cp)
    {
      memset(cp, 0, cp_size);
      /* useless: cp->cert_pem_size = 0; */
      /* useless: cp->private_key_pem_size = key_len; */
      (void)memcpy(&cp->cert_data[0], client_private_key, key_len);

      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_TLS_SET_CLIENT_CERT_CMD,
                                            (uint8_t *)cp, cp_size,
                                            (uint8_t *)&rp, &rp_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        if (MIPC_CODE_SUCCESS == rp.ret)
        {
          ret = MX_WIFI_STATUS_OK;
        }
      }
      MX_WIFI_FREE(cp);
    }
  }
  return (int32_t)ret;
}


int32_t MX_WIFI_TLS_connect(MX_WIFIObject_t *Obj, int32_t domain, int32_t type, int32_t protocol,
                            const struct mx_sockaddr *Addr, int32_t AddrLen, mx_char_t *ca, int32_t calen)
{
  int32_t ret = (int32_t)MX_WIFI_STATUS_PARAM_ERROR;
  (void)domain;
  (void)type;
  (void)protocol;

  if ((NULL != Obj) && (NULL != Addr) && (0 < AddrLen))
  {
    ret = MX_WIFI_TLS_connect_sni(Obj, NULL, 0, Addr, AddrLen, ca, calen);
  }

  return ret;
}


int32_t MX_WIFI_TLS_connect_sni(MX_WIFIObject_t *Obj, const mx_char_t *sni_servername, int32_t sni_servername_len,
                                const struct mx_sockaddr *addr, int32_t addrlen, mx_char_t *ca, int32_t calen)
{
  int32_t ret = (int32_t)MX_WIFI_STATUS_ERROR;
  tls_connect_sni_cparams_t *cp = NULL;
  const uint16_t cp_size = (uint16_t)(sizeof(tls_connect_sni_cparams_t) - 1 + (uint16_t)calen);

  if ((NULL == Obj) || (NULL == addr) || (addrlen <= 0) || (cp_size > (uint16_t)(MX_WIFI_IPC_PAYLOAD_SIZE)))
  {
    ret = 0; /* null for MX_WIFI_STATUS_PARAM_ERROR */
  }
  else
  {
    tls_connect_sni_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    /* useless: rp.tls = NULL; */

    cp = (tls_connect_sni_cparams_t *)MX_WIFI_MALLOC(cp_size);
    if (NULL != cp)
    {
      memset(cp, 0, cp_size);
      if ((sni_servername_len > 0) && (NULL != sni_servername))
      {
        MX_WIFI_STRNCPY(cp->sni_servername, sni_servername);
      }
      (void)memcpy(&cp->addr, addr, sizeof(cp->addr));
      cp->length = (mx_socklen_t)addrlen;
      cp->calen = calen;
      if ((calen > 0) && (NULL != ca))
      {
        (void)memcpy(&cp->ca[0], ca, (size_t)calen);
      }
      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_TLS_CONNECT_SNI_CMD,
                                            (uint8_t *)cp, cp_size,
                                            (uint8_t *)&rp, &rp_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        if (NULL == rp.tls)
        {
          ret = 0;  /* NULL */
        }
        else
        {
          ret = (int32_t)rp.tls;
        }
      }
      MX_WIFI_FREE(cp);
    }
  }

  return ret;
}


int32_t MX_WIFI_TLS_send(MX_WIFIObject_t *Obj, mtls_t tls, const void *Data, int32_t Len)
{
  int32_t ret = (int32_t)MX_WIFI_STATUS_ERROR;

  if ((NULL == Obj) || (NULL == tls) || (NULL == Data) || (Len <= 0))
  {
    ret = (int32_t)MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    tls_send_cparams_t *cp = NULL;
    uint16_t cp_size;
    tls_send_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);
    size_t data_len = (size_t)Len;

    if ((data_len + sizeof(tls_send_cparams_t) - 1) > MX_WIFI_IPC_PAYLOAD_SIZE)
    {
      /* Restrict to the length which corresponds to the maximum size of the IPC transfer. */
      data_len = MX_WIFI_IPC_PAYLOAD_SIZE - (sizeof(tls_send_cparams_t) - 1);
    }

    cp_size = (uint16_t)(sizeof(tls_send_cparams_t) - 1 + data_len);
    cp = (tls_send_cparams_t *)MX_WIFI_MALLOC(cp_size);
    if (NULL != cp)
    {
      cp->tls = tls;
      (void)memcpy(&cp->buffer[0], Data, data_len);
      cp->size = data_len;
      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_TLS_SEND_CMD,
                                            (uint8_t *)cp, cp_size,
                                            (uint8_t *)&rp, &rp_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        ret = rp.sent;
      }
      MX_WIFI_FREE(cp);
    }
  }
  return ret;
}


int32_t MX_WIFI_TLS_recv(MX_WIFIObject_t *Obj, mtls_t tls, void *Data, int32_t Len)
{
  int32_t ret = (int32_t)MX_WIFI_STATUS_ERROR;

  if ((NULL == Obj) || (NULL == tls) || (NULL == Data) || (Len <= 0))
  {
    ret = (int32_t)MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    tls_recv_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    tls_recv_rparams_t *rp = NULL;
    uint16_t rp_size;
    size_t data_len = (size_t)Len;

    ret = 0;

    if ((data_len + sizeof(tls_recv_rparams_t) - 1) > MX_WIFI_IPC_PAYLOAD_SIZE)
    {
      /* Restrict to the length which corresponds to the maximum size of the IPC transfer. */
      data_len = MX_WIFI_IPC_PAYLOAD_SIZE - (sizeof(tls_recv_rparams_t) - 1);
    }

    rp_size = (uint16_t)(sizeof(tls_recv_rparams_t) - 1 + data_len);
    rp = (tls_recv_rparams_t *)MX_WIFI_MALLOC(rp_size);
    if (NULL != rp)
    {
      rp->received = 0;
      cp.tls = tls;
      cp.size = data_len;
      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_TLS_RECV_CMD,
                                            (uint8_t *)&cp, cp_size,
                                            (uint8_t *)rp, &rp_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        if (rp->received > 0)
        {
          const size_t received_len = (size_t)rp->received;
          if (received_len <= data_len)
          {
            (void)memcpy(Data, &rp->buffer[0], received_len);
          }
        }
        ret = rp->received;
      }
      MX_WIFI_FREE(rp);
    }
  }

  return ret;
}


int32_t MX_WIFI_TLS_close(MX_WIFIObject_t *Obj, mtls_t tls)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if ((NULL == Obj) || (NULL == tls))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    tls_close_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    tls_close_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    cp.tls = tls;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_TLS_CLOSE_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (rp.ret == MIPC_CODE_SUCCESS)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return (int32_t)ret;
}


int32_t MX_WIFI_TLS_set_nonblock(MX_WIFIObject_t *Obj, mtls_t tls, int32_t nonblock)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if ((NULL == Obj) || (NULL == tls))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    tls_set_nonblock_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));
    tls_set_nonblock_rparams_t rp = {0};
    uint16_t rp_size = (uint16_t)sizeof(rp);

    cp.tls = tls;
    cp.nonblock = nonblock;

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_TLS_SET_NONBLOCK_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&rp, &rp_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (rp.ret == MIPC_CODE_SUCCESS)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return (int32_t)ret;
}


int32_t MX_WIFI_Webserver_start(MX_WIFIObject_t *Obj)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SYS_CFG_SERVER_START_CMD,
                                          NULL, 0,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_Webserver_stop(MX_WIFIObject_t *Obj)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SYS_CFG_SERVER_STOP_CMD,
                                          NULL, 0,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}


int32_t MX_WIFI_FOTA_start(MX_WIFIObject_t *Obj, const char *Url, const char *Md5,
                           mx_wifi_fota_status_cb_t FotaStatusCallback, uint32_t UserArgs)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if ((NULL != Obj) && (NULL != Url) && (NULL != Md5) && \
      (strlen(Url) <= FOTA_FILE_URL_MAX_LEN) && \
      (strlen(Md5) <= FOTA_COMPONENT_MD5_MAX_LEN))
  {
    sys_fota_start_cparams_t cp = {0};
    const uint16_t cp_size = (uint16_t)(sizeof(cp));

    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    ret = MX_WIFI_STATUS_ERROR;

    if (NULL != FotaStatusCallback)
    {
      Obj->Runtime.fota_status_cb = FotaStatusCallback;
      Obj->Runtime.fota_user_args = UserArgs;
    }
    else
    {
      Obj->Runtime.fota_status_cb = NULL;
      Obj->Runtime.fota_user_args = 0;
    }

    MX_WIFI_STRNCPY(cp.url, Url);
    MX_WIFI_STRNCPY(cp.md5, Md5);

    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_SYS_FOTA_START_CMD,
                                          (uint8_t *)&cp, cp_size,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }

  return (int32_t)ret;
}

#else /* MX_WIFI_NETWORK_BYPASS_MODE */


MX_WIFI_STATUS_T MX_WIFI_Network_bypass_mode_set(MX_WIFIObject_t *Obj, int32_t enable,
                                                 mx_wifi_netlink_input_cb_t netlink_input_callbck,
                                                 void *user_args)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;
  wifi_bypass_set_cparams_t cparams = {0};
  int32_t status = MIPC_CODE_ERROR;
  uint16_t status_size = (uint16_t)sizeof(status);

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    if ((NULL != netlink_input_callbck) && (1 == enable))
    {
      Obj->Runtime.netlink_input_cb = netlink_input_callbck;
      Obj->Runtime.netlink_user_args = user_args;
    }
    else
    {
      Obj->Runtime.netlink_input_cb = NULL;
      Obj->Runtime.netlink_user_args = NULL;
    }

    cparams.mode = enable;
    if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_BYPASS_SET_CMD,
                                          (uint8_t *)&cparams, (uint16_t)sizeof(cparams),
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}


MX_WIFI_STATUS_T MX_WIFI_Network_bypass_netlink_output(MX_WIFIObject_t *Obj, void *data,
                                                       int32_t len,
                                                       int32_t interface)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_ERROR;
  wifi_bypass_out_cparams_t *cparams = NULL;
  const uint16_t cparams_size = (uint16_t)(sizeof(wifi_bypass_out_cparams_t) + (size_t)len);
  int32_t status = MIPC_CODE_ERROR;
  uint16_t status_size = (uint16_t)sizeof(status);

  if ((NULL == Obj) || (len <= 0) || (((int32_t)STATION_IDX != interface) && ((int32_t)SOFTAP_IDX != interface)))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    if (cparams_size > MX_WIFI_IPC_PAYLOAD_SIZE)
    {
      len = MX_WIFI_IPC_PAYLOAD_SIZE - sizeof(wifi_bypass_out_cparams_t);
    }
#if MX_WIFI_TX_BUFFER_NO_COPY
    /* structure of data buffer must support head room provision to add information in from of data payload */
    cparams = (wifi_bypass_out_cparams_t *)((uint8_t *)data - sizeof(wifi_bypass_out_cparams_t));
#else
    cparams = (wifi_bypass_out_cparams_t *)MX_WIFI_MALLOC(len + sizeof(wifi_bypass_out_cparams_t));
#endif /* MX_WIFI_TX_BUFFER_NO_COPY */

    if (NULL != cparams)
    {
      cparams->idx = interface;
      cparams->data_len = (uint16_t)len;

#if MX_WIFI_TX_BUFFER_NO_COPY == 0
      /* copy the whole payload */
      uint8_t *dst;
      dst = (uint8_t *)cparams + sizeof(wifi_bypass_out_cparams_t);
      (void)memcpy(dst, data, len);
#endif  /* MX_WIFI_TX_BUFFER_NO_COPY */

      if (MIPC_CODE_SUCCESS == mipc_request(MIPC_API_WIFI_BYPASS_OUT_CMD,
                                            (uint8_t *)cparams, cparams_size,
                                            (uint8_t *)&status, &status_size,
                                            MX_WIFI_CMD_TIMEOUT))
      {
        if (MIPC_CODE_SUCCESS == status)
        {
          ret = MX_WIFI_STATUS_OK;
        }
      }
#if MX_WIFI_TX_BUFFER_NO_COPY == 0
      MX_WIFI_FREE(cparams);
#endif /* MX_WIFI_TX_BUFFER_NO_COPY */
    }
    else
    {
      /*  no memory */
      DEBUG_LOG("No memory!!!\n");
    }
  }

  return ret;
}
#endif /* MX_WIFI_NETWORK_BYPASS_MODE */


int32_t MX_WIFI_station_powersave(MX_WIFIObject_t *Obj, int32_t ps_onoff)
{
  MX_WIFI_STATUS_T ret = MX_WIFI_STATUS_PARAM_ERROR;

  if (NULL != Obj)
  {
    uint16_t mipc_ps_cmd;
    int32_t status = MIPC_CODE_ERROR;
    uint16_t status_size = (uint16_t)sizeof(status);

    ret = MX_WIFI_STATUS_ERROR;

    if (0 != ps_onoff)
    {
      mipc_ps_cmd = MIPC_API_WIFI_PS_ON_CMD;
    }
    else
    {
      mipc_ps_cmd = MIPC_API_WIFI_PS_OFF_CMD;
    }

    if (MIPC_CODE_SUCCESS == mipc_request(mipc_ps_cmd, NULL, 0,
                                          (uint8_t *)&status, &status_size,
                                          MX_WIFI_CMD_TIMEOUT))
    {
      if (MIPC_CODE_SUCCESS == status)
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return (int32_t)ret;
}


#if (MX_WIFI_NETWORK_BYPASS_MODE == 0)
static struct mx_sockaddr_storage mx_s_addr_in_to_packed(const struct mx_sockaddr *Addr)
{
  struct mx_sockaddr_storage s_addr_storage = {0};
  const struct mx_sockaddr_in *const p_s_addr_in = (const struct mx_sockaddr_in *) Addr;

  s_addr_storage.s2_len = p_s_addr_in->sin_len;
  s_addr_storage.ss_family = p_s_addr_in->sin_family;
  s_addr_storage.s2_data1[0] = (uint8_t)(p_s_addr_in->sin_port);
  s_addr_storage.s2_data1[1] = (uint8_t)(p_s_addr_in->sin_port >> 8);
  s_addr_storage.s2_data2[0] = p_s_addr_in->sin_addr.s_addr;
  /* useless: s_addr_storage.s2_data2[1] = 0; */
  /* useless: s_addr_storage.s2_data2[2] = 0; */
  /* useless: s_addr_storage.s2_data3[0] = 0; */
  /* useless: s_addr_storage.s2_data3[1] = 0; */
  /* useless: s_addr_storage.s2_data3[2] = 0; */

  return s_addr_storage;
}

static struct mx_sockaddr_in mx_s_addr_in_from_packed(const struct mx_sockaddr_storage *Addr)
{
  struct mx_sockaddr_in s_addr_in = {0};

  s_addr_in.sin_len = Addr->s2_len;
  s_addr_in.sin_family = Addr->ss_family;
  {
    uint16_t port_in = (uint16_t)Addr->s2_data1[0];
    port_in |= (uint16_t)((uint16_t)Addr->s2_data1[1] << 8);
    s_addr_in.sin_port = port_in;
  }
  s_addr_in.sin_addr.s_addr = Addr->s2_data2[0];
  /* useless: s_addr_in.sin_zero[0] = 0; */
  /* useless: s_addr_in.sin_zero[1] = 0; */
  /* useless: s_addr_in.sin_zero[2] = 0; */
  /* useless: s_addr_in.sin_zero[3] = 0; */
  /* useless: s_addr_in.sin_zero[4] = 0; */
  /* useless: s_addr_in.sin_zero[5] = 0; */
  /* useless: s_addr_in.sin_zero[6] = 0; */
  /* useless: s_addr_in.sin_zero[7] = 0; */

  return s_addr_in;
}

static struct mx_sockaddr_storage mx_s_addr_in6_to_packed(const struct mx_sockaddr *Addr)
{
  struct mx_sockaddr_storage s_addr_storage = {0};
  const struct mx_sockaddr_in6 *const p_s_addr_in6 = (const struct mx_sockaddr_in6 *) Addr;

  s_addr_storage.s2_len = p_s_addr_in6->sin6_len;
  s_addr_storage.ss_family = p_s_addr_in6->sin6_family;
  s_addr_storage.s2_data1[0] = (uint8_t)(p_s_addr_in6->sin6_port);
  s_addr_storage.s2_data1[1] = (uint8_t)(p_s_addr_in6->sin6_port >> 8);
  s_addr_storage.s2_data2[0] = p_s_addr_in6->sin6_flowinfo;
  s_addr_storage.s2_data2[1] = p_s_addr_in6->sin6_addr.un.u32_addr[0];
  s_addr_storage.s2_data2[2] = p_s_addr_in6->sin6_addr.un.u32_addr[1];
  s_addr_storage.s2_data3[0] = p_s_addr_in6->sin6_addr.un.u32_addr[2];
  s_addr_storage.s2_data3[1] = p_s_addr_in6->sin6_addr.un.u32_addr[3];
  s_addr_storage.s2_data3[2] = p_s_addr_in6->sin6_scope_id;

  return s_addr_storage;
}

static struct mx_sockaddr_in6 mx_s_addr_in6_from_packed(const struct mx_sockaddr_storage *Addr)
{
  struct mx_sockaddr_in6 s_addr_in6 = {0};

  s_addr_in6.sin6_len = Addr->s2_len;
  s_addr_in6.sin6_family = Addr->ss_family;
  {
    uint16_t port_in = (uint16_t)Addr->s2_data1[0];
    port_in |= (uint16_t)((uint16_t)Addr->s2_data1[1] << 8);
    s_addr_in6.sin6_port = port_in;
  }
  s_addr_in6.sin6_flowinfo = Addr->s2_data2[0];
  s_addr_in6.sin6_addr.un.u32_addr[0] = Addr->s2_data2[1];
  s_addr_in6.sin6_addr.un.u32_addr[1] = Addr->s2_data2[2];
  s_addr_in6.sin6_addr.un.u32_addr[2] = Addr->s2_data3[0];
  s_addr_in6.sin6_addr.un.u32_addr[3] = Addr->s2_data3[1];
  s_addr_in6.sin6_scope_id = Addr->s2_data3[2];

  return s_addr_in6;
}
#endif /* (MX_WIFI_NETWORK_BYPASS_MODE == 0) */

static void void_strncpy(char *Destination, const char *Source, size_t Num)
{
  (void)strncpy(Destination, Source, Num);
}
