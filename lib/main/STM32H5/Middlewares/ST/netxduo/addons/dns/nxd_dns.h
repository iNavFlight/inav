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
/**   Domain Name System (DNS)                                            */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */ 
/*                                                                        */  
/*    nxd_dns.h                                           PORTABLE C      */
/*                                                           6.1.5        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX Domain Name System Protocol (DNS)        */ 
/*    component, including all data types and external references.        */ 
/*    It is assumed that nx_api.h and nx_port.h have already been         */ 
/*    included.                                                           */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Yuxin Zhou               Modified comment(s), prevented*/
/*                                            infinite loop in name       */
/*                                            compression, resulting in   */
/*                                            version 6.1.3               */
/*  03-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved the logic of       */
/*                                            receiving dns response,     */
/*                                            resulting in version 6.1.5  */
/*                                                                        */
/**************************************************************************/
          
#ifndef NXD_DNS_H
#define NXD_DNS_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif
    
#include "nx_udp.h"
#include "nx_ipv4.h"
#include "nx_ipv6.h"

/* Define the DNS ID.  */

#define NX_DNS_ID                       0x444e5320UL

/* Define message and name limits.  */

#define NX_DNS_LABEL_MAX                63          /* Maximum Label (between to dots) size                 */
#define NX_DNS_NAME_MAX                 255         /* Maximum Name size                                    */
#define NX_DNS_IP_LOOKUP_SIZE           75          /* IPv6 needs 66 characters for the address plus 8 for the ip6.arpa plus null           */
                                                    /* IPv4 needs 12 characters max for the address plus 12 for the IN-ADDR.ARPA plus null  */


/* Define offsets into the DNS message buffer.  */

#define NX_DNS_ID_OFFSET                0           /* Offset to ID code in DNS buffer                      */
#define NX_DNS_FLAGS_OFFSET             2           /* Offset to flags in DNS buffer                        */
#define NX_DNS_QDCOUNT_OFFSET           4           /* Offset to Question Count in DNS buffer               */
#define NX_DNS_ANCOUNT_OFFSET           6           /* Offset to Answer Count in DNS buffer                 */
#define NX_DNS_NSCOUNT_OFFSET           8           /* Offset to Authority Count in DNS buffer              */
#define NX_DNS_ARCOUNT_OFFSET           10          /* Offset to Additional Info. Count in DNS buffer       */
#define NX_DNS_QDSECT_OFFSET            12          /* Offset to Question Section in DNS buffer             */


/* Define return code constants.  */

#define NX_DNS_SUCCESS                  0x00        /* DNS success                                          */ 
#define NX_DNS_ERROR                    0xA0        /* DNS internal error                                   */ 
#define NX_DNS_NO_SERVER                0xA1        /* No DNS server was specified                          */ 
#define NX_DNS_TIMEOUT                  0xA2        /* DNS timeout occurred                                 */ 
#define NX_DNS_QUERY_FAILED             0xA3        /* DNS query failed; no DNS server sent an 'answer'     */ 
#define NX_DNS_BAD_ADDRESS_ERROR        0xA4        /* Improperly formatted IPv4 or IPv6 address            */ 
#define NX_DNS_SIZE_ERROR               0xA5        /* DNS destination size is too small                    */ 
#define NX_DNS_MALFORMED_PACKET         0xA6        /* Improperly formed or corrupted DNS packet received   */ 
#define NX_DNS_BAD_ID_ERROR             0xA7        /* DNS packet from server does not match query ID       */ 
#define NX_DNS_PARAM_ERROR              0xA8        /* Invalid non pointer input to API                     */
#define NX_DNS_SERVER_NOT_FOUND         0xA9        /* Server not found in Client list of DNS servers       */
#define NX_DNS_PACKET_CREATE_ERROR      0xAA        /* Error creating DNS packet                            */
#define NX_DNS_EMPTY_DNS_SERVER_LIST    0xAB        /* DNS Client's list of DNS servers is empty            */
#define NX_DNS_SERVER_AUTH_ERROR        0xAC        /* Server not able to authenticate answer/authority data*/
#define NX_DNS_ZERO_GATEWAY_IP_ADDRESS  0xAD        /* DNS Client IP instance has a zero gateway IP address */
#define NX_DNS_MISMATCHED_RESPONSE      0xAE        /* Server response type does not match the query request*/
#define NX_DNS_DUPLICATE_ENTRY          0xAF        /* Duplicate entry exists in DNS server table           */
#define NX_DNS_RETRY_A_QUERY            0xB0        /* SOA status returned; web site only exists as IPv4    */
#define NX_DNS_IPV6_DISABLED_ERROR      0xB1        /* Cannot process AAAA or PTR record with IPv6 disabled */
#define NX_DNS_INVALID_ADDRESS_TYPE     0xB2        /* IP address type (e.g. IPv6) not supported            */ 
#define NX_DNS_IPV6_NOT_SUPPORTED       0xB3        /* Cannot process AAAA or PTR record with IPv6 disabled */
#define NX_DNS_NEED_MORE_RECORD_BUFFER  0xB4        /* The buffer size is not enough.                       */
#define NX_DNS_FEATURE_NOT_SUPPORTED    0xB5        /* The requested feature is not supported in this build */
#define NX_DNS_NAME_MISMATCH            0xB6        /* The name mismatch.                                   */
#define NX_DNS_CACHE_ERROR              0xB7        /* The Cache size is not enough.                        */ 


/* Define constants for the flags word.  */

#define NX_DNS_QUERY_MASK               0x8000
#define NX_DNS_RESPONSE_FLAG            0x8000
#define NX_DNS_ERROR_MASK               0x8002      /* Server response indicates an error or data not authenticated by server */


