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
/**   Dynamic Host Configuration Protocol (DHCP)                          */ 
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/ 
/*                                                                        */ 
/*  APPLICATION INTERFACE DEFINITION                       RELEASE        */  
/*                                                                        */   
/*    nxd_dhcp_server.h                                   PORTABLE C      */ 
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX Dynamic Host Configuration Protocol      */ 
/*    (DHCP) server component, including all data types and external      */ 
/*    references. It is assumed that nx_api.h and nx_port.h have already  */ 
/*    been   included.                                                    */
/*                                                                        */ 
/*  RELEASE HISTORY                                                       */ 
/*                                                                        */ 
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            modified the type of        */
/*                                            nx_dhcp_user_options,       */
/*                                            improved buffer length      */
/*                                            verification,               */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), included */
/*                                            necessary header file,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/

#ifndef  NXD_DHCP_SERVER_H
#define  NXD_DHCP_SERVER_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"

/* Define the DHCP Server ID that is used to mark the DHCP Server structure as created.  */

#define NX_DHCP_SERVER_ID                       0x44484360UL
                                                                    

/* Define the DHCP stack size.  */

#ifndef NX_DHCP_SERVER_THREAD_STACK_SIZE
#define NX_DHCP_SERVER_THREAD_STACK_SIZE        1024  
#endif


/* Define the DHCP stack priority.  */

#ifndef NX_DHCP_SERVER_THREAD_PRIORITY
#define NX_DHCP_SERVER_THREAD_PRIORITY          2
#endif


/* Define the DHCP packet allocation timeout in timer ticks.  */

#ifndef NX_DHCP_PACKET_ALLOCATE_TIMEOUT
#define NX_DHCP_PACKET_ALLOCATE_TIMEOUT         NX_IP_PERIODIC_RATE
#endif


/* Define UDP socket create options.  */

#ifndef NX_DHCP_TYPE_OF_SERVICE
#define NX_DHCP_TYPE_OF_SERVICE                 NX_IP_NORMAL
#endif

#ifndef NX_DHCP_FRAGMENT_OPTION
#define NX_DHCP_FRAGMENT_OPTION                 NX_DONT_FRAGMENT
#endif  

#ifndef NX_DHCP_TIME_TO_LIVE
#define NX_DHCP_TIME_TO_LIVE                    0x80
#endif

#ifndef NX_DHCP_QUEUE_DEPTH
#define NX_DHCP_QUEUE_DEPTH                     5      
#endif


/* Define the server interface subnet mask (DHCP Client standard option). */

#ifndef NX_DHCP_SUBNET_MASK
#define NX_DHCP_SUBNET_MASK                     0xFFFFFF00UL
#endif


                      
/* Define the client's host name buffer size. */

#ifndef NX_DHCP_CLIENT_HOSTNAME_MAX
#define NX_DHCP_CLIENT_HOSTNAME_MAX             32
#endif


/* Define DHCP server's name and buffer size. */
#ifndef NX_DHCP_SERVER_HOSTNAME_MAX
#define NX_DHCP_SERVER_HOSTNAME_MAX             32
#endif

#ifndef NX_DHCP_SERVER_NAME
#define NX_DHCP_SERVER_NAME                     "NetX DHCP Server" 
#endif



/* Define an interval in seconds for the session timer to check the time remaining on 
   all the active clients in the server database. */

#ifndef NX_DHCP_FAST_PERIODIC_TIME_INTERVAL
#define NX_DHCP_FAST_PERIODIC_TIME_INTERVAL     10
#endif

/* Set a length of time in seconds in which server can expect a client response during
   Client DHCP session. Once a client is BOUND, the session timeout is set
   to the IP address lease time. Note the maximum wait time between Client retransmissions
   is 64 seconds, so this timeout should be in that neighborhood. Must be greater than
   the fast periodic time interva. */

#ifndef NX_DHCP_CLIENT_SESSION_TIMEOUT
#define NX_DHCP_CLIENT_SESSION_TIMEOUT          (10 * NX_DHCP_FAST_PERIODIC_TIME_INTERVAL)
#endif


/* Define an interval in seconds for the IP lease timer to check the time remaining on 
   all the assigned IP addresses in the server database. */

