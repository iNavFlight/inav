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
/*    nxd_dhcp_client.h                                   PORTABLE C      */ 
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */ 
/*                                                                        */ 
/*    This file defines the NetX Dynamic Host Configuration Protocol      */ 
/*    (DHCP) component, including all data types and external references. */ 
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
/*  08-02-2021     Yuxin Zhou               Modified comment(s), supported*/
/*                                            adding additional request   */
/*                                            option in parameter request,*/
/*                                            resulting in version 6.1.8  */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), supported*/
/*                                            multiple client instances,  */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/

#ifndef NXD_DHCP_CLIENT_H
#define NXD_DHCP_CLIENT_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"
#include "nx_udp.h"
#include "nx_ip.h"


/* Enable BOOTP protocol instead of DHCP, define this option.    
#define NX_DHCP_ENABLE_BOOTP      
*/

/*  Enable support for client state preserved between reboots   
#define NX_DHCP_CLIENT_RESTORE_STATE  
*/

/* Enable the DHCP Client to accept a pointer to the DHCP packet pool.  
#define NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL 
*/

/* Enables an ARP probe for verifying the assigned DHCP address is
   not owned by another host.  This is recommended, but not required by RFC 2131 (4.4.1).
#define NX_DHCP_CLIENT_SEND_ARP_PROBE
*/

/* Enables DHCP Client send Maximum DHCP Message Size Option. RFC2132, Section9.10, Page29.
#define NX_DHCP_CLIENT_SEND_MAX_DHCP_MESSAGE_OPTION
*/

/* Defined, the host name is checked, the host name must follow the rules for ARPANET host names.
   RFC 1035, Section 2.3.1, Page 8.  The default is disabled.
#define NX_DHCP_CLIENT_ENABLE_HOST_NAME_CHECK
*/

/* Define the DHCP ID that is used to mark the DHCP structure as created.  */
#define NX_DHCP_ID                      0x44484350UL


/* Define the DHCP stack size.  */
#ifndef NX_DHCP_THREAD_STACK_SIZE
#define NX_DHCP_THREAD_STACK_SIZE       (4096) 
#endif


/* Define the number of interfaces the Client is running on. This can be any
   size but practically speaking should be less than or equal to the number
   of physical interfaces attached to the IP instance (NX_MAX_PHYSICAL_INTERFACES).  */
/* Note: User can redefine the symbol to reduce the size of records if the dhcp interfaces
         are less than true physical interfaces.
   For example: There are three physical interfaces, but only two interfaces enable DHCP,
                the recommended value for NX_DHCP_CLIENT_MAX_RECORDS should be 2.  */
#ifndef NX_DHCP_CLIENT_MAX_RECORDS
#define NX_DHCP_CLIENT_MAX_RECORDS      (NX_MAX_PHYSICAL_INTERFACES)
#endif


/* Define the DHCP stack priority. This priority must be high enough to insure the 
   DHCP client gets scheduled promptly, and thus assigned an IP address.  Assigning
   it a higher priority increases the risk of 'starving' out the IP thread task which
   needs to initialize the network driver (which is required to be able to transmit packets). */
#ifndef NX_DHCP_THREAD_PRIORITY
#define NX_DHCP_THREAD_PRIORITY         3
#endif


/* Define DHCP timer expiration interval. */  
#ifndef NX_DHCP_TIME_INTERVAL
#define NX_DHCP_TIME_INTERVAL          (1 * NX_IP_PERIODIC_RATE)
#endif


/* Define the max number of user request parameter.
   Subnet mask, gateway and dns server options are added in _nx_dhcp_request_parameters arrary by default.  */
#ifndef NX_DHCP_CLIENT_MAX_USER_REQUEST_PARAMETER
#define NX_DHCP_CLIENT_MAX_USER_REQUEST_PARAMETER 4
#endif /* NX_DHCP_CLIENT_MAX_USER_REQUEST_PARAMETER */


/* Define the size of DHCP options buffer.  */
/* A DHCP client must be prepared to receive DHCP messages with an 'options' field of 
   at least length 312 octets. RFC 2131; Section 2. Protocol Summary.  */
#ifndef NX_DHCP_OPTIONS_BUFFER_SIZE
#define NX_DHCP_OPTIONS_BUFFER_SIZE     312
#endif


/* Define the size of the BOOT buffer. This should be large enough for all the
   required DHCP header fields plus the minimum requirement of 312 bytes of option data
   (total: 548 bytes) as per RFC 2131; Section 2. Protocol Summary. */
#define NX_BOOT_CLIENT_BUFFER_SIZE      548


/* Define the minimum IP datafram size as per RFC 2131; Section 2. Protocol Summary.  
   A DHCP Client must be prepared to receive a message of up to 576 bytes:
   IP header(20 bytes), UDP header (8 bytes), required DHCP header fields (236 bytes) 
   and the minimum requirement of option data(312 bytes).  */
#define NX_DHCP_MINIMUM_IP_DATAGRAM     576


/* Define the packet payload size, keeping in mind the DHCP Client must be prepared to 
   receive a message of up to 576 octets and allow room for physical network header, 
   as per RFC 2131; Section 2. Protocol Summary.  */
#ifndef NX_DHCP_PACKET_PAYLOAD
#define NX_DHCP_PACKET_PAYLOAD          (NX_DHCP_MINIMUM_IP_DATAGRAM + NX_PHYSICAL_HEADER)
#endif /* NX_DHCP_PACKET_PAYLOAD */


/* Define the packet pool size.  */
#ifndef NX_DHCP_PACKET_POOL_SIZE        
#define NX_DHCP_PACKET_POOL_SIZE        (5 * NX_DHCP_PACKET_PAYLOAD)
#endif


/* Define time out options for retransmission in seconds.  */
/* Define the minimum amount of time to retransmit a DHCP IP address request. The 
   recommended wait time is 4 seconds in RFC 2131. */ 
#ifndef NX_DHCP_MIN_RETRANS_TIMEOUT 
#define NX_DHCP_MIN_RETRANS_TIMEOUT     (4 * NX_IP_PERIODIC_RATE)
#endif


/* Define the maximum amount of time to retransmit a DHCP IP address request. The 
   recommended wait time is 64 seconds in RFC 2131. */  
#ifndef NX_DHCP_MAX_RETRANS_TIMEOUT 
#define NX_DHCP_MAX_RETRANS_TIMEOUT     (64 * NX_IP_PERIODIC_RATE)
#endif


