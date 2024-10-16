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
/**   Dynamic Host Configuration Protocol over IPv6 (DHCPv6)              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */ 
/*                                                                        */ 
/*    nxd_dhcpv6_server.h                                 PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX Dynamic Host Configuration Protocol over */ 
/*    IPv6 (DHCPv6) component, including all data types and external      */ 
/*    references. It is assumed that nx_api.h and nx_port.h have already  */ 
/*    been included.                                                      */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), improved */
/*                                            packet length verification, */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), included */
/*                                            necessary header file,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/

#ifndef NXD_DHCPV6_SERVER_H
#define NXD_DHCPV6_SERVER_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"

/* Define the DHCPv6 ID to mark the DHCPV6_CLIENT and DHCPV6_SERVER structures as created.  */

#define NX_DHCPV6_CLIENT_ID               0x44484336UL
#define NX_DHCPV6_SERVER_ID               0x44484337UL


/* Define the conversion between timer ticks and seconds (processor dependent). */

#define NX_DHCPV6_SERVER_TICKS_PER_SECOND                   NX_IP_PERIODIC_RATE


/* Set up interface defines for NetX Duo. */

#define LINK_LOCAL_INTERFACE                                0
#define GLOBAL_IP_INTERFACE                                 1 


/* Set the Client lease time. An infinate lease time is not recommended by the RFC 
   unless the Client requires a permanent IP address.  Most servers will likely not
   grant an infinite IP address lease. */

#define NX_DHCPV6_INFINITE_LEASE                            0xffffffffUL
#define NX_DHCPV6_MULTICAST_MASK                            0xff000000UL


/* Define RFC mandated (draft only) option codes. */

#define NX_DHCPV6_RFC_DNS_SERVER_OPTION                     0x00000017UL    /* RFC Option code for requesting DNS server IP address  */
#define NX_DHCPV6_RFC_TIME_SERVER_OPTION                    0x0000001FUL    /* RFC Option code for requesting tme server IP address. */
#define NX_DHCPV6_RFC_TIME_ZONE_OPTION                      0x00000029UL    /* RFC Option code for requesting Time zone. */
#define NX_DHCPV6_RFC_DOMAIN_NAME                           0x00000018UL    /* RFC Option code for requesting domain name. */


/* Define the DHCPv6 DUID types supported by the NetX DHCPv6 Server. */

typedef enum 
{
NX_DHCPV6_SERVER_DUID_TYPE_LINK_TIME =                         1,
NX_DHCPV6_SERVER_DUID_TYPE_VENDOR_ASSIGNED,                    
NX_DHCPV6_SERVER_DUID_TYPE_LINK_ONLY

} NX_DHCPV6_DUID_TYPE_SERVER;

/* Define constants for denoting server vs client DUIDs. */

#define NX_DHCPV6_SERVER_DUID_TYPE                          1
#define NX_DHCPV6_CLIENT_DUID_TYPE                          2

/* Define approximate time since Jan 1, 2000 for computing DUID time. This will form the 
   basis for the DUID time ID field.  */

#define SECONDS_SINCE_JAN_1_2000_MOD_32                     2563729999UL 


/* Define the Hardware types.  */
#define NX_DHCPV6_SERVER_HARDWARE_TYPE_ETHERNET             1
#define NX_DHCPV6_SERVER_HARDWARE_TYPE_EUI_64               27

/* NX_DHCPV6_HW_TYPE_IEEE_802 is defined as 1 to indicate Ethernet hardware type in old releases, for backward compatibility.

   Note: NX_DHCPV6_HW_TYPE_IEEE_802 will be deprecated by NX_DHCPV6_SERVER_HARDWARE_TYPE_ETHERNET
         in future releases, should use above symbols to define hardware types.
*/
#define NX_DHCPV6_HW_TYPE_IEEE_802                          1


/* Define the symbol for a static IP address lease, e.g. infinity. */

#define NX_DHCPV6_INFINTY_LEASE                             0xFFFFFFFF


/* Define the DHCPv6 Message Types.  */

#define NX_DHCPV6_MESSAGE_TYPE_DHCPSILENT               0
#define NX_DHCPV6_MESSAGE_TYPE_SOLICIT                  1
#define NX_DHCPV6_MESSAGE_TYPE_ADVERTISE                2
#define NX_DHCPV6_MESSAGE_TYPE_REQUEST                  3
#define NX_DHCPV6_MESSAGE_TYPE_CONFIRM                  4
#define NX_DHCPV6_MESSAGE_TYPE_RENEW                    5
#define NX_DHCPV6_MESSAGE_TYPE_REBIND                   6
#define NX_DHCPV6_MESSAGE_TYPE_REPLY                    7
#define NX_DHCPV6_MESSAGE_TYPE_RELEASE                  8
#define NX_DHCPV6_MESSAGE_TYPE_DECLINE                  9
#define NX_DHCPV6_MESSAGE_TYPE_RECONFIGURE              10
#define NX_DHCPV6_MESSAGE_TYPE_INFORM_REQUEST           11


/* Define the DHCPv6 Options.  */

#define NX_DHCPV6_OP_DUID_CLIENT                        1         /* Client DUID (DHCP unique identifier) */
#define NX_DHCPV6_OP_DUID_SERVER                        2         /* Server DUID (DHCP unique identifier) */
#define NX_DHCPV6_OP_IA_NA                              3         /* Identity association for non temporary addresses */
#define NX_DHCPV6_OP_IA_TA                              4         /* Identity association for temporary addresses */  
#define NX_DHCPV6_OP_IA_ADDRESS                         5         /* Address associated with IA_NA or IA_TA */
#define NX_DHCPV6_OP_OPTION_REQUEST                     6         /* Identifies a list of options */
#define NX_DHCPV6_OP_PREFERENCE                         7         /* Server's means of affecting Client choice of servers. */
#define NX_DHCPV6_OP_ELAPSED_TIME                       8         /* Duration of Client exchange with DHCPv6 server  */
#define NX_DHCPV6_OP_RELAY_MESSAGE                      9         /* Not in use in NetX DHCPV6 */
#define NX_DHCPV6_OP_AUTHENTICATION                     11        /* Not in use in NetX DHCPV6 */
#define NX_DHCPV6_OP_SERVER_UNICAST                     12        /* Server ok's allowing the client to address it in Unicast */
#define NX_DHCPV6_OP_OPTION_STATUS                      13        /* Option request status.  */
#define NX_DHCPV6_OP_RAPID_COMMIT                       14        /* Option requesting exchange of two DHCPv6 packets to assign address  */


