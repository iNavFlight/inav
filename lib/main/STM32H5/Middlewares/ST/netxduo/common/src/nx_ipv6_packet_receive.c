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
#include "nx_packet.h"
#include "nx_ipv6.h"
#include "nx_icmpv6.h"

#ifdef FEATURE_NX_IPV6



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv6_packet_receive                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives IPv6 packets from the nx_ip_packet_receive.  */
/*    The packet is either processed here or placed it in a deferred      */
/*    processing queue, depending on the complexity of the packet.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet received    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Packet release function       */
/*    _nx_ip_dispatch_process               The routine that examines     */
/*                                            other optional headers and  */
/*                                            upper layer protocols.      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application I/O Driver                                              */
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
VOID  _nx_ipv6_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

UINT              error;
ULONG             delta;
UINT              pkt_length;
UCHAR             next_header_type;
NX_IPV6_HEADER   *ip_header_ptr;
NXD_IPV6_ADDRESS *interface_ipv6_address_next;
NXD_IPV6_ADDRESS *incoming_address = NX_NULL;
#ifndef NX_DISABLE_PACKET_CHAIN
NX_PACKET        *before_last_packet;
NX_PACKET        *last_packet;
#endif /* NX_DISABLE_PACKET_CHAIN */

#ifdef NX_ENABLE_IPV6_MULTICAST
INT               i = 0;
#endif /* NX_ENABLE_IPV6_MULTICAST  */


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Points to the base of IPv6 header. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ip_header_ptr = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

    /* Byte swap WORD 1 to obtain IPv6 payload length. */
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_1);

    pkt_length = (UINT)((ip_header_ptr -> nx_ip_header_word_1 >> 16) + sizeof(NX_IPV6_HEADER));

    /* Make sure the packet length field matches the payload length field in the IPv6 header. */
    if (packet_ptr -> nx_packet_length != (ULONG)pkt_length)
    {

        /* Determine if the packet length is less than the size reported in the IP header.  */
        if (packet_ptr -> nx_packet_length < (ULONG)pkt_length)
        {

            /* The incoming packet has a wrong payload size. */
#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP invalid packet error.  */
            ip_ptr -> nx_ip_invalid_packets++;

            /* Increment the IP receive packets dropped count.  */
            ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

            /* Release the packet!  */
            _nx_packet_release(packet_ptr);

            /* In all cases, receive processing is finished.  Return to caller.  */
            return;
        }

        /* Calculate the difference in the length.  */
        delta =  packet_ptr -> nx_packet_length - pkt_length;

        /* Adjust the packet length.  */
        packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length - delta;

        /* Adjust the append pointer.  */

#ifndef NX_DISABLE_PACKET_CHAIN
        /* Loop to process adjustment that spans multiple packets.  */
        while (delta)
        {

            /* Determine if the packet is chained (or still chained after the adjustment).  */
            if (packet_ptr -> nx_packet_last == NX_NULL)
            {

                /* No, packet is not chained, simply adjust the append pointer in the packet.  */
                packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_append_ptr - delta;

                /* Break out of the loop, since the adjustment is complete.  */
                break;
            }

            /* Pickup the pointer to the last packet.  */
            last_packet =  packet_ptr -> nx_packet_last;

            /* Determine if the amount to adjust is less than the payload in the last packet.  */
            /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
            if (((ULONG)(last_packet -> nx_packet_append_ptr - last_packet -> nx_packet_prepend_ptr)) > delta)
            {

                /* Yes, simply adjust the append pointer of the last packet in the chain.  */
                /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
                last_packet -> nx_packet_append_ptr =  last_packet -> nx_packet_append_ptr - delta;

                /* Get out of the loop, since the adjustment is complete.  */
                break;
            }
            else
            {

                /* Adjust the delta by the amount in the last packet.  */
                delta =  delta - ((ULONG)(last_packet -> nx_packet_append_ptr - last_packet -> nx_packet_prepend_ptr));

                /* Find the packet before the last packet.  */
                before_last_packet =  packet_ptr;
                while (before_last_packet -> nx_packet_next != last_packet)
                {

                    /* Move to the next packet in the chain.  */
                    before_last_packet =  before_last_packet -> nx_packet_next;
                }

                /* At this point, we need to release the last packet and adjust the other packet
                   pointers.  */

                /* Ensure the next packet pointer is NULL in what is now the last packet.  */
                before_last_packet -> nx_packet_next =  NX_NULL;

                /* Determine if the packet is still chained.  */
                if (packet_ptr != before_last_packet)
                {

                    /* Yes, the packet is still chained, setup the last packet pointer.  */
                    packet_ptr -> nx_packet_last =  before_last_packet;
                }
                else
                {

                    /* The packet is no longer chained, set the last packet pointer to NULL.  */
                    packet_ptr -> nx_packet_last =  NX_NULL;
                }

                /* Release the last packet.   */
                _nx_packet_release(last_packet);
            }
        }
#else

        /* Simply adjust the append pointer in the packet.  */
        packet_ptr -> nx_packet_append_ptr =  packet_ptr -> nx_packet_append_ptr - delta;
#endif /* NX_DISABLE_PACKET_CHAIN */
    }

    /* Byte swap the rest of the IPv6 header fields. */
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_0);
    NX_IPV6_ADDRESS_CHANGE_ENDIAN(ip_header_ptr -> nx_ip_header_destination_ip);
    NX_IPV6_ADDRESS_CHANGE_ENDIAN(ip_header_ptr -> nx_ip_header_source_ip);

    /* Get a pointer to the first address in the address list for this interface (e.g.
       the interface the packet was received on). */
    interface_ipv6_address_next = packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nxd_interface_ipv6_address_list_head;

    /* Check if this packet is intended for this host by looping through all the addresses in the IP interface table for a match. */
    while (interface_ipv6_address_next)
    {

        /* Ignore invalid addresses. */
        if (interface_ipv6_address_next -> nxd_ipv6_address_state != NX_IPV6_ADDR_STATE_UNKNOWN)
        {

            /* Does the incoming packet match one of the IP interfaces? */
            if (CHECK_IPV6_ADDRESSES_SAME(ip_header_ptr -> nx_ip_header_destination_ip,
                                          interface_ipv6_address_next -> nxd_ipv6_address))
            {

                /* Yes, we found a match! */
                incoming_address = interface_ipv6_address_next;

                break;
            }
            /* Check for multicast address. */
            else if (CHECK_IPV6_SOLICITED_NODE_MCAST_ADDRESS(ip_header_ptr -> nx_ip_header_destination_ip,
                                                             interface_ipv6_address_next -> nxd_ipv6_address))
            {

                /* Yes, this is a multicast address. */
                incoming_address = interface_ipv6_address_next;

                break;
            }
        }

        /* No match yet, get the next address. */
        interface_ipv6_address_next = interface_ipv6_address_next -> nxd_ipv6_address_next;
    }

