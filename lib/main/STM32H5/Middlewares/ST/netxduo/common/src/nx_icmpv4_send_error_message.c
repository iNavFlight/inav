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
/**   Internet Control Message Protocol (ICMPv4)                          */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_packet.h"
#include "nx_ip.h"
#include "nx_icmp.h"

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */

#if !defined(NX_DISABLE_IPV4) && !defined(NX_DISABLE_ICMPV4_ERROR_MESSAGE)
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv4_send_error_message                      PORTABLE C       */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called by various IPv4 components to send an       */
/*    error message when necessary.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP stack instance             */
/*    offending_packet                      The packet that caused the    */
/*                                              error.                    */
/*    word1                                 ICMPv4 error message header   */
/*                                              field, progarmmed by      */
/*                                              the caller.               */
/*    error_pointer                         Pointer to the byte that      */
/*                                              caused the error          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_checksum_compute               Computer ICMP checksum        */
/*    _nx_ip_packet_send                    Send ICMP packet out          */
/*    _nx_packet_allocate                   Packet allocate               */
/*    _nx_packet_release                    Release packet back to pool   */
/*    _nx_ip_route_find                     Find outgoing interface for   */
/*                                             sending packet             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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
VOID _nx_icmpv4_send_error_message(NX_IP *ip_ptr, NX_PACKET *offending_packet,
                                   ULONG word1, ULONG error_pointer)
{

NX_PACKET       *pkt_ptr;
USHORT           checksum;
#if defined(NX_DISABLE_ICMPV4_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT             compute_checksum = 1;
#endif /* defined(NX_DISABLE_ICMPV4_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
NX_ICMPV4_ERROR *icmpv4_error;
NX_IPV4_HEADER  *ip_header_ptr;
UINT             ip_header_size;
UINT             bytes_to_copy, i;
ULONG            src_ip;
ULONG            next_hop_address = NX_NULL;
ULONG           *src_packet, *dest_packet;
NX_INTERFACE    *if_ptr;

#ifdef NX_IPSEC_ENABLE
VOID            *sa = NX_NULL;
UINT             ret = 0;
ULONG            data_offset;
NXD_ADDRESS      src_addr;
NXD_ADDRESS      dest_addr;
#endif /* NX_IPSEC_ENABLE */


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, offending_packet);

    /* Do not send ICMPv4 error message if ICMPv4 is not enabled. */
    if (ip_ptr -> nx_ip_icmpv4_packet_process == NX_NULL)
    {
        return;
    }

    /* Find out the source and destination IP addresses of the offending packet. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ip_header_ptr = (NX_IPV4_HEADER *)(offending_packet -> nx_packet_ip_header);
    src_ip = ip_header_ptr -> nx_ip_header_source_ip;

    /* Get the incoming interface. */
    if_ptr = offending_packet -> nx_packet_address.nx_packet_interface_ptr;

    /* An ICMP error message MUST NOT be sent as the result of receiving:
       RFC1122, Section3.2.2, Page39.  */

    /* A datagram destined to an IP broadcast or IP multicast address.  */
    if ((ip_header_ptr -> nx_ip_header_destination_ip == NX_IP_LIMITED_BROADCAST) ||
        ((ip_header_ptr -> nx_ip_header_destination_ip & NX_IP_CLASS_D_MASK) == NX_IP_CLASS_D_TYPE))
    {
        return;
    }

    /* A datagram sent as a link-layer broadcast.  */
    if (((ip_header_ptr -> nx_ip_header_destination_ip & if_ptr -> nx_interface_ip_network_mask) ==
         if_ptr -> nx_interface_ip_network) &&
        ((ip_header_ptr -> nx_ip_header_destination_ip & ~(if_ptr -> nx_interface_ip_network_mask)) ==
         ~(if_ptr -> nx_interface_ip_network_mask)))
    {
        return;
    }

    /* A non-initial fragment.  */
    if (ip_header_ptr -> nx_ip_header_word_1 & NX_IP_OFFSET_MASK)
    {
        return;
    }

    /* A datagram whose source address does not define a single host,
       e.g., a zero address, a loopback address, a broadcast address,
       a multicast address, or a Class E address.  */
    if ((ip_header_ptr -> nx_ip_header_source_ip == 0) ||
        ((ip_header_ptr -> nx_ip_header_source_ip >= NX_IP_LOOPBACK_FIRST) &&
         (ip_header_ptr -> nx_ip_header_source_ip <= NX_IP_LOOPBACK_LAST)) ||
        (ip_header_ptr -> nx_ip_header_source_ip == NX_IP_LIMITED_BROADCAST) ||
        ((ip_header_ptr -> nx_ip_header_source_ip & NX_IP_CLASS_D_MASK) == NX_IP_CLASS_D_TYPE))
    {
        return;
    }

    /* Allocate a packet to build the ICMPv4 error message in.  */
    if (_nx_packet_allocate(ip_ptr -> nx_ip_default_packet_pool, &pkt_ptr, NX_IPv4_ICMP_PACKET, NX_NO_WAIT))
    {

        /* Error getting packet, so just get out!  */
        return;
    }

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, pkt_ptr);

    /* Mark the packet as IPv4. */
    /*lint -e{644} suppress variable might not be initialized, since "pkt_ptr" was initialized in _nx_packet_allocate. */
    pkt_ptr -> nx_packet_ip_version = NX_IP_VERSION_V4;

    /* Size of the message is ICMPv4 */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    icmpv4_error = (NX_ICMPV4_ERROR *)(pkt_ptr -> nx_packet_prepend_ptr);
    icmpv4_error -> nx_icmpv4_error_header.nx_icmpv4_header_type = (UCHAR)((word1 >> 24) & 0xFF);
    icmpv4_error -> nx_icmpv4_error_header.nx_icmpv4_header_code = (UCHAR)((word1 >> 16) & 0xFF);
    icmpv4_error -> nx_icmpv4_error_header.nx_icmpv4_header_checksum = 0;
    icmpv4_error -> nx_icmpv4_error_pointer = (error_pointer << 24);

    /* Change to network byte order. */
    NX_CHANGE_ULONG_ENDIAN(icmpv4_error -> nx_icmpv4_error_pointer);

    /* IP Header + 64 bits (64 bits = 2 ULONGs) of Data Datagram.  */
    ip_header_size = ((ip_header_ptr -> nx_ip_header_word_0 & 0x0F000000) >> 24);
    bytes_to_copy = (UINT)((ip_header_size + 2) * sizeof(ULONG));

    /* Set the packet length and pointers.  The length will be increased to include
       the IPv4 header in the IP send function.  The Prepend function will be similarly
       updated in the IP send function. */
    pkt_ptr -> nx_packet_length = bytes_to_copy + (ULONG)sizeof(NX_ICMPV4_ERROR);
    pkt_ptr -> nx_packet_append_ptr = pkt_ptr -> nx_packet_prepend_ptr + pkt_ptr -> nx_packet_length;

    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    src_packet  = (ULONG *)(offending_packet -> nx_packet_ip_header);

    /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
    dest_packet = (ULONG *)NX_UCHAR_POINTER_ADD(icmpv4_error, sizeof(NX_ICMPV4_ERROR));

    /* Endian swap the incoming IPv4 normal header to network byte order. */
    for (i = 0; i < NX_IP_NORMAL_LENGTH; i++)
    {
        NX_CHANGE_ULONG_ENDIAN(*src_packet);
        src_packet++;
    }

    /* Reset the packet pointer to the received packet IP header. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    src_packet  = (ULONG *)(offending_packet -> nx_packet_ip_header);

    /* Copy the data from the received packet to the ICMPv4 error packet. */
    for (; bytes_to_copy > 0; bytes_to_copy -= 4)
    {

        *dest_packet++ = *src_packet++;
    }

    /* Get the IP header pointer.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    src_packet  = (ULONG *)(offending_packet -> nx_packet_ip_header);

    /* Endian swap the IPv4 normal header back to host byte order. */
    for (i = 0; i < NX_IP_NORMAL_LENGTH; i++)
    {
        NX_CHANGE_ULONG_ENDIAN(*src_packet);
        src_packet++;
    }

    /* Use the corresponding interface address as sender's address. */
    pkt_ptr -> nx_packet_address.nx_packet_interface_ptr = offending_packet -> nx_packet_address.nx_packet_interface_ptr;

    /* Figure out the best interface to send the ICMP packet on. */
    _nx_ip_route_find(ip_ptr, src_ip,
                      &pkt_ptr -> nx_packet_address.nx_packet_interface_ptr,
                      &next_hop_address);

