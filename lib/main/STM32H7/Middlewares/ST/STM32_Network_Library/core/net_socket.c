/**
  ******************************************************************************
  * @file    net_socket.c
  * @author  MCD Application Team
  * @brief   BSD Like Socket APIs
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

/* Includes ------------------------------------------------------------------*/
#include "net_connect.h"
#include "net_internals.h"

#ifndef NET_BYPASS_NET_SOCKET
#define OPTCHECKTYPE(type,optlen)       if (sizeof(type)!= optlen) {ret = NET_ERROR_PARAMETER; break;}


#define OPTCHECKSTRING(opt,optlen)       if (strlen(opt)!= (optlen-1U)) { ret = NET_ERROR_PARAMETER;break;}


static int32_t create_low_level_socket(int32_t sock);
static int32_t check_low_level_socket(int32_t sock);
static int32_t clone_socket(int32_t sock);

static net_socket_t sockets[NET_MAX_SOCKETS_NBR] = {0};


static net_socket_t *net_socket_get_and_lock(int32_t sock)
{
  net_socket_t *pSocket;
  LOCK_SOCK(sock);
  pSocket = &sockets[sock];
  return    pSocket;
}

/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
static bool is_valid_socket(int32_t socketnum)
{
  bool ret = false;
  if ((socketnum >= 0) && (socketnum < (int32_t) NET_MAX_SOCKETS_NBR))
  {
    if (sockets[socketnum].status != SOCKET_NOT_ALIVE)
    {
      ret = true;
    }
  }
  return ret;
}


/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
static int32_t create_low_level_socket(int32_t sock)
{
  net_socket_t *pSocket;
  pSocket = &sockets[sock];
  if (pSocket->ulsocket == -1)
  {
    if (net_access_control(pSocket->pnetif, NET_ACCESS_SOCKET, &pSocket->ulsocket))
    {
      pSocket->ulsocket = pSocket->pnetif->pdrv->psocket(sockets[sock].domain,
                                                         sockets[sock].type,
                                                         sockets[sock].protocol);
    }
  }
  return pSocket->ulsocket;
}

static int32_t check_low_level_socket(int32_t sock)
{
  net_socket_t *pSocket;
  pSocket = &sockets[sock];
  return pSocket->ulsocket;
}


/**
  * @brief  function description
  * @param  Params
  * @retval socket status
  */
static int32_t find_free_socket(void)
{
  int32_t sidx;
  int32_t ret = NET_ERROR_INVALID_SOCKET;
  LOCK_SOCK_ARRAY();
  for (sidx = 0; sidx < NET_MAX_SOCKETS_NBR; sidx++)
  {
    if (sockets[sidx].status == SOCKET_NOT_ALIVE)
    {
      sockets[sidx].idx = sidx;
      sockets[sidx].status = SOCKET_ALLOCATED;
      sockets[sidx].domain   = 0;
      sockets[sidx].type     = 0;
      sockets[sidx].protocol = 0;
#ifdef NET_MBEDTLS_HOST_SUPPORT
      sockets[sidx].is_secure = false;
      sockets[sidx].tlsData = 0;
#endif /* NET_MBEDTLS_HOST_SUPPORT */
      sockets[sidx].read_timeout = NET_SOCK_DEFAULT_RECEIVE_TO;
      sockets[sidx].write_timeout = NET_SOCK_DEFAULT_SEND_TO;
      sockets[sidx].blocking = true;
      sockets[sidx].ulsocket = -1;
      LOCK_SOCK(sidx);
      ret =  sidx;
      break;
    }
  }
  UNLOCK_SOCK_ARRAY();
  return ret;
}

static int32_t   clone_socket(int32_t sock)
{
  int32_t   newsock;
  newsock = find_free_socket();
  if (newsock >= 0)
  {
    (void) memcpy(&sockets[newsock], &sockets[sock], sizeof(net_socket_t));
  }
  return newsock;
}


/** @defgroup Socket Socket Management API
  * Application uses this manage socket. Its a Socket BSD like interface. It supports TCP and UDP
  * protocol on IPV4 address.IPV6 address to be supported in future versions.
  * @{
  */

/**
  * @brief  Create a socket
  * @param  domain [in] integer should be set to NET_AF_INET
  * @param  type [in] integer should be NET_SOCK_STREAM,NET_SOCK_DGRAM or NET_SOCK_RAW
  * @param  protocol [in] integer should be NET_IPPROTO_TCP,NET_IPPROTO_ICMP,NET_IPPROTO_UDP or NET_IPPROTO_TCP_TLS
  * @retval socket number as an integer greater than zero in case of success, zero or less than zero otherwise
  */