#define NX_DNS_OPCODE_QUERY             (0 << 12)   /* Shifted right 12 is still 0                          */
#define NX_DNS_OPCODE_IQUERY            (1 << 12)   /* 1 shifted right by 12                                */
#define NX_DNS_OPCODE_STATUS            (2 << 12)   /* 2 shifted right by 12                                */

#define NX_DNS_AA_FLAG                  0x0400      /* Authoritative Answer                                 */
#define NX_DNS_TC_FLAG                  0x0200      /* Truncated                                            */
#define NX_DNS_RD_FLAG                  0x0100      /* Recursive Query                                      */
#define NX_DNS_RA_FLAG                  0x0080      /* Recursion Available                                  */
#define NX_DNS_FA_FLAG                  0x0010      /* Force Authentication                                 */

#define NX_DNS_RCODE_MASK               0x000f      /* Isolate the Result Code                              */
#define NX_DNS_RCODE_SUCCESS            0
#define NX_DNS_RCODE_FORMAT_ERR         1
#define NX_DNS_RCODE_SERVER_ERR         2
#define NX_DNS_RCODE_NAME_ERR           3
#define NX_DNS_RCODE_NOT_IMPL           4
#define NX_DNS_RCODE_REFUSED            5

#define NX_DNS_QUERY_FLAGS  (NX_DNS_OPCODE_QUERY | NX_DNS_RD_FLAG)  /* | NX_DNS_FA_FLAG */

/* Define name compression masks.  */

#define NX_DNS_COMPRESS_MASK            0xc0
#define NX_DNS_COMPRESS_VALUE           0xc0
#define NX_DNS_POINTER_MASK             0xc000

/* Define resource record types.  */

#define NX_DNS_RR_TYPE_A                1           /* Host address                                         */
#define NX_DNS_RR_TYPE_NS               2           /* Authoritative name server                            */
#define NX_DNS_RR_TYPE_MD               3           /* Mail destination (Obsolete - use MX)                 */
#define NX_DNS_RR_TYPE_MF               4           /* Mail forwarder (Obsolete - use MX)                   */
#define NX_DNS_RR_TYPE_CNAME            5           /* Canonical name for an alias                          */
#define NX_DNS_RR_TYPE_SOA              6           /* Marks the start of a zone of authority               */
#define NX_DNS_RR_TYPE_MB               7           /* Mailbox domain name (EXPERIMENTAL)                   */
#define NX_DNS_RR_TYPE_MG               8           /* Mail group member (EXPERIMENTAL)                     */
#define NX_DNS_RR_TYPE_MR               9           /* Mail rename domain name (EXPERIMENTAL)               */
#define NX_DNS_RR_TYPE_NULL             10          /* Null RR (EXPERIMENTAL)                               */
#define NX_DNS_RR_TYPE_WKS              11          /* Well known service description                       */
#define NX_DNS_RR_TYPE_PTR              12          /* Domain name pointer                                  */
#define NX_DNS_RR_TYPE_HINFO            13          /* Host information                                     */
#define NX_DNS_RR_TYPE_MINFO            14          /* Mailbox or mail list information                     */
#define NX_DNS_RR_TYPE_MX               15          /* Mail exchange                                        */
#define NX_DNS_RR_TYPE_TXT              16          /* Text strings                                         */
#define NX_DNS_RR_TYPE_AAAA             28          /* IPv6 Host address                                    */
#define NX_DNS_RR_TYPE_SRV              33          /* The location of services                             */


/* Define constants for Qtypes (queries).  */

#define NX_DNS_RR_TYPE_AXFR             252         /* Request for a transfer of an entire zone             */
#define NX_DNS_RR_TYPE_MAILB            253         /* Request for mailbox-related records (MB, MG or MR)   */
#define NX_DNS_RR_TYPE_MAILA            254         /* Request for mail agent RRs (Obsolete - see MX)       */
#define NX_DNS_RR_TYPE_ALL              255         /* Request for all records                              */

/* Define resource record classes.  */

#define NX_DNS_RR_CLASS_IN              1           /* Internet                                             */
#define NX_DNS_RR_CLASS_CS              2           /* CSNET class (Obsolete)                               */
#define NX_DNS_RR_CLASS_CH              3           /* CHAOS class                                          */
#define NX_DNS_RR_CLASS_HS              4           /* Hesiod [Dyer 87]                                     */


/* Define constant valid for Qtypes (queries).  */

#define NX_DNS_RR_CLASS_ALL                     255         /* Any class   */

/* Define the TCP and UDP port number */

#define NX_DNS_PORT                             53          /* Port for TX,RX and TCP/UDP */


/* Start of configurable options. */

/* Set the IP instance gateway server to be the DNS server if the gateway address is non zero.  */
/*
#define NX_DNS_IP_GATEWAY_AND_DNS_SERVER 
*/

/* Determine if the Client will create its own packet pool
   or let the host application create one. See nx_dns_packet_pool_set
   for how to set the DNS packet pool from the host application.   */
/*
#define NX_DNS_CLIENT_USER_CREATE_PACKET_POOL   
*/

/* Enable the feature to clear off old DNS packets before sending a fresh query.  */
/*
#define NX_DNS_CLIENT_CLEAR_QUEUE
*/

/* Enable the feature to send the TXT, CNAME, NS, MX, SRV, SOA DNS types query.  */
/*
#define NX_DNS_ENABLE_EXTENDED_RR_TYPES
*/

/* Enable the cache to store the resource record.  */
/*
#define NX_DNS_CACHE_ENABLE
*/

/* Define UDP socket create options.  */

#ifndef NX_DNS_TYPE_OF_SERVICE
#define NX_DNS_TYPE_OF_SERVICE          NX_IP_NORMAL
#endif

#ifndef NX_DNS_FRAGMENT_OPTION
#define NX_DNS_FRAGMENT_OPTION          NX_DONT_FRAGMENT
#endif  

#ifndef NX_DNS_TIME_TO_LIVE
#define NX_DNS_TIME_TO_LIVE                     0x80
#endif

