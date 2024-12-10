/**
  ******************************************************************************
  * @file    net_cellular.c
  * @author  MCD Application Team
  * @brief   CELLULAR-specific BSD-like socket wrapper
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

/*#define ENABLE_NET_DBG_INFO */
#include "net_connect.h"
#include "net_internals.h"
/*cstat -MISRAC* -DEFINE-* -CERT-EXP19*  */
#include "cellular_version.h"

#if defined(CELLULAR_INIT_FIRMWARE_VERSION)
#include "cellular_init.h"
#include "cellular_service_os.h"
#elif defined(CELLULAR_VERSION_FIRMWARE_VERSION)
#include "cellular_mngt.h"
#include "cellular_datacache.h"
#include "com_sockets.h"
#endif /* defined(CELLULAR_INIT_FIRMWARE_VERSION) */

#include "dc_common.h"
/*cstat +MISRAC* +DEFINE-* +CERT-EXP19*  */

#define OSWAITFOREVER 0xFFFFFFFFU
/* Private defines -----------------------------------------------------------*/
#define NET_CELLULAR_MAX_CHANNEL_NBR    4

#define CELLULAR_FREE_SOCKET            0U
#define CELLULAR_ALLOCATED_SOCKET       1U
#define CELLULAR_BIND_SOCKET            2U
#define CELLULAR_SEND_OK                4U
#define CELLULAR_RECV_OK                8U
#define CELLULAR_CONNECTED_SOCKET      16U
#define CELLULAR_CONNECTED_SOCKET_RW   (CELLULAR_CONNECTED_SOCKET | CELLULAR_SEND_OK | CELLULAR_RECV_OK)

#define CELLULAR_SOCK_DEFAULT_TO    10000
#define CELLULAR_ATTACHMENT_TIMEOUT    180000U

#if !defined NET_CELLULAR_THREAD_PRIO
#define NET_CELLULAR_THREAD_PRIO   osPriorityAboveNormal
#endif  /* !defined NET_CELLULAR_THREAD_PRIO */

#if !defined NET_CELLULAR_THREAD_STACK_SIZE
#define NET_CELLULAR_THREAD_STACK_SIZE  DEFAULT_THREAD_STACK_SIZE
#endif  /* !defined NET_CELLULAR_THREAD_PRIO */

typedef enum
{
  CELLULAR_RET_OK                = 0,
  CELLULAR_RET_ERROR             = 1,
  CELLULAR_RET_SIM_NOT_INSERTED  = 2,
} CELLULAR_Ret_t;

/* Private typedef -----------------------------------------------------------*/
typedef struct
{
  const char_t *apn;
  const char_t *username;
  const char_t *password;
  bool use_internal_sim;
} cellular_data_t;

typedef struct
{
  uint8_t  status;
} cellular_Channel_t;

/* Private variables ---------------------------------------------------------*/
static osThreadId CellIfThreadId = NULL;
static osMessageQId cellular_queue;
static volatile bool connection_requested;
static volatile bool stop_requested;
static dc_cellular_info_t dc_cellular_info;

static cellular_Channel_t CellularChannel[NET_CELLULAR_MAX_CHANNEL_NBR] = {.0};



/* Private function prototypes -----------------------------------------------*/

/* Interface APIs */
static int32_t net_cellular_if_init(net_if_handle_t *pnetif);
static int32_t net_cellular_if_deinit(net_if_handle_t *pnetif);

static int32_t net_cellular_if_start(net_if_handle_t *pnetif);
static int32_t net_cellular_if_stop(net_if_handle_t *pnetif);

static int32_t net_cellular_if_connect(net_if_handle_t *pnetif);
static int32_t net_cellular_if_disconnect(net_if_handle_t *pnetif);

/* Socket BSD Like APIs */
static int32_t net_cellular_socket(int32_t domain, int32_t type, int32_t protocol);
static int32_t net_cellular_bind(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen);
static int32_t net_cellular_listen(int32_t sock, int32_t backlog);
static int32_t net_cellular_accept(int32_t sock, net_sockaddr_t *addr, uint32_t *addrlen);
static int32_t net_cellular_connect(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen);
static int32_t net_cellular_send(int32_t sock, uint8_t *buf, int32_t len, int32_t flags);
static int32_t net_cellular_recv(int32_t sock, uint8_t *buf, int32_t len, int32_t flags);
static int32_t net_cellular_sendto(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *to,
                                   uint32_t tolen);
static int32_t net_cellular_recvfrom(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *from,
                                     uint32_t *fromlen);
static int32_t net_cellular_setsockopt(int32_t sock, int32_t level, int32_t optname, const void *optvalue,
                                       uint32_t optlen);
static int32_t net_cellular_getsockopt(int32_t sock, int32_t level, int32_t optname, void *optvalue, uint32_t *optlen);
static int32_t net_cellular_getsockname(int32_t sock, net_sockaddr_t *name, uint32_t *namelen);
static int32_t net_cellular_getpeername(int32_t sock, net_sockaddr_t *name, uint32_t *namelen);
static int32_t net_cellular_close(int32_t sock, bool isaclone);
static int32_t net_cellular_shutdown(int32_t sock, int32_t mode);

/* Service APIs */
static int32_t net_cellular_if_gethostbyname(net_if_handle_t *pnetif, net_sockaddr_t *addr, char_t *name);
static int32_t net_cellular_ping(net_if_handle_t *pnetif, net_sockaddr_t *addr, int32_t count,
 int32_t delay, int32_t response[]);

/* class extension APIs */
static int32_t net_cellular_get_radio_info(net_cellular_radio_results_t *results);


/* Internal usage */
static void cellif_input(void const *argument);

#ifdef NET_CELLULAR_CREDENTIAL_V2
static void cellular_set_config(const net_cellular_credentials_t* credentials);
#else
static void cellular_set_config(const char_t *oper_ap_code,const char_t *username,const char_t *password,bool use_internal_sim);
#endif /* NET_CELLULAR_CREDENTIAL_V2 */

static void cellular_notif_cb(dc_com_event_id_t dc_event_id, const void *private_gui_data);
static uint16_t Generate_RngLocalPort(uint16_t localport);


static void netsockaddr2com(com_sockaddr_t *com_addr,const net_sockaddr_t *addr, uint32_t addrlen)
{
  com_addr->sa_len = (uint8_t) addrlen;
  com_addr->sa_family =(uint8_t) ( (addr->sa_family == (uint8_t) NET_AF_INET) ? COM_AF_INET : COM_AF_UNSPEC);
  (void) memcpy(&(com_addr->sa_data), &(addr->sa_data), 14);
}