int32_t net_socket(int32_t domain, int32_t type, int32_t protocol)
{
  int32_t   newsock;
  newsock = find_free_socket();
  if (newsock >= 0)
  {
    sockets[newsock].domain   = domain;
    sockets[newsock].type     = type;
    sockets[newsock].protocol = protocol;
    UNLOCK_SOCK(newsock);
  }
  else
  {
    NET_DBG_ERROR("Socket allocation failed.\n");
  }
  return newsock;
}

/**
  * @brief  Bind a socket
  * @param  sock [in] integer socket number
  * @param  addr [in] pointer to net_sockaddr_t structure
  * @param  addrlen [in] unsigned integer length of the net_sockaddr_t
  * @retval zero in case of success, error code otherwise
  */
int32_t net_bind(int32_t sock, net_sockaddr_t *addr, uint32_t addrlen)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;
  net_if_handle_t *pnetif;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    /* find the interface */
    pnetif = net_if_find(addr);
    if (pnetif != NULL)
    {
      pSocket = net_socket_get_and_lock(sock);
      pSocket->pnetif = pnetif;
      if (create_low_level_socket(sock) < 0)
      {
        ret = NET_ERROR_SOCKET_FAILURE;
        NET_DBG_ERROR("low level socket creation failed.\n");
      }
      else
      {
        if (net_access_control(pnetif, NET_ACCESS_BIND, &ret))
        {
          UNLOCK_SOCK(sock);
          ret = pSocket->pnetif->pdrv->pbind(pSocket->ulsocket, addr, addrlen);
          LOCK_SOCK(sock);
          if (ret != NET_OK)
          {
            NET_DBG_ERROR("Socket cannot be bound");
          }
        }
      }
      UNLOCK_SOCK(sock);
    }
    else
    {
      NET_DBG_ERROR("No physical interface can be bound");
    }
  }
  return ret;
}

/**
  * @brief  Accept a connection from a client
  * @param  sock [in] integer socket number
  * @param  addr [out] pointer to net_sockaddr_t structure of remote connection
  * @param  addrlen [out] pointer to unsigned integer, length of the remote net_sockaddr_t
  * @retval socket number as an integer greater than zero in case of success, zero or less than zero otherwise
  */

int32_t net_accept(int32_t sock, net_sockaddr_t *addr, uint32_t *addrlen)
{
  int32_t newsock;
  int32_t ulnewsock;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR("Invalid socket.\n");
    newsock =  NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if (check_low_level_socket(sock) < 0)
    {
      NET_DBG_ERROR("low level Socket has not been created.\n");
      newsock =  NET_ERROR_SOCKET_FAILURE;
    }
    else
    {
      pSocket = net_socket_get_and_lock(sock);
      if (net_access_control(pSocket->pnetif, NET_ACCESS_BIND, &ulnewsock))
      {
        UNLOCK_SOCK(sock);
        ulnewsock = pSocket->pnetif->pdrv->paccept(pSocket->ulsocket, addr, addrlen);
        LOCK_SOCK(sock);

      }
      if (ulnewsock < 0)
      {
        NET_DBG_ERROR("No connection has been established.\n");
        newsock = ulnewsock;
      }
      else
      {
        sockets[sock].status = SOCKET_CONNECTED;
        newsock = clone_socket(sock);
        if (newsock >= 0)
        {
          sockets[newsock].ulsocket = ulnewsock;
          sockets[newsock].cloneserver = true;
          UNLOCK_SOCK(newsock);
        }
      }
      UNLOCK_SOCK(sock);
    }
  }
  return newsock;
}

/**
  * @brief  socket is waiting for a connection
  * @param  sock [in] integer socket number
  * @param  backlog [in] integer maximum number of queued connection
  * @retval zero in case of success, none zero value in case of error
  */
int32_t net_listen(int32_t sock, int32_t backlog)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if (check_low_level_socket(sock) < 0)
    {
      NET_DBG_ERROR("low level socket has not been created.\n");
      ret = NET_ERROR_SOCKET_FAILURE;
    }
    else
    {
      pSocket = net_socket_get_and_lock(sock);
      if (net_access_control(pSocket->pnetif, NET_ACCESS_LISTEN, &ret))
      {
        UNLOCK_SOCK(sock);
        ret = pSocket->pnetif->pdrv->plisten(pSocket->ulsocket, backlog);
        LOCK_SOCK(sock);

        if (ret != NET_OK)
        {
          NET_DBG_ERROR("Listen state cannot be set.\n");
        }
      }
      UNLOCK_SOCK(sock);
    }
  }
  return ret;
}

