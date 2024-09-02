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
/*    nxd_dhcpv6_client.h                                 PORTABLE C      */ 
/*                                                           6.1.12       */
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
/*  07-29-2022     Yuxin Zhou               Modified comment(s), supported*/
/*                                            adding user options,        */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/

#ifndef NXD_DHCPV6_CLIENT_H
#define NXD_DHCPV6_CLIENT_H

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */

#ifdef   __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_api.h"

/*  Enable support for DHCPv6 client state preserved between reboots  */ 
/*
#define NX_DHCPV6_CLIENT_RESTORE_STATE  
*/


/* Define the DHCPv6 ID "DHCPV6" that is used to mark the DHCPv6 structure as created.  */

#define NX_DHCPV6_ID                                    0x44484336UL


/* Set the Client lease time. An infinate lease time is not recommended by the RFC 
   unless the Client requires a permanent IP address.  Most servers will likely not
   grant an infinite IP address lease. */

#define NX_DHCPV6_INFINITE_LEASE                        0xffffffffUL
#define NX_DHCPV6_MULTICAST_MASK                        0xff000000UL         
    
typedef enum 
{
    NX_DHCPV6_DUID_TYPE_LINK_TIME =                     1,
    NX_DHCPV6_DUID_TYPE_VENDOR_ASSIGNED, 
    NX_DHCPV6_DUID_TYPE_LINK_ONLY

} NX_DHCPV6_DUID_TYPE;


/* Define the Hardware types.  */
#define NX_DHCPV6_CLIENT_HARDWARE_TYPE_ETHERNET         1
#define NX_DHCPV6_CLIENT_HARDWARE_TYPE_EUI_64           27

/* NX_DHCPV6_HW_TYPE_IEEE_802 is defined as 1 to indicate Ethernet hardware type in old releases, for backward compatibility.

   Note: NX_DHCPV6_HW_TYPE_IEEE_802 will be deprecated by NX_DHCPV6_CLIENT_HARDWARE_TYPE_ETHERNET
         in future releases, should use above symbols to define hardware types.
*/
#define NX_DHCPV6_HW_TYPE_IEEE_802                      1


/* Define approximate time since Jan 1, 2000 for computing DUID time. This will form the 
   basis for the DUID time ID field.  */

#define SECONDS_SINCE_JAN_1_2000_MOD_32                 2563729999UL

/* Define the value for IA address.  */

#define NX_DHCPV6_REMOVE_ALL_IA_ADDRESS                 0xFFFFFFFF

/* Define the DHCPv6 Message Types.  */

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

/* Define the DHCPv6 Options, RFC3315, RFC 3646 and RFC 4242,etc.  */

#define NX_DHCPV6_OP_CLIENT_ID                          1         /* Client DUID (DHCP unique identifier) */
#define NX_DHCPV6_OP_SERVER_ID                          2         /* Server DUID (DHCP unique identifier) */
#define NX_DHCPV6_OP_IA_NA                              3         /* Identity association for non temporary addresses */
#define NX_DHCPV6_OP_IA_TA                              4         /* Identity association for temporary addresses */  
#define NX_DHCPV6_OP_IA_ADDRESS                         5         /* Address associated with IA_NA or IA_TA */
#define NX_DHCPV6_OP_OPTION_REQUEST                     6         /* Identifies a list of options */
#define NX_DHCPV6_OP_PREFERENCE                         7         /* Server's means of affecting Client choice of servers. */
#define NX_DHCPV6_OP_ELAPSED_TIME                       8         /* Duration of Client exchange with DHCPv6 server  */
#define NX_DHCPV6_OP_RELAY_MESSAGE                      9         /* Not in use in NetX DHCPV6 */
#define NX_DHCPV6_OP_AUTHENTICATION                     11        /* Not in use in NetX DHCPV6 */
#define NX_DHCPV6_OP_SERVER_UNICAST                     12        /* Server ok's allowing the client to address it in Unicast */
#define NX_DHCPV6_OP_STATUS_CODE                        13        /* Status option.  */
#define NX_DHCPV6_OP_RAPID_COMMIT                       14        /* Rapid commit option.  */
#define NX_DHCPV6_OP_USER_CLASS                         15        /* User calss option.  */
#define NX_DHCPV6_OP_VENDOR_CLASS                       16        /* Vendor Class option.  */
#define NX_DHCPV6_OP_VENDOR_SPEC_INFO                   17        /* Vendor specific information option.  */
#define NX_DHCPV6_OP_INTERFACE_ID                       18        /* Interface ID option.  */
#define NX_DHCPV6_OP_RCONF_MESSAGE                      19        /* Reconfigure message option.  */
#define NX_DHCPV6_OP_RCONF_ACCEPT                       20        /* Reconfigure accept option.  */

#define NX_DHCPV6_OP_DNS_SERVER                         23        /* Dns servers option.  */
#define NX_DHCPV6_OP_DOMAIN_NAME                        24        /* Domain names list option.  */

#define NX_DHCPV6_OP_SNTP_SERVER                        31        /* Network time protocol servers option.  */
#define NX_DHCPV6_OP_CLIENT_FQDN                        39        /* Client Fully Qualified Domain Name Option.  */
#define NX_DHCPV6_OP_NEW_POSIX_TIMEZONE                 41        /* New timezone POSIX string index option.  */
#define NX_DHCPV6_OP_NEW_TZDB_TIMEZONE                  42        /* New timezone database string option.  */

/* Define the Error Status Code Location.  */
#define NX_DHCPV6_ERROR_STATUS_CODE_IN_OPTION_FIELD     0         /* An Error Status Code option appears in options field of DHCP message.  */
#define NX_DHCPV6_ERROR_STATUS_CODE_IN_IA_NA            1         /* An Error Status Code option appears in options field of IA_NA option.  */
#define NX_DHCPV6_ERROR_STATUS_CODE_IN_IA_ADDRESS       2         /* An Error Status Code option appears in options field of IA_ADDRESS option .  */

/* Define the options included in the reply message.  */

#define NX_DHCPV6_INCLUDE_CLIENT_ID_OPTION              0x00000001    /* The reply message includes a Client Identifier option. */
#define NX_DHCPV6_INCLUDE_SERVER_ID_OPTION              0x00000002    /* The reply message includes a Server Identifier option. */
#define NX_DHCPV6_INCLUDE_IA_NA_OPTION                  0x00000004    /* The reply message includes a IA_NA option. */
#define NX_DHCPV6_INCLUDE_IA_ADDRESS_OPTION             0x00000008    /* The reply message includes a IA address option. */
#define NX_DHCPV6_INCLUDE_OPTION_REQUEST_OPTION         0x00000010    /* The reply message includes a option request option. */
#define NX_DHCPV6_INCLUDE_PREFERENCE_OPTION             0x00000020    /* The reply message includes a preference option. */
#define NX_DHCPV6_INCLUDE_ELAPSED_TIME_OPTION           0x00000040    /* The reply message includes a elapsed time option. */
#define NX_DHCPV6_INCLUDE_RELAY_MESSAGE_OPTION          0x00000080    /* The reply message includes a relay message option. */
#define NX_DHCPV6_INCLUDE_AUTHENTICATION_OPTION         0x00000100    /* The reply message includes a authentication option. */
#define NX_DHCPV6_INCLUDE_SERVER_UNICAST_OPTION         0x00000200    /* The reply message includes a server unicast option. */
#define NX_DHCPV6_INCLUDE_STATUS_SUCCESS_OPTION         0x00000400    /* The reply message includes a status code option with success. */
#define NX_DHCPV6_INCLUDE_STATUS_UNSPEC_FAIL_OPTION     0x00000800    /* The reply message includes a status code option with failure. */
#define NX_DHCPV6_INCLUDE_STATUS_NO_ADDR_AVAIL_OPTION   0x00001000    /* The reply message includes a status code option with no addresses available. */
#define NX_DHCPV6_INCLUDE_STATUS_NO_BIND_OPTION         0x00002000    /* The reply message includes a status code option with unavailable. */
#define NX_DHCPV6_INCLUDE_STATUS_NOT_ONLINK_OPTION      0x00004000    /* The reply message includes a status code option with not onlink. */
#define NX_DHCPV6_INCLUDE_STATUS_USE_MULTICAST_OPTION   0x00008000    /* The reply message includes a status code option with use multicast. */
#define NX_DHCPV6_INCLUDE_RAPID_COMMIT_OPTION           0x00010000    /* The reply message includes a rapid commit option. */
#define NX_DHCPV6_INCLUDE_USER_CLASS_OPTION             0x00020000    /* The reply message includes a user class option. */
#define NX_DHCPV6_INCLUDE_VENDOR_CLASS_OPTION           0x00040000    /* The reply message includes a vendor class option. */
#define NX_DHCPV6_INCLUDE_VENDOR_SPEC_INFO_OPTION       0x00080000    /* The reply message includes a vendor specifc information option. */
#define NX_DHCPV6_INCLUDE_INTERFACE_ID_OPTION           0x00100000    /* The reply message includes a interface Id option. */
#define NX_DHCPV6_INCLUDE_RECONF_MESSAGE_OPTION         0x00200000    /* The reply message includes a reconfigure message option. */
#define NX_DHCPV6_INCLUDE_RECONF_ACCEPT_OPTION          0x00400000    /* The reply message includes a reconfigure accept option. */
#define NX_DHCPV6_INCLUDE_DNS_SERVER_OPTION             0x00800000    /* The reply message includes a dns server option. */
#define NX_DHCPV6_INCLUDE_DOMAIN_NAME_OPTION            0x01000000    /* The reply message includes a domain name option. */
#define NX_DHCPV6_INCLUDE_SNTP_SERVER_OPTION            0x02000000    /* The reply message includes a network time protocol servers option. */
#define NX_DHCPV6_INCLUDE_NEW_POSIX_TIIMEZONE_OPTION    0x04000000    /* The reply message includes a new timezone POSIX string index option. */
#define NX_DHCPV6_INCLUDE_CLIENT_FQDN_OPTION            0x08000000    /* The reply message includes a client FQDN option. */
             