#ifdef NX_IPSEC_ENABLE

    /* Check for possible SA match. */
    if (ip_ptr -> nx_ip_packet_egress_sa_lookup != NX_NULL)               /* IPsec is enabled. */
    {

        /* Set up IP address. */
        src_addr.nxd_ip_version = NX_IP_VERSION_V4;
        src_addr.nxd_ip_address.v4 = pkt_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_address;
        dest_addr.nxd_ip_version = NX_IP_VERSION_V4;
        dest_addr.nxd_ip_address.v4 = src_ip;

        /* If the SA has not been set. */
        ret = ip_ptr -> nx_ip_packet_egress_sa_lookup(ip_ptr,                 /* IP ptr */
                                                      &src_addr,              /* src_addr */
                                                      &dest_addr,             /* dest_addr */
                                                      NX_PROTOCOL_ICMP,       /* protocol */
                                                      0,                      /* src_port */
                                                      0,                      /* dest_port */
                                                      &data_offset, &sa,
                                                      ((word1 >> 16) & 0xFFFF));
        if (ret == NX_IPSEC_TRAFFIC_BYPASS)
        {
            sa = NX_NULL;
            data_offset = 0;
        }
        else if (ret == NX_IPSEC_TRAFFIC_DROP || ret == NX_IPSEC_TRAFFIC_PENDING_IKEV2)
        {

            /* IPSec SA disallows this packet. Drop the packet and return. */
            _nx_packet_release(pkt_ptr);

            return;
        }
    }

    pkt_ptr -> nx_packet_ipsec_sa_ptr = sa;

