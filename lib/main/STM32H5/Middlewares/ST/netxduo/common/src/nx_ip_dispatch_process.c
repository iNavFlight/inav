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
#include "nx_ipv6.h"
#include "nx_icmp.h"
#include "nx_packet.h"


#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_dispatch_process                             PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function goes through IP header and option fields, and         */
/*    dispatches into various process routines depending on the header    */
/*    options.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    packet_ptr                            Incoming IP packet            */
/*    protocol                              The first protocol immediately*/
/*                                            following IP header         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                0: do not drop packet         */
/*                                          1: drop packet                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_ip_icmpv6_packet_process]         ICMPv6 header process         */
/*    [nx_ip_tcp_packet_receive]            TCP packet process            */
/*    [nx_ip_udp_packet_receive]            UDP packet process            */
/*                                            ICMP ping request           */
/*    _nx_ipv6_process_hop_by_hop_option    IPv6 hop by hop option        */
/*                                            process                     */
/*    NX_ICMPV6_SEND_PARAMETER_PROBELM      Send ICMP parameter problem   */
/*    _nx_ipv6_process_routing_option       IPv6 routing option process   */
/*    _nx_ipv6_process_fragment_option      IPv6 fragment option process  */
/*    [nx_ipsec_authentication_header_receive]                            */
/*                                          IPSec authentication header   */
/*                                            process                     */
/*    [nx_ipsec_encapsulating_security_payload_receive                    */
/*                                          IPSec encapsulating security  */
/*                                            payload process             */
/*    (ip_icmp_packet_receive)              Receive a ICMP packet         */
/*    (ip_igmp_packet_receive)              Receive a IGMP packet         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), fixed    */
/*                                            destination header check,   */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Yuxin Zhou               Modified comment(s), expanded */
/*                                            protocols support for raw   */
/*                                            packet,                     */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
UINT _nx_ip_dispatch_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr, UINT protocol)
{

UINT              drop_packet;

#ifdef FEATURE_NX_IPV6
NXD_IPV6_ADDRESS *incoming_addr;
UINT              next_option_offset;

#ifndef NX_DISABLE_ICMPV6_ERROR_MESSAGE
UINT              nx_packet_option_offset;
#endif /* NX_DISABLE_ICMPV6_ERROR_MESSAGE  */
#endif /* FEATURE_NX_IPV6 */

#ifdef NX_IPSEC_ENABLE
UINT              ret;
ULONG             next_protocol = 0;
NXD_ADDRESS       src_addr, dest_addr;
#ifndef NX_DISABLE_IPV4
NX_IPV4_HEADER   *ipv4_header;
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
NX_IPV6_HEADER   *ipv6_header;
NX_ICMPV6_HEADER *icmp_header_ptr;
#endif /* FEATURE_NX_IPV6 */
#endif /* NX_IPSEC_ENABLE */


    /* Initialize local variables. */
    drop_packet = 0;
#ifdef FEATURE_NX_IPV6
    next_option_offset = (UINT)sizeof(NX_IPV6_HEADER);
    incoming_addr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr;
#endif /* FEATURE_NX_IPV6 */

    /* Parse all options in the packet till we're done or an error is encountered. */
    while (!drop_packet)
    {

        /* Add debug information. */
        NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

#if defined(FEATURE_NX_IPV6) && !defined(NX_DISABLE_ICMPV6_ERROR_MESSAGE)
        /* Set a local variable for convenience. */
        nx_packet_option_offset = packet_ptr -> nx_packet_option_offset;
#endif /* defined(FEATURE_NX_IPV6) && !defined(NX_DISABLE_ICMPV6_ERROR_MESSAGE) */
        switch (protocol)
        {

#ifdef FEATURE_NX_IPV6
        case NX_PROTOCOL_NEXT_HEADER_HOP_BY_HOP:

            /* This should be the first header; if it is not, this is a malformed packet. */
            if (packet_ptr -> nx_packet_option_state >= (UCHAR)HOP_BY_HOP_HEADER)
            {

                drop_packet = 1;

#ifndef NX_DISABLE_ICMPV6_ERROR_MESSAGE
                NX_ICMPV6_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, 1, nx_packet_option_offset);
#endif /* NX_DISABLE_ICMPV6_ERROR_MESSAGE */
            }
            else
            {

                /* Start the option header handling. */
                packet_ptr -> nx_packet_option_state = (UCHAR)HOP_BY_HOP_HEADER;

                /* Dispatch packet to the Option handler. */
                drop_packet = _nx_ipv6_process_hop_by_hop_option(ip_ptr, packet_ptr);
            }

            break;

        case NX_PROTOCOL_NEXT_HEADER_DESTINATION:

            /* Invalid header option if we have already processed 1 destination option. */
            if (packet_ptr -> nx_packet_destination_header >= 1)
            {

                /* If we already have processed one destination option, we expect this
                   to be the second one. */
                if ((packet_ptr -> nx_packet_option_state < (UCHAR)DESTINATION_HEADER_1) ||
                    (packet_ptr -> nx_packet_destination_header > 1))
                {
                    drop_packet = 1;
                }
                else
                {
                    packet_ptr -> nx_packet_option_state = (UCHAR)DESTINATION_HEADER_2;
                }
            }
            else
            {

                /* This is the first time we encounter a destination header option. */
                /* If we are before the routing header option, this must be the 1st one.
                   Otherwise, it must be the 2nd one. */

                if (packet_ptr -> nx_packet_option_state < (UCHAR)ROUTING_HEADER)
                {

                    packet_ptr -> nx_packet_option_state = (UCHAR)DESTINATION_HEADER_1;
                }
                else
                {
                    packet_ptr -> nx_packet_option_state = (UCHAR)DESTINATION_HEADER_2;
                }
            }

            packet_ptr -> nx_packet_destination_header++;

            if (!drop_packet)
            {
                /* Proceed with hop by hop handling if there are no errors. */
                drop_packet = _nx_ipv6_process_hop_by_hop_option(ip_ptr, packet_ptr);
            }
#ifndef NX_DISABLE_ICMPV6_ERROR_MESSAGE
            else
            {

                /* Return an error message to the sender of the packet. */
                NX_ICMPV6_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, 1, nx_packet_option_offset);
            }
#endif /* NX_DISABLE_ICMPV6_ERROR_MESSAGE */

            break;

        case NX_PROTOCOL_NEXT_HEADER_ROUTING:

            if (packet_ptr -> nx_packet_option_state >= (UCHAR)ROUTING_HEADER)
            {

#ifndef NX_DISABLE_ICMPV6_ERROR_MESSAGE
                NX_ICMPV6_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, 1, nx_packet_option_offset);
#endif /* NX_DISABLE_ICMPV6_ERROR_MESSAGE */

                drop_packet = 1;
            }
            else
            {

                packet_ptr -> nx_packet_option_state = (UCHAR)ROUTING_HEADER;

                drop_packet = _nx_ipv6_process_routing_option(ip_ptr, packet_ptr);
            }
            break;

        case NX_PROTOCOL_NEXT_HEADER_FRAGMENT:

#ifndef NX_DISABLE_FRAGMENTATION
            if (packet_ptr -> nx_packet_option_state >= (UCHAR)FRAGMENT_HEADER)
            {
#endif /* NX_DISABLE_FRAGMENTATION */

#ifndef NX_DISABLE_ICMPV6_ERROR_MESSAGE
                NX_ICMPV6_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, 1, nx_packet_option_offset);
#endif /* NX_DISABLE_ICMPV6_ERROR_MESSAGE */

                drop_packet = 1;
#ifndef NX_DISABLE_FRAGMENTATION
            }
            else
            {

                packet_ptr -> nx_packet_option_state = (UCHAR)FRAGMENT_HEADER;

#ifdef NX_ENABLE_LOW_WATERMARK
                if (packet_ptr -> nx_packet_pool_owner -> nx_packet_pool_available >=
                    packet_ptr -> nx_packet_pool_owner -> nx_packet_pool_low_watermark)
#endif
                {
                    drop_packet = _nx_ipv6_process_fragment_option(ip_ptr, packet_ptr);
                }
#ifdef NX_ENABLE_LOW_WATERMARK
                else
                {
                    drop_packet = NX_POOL_ERROR;
                }
#endif

                if (drop_packet != NX_CONTINUE)
                {

                    /* Special case: do not further process the packet here.
                       Once all fragments are received, we will continue processing the headers. */
                    return(drop_packet);
                }
                else
                {

                    /* Continue processing the packet. */
                    drop_packet = 0;
                }
            }