#ifndef NX_DHCP_SLOW_PERIODIC_TIME_INTERVAL
#define NX_DHCP_SLOW_PERIODIC_TIME_INTERVAL     1000
#endif


/* Set the default lease time in seconds on the IP address the server 
   will assign to the Client. */

#ifndef  NX_DHCP_DEFAULT_LEASE_TIME
#define  NX_DHCP_DEFAULT_LEASE_TIME            (10 * NX_DHCP_SLOW_PERIODIC_TIME_INTERVAL)
#endif



/* Set a size of the Server option list requested by the Client. Since the NetX DHCP Server
   does not as yet support the complete set of options in DHCP, this number can be optimized
   down to a smaller size that the largest possible number of options a Client could request. 
*/

#ifndef NX_DHCP_CLIENT_OPTIONS_MAX
#define NX_DHCP_CLIENT_OPTIONS_MAX             12
#endif


/* There are optional and required elements of the server option list.
   Use the required plus the user configurable option list for the 
   server list to respond back to the client. 
*/

/* At the very least, a DHCP server should supply the 
   Client's subnet mask, router (default gateway) address and dns server. This list must be 
   space separated. The combined required and optional server options
   should be less than NX_DHCP_CLIENT_OPTIONS_MAX.
*/

#ifndef NX_DHCP_OPTIONAL_SERVER_OPTION_LIST
#define NX_DHCP_OPTIONAL_SERVER_OPTION_LIST  "1 3 6"
#endif

/* Set the number of default options in the optional server list above. */
#ifndef NX_DHCP_OPTIONAL_SERVER_OPTION_SIZE  
#define NX_DHCP_OPTIONAL_SERVER_OPTION_SIZE   3
#endif


/* The NetX DHCP Server includes its server identifier, message type in all messages. */
#define NX_DHCP_REQUIRED_SERVER_OPTION_LIST  "53 54"  
#define NX_DHCP_REQUIRED_SERVER_OPTION_SIZE   2

/* Compute the total option  list size. */
#define NX_DHCP_SERVER_OPTION_LIST_SIZE      (NX_DHCP_REQUIRED_SERVER_OPTION_SIZE + NX_DHCP_OPTIONAL_SERVER_OPTION_SIZE)

/* Combine the actual lists maintaining a space between all options. */    
#define NX_DHCP_SERVER_OPTION_LIST           NX_DHCP_REQUIRED_SERVER_OPTION_LIST \
                                             " "                                 \
                                             NX_DHCP_OPTIONAL_SERVER_OPTION_LIST


/* Define the maximum size of each IP address list.  Each interface the server assigns
   IP addresses for will be limited to this size. */

/* Define the max size of an IP addresses list (applies to each interfaces). */

#ifndef NX_DHCP_IP_ADDRESS_MAX_LIST_SIZE
#define NX_DHCP_IP_ADDRESS_MAX_LIST_SIZE      20        
#endif

/* Define the size of client record table (All clients e.g.on all interfaces). */

#ifndef NX_DHCP_CLIENT_RECORD_TABLE_SIZE
#define NX_DHCP_CLIENT_RECORD_TABLE_SIZE      50 
#endif

    /* END OF CONFIGURABLE OPTIONS */


/* Define the size of the BOOT buffer. This should be large enough for all the
   required DHCP fields plus the minimum requirement of 312 bytes of option data
   stated in RFC 2131; 2. Protocol Summary. */
#ifndef NX_BOOT_BUFFER_SIZE
#define NX_BOOT_BUFFER_SIZE             548
#endif

/* Define the size of the DHCP header as per RFC 213 Section Protocol Summery, page 10.  */

#ifndef NX_DHCP_HEADER_SIZE
#define NX_DHCP_HEADER_SIZE             236
#endif


/* 
   Define the packet payload size, keeping in mind the DHCP Server must support
   at least a 548 byte DHCP Client message as per RFC 2131 and allow room for Ethernet, UDP and 
   IP headers and 4 byte alignment. 
*/
#ifndef NX_DHCP_MINIMUM_PACKET_PAYLOAD
#define NX_DHCP_MINIMUM_PACKET_PAYLOAD          (NX_BOOT_BUFFER_SIZE + NX_PHYSICAL_HEADER +  sizeof(NX_IPV4_HEADER) + sizeof(NX_UDP_HEADER))
#endif /* NX_DHCP_MINIMUM_PACKET_PAYLOAD */