/* Define the queue depth of the DNS Client socket. */

#ifndef NX_DNS_QUEUE_DEPTH
#define NX_DNS_QUEUE_DEPTH                      5
#endif

/* Define the maximum size of DNS message. 512 is the maximum size defined in RFC 1035 section 2.3.4. */

#ifndef NX_DNS_MESSAGE_MAX              
#define NX_DNS_MESSAGE_MAX                      512         
#endif

/* Define a payload to include the maximum size DNS message plus the Ethernet, IP and UDP overhead. */

#ifndef NX_DNS_PACKET_PAYLOAD_UNALIGNED
#define NX_DNS_PACKET_PAYLOAD_UNALIGNED         (NX_UDP_PACKET + NX_DNS_MESSAGE_MAX)
#endif

/* Round up to a 4 byte aligned packet payload. */

#define NX_DNS_PACKET_PAYLOAD                   (((NX_DNS_PACKET_PAYLOAD_UNALIGNED + sizeof(ULONG) - 1)/sizeof(ULONG)) * sizeof(ULONG))

/* Define the size (e.g. number of packets) of the DNS Client
   packet pool for sending out DNS messages. */

#ifndef NX_DNS_PACKET_POOL_SIZE
#define NX_DNS_PACKET_POOL_SIZE                 (16 * (NX_DNS_PACKET_PAYLOAD + sizeof(NX_PACKET)))
#endif


/* Define the maximum number of retries to a DNS server. */

#ifndef NX_DNS_MAX_RETRIES              
#define NX_DNS_MAX_RETRIES                      3
#endif

/* Define size of the DNS server list. Remember to allow for one 'null' terminating
   e.g. zero IP address entry for this list. */

#ifndef NX_DNS_MAX_SERVERS
#define NX_DNS_MAX_SERVERS                      5
#endif

/* Define the maximum amount of time to retransmit a DNS query. The default wait time is 64 seconds. 
   the retransmission policy are recommended in RFC1035 page 32.  */
#ifndef NX_DNS_MAX_RETRANS_TIMEOUT 
#define NX_DNS_MAX_RETRANS_TIMEOUT             (64 * NX_IP_PERIODIC_RATE)
#endif

/* Define the timeout option in timer ticks for allocating a packet
   from the DNS Client packet pool. */

#ifndef NX_DNS_PACKET_ALLOCATE_TIMEOUT
#define NX_DNS_PACKET_ALLOCATE_TIMEOUT          NX_IP_PERIODIC_RATE
#endif

/* Define the maximum number of pointers allowed in name compression.  */
#ifndef NX_DNS_MAX_COMPRESSION_POINTERS
#define NX_DNS_MAX_COMPRESSION_POINTERS        16
#endif

/* Define the basic DNS data structure.  */

typedef struct NX_IP_DNS_STRUCT 
{
    ULONG           nx_dns_id;                                      /* DNS ID                                                   */
    UCHAR           *nx_dns_domain;                                 /* Pointer to domain name                                   */ 
    USHORT          nx_dns_lookup_type;                             /* DNS look up type                                         */ 
    USHORT          nx_dns_transmit_id;                             /* DNS message transmit identifier                          */ 
    NX_IP           *nx_dns_ip_ptr;                                 /* Pointer to associated IP structure                       */ 
    NXD_ADDRESS     nx_dns_server_ip_array[NX_DNS_MAX_SERVERS];     /* List of DNS server IP addresses                          */ 
    ULONG           nx_dns_retries;                                 /* DNS query retries                                        */ 
#ifndef NX_DNS_CLIENT_USER_CREATE_PACKET_POOL
    NX_PACKET_POOL  nx_dns_pool;                                    /* The pool of UDP data packets for DNS messages            */
    UCHAR           nx_dns_pool_area[NX_DNS_PACKET_POOL_SIZE];
#endif
    NX_PACKET_POOL  *nx_dns_packet_pool_ptr;                        /* Pointer to DNS Client packet pool                        */
    NX_UDP_SOCKET   nx_dns_socket;                                  /* DNS Socket                                               */
    TX_MUTEX        nx_dns_mutex;                                   /* DNS Mutex to protect DNS instance                        */
#ifdef NX_DNS_CACHE_ENABLE
    UCHAR*          nx_dns_cache;                                   /* Pointer to the cache.                                    */        
    UINT            nx_dns_cache_size;                              /* The size of cache.                                       */
    ULONG           nx_dns_rr_count;                                /* The number of resource records in the cache.             */        
    ULONG           nx_dns_string_count;                            /* The number of strings in the cache.                      */         
    ULONG           nx_dns_string_bytes;                            /* The number of total bytes in string table in the cache.  */ 
    VOID            (*nx_dns_cache_full_notify)(struct NX_IP_DNS_STRUCT *);
#endif /* NX_DNS_CACHE_ENABLE  */
} NX_DNS;


/* Define the RDATA structure.  */
/********************************************************************************/
/*                               Type A                                         */     
/********************************************************************************/

/* A RDATA format
                        1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1                                             
    |-------------------------------------------------------------|
    |                           ADDRESS                           |
    |-------------------------------------------------------------|
*/
/* The host name may have multiple addresses,so we need a buffer to record the all addresses.
   return_buffer must be 4-byte aligned.  This return buffer is used to store IP addresses returned from the server. 
   record_number stores the number of entries in the buffer. If the server returns more entries than the buffer
   can hold, the entries exceeding the buffer size are dropped. */

/* Layout of the buffer
   |-------------------------------------------------------------------------------------| 
   |ip.address.0|ip.address.1|ip.address.2|.......                          |ip.address.n|
   |-------------------------------------------------------------------------------------| 
*/
        
