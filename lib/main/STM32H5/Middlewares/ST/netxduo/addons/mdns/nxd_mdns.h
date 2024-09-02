/**************************************************************************/
/*                                                                        */
/*       Copyright (c) Microsoft Corporation. All rights reserved.        */
/*                                                                        */
/*       This software is licensed under the Microsoft Software License   */
/*       Terms for Microsoft Azure RTOS. Full text of the license can be  */
/*       found in the LICENSE file at https://aka.ms/AzureRTOS_EULA       */
/*       and in the root directory of this software.                      */
/*                                                                        */
/**************************************************************************/


/**************************************************************************/
/**************************************************************************/
/**                                                                       */ 
/** NetX Component                                                        */
/**                                                                       */
/**   Multicast Domain Name System (mDNS)                                 */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

/**************************************************************************/
/*                                                                        */
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */
/*                                                                        */
/*    nxd_mdns.h                                          PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Multicast Domain Name System (mDNS)      */
/*    component, including all data types and external references.        */
/*    It is assumed that nx_api.h and nx_port.h have already been         */
/*    included.                                                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            prevented infinite loop in  */
/*                                            name compression,           */
/*                                            resulting in version 6.1.3  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the issue of timer,   */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/

#ifndef NXD_MDNS_H
#define NXD_MDNS_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */



#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"
#include "nx_udp.h"
#include "nx_ip.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif /* FEATURE_NX_IPV6  */


/* Define the Multicast DNS ID.  */
#define NX_MDNS_ID                              0x4d444e53UL

/* Define macros that get the data on unaligned address.  */
#define NX_MDNS_GET_USHORT_DATA(data)           (USHORT)((*(data) << 8) | (*(data + 1)))
#define NX_MDNS_GET_ULONG_DATA(data)            (ULONG)((*(data) << 24) | (*(data + 1) << 16) | (*(data + 2) << 8) | (*(data + 3)))

/* Disable the mDNS Server functionality.  By default mDNS server
   function is enabled.  To remove the mDNS server function from 
   the mDNS library, uncomment the following line.  */
/*
#define NX_MDNS_DISABLE_SERVER 
*/

/* Disable the mDNS Client functionality.  By default mDNS client
   function is enabled.  To remove the mDNS client function from 
   the mDNS library, uncomment the following line.  */
/*
#define NX_MDNS_DISABLE_CLIENT
*/

/* Enable the feature to check the address and port of mDNS message.
   By default is enabled. To remove it from the mDNS library,
   comment the following line.  */
#define NX_MDNS_ENABLE_ADDRESS_CHECK

/* Enable the Passive Observation Of Failures feature for Client.
   By default is enabled. To remove it from the mDNS library,
   comment the following line.
   Note: This will have no effect if NX_MDNS_DISABLE_CLIENT is defined.  */
#define NX_MDNS_ENABLE_CLIENT_POOF

/* Enable the feature for Server to generate the Negative Responses.
   By default is enabled. To remove it from the mDNS library,
   comment the following line.  
   Note: This will have no effect if NX_MDNS_DISABLE_SERVER is defined.  */
#define NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES


/* Enable mDNS IPv6 feature, send/process mDNS message over IPv6 address.
   By default mDNS IPv6 function is disabled. To enable this feature,
   uncomment the following line.  */
/*
#define NX_MDNS_ENABLE_IPV6
*/

#ifdef NX_MDNS_ENABLE_IPV6

#ifndef FEATURE_NX_IPV6
#error "mDNS IPv6 is not supported if IPv6 is not enabled."
#endif /* FEATURE_NX_IPV6  */

#ifndef NX_IPV6_MULTICAST_ENABLE
#error "mDNS IPv6 is not supported if IPv6 multicast is not enabled."
#endif /* NX_IPV6_MULTICAST_ENABLE  */

#ifndef NX_MDNS_DISABLE_SERVER
#ifndef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY 
#error "mDNS IPv6 is not supported if IPv6 address change notify is not enabled."
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY  */
#endif /* NX_MDNS_DISABLE_SERVER  */

#endif /* NX_MDNS_ENABLE_IPV6  */


/* Define the IPV6 address count of the host name. */
#ifndef NX_MDNS_IPV6_ADDRESS_COUNT
#define NX_MDNS_IPV6_ADDRESS_COUNT              2
#endif /* NX_MDNS_IPV6_ADDRESS_COUNT  */


/* Define message and name limits.  */
#define NX_MDNS_TYPE_MAX                        21          /* Maximum Type size, <sn>._tcp, <sn> may be up to 15 bytes,
                                                               plus the underscore and length byte. Including the "_udp" or "_tcp". */
#define NX_MDNS_LABEL_MAX                       63          /* Maximum Label (between two dots) size                 */
#define NX_MDNS_NAME_MAX                        255         /* Maximum Name size                                     */
#define NX_MDNS_IP_LOOKUP_SIZE                  75          /* IPv6 needs 66 characters for the address plus 8 for the ip6.arpa plus null           */
                                                            /* IPv4 needs 12 characters max for the address plus 12 for the IN-ADDR.ARPA plus null  */


/* Define the mDNS's host name cache size. Keep four-byte alignment.   */
/* Note: The real host name size plus conflict size (4)
         must not exceed NX_MDNS_LABEL_MAX and NX_MDNS_HOST_NAME_MAX.  
         such as:
         the real host name: "NETX-MDNS-HOST"
         the conflict name: " (2)" 
         the final name: "NETX-MDNS-HOST (2)".  */
#ifndef NX_MDNS_HOST_NAME_MAX
#define NX_MDNS_HOST_NAME_MAX                   64
#endif /* NX_MDNS_HOST_NAME_MAX  */


/* Define the mDNS's service name cache size. Keep four-byte alignment.  */ 
/* Note: The real service name size plus conflict size (4)
         must not exceed NX_MDNS_LABEL_MAX and NX_MDNS_SERVICE_NAME_MAX.
         such as:
         the real service name: "NETX-MDNS-SERVICE"
         the conflict name: " (2)" 
         the final service name: "NETX-MDNS-SERVICE (2)".  */
#ifndef NX_MDNS_SERVICE_NAME_MAX
#define NX_MDNS_SERVICE_NAME_MAX                64
#endif /* NX_MDNS_SERVICE_NAME_MAX  */


/* Define the mDNS's domain name max size. */
#ifndef NX_MDNS_DOMAIN_NAME_MAX
#define NX_MDNS_DOMAIN_NAME_MAX                 16
#endif /* NX_MDNS_DOMAIN_NAME_MAX  */

/* NX_MDNS_DOMAIN_NAME_MAX must be at least 5 bytes to hold "local". */
#if (NX_MDNS_DOMAIN_NAME_MAX < 5)
#error "NX_MDNS_DOMAIN_NAME_MAX must be at least 5 bytes!"
#endif /* (NX_MDNS_DOMAIN_NAME_MAX < 5) */

/* (NX_MDNS_HOST_NAME_MAX + "."(1) + NX_MDNS_DOMAIN_NAME_MAX) must be no more than NX_MDNS_NAME_MAX. */
#if ((NX_MDNS_HOST_NAME_MAX + 1 + NX_MDNS_DOMAIN_NAME_MAX) > NX_MDNS_NAME_MAX)
#error "(NX_MDNS_HOST_NAME_MAX + 1 + NX_MDNS_DOMAIN_NAME_MAX) must be no more than NX_MDNS_NAME_MAX!"
#endif

/* Define the conflict count of service name or host name.   */
/* Note: the confilict count should be less than 8, since we just append " (x)" 
         between " (2)" and " (9)" during the conflict resolution process.  */
#ifndef NX_MDNS_CONFLICT_COUNT
#define NX_MDNS_CONFLICT_COUNT                  8
#endif/* NX_MDNS_CONFLICT_COUNT  */