/* Define the minimum amount of time to retransmit a DHCP renew/rebind request. The 
   recommended wait time is 60 seconds in RFC 2131. */ 
#ifndef NX_DHCP_MIN_RENEW_TIMEOUT 
#define NX_DHCP_MIN_RENEW_TIMEOUT      (60 * NX_IP_PERIODIC_RATE)
#endif


/* Define UDP socket create options.  */
#ifndef NX_DHCP_TYPE_OF_SERVICE
#define NX_DHCP_TYPE_OF_SERVICE         NX_IP_NORMAL
#endif

#ifndef NX_DHCP_FRAGMENT_OPTION
#define NX_DHCP_FRAGMENT_OPTION         NX_DONT_FRAGMENT
#endif  

#ifndef NX_DHCP_TIME_TO_LIVE
#define NX_DHCP_TIME_TO_LIVE            0x80
#endif

#ifndef NX_DHCP_QUEUE_DEPTH
#define NX_DHCP_QUEUE_DEPTH             4
#endif


#ifdef NX_DHCP_CLIENT_SEND_ARP_PROBE

/* Define the timing and retry constants for ARP probe. RFC5227, Section1.1, Page 5.  */
#ifndef NX_DHCP_ARP_PROBE_WAIT
#define NX_DHCP_ARP_PROBE_WAIT          (1 * NX_IP_PERIODIC_RATE)
#endif

/* Define the number of ARP probes sent. */
#ifndef NX_DHCP_ARP_PROBE_NUM
#define NX_DHCP_ARP_PROBE_NUM           3
#endif

/* Define the minimum and maximum variation in the interval between ARP probe transmissions. */
#ifndef NX_DHCP_ARP_PROBE_MIN
#define NX_DHCP_ARP_PROBE_MIN           (1 * NX_IP_PERIODIC_RATE)
#endif

#ifndef NX_DHCP_ARP_PROBE_MAX
#define NX_DHCP_ARP_PROBE_MAX           (2 * NX_IP_PERIODIC_RATE)
#endif
#endif /* NX_DHCP_CLIENT_SEND_ARP_PROBE  */


/*  Define the wait time before restarting the configuration process when DHCP detects that the address is
    already in use.
 
    The client SHOULD wait a minimum of ten seconds before restarting the configuration process
    to avoid excessive network traffic in case of looping. RFC2131, Section 3.1, Page 17.  */
#ifndef NX_DHCP_RESTART_WAIT
#define NX_DHCP_RESTART_WAIT            (10 * NX_IP_PERIODIC_RATE)
#endif


/* Define the BootP Message Area Offsets.  The DHCP message format is identical to that of BootP, except
   for the Vendor options that start at the offset specified by NX_BOOTP_OFFSET_OPTIONS.  */
#define NX_BOOTP_OFFSET_OP              0       /* 1 BootP Operation 1=req, 2=reply                         */
#define NX_BOOTP_OFFSET_HTYPE           1       /* 1 Hardware type 1 = Ethernet                             */
#define NX_BOOTP_OFFSET_HLEN            2       /* 1 Hardware address length, 6 for Ethernet                */
#define NX_BOOTP_OFFSET_HOPS            3       /* 1 Number of hops, usually 0                              */
#define NX_BOOTP_OFFSET_XID             4       /* 4 Transaction ID, pseudo random number                   */
#define NX_BOOTP_OFFSET_SECS            8       /* 2 Seconds since boot                                     */
#define NX_BOOTP_OFFSET_FLAGS           10      /* 2 Flags, 0x80 = Broadcast response, 0 = unicast response */
#define NX_BOOTP_OFFSET_CLIENT_IP       12      /* 4 Initial client IP, used as dest for unicast response   */
#define NX_BOOTP_OFFSET_YOUR_IP         16      /* 4 Assigned IP, initialized to 0.0.0.0                    */
#define NX_BOOTP_OFFSET_SERVER_IP       20      /* 4 Server IP, usually initialized to 0.0.0.0              */
#define NX_BOOTP_OFFSET_GATEWAY_IP      24      /* 4 gateway IP, usually 0.0.0.0, only for BootP and TFTP   */
#define NX_BOOTP_OFFSET_CLIENT_HW       28      /* 16 Client hardware address                               */
#define NX_BOOTP_OFFSET_SERVER_NM       44      /* 64 Server name, nulls if unused                          */
#define NX_BOOTP_OFFSET_BOOT_FILE       108     /* 128 Boot file name, null if unused                       */
#define NX_BOOTP_OFFSET_VENDOR          236     /* 64 Vendor options, set first 4 bytes to a magic number   */
#define NX_BOOTP_OFFSET_OPTIONS         240     /* First variable vendor option                             */
#define NX_BOOTP_OFFSET_END             300     /* End of BOOTP buffer                                      */


/* Define the DHCP Specific Vendor Extensions. */
#define NX_DHCP_OPTION_PAD              0
#define NX_DHCP_OPTION_PAD_SIZE         0
#define NX_DHCP_OPTION_SUBNET_MASK      1
#define NX_DHCP_OPTION_SUBNET_MASK_SIZE 4
#define NX_DHCP_OPTION_TIME_OFFSET      2
#define NX_DHCP_OPTION_TIME_OFFSET_SIZE 4
#define NX_DHCP_OPTION_GATEWAYS         3
#define NX_DHCP_OPTION_TIMESVR          4
#define NX_DHCP_OPTION_DNS_SVR          6
#define NX_DHCP_OPTION_HOST_NAME        12
#define NX_DHCP_OPTION_DNS_NAME         15
#define NX_DHCP_OPTION_NTP_SVR          42
#define NX_DHCP_OPTION_VENDOR_OPTIONS   43
#define NX_DHCP_OPTION_DHCP_IP_REQ      50
#define NX_DHCP_OPTION_DHCP_IP_REQ_SIZE 4
#define NX_DHCP_OPTION_DHCP_LEASE       51
#define NX_DHCP_OPTION_DHCP_LEASE_SIZE  4
#define NX_DHCP_OPTION_DHCP_TYPE        53
#define NX_DHCP_OPTION_DHCP_TYPE_SIZE   1
#define NX_DHCP_OPTION_DHCP_SERVER      54
#define NX_DHCP_OPTION_DHCP_SERVER_SIZE 4
#define NX_DHCP_OPTION_DHCP_PARAMETERS  55
#define NX_DHCP_OPTION_DHCP_MESSAGE     56 
#define NX_DHCP_OPTION_MAX_DHCP_MESSAGE 57
#define NX_DHCP_OPTION_RENEWAL          58
#define NX_DHCP_OPTION_RENEWAL_SIZE     4
#define NX_DHCP_OPTION_REBIND           59
#define NX_DHCP_OPTION_REBIND_SIZE      4
#define NX_DHCP_OPTION_CLIENT_ID        61
#define NX_DHCP_OPTION_CLIENT_ID_SIZE   7 /* 1 byte for address type (01 = Ethernet), 6 bytes for address */ 
#define NX_DHCP_OPTION_FDQN             81
#define NX_DHCP_OPTION_FDQN_FLAG_N      8
#define NX_DHCP_OPTION_FDQN_FLAG_E      4
#define NX_DHCP_OPTION_FDQN_FLAG_O      2
#define NX_DHCP_OPTION_FDQN_FLAG_S      1
#define NX_DHCP_OPTION_END              255
#define NX_DHCP_OPTION_END_SIZE         0


