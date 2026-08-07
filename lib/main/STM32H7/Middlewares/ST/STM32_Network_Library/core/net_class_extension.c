/**
  ******************************************************************************
  * @file    net_class_extension.c
  * @author  MCD Application Team
  * @brief   Specific class interface function implementation
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
#define MATCH(a,b) (a & (b) == (b))



/**
  * @brief  Convert wifi security enum value to string
  * @param  sec is an unsigned integer
  * @retval a constant string , for instance "Open" or "WPA2-AES"
  */
const char_t *net_wifi_security_to_string(uint32_t sec)
{
  const char_t *s;
  if (sec == NET_WIFI_SM_OPEN)
  {
    s =  "Open";
  }
  else if (sec == NET_WIFI_SM_WEP_SHARED)
  {
    s = "WEP-shared";
  }
  else if (sec == NET_WIFI_SM_WPA_TKIP_PSK)
  {
    s = "WPA-TKIP";
  }
  else if (sec == NET_WIFI_SM_WPA_MIXED_PSK)
  {
    s = "WPA-Mixed";
  }
  else if (sec == NET_WIFI_SM_WPA2_AES_PSK)
  {
    s = "WPA2-AES";
  }
  else if (sec == NET_WIFI_SM_WPA2_TKIP_PSK)
  {
    s = "WPA2-TKIP";
  }
  else if (sec == NET_WIFI_SM_WPA2_MIXED_PSK)
  {
    s = "WPA2_Mixed";
  }
  else if (sec == NET_WIFI_SM_WPA2_FBT_PSK)
  {
    s = "WPA2-FBT";
  }
  else if (sec == NET_WIFI_SM_WPA3_SAE)
  {
    s = "WPA3";
  }
  else if (sec == NET_WIFI_SM_WPA3_WPA2_PSK)
  {
    s = "WPA3-WPA2";
  }

  else if (sec == NET_WIFI_SM_WPA_TKIP_ENT)
  {
    s = "WPA-TKIP-Ent";
  }
  else if (sec == NET_WIFI_SM_WPA_AES_ENT)
  {
    s = "WPA-AES-Ent";
  }
  else if (sec == NET_WIFI_SM_WPA2_TKIP_ENT)
  {
    s = "WPA2-TKIP-Ent";
  }
  else if (sec == NET_WIFI_SM_WPA2_AES_ENT)
  {
    s = "WPA2-AES-Ent";
  }
  else if (sec == NET_WIFI_SM_WPA2_MIXED_ENT)
  {
    s = "WPA2-Mixed-Ent";
  }
  else if (sec == NET_WIFI_SM_WPA2_FBT_ENT)
  {
    s = "WPA-FBT-Ent";
  }

  else if (sec == NET_WIFI_SM_IBSS_OPEN)
  {
    s = "IBS";
  }
  else if (sec == NET_WIFI_SM_WPS_OPEN)
  {
    s = "WPS";
  }
  else if (sec == NET_WIFI_SM_WPS_SECURE)
  {
    s = "WPS-AES";
  }
  else
  {
    s = "unknown";
  }
  return s;
}

/**
  * @brief  Convert a string to a security enum value
  * @param  sec is a pointer to a string
  */