/**
  * @brief  connect socket to a server
  * @param  sock [in] integer socket number
  * @param  addr [in] pointer to net_sockaddr_t structure of server
  * @param  addrlen [in] pointer to unsigned integer, length of the server net_sockaddr_t
  * @retval zero in case of success, none zero value in case of error
  */

int32_t net_connect(int32_t sock, net_sockaddr_t *addr, uint32_t addrlen)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    pSocket = net_socket_get_and_lock(sock);
#if (NET_USE_DEFAULT_INTERFACE == 1)
    if (pSocket->pnetif == NULL)
    {
      pSocket->pnetif = net_if_find(NULL);
    }
#endif /* NET_USE_DEFAULT_INTERFACE */

    if (pSocket->pnetif == NULL)
    {
      ret = NET_ERROR_INTERFACE_FAILURE;
      NET_DBG_ERROR("No physical interface can be bound");
    }
    else
    {
      if (create_low_level_socket(sock) < 0)
      {
        NET_DBG_ERROR("low level socket creation failed.\n");
        if (create_low_level_socket(sock) < 0)
        {
          ret = NET_ERROR_SOCKET_FAILURE;
        }
        else
        {
          NET_DBG_ERROR("2nd trail ok level socket creation success.\n");

        }
      }
      else
      {
        if (net_access_control(pSocket->pnetif, NET_ACCESS_CONNECT, &ret))
        {
          UNLOCK_SOCK(sock);
          ret = pSocket->pnetif->pdrv->pconnect(pSocket->ulsocket, addr, addrlen);
          LOCK_SOCK(sock);

          if (ret != NET_OK)
          {
            /* clear flag to avoid issue on clean up , mbedtls not started */
#ifdef NET_MBEDTLS_HOST_SUPPORT
            if ((pSocket->is_secure == true) && (pSocket->tlsData != NULL))
            {
              /*cstat -MISRAC2012-Rule-21.3 -MISRAC2012-Dir-4.13_h */
              NET_FREE(pSocket->tlsData);
              /*cstat +MISRAC2012-Rule-21.3 +MISRAC2012-Dir-4.13_h */
            }
            pSocket->is_secure = false;
#endif /* NET_MBEDTLS_HOST_SUPPORT */
            NET_DBG_ERROR("Connection cannot be established.\n");
          }
        }
      }
      if (ret == NET_OK)
      {
#ifdef NET_MBEDTLS_HOST_SUPPORT
        if (pSocket->is_secure)
        {
          if (net_mbedtls_start(pSocket) != NET_OK)
          {
            /* to avoid useless cleanup */
            pSocket->is_secure = false;
            UNLOCK_SOCK(sock);
            (void) net_closesocket(sock);
            LOCK_SOCK(sock);
            ret = NET_ERROR_SOCKET_FAILURE;
          }
          else
          {
            pSocket->tls_started = true;
          }
        }
        if (NET_OK == ret)
        {
#endif /* NET_MBEDTLS_HOST_SUPPORT */
          pSocket->status = SOCKET_CONNECTED;
#ifdef NET_MBEDTLS_HOST_SUPPORT
        }
#endif /* NET_MBEDTLS_HOST_SUPPORT */
      }
    }
    UNLOCK_SOCK(sock);
  }
  return ret;
}


/**
  * @brief  send data to a connected socket
  * @param  sock [in] integer source socket number
  * @param  buf [in] pointer to an array of unsigned byte
  * @param  len [in] number of byte to send
  * @retval number of byte transmitted, negative value in case of error or timeout
  */
int32_t net_send(int32_t sock, uint8_t *buf, uint32_t len, int32_t flags)
{
  int32_t       ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if (buf == NULL)
    {
      ret = NET_ERROR_INVALID_SOCKET;
    }
    else
    {
      if (check_low_level_socket(sock) < 0)
      {
        NET_DBG_ERROR("low level socket has not been created.\n");
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      else
      {
        pSocket = net_socket_get_and_lock(sock);

#ifdef NET_MBEDTLS_HOST_SUPPORT
        if (pSocket->is_secure)
        {
          ret = (int32_t) net_mbedtls_sock_send(pSocket,  buf,  len);
        }
        else
#endif /* NET_MBEDTLS_HOST_SUPPORT */
        {
          if (net_access_control(pSocket->pnetif, NET_ACCESS_SEND, &ret))
          {
            UNLOCK_SOCK(sock);
            ret = pSocket->pnetif->pdrv->psend(pSocket->ulsocket, buf, len, flags);
            LOCK_SOCK(sock);

            if ((ret < 0) && (ret != NET_ERROR_CLOSE_SOCKET))
            {
              NET_DBG_ERROR("Error during sending data.\n");
            }
          }
        }
        UNLOCK_SOCK(sock);
      }
    }
  }
  return ret;
}

