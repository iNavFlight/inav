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

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_igmp.h"

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_header_add                                   PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function prepends an IP header.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*    source_ip                             Source IP address             */
/*    destination_ip                        Destination IP address        */
/*    type_of_service                       Type of service for packet    */
/*    time_to_live                          Time to live value for packet */
/*    protocol                              Protocol being encapsulated   */
/*    fragment                              Don't fragment bit            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Added the header successfully */
/*    NX_UNDERFLOW                          Invalid packet header         */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_checksum_compute               Compute IP checksum           */
/*    _nx_packet_transmit_release           Release transmit packet       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
/*                                                                        */
/**************************************************************************/
UINT  _nx_ip_header_add(NX_IP *ip_ptr, NX_PACKET *packet_ptr, ULONG source_ip, ULONG destination_ip,
                        ULONG type_of_service, ULONG time_to_live,  ULONG protocol, ULONG fragment)
{
ULONG           router_alert = 0;
NX_IPV4_HEADER *ip_header_ptr;
ULONG           checksum;
#if defined(NX_DISABLE_IP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT            compute_checksum = 1;
#endif /* defined(NX_DISABLE_IP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
ULONG           val;

#ifndef NX_DISABLE_IGMPV2
    /* Check IGMPv2 protocol. */
    if ((protocol == NX_IP_IGMP) && (ip_ptr -> nx_ip_igmp_router_version == NX_IGMP_HOST_VERSION_2))
    {
        router_alert = 4;
    }
#endif

    /* Prepend the IP header to the packet.  First, make room for the IP header.  */
    packet_ptr -> nx_packet_prepend_ptr =  (packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_IPV4_HEADER)) - router_alert;

    /* Increase the packet length.  */
    packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length + (ULONG)sizeof(NX_IPV4_HEADER) + router_alert;

    /* Assert prepend pointer is no less than data start pointer.  */
    /*lint -e{946} suppress pointer subtraction, since it is necessary. */
    NX_ASSERT(packet_ptr -> nx_packet_prepend_ptr >= packet_ptr -> nx_packet_data_start);

    /* Setup the IP header pointer.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ip_header_ptr =  (NX_IPV4_HEADER *)packet_ptr -> nx_packet_prepend_ptr;
    packet_ptr -> nx_packet_ip_header = packet_ptr -> nx_packet_prepend_ptr;
    packet_ptr -> nx_packet_ip_header_length = (UCHAR)(packet_ptr -> nx_packet_ip_header_length +
                                                       sizeof(NX_IPV4_HEADER) + router_alert);

    /* Determine if this is an identical copy for TCP retransmission.
       RFC1122, Section3.2.1.5, Page32-33. RFC1122, Section4.2.2.15, Page90-91.  */
    if (packet_ptr -> nx_packet_identical_copy == NX_TRUE)
    {

        /* Yes, this an identical copy for TCP retransmission.
           The IP header has been added, return.  */
        return(NX_SUCCESS);
    }

    /* Build the IP header.  */

#ifndef NX_DISABLE_IGMPV2
    if (router_alert)
    {

        /* Build the first 32-bit word of the IP header.  */
        ip_header_ptr -> nx_ip_header_word_0 =  (ULONG)((NX_IP_VERSION_V4 << 28) |
                                                        (NX_IP_HEADER_LENGTH_ENCODE_6 << 24) |
                                                        type_of_service |
                                                        (0xFFFF & packet_ptr -> nx_packet_length));
    }
    else
#endif
    {

        /* Build the first 32-bit word of the IP header.  */
        ip_header_ptr -> nx_ip_header_word_0 =  (NX_IP_VERSION | type_of_service | (0xFFFF & packet_ptr -> nx_packet_length));
    }

    /* Build the second 32-bit word of the IP header.  */
    ip_header_ptr -> nx_ip_header_word_1 =  (ip_ptr -> nx_ip_packet_id++ << NX_SHIFT_BY_16) | fragment;

    /* Build the third 32-bit word of the IP header.  */
    ip_header_ptr -> nx_ip_header_word_2 =  ((time_to_live << NX_IP_TIME_TO_LIVE_SHIFT) | protocol);

    /* Place the source IP address in the IP header.  */
    ip_header_ptr -> nx_ip_header_source_ip =  source_ip;

    /* Place the destination IP address in the IP header.  */
    ip_header_ptr -> nx_ip_header_destination_ip =  destination_ip;

#ifndef NX_DISABLE_IGMPV2
    if (router_alert)
    {

        /* Append Router Alert Option. */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        *((ULONG *)(packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_IPV4_HEADER))) = (NX_IP_OPTION_COPY_FLAG |
                                                                                      NX_IP_OPTION_CLASS |
                                                                                      NX_IP_OPTION_ROUTER_ALERT_NUMBER |
                                                                                      NX_IP_OPTION_ROUTER_ALERT_LENGTH |
                                                                                      NX_IP_OPTION_ROUTER_ALERT_VALUE);
    }
#endif

    /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
       swap the endian of the IP header.  */
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_1);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_2);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_source_ip);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_destination_ip);
#ifndef NX_DISABLE_IGMPV2
    if (router_alert)
    {

        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        NX_CHANGE_ULONG_ENDIAN(*((ULONG *)(packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_IPV4_HEADER))));
    }
#endif

#ifdef NX_DISABLE_IP_TX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_IP_TX_CHECKSUM */
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM)
    {
        compute_checksum = 0;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
#ifdef NX_IPSEC_ENABLE
    if (packet_ptr -> nx_packet_ipsec_sa_ptr != NX_NULL)
    {
        if ((((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_mode == NX_IPSEC_TUNNEL_MODE) &&
            (((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_encryption_method != NX_CRYPTO_NONE))
        {
            compute_checksum = 1;
        }
    }

#endif /* NX_IPSEC_ENABLE */

#if defined(NX_DISABLE_IP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_IP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
    {
        checksum = _nx_ip_checksum_compute(packet_ptr, NX_IP_VERSION_V4,
                                           /* Length is the size of IP header, including options */
                                           (UINT)(20 + router_alert),
                                           /* IPv4 header checksum does not use src/dest addresses */
                                           NULL, NULL);

        val = (ULONG)(~checksum);
        val = val & NX_LOWER_16_MASK;

        /* Convert to network byte order. */
        NX_CHANGE_ULONG_ENDIAN(val);

        /* Now store the checksum in the IP header.  */
        ip_header_ptr -> nx_ip_header_word_2 =  ip_header_ptr -> nx_ip_header_word_2 | val;
    }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
    {
        packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    /* Return...  */
    return(NX_SUCCESS);
}

#endif /* NX_DISABLE_IPV4 */