/********************************************************************************/
/*                            Type AAAA                                         */     
/********************************************************************************/
/* AAAA RDATA format
                        1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1                                             
    |-------------------------------------------------------------|
    |                           ADDRESS                           |
    |-------------------------------------------------------------|                                        
    |                           ADDRESS                           |
    |-------------------------------------------------------------|                                        
    |                           ADDRESS                           |
    |-------------------------------------------------------------|                                        
    |                           ADDRESS                           |
    |-------------------------------------------------------------|
*/

typedef struct NX_DNS_IPV6_ADDRESS_STRUCT
{
    ULONG           ipv6_address[4];
} NX_DNS_IPV6_ADDRESS;


/* The return_buffer must be 4-byte aligned.  This return buffer is used to store IP addresses returned from the server. */
/* The record_number in the query stores the number of entries in the buffer. If the server returns more entries than the buffer
   can hold, the entries exceeding the buffer size are dropped. */

/* Layout of the buffer
   |---------------------------------------------------------------| 
   |ip.address.0[0]|ip.address.0[1]|ip.address.0[2]|ip.address.0[3]|
   |---------------------------------------------------------------| 
   |ip.address.1[0]|ip.address.1[1]|ip.address.1[2]|ip.address.1[3]|
   |---------------------------------------------------------------|
   |                                                               |
   |        ..........................................             |
   |                                                               |
   |---------------------------------------------------------------| 
   |ip.address.n[0]|ip.address.n[1]|ip.address.n[2]|ip.address.n[3]|
   |---------------------------------------------------------------|
 
*/

/********************************************************************************/
/*                                PTR                                           */
/********************************************************************************/
/* PTR RDATA format
                        1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1                                            
    |-------------------------------------------------------------|
    /                            PTRDNAME                         /
    /                                                             /
    |-------------------------------------------------------------|
*/


#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES

/********************************************************************************/
/*                             Type NS                                          */
/********************************************************************************/
/* NS RDATA format
                        1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1                                            
    |-------------------------------------------------------------|
    /                             NSDNAME                         /
    /                                                             /
    |-------------------------------------------------------------|
*/

typedef struct NX_DNS_NS_ENTRY_STRUCT
{
    ULONG           nx_dns_ns_ipv4_address;        /* The name server ipv4 address.  */
    UCHAR           *nx_dns_ns_hostname_ptr;       /* The name server.  */
} NX_DNS_NS_ENTRY;

/* Store the message in one buffer.  Layout of the buffer:

 record_buffer         |---------------------------------------------------------------| 
             entry 0   |xx.xx.xx.xx               |Pointer to the host name string     |
 record_buffer + 8     |---------------------------------------------------------------| 
             entry 1   |xx.xx.xx.xx               |Pointer to the host name string     |
 record_buffer + 16    |---------------------------------------------------------------|
             entry 2   |xx.xx.xx.xx               |Pointer to the host name string     | 
 record_buffer + 24    |---------------------------------------------------------------|
             entry 3   |xx.xx.xx.xx               |Pointer to the host name string     | 
 record_buffer + 32    |---------------------------------------------------------------| 
             entry 4   |xx.xx.xx.xx               |Pointer to the host name string     |
 record_buffer + 40    |---------------------------------------------------------------|
                       |xx.xx.xx.xx               |Pointer to the host name string     | 
                       |---------------------------------------------------------------|
                       |                               | ns_hostname 4                 |
                       |ns_hostname 3              | ns_hostname 2                     |
                       |ns_hostname 1         | ns_hostname 0                          |
 record_buffer + max   |---------------------------------------------------------------| 
 */

/********************************************************************************/
/*                                CNAME                                         */
/********************************************************************************/
/* CNAME RDATA format
                        1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1                                            
    |-------------------------------------------------------------|
    /                            CNAME                            /
    /                                                             /
    |-------------------------------------------------------------|
*/


/********************************************************************************/
/*                                 MX                                           */
/********************************************************************************/
/* MX RDATA format
                        1         
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5                                           
    |-----------------------------|
    |         PREFERECE           |
    |-----------------------------|
    /         EXCHANGE            /
    /                             /
    |-----------------------------|
*/

/* Define the mail exchange record type */
typedef struct NX_DNS_MAIL_EXCHANGE_ENTRY_STRUCT
{
    ULONG           nx_dns_mx_ipv4_address;           /* The mail exchange server ipv4 address.  */
    USHORT          nx_dns_mx_preference;             /* The preference given to this RR, Lower values are preferred. */
    USHORT          nx_dns_mx_reserved0;              /* Keep 4-byte aligned.  */
    UCHAR           *nx_dns_mx_hostname_ptr;          /* The mail exchange server host name.  */
} NX_DNS_MX_ENTRY;

/* Layout of the buffer
 record_buffer         |---------------------------------------------------------| 
             entry 0   |xx.xx.xx.xx|preference|res|pointer to host name string   |
 record_buffer + 12    |---------------------------------------------------------| 
             entry 1   |xx.xx.xx.xx|preference|res|pointer to host name string   |
 record_buffer + 24    |---------------------------------------------------------|
             entry 2   |xx.xx.xx.xx|preference|res|pointer to host name string   |
 record_buffer + 36    |---------------------------------------------------------|
             entry 3   |xx.xx.xx.xx|preference|res|pointer to host name string   |
 record_buffer + 48    |---------------------------------------------------------| 
             entry 4   |xx.xx.xx.xx|preference|res|pointer to host name string   |
 record_buffer + 60    |---------------------------------------------------------|
                       |xx.xx.xx.xx|preference|res|pointer to host name string   |
                       |---------------------------------------------------------|
                       |                         | mx_hostname 4                 |
                       |mx_hostname 3        | mx_hostname 2                     |
                       |mx_hostname 1   | mx_hostname 0                          |
 record_buffer + max   |---------------------------------------------------------| 
*/

/********************************************************************************/
/*                                SRV                                           */
/********************************************************************************/