/**
  * @brief  receive data from a connected socket
  * @param  sock [in] integer socket number
  * @param  buf [in] pointer to an array of unsigned byte
  * @param  len [in] number of byte to read
  * @param  flags [in] specify blocking or non blocking , 0 is blocking mode, NET_MSG_DONTWAIT is non blocking
  * @retval number of byte received, negative value in case of error or timeout
  */
int32_t net_recv(int32_t sock, uint8_t *buf, uint32_t len, int32_t flags_in)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  int32_t flags = flags_in;

  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if (buf == NULL)
    {
      ret = NET_ERROR_INVALID_SOCKET;
    }
    else
    {
      if (check_low_level_socket(sock) < 0)
      {
        NET_DBG_ERROR("low level socket has not been created.\n");
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      else
      {
        pSocket = net_socket_get_and_lock(sock);

#ifdef NET_MBEDTLS_HOST_SUPPORT
        if (pSocket->is_secure)
        {
          ret = net_mbedtls_sock_recv(pSocket,  buf,  len);
        }
        else
#endif /* NET_MBEDTLS_HOST_SUPPORT */
        {
          if (net_access_control(pSocket->pnetif, NET_ACCESS_RECV, &ret))
          {
            UNLOCK_SOCK(sock);
            if (pSocket->read_timeout == 0)
            {
              flags = (int8_t) NET_MSG_DONTWAIT;
            }
            ret = pSocket->pnetif->pdrv->precv(pSocket->ulsocket, buf, len, flags);
            LOCK_SOCK(sock);
            if ((ret < 0) && (ret != NET_TIMEOUT) && (ret != NET_ERROR_CLOSE_SOCKET))
            {
              NET_DBG_ERROR("Error during receiving data. %ld\n", ret);
            }
          }
        }
        UNLOCK_SOCK(sock);
      }
    }
  }

  return ret;
}

/**
  * @brief  send data to a socket at specific address
  * @param  sock [in] integer source socket number
  * @param  buf [in] pointer to an array of unsigned byte
  * @param  len [in] number of byte to send
  * @param  flags [in] specify blocking or non blocking , 0 is blocking mode, NET_MSG_DONTWAIT is non blocking
  * @param  to [in] pointer to net_sockaddr_t structure destination address
  * @param  tolen [in] pointer to unsigned integer, length of the destination net_sockaddr_t
  * @retval number of byte transmitted, negative value in case of error or timeout
  */
int32_t net_sendto(int32_t sock, uint8_t *buf, uint32_t len, int32_t flags, net_sockaddr_t *to, uint32_t tolen)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if (buf == NULL)
    {
      ret = NET_ERROR_INVALID_SOCKET;
    }
    else
    {
      if (create_low_level_socket(sock) < 0)
      {
        NET_DBG_ERROR("low level socket creation failed.\n");
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      else
      {
        pSocket = net_socket_get_and_lock(sock);
        if (net_access_control(pSocket->pnetif, NET_ACCESS_SENDTO, &ret))
        {
          UNLOCK_SOCK(sock);
          ret = pSocket->pnetif->pdrv->psendto(pSocket->ulsocket, buf, len, flags, to, tolen);
          LOCK_SOCK(sock);
          if ((ret < 0) && (ret != NET_ERROR_CLOSE_SOCKET))
          {
            NET_DBG_ERROR("Error during sending data.\n");
          }
        }
        UNLOCK_SOCK(sock);
      }
    }
  }
  return ret;
}

/**
  * @brief  receive data from a socket at specific address
  * @param  sock [in] integer source socket number
  * @param  buf [in] pointer to an array of unsigned byte
  * @param  len [in] number of byte to receive
  * @param  flags [in] specify blocking or non blocking , 0 is blocking mode, NET_MSG_DONTWAIT is non blocking
  * @param  from [in] pointer to net_sockaddr_t structure to store the source address
  * @param  fromlen [in] pointer to unsigned integer, length of the source net_sockaddr_t
  * @retval number of byte received, negative value in case of error or timeout
  */