#endif /* NX_DISABLE_FRAGMENTATION */
            break;

        case NX_PROTOCOL_NO_NEXT_HEADER:

            drop_packet = 1;
            break;

#endif /* FEATURE_NX_IPV6 */

        case NX_PROTOCOL_NEXT_HEADER_AUTHENTICATION:

#ifdef NX_IPSEC_ENABLE
            if (ip_ptr -> nx_ip_ipsec_authentication_header_receive == NX_NULL)
            {

                /* If IPsec is not enabled by the application, drop the packet. */
                return(1);
            }
            else
            {

                ret =  ip_ptr -> nx_ip_ipsec_authentication_header_receive(ip_ptr, packet_ptr, &next_protocol, &packet_ptr);

                if (ret == NX_SUCCESS)
                {

                    /* Indicate that IPSec consumed the packet. */
                    return(0);
                }

                if (ret != NX_IPSEC_PKT_CONT)
                {

                    return(1);
                }

                /* Continue processing the packet if status = NX_IPSEC_PKT_CONT */
            }
#else /* NX_IPSEC_ENABLE */

            /* Drop this packet if IPsec module is not present. */
            drop_packet = 1;
#endif /* NX_IPSEC_ENABLE */

            break;

        case NX_PROTOCOL_NEXT_HEADER_ENCAP_SECURITY:

#ifdef NX_IPSEC_ENABLE
            if (ip_ptr -> nx_ip_ipsec_encapsulating_security_payload_receive == NX_NULL)
            {

                /* If IPsec is not enabled by the application, drop the packet. */
                return(1);
            }
            else
            {

                ret =  ip_ptr -> nx_ip_ipsec_encapsulating_security_payload_receive(ip_ptr, packet_ptr, &next_protocol, &packet_ptr);

                if (ret == NX_SUCCESS)
                {

                    /* Indicate IPSec consumed the packet. */
                    return(0);
                }

                if (ret != NX_IPSEC_PKT_CONT)
                {
                    return(1);
                }

                /* Continue processing the packet if status = NX_IPSEC_PKT_CONT */
            }
            break;

#else /* NX_IPSEC_ENABLE */
            /* Drop this packet if IPsec module is not present. */
            return(1);
#endif /* NX_IPSEC_ENABLE */

        default:

            /* Not part of the IP headers. */
