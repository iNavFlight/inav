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

#ifndef NX_DISABLE_FRAGMENTATION
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_fragment_assembly                            PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the received fragment queue and attempts to */
/*    reassemble fragmented IP datagrams.  Once a datagram is reassembled */
/*    it is dispatched to the appropriate component.                      */
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
/*    _nx_ip_dispatch_process               The routine that examines     */
/*                                            other optional headers and  */
/*                                            upper layer protocols.      */
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
/*                                            removed duplicated code,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/

VOID  _nx_ip_fragment_assembly(NX_IP *ip_ptr)
{
TX_INTERRUPT_SAVE_AREA

NX_PACKET                      *new_fragment_head;
NX_PACKET                      *current_fragment;
NX_PACKET                      *previous_fragment =  NX_NULL;
NX_PACKET                      *fragment_head;
NX_PACKET                      *search_ptr;
NX_PACKET                      *previous_ptr;
NX_PACKET                      *found_ptr;
NX_PACKET                      *old_ptr;
#ifndef NX_DISABLE_IPV4
NX_IPV4_HEADER                 *search_header = NX_NULL;
NX_IPV4_HEADER                 *current_header = NX_NULL;
ULONG                           current_ttl = 0;
#endif /* NX_DISABLE_IPV4 */
ULONG                           current_id = 0;
ULONG                           current_offset = 0;
ULONG                           protocol = NX_PROTOCOL_NO_NEXT_HEADER;
ULONG                           incomplete;
ULONG                           ip_version = NX_IP_VERSION_V4;
UCHAR                           copy_packet;
#ifdef FEATURE_NX_IPV6
NX_IPV6_HEADER_FRAGMENT_OPTION *search_v6_fragment_option = NX_NULL;
NX_IPV6_HEADER_FRAGMENT_OPTION *current_v6_fragment_option = NX_NULL;
NX_IPV6_HEADER                 *current_pkt_ip_header = NX_NULL;
#endif /* FEATURE_NX_IPV6 */
#ifdef NX_NAT_ENABLE
UINT                            packet_consumed;
#endif


    /* Disable interrupts.  */
    TX_DISABLE

    /* Remove the packets from the incoming IP fragment queue.  */
    new_fragment_head =  ip_ptr -> nx_ip_received_fragment_head;
    ip_ptr -> nx_ip_received_fragment_head =  NX_NULL;
    ip_ptr -> nx_ip_received_fragment_tail =  NX_NULL;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Process each IP packet in the received IP fragment queue.  */
    while (new_fragment_head)
    {

        /* Setup the copy packet flag.  */
        copy_packet = NX_FALSE;

        /* pick up the version number of this packet. */
        ip_version = new_fragment_head -> nx_packet_ip_version;

        /* Setup the current fragment pointer.  */
        current_fragment =  new_fragment_head;

        /* Move the head pointer.  */
        new_fragment_head =  new_fragment_head -> nx_packet_queue_next;

#ifndef NX_DISABLE_IPV4
        if (ip_version == NX_IP_VERSION_V4)
        {

            /* Setup header pointer for this packet.  */
            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            current_header =  (NX_IPV4_HEADER *)current_fragment -> nx_packet_prepend_ptr;

            /* Pickup the ID of this fragment.  */
            current_id =  (current_header ->  nx_ip_header_word_1 >> NX_SHIFT_BY_16);

            /* Pickup the offset of the new IP fragment.  */
            current_offset =  current_header -> nx_ip_header_word_1 & NX_IP_OFFSET_MASK;

            /* Pickup the time to live of this fragment.  */
            current_ttl = (current_header -> nx_ip_header_word_2 & NX_IP_TIME_TO_LIVE_MASK) >> NX_IP_TIME_TO_LIVE_SHIFT;

            /* Set the IPv4 reassembly time. RFC791, Section3.2, Page27.  */
            current_fragment -> nx_packet_reassembly_time = NX_IPV4_MAX_REASSEMBLY_TIME;
            if (current_fragment -> nx_packet_reassembly_time < current_ttl)
            {
                current_fragment -> nx_packet_reassembly_time = current_ttl;
            }
        }
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
        if (ip_version == NX_IP_VERSION_V6)
        {

            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            current_pkt_ip_header = (NX_IPV6_HEADER *)current_fragment -> nx_packet_ip_header;

            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            current_v6_fragment_option = (NX_IPV6_HEADER_FRAGMENT_OPTION *)current_fragment -> nx_packet_prepend_ptr;

            /* Pickup the ID of this fragment.  Note that the fragment ID is still in network byte order.
               The ID is only used as a way to make comparisons.  We don't need to byte swap this field. */
            current_id = current_v6_fragment_option -> nx_ipv6_header_fragment_option_packet_id;

            /* The offset field is left shifted by 3 bits.  However when we search through all the fragments
               for the right location to insert this frame, we don't need get the accurate value of the offset
               because the relative order of these offsets are preserved.  In other words, the lower 3 bits do
               not affect the outcome of a simple comparison. */
            current_offset = (current_v6_fragment_option -> nx_ipv6_header_fragment_option_offset_flag & 0xFFF8);

            /* Set the IPv6 reassembly time. RFC2460, Section4.5, Page22. */
            current_fragment -> nx_packet_reassembly_time = NX_IPV6_MAX_REASSEMBLY_TIME;
        }
#endif

        /* Set the found pointer to NULL.  */
        found_ptr =  NX_NULL;

        /* Does the assembly list have anything in it?  */
        if (ip_ptr -> nx_ip_fragment_assembly_head)
        {

            /* Yes, we need to search the assembly queue to see if this fragment belongs
               to another fragment.  */
            search_ptr =    ip_ptr -> nx_ip_fragment_assembly_head;
            previous_fragment =  NX_NULL;
            while (search_ptr)
            {

                /* Check this packet only if the version number matches. */
                if (ip_version == search_ptr -> nx_packet_ip_version)
                {
#ifndef NX_DISABLE_IPV4
                    if (ip_version == NX_IP_VERSION_V4)
                    {
                        /* Setup a pointer to the IP header of the packet in the assembly list.  */
                        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                        search_header =  (NX_IPV4_HEADER *)search_ptr -> nx_packet_prepend_ptr;

                        /* Determine if the IP header fields match. RFC 791 Section 3.2 recommends that packet
                           fragments be compared for source IP, destination IP, protocol and IP header ID.  */
                        /*lint -e{644} suppress variable might not be initialized, since "current_head" was initialized. */
                        if ((current_id == (search_header -> nx_ip_header_word_1 >> NX_SHIFT_BY_16)) &&
                            ((search_header -> nx_ip_header_word_2 & NX_IP_PROTOCOL_MASK) ==
                             (current_header -> nx_ip_header_word_2 & NX_IP_PROTOCOL_MASK)) &&
                            (search_header -> nx_ip_header_source_ip == current_header -> nx_ip_header_source_ip) &&
                            (search_header -> nx_ip_header_destination_ip == current_header -> nx_ip_header_destination_ip))
                        {

                            /* Yes, we found a match, just set the found_ptr and get out of
                               this loop!  */
                            found_ptr =  search_ptr;

                            /* The reassmebly timer should be MAX(reassembly time, Time To Live). RFC791, Section3.2, Page27.  */
                            if (search_ptr -> nx_packet_reassembly_time < current_ttl)
                            {
                                search_ptr -> nx_packet_reassembly_time = current_ttl;
                            }

                            /* Updated the reassembly time.  */
                            current_fragment -> nx_packet_reassembly_time = search_ptr -> nx_packet_reassembly_time;

                            break;
                        }
                    }
#endif /* NX_DISABLE_IPV4 */

#ifdef FEATURE_NX_IPV6
                    if (ip_version == NX_IP_VERSION_V6)
                    {

                    NX_IPV6_HEADER *search_pkt_ip_header;

                        /* Set up a pointer to the IPv6 fragment option field.*/
                        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                        search_v6_fragment_option = (NX_IPV6_HEADER_FRAGMENT_OPTION *)search_ptr -> nx_packet_prepend_ptr;

                        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                        search_pkt_ip_header = (NX_IPV6_HEADER *)search_ptr -> nx_packet_ip_header;

                        /* Determine if the IP packet IDs match. */
                        /*lint -e{613} suppress possible use of null pointer, since "current_pkt_ip_header" was set to none NULL above. */
                        if ((current_id == search_v6_fragment_option -> nx_ipv6_header_fragment_option_packet_id) &&
                            (CHECK_IPV6_ADDRESSES_SAME(search_pkt_ip_header -> nx_ip_header_source_ip, current_pkt_ip_header -> nx_ip_header_source_ip)) &&
                            (CHECK_IPV6_ADDRESSES_SAME(search_pkt_ip_header -> nx_ip_header_destination_ip, current_pkt_ip_header -> nx_ip_header_destination_ip)))
                        {

                            /* Yes, we found a match, just set the found_ptr and get out of
                               this loop!  */
                            found_ptr =  search_ptr;
                            current_fragment -> nx_packet_reassembly_time = search_ptr -> nx_packet_reassembly_time;
                            break;
                        }
                    }
#endif /* FEATURE_NX_IPV6 */
                }

                /* Remember the previous pointer.  */
                previous_fragment =  search_ptr;

                /* Move to the next IP fragment in the re-assembly queue.  */
                search_ptr =  search_ptr -> nx_packet_queue_next;
            }
        }

        /* Was another IP packet fragment found?  */
        if (found_ptr)
        {

            /* Save the fragment head pointer.  */
            fragment_head =  found_ptr;

            /* Another packet fragment was found...  find the proper place in the list
               for this packet and check for complete re-assembly.  */

            /* Setup the previous pointer.  Note that the search pointer still points
               to the first fragment in the list.  */
            previous_ptr =  NX_NULL;
            search_ptr =    found_ptr;

            /* Loop to walk through the fragment list.  */
            do
            {

#ifndef NX_DISABLE_IPV4
                if (ip_version == NX_IP_VERSION_V4)
                {

                    /* Pickup a pointer to the IP header of the fragment.  */
                    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                    search_header =  (NX_IPV4_HEADER *)search_ptr -> nx_packet_prepend_ptr;

                    /* Determine if the incoming IP fragment goes before this packet.  */
                    if (current_offset <= (search_header -> nx_ip_header_word_1 & NX_IP_OFFSET_MASK))
                    {

                        /* Determine if the incoming IP fragment is the copy packet. Link the new packet before old packet.  */
                        if (current_offset == (search_header -> nx_ip_header_word_1 & NX_IP_OFFSET_MASK))
                        {
                            copy_packet = NX_TRUE;
                        }

                        /* Yes, break out of the loop and insert the current packet.  */
                        break;
                    }
                }
#endif /* NX_DISABLE_IPV4 */

#ifdef FEATURE_NX_IPV6
                if (ip_version == NX_IP_VERSION_V6)
                {

                    /* Build the IP header pointer.  */
                    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                    search_v6_fragment_option = (NX_IPV6_HEADER_FRAGMENT_OPTION *)search_ptr -> nx_packet_prepend_ptr;

                    /* Determine if the incoming IP fragment goes before this packet.  */
                    if (current_offset <= (ULONG)(search_v6_fragment_option -> nx_ipv6_header_fragment_option_offset_flag & 0xFFF8))
                    {

                        /* Determine if the incoming IP fragment is copy packet. Link the new packet before old packet.  */
                        if (current_offset == (ULONG)(search_v6_fragment_option -> nx_ipv6_header_fragment_option_offset_flag & 0xFFF8))
                        {
                            copy_packet = NX_TRUE;
                        }

                        /* Yes, break out of the loop and insert the current packet.  */
                        break;
                    }
                }
#endif /* FEATURE_NX_IPV6 */

                /* Otherwise, move the search and previous pointers to the next fragment in the
                   chain.  */
                previous_ptr =  search_ptr;
                search_ptr   =  search_ptr -> nx_packet_union_next.nx_packet_fragment_next;
            } while (search_ptr);


            /* At this point, the previous pointer determines where to place the new fragment.  */
            if (previous_ptr)
            {

                /* Add new fragment after the previous ptr.  */
                current_fragment -> nx_packet_union_next.nx_packet_fragment_next =  previous_ptr -> nx_packet_union_next.nx_packet_fragment_next;
                previous_ptr -> nx_packet_union_next.nx_packet_fragment_next =      current_fragment;
            }
            else
            {

                /* This packet needs to be inserted at the front of the fragment chain.  */
                current_fragment -> nx_packet_queue_next =     fragment_head -> nx_packet_queue_next;
                current_fragment -> nx_packet_union_next.nx_packet_fragment_next =  fragment_head;
                if (previous_fragment)
                {

                    /* We need to link up a different IP packet fragment chain that is in
                       front of this one on the re-assembly queue.  */
                    previous_fragment -> nx_packet_queue_next =  current_fragment;
                }
                else
                {

                    /* Nothing prior to this IP fragment chain, we need to just change the
                       list header.  */
                    ip_ptr -> nx_ip_fragment_assembly_head =  current_fragment;

                    /* Clear the timeout fragment pointer.  */
                    ip_ptr -> nx_ip_timeout_fragment =  NX_NULL;
                }

                /* Determine if we need to adjust the tail pointer.  */
                if (fragment_head == ip_ptr -> nx_ip_fragment_assembly_tail)
                {

                    /* Setup the new tail pointer.  */
                    ip_ptr -> nx_ip_fragment_assembly_tail =  current_fragment;
                }

                /* Setup the new fragment head.  */
                fragment_head =  current_fragment;
            }

            /* Determine if the incoming IP fragment is the same packet.  */
            if (copy_packet == NX_TRUE)
            {

                /* Fragments contain the same data, use the more recently arrived copy. RFC791, Section3.2, Page29.  */

                /* Removed the old packet from the packet list, the old packet must be linked after new packet.  */
                old_ptr = current_fragment -> nx_packet_union_next.nx_packet_fragment_next;
                current_fragment -> nx_packet_union_next.nx_packet_fragment_next =  old_ptr -> nx_packet_union_next.nx_packet_fragment_next;

                /* Reset tcp_queue_next before releasing. */
                /* Cast the ULONG into a packet pointer. Since this is exactly what we wish to do, disable the lint warning with the following comment:  */
                /*lint -e{923} suppress cast of ULONG to pointer.  */
                old_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ALLOCATED;

                /* Release the old packet.  */
                _nx_packet_release(old_ptr);
            }

            /* At this point, the new IP fragment is in its proper place on the re-assembly
               list.  We now need to walk the list and determine if all the fragments are
               present.  */

            /* Setup the search pointer to the fragment head.  */
            search_ptr =  fragment_head;

            /* Set the current expected offset to 0.  */
            current_offset =  0;

            /* Loop through the packet chain to see if all the fragments have
               arrived.  */
            incomplete = 0;
            do
            {

#ifndef NX_DISABLE_IPV4
                if (ip_version == NX_IP_VERSION_V4)
                {

                    /* Build the IP header pointer.  */
                    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                    search_header =  (NX_IPV4_HEADER *)search_ptr -> nx_packet_prepend_ptr;

                    /* Check for the expected current offset.  */
                    if (current_offset != (search_header -> nx_ip_header_word_1 & NX_IP_OFFSET_MASK))
                    {

                        incomplete = 1;
                        /* There are still more fragments necessary to reassemble this packet
                           so just return.  */
                        break;
                    }

                    /* Calculate the next expected offset.  */
                    current_offset =  current_offset +
                        ((search_header -> nx_ip_header_word_0 & NX_LOWER_16_MASK) - (ULONG)sizeof(NX_IPV4_HEADER)) /
                        NX_IP_ALIGN_FRAGS;
                }
#endif /* NX_DISABLE_IPV4 */

#ifdef FEATURE_NX_IPV6
                if (ip_version == NX_IP_VERSION_V6)
                {

                    /* Build the IP header pointer.  */
                    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                    search_v6_fragment_option = (NX_IPV6_HEADER_FRAGMENT_OPTION *)search_ptr -> nx_packet_prepend_ptr;

                    /* Check for the expected current offset.  */
                    if (current_offset != (ULONG)(search_v6_fragment_option -> nx_ipv6_header_fragment_option_offset_flag & 0xFFF8))
                    {

                        incomplete = 1;
                        /* There are still more fragments necessary to reassemble this packet
                           so just return.  */
                        break;
                    }

                    /* Calculate the next expected offset.  */
                    current_offset =  current_offset +
                        (search_ptr -> nx_packet_length - (ULONG)sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION));
                }
#endif /* FEATURE_NX_IPV6 */

                /* Move the search pointer forward to the next fragment.  */
                search_ptr =    search_ptr -> nx_packet_union_next.nx_packet_fragment_next;
            } while (search_ptr);

            if (incomplete)
            {
                continue;
            }

            /* At this point the search header points to the last fragment in the chain.  In
               order for the packet to be complete, the "more fragments" bit in its IP header
               must be clear.  */
            /*lint -e{613} suppress possible use of null pointer, since "search_header" and "search_v6_fragment_option" were set to none NULL in the while loop above. */
            if (
#ifndef NX_DISABLE_IPV4
                ((ip_version == NX_IP_VERSION_V4) && (search_header -> nx_ip_header_word_1 & NX_IP_MORE_FRAGMENT))
#ifdef FEATURE_NX_IPV6
                ||
#endif /* FEATURE_NX_IPV6 */
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
                ((ip_version == NX_IP_VERSION_V6) && (search_v6_fragment_option -> nx_ipv6_header_fragment_option_offset_flag & 1))
#endif /* FEATURE_NX_IPV6 */
               )
            {

                /* There are still more fragments necessary to re-assembly this packet
                   so just return.  */
                continue;
            }

            /* The packet can be reassembled under the fragment head pointer now.  It must now
               be removed from the re-assembly list.  */
            if (previous_fragment)
            {

                /* Remove the fragment from a position other than the head of the assembly list.  */
                previous_fragment -> nx_packet_queue_next =  fragment_head -> nx_packet_queue_next;
            }
            else
            {

                /* Modify the head of the re-assembly list.  */
                ip_ptr -> nx_ip_fragment_assembly_head =  fragment_head -> nx_packet_queue_next;

                /* Clear the timeout fragment pointer since we are removing the first
                   fragment (the oldest) on the assembly list.  */
                ip_ptr -> nx_ip_timeout_fragment =  NX_NULL;
            }

            /* Determine if we need to adjust the tail pointer.  */
            if (fragment_head == ip_ptr -> nx_ip_fragment_assembly_tail)
            {

                /* Setup the new tail pointer.  */
                ip_ptr -> nx_ip_fragment_assembly_tail =  previous_fragment;
            }

            /* If we get here, the necessary fragments to reassemble the packet
               are indeed available.  We now need to loop through the packet and reassemble
               it.  */
            search_ptr =       fragment_head -> nx_packet_union_next.nx_packet_fragment_next;
            previous_fragment = fragment_head;

            /* Loop through the fragments and assemble the IP fragment.  */
            while (search_ptr)
            {

                /* Reset tcp_queue_next before releasing. */
                /*lint -e{923} suppress cast of ULONG to pointer.  */
                previous_fragment -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ALLOCATED;

#ifndef NX_DISABLE_IPV4
                if (ip_version == NX_IP_VERSION_V4)
                {
                    /* Accumulate the new length into the head packet.  */
                    fragment_head -> nx_packet_length =  fragment_head -> nx_packet_length +
                        search_ptr -> nx_packet_length - (ULONG)sizeof(NX_IPV4_HEADER);

                    /* Position past the IP header in the subsequent packets.  */
                    search_ptr -> nx_packet_prepend_ptr =  search_ptr -> nx_packet_prepend_ptr +
                        sizeof(NX_IPV4_HEADER);
                }
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
                if (ip_version == NX_IP_VERSION_V6)
                {

                    /* For IPv6, we move the prepend ptr to the next option .*/
                    search_ptr -> nx_packet_prepend_ptr += sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION);
                    search_ptr -> nx_packet_length -= (ULONG)sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION);

                    /* Accumulate the new length into the head packet. */
                    fragment_head -> nx_packet_length += search_ptr -> nx_packet_length;
                }