/* Define internal DHCPv6 option flags. */

#define NX_DHCPV6_DNS_SERVER_OPTION                     0x00000001UL    /* Option code for requesting DNS server IP address  */
#define NX_DHCPV6_DOMAIN_NAME_OPTION                    0x00000002UL    /* Option code for requesting domain name. */
#define NX_DHCPV6_SNTP_SERVER_OPTION                    0x00000004UL    /* Option code for requesting time server IP address. */
#define NX_DHCPV6_NEW_POSIX_TIMEZONE_OPTION             0x00000008UL    /* Option code for requesting Time zone. */               
#define NX_DHCPV6_CLIENT_FQDN_OPTION                    0x00000010UL    /* Option code for requesting FQDN.  */


/* RFC defined DHCPv6 server status codes */

#define NX_DHCPV6_SUCCESS                               0           /* Server indicates Client DHCPv6 request is granted. */
#define NX_DHCPV6_UNSPECIFIED_FAILURE                   1           /* Unspecified reason e.g. not found in RFC 3315 */
#define NX_DHCPV6_NO_ADDRESS_AVAILABLE                  2           /* Server unable to assign IP address because none are available. */
#define NX_DHCPV6_NO_BINDING                            3           /* Client record (binding) unavailable */
#define NX_DHCPV6_NOT_ON_LINK                           4           /* Client's IPv6 address is not on the Server link */
#define NX_DHCPV6_USE_MULTICAST                         5           /* Server indicates Client must use multicast ALL_SERVERS address to get IP address */

/* Internal DHCPv6 Client status codes */

#define NX_DHCPV6_STATE_INIT                            1           /* Client state with no bound IP address */
#define NX_DHCPV6_STATE_SENDING_SOLICIT                 2           /* Client sends Sollicit to identify a DHCP server */
#define NX_DHCPV6_STATE_SENDING_REQUEST                 3           /* Address requested, Client initiating a request after receiving server advertisement */
#define NX_DHCPV6_STATE_SENDING_RENEW                   4           /* Address established, Client is initiating a renew request */
#define NX_DHCPV6_STATE_SENDING_REBIND                  5           /* Address established, Client is initiating a rebind request */
#define NX_DHCPV6_STATE_SENDING_DECLINE                 6           /* Address was established but Client can't use it e.g. duplicate address check failed. */
#define NX_DHCPV6_STATE_SENDING_CONFIRM                 7           /* Client IP Address is established but Client requires confirmation its still ok */
#define NX_DHCPV6_STATE_SENDING_INFORM_REQUEST          8           /* Client IP Address is established but Client requests information other than IP address */
#define NX_DHCPV6_STATE_SENDING_RELEASE                 9           /* Requesting an IP address release of a recently assigned IP address. */
#define NX_DHCPV6_STATE_BOUND_TO_ADDRESS                15          /* Client is bound to an assigned address; DHCP Client task is basically idle. */

/* Internal DHCPv6 Client address status codes. */

#define NX_DHCPV6_IA_ADDRESS_STATE_INVALID              0           /* The IA does not inlcude IPv6 address. */
#define NX_DHCPV6_IA_ADDRESS_STATE_INITIAL              1           /* The IA inlcude one IPv6 address,but the DHCPv6 interactive is not complete. */
#define NX_DHCPV6_IA_ADDRESS_STATE_DAD_TENTATIVE        2           /* After DHCPv6 interactive, NetX pefrome the DAD to check this IPv6 address.  */
#define NX_DHCPV6_IA_ADDRESS_STATE_DAD_FAILURE          3           /* DAD process failure,the DHCPv6 Client should send DECLINE message.*/
#define NX_DHCPV6_IA_ADDRESS_STATE_VALID                4           /* The global IP address is valid and set it to the IP instance. */          

/* Internal DHCPv6 event flags.  These events are processed by the Client DHCPv6 thread. */

#define NX_DHCPV6_ALL_EVENTS                            0xFFFFFFFFUL    /* All Client DHCPv6 event flags */
#define NX_DHCPV6_DAD_FAILURE_EVENT                     0x00000001UL    /* The DHCPv6 Client perform DAD failure.  */
#define NX_DHCPV6_DAD_SUCCESSFUL_EVENT                  0x00000002UL    /* The DHCPv6 Client perform DAD failure.  */

/* The behavior of a DHCPv6 client that implements the Client FQDN option. RFC4704, Section5, Page7.  */

#define NX_DHCPV6_CLIENT_DESIRES_UPDATE_AAAA_RR         0           /* DHCPv6 Client choose to updating the FQDN-to-IPv6 address mapping for FQDN and address(es) used by the client.  */
#define NX_DHCPV6_CLIENT_DESIRES_SERVER_DO_DNS_UPDATE   1           /* DHCPv6 Client choose to updating the FQDN-to-IPv6 address mapping for FQDN and address(es) used by the client to the server.  */
#define NX_DHCPV6_CLIENT_DESIRES_NO_SERVER_DNS_UPDATE   2           /* DHCPv6 Client choose to request that the server perform no DNS updatest on its behalf.  */   


/* RFC mandated DHCPv6 client and server ports.  */

#define NX_DHCPV6_SERVER_UDP_PORT                       547
#define NX_DHCPV6_CLIENT_UDP_PORT                       546


/* The solicit mode.  */

#define NX_DHCPV6_SOLICIT_NORMAL                        1
#define NX_DHCPV6_SOLICIT_RAPID                         2


/* Define name compression masks.  */

#define NX_DHCPV6_LABEL_MAX                             63       /* Maximum Label (between to dots) size.  */
#define NX_DHCPV6_COMPRESS_MASK                         0xc0
#define NX_DHCPV6_COMPRESS_VALUE                        0xc0
#define NX_DHCPV6_POINTER_MASK                          0xc000


/* Internal error codes for DHCPv6 Client services.  */

#define NX_DHCPV6_TASK_SUSPENDED                        0xE90    /* DHCPv6 task suspended by host application. */
#define NX_DHCPV6_ALREADY_STARTED                       0xE91    /* DHCPv6 already started when API called to start it. */
#define NX_DHCPV6_NOT_STARTED                           0xE92    /* DHCPv6 was not started when API was called  */ 
#define NX_DHCPV6_PARAM_ERROR                           0xE93    /* Invalid non pointer input to API */     
#define NX_DHCPV6_NOT_BOUND                             0xE94    /* DHCPv6 was not bound when API was called.  */

#define NX_DHCPV6_INVALID_CLIENT_DUID                   0xE95    /* Client DUID received from Server with invalid data or mismatches Client server DUID on record. */
#define NX_DHCPV6_INVALID_SERVER_DUID                   0xE96    /* Server DUID received by Client has bad syntax or missing data*/
#define NX_DHCPV6_MESSAGE_MISSING_DUID                  0xE97    /* Client receives a message type missing server or client DUID. */
#define NX_DHCPV6_UNSUPPORTED_DUID_TYPE                 0xE98    /* Client configuration involves a DUID type not supported by this API. */
#define NX_DHCPV6_UNSUPPORTED_DUID_HW_TYPE              0xE99    /* Client configuration involves a network hardware type not supported by this API. */
#define NX_DHCPV6_NO_DUID_OPTION                        0xE9A    /* The reply message does not include server or client identifier option. */
#define NX_DHCPV6_NO_RAPID_COMMIT_OPTION                0xE9B    /* The reply message does not iclude rapid commit option. */    
#define NX_DHCPV6_EQUAL_OR_LESS_PREF_VALUE              0xE9C    /* The current preference value is equal to or less than the recorded preference valude. */

#define NX_DHCPV6_INVALID_IANA_TIME                     0xEA0    /* Server IA-NA option T1 vs T2 address lease time is invalid. */
#define NX_DHCPV6_MISSING_IANA_OPTION                   0xEA1    /* Client received IA address option not belonging to an IA block */
#define NX_DHCPV6_BAD_IANA_ID                           0xEA2    /* Server IA-NA option does not contain the Client's original IA-NA ID. */
#define NX_DHCPV6_INVALID_IANA_DATA                     0xEA3    /* Server IA-NA option block has bad syntax or missing data */
#define NX_DHCPV6_INVALID_IA_ADDRESS                    0xEA4    /* Client inquiring about an unknown IA address */
#define NX_DHCPV6_INVALID_IA_DATA                       0xEA5    /* Server IA address option block has bad syntax or missing data */
#define NX_DHCPV6_INVALID_IA_TIME                       0xEA6    /* Server IA option preferred vs valid lease time is invalid. */
#define NX_DHCPV6_INVALID_PREF_DATA                     0xEA7    /* Client received Preference block with missing data or bad syntax */
#define NX_DHCPV6_INCOMPLETE_OPTION_BLOCK               0xEA8    /* Empty option block data; either zero length or zero option parsed. */    
#define NX_DHCPV6_MISSING_REQUIRED_OPTIONS              0xEA9    /* Cannot start the DHCPv6 Client because required options are missing e.g. IANA, DUID etc */
#define NX_DHCPV6_INVALID_OPTION_DATA                   0xEAA    /* Client received option data with missing data or bad syntax */
#define NX_DHCPV6_UNKNOWN_OPTION                        0xEAB    /* Client received an unknown or unsupported option from server */
#define NX_DHCPV6_INVALID_SERVER_PACKET                 0xEAC    /* Server reply invalid e.g. bad port, invalid DHCP header or invalid reply message.  */
#define NX_DHCPV6_IA_ADDRESS_NOT_VALID                  0xEAD    /* Client not assigned an IP address from the DHCPv6 Server */
#define NX_DHCPV6_REACHED_MAX_IA_ADDRESS                0xEAE    /* IA address exceeds the maximum IAs specified by NX_DHCPV6_MAX_IA_ADDRESS.  */
#define NX_DHCPV6_IA_ADDRESS_ALREADY_EXIST              0xEAF    /* The IA address already added to the Client.  */