/* Define the DNS-SD buffer size for storing "_services._dns-sd._udp.local"  */ 
/* Note: the domain can be updated, so the max size should be NX_MDNS_DOMAIN_NAME_MAX + 23.
         "_services._dns-sd._udp." (23) + NX_MDNS_DOMAIN_NAME_MAX. */
#define NX_MDNS_DNS_SD_MAX                      (NX_MDNS_DOMAIN_NAME_MAX + 23)


/* Define mDNS Multicast Address.  */
#define NX_MDNS_IPV4_MULTICAST_ADDRESS          (IP_ADDRESS(224,0,0,251))


/* Define mDNS port.  */
#define NX_MDNS_UDP_PORT                        5353


/* Define UDP socket create options.  */
#ifndef NX_MDNS_UDP_TYPE_OF_SERVICE
#define NX_MDNS_UDP_TYPE_OF_SERVICE             NX_IP_NORMAL
#endif /* NX_MDNS_UDP_TYPE_OF_SERVICE  */

#ifndef NX_MDNS_UDP_FRAGMENT_OPTION
#define NX_MDNS_UDP_FRAGMENT_OPTION             NX_DONT_FRAGMENT
#endif /* NX_MDNS_UDP_FRAGMENT_OPTION  */

#ifndef NX_MDNS_UDP_TIME_TO_LIVE
#define NX_MDNS_UDP_TIME_TO_LIVE                0xFF
#endif /* NX_MDNS_UDP_TIME_TO_LIVE  */

#ifndef NX_MDNS_UDP_QUEUE_DEPTH
#define NX_MDNS_UDP_QUEUE_DEPTH                 4
#endif /* NX_MDNS_UDP_QUEUE_DEPTH  */


/* Define the resource record TTL. In second.  */
/* The recommended TTL value for Multicast DNS resource records with a host name as the resource record's name
   (e.g., A, AAAA, HINFO) or a host name contaned within the resource record's rdata (e.g., SRV, reverse mapping PTR record)
   should be 120 seconds. The recommended TTL value for other Multicast DNS resource records is 75 minutes.
   RFC6762, Section10, Page33.  */
#ifndef NX_MDNS_RR_TTL_HOST
#define NX_MDNS_RR_TTL_HOST                     120
#endif /* NX_MDNS_RR_TTL_HOST  */

#ifndef NX_MDNS_RR_TTL_OTHER
#define NX_MDNS_RR_TTL_OTHER                    4500        /* 75 minutes.  */
#endif /* NX_MDNS_RR_TTL_OTHER  */


/* Define the mDNS's probing timer interval, default 250ms in spec. In tick.
   should be: 250 * NX_IP_PERIODIC_RATE / 1000.  */
#ifndef NX_MDNS_PROBING_TIMER_COUNT
#define NX_MDNS_PROBING_TIMER_COUNT             (250 * NX_IP_PERIODIC_RATE / 1000)
#endif /* NX_MDNS_PROBING_TIMER_COUNT  */


/* Define the mDNS's announcing timer interval, default 250ms in spec. In tick.
   should be: 250 * NX_IP_PERIODIC_RATE / 1000.  */
#ifndef NX_MDNS_ANNOUNCING_TIMER_COUNT
#define NX_MDNS_ANNOUNCING_TIMER_COUNT          (250 * NX_IP_PERIODIC_RATE / 1000)
#endif /* NX_MDNS_ANNOUNCING_TIMER_COUNT  */


/* Define the mDNS's goodbye timer interval, default 250ms in spec. In tick.
   should be: 250 * NX_IP_PERIODIC_RATE / 1000.  */
#ifndef NX_MDNS_GOODBYE_TIMER_COUNT
#define NX_MDNS_GOODBYE_TIMER_COUNT             (250 * NX_IP_PERIODIC_RATE / 1000)
#endif /* NX_MDNS_GOODBYE_TIMER_COUNT  */


/* Define the mDNS's min query timer interval between two queries, 1 second. In tick.  */
#ifndef NX_MDNS_QUERY_MIN_TIMER_COUNT
#define NX_MDNS_QUERY_MIN_TIMER_COUNT           NX_IP_PERIODIC_RATE
#endif /* NX_MDNS_QUERY_MIN_TIMER_COUNT  */


/* Define the mDNS's max query timer interval between first two queries, 60 minutes. In tick.  */
#ifndef NX_MDNS_QUERY_MAX_TIMER_COUNT
#define NX_MDNS_QUERY_MAX_TIMER_COUNT           (3600 * NX_IP_PERIODIC_RATE)
#endif /* NX_MDNS_QUERY_MAX_TIMER_COUNT  */


/* Define the mDNS's query delay timer interval, 20-120ms. In tick. */
#ifndef NX_MDNS_QUERY_DELAY_MIN
#define NX_MDNS_QUERY_DELAY_MIN                 (20 * NX_IP_PERIODIC_RATE / 1000)
#endif /* NX_MDNS_QUERY_DELAY_MIN  */

#ifndef NX_MDNS_QUERY_DELAY_RANGE
#define NX_MDNS_QUERY_DELAY_RANGE               (100 * NX_IP_PERIODIC_RATE / 1000)
#endif /* NX_MDNS_QUERY_DELAY_RANGE  */


/* Define the mDNS's response delay time interval for multicast query.  */
/* The time interval, in ticks, in responding to a query to ensure an interval of at least 1s 
   since the last time the record was multicast. The default value is NX_IP_PERIODIC_RATE ticks.  */
#ifndef NX_MDNS_RESPONSE_INTERVAL
#define NX_MDNS_RESPONSE_INTERVAL               NX_IP_PERIODIC_RATE
#endif /* NX_MDNS_RESPONSE_INTERVAL  */


/* Define the mDNS's response delay time interval for probe message.  */
/* The time interval, in ticks, in responding to a probe queries to ensure an interval of 
   at least 250ms since the last time the record was multicast. The default value is (250 * NX_IP_PERIODIC_RATE / 1000) ticks.  */
#ifndef NX_MDNS_RESPONSE_PROBING_TIMER_COUNT
#define NX_MDNS_RESPONSE_PROBING_TIMER_COUNT    (250 * NX_IP_PERIODIC_RATE / 1000)
#endif /* NX_MDNS_RESPONSE_PROBING_TIMER_COUNT  */


/* Define the mDNS's response delay time interval for unicast query, 10 ms. In tick.  */
#ifndef NX_MDNS_RESPONSE_UNIQUE_DELAY
#define NX_MDNS_RESPONSE_UNIQUE_DELAY           (10 * NX_IP_PERIODIC_RATE / 1000)
#endif /* NX_MDNS_RESPONSE_UNIQUE_DELAY  */


/* Define the mDNS's response delay time interval for shared message, 20-120ms. In tick.  */
#ifndef NX_MDNS_RESPONSE_SHARED_DELAY_MIN
#define NX_MDNS_RESPONSE_SHARED_DELAY_MIN       (20 * NX_IP_PERIODIC_RATE / 1000)
#endif /* NX_MDNS_RESPONSE_SHARED_DELAY_MIN */

#ifndef NX_MDNS_RESPONSE_SHARED_DELAY_RANGE
#define NX_MDNS_RESPONSE_SHARED_DELAY_RANGE     (100 * NX_IP_PERIODIC_RATE / 1000)
#endif /* NX_MDNS_RESPONSE_SHARED_DELAY_RANGE  */


/* Define the mDNS's response delay time interval for query with TC, 400-500 ms. In tick.  */
#ifndef NX_MDNS_RESPONSE_TC_DELAY_MIN
#define NX_MDNS_RESPONSE_TC_DELAY_MIN           (400 * NX_IP_PERIODIC_RATE / 1000)
#endif /* NX_MDNS_RESPONSE_TC_DELAY_MIN  */

#ifndef NX_MDNS_RESPONSE_TC_DELAY_RANGE
#define NX_MDNS_RESPONSE_TC_DELAY_RANGE         (100 * NX_IP_PERIODIC_RATE / 1000)
#endif /* NX_MDNS_RESPONSE_TC_DELAY_RANGE  */