/* Define the DHCP Message Area Offsets.  The DHCP message format is identical to that of BootP, except
   for the Vendor options that start at the offset specified by NX_DHCP_OFFSET_OPTIONS.  */

#define NX_DHCP_OFFSET_OP              0       /* 1 BootP Operation 1=req, 2=reply                         */
#define NX_DHCP_OFFSET_HTYPE           1       /* 1 Hardware type 1 = Ethernet                             */
#define NX_DHCP_OFFSET_HLEN            2       /* 1 Hardware address length, 6 for Ethernet                */
#define NX_DHCP_OFFSET_HOPS            3       /* 1 Number of hops, usually 0                              */
#define NX_DHCP_OFFSET_XID             4       /* 4 Transaction ID, pseudo random number                   */
#define NX_DHCP_OFFSET_SECS            8       /* 2 Seconds since boot                                     */
#define NX_DHCP_OFFSET_FLAGS           10      /* 2 Flags, 0x80 = Broadcast response, 0 = unicast response */
#define NX_DHCP_OFFSET_CLIENT_IP       12      /* 4 Initial client IP, used as dest for unicast response   */
#define NX_DHCP_OFFSET_YOUR_IP         16      /* 4 Assigned IP, initialized to 0.0.0.0                    */
#define NX_DHCP_OFFSET_SERVER_IP       20      /* 4 Server IP, usually initialized to 0.0.0.0              */
#define NX_DHCP_OFFSET_RELAY_IP        24      /* 4 Relay IP, usually 0.0.0.0, only for DHCP               */
#define NX_DHCP_OFFSET_CLIENT_HW       28      /* 16 Client hardware address                               */
#define NX_DHCP_OFFSET_SERVER_NM       44      /* 64 Server name, nulls if unused                          */
#define NX_DHCP_OFFSET_BOOT_FILE       108     /* 128 Boot file name, null if unused                       */
#define NX_DHCP_OFFSET_VENDOR          236     /* 64 Vendor options, set first 4 bytes to a magic number   */
#define NX_DHCP_OFFSET_OPTIONS         240     /* First variable vendor option                             */
#define NX_DHCP_OFFSET_END             300     /* End of BOOTP buffer                                      */
#define NX_DHCP_SERVER_OPTION_ADDRESS_SIZE     4

/* Define the DHCP Specific Vendor Extensions. */

#define NX_DHCP_SERVER_OPTION_PAD              0
#define NX_DHCP_SERVER_OPTION_PAD_SIZE         0
#define NX_DHCP_SERVER_OPTION_SUBNET_MASK      1
#define NX_DHCP_SERVER_OPTION_SUBNET_MASK_SIZE NX_DHCP_SERVER_OPTION_ADDRESS_SIZE
#define NX_DHCP_SERVER_OPTION_TIME_OFFSET      2
#define NX_DHCP_SERVER_OPTION_TIME_OFFSET_SIZE 4
#define NX_DHCP_SERVER_OPTION_ROUTER           3
#define NX_DHCP_SERVER_OPTION_ROUTER_SIZE      NX_DHCP_SERVER_OPTION_ADDRESS_SIZE
#define NX_DHCP_SERVER_OPTION_TIMESVR          4
#define NX_DHCP_SERVER_OPTION_DNS_SVR          6
#define NX_DHCP_SERVER_OPTION_HOST_NAME        12
#define NX_DHCP_SERVER_OPTION_DNS_NAME         15
#define NX_DHCP_SERVER_OPTION_NTP_SVR          42
#define NX_DHCP_SERVER_OPTION_VENDOR_OPTIONS   43
#define NX_DHCP_SERVER_OPTION_DHCP_IP_REQ      50
#define NX_DHCP_SERVER_OPTION_DHCP_IP_REQ_SIZE NX_DHCP_SERVER_OPTION_ADDRESS_SIZE
#define NX_DHCP_SERVER_OPTION_DHCP_LEASE       51
#define NX_DHCP_SERVER_OPTION_DHCP_LEASE_SIZE  4
#define NX_DHCP_SERVER_OPTION_DHCP_TYPE        53
#define NX_DHCP_SERVER_OPTION_DHCP_TYPE_SIZE   1
#define NX_DHCP_SERVER_OPTION_DHCP_SERVER_ID   54
#define NX_DHCP_SERVER_OPTION_DHCP_SERVER_SIZE NX_DHCP_SERVER_OPTION_ADDRESS_SIZE
#define NX_DHCP_SERVER_OPTION_DHCP_PARAMETERS  55
#define NX_DHCP_SERVER_OPTION_DHCP_MESSAGE     56
#define NX_DHCP_SERVER_OPTION_RENEWAL          58
#define NX_DHCP_SERVER_OPTION_RENEWAL_SIZE     4
#define NX_DHCP_SERVER_OPTION_REBIND           59
#define NX_DHCP_SERVER_OPTION_REBIND_SIZE      4
#define NX_DHCP_SERVER_OPTION_CLIENT_ID        61
#define NX_DHCP_SERVER_OPTION_CLIENT_ID_SIZE   7  
#define NX_DHCP_SERVER_OPTION_FDQN             81
#define NX_DHCP_SERVER_OPTION_FDQN_FLAG_N      8
#define NX_DHCP_SERVER_OPTION_FDQN_FLAG_E      4
#define NX_DHCP_SERVER_OPTION_FDQN_FLAG_O      2
#define NX_DHCP_SERVER_OPTION_FDQN_FLAG_S      1
#define NX_DHCP_SERVER_OPTION_END              255
#define NX_DHCP_SERVER_OPTION_END_SIZE         0

