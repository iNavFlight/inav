/**
  ******************************************************************************
  * @file    net_state.c
  * @author  MCD Application Team
  * @brief   Management of state machine for network interfaces
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
#include "stdarg.h"
#ifdef DEBUGSTATE
static const char_t    *eventstr[] =
{
  "NET_EVENT_CMD_INIT",
  "NET_EVENT_CMD_START",
  "NET_EVENT_CMD_CONNECT",
  "NET_EVENT_CMD_DISCONNECT",
  "NET_EVENT_CMD_STOP",
  "NET_EVENT_CMD_DEINIT",
  "NET_EVENT_INTERFACE_INITIALIZED",
  "NET_EVENT_INTERFACE_READY",
  "NET_EVENT_LINK_UP",
  "NET_EVENT_LINK_DOWN",
  "NET_EVENT_IPADDR",
};


static const char_t   *statestr[] =
{
  "NET_STATE_DEINITIALIZED",
  "NET_STATE_INITIALIZED",
  "NET_STATE_STARTING",
  "NET_STATE_READY",
  "NET_STATE_CONNECTING",
  "NET_STATE_CONNECTED",
  "NET_STATE_STOPPING",
  "NET_STATE_DISCONNECTING",
  "NET_STATE_CONNECTION_LOST",
};
#endif /* DEBUGSTATE */

static void   set_state(net_if_handle_t *pnetif, net_state_t state)
{
  pnetif->state = state;
  net_if_notify(pnetif, NET_EVENT_STATE_CHANGE, (uint32_t) state, NULL);
  SIGNAL_STATE_CHANGE();
}


static  int32_t net_state_initialized(net_if_handle_t *pnetif, net_state_event_t event)
{
  int32_t       ret = NET_OK;
  switch (event)
  {
    case  NET_EVENT_CMD_START:
      set_state(pnetif, NET_STATE_STARTING);
      ret = pnetif->pdrv->if_start(pnetif);
      if (NET_OK != ret)
      {
        NET_DBG_ERROR("Interface cannot be started.");
        ret = NET_ERROR_INTERFACE_FAILURE;
      }
      break;

    case  NET_EVENT_CMD_DEINIT:
      ret = pnetif->pdrv->if_deinit(pnetif);
      set_state(pnetif, NET_STATE_DEINITIALIZED);
      if (NET_OK != ret)
      {
        NET_DBG_ERROR("Interface cannot be deinitialized.");
        ret = NET_ERROR_INTERFACE_FAILURE;
      }
      break;


    default:
      break;
  }
  return ret;
}

static  int32_t net_state_starting(net_if_handle_t *pnetif, net_state_event_t  event)
{
  int32_t       ret = NET_OK;
  switch (event)
  {
    case  NET_EVENT_INTERFACE_READY:
      set_state(pnetif, NET_STATE_READY);
      break;

    default:
      break;
  }
  return ret;
}

static  int32_t net_state_ready(net_if_handle_t *pnetif, net_state_event_t event)
{
  int32_t       ret = NET_OK;
  switch (event)
  {
    case  NET_EVENT_CMD_CONNECT:
      set_state(pnetif, NET_STATE_CONNECTING);
      ret = pnetif->pdrv->if_connect(pnetif);
      if (NET_OK != ret)
      {
        NET_DBG_ERROR("Interface cannot connect.");
        ret = NET_ERROR_INTERFACE_FAILURE;
      }
      break;

    case  NET_EVENT_CMD_STOP:
      set_state(pnetif, NET_STATE_STOPPING);
      ret = pnetif->pdrv->if_stop(pnetif);
      if (NET_OK != ret)
      {
        NET_DBG_ERROR("Interface cannot stop.");
        ret = NET_ERROR_INTERFACE_FAILURE;
      }
      break;

    default:
      break;
  }
  return ret;
}

static  int32_t net_state_connecting(net_if_handle_t *pnetif, net_state_event_t  event)
{
  int32_t       ret = NET_OK;
  switch (event)
  {
    case NET_EVENT_IPADDR:
      set_state(pnetif, NET_STATE_CONNECTED);
      break;

    case NET_EVENT_CMD_DISCONNECT:
      set_state(pnetif, NET_STATE_READY);
      break;

    default:
      break;
  }
  return ret;
}

static  int32_t net_state_connected(net_if_handle_t *pnetif, net_state_event_t event)
{
  int32_t       ret = NET_OK;
  switch (event)
  {
    case  NET_EVENT_CMD_DISCONNECT:
      set_state(pnetif, NET_STATE_DISCONNECTING);
      ret = pnetif->pdrv->if_disconnect(pnetif);
      if (NET_OK != ret)
      {
        NET_DBG_ERROR("Interface cannot disconnect.");
        ret = NET_ERROR_INTERFACE_FAILURE;
      }
      break;

    case  NET_EVENT_LINK_DOWN:
      set_state(pnetif, NET_STATE_CONNECTION_LOST);
      break;

    default:
      break;
  }
  return ret;
}

static  int32_t net_state_disconnecting(net_if_handle_t *pnetif, net_state_event_t event)
{
  int32_t       ret = NET_OK;
  switch (event)
  {
    case  NET_EVENT_INTERFACE_READY:
      set_state(pnetif, NET_STATE_READY);
      break;

    default:
      break;
  }
  return ret;
}

static  int32_t net_state_stopping(net_if_handle_t *pnetif, net_state_event_t  event)
{
  int32_t       ret = NET_OK;
  switch (event)
  {
    case  NET_EVENT_INTERFACE_INITIALIZED:
      set_state(pnetif, NET_STATE_INITIALIZED);
      break;

    default:
      break;
  }
  return ret;
}

static  int32_t net_state_connection_lost(net_if_handle_t *pnetif, net_state_event_t event)
{
  int32_t       ret = NET_OK;
  switch (event)
  {

    case  NET_EVENT_LINK_UP:
      set_state(pnetif, NET_STATE_CONNECTED);
      break;

    default:
      break;
  }
  return ret;
}




int32_t net_state_manage_event(net_if_handle_t *pnetif_in, net_state_event_t event)
{
  int32_t       ret;
  net_if_handle_t *pnetif;

  pnetif = netif_check(pnetif_in);
  if (pnetif == NULL)
  {
    NET_DBG_ERROR("Invalid interface.");
    ret = NET_ERROR_PARAMETER;
  }
  else
  {
#ifdef DEBUGSTATE
    printf("In state %s , received event %s\n", statestr[pnetif->state], eventstr[event]);
#endif /* DEBUGSTATE */
    switch (pnetif->state)
    {
      case  NET_STATE_INITIALIZED:
        ret = net_state_initialized(pnetif, event);
        break;

      case  NET_STATE_STARTING:
        ret = net_state_starting(pnetif, event);
        break;

      case  NET_STATE_READY:
        ret = net_state_ready(pnetif, event);
        break;

      case  NET_STATE_CONNECTING:
        ret = net_state_connecting(pnetif, event);
        break;

      case  NET_STATE_CONNECTED:
        ret = net_state_connected(pnetif, event);
        break;
      case  NET_STATE_DISCONNECTING:
        ret = net_state_disconnecting(pnetif, event);
        break;

      case  NET_STATE_CONNECTION_LOST:
        ret = net_state_connection_lost(pnetif, event);
        break;

      case  NET_STATE_STOPPING:
        ret = net_state_stopping(pnetif, event);
        break;

      default:
        ret = NET_ERROR_INVALID_STATE;
        break;
    }
  }
  return ret;
}


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