#define NX_DHCPV6_UNKNOWN_PROCESS_STATE                 0xEB0    /* Internal DHCPv6 state machine in an unknown state */
#define NX_DHCPV6_ILLEGAL_MESSAGE_TYPE                  0xEB1    /* Client receives a message type intended for a DHCPv6 server e.g. REQUEST or CONFIRM */
#define NX_DHCPV6_UNKNOWN_MSG_TYPE                      0xEB2    /* NetX DHCPv6 receives an unknown message type  */
#define NX_DHCPV6_BAD_TRANSACTION_ID                    0xEB3    /* Client received message with bad transaction ID */
#define NX_DHCPV6_BAD_IPADDRESS_ERROR                   0xEB4    /* Unable to parse a valid IPv6 address from specified data buffer  */
#define NX_DHCPV6_PROCESSING_ERROR                      0xEB5    /* Server packet size received out of synch with NetX packet length - no assignment of blame */
#define NX_DHCPV6_INSUFFICIENT_PACKET_PAYLOAD           0xEB6    /* Client DHCPv6 message will not fit in Client packet pool packet buffer. */
#define NX_DHCPV6_INVALID_DATA_SIZE                     0xEB7    /* Attempting to parse too large a data object to/from DHCPv6 request. */
#define NX_DHCPV6_ADDRESS_MISMATCH                      0xEB8    /* Client IPv6 address index into the IP table is incorrect (addresses do not match). */

#define NX_DHCPV6_REACHED_MAX_RETRANSMISSION_COUNT      0xEC0    /* No response from server after maximum number of retries. */
#define NX_DHCPV6_REACHED_MAX_RETRANSMISSION_TIMEOUT    0xEC1    /* No response from server after maximum retry timeout. */


/* Define DHCPv6 timeout for checking DHCPv6 flag status. */
#define NX_DHCPV6_TIME_INTERVAL                         (NX_IP_PERIODIC_RATE)   


/* Define the conversion between timer ticks and seconds (processor dependent). */
#define NX_DHCPV6_TICKS_PER_SECOND                      (NX_IP_PERIODIC_RATE)


/* Define the max name size. RFC1035, Section 3.1.  */
#define NX_DHCPV6_MAX_NAME_SIZE                         255


/* Define the DHCP stack priority.  */

#ifndef NX_DHCPV6_THREAD_PRIORITY
#define NX_DHCPV6_THREAD_PRIORITY                       2
#endif


/* Define the time out option to obtain a DHCPv6 Client mutex lock. If the 
   the Client appears to be locking up, this can be set to a finite value
   for debugging as well as restore responsiveness to the Client */

#ifndef NX_DHCPV6_MUTEX_WAIT
#define NX_DHCPV6_MUTEX_WAIT                            TX_WAIT_FOREVER 
#endif


/* Define DHCPv6 Client record parameters */

/* Define the timer interval for the IP lifetime timer in seconds.  */

#ifndef NX_DHCPV6_IP_LIFETIME_TIMER_INTERVAL
#define NX_DHCPV6_IP_LIFETIME_TIMER_INTERVAL            1
#endif


/* Define the timer interval for the session duration timer in seconds.  */

#ifndef NX_DHCPV6_SESSION_TIMER_INTERVAL
#define NX_DHCPV6_SESSION_TIMER_INTERVAL                1
#endif


/* Define the number of DNS name servers the Client will store. */

#ifndef NX_DHCPV6_NUM_DNS_SERVERS
#define NX_DHCPV6_NUM_DNS_SERVERS                       2
#endif


/* Define the number of time servers the Client will store. */

#ifndef NX_DHCPV6_NUM_TIME_SERVERS
#define NX_DHCPV6_NUM_TIME_SERVERS                      1
#endif


/* Define the buffer size for storing the DHCPv6 Client domain name. */

#ifndef NX_DHCPV6_DOMAIN_NAME_BUFFER_SIZE
#define NX_DHCPV6_DOMAIN_NAME_BUFFER_SIZE               32
#endif


/* Define the buffer size for storing the DHCPv6 Client time zone e.g. CET, PST etc. */

#ifndef NX_DHCPV6_TIME_ZONE_BUFFER_SIZE
#define NX_DHCPV6_TIME_ZONE_BUFFER_SIZE                 16
#endif

/* Define the amount of packet payload to store DHCPv6 server messages. */

#ifndef NX_DHCPV6_MAX_MESSAGE_SIZE      
#define NX_DHCPV6_MAX_MESSAGE_SIZE                      100
#endif  

/* Define the amount of IA addresses to store IPv6 addresses. */

#ifndef NX_DHCPV6_MAX_IA_ADDRESS      
#define NX_DHCPV6_MAX_IA_ADDRESS                        1
#endif  


/* DHCPv6 Client Network Configuration */

/* Define the generic time out option for NetX operations (packet allocate, packet send.  */

#ifndef NX_DHCPV6_PACKET_TIME_OUT
#define NX_DHCPV6_PACKET_TIME_OUT                       (3 * NX_DHCPV6_TICKS_PER_SECOND)
#endif


/* Define UDP socket type of service.  */

#ifndef NX_DHCPV6_TYPE_OF_SERVICE
#define NX_DHCPV6_TYPE_OF_SERVICE                       NX_IP_NORMAL
#endif

 
/* Define the number of routers a UDP packet passes before it is discarded. */

#ifndef NX_DHCPV6_TIME_TO_LIVE
#define NX_DHCPV6_TIME_TO_LIVE                          0x80
#endif

/* Define the stored packets in the UDP socket queue. */

#ifndef NX_DHCPV6_QUEUE_DEPTH
#define NX_DHCPV6_QUEUE_DEPTH                           5
#endif


/* Define the initial retransmission timeout in timer ticks for DHCPv6 messages. 
   For no limit on the retransmission timeout set to 0, for no limit
   on the retries, set to 0. 

   Note that regardless of length of timeout or number of retries, when the IP address 
   valid lifetime expires, the Client can no longer use its global IP address 
   assigned by the DHCPv6 Server. */

#ifndef NX_DHCPV6_FIRST_SOL_MAX_DELAY                 
#define NX_DHCPV6_FIRST_SOL_MAX_DELAY                   (1 * NX_DHCPV6_TICKS_PER_SECOND) 
#endif

#ifndef NX_DHCPV6_INIT_SOL_TRANSMISSION_TIMEOUT
#define NX_DHCPV6_INIT_SOL_TRANSMISSION_TIMEOUT         (1 * NX_DHCPV6_TICKS_PER_SECOND) 
#endif

#ifndef NX_DHCPV6_MAX_SOL_RETRANSMISSION_TIMEOUT 
#define NX_DHCPV6_MAX_SOL_RETRANSMISSION_TIMEOUT        (120 * NX_DHCPV6_TICKS_PER_SECOND) 
#endif

#ifndef NX_DHCPV6_MAX_SOL_RETRANSMISSION_COUNT
#define NX_DHCPV6_MAX_SOL_RETRANSMISSION_COUNT           0
#endif

#ifndef NX_DHCPV6_MAX_SOL_RETRANSMISSION_DURATION
#define NX_DHCPV6_MAX_SOL_RETRANSMISSION_DURATION        0
#endif

#ifndef NX_DHCPV6_INIT_REQ_TRANSMISSION_TIMEOUT
#define NX_DHCPV6_INIT_REQ_TRANSMISSION_TIMEOUT         (1 * NX_DHCPV6_TICKS_PER_SECOND) 
#endif

#ifndef NX_DHCPV6_MAX_REQ_RETRANSMISSION_TIMEOUT
#define NX_DHCPV6_MAX_REQ_RETRANSMISSION_TIMEOUT        (30 * NX_DHCPV6_TICKS_PER_SECOND) 
#endif

#ifndef NX_DHCPV6_MAX_REQ_RETRANSMISSION_COUNT
#define NX_DHCPV6_MAX_REQ_RETRANSMISSION_COUNT           10
#endif


#ifndef NX_DHCPV6_MAX_REQ_RETRANSMISSION_DURATION        
#define NX_DHCPV6_MAX_REQ_RETRANSMISSION_DURATION        0
#endif

#ifndef NX_DHCPV6_INIT_RENEW_TRANSMISSION_TIMEOUT
#define NX_DHCPV6_INIT_RENEW_TRANSMISSION_TIMEOUT       (10 * NX_DHCPV6_TICKS_PER_SECOND)     
#endif

#ifndef NX_DHCPV6_MAX_RENEW_RETRANSMISSION_TIMEOUT
#define NX_DHCPV6_MAX_RENEW_RETRANSMISSION_TIMEOUT      (600 * NX_DHCPV6_TICKS_PER_SECOND)  
#endif

#ifndef NX_DHCPV6_MAX_RENEW_RETRANSMISSION_COUNT
#define NX_DHCPV6_MAX_RENEW_RETRANSMISSION_COUNT         0
#endif

#ifndef NX_DHCPV6_INIT_REBIND_TRANSMISSION_TIMEOUT
#define NX_DHCPV6_INIT_REBIND_TRANSMISSION_TIMEOUT      (10 * NX_DHCPV6_TICKS_PER_SECOND)     
#endif

#ifndef NX_DHCPV6_MAX_REBIND_RETRANSMISSION_TIMEOUT
#define NX_DHCPV6_MAX_REBIND_RETRANSMISSION_TIMEOUT     (600 * NX_DHCPV6_TICKS_PER_SECOND)  
#endif

