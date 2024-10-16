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

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_driver_packet_send                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends an IP packet to the appropriate link driver.    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*    destination_ip                        Destination IP address        */
/*    fragment                              Don't fragment bit            */
/*    next_hop_address                      Next Hop address              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    (_nx_arp_entry_allocate)              ARP entry allocate service    */
/*    (_nx_arp_packet_send)                 Send an ARP packet            */
/*    _nx_ip_packet_deferred_receive        Receive loopback packet       */
/*    _nx_packet_copy                       Copy packet to input packet   */
/*    _nx_packet_transmit_release           Release transmit packet       */
/*    (nx_ip_fragment_processing)           Fragment processing           */
/*    (ip_link_driver)                      User supplied link driver     */
/*    _nx_ip_packet_checksum_compute        Compute checksum              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
VOID  _nx_ip_driver_packet_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr, ULONG destination_ip, ULONG fragment, ULONG next_hop_address)
{

TX_INTERRUPT_SAVE_AREA
NX_IP_DRIVER driver_request;
UINT         index;
ULONG        network_mask;
ULONG        network;
UCHAR        loopback = NX_FALSE;
NX_ARP      *arp_ptr;
NX_PACKET   *last_packet;
NX_PACKET   *remove_packet;
NX_PACKET   *packet_copy;
UINT         queued_count;

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Initialize the driver request. */
    driver_request.nx_ip_driver_ptr =                   ip_ptr;
    driver_request.nx_ip_driver_packet =                packet_ptr;
    driver_request.nx_ip_driver_interface =             packet_ptr -> nx_packet_address.nx_packet_interface_ptr;
    driver_request.nx_ip_driver_command =               NX_LINK_PACKET_SEND;

    /* Determine if physical mapping is needed by the link driver.  */
    if (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_address_mapping_needed)
    {

        /* Get the network and network mask.*/
        network_mask = packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_network_mask;
        network = packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_network;

        /* Determine if an IP limited or directed broadcast is requested.  */
        if ((destination_ip == NX_IP_LIMITED_BROADCAST) ||
            (((destination_ip & network_mask) == network) &&
             ((destination_ip & ~network_mask) == ~network_mask)))
        {

            /* Build the driver request.  */
            driver_request.nx_ip_driver_command =               NX_LINK_PACKET_BROADCAST;
            driver_request.nx_ip_driver_physical_address_msw =  0xFFFFUL;
            driver_request.nx_ip_driver_physical_address_lsw =  0xFFFFFFFFUL;
        }
        /* Determine if we have a loopback address.  */
        else if (destination_ip == packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_address)
        {
            loopback = NX_TRUE;
            driver_request.nx_ip_driver_interface = NX_NULL;
        }
        /* Determine if we have a class D multicast address.  */
        else if ((destination_ip & NX_IP_CLASS_D_MASK) == NX_IP_CLASS_D_TYPE)
        {

            /* Yes, we have a class D multicast address.  Derive the physical mapping from
               the class D address.  */

            /* Determine if the group address has been joined in this IP instance.  */
            index =  0;
            while (index < NX_MAX_MULTICAST_GROUPS)
            {

                /* Determine if the destination address matches the requested address.  */
                if (ip_ptr -> nx_ipv4_multicast_entry[index].nx_ipv4_multicast_join_list == destination_ip)
                {

                    /* Yes, break the loop!  */
                    break;
                }

                /* Increment the join list index.  */
                index++;
            }

            /* Determine if the group was joined by this IP instance.  */
            if (index < NX_MAX_MULTICAST_GROUPS)
            {

                /* Determine if the group has loopback enabled.  */
                if (ip_ptr -> nx_ipv4_multicast_entry[index].nx_ipv4_multicast_loopback_enable)
                {
                    loopback = NX_TRUE;
                }
            }

            /* Build the driver request. Derive the physical mapping from
               the class D address.  */
            driver_request.nx_ip_driver_physical_address_msw =  NX_IP_MULTICAST_UPPER;
            driver_request.nx_ip_driver_physical_address_lsw =  NX_IP_MULTICAST_LOWER | (destination_ip & NX_IP_MULTICAST_MASK);
        }
        else
        {

            NX_PARAMETER_NOT_USED(fragment);
            /* Look into the ARP Routing Table to derive the physical address.  */

            /* If we get here, the packet destination is a unicast address.  */
            destination_ip = next_hop_address;

            /* Calculate the hash index for the destination IP address.  */
            index =  (UINT)((destination_ip + (destination_ip >> 8)) & NX_ARP_TABLE_MASK);

            /* Determine if there is an entry for this IP address.  */
            arp_ptr =  ip_ptr -> nx_ip_arp_table[index];

            /* Loop to look for an ARP match.  */
            while (arp_ptr)
            {

                /* Determine if this arp entry matches the destination IP address.  */
                if (arp_ptr -> nx_arp_ip_address == destination_ip)
                {

                    /* Yes, we found a match.  Get out of the loop!  */
                    break;
                }

                /* Move to the next active ARP entry.  */
                arp_ptr =  arp_ptr -> nx_arp_active_next;

                /* Determine if we are at the end of the ARP list.  */
                if (arp_ptr == ip_ptr -> nx_ip_arp_table[index])
                {
                    /* Clear the ARP pointer.  */
                    arp_ptr =  NX_NULL;
                    break;
                }
            }

            /* Determine if we actually found a matching and effective ARP entry.  */
            if ((arp_ptr) && (arp_ptr -> nx_arp_physical_address_msw | arp_ptr -> nx_arp_physical_address_lsw))
            {

                /* Disable interrupts temporarily.  */
                TX_DISABLE

                /* Yes, we have a physical mapping.  Copy the physical address into the driver
                   request structure.  */
                driver_request.nx_ip_driver_physical_address_msw =  arp_ptr -> nx_arp_physical_address_msw;
                driver_request.nx_ip_driver_physical_address_lsw =  arp_ptr -> nx_arp_physical_address_lsw;

                /* Move this ARP entry to the head of the list.  */
                ip_ptr -> nx_ip_arp_table[index] =  arp_ptr;

                /* Restore interrupts.  */
                TX_RESTORE
            }
            else
            {

                /* Determine if fragmentation is needed before queue the packet on the ARP waiting queue.  */
                if (packet_ptr -> nx_packet_length > packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_mtu_size)
                {

#ifndef NX_DISABLE_FRAGMENTATION
                    /* Check the DF bit flag.  */
                    if ((ip_ptr -> nx_ip_fragment_processing == NX_NULL) || (fragment != NX_FRAGMENT_OKAY))
#endif
                    {

#ifndef NX_DISABLE_IP_INFO

                        /* Increment the IP send packets dropped count.  */
                        ip_ptr -> nx_ip_send_packets_dropped++;
#endif
                        /* Just release the packet.  */
                        _nx_packet_transmit_release(packet_ptr);

                        /* Return... nothing more can be done!  */
                        return;
                    }
                }

                /* Determine if we actually found a matching ARP entry.  */
                if (arp_ptr)
                {

                    /* Yes, we have an existing ARP mapping entry.  */

                    /* Disable interrupts temporarily.  */
                    TX_DISABLE

                    /* Ensure the current packet's queue next pointer to NULL.  */
                    packet_ptr -> nx_packet_queue_next =  NX_NULL;

                    /* Determine if the queue is empty.  */
                    if (arp_ptr -> nx_arp_packets_waiting == NX_NULL)
                    {

                        /* Yes, we have an empty ARP packet queue.  Simply place the
                           packet at the head of the list.  */
                        arp_ptr -> nx_arp_packets_waiting =  packet_ptr;

                        /* Add debug information. */
                        NX_PACKET_DEBUG(NX_PACKET_ARP_WAITING_QUEUE, __LINE__, packet_ptr);

                        /* Restore interrupts.  */
                        TX_RESTORE
                    }
                    else
                    {

                        /* Determine how many packets are on the ARP entry's packet
                           queue and remember the last packet in the queue.  We know
                           there is at least one on the queue and another that is
                           going to be queued.  */
                        last_packet =  arp_ptr -> nx_arp_packets_waiting;
                        queued_count = 1;
                        while (last_packet -> nx_packet_queue_next)
                        {

                            /* Increment the queued count.  */
                            queued_count++;

                            /* Move to the next packet in the queue.  */
                            last_packet =  last_packet -> nx_packet_queue_next;
                        }

                        /* Add debug information. */
                        NX_PACKET_DEBUG(NX_PACKET_ARP_WAITING_QUEUE, __LINE__, packet_ptr);

                        /* Place the packet at the end of the list.  */
                        last_packet -> nx_packet_queue_next =  packet_ptr;

                        /* Default the remove packet pointer to NULL.  */
                        remove_packet =  NX_NULL;

                        /* Determine if the packets queued has exceeded the queue
                           depth.  */
                        if (queued_count >= NX_ARP_MAX_QUEUE_DEPTH)
                        {

                            /* Save the packet pointer at the head of the list.  */
                            remove_packet =  arp_ptr -> nx_arp_packets_waiting;

                            /* Remove the packet from the ARP queue.  */
                            arp_ptr -> nx_arp_packets_waiting =  remove_packet -> nx_packet_queue_next;

                            /* Clear the remove packet queue next pointer.  */
                            remove_packet -> nx_packet_queue_next =  NX_NULL;

#ifndef NX_DISABLE_IP_INFO

                            /* Increment the IP transmit resource error count.  */
                            ip_ptr -> nx_ip_transmit_resource_errors++;

                            /* Increment the IP send packets dropped count.  */
                            ip_ptr -> nx_ip_send_packets_dropped++;
#endif
                        }

                        /* Restore interrupts.  */
                        TX_RESTORE

                        /* Determine if there is a packet to remove.  */
                        if (remove_packet)
                        {

                            /* Yes, the packet queue depth for this ARP entry was exceeded
                               so release the packet that was removed from the queue.  */
                            _nx_packet_transmit_release(remove_packet);
                        }
                    }
                }
                else
                {

                    /* No ARP entry was found.  We need to allocate a new ARP entry, populate it, and
                       initiate an ARP request to get the specific physical mapping.  */

                    /* Allocate a new ARP entry.  */
                    if ((!ip_ptr -> nx_ip_arp_allocate) ||
                        ((ip_ptr -> nx_ip_arp_allocate)(ip_ptr, &(ip_ptr -> nx_ip_arp_table[index]), NX_FALSE)))
                    {

                        /* Error, release the protection and the packet.  */

#ifndef NX_DISABLE_IP_INFO

                        /* Increment the IP transmit resource error count.  */
                        ip_ptr -> nx_ip_transmit_resource_errors++;

                        /* Increment the IP send packets dropped count.  */
                        ip_ptr -> nx_ip_send_packets_dropped++;
#endif

                        /* Release the packet.  */
                        _nx_packet_transmit_release(packet_ptr);

                        /* Just return!  */
                        return;
                    }

                    /* Otherwise, setup a pointer to the new ARP entry.  */
                    arp_ptr =  (ip_ptr -> nx_ip_arp_table[index]) -> nx_arp_active_previous;

                    /* Setup the IP address and clear the physical mapping.  */
                    arp_ptr -> nx_arp_ip_address =            destination_ip;
                    arp_ptr -> nx_arp_physical_address_msw =  0;
                    arp_ptr -> nx_arp_physical_address_lsw =  0;
                    arp_ptr -> nx_arp_entry_next_update =     NX_ARP_UPDATE_RATE;
                    arp_ptr -> nx_arp_retries =               0;
                    arp_ptr -> nx_arp_ip_interface =          packet_ptr -> nx_packet_address.nx_packet_interface_ptr;

                    /* Ensure the queue next pointer is NULL for the packet before it
                       is placed on the ARP waiting queue.  */
                    packet_ptr -> nx_packet_queue_next =  NX_NULL;

                    /* Add debug information. */
                    NX_PACKET_DEBUG(NX_PACKET_ARP_WAITING_QUEUE, __LINE__, packet_ptr);

                    /* Queue the packet for output.  */
                    arp_ptr -> nx_arp_packets_waiting =  packet_ptr;

                    /* Call ARP send to send an ARP request.  */
                    (ip_ptr -> nx_ip_arp_packet_send)(ip_ptr, destination_ip, packet_ptr -> nx_packet_address.nx_packet_interface_ptr);
                }

                /* Just return!  */
                return;
            }
        }
    }
    else
    {

        /* This IP instance does not require any IP-to-physical mapping.  */

        /* Determine if we have a loopback address.  */
        if ((((destination_ip >= NX_IP_LOOPBACK_FIRST) &&
              (destination_ip <= NX_IP_LOOPBACK_LAST))) ||
            (destination_ip == packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_address))
        {

            /* Yes, we have an internal loopback address.  */
            loopback = NX_TRUE;
            driver_request.nx_ip_driver_interface = NX_NULL;
        }
    }

    /* Check whether the packet should be loop back. */
    if (loopback == NX_TRUE)
    {

        /* Copy the packet so it can be enqueued properly by the receive
           processing.  */
        if (_nx_packet_copy(packet_ptr, &packet_copy, ip_ptr -> nx_ip_default_packet_pool, NX_NO_WAIT) == NX_SUCCESS)
        {

#ifdef NX_ENABLE_INTERFACE_CAPABILITY

            /* Compute checksum for upper layer protocol. */
            /*lint --e{644} suppress variable might not be initialized, since "packet_copy" was initialized as long as return value is NX_SUCCESS. */
            if (packet_copy -> nx_packet_interface_capability_flag)
            {
                _nx_ip_packet_checksum_compute(packet_copy);
            }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP packet sent count.  */
            ip_ptr -> nx_ip_total_packets_sent++;

            /* Increment the IP bytes sent count.  */
            ip_ptr -> nx_ip_total_bytes_sent +=  packet_ptr -> nx_packet_length - (ULONG)sizeof(NX_IPV4_HEADER);
#endif

#ifdef NX_IPSEC_ENABLE
            /* Clear the ipsec sa pointer.  */
            packet_copy -> nx_packet_ipsec_sa_ptr = NX_NULL;
#endif /* NX_IPSEC_ENABLE  */

            /* Add debug information. */
            /*lint --e{644} suppress variable might not be initialized, since "packet_copy" was initialized as long as return value is NX_SUCCESS. */
            NX_PACKET_DEBUG(__FILE__, __LINE__, packet_copy);

            /* Send the packet to this IP's receive processing like it came in from the
               driver.  */
            _nx_ip_packet_deferred_receive(ip_ptr, packet_copy);
        }
#ifndef NX_DISABLE_IP_INFO
        else
        {

            /* Increment the IP send packets dropped count.  */
            ip_ptr -> nx_ip_send_packets_dropped++;

            /* Increment the IP transmit resource error count.  */
            ip_ptr -> nx_ip_transmit_resource_errors++;
        }
#endif
    }

    /* Check whether the packet should be sent through driver. */
    if (driver_request.nx_ip_driver_interface)
    {

        /* Determine if fragmentation is needed.  */
        if (packet_ptr -> nx_packet_length > packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_mtu_size)
        {

#ifndef NX_DISABLE_FRAGMENTATION
            /* Check the DF bit flag.  */
            if ((ip_ptr -> nx_ip_fragment_processing) && (fragment != NX_DONT_FRAGMENT))
            {

                /* Fragmentation is needed, call the IP fragment processing routine.  */
                (ip_ptr -> nx_ip_fragment_processing)(&driver_request);
            }
            else
#endif /* NX_DISABLE_FRAGMENTATION */
            {

#ifndef NX_DISABLE_IP_INFO

                /* Increment the IP send packets dropped count.  */
                ip_ptr -> nx_ip_send_packets_dropped++;
#endif
                /* Just release the packet.  */
                _nx_packet_transmit_release(packet_ptr);
            }

            /* In either case, this packet send is complete, just return.  */
            return;
        }

#ifndef NX_DISABLE_IP_INFO

        /* Increment the IP packet sent count.  */
        ip_ptr -> nx_ip_total_packets_sent++;

        /* Increment the IP bytes sent count.  */
        ip_ptr -> nx_ip_total_bytes_sent +=  packet_ptr -> nx_packet_length - (ULONG)sizeof(NX_IPV4_HEADER);
#endif

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_PACKET_SEND, ip_ptr, packet_ptr, packet_ptr -> nx_packet_length, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Add debug information. */
        NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

        /* Driver entry must not be NULL. */
        NX_ASSERT(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_link_driver_entry != NX_NULL);

        /* Broadcast packet.  */
        (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_link_driver_entry)(&driver_request);
    }
    else
    {

        /* Release the transmit packet.  */
        _nx_packet_transmit_release(packet_ptr);
    }
}
#endif /* !NX_DISABLE_IPV4  */

