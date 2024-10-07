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
#include "nx_igmp.h"
#include "nx_packet.h"
#include "nx_udp.h"

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ipv4_packet_receive                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives a packet from the nx_ip_packet_receive.      */
/*    nx_ip_packet_receive forwards only IPv4 packet to this function.    */
/*    Here it is either processes it or places it in a deferred           */
/*    processing queue, depending on the complexity of the packet.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_checksum_compute               Compute IP checksum           */
/*    _nx_igmp_multicast_check              Check for Multicast match     */
/*    _nx_packet_release                    Release packet to packet pool */
/*    tx_event_flags_set                    Set events for IP thread      */
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
VOID  _nx_ipv4_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

TX_INTERRUPT_SAVE_AREA
#ifndef NX_DISABLE_PACKET_CHAIN
NX_PACKET      *before_last_packet;
NX_PACKET      *last_packet;
#endif /* NX_DISABLE_PACKET_CHAIN */
NX_IPV4_HEADER *ip_header_ptr;
ULONG          *word_ptr;
ULONG           ip_header_length;
ULONG           protocol;
ULONG           delta;
ULONG           val;
ULONG           pkt_length;
ULONG           checksum;
NX_INTERFACE   *if_ptr;
NX_UDP_HEADER  *udp_header_ptr;
UINT            dest_port;
UINT            option_processed;
#if defined(NX_DISABLE_IP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY)
UINT            compute_checksum = 1;
#endif /* defined(NX_DISABLE_IP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) */
#ifdef NX_NAT_ENABLE
UINT            packet_consumed;
#endif

#ifdef NX_DISABLE_IP_RX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_IP_RX_CHECKSUM */

    /* It's assumed that the IP link driver has positioned the top pointer in the
       packet to the start of the IP address... so that's where we will start.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ip_header_ptr = (NX_IPV4_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IP_RECEIVE, ip_ptr, ip_header_ptr -> nx_ip_header_source_ip, packet_ptr, packet_ptr -> nx_packet_length, NX_TRACE_INTERNAL_EVENTS, 0, 0);


    /* Pick up the first word in the IP header. */
    val = ip_header_ptr -> nx_ip_header_word_0;

    /* Convert to host byte order. */
    NX_CHANGE_ULONG_ENDIAN(val);

    /* Obtain packet length. */
    pkt_length = val & NX_LOWER_16_MASK;

    /* Make sure the IP length matches the packet length.  Some Ethernet devices
       add padding to small packets, which results in a discrepancy between the
       packet length and the IP header length.  */
    if (packet_ptr -> nx_packet_length != pkt_length)
    {

        /* Determine if the packet length is less than the size reported in the IP header.  */
        if (packet_ptr -> nx_packet_length < pkt_length)
        {

            /* Packet is too small!  */

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP invalid packet error.  */
            ip_ptr -> nx_ip_invalid_packets++;

            /* Increment the IP receive packets dropped count.  */
            ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

            /* Invalid packet length, just release it.  */
            _nx_packet_release(packet_ptr);

            /* The function is complete, just return!  */
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

    /* Get the incoming interface. */
    if_ptr = packet_ptr -> nx_packet_address.nx_packet_interface_ptr;

    /* Obtain IP header length. */
    ip_header_length =  (val & NX_IP_LENGTH_MASK) >> 24;

#ifndef NX_DISABLE_RX_SIZE_CHECKING

    /* Check for minimal packet length. The check is done after the endian swapping
       since the compiler may possibly be able to optimize the lookup of
       "nx_packet_length" and therefore reduce the amount of work performing these
       size checks. The endian logic is okay since packets must always have
       payloads greater than the IP header in size.  */
    if ((packet_ptr -> nx_packet_length <= (ip_header_length << 2)) ||
        (ip_header_length < NX_IP_NORMAL_LENGTH))
    {

        /* Packet is too small!  */

#ifndef NX_DISABLE_IP_INFO

        /* Increment the IP invalid packet error.  */
        ip_ptr -> nx_ip_invalid_packets++;

        /* Increment the IP receive packets dropped count.  */
        ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

        /* Invalid packet length, just release it.  */
        _nx_packet_release(packet_ptr);

        /* The function is complete, just return!  */
        return;
    }
#endif

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (if_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_IPV4_RX_CHECKSUM)
    {
        compute_checksum = 0;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
#if defined(NX_DISABLE_IP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY)
    if (compute_checksum == 1)
#endif /* defined(NX_DISABLE_IP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) */
    {


        checksum = _nx_ip_checksum_compute(packet_ptr, NX_IP_VERSION_V4,
                                           /* length is the size of IP header, including options */
                                           (UINT)(ip_header_length << 2),
                                           /* IPv4 header checksum doesn't care src/dest addresses */
                                           NULL, NULL);
        checksum =  ~checksum & NX_LOWER_16_MASK;

        /* Check the checksum again.  */
        if (checksum)
        {

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP invalid packet error.  */
            ip_ptr -> nx_ip_invalid_packets++;

            /* Increment the IP checksum error.  */
            ip_ptr -> nx_ip_receive_checksum_errors++;

            /* Increment the IP receive packets dropped count.  */
            ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

            /* Checksum error, just release it.  */
            _nx_packet_release(packet_ptr);

            /* The function is complete, just return!  */
            return;
        }
    }

    /* IP receive checksum processing is disabled... just check for and remove if
       necessary the IP option words.  */

    /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
       swap the endian of the IP header.  */
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_1);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_word_2);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_source_ip);
    NX_CHANGE_ULONG_ENDIAN(ip_header_ptr -> nx_ip_header_destination_ip);