uint32_t net_wifi_string_to_security(char_t *sec)
{
  uint32_t       ret = NET_WIFI_SM_UNKNOWN;
  if (strcmp(sec, "Open") == 0)
  {
    ret = NET_WIFI_SM_OPEN;
  }
  if (strcmp(sec, "WEP-shared") == 0)
  {
    ret = NET_WIFI_SM_WEP_SHARED;
  }
  if (strcmp(sec, "WPA-TKIP") == 0)
  {
    ret = NET_WIFI_SM_WPA_TKIP_PSK;
  }
  if (strcmp(sec, "WPA-Mixed") == 0)
  {
    ret = NET_WIFI_SM_WPA_MIXED_PSK;
  }
  if (strcmp(sec, "WPA2-AES") == 0)
  {
    ret = NET_WIFI_SM_WPA2_AES_PSK;
  }
  if (strcmp(sec, "WPA2-TKIP") == 0)
  {
    ret = NET_WIFI_SM_WPA2_TKIP_PSK;
  }
  if (strcmp(sec, "WPA2-Mixed") == 0)
  {
    ret = NET_WIFI_SM_WPA2_MIXED_PSK;
  }
  if (strcmp(sec, "WPA2-DBT") == 0)
  {
    ret = NET_WIFI_SM_WPA2_FBT_PSK;
  }
  if (strcmp(sec, "WPA3") == 0)
  {
    ret = NET_WIFI_SM_WPA3_SAE;
  }
  if (strcmp(sec, "WPA3-WPA2") == 0)
  {
    ret = NET_WIFI_SM_WPA3_WPA2_PSK;
  }

  if (strcmp(sec, "WPA-TKIP-Ent") == 0)
  {
    ret = NET_WIFI_SM_WPA_TKIP_ENT;
  }
  if (strcmp(sec, "WPA-AES-Ent") == 0)
  {
    ret = NET_WIFI_SM_WPA_AES_ENT;
  }
  if (strcmp(sec, "WPA2-TKIP-Ent") == 0)
  {
    ret = NET_WIFI_SM_WPA2_TKIP_ENT;
  }
  if (strcmp(sec, "WPA2-AES-Ent") == 0)
  {
    ret = NET_WIFI_SM_WPA2_AES_ENT;
  }
  if (strcmp(sec, "WPA2-Mixed-Ent") == 0)
  {
    ret = NET_WIFI_SM_WPA2_MIXED_ENT;
  }
  if (strcmp(sec, "WPA-FBT-Ent") == 0)
  {
    ret = NET_WIFI_SM_WPA2_FBT_ENT;
  }
  if (strcmp(sec, "IBS") == 0)
  {
    ret = NET_WIFI_SM_IBSS_OPEN;
  }
  if (strcmp(sec, "WPS") == 0)
  {
    ret = NET_WIFI_SM_WPS_OPEN;
  }
  if (strcmp(sec, "WPS-AES") == 0)
  {
    ret = NET_WIFI_SM_WPS_SECURE;
  }

  return ret;
}

/**
  * @brief  start a wifi scan operation
  * @param  pnetif_in is  a pointer to an allocated network interface structure
  * @param  mode is  an enum to specify type of scan mode to be performed
  * @param  ssid is a pointer to a string, when not null, scan searches only this ssid
  * @retval return the number of found access point , max value is "number".
   * This function is a synchronous function.
  */

int32_t net_wifi_scan(net_if_handle_t *pnetif_in, net_wifi_scan_mode_t mode, char_t *ssid)
{
  int32_t ret = NET_OK;
  net_if_handle_t *pnetif;

  pnetif = netif_check(pnetif_in);
  if (pnetif == NULL)
  {
    NET_DBG_ERROR("No network interface defined");
    ret = NET_ERROR_PARAMETER;
  }
  else if (pnetif->pdrv->if_class != NET_INTERFACE_CLASS_WIFI)
  {
    NET_DBG_ERROR("Incorrect class interface when calling net_wifi_scan function\n");
    ret = NET_ERROR_PARAMETER;
  }
  else
  {
    if (pnetif->pdrv->extension.wifi->scan(pnetif, mode, ssid) != NET_OK)
    {
      NET_DBG_ERROR("Error when executing net_wifi_scan function\n");
      ret = NET_ERROR_GENERIC;
    }
  }

  return ret;
}

/**
  * @brief  Get the result of scan operation , once event has been recevied
  * @param  pnetif_is a pointer to an allocated network interface structure
  * @param  results is a pointer to an allocated array of net_wifi_scan_results_t
  * @param  number is unsigned integer , size of the array 'results'
  * @retval return the number of found access point , max value is "number".
  */
int32_t net_wifi_get_scan_results(net_if_handle_t *pnetif_in, net_wifi_scan_results_t *results, uint8_t number)
{
  int32_t ret;
  net_if_handle_t *pnetif;

  pnetif = netif_check(pnetif_in);
  if (pnetif == NULL)
  {
    NET_DBG_ERROR("No network interface defined");
    ret = NET_ERROR_PARAMETER;
  }
  else if (pnetif->pdrv->if_class != NET_INTERFACE_CLASS_WIFI)
  {
    NET_DBG_ERROR("Incorrect class interface when calling net_wifi_scan function\n");
    ret = NET_ERROR_PARAMETER;
  }
  else
  {
    ret = pnetif->pdrv->extension.wifi->get_scan_results(pnetif, results, number);
  }
  return ret;
}

