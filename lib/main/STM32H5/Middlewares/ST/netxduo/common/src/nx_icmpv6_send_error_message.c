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
/**   Internet Control Message Protocol (ICMPv6)                          */
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
#ifndef NX_DISABLE_ICMPV6_ERROR_MESSAGE
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_send_error_message                      PORTABLE C       */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called by various IPv6 components to send an       */
/*    error message when necessary.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP stack instance             */
/*    offending_packet                      The packet that caused the    */
/*                                              error.                    */
/*    word1                                 ICMPv6 error message header   */
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
/*    _nx_ipv6_packet_send                  Send ICMP packet out          */
/*    _nx_packet_allocate                   Packet allocate               */
/*    _nx_packet_release                    Release packet back to pool   */
/*    _nxd_ipv6_interface_find              Find outgoing interface for   */
/*                                             sending packet             */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_fragment_assembly                                            */
/*    _nx_ipv6_packet_receive                                             */
/*    _nx_udp_packet_receive                                              */
/*    _nx_ipv6_header_option_process                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  01-31-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed unsigned integers     */
/*                                            comparison,                 */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
VOID _nx_icmpv6_send_error_message(NX_IP *ip_ptr, NX_PACKET *offending_packet,
                                   ULONG word1, ULONG error_pointer)
{

NX_PACKET       *pkt_ptr;
USHORT           checksum;
#if defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT             compute_checksum = 1;
#endif /* defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
NX_ICMPV6_ERROR *icmpv6_error;
UINT             bytes_to_copy, i;
ULONG           *src_ip, *dest_ip;
ULONG           *src_packet, *dest_packet;
UINT             payload;
#ifdef NX_IPSEC_ENABLE
VOID            *sa = NX_NULL;
UINT             ret = 0;
ULONG            data_offset;
NXD_ADDRESS      src_addr;
NXD_ADDRESS      dest_addr;
#endif /* NX_IPSEC_ENABLE */


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, offending_packet);

    /* Do not send ICMPv6 error message if ICMPv6 is not enabled. */
    if (ip_ptr -> nx_ip_icmpv6_packet_process == NX_NULL)
    {
        return;
    }

    /* Find out the source and destination IP addresses of the offending packet. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    src_ip = (((NX_IPV6_HEADER *)(offending_packet -> nx_packet_ip_header)) -> nx_ip_header_source_ip);

    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    dest_ip = (((NX_IPV6_HEADER *)(offending_packet -> nx_packet_ip_header)) -> nx_ip_header_destination_ip);

    if (CHECK_UNSPECIFIED_ADDRESS(src_ip))
    {
        /*
         * Sender of the offending packet is unspecified.
         * So we shouldn't send out ICMP error message.
         * Drop the packet and return.
         */
        return;
    }

    /* Allocate a packet to build the ICMPv6 error message in.  */
    if (_nx_packet_allocate(ip_ptr -> nx_ip_default_packet_pool, &pkt_ptr, NX_IPv6_ICMP_PACKET, NX_NO_WAIT))
    {

        /* Error getting packet, so just get out!  */
        return;
    }

    /* Check to see if the packet has enough room to fill with the ICMPv6 error header.  */
    if ((UINT)(pkt_ptr -> nx_packet_data_end - pkt_ptr -> nx_packet_prepend_ptr) < sizeof(NX_ICMPV6_ERROR))
    {

        /* Error getting packet, so just get out!  */
        _nx_packet_release(pkt_ptr);
        return;
    }

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, pkt_ptr);

    /* Mark the packet as IPv6. */
    /*lint -e{644} suppress variable might not be initialized, since "pkt_ptr" was initialized in _nx_packet_allocate. */
    pkt_ptr -> nx_packet_ip_version = NX_IP_VERSION_V6;

    /* Setup the size of the ICMPv6 NA message */

    /* Size of the message is ICMPv6 */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    icmpv6_error = (NX_ICMPV6_ERROR *)(pkt_ptr -> nx_packet_prepend_ptr);
    icmpv6_error -> nx_icmpv6_error_header.nx_icmpv6_header_type = (UCHAR)((word1 >> 24) & 0xFF);
    icmpv6_error -> nx_icmpv6_error_header.nx_icmpv6_header_code = (UCHAR)((word1 >> 16) & 0xFF);
    icmpv6_error -> nx_icmpv6_error_header.nx_icmpv6_header_checksum = 0;

    icmpv6_error -> nx_icmpv6_error_pointer = error_pointer;

    /* Change to network byte order. */
    NX_CHANGE_ULONG_ENDIAN(icmpv6_error -> nx_icmpv6_error_pointer);

    /* Figure out how many bytes we should copy from the offending packet not including ethernet
       frame header. */
    /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
    bytes_to_copy = (UINT)(offending_packet -> nx_packet_append_ptr - offending_packet -> nx_packet_ip_header);

    /* Check that the number of bytes to copy does not exceed the minimum size ICMPv6 message
       as per RFC 2460. */
    if ((bytes_to_copy + sizeof(NX_ICMPV6_ERROR) + sizeof(NX_IPV6_HEADER)) >= NX_MINIMUM_IPV6_PATH_MTU)
    {

        /* Subtract size of IPv6 and ICMPv6 headers from the ICMPv6 error message packet. */
        bytes_to_copy = (UINT)(NX_MINIMUM_IPV6_PATH_MTU - (sizeof(NX_IPV6_HEADER) + sizeof(NX_ICMPV6_ERROR)));
    }

    /* Check how much of the offending packet data will fit in the allocated packet, leaving
       room for the Physical frame header, IPv6 header and ICMPv6 header of the error message. */
    payload = pkt_ptr -> nx_packet_pool_owner -> nx_packet_pool_payload_size;

    if (((INT)((bytes_to_copy + sizeof(NX_IPV6_HEADER) + sizeof(NX_ICMPV6_ERROR) + NX_PHYSICAL_HEADER) - payload)) > 0)
    {

        bytes_to_copy = (UINT)(payload - (sizeof(NX_IPV6_HEADER) + sizeof(NX_ICMPV6_ERROR) + NX_PHYSICAL_HEADER));
    }

    /* Set the packet length and pointers.  The length will be increased to include
       the IPv6 header in the IP send function.  The Prepend function will be similarly
       updated in the IP send function. */
    pkt_ptr -> nx_packet_length = bytes_to_copy + (ULONG)sizeof(NX_ICMPV6_ERROR);
    pkt_ptr -> nx_packet_append_ptr = pkt_ptr -> nx_packet_prepend_ptr + pkt_ptr -> nx_packet_length;

    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    src_packet  = (ULONG *)(offending_packet -> nx_packet_ip_header);

    /*lint -e{923} suppress cast between pointer and ULONG, since it is necessary  */
    dest_packet = (ULONG *)NX_UCHAR_POINTER_ADD(icmpv6_error, sizeof(NX_ICMPV6_ERROR));

    /* Endian swap the incoming IPv6 header (10 ULONGs = 40 bytes)
       to network byte order. */
    for (i = 0; i < 10; i++)
    {
        NX_CHANGE_ULONG_ENDIAN(*src_packet);
        src_packet++;
    }

    /* Reset the packet pointer to the received packet IP header. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    src_packet  = (ULONG *)(offending_packet -> nx_packet_ip_header);

    /* Copy the data from the received packet to the ICMPv6 error packet. */
    for (; (INT)bytes_to_copy > 0; bytes_to_copy -= 4)
    {

        *dest_packet++ = *src_packet++;
    }

    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    src_packet  = (ULONG *)(offending_packet -> nx_packet_ip_header);

    /* Endian swap the IPv6 header back to host byte order. */
    for (i = 0; i < 10; i++)
    {
        NX_CHANGE_ULONG_ENDIAN(*src_packet);
        src_packet++;
    }

    /* If we received the packet through a Multicast address, we pick an outgoing address
       based on multicast scope (RFC 3484, 3.1) */
    if (IPv6_Address_Type(dest_ip) & IPV6_ADDRESS_MULTICAST)
    {

        if (_nxd_ipv6_interface_find(ip_ptr, dest_ip,
                                     &pkt_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr,
                                     NX_NULL))
        {

            /* Cannot find usable outgoing interface. */
            _nx_packet_release(pkt_ptr);
            return;
        }
    }
    else
    {

        /* If this ICMPv6 error message is a response to a packet sent to link local or global address,
           use the corresponding interface address as sender's address. */
        pkt_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr = offending_packet -> nx_packet_address.nx_packet_ipv6_address_ptr;
    }

    /*
       Check if a suitable outoing address was found, and the
       outgoing address is not valid:
     */
    if ((pkt_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr == NX_NULL) ||
        (pkt_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_state != NX_IPV6_ADDR_STATE_VALID))
    {

        /* No good. Drop the packet and return. */
        _nx_packet_release(pkt_ptr);
        return;
    }

