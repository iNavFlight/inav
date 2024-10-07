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
#include "nx_tcp.h"
#include "nx_udp.h"
#include "nx_icmp.h"
#include "nx_igmp.h"


#ifdef NX_ENABLE_INTERFACE_CAPABILITY
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_packet_checksum_compute                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function calculates checksum for packet need to be fragmented. */
/*    Only checksum upon IP layer is calculate.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Packet pointer                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_ip_checksum_compute                Compute UDP header checksum   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ipv6_packet_send                                                */
/*    _nx_ip_driver_packet_send                                           */
/*    _nx_ip_fragment_packet                                              */
/*    _nx_ipv6_fragment_process                                           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID  _nx_ip_packet_checksum_compute(NX_PACKET *packet_ptr)
{
ULONG             next_protocol;
UCHAR            *org_prepend_ptr;
ULONG             checksum;
ULONG             val;
UCHAR             is_done = NX_FALSE;
ULONG             ip_header_length;
ULONG             ip_src_addr[4];
ULONG             ip_dst_addr[4];
ULONG             data_length = 0;
NX_IPV4_HEADER   *ip_header_ptr;
NX_TCP_HEADER    *tcp_header_ptr;
NX_UDP_HEADER    *udp_header_ptr;
NX_ICMP_HEADER   *icmpv4_header_ptr;
NX_IGMP_HEADER   *igmp_header_ptr;
#ifdef FEATURE_NX_IPV6
USHORT            short_val;
NX_ICMPV6_HEADER *icmpv6_header_ptr;
NX_IPV6_HEADER   *ipv6_header_ptr;
#endif

    /* Get IP version. */
#ifdef FEATURE_NX_IPV6
    if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
    {
#endif
        next_protocol = NX_PROTOCOL_IPV4;
#ifdef FEATURE_NX_IPV6
    }
    else
    {
        next_protocol = NX_PROTOCOL_IPV6;
    }