/* Define various BootP/DHCP constants.  */
#define NX_DHCP_SERVER_UDP_PORT         67
#define NX_DHCP_SERVER_TCP_PORT         67
#define NX_DHCP_CLIENT_UDP_PORT         68
#define NX_DHCP_CLIENT_TCP_PORT         68

#define NX_BOOTP_OP_REQUEST             1
#define NX_BOOTP_OP_REPLY               2
#define NX_BOOTP_TYPE_ETHERNET          1
#define NX_BOOTP_HLEN_ETHERNET          6
#define NX_BOOTP_FLAGS_BROADCAST        0x80 
#define NX_BOOTP_FLAGS_UNICAST          0x00
#define NX_BOOTP_MAGIC_COOKIE           IP_ADDRESS(99, 130, 83, 99)
#define NX_BOOTP_NO_ADDRESS             IP_ADDRESS(0, 0, 0, 0)
#define NX_BOOTP_BC_ADDRESS             IP_ADDRESS(255, 255, 255, 255)
#define NX_AUTO_IP_ADDRESS              IP_ADDRESS(169, 254, 0, 0)
#define NX_AUTO_IP_ADDRESS_MASK         0xFFFF0000UL

#define NX_DHCP_INFINITE_LEASE          0xffffffffUL


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
#ifdef NX_DHCP_ENABLE_BOOTP
#define NX_DHCP_TYPE_BOOT_REQUEST       10
#endif


/* Define the states of the DHCP state machine.  */
#define NX_DHCP_STATE_NOT_STARTED       0       /* Not started                                              */
#define NX_DHCP_STATE_BOOT              1       /* Started with a previous address                          */
#define NX_DHCP_STATE_INIT              2       /* Started with no previous address                         */
#define NX_DHCP_STATE_SELECTING         3       /* Waiting to identify a DHCP server                        */
#define NX_DHCP_STATE_REQUESTING        4       /* Address requested, waiting for the Ack                   */
#define NX_DHCP_STATE_BOUND             5       /* Address established, no time outs                        */
#define NX_DHCP_STATE_RENEWING          6       /* Address established, renewal time out                    */
#define NX_DHCP_STATE_REBINDING         7       /* Address established, renewal and rebind time out         */
#define NX_DHCP_STATE_FORCERENEW        8       /* Address established, force renewal                       */
#define NX_DHCP_STATE_ADDRESS_PROBING   9       /* Address probing, address conflict detection              */

/* Define error codes from DHCP API.  */
#define NX_DHCP_ERROR                   0x90    /* General DHCP error code                                  */ 
#define NX_DHCP_NO_RESPONSE             0x91    /* No response from server for option request               */ 
#define NX_DHCP_BAD_IP_ADDRESS          0x92    /* Bad IP address or invalid interface input                */ 
#define NX_DHCP_ALREADY_STARTED         0x93    /* DHCP was already started                                 */
#define NX_DHCP_NOT_BOUND               0x94    /* DHCP is not in a bound state                             */ 
#define NX_DHCP_DEST_TO_SMALL           0x95    /* DHCP response is too big for destination                 */ 
#define NX_DHCP_NOT_STARTED             0x96    /* DHCP was not started when stop was issued                */ 
#define NX_DHCP_PARSE_ERROR             0x97    /* Error extracting DHCP option data                        */
#define NX_DHCP_BAD_XID                 0x98    /* DHCP packet received with mismatched XID                 */
#define NX_DHCP_BAD_MAC_ADDRESS         0x99    /* DHCP packet received with mismatched MAC address         */
#define NX_DHCP_INVALID_MESSAGE         0x9B    /* Invalid message received or requested to send            */
#define NX_DHCP_INVALID_PAYLOAD         0x9C    /* Client receives DHCP message exceeding packet payload    */
#define NX_DHCP_INVALID_IP_REQUEST      0x9D    /* Null IP address input for requesting IP address          */
#define NX_DHCP_UNKNOWN_OPTION          0x9F    /* This option is unknow.                                   */
#define NX_DHCP_INTERFACE_ALREADY_ENABLED 0xA3  /* Interface is already enabled                             */
#define NX_DHCP_INTERFACE_NOT_ENABLED   0xA4    /* If interface not enabled for DHCP interaction            */ 
#define NX_DHCP_NO_INTERFACES_ENABLED   0xA5    /* No interfaces enabled for DHCP interaction               */ 
#define NX_DHCP_NO_INTERFACES_STARTED   0xA6    /* If DHCP CLient fails to start any interfacers            */
#define NX_DHCP_NO_RECORDS_AVAILABLE    0xA7    /* No Client record available to start DHCP on an interface */
#define NX_DHCP_INVALID_NAME            0xA8    /* Client host name has invalid characters                  */


/* Define DHCP Client thread events.  */
#define NX_DHCP_CLIENT_RECEIVE_EVENT    0x00000001        /* DHCP Server data received                            */
#define NX_DHCP_CLIENT_TIMER_EVENT      0x00000002        /* DHCP timer expires                                   */ 
#define NX_DHCP_CLIENT_CONFLICT_EVENT   0x00000004        /* IP conflict event detected                           */
#define NX_DHCP_CLIENT_ALL_EVENTS       0xFFFFFFFF        /* All DHCP events                                      */


#ifdef NX_DHCP_CLIENT_RESTORE_STATE