/* Define the mDNS's random delay timer count, 120 ms. In tick.  */
/* This value allows a response to include messages that would be sent within
   the next 120ms range.  */
#ifndef NX_MDNS_TIMER_COUNT_RANGE
#define NX_MDNS_TIMER_COUNT_RANGE               (120 * NX_IP_PERIODIC_RATE / 1000)
#endif /* NX_MDNS_TIMER_COUNT_RANGE  */


/* Define the mDNS's probing retransmit count. The defaul value is 3.  */
#ifndef NX_MDNS_PROBING_RETRANSMIT_COUNT
#define NX_MDNS_PROBING_RETRANSMIT_COUNT        3
#endif /* NX_MDNS_PROBING_RETRANSMIT_COUNT  */


/* Define the mDNS's goodbye retransmit count. The defaul value is 1.  */
#ifndef NX_MDNS_GOODBYE_RETRANSMIT_COUNT
#define NX_MDNS_GOODBYE_RETRANSMIT_COUNT        1
#endif /* NX_MDNS_GOODBYE_RETRANSMIT_COUNT  */


/* Passive Observation Of Failures.
   If a host sees queries, for which a record in its cache would be
   expected to be given as an answer in a multicast response, but no
   such answer is seen, then the host may take this as an indication
   that the record may no longer be valid.  
   After seesing NX_MDNS_POOF_MIN_COUNT of these queries, and seeing
   no multicast response containing the expected answer with in
   NX_MDNS_POOF_TIMER_COUNT, delete the RR from the cache.  
   RFC6762, Section10.5, Page38.  */

/* Define the Passive Observation Of Failures min count. The defaul value is 2.  */
#ifndef NX_MDNS_POOF_MIN_COUNT
#define NX_MDNS_POOF_MIN_COUNT                  2
#endif /* NX_MDNS_POOF_MIN_COUNT  */

/* Define the Passive Observation Of Failures timer count, 10 seconds in spec. In tick. */
#ifndef NX_MDNS_POOF_TIMER_COUNT
#define NX_MDNS_POOF_TIMER_COUNT                (10 * NX_IP_PERIODIC_RATE)
#endif /* NX_MDNS_POOF_TIMER_COUNT  */


/* Define the RR delete delay timer.  */
/* Queriers receiving a Multicast DNS response with a TTL of zero SHOULD
   NOT immediately delete the record from the cache, but instead record
   a TTL of 1 and then delete the record one second later. In tick.  */
#ifndef NX_MDNS_RR_DELETE_DELAY_TIMER_COUNT
#define NX_MDNS_RR_DELETE_DELAY_TIMER_COUNT     NX_IP_PERIODIC_RATE
#endif /* NX_MDNS_RR_DELETE_DELAY_TIMER_COUNT  */

/* Define the maximum number of pointers allowed in name compression.  */
#ifndef NX_MDNS_MAX_COMPRESSION_POINTERS
#define NX_MDNS_MAX_COMPRESSION_POINTERS        16
#endif /* NX_MDNS_MAX_COMPRESSION_POINTERS  */


/* Define the default mDNS's announcing value.  */
#define NX_MDNS_ANNOUNCING_PERIOD               NX_IP_PERIODIC_RATE
#define NX_MDNS_ANNOUNCING_COUNT                1
#define NX_MDNS_ANNOUNCING_FACTOR               1
#define NX_MDNS_ANNOUNCING_RETRANS_INTERVAL     0
#define NX_MDNS_ANNOUNCING_PERIOD_INTERVAL      0xFFFFFFFF
#define NX_MDNS_ANNOUNCING_MAX_TIME             3
#define NX_MDNS_ANNOUNCING_FOREVER              0xFF


/* Define the max ttl. In second.  */
#define NX_MDNS_RR_MAX_TTL                      0xFFFFFFFF/NX_IP_PERIODIC_RATE


/* Define the RR update retransmit count.
   The querier should plan to issure a query at 80%, 85%, 90%, 95% of the record lifetime.  */
#define NX_MDNS_RR_UPDATE_COUNT                 4


/* Define the Address length.  */
#define NX_MDNS_IPV4_ADDRESS_LENGTH             4
#define NX_MDNS_IPV6_ADDRESS_LENGTH             16


/* Define mDNS event flags.  */
#define NX_MDNS_ALL_EVENTS                      ((ULONG) 0xFFFFFFFF)    /* All event flags.                         */
#define NX_MDNS_PKT_RX_EVENT                    ((ULONG) 0x00000001)    /* Receive the mDNS packet.                 */
#define NX_MDNS_QUERY_SEND_EVENT                ((ULONG) 0x00000002)    /* Send the mDNS Query packet.              */
#define NX_MDNS_RESPONSE_SEND_EVENT             ((ULONG) 0x00000004)    /* Send the mDNS Response packet.           */
#define NX_MDNS_PROBING_SEND_EVENT              ((ULONG) 0x00000008)    /* Send the mDNS Probing packet.            */
#define NX_MDNS_ANNOUNCING_SEND_EVENT           ((ULONG) 0x00000010)    /* Send the mDNS Announcing packet.         */
#define NX_MDNS_GOODBYE_SEND_EVENT              ((ULONG) 0x00000020)    /* Send the mDNS Goodbye packet.            */
#define NX_MDNS_TIMER_EVENT                     ((ULONG) 0x00000040)    /* Process the mDNS timer event.            */
#define NX_MDNS_ADDRESS_CHANGE_EVENT            ((ULONG) 0x00000080)    /* Process the mDNS address change event.   */
#define NX_MDNS_RR_ELAPSED_TIMER_EVENT          ((ULONG) 0x00000100)    /* Process the mDNS rr elapsed tiemr event. */


/* Define return code constants.  */
#define NX_MDNS_SUCCESS                         0x00        /* mDNS Success.                                        */ 
#define NX_MDNS_ERROR                           0xA1        /* mDNS internal error.                                 */ 
#define NX_MDNS_PARAM_ERROR                     0xA2        /* mDNS parameters error.                               */ 
#define NX_MDNS_CACHE_ERROR                     0xA3        /* The Cache size is not enough.                        */ 
#define NX_MDNS_UNSUPPORTED_TYPE                0xA4        /* The unsupported resource record type.                */ 
#define NX_MDNS_DATA_SIZE_ERROR                 0xA5        /* The data size is too big.                            */
#define NX_MDNS_AUTH_ERROR                      0xA6        /* Attempting to parse too large a data.                */
#define NX_MDNS_PACKET_ERROR                    0xA7        /* The packet can not add the resource record.          */
#define NX_MDNS_DEST_ADDRESS_ERROR              0xA9        /* The destination address error.                       */
#define NX_MDNS_UDP_PORT_ERROR                  0xB0        /* The udp port error.                                  */
#define NX_MDNS_NOT_LOCAL_LINK                  0xB1        /* The message that not originate from the local link.  */
#define NX_MDNS_EXCEED_MAX_LABEL                0xB2        /* The data exceed the max laber size.                  */
#define NX_MDNS_EXIST_UNIQUE_RR                 0xB3        /* At least one Unqiue record in the cache.             */
#define NX_MDNS_EXIST_SHARED_RR                 0xB4        /* At least one shared record in the cache.             */
#define NX_MDNS_EXIST_SAME_QUERY                0xB5        /* Exist the same query record in the cache.            */
#define NX_MDNS_EXIST_SAME_SERVICE              0xB6        /* Exist the same service.                              */
#define NX_MDNS_NO_RR                           0xB7        /* No response for one-shot query.                      */
#define NX_MDNS_NO_KNOWN_ANSWER                 0xB8        /* No known answer for query.                           */
#define NX_MDNS_NAME_MISMATCH                   0xB9        /* The name mismatch.                                   */
#define NX_MDNS_NOT_STARTED                     0xC0        /* mDNS does not start.                                 */
#define NX_MDNS_HOST_NAME_ERROR                 0xC1        /* mDNS host name error.                                */
#define NX_MDNS_NO_MORE_ENTRIES                 0xC2        /* No more entries be found.                            */
#define NX_MDNS_SERVICE_TYPE_MISMATCH           0xC3        /* The service type mismatch                            */
#define NX_MDNS_NOT_ENABLED                     0xC4        /* mDNS does not enable.                                */
#define NX_MDNS_ALREADY_ENABLED                 0xC5        /* mDNS already enabled.                                */


