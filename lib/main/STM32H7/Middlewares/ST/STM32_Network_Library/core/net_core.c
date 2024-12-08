/**
  ******************************************************************************
  * @file    net_core.c
  * @author  MCD Application Team
  * @brief   Network interface core implementation
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
static void netif_add_to_list(net_if_handle_t *pnetif);
static void netif_remove_from_list(net_if_handle_t *pnetif);
static net_if_handle_t *net_if_list = NULL;

#ifndef __IO
/*cstat -MISRAC2012-Rule-21.1 */
#define __IO volatile
/*cstat +MISRAC2012-Rule-21.1 */
#endif /* IO */

static void netif_add_to_list(net_if_handle_t *pnetif)
{
  LOCK_NETIF_LIST();
  if (net_if_list == NULL)
  {
    net_if_list = pnetif;
  }
  else
  {
    /*add it to end of the list*/
    net_if_handle_t *plastnetif;
    plastnetif = net_if_list;
    while (plastnetif->next != NULL)
    {
      plastnetif = plastnetif->next;
    }
    plastnetif->next = pnetif;
  }
  UNLOCK_NETIF_LIST();
}


static void netif_remove_from_list(net_if_handle_t *pnetif)
{
  net_if_handle_t       *pnetif_prev;
  LOCK_NETIF_LIST();

  if (net_if_list == pnetif)
  {
    net_if_list = net_if_list->next;
  }
  else
  {
    for (pnetif_prev = net_if_list; pnetif_prev->next != NULL; pnetif_prev = pnetif_prev->next)
    {
      if (pnetif_prev->next == pnetif)
      {
        pnetif_prev->next = pnetif->next;
        break;
      }
    }
  }
  UNLOCK_NETIF_LIST();
}



/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
net_if_handle_t *net_if_find(net_sockaddr_t *addr)
{
  net_if_handle_t *ptr;
  net_ip_addr_t ipaddr;
  net_ip_addr_t ipaddr_zero;

  NET_ZERO(ipaddr_zero);
  NET_ZERO(ipaddr);
  if (addr != NULL)
  {
    ipaddr = net_get_ip_addr(addr);

  }
  LOCK_NETIF_LIST();

  ptr = net_if_list;

  if (NET_DIFF(ipaddr, ipaddr_zero) != 0)
  {
    do
    {
      if (NET_EQUAL(ptr->ipaddr, ipaddr))
      {
        break;
      }
      ptr = ptr->next;
    } while (ptr != NULL);
  }
  UNLOCK_NETIF_LIST();
  return ptr;
}


net_if_handle_t *netif_check(net_if_handle_t *pnetif_in)
{
  net_if_handle_t *pnetif = pnetif_in;
  if (pnetif == NULL)
  {
    /* get default interface*/
    pnetif = net_if_find(NULL);
    if (pnetif == NULL)
    {
      NET_DBG_ERROR("No network interface defined");
    }
  }
  return pnetif;
}




/**
  * @brief  Wait for state transtion
  * @param  pnetif a pointer to the selected network interface
  * @param  state  the expected state
  * @param  timeout max time to wait in ms for the transition
  * @retval 0 in case of success, an error code otherwise
  */
extern uint32_t HAL_GetTick(void);

int32_t net_if_wait_state(net_if_handle_t *pnetif, net_state_t state, uint32_t timeout)
{
  int32_t  ret = NET_OK;
  __IO net_state_t      *p;
  p = &pnetif->state;

  uint32_t start_time = HAL_GetTick();
  while (*p != state)
  {
    if (HAL_GetTick() >= (start_time + timeout))
    {
      ret = NET_TIMEOUT;
      break;
    }
    WAIT_STATE_CHANGE(timeout);
  }
  return ret;

}




void net_if_notify(net_if_handle_t *pnetif, net_evt_t event_class, uint32_t event_id, void  *event_data)
{
  /* call the user Handler first ,FIXME , first or not , race between wait state transition and user handler */
  if ((NULL != pnetif->event_handler) && (NULL != pnetif->event_handler->callback))
  {
    pnetif->event_handler->callback(pnetif->event_handler->context, event_class, event_id, event_data);
  }
}


#ifdef NET_USE_RTOS
static int32_t net_initialized = 0;
#endif /* NET_USE_RTOS */

/** @defgroup State State Management Network Framework
  * Application uses this set of function to control the network interface state.
  * Normal state flow is init => start => connect.
  * Socket interface can be used when connected state is reached.Network interface is connected and got an IP address.
  * To finish, flow is disconnect,stop,deinit. All connection are closed and allocated resources are freed.
  * State transition are asynchronous but could be implemented in a synchronous way depending on the selected
  * network interface. Once a transition is requested,the net_wait_state primitive should be used to wait
  * for transition to occur.
  * @{
  */


/**
  * @brief  Perform network interface initialization
  * @param  pnetif a pointer to an allocated network interface structure
  * @param  driver_init a pointer to a function which define the driver to use
  * @param  event_handler  a calback function to manage event from network framework
  * @retval 0 in case of success, an error code otherwise
   * This function is a synchronous function.
  */