/* Define the service record type. */
typedef struct NX_DNS_SERVICE_ENTRY_STRUCT
{
    ULONG           nx_dns_srv_ipv4_address;
    USHORT          nx_dns_srv_priority;
    USHORT          nx_dns_srv_weight;
    USHORT          nx_dns_srv_port_number;
    USHORT          nx_dns_srv_reserved0;
    UCHAR           *nx_dns_srv_hostname_ptr;
} NX_DNS_SRV_ENTRY;

/* Layout of the buffer:

 record_buffer         |-------------------------------------------------------------------| 
             entry 0   |xx.xx.xx.xx|priority|weight|port|res|pointer to host name string   |
 record_buffer + 16    |-------------------------------------------------------------------| 
             entry 1   |xx.xx.xx.xx|priority|weight|port|res|pointer to host name string   |
 record_buffer + 32    |-------------------------------------------------------------------|
             entry 2   |xx.xx.xx.xx|priority|weight|port|res|pointer to host name string   |
 record_buffer + 48    |-------------------------------------------------------------------|
             entry 3   |xx.xx.xx.xx|priority|weight|port|res|pointer to host name string   |
 record_buffer + 64    |-------------------------------------------------------------------| 
             entry 4   |xx.xx.xx.xx|priority|weight|port|res|pointer to host name string   |
 record_buffer + 80    |-------------------------------------------------------------------|
                       |xx.xx.xx.xx|priority|weight|port|res|pointer to host name string   |
                       |                                   | srv_hostname 4                |
                       |srv_hostname 3                  | srv_hostname 2                   |
                       |srv_hostname 1             | srv_hostname 0                        |
 record_buffer + max   |-------------------------------------------------------------------| 

*/
/********************************************************************************/
/*                                TXT                                           */
/********************************************************************************/
/* TXT RDATA format
                        1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1                                            
    |-------------------------------------------------------------|
    /                            TXT-DATA                         /
    /                                                             /
    |-------------------------------------------------------------|
*/

/********************************************************************************/
/*                                SOA                                           */
/********************************************************************************/
/* SOA RDATA format
                        1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1                                            
    |-------------------------------------------------------------|
    /                            MNAME                            /
    /                                                             /
    |-------------------------------------------------------------| 
    /                            RNAME                            /
    /                                                             / 
    |-------------------------------------------------------------|                                      
    |                            SERIAL                           | 
    |-------------------------------------------------------------|                                      
    |                            REFRESH                          | 
    |-------------------------------------------------------------|                                      
    |                            RETRY                            | 
    |-------------------------------------------------------------|                                      
    |                            EXPIRE                           | 
    |-------------------------------------------------------------|                                      
    |                            MINMUM                           | 
    |-------------------------------------------------------------|
*/

/* Define the service record type. */

typedef struct NX_DNS_SOA_ENTRY_STRUCT
{
    
    UCHAR           *nx_dns_soa_host_mname_ptr;
    UCHAR           *nx_dns_soa_host_rname_ptr;
    ULONG           nx_dns_soa_serial;
    ULONG           nx_dns_soa_refresh;
    ULONG           nx_dns_soa_retry;
    ULONG           nx_dns_soa_expire;
    ULONG           nx_dns_soa_minmum;
} NX_DNS_SOA_ENTRY;

/* Layout of the buffer
 record_buffer         
                       
                       |-------------------------------------------------------------------| 
                       | pointer to primary name server| pointer to responsible mailbox    |
                       |-------------------------------------------------------------------| 
                       |   serial   |  refresh   |    retry   |   expire   |   minmum      |
                       |-------------------------------------------------------------------| 
                       | soa_host_mname         | soa_host_rname                           |
                       |-------------------------------------------------------------------| 
 */

/********************************************************************************/
/*                                LOC                                           */
/********************************************************************************/
/* To be implemented */

/********************************************************************************/
/*                                NAPTR                                         */
/********************************************************************************/
/* To be implemented */

/********************************************************************************/
/*                                DNAME                                           */
/********************************************************************************/
/* To be implemented */

#endif

               
/* Define the RDATA structure.  */

/* A RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    |                     ADDRESS                   |
    |                                               |
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_DNS_RR_A_STRUCT
{
    ULONG           nx_dns_rr_a_address;
} NX_DNS_RR_A;
                    
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

typedef struct NX_DNS_RR_AAAA_STRUCT
{
    ULONG*          nx_dns_rr_aaaa_address;         /* Keep the same resource record structure size, store the rdata information into cache with string.  */   
} NX_DNS_RR_AAAA;

                       
/* PTR RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     PTRDNAME                  /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_DNS_RR_PTR_STRUCT
{
    UCHAR*          nx_dns_rr_ptr_name;
} NX_DNS_RR_PTR;


/* NS RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     NSDNAME                   /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_DNS_RR_NS_STRUCT
{
    UCHAR*          nx_dns_rr_ns_name;
} NX_DNS_RR_NS;
                  
/* CNMAE RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     CNAME                     /
    /                                               /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_DNS_RR_CNAME_STRUCT
{
    UCHAR*          nx_dns_rr_cname_name;
} NX_DNS_RR_CNAME;
              
                   
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

typedef struct NX_DNS_RR_MX_STRUCT
{
    UCHAR*          nx_dns_rr_mx_rdata;             /* Keep the same resource record structure size, store the rdata information into cache with string. "preference, newname" */
} NX_DNS_RR_MX;

/* TXT RDATA format
      0                             1
      0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5                                           
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
    /                     TXT-DATA                  /
    |--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--|
*/

typedef struct NX_DNS_RR_TXT_STRUCT
{
    UCHAR*          nx_dns_rr_txt_data;
} NX_DNS_RR_TXT;

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

typedef struct NX_DNS_RR_SRV_STRUCT
{
    UCHAR*          nx_dns_rr_srv_rdata;        /* Keep the same resource record structure size, store the rdata information into cache with string, "priority, weights, port,target".  */
} NX_DNS_RR_SRV;
                    

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