/* Define the local host/service register notify state.  */
#define NX_MDNS_LOCAL_HOST_REGISTERED_SUCCESS   1           /* mDNS Server: Local host registered success.          */
#define NX_MDNS_LOCAL_HOST_REGISTERED_FAILURE   2           /* mDNS Server: Local host registered failure.          */
#define NX_MDNS_LOCAL_SERVICE_REGISTERED_SUCCESS 3          /* mDNS Server: Local service registered success.       */
#define NX_MDNS_LOCAL_SERVICE_REGISTERED_FAILURE 4          /* mDNS Server: Local service registered failure.       */

/* Define the peer service change notify state.  */
#define NX_MDNS_PEER_SERVICE_RECEIVED           1           /* mDNS Client: Peer service received.                  */
#define NX_MDNS_PEER_SERVICE_DELETED            2           /* mDNS Client: Peer service deleted.                   */
#define NX_MDNS_PEER_SERVICE_UPDATED            3           /* mDNS Client: Peer service address updated.           */

/* Define the cache notify state.  */
#define NX_MDNS_CACHE_STATE_FULL                1           /* mDNS Cache is full, and cannot fill the RR.          */
#define NX_MDNS_CACHE_STATE_FRAGMENTED          2           /* mDNS Cache is fragmented, and cannot fill the RR.    */

/* Define cache type.  */
#define NX_MDNS_CACHE_TYPE_LOCAL                0           /* Local cache.                                         */
#define NX_MDNS_CACHE_TYPE_PEER                 1           /* Peer cache.                                          */


/* Define offsets into the DNS message buffer.  */
#define NX_MDNS_ID_OFFSET                       0           /* Offset to ID code in DNS buffer                      */
#define NX_MDNS_FLAGS_OFFSET                    2           /* Offset to flags in DNS buffer                        */
#define NX_MDNS_QDCOUNT_OFFSET                  4           /* Offset to Question Count in DNS buffer               */
#define NX_MDNS_ANCOUNT_OFFSET                  6           /* Offset to Answer Count in DNS buffer                 */
#define NX_MDNS_NSCOUNT_OFFSET                  8           /* Offset to Authority Count in DNS buffer              */
#define NX_MDNS_ARCOUNT_OFFSET                  10          /* Offset to Additional Info. Count in DNS buffer       */
#define NX_MDNS_QDSECT_OFFSET                   12          /* Offset to Question Section in DNS buffer             */


/* Define constants for the flags word.  */
#define NX_MDNS_QUERY_MASK                      0x0000
#define NX_MDNS_QUERY_FLAG                      0x0000
#define NX_MDNS_RESPONSE_FLAG                   0x8000
#define NX_MDNS_ERROR_MASK                      0x000F
#define NX_MDNS_TOP_BIT_MASK                    0x7FFF

#define NX_MDNS_OPCODE_QUERY                    (0 << 12)   /* Shifted right 12 is still 0                          */
#define NX_MDNS_OPCODE_IQUERY                   (1 << 12)   /* 1 shifted right by 12                                */
#define NX_MDNS_OPCODE_STATUS                   (2 << 12)   /* 2 shifted right by 12                                */

#define NX_MDNS_AA_FLAG                         0x0400      /* Authoritative Answer                                 */
#define NX_MDNS_TC_FLAG                         0x0200      /* Truncated                                            */
#define NX_MDNS_RD_FLAG                         0x0100      /* Recursive Query                                      */
#define NX_MDNS_RA_FLAG                         0x0080      /* Recursion Available                                  */
#define NX_MDNS_FA_FLAG                         0x0010      /* Force Authentication                                 */

/* Define name compression masks.  */
#define NX_MDNS_COMPRESS_MASK                   0xc0
#define NX_MDNS_COMPRESS_VALUE                  0xc0
#define NX_MDNS_POINTER_MASK                    0xc000

/* Define resource record types.  */
#define NX_MDNS_RR_TYPE_A                       1           /* Host address                                         */
#define NX_MDNS_RR_TYPE_NS                      2           /* Authoritative name server                            */
#define NX_MDNS_RR_TYPE_MD                      3           /* Mail destination (Obsolete - use MX)                 */
#define NX_MDNS_RR_TYPE_MF                      4           /* Mail forwarder (Obsolete - use MX)                   */
#define NX_MDNS_RR_TYPE_CNAME                   5           /* Canonical name for an alias                          */
#define NX_MDNS_RR_TYPE_SOA                     6           /* Marks the start of a zone of authority               */
#define NX_MDNS_RR_TYPE_MB                      7           /* Mailbox domain name (EXPERIMENTAL)                   */
#define NX_MDNS_RR_TYPE_MG                      8           /* Mail group member (EXPERIMENTAL)                     */
#define NX_MDNS_RR_TYPE_MR                      9           /* Mail rename domain name (EXPERIMENTAL)               */
#define NX_MDNS_RR_TYPE_NULL                    10          /* Null RR (EXPERIMENTAL)                               */
#define NX_MDNS_RR_TYPE_WKS                     11          /* Well known service description                       */
#define NX_MDNS_RR_TYPE_PTR                     12          /* Domain name pointer                                  */
#define NX_MDNS_RR_TYPE_HINFO                   13          /* Host information                                     */
#define NX_MDNS_RR_TYPE_MINFO                   14          /* Mailbox or mail list information                     */
#define NX_MDNS_RR_TYPE_MX                      15          /* Mail exchange                                        */
#define NX_MDNS_RR_TYPE_TXT                     16          /* Text strings                                         */
#define NX_MDNS_RR_TYPE_AAAA                    28          /* IPv6 Host address                                    */
#define NX_MDNS_RR_TYPE_SRV                     33          /* The location of services                             */
#define NX_MDNS_RR_TYPE_NSEC                    47          /* The type of NSEC RR.                                 */


/* Define constants for Qtypes (queries).  */
#define NX_MDNS_RR_TYPE_AXFR                    252         /* Request for a transfer of an entire zone             */
#define NX_MDNS_RR_TYPE_MAILB                   253         /* Request for mailbox-related records (MB, MG or MR)   */
#define NX_MDNS_RR_TYPE_MAILA                   254         /* Request for mail agent RRs (Obsolete - see MX)       */
#define NX_MDNS_RR_TYPE_ALL                     255         /* Request for all records                              */


/* Define resource record classes.  */
#define NX_MDNS_RR_CLASS_IN                     1           /* Internet                                             */
#define NX_MDNS_RR_CLASS_CS                     2           /* CSNET class (Obsolete)                               */
#define NX_MDNS_RR_CLASS_CH                     3           /* CHAOS class                                          */
#define NX_MDNS_RR_CLASS_HS                     4           /* Hesiod [Dyer 87]                                     */
#define NX_MDNS_RR_CLASS_ALL                    255         /* Any class                                            */
#define NX_MDNS_RR_CLASS_TOP_BIT                0x8000      /* The top bit of class.                                */