/* Define the DHCPv6 Client states */
                                                                                   
#define NX_DHCPV6_STATE_BOUND                            1          /* Client not known to be bound to an IP address.   */
#define NX_DHCPV6_STATE_UNBOUND                          2          /* Client is bound to an IP address (not necessarily by this server).  */


/* Define DHCPv6 event flags.  These events are processed by the Server DHCPv6 thread. */

#define NX_DHCPV6_ALL_EVENTS                            0xFFFFFFFFUL    /* All Server DHCPv6 event flags */
#define NX_DHCPV6_SERVER_RECEIVE_EVENT                  0x00000001UL    /* Packet received on the DHCPv6 server queue. */
#define NX_DHCPV6_IP_LEASE_CHECK_PERIODIC_EVENT         0x00000008UL    /* Time keeper to check for expiration on leased IP addresses. */
#define NX_DHCPV6_CHECK_SESSION_PERIODIC_EVENT          0x00000004UL    /* Time keeper to check if each session with active client has timed out. */


/* Define DHCPv6 client and server ports.  */

#define NX_DHCPV6_SERVER_UDP_PORT                       547
#define NX_DHCPV6_CLIENT_UDP_PORT                       546


/* Define error codes from DHCPv6 API.  */

#define NX_DHCPV6_ALREADY_STARTED                       0xE91    /* DHCPv6 already started when API called to start it. */
#define NX_DHCPV6_NOT_STARTED                           0xE92    /* DHCPv6 was not started when API was called  */ 
#define NX_DHCPV6_PARAM_ERROR                           0xE93    /* Invalid non pointer input to API */
#define NX_DHCPV6_INVALID_DEVICE_MAC_ADDRESS            0xE94    /* DHCPv6 server device mac address unknown or undefined */
#define NX_DHCPV6_INVALID_INTERFACE_IP_ADDRESS          0xE95    /* DHCPv6 server interface global IP address unknown or undefined */
#define NX_DHCPV6_INVALID_INTERFACE_LL_ADDRESS          0xE96    /* DHCPv6 server interface link local address unknown or undefined */
#define NX_DHCPV6_INVALID_GLOBAL_INDEX                  0xE97    /* DHCPv6 global address index not set */
#define NX_DHCPV6_BAD_SERVER_DUID                       0xE98    /* Invalid or inappropriate server DUID (Should not be in some DHCPv6 message types).  */
#define NX_DHCPV6_MESSAGE_DUID_MISSING                  0xE99    /* Received a message type missing server or client DUID. */
#define NX_DHCPV6_INVALID_INTERFACE_INDEX               0xEBC    /* Specified interface does not exist for the DHCPv6 server instance  */
#define NX_DHCPV6_INVALID_IANA_TIME                     0xEA0    /* Client IA-NA option T1 vs T2 address lease time is invalid. */
#define NX_DHCPV6_IANA_OPTION_MISSING                   0xEA1    /* Received IA address option not belonging to an IA block */
#define NX_DHCPV6_NO_SERVER_DUID                        0xEA2    /* No server DUID detected; DHCPv6 server cannot start without one */
#define NX_DHCPV6_INVALID_IANA_DATA                     0xEA3    /* Client IA-NA option block has bad syntax or missing data */
#define NX_DHCPV6_INVALID_IA_DATA                       0xEA5    /* Server IA address option block has bad syntax or missing data */
#define NX_DHCPV6_INVALID_IA_TIME                       0xEA6    /* Server IA option preferred vs valid lease time is invalid. */
#define NX_DHCPV6_NO_ASSIGNABLE_ADDRESSES               0xEA7    /* Server created with no assignable addresses. */
#define NX_DHCPV6_ADDRESS_NOT_FOUND                     0xEA8    /* Client address not found in server lease table. */
#define NX_DHCPV6_OPTION_BLOCK_INCOMPLETE               0xEA9    /* Empty option block data; either zero length or zero option parsed. */    
#define NX_DHCPV6_INVALID_OPTION_DATA                   0xEAA    /* Server received option data with missing data or bad syntax */
#define NX_DHCPV6_ILLEGAL_MESSAGE_TYPE                  0xEB1    /* Received invalid message request e.g. RENEW (server should discard),  */
#define NX_DHCPV6_PROCESSING_ERROR                      0xEB5    /* Invalid Client packet with DHCPv6 format or packet size  */
#define NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD           0xEB6    /* Server DHCPv6 reply will not fit in packet pool packet buffer. */
#define NX_DHCPV6_INVALID_DATA_SIZE                     0xEB7    /* Attempting to parse too large a data object from DHCPv6 request. */
#define NX_DHCPV6_TABLE_FULL                            0xEC4    /* Server table (e.g. Client records or IP address leases) is full. */
#define NX_DHCPV6_INVALID_IP_ADDRESS                    0xECB    /* Invalid IP address e.g. null address received or obtained from client record. */
#define NX_DHCPV6_INVALID_DUID                          0xECC    /* Invalid DUID received from client or retrieved from memory. */
#define NX_DHCPV6_CLIENT_RECORD_NOT_FOUND               0xECF    /* Unable to find client in Client records table. */
#define NX_DHCPV6_TIMER_INTERNAL_ERROR                  0xED4    /* Problem setting or accessing real time clock server for DUID time field. */
#define NX_DHCPV6_INVALID_VENDOR_DATA                   0xED5    /* Vendor data invalid (ID is too long, null data etc). */


/* Define the DHCPv6 status codes.  */

#define NX_DHCPV6_STATUS_SUCCESS               0        /* Client request granted. */
#define NX_DHCPV6_STATUS_UNSPECIFIED           1        /* Failure, reason unspecified; */
#define NX_DHCPV6_STATUS_NO_ADDRS_AVAILABLE    2        /* Server has no addresses available to assign to  the IA(s) */
#define NX_DHCPV6_STATUS_NO_BINDING            3        /* Client record (binding) unavailable or not possible */
#define NX_DHCPV6_STATUS_NOT_ON_LINK           4        /* Address prefix not appropriate for client domain */
#define NX_DHCPV6_STATUS_USE_MULTICAST         5        /* Server informs client to use the All_DHCP_Relay_Agents_and_Servers address. */


/* Define the server messages for IANA status codes.  Note that these
   messages must be shorter than the NX_DHCPV6_STATUS_MESSAGE_MAX
   option defined for the client IANA status option. 
*/
#ifndef NX_DHCPV6_STATUS_MESSAGE_SUCCESS
#define NX_DHCPV6_STATUS_MESSAGE_SUCCESS                "IA OPTION GRANTED"
#endif