#define NX_DHCP_MINIMUM_PACKET          (NX_DHCP_OFFSET_END + NX_UDP_PACKET+ 100)


/* Define various BootP/DHCP constants.  */

#define NX_DHCP_SERVER_UDP_PORT         67
#define NX_DHCP_SERVER_TCP_PORT         67
#define NX_DHCP_CLIENT_UDP_PORT         68
#define NX_DHCP_CLIENT_TCP_PORT         68

#define NX_DHCP_OP_REQUEST             1
#define NX_DHCP_OP_REPLY               2
#define NX_DHCP_FLAGS_BROADCAST        0x80 
#define NX_DHCP_FLAGS_UNICAST          0x00
#define NX_DHCP_MAGIC_COOKIE           IP_ADDRESS(99, 130, 83, 99)
#define NX_DHCP_NO_ADDRESS             IP_ADDRESS(0, 0, 0, 0)
#define NX_DHCP_BC_ADDRESS             IP_ADDRESS(255, 255, 255, 255)
#define NX_AUTO_IP_ADDRESS             IP_ADDRESS(169, 254, 0, 0) 
#define NX_AUTO_IP_ADDRESS_MASK        0xFFFF0000UL
#define NX_DHCP_SERVER_INFINITE_LEASE  0xFFFFFFFFUL


/* Define the DHCP Message Types.  */

#define NX_DHCP_TYPE_DHCPDISCOVER       1
#define NX_DHCP_TYPE_DHCPOFFER          2
#define NX_DHCP_TYPE_DHCPREQUEST        3
#define NX_DHCP_TYPE_DHCPDECLINE        4
#define NX_DHCP_TYPE_DHCPACK            5
#define NX_DHCP_TYPE_DHCPNACK           6
#define NX_DHCP_TYPE_DHCPRELEASE        7
#define NX_DHCP_TYPE_DHCPINFORM         8
#define NX_DHCP_TYPE_DHCPFORCERENEW     9
#define NX_DHCP_TYPE_DHCPSILENT         10      


/* Define the states of the DHCP state machine. */

#define NX_DHCP_STATE_BOOT              1       /* Started with a previous address                          */
#define NX_DHCP_STATE_INIT              2       /* Started with no previous address                         */
#define NX_DHCP_STATE_SELECTING         3       /* Waiting to identify a DHCP server                        */
#define NX_DHCP_STATE_REQUESTING        4       /* Address requested, waiting for the Ack                   */
#define NX_DHCP_STATE_BOUND             5       /* Address established, no time outs                        */
#define NX_DHCP_STATE_RENEWING          6       /* Address established, renewal time out                    */
#define NX_DHCP_STATE_REBINDING         7       /* Address established, renewal and rebind time out         */
#define NX_DHCP_STATE_FORCERENEW        8       /* Address established, force renewal                       */


/* Define error codes from DHCP Server operation.  */
      