int32_t net_recvfrom(int32_t sock, uint8_t *buf, uint32_t len, int32_t flags_in, net_sockaddr_t *from,
                     uint32_t *fromlen)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  int32_t flags = flags_in;

  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if (buf == NULL)
    {
      ret = NET_ERROR_INVALID_SOCKET;
    }
    else
    {
      if (create_low_level_socket(sock) < 0)
      {
        NET_DBG_ERROR("low level socket creation failed.\n");
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      else
      {
        pSocket = net_socket_get_and_lock(sock);
        if (net_access_control(pSocket->pnetif, NET_ACCESS_RECVFROM, &ret))
        {
          UNLOCK_SOCK(sock);
          if (pSocket->read_timeout == 0)
          {
            flags = (int8_t) NET_MSG_DONTWAIT;
          }
          ret = pSocket->pnetif->pdrv->precvfrom(pSocket->ulsocket, buf, len, flags, from, fromlen);
          LOCK_SOCK(sock);
          if ((ret < 0) && (ret != NET_TIMEOUT) && (ret != NET_ERROR_CLOSE_SOCKET))
          {
            /*   NET_DBG_ERROR("Error during receiving data %ld\n", ret);*/
          }
        }
        UNLOCK_SOCK(sock);
      }
    }
  }
  return ret;
}


/**
  * @brief  shutdown a connection
  * @param  sock [in] integer socket number
  * @param  mode [in] integer, shutdown mode among NET_SHUTDOWN_R,NET_SHUTDOWN_W or NET_SHUTDOWN_RW
  * @retval zero on success, negative value in case of error
  */
int32_t net_shutdown(int32_t sock, int32_t      mode)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    pSocket = net_socket_get_and_lock(sock);
#ifdef NET_MBEDTLS_HOST_SUPPORT
    if (pSocket->is_secure)
    {
      if (pSocket->tls_started)
      {
        pSocket->tls_started = false;
        (void) net_mbedtls_stop(pSocket);
      }
      pSocket->is_secure = false;
    }
#endif /* NET_MBEDTLS_HOST_SUPPORT */

    if (check_low_level_socket(sock) < 0)
    {
      NET_DBG_ERROR("failed to shutdown :low level socket not existing.\n");
      pSocket = &sockets[sock];
      pSocket->status = SOCKET_NOT_ALIVE;
      ret = NET_OK;
    }
    else
    {
      if (net_access_control(pSocket->pnetif, NET_ACCESS_CLOSE, &ret))
      {
        UNLOCK_SOCK(sock);
        ret = pSocket->pnetif->pdrv->pshutdown(pSocket->ulsocket, mode);
        LOCK_SOCK(sock);

        if (ret != NET_OK)
        {
          NET_DBG_ERROR("Socket cannot be shutdown.\n");
        }
        /*     FIXME
                pSocket->status = SOCKET_NOT_ALIVE; */
      }
    }
    UNLOCK_SOCK(sock);
  }

  return ret;
}

/**
  * @brief  close a socket
  * @param  sock [in] integer socket number
  * @retval zero on success, negative value in case of error
  */
int32_t net_closesocket(int32_t sock)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    pSocket = net_socket_get_and_lock(sock);
#ifdef NET_MBEDTLS_HOST_SUPPORT
    if (pSocket->is_secure)
    {
      if (pSocket->tls_started)
      {
        pSocket->tls_started = false;
        (void) net_mbedtls_stop(pSocket);
      }
      pSocket->is_secure = false;
    }
#endif /* NET_MBEDTLS_HOST_SUPPORT */

    if (check_low_level_socket(sock) < 0)
    {
      NET_DBG_ERROR("failed to close :low level socket not existing.\n");
      pSocket = &sockets[sock];
      pSocket->status = SOCKET_NOT_ALIVE;
      ret = NET_OK;
    }
    else
    {
      if (net_access_control(pSocket->pnetif, NET_ACCESS_CLOSE, &ret))
      {
        UNLOCK_SOCK(sock);
        ret = pSocket->pnetif->pdrv->pclose(pSocket->ulsocket, pSocket->cloneserver);
        LOCK_SOCK(sock);

        if (ret != NET_OK)
        {
          NET_DBG_ERROR("Socket cannot be closed.\n");
        }
        pSocket->ulsocket = -1;
        pSocket->status = SOCKET_NOT_ALIVE;
      }
    }
    UNLOCK_SOCK(sock);
  }

  return ret;
}


/**
  * @brief  get socket peer information
  * @param  sock [in] integer source socket number
  * @param  name [in] pointer to net_sockaddr_t structure to write the peer address
  * @param  namelen [in] pointer to unsigned integer, length of the peer net_sockaddr_t
  * @retval negative value in case of error
  */