static void com2netsockaddr(net_sockaddr_t *addr, com_sockaddr_t *com_addr, uint32_t addrlen)
{
  (void) memset(addr, 0, addrlen);
  addr->sa_len = (uint8_t)  addrlen;
  addr->sa_family = NET_AF_INET;
  (void) memcpy(&(addr->sa_data), &(com_addr->sa_data), 14);
}



static net_if_drv_t   *net_cellular_init_class(void)
{
  net_if_drv_t *p;
/*cstat -MISRAC2012-Rule-11.5 */
  p = NET_MALLOC(sizeof(net_if_drv_t));
/*cstat +MISRAC2012-Rule-11.5 */

  if (p != NULL)
  {
    p->if_class = NET_INTERFACE_CLASS_CELLULAR;

    /* Interface APIs */
    p->if_init = net_cellular_if_init;
    p->if_deinit = net_cellular_if_deinit;

    p->if_start = net_cellular_if_start;
    p->if_stop = net_cellular_if_stop;

    p->if_connect = net_cellular_if_connect;
    p->if_disconnect = net_cellular_if_disconnect;

    /* Socket BSD Like APIs */
    p->psocket        = net_cellular_socket;
    p->pbind          = net_cellular_bind;
    p->plisten        = net_cellular_listen;
    p->paccept        = net_cellular_accept;
    p->pconnect       = net_cellular_connect;
    p->psend          = net_cellular_send;
    p->precv          = net_cellular_recv;
    p->psendto        = net_cellular_sendto;
    p->precvfrom      = net_cellular_recvfrom;
    p->psetsockopt    = net_cellular_setsockopt;
    p->pgetsockopt    = net_cellular_getsockopt;
    p->pgetsockname   = net_cellular_getsockname;
    p->pgetpeername   = net_cellular_getpeername;
    p->pclose         = net_cellular_close;
    p->pshutdown      = net_cellular_shutdown;

    /* Service */
    p->pgethostbyname = net_cellular_if_gethostbyname;

    p->pping = net_cellular_ping;
  }
  else
  {
    NET_DBG_ERROR("can't allocate memory for cellular_driver class");
    p = NULL;
  }
  return p;
}

/**
  * @brief  Build the Network interface
  *         It includes memory allocation for all internal class and extension
  * @param  pnetif points to a interface handle (in/out)
  * @retval
  *          = @if_init.
  */
int32_t cellular_net_driver(net_if_handle_t *pnetif)
{
  int32_t ret;
  net_if_drv_t *p;

  p = net_cellular_init_class();

  if (NULL == p)
  {
    NET_DBG_ERROR("can't allocate memory for cellular driver class");
    ret = NET_ERROR_NO_MEMORY;
  }
  else
  {
    /* class extension */
    /*cstat -MISRAC2012-Rule-11.5 */
    p->extension.cellular = NET_MALLOC(sizeof(net_if_cellular_class_extension_t));
    /*cstat +MISRAC2012-Rule-11.5 */
    if (NULL == p->extension.cellular)
    {
      NET_DBG_ERROR("can't allocate memory for cellular_driver class\n");
      NET_FREE(p);
      ret = NET_ERROR_NO_MEMORY;
    }
    else
    {
      p->extension.cellular->get_radio_results = net_cellular_get_radio_info;
      pnetif->pdrv = p;
      ret = net_cellular_if_init(pnetif);
    }
  }
  return ret;
}

/**
  * @brief  Initialize the cellular (modules and memory init)
  * @param  pnetif points to a interface handle (in/out)
  * @retval
  *          = 0  : done.
  */