#endif /* NX_IPSEC_ENABLE */

#ifdef NX_DISABLE_ICMPV4_TX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_ICMPV4_TX_CHECKSUM */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (pkt_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM)
    {
        compute_checksum = 0;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#ifdef NX_IPSEC_ENABLE
    if ((sa != NX_NULL) && (((NX_IPSEC_SA *)sa) -> nx_ipsec_sa_encryption_method != NX_CRYPTO_NONE))
    {
        compute_checksum = 1;
    }
#endif /* NX_IPSEC_ENABLE */
#if defined(NX_DISABLE_ICMPV4_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_ICMPV4_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
    {

        /* Compute the checksum of the ICMP packet.  */
        checksum = _nx_ip_checksum_compute(pkt_ptr, NX_IP_ICMP,
                                           (UINT)pkt_ptr -> nx_packet_length,
                                           /* ICMPV4 checksum does not include
                                              src/dest addresses */
                                           NX_NULL, NX_NULL);

        icmpv4_error -> nx_icmpv4_error_header.nx_icmpv4_header_checksum = (USHORT)(~checksum);

        /* Swap to network byte order. */
        NX_CHANGE_USHORT_ENDIAN(icmpv4_error -> nx_icmpv4_error_header.nx_icmpv4_header_checksum);
    }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
    {
        pkt_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    /* Send the ICMP packet to the IP component. The time to live is set to 255.  */
    /*lint -e{644} suppress variable might not be initialized, since "next_hop_address" was initialized in _nx_ip_route_find. */
    _nx_ip_packet_send(ip_ptr, pkt_ptr, src_ip,
                       NX_IP_NORMAL, 255, NX_IP_ICMP, NX_FRAGMENT_OKAY, next_hop_address);

    return;
}
#endif /* !NX_DISABLE_IPV4 && !NX_DISABLE_ICMPV4_ERROR_MESSAGE  */