typedef struct NX_DNS_RR_SOA_STRUCT
{
    UCHAR*          nx_dns_rr_soa_rdata;            /* Keep the same resource record structure size, store the rdata information into cache with string. 
                                                       "Mname, Rname, Serial, Refresh, Retry, Expire, Minmum"  */
} NX_DNS_RR_SOA;


/* The following data structure is used to store the resource records(RRs). */
typedef struct NX_DNS_RR_STRUCT
{   

    UCHAR*  nx_dns_rr_name;                     /* Owner's name,i.e., the name of the node to which this resource record pertains. */

    USHORT  nx_dns_rr_type;                     /* RR TYPE.                                                 */

    USHORT  nx_dns_rr_class;                    /* RR CLASS.                                                */

    ULONG   nx_dns_rr_ttl;                      /* The time interval that the RR may be cached before the source of the information should again be consulted. */
                                     
    ULONG   nx_dns_rr_last_used_time;           /* Define the last used time for the peer RR.               */

    /* Union that holds resource record data. */
    union   nx_dns_rr_rdata_union
    {
        NX_DNS_RR_A         nx_dns_rr_rdata_a;
        NX_DNS_RR_AAAA      nx_dns_rr_rdata_aaaa;
        NX_DNS_RR_PTR       nx_dns_rr_rdata_ptr;
        NX_DNS_RR_SRV       nx_dns_rr_rdata_srv;
        NX_DNS_RR_TXT       nx_dns_rr_rdata_txt;
        NX_DNS_RR_CNAME     nx_dns_rr_rdata_cname;
        NX_DNS_RR_NS        nx_dns_rr_rdata_ns;
        NX_DNS_RR_MX        nx_dns_rr_rdata_mx;  
        NX_DNS_RR_SOA       nx_dns_rr_rdata_soa; 
    } nx_dns_rr_rdata;

}NX_DNS_RR;


#ifndef NX_DNS_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_dns_create                               _nx_dns_create
#define nx_dns_delete                               _nx_dns_delete
#define nx_dns_packet_pool_set                      _nx_dns_packet_pool_set
#define nx_dns_host_by_address_get                  _nx_dns_host_by_address_get
#define nx_dns_host_by_name_get                     _nx_dns_host_by_name_get
#define nx_dns_ipv4_address_by_name_get             _nx_dns_ipv4_address_by_name_get
   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
#define nx_dns_cname_get                            _nx_dns_cname_get 
#define nx_dns_domain_name_server_get               _nx_dns_domain_name_server_get
#define nx_dns_host_text_get                        _nx_dns_host_text_get
#define nx_dns_domain_mail_exchange_get             _nx_dns_domain_mail_exchange_get
#define nx_dns_domain_service_get                   _nx_dns_domain_service_get
#define nx_dns_authority_zone_start_get             _nx_dns_authority_zone_start_get
#define nx_dns_info_by_name_get                     _nx_dns_info_by_name_get
#endif

#define nx_dns_server_add                           _nx_dns_server_add
#define nx_dns_server_remove                        _nx_dns_server_remove
#define nx_dns_server_remove_all                    _nx_dns_server_remove_all
#define nx_dns_server_get                           _nx_dns_server_get
#define nx_dns_get_serverlist_size                  _nx_dns_get_serverlist_size

#define nxd_dns_ipv6_address_by_name_get            _nxd_dns_ipv6_address_by_name_get
#define nxd_dns_host_by_address_get                 _nxd_dns_host_by_address_get
#define nxd_dns_host_by_name_get                    _nxd_dns_host_by_name_get
#define nxd_dns_server_add                          _nxd_dns_server_add
#define nxd_dns_server_remove                       _nxd_dns_server_remove
#define nxd_dns_server_get                          _nxd_dns_server_get

#ifdef NX_DNS_CACHE_ENABLE
#define nx_dns_cache_initialize                     _nx_dns_cache_initialize   
#define nx_dns_cache_notify_set                     _nx_dns_cache_notify_set
#define nx_dns_cache_notify_clear                   _nx_dns_cache_notify_clear
#endif /* NX_DNS_CACHE_ENABLE  */

#else

/* Services with error checking.  */

#define nx_dns_create                               _nxe_dns_create
#define nx_dns_delete                               _nxe_dns_delete
#define nx_dns_packet_pool_set                      _nxe_dns_packet_pool_set
#define nx_dns_host_by_address_get                  _nxe_dns_host_by_address_get
#define nx_dns_host_by_name_get                     _nxe_dns_host_by_name_get
#define nx_dns_ipv4_address_by_name_get             _nxe_dns_ipv4_address_by_name_get
   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
#define nx_dns_cname_get                            _nxe_dns_cname_get 
#define nx_dns_domain_name_server_get               _nxe_dns_domain_name_server_get
#define nx_dns_host_text_get                        _nxe_dns_host_text_get
#define nx_dns_domain_mail_exchange_get             _nxe_dns_domain_mail_exchange_get
#define nx_dns_domain_service_get                   _nxe_dns_domain_service_get
#define nx_dns_authority_zone_start_get             _nxe_dns_authority_zone_start_get
#define nx_dns_info_by_name_get                     _nxe_dns_info_by_name_get
#endif

#define nx_dns_server_add                           _nxe_dns_server_add
#define nx_dns_server_remove                        _nxe_dns_server_remove
#define nx_dns_server_remove_all                    _nxe_dns_server_remove_all
#define nx_dns_server_get                           _nxe_dns_server_get
#define nx_dns_get_serverlist_size                  _nxe_dns_get_serverlist_size