#ifndef NX_DHCPV6_STATUS_MESSAGE_UNSPECIFIED
#define NX_DHCPV6_STATUS_MESSAGE_UNSPECIFIED            "IA OPTION NOT GRANTED-FAILURE UNSPECIFIED"
#endif

#ifndef NX_DHCPV6_STATUS_MESSAGE_NO_ADDRS_AVAILABLE
#define NX_DHCPV6_STATUS_MESSAGE_NO_ADDRS_AVAILABLE     "IA OPTION NOT GRANTED-NO ADDRESSES AVAILABLE"
#endif


#ifndef NX_DHCPV6_STATUS_MESSAGE_NO_BINDING
#define NX_DHCPV6_STATUS_MESSAGE_NO_BINDING             "IA OPTION NOT GRANTED-INVALID CLIENT REQUEST"
#endif


#ifndef NX_DHCPV6_STATUS_MESSAGE_NOT_ON_LINK
#define NX_DHCPV6_STATUS_MESSAGE_NOT_ON_LINK            "IA OPTION NOT GRANTED-CLIENT NOT ON LINK"
#endif


#ifndef NX_DHCPV6_STATUS_MESSAGE_USE_MULTICAST
#define NX_DHCPV6_STATUS_MESSAGE_USE_MULTICAST          "IA OPTION NOT GRANTED-CLIENT MUST USE MULTICAST"
#endif


/* Define the wait option on receiving packets on the DHCPv6 server queue. This
   is on a receive notify callback, so there need not be any wait involved.  */

#ifndef NX_DHCPV6_PACKET_WAIT_OPTION
#define NX_DHCPV6_PACKET_WAIT_OPTION                   NX_IP_PERIODIC_RATE
#endif

/* Note the DHCPv6 server only supports the DNS server option in the current release. */



/* Define the DHCPv6 Server application stack size. This is more than enough
   for most DHCPv6 Server applications. */

#ifndef NX_DHCPV6_SERVER_THREAD_STACK_SIZE
#define NX_DHCPV6_SERVER_THREAD_STACK_SIZE             4096
#endif


/* Define the DHCPv6 Server thread priority.  */

#ifndef NX_DHCPV6_SERVER_THREAD_PRIORITY
#define NX_DHCPV6_SERVER_THREAD_PRIORITY               2
#endif


/* Define the timer interval in seconds for checking Client lease time expirations.  */

#ifndef NX_DHCPV6_IP_LEASE_TIMER_INTERVAL
#define NX_DHCPV6_IP_LEASE_TIMER_INTERVAL       (60)  
#endif


/* Define the timer interval in seconds for the session duration timer.  */

#ifndef NX_DHCPV6_SESSION_TIMER_INTERVAL
#define NX_DHCPV6_SESSION_TIMER_INTERVAL         (3)
#endif


/* Define the session time out in seconds. This is the timer for how long the server has not 
   received a client response. */

#ifndef NX_DHCPV6_SESSION_TIMEOUT
#define NX_DHCPV6_SESSION_TIMEOUT                (20)
#endif

/* Define the private ID of the vendor DUID tuple. This is a 32 bit word. */

#ifndef NX_DHCPV6_SERVER_DUID_VENDOR_PRIVATE_ID
#define NX_DHCPV6_SERVER_DUID_VENDOR_PRIVATE_ID   0x12345678
#endif


/* Define the size limit on the Vendor ID buffer. */

#ifndef NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_LENGTH                 
#define NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_LENGTH               48     
#endif


/* Define the vendor ID. The server must have this data to create its DUID required for sending
   responses to the DHCPv6 client. */

#ifndef NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_ID
#define NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_ID                  "abcdeffghijklmnopqrstuvwxyz"
#endif


/* Define parameters for the server DUID */

/* Define the server DUID type.  The most common are link layer, based on mac address, 
   and link layer-time which is based on mac address and current time.  */

#ifndef NX_DHCPV6_SERVER_DUID_TYPE
#define NX_DHCPV6_SERVER_DUID_TYPE                          NX_DHCPV6_DUID_TYPE_LINK_TIME
#endif


/* Default DUID hardware type to Ethernet */

#ifndef NX_DHCPV6_SERVER_HW_TYPE
#define NX_DHCPV6_SERVER_HW_TYPE                            NX_DHCPV6_SERVER_HARDWARE_TYPE_ETHERNET
#endif


/* Define the preference option pref-value. When multiple DHCPv6 servers exist, this
   enables a server to tell the client which preference to place on this server. The higher
   the value the greater the preference. A value of 255 received by a DHCPv6
   client will instruct the client to choose this DHPCv6 server immediately, rather
   than wait for the full round trip time out option specified in RFC 3315. RFC 3315
   sect 17.2.2 requires the server set this value to zero by default*/

#ifndef NX_DHCPV6_PREFERENCE_VALUE
#define NX_DHCPV6_PREFERENCE_VALUE              0
#endif


/* Define the maximum options to extract from a client message. This makes
   no assumptions about the packet size limitations which the host application
   must ensure can handle both the client request and the server reply. */

#ifndef NX_DHCPV6_MAX_OPTION_REQUEST_OPTIONS
#define NX_DHCPV6_MAX_OPTION_REQUEST_OPTIONS    6
#endif


/* Set the default T1 time, which is when the Client should begin renewing its IP address,
   for leased IP addresses in seconds. RFC 3315 recommends
   T1 is 0.5 the value of the preferred lifetime. */

#ifndef  NX_DHCPV6_DEFAULT_T1_TIME
#define  NX_DHCPV6_DEFAULT_T1_TIME              (2000)   
#endif


/* Set the default T2 time, which is when the Client should begin renewing its IP address, 
   assuming renewal attempts failed, for leased IP addresses in seconds. 
   T2 must be greater than T2 and less than the value of the preferred lifetime. */

#ifndef  NX_DHCPV6_DEFAULT_T2_TIME
#define  NX_DHCPV6_DEFAULT_T2_TIME              (3000)  
#endif


/* Set the default preferred lifetime for leased IP addresses in seconds, which is 
    when the Client IP address becomes deprecated. A value of zero in 
    this field indicates that the server leaves it to the 
   client discretion to renew its IP address lease. */

#ifndef  NX_DHCPV6_DEFAULT_PREFERRED_TIME
#define  NX_DHCPV6_DEFAULT_PREFERRED_TIME        (2 * NX_DHCPV6_DEFAULT_T1_TIME)
#endif


/* Set the default valid lifetime in seconds for leased IP addresses in secs, which is when 
   the Client IP address becomes obsolete. A value of zero in this field indicates 
   that the server leaves it to the client discretion to renew its IP address lease. 
   The valid time must be greater than the preferred time. */