/* Define the resource record state value.  */
#define NX_MDNS_RR_STATE_INVALID                0           /* Invalid resource records.                            */
#define NX_MDNS_RR_STATE_PROBING                1           /* mDNS Server probing the resource records.            */
#define NX_MDNS_RR_STATE_ANNOUNCING             2           /* mDNS Server announcing the resource records.         */
#define NX_MDNS_RR_STATE_GOODBYE                3           /* mDNS Server send the Goodbye.                        */
#define NX_MDNS_RR_STATE_SUSPEND                4           /* mDNS Server suspend the resource record.             */
#define NX_MDNS_RR_STATE_QUERY                  5           /* mDNS Client send the initial query.                  */
#define NX_MDNS_RR_STATE_UPDATING               6           /* mDNS Client send the query to update the RR.         */
#define NX_MDNS_RR_STATE_DELETE                 7           /* mDNS Client delete the resource records.             */
#define NX_MDNS_RR_STATE_POOF_DELETE            8           /* mDNS Client delete the resource records by Passive Observation Of Failures.*/
#define NX_MDNS_RR_STATE_VALID                  9           /* Valid resource records.                              */


/* Define the sending type of resource records.  */
#define NX_MDNS_RR_SEND_FLAG_CLEAR              0x00        /* Response should not send the Resource records.       */
#define NX_MDNS_RR_SEND_MULTICAST               0x01        /* Send the Resource records via multicast.             */
#define NX_MDNS_RR_SEND_UNICAST                 0x02        /* Send the Resource records via unicast.               */
#define NX_MDNS_RR_SEND_FLAG_MASK               0xF0        /* The mask of send flag.                               */


/* Define resource record owner.  */
#define NX_MDNS_RR_OWNER_LOCAL                  0           /* Local Resource records.                              */
#define NX_MDNS_RR_OWNER_REMOTE                 1           /* Remote Resource records.                             */


/* Define resource record set.  */
#define NX_MDNS_RR_SET_SHARED                   0           /* Shared Resource records.                             */
#define NX_MDNS_RR_SET_UNIQUE                   1           /* Unique Resource records.                             */


/* Define the resource record state mask.  */
#define NX_MDNS_RR_FLAG_PEER                    0x0001      /* RR owner flag.                                       */
#define NX_MDNS_RR_FLAG_UNIQUE                  0x0002      /* RR set flag.                                         */
#define NX_MDNS_RR_FLAG_CONTINUOUS_QUERY        0x0004      /* RR query type flag.                                  */
#define NX_MDNS_RR_FLAG_DUPLICATE_QUERY         0x0008      /* RR duplicate query flag.                             */
#define NX_MDNS_RR_FLAG_ADDITIONAL              0x0010      /* RR additional flag.                                  */
#define NX_MDNS_RR_FLAG_UPDATING                0x0020      /* RR updating flag.                                    */
#define NX_MDNS_RR_FLAG_DELETE                  0x0040      /* RR delete flag.                                      */
#define NX_MDNS_RR_FLAG_ANSWER                  0x0080      /* RR answer flag.                                      */
#define NX_MDNS_RR_FLAG_KNOWN_ANSWER            0x0100      /* RR known answer flag.                                */
#define NX_MDNS_RR_FLAG_AUTHORIY_ANSWER         0x0200      /* RR authority answer flag.                            */


/* Define the type to search the same resource record.  */
#define NX_MDNS_RR_MATCH_ALL                    1           /* Match all data: name, type, class, rdata.            */
#define NX_MDNS_RR_MATCH_EXCEPT_RDATA           2           /* Match the name, type, class, except rdata.           */


/* Define the type to set/add the resource record in cache from packet.  */
#define NX_MDNS_RR_OP_LOCAL_SET_QUESTION        0           /* Set resource record question in local cache.         */
#define NX_MDNS_RR_OP_LOCAL_SET_ANSWER          1           /* Set resource record answer in local cache.           */
#define NX_MDNS_RR_OP_PEER_SET_QUESTION         2           /* Set resource record question in peer cache.          */
#define NX_MDNS_RR_OP_PEER_ADD_ANSWER           3           /* Add resource record answer in peer cache.            */


/* Define the type for adding question/answer RR into the packet.  */
#define NX_MDNS_PACKET_ADD_RR_QUESTION          1           /* Add the question resource record into packet.        */
#define NX_MDNS_PACKET_ADD_RR_ANSWER            2           /* Add the answer resource record into packet.          */


/* Define mDNS packet type.  */
#define NX_MDNS_PACKET_QUERY                    1           /* Query packet.                                        */
#define NX_MDNS_PACKET_PROBING                  2           /* Probing packet. Is specific query.                   */
#define NX_MDNS_PACKET_RESPONSE                 3           /* Response packet.                                     */


/* Define the Add NSEC type.  */
#define NX_MDNS_ADD_NSEC_FOR_HOST               0           /* Add the NSEC for host name.                          */   
#define NX_MDNS_ADD_NSEC_FOR_SERVICE            1           /* Add the NSEC for service.                            */


/* Define the RDATA structure.  */

/* A RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                     ADDRESS                   |
    |                                               |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_A_STRUCT
{
    ULONG           nx_mdns_rr_a_address;
} NX_MDNS_RR_A;


/* NS RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     NSDNAME                   /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_NS_STRUCT
{
    UCHAR*          nx_mdns_rr_ns_name;
} NX_MDNS_RR_NS;

/* MD RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     MADNAME                   /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_MD_STRUCT
{
    UCHAR*          nx_mdns_rr_md_name;
} NX_MDNS_RR_MD;

/* MF RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     MADNAME                   /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_MF_STRUCT
{
    UCHAR*          nx_mdns_rr_mf_name;
} NX_MDNS_RR_MF;

/* CNMAE RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     CNAME                     /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_CNAME_STRUCT
{
    UCHAR*          nx_mdns_rr_cname_name;
} NX_MDNS_RR_CNAME;

/* SOA RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     MNAME                     /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     RNAME                     /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                     SERIAL                    |
    |                                               |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                     REFRESH                   |
    |                                               |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                     RETRY                     |
    |                                               |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                     EXPIRE                    |
    |                                               |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                     MINMUM                    |
    |                                               |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_SOA_STRUCT
{
    UCHAR*          nx_mdns_rr_soa_mname;
    UCHAR*          nx_mdns_rr_soa_rname;
    ULONG           nx_mdns_rr_soa_serial;
    ULONG           nx_mdns_rr_soa_refresh;
    ULONG           nx_mdns_rr_soa_retry;
    ULONG           nx_mdns_rr_soa_expire;
    ULONG           nx_mdns_rr_soa_minmum;
} NX_MDNS_RR_SOA;

/* MB RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     MADNAME                   /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_MB_STRUCT
{
    UCHAR*          nx_mdns_rr_mb_name;
} NX_MDNS_RR_MB;

/* MG RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     MGMNAME                   /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_MG_STRUCT
{
    UCHAR*          nx_mdns_rr_mg_name;
} NX_MDNS_RR_MG;

/* MR RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     NEWNAME                   /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_MR_STRUCT
{
    UCHAR*          nx_mdns_rr_mr_name;
} NX_MDNS_RR_MR;

/* NULL RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                   <anything>                  /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_NULL_STRUCT
{
    UCHAR*          nx_mdns_rr_null_ptr;
} NX_MDNS_RR_NULL;

/* WKS RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                     ADDRESS                   |
    |                                               |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |      PROTOCOL         |                       |
    |--+--+--+--+--+--+--+--+                       |
    /                   <BIT MAP>                   /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_WKS_STRUCT
{
    ULONG           nx_mdns_rr_wks_address;
    UCHAR           nx_mdns_rr_wks_protocol;
    UCHAR           nx_mdns_rr_wks_reserved[3];
    UCHAR*          nx_mdns_rr_wks_bit_map;
} NX_MDNS_RR_WKS;

/* PTR RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     PTRDNAME                  /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_PTR_STRUCT
{
    UCHAR*          nx_mdns_rr_ptr_name;
} NX_MDNS_RR_PTR;

/* HINFO RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     CPU                       /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     OS                        /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_HINFO_STRUCT
{
    UCHAR*          nx_mdns_rr_hinfo_cpu;
    UCHAR*          nx_mdns_rr_hinfo_os;
} NX_MDNS_RR_HINFO;

/* MINFO RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     RMAILBX                   /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     EMAILBX                   /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_MINFO_STRUCT
{
    UCHAR*          nx_mdns_rr_minfo_cpu;
    UCHAR*          nx_mdns_rr_minfo_os;
} NX_MDNS_RR_MINFO;

/* MX RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                     PREFERENCE                |                                         
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     NEWNAME                   /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_MX_STRUCT
{
    UCHAR*          nx_mdns_rr_mx_name;
    USHORT          nx_mdns_rr_mx_preference;
    USHORT          nx_mdns_rr_mx_reserved;
} NX_MDNS_RR_MX;

/* TXT RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     TXT-DATA                  /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_TXT_STRUCT
{
    UCHAR*          nx_mdns_rr_txt_data;
} NX_MDNS_RR_TXT;

/* AAAA RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                                               |
    |                                               |
    |                                               |
    |                   IPv6 ADDRESS                |
    |                                               |
    |                                               |
    |                                               |
    |                                               |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_AAAA_STRUCT
{
    ULONG           nx_mdns_rr_aaaa_address[4];
} NX_MDNS_RR_AAAA;

/* SRV RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                     PRIORITY                  |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                     WEIGHTS                   |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                     PORT                      |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     TARGET                    /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_SRV_STRUCT
{
    UCHAR*          nx_mdns_rr_srv_target;
    USHORT          nx_mdns_rr_srv_priority;
    USHORT          nx_mdns_rr_srv_weights;
    USHORT          nx_mdns_rr_srv_port;
    USHORT          nx_mdns_rr_srv_reserved;
} NX_MDNS_RR_SRV;


/* NSEC RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                Next Domain Name               /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                Type Bit Maps                  /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_MDNS_RR_NSEC_STRUCT
{
    UCHAR*          nx_mdns_rr_nsec_next_domain;
    UCHAR           nx_mdns_rr_nsec_window_block;
    UCHAR           nx_mdns_rr_nsec_bitmap_length;
    UCHAR           nx_mdns_rr_nsec_bitmap[5];          /* Can match the max rr type 6 * 7 = 42.  */
    UCHAR           nx_mdns_rr_nsec_additional_send;    /* 0 indicate answer, 1 indicate additional.  */
} NX_MDNS_RR_NSEC;


