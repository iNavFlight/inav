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
#include "nx_ip.h"
#include "nx_icmp.h"
#include "nx_packet.h"
#include "tx_thread.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmp_packet_receive                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives an ICMP packet from the IP receive           */
/*    processing.  If this routine is called from an ISR, it simply       */
/*    places the new message on the ICMP message queue, and wakes up the  */
/*    IP processing thread.  If this routine is called from the IP helper */
/*    thread, then the ICMP message is processed directly.                */
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
/*    _nx_icmp_packet_process               Process ICMP packet           */
/*    tx_event_flags_set                    Set event flags for IP helper */
/*                                            thread                      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_packet_receive                 Dispatch received IP packets  */
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
VOID  _nx_icmp_packet_receive(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{
#ifndef NX_DISABLE_IPV4

TX_INTERRUPT_SAVE_AREA

#ifdef NX_ENABLE_ICMP_ADDRESS_CHECK
NX_IPV4_HEADER *ip_header_ptr;
NX_INTERFACE   *interface_ptr;
#endif /* NX_ENABLE_ICMP_ADDRESS_CHECK  */


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

#ifndef NX_DISABLE_ICMP_INFO

    /* Increment the ICMP total messages received counter.  */
    ip_ptr -> nx_ip_icmp_total_messages_received++;
#endif

#ifndef NX_DISABLE_RX_SIZE_CHECKING

    /* Check for valid packet length.  */
    if (packet_ptr -> nx_packet_length < sizeof(NX_ICMP_HEADER))
    {

#ifndef NX_DISABLE_ICMP_INFO

        /* Increment the ICMP invalid packet error.  */
        ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif

        /* Invalid packet length, just release it.  */
        _nx_packet_release(packet_ptr);

        /* The function is complete, just return!  */
        return;
    }
#endif

#ifdef NX_ENABLE_ICMP_ADDRESS_CHECK

    /* An ICMP Echo Request destined to an IP broadcast or IP multicast address
       MAY be silently discarded.
       RFC 1122, Section 3.2.2.6, Page42.  */

    /* Pickup the IP header and interface.  */
    ip_header_ptr = (NX_IPV4_HEADER *)(packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_IPV4_HEADER));
    interface_ptr = packet_ptr -> nx_packet_address.nx_packet_interface_ptr;

    if (
        /* Check for Multicast address */
        ((ip_header_ptr -> nx_ip_header_destination_ip & NX_IP_CLASS_D_MASK) == NX_IP_CLASS_D_TYPE) ||
        /* Check for subnet-directed broadcast */
        (((ip_header_ptr -> nx_ip_header_destination_ip & interface_ptr -> nx_interface_ip_network_mask) == interface_ptr -> nx_interface_ip_network) &&
         ((ip_header_ptr -> nx_ip_header_destination_ip & ~(interface_ptr -> nx_interface_ip_network_mask)) == ~(interface_ptr -> nx_interface_ip_network_mask))) ||
        /* Check for local subnet address */
        (ip_header_ptr -> nx_ip_header_destination_ip == interface_ptr -> nx_interface_ip_network)  ||
        /* Check for limited broadcast */
        (ip_header_ptr -> nx_ip_header_destination_ip == NX_IP_LIMITED_BROADCAST)
       )
    {

#ifndef NX_DISABLE_ICMP_INFO

        /* Increment the ICMP invalid packet error.  */
        ip_ptr -> nx_ip_icmp_invalid_packets++;
#endif

        /* Invalid packet length, just release it.  */
        _nx_packet_release(packet_ptr);

        /* The function is complete, just return!  */
        return;
    }
#endif /* NX_ENABLE_ICMP_ADDRESS_CHECK  */

    /* Determine if this routine is being called from an ISR.  */
    if ((TX_THREAD_GET_SYSTEM_STATE()) || (&(ip_ptr -> nx_ip_thread) != _tx_thread_current_ptr))
    {

        /* If system state is non-zero, we are in an ISR. If the current thread is not the IP thread,
           we need to prevent unnecessary recursion in loopback. Just place the message at the
           end of the ICMP message queue and wakeup the IP helper thread.  */

        /* Disable interrupts.  */
        TX_DISABLE

        /* Add the packet to the ICMP message queue.  */
        if (ip_ptr -> nx_ip_icmp_queue_head)
        {

            /* Link the current packet to the list head.  */
            packet_ptr -> nx_packet_queue_next =  ip_ptr -> nx_ip_icmp_queue_head;
        }
        else
        {

            /* Empty queue, add to the head of the ICMP message queue.  */
            packet_ptr -> nx_packet_queue_next =  NX_NULL;
        }

        /* Update the queue head pointer.  */
        ip_ptr -> nx_ip_icmp_queue_head =  packet_ptr;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Wakeup IP thread for processing one or more messages in the ICMP queue.  */
        tx_event_flags_set(&(ip_ptr -> nx_ip_events), NX_IP_ICMP_EVENT, TX_OR);
    }
    else
    {

        /* The IP message was deferred, so this routine is called from the IP helper
           thread and thus may call the ICMP processing directly.  */
        _nx_icmp_packet_process(ip_ptr, packet_ptr);
    }
#else
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(packet_ptr);
#endif /* NX_DISABLE_IPV4 */
}