#endif /* FEATURE_NX_IPV6 */

                /* Link the addition fragment to the head fragment.  */
                if (fragment_head -> nx_packet_last)
                {
                    (fragment_head -> nx_packet_last) -> nx_packet_next =  search_ptr;
                }
                else
                {
                    fragment_head -> nx_packet_next =  search_ptr;
                }
                if (search_ptr -> nx_packet_last)
                {
                    fragment_head -> nx_packet_last =  search_ptr -> nx_packet_last;
                }
                else
                {
                    fragment_head -> nx_packet_last =  search_ptr;
                }

                /* Set the previous fragment. */
                previous_fragment = search_ptr;

                /* Move to the next fragment in the chain.  */
                search_ptr =  search_ptr -> nx_packet_union_next.nx_packet_fragment_next;
            }

            /* Reset tcp_queue_next before releasing. */
            /*lint -e{923} suppress cast of ULONG to pointer.  */
            previous_fragment -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ALLOCATED;

            /* We are now ready to dispatch this packet just like the normal IP receive packet
               processing.  */

#ifndef NX_DISABLE_IP_INFO

            /* Increment the number of packets reassembled.  */
            ip_ptr -> nx_ip_packets_reassembled++;

            /* Increment the number of packets delivered.  */
            ip_ptr -> nx_ip_total_packets_delivered++;

            /* Increment the IP packet bytes received (not including the header).  */
            ip_ptr -> nx_ip_total_bytes_received +=  fragment_head -> nx_packet_length;
