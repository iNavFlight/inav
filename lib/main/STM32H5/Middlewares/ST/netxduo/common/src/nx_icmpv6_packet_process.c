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
/**   Internet Control Message Protocol (ICMP)                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_packet.h"
#include "nx_ip.h"
#include "nx_ipv6.h"
#include "nx_icmpv6.h"

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */


#ifdef FEATURE_NX_IPV6




/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_packet_process                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the ICMPv6 received packet, computes the    */
/*    ICMP header checksum, and determines ICMPv6 message type and which  */
/*    handler to process it.  It also lifts any associated threads        */
/*    suspended on it.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Received ICMP packet pointer  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_checksum_compute               Computer ICMP checksum        */
/*    _nx_packet_release                    Packet release function       */
/*    _tx_thread_system_preempt_check       Check for preemption          */
/*    _nx_icmpv6_process_echo_reply         Function that processes the   */
/*                                             echo reply message.        */
/*    _nx_icmpv6_process_echo_request       Function that processes the   */
/*                                             echo request message.      */
/*    _nx_icmpv6_process_ra                 Function that processes the   */
/*                                             router advertisement       */
/*                                             message.                   */
/*    _nx_icmpv6_process_na                 Function that processes the   */
/*                                             neighbor advertisement.    */
/*    _nx_icmpv6_process_ns                 Function that processes the   */
/*                                             neighbor solicitation      */
/*                                             message.                   */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmp_packet_process               Main ICMP packet processer    */
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
VOID  _nx_icmpv6_packet_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

NX_ICMPV6_HEADER *header_ptr;
USHORT            checksum;
#if defined(NX_DISABLE_ICMPV6_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT              compute_checksum = 1;
#endif /* defined(NX_DISABLE_ICMPV6_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
NX_IPV6_HEADER   *ipv6_header;


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Points to the ICMP message header.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    header_ptr =  (NX_ICMPV6_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

#ifdef NX_DISABLE_ICMPV6_RX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_ICMPV6_RX_CHECKSUM */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_ICMPV6_RX_CHECKSUM)
    {
        compute_checksum = 0;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
#ifdef NX_IPSEC_ENABLE
    if ((packet_ptr -> nx_packet_ipsec_sa_ptr != NX_NULL) && (((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_encryption_method != NX_CRYPTO_NONE))
    {
        compute_checksum = 1;
    }
#endif
#if defined(NX_DISABLE_ICMPV6_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_ICMPV6_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
    {
        /* Points to the IPv6 header. */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ipv6_header = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

        /* Calculate the ICMP message checksum.  */
        checksum =  _nx_ip_checksum_compute(packet_ptr, NX_PROTOCOL_ICMPV6,
                                            (UINT)packet_ptr -> nx_packet_length,
                                            (ipv6_header -> nx_ip_header_source_ip),
                                            (ipv6_header -> nx_ip_header_destination_ip));

        checksum =  (USHORT)(~checksum) & NX_LOWER_16_MASK;

        /* Determine if the checksum is valid.  */
        if (checksum)
        {

#ifndef NX_DISABLE_ICMP_INFO

            /* Increment the ICMP invalid packet error.  */
            ip_ptr -> nx_ip_icmp_invalid_packets++;

            /* Increment the ICMP checksum error count.  */
            ip_ptr -> nx_ip_icmp_checksum_errors++;
#endif

            /* Nope, the checksum is invalid.  Toss this ICMP packet out.  */
            _nx_packet_release(packet_ptr);
            return;
        }
    }

    /* Determine the message type and call the appropriate handler.  */
    if (header_ptr -> nx_icmpv6_header_type == NX_ICMPV6_ECHO_REPLY_TYPE)
    {
        _nx_icmpv6_process_echo_reply(ip_ptr, packet_ptr);
    }
    else if (header_ptr -> nx_icmpv6_header_type == NX_ICMPV6_ECHO_REQUEST_TYPE)
    {
        _nx_icmpv6_process_echo_request(ip_ptr, packet_ptr);
    }
    else if (header_ptr -> nx_icmpv6_header_type == NX_ICMPV6_NEIGHBOR_SOLICITATION_TYPE)
    {

        _nx_icmpv6_process_ns(ip_ptr, packet_ptr);
    }

#ifndef NX_DISABLE_ICMPV6_ROUTER_ADVERTISEMENT_PROCESS
    else if (header_ptr -> nx_icmpv6_header_type == NX_ICMPV6_ROUTER_ADVERTISEMENT_TYPE)
    {

        _nx_icmpv6_process_ra(ip_ptr, packet_ptr);
    }
#endif
    else if (header_ptr -> nx_icmpv6_header_type == NX_ICMPV6_NEIGHBOR_ADVERTISEMENT_TYPE)
    {

        _nx_icmpv6_process_na(ip_ptr, packet_ptr);
    }
#ifndef NX_DISABLE_ICMPV6_REDIRECT_PROCESS
    else if (header_ptr -> nx_icmpv6_header_type == NX_ICMPV6_REDIRECT_MESSAGE_TYPE)
    {

        _nx_icmpv6_process_redirect(ip_ptr, packet_ptr);
    }
#endif

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY
    else if (header_ptr -> nx_icmpv6_header_type == NX_ICMPV6_PACKET_TOO_BIG_TYPE)
    {
        _nx_icmpv6_process_packet_too_big(ip_ptr, packet_ptr);
    }
#endif /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */

    else
    {

#ifndef NX_DISABLE_ICMP_INFO

        /* Increment the ICMP unhandled message count.  */
        ip_ptr -> nx_ip_icmp_unhandled_messages++;
#endif

        /* Unhandled ICMP message, just release it.  */
        _nx_packet_release(packet_ptr);
    }
}


#endif /* FEATURE_NX_IPV6 */

