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
/*    _nx_icmpv6_process_echo_request                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function processes incoming echo request message.     */
/*    It validates the echo request and sends an echo reply back to the   */
/*    sender.  Note that when formulating an echo reply, the function     */
/*    updates the corresponding IPv6 and ICMPv6 header. The content       */
/*    of the ICMP echo request message is untouched.  The ICMPv6          */
/*    checksum computation also takes a shortcut by adjusting the         */
/*    original checksum values for ICMPv6 header field changes.           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                  IP stack instance                           */
/*    packet_ptr              Received echo request packet                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release      Release packet back to the packet pool      */
/*    _nx_ipv6_packet_send    Transmit IPv6 packet to remote host         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_packet_process            Main ICMPv6 packet handler     */
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

VOID _nx_icmpv6_process_echo_request(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

UINT              status;
ULONG             tmp;
USHORT            checksum;
#if defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT              compute_checksum = 1;
#endif /* defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
NX_ICMPV6_HEADER *header_ptr;
NX_IPV6_HEADER   *ipv6_header;
ULONG             hop_limit = 255;
NXD_ADDRESS       dest_addr;
NX_INTERFACE     *interface_ptr;

#ifdef NX_IPSEC_ENABLE
ULONG             data_offset;
VOID             *sa = NX_NULL;
NXD_ADDRESS       src_addr;
UINT              ret;
#endif /* NX_IPSEC_ENABLE */


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

#ifndef NX_DISABLE_RX_SIZE_CHECKING
    /* Check packet length. */
    if (packet_ptr -> nx_packet_length < sizeof(NX_ICMPV6_ECHO))
    {
#ifndef NX_DISABLE_ICMP_INFO

        /* Increment the ICMP invalid message count.  */
        ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif

        /* Invalid ICMP message, just release it.  */
        _nx_packet_release(packet_ptr);
        return;
    }
#endif /* NX_DISABLE_RX_SIZE_CHECKING */

    /* Points to the ICMP message header.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    header_ptr  = (NX_ICMPV6_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

    /* Points to the IPv6 header. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ipv6_header = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

    /* Check if the destination address is multicast address.  */
    if (IPv6_Address_Type(ipv6_header -> nx_ip_header_destination_ip) & IPV6_ADDRESS_MULTICAST)
    {

        /* Yes, Set the interface.  */
        interface_ptr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached;

        /* Find a suitable outgoing address. */
        status = _nxd_ipv6_interface_find(ip_ptr, ipv6_header -> nx_ip_header_source_ip,
                                          &packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr, interface_ptr);

        /* Cannot find usable outgoing interface. */
        if (status != NX_SUCCESS)
        {

            /* Release the packet. */
            _nx_packet_release(packet_ptr);

            return;
        }
    }
    else
    {

        /* Make sure the interface IP address has been validated. */
        if (packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_state != NX_IPV6_ADDR_STATE_VALID)
        {

            /* Not validated, so release the packet and abort.*/
            _nx_packet_release(packet_ptr);

            return;
        }
    }

    /* Discard the packet if source address is unspecified (::). */
    if (CHECK_UNSPECIFIED_ADDRESS(ipv6_header -> nx_ip_header_source_ip))
    {

        /* NULL address in the header. Release the packet and abort. */
        _nx_packet_release(packet_ptr);

        return;
    }

#ifndef NX_DISABLE_ICMP_INFO
    /* Increment the ICMP pings received count.  */
    ip_ptr -> nx_ip_pings_received++;
#endif

    /* Respond to echo request packet.  */

    /* Set up the destination address. */
    dest_addr.nxd_ip_version = NX_IP_VERSION_V6;
    dest_addr.nxd_ip_address.v6[0] = ipv6_header -> nx_ip_header_source_ip[0];
    dest_addr.nxd_ip_address.v6[1] = ipv6_header -> nx_ip_header_source_ip[1];
    dest_addr.nxd_ip_address.v6[2] = ipv6_header -> nx_ip_header_source_ip[2];
    dest_addr.nxd_ip_address.v6[3] = ipv6_header -> nx_ip_header_source_ip[3];

#ifdef NX_IPSEC_ENABLE

    /* Set up the source address for IPSec SA lookup. */
    src_addr.nxd_ip_version = NX_IP_VERSION_V6;

    COPY_IPV6_ADDRESS(packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address,
                      src_addr.nxd_ip_address.v6);

    /* Check if IPsec is enabled. */
    if (ip_ptr -> nx_ip_packet_egress_sa_lookup != NX_NULL)
    {

        /* Check for possible SA match. */
        ret = ip_ptr -> nx_ip_packet_egress_sa_lookup(ip_ptr,                                    /* IP ptr */
                                                      &src_addr,                                 /* src_addr */
                                                      &dest_addr,                                /* dest_addr */
                                                      NX_PROTOCOL_ICMPV6,                        /* protocol */
                                                      0,                                         /* port, not used. */
                                                      0,                                         /* port, not used. */
                                                      &data_offset, &sa, (NX_ICMPV6_ECHO_REPLY_TYPE << 8));

        /* We have a match; apply IPSec processing on this packet. */
        if (ret == NX_IPSEC_TRAFFIC_PROTECT)
        {

            /* Make sure the outgoing packet has enough space for IPsec header info. */
            if ((ULONG)(packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start) <
                (NX_IPv6_PACKET + data_offset))
            {

                /* Not enough space.   Release the packet and return. */
                _nx_packet_release(packet_ptr);
                return;
            }

            /* Save the SA to the packet. */
            packet_ptr -> nx_packet_ipsec_sa_ptr = sa;
        }
        else if (ret == NX_IPSEC_TRAFFIC_DROP || ret == NX_IPSEC_TRAFFIC_PENDING_IKEV2)
        {

            /* IPSec SA disallows this packet. Drop the packet and return. */
            _nx_packet_release(packet_ptr);

            return;
        }
        else
        {
            /* IPSec SA indicates the packet requires no IPSec processing.
               Zero out sa information. */
            packet_ptr -> nx_packet_ipsec_sa_ptr = NX_NULL;
        }
    }
#endif /* NX_IPSEC_ENABLE */


    /* Change the type to Echo Reply and send back the message to the caller.  */
    header_ptr -> nx_icmpv6_header_type = NX_ICMPV6_ECHO_REPLY_TYPE;

#ifdef NX_DISABLE_ICMPV6_TX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_ICMPV6_TX_CHECKSUM */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM)
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
#if defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
    {

        /* Take a short cut to fix the checksum. */
        checksum = header_ptr -> nx_icmpv6_header_checksum;

        /* Change to host byte order. */
        NX_CHANGE_USHORT_ENDIAN(checksum);

        tmp = ((USHORT)(~checksum) & 0xFFFF);

        /* The original ICMP type is ECHO_REQUEST. */
        tmp -= (NX_ICMPV6_ECHO_REQUEST_TYPE << 8);
        if (tmp > (ULONG)0x80000000)
        {
            tmp = (tmp & 0xFFFF) - 1;
        }


        tmp += (ULONG)(header_ptr -> nx_icmpv6_header_type << 8);

        /* Compute the checksum differently depending if the echo request sends to
           a multicast or unicast address.   */
        if ((IPv6_Address_Type(ipv6_header -> nx_ip_header_destination_ip) &
             IPV6_ADDRESS_MULTICAST) == IPV6_ADDRESS_MULTICAST)
        {

            /* Compute the checksum for a multicast address. */
            header_ptr -> nx_icmpv6_header_checksum = 0;

            tmp = _nx_ip_checksum_compute(packet_ptr,
                                          NX_PROTOCOL_ICMPV6,
                                          (UINT)packet_ptr -> nx_packet_length,
                                          packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address,
                                          ipv6_header -> nx_ip_header_source_ip);

            tmp = ~tmp;

            header_ptr -> nx_icmpv6_header_checksum = (USHORT)(tmp);

            NX_CHANGE_USHORT_ENDIAN(header_ptr -> nx_icmpv6_header_checksum);

            hop_limit = 255;
        }
        else
        {
            /* Compute the checksum for a unicast address. */
            hop_limit = ip_ptr -> nx_ipv6_hop_limit;

            tmp = (tmp >> 16) + (tmp & 0xFFFF);

            /* Do it again in case of carrying */
            tmp = (tmp >> 16) + (tmp & 0xFFFF);
            header_ptr -> nx_icmpv6_header_checksum = (USHORT)(~tmp);
            NX_CHANGE_USHORT_ENDIAN(header_ptr -> nx_icmpv6_header_checksum);
        }
    }
#if defined(NX_DISABLE_ICMPV6_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY)
    else
    {

        /* Fix the bug when NX_DISABLE_ICMP_TX_CHECKSUM has been set, the nx_icmpv6_header_type is not modified.  */
        /* Change the type to Echo Reply and send back the message to the caller.  */
        header_ptr -> nx_icmpv6_header_checksum = 0;

        if ((IPv6_Address_Type(ipv6_header -> nx_ip_header_destination_ip) &
             IPV6_ADDRESS_MULTICAST) == IPV6_ADDRESS_MULTICAST)
        {
            hop_limit = 255;
        }
        else
        {

            hop_limit = ip_ptr -> nx_ipv6_hop_limit;
        }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_ICMPV6_TX_CHECKSUM;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
    }
#endif

#ifndef NX_DISABLE_ICMP_INFO
    /* Increment the ICMP pings responded to count.  */
    ip_ptr -> nx_ip_pings_responded_to++;
#endif

    /* Send the ICMP packet to the IP component.  */
    _nx_ipv6_packet_send(ip_ptr, packet_ptr, NX_PROTOCOL_ICMPV6,
                         packet_ptr -> nx_packet_length, hop_limit,
                         packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address,
                         dest_addr.nxd_ip_address.v6);
}


#endif /* FEATURE_NX_IPV6 */

