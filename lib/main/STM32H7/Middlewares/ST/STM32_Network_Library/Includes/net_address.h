/**
  ******************************************************************************
  * @file    net_address.h
  * @author  MCD Application Team
  * @brief   Header for the network address management functions
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

#ifndef NET_ADDRESS_H
#define NET_ADDRESS_H

#include "string.h"
#include <stdlib.h>
#include "net_types.h"

#define NET_ZERO(a)     (void) memset(&a,0,sizeof(a))
#define NET_EQUAL(a,b)   (memcmp(&(a),&(b),sizeof(a))==0)
#define NET_DIFF(a,b)    (memcmp(&(a),&(b),sizeof(a))!=0)
#define NET_COPY(a,b)    (void) memcpy(&a,&b,sizeof(a))

#ifdef NET_USE_LWIP_DEFINITIONS
/*cstat -MISRAC* -DEFINE-* -CERT-EXP19*  */
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"
#include "lwip/inet.h"

/*cstat +MISRAC* +DEFINE-* +CERT-EXP19*  */
typedef struct sockaddr_in       net_sockaddr_in_t;
typedef struct sockaddr_in6       net_sockaddr_in6_t;
typedef struct sockaddr          net_sockaddr_t;
typedef ip_addr_t                net_ip_addr_t;
typedef in_addr_t                net_in_addr_t;


#define IP4ADDR_PORT_TO_SOCKADDR(sin, ipaddr, port) do { \
                                                         (sin)->sin_len = (uint8_t) sizeof(struct sockaddr_in); \
                                                         (sin)->sin_family = AF_INET; \
                                                         (sin)->sin_port = lwip_htons((port)); \
                                                         inet_addr_from_ip4addr(&(sin)->sin_addr, ipaddr); \
                                                       (void) memset((void*)(sin)->sin_zero, 0, SIN_ZERO_LEN); }while(0)


#define NET_IPADDR_PORT_TO_SOCKADDR(sockaddr, ipaddr, port) \
                                                        IP4ADDR_PORT_TO_SOCKADDR(( struct sockaddr_in*)( void*)(sockaddr), ipaddr, port)


#define SOCKADDR4_TO_IP4ADDR_PORT(sin, ipaddr, port) do { \
                                                         inet_addr_to_ip4addr(ip_2_ip4(ipaddr), &((sin)->sin_addr)); \
                                                       (port) = lwip_ntohs((sin)->sin_port); }while(0)

#define NET_SOCKADDR_TO_IPADDR_PORT(sockaddr, ipaddr, port) \
                                                       SOCKADDR4_TO_IP4ADDR_PORT((const struct sockaddr_in*)(const void*)(sockaddr), ipaddr, port)



#define NET_IP_ADDR_CMP                         ip_addr_cmp
#define NET_IP_ADDR_ISANY_VAL                   ip_addr_isany_val
#define NET_IP_ADDR_COPY                        ip_addr_copy
#define NET_HTONL                               htonl
#define NET_NTOHL                               ntohl
#define NET_HTONS                               htons
#define NET_NTOHS                               ntohs
#define NET_NTOA_R                              ipaddr_ntoa_r
#define NET_NTOA                                ipaddr_ntoa
#define NET_ATON                                ip4addr_aton
#define NET_ATON_R                              ipaddr_addr
#define NET_IPADDR4_INIT(u32val)                { u32val }
#define NET_IPADDR4_INIT_BYTES(a,b,c,d)         NET_IPADDR4_INIT(PP_HTONL(LWIP_MAKEU32(a,b,c,d)))


#else /* NET_USE_LWIP_DEFINTIONS */


#define NET_IP_ADDR_CMP(addr1, addr2) ((addr1)->addr == (addr2)->addr)
#define NET_IP_ADDR_COPY(dest, src) ((dest).addr = (src).addr)
/** Safely copy one IP address to another (src may be NULL) */
#define NET_IP_ADDR_SET(dest, src) ((dest)->addr = \
                                                         ((src) == NULL ? 0 : \
                                                         (src)->addr))
/** Set complete address to zero */
#define NET_IP_ADDR_SET_ZERO(ipaddr)     ((ipaddr)->addr = 0U)
#define NET_IP_ADDR_ISANY_VAL(addr1)   ((addr1).addr == 0U)

#define NET_SOCKADDR_TO_IPADDR_PORT(sockaddr, ipaddr, port)  do { \
                                                           (ipaddr)->addr = (sockaddr)->sin_addr.s_addr;\
                                                         (port) = NET_NTOHS((sockaddr)->sin_port); }while(0)

#define NET_IP4ADDR_PORT_TO_SOCKADDR(sin, ipaddr, port) do { \
                                                           (sin)->sin_len = (uint8_t) sizeof(net_sockaddr_in_t); \
                                                           (sin)->sin_family = NET_AF_INET; \
                                                           (sin)->sin_port = NET_HTONS((port)); \
                                                           (sin)->sin_addr.s_addr = (ipaddr)->addr;\
                                                         memset((void*)(sin)->sin_zero, 0, NET_SIN_ZERO_LEN); }while(0)