#ifndef NX_DHCPV6_MAX_REBIND_RETRANSMISSION_COUNT
#define NX_DHCPV6_MAX_REBIND_RETRANSMISSION_COUNT        0 
#endif

#ifndef NX_DHCPV6_INIT_RELEASE_TRANSMISSION_TIMEOUT
#define NX_DHCPV6_INIT_RELEASE_TRANSMISSION_TIMEOUT     (1 * NX_DHCPV6_TICKS_PER_SECOND)
#endif

#ifndef NX_DHCPV6_MAX_RELEASE_RETRANSMISSION_TIMEOUT
#define NX_DHCPV6_MAX_RELEASE_RETRANSMISSION_TIMEOUT     0 
#endif

#ifndef NX_DHCPV6_MAX_RELEASE_RETRANSMISSION_COUNT
#define NX_DHCPV6_MAX_RELEASE_RETRANSMISSION_COUNT       5  
#endif

#ifndef NX_DHCPV6_MAX_RELEASE_RETRANSMISSION_DURATION
#define NX_DHCPV6_MAX_RELEASE_RETRANSMISSION_DURATION    0
#endif

#ifndef NX_DHCPV6_INIT_DECLINE_TRANSMISSION_TIMEOUT
#define NX_DHCPV6_INIT_DECLINE_TRANSMISSION_TIMEOUT     (1 * NX_DHCPV6_TICKS_PER_SECOND)
#endif

#ifndef NX_DHCPV6_MAX_DECLINE_RETRANSMISSION_TIMEOUT
#define NX_DHCPV6_MAX_DECLINE_RETRANSMISSION_TIMEOUT     0
#endif

#ifndef NX_DHCPV6_MAX_DECLINE_RETRANSMISSION_COUNT
#define NX_DHCPV6_MAX_DECLINE_RETRANSMISSION_COUNT       5  
#endif

#ifndef NX_DHCPV6_MAX_DECLINE_RETRANSMISSION_DURATION
#define NX_DHCPV6_MAX_DECLINE_RETRANSMISSION_DURATION    0
#endif

#ifndef NX_DHCPV6_FIRST_CONFIRM_MAX_DELAY
#define NX_DHCPV6_FIRST_CONFIRM_MAX_DELAY               (1 * NX_DHCPV6_TICKS_PER_SECOND)
#endif

#ifndef NX_DHCPV6_INIT_CONFIRM_TRANSMISSION_TIMEOUT
#define NX_DHCPV6_INIT_CONFIRM_TRANSMISSION_TIMEOUT     (1 * NX_DHCPV6_TICKS_PER_SECOND)
#endif

#ifndef NX_DHCPV6_MAX_CONFIRM_RETRANSMISSION_TIMEOUT
#define NX_DHCPV6_MAX_CONFIRM_RETRANSMISSION_TIMEOUT    (4 * NX_DHCPV6_TICKS_PER_SECOND)
#endif

#ifndef NX_DHCPV6_MAX_CONFIRM_RETRANSMISSION_COUNT
#define NX_DHCPV6_MAX_CONFIRM_RETRANSMISSION_COUNT       0  
#endif

#ifndef NX_DHCPV6_MAX_CONFIRM_RETRANSMISSION_DURATION
#define NX_DHCPV6_MAX_CONFIRM_RETRANSMISSION_DURATION    10
#endif

#ifndef NX_DHCPV6_FIRST_INFORM_MAX_DELAY
#define NX_DHCPV6_FIRST_INFORM_MAX_DELAY                (1 * NX_DHCPV6_TICKS_PER_SECOND)
#endif

#ifndef NX_DHCPV6_INIT_INFORM_TRANSMISSION_TIMEOUT
#define NX_DHCPV6_INIT_INFORM_TRANSMISSION_TIMEOUT      (1 * NX_DHCPV6_TICKS_PER_SECOND)
#endif

#ifndef NX_DHCPV6_MAX_INFORM_RETRANSMISSION_TIMEOUT
#define NX_DHCPV6_MAX_INFORM_RETRANSMISSION_TIMEOUT     (120 * NX_DHCPV6_TICKS_PER_SECOND)
#endif

#ifndef NX_DHCPV6_MAX_INFORM_RETRANSMISSION_COUNT
#define NX_DHCPV6_MAX_INFORM_RETRANSMISSION_COUNT        0 
#endif

#ifndef NX_DHCPV6_MAX_INFORM_RETRANSMISSION_DURATION
#define NX_DHCPV6_MAX_INFORM_RETRANSMISSION_DURATION     0
#endif


/* Define the Identity Association Internet Address option structure  */
typedef struct NX_DHCPV6_IA_ADDRESS_STRUCT
{

    USHORT          nx_op_code;                    /* IA internet address option code is 5 */
    USHORT          nx_option_length;              /* Length of the IA address option data = 24 not including length and op code field*/
    NXD_ADDRESS     nx_global_address;             /* Assigned Host IPv6 address */
    ULONG           nx_preferred_lifetime;         /* Server's preference for IPv6 address T1 life time for itself */
    ULONG           nx_valid_lifetime;             /* Server's assigned valid time for T2 for any server  */
    UINT            nx_address_status;             /* Indicates if the global address is registered and validated. */
    UINT            nx_address_map;                /* Map the IPv6 address with client, indicates if the IPv6 address already exists or not.  */

} NX_DHCPV6_IA_ADDRESS;

/* Define the Option status structure  */
typedef struct NX_DHCPV6_OP_STATUS_STRUCT
{

    USHORT          nx_op_code;                    /* IA address option code is 5 */
    USHORT          nx_option_length;              /* Length of the IA address option data = 24 not including length and op code field*/
} NX_DHCPV6_OP_STATUS;


/* Define the Preference Option structure  */
typedef struct NX_DHCPV6_PREFERENCE_STRUCT
{

    USHORT          nx_op_code;                    /* IA address option code is 5 */
    USHORT          nx_option_length;              /* Length of the IA address option data = 24 not including length and op code field*/
    USHORT          nx_pref_value;                 /* Assigned Host IPv6 address */

} NX_DHCPV6_PREFERENCE;


/* Define the Identity Association for Permanent ("Non Temporary" in RFC) address */

typedef struct NX_DHCPV6_IA_NA_STRUCT
{

    USHORT              nx_op_code;             /* IA NA address option code is 3 */
    USHORT              nx_option_length;       /* 12 + length of variable length fields in IA_NA option . */
    ULONG               nx_IA_NA_id;            /* IANA identifier; must be unique among all client IANA's. Must be the same on restart per IANA */
    ULONG               nx_T1;                  /* Time client can extend time before address lifetime expires from the server it got it from; applies to all addresses in IA_NA. */
    ULONG               nx_T2;                  /* Same as T1 except this is when the client will request REBIND from another server. */

} NX_DHCPV6_IA_NA;


/* Define DHCPv6 Unique Identifier (DUID); both Client and Server must send messages with their own DUID. */

typedef struct NX_DHCPV6_DUID_STRUCT
{

    USHORT            nx_op_code;                 /* Client DUID option code is 1; Server DUID code is 2  */
    USHORT            nx_option_length;           /* Option length = 14 not including length and op code field; */
    USHORT            nx_duid_type;               /* 3 main types: hw; hw + time; vendor assigned ID (not supported here); requires DUID be stored in non volatile storage */
    USHORT            nx_hardware_type;           /* Only if LL/LLT type. Hardware type specified by IANA/RFC 826 e.g. IEEE 802; network byte order */
    ULONG             nx_duid_time;               /* Only if LLT type. Time based on when DUID generated; network byte order. */
    ULONG             nx_link_layer_address_msw;  /* Only if LL/LLT type. Pointer to Unique link layer address - most significant word (2/4 bytes)*/
    ULONG             nx_link_layer_address_lsw;  /* Only if LL/LLT type. Pointer to Unique link layer address - least significant word (4 bytes) */

} NX_DHCPV6_DUID;


/* Define the elapsed time option structure.  This contains the length of the Client Server session. */

typedef struct NX_DHCPV6_ELAPSED_TIME_STRUCT
{

    USHORT            nx_op_code;                /* Elapsed time option code = 8 not including length and op code field. */
    USHORT            nx_op_length;              /* Length of time data = 2. */
    USHORT            nx_session_time;           /* Time of DHCP session e.g. first msg elapsed time is zero. */

} NX_DHCPV6_ELAPSED_TIME;


/* Define the Message Option structure. Each message from the Client must have a unique message ID. */

typedef struct NX_DHCPV6_MESSAGE_HDR_STRUCT 
{

    USHORT             nx_message_type;           /* Message type (1 byte) */
    USHORT             nx_reserved;               /* Reserved.  */
    ULONG              nx_message_xid;            /* Message transaction ID (3 bytes)*/
} NX_DHCPV6_MESSAGE_HDR;

/* Define the option request structure. This is how the Client requests information other than global IP address.  
   It can ask for domain name, DNS server, time zone, time server and other options. */

typedef struct NX_DHCPV6_OPTIONREQUEST_STRUCT
{
    USHORT             nx_op_code;                /* Option Request code  = 6*/
    USHORT             nx_option_length;          /* Length in bytes of option data = 2 * number of requests */
    USHORT             nx_op_request;             /* e.g. DNS server = 23, ... */

} NX_DHCPV6_OPTIONREQUEST;

/* Define the DHCPv6 Client FQDN Option structure,  */

typedef struct NX_DHCPV6_CLIENT_FQDN_STRUCT
{
    USHORT             nx_op_code;                /* Option Client FQDN code = 39*/
    USHORT             nx_op_length;              /* 1 + length of domain name.   */
    UCHAR              nx_flags;                  /* Flag bits used between client and server to negotiate who performs which updates.  */
    UCHAR              nx_reserved[3];            /* Reserved.  */
    CHAR              *nx_domain_name;            /* The partial or fully qualified domain name.  */

} NX_DHCPV6_CLIENT_FQDN;


