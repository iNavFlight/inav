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
#include "nx_icmpv4.h"

#include "nx_ipv6.h"
#include "nx_icmpv6.h"

#ifndef NX_DISABLE_FRAGMENTATION
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_fragment_timeout_cleanup                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function cleans up outstanding IP fragments.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    fragment                              The head of an IP fragment    */
/*                                             that needs to be cleaned.  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release packet                */
/*    IPv6_Address_Type                     Find IPv6 address type.       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_fragment_timeout_check                                       */
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
static VOID _nx_ip_fragment_cleanup(NX_IP *ip_ptr, NX_PACKET *fragment)
{

NX_PACKET                      *next_fragment;
#if !defined(NX_DISABLE_IPV4) && !defined(NX_DISABLE_ICMPV4_ERROR_MESSAGE)
NX_IPV4_HEADER                 *ipv4_header;
#endif /* !NX_DISABLE_IPV4 && !NX_DISABLE_ICMPV4_ERROR_MESSAGE  */
#if defined(FEATURE_NX_IPV6) && !defined(NX_DISABLE_ICMPV6_ERROR_MESSAGE)
NX_IPV6_HEADER_FRAGMENT_OPTION *fragment_option;
NX_IPV6_HEADER                 *ipv6_header;
#endif /* FEATURE_NX_IPV6 && !NX_DISABLE_ICMPV6_ERROR_MESSAGE  */

#ifndef NX_DISABLE_IP_INFO
    /* Increment the re-assembly failures count.  */
    ip_ptr -> nx_ip_reassembly_failures++;
#endif

#if !defined(NX_DISABLE_IPV4) && !defined(NX_DISABLE_ICMPV4_ERROR_MESSAGE)
    /* If ICMPv4 is enabled, send Destination unreachable. */
    if (fragment -> nx_packet_ip_version == NX_IP_VERSION_V4)
    {

        /* Setup header pointer for this packet.  */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ipv4_header =  (NX_IPV4_HEADER *)fragment -> nx_packet_ip_header;

        /* Time Exceeded Message, RFC 792, P2. */
        /* Check if the fragment zero is available.  */
        if ((ipv4_header -> nx_ip_header_word_1 & NX_IP_OFFSET_MASK) == 0)
        {

            /* Send a time exceeded message if fragmented datagram cannot complete the reassembly.  */
            NX_ICMPV4_SEND_TIME_EXCEED(ip_ptr, fragment, NX_ICMP_FRT_EXCEEDED_CODE);
        }
    }
#endif /* !NX_DISABLE_IPV4 && !NX_DISABLE_ICMPV4_ERROR_MESSAGE  */

#if defined(FEATURE_NX_IPV6) && !defined(NX_DISABLE_ICMPV6_ERROR_MESSAGE)
    /* If the packet is IPv6 type, we need to send out ICMP error message. */
    if (fragment -> nx_packet_ip_version == NX_IP_VERSION_V6)
    {

        /* Send out ICMP Time Exceeded message, if the first fragment has been received
           as per RFC2460 4.5 */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        fragment_option = (NX_IPV6_HEADER_FRAGMENT_OPTION *)fragment -> nx_packet_prepend_ptr;

        if ((fragment_option -> nx_ipv6_header_fragment_option_offset_flag & 0xFFF8) == 0)
        {

            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            ipv6_header = (NX_IPV6_HEADER *)fragment -> nx_packet_ip_header;

            /* First fragment has been received.  Send ICMP error message, if the destination
               is not multicast. */

            if (!(IPv6_Address_Type(ipv6_header -> nx_ip_header_destination_ip) & IPV6_ADDRESS_MULTICAST))
            {

                /* Cover offset flag field. */
                NX_CHANGE_USHORT_ENDIAN(fragment_option -> nx_ipv6_header_fragment_option_offset_flag);
                NX_ICMPV6_SEND_TIME_EXCEED(ip_ptr, fragment, 1);
            }
        }
    }
#endif /* FEATURE_NX_IPV6 && !NX_DISABLE_ICMPV6_ERROR_MESSAGE  */

    /* Walk the chain of fragments for this fragment re-assembly.  */
    do
    {

#ifndef NX_DISABLE_IP_INFO

        /* Increment the IP receive packets dropped count.  */
        ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

        /* Pickup the next fragment.  */
        next_fragment =  fragment -> nx_packet_union_next.nx_packet_fragment_next;

        /* Reset tcp_queue_next before releasing. */
        /*lint -e{923} suppress cast of ULONG to pointer.  */
        fragment -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ALLOCATED;

        /* Release this fragment.  */
        _nx_packet_release(fragment);

        /* Reassign the fragment pointer.  */
        fragment =  next_fragment;
    } while (fragment);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_fragment_timeout_check                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for timeout conditions on the first fragment   */
/*    in the IP re-assembly list.  If the head pointer is the same        */
/*    between execution of this routine, the head fragment is deleted and */
/*    its packets are released.                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
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
VOID  _nx_ip_fragment_timeout_check(NX_IP *ip_ptr)
{

NX_PACKET *fragment;
NX_PACKET *next_fragment;
NX_PACKET *previous_fragment = NX_NULL;


    /* Set a pointer to the head packet of the fragmented packet queue. */
    fragment = ip_ptr -> nx_ip_fragment_assembly_head;


    /* Loop through all the fragmented packets in the queue. */
    while (fragment)
    {

        /* Check if the timeout has expired. */
        if (fragment -> nx_packet_reassembly_time == 0)
        {

            /* Timeout occured. */
            next_fragment = fragment -> nx_packet_queue_next;

            /* Remove the packet from the queue and connect the linked list around it. */
            if (previous_fragment == NX_NULL)
            {
                ip_ptr -> nx_ip_fragment_assembly_head = next_fragment;
            }
            else
            {
                previous_fragment -> nx_packet_queue_next = next_fragment;
            }

            /* Send out an error message, release the packet fragments in this chain. */
            _nx_ip_fragment_cleanup(ip_ptr, fragment);

            /* If this was the last one in the queue, reset the tail to
               the previous fragment. */
            if (fragment == ip_ptr -> nx_ip_fragment_assembly_tail)
            {
                ip_ptr -> nx_ip_fragment_assembly_tail = previous_fragment;
            }

            /* Get the next fragmented packet awaiting assembly. */
            fragment = next_fragment;
        }
        else
        {

            /*  Decrement the time remaining to assemble the whole packet. */
            fragment -> nx_packet_reassembly_time--;

            /* Get the next packet fragment. */
            previous_fragment = fragment;
            fragment = fragment -> nx_packet_queue_next;
        }
    }
}
#endif /* NX_DISABLE_FRAGMENTATION */