int32_t net_if_init(net_if_handle_t *pnetif_in, net_if_driver_init_func driver_init,
                    const net_event_handler_t *event_handler)
{
  int32_t ret;
  net_if_handle_t *pnetif = pnetif_in;
#ifdef NET_USE_RTOS
  if (net_initialized == 0)
  {
    net_init_locks();
    net_initialized = 1;
  }
#endif /* NET_USE_RTOS */

  if (pnetif != NULL)
  {
    pnetif->event_handler = event_handler;
    pnetif->state = NET_STATE_INITIALIZED;
    netif_add_to_list(pnetif);
    ret = (*driver_init)(pnetif);
    if (NET_OK != ret)
    {
      NET_DBG_ERROR("Interface cannot be initialized.");
      ret = NET_ERROR_INTERFACE_FAILURE;
    }
  }
  else
  {
    NET_DBG_ERROR("Invalid interface.");
    ret = NET_ERROR_PARAMETER;
  }
  return ret;
}

/**
  * @brief  Perform network interface de-initialization
  * @param  pnetif a pointer to an allocated network interface structure
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_deinit(net_if_handle_t *pnetif)
{
  int32_t ret;
  ret = net_state_manage_event(pnetif, NET_EVENT_CMD_DEINIT);
  pnetif->state = NET_STATE_DEINITIALIZED;
  if (ret == NET_OK)
  {
    netif_remove_from_list(pnetif);
  }

#ifdef NET_USE_RTOS
  if (net_initialized == 1)
  {
    net_destroy_locks();
    net_initialized = 0;
  }
#endif /* NET_USE_RTOS */

  return ret;
}

/**

  * @brief  Start network interface
  * @param  pnetif a pointer to an allocated network interface structure
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_start(net_if_handle_t *pnetif)
{
  return net_state_manage_event(pnetif, NET_EVENT_CMD_START);
}

/**
  * @brief  Stop network interface
  * @param  pnetif a pointer to an allocated network interface structure
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_stop(net_if_handle_t *pnetif)
{
  return net_state_manage_event(pnetif, NET_EVENT_CMD_STOP);
}

/**

  * @brief  Yield data from network interface
  * @param  pnetif a pointer to an allocated network interface structure
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_yield(net_if_handle_t *pnetif_in, uint32_t timeout)
{
  int32_t ret = NET_OK;
  net_if_handle_t *pnetif;
  net_state_t state;

  pnetif = netif_check(pnetif_in);
  if (pnetif != NULL)
  {
    (void) net_if_getState(pnetif, &state);

    if (state == NET_STATE_CONNECTED)
    {
      if (NULL != pnetif->pdrv->if_yield)
      {
        ret = pnetif->pdrv->if_yield(pnetif, timeout);
      }
      if (ret != NET_OK)
      {
        NET_DBG_ERROR("Interface yield failed!!!");
        ret = NET_ERROR_STATE_TRANSITION;
      }
    }
    else
    {
      NET_DBG_ERROR("Incorrect requested State transition");
      ret = NET_ERROR_INVALID_STATE_TRANSITION;
    }
  }
  else
  {
    NET_DBG_ERROR("Invalid interface.");
    ret = NET_ERROR_PARAMETER;
  }
  return ret;
}

/**

  * @brief  Connect network interface
  * @param  pnetif a pointer to an allocated network interface structure
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_connect(net_if_handle_t *pnetif)
{
  return net_state_manage_event(pnetif, NET_EVENT_CMD_CONNECT);
}

/**

  * @brief  Disconnect network interface
  * @param  pnetif a pointer to an allocated network interface structure
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_disconnect(net_if_handle_t *pnetif)
{
  return net_state_manage_event(pnetif, NET_EVENT_CMD_DISCONNECT);
}

/**
  * @brief  get network interface state
  * @param  pnetif a pointer to an allocated network interface structure
  * @param  state  a pointer to a net_state_t enum
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_getState(net_if_handle_t *pnetif_in, net_state_t *state)
{
  int32_t ret;
  net_if_handle_t *pnetif;

  pnetif = netif_check(pnetif_in);
  if (pnetif != NULL)
  {
    *state = pnetif->state;
    ret = NET_OK;
  }
  else
  {
    NET_DBG_ERROR("Invalid interface.");
    ret = NET_ERROR_PARAMETER;
  }
  return ret;
}

/** @defgroup State
  * @}
  */


/**
  * @brief  Enable power save mode
  * @param  pnetif a pointer to an allocated network interface structure
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_powersave_enable(net_if_handle_t *pnetif_in)
{
  int32_t ret;
  net_if_handle_t *pnetif;
  pnetif = netif_check(pnetif_in);
  if (pnetif != NULL)
  {
    if (pnetif->state == NET_STATE_CONNECTED)
    {
      ret = pnetif->pdrv->if_powersave_enable(pnetif);
    }
    else
    {
      NET_DBG_ERROR("Power-save cannot be enabled when the device is not connected");
      ret = NET_ERROR_INVALID_STATE_TRANSITION;
    }
  }
  else
  {
    ret = NET_ERROR_PARAMETER;
    NET_DBG_ERROR("Invalid interface.");
  }
  return ret;
}

/** @defgroup GetAndSet Get and Set Network Interface information
  * @{
  */