#ifndef  NX_DHCPV6_DEFAULT_VALID_TIME
#define  NX_DHCPV6_DEFAULT_VALID_TIME             (2 * NX_DHCPV6_DEFAULT_PREFERRED_TIME)
#endif



/* Define maximum size for server messages in status option. */

#ifndef NX_DHCPV6_STATUS_MESSAGE_MAX      
#define NX_DHCPV6_STATUS_MESSAGE_MAX           100
#endif  


/* Define the max number of IPv6 address leases available to lease.  */

#ifndef NX_DHCPV6_MAX_LEASES                     
#define NX_DHCPV6_MAX_LEASES                   100     
#endif


/* Define the max number of clients the server can assign leases to at any given time. 
   These are clients either already leasing an IPv6 address or currently requesting one. 
   Note: there are more addresses than clients in the event the client declines an address
   or one assigned is already in use. The NetX DHCPv6 server currently only supports
   one IP address leased to each client. */

#ifndef NX_DHCPV6_MAX_CLIENTS                    
#define NX_DHCPV6_MAX_CLIENTS                   120     
#endif

/* Define the wait option in timer ticks for DHCPv6 server packet allocations.  */

#ifndef NX_DHCPV6_PACKET_TIME_OUT
#define NX_DHCPV6_PACKET_TIME_OUT               (3 * NX_DHCPV6_SERVER_TICKS_PER_SECOND)
#endif


/* Define the DHCPv6 packet size. Should be large enough to hold IP and UDP headers,
   plus DHCPv6 data runs about 200 - 300 bytes for a typical exchange. */

#ifndef NX_DHCPV6_PACKET_SIZE 
#define NX_DHCPV6_PACKET_SIZE                    500  
#endif


/* Define the DHCPv6 packet memory area. Packet pool size depends on client traffic and 
   available network bandwidth. */

#ifndef NX_DHCPV6_PACKET_POOL_SIZE 
#define NX_DHCPV6_PACKET_POOL_SIZE               (10 * NX_DHCPV6_PACKET_SIZE)
#endif


/* Define UDP socket type of service.  */

#ifndef NX_DHCPV6_TYPE_OF_SERVICE
#define NX_DHCPV6_TYPE_OF_SERVICE                NX_IP_NORMAL
#endif


/* Define the UDP socket fragment option. */

#ifndef NX_DHCPV6_FRAGMENT_OPTION
#define NX_DHCPV6_FRAGMENT_OPTION                NX_DONT_FRAGMENT
#endif  

/* Define the number of routers a UDP packet passes before it is discarded. */

#ifndef NX_DHCPV6_TIME_TO_LIVE
#define NX_DHCPV6_TIME_TO_LIVE                   0x80
#endif

/* Define the stored packets in the UDP server socket queue. */

#ifndef NX_DHCPV6_QUEUE_DEPTH
#define NX_DHCPV6_QUEUE_DEPTH                    5
#endif


/* Define the Identity Association Internet Address option structure  */
typedef struct NX_DHCPV6_SERVER_IA_ADDRESS_STRUCT
{

    USHORT          nx_op_code;                    /* IA internet address option code is 5 */
    USHORT          nx_option_length;              /* Length of the IA address option data = 24 not including length and op code field*/
    NXD_ADDRESS     nx_global_address;             /* Assigned Host IPv6 address */
    ULONG           nx_preferred_lifetime;         /* Server's preference for IPv6 address T1 life time for itself */
    ULONG           nx_valid_lifetime;             /* Server's assigned valid time for T2 for any server  */

} NX_DHCPV6_SERVER_IA_ADDRESS;

/* Define the Option status structure  */
typedef struct NX_DHCPV6_SERVER_IANA_STATUS_STRUCT
{

    USHORT          nx_op_code;                    /* IA address option code is 5 */
    USHORT          nx_option_length;              /* Length of the IA address option data = 24 not including length and op code field*/
    USHORT          nx_status_code;                /* Server status (reply) to client request */
    CHAR            nx_status_message[NX_DHCPV6_STATUS_MESSAGE_MAX];
                                                   /* Buffer containing server status messages */
} NX_DHCPV6_SERVER_IANA_STATUS;


/* Define the Preference Option structure  */
typedef struct NX_DHCPV6_SERVER_PREFERENCE_STRUCT
{

    USHORT          nx_op_code;                    /* IA address option code is 5 */
    USHORT          nx_option_length;              /* Length of the IA address option data = 24 not including length and op code field*/
    USHORT          nx_pref_value;                  /* Assigned Host IPv6 address */

} NX_DHCPV6_SERVER_PREFERENCE;


/* Define the Identity Association for Permanent ("Non Temporary" in RFC) address */

typedef struct NX_DHCPV6_SERVER_IA_NA_STRUCT
{

    USHORT              nx_op_code;             /* IA NA address option code is 3 */
    USHORT              nx_option_length;       /* 12 + length of variable length fields in IA_NA option . */
    ULONG               nx_IA_NA_id;            /* IANA identifier; must be unique among all client IANA's. Must be the same on restart per IANA */
    ULONG               nx_T1;                  /* Time client can extend time before address lifetime expires from the server it got it from; applies to all addresses in IA_NA. */
    ULONG               nx_T2;                  /* Same as T1 except this is when the client will request REBIND from another server. */

} NX_DHCPV6_SERVER_IA_NA;


/* Define DHCPv6 Unique Identifier (DUID); both Client and Server must send messages with their own DUID. */

typedef struct NX_DHCPV6_SVR_DUID_STRUCT
{

    USHORT            nx_op_code;                 /* Client DUID option code is 1; Server DUID code is 2  */
    USHORT            nx_option_length;           /* Option length = 14 not including length and op code field; */
    USHORT            nx_duid_type;               /* 3 main types: hw; hw + time; vendor assigned ID (not supported here); requires DUID be stored in non volatile storage */
    USHORT            nx_hardware_type;           /* Only if LL/LLT type. Hardware type specified by IANA/RFC 826 e.g. IEEE 802; network byte order */
    ULONG             nx_duid_time;               /* Only if LLT type. Time based on when DUID generated; network byte order. */
    ULONG             nx_duid_enterprise_number;  /* Only if vendor assigned (enterprise) DUID */
    UCHAR             nx_duid_private_identifier[NX_DHCPV6_SERVER_DUID_VENDOR_ASSIGNED_LENGTH];
    USHORT            nx_link_layer_address_msw;  /* Only if LL/LLT type. Pointer to Unique link layer address - most significant word (2 bytes)*/
    ULONG             nx_link_layer_address_lsw;  /* Only if LL/LLT type. Pointer to Unique link layer address - least significant word (4 bytes) */

} NX_DHCPV6_SVR_DUID;