/* Define a Client record for restore DHCP Client state from non volatile memory/across reboots. */
typedef struct NX_DHCP_CLIENT_RECORD_STRUCT 
{
    UCHAR           nx_dhcp_state;              /* The current state of the DHCP Client                     */
    ULONG           nx_dhcp_ip_address;         /* Server assigned IP Address                               */ 
    ULONG           nx_dhcp_network_mask;       /* Server assigned network mask                             */  
    ULONG           nx_dhcp_gateway_address;    /* Server assigned gateway address                          */  
    UINT            nx_dhcp_interface_index;    /* Index of DHCP Client network interface                   */  
    ULONG           nx_dhcp_timeout;            /* The current value of any timeout, in seconds             */
    ULONG           nx_dhcp_server_ip;          /* The server IP Address                                    */
    ULONG           nx_dhcp_lease_remain_time;  /* Time remaining before lease expires                      */
    ULONG           nx_dhcp_lease_time;         /* The current Lease Time in seconds                        */
    ULONG           nx_dhcp_renewal_time;       /* Renewal Time in seconds                                  */
    ULONG           nx_dhcp_rebind_time;        /* Rebind Time in seconds                                   */
    ULONG           nx_dhcp_renewal_remain_time;/* Time remaining to renew (before rebinding necessary)     */
    ULONG           nx_dhcp_rebind_remain_time; /* Time remaining to rebind (before lease expires)          */
} NX_DHCP_CLIENT_RECORD;
#endif /* NX_DHCP_CLIENT_RESTORE_STATE */

/* Define the DHCP interface record that contains all the information necessary for a DHCP 
   instance on each interface.  */
typedef struct NX_DHCP_INTERFACE_RECORD_STRUCT
{
    UCHAR           nx_dhcp_record_valid;       /* The flag indicate this record is valid. NX_TRUE: valid   */
    UCHAR           nx_dhcp_state;              /* The current state of the DHCP Client                     */
    UCHAR           nx_dhcp_user_option;        /* User option request                                      */
    UCHAR           reserved;                                                                               
    ULONG           nx_dhcp_xid;                /* Unique transaction ID                                    */
    ULONG           nx_dhcp_seconds;            /* Track number of seconds for a DHCP request process       */
    ULONG           nx_dhcp_ip_address;         /* Server assigned IP Address                               */
    ULONG           nx_dhcp_gateway_address;    /* Server assigned gateway address                          */
    ULONG           nx_dhcp_server_ip;          /* The server IP Address                                    */
    ULONG           nx_dhcp_network_mask;       /* Server assigned network mask                             */
    UINT            nx_dhcp_interface_index;    /* Index of DHCP Client network interface                   */
    ULONG           nx_dhcp_timeout;            /* Count down timer for sending out DHCP message            */
    ULONG           nx_dhcp_rtr_interval;       /* Interval between sending out another DHCP message        */
    ULONG           nx_dhcp_lease_remain_time;  /* Time remaining before lease expires                      */
    ULONG           nx_dhcp_lease_time;         /* The current Lease Time in seconds                        */
    ULONG           nx_dhcp_renewal_time;       /* Renewal Time in seconds                                  */
    ULONG           nx_dhcp_rebind_time;        /* Rebind Time in seconds                                   */
    ULONG           nx_dhcp_renewal_remain_time;/* Time remaining to renew (before rebinding necessary)     */
    ULONG           nx_dhcp_rebind_remain_time; /* Time remaining to rebind (before lease expires)          */
#ifdef NX_DHCP_CLIENT_SEND_ARP_PROBE
    UINT            nx_dhcp_probe_count;        /* Number of ARP probes to send to prove IP address unique  */
#endif
    UINT            nx_dhcp_clear_broadcast;    /* Client sends messages with unicast reply requested       */
    UINT            nx_dhcp_skip_discovery;     /* Indicate if host should skip the discovery message       */
    UCHAR           nx_dhcp_options_buffer[NX_DHCP_OPTIONS_BUFFER_SIZE];   
    UINT            nx_dhcp_options_size;       /* The total size of DHCP options.                          */ 

    ULONG           nx_dhcp_internal_errors;    /* The number of internal DHCP errors encountered           */
    ULONG           nx_dhcp_discoveries_sent;   /* The number of Discovery sent by the Client               */
    ULONG           nx_dhcp_offers_received;    /* The number of Offers received by the Client              */
    ULONG           nx_dhcp_requests_sent;      /* The number of Request attempts made by the Client        */
    ULONG           nx_dhcp_acks_received;      /* The number of ACKs received by the Client                */
    ULONG           nx_dhcp_nacks_received;     /* The number of NACKs received by the Client               */
    ULONG           nx_dhcp_releases_sent;      /* The number of Releases sent by the Client                */
    ULONG           nx_dhcp_declines_sent;      /* The number of Declines sent by the Client                */
    ULONG           nx_dhcp_force_renewal_rec;  /* The number of Forced Renewal received by the Client      */
    ULONG           nx_dhcp_informs_sent;       /* The number of Inform (option requests) sent by the Client*/
    ULONG           nx_dhcp_inform_responses;   /* The number of Inform responses                           */

} NX_DHCP_INTERFACE_RECORD;


/* Define the DHCP structure that contains all information common to all interfaces on
   which DHCP Client may run.  */