/* RR definitions. RFC1035.  */
/* RR Format.  
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                                               |
    /                      NAME                     /
    /                                               /
    |                                               |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                      TYPE                     |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                      CLASS                    |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                      TTL                      |
    |                                               |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                      RDLENGTH                 |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                      RDATA                    /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

/* The following data structure is used to store the resource records(RRs). */
typedef struct NX_MDNS_RR_STRUCT
{   

    UCHAR*  nx_mdns_rr_name;                    /* Owner's name,i.e., the name of the node to
                                                   which this resource record pertains.                                 */

    USHORT  nx_mdns_rr_type;                    /* RR TYPE.                                                             */

    USHORT  nx_mdns_rr_class;                   /* RR CLASS.                                                            */

    ULONG   nx_mdns_rr_ttl;                     /* The time interval that the RR may be cached before the source of 
                                                   the information should again be consulted.                           */

    USHORT  nx_mdns_rr_rdata_length;            /* The length of rdata. nx_mdns_rr_rdata                                */

    UCHAR   nx_mdns_rr_state;                   /* The resource records state, Probing, Annoncing, Valid.               */

    UCHAR   nx_mdns_rr_retransmit_count;        /* Define the count for the retransmit the RR.                          */

    ULONG   nx_mdns_rr_retransmit_lifetime;     /* Define the lifetime for retransmit the RR.                           */

    ULONG   nx_mdns_rr_timer_count;             /* Define the timer count for the retransmit the RR.                    */

    ULONG   nx_mdns_rr_elapsed_time;            /* Define the time elasped for the peer RR.                             */

    ULONG   nx_mdns_rr_remaining_ticks;         /* Define the remaining ticks of the peer RR.                           */

    ULONG   nx_mdns_rr_response_interval;       /* Define the time interval for two responses. Local RR.                */

    UCHAR   nx_mdns_rr_interface_index;         /* RR interface index.                                                  */

    UCHAR   nx_mdns_rr_announcing_max_time;     /* RR announcing max time. Local RR.                                    */

    UCHAR   nx_mdns_rr_conflict_count;          /* The conflict count of service name or host name. Local RR.           */

    UCHAR   nx_mdns_rr_poof_count;              /* The Passive Observation Of Failures count of resource record. Peer RR.*/

    UCHAR   nx_mdns_rr_count;                   /* The count of same _services._dns-sd._udp.local PTR resource reocrd.  */
 
    UCHAR   nx_mdns_rr_send_flag;               /* The flag for seding resource record.                                 */

    USHORT  nx_mdns_rr_word;                    

    /* Define the state, owner, flag, set of the resource records. This word contains
       the following information:

            bits 0      RR Owner, the owner of resource record, 
                        0 indicates local RR, 1 indicates peer RR.
            bits 1      RR Set, the set of resource record,
                        0 indicates shared RR, 1 indicates unique RR.
            bits 2      RR query type, the query type of resource record, 
                        0 indicates query the RR use One-shot method, 1 indicates query the RR use Continuous method.
            bits 3      RR additional flag, send this resource record as the additional RR.
            bits 4      RR query duplicate flag, this resource record is the duplicate quesiton.
            bits 5      RR updating flag, this resource record need to be maintenance.     
            bits 6      RR delete flag, this resource record need to be deleted.
            bits 7      RR answer flag, this resource record has been added in answer section.
            bits 8      RR known answer flag, send this resource record as the known answer.    
            bits 9      RR authority answer flag, send this resource record as the authority answer.  
    */
    
    /* Union that holds resource record data. */
    union   nx_mdns_rr_rdata_union
    {
        NX_MDNS_RR_A        nx_mdns_rr_rdata_a;
        NX_MDNS_RR_AAAA     nx_mdns_rr_rdata_aaaa;
        NX_MDNS_RR_PTR      nx_mdns_rr_rdata_ptr;
        NX_MDNS_RR_SRV      nx_mdns_rr_rdata_srv;
        NX_MDNS_RR_TXT      nx_mdns_rr_rdata_txt;

#ifdef NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES
        NX_MDNS_RR_NSEC     nx_mdns_rr_rdata_nsec;
#endif /* NX_MDNS_ENABLE_SERVER_NEGATIVE_RESPONSES  */

#ifdef NX_MDNS_ENABLE_EXTENDED_RR_TYPES
        NX_MDNS_RR_CNAME    nx_mdns_rr_rdata_cname;
        NX_MDNS_RR_NS       nx_mdns_rr_rdata_ns;
        NX_MDNS_RR_MX       nx_mdns_rr_rdata_mx;
#endif /* NX_MDNS_ENABLE_EXTENDED_RR_TYPES  */
    } nx_mdns_rr_rdata;

}NX_MDNS_RR;

/* Buffer usage. */
/*
 
    |-----------+-----------+-----------+-----------+-----------+------------+-----------+------------+------------|
    |   HEAD    |NX_MDNS_RR |NX_MDNS_RR | ..........|  STRING   | CNT | LEN  |  STRING   | CNT | LEN  |    TAIL    |
    |-----------+-----------+-----------+-----------+-----------+------------+-----------+------------+------------|
    
    HEAD: First unused memory at the front of cache. The size is 4 bytes.
    NX_MDNS_RR: The Resource Record struct, the size is sizeof(NX_MDNS_RR).
    TAIL: It points to the last used memory at the back of cache. Its size is 4 bytes.
    LEN: It is the length of STRING, plus the CNT and LEN fields. This field takes 2 bytes.
    CNT: It is the reference count of STRING. It takes 2 bytes.
    STRING: The content of name string and rdata string, must be null-terminated. Padded with 0s to be 4 byte aligned.
*/