#endif

    /* Store original prepend_ptr. */
    org_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr;

    /* Loop to process headers. */
    while (!is_done)
    {
        switch (next_protocol)
        {
        case NX_PROTOCOL_IPV4:
        {

            /* It's assumed that the IP link driver has positioned the top pointer in the
               packet to the start of the IP address... so that's where we will start.  */
            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            ip_header_ptr = (NX_IPV4_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

            /* Pick up the first word in the IP header. */
            val = ip_header_ptr -> nx_ip_header_word_0;

            /* Convert to host byte order. */
            NX_CHANGE_ULONG_ENDIAN(val);

            /* Obtain IP header length. */
            ip_header_length =  (val & NX_IP_LENGTH_MASK) >> 24;

            /* Check if IPv4 checksum is enabled. */
            if (packet_ptr -> nx_packet_interface_capability_flag & NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM)
            {

                checksum = _nx_ip_checksum_compute(packet_ptr, NX_IP_VERSION_V4,
                                                   /* length is the size of IP header, including options */
                                                   ip_header_length << 2,
                                                   /* IPv4 header checksum doesn't care src/dest addresses */
                                                   NULL, NULL);

                val = (ULONG)(~checksum);
                val = val & NX_LOWER_16_MASK;

                /* Convert to network byte order. */
                NX_CHANGE_ULONG_ENDIAN(val);

                /* Now store the checksum in the IP header.  */
                ip_header_ptr -> nx_ip_header_word_2 =  ip_header_ptr -> nx_ip_header_word_2 | val;

                /* Clear checksum flag. */
                packet_ptr -> nx_packet_interface_capability_flag  &= (ULONG)(~NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM);
            }


            /* Get src and dst addresses. */
            ip_src_addr[0] = ip_header_ptr -> nx_ip_header_source_ip;
            ip_dst_addr[0] = ip_header_ptr -> nx_ip_header_destination_ip;
            NX_CHANGE_ULONG_ENDIAN(ip_src_addr[0]);
            NX_CHANGE_ULONG_ENDIAN(ip_dst_addr[0]);

            /* Get next protocol. */
            val = ip_header_ptr -> nx_ip_header_word_2;
            NX_CHANGE_ULONG_ENDIAN(val);
            next_protocol = (val >> 16) & 0xFF;

            /* Remove IPv4 header. */
            packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr + (ip_header_length << 2);
            data_length = packet_ptr -> nx_packet_length - (ip_header_length << 2);
            break;
        }

        case NX_PROTOCOL_TCP:
        {

            /* Check if TCP checksum is enabled. */
            if (packet_ptr -> nx_packet_interface_capability_flag  & NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM)
            {

                /* Calculate the TCP checksum without protection.  */
                checksum =  _nx_ip_checksum_compute(packet_ptr, NX_PROTOCOL_TCP,
                                                    data_length,
                                                    ip_src_addr, ip_dst_addr);

                /* Pickup the pointer to the head of the TCP packet.  */
                /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                tcp_header_ptr =  (NX_TCP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

                checksum = ~checksum & NX_LOWER_16_MASK;

                /* Move the checksum into header.  */
                NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_4);
                tcp_header_ptr -> nx_tcp_header_word_4 |=  (checksum << NX_SHIFT_BY_16);
                NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_4);

                /* Clear checksum flag. */
                packet_ptr -> nx_packet_interface_capability_flag  &= (ULONG)(~NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM);
            }

            /* No necessary to process next protocol. */
            is_done = NX_TRUE;
            break;
        }

        case NX_PROTOCOL_UDP:
        {

            /* Check if UDP checksum is enabled. */
            if (packet_ptr -> nx_packet_interface_capability_flag  & NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM)
            {

                /* Calculate the UDP checksum without protection.  */
                checksum =  _nx_ip_checksum_compute(packet_ptr, NX_PROTOCOL_UDP,
                                                    data_length,
                                                    ip_src_addr, ip_dst_addr);

                /* Pickup the pointer to the head of the UDP packet.  */
                /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                udp_header_ptr = (NX_UDP_HEADER *)(packet_ptr -> nx_packet_prepend_ptr);

                /* Move the checksum into header.  */
                NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);
                udp_header_ptr -> nx_udp_header_word_1 = udp_header_ptr -> nx_udp_header_word_1 | (~checksum & NX_LOWER_16_MASK);
                NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);

                /* Clear checksum flag. */
                packet_ptr -> nx_packet_interface_capability_flag  &= (ULONG)(~NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM);
            }

            /* No necessary to process next protocol. */
            is_done = NX_TRUE;
            break;
        }

        case NX_PROTOCOL_ICMP:
        {

            /* Check if ICMPv4 checksum is enabled. */
            if (packet_ptr -> nx_packet_interface_capability_flag  & NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM)
            {

                /* Calculate the ICMPv4 checksum without protection.  */
                checksum =  _nx_ip_checksum_compute(packet_ptr, NX_IP_ICMP,
                                                    data_length,
                                                    /* ICMPV4 header checksum doesn't care src/dest addresses */
                                                    NULL, NULL);

                /* Pickup the pointer to the head of the ICMPv4 packet.  */
                /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                icmpv4_header_ptr =  (NX_ICMP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

                /* Move the checksum into header.  */
                NX_CHANGE_ULONG_ENDIAN(icmpv4_header_ptr -> nx_icmp_header_word_0);
                icmpv4_header_ptr -> nx_icmp_header_word_0 =  icmpv4_header_ptr -> nx_icmp_header_word_0 | (~checksum & NX_LOWER_16_MASK);
                NX_CHANGE_ULONG_ENDIAN(icmpv4_header_ptr -> nx_icmp_header_word_0);

                /* Clear checksum flag. */
                packet_ptr -> nx_packet_interface_capability_flag  &= (ULONG)(~NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM);
            }

            /* No necessary to process next protocol. */
            is_done = NX_TRUE;
            break;
        }

        case NX_PROTOCOL_IGMP:
        {

            /* Check if IGMP checksum is enabled. */
            if (packet_ptr -> nx_packet_interface_capability_flag  & NX_INTERFACE_CAPABILITY_IGMP_TX_CHECKSUM)
            {

                /* Pickup the pointer to the head of the IGMP packet.  */
                /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                igmp_header_ptr =  (NX_IGMP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

                /* Change the endian.  */
                NX_CHANGE_ULONG_ENDIAN(igmp_header_ptr -> nx_igmp_header_word_0);
                NX_CHANGE_ULONG_ENDIAN(igmp_header_ptr -> nx_igmp_header_word_1);

                /* Calculate the checksum.  */
                val =       igmp_header_ptr -> nx_igmp_header_word_0;
                checksum =  (val >> NX_SHIFT_BY_16);
                checksum += (val & NX_LOWER_16_MASK);
                val =      igmp_header_ptr -> nx_igmp_header_word_1;
                checksum += (val >> NX_SHIFT_BY_16);
                checksum += (val & NX_LOWER_16_MASK);

                /* Add in the carry bits into the checksum.  */
                checksum = (checksum >> NX_SHIFT_BY_16) + (checksum & NX_LOWER_16_MASK);

                /* Do it again in case previous operation generates an overflow.  */
                checksum = (checksum >> NX_SHIFT_BY_16) + (checksum & NX_LOWER_16_MASK);

                /* Place the checksum into the first header word.  */
                igmp_header_ptr -> nx_igmp_header_word_0 =  igmp_header_ptr -> nx_igmp_header_word_0 | (~checksum & NX_LOWER_16_MASK);

                /* Change the endian.  */
                NX_CHANGE_ULONG_ENDIAN(igmp_header_ptr -> nx_igmp_header_word_0);
                NX_CHANGE_ULONG_ENDIAN(igmp_header_ptr -> nx_igmp_header_word_1);

                /* Clear checksum flag. */
                packet_ptr -> nx_packet_interface_capability_flag  &= (ULONG)(~NX_INTERFACE_CAPABILITY_IGMP_TX_CHECKSUM);
            }

            /* No necessary to process next protocol. */
            is_done = NX_TRUE;
            break;
        }

#ifdef FEATURE_NX_IPV6
        case NX_PROTOCOL_ICMPV6:
        {

            /* Check if ICMPv6 checksum is enabled. */
            if (packet_ptr -> nx_packet_interface_capability_flag  & NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM)
            {

                /* Calculate the ICMPv6 checksum without protection.  */
                checksum =  _nx_ip_checksum_compute(packet_ptr, NX_PROTOCOL_ICMPV6,
                                                    data_length,
                                                    ip_src_addr, ip_dst_addr);

                /* Pickup the pointer to the head of the ICMPv6 packet.  */
                /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                icmpv6_header_ptr =  (NX_ICMPV6_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

                short_val = (USHORT) ~checksum;

                /* Move the checksum into header.  */
                NX_CHANGE_USHORT_ENDIAN(short_val);
                icmpv6_header_ptr -> nx_icmpv6_header_checksum = short_val;

                /* Clear checksum flag. */
                packet_ptr -> nx_packet_interface_capability_flag  &= (ULONG)(~NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM);
            }

            /* No necessary to process next protocol. */
            is_done = NX_TRUE;
            break;
        }

        case NX_PROTOCOL_IPV6:
        {

            /* Points to the base of IPv6 header. */
            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            ipv6_header_ptr = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

            /* Get src and dst addresses. */
            COPY_IPV6_ADDRESS(ipv6_header_ptr -> nx_ip_header_source_ip, ip_src_addr);
            COPY_IPV6_ADDRESS(ipv6_header_ptr -> nx_ip_header_destination_ip, ip_dst_addr);
            NX_IPV6_ADDRESS_CHANGE_ENDIAN(ip_src_addr);
            NX_IPV6_ADDRESS_CHANGE_ENDIAN(ip_dst_addr);

            /* Get next protocol. */
            val = ipv6_header_ptr -> nx_ip_header_word_1;
            NX_CHANGE_ULONG_ENDIAN(val);
            next_protocol = (val >> 8) & 0xFF;

            /* Remove IPv6 header. */
            packet_ptr -> nx_packet_prepend_ptr += (ULONG)sizeof(NX_IPV6_HEADER);
            data_length = packet_ptr -> nx_packet_length - (ULONG)sizeof(NX_IPV6_HEADER);
            break;
        }
#endif

        default:
            /* Unsupported protocol. */
            is_done = NX_TRUE;
            break;
        }
    }


    /* Restore origianl prepend_ptr. */
    packet_ptr -> nx_packet_prepend_ptr = org_prepend_ptr;
    return;
}
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