int32_t net_getpeername(int32_t sock, net_sockaddr_t *name, uint32_t *namelen)
{
  int32_t       ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if (name == NULL)
    {
      ret = NET_ERROR_INVALID_SOCKET;
    }
    else
    {
      if (check_low_level_socket(sock) < 0)
      {
        NET_DBG_ERROR("low level socket has not been created.\n");
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      else
      {
        pSocket = net_socket_get_and_lock(sock);
        {
          if (net_access_control(pSocket->pnetif, NET_ACCESS_SOCKET, &ret))
          {
            UNLOCK_SOCK(sock);
            ret = pSocket->pnetif->pdrv->pgetpeername(pSocket->ulsocket, name, namelen);
            LOCK_SOCK(sock);
            if (ret < 0)
            {
              NET_DBG_ERROR("Error during getpeername data.\n");
            }
          }
        }
        UNLOCK_SOCK(sock);
      }
    }
  }
  return ret;
}

/**
  * @brief  get socket name information
  * @param  sock [in] integer source socket number
  * @param  name [in] pointer to net_sockaddr_t structure to write the socket address
  * @param  namelen [in] pointer to unsigned integer, length of the socket net_sockaddr_t
  * @retval negative value in case of error
  */
int32_t net_getsockname(int32_t sock, net_sockaddr_t *name, uint32_t *namelen)
{
  int32_t       ret = NET_ERROR_FRAMEWORK;
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if (name == NULL)
    {
      ret = NET_ERROR_INVALID_SOCKET;
    }
    else
    {
      if (check_low_level_socket(sock) < 0)
      {
        NET_DBG_ERROR("low level socket has not been created.\n");
        ret = NET_ERROR_SOCKET_FAILURE;
      }
      else
      {
        pSocket = net_socket_get_and_lock(sock);
        {
          if (net_access_control(pSocket->pnetif, NET_ACCESS_SOCKET, &ret))
          {
            UNLOCK_SOCK(sock);
            ret = pSocket->pnetif->pdrv->pgetsockname(pSocket->ulsocket, name, namelen);
            LOCK_SOCK(sock);
            if (ret < 0)
            {
              NET_DBG_ERROR("Error during getpeername data.\n");
            }
          }
        }
        UNLOCK_SOCK(sock);
      }
    }
  }
  return ret;
}


/**
  * @brief  set socket option
  * @param  sock [in] integer socket number
  * @param  level [in] integer, protocol layer thta option is to be applied, must be set to NET_SOL_SOCKET
  * @param  optname [in] integer, option from the supported option list
  * @param  optvalue [in] void pointer to the wanted option value
  * @param  optlen [in] length of data pointed by optvalue
  * @retval zero on success, negative value in case of error
  */