#ifdef NX_ENABLE_SOURCE_ADDRESS_CHECK
    /* Check whether source address is valid. */
    /* Section 3.2.1.3, page 30, RFC 1122. */
    if (if_ptr -> nx_interface_address_mapping_needed == NX_TRUE)
    {
        if (((ip_header_ptr -> nx_ip_header_source_ip & ~(if_ptr -> nx_interface_ip_network_mask)) == ~(if_ptr -> nx_interface_ip_network_mask)) ||
            (((ip_header_ptr -> nx_ip_header_source_ip & ~(if_ptr -> nx_interface_ip_network_mask)) == 0) &&
             (ip_header_ptr -> nx_ip_header_source_ip != 0)) ||
            ((ip_header_ptr -> nx_ip_header_source_ip & NX_IP_CLASS_D_MASK) == NX_IP_CLASS_D_TYPE))
        {

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP invalid address error.  */
            ip_ptr -> nx_ip_invalid_receive_address++;

            /* Increment the IP receive packets dropped count.  */
            ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

            /* Toss the IP packet since we don't know what to do with it!  */
            _nx_packet_release(packet_ptr);

            /* Return to caller.  */
            return;
        }
    }
#endif /* NX_ENABLE_SOURCE_ADDRESS_CHECK */

    /* Determine if there are options in the IP header that make the length greater
       than the default length.  */
    if (ip_header_length > NX_IP_NORMAL_LENGTH)
    {

        /* Process the IPv4 option.  */
        option_processed = _nx_ipv4_option_process(ip_ptr, packet_ptr);

        /* Check the status.  */
        if (option_processed == NX_FALSE)
        {

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP receive packets dropped count.  */
            ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

            /* IPv4 option error, toss the packet!  */
            _nx_packet_release(packet_ptr);

            /* In all cases, receive processing is finished.  Return to caller.  */
            return;
        }

        /* Setup a pointer to the last option word.  */
        word_ptr = ((ULONG *)((VOID *)ip_header_ptr)) + ip_header_length - 1;

        /* Remove the option words prior to handling the IP header.  */
        *word_ptr-- = ip_header_ptr -> nx_ip_header_destination_ip;
        *word_ptr-- = ip_header_ptr -> nx_ip_header_source_ip;
        *word_ptr-- = ip_header_ptr -> nx_ip_header_word_2;
        *word_ptr-- = ip_header_ptr -> nx_ip_header_word_1;
        *word_ptr = (ULONG)(((ip_header_ptr -> nx_ip_header_word_0) & (~NX_IP_LENGTH_MASK)) | NX_IP_VERSION);

        /* Update the ip_header_ptr and the packet and the packet prepend pointer, ip header pointer and length.  */
        /*lint -e{929} -e{740} -e{826} suppress cast from pointer to pointer, since it is necessary  */
        ip_header_ptr =  (NX_IPV4_HEADER *)word_ptr;

        /*lint -e{928} suppress cast from pointer to pointer, since it is necessary  */
        packet_ptr -> nx_packet_prepend_ptr = (UCHAR *)word_ptr;
        packet_ptr -> nx_packet_ip_header = packet_ptr -> nx_packet_prepend_ptr;
        packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length - ((ip_header_length -  NX_IP_NORMAL_LENGTH) * (ULONG)sizeof(ULONG));
    }

    /* Check if this IP interface has a NAT forwarding service.  If so, let NAT get the
       packet first and if it is not a packet that should be forwarded by NAT, then
       let NetX process the packet in the normal way.  */