#define NX_DHCP_SERVER_ALREADY_STARTED          0x90    /* DHCP Server already started      */
#define NX_DHCP_SERVER_NOT_STARTED              0x91    /* DHCP Server not started when stop was issued   */
#define NX_DHCP_PARAMETER_ERROR                 0x92    /* Invalid non pointer input   */
#define NX_DHCP_INADEQUATE_PACKET_POOL_PAYLOAD  0x93    /* DHCP Server packet pool has inadequate packet payload for DHCP messages. */
#define NX_DHCP_BAD_OPTION_LIST_ERROR           0x94    /* Server default option list has invalid characters. */
#define NX_DHCP_INTERNAL_OPTION_PARSE_ERROR     0x95    /* Internal error parsing options from string to ULONG. */
#define NX_DHCP_NO_SERVER_OPTION_LIST           0x96    /* Server default option list is empty when server is configured to use it. */
#define NX_DHCP_IMPROPERLY_TERMINATED_OPTION    0x97    /* Improperly formatted client option data e.g missing terminating symbol 0xFF. */
#define NX_DHCP_ADD_OPTION_ERROR                0x98    /* Unable to add option to buffer of server DHCP response packet */
#define NX_DHCP_INVALID_IP_ADDRESS_LIST         0x99    /* Invalid start or end IP address list parameter for creating assignable IP address list.   */
#define NX_DHCP_NO_AVAILABLE_IP_ADDRESSES       0x9A    /* Server has no available IP addresses for DHCP Clients  */
#define NX_DHCP_INVALID_IP_ADDRESS              0x9B    /* Invalid IP address e.g. null address received or obtained from client record. */
#define NX_DHCP_IP_ADDRESS_NOT_FOUND            0x9C    /* No match found for IP address search */
#define NX_DHCP_IP_ADDRESS_ASSIGNED_TO_OTHER    0x9D    /* IP address not assigned to the current client */
#define NX_DHCP_INVALID_UPDATE_ADDRESS_CMD      0x9E    /* Unable to ascertain assigned ip address status (e.g. assign, mark available etc) */
#define NX_DHCP_CLIENT_RECORD_NOT_FOUND         0x9F    /* Unable to find client in Client records table. */
#define NX_DHCP_CLIENT_TABLE_FULL               0xA0    /* Server unable to add record to client table; table is full. */
#define NX_DHCP_SERVER_BAD_INTERFACE_INDEX      0xA1    /* Interface index exceeds the NX_MAX_PHYSICAL_INTERFACES limit.  */
#define NX_DHCP_INVALID_HW_ADDRESS              0xA2    /* Invalid hardware address e.g. null or exceeding 6 bytes. */
#define NX_DHCP_INVALID_NETWORK_PARAMETERS      0xA3    /* Invalid network parameters (subnet, dns server, default gateway/router). */
#define NX_DHCP_MALFORMED_DHCP_PACKET           0xA4    /* DHCP packet was malformed. */


/* Define DHCP Server event flags.  These events are processed by the DHCP Server thread. */
#define NX_DHCP_SERVER_ALL_EVENTS               0xFFFFFFFFUL    /* All DHCP Server event flags. */
#define NX_DHCP_SERVER_RECEIVE_EVENT            0x00000001UL    /* Packet received event.       */
#define NX_DHCP_SERVER_FAST_PERIODIC_EVENT      0x00000002UL    /* Fast periodic timer event.   */
#define NX_DHCP_SERVER_SLOW_PERIODIC_EVENT      0x00000004UL    /* Slow periodic timer event.   */


/* Define Assignable IP status 'commands'. */

#define NX_DHCP_ADDRESS_STATUS_NO_ACTION        0x01    /* No change in Client address. */
#define NX_DHCP_ADDRESS_STATUS_SERVER_ASSIGN    0x02    /* Server will assign IP address to client. */
#define NX_DHCP_ADDRESS_STATUS_ASSIGNED_EXT     0x03    /* Client assigned IP address elesewhere, mark client as owner and IP as not available. */
#define NX_DHCP_ADDRESS_STATUS_MARK_AVAILABLE   0x04    /* Mark client's IP address as available and clear owner field. */
#define NX_DHCP_ADDRESS_STATUS_ASSIGNED_OTHER   0x05    /* Mark client's IP address as assigned already (owner unknown). */

