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
/**   Internet Protocol (IP)                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_ipv4.h                                           PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Internet Protocol component,             */
/*    including all data types and external references.  It is assumed    */
/*    that nx_api.h and nx_port.h have already been included.             */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported TCP/IP offload,   */
/*                                            resulting in version 6.1.8  */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), included */
/*                                            necessary header file,      */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/

#ifndef NX_IPV4_H
#define NX_IPV4_H

#include "nx_api.h"


#ifndef NX_DISABLE_IPV4
#define NX_IP_VERSION                    0x45000000UL /* Version 4, Length of 5   */

#define NX_IP_NORMAL_LENGTH              5            /* Normal IP header length  */
#define NX_IP_HEADER_LENGTH_ENCODE_6     6            /* IP header length 6. */

/* Define IP options.  */

#define NX_IP_OPTION_COPY_FLAG           0x80000000UL       /* All fragments must carry the option. */
#define NX_IP_OPTION_CLASS               0x00000000UL       /* Control. */
#define NX_IP_OPTION_ROUTER_ALERT_NUMBER 0x14000000UL
#define NX_IP_OPTION_ROUTER_ALERT_LENGTH 0x00040000UL
#define NX_IP_OPTION_ROUTER_ALERT_VALUE  0x00000000UL


/* Define IP option type.  */
#define NX_IP_OPTION_END                 0
#define NX_IP_OPTION_NO_OPERATION        1
#define NX_IP_OPTION_INTERNET_TIMESTAMP  68


/* Define IP fragmenting information.  */

#ifdef NX_DONT_FRAGMENT
#define NX_IP_DONT_FRAGMENT              NX_DONT_FRAGMENT
#else
#define NX_IP_DONT_FRAGMENT              0x00004000UL /* Don't fragment bit       */
#endif /* NX_DONT_FRAGMENT */
#ifdef NX_MORE_FRAGMENTS
#define NX_IP_MORE_FRAGMENT              NX_MORE_FRAGMENTS
#else
#define NX_IP_MORE_FRAGMENT              0x00002000UL /* More fragments           */
#endif /* NX_DONT_FRAGMENT */
#define NX_IP_FRAGMENT_MASK              0x00003FFFUL /* Mask for fragment bits   */
#ifdef NX_FRAG_OFFSET_MASK
#define NX_IP_OFFSET_MASK                NX_FRAG_OFFSET_MASK
#else
#define NX_IP_OFFSET_MASK                0x00001FFFUL /* Mask for fragment offset */
#endif /* NX_FRAG_OFFSET_MASK */
#define NX_IP_ALIGN_FRAGS                8            /* Fragment alignment       */

/* Define basic IP Header constant.  */

/* Define Basic Internet packet header data type.  This will be used to
   build new IP packets and to examine incoming packets into NetX.  */

typedef  struct NX_IPV4_HEADER_STRUCT
{
    /* Define the first 32-bit word of the IP header.  This word contains
       the following information:

            bits 31-28  IP Version = 0x4  (IP Version4)
            bits 27-24  IP Header Length of 32-bit words (5 if no options)
            bits 23-16  IP Type of Service, where 0x00 -> Normal
                                                  0x10 -> Minimize Delay
                                                  0x08 -> Maximize Throughput
                                                  0x04 -> Maximize Reliability
                                                  0x02 -> Minimize Monetary Cost
            bits 15-0   IP Datagram length in bytes
     */
    ULONG nx_ip_header_word_0;

    /* Define the second word of the IP header.  This word contains
       the following information:

            bits 31-16  IP Packet Identification (just an incrementing number)
            bits 15-0   IP Flags and Fragment Offset (used for IP fragmenting)
                            bit  15         Zero
                            bit  14         Don't Fragment
                            bit  13         More Fragments
                            bits 12-0       (Fragment offset)/8
     */
    ULONG nx_ip_header_word_1;

    /* Define the third word of the IP header.  This word contains
       the following information:

            bits 31-24  IP Time-To-Live (maximum number of routers
                                         packet can traverse before being
                                         thrown away.  Default values are typically
                                         32 or 64)
            bits 23-16  IP Protocol, where  0x01 -> ICMP Messages
                                            0x02 -> IGMP Messages
                                            0x06 -> TCP  Messages
                                            0x11 -> UDP  Messages
            bits 15-0   IP Checksum
     */
    ULONG nx_ip_header_word_2;

    /* Define the source IP address.  */
    ULONG nx_ip_header_source_ip;

    /* Define the destination IP address.  */
    ULONG nx_ip_header_destination_ip;
} NX_IPV4_HEADER;

/* Define IPv4 internal function prototypes.  */
VOID  _nx_ip_forward_packet_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
VOID  _nx_ip_fragment_forward_packet(NX_IP *ip_ptr, NX_PACKET *packet_ptr, ULONG destination_ip, ULONG fragment, ULONG next_hop_address);
void  _nx_ip_packet_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr, ULONG destination_ip, ULONG type_of_service, ULONG time_to_live, ULONG protocol, ULONG fragment, ULONG next_hop_address);
UINT  _nx_ip_header_add(NX_IP *ip_ptr, NX_PACKET *packet_ptr, ULONG source_ip, ULONG destination_ip,
                        ULONG type_of_service, ULONG time_to_live, ULONG protocol, ULONG fragment);
