/**
  ******************************************************************************
  * @file    net_ping.c
  * @author  MCD Application Team
  * @brief   application to send icmp ping
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
/*cstat -MISRAC* -DEFINE-* -CERT-EXP19*  */
#include "lwip/tcpip.h"
#include "lwip/icmp.h"
#include "lwip/inet_chksum.h"
#include "lwip/api.h" /* HAL_GetTick() */
#include "lwip/ip4.h"
/*cstat +MISRAC* +DEFINE-* +CERT-EXP19*  */



/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


/** ping identifier - must fit on a u16_t */
#ifndef PING_ID
#define PING_ID        0xAFAF
#endif /* PING_ID */

/** ping additional data size to include in the packet */
#ifndef PING_DATA_SIZE
#define PING_DATA_SIZE 32
#endif /* PING_DATA_SIZE */






/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static void ping_prepare_echo(struct icmp_echo_hdr *iecho, uint16_t len, uint16_t ping_seq_num)
{
  size_t i;
  size_t data_len = len - sizeof(struct icmp_echo_hdr);

  ICMPH_TYPE_SET(iecho, ICMP_ECHO);
  ICMPH_CODE_SET(iecho, 0);
  iecho->chksum = 0;
  iecho->id     = PING_ID;
  iecho->seqno  = (uint16_t) lwip_htons(ping_seq_num);

  /* fill the additional data buffer with some data */
  for (i = 0; i < data_len; i++)
  {
    ((char_t *)iecho)[sizeof(struct icmp_echo_hdr) + i] = (char_t)i;
  }
  /* Ping data are sent in RAM mode , so LWIP is not computing the checksum by SW */
#ifndef CHECKSUM_BY_HARDWARE
  iecho->chksum = inet_chksum(iecho, len);
#endif /* CHECKSUM_BY_HARDWARE */
}

uint32_t        sys_now(void);


#define NET_IPH_HL(hdr) ((hdr)->_v_hl & 0x0fU)

int32_t icmp_ping(net_if_handle_t *pnetif, net_sockaddr_t *addr, int32_t count, int32_t timeout, int32_t response[])
{
  int32_t sock;
  int32_t ret = 0;
  uint32_t ping_start_time;
  net_sockaddr_t from;
  uint32_t fromlen;
  int32_t len;
  static int32_t ping_seq_num = 1;
  char_t buf[64] = "";
  struct ip_hdr *iphdr;
  struct icmp_echo_hdr *iecho, *pecho = NULL;
  size_t ping_size = sizeof(struct icmp_echo_hdr) + (uint32_t) PING_DATA_SIZE;
  u16_t seqnum;
  (void) pnetif;
  sock = net_socket(NET_AF_INET, NET_SOCK_RAW, NET_IPPROTO_ICMP);
  if (sock < 0)
  {
    NET_DBG_ERROR("ping: socket fail\r\n");
    ret = -1;
  }
  else if (net_setsockopt(sock, NET_SOL_SOCKET, NET_SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
  {
    NET_DBG_ERROR("ping: setsockopt() fail\r\n");
    ret = -1;
  }
  else
  {
    /*cstat -MISRAC2012-Rule-11.5 Malloc */
    pecho = (struct icmp_echo_hdr *)mem_malloc((mem_size_t)ping_size);
    /*cstat +MISRAC2012-Rule-11.5 */

    if (pecho == NULL)
    {
      NET_DBG_ERROR("ping_client_process : message alloc fails\r\n");
      ret = -1;
    }
  }

  if (ret == 0)
  {
    addr->sa_family      = NET_AF_INET;
    net_set_port(addr, 0U);

    for (int32_t i = 0; i < count; i++)
    {
      response[i] = -1;
      /* add useless test for MISRA on pecho */
      if (pecho != NULL)
      {
        ping_prepare_echo(pecho, (uint16_t) ping_size, (uint16_t) ping_seq_num);
      }

      if (net_sendto(sock, (uint8_t *)pecho, (int32_t) ping_size, 0, addr, (int32_t) sizeof(net_sockaddr_t)) < 0)
      {
        NET_DBG_INFO("ping_client_process : send fail\r\n");
        break;
      }

      ping_start_time = sys_now();
      do
      {

        len = net_recvfrom(sock, (uint8_t *)buf, (int32_t) ping_size, 0, &from, &fromlen);
        if (len >= (int32_t)(sizeof(struct ip_hdr) + sizeof(struct icmp_echo_hdr)))
        {

          /*cstat -MISRAC2012-Rule-11.3 Cast */
          iphdr = (struct ip_hdr *)buf;
          iecho = (struct icmp_echo_hdr *)(buf + ((NET_IPH_HL(iphdr)) * 4U));
          /*cstat +MISRAC2012-Rule-11.3 Cast */
          seqnum = lwip_htons((uint16_t) ping_seq_num);
          if ((iecho->id == (uint16_t)PING_ID) && (iecho->seqno == seqnum))
          {
            if (ICMPH_TYPE(iecho) == (uint8_t) ICMP_ER)
            {
              uint32_t  delta;
              ret = 0;
              delta = sys_now() - ping_start_time;
              response[i] = (int32_t) delta;
              break;
            }
            else
            {
              NET_DBG_ERROR("ICMP Other Response received \r\n");
            }
          }
        }
        else
        {
          uint32_t now = sys_now();
          NET_DBG_ERROR("no data start %ld : %lu  .. %lu\r\n", len, ping_start_time,  now);
        }
      } while (sys_now() < (ping_start_time + (uint32_t) timeout));
      ping_seq_num++;
    }
    if (pecho != NULL)
    {
      mem_free(pecho);
    }
    (void) net_closesocket(sock);
  }
  return ret;
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