#define nxd_dns_ipv6_address_by_name_get            _nxde_dns_ipv6_address_by_name_get
#define nxd_dns_host_by_address_get                 _nxde_dns_host_by_address_get
#define nxd_dns_host_by_name_get                    _nxde_dns_host_by_name_get
#define nxd_dns_server_add                          _nxde_dns_server_add
#define nxd_dns_server_remove                       _nxde_dns_server_remove
#define nxd_dns_server_get                          _nxde_dns_server_get

#ifdef NX_DNS_CACHE_ENABLE
#define nx_dns_cache_initialize                     _nxe_dns_cache_initialize   
#define nx_dns_cache_notify_set                     _nxe_dns_cache_notify_set
#define nx_dns_cache_notify_clear                   _nxe_dns_cache_notify_clear
#endif /* NX_DNS_CACHE_ENABLE  */

#endif

/* Define the prototypes accessible to the application software.  */

UINT        nx_dns_create(NX_DNS *dns_ptr, NX_IP *ip_ptr, UCHAR *domain_name);
UINT        nx_dns_delete(NX_DNS *dns_ptr);
UINT        nx_dns_packet_pool_set(NX_DNS *dns_ptr, NX_PACKET_POOL *packet_pool_ptr);
UINT        nx_dns_server_add(NX_DNS *dns_ptr, ULONG server_address);
UINT        nx_dns_server_remove(NX_DNS *dns_ptr, ULONG server_address);
UINT        nx_dns_server_remove_all(NX_DNS *dns_ptr);
UINT        nx_dns_server_get(NX_DNS *dns_ptr, UINT index, ULONG *dns_server_address);
UINT        nx_dns_get_serverlist_size(NX_DNS *dns_ptr, UINT *server_list_size);
UINT        nx_dns_host_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, ULONG *host_address_ptr, ULONG wait_option);
UINT        nx_dns_host_by_address_get(NX_DNS *dns_ptr, ULONG ip_address, UCHAR *host_name_ptr, UINT host_name_buffer_size, ULONG wait_option);
UINT        nx_dns_ipv4_address_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name,  VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
UINT        nx_dns_cname_get (NX_DNS *dns_ptr, UCHAR *host_name,  UCHAR *record_buffer, UINT buffer_size, ULONG wait_option);
UINT        nx_dns_domain_name_server_get(NX_DNS *dns_ptr, UCHAR *host_name,  VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
UINT        nx_dns_host_text_get(NX_DNS *dns_ptr, UCHAR *host_name,  UCHAR *record_buffer, UINT buffer_size, ULONG wait_option);
UINT        nx_dns_domain_mail_exchange_get(NX_DNS *dns_ptr, UCHAR *host_name,  VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
UINT        nx_dns_domain_service_get(NX_DNS *dns_ptr, UCHAR *host_name,  VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
UINT        nx_dns_authority_zone_start_get (NX_DNS *dns_ptr, UCHAR *host_name,  UCHAR *record_buffer, UINT buffer_size, ULONG wait_option);
UINT        nx_dns_info_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, ULONG *host_address_ptr, USHORT* host_port, ULONG wait_option);
#endif

UINT        nxd_dns_ipv6_address_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
UINT        nxd_dns_host_by_address_get(NX_DNS *dns_ptr, NXD_ADDRESS *ip_address, UCHAR *host_name_ptr, UINT host_name_buffer_size, ULONG wait_option);
UINT        nxd_dns_host_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, NXD_ADDRESS *host_address_ptr, ULONG wait_option, UINT lookup_type);
UINT        nxd_dns_server_add(NX_DNS *dns_ptr, NXD_ADDRESS *dns_server_address);
UINT        nxd_dns_server_remove(NX_DNS *dns_ptr, NXD_ADDRESS *server_address);
UINT        nxd_dns_server_get(NX_DNS *dns_ptr, UINT index, NXD_ADDRESS *dns_server_address);

#ifdef NX_DNS_CACHE_ENABLE
UINT        nx_dns_cache_initialize(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size); 
UINT        nx_dns_cache_notify_set(NX_DNS *dns_ptr, VOID (*cache_full_notify_cb)(NX_DNS *dns_ptr));
UINT        nx_dns_cache_notify_clear(NX_DNS *dns_ptr);    
#endif /* NX_DNS_CACHE_ENABLE  */

#else

/* DNS source code is being compiled, do not perform any API mapping.  */

UINT        _nxe_dns_create(NX_DNS *dns_ptr, NX_IP *ip_ptr, UCHAR *domain_name);
UINT        _nx_dns_create(NX_DNS *dns_ptr, NX_IP *ip_ptr, UCHAR *domain_name);
UINT        _nxe_dns_delete(NX_DNS *dns_ptr);
UINT        _nx_dns_delete(NX_DNS *dns_ptr);
UINT        _nxe_dns_packet_pool_set(NX_DNS *dns_ptr, NX_PACKET_POOL *packet_pool_ptr);
UINT        _nx_dns_packet_pool_set(NX_DNS *dns_ptr, NX_PACKET_POOL *packet_pool_ptr);
UINT        _nxe_dns_host_by_address_get(NX_DNS *dns_ptr, ULONG ip_address, UCHAR *host_name_ptr, UINT host_name_buffer_size, ULONG wait_option);
UINT        _nx_dns_host_by_address_get(NX_DNS *dns_ptr, ULONG ip_address, UCHAR *host_name_ptr, UINT host_name_buffer_size, ULONG wait_option);
UINT        _nxe_dns_host_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, ULONG *host_address_ptr, ULONG wait_option);
UINT        _nx_dns_host_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, ULONG *host_address_ptr, ULONG wait_option);
UINT        _nxe_dns_ipv4_address_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
UINT        _nx_dns_ipv4_address_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
   