#ifdef NX_NAT_ENABLE
    /* Check if this IP interface has a NAT forwarding service. */
    if (ip_ptr -> nx_ip_nat_packet_process)
    {

        /* Call NAT preprocess hanlder to check if NAT module can process this packet.  */
        if ((ip_ptr -> nx_ip_nat_packet_process)(ip_ptr, packet_ptr, NX_FALSE) == NX_TRUE)
        {

            /* NAT router would need to assemble the fragments together first
               and then translate prior to forwarding. RFC2663, RFC2766. */

            /* Determine if this packet is fragmented.  If so, place it on the deferred processing
               queue. The input packet will then be processed by an IP system thread.  */
            if (ip_header_ptr -> nx_ip_header_word_1 & NX_IP_FRAGMENT_MASK)
            {

#ifndef NX_DISABLE_IP_INFO

                /* Increment the IP receive fragments count.  */
                ip_ptr -> nx_ip_total_fragments_received++;
#endif

                /* Yes, the incoming IP header is fragmented.  Check to see if IP fragmenting
                   has been enabled.  */
                if (ip_ptr -> nx_ip_fragment_assembly)
                {

                    /* Yes, fragmenting is available.  Place the packet on the incoming
                       fragment queue.  */

                    /* Disable interrupts.  */
                    TX_DISABLE

                    /* Determine if the queue is empty.  */
                    if (ip_ptr -> nx_ip_received_fragment_head)
                    {

                        /* Reassembly queue is not empty, add this packet to the end of
                           the queue.  */
                        (ip_ptr -> nx_ip_received_fragment_tail) -> nx_packet_queue_next =  packet_ptr;
                        packet_ptr -> nx_packet_queue_next =  NX_NULL;
                        ip_ptr -> nx_ip_received_fragment_tail =  packet_ptr;
                    }
                    else
                    {

                        /* Reassembly queue is empty.  Just setup the head and tail pointers
                           to point to this packet.  */
                        ip_ptr -> nx_ip_received_fragment_head =  packet_ptr;
                        ip_ptr -> nx_ip_received_fragment_tail =  packet_ptr;
                        packet_ptr -> nx_packet_queue_next =      NX_NULL;
                    }

                    /* Add debug information. */
                    NX_PACKET_DEBUG(NX_PACKET_IP_FRAGMENT_QUEUE, __LINE__, packet_ptr);

                    /* Restore interrupts.  */
                    TX_RESTORE

#ifndef NX_FRAGMENT_IMMEDIATE_ASSEMBLY
                    /* Wakeup IP helper thread to process the IP fragment re-assembly.  */
                    tx_event_flags_set(&(ip_ptr -> nx_ip_events), NX_IP_UNFRAG_EVENT, TX_OR);
#else
                    /* Process the IP fragment reassemble.  */
                    (ip_ptr -> nx_ip_fragment_assembly)(ip_ptr);
#endif /* NX_FRAGMENT_IMMEDIATE_ASSEMBLY */
                }
                else
                {

#ifndef NX_DISABLE_IP_INFO

                    /* Increment the IP receive packets dropped count.  */
                    ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

                    /* Fragmentation has not been enabled, toss the packet!  */
                    _nx_packet_release(packet_ptr);
                }

                /* In all cases, receive processing is finished.  Return to caller.  */
                return;
            }

            /* Normal packet, so forward this packet to the NAT handler. If NAT does not 'consume' this
               packet, allow NetX to process the packet.  */
            packet_consumed = (ip_ptr -> nx_ip_nat_packet_process)(ip_ptr, packet_ptr, NX_TRUE);

            /* Check to see if the packet has been consumed by NAT.  */
            if (packet_consumed)
            {

#ifndef NX_DISABLE_IP_INFO

                /* Increment the IP packets forwarded counter.  */
                ip_ptr -> nx_ip_packets_forwarded++;
#endif /* NX_DISABLE_IP_INFO */

                return;
            }
        }

        /* (NetX will process all packets that drop through here.) */
    }