/* Define types of options to load in server response to DHCP client. */

#define NX_DHCP_OPTIONS_FOR_ALL_REPLIES         0x01
#define NX_DHCP_OPTIONS_FOR_REPLY_TO_OFFER      0x02
#define NX_DHCP_OPTIONS_FOR_REPLY_TO_REQUEST    0x03
#define NX_DHCP_OPTIONS_FOR_REPLY_TO_INFORM     0x04
#define NX_DHCP_OPTIONS_FOR_GENERIC_ACK         0x05
#define NX_DHCP_OPTIONS_REQUESTED_BY_CLIENT     0x06

/* Define the DHCP structure that contains DHCP client information during DHCP session.  Note
   this is not the same control block as the NX_DHCP_STRUCT in nx_dhcp.h for the NetX DHCP Client
   package.  */
 
typedef struct NX_DHCP_CLIENT_STRUCT 
{

    UINT            nx_dhcp_client_state;        /* Client DHCP state: e.g. INIT, BOOT, SELECTING, RENEWING etc */
    UCHAR           nx_dhcp_message_type;        /* DHCP message type (DISCOVER, REQUEST etc) received from Client. */
    CHAR            nx_dhcp_client_name[NX_DHCP_CLIENT_HOSTNAME_MAX];    
                                                 /* DHCP Client host name buffer. */
    ULONG           nx_dhcp_xid;                 /* Transaction ID for client DHCP session   */
    ULONG           nx_dhcp_source_ip_address;   /* Source IP of the client DHCP message. */
    ULONG           nx_dhcp_destination_ip_address;   
                                                 /* Destination IP of the client DHCP message. */
    ULONG           nx_dhcp_clientip_address;    /* "Client IP address" (sometimes called "ci-addr" field). */
    ULONG           nx_dhcp_your_ip_address;     /* "Your IP address" field in client DHCP message. */ 
    ULONG           nx_dhcp_requested_ip_address;/* IP address requested in client message option. */
    ULONG           nx_dhcp_requested_lease_time;/* IP address lease time requested in client message option. */
    ULONG           nx_dhcp_assigned_ip_address; /* IP address the Server offers/assigns the client. */
    UINT            nx_dhcp_client_iface_index;  /* Index into the server interface table matching the DHCP client's packet interface. */
    ULONG           nx_dhcp_clientrec_server_ip; /* Next Server IP Address (may be another DHCP server) for advanced DHCP features */ 
    ULONG           nx_dhcp_server_id;           /* Requested/assigned Server ID (usually set to DHCP server IP address) */ 
    ULONG           nx_dhcp_router_ip_address;   /* Requested/assigned router on DHCP Client network. */
    ULONG           nx_dhcp_dns_ip_address;      /* Requested/assigned DNS IP address; usually set to zero. */
    ULONG           nx_dhcp_relay_ip_address;    /* Requested/assigned Relay IP address; usually set to zero. */
    ULONG           nx_dhcp_subnet_mask;         /* Requested/assigned network mask */
    ULONG           nx_dhcp_client_mac_msw;      /* Client MAC address high bits */
    ULONG           nx_dhcp_client_mac_lsw;      /* Client MAC address low bits */
    UINT            nx_dhcp_client_hwlen;        /* Length of hardware address. */
    UINT            nx_dhcp_client_hwtype;       /* Client interface hardware type e.g. Ethernet. */
    ULONG           nx_dhcp_broadcast_flag_set;  /* Parse broadcast flags from DHCP messages. */
    UINT            nx_dhcp_client_option_count; /* Number of user options in client request */ 
    UCHAR           nx_dhcp_user_options[NX_DHCP_CLIENT_OPTIONS_MAX];   
    ULONG           nx_dhcp_session_timeout;     /* Time out on waiting for client's next response */
    UINT            nx_dhcp_response_type_to_client; 
                                                 /* DHCP code for response to send back to client. */

} NX_DHCP_CLIENT;

/* Define the interface address structure which the server will use to assign IP addresses
   in a previously specified network interface. */