/* Define the elapsed time option structure.  This contains the length of the Client Server session. */

typedef struct NX_DHCPV6_SERVER_ELAPSED_TIME_STRUCT
{

    USHORT            nx_op_code;                /* Elapsed time option code = 8 not including length and op code field. */
    USHORT            nx_option_length;              /* Length of time data = 2. */
    USHORT            nx_session_time;           /* Time of DHCP session e.g. first msg elapsed time is zero. */

} NX_DHCPV6_SERVER_ELAPSED_TIME;


/* Define the option request structure. This is how the Client requests information other than global IP address.  
   It can ask for domain name, DNS server, time zone, time server and other options. */

typedef struct NX_DHCPV6_SERVER_OPTIONREQUEST_STRUCT
{
    USHORT             nx_op_code;                /* Option Request code  = 6*/
    USHORT             nx_option_length;          /* Length in bytes of option data = 2 * number of requests */
    USHORT             nx_op_request[NX_DHCPV6_MAX_OPTION_REQUEST_OPTIONS];
                                                  /* List of option request options e.g. DNS server */

} NX_DHCPV6_SERVER_OPTIONREQUEST;

/* Define the Client DHCPv6 structure containing the DHCPv6 Client record (DHCPv6 status, server DUID etc).  */

typedef struct NX_DHCPV6_CLIENT_STRUCT 
{

    ULONG                   nx_dhcpv6_id;                               /* DHCPv6 Structure ID  */
    ULONG                   nx_dhcpv6_message_xid;                      /* Message transaction ID (3 bytes)*/
    UCHAR                   nx_dhcpv6_state;                            /* The current state of the DHCPv6 Client */
    UINT                    nx_dhcpv6_message_type;                     /* DHCPv6 message type most recently received from client. */
    UINT                    nx_dhcpv6_response_back_from_server;        /* Response server will send back to client based on previous client message */
    NX_DHCPV6_SVR_DUID      nx_dhcpv6_client_duid;                      /* Client DUID parsed from client DHCPv6 requests */
    NX_DHCPV6_SVR_DUID      nx_dhcpv6_server_duid;                      /* Server DUID parsed from client DHCPv6 requests */
    ULONG                   nx_dhcpv6_client_session_time;              /* Duration of client server session. Used to determine if client quit on the session. */
    UINT                    nx_dhcpv6_rapid_commit_status;              /* Client status on rapid commit (set to true if requests and server approves). */
    NX_DHCPV6_SERVER_ELAPSED_TIME
                            nx_dhcpv6_elapsed_time;                     /* Time duration of the current DHCP msg exchange between Client and Server. */
    NX_DHCPV6_SERVER_IA_NA  nx_dhcpv6_iana;                             /* Identity Association for non temp address - must be stored in non volatile memory */
    NX_DHCPV6_SERVER_IA_ADDRESS    
                            nx_dhcpv6_ia;                               /* Client internet address option */
    NX_DHCPV6_SERVER_OPTIONREQUEST
                            nx_dhcpv6_option_request;                   /* Set of request options in Solicit, Renew, Confirm or Rebind message types. */
    NX_DHCPV6_SERVER_IANA_STATUS   
                            nx_dhcpv6_iana_status;                      /* Status option set by the server for client IANA */
    NX_DHCPV6_SERVER_PREFERENCE    
                            nx_dhcpv6_preference;                       /* Preference option set by the server */
    ULONG                   nx_dhcpv6_IP_lease_time_accrued ;           /* Time remaining on the Client IP address lease. */
    NXD_ADDRESS             nx_dhcp_source_ip_address;                  /* Source IP of the client DHCP message. */
    NXD_ADDRESS             nx_dhcp_destination_ip_address;             /* Destination IP of the client DHCP message. */

} NX_DHCPV6_CLIENT;

/* Define the DHCPv6 server interface IP address table. */ 

typedef struct NX_DHCPV6_ADDRESS_LEASE_STRUCT
{

    NXD_ADDRESS             nx_dhcpv6_lease_IP_address;                       /* Address to assign */
    ULONG                   nx_dhcpv6_lease_T1_lifetime;                      /* T1 value set if assigned to client */
    ULONG                   nx_dhcpv6_lease_T2_lifetime;                      /* T2 value set if assigned to client */
    ULONG                   nx_dhcpv6_lease_valid_lifetime;                   /* valid lifetime set if assigned to client */
    ULONG                   nx_dhcpv6_lease_preferred_lifetime;               /* preferred lifetime set if assigned to client */
    NX_DHCPV6_CLIENT        *nx_dhcpv6_lease_assigned_to;                      /* Client DUID assigned the address */

}NX_DHCPV6_ADDRESS_LEASE;


/* Define the Server DHCPv6 structure containing the Client tables and assignable IP address tables.  */

typedef struct NX_DHCPV6_SERVER_STRUCT 
{
    ULONG                   nx_dhcpv6_id;                                   /* DHCPv6 Structure ID  */
    CHAR                    *nx_dhcpv6_server_name;                         /* DHCPv6 name supplied at create */ 
    NX_IP                   *nx_dhcpv6_ip_ptr;                              /* The associated IP pointer for this DHCPV6 instance */ 
    UINT                    nx_dhcpv6_server_interface_index;               /* Index indicating interface DHCPv6 requests accepted */
    UINT                    nx_dhcpv6_server_ga_address_index;              /* Global address index of the DHCPv6 server*/
    NX_DHCPV6_SVR_DUID       nx_dhcpv6_server_duid;                          /* DUID by which server identifies itself to clients */
    NX_DHCPV6_CLIENT        nx_dhcpv6_clients[NX_DHCPV6_MAX_CLIENTS];       /* list of clients leased an IPv6 address */
    NX_DHCPV6_ADDRESS_LEASE nx_dhcpv6_lease_list[NX_DHCPV6_MAX_LEASES];     /* List of IP addresses available for DHCPv6 Clients */
    UINT                    nx_dhcpv6_assignable_addresses;                 /* Number of assignable addresses the server starts with. */
    UINT                    nx_dhcpv6_server_multicast_only;                /* Indicate if the client should send requests using all servers multicast address */
    NX_UDP_SOCKET           nx_dhcpv6_server_socket;                        /* UDP socket for communicating with DHCPv6 clients */
    TX_MUTEX                nx_dhcpv6_server_mutex;                         /* Mutex protection of server control block */
    TX_TIMER                nx_dhcpv6_lease_timer;                          /* Timer for tracking IP lease expiration   */
    TX_TIMER                nx_dhcpv6_session_timer;                        /* Server session duration timer */ 
    TX_THREAD               nx_dhcpv6_server_thread;                        /* DHCPv6 Server processing thread */
    NX_PACKET_POOL          *nx_dhcpv6_packet_pool_ptr;                     /* Pointer to packet pool for sending DHCPV6 messages */
    TX_EVENT_FLAGS_GROUP    nx_dhcpv6_server_timer_events;                  /* Message queue for IP lease and session timer events. */
    UINT                    nx_dhcpv6_server_running;                       /* Status of dhcpv6 server; idle (NX_FALSE) or running (NX_TRUE) */
    NXD_ADDRESS             nx_dhcpv6_dns_ip_address;                     /* DHCP server DNS Server Address in message to DHCP Client  */

    /* Define a handler for receiving a DECLINE or RELEASE message. */
    VOID (*nx_dhcpv6_IP_address_declined_handler)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, UINT message_type);
    /* Define a handler for Option Request options. */
    VOID (*nx_dhcpv6_server_option_request_handler)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, UINT option_request, UCHAR *buffer_ptr, UINT *index);

    /* Define a extended handler for Option Request options. */
    VOID (*nx_dhcpv6_server_option_request_handler_extended)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, UINT option_request, UCHAR *buffer_ptr, UINT *index, UINT available_payload);

} NX_DHCPV6_SERVER;

