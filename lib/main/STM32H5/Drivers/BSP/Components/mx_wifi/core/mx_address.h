/**
  ******************************************************************************
  * @file    mx_address.h
  * @author  MCD Application Team
  * @brief   Header for the network address management functions
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MX_ADDRESS_H
#define MX_ADDRESS_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>

typedef char          mx_char_t;
typedef unsigned char mx_uchar_t;
typedef int           mx_int_t;
typedef uint32_t      mx_socklen_t;

#define MX_SERVICE_NAME_SIZE            (255)

/**
  * @brief timeval struct
  */
struct mx_timeval
{
  long tv_sec;     /* seconds      */
  long tv_usec;    /* microseconds */
};


/**
  * @brief socket address struct
  */
struct mx_ip_addr
{
  uint32_t addr;
};


struct mx_sockaddr
{
  uint8_t sa_len;
  uint8_t sa_family;
  uint8_t sa_data[14];
};


/**
  * @brief socket address (net format)
  */
struct mx_in_addr
{
  uint32_t s_addr;
};


/**
  * @brief socket address_in struct
  */
struct mx_sockaddr_in
{
  uint8_t           sin_len;
  uint8_t           sin_family;
  uint16_t          sin_port;
  struct mx_in_addr sin_addr;
  mx_char_t         sin_zero[8];
};


/**
  * @brief socket address (IPv6 net format)
  */
struct mx_in6_addr
{
  union _un
  {
    uint32_t u32_addr[4];
    uint8_t  u8_addr[16];
  } un; /* MISRAC2012-Rule-19.2 use of union */
  /* #define s6_addr  un.u8_addr */
};


/**
  * @brief socket address_in6 struct
  */
struct mx_sockaddr_in6
{
  uint8_t            sin6_len;      /* length of this structure    */
  uint8_t            sin6_family;   /* MX_AF_INET6                 */
  uint16_t           sin6_port;     /* Transport layer port #      */
  uint32_t           sin6_flowinfo; /* IPv6 flow information       */
  struct mx_in6_addr sin6_addr;     /* IPv6 address                */
  uint32_t           sin6_scope_id; /* Set of interfaces for scope */
};


/**
  * @brief socket address_in6 info
  */
#pragma pack(1)
struct mx_sockaddr_storage
{
  uint8_t   s2_len;
  uint8_t   ss_family;
  uint8_t   s2_data1[2];
  uint32_t  s2_data2[3];
  uint32_t  s2_data3[3];
};
#pragma pack()

#define MX_HOSTNAME_LEN_MAX         (255)
#pragma pack(1)
struct mx_addrinfo
{
  int32_t                    ai_flags;                               /* Input flags. */
  int32_t                    ai_family;                              /* Address family of socket. */
  int32_t                    ai_socktype;                            /* Socket type. */
  int32_t                    ai_protocol;                            /* Protocol of socket. */
  mx_socklen_t               ai_addrlen;                             /* Length of socket address. */
  struct mx_sockaddr_storage ai_addr;                                /* Socket address of socket. */
  mx_char_t                  ai_canonname[MX_SERVICE_NAME_SIZE + 1]; /* Canonical name of service location. */
  struct mx_addrinfo        *ai_next;                                /* Pointer to next in list, NULL for mx_cmd. */
};
#pragma pack()


/**
  * @brief Socket protocol types (TCP/UDP)
  */
#define MX_AF_UNSPEC       0
#define MX_AF_INET         2
#define MX_AF_INET6        10

#define MX_SOCK_STREAM     1
#define MX_SOCK_DGRAM      2

#define MX_IPPROTO_IP      0
#define MX_IPPROTO_TCP     6
#define MX_IPPROTO_UDP     17

/**
  * @brief socket options
  */
#define MX_SOL_SOCKET  0xfff    /* options for socket level */

typedef enum
{
  MX_SO_DEBUG              = 0x0001,     /**< Unimplemented: turn on debugging info recording */
  MX_SO_ACCEPTCONN         = 0x0002,     /**< socket has had listen() */
  MX_SO_REUSEADDR          = 0x0004,     /**< Allow local address reuse */
  MX_SO_KEEPALIVE          = 0x0008,     /**< keep connections alive */
  MX_SO_DONTROUTE          = 0x0010,     /**< Just use interface addresses */
  MX_SO_BROADCAST          = 0x0020,     /**< Permit to send and to receive broadcast messages */
  MX_SO_USELOOPBACK        = 0x0040,     /**< Bypass hardware when possible */
  MX_SO_LINGER             = 0x0080,     /**< linger on close if data present */
  MX_SO_OOBINLINE          = 0x0100,     /**< Leave received OOB data in line */
  MX_SO_REUSEPORT          = 0x0200,     /**< Allow local address & port reuse */
  MX_SO_BLOCKMODE          = 0x1000,     /**< set socket as block(optval=0)/non-block(optval=1) mode.
                                             Default is block mode. */
  MX_SO_SNDBUF             = 0x1001,
  MX_SO_SNDTIMEO           = 0x1005,     /**< Send timeout in block mode. block for ever in default mode. */
  MX_SO_RCVTIMEO           = 0x1006,     /**< Recv timeout in block mode. block 1 second in default mode. */
  MX_SO_ERROR              = 0x1007,     /**< Get socket error number. */
  MX_SO_TYPE               = 0x1008,     /**< Get socket type. */
  MX_SO_NO_CHECK           = 0x100A,     /**< Don't create UDP checksum. */
  MX_SO_BINDTODEVICE       = 0x100B      /* bind to device */
} MX_SOCK_OPT_VAL;