/* Define the Client DHCPv6 structure containind the DHCPv6 Client record (DHCPv6 status, server DUID etc).  */

typedef struct NX_DHCPV6_STRUCT 
{
    ULONG                   nx_dhcpv6_id;                               /* DHCPv6 Structure ID  */
    CHAR                    *nx_dhcpv6_name;                            /* DHCPv6 name supplied at create */ 
    UINT                    nx_dhcpv6_client_address_index[NX_DHCPV6_MAX_IA_ADDRESS];         
                                                                        /* Index in IP address table where the Client assigned address is located. */
    UINT                    nx_dhcpv6_client_interface_index;           /* DHCPv6 outgoing network interface index */
    TX_THREAD               nx_dhcpv6_thread;                           /* Client processing thread */
    TX_EVENT_FLAGS_GROUP    nx_dhcpv6_events;                           /* DHCPv6 Client event flags. */
    TX_MUTEX                nx_dhcpv6_client_mutex;                     /* Mutex for exclusive access to the DHCP Client instance */ 
    TX_TIMER                nx_dhcpv6_IP_lifetime_timer;                /* Client IP lifetime timeout timer. */ 
    TX_TIMER                nx_dhcpv6_session_timer;                    /* Client session duration timer. */ 
    NX_IP                   *nx_dhcpv6_ip_ptr;                          /* The associated IP pointer for this DHCPV6 instance */ 
    NX_PACKET_POOL          *nx_dhcpv6_pool_ptr;                        /* Pointer to packet pool for sending DHCPV6 messages */
    NX_UDP_SOCKET           nx_dhcpv6_socket;                           /* UDP socket for communicating with DHCPv6 server */
    UCHAR                   nx_dhcpv6_started;                          /* DHCPv6 client task has been started */ 
    UCHAR                   nx_dhcpv6_state;                            /* The current state of the DHCPv6 Client */     
    USHORT                  nx_status_code;                             /* Status of current option received by Client */
    UINT                    nx_dhcpv6_sleep_flag;                       /* If true, the DHCPv6 client is in a position where it can be stopped */ 
    NX_DHCPV6_MESSAGE_HDR   nx_dhcpv6_message_hdr;                      /* Message Header for all client messages to DHCPv6 Servers */
    NX_DHCPV6_DUID          nx_dhcpv6_client_duid;                      /* Client DUID; ID by which Client and Server identify each other's DUID */
    NX_DHCPV6_DUID          nx_dhcpv6_server_duid;                      /* Server DUID; ID by which Client and Server identify each other's DUID */
    NX_DHCPV6_ELAPSED_TIME  nx_dhcpv6_elapsed_time;                     /* Time duration of the current DHCP msg exchange between Client and Server. */
    NX_DHCPV6_IA_NA         nx_dhcpv6_iana;                             /* Identity Association for non temp address - must be stored in non volatile memory */
    NX_DHCPV6_IA_ADDRESS    nx_dhcpv6_ia[NX_DHCPV6_MAX_IA_ADDRESS];     /* Client internet address option */
    NX_DHCPV6_PREFERENCE    nx_dhcpv6_preference;                       /* Server's preference affecting the Client's DHCPv6 server selection. */
    NX_DHCPV6_OPTIONREQUEST nx_dhcpv6_option_request;                   /* Set of request options in Solicit, Renew, Confirm or Rebind message types. */
    NX_DHCPV6_CLIENT_FQDN   nx_dhcpv6_client_FQDN;                      /* Set of Client options in Solicit, Request, Renew, or Rebind message types. */
    ULONG                   nx_dhcpv6_IP_lifetime_time_accrued;         /* Time since Client set received or renewed its IP address with the DHCPv6 server. */
    UCHAR                   nx_status_message[NX_DHCPV6_MAX_MESSAGE_SIZE];                  /* Server's message in its Option status to client.  */
    NXD_ADDRESS             nx_dhcpv6_server_address;                   /* DHCPv6 server address.  */
    NXD_ADDRESS             nx_dhcpv6_DNS_name_server_address[NX_DHCPV6_NUM_DNS_SERVERS];   /* DNS name server IP address */
    NXD_ADDRESS             nx_dhcpv6_time_server_address[NX_DHCPV6_NUM_TIME_SERVERS];      /* time server IP address */
    NXD_ADDRESS             nx_dhcpv6_client_destination_address;       /* The destination address where DHCP message should be sent, by default All_DHCP_Relay_Agents_and_Servers(FF02::1:2).  */
    UCHAR                   nx_dhcpv6_domain_name[NX_DHCPV6_DOMAIN_NAME_BUFFER_SIZE];       /* Buffer for holding domain name. */
    UCHAR                   nx_dhcpv6_time_zone[NX_DHCPV6_TIME_ZONE_BUFFER_SIZE];           /* Buffer for holding time zone. */
    ULONG                   nx_dhcpv6_solicitations_sent;               /* The number of Solicit messages sent */ 
    ULONG                   nx_dhcpv6_solicitation_responses;           /* The number of solicitations server responded to */ 
    ULONG                   nx_dhcpv6_requests_sent;                    /* The number of Request messages sent */ 
    ULONG                   nx_dhcpv6_request_responses;                /* The number of requests server responded to */ 
    ULONG                   nx_dhcpv6_renews_sent;                      /* The number of renew messages sent */ 
    ULONG                   nx_dhcpv6_renew_responses;                  /* The number of renews server responded to */ 
    ULONG                   nx_dhcpv6_rebinds_sent;                     /* The number of Rebind messages sent */ 
    ULONG                   nx_dhcpv6_rebind_responses;                 /* The number of Rebind requests Server responded to */ 
    ULONG                   nx_dhcpv6_releases_sent;                    /* The number of Release messages sent */ 
    ULONG                   nx_dhcpv6_release_responses;                /* The number of Releases server responded to  */ 
    ULONG                   nx_dhcpv6_confirms_sent;                    /* The number of confirmations sent */ 
    ULONG                   nx_dhcpv6_confirm_responses;                /* The number of confirmations server responded to */ 
    ULONG                   nx_dhcpv6_declines_sent;                    /* The number of declines sent */ 
    ULONG                   nx_dhcpv6_decline_responses;                /* The number of declines server responded to */ 
    ULONG                   nx_dhcpv6_inform_req_sent;                  /* The number of Inform (option requests) sent */ 
    ULONG                   nx_dhcpv6_inform_req_responses;             /* The number of Inform server responsed to */ 
    ULONG                   nx_dhcpv6_transmission_timeout;             /* Timeout on Client messages before resending a request to the server. */
    ULONG                   nx_dhcpv6_retransmission_count;             /* The number of request retransmissions to the server. */
    ULONG                   nx_dhcpv6_init_retransmission_timeout;      /* The initial retransmission time. */
    ULONG                   nx_dhcpv6_max_retransmission_count;         /* The maximum retransmission count. */
    ULONG                   nx_dhcpv6_max_retransmission_timeout;       /* The maximum retransmission time. */
    ULONG                   nx_dhcpv6_max_retransmission_duration;      /* The maximum retransmissions duration. */
    UINT                    nx_dhcpv6_request_solicit_mode;             /* The mode of sending the solicit message with rapid commit option. */
    UINT                    nx_dhcpv6_reply_option_flags;               /* The flags indicate options the reply messages included. */
    USHORT                  nx_dhcpv6_reply_option_current_pref_value;  /* The preference value of current advertise message. */
    UCHAR                   nx_dhcpv6_received_message_type;            /* The type of received message  */
    UCHAR                   nx_dhcpv6_reserved;                         /* Reserved.  */
   
    /* Define the callback function for DHCP state change notification. If specified
       by the application, this function is called whenever a state change occurs for
       the DHCP associated with this IP instance.  */
    VOID (*nx_dhcpv6_state_change_callback)(struct NX_DHCPV6_STRUCT *dhcpv6_ptr, UINT old_state, UINT new_state);

    /* Define the callback function for receiving a non successful status from the Server.  The
       context of the status/error is defined by the message type is was received in and what
       option the status is referring to e.g. IA Address.  */
    VOID (*nx_dhcpv6_server_error_handler)(struct NX_DHCPV6_STRUCT *dhcpv6_ptr, UINT op_code, UINT status_code, UINT message_type);

    /* Define the callback function for adding specific DHCPv6 user option.  */
    UINT (*nx_dhcpv6_user_option_add)(struct NX_DHCPV6_STRUCT *dhcpv6_ptr, UINT interface_index, UINT message_type, UCHAR *user_option_ptr, UINT *user_option_length);

} NX_DHCPV6;

                                          

#ifdef NX_DHCPV6_CLIENT_RESTORE_STATE
/* Define the DHCPv6 Client record structure for restore the DHCPv6 Client state from non volatile memory/across reboots.  */

