/**
  ******************************************************************************
  * @file    mx_wifi.c
  * @author  MCD Application Team
  * @brief   Host driver API of MXCHIP Wi-Fi component.
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
#include "mx_wifi.h"
#include "lib_ipc/lib_ipc.h"
#include "mx_wifi_ipc.h"
/*cstat -MISRAC2012-* */
#include "net_address.h"
/*cstat +MISRAC2012-* */

/* Private defines -----------------------------------------------------------*/
#ifdef MX_WIFI_API_DEBUG
#define DEBUG_LOG(M, ...)  printf(M, ##__VA_ARGS__);
#else
#define DEBUG_LOG(M, ...)
#endif

/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

MX_WIFI_Status_t MX_WIFI_RegisterBusIO(MX_WIFIObject_t *Obj,
                                       IO_Init_Func IO_Init, IO_DeInit_Func IO_DeInit,
                                       IO_Delay_Func IO_Delay,
                                       IO_Send_Func IO_Send, IO_Receive_Func IO_Receive)
{
  MX_WIFI_Status_t rc;

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

MX_WIFI_Status_t MX_WIFI_HardResetModule(MX_WIFIObject_t *Obj)
{
  MX_WIFI_Status_t rc;
  int32_t ret;

  if (NULL == Obj)
  {
    rc = MX_WIFI_STATUS_ERROR;
  }
  else
  {
    ret = Obj->fops.IO_Init(MX_WIFI_RESET);
    if ((int32_t)0 == ret)
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

MX_WIFI_Status_t MX_WIFI_SetTimeout(MX_WIFIObject_t *Obj, uint32_t Timeout)
{
  MX_WIFI_Status_t rc;

  if (NULL == Obj)
  {
    rc = MX_WIFI_STATUS_ERROR;
  }
  else
  {
#if MX_WIFI_USE_CMSIS_OS
    if (!MX_WIFI_LOCK((Obj)))
    {
      rc = MX_WIFI_STATUS_TIMEOUT;
    }
    else
#endif
    {
      Obj->Runtime.Timeout = Timeout;
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((Obj));
#endif
      rc = MX_WIFI_STATUS_OK;
    }
  }

  return rc;
}

#if MX_WIFI_USE_CMSIS_OS
#define MX_WIFI_THREAD_STACK_SIZE (0x500)
static osThreadId MX_WIFI_RecvThreadId = NULL;

extern MX_WIFIObject_t *wifi_obj_get(void);
static void _MX_WIFI_RecvThread(void const *argument)
{
  MX_WIFIObject_t *Obj = wifi_obj_get();

  if (NULL != Obj)
  {
    while (true)
    {
      (void)MX_WIFI_IO_YIELD(Obj, 500);
      (void)osDelay(1);
    }
  }

  // Delete the Thread
  (void)osThreadTerminate(NULL);
}

#if (osCMSIS >= 0x20000U )
static const   osThreadAttr_t attr =
{
  .name = "MxWifiRecvThread",
  .priority = osPriorityBelowNormal,
  . stack_size = MX_WIFI_THREAD_STACK_SIZE
};
#endif


static MX_WIFI_Status_t _mx_wifi_start_recv_task(MX_WIFIObject_t *Obj)
{
  MX_WIFI_Status_t rc;

#if (osCMSIS < 0x20000U )
  osThreadDef(mx_wifi_recv, _MX_WIFI_RecvThread, osPriorityBelowNormal, 0, MX_WIFI_THREAD_STACK_SIZE);
  MX_WIFI_RecvThreadId = osThreadCreate(osThread(mx_wifi_recv), Obj);
#else
  MX_WIFI_RecvThreadId = osThreadNew((osThreadFunc_t)_MX_WIFI_RecvThread, Obj, &attr);
#endif

  if (NULL == MX_WIFI_RecvThreadId)
  {
    rc = MX_WIFI_STATUS_ERROR;
  }
  else
  {
    rc = MX_WIFI_STATUS_OK;
  }

  return rc;
}

static MX_WIFI_Status_t _mx_wifi_stop_recv_task(MX_WIFIObject_t *Obj)
{
  MX_WIFI_Status_t rc;

  (void)Obj;

  if (osOK == osThreadTerminate(MX_WIFI_RecvThreadId))
  {
    rc = MX_WIFI_STATUS_OK;
  }
  else
  {
    rc = MX_WIFI_STATUS_ERROR;
  }

  return rc;
}
#endif

MX_WIFI_Status_t MX_WIFI_Init(MX_WIFIObject_t *Obj)
{
  MX_WIFI_Status_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t ret_value = IPC_RET_ERROR;

  int32_t addr_mac, addr_fw, addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;
  uint8_t *p_addr_fw, *p_addr_mac;

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
#if MX_WIFI_USE_CMSIS_OS
#if (osCMSIS < 0x20000U )
    Obj->wifi_mutex_id = osMutexCreate(&(Obj->wifi_mutex));
#else
    Obj->wifi_mutex_id = osMutexNew(&(Obj->wifi_mutex));
#endif /* osCMSIS < 0x20000U */

    if (!MX_WIFI_LOCK((Obj)))
    {
      ret = MX_WIFI_STATUS_TIMEOUT;
    }
    else
#endif /* MX_WIFI_USE_CMSIS_OS */
    {
      // 0. set cmd timeout
      Obj->Runtime.Timeout = MX_WIFI_TIMEOUT;

      // 1. init wifi low level IO(UART/SPI)
      if (0 == Obj->fops.IO_Init(MX_WIFI_INIT))
      {
        // 2. init wifi ipc
        if (IPC_RET_OK == mipc_init(Obj->fops.IO_Send, Obj->fops.IO_Receive, Obj->Runtime.Timeout))
        {
          // 3. get version
          (void)memset(&(Obj->SysInfo.FW_Rev[0]), 0, MX_WIFI_FW_REV_SIZE);
          p_addr_fw = &(Obj->SysInfo.FW_Rev[0]);
          p_addr_ret_val = &ret_value;
          p_addr_ret_flag = &ret_flag;
          (void)memcpy(&addr_fw, &p_addr_fw, sizeof(addr_fw));
          (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
          (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
          if (IPC_RET_OK == system_firmware_version_get_cmd(addr_fw,
                                                            MX_WIFI_FW_REV_SIZE,
                                                            addr_ret_val, addr_ret_flag))
          {
            (void)strncpy((char_t *)(Obj->SysInfo.Product_Name), MX_WIFI_PRODUCT_NAME, MX_WIFI_PRODUCT_NAME_SIZE);
            (void)strncpy((char_t *)(Obj->SysInfo.Product_ID), MX_WIFI_PRODUCT_ID, MX_WIFI_PRODUCT_ID_SIZE);

            if (IPC_RET_OK == mipc_poll(&ret_flag, &ret_value, Obj->Runtime.Timeout))
            {
              // 4. get MAC
              ret_flag = MIPC_RET_FLAG_CLR;
              ret_value = IPC_RET_ERROR;
              p_addr_mac = &(Obj->SysInfo.MAC[0]);
              (void)memcpy(&addr_mac, &p_addr_mac, sizeof(addr_mac));
              if (IPC_RET_OK == wifi_mac_get_cmd(addr_mac, MX_WIFI_MAC_SIZE,
                                                 addr_ret_val, addr_ret_flag))
              {
                if (IPC_RET_OK == mipc_poll(&ret_flag, &ret_value, Obj->Runtime.Timeout))
                {
                  // start poll task
#if MX_WIFI_USE_CMSIS_OS
                  (void)MX_WIFI_UNLOCK((Obj));
                  ret = _mx_wifi_start_recv_task(Obj);
#else
                  ret = MX_WIFI_STATUS_OK;
#endif
                }
              } // get MAC
            }
          } // get version
        }
      }
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((Obj));
#endif
    }
  }

  return ret;
}

MX_WIFI_Status_t MX_WIFI_DeInit(MX_WIFIObject_t *Obj)
{
  MX_WIFI_Status_t ret;

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
#if MX_WIFI_USE_CMSIS_OS
    if (!MX_WIFI_LOCK((Obj)))
    {
      ret = MX_WIFI_STATUS_TIMEOUT;
    }
    else
    {
      if (MX_WIFI_STATUS_OK == _mx_wifi_stop_recv_task(Obj))
      {
        (void)mipc_deinit();
        Obj->fops.IO_DeInit();
        ret = MX_WIFI_STATUS_OK;
      }
      else
      {
        ret = MX_WIFI_STATUS_ERROR;
      }

      (void)MX_WIFI_UNLOCK((Obj));
    }
#else
    ret = MX_WIFI_STATUS_OK;
#endif
  }

  return ret;
}

MX_WIFI_Status_t MX_WIFI_IO_YIELD(MX_WIFIObject_t *Obj, uint32_t timeout)
{
  MX_WIFI_Status_t ret = MX_WIFI_STATUS_ERROR;

  if (NULL != Obj)
  {
    (void)mipc_poll(NULL, NULL, timeout);
    ret = MX_WIFI_STATUS_OK;
  }

  return ret;
}

MX_WIFI_Status_t MX_WIFI_ResetModule(MX_WIFIObject_t *Obj)
{
  MX_WIFI_Status_t ret = MX_WIFI_STATUS_ERROR;

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
#if MX_WIFI_USE_CMSIS_OS
    if (!MX_WIFI_LOCK((Obj)))
    {
      ret = MX_WIFI_STATUS_TIMEOUT;
    }
    else
#endif
    {
      if (IPC_RET_OK == system_reboot_cmd())
      {
        ret = MX_WIFI_STATUS_OK;
      }
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((Obj));
#endif
    }
  }

  return ret;
}

MX_WIFI_Status_t MX_WIFI_ResetToFactoryDefault(MX_WIFIObject_t *Obj)
{
  MX_WIFI_Status_t ret = MX_WIFI_STATUS_ERROR;

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
#if MX_WIFI_USE_CMSIS_OS
    if (!MX_WIFI_LOCK((Obj)))
    {
      ret = MX_WIFI_STATUS_TIMEOUT;
    }
    else
#endif
    {
      if (IPC_RET_OK == system_factory_cmd())
      {
        ret = MX_WIFI_STATUS_OK;
      }
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((Obj));
#endif
    }
  }
  return ret;
}

MX_WIFI_Status_t MX_WIFI_GetVersion(MX_WIFIObject_t *Obj, uint8_t *version, uint32_t size)
{
  MX_WIFI_Status_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t ret_value = IPC_RET_ERROR;
  int32_t addr_fw, addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;
  uint8_t *p_addr_fw;

  if ((NULL == Obj) || (NULL == version) || (size > (uint32_t)MX_WIFI_FW_REV_SIZE) \
      || ((uint32_t)0 == size))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
#if MX_WIFI_USE_CMSIS_OS
    if (!MX_WIFI_LOCK((Obj)))
    {
      ret = MX_WIFI_STATUS_TIMEOUT;
    }
    else
#endif
    {
      (void)memset(&(Obj->SysInfo.FW_Rev[0]), 0, MX_WIFI_FW_REV_SIZE);
      p_addr_fw = &(Obj->SysInfo.FW_Rev[0]);
      p_addr_ret_val = &ret_value;
      p_addr_ret_flag = &ret_flag;
      (void)memcpy(&addr_fw, &p_addr_fw, sizeof(addr_fw));
      (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
      (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
      if (IPC_RET_OK == system_firmware_version_get_cmd(addr_fw,
                                                        MX_WIFI_FW_REV_SIZE,
                                                        addr_ret_val, addr_ret_flag))
      {
        if (IPC_RET_OK == mipc_poll(&ret_flag, &ret_value, Obj->Runtime.Timeout))
        {
          (void)memcpy(version, &(Obj->SysInfo.FW_Rev[0]), MX_WIFI_FW_REV_SIZE);
#if MX_WIFI_USE_CMSIS_OS
          (void)MX_WIFI_UNLOCK((Obj));
#endif
          ret = MX_WIFI_STATUS_OK;
        }
      }
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((Obj));
#endif
    }
  }
  return ret;
}

MX_WIFI_Status_t MX_WIFI_GetMACAddress(MX_WIFIObject_t *Obj, uint8_t *mac)
{
  MX_WIFI_Status_t ret;

  if ((NULL == Obj) || (NULL == mac))
  {
    ret = MX_WIFI_STATUS_ERROR;
  }
  else
  {
    (void)memcpy(mac, Obj->SysInfo.MAC, MX_WIFI_MAC_SIZE);
    ret = MX_WIFI_STATUS_OK;
  }

  return ret;
}

MX_WIFI_Status_t MX_WIFI_Scan(MX_WIFIObject_t *Obj, mc_wifi_scan_mode_t scan_mode,
                              char_t *ssid, int32_t len)
{
  MX_WIFI_Status_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;

  int32_t addr_ret_flag;
  int32_t *p_addr_ret_flag;

  if ( (NULL == Obj) || ( (MC_SCAN_ACTIVE == scan_mode) && ((NULL == ssid) || (len <= 0)) ) )
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
#if MX_WIFI_USE_CMSIS_OS
    if (!MX_WIFI_LOCK((Obj)))
    {
      ret = MX_WIFI_STATUS_TIMEOUT;
    }
    else
#endif
    {
      p_addr_ret_flag = &ret_flag;
      (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
      if (IPC_RET_OK == wifi_scan_cmd((int32_t)scan_mode, ssid, len, addr_ret_flag))
      {
        if (IPC_RET_OK == mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout))
        {
          ret = MX_WIFI_STATUS_OK;
        }
      }
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((Obj));
#endif
    }
  }
  return ret;
}

int8_t MX_WIFI_Get_scan_result(MX_WIFIObject_t *Obj, uint8_t *results, uint8_t number)
{
  int8_t copy_number;

  if ((NULL == Obj) || (NULL == results) || ((uint8_t)0 == number))
  {
    copy_number = 0;
  }
  else
  {
#if MX_WIFI_USE_CMSIS_OS
    if (!MX_WIFI_LOCK((Obj)))
    {
      copy_number = 0;
    }
    else
#endif
    {
      copy_number = (int8_t)(MIN(Obj->Runtime.scan_number, number));
      (void)memcpy(results, Obj->Runtime.scan_result, (size_t)copy_number * sizeof(mc_wifi_ap_info_t));
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((Obj));
#endif
    }
  }
  return copy_number;
}

MX_WIFI_Status_t MX_WIFI_RegisterStatusCallback(MX_WIFIObject_t *Obj,
                                                mx_wifi_status_callback_t cb,
                                                void *arg)
{
  MX_WIFI_Status_t rc;

  if (NULL == Obj)
  {
    rc = MX_WIFI_STATUS_ERROR;
  }
  else
  {
#if MX_WIFI_USE_CMSIS_OS
    if (!MX_WIFI_LOCK((Obj)))
    {
      rc = MX_WIFI_STATUS_TIMEOUT;
    }
    else
#endif
    {
      Obj->Runtime.status_cb = cb;
      Obj->Runtime.callback_arg = arg;
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((Obj));
#endif
      rc = MX_WIFI_STATUS_OK;
    }
  }
  return rc;
}

MX_WIFI_Status_t MX_WIFI_UnRegisterStatusCallback(MX_WIFIObject_t *Obj)
{
  MX_WIFI_Status_t rc;

  if (NULL == Obj)
  {
    rc = MX_WIFI_STATUS_ERROR;
  }
  else
  {
#if MX_WIFI_USE_CMSIS_OS
    if (!MX_WIFI_LOCK((Obj)))
    {
      rc = MX_WIFI_STATUS_TIMEOUT;
    }
    else
#endif
    {
      Obj->Runtime.status_cb = NULL;
      Obj->Runtime.callback_arg = NULL;
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((Obj));
#endif
      rc = MX_WIFI_STATUS_OK;
    }
  }
  return rc;
}

MX_WIFI_Status_t MX_WIFI_Connect(MX_WIFIObject_t *Obj, const char_t *SSID,
                                 const char_t *Password, MX_WIFI_SecurityType_t SecType)
{
  MX_WIFI_Status_t ret = MX_WIFI_STATUS_ERROR;
  mwifi_ip_attr_t ip_attr, *p_ip_attr = NULL;
  int32_t ip_attr_len = 0;
  net_ip_addr_t net_ipaddr;

  (void)SecType;

  if ((NULL == Obj) || (NULL == SSID) || (strlen(SSID) > mxMaxSsidLen) || (strlen(Password) > mcMaxKeyLen) )
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
#if MX_WIFI_USE_CMSIS_OS
    if (!MX_WIFI_LOCK((Obj)))
    {
      ret = MX_WIFI_STATUS_TIMEOUT;
    }
    else
#endif
    {
      if ((uint8_t)0 == Obj->NetSettings.DHCP_IsEnabled)
      {
        (void)memset(&ip_attr, 0, sizeof(mwifi_ip_attr_t));
        (void)memcpy(&net_ipaddr, Obj->NetSettings.IP_Addr, 4);
        (void)memcpy(ip_attr.localip, net_ntoa(&net_ipaddr), mcMaxIpLen);
        (void)memcpy(&net_ipaddr, Obj->NetSettings.IP_Mask, 4);
        (void)memcpy(ip_attr.netmask, net_ntoa(&net_ipaddr), mcMaxIpLen);
        (void)memcpy(&net_ipaddr, Obj->NetSettings.Gateway_Addr, 4);
        (void)memcpy(ip_attr.gateway, net_ntoa(&net_ipaddr), mcMaxIpLen);
        (void)memcpy(&net_ipaddr, Obj->NetSettings.DNS1, 4);
        (void)memcpy(ip_attr.dnserver, net_ntoa(&net_ipaddr), mcMaxIpLen);

        p_ip_attr = &ip_attr;
        ip_attr_len = (int32_t)sizeof(mwifi_ip_attr_t);
      }

      if (IPC_RET_OK == wifi_connect_cmd((const void *) SSID, (int32_t)strlen(SSID), (const void *) Password,
                                         (int32_t)strlen(Password), NULL, 0, p_ip_attr, ip_attr_len))
      {
        ret = MX_WIFI_STATUS_OK;
      }
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((Obj));
#endif
    }
  }
  return ret;
}