#ifdef NX_IPSEC_ENABLE

    /* Check for possible SA match. */
    if (ip_ptr -> nx_ip_packet_egress_sa_lookup != NX_NULL)               /* IPsec is enabled. */
    {

        /* Set up IP address. */
        src_addr.nxd_ip_version = NX_IP_VERSION_V6;

        COPY_IPV6_ADDRESS(pkt_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address,
                          src_addr.nxd_ip_address.v6);

        dest_addr.nxd_ip_version = NX_IP_VERSION_V6;

        COPY_IPV6_ADDRESS(src_ip, dest_addr.nxd_ip_address.v6);

        /* If the SA has not been set. */
        ret = ip_ptr -> nx_ip_packet_egress_sa_lookup(ip_ptr,                 /* IP ptr */
                                                      &src_addr,              /* src_addr */
                                                      &dest_addr,             /* dest_addr */
                                                      NX_PROTOCOL_ICMPV6,     /* protocol */
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

#ifdef NX_DISABLE_ICMPV6_TX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_ICMPV6_TX_CHECKSUM */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (pkt_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM)
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
#if defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
    {
        /* Compute the check sum */
        checksum = _nx_ip_checksum_compute(pkt_ptr, NX_PROTOCOL_ICMPV6,
                                           (UINT)pkt_ptr -> nx_packet_length,
                                           pkt_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address,
                                           src_ip);

        icmpv6_error -> nx_icmpv6_error_header.nx_icmpv6_header_checksum = (USHORT)(~checksum);

        /* Swap to network byte order. */
        NX_CHANGE_USHORT_ENDIAN(icmpv6_error -> nx_icmpv6_error_header.nx_icmpv6_header_checksum);
    }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    else
    {
        pkt_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

    /* Transmit the packet.  The hop limit is set to 255. */
    _nx_ipv6_packet_send(ip_ptr, pkt_ptr, NX_PROTOCOL_ICMPV6, pkt_ptr -> nx_packet_length, 255,
                         pkt_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address,
                         src_ip);

    return;
}
#endif /* NX_DISABLE_ICMPV6_ERROR_MESSAGE */
#endif /* FEATURE_NX_IPV6 */