#ifdef NX_ENABLE_IPV6_MULTICAST

    if ((incoming_address == NX_NULL) && ((ip_header_ptr -> nx_ip_header_destination_ip[0] & 0xFF000000) == 0xFF000000))
    {

        /* Search the address whether match our multicast join list.  */
        for (i = 0; i < NX_MAX_MULTICAST_GROUPS; i++)
        {

            /* Match the destination address with the multicast list */
            if ((ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_interface_list == packet_ptr -> nx_packet_address.nx_packet_interface_ptr) &&
                (CHECK_IPV6_ADDRESSES_SAME(ip_header_ptr -> nx_ip_header_destination_ip, (ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_list))))
            {
                incoming_address = ip_ptr -> nx_ipv6_multicast_entry[i].nx_ip_mld_join_interface_list -> nxd_interface_ipv6_address_list_head;
                break;
            }
        }
    }
#endif /* NX_ENABLE_IPV6_MULTICAST  */

    /* Check for valid interface. */
    if (incoming_address == NX_NULL)
    {

        /* The incoming packet has a destination address that does not match any of
           the local interface addresses so its not for me. */

#ifndef NX_DISABLE_IP_INFO
        /* Increment the IP receive packets dropped count.  */
        ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

        /* Release the packet. */
        _nx_packet_release(packet_ptr);

        /* In all cases, receive processing is finished.  Return to caller.  */
        return;
    }

    /* Set the matching address to the packet address. */
    packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr = incoming_address;

    /*
       Update the IP header pointer, packet length and packet prepend pointer
       to point to the next header (either IP option header or
       upper layer protocol header.
     */

    packet_ptr -> nx_packet_prepend_ptr += sizeof(NX_IPV6_HEADER);
    packet_ptr -> nx_packet_length -= (ULONG)sizeof(NX_IPV6_HEADER);

    packet_ptr -> nx_packet_option_offset = 6;
    next_header_type = (UCHAR)((ip_header_ptr -> nx_ip_header_word_1 >> 8) & 0xFF);

    /*
        Search all the extension headers, terminate after upper layer protocols
       (such as UDP, TCP, ICMP).

       Also check the order of the optional headers.  Once an out-of-order option
       field is detected, the search is terminated and an ICMPv6 error message is
       generated.
     */

    /* Initialize start of search for just passed the IP header. */
    packet_ptr -> nx_packet_option_state = (UCHAR)IPV6_BASE_HEADER;
    packet_ptr -> nx_packet_destination_header = 0;

#ifndef NX_DISABLE_IP_INFO

    /* Increment the number of packets delivered.  */
    ip_ptr -> nx_ip_total_packets_delivered++;

    /* Increment the IP packet bytes received (not including the header).  */
    ip_ptr -> nx_ip_total_bytes_received +=  packet_ptr -> nx_packet_length;
#endif

    error = _nx_ip_dispatch_process(ip_ptr, packet_ptr, next_header_type);

    if (error)
    {
        _nx_packet_release(packet_ptr);
    }

    return;
}


#endif /* FEATURE_NX_IPV6 */