MX_WIFI_Status_t MX_WIFI_EAP_Connect(MX_WIFIObject_t *Obj, const char_t *SSID, 
                                     const char *Identity, const char_t *Password,  
                                     mwifi_eap_attr_t *attr, mwifi_ip_attr_t *ip)
{
  MX_WIFI_Status_t ret = MX_WIFI_STATUS_OK;
  lib_ipc_eap_attr_t ipc_eap_attr;

  if ((NULL == Obj) || (NULL == SSID) || (strlen(SSID) > mxMaxSsidLen) || \
      (NULL == Identity) || (strlen(Identity) > mcMaxIdentityLen) || \
      (NULL == Password) || (strlen(Password) > mcMaxKeyLen))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
#if MX_WIFI_USE_CMSIS_OS
    if (!MX_WIFI_LOCK((Obj)))
    {
      ret = MX_WIFI_STATUS_TIMEOUT;
    }
    else
#endif
    {
      (void)memset(&ipc_eap_attr, 0, sizeof(ipc_eap_attr));
      
      if(NULL != attr)
      {
        /* just set eap type here, rootca/cert/key set by specifical API */
        if( (EAP_TYPE_TLS != attr->eap_type) && (EAP_TYPE_TTLS != attr->eap_type) && 
            (EAP_TYPE_PEAP != attr->eap_type) )
        {
          ret = MX_WIFI_STATUS_PARAM_ERROR;
        }
        else
        {
          ipc_eap_attr.eap_type = attr->eap_type;
          
          if(NULL != attr->rootca)
          {
            if (IPC_RET_OK == wifi_eap_set_rootca_cmd((void*)(attr->rootca), strlen(attr->rootca)))
            {
              ret = MX_WIFI_STATUS_OK;
            }
            else
            {
              ret = MX_WIFI_STATUS_ERROR;
            }
          }
          
          if((MX_WIFI_STATUS_OK == ret) && (NULL != attr->client_cert))
          {
            if (IPC_RET_OK != wifi_eap_set_client_cert_cmd((void*)(attr->client_cert), strlen(attr->client_cert)))
            {
              ret = MX_WIFI_STATUS_ERROR;
            }
          }
          
          if((MX_WIFI_STATUS_OK == ret) && (NULL != attr->client_key))
          {
            if (IPC_RET_OK != wifi_eap_set_client_key_cmd((void*)(attr->client_key), strlen(attr->client_key)))
            {
              ret = MX_WIFI_STATUS_ERROR;
            }
          }
        }
      }
      else
      {
        ipc_eap_attr.eap_type = EAP_TYPE_PEAP;  /* default eap type */
      }
      
      if(MX_WIFI_STATUS_OK == ret)
      {
        if (IPC_RET_OK != wifi_eap_connect_cmd( (void *) SSID, (int32_t)strlen(SSID), 
                                               ( void *) Identity, (int32_t)strlen(Identity), 
                                               ( void *) Password, (int32_t)strlen(Password), 
                                               &ipc_eap_attr, sizeof(ipc_eap_attr),
                                               ip, (NULL != ip) ? sizeof(mwifi_ip_attr_t):0))
        {
          ret = MX_WIFI_STATUS_ERROR;
        }
      }
      
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((Obj));
#endif
    }
  }
  return ret;
}