/**
  * @brief  IP option types, level: MX_IPPROTO_IP
  */
typedef enum
{
  MX_IP_ADD_MEMBERSHIP       = 0x0003,     /**< Join multicast group.  */
  MX_IP_DROP_MEMBERSHIP      = 0x0004,     /**< Leave multicast group. */
  MX_IP_MULTICAST_TTL        = 0x0005,
  MX_IP_MULTICAST_IF         = 0x0006,
  MX_IP_MULTICAST_LOOP       = 0x0007
} MX_IP_OPT_VAL;


/**
  * @brief FD SET for MXCHIP socket
  */
#define MX_FD_SETSIZE        64                                      /**< MAX fd number is 64 in MXOS. */
#define MX_HOWMANY(x, y)     (((x) + ((y) - 1)) / (y))
#define MX_NBBY              8                                       /**< number of bits in a byte. */
#define MX_NFDBITS          (sizeof(unsigned long) * MX_NBBY)        /**< bits per mask */
#define MX_FDSET_MASK(n)    ((unsigned long)1 << ((n) % MX_NFDBITS))


typedef struct _mx_fd_set
{
  unsigned long fds_bits[MX_HOWMANY(MX_FD_SETSIZE, MX_NFDBITS)];
} mx_fd_set;

#define MX_FD_SET(n, p)      ((p)->fds_bits[(n)/MX_NFDBITS] |= MX_FDSET_MASK(n))  /**< Add a fd to FD set. */
#define MX_FD_CLR(n, p)      ((p)->fds_bits[(n)/MX_NFDBITS] &= ~MX_FDSET_MASK(n)) /**< Remove fd from FD set. */
#define MX_FD_ISSET(n, p)    ((p)->fds_bits[(n)/MX_NFDBITS] & MX_FDSET_MASK(n))   /**< Check if the fd is set in FD set.*/
#define MX_FD_ZERO(P)        memset((P), 0, sizeof(*(P)))                         /**< Clear FD set. */

typedef struct mx_ip_addr        mx_ip_addr_t;
typedef struct mx_sockaddr       mx_sockaddr_t;
typedef struct mx_sockaddr_in    mx_sockaddr_in_t;
typedef struct mx_sockaddr_in6   mx_sockaddr_in6_t;


/* IPv6 address states. */
#define MX_IP6_ADDR_INVALID      0x00
#define MX_IP6_ADDR_TENTATIVE    0x08
#define MX_IP6_ADDR_TENTATIVE_1  0x09 /* 1 probe sent */
#define MX_IP6_ADDR_TENTATIVE_2  0x0a /* 2 probes sent */
#define MX_IP6_ADDR_TENTATIVE_3  0x0b /* 3 probes sent */
#define MX_IP6_ADDR_TENTATIVE_4  0x0c /* 4 probes sent */
#define MX_IP6_ADDR_TENTATIVE_5  0x0d /* 5 probes sent */
#define MX_IP6_ADDR_TENTATIVE_6  0x0e /* 6 probes sent */
#define MX_IP6_ADDR_TENTATIVE_7  0x0f /* 7 probes sent */
#define MX_IP6_ADDR_VALID        0x10
#define MX_IP6_ADDR_PREFERRED    0x30
#define MX_IP6_ADDR_DEPRECATED   0x50

#define MX_IP6_ADDR_ISINVALID(addr_state)    ((addr_state) == MX_IP6_ADDR_INVALID)
#define MX_IP6_ADDR_ISTENTATIVE(addr_state)  ((addr_state) & MX_IP6_ADDR_TENTATIVE)
#define MX_IP6_ADDR_ISVALID(addr_state)      ((addr_state) & MX_IP6_ADDR_VALID) /* Include valid, preferred, and deprecated.*/
#define MX_IP6_ADDR_ISPREFERRED(addr_state)  ((addr_state) == MX_IP6_ADDR_PREFERRED)
#define MX_IP6_ADDR_ISDEPRECATED(addr_state) ((addr_state) == MX_IP6_ADDR_DEPRECATED)


/**
  * @brief Convert IPv4 address from string to int32_t
  * @note  IPv4 ONLY
  *
  * @param cp IPv4 address string, like "192.168.1.10"
  * @return IPv4 address in 4 bytes
  */
int32_t mx_aton_r(const mx_char_t *cp);


/**
  * @brief Convert IPv4 address from structure to string
  * @note  IPv4 ONLY
  *
  * @param addr IP address structure
  * @return IPv4 address string, like "192.168.1.10"
  */
mx_char_t *mx_ntoa(const mx_ip_addr_t *addr);

#endif /* MX_ADDRESS_H */