typedef struct NX_DHCP_STRUCT 
{

    ULONG           nx_dhcp_id;                 /* DHCP Structure ID                                        */
    CHAR           *nx_dhcp_name;               /* DHCP name supplied at create                             */ 
    NX_IP          *nx_dhcp_ip_ptr;             /* The associated IP pointer for this DHCP instance         */ 
#ifndef NX_DHCP_CLIENT_USER_CREATE_PACKET_POOL
    NX_PACKET_POOL  nx_dhcp_pool;               /* The pool of UDP data packets for DHCP messages           */
    UCHAR           nx_dhcp_pool_area[NX_DHCP_PACKET_POOL_SIZE];
#endif
    NX_PACKET_POOL *nx_dhcp_packet_pool_ptr;    /* Pointer to DHCP Client packet pool                       */
    NX_UDP_SOCKET   nx_dhcp_socket;             /* The Socket used for DHCP messages                        */
    TX_THREAD       nx_dhcp_thread;             /* The DHCP processing thread                               */
    UCHAR           nx_dhcp_thread_stack[NX_DHCP_THREAD_STACK_SIZE];
    TX_MUTEX        nx_dhcp_mutex;              /* The DHCP mutex for protecting access                     */
    TX_EVENT_FLAGS_GROUP
                    nx_dhcp_events;              /* DHCP Client thread events                               */ 
    TX_TIMER        nx_dhcp_timer;               /* DHCP Client timeout timer                               */ 
    NX_DHCP_INTERFACE_RECORD 
                    nx_dhcp_interface_record[NX_DHCP_CLIENT_MAX_RECORDS];  
                                                /* Record of DHCP Client state on specific interface        */
    UCHAR           nx_dhcp_user_request_parameter[NX_DHCP_CLIENT_MAX_USER_REQUEST_PARAMETER];  
                                                /* User request parameter                                   */
    UINT            nx_dhcp_user_request_parameter_size;  
                                                /* User request parameter size                              */

#ifdef NX_DHCP_CLIENT_SEND_MAX_DHCP_MESSAGE_OPTION
    ULONG           nx_dhcp_max_dhcp_message_size;
                                                /* Maximum length DHCP message DHCP Client will accept     */
#endif /* NX_DHCP_CLIENT_SEND_MAX_DHCP_MESSAGE_OPTION */

#ifdef NX_DHCP_CLIENT_SEND_ARP_PROBE
    UINT            nx_dhcp_interface_conflict_flag;    /* The flag indicate IP addresses conflict on which interfaces, one bit represent one interface.
                                                           For examples:
                                                           0x00000001: interface index 0;
                                                           0x00000002: interface index 1;
                                                           0x00000003: interface index 0 and 1;  */
#endif /* NX_DHCP_CLIENT_SEND_ARP_PROBE  */

    /* Define the callback function for DHCP state change notification. If specified
       by the application, this function is called whenever a state change occurs for
       the DHCP associated with this IP instance.  */
    VOID (*nx_dhcp_state_change_callback)(struct NX_DHCP_STRUCT *dhcp_ptr, UCHAR new_state);
     
    /* Define the callback function for DHCP interface state change notification. 
       this function is similar as nx_dhcp_state_change_callback, 

       Note: Suggest using state change notification, and use nx_dhcp_interface_state_change_callback
             if DHCP is running on multiple interfaces.  */
    VOID (*nx_dhcp_interface_state_change_callback)(struct NX_DHCP_STRUCT *dhcp_ptr, UINT iface_index, UCHAR new_state);

    /* Define the callback function for adding specific DHCP user option.  */
    UINT (*nx_dhcp_user_option_add)(struct NX_DHCP_STRUCT *dhcp_ptr, UINT iface_index, UINT message_type, UCHAR *user_option_ptr, UINT *user_option_length);

    /* Define the link between other DHCP structures created by the application.  */
    struct NX_DHCP_STRUCT *nx_dhcp_created_next;

    /* This pointer is reserved for application specific use.  */
    void            *nx_dhcp_reserved_ptr;
    
} NX_DHCP;


#ifndef NX_DHCP_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map DHCP API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */

#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_dhcp_create                                  _nx_dhcp_create
#define nx_dhcp_packet_pool_set                         _nx_dhcp_packet_pool_set
#define nx_dhcp_request_client_ip                       _nx_dhcp_request_client_ip
#define nx_dhcp_delete                                  _nx_dhcp_delete
#define nx_dhcp_decline                                 _nx_dhcp_decline
#define nx_dhcp_force_renew                             _nx_dhcp_force_renew
#define nx_dhcp_release                                 _nx_dhcp_release
#define nx_dhcp_start                                   _nx_dhcp_start
#define nx_dhcp_stop                                    _nx_dhcp_stop
#define nx_dhcp_server_address_get                      _nx_dhcp_server_address_get
#define nx_dhcp_state_change_notify                     _nx_dhcp_state_change_notify
#define nx_dhcp_user_option_request                     _nx_dhcp_user_option_request
#define nx_dhcp_user_option_retrieve                    _nx_dhcp_user_option_retrieve
#define nx_dhcp_user_option_convert                     _nx_dhcp_user_option_convert
#define nx_dhcp_user_option_add_callback_set            _nx_dhcp_user_option_add_callback_set
#define nx_dhcp_reinitialize                            _nx_dhcp_reinitialize
#define nx_dhcp_send_request                            _nx_dhcp_send_request
#define nx_dhcp_set_interface_index                     _nx_dhcp_set_interface_index  
#define nx_dhcp_clear_broadcast_flag                    _nx_dhcp_clear_broadcast_flag
#define nx_dhcp_interface_clear_broadcast_flag          _nx_dhcp_interface_clear_broadcast_flag
#define nx_dhcp_interface_enable                        _nx_dhcp_interface_enable
#define nx_dhcp_interface_disable                       _nx_dhcp_interface_disable
#define nx_dhcp_interface_decline                       _nx_dhcp_interface_decline
#define nx_dhcp_interface_force_renew                   _nx_dhcp_interface_force_renew
#define nx_dhcp_interface_reinitialize                  _nx_dhcp_interface_reinitialize
#define nx_dhcp_interface_release                       _nx_dhcp_interface_release
#define nx_dhcp_interface_request_client_ip             _nx_dhcp_interface_request_client_ip
#define nx_dhcp_interface_start                         _nx_dhcp_interface_start
#define nx_dhcp_interface_stop                          _nx_dhcp_interface_stop
#define nx_dhcp_interface_send_request                  _nx_dhcp_interface_send_request
#define nx_dhcp_interface_server_address_get            _nx_dhcp_interface_server_address_get
#define nx_dhcp_interface_state_change_notify           _nx_dhcp_interface_state_change_notify
#define nx_dhcp_interface_user_option_retrieve          _nx_dhcp_interface_user_option_retrieve
#ifdef NX_DHCP_CLIENT_RESTORE_STATE
#define nx_dhcp_resume                                  _nx_dhcp_resume
#define nx_dhcp_suspend                                 _nx_dhcp_suspend
#define nx_dhcp_client_get_record                       _nx_dhcp_client_get_record
#define nx_dhcp_client_restore_record                   _nx_dhcp_client_restore_record
#define nx_dhcp_client_update_time_remaining            _nx_dhcp_client_update_time_remaining
#define nx_dhcp_client_interface_get_record             _nx_dhcp_client_interface_get_record
#define nx_dhcp_client_interface_restore_record         _nx_dhcp_client_interface_restore_record
#define nx_dhcp_client_interface_update_time_remaining  _nx_dhcp_client_interface_update_time_remaining
#endif /* NX_DHCP_CLIENT_RESTORE_STATE */

#else

/* Services with error checking.  */