#ifndef NX_DHCPV6_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map DHCP API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */


#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

                
#define nx_dhcpv6_set_server_duid               _nx_dhcpv6_set_server_duid
#define nx_dhcpv6_create_dns_address            _nx_dhcpv6_create_dns_address
#define nx_dhcpv6_create_ip_address_range       _nx_dhcpv6_create_ip_address_range
#define nx_dhcpv6_add_ip_address_lease          _nx_dhcpv6_add_ip_address_lease 
#define nx_dhcpv6_add_client_record             _nx_dhcpv6_add_client_record
#define nx_dhcpv6_retrieve_client_record        _nx_dhcpv6_retrieve_client_record
#define nx_dhcpv6_retrieve_ip_address_lease     _nx_dhcpv6_retrieve_ip_address_lease
#define nx_dhcpv6_reserve_ip_address_range      _nx_dhcpv6_reserve_ip_address_range
#define nx_dhcpv6_server_create                 _nx_dhcpv6_server_create
#define nx_dhcpv6_server_delete                 _nx_dhcpv6_server_delete
#define nx_dhcpv6_server_resume                 _nx_dhcpv6_server_resume
#define nx_dhcpv6_server_suspend                _nx_dhcpv6_server_suspend
#define nx_dhcpv6_server_start                  _nx_dhcpv6_server_start
#define nx_dhcpv6_server_interface_set          _nx_dhcpv6_server_interface_set
#define nx_dhcpv6_server_option_request_handler_set _nx_dhcpv6_server_option_request_handler_set

#else

/* Services with error checking.  */
                
#define nx_dhcpv6_set_server_duid               _nxe_dhcpv6_set_server_duid
#define nx_dhcpv6_create_dns_address            _nxe_dhcpv6_create_dns_address
#define nx_dhcpv6_create_ip_address_range       _nxe_dhcpv6_create_ip_address_range
#define nx_dhcpv6_add_ip_address_lease          _nxe_dhcpv6_add_ip_address_lease 
#define nx_dhcpv6_add_client_record             _nxe_dhcpv6_add_client_record
#define nx_dhcpv6_retrieve_client_record        _nxe_dhcpv6_retrieve_client_record
#define nx_dhcpv6_retrieve_ip_address_lease     _nxe_dhcpv6_retrieve_ip_address_lease
#define nx_dhcpv6_reserve_ip_address_range      _nxe_dhcpv6_reserve_ip_address_range
#define nx_dhcpv6_server_create                 _nxe_dhcpv6_server_create
#define nx_dhcpv6_server_delete                 _nxe_dhcpv6_server_delete
#define nx_dhcpv6_server_resume                 _nxe_dhcpv6_server_resume
#define nx_dhcpv6_server_suspend                _nxe_dhcpv6_server_suspend
#define nx_dhcpv6_server_start                  _nxe_dhcpv6_server_start
#define nx_dhcpv6_server_interface_set          _nxe_dhcpv6_server_interface_set
#define nx_dhcpv6_server_option_request_handler_set _nxe_dhcpv6_server_option_request_handler_set
#endif   /* NX_DISABLE_ERROR_CHECKING */


/* Define the prototypes accessible to the application software.  */
UINT        nx_dhcpv6_set_server_duid(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT duid_type, UINT hardware_type, ULONG mac_address_msw, ULONG mac_address_lsw, ULONG time);
UINT        nx_dhcpv6_create_dns_address(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NXD_ADDRESS *dns_ipv6_address);
UINT        nx_dhcpv6_create_ip_address_range(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NXD_ADDRESS *start_ipv6_address, NXD_ADDRESS *end_ipv6_address, UINT *addresses_added);
UINT        nx_dhcpv6_add_ip_address_lease(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, NXD_ADDRESS *lease_IP_address, ULONG T1, ULONG T2, ULONG valid_lifetime, ULONG preferred_lifetimeo);
UINT        nx_dhcpv6_add_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, ULONG message_xid, NXD_ADDRESS *client_address, UINT client_state, 
                                  ULONG IP_lease_time_accrued    , ULONG valid_lifetime, UINT duid_type,  UINT duid_hardware, ULONG physical_address_msw, 
                                  ULONG physical_address_lsw, ULONG duid_time, ULONG duid_vendor_number, UCHAR *duid_vendor_private, UINT duid_private_length);
UINT        nx_dhcpv6_retrieve_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, ULONG *message_xid, NXD_ADDRESS *client_address, UINT *client_state, 
                                  ULONG *IP_lease_time_accrued    , ULONG *valid_lifetime, UINT *duid_type,  UINT *duid_hardware, ULONG *physical_address_msw, 
                                  ULONG *physical_address_lsw, ULONG *duid_time, ULONG *duid_vendor_number, UCHAR *duid_vendor_private, UINT *duid_private_length);