#endif

            /* Build a pointer to the IP header.  */
#ifndef NX_DISABLE_IPV4
            if (ip_version == NX_IP_VERSION_V4)
            {

                /* The packet is now reassembled. */

                /* Check if this IP interface has a NAT forwarding service. If so, let NAT get the
                   packet first and if it is not a packet that should be forwarded by NAT, then
                   let NetX process the packet in the normal way.  */

#ifdef NX_NAT_ENABLE

                /* Check if this IP interface has a NAT forwarding service. */
                if (ip_ptr -> nx_ip_nat_packet_process)
                {

                    /* Yes, so forward this packet to the NAT handler.  If NAT does not 'consume' this
                       packet, allow NetX to process the packet.  */
                    packet_consumed = (ip_ptr -> nx_ip_nat_packet_process)(ip_ptr, fragment_head, NX_TRUE);

                    /* Check to see if the packet has been consumed by NAT.  */
                    if (packet_consumed)
                    {

#ifndef NX_DISABLE_IP_INFO

                        /* Increment the IP packets forwarded counter.  */
                        ip_ptr -> nx_ip_packets_forwarded++;
#endif /* NX_DISABLE_IP_INFO */

                        continue;
                    }

                    /* (NetX will process all packets that drop through here.) */
                }
#endif

                /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                current_header = (NX_IPV4_HEADER *)fragment_head -> nx_packet_ip_header;

                /* Determine what protocol the current IP datagram is.  */
                protocol = (current_header -> nx_ip_header_word_2 >> 16) & 0xFF;

                /* Remove the IP header from the packet.  */
                fragment_head -> nx_packet_prepend_ptr = fragment_head -> nx_packet_prepend_ptr + sizeof(NX_IPV4_HEADER);

                /* Adjust the length.  */
                fragment_head -> nx_packet_length = fragment_head -> nx_packet_length - (ULONG)sizeof(NX_IPV4_HEADER);
            }
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
            if (ip_version == NX_IP_VERSION_V6)
            {
                fragment_head -> nx_packet_prepend_ptr += sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION);
                fragment_head -> nx_packet_length -= (ULONG)sizeof(NX_IPV6_HEADER_FRAGMENT_OPTION);

                /*lint -e{613} suppress possible use of null pointer, since "current_pkt_ip_header" was set to none NULL above. */
                protocol = current_v6_fragment_option -> nx_ipv6_header_fragment_option_next_header;
            }