/* Define the Service structure.  */
typedef struct NX_MDNS_SERVICE_STRUCT
{

    UCHAR*                  service_name;           /* Service instance.                                    */

    UCHAR*                  service_type;           /* Service type.                                        */

    UCHAR*                  service_domain;         /* Service domain.                                      */

    UCHAR                   service_text[NX_MDNS_NAME_MAX+1];       /* Service txt information.             */

    UCHAR                   service_host[NX_MDNS_HOST_NAME_MAX + 1];    /* The target host of the service.  */

    USHORT                  service_port;           /* Service port.                                        */

    USHORT                  service_weight;         /* Service weight.                                      */

    USHORT                  service_priority;       /* Service priority.                                    */

    UCHAR                   interface_index;        /* Service interface idnex.                             */

    UCHAR                   service_text_valid;     /* Flag indicating text fields are valid.               */

    ULONG                   service_ipv4;           /* The IPv4 address for service.                        */

    ULONG                   service_ipv6[NX_MDNS_IPV6_ADDRESS_COUNT][4];    /* The IPv6 address for service.*/

    UCHAR                   buffer[NX_MDNS_NAME_MAX+1]; /* The buffer for service instance, type and domain.*/
    
} NX_MDNS_SERVICE;


/* Define the basic Multicast DNS data structure.  */

typedef struct NX_MDNS_STRUCT 
{
    ULONG                   nx_mdns_id;             /* Multicast DNS ID.                                    */

    NX_IP*                  nx_mdns_ip_ptr;         /* Pointer to associated IP structure.                  */ 
    
    NX_UDP_SOCKET           nx_mdns_socket;         /* Multicast DNS Socket.                                */
    
    TX_MUTEX                nx_mdns_mutex;          /* Multicast DNS Mutex.                                 */

    TX_TIMER                nx_mdns_timer;          /* Multicast DNS TIMER.                                 */
         
    TX_THREAD               nx_mdns_thread;         /* Multicast DNS processing thread.                     */   
    
    TX_EVENT_FLAGS_GROUP    nx_mdns_events;         /* Multicast DNS event.                                 */

    NX_PACKET_POOL*         nx_mdns_packet_pool_ptr;    /* Pointer to the packet pool.                      */

    UCHAR                   nx_mdns_host_name[NX_MDNS_HOST_NAME_MAX];   /* The Embedded Device name supplied at create.  */

    UCHAR                   nx_mdns_domain_name[NX_MDNS_DOMAIN_NAME_MAX + 1];   /* The mDNS domain name.    */

    UINT                    nx_mdns_interface_enabled[NX_MAX_PHYSICAL_INTERFACES];  /* Interface enable status. */

#ifdef NX_MDNS_ENABLE_IPV6
    UINT                    nx_mdns_ipv6_address_index[NX_MAX_PHYSICAL_INTERFACES]; /* IPv6 link local address index.   */
#endif /* NX_MDNS_ENABLE_IPV6  */

    /* Define the UDP receive suspension list head associated with a count of 
       how many threads are suspended attempting to receive from the same TCP port.  */
    TX_THREAD*              nx_mdns_rr_receive_suspension_list;                 /* The suspended thread.    */

    ULONG                   nx_mdns_rr_receive_suspended_count;                 /* The suspended count.     */

    UCHAR*                  nx_mdns_local_service_cache;        /* Pointer to the cache.                    */
    
    UINT                    nx_mdns_local_service_cache_size;   /* The size of cache.                       */
    
    UCHAR*                  nx_mdns_peer_service_cache;         /* Pointer to the cache.                    */
    
    UINT                    nx_mdns_peer_service_cache_size;    /* The size of cache.                       */ 

    ULONG                   nx_mdns_service_ignore_mask;        /* The mask specifying services types to be ignored.                        */

    ULONG                   nx_mdns_service_notify_mask;        /* The service mask for listen the service and notify the applicaiton.      */

    ULONG                   nx_mdns_timer_min_count;            /* The minimum timer count indicate the intervals between last timer event and next timer event. */
        
    USHORT                  nx_mdns_announcing_period;          /* Number of ticks for the initial period. Default value is 1 second.       */

    USHORT                  nx_mdns_announcing_retrans_interval;    /* Number of ticks to wait ticks before sending out repeated announcement message. */    
       
    ULONG                   nx_mdns_announcing_period_interval;     /* Number of ticks between two announcing period.                       */

    UCHAR                   nx_mdns_announcing_count;           /* Number of repetitions between one announcing period. Default value is 1. */

    UCHAR                   nx_mdns_announcing_factor;          /* Telescopic factor, default value is 0. 2^k.                              */

    UCHAR                   nx_mdns_announcing_max_time;        /* Max time of Announcing period, default value is 3.                       */
    
    UCHAR                   nx_mdns_started;                    /* mDNS task has been started.                                              */
    
    ULONG                   nx_mdns_local_rr_count;             /* The number of resource records in the local cache.                       */
    
    ULONG                   nx_mdns_local_string_count;         /* The number of strings in the local cache.                                */
    
    ULONG                   nx_mdns_local_string_bytes;         /* The number of total bytes in string table in the local cache.            */
    
    ULONG                   nx_mdns_peer_rr_count;              /* The number of resource records in the peer cache.                        */
    
    ULONG                   nx_mdns_peer_string_count;          /* The number of strings in the peer cache.                                 */
    
    ULONG                   nx_mdns_peer_string_bytes;          /* The number of total bytes in string table in the peer cache.             */
    
    ULONG                   nx_mdns_first_probing_delay;        /* The delay of first probing for unique RR.                                */

    VOID                    (*nx_mdns_probing_notify)(struct NX_MDNS_STRUCT *, UCHAR *, UINT);

    VOID                    (*nx_mdns_service_change_notify)(struct NX_MDNS_STRUCT *, struct NX_MDNS_SERVICE_STRUCT *, UINT);

    VOID                    (*nx_mdns_cache_full_notify)(struct NX_MDNS_STRUCT *, UINT, UINT);

} NX_MDNS;


#ifndef NX_MDNS_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired. If so, map mDNS API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_mdns_create                          _nx_mdns_create
#define nx_mdns_delete                          _nx_mdns_delete
#define nx_mdns_enable                          _nx_mdns_enable
#define nx_mdns_disable                         _nx_mdns_disable
#define nx_mdns_cache_notify_set                _nx_mdns_cache_notify_set
#define nx_mdns_cache_notify_clear              _nx_mdns_cache_notify_clear
#define nx_mdns_domain_name_set                 _nx_mdns_domain_name_set

#ifndef NX_MDNS_DISABLE_SERVER
#define nx_mdns_service_announcement_timing_set _nx_mdns_service_announcement_timing_set
#define nx_mdns_service_add                     _nx_mdns_service_add
#define nx_mdns_service_delete                  _nx_mdns_service_delete
#define nx_mdns_local_cache_clear               _nx_mdns_local_cache_clear
#endif /* NX_MDNS_DISABLE_SERVER  */

#ifndef NX_MDNS_DISABLE_CLIENT
#define nx_mdns_service_one_shot_query          _nx_mdns_service_one_shot_query
#define nx_mdns_service_continuous_query        _nx_mdns_service_continuous_query
#define nx_mdns_service_query_stop              _nx_mdns_service_query_stop
#define nx_mdns_service_lookup                  _nx_mdns_service_lookup
#define nx_mdns_service_ignore_set              _nx_mdns_service_ignore_set 
#define nx_mdns_service_notify_set              _nx_mdns_service_notify_set
#define nx_mdns_service_notify_clear            _nx_mdns_service_notify_clear
#define nx_mdns_host_address_get                _nx_mdns_host_address_get
#define nx_mdns_peer_cache_clear                _nx_mdns_peer_cache_clear
#endif /* NX_MDNS_DISABLE_CLIENT  */

#else