#endif

    /* Determine if the IP datagram is for this IP address or a broadcast IP on this
       network.  */
    if ((ip_header_ptr -> nx_ip_header_destination_ip == if_ptr -> nx_interface_ip_address) ||

        /* Check for incoming IP address of zero.  Incoming IP address of zero should
           be received regardless of our current IP address.  */
        (ip_header_ptr -> nx_ip_header_destination_ip == 0) ||

        /* Check for IP broadcast.  */
        (((ip_header_ptr -> nx_ip_header_destination_ip & if_ptr -> nx_interface_ip_network_mask) ==
          if_ptr -> nx_interface_ip_network) &&
         ((ip_header_ptr -> nx_ip_header_destination_ip & ~(if_ptr -> nx_interface_ip_network_mask)) ==
          ~(if_ptr -> nx_interface_ip_network_mask))) ||

        /* Check for limited broadcast.  */
        (ip_header_ptr -> nx_ip_header_destination_ip == NX_IP_LIMITED_BROADCAST) ||

        /* Check for loopback address.  */
        ((ip_header_ptr -> nx_ip_header_destination_ip >= NX_IP_LOOPBACK_FIRST) &&
         (ip_header_ptr -> nx_ip_header_destination_ip <= NX_IP_LOOPBACK_LAST)) ||

        /* Check for valid Multicast address.  */
        (_nx_igmp_multicast_check(ip_ptr, ip_header_ptr -> nx_ip_header_destination_ip, if_ptr)))
    {

        /* Determine if this packet is fragmented.  If so, place it on the deferred processing
           queue.  The input packet will then be processed by an IP system thread.  */
        if (ip_header_ptr -> nx_ip_header_word_1 & NX_IP_FRAGMENT_MASK)
        {

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP receive fragments count.  */
            ip_ptr -> nx_ip_total_fragments_received++;
#endif

            /* Yes, the incoming IP header is fragmented.  Check to see if IP fragmenting
               has been enabled.  */
#ifdef NX_ENABLE_LOW_WATERMARK
            if (ip_ptr -> nx_ip_fragment_assembly &&
                (packet_ptr -> nx_packet_pool_owner -> nx_packet_pool_available >=
                 packet_ptr -> nx_packet_pool_owner -> nx_packet_pool_low_watermark))
#else
            if (ip_ptr -> nx_ip_fragment_assembly)
#endif
            {

                /* Yes, fragmenting is available.  Place the packet on the incoming
                   fragment queue.  */

                /* Disable interrupts.  */
                TX_DISABLE

                /* Determine if the queue is empty.  */
                if (ip_ptr -> nx_ip_received_fragment_head)
                {

                    /* Reassembly queue is not empty, add this packet to the end of
                       the queue.  */
                    (ip_ptr -> nx_ip_received_fragment_tail) -> nx_packet_queue_next =  packet_ptr;
                    packet_ptr -> nx_packet_queue_next =  NX_NULL;
                    ip_ptr -> nx_ip_received_fragment_tail =  packet_ptr;
                }
                else
                {

                    /* Reassembly queue is empty.  Just setup the head and tail pointers
                       to point to this packet.  */
                    ip_ptr -> nx_ip_received_fragment_head =  packet_ptr;
                    ip_ptr -> nx_ip_received_fragment_tail =  packet_ptr;
                    packet_ptr -> nx_packet_queue_next =      NX_NULL;
                }

                /* Add debug information. */
                NX_PACKET_DEBUG(NX_PACKET_IP_FRAGMENT_QUEUE, __LINE__, packet_ptr);

                /* Restore interrupts.  */
                TX_RESTORE

#ifndef NX_FRAGMENT_IMMEDIATE_ASSEMBLY
                /* Wakeup IP helper thread to process the IP fragment re-assembly.  */
                tx_event_flags_set(&(ip_ptr -> nx_ip_events), NX_IP_UNFRAG_EVENT, TX_OR);
#else
                /* Process the IP fragment reassemble.  */
                (ip_ptr -> nx_ip_fragment_assembly)(ip_ptr);
#endif /* NX_FRAGMENT_IMMEDIATE_ASSEMBLY */
            }
            else
            {

#ifndef NX_DISABLE_IP_INFO

                /* Increment the IP receive packets dropped count.  */
                ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

                /* Fragmentation has not been enabled, toss the packet!  */
                _nx_packet_release(packet_ptr);
            }

            /* In all cases, receive processing is finished.  Return to caller.  */
            return;
        }

        /* Determine what protocol the current IP datagram is.  */
        protocol =  (ip_header_ptr -> nx_ip_header_word_2 >> 16) & 0xFF;

        /* Remove the IP header from the packet.  */
        packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_IPV4_HEADER);

        /* Adjust the length.  */
        packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length - (ULONG)sizeof(NX_IPV4_HEADER);