#endif

            /* Call the dispatch function go to process the packet. */
            if (_nx_ip_dispatch_process(ip_ptr, fragment_head, (UINT)protocol))
            {

                /* Toss the IP packet since we don't know what to do with it!  */
                _nx_packet_release(fragment_head);
            }
        }
        else
        {

            /* No other packet was found on the re-assembly list so this packet must be the
               first one of a new IP packet.  Just add it to the end of the assembly queue.  */
            if (ip_ptr -> nx_ip_fragment_assembly_head)
            {

                /* Re-assembly list is not empty.  Just place this IP packet at the
                   end of the IP fragment assembly list.  */
                ip_ptr -> nx_ip_fragment_assembly_tail -> nx_packet_queue_next =   current_fragment;
                ip_ptr -> nx_ip_fragment_assembly_tail =                           current_fragment;
                current_fragment -> nx_packet_queue_next =                         NX_NULL;
                current_fragment -> nx_packet_union_next.nx_packet_fragment_next = NX_NULL;
            }
            else
            {

                /* First IP fragment on the assembly list.  Setup the head and tail pointers to
                   this packet.  */
                ip_ptr -> nx_ip_fragment_assembly_head =                           current_fragment;
                ip_ptr -> nx_ip_fragment_assembly_tail =                           current_fragment;
                current_fragment -> nx_packet_queue_next =                         NX_NULL;
                current_fragment -> nx_packet_union_next.nx_packet_fragment_next = NX_NULL;
            }
        }
    }
}
#endif /* NX_DISABLE_FRAGMENTATION */