MX_WIFI_Status_t MX_WIFI_Disconnect(MX_WIFIObject_t *Obj)
{
  MX_WIFI_Status_t ret = MX_WIFI_STATUS_ERROR;

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
#if MX_WIFI_USE_CMSIS_OS
    if (!MX_WIFI_LOCK((Obj)))
    {
      ret = MX_WIFI_STATUS_TIMEOUT;
    }
    else
#endif
    {
      if (IPC_RET_OK == wifi_disconnect_cmd())
      {
        ret = MX_WIFI_STATUS_OK;
      }
#if MX_WIFI_USE_CMSIS_OS
      (void)MX_WIFI_UNLOCK((Obj));
#endif
    }
  }
  return ret;
}

int8_t MX_WIFI_IsConnected(MX_WIFIObject_t *Obj)
{
  int8_t ret = 0;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_flag;
  int32_t *p_addr_ret_flag;

  if (NULL != Obj)
  {
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == wifi_link_info_get_cmd(addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout))
      {
        ret = Obj->NetSettings.IsConnected;
      }
    }
  }
  return ret;
}

MX_WIFI_Status_t MX_WIFI_GetIPAddress(MX_WIFIObject_t *Obj, uint8_t *ipaddr, mc_wifi_if_t wifi_if)
{
  MX_WIFI_Status_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_flag;
  int32_t *p_addr_ret_flag;

  if (NULL != Obj)
  {
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == wifi_ip_get_cmd((int32_t)wifi_if, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout))
      {
        (void)memcpy(ipaddr, Obj->NetSettings.IP_Addr, 4);
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

/* SoftAP */
MX_WIFI_Status_t MX_WIFI_StartAP(MX_WIFIObject_t *Obj, MX_WIFI_APSettings_t *ap_settings)
{
  MX_WIFI_Status_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (NULL == ap_settings))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == wifi_softap_start_cmd(ap_settings->SSID, (int32_t)strlen(ap_settings->SSID),
                                            ap_settings->pswd, (int32_t)strlen(ap_settings->pswd),
                                            (int32_t)ap_settings->channel,
                                            &ap_settings->ip, (int32_t)sizeof(ap_settings->ip),
                                            addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

MX_WIFI_Status_t MX_WIFI_StopAP(MX_WIFIObject_t *Obj)
{
  MX_WIFI_Status_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if (NULL != Obj)
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == wifi_softap_stop_cmd(addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_Socket_create(MX_WIFIObject_t *Obj, int32_t domain, int32_t type, int32_t protocol)
{
  int32_t ret_fd = -1;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if (NULL != Obj)
  {
    p_addr_ret_val = &ret_fd;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == socket_create_cmd(domain, type, protocol, addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret_fd;
}

int32_t MX_WIFI_Socket_setsockopt(MX_WIFIObject_t *Obj, int32_t sockfd, int32_t level,
                                  int32_t optname, const void *optvalue, int32_t optlen)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (sockfd < 0) || (NULL == optvalue) || (optlen <= 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == socket_setsockopt_cmd(sockfd, level, optname, (const void *)optvalue, optlen,
                                            addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret;
}

int32_t MX_WIFI_Socket_getsockopt(MX_WIFIObject_t *Obj, int32_t sockfd, int32_t level,
                                  int32_t optname, void *optvalue, uint32_t *optlen)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag, addr_optval, addr_optlen;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (sockfd < 0) || (NULL == optvalue) || (NULL == optlen))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    (void)memcpy(&addr_optval, &optvalue, sizeof(addr_optval));
    (void)memcpy(&addr_optlen, &optlen, sizeof(addr_optlen));
    if (IPC_RET_OK == socket_getsockopt_cmd(sockfd, level, optname,
                                            addr_optval, addr_optlen, (int32_t)(*optlen),
                                            addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret;
}

int32_t MX_WIFI_Socket_bind(MX_WIFIObject_t *Obj, int32_t sockfd,
                            const struct sockaddr *addr, int32_t addrlen)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (sockfd < 0) || (NULL == addr) || (addrlen <= 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == socket_bind_cmd(sockfd, (const void *)addr, addrlen, addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret;
}

int32_t MX_WIFI_Socket_listen(MX_WIFIObject_t *Obj, int32_t sockfd, int32_t backlog)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (sockfd < 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == socket_listen_cmd(sockfd, backlog, addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret;
}

int32_t MX_WIFI_Socket_accept(MX_WIFIObject_t *Obj, int32_t sockfd,
                              struct sockaddr *addr, uint32_t *addrlen)
{
  int32_t ret_fd = -1;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag, addr_addr, addr_addrlen;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (sockfd < 0) || (NULL == addr) || (NULL == addrlen))
  {
    ret_fd = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret_fd;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));

    (void)memcpy(&addr_addr, &addr, sizeof(addr_addr));
    (void)memcpy(&addr_addrlen, &addrlen, sizeof(addr_addrlen));
    if (IPC_RET_OK == socket_accept_cmd(sockfd, addr_addr, addr_addrlen, (int32_t)*addrlen,
                                        addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret_fd;
}

int32_t MX_WIFI_Socket_connect(MX_WIFIObject_t *Obj, int32_t sockfd,
                               const struct sockaddr *addr, int32_t addrlen)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (sockfd < 0) || (NULL == addr) || (addrlen <= 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == socket_connect_cmd(sockfd, (const void *)addr, addrlen,
                                         addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret;
}

int32_t MX_WIFI_Socket_shutdown(MX_WIFIObject_t *Obj, int32_t sockfd, int32_t mode)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (sockfd < 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == socket_shutdown_cmd(sockfd, mode, addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret;
}

int32_t MX_WIFI_Socket_close(MX_WIFIObject_t *Obj, int32_t sockfd)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (sockfd < 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == socket_close_cmd(sockfd, addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret;
}

int32_t MX_WIFI_Socket_send(MX_WIFIObject_t *Obj, int32_t sockfd, uint8_t *buf,
                            int32_t len, int32_t flags)
{
  int32_t ret = -1;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;
  int32_t datalen;

  if ((NULL == Obj) || (sockfd < 0) || (NULL == buf) || (len <= 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    datalen = len;
    if (datalen > MX_WIFI_DATA_SIZE)
    {
      datalen = MX_WIFI_DATA_SIZE;
    }

    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == socket_send_cmd(sockfd, buf, (size_t)datalen, flags,
                                      addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret;
}

int32_t MX_WIFI_Socket_sendto(MX_WIFIObject_t *Obj, int32_t sockfd, uint8_t *buf,
                              int32_t len, int32_t flags,
                              struct sockaddr *toaddr, int32_t toaddrlen)
{
  int32_t ret = -1;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;
  int32_t datalen;

  if ((NULL == Obj) || (sockfd < 0) || (NULL == buf) || (len <= 0) || (NULL == toaddr) || (toaddrlen <= 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    datalen = len;
    if (datalen > MX_WIFI_DATA_SIZE)
    {
      datalen = MX_WIFI_DATA_SIZE;
    }

    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == socket_sendto_cmd(sockfd, buf, (size_t)datalen, flags, toaddr, toaddrlen,
                                        addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret;
}

int32_t MX_WIFI_Socket_recv(MX_WIFIObject_t *Obj, int32_t sockfd, uint8_t *buf,
                            int32_t len, int32_t flags)
{
  int32_t ret = -1;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag, addr_buf;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;
  int32_t datalen;

  if ((NULL == Obj) || (sockfd < 0) || (NULL == buf) || (len <= 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    datalen = len;
    if (datalen > MX_WIFI_DATA_SIZE)
    {
      datalen = MX_WIFI_DATA_SIZE;
    }

    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    (void)memcpy(&addr_buf, &buf, sizeof(addr_buf));
    if (IPC_RET_OK == socket_recv_cmd(sockfd, addr_buf, (size_t)datalen, flags,
                                      addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret;
}

int32_t MX_WIFI_Socket_recvfrom(MX_WIFIObject_t *Obj, int32_t sockfd, uint8_t *buf,
                                int32_t len, int32_t flags,
                                struct sockaddr *fromaddr, uint32_t *fromaddrlen)
{
  int32_t ret = -1;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag, addr_buf, addr_fromaddr, addr_fromaddrlen;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;
  int32_t datalen;

  if ((NULL == Obj) || (sockfd < 0) || (NULL == buf) || (len <= 0) || (NULL == fromaddr) \
      || (NULL == fromaddrlen))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    datalen = len;
    if (datalen > MX_WIFI_DATA_SIZE)
    {
      datalen = MX_WIFI_DATA_SIZE;
    }

    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));

    (void)memcpy(&addr_buf, &buf, sizeof(addr_buf));
    (void)memcpy(&addr_fromaddr, &fromaddr, sizeof(addr_fromaddr));
    (void)memcpy(&addr_fromaddrlen, &fromaddrlen, sizeof(addr_fromaddrlen));
    if (IPC_RET_OK == socket_recvfrom_cmd(sockfd, addr_buf, (size_t)datalen, flags,
                                          addr_fromaddr, addr_fromaddrlen,
                                          (int32_t)*fromaddrlen,
                                          addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret;
}

int32_t MX_WIFI_Socket_gethostbyname(MX_WIFIObject_t *Obj, struct sockaddr *addr, char_t *name)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  sockaddr_in_t addr_in;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_flag, addr_sin_addr;
  int32_t *p_addr_ret_flag;
  uint32_t *p_addr_sin_addr;

  if ((NULL == Obj) || (NULL == addr) || (NULL == name) || (strlen(name) > mcMaxDnsNameLen) )
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    (void)memset(&addr_in, 0, sizeof(addr_in));
    p_addr_ret_flag = &ret_flag;
    p_addr_sin_addr = &(addr_in.sin_addr.s_addr);
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    (void)memcpy(&addr_sin_addr, &p_addr_sin_addr, sizeof(addr_sin_addr));
    addr_in.sin_family = AF_INET;
    if (IPC_RET_OK == socket_gethostbyname_cmd(name, (int32_t)strlen(name), addr_sin_addr,
                                               (int32_t)sizeof(addr_in.sin_addr), addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout))
      {
        (void)memcpy(addr, &addr_in, sizeof(struct sockaddr));

        ret = 0;  // success
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_Socket_ping(MX_WIFIObject_t *Obj, const char_t *hostname,
                            int32_t count, int32_t delay, int32_t response[])
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_flag, addr_response;
  int32_t *p_addr_ret_flag, *p_addr_response;

  if ((NULL == Obj) || (NULL == hostname) || (count <= 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_flag = &ret_flag;
    p_addr_response = &(response[0]);
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    (void)memcpy(&addr_response, &p_addr_response, sizeof(addr_response));
    if (IPC_RET_OK == socket_ping_cmd(hostname, (int32_t)strlen(hostname),
                                      count, delay,
                                      addr_response, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, NULL,
                                  (Obj->Runtime.Timeout < ((uint32_t)1000 + ((uint32_t)count * (uint32_t)delay))) ? \
                                  ((uint32_t)1000 + ((uint32_t)count * (uint32_t)delay)) : Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

/* mDNS */
int32_t MX_WIFI_MDNS_start(MX_WIFIObject_t *Obj, const char_t *domain, char_t *hostname)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == mdns_start_cmd(domain, hostname, addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_MDNS_stop(MX_WIFIObject_t *Obj)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == mdns_stop_cmd(addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_MDNS_announce_service(MX_WIFIObject_t *Obj,
                                      struct mc_mdns_service *service, mc_wifi_if_t iface)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (NULL == service))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == mdns_announce_service_cmd(service, (int32_t)sizeof(struct mc_mdns_service),
                                                (int32_t)iface, addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_MDNS_deannounce_service(MX_WIFIObject_t *Obj,
                                        struct mc_mdns_service *service,
                                        mc_wifi_if_t iface)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (NULL == service))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == mdns_deannounce_service_cmd(service, (int32_t)sizeof(struct mc_mdns_service),
                                                  (int32_t)iface, addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_MDNS_deannounce_service_all(MX_WIFIObject_t *Obj, mc_wifi_if_t iface)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == mdns_deannounce_service_all_cmd((int32_t)iface,
                                                      addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_MDNS_iface_state_change(MX_WIFIObject_t *Obj, mc_wifi_if_t iface,
                                        enum iface_state state)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == mdns_iface_state_change_cmd((int32_t)iface, (int32_t)state,
                                                  addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_MDNS_set_hostname(MX_WIFIObject_t *Obj, char_t *hostname)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == mdns_set_hostname_cmd(hostname, (int32_t)strlen(hostname),
                                            addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_MDNS_set_txt_rec(MX_WIFIObject_t *Obj, struct mc_mdns_service *service, char_t *keyvals,
                                 char_t seprator)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (NULL == service))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    (void)strncpy(service->keyvals, keyvals, MDNS_MAX_KEYVAL_LEN);
    service->seprator = seprator;

    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == mdns_set_txt_rec_cmd(service, (int32_t)sizeof(struct mc_mdns_service),
                                           addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

/* TLS */
int32_t MX_WIFI_TLS_set_ver(MX_WIFIObject_t *Obj, mtls_ver_t version)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == tls_set_ver_cmd((int32_t)version, addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_TLS_set_clientCertificate(MX_WIFIObject_t *Obj, uint8_t *client_cert, uint16_t cert_len)
{
	int32_t ret = MX_WIFI_STATUS_ERROR;
	int32_t ret_flag = MIPC_RET_FLAG_CLR;
	int32_t addr_ret_val, addr_ret_flag;
	int32_t *p_addr_ret_val, *p_addr_ret_flag;
	int32_t tmplen;

	if ((NULL == Obj) || (NULL == client_cert) || (cert_len == 0) || (cert_len > (MX_WIFI_DATA_SIZE - 20)))
	{
		ret = MX_WIFI_STATUS_PARAM_ERROR;
	}
	else
	{
		tmplen = cert_len;

		p_addr_ret_val = &ret;
		p_addr_ret_flag = &ret_flag;
		(void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
		(void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
		if (IPC_RET_OK == tls_set_client_certificate_cmd(client_cert, tmplen,
				addr_ret_val, addr_ret_flag))
		{
			(void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
		}
	}
	return ret;
}

int32_t MX_WIFI_TLS_set_clientPrivateKey(MX_WIFIObject_t *Obj, uint8_t *client_private_key, uint16_t key_len)
{
	int32_t ret = MX_WIFI_STATUS_ERROR;
	int32_t ret_flag = MIPC_RET_FLAG_CLR;
	int32_t addr_ret_val, addr_ret_flag;
	int32_t *p_addr_ret_val, *p_addr_ret_flag;
	int32_t tmplen;

	if ((NULL == Obj) || (NULL == client_private_key) || (key_len == 0) || (key_len > (MX_WIFI_DATA_SIZE - 20)))
	{
		ret = MX_WIFI_STATUS_PARAM_ERROR;
	}
	else
	{
		tmplen = key_len;

		p_addr_ret_val = &ret;
		p_addr_ret_flag = &ret_flag;
		(void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
		(void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
		if (IPC_RET_OK == tls_set_client_private_key_cmd(client_private_key, tmplen,
				addr_ret_val, addr_ret_flag))
		{
			(void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
		}
	}
	return ret;
}

int32_t MX_WIFI_TLS_connect(MX_WIFIObject_t *Obj, int32_t domain, int32_t type, int32_t protocol,
                            const struct sockaddr *addr, int32_t addrlen, char_t *ca, int32_t calen)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (NULL == addr) || (addrlen <= 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == tls_connect_cmd(domain, type, protocol, (const void *)addr, addrlen,
                                      ca, (int32_t)calen,
                                      addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, 30000);  // Obj->Runtime.Timeout�� tls connect time longer for bad network
    }

    if (ret < 0) // invalid tls handle
    {
      ret = 0;  // NULL
    }
  }

  return ret;
}

int32_t MX_WIFI_TLS_connect_sni(MX_WIFIObject_t *Obj, const char* sni_servername, int32_t sni_servername_len,
                            const struct sockaddr *addr, int32_t addrlen, char_t *ca, int32_t calen)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (NULL == addr) || (addrlen <= 0) || (sni_servername_len > MX_WIFI_TLS_SNI_SERVERNAME_SIZE))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == tls_connect_sni_cmd(sni_servername, sni_servername_len, (const void *)addr, addrlen,
                                      ca, (int32_t)calen,
                                      addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, 30000);  // Obj->Runtime.Timeout�� tls connect time longer for bad network
    }

    if (ret < 0) // invalid tls handle
    {
      ret = 0;  // NULL
    }
  }

  return ret;
}

int32_t MX_WIFI_TLS_send(MX_WIFIObject_t *Obj, mtls_t tls, void *data, int32_t len)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag, addr_tls;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;
  int32_t datalen;

  if ((NULL == Obj) || (NULL == tls) || (NULL == data) || (len <= 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    datalen = len;
    if (datalen > MX_WIFI_DATA_SIZE)
    {
      datalen = MX_WIFI_DATA_SIZE;
    }

    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    (void)memcpy(&addr_tls, &tls, sizeof(addr_tls));
    if (IPC_RET_OK == tls_send_cmd(addr_tls, data, datalen,
                                   addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret;
}

int32_t MX_WIFI_TLS_recv(MX_WIFIObject_t *Obj, mtls_t tls, void *buf, int32_t len)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag, addr_tls, addr_buf;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;
  int32_t datalen;

  if ((NULL == Obj) || (NULL == tls) || (NULL == buf) || (len <= 0))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    datalen = len;
    if (datalen > MX_WIFI_DATA_SIZE)
    {
      datalen = MX_WIFI_DATA_SIZE;
    }

    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    (void)memcpy(&addr_tls, &tls, sizeof(addr_tls));
    (void)memcpy(&addr_buf, &buf, sizeof(addr_buf));
    if (IPC_RET_OK == tls_recv_cmd(addr_tls, addr_buf, (int32_t)datalen,
                                   addr_ret_val, addr_ret_flag))
    {
      (void)mipc_poll(&ret_flag, NULL, Obj->Runtime.Timeout);
    }
  }
  return ret;
}

int32_t MX_WIFI_TLS_close(MX_WIFIObject_t *Obj, mtls_t tls)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag, addr_tls;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (NULL == tls))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    (void)memcpy(&addr_tls, &tls, sizeof(addr_tls));
    if (IPC_RET_OK == tls_close_cmd(addr_tls, addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_TLS_set_nonblock(MX_WIFIObject_t *Obj, mtls_t tls, int32_t nonblock)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag, addr_tls;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (NULL == tls))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    (void)memcpy(&addr_tls, &tls, sizeof(addr_tls));
    if (IPC_RET_OK == tls_set_nonblock_cmd(addr_tls, (int32_t)nonblock,
                                           addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_Webserver_start(MX_WIFIObject_t *Obj)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == web_start_cmd(addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_Webserver_stop(MX_WIFIObject_t *Obj)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if (NULL == Obj)
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == web_stop_cmd(addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

int32_t MX_WIFI_FOTA_start(MX_WIFIObject_t *Obj, const char_t *url, const char_t *md5,
                           mx_wifi_fota_status_cb_t fota_status_callback, uint32_t user_args)
{
  int32_t ret = MX_WIFI_STATUS_ERROR;
  int32_t ret_flag = MIPC_RET_FLAG_CLR;
  int32_t addr_ret_val, addr_ret_flag;
  int32_t *p_addr_ret_val, *p_addr_ret_flag;

  if ((NULL == Obj) || (NULL == url) || (NULL == md5))
  {
    ret = MX_WIFI_STATUS_PARAM_ERROR;
  }
  else
  {
    if (NULL != fota_status_callback)
    {
      Obj->Runtime.fota_status_cb = fota_status_callback;
      Obj->Runtime.fota_user_args = user_args;
    }
    else
    {
      Obj->Runtime.fota_status_cb = NULL;
      Obj->Runtime.fota_user_args = 0;
    }

    p_addr_ret_val = &ret;
    p_addr_ret_flag = &ret_flag;
    (void)memcpy(&addr_ret_val, &p_addr_ret_val, sizeof(addr_ret_val));
    (void)memcpy(&addr_ret_flag, &p_addr_ret_flag, sizeof(addr_ret_flag));
    if (IPC_RET_OK == fota_start_cmd(url, (int32_t)strlen(url), md5, (int32_t)strlen(md5),
                                     addr_ret_val, addr_ret_flag))
    {
      if (IPC_RET_OK == mipc_poll(&ret_flag, &ret, Obj->Runtime.Timeout))
      {
        ret = MX_WIFI_STATUS_OK;
      }
    }
  }
  return ret;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