#ifndef NX_DISABLE_IP_INFO

        /* Increment the number of packets delivered.  */
        ip_ptr -> nx_ip_total_packets_delivered++;

        /* Increment the IP packet bytes received (not including the header).  */
        ip_ptr -> nx_ip_total_bytes_received +=  packet_ptr -> nx_packet_length;
#endif
        if (_nx_ip_dispatch_process(ip_ptr, packet_ptr, (UINT)protocol))
        {
            _nx_packet_release(packet_ptr);
        }
    }
    /* Try to receive the DHCP message before release this packet.
       NetX should receive the unicast DHCP message when interface IP address is zero.  */

    /* Check if this IP interface has IP address.  */
    else if (if_ptr -> nx_interface_ip_address == 0)
    {

        /* Determine what protocol the current IP datagram is.  */
        protocol =  ip_header_ptr -> nx_ip_header_word_2 & NX_IP_PROTOCOL_MASK;

        /* Check if this packet is UDP message.  */
        if (protocol == NX_IP_UDP)
        {

            /* Remove the IP header from the packet.  */
            packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_IPV4_HEADER);

            /* Adjust the length.  */
            packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length - (ULONG)sizeof(NX_IPV4_HEADER);

#ifndef NX_DISABLE_IP_INFO

            /* Increment the number of packets delivered.  */
            ip_ptr -> nx_ip_total_packets_delivered++;

            /* Increment the IP packet bytes received (not including the header).  */
            ip_ptr -> nx_ip_total_bytes_received +=  packet_ptr -> nx_packet_length;
#endif

            /* Pickup the pointer to the head of the UDP packet.  */
            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            udp_header_ptr =  (NX_UDP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

            /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
               swap the endian of the UDP header.  */
            NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_0);

            /* Pickup the destination UDP port.  */
            dest_port =  (UINT)(udp_header_ptr -> nx_udp_header_word_0 & NX_LOWER_16_MASK);

            /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
               swap the endian of the UDP header.  */
            NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_0);

            /* Check if this packet is DHCP message.  */
            if (dest_port == 68)
            {
                if (ip_ptr -> nx_ip_udp_packet_receive)
                {

                    /* Yes, dispatch it to the appropriate UDP handler if present.  */
                    (ip_ptr -> nx_ip_udp_packet_receive)(ip_ptr, packet_ptr);

                    return;
                }
            }
        }

#ifndef NX_DISABLE_IP_INFO

        /* Decrement the number of packets delivered.  */
        ip_ptr -> nx_ip_total_packets_delivered--;

        /* Decrement the IP packet bytes received (not including the header).  */
        ip_ptr -> nx_ip_total_bytes_received -=  packet_ptr -> nx_packet_length;

        /* Increment the IP invalid address error.  */
        ip_ptr -> nx_ip_invalid_receive_address++;

        /* Increment the IP receive packets dropped count.  */
        ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

        /* Toss the IP packet since we don't know what to do with it!  */
        _nx_packet_release(packet_ptr);

        /* Return to caller.  */
        return;
    }
    else if (ip_ptr -> nx_ip_forward_packet_process)
    {

#ifndef NX_DISABLE_IP_INFO

        /* Increment the IP packets forwarded counter.  */
        ip_ptr -> nx_ip_packets_forwarded++;
#endif

        /* The packet is not for this IP instance so call the
           forward IP packet processing routine.  */
        (ip_ptr -> nx_ip_forward_packet_process)(ip_ptr, packet_ptr);
    }
    else
    {

#ifndef NX_DISABLE_IP_INFO

        /* Increment the IP invalid address error.  */
        ip_ptr -> nx_ip_invalid_receive_address++;

        /* Increment the IP receive packets dropped count.  */
        ip_ptr -> nx_ip_receive_packets_dropped++;
#endif

        /* Toss the IP packet since we don't know what to do with it!  */
        _nx_packet_release(packet_ptr);

        /* Return to caller.  */
        return;
    }
}
#endif /* !NX_DISABLE_IPV4  */