/**
  * @brief  get MAC address
  * @param  pnetif a pointer to an allocated network interface structure
  * @param  mac a pointer to an allocated macaddr_t structure
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_get_mac_address(net_if_handle_t *pnetif_in, macaddr_t *mac)
{
  int32_t ret;
  net_if_handle_t *pnetif;

  pnetif = netif_check(pnetif_in);
  if (pnetif != NULL)
  {
    if (NET_STATE_DEINITIALIZED != pnetif->state)
    {
      (void) memcpy(mac, &pnetif->macaddr, sizeof(macaddr_t));
      ret = NET_OK;
    }
    else
    {
      ret = NET_ERROR_INTERFACE_FAILURE;
      NET_DBG_ERROR("Interface not yet initialized or in error state");
    }
  }
  else
  {
    NET_DBG_ERROR("Invalid interface.");
    ret = NET_ERROR_PARAMETER;
  }
  return ret;
}

/**
  * @brief  get IP address
  * @param  pnetif a pointer to an allocated network interface structure
  * @param  ip a pointer to an allocated net_ip_addr_t structure
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_get_ip_address(net_if_handle_t *pnetif_in, net_ip_addr_t *ip)
{
  int32_t ret;
  net_if_handle_t *pnetif;

  pnetif = netif_check(pnetif_in);
  if (pnetif != NULL)
  {
    if (pnetif->state == NET_STATE_CONNECTED)
    {
      *ip = pnetif->ipaddr;
      ret = NET_OK;
    }
    else
    {
      NET_DBG_ERROR("Can get ipaddr for un connected network interface");
      ret = NET_ERROR_INTERFACE_FAILURE;
    }
  }
  else
  {
    NET_DBG_ERROR("Invalid interface.");
    ret = NET_ERROR_PARAMETER;
  }
  return ret;
}


/**
  * @brief  get host by name
  * @param  pnetif a pointer to an allocated network interface structure
  * @param  name is a pointer to the hostname string
  * @param  addr is a pointer to the structure net_sockaddr_t
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_gethostbyname(net_if_handle_t *pnetif_in, net_sockaddr_t *addr, char_t *name)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_if_handle_t *pnetif;

  pnetif = netif_check(pnetif_in);
  if (pnetif != NULL)
  {
    ret =  pnetif->pdrv->pgethostbyname(pnetif, addr, name);
  }
  return ret;
}

/**
  * @brief  ping a remote machine
  * @param  pnetif a pointer to an allocated network interface structure
  * @param  addr is a pointer to the socketaddr of the remote host
  * @param  count is an integer, number of iteration to ping the remote machine
  * @param  delay is an integer, maximum delay in millisecond to wait for remote answer
  * @param  response is an array of <count> integer, containing the time to get response for each iteration.
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_ping(net_if_handle_t *pnetif_in, net_sockaddr_t *addr, int32_t count, int32_t delay, int32_t response[])
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_if_handle_t *pnetif;

  pnetif = netif_check(pnetif_in);
  if (pnetif != NULL)
  {
    ret =  pnetif->pdrv->pping(pnetif, addr, count, delay, response);
  }
  return ret;
}

/**
  * @brief  enable or disable dhcp mode
  * @param  pnetif a pointer to an allocated network interface structure
  * @param  mode is a boolean , true to activate DHCP
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_set_dhcp_mode(net_if_handle_t *pnetif_in, bool mode)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_if_handle_t *pnetif;

  pnetif = netif_check(pnetif_in);
  if (pnetif != NULL)
  {
    pnetif->dhcp_mode = mode;
    ret = NET_OK;
  }
  return ret;
}

/**
  * @brief  setting ipaddr , gateway and netmask forcurrent network interface
  * @param  pnetif a pointer to an allocated network interface structure
  * @param  ipaddr is a pointer to and net_ip_addr_t structure used as ip address
  * @param  gateway is a pointer to the net_ip_addr_t structure used as gateway address
  * @param  netmask is a pointer to the net_ip_addr_t structure used as the netmask
  * @retval 0 in case of success, an error code otherwise
  */
int32_t net_if_set_ipaddr(net_if_handle_t *pnetif_in, net_ip_addr_t ipaddr,
                          net_ip_addr_t gateway, net_ip_addr_t netmask)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_if_handle_t *pnetif;

  pnetif = netif_check(pnetif_in);
  if (pnetif != NULL)
  {
    pnetif->static_ipaddr  = ipaddr;
    pnetif->static_gateway = gateway;
    pnetif->static_netmask = netmask;
    ret = NET_OK;
  }
  return ret;
}

/** @defgroup GetAndSet
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

