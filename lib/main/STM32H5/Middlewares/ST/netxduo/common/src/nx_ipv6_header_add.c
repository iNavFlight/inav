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
#include "nx_packet.h"
#include "nx_icmpv6.h"

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */

#ifdef FEATURE_NX_IPV6


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv6_header_add                                 PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function prepends an IPv6 header.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_pptr                           Pointer to packet to send     */
/*    protocol                              Protocol being encapsulated   */
/*    payload_size                          Size of the payload           */
/*    hop_limit                             Hop limit value to set in IP  */
/*                                             header.                    */
/*    src_address                           Source address                */
/*    dest_address                          Destination address           */
/*    fragment                              Fragmentable or not           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_transmit_release           Release transmit packet       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ipv6_packet_send                  IPv6 packet transmit process  */
/*    _nx_icmpv6_send_ns                    Send NS packet                */
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
UINT _nx_ipv6_header_add(NX_IP *ip_ptr, NX_PACKET **packet_pptr,
                         ULONG protocol, ULONG payload_size, ULONG hop_limit,
                         ULONG *src_address, ULONG *dest_address, ULONG *fragment)
{

NX_IPV6_HEADER            *ip_header_ptr;
NX_PACKET                 *packet_ptr = *packet_pptr;
#ifdef NX_IPSEC_ENABLE
UINT                       status = NX_SUCCESS;
UCHAR                      is_hw_processed = NX_FALSE;
USHORT                     short_val;
#endif /* NX_IPSEC_ENABLE */

#if defined(NX_DISABLE_IP_INFO) && !defined(NX_IPSEC_ENABLE) && !defined(NX_ENABLE_IP_PACKET_FILTER)
    NX_PARAMETER_NOT_USED(ip_ptr);
#endif

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    if (fragment)
    {
#ifndef NX_DISABLE_FRAGMENTATION
        /* By default, it is fragmentable. */
        *fragment = NX_TRUE;
#else
        /* By default, it is not fragmentable. */
        *fragment = NX_FALSE;
#endif /* NX_DISABLE_FRAGMENTATION */
    }

#ifndef NX_DISABLE_IP_INFO

    /* Increment the total send requests counter.  */
    ip_ptr -> nx_ip_total_packet_send_requests++;
#endif

    /* Initialize the IP header incase this function returns fail. */
    packet_ptr -> nx_packet_ip_header = packet_ptr -> nx_packet_prepend_ptr;

#ifdef NX_IPSEC_ENABLE
    /* Check if this packet is continued after HW crypto engine. */
    if (packet_ptr -> nx_packet_ipsec_sa_ptr &&
        ((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_mode == NX_IPSEC_TRANSPORT_MODE &&
        (packet_ptr -> nx_packet_ipsec_state == NX_IPSEC_AH_PACKET ||
         packet_ptr -> nx_packet_ipsec_state == NX_IPSEC_ESP_PACKET))
    {
        is_hw_processed = NX_TRUE;
    }

    /* IPsec transport mode enabled? */
    if (packet_ptr -> nx_packet_ipsec_sa_ptr &&
        packet_ptr -> nx_packet_ipsec_state != NX_IPSEC_ESP_PACKET &&
        packet_ptr -> nx_packet_ipsec_state != NX_IPSEC_AH_PACKET &&
        ((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_mode == NX_IPSEC_TRANSPORT_MODE)
    {

        /* Yes, process the packet */
        status = _nx_ipsec_ip_output_packet_process(ip_ptr, &packet_ptr, protocol, payload_size, (&payload_size));

        /* Check for errors. */
        if ((status != NX_SUCCESS) &&
            (status != NX_IPSEC_HW_PENDING))
        {

            /* IPsec output packet process failed. */

            /* Release the packet.  */
            _nx_packet_transmit_release(packet_ptr);

            return(status);
        }

        /* Update the packet pointer. */
        *packet_pptr = packet_ptr;

        /* Change protocol to ESP or AH. */
        protocol = (((NX_IPSEC_SA *)packet_ptr -> nx_packet_ipsec_sa_ptr) -> nx_ipsec_sa_protocol);

#ifndef NX_DISABLE_FRAGMENTATION
        /* Set the fragment flag to false. Transport mode SAs have been defined to not carry fragments (IPv4 or IPv6), RFC 4301 page 66 and page 88.*/
        if (fragment)
        {
            *fragment = NX_FALSE;
        }
#endif /* NX_DISABLE_FRAGMENTATION */
    }
#endif /* NX_IPSEC_ENABLE  */

#ifdef NX_IPSEC_ENABLE
    if (!is_hw_processed)
    {
#endif /* NX_IPSEC_ENABLE  */
        /* Prepend the IP header to the packet.  First, make room for the IP header.  */
        packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_IPV6_HEADER);

        /* Increase the packet length.  */
        packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length + (ULONG)sizeof(NX_IPV6_HEADER);

        /* Increase header length. */
        packet_ptr -> nx_packet_ip_header_length = (UCHAR)(packet_ptr -> nx_packet_ip_header_length +
                                                           sizeof(NX_IPV6_HEADER));


        /* If the interface IP address is not valid (in DAD state), only ICMP is allowed */
        if (packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_state != NX_IPV6_ADDR_STATE_VALID)
        {

#ifndef NX_DISABLE_IPV6_DAD
        NX_ICMPV6_HEADER *icmpv6_header = (NX_ICMPV6_HEADER *)(packet_ptr -> nx_packet_prepend_ptr +
                                                               packet_ptr -> nx_packet_ip_header_length);

            /* Interface IP address is invalid.  Before dropping the outgoing packet,
               check whether the interface address is in tentative state and the protocol
               is ICMPv6-DAD. */

            /* This check is needed only if DAD is not disabled.
               If DAD is disabled, we drop the packet. */
            if (!((protocol == NX_PROTOCOL_ICMPV6) &&
                  (packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_state == NX_IPV6_ADDR_STATE_TENTATIVE) &&
                  (icmpv6_header -> nx_icmpv6_header_type == NX_ICMPV6_NEIGHBOR_SOLICITATION_TYPE)))
#endif /* NX_DISABLE_IPV6_DAD */
            {
#ifndef NX_DISABLE_IP_INFO

                /* Increment the IP invalid packet error.  */
                ip_ptr -> nx_ip_invalid_transmit_packets++;
#endif

                /* Release the packet.  */
                _nx_packet_transmit_release(packet_ptr);

                /* Return... nothing more can be done!  */
                return(NX_NO_INTERFACE_ADDRESS);
            }
        }

        /* If the IP header won't fit, return an error.  */
        /*lint -e{946} suppress pointer subtraction, since it is necessary. */
        NX_ASSERT(packet_ptr -> nx_packet_prepend_ptr >= packet_ptr -> nx_packet_data_start);

        /* Build the IP header.  */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ip_header_ptr = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_prepend_ptr;
        packet_ptr -> nx_packet_ip_header = packet_ptr -> nx_packet_prepend_ptr;

        /* bits 31-28: IP version.  Bits 27-20: Traffic Class.  Bits 19-00: Flow Lable */
        ip_header_ptr -> nx_ip_header_word_0 = (ULONG)(6 << 28);
        NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_0);

        /* bits 31-16: payload size.  Bits 15-8: Next Header.   Bits 7-0 Hop limit */
        /* ip_header_ptr -> nx_ip_header_word_1 = (payload_size << 16) | (protocol << 8) | (ip_ptr -> nx_ipv6_hop_limit);*/
        ip_header_ptr -> nx_ip_header_word_1 = (payload_size << 16) | (protocol << 8) | (hop_limit);
        NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_1);


        /* Fill in local IPv6 address as sender's address*/
        COPY_IPV6_ADDRESS(src_address, ip_header_ptr -> nx_ip_header_source_ip);

        COPY_IPV6_ADDRESS(dest_address, ip_header_ptr -> nx_ip_header_destination_ip);

        /* Fix endianness */
        NX_IPV6_ADDRESS_CHANGE_ENDIAN(ip_header_ptr -> nx_ip_header_destination_ip);
        NX_IPV6_ADDRESS_CHANGE_ENDIAN(ip_header_ptr -> nx_ip_header_source_ip);

#ifdef NX_IPSEC_ENABLE
    }
    else
    {

        /* Fix payload size.  */
        /* Build the IP header.  */
        ip_header_ptr = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

        payload_size -= sizeof(NX_IPV6_HEADER);
        short_val = (USHORT)payload_size;
        NX_CHANGE_USHORT_ENDIAN(short_val);
        payload_size = short_val;

        /* First clear payload_size field.  */
        ip_header_ptr -> nx_ip_header_word_1 &= 0xFFFF0000;

        /* Fill payload_size field.  */
        ip_header_ptr -> nx_ip_header_word_1 |= short_val;
    }

    /* IPsec tunnel mode. */
    if (packet_ptr -> nx_packet_ipsec_sa_ptr &&
        packet_ptr -> nx_packet_ipsec_state != NX_IPSEC_ESP_PACKET &&
        packet_ptr -> nx_packet_ipsec_state != NX_IPSEC_AH_PACKET &&
        ((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_mode == NX_IPSEC_TUNNEL_MODE)
    {
        status = _nx_ipsec_ip_output_packet_process(ip_ptr, &packet_ptr, NX_PROTOCOL_IPV6, (ULONG)payload_size, (ULONG *)(&payload_size));

        if ((status != NX_SUCCESS) &&
            (status != NX_IPSEC_HW_PENDING))
        {
            /* IPsec output packet process failed. */

            /* Release the packet.  */
            _nx_packet_transmit_release(packet_ptr);

            return(status);
        }

        /* Update the packet pointer. */
        *packet_pptr = packet_ptr;

        /* Tunnel consume the packet. */
        return(NX_IPSEC_PKT_CONT);
    }

    /* ICV calculation before the packet sent over the wire if packet went through AH processing. */
    if (packet_ptr -> nx_packet_ipsec_sa_ptr &&
        packet_ptr -> nx_packet_ipsec_state == NX_IPSEC_AH_PACKET)
    {
        status = ip_ptr -> nx_ip_ipsec_authentication_header_transmit(ip_ptr, &packet_ptr, protocol, 1);

        if ((status != NX_SUCCESS) &&
            (status != NX_IPSEC_HW_PENDING))
        {
            /* Release the packet.  */
            _nx_packet_transmit_release(packet_ptr);

            return(status);
        }

        /* Update the packet pointer. */
        *packet_pptr = packet_ptr;
    }

    /* HW crypto driver is processing packet. */
    if (status == NX_IPSEC_HW_PENDING)
    {

#ifndef NX_DISABLE_IP_INFO

        /* Decrement the total send requests counter.  */
        ip_ptr -> nx_ip_total_packet_send_requests--;
#endif
        return(status);
    }

#endif /* NX_IPSEC_ENABLE */

    return(NX_SUCCESS);
}

#endif /* FEATURE_NX_IPV6 */