#define nx_mdns_create                          _nxe_mdns_create
#define nx_mdns_delete                          _nxe_mdns_delete
#define nx_mdns_enable                          _nxe_mdns_enable
#define nx_mdns_disable                         _nxe_mdns_disable
#define nx_mdns_cache_notify_set                _nxe_mdns_cache_notify_set
#define nx_mdns_cache_notify_clear              _nxe_mdns_cache_notify_clear
#define nx_mdns_domain_name_set                 _nxe_mdns_domain_name_set

#ifndef NX_MDNS_DISABLE_SERVER
#define nx_mdns_service_announcement_timing_set _nxe_mdns_service_announcement_timing_set
#define nx_mdns_service_add                     _nxe_mdns_service_add
#define nx_mdns_service_delete                  _nxe_mdns_service_delete
#define nx_mdns_local_cache_clear               _nxe_mdns_local_cache_clear
#endif /* NX_MDNS_DISABLE_SERVER  */

#ifndef NX_MDNS_DISABLE_CLIENT
#define nx_mdns_service_one_shot_query          _nxe_mdns_service_one_shot_query
#define nx_mdns_service_continuous_query        _nxe_mdns_service_continuous_query
#define nx_mdns_service_query_stop              _nxe_mdns_service_query_stop
#define nx_mdns_service_lookup                  _nxe_mdns_service_lookup  
#define nx_mdns_service_ignore_set              _nxe_mdns_service_ignore_set 
#define nx_mdns_service_notify_set              _nxe_mdns_service_notify_set
#define nx_mdns_service_notify_clear            _nxe_mdns_service_notify_clear
#define nx_mdns_host_address_get                _nxe_mdns_host_address_get
#define nx_mdns_peer_cache_clear                _nxe_mdns_peer_cache_clear
#endif /* NX_MDNS_DISABLE_CLIENT  */

#endif  /* NX_DISABLE_ERROR_CHECKING */

#endif   /* NX_MDNS_SOURCE_CODE */


/* Define Thread function prototypes.  */
UINT        _nx_mdns_create(NX_MDNS *mdns_ptr, NX_IP *ip_ptr, NX_PACKET_POOL *packet_pool,
                            UINT priority, VOID *stack_ptr, ULONG stack_size, UCHAR *host_name,
                            VOID *local_cache_ptr, UINT local_cache_size, 
                            VOID *peer_cache_ptr, UINT peer_cache_size,
                            VOID (*probing_notify)(NX_MDNS *mdns_ptr, UCHAR *name, UINT probing_state));
UINT        _nxe_mdns_create(NX_MDNS *mdns_ptr, NX_IP *ip_ptr, NX_PACKET_POOL *packet_pool,
                             UINT priority, VOID *stack_ptr, ULONG stack_size, UCHAR *host_name,
                             VOID *local_cache_ptr, UINT local_cache_size, 
                             VOID *peer_cache_ptr, UINT peer_cache_size,
                             VOID (*probing_notify)(NX_MDNS *mdns_ptr, UCHAR *name, UINT probing_state));
UINT        _nx_mdns_delete(NX_MDNS *mdns_ptr);
UINT        _nxe_mdns_delete(NX_MDNS *mdns_ptr);
UINT        _nx_mdns_enable(NX_MDNS *mdns_ptr, UINT interface_index);
UINT        _nxe_mdns_enable(NX_MDNS *mdns_ptr, UINT interface_index);
UINT        _nx_mdns_disable(NX_MDNS *mdns_ptr, UINT interface_index); 
UINT        _nxe_mdns_disable(NX_MDNS *mdns_ptr, UINT interface_index);
UINT        _nx_mdns_cache_notify_set(NX_MDNS *mdns_ptr, VOID (*cache_full_notify_cb)(NX_MDNS *mdns_ptr, UINT state, UINT cache_type));
UINT        _nxe_mdns_cache_notify_set(NX_MDNS *mdns_ptr, VOID (*cache_full_notify_cb)(NX_MDNS *mdns_ptr, UINT state, UINT cache_type));
UINT        _nx_mdns_cache_notify_clear(NX_MDNS *mdns_ptr);
UINT        _nxe_mdns_cache_notify_clear(NX_MDNS *mdns_ptr);
UINT        _nx_mdns_domain_name_set(NX_MDNS *mdns_ptr, UCHAR *domain_name);
UINT        _nxe_mdns_domain_name_set(NX_MDNS *mdns_ptr, UCHAR *domain_name);

#ifndef NX_MDNS_DISABLE_SERVER
UINT        _nx_mdns_service_announcement_timing_set(NX_MDNS *mdns_ptr, UINT t, UINT p, UINT k, UINT retrans_interval, ULONG period_interval, UINT max_time);
UINT        _nxe_mdns_service_announcement_timing_set(NX_MDNS *mdns_ptr, UINT t, UINT p, UINT k, UINT retrans_interval, ULONG period_interval, UINT max_time);
UINT        _nx_mdns_service_add(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, UCHAR *txt, ULONG ttl,
                                 USHORT priority, USHORT weights, USHORT port, UCHAR is_unique, UINT interface_index);
UINT        _nxe_mdns_service_add(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, UCHAR *txt, ULONG ttl,
                                  USHORT priority, USHORT weights, USHORT port, UCHAR is_unique, UINT interface_index);
UINT        _nx_mdns_service_delete(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type); 
UINT        _nxe_mdns_service_delete(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type);
UINT        _nx_mdns_local_cache_clear(NX_MDNS *mdns_ptr);
UINT        _nxe_mdns_local_cache_clear(NX_MDNS *mdns_ptr);
#endif /* NX_MDNS_DISABLE_SERVER  */

#ifndef NX_MDNS_DISABLE_CLIENT
UINT        _nx_mdns_service_one_shot_query(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, NX_MDNS_SERVICE *service, UINT timeout);
UINT        _nxe_mdns_service_one_shot_query(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, NX_MDNS_SERVICE *service, UINT timeout);
UINT        _nx_mdns_service_continuous_query(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type);
UINT        _nxe_mdns_service_continuous_query(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type);
UINT        _nx_mdns_service_query_stop(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type); 
UINT        _nxe_mdns_service_query_stop(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type);
UINT        _nx_mdns_service_lookup(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, UINT service_index, NX_MDNS_SERVICE *service);
UINT        _nxe_mdns_service_lookup(NX_MDNS *mdns_ptr, UCHAR *name, UCHAR *type, UCHAR *sub_type, UINT service_index, NX_MDNS_SERVICE *service);    
UINT        _nx_mdns_service_ignore_set(NX_MDNS *mdns_ptr, ULONG service_mask);
UINT        _nxe_mdns_service_ignore_set(NX_MDNS *mdns_ptr, ULONG service_mask);
UINT        _nx_mdns_service_notify_set(NX_MDNS *mdns_ptr, ULONG service_mask,
                                        VOID (*service_change_notify)(NX_MDNS *mdns_ptr, NX_MDNS_SERVICE *service_ptr, UINT state)); 
UINT        _nxe_mdns_service_notify_set(NX_MDNS *mdns_ptr, ULONG service_mask,
                                        VOID (*service_change_notify)(NX_MDNS *mdns_ptr, NX_MDNS_SERVICE *service_ptr, UINT state));
UINT        _nx_mdns_service_notify_clear(NX_MDNS *mdns_ptr); 
UINT        _nxe_mdns_service_notify_clear(NX_MDNS *mdns_ptr);
UINT        _nx_mdns_host_address_get(NX_MDNS *mdns_ptr, UCHAR *host_name, ULONG *ipv4_address, ULONG *ipv6_address, UINT timeout);
UINT        _nxe_mdns_host_address_get(NX_MDNS *mdns_ptr, UCHAR *host_name, ULONG *ipv4_address, ULONG *ipv6_address, UINT timeout);
UINT        _nx_mdns_peer_cache_clear(NX_MDNS *mdns_ptr); 
UINT        _nxe_mdns_peer_cache_clear(NX_MDNS *mdns_ptr);
#endif /* NX_MDNS_DISABLE_CLIENT  */


/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif  /* NXD_MDNS_H */