#ifdef NX_DNS_ENABLE_EXTENDED_RR_TYPES 
UINT        _nxe_dns_cname_get (NX_DNS *dns_ptr, UCHAR *host_name,  UCHAR *record_buffer, UINT buffer_size,ULONG wait_option);
UINT        _nx_dns_cname_get (NX_DNS *dns_ptr, UCHAR *host_name,  UCHAR *record_buffer, UINT buffer_size,ULONG wait_option);
UINT        _nxe_dns_domain_name_server_get(NX_DNS *dns_ptr, UCHAR *host_name,  VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
UINT        _nx_dns_domain_name_server_get(NX_DNS *dns_ptr, UCHAR *host_name,  VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
UINT        _nxe_dns_host_text_get(NX_DNS *dns_ptr, UCHAR *host_name,  UCHAR *record_buffer, UINT buffer_size, ULONG wait_option);
UINT        _nx_dns_host_text_get(NX_DNS *dns_ptr, UCHAR *host_name,  UCHAR *record_buffer, UINT buffer_size, ULONG wait_option);
UINT        _nxe_dns_domain_mail_exchange_get(NX_DNS *dns_ptr, UCHAR *host_name,  VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
UINT        _nx_dns_domain_mail_exchange_get(NX_DNS *dns_ptr, UCHAR *host_name,  VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
UINT        _nxe_dns_domain_service_get(NX_DNS *dns_ptr, UCHAR *host_name,  VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
UINT        _nx_dns_domain_service_get(NX_DNS *dns_ptr, UCHAR *host_name,  VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
UINT        _nxe_dns_authority_zone_start_get (NX_DNS *dns_ptr, UCHAR *host_name,  UCHAR *record_buffer, UINT buffer_size, ULONG wait_option);
UINT        _nx_dns_authority_zone_start_get (NX_DNS *dns_ptr, UCHAR *host_name,  UCHAR *record_buffer, UINT buffer_size, ULONG wait_option);
UINT        _nxe_dns_info_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, ULONG *host_address_ptr, USHORT* host_port, ULONG wait_option);
UINT        _nx_dns_info_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, ULONG *host_address_ptr, USHORT* host_port, ULONG wait_option);
#endif

UINT        _nxe_dns_server_add(NX_DNS *dns_ptr, ULONG server_address);
UINT        _nx_dns_server_add(NX_DNS *dns_ptr, ULONG server_address);
UINT        _nxe_dns_server_remove(NX_DNS *dns_ptr, ULONG server_address);
UINT        _nx_dns_server_remove(NX_DNS *dns_ptr, ULONG server_address);
UINT        _nxe_dns_server_remove_all(NX_DNS *dns_ptr);
UINT        _nx_dns_server_remove_all(NX_DNS *dns_ptr);
UINT        _nxe_dns_server_get(NX_DNS *dns_ptr, UINT index, ULONG *dns_server_address);
UINT        _nx_dns_server_get(NX_DNS *dns_ptr, UINT index, ULONG *dns_server_address);
UINT        _nxe_dns_get_serverlist_size(NX_DNS *dns_ptr, UINT *server_list_size); 
UINT        _nx_dns_get_serverlist_size(NX_DNS *dns_ptr, UINT *server_list_size);
UINT        _nxde_dns_ipv6_address_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
UINT        _nxd_dns_ipv6_address_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, VOID *record_buffer, UINT buffer_size, UINT *record_count, ULONG wait_option);
UINT        _nxde_dns_host_by_address_get(NX_DNS *dns_ptr, NXD_ADDRESS *host_address_ptr, UCHAR *host_name_ptr, UINT host_name_buffer_size, ULONG wait_option);
UINT        _nxd_dns_host_by_address_get(NX_DNS *dns_ptr, NXD_ADDRESS *host_address_ptr, UCHAR *host_name_ptr, UINT host_name_buffer_size, ULONG wait_option);
UINT        _nxde_dns_host_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, NXD_ADDRESS *host_address_ptr, ULONG wait_option, UINT lookup_type);
UINT        _nxd_dns_host_by_name_get(NX_DNS *dns_ptr, UCHAR *host_name, NXD_ADDRESS *host_address_ptr, ULONG wait_option, UINT lookup_type);
UINT        _nxde_dns_server_add(NX_DNS *dns_ptr, NXD_ADDRESS *server_address);
UINT        _nxd_dns_server_add(NX_DNS *dns_ptr, NXD_ADDRESS *server_address);
UINT        _nxde_dns_server_remove(NX_DNS *dns_ptr, NXD_ADDRESS *server_address);
UINT        _nxd_dns_server_remove(NX_DNS *dns_ptr, NXD_ADDRESS *server_address);
UINT        _nxde_dns_server_get(NX_DNS *dns_ptr, UINT index, NXD_ADDRESS *dns_server_address);
UINT        _nxd_dns_server_get(NX_DNS *dns_ptr, UINT index, NXD_ADDRESS *dns_server_address);

#ifdef NX_DNS_CACHE_ENABLE                                                               
UINT        _nxe_dns_cache_initialize(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size); 
UINT        _nx_dns_cache_initialize(NX_DNS *dns_ptr, VOID *cache_ptr, UINT cache_size);            
UINT        _nxe_dns_cache_notify_set(NX_DNS *dns_ptr, VOID (*cache_full_notify_cb)(NX_DNS *dns_ptr));
UINT        _nx_dns_cache_notify_set(NX_DNS *dns_ptr, VOID (*cache_full_notify_cb)(NX_DNS *dns_ptr)); 
UINT        _nxe_dns_cache_notify_clear(NX_DNS *dns_ptr);     
UINT        _nx_dns_cache_notify_clear(NX_DNS *dns_ptr);    
#endif /* NX_DNS_CACHE_ENABLE  */

#endif

/* Internal DNS response getting function.  */
UINT        _nx_dns_response_get(NX_DNS *dns_ptr, UCHAR *host_name, UCHAR *record_buffer, UINT buffer_size, 
                                 UINT *record_count, ULONG wait_option);

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif  /* NXD_DNS_H */