static int32_t net_cellular_if_init(net_if_handle_t *pnetif)
{
  (void) pnetif;
  /* statical init of cellular components */
  connection_requested = false;
  stop_requested = false;
  cellular_init();

  return  NET_OK;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_cellular_if_deinit(net_if_handle_t *pnetif)
{
  NET_DBG_PRINT("\nEntering in net_cellular_if_deinit()");

  NET_FREE(pnetif->pdrv->extension.cellular);
  pnetif->pdrv->extension.cellular = NULL;
  NET_FREE(pnetif->pdrv);
  pnetif->pdrv = NULL;

  return  NET_OK;
}



/**
  * @brief  Start the cellular (modem switch on)
  * @param  pnetif points to a interface handle (in/out)
  * @retval
  *          < 0  : if there is an error.
  *          = 0  : on success.
  */
static int32_t net_cellular_if_start(net_if_handle_t *pnetif)
{
  const net_cellular_credentials_t *credentials =  pnetif->pdrv->extension.cellular->credentials;
  int32_t       ret;

  NET_PRINT("\n*** C2C connection ***\n\n");

  /* check APN is entered */
#ifdef NET_CELLULAR_CREDENTIAL_V2
  if ((credentials->sim_slot[NET_CELLULAR_DEFAULT_SIM_SLOT].apn == 0) ||
      (strlen((char_t *)credentials->sim_slot[NET_CELLULAR_DEFAULT_SIM_SLOT].apn) == 0U))
#else
  if ((credentials->apn == 0) || (strlen(credentials->apn) == 0U))
#endif /* NET_CELLULAR_CREDENTIAL_V2 */
  {
    NET_DBG_ERROR("operator credentials (APN at least) must be set");
    ret =  NET_ERROR_PARAMETER;
  }
  else
  {
    osMessageQDef(CellIfqueue, 6, uint32_t);
    cellular_queue = osMessageCreate(osMessageQ(CellIfqueue), NULL);

    /* create the task that handles the cellular */
    osThreadDef(CellIf, cellif_input, NET_CELLULAR_THREAD_PRIO, 0, NET_CELLULAR_THREAD_STACK_SIZE);
    CellIfThreadId = osThreadCreate(osThread(CellIf), pnetif);

    if (CellIfThreadId == NULL)
    {
      Error_Handler();

    }
#if (STACK_ANALYSIS_TRACE == 1)
    else
    {
      stackAnalysis_addStackSizeByHandle(CellIfThreadId, NET_CELLULAR_THREAD_STACK_SIZE);
    }
#endif /* STACK_ANALYSIS_TRACE */
    ret = NET_OK;
  }
  return ret;
}


static int32_t net_cellular_if_stop(net_if_handle_t *pnetif)
{
  (void) pnetif;
  dc_cellular_target_state_t target_state;
  stop_requested = true;

  target_state.rt_state     = DC_SERVICE_ON;
  target_state.target_state = DC_TARGET_STATE_OFF;

  (void)dc_com_write(&dc_com_db,
                     DC_CELLULAR_TARGET_STATE_CMD,
                     (void *)&target_state,
                     sizeof(target_state));
  return  NET_OK;
}


/**
  * @brief  Connect the cellular (@IP allocation)
  * @param  pnetif points to a interface handle (in/out)
  * @retval
  *          < 0  : if there is an error.
  *          = 0  : on success.
  */
static int32_t net_cellular_if_connect(net_if_handle_t *pnetif)
{
  (void) pnetif;
  dc_cellular_target_state_t target_state;

  /* 'cst targetstate full' command:  new modem state requested: modem manages full data transfert */
  target_state.rt_state     = DC_SERVICE_ON;
  target_state.target_state = DC_TARGET_STATE_FULL;
  (void)dc_com_write(&dc_com_db, DC_CELLULAR_TARGET_STATE_CMD, (void *)&target_state, sizeof(target_state));
  connection_requested = true;
  return NET_OK;
}

/**
  * @brief  Function description
  * @param  Params
  * @retval socket status
  */
static int32_t net_cellular_if_disconnect(net_if_handle_t *pnetif)
{
  (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_READY);
  return  NET_OK;
}


/**
  * @brief  create a socket.
  *
  * @param  domain is currently only NET_AF_INET (IPv6 not supported)
  *         type can be :
  *           - connected (NET_SOCK_STREAM)
  *           - connectionless (NET_SOCK_DGRAM), not supported in this version
  *         protocol is NET_IPPROTO_TCP
  * @retval ret
  *          <  0  : if there is an error.
  *          >= 0  : socket number.
  */
static int32_t net_cellular_socket(int32_t domain, int32_t type, int32_t protocol)
{
  int32_t ret;

  int32_t net_socketv;
  int32_t net_family = COM_AF_INET;
  int32_t net_type = COM_SOCK_DGRAM;
  int32_t net_protocol = COM_IPPROTO_UDP;

  if (domain != NET_AF_INET)
  {
    NET_DBG_ERROR("invalid domain");
    ret = NET_ERROR_UNSUPPORTED;
  }
  else
  {
    /* Only support PROT_IP/TCP/UDP/IPV4 are supported */
    if ((protocol != NET_IPPROTO_TCP) && (protocol != NET_IPPROTO_UDP))
    {
      NET_DBG_ERROR("invalid protocol");
      ret = NET_ERROR_UNSUPPORTED;
    }
    else
    {
      /* currently only SOCK_DGRAM and SOCK_STREAM are supported */
      switch (type)
      {
        case NET_SOCK_STREAM:
          net_family = COM_AF_INET;
          net_type = COM_SOCK_STREAM;
          net_protocol = COM_IPPROTO_TCP;

          ret = NET_OK;

          break;

        case NET_SOCK_DGRAM:
          net_family = COM_AF_INET;
          net_type = COM_SOCK_DGRAM;
          net_protocol = COM_IPPROTO_UDP;

          ret = NET_OK;

          break;
        default:

          ret = NET_ERROR_UNSUPPORTED;
          NET_DBG_ERROR("Undefined protocol");
          break;
      }
    }
  }

  if (ret == NET_OK)
  {
    NET_DBG_INFO("Trying to find a free socket\n");

    /* Find and create a free socket on the network interface */
    net_socketv = com_socket(net_family, net_type, net_protocol);

    /* A free socket has been found */
    if (net_socketv >= 0)
    {
      NET_DBG_INFO("Found socket : %lu\n", net_socketv);

      if (net_socketv < NET_CELLULAR_MAX_CHANNEL_NBR)
      {
        CellularChannel[net_socketv].status = CELLULAR_ALLOCATED_SOCKET;
        ret = net_socketv;
      }
      else
      {
        NET_DBG_ERROR("More socket supported (%lu) than allowed (%d)",
                      net_socketv,
                      NET_CELLULAR_MAX_CHANNEL_NBR);
        ret = NET_ERROR_INVALID_SOCKET;
      }
    }
    else
    {
      NET_DBG_ERROR("Could not find a free socket.");
      ret = NET_ERROR_INVALID_SOCKET;
    }
  }

  return ret;
}

static int32_t net_cellular_bind(int32_t sock, const net_sockaddr_t *addr, uint32_t addrlen)
{
  int32_t ret;
  (void) addrlen;
  com_sockaddr_t saddr;
  netsockaddr2com(&saddr, addr, sizeof(com_sockaddr_t));
  ret = com_bind(sock, &saddr, (int32_t) sizeof(com_sockaddr_t));

  return (ret == COM_SOCKETS_ERR_OK) ? NET_OK : NET_ERROR_GENERIC;
}

static int32_t net_cellular_listen(int32_t sock, int32_t backlog)
{
  int32_t ret;
  ret = com_listen(sock, backlog);
  return (ret == COM_SOCKETS_ERR_OK) ? NET_OK : NET_ERROR_GENERIC;
}

static int32_t net_cellular_accept(int32_t sock, net_sockaddr_t *addr, uint32_t *addrlenv)
{
  int32_t ret;
  com_sockaddr_t        com_addr;
  int32_t               addrlen = (int32_t) sizeof(com_sockaddr_t);

  ret = com_accept(sock, &com_addr, &addrlen);
  if (ret > 0)
  {
    com2netsockaddr(addr, &com_addr, *addrlenv);
    CellularChannel[ret].status = CELLULAR_CONNECTED_SOCKET_RW;
  }
  return (ret > 0) ? ret : NET_ERROR_GENERIC;

}


/**
  * @brief  connect a socket to a remote host on a given port.
  *
  * @param  sock must be created
  *         addr point to an addresses structure from the Socket library
  *         (If this is a TCP socket,
  *         connect() will actually perfom TCP negociation to open a connection)
  *         addrlen has a size compatible with struct sockaddr_in*
  * @retval ret
  *          < 0  : if there is an error.
  *          = 0  : on success.
  */
static int32_t net_cellular_connect(int32_t sock, const net_sockaddr_t *addrp, uint32_t addrlen)
{
  int32_t  ret;
  uint16_t            port;
  net_sockaddr_t        addr;

  (void) memcpy(&addr, addrp, addrlen);

  if ((sock < 0) || (sock >= NET_CELLULAR_MAX_CHANNEL_NBR))
  {
    NET_DBG_ERROR("invalid socket");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    /* addr is an outbound address */
    if (addrlen == sizeof(net_sockaddr_in_t))
    {
      NET_DBG_INFO("Trying to connect socket %ld\n", sock);
      port = net_get_port(&addr);
      port = Generate_RngLocalPort(port);
      net_set_port(&addr, port);

      addr.sa_family = COM_AF_INET;
      if (com_connect(sock,
                      /*cstat -MISRAC2012-Rule-11.3 */
                      (com_sockaddr_t *) &addr,
                      /*cstat +MISRAC2012-Rule-11.3*/
                      (int32_t) addrlen) == COM_SOCKETS_ERR_OK)
      {
        NET_DBG_INFO("connect done on socket %ld\n", sock);
        ret = NET_OK;
        CellularChannel[sock].status = CELLULAR_CONNECTED_SOCKET_RW;
      }
      else
      {
        NET_DBG_ERROR("Socket %ld cannot be connected", sock);
        ret = NET_ERROR_SOCKET_FAILURE;
      }
    }
    else
    {
      NET_DBG_ERROR("Cannot initiate connect due to an invalid address length");
      ret = NET_ERROR_PARAMETER;
    }
  }
  return ret;
}

/**
  * @brief  Generate a Random Local Port between 49152 and 65535
  *
  * @param  Local port will be randomized if it is 0
  *         The IANA range for ephemeral ports is 49152-65535.
  *         This is mandatory for UG96 FW modem version until R02A08.
  *         Beyond this version, it is done in the modem.
  * @retval ret
  *          Random Local Port
  */

#define PORTBASE  49152U
static uint16_t Generate_RngLocalPort(uint16_t localport)
{
  uint16_t RngPort;
  static uint16_t rnglocalport = 0;
  uint16_t random_number;

  if (localport != 0u)
  {
    RngPort = localport;
  }
  else
  {
    if (rnglocalport == 0u)  /* just at first open since board reboot */
    {
      random_number = (uint16_t) rand() & 0xffffu;
      rnglocalport = ((uint16_t)(random_number & 0xFFFFu) >> 2u) + PORTBASE;
    }
    else /* from second time function execution, increment by one */
    {
      rnglocalport += 1u;
    }

    if (rnglocalport < PORTBASE) /* Wrap-around */
    {
      rnglocalport = PORTBASE;
    }

    RngPort = rnglocalport;
  }
  return RngPort;
}



static int32_t conv(int32_t retv)
{
  int32_t ret = retv;
  if (ret <= 0)
  {

    switch (ret)
    {
      case 0:
        /* With DONT WAIT MODE return 0 when no data is available
        ret = NET_ERROR_CLOSE_SOCKET;  */
        ret = NET_TIMEOUT;

        break;

      case COM_SOCKETS_ERR_WOULDBLOCK:
        ret = NET_TIMEOUT;
        break;

      case COM_SOCKETS_ERR_CLOSING:
        ret = NET_ERROR_CLOSE_SOCKET;
        break;

      case COM_SOCKETS_ERR_TIMEOUT:
        ret = NET_TIMEOUT;
        break;

      default:
        ret = NET_ERROR_CLOSE_SOCKET;
        break;
    }
  }
  return ret;
}
/**
  * @brief  write data to the socket from buffer.
  *
  * @param  sock must be connected
  *         buf points to a buffer
  *         len is the amount of data available in the buffer
  *         If flags is set to NET_MSG_DONTWAIT,
  *           then send() will not block waiting for buffer to become free.
  * @retval ret
  *          <  0  : if there is an error.
  *          >= 0  : the amount of data which was sent.
  */
static int32_t net_cellular_send(int32_t sock, uint8_t *buf, int32_t len, int32_t flags)
{
  int32_t  ret;


  if ((sock < 0) || (sock >= NET_CELLULAR_MAX_CHANNEL_NBR))
  {
    NET_DBG_ERROR("invalid socket");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if ((CellularChannel[sock].status & CELLULAR_SEND_OK) == 0U)
    {
      NET_DBG_INFO("Socket %ld has shutdown its sending\n", sock);
      ret = NET_ERROR_SOCKET_FAILURE;
    }
    else
    {

      NET_DBG_INFO("Trying to send (%lu) on socket %ld\n", len, sock);

      ret = com_send(sock,
                     buf,
                     len,
                     (((uint8_t) flags & NET_MSG_DONTWAIT) != 0U) ? COM_MSG_DONTWAIT : COM_MSG_WAIT);
      if (ret < 0)
      {
        NET_DBG_ERROR("Send failed : %lu", ret);
      }
      else
      {
        NET_DBG_INFO("Data sent ! (%lu)\n", ret);
      }
      ret  = conv(ret);
    }
  }
  return ret;
}

/**
  * @brief  read data from the socket and write them into buffer.
  *
  * @param  sock must be connected
  *         buf points to a buffer
  *         len is the maximum amount of data allowed to be written in the buffer
  *         (all the data may not fill the entire buffer)
  *         If flags is set to NET_MSG_DONTWAIT,
  *           then recv() will not block if no data is available.
  * @retval ret
  *          < 0   : if there is an error.
  *          = 0   : no more data
  *                   or the remote host closed the connection gracefully (EOF)
  *          > 0   : the amount of data which was received.
  *                  (may be lower than len, data are reported at the earliest)
  */
static int32_t net_cellular_recv(int32_t sock, uint8_t *buf, int32_t len, int32_t flags)
{
  int32_t ret;

  if ((sock < 0) || (sock >= NET_CELLULAR_MAX_CHANNEL_NBR))
  {
    NET_DBG_ERROR("invalid socket");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    if ((CellularChannel [sock].status & CELLULAR_RECV_OK) == 0U)
    {
      NET_DBG_INFO("Socket %ld has shutdown its receipt\n", sock);
      ret = NET_ERROR_SOCKET_FAILURE;
    }
    else
    {
      NET_DBG_INFO("com_recv in progress of %ld bytes\n", len);
      ret = com_recv(sock,
                     buf,
                     len,
                     (((uint8_t)flags & NET_MSG_DONTWAIT) != 0U) ? COM_MSG_DONTWAIT : COM_MSG_WAIT);

      if (ret > 0)
      {
        NET_DBG_INFO("Amount of received data (ret) = %d\n", ret);
        if (ret > len)
        {
          NET_WARNING("Received more than requested (received %ld - requested %ld)\n", ret, len);
        }
      }
      ret = conv(ret);
    }
  }
  return ret;
}

static int32_t net_cellular_sendto(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *to,
                                   uint32_t tolen)
{
  int32_t ret;
  com_sockaddr_t addr;
  (void) tolen;
  netsockaddr2com(&addr, to, sizeof(com_sockaddr_t));
  ret = com_sendto(sock, buf, len, flags, &addr, (int32_t) sizeof(com_sockaddr_t));

  return conv(ret);
}

static int32_t net_cellular_recvfrom(int32_t sock, uint8_t *buf, int32_t len, int32_t flags, net_sockaddr_t *from,
                                     uint32_t *fromlen)
{
  int32_t ret;
  com_sockaddr_t addr;
  int32_t addrlen = (int32_t) sizeof(com_sockaddr_t);

  ret = com_recvfrom(sock, buf, len, flags, &addr, &addrlen);
  if (ret >= 0)
  {
    com2netsockaddr(from, &addr, *fromlen);
  }

  return conv(ret);
}

/**
  * @brief  set options associated with a socket.
  *
  * @param  sock number
  *         level gives the layer on which the option resides
  *         level is always specified at socket level in this implementation
  *         optname
  *         optvalue points to a valid value when a option name is entered
  *         optlen
  * @retval ret
  *          < 0   : if there is an error.
  *          = 0   : on success
  */
static int32_t net_cellular_setsockopt(int32_t sock, int32_t level, int32_t optname, const void *optvalue,
                                       uint32_t optlen)
{
  int32_t ret = NET_ERROR_PARAMETER;
  /*cstat -MISRAC2012-Rule-11.5 -MISRAC2012-Rule-11.8 */
  int32_t       *optint32 = (int32_t *) optvalue;
  /*cstat +MISRAC2012-Rule-11.5 +MISRAC2012-Rule-11.8 */
  (void) level;
  (void) optlen;
  if ((sock < 0) || (sock >= NET_CELLULAR_MAX_CHANNEL_NBR))
  {
    NET_DBG_ERROR("invalid socket");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    switch (optname)
    {
      case NET_SO_RCVTIMEO:
        if (NULL != optvalue)
        {
          if (0 == com_setsockopt(sock, COM_SOL_SOCKET, COM_SO_RCVTIMEO, optint32, (int32_t) sizeof(int)))
          {
            ret = NET_OK;
          }
        }
        break;

      case NET_SO_SNDTIMEO:
        if (NULL != optvalue)
        {
          if (0 == com_setsockopt(sock, COM_SOL_SOCKET, COM_SO_SNDTIMEO, optint32, (int32_t) sizeof(int)))
          {
            ret = NET_OK;
          }
        }
        break;

      default:
        ret = NET_ERROR_UNSUPPORTED;
        NET_DBG_ERROR("Options %lu not supported in the modem", optname);
        break;
    }
  }

  return ret;
}


static int32_t net_cellular_getsockopt(int32_t sock, int32_t level, int32_t optname, void *optvalue, uint32_t *optlen)
{
  (void) sock;
  (void) level;
  (void) optname;
  (void) optvalue;
  (void) optlen;
  return NET_ERROR_UNSUPPORTED;
}
static int32_t net_cellular_getsockname(int32_t sock, net_sockaddr_t *name, uint32_t *namelen)
{
  (void) sock;
  (void) name;
  (void) namelen;
  return NET_ERROR_UNSUPPORTED;
}
static int32_t net_cellular_getpeername(int32_t sock, net_sockaddr_t *name, uint32_t *namelen)
{
  (void) sock;
  (void) name;
  (void) namelen;
  return NET_ERROR_UNSUPPORTED;
}

/**
  * @brief  Free a socket, disconnecting it from the remotehost, if necessary.
  *
  * @param  sock number
  *         isaclone, not supported
  * @retval ret
  *          < 0   : if there is an error.
  *          = 0   : on success
  */
static int32_t net_cellular_close(int32_t sock, bool isaclone)
{
  int32_t ret;
  (void) isaclone;
  if ((sock < 0) || (sock >= NET_CELLULAR_MAX_CHANNEL_NBR))
  {
    NET_DBG_ERROR("invalid socket");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    NET_DBG_INFO("Trying to close socket %ld\n", sock);

    if (com_closesocket(sock) == COM_SOCKETS_ERR_OK)
    {
      NET_DBG_INFO("socket %ld is now closed\n", sock);
      ret = NET_OK;
    }
    else
    {
      NET_DBG_ERROR("socket %ld cannot be closed", sock);
      ret = NET_ERROR_SOCKET_FAILURE;
    }
    CellularChannel[sock].status = CELLULAR_FREE_SOCKET;
  }
  return ret;
}

static int32_t net_cellular_shutdown(int32_t sock, int32_t mode)
{
  int32_t ret = NET_OK;

  if ((sock < 0) || (sock >= NET_CELLULAR_MAX_CHANNEL_NBR))
  {
    NET_DBG_ERROR("invalid socket");
    ret = NET_ERROR_INVALID_SOCKET;
  }
  else
  {
    NET_DBG_INFO("Force local Shutdown on socket %ld\n", sock);

    if ((CellularChannel[sock].status & CELLULAR_CONNECTED_SOCKET) != 0U)
    {
      /* reading on the socket is no more possible */
      if (mode == NET_SHUTDOWN_R)
      {
        CellularChannel[sock].status &= (uint8_t)(~CELLULAR_RECV_OK);
      }

      /* writing on the socket is no more possible */
      if (mode == NET_SHUTDOWN_W)
      {
        CellularChannel[sock].status &= (uint8_t)(~CELLULAR_SEND_OK);
      }

      /* reading/writing on the socket is no more possible */
      if (mode == NET_SHUTDOWN_RW)
      {
        CellularChannel[sock].status &= (uint8_t)(~(CELLULAR_SEND_OK | CELLULAR_RECV_OK));
      }
    }
  }
  return ret;
}

static int32_t net_cellular_if_gethostbyname(net_if_handle_t *pnetif, net_sockaddr_t *addr, char_t *name)
{
  int32_t ret = NET_ERROR_DNS_FAILURE;
  com_sockaddr_t remoteaddr;
  (void) pnetif;

  if (addr->sa_len < sizeof(net_sockaddr_in_t))
  {
    ret = NET_ERROR_PARAMETER;
  }
  else
  {
    remoteaddr.sa_len = (uint8_t) sizeof(com_sockaddr_t);

    if (COM_SOCKETS_ERR_OK == com_gethostbyname((const com_char_t *)name, &remoteaddr))
    {
      com2netsockaddr(addr, &remoteaddr, sizeof(net_sockaddr_t));
      if (remoteaddr.sa_family == (uint8_t) COM_AF_INET)
      {
        ret = NET_OK;
      }
    }
  }
  return ret;
}

static int32_t net_cellular_ping(net_if_handle_t *pnetif, net_sockaddr_t *addr, int32_t count,
                                 int32_t delay,
                                 int32_t response[])
{
  int32_t ret = NET_OK;
  uint8_t     ipaddr[4];

  int32_t  ping_handle;
  int32_t  ping_result;
  uint8_t  ping_counter;

  com_sockaddr_t ping_target;
  com_ping_rsp_t    ping_rsp;

  (void) pnetif;
  netsockaddr2com(&ping_target, addr, sizeof(net_sockaddr_t));
  (void) memcpy(ipaddr, &addr->sa_data[2], 4);


  ping_handle = com_ping();

  if (ping_handle >= 0)
  {
    for (ping_counter = 0; ping_counter < (uint8_t) count; ping_counter++)
    {
      ping_result = com_ping_process(ping_handle,
                                     &ping_target,
                                     (int32_t)sizeof(com_sockaddr_t),
                                     (uint8_t) delay, &ping_rsp);

      if ((ping_result == COM_SOCKETS_ERR_OK)
          && (ping_rsp.status == COM_SOCKETS_ERR_OK))
      {
        NET_DBG_INFO("Ping: from %d.%d.%d.%d: time= %dms\r\n",
                     ipaddr[0],
                     ipaddr[1],
                     ipaddr[2],
                     ipaddr[3],
                     ping_rsp.time);
        response[ping_counter] = (int32_t) ping_rsp.time;
      }
      else
      {
        if (ping_result == COM_SOCKETS_ERR_TIMEOUT)
        {
          NET_DBG_INFO("Ping: timeout from %d.%d.%d.%d\r\n",
                       ipaddr[0],
                       ipaddr[1],
                       ipaddr[2],
                       ipaddr[3]);
          ret = NET_TIMEOUT;
        }
        else
        {
          NET_DBG_INFO("Ping: error from %d.%d.%d.%d\r\n",
                       ipaddr[0],
                       ipaddr[1],
                       ipaddr[2],
                       ipaddr[3]);
          ret = NET_ERROR_GENERIC;
        }
      }
    }

    if (com_closeping(ping_handle) != COM_SOCKETS_ERR_OK)
    {
      NET_DBG_ERROR("ping cannot be closed");
    }
  }
  else
  {
    /* Ping handle not received */
    NET_DBG_ERROR("Ping: low-level not ready");
    ret = ping_handle;
  }

  return ret;
}


static int32_t net_cellular_get_radio_info(net_cellular_radio_results_t *results)
{
  int32_t ret = NET_ERROR_PARAMETER;

  if (DC_SERVICE_ON == dc_cellular_info.rt_state)
  {
    results->signal_level_db = (int8_t)dc_cellular_info.cs_signal_level_db;
    ret = NET_OK;
  }
  return ret;
}

#define ONESECOND       1000

static void cellif_input(void const  *argument)
{
  static dc_cellular_data_info_t  dc_cellular_data_info;
  static dc_nifman_info_t dc_nifman_info;
  static dc_sim_info_t dc_sim_info;
  /*cstat -MISRAC2012-Rule-11.5 -MISRAC2012-Rule-11.8 */
  net_if_handle_t *pnetif = (net_if_handle_t *)argument;
  /*cstat +MISRAC2012-Rule-11.5 +MISRAC2012-Rule-11.8 */
  osEvent event;
  dc_com_event_id_t dc_event_id;
  bool  registrationMessage = false;
  bool  ModuleMessage = false;
  int32_t   levelMessage = 1;
  uint32_t   tickstart;
  uint32_t   tickcurrent;
  int32_t   connection_iteration = 0;

  const net_cellular_credentials_t *cellular_data =  pnetif->pdrv->extension.cellular->credentials;

  tickstart = HAL_GetTick();
#ifdef NET_CELLULAR_CREDENTIAL_V2
  cellular_set_config(cellular_data);
#else
  cellular_set_config(cellular_data->apn,
                      cellular_data->username,
                      cellular_data->password,
                      cellular_data->use_internal_sim);
#endif /* NET_CELLULAR_CREDENTIAL_V2 */
  cellular_start();
  (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_INITIALIZED);

  /* Registration for Cellular Data Cache */
  (void) dc_com_register_gen_event_cb(&dc_com_db, cellular_notif_cb, (void *) NULL);


  (void) memset((void *)&dc_nifman_info,     0, sizeof(dc_nifman_info_t));
  (void) memset((void *)&dc_cellular_info,   0, sizeof(dc_cellular_info_t));
  (void) memset((void *)&dc_sim_info,        0, sizeof(dc_sim_info_t));

  /* Infinite loop */
  for (;;)
  {
    event = osMessageGet(cellular_queue, ONESECOND);

    if (event.status == osEventMessage)
    {
      /*cstat -MISRAC2012-Rule-19.2 union*/
      dc_event_id = (dc_com_event_id_t)event.value.v;
      /*cstat +MISRAC2012-Rule-19.2 */
      NET_DBG_INFO(" \n\n****> event %d get from cellular_queue\n\n", dc_event_id);

      if (dc_event_id == DC_COM_CELLULAR_INFO)
      {
        NET_DBG_INFO("\n***** cellular_info available *****\n");

        (void) dc_com_read(&dc_com_db,
                           DC_COM_CELLULAR_INFO,
                           (void *)&dc_cellular_info,
                           sizeof(dc_cellular_info));
        NET_DBG_INFO("- cellular_info cb -\n");

        NET_DBG_INFO("   state              = %d\n", dc_cellular_info.rt_state);

        if ((dc_cellular_info.rt_state == DC_SERVICE_RUN) || (dc_cellular_info.rt_state == DC_SERVICE_ON))
        {
          NET_DBG_INFO("   modem_state        = %d\n", dc_cellular_info.modem_state);
          NET_DBG_INFO("   mno_name           = %s\n", dc_cellular_info.mno_name);
          NET_DBG_INFO("   state              = %d\n", dc_cellular_info.rt_state);
          NET_DBG_INFO("   modem_state        = %d\n", dc_cellular_info.modem_state);
          NET_DBG_INFO("   cs_signal_level    = %d\n", (uint8_t)dc_cellular_info.cs_signal_level);
          NET_DBG_INFO("   cs_signal_level_db = %d\n", (int8_t)dc_cellular_info.cs_signal_level_db);
          NET_DBG_INFO("   imei               = %s\n", (char_t *)&dc_cellular_info.imei);
          NET_DBG_INFO("   manufacturer_name  = %s\n", (char_t *)&dc_cellular_info.manufacturer_name);
          NET_DBG_INFO("   model              = %s\n", (char_t *)&dc_cellular_info.model);
          NET_DBG_INFO("   revision           = %s\n", (char_t *)&dc_cellular_info.revision);
          NET_DBG_INFO("   serial_number      = %s\n", (char_t *)&dc_cellular_info.serial_number);
          NET_DBG_INFO("   iccid              = %s\n", (char_t *)&dc_cellular_info.iccid);
          NET_DBG_INFO("--------------------\n");

          if (registrationMessage == false)
          {
            if (strlen((char_t *)&dc_cellular_info.mno_name) > 0U)
            {
              NET_PRINT("\nC2C module registered\n");
              tickcurrent = HAL_GetTick() - tickstart;
              NET_PRINT("Registration done in %ld msseconds\n", tickcurrent);
              NET_PRINT("Retrieving the cellular operator: %s\n",
                        (char_t *)&dc_cellular_info.mno_name);
              registrationMessage = true;
            }
          }

          if (ModuleMessage == false)
          {
            if (strlen((char_t *)&dc_cellular_info.manufacturer_name) > 0U)
            {
              NET_PRINT("Module initialized successfully: %s\n",
                        (char_t *)&dc_cellular_info.manufacturer_name);
              NET_PRINT("ProductID: %s\n",
                        (char_t *)&dc_cellular_info.model);
              NET_PRINT("FW version: %s\n",
                        (char_t *)&dc_cellular_info.revision);
              NET_PRINT("SIM Id (IccID): %s\n",
                        (char_t *)&dc_cellular_info.iccid);
              ModuleMessage = true;

              /* Retrieve the Cellular module information */
              (void) strncpy(pnetif->DeviceName, (char_t *)&dc_cellular_info.manufacturer_name,
                             NET_DEVICE_NAME_LEN);
              (void) strncpy(pnetif->DeviceID, (char_t *)&dc_cellular_info.model, NET_DEVICE_ID_LEN);
              (void) strncpy(pnetif->DeviceVer, (char_t *)&dc_cellular_info.revision, NET_DEVICE_VER_LEN);

              /* Initialise the Channels*/
              for (int32_t i = 0; i < NET_CELLULAR_MAX_CHANNEL_NBR; i++)
              {
                CellularChannel[i].status          = CELLULAR_FREE_SOCKET;
              }
              (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_READY);
            }
          }
        }

        /* Check switch off  */
        if ((stop_requested) && (dc_cellular_info.rt_state != DC_SERVICE_ON))
        {
          NET_DBG_INFO("Modem is powered off\n\n");
          stop_requested = false;
          (void) net_state_manage_event(pnetif, NET_EVENT_INTERFACE_INITIALIZED);
          (void) osThreadTerminate(CellIfThreadId);
          CellIfThreadId = NULL;
        }

        if (dc_cellular_info.rt_state == DC_SERVICE_ON)
        {
          /* check signal level availability or not, displayed only once */
          if (levelMessage != dc_cellular_info.cs_signal_level_db)
          {
            if (dc_cellular_info.cs_signal_level_db != 0)
            {
              NET_PRINT("\nSignal Level: %d dBm \n",
                        (int8_t)dc_cellular_info.cs_signal_level_db);
            }
            else
            {
              NET_PRINT("Signal not known or not detectable yet (be patient)\n");
            }
            levelMessage = dc_cellular_info.cs_signal_level_db;
          }
        }
      }
      else if (dc_event_id == DC_COM_CELLULAR_DATA_INFO)
      {
        NET_DBG_INFO("\n***** cellular_data_info available *****\n");
        (void) dc_com_read(&dc_com_db,
                           DC_COM_CELLULAR_DATA_INFO,
                           (void *)&dc_cellular_data_info,
                           sizeof(dc_cellular_data_info));
        NET_DBG_INFO("- dc_cellular_data_info cb -\n");
        NET_DBG_INFO("   state              = %d\n", dc_cellular_data_info.rt_state);
        NET_DBG_INFO("----------------------------\n");
      }
      else if (dc_event_id == DC_COM_SIM_INFO)
      {
        /* check SIM availability */
        (void) dc_com_read(&dc_com_db,
                           DC_COM_SIM_INFO,
                           (void *)&dc_sim_info,
                           sizeof(dc_sim_info));
        NET_DBG_INFO("\n***** sim_info available *****\n");
        NET_DBG_INFO("   state              = %d\n", dc_sim_info.rt_state);
        if (dc_sim_info.rt_state == DC_SERVICE_ON)
        {
          NET_DBG_INFO("- dc_sim_info cb -\n");
          NET_DBG_INFO("   active_slot        = %d\n", dc_sim_info.active_slot);
          NET_DBG_INFO("   sim_status         = %d\n", dc_sim_info.sim_status[dc_sim_info.active_slot]);
          NET_DBG_INFO("------------------\n");
          if (dc_sim_info.sim_status[dc_sim_info.active_slot] == DC_SIM_NOT_INSERTED)
          {
            NET_PRINT("SIM is not inserted\n");
          }
        }
      }
      else if (dc_event_id == DC_COM_NIFMAN_INFO)
      {
        NET_DBG_INFO("\n***** nifman_info available *****\n");
        (void) dc_com_read(&dc_com_db,
                           DC_COM_NIFMAN_INFO,
                           (void *)&dc_nifman_info,
                           sizeof(dc_nifman_info));

        if (connection_requested && (dc_nifman_info.rt_state == DC_SERVICE_ON))
        {
          NET_DBG_INFO(" +++++++++++++++++++ Cellular is Data READY +++++++++++++++++++++\n\n");
          NET_DBG_INFO(" +++++++++++++++++++ IP @ allocated +++++++++++++++++++++\n\n");
          pnetif->ipaddr.addr = dc_nifman_info.ip_addr.addr;
          connection_requested = false;
          (void) net_state_manage_event(pnetif, NET_EVENT_IPADDR);
        }
        NET_DBG_INFO("- nifman_info cb -\n");
        NET_DBG_INFO("   state              = %d\n", dc_nifman_info.rt_state);
        NET_DBG_INFO("   network            = %d\n", dc_nifman_info.network);
        NET_DBG_INFO("   IP@                = %lu\n", dc_nifman_info.ip_addr.addr);
        NET_DBG_INFO("------------------\n");
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
      }
      else if (dc_event_id == DC_COM_PPP_CLIENT_INFO)
      {
        dc_ppp_client_info_t       dc_ppp_client_info;
        NET_DBG_INFO("\n***** ppp_client_info available *****\n");
        (void) dc_com_read(&dc_com_db,
                           DC_COM_PPP_CLIENT_INFO,
                           (void *)&dc_ppp_client_info,
                           sizeof(dc_ppp_client_info));
        NET_DBG_INFO("-- ppp_client_info cb --\n");
        NET_DBG_INFO("   state              = %d\n", dc_ppp_client_info.rt_state);
        NET_DBG_INFO("   IP@                = %lu\n", dc_ppp_client_info.ip_addr.addr);
        NET_DBG_INFO("-------------------------\n");
#endif /* USE_SOCKETS_LWIP */
      }
      else
      {
        NET_DBG_INFO("Unexpected Event message : %d \n", dc_event_id);
      }
    }

    if ((registrationMessage == false) || (ModuleMessage == false))
    {
      NET_PRINT_WO_CR(".");
      connection_iteration++;
      if ((connection_iteration % 60) == 0)
      {
        NET_PRINT(" ");
      }
    }
  }
}

/**
  * @brief  Configure the SIM(s)
  *
  * @param  Access Point Name
  *         Username
  *         password
  *         APN, Username and password are all provided by the cellular operator
  *         use_internal_sim,
  *         - true for the external slot with a dynamic APN
  *         - false with the embedded SIM with a static APN
  */
#ifdef NET_CELLULAR_CREDENTIAL_V2
static void cellular_set_config(const net_cellular_credentials_t *credentials)
{
  dc_cellular_params_t cellular_params;
  uint8_t i;

  (void) memset((void *)&cellular_params,     0, sizeof(dc_cellular_params_t));
  (void) dc_com_read(&dc_com_db, DC_COM_CELLULAR_PARAM, (void *)&cellular_params, sizeof(cellular_params));

  cellular_params.sim_slot_nb = credentials->sim_socket_nb;
  cellular_params.target_state = DC_TARGET_STATE_SIM_ONLY;
  for (i = 0; i < NET_CELLULAR_MAX_SUPPORTED_SLOT ; i++)
  {
    cellular_params.sim_slot[i].sim_slot_type = credentials->sim_slot[i].sim_slot_type;
    cellular_params.sim_slot[i].cid           = credentials->sim_slot[i].cid ;

    (void)strcpy((char_t *)cellular_params.sim_slot[i].apn, (const char_t *)credentials->sim_slot[i].apn);
    (void)strcpy((char_t *)cellular_params.sim_slot[i].username, credentials->sim_slot[i].username);
    (void)strcpy((char_t *)cellular_params.sim_slot[i].password, credentials->sim_slot[i].password);
  }
  /* Initialize all other fields */
  cellular_params.set_pdn_mode = 1U;
  cellular_params.target_state = DC_TARGET_STATE_SIM_ONLY;
  cellular_params.nfmc_active  = 0U;
#ifdef CELLULAR_VERSION_FIRMWARE_VERSION
  cellular_params.attachment_timeout  = CELLULAR_ATTACHMENT_TIMEOUT;
#endif /* CELLULAR_VERSION_FIRMWARE_VERSION */

  (void) dc_com_write(&dc_com_db, DC_COM_CELLULAR_PARAM, (void *)&cellular_params, sizeof(cellular_params));
}
#else
static void cellular_set_config(const char_t *oper_ap_code,
                                const char_t *username,
                                const char_t *password,
                                bool use_internal_sim)
{
  dc_cellular_params_t cellular_params;

  (void) memset((void *)&cellular_params,     0, sizeof(dc_cellular_params_t));
  (void) dc_com_read(&dc_com_db, DC_COM_CELLULAR_PARAM, (void *)&cellular_params, sizeof(cellular_params));



  if (use_internal_sim == true)
  {
    /* Specify which SIM slot to use at cellular boot time */
    cellular_params.sim_slot[0].sim_slot_type = DC_SIM_SLOT_MODEM_EMBEDDED_SIM;
    cellular_params.sim_slot_nb = 1;

    /* Specify operator credencials : Access Point Number, username and password */
    (void) memcpy(cellular_params.sim_slot[0].apn, "EM", (size_t)sizeof("EM"));

    /* Specify the Context ID */
    cellular_params.sim_slot[0].cid          = CS_PDN_USER_CONFIG_1;

    NET_PRINT("Trying to connect with the embedded SIM\n");
  }
  else
  {
    /* Specify which SIM slot to use at cellular boot time */
    cellular_params.sim_slot[0].sim_slot_type = DC_SIM_SLOT_MODEM_SOCKET;
    cellular_params.sim_slot[1].sim_slot_type = DC_SIM_SLOT_MODEM_EMBEDDED_SIM;
    cellular_params.sim_slot_nb = 2;

    /* Specify operator credencials : Access Point Number, username and password */
    (void) memcpy(cellular_params.sim_slot[0].apn, oper_ap_code, strlen(oper_ap_code) + 1U);
    (void) memcpy(cellular_params.sim_slot[0].username, username, strlen(username) + 1U);
    (void) memcpy(cellular_params.sim_slot[0].password, password, strlen(password) + 1U);

    (void) memcpy(cellular_params.sim_slot[1].apn, "EM", (size_t)sizeof("EM"));


    /* Specify the Context ID */
    cellular_params.sim_slot[0].cid          = CS_PDN_USER_CONFIG_1;
    cellular_params.sim_slot[1].cid          = CS_PDN_USER_CONFIG_1;

    NET_PRINT("Trying to connect with the external SIM\n");
  }

  /* Initialize all other fields */
  cellular_params.set_pdn_mode = 1U;
  cellular_params.target_state = DC_TARGET_STATE_SIM_ONLY;
  cellular_params.nfmc_active  = 0U;
#ifdef CELLULAR_VERSION_FIRMWARE_VERSION
  cellular_params.attachment_timeout  = CELLULAR_ATTACHMENT_TIMEOUT;
#endif /* CELLULAR_VERSION_FIRMWARE_VERSION */

  (void) dc_com_write(&dc_com_db, DC_COM_CELLULAR_PARAM, (void *)&cellular_params, sizeof(cellular_params));
}

#endif /* NET_CELLULAR_CREDENTIAL_V2 */


static void cellular_notif_cb(dc_com_event_id_t dc_event_id, const void *private_gui_data)
{
  (void) private_gui_data;
  if ((dc_event_id == DC_COM_CELLULAR_INFO) ||
#if (USE_SOCKETS_TYPE == USE_SOCKETS_LWIP)
      (dc_event_id == DC_COM_PPP_CLIENT_INFO) ||
#endif /* USE_SOCKETS_LWIP */
      (dc_event_id == DC_COM_CELLULAR_DATA_INFO) ||
      (dc_event_id == DC_COM_NIFMAN_INFO) ||
      (dc_event_id == DC_COM_SIM_INFO))
  {
    NET_DBG_INFO(" \n\n****> event %d put in cellular_queue\n\n", dc_event_id);
    while (osMessagePut(cellular_queue, (uint32_t)dc_event_id, OSWAITFOREVER) != osOK) {};
  }
}





/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