VOID  _nx_ip_driver_packet_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr, ULONG destination_ip, ULONG fragment, ULONG next_hop_address);
ULONG _nx_ip_route_find(NX_IP *ip_ptr, ULONG destination_address, NX_INTERFACE **nx_ip_interface, ULONG *next_hop_address);
VOID  _nx_ipv4_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
UINT  _nx_ipv4_option_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr);
#endif /* NX_DISABLE_IPV4 */

/* Define IPv4 function prototypes.  */

UINT _nx_ip_address_change_notify(NX_IP *ip_ptr, VOID (*ip_address_change_notify)(NX_IP *, VOID *), VOID *additional_info);
UINT _nx_ip_address_get(NX_IP *ip_ptr, ULONG *ip_address, ULONG *network_mask);
UINT _nx_ip_address_set(NX_IP *ip_ptr, ULONG ip_address, ULONG network_mask);
UINT _nx_ip_gateway_address_set(NX_IP *ip_ptr, ULONG ip_address);
UINT _nx_ip_gateway_address_get(NX_IP *ip_ptr, ULONG *ip_address);
UINT _nx_ip_gateway_address_clear(NX_IP *ip_ptr);
UINT _nx_ip_interface_address_get(NX_IP *ip_ptr, UINT interface_index, ULONG *ip_address, ULONG *network_mask);
UINT _nx_ip_interface_address_set(NX_IP *ip_ptr, UINT interface_index, ULONG ip_address, ULONG network_mask);
UINT _nx_ip_raw_packet_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                            ULONG destination_ip, ULONG type_of_service);
UINT _nx_ip_raw_packet_source_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr, ULONG destination_ip, UINT address_index, ULONG type_of_service);
UINT _nx_ip_static_route_add(NX_IP *ip_ptr, ULONG network_address,
                             ULONG net_mask, ULONG next_hop);
UINT _nx_ip_static_route_delete(NX_IP *ip_ptr, ULONG network_address, ULONG net_mask);
UINT _nx_ipv4_multicast_interface_join(NX_IP *ip_ptr, ULONG group_address, UINT interface_index);
UINT _nx_ipv4_multicast_interface_leave(NX_IP *ip_ptr, ULONG group_address, UINT interface_index);


/* Define error checking shells for API services.  These are only referenced by the
   application.  */

UINT _nxe_ip_address_change_notify(NX_IP *ip_ptr, VOID (*ip_address_change_notify)(NX_IP *, VOID *), VOID *additional_info);
UINT _nxe_ip_address_get(NX_IP *ip_ptr, ULONG *ip_address, ULONG *network_mask);
UINT _nxe_ip_address_set(NX_IP *ip_ptr, ULONG ip_address, ULONG network_mask);
UINT _nxe_ip_interface_address_get(NX_IP *ip_ptr, UINT interface_index, ULONG *ip_address, ULONG *network_mask);
UINT _nxe_ip_interface_address_set(NX_IP *ip_ptr, UINT interface_index, ULONG ip_address, ULONG network_mask);
UINT _nxe_ip_gateway_address_set(NX_IP *ip_ptr, ULONG ip_address);
UINT _nxe_ip_gateway_address_get(NX_IP *ip_ptr, ULONG *ip_address);
UINT _nxe_ip_gateway_address_clear(NX_IP *ip_ptr);
UINT _nxe_ip_raw_packet_send(NX_IP *ip_ptr, NX_PACKET **packet_ptr_ptr,
                             ULONG destination_ip, ULONG type_of_service);
UINT _nxe_ip_raw_packet_source_send(NX_IP *ip_ptr, NX_PACKET **packet_ptr_ptr,
                                    ULONG destination_ip, UINT address_index, ULONG type_of_service);
UINT _nxe_ip_static_route_add(NX_IP *ip_ptr, ULONG network_address,
                              ULONG net_mask, ULONG next_hop);
UINT _nxe_ip_static_route_delete(NX_IP *ip_ptr, ULONG network_address, ULONG net_mask);
UINT _nxe_ipv4_multicast_interface_join(NX_IP *ip_ptr, ULONG group_address, UINT interface_index);
UINT _nxe_ipv4_multicast_interface_leave(NX_IP *ip_ptr, ULONG group_address, UINT interface_index);

#ifndef FEATURE_NX_IPV6
#ifdef NX_IPSEC_ENABLE
#define AUTHENTICATION_HEADER 5
#define ENCAP_SECURITY_HEADER 6
#endif /* NX_IPSEC_ENABLE */
#endif /* FEATURE_NX_IPV6 */
#endif /* NX_IPV4_H */