UINT        nx_dhcpv6_retrieve_ip_address_lease(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, NXD_ADDRESS *lease_IP_address, ULONG *T1, ULONG *T2, ULONG *valid_lifetime, ULONG *preferred_lifetime);
UINT        nx_dhcpv6_reserve_ip_address_range(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NXD_ADDRESS *start_ipv6_address, NXD_ADDRESS *end_ipv6_address, UINT *addresses_reserved);
UINT        nx_dhcpv6_server_create(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_IP *ip_ptr, CHAR *name_ptr, NX_PACKET_POOL *packet_pool_ptr, VOID *stack_ptr, ULONG stack_size,
                              VOID (*dhcpv6_address_declined_handler)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, UINT message),
                              VOID (*dhcpv6_option_request_handler)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, UINT option_request, UCHAR *buffer_ptr, UINT *index));
UINT        nx_dhcpv6_server_delete(NX_DHCPV6_SERVER *dhcpv6_server_ptr);
UINT        nx_dhcpv6_server_resume(NX_DHCPV6_SERVER *dhcpv6_server_ptr);
UINT        nx_dhcpv6_server_start(NX_DHCPV6_SERVER *dhcpv6_server_ptr);
UINT        nx_dhcpv6_server_suspend(NX_DHCPV6_SERVER *dhcpv6_server_ptr);
UINT        nx_dhcpv6_server_interface_set(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT iface_index, UINT ga_address_index);
UINT        nx_dhcpv6_server_option_request_handler_set(NX_DHCPV6_SERVER *dhcpv6_server_ptr,
                                                        VOID (*dhcpv6_option_request_handler_extended)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, UINT option_request, 
                                                                                                       UCHAR *buffer_ptr, UINT *index, UINT available_payload));

#else

/* DHCP source code is being compiled, do not perform any API mapping.  */

UINT        _nxe_dhcpv6_set_server_duid(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT duid_type, UINT hardware_type, ULONG mac_address_msw, ULONG mac_address_lsw, ULONG time);
UINT        _nx_dhcpv6_set_server_duid(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT duid_type, UINT hardware_type, ULONG mac_address_msw, ULONG mac_address_lsw, ULONG time);
UINT        _nxe_dhcpv6_create_dns_address(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NXD_ADDRESS *dns_ipv6_address);
UINT        _nx_dhcpv6_create_dns_address(NX_DHCPV6_SERVER *dhcpv6_server_ptr,  NXD_ADDRESS *dns_ipv6_address);
UINT        _nxe_dhcpv6_create_ip_address_range(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NXD_ADDRESS *start_ipv6_address, NXD_ADDRESS *end_ipv6_address, UINT *addresses_added);
UINT        _nx_dhcpv6_create_ip_address_range(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NXD_ADDRESS *start_ipv6_address, NXD_ADDRESS *end_ipv6_address, UINT *addresses_added);
UINT        _nxe_dhcpv6_reserve_ip_address_range(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NXD_ADDRESS *start_ipv6_address, NXD_ADDRESS *end_ipv6_address, UINT *addresses_reserved);
UINT        _nx_dhcpv6_reserve_ip_address_range(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NXD_ADDRESS *start_ipv6_address, NXD_ADDRESS *end_ipv6_address, UINT *addresses_reserved);
UINT        _nxe_dhcpv6_add_ip_address_lease(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, NXD_ADDRESS *lease_IP_address, ULONG T1, ULONG T2, ULONG valid_lifetime, ULONG preferred_lifetime);
UINT        _nx_dhcpv6_add_ip_address_lease(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, NXD_ADDRESS *lease_IP_address, ULONG T1, ULONG T2, ULONG valid_lifetime, ULONG preferred_lifetime);
UINT        _nxe_dhcpv6_add_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, ULONG message_xid, NXD_ADDRESS *client_address, UINT client_state, 
                                  ULONG IP_lease_time_accrued    , ULONG valid_lifetime, UINT duid_type,  UINT duid_hardware, ULONG physical_address_msw, 
                                  ULONG physical_address_lsw, ULONG duid_time, ULONG duid_vendor_number, UCHAR *duid_vendor_private, UINT duid_private_length);
UINT        _nx_dhcpv6_add_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, ULONG message_xid, NXD_ADDRESS *client_address, UINT client_state, 
                                  ULONG IP_lease_time_accrued    , ULONG valid_lifetime, UINT duid_type,  UINT duid_hardware, ULONG physical_address_msw, 
                                  ULONG physical_address_lsw, ULONG duid_time, ULONG duid_vendor_number, UCHAR *duid_vendor_private, UINT duid_private_length);
UINT        _nxe_dhcpv6_retrieve_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, ULONG *message_xid, NXD_ADDRESS *client_address, UINT *client_state, 
                                  ULONG *IP_lease_time_accrued    , ULONG *valid_lifetime, UINT *duid_type,  UINT *duid_hardware, ULONG *physical_address_msw, 
                                  ULONG *physical_address_lsw, ULONG *duid_time, ULONG *duid_vendor_number, UCHAR *duid_vendor_private, UINT *duid_private_length);
UINT        _nx_dhcpv6_retrieve_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, ULONG *message_xid, NXD_ADDRESS *client_address, UINT *client_state, 
                                  ULONG *IP_lease_time_accrued    , ULONG *valid_lifetime, UINT *duid_type,  UINT *duid_hardware, ULONG *physical_address_msw, 
                                  ULONG *physical_address_lsw, ULONG *duid_time, ULONG *duid_vendor_number, UCHAR *duid_vendor_private, UINT *duid_private_length);
UINT        _nxe_dhcpv6_retrieve_ip_address_lease(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, NXD_ADDRESS *lease_IP_address, ULONG *T1, ULONG *T2, ULONG *valid_lifetime, ULONG *preferred_lifetime);
UINT        _nx_dhcpv6_retrieve_ip_address_lease(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT table_index, NXD_ADDRESS *lease_IP_address, ULONG *T1, ULONG *T2, ULONG *valid_lifetime, ULONG *preferred_lifetime);
UINT        _nxe_dhcpv6_server_create(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_IP *ip_ptr, CHAR *name_ptr, NX_PACKET_POOL *packet_pool_ptr, VOID *stack_ptr,  ULONG stack_size,
                             VOID (*dhcpv6_address_declined_handler)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, UINT message),
                             VOID (*dhcpv6_option_request_handler)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, UINT option_request, UCHAR *buffer_ptr, UINT *index));
UINT        _nx_dhcpv6_server_create(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_IP *ip_ptr, CHAR *name_ptr, NX_PACKET_POOL *packet_pool_ptr, VOID *stack_ptr,   ULONG stack_size,
                             VOID (*dhcpv6_address_declined_handler)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, UINT message),
                             VOID (*dhcpv6_option_request_handler)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, UINT option_request, UCHAR *buffer_ptr, UINT *index));