/**
  * @brief  set the credential of a wifi interface, can be AP or STA credentials
  * @param  pnetif_is a pointer to an allocated network interface structure
  * @param  credentials a pointer to a const allocated structure which contain credentials values (ssid , passwd)
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_wifi_set_credentials(net_if_handle_t *pnetif, const net_wifi_credentials_t *credentials)
{
  pnetif->pdrv->extension.wifi->credentials = credentials;
  return NET_OK;
}

/**
  * @brief  set the acess mode for a wifi interface: AP or STA mode
  * @param  pnetif_is a pointer to an allocated network interface structure
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_wifi_set_access_mode(net_if_handle_t *pnetif, net_wifi_mode_t mode)
{
  pnetif->pdrv->extension.wifi->mode = mode;
  return NET_OK;
}

/**
  * @brief  set the wifi channel to used for an AP
  * @param  pnetif_is a pointer to an allocated network interface structure
  * @param  channel is an unsigned 8 bit integer
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_wifi_set_access_channel(net_if_handle_t *pnetif,  uint8_t channel)
{
  pnetif->pdrv->extension.wifi->access_channel = channel;
  return NET_OK;
}

/**
  * @brief  set wifi power save mode
  * @param  pnetif_is a pointer to an allocated network interface structure
  * @param  powersave is a pointer to an allocated structute to define the powersave mode
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_wifi_set_powersave(net_if_handle_t *pnetif_in, const net_wifi_powersave_t *powersave)
{
  int32_t ret = NET_OK;
  net_if_handle_t *pnetif;
  pnetif = netif_check(pnetif_in);
  if (pnetif == NULL)
  {
    NET_DBG_ERROR("No network interface defined");
    ret =  NET_ERROR_PARAMETER;
  }
  else
  {
    if (pnetif->pdrv->if_class != NET_INTERFACE_CLASS_WIFI)
    {
      NET_DBG_ERROR("Incorrect class interface when calling net_wifi_set_powersave function\n");
      ret = NET_ERROR_PARAMETER;
    }
    else
    {
      pnetif->pdrv->extension.wifi->powersave = powersave;
    }
  }
  return ret;
}

/**
  * @brief  set wifi extension parameter
  * @param  pnetif_is a pointer to an allocated network interface structure
  * @param  param is an enum value to specify which parameter to set
  * @param  data is a pointer to an allocated opaque structure to specify the parameter value
  * @retval 0 in case of success, an error code otherwise
  */

int32_t net_wifi_set_param(net_if_handle_t *pnetif, const net_wifi_param_t param, void *data)
{
  int32_t ret;

  if (pnetif->pdrv->if_class != NET_INTERFACE_CLASS_WIFI)
  {
    NET_DBG_ERROR("Incorrect class interface when calling net_wifi_set_param function\n");
    ret = NET_ERROR_PARAMETER;
  }
  else
  {
    ret = pnetif->pdrv->extension.wifi->set_param(param, data);
  }
  return ret;
}


/**
  * @brief  set the credential of a cellular interface
  * @param  pnetif_is a pointer to an allocated network interface structure
  * @param  credentials a pointer to a const allocated structure which contains credentials values (ssid , passwd)
  * @retval 0 in case of success, an error code otherwise
  */

int32_t net_cellular_set_credentials(net_if_handle_t *pnetif_in, const net_cellular_credentials_t *credentials)
{
  int32_t ret;
  net_if_handle_t *pnetif;

  pnetif = netif_check(pnetif_in);
  if (pnetif == NULL)
  {
    NET_DBG_ERROR("No network interface defined");
    ret = NET_ERROR_PARAMETER;
  }
  else
  {
    if (pnetif->pdrv->if_class != NET_INTERFACE_CLASS_CELLULAR)
    {
      NET_DBG_ERROR("Incorrect class interface when calling net_cellular_get_radio_results function\n");
      ret = NET_ERROR_PARAMETER;
    }
    else
    {
      pnetif->pdrv->extension.cellular->credentials = credentials;
      ret = NET_OK;
    }
  }
  return ret;
}

/**
  * @brief  get cellular radio information
  * @param  pnetif_is a pointer to an allocated network interface structure
  * @param  reults is a pointer to an allocated net_cellular_radio_results_t structure
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_cellular_get_radio_results(net_if_handle_t *pnetif_in, net_cellular_radio_results_t *results)
{
  int32_t ret;

  net_if_handle_t *pnetif;
  pnetif = netif_check(pnetif_in);
  if (pnetif == NULL)
  {
    NET_DBG_ERROR("No network interface defined");
    ret = NET_ERROR_PARAMETER;
  }
  else
  {
    if (pnetif->pdrv->if_class != NET_INTERFACE_CLASS_CELLULAR)
    {
      NET_DBG_ERROR("Incorrect class interface when calling net_cellular_get_radio_results function\n");
      ret = NET_ERROR_PARAMETER;
    }
    else
    {
      ret = pnetif->pdrv->extension.cellular->get_radio_results(results);
    }
  }
  return ret;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