typedef struct NX_DHCPV6_CLIENT_RECORD_STRUCT 
{                                                                                 
    UCHAR                   nx_dhcpv6_state;                            /* The current state of the DHCPv6 Client */      
    UCHAR                   nx_dhcpv6_reserved[3];                      /* Reserved.  */
    UINT                    nx_dhcpv6_client_interface_index;           /* DHCPv6 outgoing network interface index */ 
    UINT                    nx_dhcpv6_client_address_index[NX_DHCPV6_MAX_IA_ADDRESS];         
                                                                        /* Index in IP address table where the Client assigned address is located. */
    ULONG                   nx_dhcpv6_IP_lifetime_time_accrued;         /* Time since Client set received or renewed its IP address with the DHCPv6 server. */
    NX_DHCPV6_DUID          nx_dhcpv6_client_duid;                      /* Client DUID; ID by which Client and Server identify each other's DUID */
    NX_DHCPV6_DUID          nx_dhcpv6_server_duid;                      /* Server DUID; ID by which Client and Server identify each other's DUID */
    NX_DHCPV6_IA_NA         nx_dhcpv6_iana;                             /* Identity Association for non temp address - must be stored in non volatile memory */
    NX_DHCPV6_IA_ADDRESS    nx_dhcpv6_ia[NX_DHCPV6_MAX_IA_ADDRESS];     /* Client internet address option */
    NX_DHCPV6_OPTIONREQUEST nx_dhcpv6_option_request;                   /* Set of request options in Solicit, Renew, Confirm or Rebind message types. */
    NX_DHCPV6_CLIENT_FQDN   nx_dhcpv6_client_FQDN;                      /* Set of Client options in Solicit, Request, Renew, or Rebind message types. */              
    NXD_ADDRESS             nx_dhcpv6_DNS_name_server_address[NX_DHCPV6_NUM_DNS_SERVERS];   /* DNS name server IP address */
    NXD_ADDRESS             nx_dhcpv6_time_server_address[NX_DHCPV6_NUM_TIME_SERVERS];      /* time server IP address */
    UCHAR                   nx_dhcpv6_domain_name[NX_DHCPV6_DOMAIN_NAME_BUFFER_SIZE];       /* Buffer for holding domain name. */
    UCHAR                   nx_dhcpv6_time_zone[NX_DHCPV6_TIME_ZONE_BUFFER_SIZE];           /* Buffer for holding time zone. */
} NX_DHCPV6_CLIENT_RECORD;
#endif

#ifndef NX_DHCPV6_SOURCE_CODE

/* Application caller is present, perform API mapping.  */

/* Determine if error checking is desired.  If so, map DHCP API functions 
   to the appropriate error checking front-ends.  Otherwise, map API
   functions to the core functions that actually perform the work. 
   Note: error checking is enabled by default.  */


#ifdef NX_DISABLE_ERROR_CHECKING

/* Services without error checking.  */

#define nx_dhcpv6_client_create                             _nx_dhcpv6_client_create
#define nx_dhcpv6_client_delete                             _nx_dhcpv6_client_delete
#define nx_dhcpv6_create_client_duid                        _nx_dhcpv6_create_client_duid
#define nx_dhcpv6_create_client_iana                        _nx_dhcpv6_create_client_iana
#define nx_dhcpv6_create_client_ia                          _nx_dhcpv6_add_client_ia
#define nx_dhcpv6_add_client_ia                             _nx_dhcpv6_add_client_ia
#define nx_dhcpv6_client_set_interface                      _nx_dhcpv6_client_set_interface 
#define nx_dhcpv6_client_set_destination_address            _nx_dhcpv6_client_set_destination_address
#define nx_dhcpv6_set_time_accrued                          _nx_dhcpv6_set_time_accrued
#define nx_dhcpv6_get_client_duid_time_id                   _nx_dhcpv6_get_client_duid_time_id
#define nx_dhcpv6_get_IP_address                            _nx_dhcpv6_get_IP_address
#define nx_dhcpv6_get_lease_time_data                       _nx_dhcpv6_get_lease_time_data
#define nx_dhcpv6_get_other_option_data                     _nx_dhcpv6_get_other_option_data
#define nx_dhcpv6_get_DNS_server_address                    _nx_dhcpv6_get_DNS_server_address 
#define nx_dhcpv6_get_time_server_address                   _nx_dhcpv6_get_time_server_address
#define nx_dhcpv6_get_time_accrued                          _nx_dhcpv6_get_time_accrued
#define nx_dhcpv6_get_iana_lease_time                       _nx_dhcpv6_get_iana_lease_time
#define nx_dhcpv6_get_valid_ip_address_count                _nx_dhcpv6_get_valid_ip_address_count
#define nx_dhcpv6_get_valid_ip_address_lease_time           _nx_dhcpv6_get_valid_ip_address_lease_time
#define nx_dhcpv6_reinitialize                              _nx_dhcpv6_reinitialize
#define nx_dhcpv6_request_solicit                           _nx_dhcpv6_request_solicit
#define nx_dhcpv6_request_solicit_rapid                     _nx_dhcpv6_request_solicit_rapid
#define nx_dhcpv6_request_confirm                           _nx_dhcpv6_request_confirm
#define nx_dhcpv6_request_release                           _nx_dhcpv6_request_release
#define nx_dhcpv6_request_inform_request                    _nx_dhcpv6_request_inform_request
#define nx_dhcpv6_request_option_DNS_server                 _nx_dhcpv6_request_option_DNS_server
#define nx_dhcpv6_request_option_domain_name                _nx_dhcpv6_request_option_domain_name
#define nx_dhcpv6_request_option_time_server                _nx_dhcpv6_request_option_time_server
#define nx_dhcpv6_request_option_timezone                   _nx_dhcpv6_request_option_timezone
#define nx_dhcpv6_request_option_FQDN                       _nx_dhcpv6_request_option_FQDN
#define nx_dhcpv6_start                                     _nx_dhcpv6_start
#define nx_dhcpv6_stop                                      _nx_dhcpv6_stop
#define nx_dhcpv6_suspend                                   _nx_dhcpv6_suspend
#define nx_dhcpv6_resume                                    _nx_dhcpv6_resume   
#define nx_dhcpv6_user_option_add_callback_set              _nx_dhcpv6_user_option_add_callback_set
#ifdef NX_DHCPV6_CLIENT_RESTORE_STATE                                     
#define nx_dhcpv6_client_get_record                         _nx_dhcpv6_client_get_record
#define nx_dhcpv6_client_restore_record                     _nx_dhcpv6_client_restore_record
#endif                                                                

#else

/* Services with error checking.  */

#define nx_dhcpv6_client_create                             _nxe_dhcpv6_client_create
#define nx_dhcpv6_client_delete                             _nxe_dhcpv6_client_delete
#define nx_dhcpv6_create_client_duid                        _nxe_dhcpv6_create_client_duid
#define nx_dhcpv6_create_client_iana                        _nxe_dhcpv6_create_client_iana
#define nx_dhcpv6_create_client_ia                          _nxe_dhcpv6_add_client_ia
#define nx_dhcpv6_add_client_ia                             _nxe_dhcpv6_add_client_ia
#define nx_dhcpv6_client_set_interface                      _nxe_dhcpv6_client_set_interface
#define nx_dhcpv6_client_set_destination_address            _nxe_dhcpv6_client_set_destination_address
#define nx_dhcpv6_set_time_accrued                          _nxe_dhcpv6_set_time_accrued
#define nx_dhcpv6_get_client_duid_time_id                   _nxe_dhcpv6_get_client_duid_time_id
#define nx_dhcpv6_get_IP_address                            _nxe_dhcpv6_get_IP_address
#define nx_dhcpv6_get_lease_time_data                       _nxe_dhcpv6_get_lease_time_data
#define nx_dhcpv6_get_other_option_data                     _nxe_dhcpv6_get_other_option_data
#define nx_dhcpv6_get_DNS_server_address                    _nxe_dhcpv6_get_DNS_server_address 
#define nx_dhcpv6_get_time_server_address                   _nxe_dhcpv6_get_time_server_address
#define nx_dhcpv6_get_time_accrued                          _nxe_dhcpv6_get_time_accrued
#define nx_dhcpv6_get_iana_lease_time                       _nxe_dhcpv6_get_iana_lease_time
#define nx_dhcpv6_get_valid_ip_address_count                _nxe_dhcpv6_get_valid_ip_address_count
#define nx_dhcpv6_get_valid_ip_address_lease_time           _nxe_dhcpv6_get_valid_ip_address_lease_time
#define nx_dhcpv6_reinitialize                              _nxe_dhcpv6_reinitialize
#define nx_dhcpv6_request_solicit                           _nxe_dhcpv6_request_solicit
#define nx_dhcpv6_request_solicit_rapid                     _nxe_dhcpv6_request_solicit_rapid
#define nx_dhcpv6_request_confirm                           _nxe_dhcpv6_request_confirm
#define nx_dhcpv6_request_release                           _nxe_dhcpv6_request_release
#define nx_dhcpv6_request_inform_request                    _nxe_dhcpv6_request_inform_request
#define nx_dhcpv6_request_option_DNS_server                 _nxe_dhcpv6_request_option_DNS_server
#define nx_dhcpv6_request_option_domain_name                _nxe_dhcpv6_request_option_domain_name
#define nx_dhcpv6_request_option_time_server                _nxe_dhcpv6_request_option_time_server
#define nx_dhcpv6_request_option_timezone                   _nxe_dhcpv6_request_option_timezone
#define nx_dhcpv6_request_option_FQDN                       _nxe_dhcpv6_request_option_FQDN
#define nx_dhcpv6_start                                     _nxe_dhcpv6_start
#define nx_dhcpv6_stop                                      _nxe_dhcpv6_stop
#define nx_dhcpv6_suspend                                   _nxe_dhcpv6_suspend
#define nx_dhcpv6_resume                                    _nxe_dhcpv6_resume   
#define nx_dhcpv6_user_option_add_callback_set              _nxe_dhcpv6_user_option_add_callback_set
#ifdef NX_DHCPV6_CLIENT_RESTORE_STATE                                     
#define nx_dhcpv6_client_get_record                         _nxe_dhcpv6_client_get_record
#define nx_dhcpv6_client_restore_record                     _nxe_dhcpv6_client_restore_record
#endif     

#endif