typedef struct NX_DHCP_INTERFACE_IP_ADDRESS_STRUCT
{
    ULONG           nx_assignable_ip_address;       /* IP address available to assign to DHCP Client. */
    UINT            assigned;                       /* IP address status e.g if currently assigned to a client host. */
    UINT            lease_time;                     /* Lease duration in secs. */
    UINT            owner_hwtype;                   /* Hardware type.  */
    UINT            owner_mac_msw;                  /* MAC address high bits.  */
    UINT            owner_mac_lsw;                  /* MAC address low bits.  */
} NX_DHCP_INTERFACE_IP_ADDRESS;


typedef struct NX_DHCP_INTERFACE_TABLE_STRUCT
{
    NX_DHCP_INTERFACE_IP_ADDRESS
                    nx_dhcp_ip_address_list[NX_DHCP_IP_ADDRESS_MAX_LIST_SIZE];    
                                                    /* IP address available to assign to DHCP Client. */
    NX_INTERFACE    *nx_dhcp_incoming_interface;    /* Pointer to DHCP Server interface. */
    ULONG           nx_dhcp_server_ip_address;      /* DHCP Server's IP address for this interface. */
    ULONG           nx_dhcp_dns_ip_address;         /* DHCP server DNS Server Address in message to DHCP Client..  */
    ULONG           nx_dhcp_subnet_mask;            /* DHCP server interface subnet mask. */
    ULONG           nx_dhcp_subnet;                 /* DHCP server interface subnet. */
    ULONG           nx_dhcp_router_ip_address;      /* The router IP Address for DHCP client configuration  */
    UINT            nx_dhcp_address_list_size;      /* Actual number of assignable addresses for this interface. */

} NX_DHCP_INTERFACE_TABLE;

/* Define the Interface address list. */

/* Define the DHCP structure that contains all the server information necessary for this DHCP 
   instance.  */

typedef struct NX_DHCP_SERVER_STRUCT 
{
    ULONG           nx_dhcp_id;                     /* DHCP thread ID */
    CHAR           *nx_dhcp_name;                   /* DHCP server name */
    NX_PACKET_POOL *nx_dhcp_packet_pool_ptr;        /* Pointer to DHCP server packet pool */
    TX_TIMER        nx_dhcp_slow_periodic_timer;    /* Timer for watching IP lease time outs. */
    TX_TIMER        nx_dhcp_fast_periodic_timer;    /* Timer for watching session time outs. */
    UCHAR           nx_dhcp_started;                /* DHCP started flag */ 
    NX_IP          *nx_dhcp_ip_ptr;                 /* The Server IP Instance   */ 
    TX_THREAD       nx_dhcp_server_thread;          /* The DHCP server processing thread   */
    TX_MUTEX        nx_dhcp_mutex;                  /* Mutex protection for client and interface tables. */
    TX_EVENT_FLAGS_GROUP nx_dhcp_server_events;     /* DHCP Server events. */
    UINT            nx_dhcp_number_clients;         /* Number of clients currently assigned IP address by this server. */
    NX_DHCP_CLIENT  client_records[NX_DHCP_CLIENT_RECORD_TABLE_SIZE];   /* Table of DHCP clients.*/
                                                    /* List of IP addresses server can assign to DHCP Clients */
    NX_UDP_SOCKET   nx_dhcp_socket;                 /* DHCP server socket to receive DHCP messages on its interfaces. */
    UINT            nx_dhcp_server_options[NX_DHCP_SERVER_OPTION_LIST_SIZE]; 
                                                    /* List of max number of options in supply data to Client */
    UINT            nx_dhcp_server_option_count;    /* Actual number of options the server is supplying back to client. */ 
                                                    
    NX_DHCP_INTERFACE_TABLE nx_dhcp_interface_table[NX_MAX_PHYSICAL_INTERFACES];
                                                    /* Interface specific table of addresses available for DHCP clients. */
    ULONG           nx_dhcp_discoveries_received;   /* The number of Discovery messages received   */ 
    ULONG           nx_dhcp_requests_received;      /* The number of Request messages received  */ 
    ULONG           nx_dhcp_informs_received;       /* The number of Inform messages received  */ 
    ULONG           nx_dhcp_declines_received;      /* The number of Decline messages received  */ 
    ULONG           nx_dhcp_releases_received;      /* The number of Release messages received  */ 

} NX_DHCP_SERVER;


