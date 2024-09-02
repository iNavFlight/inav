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
#include "nx_icmp.h"

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv4_process_echo_request                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function processes incoming echo request message.     */
/*    It validates the echo request and sends an echo reply back to the   */
/*    sender.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            ICMP packet pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_route_find                     Find a suitable outgoing      */
/*                                            interface.                  */
/*    _nx_ip_packet_send                    Send ICMP packet out          */
/*    _nx_packet_release                    Release packet to packet pool */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv4_packet_process             Main ICMP packet pocess       */
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
VOID  _nx_icmpv4_process_echo_request(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

NX_ICMPV4_HEADER *header_ptr;
ULONG             checksum;
ULONG             old_m;
#if defined(NX_DISABLE_ICMPV4_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
ULONG             compute_checksum = 1;
#endif /* defined(NX_DISABLE_ICMPV4_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
NX_IPV4_HEADER   *ipv4_header;
ULONG             next_hop_address = NX_NULL;
#ifdef NX_IPSEC_ENABLE
ULONG             data_offset;
VOID             *sa = NX_NULL;
NXD_ADDRESS       src_addr;
NXD_ADDRESS       dest_addr;
UINT              ret;
#endif /* NX_IPSEC_ENABLE */


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);


    /* Point to the ICMP message header.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    header_ptr =  (NX_ICMPV4_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

    /* Pickup the return IP address.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ipv4_header = (NX_IPV4_HEADER *)packet_ptr -> nx_packet_ip_header;

#ifndef NX_DISABLE_ICMP_INFO
    /* Increment the ICMP pings received count.  */
    ip_ptr -> nx_ip_pings_received++;
#endif

    /* Change the type to Echo Reply and send back the message to the caller.  */
    header_ptr -> nx_icmpv4_header_type = NX_ICMP_ECHO_REPLY_TYPE;

#ifdef NX_IPSEC_ENABLE

    src_addr.nxd_ip_version = NX_IP_VERSION_V4;
    src_addr.nxd_ip_address.v4 = packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_address;

    dest_addr.nxd_ip_version = NX_IP_VERSION_V4;
    dest_addr.nxd_ip_address.v4 = ipv4_header -> nx_ip_header_source_ip;

    /* Check if IPsec is enabled. */
    if (ip_ptr -> nx_ip_packet_egress_sa_lookup != NX_NULL)
    {

        /* Check for possible SA match. */
        ret = ip_ptr -> nx_ip_packet_egress_sa_lookup(ip_ptr,                                    /* IP ptr */
                                                      &src_addr,                                 /* src_addr */
                                                      &dest_addr,                                /* dest_addr */
                                                      NX_PROTOCOL_ICMP,                          /* protocol */
                                                      0,                                         /* port, not used. */
                                                      0,                                         /* port, not used. */
                                                      &data_offset, &sa, (NX_ICMP_ECHO_REPLY_TYPE << 8));

        /* Do the IPSec SA rules permit this packet to be sent? */
        if (ret == NX_IPSEC_TRAFFIC_PROTECT)
        {

            /* Yes; make sure the outgoing packet has enough space for IPsec header info. */
            if ((ULONG)(packet_ptr -> nx_packet_prepend_ptr - packet_ptr -> nx_packet_data_start) <
                (NX_IPv4_PACKET + data_offset))
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

            /* No; Drop the packet and return. */
            _nx_packet_release(packet_ptr);

            return;
        }
        else
        {
            /* SA rules indicate to bypass IPSec processing. Zero out the
               SA information. */
            packet_ptr -> nx_packet_ipsec_sa_ptr = NX_NULL;
        }
    }
#endif /* NX_IPSEC_ENABLE */

#ifdef NX_DISABLE_ICMPV4_TX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_ICMPV4_TX_CHECKSUM */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM)
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

        /* Update the checksum, according to the RFC1624 page2, Eqn.3.
           HC  - old checksum in header
           HC' - new checksum in header
           m   - old value of a 16-bit field
           m'  - new value of a 16-bit field
           HC' = ~(C + (-m) + m')
           = ~(~HC + ~m + m') */

        /* Endian swapping logic.  */
        NX_CHANGE_USHORT_ENDIAN(header_ptr -> nx_icmpv4_header_checksum);

        /* Get the old checksum (HC) in header. */
        checksum = header_ptr -> nx_icmpv4_header_checksum;

        /* Get the old type(m). */
        old_m = (ULONG)(NX_ICMP_ECHO_REQUEST_TYPE << 8);

        /* Update the checksum, get the new checksum(HC'). */
        /* The m' is value of echo reply type. It is zero so can be ignored. */
        checksum = ((~checksum) & 0xFFFF) + ((~old_m) & 0xFFFF);

        /* Fold a 4-byte value into a two byte value */
        checksum = (checksum >> 16) + (checksum & 0xFFFF);

        /* Do it again in case previous operation generates an overflow */
        checksum = (checksum >> 16) + (checksum & 0xFFFF);

        /* Store the checksum.  */
        header_ptr -> nx_icmpv4_header_checksum = (~checksum & NX_LOWER_16_MASK);

        /* If NX_LITTLE_ENDIAN is defined, the header need to be swapped back
           for output (network byte order).  */
        NX_CHANGE_USHORT_ENDIAN(header_ptr -> nx_icmpv4_header_checksum);
    }
#if defined(NX_DISABLE_ICMPV4_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY)
    else
    {

        /* Clear the checksum.  */
        header_ptr -> nx_icmpv4_header_checksum = 0;

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_ICMPV4_TX_CHECKSUM;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
    }
#endif

    /* Figure out the best interface to send the ICMP packet on. */
    _nx_ip_route_find(ip_ptr, ipv4_header -> nx_ip_header_source_ip,
                      &packet_ptr -> nx_packet_address.nx_packet_interface_ptr,
                      &next_hop_address);

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_ICMP_RECEIVE, ip_ptr, ipv4_header -> nx_ip_header_source_ip, packet_ptr, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

#ifndef NX_DISABLE_ICMP_INFO
    /* Increment the ICMP pings responded to count.  */
    ip_ptr -> nx_ip_pings_responded_to++;
#endif

    /* Send the ICMP packet to the IP component.  */
    /*lint -e{644} suppress variable might not be initialized, since "next_hop_address" was initialized in _nx_ip_route_find. */
    _nx_ip_packet_send(ip_ptr, packet_ptr, ipv4_header -> nx_ip_header_source_ip,
                       NX_IP_NORMAL, NX_IP_TIME_TO_LIVE, NX_IP_ICMP, NX_FRAGMENT_OKAY, next_hop_address);
}
#endif /* !NX_DISABLE_IPV4  */