/* Define the prototypes accessible to the application software.  */
UINT        nx_dhcpv6_client_create(NX_DHCPV6 *dhcpv6_ptr, NX_IP *ip_ptr, CHAR *name_ptr, NX_PACKET_POOL *packet_pool_ptr, VOID *stack_ptr, ULONG stack_size,
                                    VOID (*dhcpv6_state_change_notify)(struct NX_DHCPV6_STRUCT *dhcpv6_ptr, UINT old_state, UINT new_state),                             
                                    VOID (*dhcpv6_server_error_handler)(struct NX_DHCPV6_STRUCT *dhcpv6_ptr, UINT op_code, UINT status_code, UINT message_type));
UINT        nx_dhcpv6_client_delete(NX_DHCPV6 *dhcpv6_ptr);
UINT        nx_dhcpv6_create_client_duid(NX_DHCPV6 *dhcpv6_ptr, UINT duid_type, UINT hardware_type, ULONG time);
UINT        nx_dhcpv6_create_client_iana(NX_DHCPV6 *dhcpv6_ptr, UINT IA_ident, ULONG T1, ULONG T2);
UINT        nx_dhcpv6_create_client_ia(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *ipv6_address, ULONG preferred_lifetime, ULONG valid_lifetime);  
UINT        nx_dhcpv6_add_client_ia(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *ipv6_address, ULONG preferred_lifetime, ULONG valid_lifetime);  
UINT        nx_dhcpv6_client_set_interface(NX_DHCPV6 *dhcpv6_ptr, UINT interface_index);
UINT        nx_dhcpv6_client_set_destination_address(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *destination_address);
UINT        nx_dhcpv6_set_time_accrued(NX_DHCPV6 *dhcpv6_ptr, ULONG time_accrued);
UINT        nx_dhcpv6_get_client_duid_time_id(NX_DHCPV6 *dhcpv6_ptr, ULONG *time_id);
UINT        nx_dhcpv6_get_IP_address(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *ip_address);
UINT        nx_dhcpv6_get_lease_time_data(NX_DHCPV6 *dhcpv6_ptr, ULONG *T1, ULONG *T2, ULONG *preferred_lifetime, ULONG *valid_lifetime);
UINT        nx_dhcpv6_get_other_option_data(NX_DHCPV6 *dhcpv6_ptr, UINT option_code, UCHAR *buffer, UINT buffer_length);
UINT        nx_dhcpv6_get_DNS_server_address(NX_DHCPV6 *dhcpv6_ptr, UINT index, NXD_ADDRESS *server_address);   
UINT        nx_dhcpv6_get_time_server_address(NX_DHCPV6 *dhcpv6_ptr, UINT index, NXD_ADDRESS *server_address);
UINT        nx_dhcpv6_get_time_accrued(NX_DHCPV6 *dhcpv6_ptr, ULONG *time_accrued);
UINT        nx_dhcpv6_get_iana_lease_time(NX_DHCPV6 *dhcpv6_ptr, ULONG *T1, ULONG *T2);
UINT        nx_dhcpv6_get_valid_ip_address_count(NX_DHCPV6 *dhcpv6_ptr, UINT *address_count);
UINT        nx_dhcpv6_get_valid_ip_address_lease_time(NX_DHCPV6 *dhcpv6_ptr, UINT address_index, NXD_ADDRESS *ip_address, ULONG *preferred_lifetime, ULONG *valid_lifetime);
UINT        nx_dhcpv6_reinitialize(NX_DHCPV6 *dhcpv6_ptr);
UINT        nx_dhcpv6_request_solicit(NX_DHCPV6 *dhcpv6_ptr);
UINT        nx_dhcpv6_request_solicit_rapid(NX_DHCPV6 *dhcpv6_ptr);
UINT        nx_dhcpv6_request_confirm(NX_DHCPV6 *dhcpv6_ptr);
UINT        nx_dhcpv6_request_release(NX_DHCPV6 *dhcpv6_ptr);
UINT        nx_dhcpv6_request_inform_request(NX_DHCPV6 *dhcpv6_ptr);
UINT        nx_dhcpv6_request_option_DNS_server(NX_DHCPV6 *dhcpv6_ptr, UINT enable);
UINT        nx_dhcpv6_request_option_domain_name(NX_DHCPV6 *dhcpv6_ptr, UINT enable);
UINT        nx_dhcpv6_request_option_time_server(NX_DHCPV6 *dhcpv6_ptr, UINT enable);
UINT        nx_dhcpv6_request_option_timezone(NX_DHCPV6 *dhcpv6_ptr, UINT enable);
UINT        nx_dhcpv6_request_option_FQDN(NX_DHCPV6 *dhcpv6_ptr, CHAR *domain_name, UINT op);
UINT        nx_dhcpv6_start(NX_DHCPV6 *dhcpv6_ptr);
UINT        nx_dhcpv6_stop(NX_DHCPV6 *dhcpv6_ptr);
UINT        nx_dhcpv6_suspend(NX_DHCPV6 *dhcpv6_ptr);
UINT        nx_dhcpv6_resume(NX_DHCPV6 *dhcpv6_ptr);  
UINT        nx_dhcpv6_user_option_add_callback_set(NX_DHCPV6 *dhcpv6_ptr, UINT (*dhcpv6_user_option_add)(NX_DHCPV6 *dhcpv6_ptr, UINT interface_index, UINT message_type, UCHAR *user_option_ptr, UINT *user_option_length));
#ifdef NX_DHCPV6_CLIENT_RESTORE_STATE
UINT        nx_dhcpv6_client_get_record(NX_DHCPV6 *dhcpv6_ptr, NX_DHCPV6_CLIENT_RECORD *client_record_ptr);  
UINT        nx_dhcpv6_client_restore_record(NX_DHCPV6 *dhcpv6_ptr, NX_DHCPV6_CLIENT_RECORD *client_record_ptr, ULONG time_elapsed);
#endif


#else

/* DHCP source code is being compiled, do not perform any API mapping.  */

UINT        _nxe_dhcpv6_client_create(NX_DHCPV6 *dhcpv6_ptr, NX_IP *ip_ptr, CHAR *name_ptr, NX_PACKET_POOL *packet_pool_ptr, VOID *stack_ptr, ULONG stack_size,
                                      VOID (*dhcpv6_state_change_notify)(struct NX_DHCPV6_STRUCT *dhcpv6_ptr, UINT old_state, UINT new_state),
                                      VOID (*dhcpv6_server_error_handler)(struct NX_DHCPV6_STRUCT *dhcpv6_ptr, UINT op_code, UINT status_code, UINT message_type));
UINT        _nx_dhcpv6_client_create(NX_DHCPV6 *dhcpv6_ptr, NX_IP *ip_ptr, CHAR *name_ptr, NX_PACKET_POOL *packet_pool_ptr, VOID *stack_ptr, ULONG stack_size,
                                     VOID (*dhcpv6_state_change_notify)(struct NX_DHCPV6_STRUCT *dhcpv6_ptr, UINT old_state, UINT new_state),
                                     VOID (*dhcpv6_server_error_handler)(struct NX_DHCPV6_STRUCT *dhcpv6_ptr, UINT op_code, UINT status_code, UINT message_type));