#ifndef NX_DHCP_SERVER_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map DHCP API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_dhcp_server_create                  _nx_dhcp_server_create
#define nx_dhcp_create_server_ip_address_list  _nx_dhcp_create_server_ip_address_list
#define nx_dhcp_set_interface_network_parameters  _nx_dhcp_set_interface_network_parameters
#define nx_dhcp_server_delete                  _nx_dhcp_server_delete
#define nx_dhcp_server_start                   _nx_dhcp_server_start
#define nx_dhcp_server_stop                    _nx_dhcp_server_stop
#define nx_dhcp_clear_client_record            _nx_dhcp_clear_client_record

#else

/* Services with error checking.  */

#define nx_dhcp_server_create                  _nxe_dhcp_server_create
#define nx_dhcp_create_server_ip_address_list  _nxe_dhcp_create_server_ip_address_list
#define nx_dhcp_set_interface_network_parameters  _nxe_dhcp_set_interface_network_parameters
#define nx_dhcp_server_delete                  _nxe_dhcp_server_delete
#define nx_dhcp_server_start                   _nxe_dhcp_server_start
#define nx_dhcp_server_stop                    _nxe_dhcp_server_stop
#define nx_dhcp_clear_client_record            _nxe_dhcp_clear_client_record
#endif

/* Define the prototypes accessible to the application software.  */

UINT        nx_dhcp_server_create(NX_DHCP_SERVER *dhcp_ptr, NX_IP *ip_ptr, VOID *stack_ptr, ULONG stack_size, CHAR *name_ptr, NX_PACKET_POOL *packet_pool);
UINT        nx_dhcp_create_server_ip_address_list(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index, ULONG start_ip_address, ULONG end_ip_address, UINT *addresses_added);
UINT        nx_dhcp_set_interface_network_parameters(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index,  ULONG subnet_mask, ULONG default_gateway_address, ULONG dns_server_address);
UINT        nx_dhcp_server_delete(NX_DHCP_SERVER *dhcp_ptr);
UINT        nx_dhcp_server_start(NX_DHCP_SERVER *dhcp_ptr);
UINT        nx_dhcp_server_stop(NX_DHCP_SERVER *dhcp_ptr);
UINT        nx_dhcp_clear_client_record(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr);

#else

/* DHCP source code is being compiled, do not perform any API mapping.  */


UINT        _nxe_dhcp_server_create(NX_DHCP_SERVER *dhcp_ptr, NX_IP *ip_ptr, VOID *stack_ptr, ULONG stack_size, CHAR *name_ptr, NX_PACKET_POOL *packet_pool);
UINT        _nx_dhcp_server_create(NX_DHCP_SERVER *dhcp_ptr, NX_IP *ip_ptr, VOID *stack_ptr, ULONG stack_size, CHAR *name_ptr, NX_PACKET_POOL *packet_pool);
UINT        _nx_dhcp_create_server_ip_address_list(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index, ULONG start_ip_address, ULONG end_ip_address, UINT *addresses_added);
UINT        _nxe_dhcp_create_server_ip_address_list(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index, ULONG start_ip_address, ULONG end_ip_address, UINT *addresses_added);
UINT        _nxe_dhcp_set_interface_network_parameters(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index,  ULONG subnet_mask, ULONG default_gateway_address, ULONG dns_server_address);
UINT        _nx_dhcp_set_interface_network_parameters(NX_DHCP_SERVER *dhcp_ptr, UINT iface_index,  ULONG subnet_mask, ULONG default_gateway_address, ULONG dns_server_address);
UINT        _nxe_dhcp_server_delete(NX_DHCP_SERVER *dhcp_ptr);
UINT        _nx_dhcp_server_delete(NX_DHCP_SERVER *dhcp_ptr);
UINT        _nxe_dhcp_server_start(NX_DHCP_SERVER *dhcp_ptr);
UINT        _nx_dhcp_server_start(NX_DHCP_SERVER *dhcp_ptr);
UINT        _nxe_dhcp_server_stop(NX_DHCP_SERVER *dhcp_ptr);
UINT        _nx_dhcp_server_stop(NX_DHCP_SERVER *dhcp_ptr);
UINT        _nxe_dhcp_clear_client_record(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr);
UINT        _nx_dhcp_clear_client_record(NX_DHCP_SERVER *dhcp_ptr, NX_DHCP_CLIENT *dhcp_client_ptr);


#endif

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
}
#endif  /* __cplusplus */

#endif  /* NXD_DHCP_SERVER_H */