#define nx_dhcp_create                                  _nxe_dhcp_create
#define nx_dhcp_packet_pool_set                         _nxe_dhcp_packet_pool_set
#define nx_dhcp_request_client_ip                       _nxe_dhcp_request_client_ip
#define nx_dhcp_delete                                  _nxe_dhcp_delete
#define nx_dhcp_decline                                 _nxe_dhcp_decline
#define nx_dhcp_force_renew                             _nxe_dhcp_force_renew
#define nx_dhcp_release                                 _nxe_dhcp_release
#define nx_dhcp_start                                   _nxe_dhcp_start
#define nx_dhcp_stop                                    _nxe_dhcp_stop
#define nx_dhcp_server_address_get                      _nxe_dhcp_server_address_get
#define nx_dhcp_state_change_notify                     _nxe_dhcp_state_change_notify
#define nx_dhcp_user_option_request                     _nxe_dhcp_user_option_request
#define nx_dhcp_user_option_retrieve                    _nxe_dhcp_user_option_retrieve
#define nx_dhcp_user_option_convert                     _nxe_dhcp_user_option_convert
#define nx_dhcp_user_option_add_callback_set            _nxe_dhcp_user_option_add_callback_set
#define nx_dhcp_reinitialize                            _nxe_dhcp_reinitialize
#define nx_dhcp_send_request                            _nxe_dhcp_send_request
#define nx_dhcp_set_interface_index                     _nxe_dhcp_set_interface_index  
#define nx_dhcp_clear_broadcast_flag                    _nxe_dhcp_clear_broadcast_flag 
#define nx_dhcp_interface_clear_broadcast_flag          _nxe_dhcp_interface_clear_broadcast_flag
#define nx_dhcp_interface_enable                        _nxe_dhcp_interface_enable
#define nx_dhcp_interface_disable                       _nxe_dhcp_interface_disable
#define nx_dhcp_interface_decline                       _nxe_dhcp_interface_decline
#define nx_dhcp_interface_force_renew                   _nxe_dhcp_interface_force_renew 
#define nx_dhcp_interface_reinitialize                  _nxe_dhcp_interface_reinitialize
#define nx_dhcp_interface_release                       _nxe_dhcp_interface_release
#define nx_dhcp_interface_request_client_ip             _nxe_dhcp_interface_request_client_ip
#define nx_dhcp_interface_start                         _nxe_dhcp_interface_start
#define nx_dhcp_interface_stop                          _nxe_dhcp_interface_stop
#define nx_dhcp_interface_send_request                  _nxe_dhcp_interface_send_request
#define nx_dhcp_interface_server_address_get            _nxe_dhcp_interface_server_address_get 
#define nx_dhcp_interface_state_change_notify           _nxe_dhcp_interface_state_change_notify
#define nx_dhcp_interface_user_option_retrieve          _nxe_dhcp_interface_user_option_retrieve
#ifdef NX_DHCP_CLIENT_RESTORE_STATE                        
#define nx_dhcp_resume                                  _nxe_dhcp_resume
#define nx_dhcp_suspend                                 _nxe_dhcp_suspend
#define nx_dhcp_client_get_record                       _nxe_dhcp_client_get_record
#define nx_dhcp_client_restore_record                   _nxe_dhcp_client_restore_record
#define nx_dhcp_client_update_time_remaining            _nxe_dhcp_client_update_time_remaining
#define nx_dhcp_client_interface_get_record             _nxe_dhcp_client_interface_get_record
#define nx_dhcp_client_interface_restore_record         _nxe_dhcp_client_interface_restore_record
#define nx_dhcp_client_interface_update_time_remaining  _nxe_dhcp_client_interface_update_time_remaining
#endif /* NX_DHCP_CLIENT_RESTORE_STATE */

#endif  /* NX_DISABLE_ERROR_CHECKING */

/* Define the prototypes accessible to the application software.  */