int32_t net_setsockopt(int32_t sock, int32_t level, net_socketoption_t optname,  const void *optvalue, uint32_t optlen)
{
  int32_t ret = NET_ERROR_FRAMEWORK;
  bool        forward = false;
#ifdef NET_MBEDTLS_HOST_SUPPORT
  /*cstat -MISRAC2012-Rule-11.5 */
  const char_t *optvalue_string = optvalue;
  /*cstat +MISRAC2012-Rule-11.5 */
#endif /* NET_MBEDTLS_HOST_SUPPORT */
  net_socket_t *pSocket;

  if (!is_valid_socket(sock))
  {
    NET_DBG_ERROR("Invalid socket.\n");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {

    pSocket = net_socket_get_and_lock(sock);
    switch (optname)
    {
      case NET_SO_BINDTODEVICE:
      {
        OPTCHECKTYPE(void *, optlen);

        /*cstat -MISRAC2012-Rule-11.8 -MISRAC2012-Rule-11.5 */
        pSocket->pnetif = (net_if_handle_t *)optvalue;
        /*cstat +MISRAC2012-Rule-11.8 +MISRAC2012-Rule-11.5 */
        ret = NET_OK;
        break;
      }
      case NET_SO_RCVTIMEO:
      {
        OPTCHECKTYPE(int32_t, optlen);
        /*cstat -MISRAC2012-Rule-11.5 */
        pSocket->read_timeout = *(const int32_t *)optvalue;
        /*cstat +MISRAC2012-Rule-11.5 */

#ifdef NET_MBEDTLS_HOST_SUPPORT
        net_mbedtls_set_read_timeout(pSocket);
#endif /* NET_MBEDTLS_HOST_SUPPORT */
        forward = true;
        break;
      }

      case NET_SO_SNDTIMEO:
      {
        OPTCHECKTYPE(int32_t, optlen);
        /*cstat -MISRAC2012-Rule-11.5 */
        pSocket->write_timeout = *(const int32_t *)optvalue;
        /*cstat +MISRAC2012-Rule-11.5 */

        forward = true;
        break;
      }
#ifdef NET_MBEDTLS_HOST_SUPPORT
      case NET_SO_SECURE:
      {
        if (pSocket->status == SOCKET_CONNECTED)
        {
          ret = NET_ERROR_IS_CONNECTED;
        }
        else
        {
          if (!net_mbedtls_check_tlsdata(pSocket))
          {
            NET_DBG_ERROR("Failed to set tls device certificats, Allocation failure\n");
            ret = NET_ERROR_NO_MEMORY;
          }
          else
          {
            pSocket->is_secure = true;
            ret = NET_OK;
          }
        }
        break;
      }
      case NET_SO_TLS_DEV_CERT:
      {
        if (pSocket->status == SOCKET_CONNECTED)
        {
          ret = NET_ERROR_IS_CONNECTED;
        }
        else
        {
          OPTCHECKSTRING(optvalue_string, optlen);
          if (!net_mbedtls_check_tlsdata(pSocket))
          {
            NET_DBG_ERROR("Failed to set tls device certificats, Allocation failure\n");
            ret = NET_ERROR_NO_MEMORY;
          }
          else
          {
            pSocket->tlsData->tls_dev_cert = optvalue_string;
            ret = NET_OK;
          }
        }
        break;
      }

      case NET_SO_TLS_DEV_KEY:
      {
        if (pSocket->status == SOCKET_CONNECTED)
        {
          ret = NET_ERROR_IS_CONNECTED;
        }
        else
        {
          OPTCHECKSTRING(optvalue_string, optlen);
          if (!net_mbedtls_check_tlsdata(pSocket))
          {
            NET_DBG_ERROR("Failed to set tls device key, Allocation failure\n");
            ret = NET_ERROR_NO_MEMORY;
          }
          else
          {
            pSocket->tlsData->tls_dev_key = optvalue_string;
            ret = NET_OK;
          }
        }
        break;
      }

      case NET_SO_TLS_PASSWORD:
      {
        if (pSocket->status == SOCKET_CONNECTED)
        {
          ret = NET_ERROR_IS_CONNECTED;
        }
        else
        {
          OPTCHECKSTRING(optvalue_string, optlen);
          if (!net_mbedtls_check_tlsdata(pSocket))
          {
            NET_DBG_ERROR("Failed to set tls password, Allocation failure\n");
            ret = NET_ERROR_NO_MEMORY;
          }
          else
          {
            /*cstat -MISRAC2012-Rule-11.5 */
            pSocket->tlsData->tls_dev_pwd = (const  uint8_t *) optvalue;
            /*cstat +MISRAC2012-Rule-11.5 */
            ret = NET_OK;
          }
        }
        break;
      }
      case NET_SO_TLS_CA_CERT:
      {
        if (pSocket->status == SOCKET_CONNECTED)
        {
          ret = NET_ERROR_IS_CONNECTED;
        }
        else
        {
          OPTCHECKSTRING(optvalue_string, optlen);
          if (!net_mbedtls_check_tlsdata(pSocket))
          {
            NET_DBG_ERROR("Failed to set tls root ca, Allocation failure\n");
            ret = NET_ERROR_NO_MEMORY;
          }
          else
          {
            pSocket->tlsData->tls_ca_certs = optvalue_string;
            ret = NET_OK;
          }
        }
        break;
      }

      case NET_SO_TLS_CA_CRL:
      {
        if (pSocket->status == SOCKET_CONNECTED)
        {
          ret = NET_ERROR_IS_CONNECTED;
        }
        else
        {
          OPTCHECKSTRING(optvalue_string, optlen);
          if (!net_mbedtls_check_tlsdata(pSocket))
          {
            NET_DBG_ERROR("Failed to set tls revocation certification list, Allocation failure\n");
            ret = NET_ERROR_NO_MEMORY;
          }
          else
          {
            pSocket->tlsData->tls_ca_crl = optvalue_string;
            ret = NET_OK;
          }
        }
        break;
      }

      case NET_SO_TLS_SERVER_VERIFICATION:
      {
        if (pSocket->status == SOCKET_CONNECTED)
        {
          ret = NET_ERROR_IS_CONNECTED;
        }
        else
        {
          OPTCHECKTYPE(bool, optlen);
          if (!net_mbedtls_check_tlsdata(pSocket))
          {
            NET_DBG_ERROR("Failed to set tls server verification mode, Allocation failure\n");
            ret = NET_ERROR_NO_MEMORY;
          }
          else
          {
            /*cstat -MISRAC2012-Rule-11.5 */
            pSocket->tlsData->tls_srv_verification = (*(const bool *)optvalue > 0) ? true : false;
            /*cstat +MISRAC2012-Rule-11.5 */
            ret = NET_OK;
          }
        }
        break;
      }

      case NET_SO_TLS_SERVER_NAME:
      {
        if (pSocket->status == SOCKET_CONNECTED)
        {
          ret = NET_ERROR_IS_CONNECTED;
        }
        else
        {
          OPTCHECKSTRING(optvalue_string, optlen);
          if (!net_mbedtls_check_tlsdata(pSocket))
          {
            NET_DBG_ERROR("Failed to set tls server name, Allocation failure\n");
            ret = NET_ERROR_NO_MEMORY;
          }
          else
          {
            pSocket->tlsData->tls_srv_name = optvalue_string;
            ret = NET_OK;
          }
        }
        break;
      }

      /* Set the X.509 security profile */
      case NET_SO_TLS_CERT_PROF:
      {
        if (pSocket->status == SOCKET_CONNECTED)
        {
          ret = NET_ERROR_IS_CONNECTED;
        }
        else
        {
          OPTCHECKTYPE(mbedtls_x509_crt_profile, optlen);
          if (!net_mbedtls_check_tlsdata(pSocket))
          {
            NET_DBG_ERROR("Failed to set tls X.509 security profile, Allocation failure\n");
            ret = NET_ERROR_NO_MEMORY;
          }
          else
          {
            /*cstat -MISRAC2012-Rule-11.5 */
            pSocket->tlsData->tls_cert_prof = (const mbedtls_x509_crt_profile *) optvalue;
            /*cstat +MISRAC2012-Rule-11.5 */
            ret = NET_OK;
          }
        }
        break;
      }
#endif /* NET_MBEDTLS_HOST_SUPPORT */

      default:
        forward = true;
        break;

    }

    if (true == forward)
    {
#if (NET_USE_DEFAULT_INTERFACE == 1)
      if (pSocket->pnetif == NULL)
      {
        pSocket->pnetif = net_if_find(NULL);
      }
#endif /* NET_USE_DEFAULT_INTERFACE */
      if (pSocket->pnetif == NULL)
      {
        NET_DBG_ERROR("No physical interface can be bound");
        ret = NET_ERROR_INTERFACE_FAILURE;
      }
      else
      {
        if (create_low_level_socket(sock) < 0)
        {
          NET_DBG_ERROR("low level socket creation failed.\n");
          ret = NET_ERROR_SOCKET_FAILURE;
        }
        else
        {
          if (net_access_control(pSocket->pnetif, NET_ACCESS_SETSOCKOPT, &ret))
          {
            UNLOCK_SOCK(sock);
            ret = pSocket->pnetif->pdrv->psetsockopt(pSocket->ulsocket, level, optname, optvalue, optlen);
            LOCK_SOCK(sock);
            if (ret < 0)
            {
              NET_DBG_ERROR("Error %ld while setting socket option (optname=%d).\n", ret, optname);
            }
          }
        }
      }
    }
    UNLOCK_SOCK(sock);
  }
  return ret;
}

/** @defgroup Socket
  * @}
  */

bool    net_access_control(net_if_handle_t *pnetif, net_access_t func, int32_t *code)
{
  bool ret = true;
  if (pnetif->state == NET_STATE_CONNECTION_LOST)
  {
    /* send ,recv functino return zero , so user application should normaly retry transfer */
    ret = false;
    *code = 0;
  }
  switch (func)
  {
    case NET_ACCESS_SOCKET:
      ret = true;

      break;

    case NET_ACCESS_BIND:
      break;

    case NET_ACCESS_LISTEN:
      break;

    case NET_ACCESS_CONNECT:
      break;

    case NET_ACCESS_CLOSE:
      ret = true;
      break;

    case NET_ACCESS_SEND:
      *code = 0;
      break;

    case NET_ACCESS_SENDTO:
      *code = 0;
      break;

    case NET_ACCESS_RECV:
      *code = 0;
      break;

    case NET_ACCESS_RECVFROM:
      *code = 0;
      break;

    case NET_ACCESS_SETSOCKOPT:
      ret = true;
      break;

    default:
      *code = NET_ERROR_FRAMEWORK;
      break;
  }
  return ret;
}

#endif /* NET_BYPASS_NET_SOCKET */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