#define NET_IPADDR_PORT_TO_SOCKADDR(sockaddr, ipaddr, port) \
                                                         NET_IP4ADDR_PORT_TO_SOCKADDR((  net_sockaddr_in_t*)( void*)(sockaddr), ipaddr, port)


#define NET_HTONL(A)   ((((uint32_t)(A) & 0xff000000U) >> 24) | \
                                                         (((uint32_t)(A) & 0x00ff0000U) >> 8) | \
                                                         (((uint32_t)(A) & 0x0000ff00U) << 8) | \
                                                         (((uint32_t)(A) & 0x000000ffU) << 24))

#define NET_NTOHL       NET_HTONL

#define NET_HTONS(A)     ((((uint16_t)(A) & 0xff00U) >> 8U) | \
                                                         (((uint16_t)(A) & 0x00ffU) << 8U))
#define NET_NTOHS       NET_HTONS


#define NET_NTOA                               net_ntoa
#define NET_NTOA_R                             net_ntoa_r
#define NET_ATON                               net_aton
#define NET_ATON_R                             net_aton_r


#define NET_NULL_IP_ADDR       0U

#define NET_PP_HTONL(x) ((((x) & 0x000000ffUL) << 24) | \
                                                         (((x) & 0x0000ff00UL) <<  8) | \
                                                         (((x) & 0x00ff0000UL) >>  8) | \
                                                         (((x) & 0xff000000UL) >> 24))


#define NET_ASLWIP_MAKEU32(a,b,c,d) (((uint32_t)((a) & 0xff) << 24) | \
                                                         ((uint32_t)((b) & 0xff) << 16) | \
                                                         ((uint32_t)((c) & 0xff) << 8)  | \
                                                         (uint32_t)((d) & 0xff))


#define NET_IPADDR4_INIT(u32val)                { u32val }
#define NET_IPADDR4_INIT_BYTES(a,b,c,d)          NET_IPADDR4_INIT(NET_PP_HTONL(NET_ASLWIP_MAKEU32(a,b,c,d)))

#define inet_addr_from_ip4addr(target_inaddr, source_ipaddr) ((target_inaddr)->s_addr = ip4_addr_get_u32(source_ipaddr))
#define inet_addr_to_ip4addr(target_ipaddr, source_inaddr)   (ip4_addr_set_u32(target_ipaddr, (source_inaddr)->s_addr))
/* ATTENTION: the next define only works because both s_addr and ip4_addr_t are an u32_t effectively! */
#define inet_addr_to_ip4addr_p(target_ip4addr_p, source_inaddr)   ((target_ip4addr_p) = (ip4_addr_t*\
                                                           )&((source_inaddr)->s_addr))
/** IPv4 only: set the IP address given as an u32_t */
#define ip4_addr_set_u32(dest_ipaddr, src_u32) ((dest_ipaddr)->addr = (src_u32))
/** IPv4 only: get the IP address as an u32_t */
#define ip4_addr_get_u32(src_ipaddr) ((src_ipaddr)->addr)

/* generic socket address structure to support IPV6 and IPV4              */
/* size is 16 bytes and is aligned on LWIP definition to ease integration */
typedef struct
{
  uint32_t addr;
} net_ip4_addr_t;

typedef struct net_in_addr
{
  uint32_t s_addr;
} net_in_addr_t;

/* Only IPv4 is managed */
typedef net_ip4_addr_t net_ip_addr_t;

typedef struct net_sockaddr
{
  uint8_t    sa_len;
  uint8_t    sa_family;
  char_t sa_data[14];
} net_sockaddr_t;

/* IPV4 address , with 8 stuffing byte */
#define NET_SIN_ZERO_LEN    8
typedef struct net_sockaddr_in
{
  uint8_t       sin_len;
  uint8_t       sin_family;
  uint16_t      sin_port;
  net_in_addr_t sin_addr;
  char_t        sin_zero[NET_SIN_ZERO_LEN];
} net_sockaddr_in_t;

char_t *net_ntoa(const net_ip_addr_t *addr);
char_t *net_ntoa_r(const net_ip_addr_t *addr, char_t *buf, int32_t buflen);
int32_t net_aton_r(const char_t *cp);
int32_t net_aton(const char_t *cp, net_ip_addr_t *addr);


#endif /* NET_USE_LWIP_DEFINTIONS */


#define S_ADDR(a) (a).s_addr

typedef net_sockaddr_in_t sockaddr_in_t;
#if  NET_USE_IPV6
typedef net_sockaddr_in6_t sockaddr_in6_t;
#endif /* NET_USE_IPV6 */
typedef net_sockaddr_t sockaddr_t;

/** MAC address. */
typedef struct
{
  uint8_t mac[6];
} macaddr_t;



void            net_set_port(net_sockaddr_t *addr, uint16_t port);
uint16_t        net_get_port(net_sockaddr_t *addr);
net_ip_addr_t   net_get_ip_addr(net_sockaddr_t *addr);
typedef struct net_if_handle_s net_if_handle_t;
#endif /* NET_ADDRESS_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