UINT        _nxe_dhcpv6_client_delete(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_client_delete(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nxe_dhcpv6_create_client_duid(NX_DHCPV6 *dhcpv6_ptr, UINT duid_type, UINT hardware_type, ULONG time);
UINT        _nx_dhcpv6_create_client_duid(NX_DHCPV6 *dhcpv6_ptr, UINT duid_type, UINT hardware_type, ULONG time);
UINT        _nxe_dhcpv6_create_client_iana(NX_DHCPV6 *dhcpv6_ptr, UINT IA_ident, ULONG T1, ULONG T2);
UINT        _nx_dhcpv6_create_client_iana(NX_DHCPV6 *dhcpv6_ptr, UINT IA_ident, ULONG T1, ULONG T2);
UINT        _nxe_dhcpv6_add_client_ia(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *ipv6_address, ULONG preferred_lifetime, ULONG valid_lifetime);
UINT        _nx_dhcpv6_add_client_ia(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *ipv6_address, ULONG preferred_lifetime, ULONG valid_lifetime);
UINT        _nxe_dhcpv6_client_set_interface(NX_DHCPV6 *dhcpv6_ptr, UINT interface_index);
UINT        _nx_dhcpv6_client_set_interface(NX_DHCPV6 *dhcpv6_ptr, UINT interface_index); 
UINT        _nxe_dhcpv6_client_set_destination_address(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *destination_address);
UINT        _nx_dhcpv6_client_set_destination_address(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *destination_address);
UINT        _nxe_dhcpv6_set_time_accrued(NX_DHCPV6 *dhcpv6_ptr, ULONG time_accrued);
UINT        _nx_dhcpv6_set_time_accrued(NX_DHCPV6 *dhcpv6_ptr, ULONG time_accrued);
UINT        _nxe_dhcpv6_get_client_duid_time_id(NX_DHCPV6 *dhcpv6_ptr, ULONG *time_id);
UINT        _nx_dhcpv6_get_client_duid_time_id(NX_DHCPV6 *dhcpv6_ptr, ULONG *time_id);
UINT        _nxe_dhcpv6_get_IP_address(NX_DHCPV6 *dhcpv6_ptr,  NXD_ADDRESS *ip_address);
UINT        _nx_dhcpv6_get_IP_address(NX_DHCPV6 *dhcpv6_ptr, NXD_ADDRESS *ip_address);
UINT        _nxe_dhcpv6_get_lease_time_data(NX_DHCPV6 *dhcpv6_ptr, ULONG *T1, ULONG *T2, ULONG *preferred_lifetime, ULONG *valid_lifetime);
UINT        _nx_dhcpv6_get_lease_time_data(NX_DHCPV6 *dhcpv6_ptr, ULONG *T1, ULONG *T2, ULONG *preferred_lifetime, ULONG *valid_lifetime);
UINT        _nxe_dhcpv6_get_other_option_data(NX_DHCPV6 *dhcpv6_ptr, UINT option_code, UCHAR *buffer, UINT buffer_length);
UINT        _nx_dhcpv6_get_other_option_data(NX_DHCPV6 *dhcpv6_ptr, UINT option_code, UCHAR *buffer, UINT buffer_length);
UINT        _nxe_dhcpv6_get_DNS_server_address(NX_DHCPV6 *dhcpv6_ptr, UINT index, NXD_ADDRESS *server_address);
UINT        _nx_dhcpv6_get_DNS_server_address(NX_DHCPV6 *dhcpv6_ptr, UINT index, NXD_ADDRESS *server_address);
UINT        _nxe_dhcpv6_get_time_server_address(NX_DHCPV6 *dhcpv6_ptr, UINT index, NXD_ADDRESS *server_address);
UINT        _nx_dhcpv6_get_time_server_address(NX_DHCPV6 *dhcpv6_ptr, UINT index, NXD_ADDRESS *server_address);
UINT        _nxe_dhcpv6_get_time_accrued(NX_DHCPV6 *dhcpv6_ptr, ULONG *time_accrued);
UINT        _nx_dhcpv6_get_time_accrued(NX_DHCPV6 *dhcpv6_ptr, ULONG *time_accrued);
UINT        _nxe_dhcpv6_get_iana_lease_time(NX_DHCPV6 *dhcpv6_ptr, ULONG *T1, ULONG *T2);
UINT        _nx_dhcpv6_get_iana_lease_time(NX_DHCPV6 *dhcpv6_ptr, ULONG *T1, ULONG *T2);
UINT        _nxe_dhcpv6_get_valid_ip_address_count(NX_DHCPV6 *dhcpv6_ptr, UINT *address_count);
UINT        _nx_dhcpv6_get_valid_ip_address_count(NX_DHCPV6 *dhcpv6_ptr, UINT *address_count);
UINT        _nxe_dhcpv6_get_valid_ip_address_lease_time(NX_DHCPV6 *dhcpv6_ptr, UINT address_index, NXD_ADDRESS *ip_address, ULONG *preferred_lifetime, ULONG *valid_lifetime);
UINT        _nx_dhcpv6_get_valid_ip_address_lease_time(NX_DHCPV6 *dhcpv6_ptr, UINT address_index, NXD_ADDRESS *ip_address, ULONG *preferred_lifetime, ULONG *valid_lifetime);
UINT        _nxe_dhcpv6_reinitialize(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_reinitialize(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nxe_dhcpv6_request_solicit(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_request_solicit(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nxe_dhcpv6_request_solicit_rapid(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_request_solicit_rapid(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nxe_dhcpv6_request_confirm(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_request_confirm(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nxe_dhcpv6_request_release(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_request_release(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nxe_dhcpv6_request_inform_request(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_request_inform_request(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nxe_dhcpv6_request_option_DNS_server(NX_DHCPV6 *dhcpv6_ptr, UINT enable);
UINT        _nx_dhcpv6_request_option_DNS_server(NX_DHCPV6 *dhcpv6_ptr, UINT enable);
UINT        _nxe_dhcpv6_request_option_domain_name(NX_DHCPV6 *dhcpv6_ptr, UINT enable);
UINT        _nx_dhcpv6_request_option_domain_name(NX_DHCPV6 *dhcpv6_ptr, UINT enable);
UINT        _nxe_dhcpv6_request_option_time_server(NX_DHCPV6 *dhcpv6_ptr, UINT enable);
UINT        _nx_dhcpv6_request_option_time_server(NX_DHCPV6 *dhcpv6_ptr, UINT enable);
UINT        _nxe_dhcpv6_request_option_timezone(NX_DHCPV6 *dhcpv6_ptr, UINT enable);
UINT        _nx_dhcpv6_request_option_timezone(NX_DHCPV6 *dhcpv6_ptr, UINT enable);
UINT        _nxe_dhcpv6_request_option_FQDN(NX_DHCPV6 *dhcpv6_ptr, CHAR *domain_name, UINT op);
UINT        _nx_dhcpv6_request_option_FQDN(NX_DHCPV6 *dhcpv6_ptr, CHAR *domain_name, UINT op);
UINT        _nxe_dhcpv6_start(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_start(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nxe_dhcpv6_stop(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_stop(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nxe_dhcpv6_suspend(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_suspend(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nxe_dhcpv6_resume(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_resume(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nxe_dhcpv6_user_option_add_callback_set(NX_DHCPV6 *dhcpv6_ptr, UINT (*dhcpv6_user_option_add)(NX_DHCPV6 *dhcpv6_ptr, UINT interface_index, UINT message_type, UCHAR *user_option_ptr, UINT *user_option_length));
UINT        _nx_dhcpv6_user_option_add_callback_set(NX_DHCPV6 *dhcpv6_ptr, UINT (*dhcpv6_user_option_add)(NX_DHCPV6 *dhcpv6_ptr, UINT interface_index, UINT message_type, UCHAR *user_option_ptr, UINT *user_option_length));
#ifdef NX_DHCPV6_CLIENT_RESTORE_STATE
UINT        _nxe_dhcpv6_client_get_record(NX_DHCPV6 *dhcpv6_ptr, NX_DHCPV6_CLIENT_RECORD *client_record_ptr);     
UINT        _nx_dhcpv6_client_get_record(NX_DHCPV6 *dhcpv6_ptr, NX_DHCPV6_CLIENT_RECORD *client_record_ptr);  
UINT        _nxe_dhcpv6_client_restore_record(NX_DHCPV6 *dhcpv6_ptr, NX_DHCPV6_CLIENT_RECORD *client_record_ptr, ULONG time_elapsed);
UINT        _nx_dhcpv6_client_restore_record(NX_DHCPV6 *dhcpv6_ptr, NX_DHCPV6_CLIENT_RECORD *client_record_ptr, ULONG time_elapsed);
#endif

#endif


/* Define DHCPv6 Client internal functions. */

VOID        _nx_dhcpv6_thread_entry(ULONG info);
VOID        _nx_dhcpv6_process(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_request(NX_DHCPV6 *dhcpv6_ptr, UINT dhcpv6_state);
UINT        _nx_dhcpv6_request_renew(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_request_rebind(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_request_decline(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_send_request(NX_DHCPV6 *dhcpv6_ptr); 
UINT        _nx_dhcpv6_add_client_duid(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index);
UINT        _nx_dhcpv6_add_elapsed_time(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index); 
UINT        _nx_dhcpv6_add_option_request(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index); 
UINT        _nx_dhcpv6_add_ia_address(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index, UINT ia_index); 
UINT        _nx_dhcpv6_add_iana(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index); 
UINT        _nx_dhcpv6_add_server_duid(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index); 
UINT        _nx_dhcpv6_add_client_FQDN(NX_DHCPV6 *dhcpv6_ptr, UCHAR *buffer_ptr, UINT *index);
UINT        _nx_dhcpv6_waiting_on_reply(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_packet_process(NX_DHCPV6 *dhcpv6_ptr, NX_PACKET *packet_ptr);
UINT        _nx_dhcpv6_scan_packet_options(NX_DHCPV6 *dhcpv6_ptr, NX_PACKET *packet_ptr);
UINT        _nx_dhcpv6_preprocess_packet_information(NX_DHCPV6 *dhcpv6_ptr, NX_PACKET *packet_ptr);
UINT        _nx_dhcpv6_extract_packet_information(NX_DHCPV6 *dhcpv6_ptr, NX_PACKET *packet_ptr);       
VOID        _nx_dhcpv6_flush_queue_packets(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_process_client_duid(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length);
UINT        _nx_dhcpv6_process_DNS_server(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length);
UINT        _nx_dhcpv6_process_domain_name(NX_DHCPV6 *dhcpv6_ptr, UCHAR *packet_start, UCHAR *option_data, UINT option_length);
UINT        _nx_dhcpv6_process_ia(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length, UINT ia_index);
UINT        _nx_dhcpv6_process_iana(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length);
UINT        _nx_dhcpv6_process_preference(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length);
UINT        _nx_dhcpv6_process_server_duid(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length);
UINT        _nx_dhcpv6_process_status(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length);
UINT        _nx_dhcpv6_process_time_zone(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length);
UINT        _nx_dhcpv6_process_time_server(NX_DHCPV6 *dhcpv6_ptr, UCHAR *option_data, UINT option_length);
VOID        _nx_dhcpv6_IP_lifetime_timeout_entry(ULONG dhcpv6_ptr_value);
VOID        _nx_dhcpv6_session_timeout_entry(ULONG dhcpv6_ptr_value);
UINT        _nx_dhcpv6_utility_get_block_option_length(UCHAR *buffer_ptr, ULONG *option, ULONG *length);
UINT        _nx_dhcpv6_utility_get_data(UCHAR *buffer, UINT size, ULONG *value);
INT         _nx_dhcpv6_utility_time_randomize(void);
UINT        _nx_dhcpv6_update_retransmit_info(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_register_IP_address(NX_DHCPV6 *dhcpv6_ptr);
UINT        _nx_dhcpv6_remove_assigned_address(NX_DHCPV6 *dhcpv6_ptr, UINT ia_index);
UINT        _nx_dhcpv6_name_string_encode(UCHAR *ptr, UCHAR *name);
UINT        _nx_dhcpv6_name_string_unencode(UCHAR *data, UINT start, UCHAR *buffer, UINT size);
#if !defined (NX_DISABLE_IPV6_DAD) && defined (NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY)
VOID        _nx_dhcpv6_ipv6_address_DAD_notify(NX_IP *ip_ptr, UINT status, UINT interface_index, UINT ipv6_addr_index, ULONG *ipv6_address);
#endif

/* Determine if a C++ compiler is being used.  If so, complete the standard
   C conditional started above.  */
#ifdef   __cplusplus
        }
#endif

#endif /* NX_DHCPV6_CLIENT_H */