#ifdef NX_IPSEC_ENABLE
            /* Check ingress_sa for packet that is not ESP or AH.  */
            if (packet_ptr -> nx_packet_ipsec_sa_ptr == NX_NULL)
            {

                /* Get source and destination address.  */
#ifdef FEATURE_NX_IPV6
                if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
                {
                    ipv6_header = (NX_IPV6_HEADER *)(packet_ptr -> nx_packet_ip_header);

                    src_addr.nxd_ip_version = NX_IP_VERSION_V6;
                    dest_addr.nxd_ip_version = NX_IP_VERSION_V6;

                    COPY_IPV6_ADDRESS(ipv6_header -> nx_ip_header_source_ip,
                                      src_addr.nxd_ip_address.v6);


                    COPY_IPV6_ADDRESS(ipv6_header -> nx_ip_header_destination_ip,
                                      dest_addr.nxd_ip_address.v6);
                }
                else
#endif /* FEATURE_NX_IPV6 */
                {
#ifndef NX_DISABLE_IPV4
                    ipv4_header = (NX_IPV4_HEADER *)(packet_ptr -> nx_packet_ip_header);

                    src_addr.nxd_ip_version = NX_IP_VERSION_V4;
                    dest_addr.nxd_ip_version = NX_IP_VERSION_V4;

                    src_addr.nxd_ip_address.v4 = ipv4_header -> nx_ip_header_source_ip;
                    dest_addr.nxd_ip_address.v4 = ipv4_header -> nx_ip_header_destination_ip;
#endif /* NX_DISABLE_IPV4 */
                }

                if (_nx_ipsec_sa_ingress_lookup(ip_ptr, &src_addr, &dest_addr, 0, (UCHAR)protocol,
                                                NX_NULL, packet_ptr -> nx_packet_prepend_ptr) != NX_IPSEC_TRAFFIC_BYPASS)
                {
#ifdef FEATURE_NX_IPV6
                    /* Check whether it is a NA packet.  */
                    if (protocol == NX_PROTOCOL_ICMPV6)
                    {

                        /* Bypass NA packet. */
                        icmp_header_ptr = (NX_ICMPV6_HEADER *)packet_ptr -> nx_packet_prepend_ptr;
                        if (icmp_header_ptr -> nx_icmpv6_header_type != NX_ICMPV6_NEIGHBOR_ADVERTISEMENT_TYPE)
                        {
                            return(NX_INVALID_PACKET);
                        }
                    }
                    else
                    {
#endif /* FEATURE_NX_IPV6 */
                        return(NX_INVALID_PACKET);
#ifdef FEATURE_NX_IPV6
                    }
#endif /* FEATURE_NX_IPV6 */
                }
            }
            /* For IPsec tunnel mode, next protocol is checked here. */
            else if (((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_mode == NX_IPSEC_TUNNEL_MODE)
            {
                if (_nx_ipsec_sa_ingress_selector_check(packet_ptr -> nx_packet_prepend_ptr,
                                                        (UCHAR)protocol,
                                                        ((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_selector_ptr) == NX_IPSEC_TRAFFIC_DROP)
                {
                    _nx_packet_release(packet_ptr);     /* Consume the packet */
                    return(NX_INVALID_PACKET);
                }
            }
#endif /* NX_IPSEC_ENABLE */

#if defined(NX_ENABLE_IP_RAW_PACKET_ALL_STACK) && defined(NX_ENABLE_IP_RAW_PACKET_FILTER)
            if ((ip_ptr -> nx_ip_raw_ip_processing) && (ip_ptr -> nx_ip_raw_packet_filter))
            {

                /* Let RAW packet filter handler filter all incoming packets.  */
                if ((ip_ptr -> nx_ip_raw_ip_processing)(ip_ptr, protocol << 16, packet_ptr) == NX_SUCCESS)
                {
                    /* No need to free the packet as it is consumed by the raw process */
                    return(0);
                }
            }
#endif /* defined(NX_ENABLE_IP_RAW_PACKET_ALL_STACK) && defined(NX_ENABLE_IP_RAW_PACKET_FILTER) */

            if (protocol == NX_PROTOCOL_TCP)
            {
#ifdef FEATURE_NX_IPV6
                if ((packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4) ||
                    ((packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6) &&
                     (incoming_addr -> nxd_ipv6_address_state == NX_IPV6_ADDR_STATE_VALID)))
                {
#endif /* FEATURE_NX_IPV6 */

                    /* Check that the host is enabled for TCP. */
                    if (ip_ptr -> nx_ip_tcp_packet_receive)
                    {

                        /* Dispatch the packet to the TCP packet handler. */
                        (ip_ptr -> nx_ip_tcp_packet_receive)(ip_ptr, packet_ptr);

                        /* No need to free the packet as it is consumed by TCP packet receive.  */
                        return(0);
                    }
#ifdef FEATURE_NX_IPV6
                }
#endif /* FEATURE_NX_IPV6 */

                /* TCP is not enabled.  Drop the packet. */
                drop_packet = 1;
            }

#ifdef FEATURE_NX_IPV6
            else if ((packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6) &&
                     (protocol == NX_PROTOCOL_ICMPV6))
            {

                /* Check that ICMPv6 is enabled for this IP instance.  */
                if (ip_ptr -> nx_ip_icmpv6_packet_process != NX_NULL)
                {

                    /* Forward to the ICMPv6 packet handler. */
                    ip_ptr -> nx_ip_icmpv6_packet_process(ip_ptr, packet_ptr);

                    /*  no need to free packet as it is consumed by ICMP packet receive.  */
                    return(0);
                }

                /* ICMPv6 is not enabled.  Drop the packet. */
                drop_packet = 1;
            }
#endif /* FEATURE_NX_IPV6 */

#ifndef NX_DISABLE_IPV4
            else if ((packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4) &&
                     (protocol == NX_PROTOCOL_ICMP))
            {

                /* Check that ICMP is enabled for this IP instance.  */
                if (ip_ptr -> nx_ip_icmp_packet_receive != NX_NULL)
                {

                    /* Yes, a ICMP packet is present, dispatch to the appropriate ICMP handler
                       if present.  */
                    ip_ptr -> nx_ip_icmp_packet_receive(ip_ptr, packet_ptr);
                    return(0);
                }

                /* ICMP is not enabled. Drop the packet. */
                drop_packet = 1;
            }
            else if ((packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4) &&
                     (protocol == NX_PROTOCOL_IGMP))
            {
                if (ip_ptr -> nx_ip_igmp_packet_receive != NX_NULL)
                {

                    /* Yes, a IGMP packet is present, dispatch to the appropriate ICMP handler
                       if present.  */
                    ip_ptr -> nx_ip_igmp_packet_receive(ip_ptr, packet_ptr);
                    return(0);
                }

                /* IGMP is not enabled. Drop the packet.  */
                drop_packet = 1;
            }
#endif /* NX_DISABLE_IPV4 */
            else if (protocol == NX_PROTOCOL_UDP)
            {

#ifdef FEATURE_NX_IPV6
                if ((packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4) ||
                    ((packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6) &&
                     (incoming_addr -> nxd_ipv6_address_state == NX_IPV6_ADDR_STATE_VALID)))
                {
#endif /* FEATURE_NX_IPV6 */

                    /* Check the host is enabled for UDP packet handling. */
                    if (ip_ptr -> nx_ip_udp_packet_receive)
                    {

                        /* Dispatch the packet to the UDP handler. */
                        (ip_ptr -> nx_ip_udp_packet_receive)(ip_ptr, packet_ptr);

                        /* No need to free the packet as it is consumed by UDP packet receive.  */
                        return(0);
                    }
#ifdef FEATURE_NX_IPV6
                }
#endif /* FEATURE_NX_IPV6 */

                /* UDP is not enabled.  Drop the packet. */
                drop_packet = 1;
            }
            else
            {
                if (ip_ptr -> nx_ip_raw_ip_processing)
                {
#if defined(NX_ENABLE_IP_RAW_PACKET_ALL_STACK) && defined(NX_ENABLE_IP_RAW_PACKET_FILTER)
                    if (ip_ptr -> nx_ip_raw_packet_filter == NX_NULL)
#endif /* defined(NX_ENABLE_IP_RAW_PACKET_ALL_STACK) && defined(NX_ENABLE_IP_RAW_PACKET_FILTER) */
                    {
                        if ((ip_ptr -> nx_ip_raw_ip_processing)(ip_ptr, protocol << 16, packet_ptr) == NX_SUCCESS)
                        {
                            /* No need to free the packet as it is consumed by the raw process */
                            return(0);
                        }
                    }
                }

#if !defined(NX_DISABLE_IPV4) && !defined(NX_DISABLE_ICMPV4_ERROR_MESSAGE)
                /* Unknown protocol, send ICMP Destination protocol unreachable. */
                if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
                {
                    NX_ICMPV4_SEND_DEST_UNREACHABLE(ip_ptr, packet_ptr, NX_ICMP_PROTOCOL_UNREACH_CODE);
                }
#endif /* !NX_DISABLE_IPV4 && !NX_DISABLE_ICMPV4_ERROR_MESSAGE  */

#ifdef FEATURE_NX_IPV6
                /* Unknown option.  Send ICMP Parameter problem and discard the packet. */
                /* RFC 2460, page 7 */
#ifndef NX_DISABLE_ICMPV6_ERROR_MESSAGE
                if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
                {
                    NX_ICMPV6_SEND_PARAMETER_PROBLEM(ip_ptr, packet_ptr, 1, nx_packet_option_offset);
                }
#endif /* NX_DISABLE_ICMPV6_ERROR_MESSAGE */
#endif /* FEATURE_NX_IPV6 */

#ifndef NX_DISABLE_IP_INFO

                /* Increment the IP unknown protocol count.  */
                ip_ptr -> nx_ip_unknown_protocols_received++;

#endif /* NX_DISABLE_IP_INFO */


                drop_packet = 1;
            }
            break;
        }


        /* If the previous header is processed without errors, move on to the next optional
           header. */
        if (!drop_packet)
        {

#ifdef FEATURE_NX_IPV6
        NX_IPV6_HEADER_OPTION *option;
        ULONG                  option_hdr_len;
#endif /* FEATURE_NX_IPV6 */

#ifdef NX_IPSEC_ENABLE

            if (protocol == NX_PROTOCOL_NEXT_HEADER_ENCAP_SECURITY ||
                protocol == NX_PROTOCOL_NEXT_HEADER_AUTHENTICATION)
            {

                /* After ESP and AH processing, ESP and AH hdr are removed. */
                protocol = next_protocol;
                continue;
            }
#endif  /* NX_IPSEC_ENABLE */

#ifdef FEATURE_NX_IPV6
            if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
            {

                /* Find the option we just processed. */
                /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                option = (NX_IPV6_HEADER_OPTION *)packet_ptr -> nx_packet_prepend_ptr;

                /* Check the protocol.  */
                if (protocol == NX_PROTOCOL_NEXT_HEADER_FRAGMENT)
                {

                    /* Fixed length for fragment option, the field of option length is reserved.  */
                    option_hdr_len = sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION);
                }
                else
                {

                    /* Compute the current option length. */
                    /* For other IPv6 optional headers, hdr_ext_len is expressed in 64-bit words. */
                    option_hdr_len = (ULONG)((option -> nx_ipv6_header_option_ext_length + 1) << 3);
                }

                /* Obtain the next option header type. */
                protocol = option -> nx_ipv6_header_option_next_header;

                if (((ALIGN_TYPE)(packet_ptr -> nx_packet_prepend_ptr) + option_hdr_len) <
                    (ALIGN_TYPE)(packet_ptr -> nx_packet_append_ptr))
                {

                    /* Advance to the next header. */
                    packet_ptr -> nx_packet_prepend_ptr += option_hdr_len;
                    packet_ptr -> nx_packet_length      -= option_hdr_len;
                }
                else
                {

                    drop_packet = 1;
                }

                /*
                   Advance the nx_packet_option_offset as well.
                   Option Offset is used when constructing ICMPv6 parameter problem message.
                 */

                packet_ptr -> nx_packet_option_offset = (USHORT)next_option_offset;

                /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
                next_option_offset = (UINT)(packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_ip_header);
            }
#endif /* FEATURE_NX_IPV6 */
        }
        else
        {
#ifndef NX_DISABLE_IP_INFO

            /* Decrement the number of packets delivered.  */
            ip_ptr -> nx_ip_total_packets_delivered--;

            /* Decrement the IP packet bytes received (not including the header).  */
            ip_ptr -> nx_ip_total_bytes_received -=  packet_ptr -> nx_packet_length;

            /* Increment the IP receive packets dropped count.  */
            ip_ptr -> nx_ip_receive_packets_dropped++;
#endif /* NX_DISABLE_IP_INFO */
        }
    }

    return(drop_packet);
}