UINT        _nxe_dhcpv6_server_delete(NX_DHCPV6_SERVER *dhcpv6_server_ptr);
UINT        _nx_dhcpv6_server_delete(NX_DHCPV6_SERVER *dhcpv6_server_ptr);
UINT        _nxe_dhcpv6_server_resume(NX_DHCPV6_SERVER *dhcpv6_server_ptr);
UINT        _nx_dhcpv6_server_resume(NX_DHCPV6_SERVER *dhcpv6_server_ptr);
UINT        _nxe_dhcpv6_server_start(NX_DHCPV6_SERVER *dhcpv6_server_ptr);
UINT        _nx_dhcpv6_server_start(NX_DHCPV6_SERVER *dhcpv6_server_ptr);
UINT        _nxe_dhcpv6_server_suspend(NX_DHCPV6_SERVER *dhcpv6_server_ptr);
UINT        _nx_dhcpv6_server_suspend(NX_DHCPV6_SERVER *dhcpv6_server_ptr);
UINT        _nxe_dhcpv6_server_interface_set(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT iface_index, UINT ga_address_index);
UINT        _nx_dhcpv6_server_interface_set(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT iface_index, UINT ga_address_index);
UINT        _nxe_dhcpv6_server_option_request_handler_set(NX_DHCPV6_SERVER *dhcpv6_server_ptr,
                                                          VOID (*dhcpv6_option_request_handler_extended)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, UINT option_request,
                                                                                                         UCHAR *buffer_ptr, UINT *index, UINT available_payload));
UINT        _nx_dhcpv6_server_option_request_handler_set(NX_DHCPV6_SERVER *dhcpv6_server_ptr,
                                                         VOID (*dhcpv6_option_request_handler_extended)(struct NX_DHCPV6_SERVER_STRUCT *dhcpv6_server_ptr, UINT option_request, 
                                                                                                        UCHAR *buffer_ptr, UINT *index, UINT available_payload));


#endif  /* NX_DHCPV6_SOURCE_CODE */


UINT        _nx_dhcpv6_add_duid(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_SVR_DUID *dhcpv6_duid_ptr, UCHAR *buffer_ptr, UINT *index, UINT duid_type);
UINT        _nx_dhcpv6_add_ia(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_SERVER_IA_ADDRESS *dhcpv6_ia_ptr, UCHAR *buffer_ptr, UINT *index);
UINT        _nx_dhcpv6_server_add_iana(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr,  UCHAR *buffer_ptr, UINT *index);
UINT        _nx_dhcpv6_add_iana_status(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_SERVER_IANA_STATUS *iana_status_ptr, UCHAR *buffer_ptr, UINT *index);
UINT        _nx_dhcpv6_add_option_requests(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, UCHAR *buffer_ptr, UINT *index);
UINT        _nx_dhcpv6_add_preference(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, UCHAR *buffer_ptr, UINT *index);
UINT        _nx_dhcpv6_assign_ip_address(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, NX_DHCPV6_ADDRESS_LEASE **interface_address_ptr);
UINT        _nx_dhcpv6_check_duids_same(NX_DHCPV6_SVR_DUID *dhcpv6_duid1_ptr, NX_DHCPV6_SVR_DUID *dhcpv6_duid2_ptr, UINT *matching);
UINT        _nx_dhcpv6_clear_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr);
UINT        _nx_dhcpv6_create_server_duid(NX_DHCPV6_SERVER *dhcpv6_server_ptr, UINT duid_type, UINT hardware_type, ULONG mac_address_msw, ULONG mac_address_lsw);
UINT        _nx_dhcpv6_find_client_record_by_duid(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_SVR_DUID *duid_ptr, UINT *record_index, UINT add_on, ULONG message_xid, UINT *matching);
UINT        _nx_dhcpv6_find_ip_address(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr, NX_DHCPV6_ADDRESS_LEASE **interface_address_ptr);
UINT        _nx_dhcpv6_server_extract_packet_information(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT **dhcpv6_client_ptr, NX_PACKET *packet_ptr, 
                                                         UINT iface_index, NXD_ADDRESS source_address, NXD_ADDRESS destination_address);
UINT        _nx_dhcpv6_listen_for_messages(NX_DHCPV6_SERVER *dhcpv6_ptr);
UINT        _nx_dhcpv6_prepare_iana_status(NX_DHCPV6_SERVER_IANA_STATUS *dhcpv6_status_ptr, UINT flag);
UINT        _nx_dhcpv6_process_duid(NX_DHCPV6_SVR_DUID *duid_ptr, ULONG option_code, UINT option_length, UCHAR *option_data);
UINT        _nx_dhcpv6_process_elapsed_time(NX_DHCPV6_CLIENT *dhcpv6_client_ptr, ULONG option_code, UINT option_length, UCHAR *option_data);
UINT        _nx_dhcpv6_server_process_ia(NX_DHCPV6_CLIENT *dhcpv6_client_ptr, ULONG option_code, UINT option_length, UCHAR *option_data, UINT process_ia);
UINT        _nx_dhcpv6_server_process_iana(NX_DHCPV6_CLIENT *dhcpv6_client_ptr, ULONG option_code, UINT option_length, UCHAR *option_data);
UINT        _nx_dhcpv6_process_option_request(NX_DHCPV6_CLIENT *client_ptr, ULONG option_code, UINT option_length, UCHAR *option_data);
UINT        _nx_dhcpv6_send_response_to_client(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr);
VOID        _nx_dhcpv6_server_lease_timeout_entry(ULONG dhcpv6_server_ptr_value);
VOID        _nx_dhcpv6_server_thread_entry(ULONG ip_instance);
VOID        _nx_dhcpv6_server_socket_receive_notify(NX_UDP_SOCKET *socket_ptr);
VOID        _nx_dhcpv6_server_session_timeout_entry(ULONG dhcpv6_server_ptr_value);
UINT        _nx_dhcpv6_update_client_record(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *from_client_ptr, NX_DHCPV6_CLIENT *to_client_ptr);
UINT        _nx_dhcpv6_validate_client_message(NX_DHCPV6_SERVER *dhcpv6_server_ptr, NX_DHCPV6_CLIENT *dhcpv6_client_ptr);
UINT        _nx_dhcpv6_server_utility_get_block_option_length(UCHAR *buffer_ptr, ULONG *option, ULONG *length);
UINT        _nx_dhcpv6_server_utility_get_data(UCHAR *data, UINT size, ULONG *value);
INT         _nx_dhcpv6_server_utility_time_randomize(void);

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif /* NX_DHCPV6_SERVER_H */