UINT        nx_dhcp_create(NX_DHCP *dhcp_ptr, NX_IP *ip_ptr, CHAR *name_ptr);
UINT        nx_dhcp_packet_pool_set(NX_DHCP *dhcp_ptr, NX_PACKET_POOL *packet_pool_ptr);
UINT        nx_dhcp_request_client_ip(NX_DHCP *dhcp_ptr, ULONG client_ip_address, UINT skip_discover_message);
UINT        nx_dhcp_delete(NX_DHCP *dhcp_ptr);
UINT        nx_dhcp_decline(NX_DHCP *dhcp_ptr);
UINT        nx_dhcp_force_renew(NX_DHCP *dhcp_ptr);
UINT        nx_dhcp_release(NX_DHCP *dhcp_ptr);
UINT        nx_dhcp_start(NX_DHCP *dhcp_ptr);
UINT        nx_dhcp_stop(NX_DHCP *dhcp_ptr);
UINT        nx_dhcp_server_address_get(NX_DHCP *dhcp_ptr, ULONG *server_address);
UINT        nx_dhcp_state_change_notify(NX_DHCP *dhcp_ptr, VOID (*dhcp_state_change_notify)(NX_DHCP *dhcp_ptr, UCHAR new_state));
UINT        nx_dhcp_user_option_request(NX_DHCP *dhcp_ptr, UINT option_code);
UINT        nx_dhcp_user_option_retrieve(NX_DHCP *dhcp_ptr, UINT request_option, UCHAR *destination_ptr, UINT *destination_size);
ULONG       nx_dhcp_user_option_convert(UCHAR *source_ptr);
UINT        nx_dhcp_user_option_add_callback_set(NX_DHCP *dhcp_ptr, UINT (*dhcp_user_option_add)(NX_DHCP *dhcp_ptr, UINT iface_index, UINT message_type, UCHAR *user_option_ptr, UINT *user_option_length));
UINT        nx_dhcp_reinitialize(NX_DHCP *dhcp_ptr);
UINT        nx_dhcp_send_request(NX_DHCP *dhcp_ptr, UINT dhcp_message_type);
UINT        nx_dhcp_set_interface_index(NX_DHCP *dhcp_ptr, UINT interface_index); 
UINT        nx_dhcp_clear_broadcast_flag(NX_DHCP *dhcp_ptr, UINT clear_flag);
UINT        nx_dhcp_interface_clear_broadcast_flag(NX_DHCP *dhcp_ptr, UINT iface_index, UINT clear_flag);
UINT        nx_dhcp_interface_enable(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        nx_dhcp_interface_disable(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        nx_dhcp_interface_decline(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        nx_dhcp_interface_force_renew(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        nx_dhcp_interface_release(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        nx_dhcp_interface_reinitialize(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        nx_dhcp_interface_request_client_ip(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG client_ip_address, UINT skip_discover_message);
UINT        nx_dhcp_interface_start(NX_DHCP *dhcp_ptr, UINT iface_index); 
UINT        nx_dhcp_interface_stop(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        nx_dhcp_interface_send_request(NX_DHCP *dhcp_ptr, UINT iface_index, UINT dhcp_message_type);  
UINT        nx_dhcp_interface_server_address_get(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG *server_address);
UINT        nx_dhcp_interface_state_change_notify(NX_DHCP *dhcp_ptr, VOID (*dhcp_interface_state_change_notify)(NX_DHCP *dhcp_ptr, UINT iface_index, UCHAR new_state));
UINT        nx_dhcp_interface_user_option_retrieve(NX_DHCP *dhcp_ptr, UINT iface_index, UINT option_request, UCHAR *destination_ptr, UINT *destination_size);

#ifdef NX_DHCP_CLIENT_RESTORE_STATE                        
UINT        nx_dhcp_resume(NX_DHCP *dhcp_ptr);
UINT        nx_dhcp_suspend(NX_DHCP *dhcp_ptr);
UINT        nx_dhcp_client_get_record(NX_DHCP *dhcp_ptr, NX_DHCP_CLIENT_RECORD *record_ptr);               
UINT        nx_dhcp_client_restore_record(NX_DHCP *dhcp_ptr, NX_DHCP_CLIENT_RECORD *record_ptr, ULONG time_elapsed);           
UINT        nx_dhcp_client_update_time_remaining(NX_DHCP *dhcp_ptr, ULONG time_elapsed);
UINT        nx_dhcp_client_interface_get_record(NX_DHCP *dhcp_ptr, UINT iface_index,  NX_DHCP_CLIENT_RECORD *record_ptr);               
UINT        nx_dhcp_client_interface_restore_record(NX_DHCP *dhcp_ptr, UINT iface_index, NX_DHCP_CLIENT_RECORD *record_ptr, ULONG time_elapsed);           
UINT        nx_dhcp_client_interface_update_time_remaining(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG time_elapsed);
#endif /* NX_DHCP_CLIENT_RESTORE_STATE */

#else

/* DHCP source code is being compiled, do not perform any API mapping.  */

UINT        _nxe_dhcp_create(NX_DHCP *dhcp_ptr, NX_IP *ip_ptr, CHAR *name_ptr);
UINT        _nx_dhcp_create(NX_DHCP *dhcp_ptr, NX_IP *ip_ptr, CHAR *name_ptr);
UINT        _nxe_dhcp_packet_pool_set(NX_DHCP *dhcp_ptr, NX_PACKET_POOL *packet_pool_ptr);
UINT        _nx_dhcp_packet_pool_set(NX_DHCP *dhcp_ptr, NX_PACKET_POOL *packet_pool_ptr);
UINT        _nxe_dhcp_request_client_ip(NX_DHCP *dhcp_ptr, ULONG client_ip_address, UINT skip_discover_message);
UINT        _nx_dhcp_request_client_ip(NX_DHCP *dhcp_ptr, ULONG client_ip_address, UINT skip_discover_message);
UINT        _nxe_dhcp_delete(NX_DHCP *dhcp_ptr);
UINT        _nx_dhcp_delete(NX_DHCP *dhcp_ptr);
UINT        _nxe_dhcp_decline(NX_DHCP *dhcp_ptr);
UINT        _nx_dhcp_decline(NX_DHCP *dhcp_ptr);
UINT        _nxe_dhcp_force_renew(NX_DHCP *dhcp_ptr);
UINT        _nx_dhcp_force_renew(NX_DHCP *dhcp_ptr);
UINT        _nxe_dhcp_release(NX_DHCP *dhcp_ptr);
UINT        _nx_dhcp_release(NX_DHCP *dhcp_ptr);
UINT        _nxe_dhcp_start(NX_DHCP *dhcp_ptr);
UINT        _nx_dhcp_start(NX_DHCP *dhcp_ptr);
UINT        _nxe_dhcp_stop(NX_DHCP *dhcp_ptr);
UINT        _nx_dhcp_stop(NX_DHCP *dhcp_ptr); 
UINT        _nxe_dhcp_server_address_get(NX_DHCP *dhcp_ptr, ULONG *server_address);
UINT        _nx_dhcp_server_address_get(NX_DHCP *dhcp_ptr, ULONG *server_address);
UINT        _nxe_dhcp_state_change_notify(NX_DHCP *dhcp_ptr,  VOID (*dhcp_state_change_notify)(NX_DHCP *dhcp_ptr, UCHAR new_state));
UINT        _nx_dhcp_state_change_notify(NX_DHCP *dhcp_ptr, VOID (*dhcp_state_change_notify)(NX_DHCP *dhcp_ptr, UCHAR new_state));
UINT        _nxe_dhcp_user_option_request(NX_DHCP *dhcp_ptr, UINT option_code);
UINT        _nx_dhcp_user_option_request(NX_DHCP *dhcp_ptr, UINT option_code);
UINT        _nxe_dhcp_user_option_retrieve(NX_DHCP *dhcp_ptr, UINT request_option, UCHAR *destination_ptr, UINT *destination_size);
UINT        _nx_dhcp_user_option_retrieve(NX_DHCP *dhcp_ptr, UINT request_option, UCHAR *destination_ptr, UINT *destination_size); 
ULONG       _nxe_dhcp_user_option_convert(UCHAR *source_ptr);
ULONG       _nx_dhcp_user_option_convert(UCHAR *source_ptr);
UINT        _nxe_dhcp_reinitialize(NX_DHCP *dhcp_ptr);
UINT        _nx_dhcp_reinitialize(NX_DHCP *dhcp_ptr);
UINT        _nxe_dhcp_send_request(NX_DHCP *dhcp_ptr, UINT dhcp_message_type);
UINT        _nx_dhcp_send_request(NX_DHCP *dhcp_ptr, UINT dhcp_message_type);
UINT        _nxe_dhcp_set_interface_index(NX_DHCP *dhcp_ptr, UINT interface_index);
UINT        _nx_dhcp_set_interface_index(NX_DHCP *dhcp_ptr, UINT interface_index);  
UINT        _nxe_dhcp_clear_broadcast_flag(NX_DHCP *dhcp_ptr, UINT clear_flag);
UINT        _nx_dhcp_clear_broadcast_flag(NX_DHCP *dhcp_ptr, UINT clear_flag);  
UINT        _nxe_dhcp_user_option_add_callback_set(NX_DHCP *dhcp_ptr, UINT (*dhcp_user_option_add)(NX_DHCP *dhcp_ptr, UINT iface_index, UINT message_type, UCHAR *user_option_ptr, UINT *user_option_length));
UINT        _nx_dhcp_user_option_add_callback_set(NX_DHCP *dhcp_ptr, UINT (*dhcp_user_option_add)(NX_DHCP *dhcp_ptr, UINT iface_index, UINT message_type, UCHAR *user_option_ptr, UINT *user_option_length));
UINT        _nxe_dhcp_interface_clear_broadcast_flag(NX_DHCP *dhcp_ptr, UINT iface_index, UINT clear_flag);
UINT        _nx_dhcp_interface_clear_broadcast_flag(NX_DHCP *dhcp_ptr, UINT iface_index, UINT clear_flag);
UINT        _nxe_dhcp_interface_enable(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        _nx_dhcp_interface_enable(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        _nxe_dhcp_interface_disable(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        _nx_dhcp_interface_disable(NX_DHCP *dhcp_ptr, UINT iface_index); 
UINT        _nxe_dhcp_interface_decline(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        _nx_dhcp_interface_decline(NX_DHCP *dhcp_ptr, UINT iface_index); 
UINT        _nxe_dhcp_interface_force_renew(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        _nx_dhcp_interface_force_renew(NX_DHCP *dhcp_ptr, UINT iface_index); 
UINT        _nxe_dhcp_interface_release(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        _nx_dhcp_interface_release(NX_DHCP *dhcp_ptr, UINT iface_index); 
UINT        _nxe_dhcp_interface_reinitialize(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        _nx_dhcp_interface_reinitialize(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        _nxe_dhcp_interface_start(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        _nx_dhcp_interface_start(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        _nxe_dhcp_interface_stop(NX_DHCP *dhcp_ptr, UINT iface_index);
UINT        _nx_dhcp_interface_stop(NX_DHCP *dhcp_ptr, UINT iface_index);           
UINT        _nxe_dhcp_interface_request_client_ip(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG client_ip_address, UINT skip_discover_message);
UINT        _nx_dhcp_interface_request_client_ip(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG client_ip_address, UINT skip_discover_message); 
UINT        _nxe_dhcp_interface_send_request(NX_DHCP *dhcp_ptr, UINT iface_index, UINT dhcp_message_type);
UINT        _nx_dhcp_interface_send_request(NX_DHCP *dhcp_ptr, UINT iface_index, UINT dhcp_message_type);
UINT        _nxe_dhcp_interface_server_address_get(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG *server_address);
UINT        _nx_dhcp_interface_server_address_get(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG *server_address); 
UINT        _nxe_dhcp_interface_state_change_notify(NX_DHCP *dhcp_ptr,  VOID (*dhcp_interface_state_change_notify)(NX_DHCP *dhcp_ptr, UINT iface_index, UCHAR new_state));
UINT        _nx_dhcp_interface_state_change_notify(NX_DHCP *dhcp_ptr, VOID (*dhcp_interface_state_change_notify)(NX_DHCP *dhcp_ptr, UINT iface_index, UCHAR new_state));
UINT        _nxe_dhcp_interface_user_option_retrieve(NX_DHCP *dhcp_ptr, UINT iface_index, UINT option_request, UCHAR *destination_ptr, UINT *destination_size);
UINT        _nx_dhcp_interface_user_option_retrieve(NX_DHCP *dhcp_ptr, UINT iface_index, UINT option_request, UCHAR *destination_ptr, UINT *destination_size);  

#ifdef NX_DHCP_CLIENT_RESTORE_STATE           
UINT        _nxe_dhcp_resume(NX_DHCP *dhcp_ptr);
UINT        _nx_dhcp_resume(NX_DHCP *dhcp_ptr);
UINT        _nxe_dhcp_suspend(NX_DHCP *dhcp_ptr);
UINT        _nx_dhcp_suspend(NX_DHCP *dhcp_ptr);
UINT        _nxe_dhcp_client_get_record(NX_DHCP *dhcp_ptr, NX_DHCP_CLIENT_RECORD *record_ptr);
UINT        _nx_dhcp_client_get_record(NX_DHCP *dhcp_ptr, NX_DHCP_CLIENT_RECORD *record_ptr);
UINT        _nxe_dhcp_client_restore_record(NX_DHCP *dhcp_ptr, NX_DHCP_CLIENT_RECORD *record_ptr, ULONG time_elapsed);
UINT        _nx_dhcp_client_restore_record(NX_DHCP *dhcp_ptr, NX_DHCP_CLIENT_RECORD *record_ptr, ULONG time_elapsed);
UINT        _nxe_dhcp_client_update_time_remaining(NX_DHCP *dhcp_ptr, ULONG time_elapsed);
UINT        _nx_dhcp_client_update_time_remaining(NX_DHCP *dhcp_ptr, ULONG time_elapsed);
UINT        _nxe_dhcp_client_interface_get_record(NX_DHCP *dhcp_ptr, UINT iface_index,  NX_DHCP_CLIENT_RECORD *record_ptr);
UINT        _nx_dhcp_client_interface_get_record(NX_DHCP *dhcp_ptr, UINT iface_index,  NX_DHCP_CLIENT_RECORD *record_ptr);
UINT        _nxe_dhcp_client_interface_restore_record(NX_DHCP *dhcp_ptr, UINT iface_index, NX_DHCP_CLIENT_RECORD *record_ptr, ULONG time_elapsed);
UINT        _nx_dhcp_client_interface_restore_record(NX_DHCP *dhcp_ptr, UINT iface_index, NX_DHCP_CLIENT_RECORD *record_ptr, ULONG time_elapsed);
UINT        _nxe_dhcp_client_interface_update_time_remaining(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG time_elapsed);
UINT        _nx_dhcp_client_interface_update_time_remaining(NX_DHCP *dhcp_ptr, UINT iface_index, ULONG time_elapsed);
#endif /* NX_DHCP_CLIENT_RESTORE_STATE */ 

#endif /* NX_DHCP_SOURCE_CODE */


/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
}
#endif  /* __cplusplus */

#endif  /* NXD_DHCP_CLIENT_H */
