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
/**   Address Resolution Protocol (ARP)                                   */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_arp.h"
#include "nx_ip.h"
#include "nx_packet.h"

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_arp_queue_send                                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends packets from the ARP queue.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    arp_ptr                               Pointer to ARP entry          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_transmit_release           Release ARP queued packet     */
/*    (nx_ip_fragment_processing)           Fragment processing           */
/*    (ip_interface_link_driver_entry)      User supplied link driver     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_arp_static_entry_create           Create static entry           */
/*    _nx_arp_dynamic_entry_set             Set dynamic entry             */
/*    _nx_arp_packet_receive                Process the ARP packet        */
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
VOID  _nx_arp_queue_send(NX_IP *ip_ptr, NX_ARP *arp_ptr)
{

TX_INTERRUPT_SAVE_AREA
NX_PACKET   *queued_list_head;
NX_PACKET   *packet_ptr;
NX_IP_DRIVER driver_request;

    /* Initialize the queued list head to NULL.  */
    queued_list_head =  NX_NULL;

    /* Determine if this ARP entry has a packet queued up for sending.  */

    /* Disable interrupts before checking.  */
    TX_DISABLE

    /* Look at the ARP packet queue pointer.  */
    if (arp_ptr -> nx_arp_packets_waiting)
    {

        /* Pickup the packet pointer and clear the ARP queue pointer.  */
        queued_list_head =  arp_ptr -> nx_arp_packets_waiting;
        arp_ptr -> nx_arp_packets_waiting =  NX_NULL;
    }

    /* Restore previous interrupt posture.  */
    TX_RESTORE

    /* Are there any packets queued to send?  */
    while (queued_list_head)
    {

        /* Pickup the first entry on the list.  */
        packet_ptr =  queued_list_head;

        /* Move to the next entry on the ARP packet queue.  */
        queued_list_head =  queued_list_head -> nx_packet_queue_next;

        /* Clear the packet's queue next pointer.  */
        packet_ptr -> nx_packet_queue_next =  NX_NULL;

        packet_ptr -> nx_packet_address.nx_packet_interface_ptr = arp_ptr -> nx_arp_ip_interface;

        /* Build the driver request packet.  */
        driver_request.nx_ip_driver_physical_address_msw =  arp_ptr -> nx_arp_physical_address_msw;
        driver_request.nx_ip_driver_physical_address_lsw =  arp_ptr -> nx_arp_physical_address_lsw;
        driver_request.nx_ip_driver_ptr                  =  ip_ptr;
        driver_request.nx_ip_driver_command              =  NX_LINK_PACKET_SEND;
        driver_request.nx_ip_driver_packet               =  packet_ptr;
        driver_request.nx_ip_driver_interface            =  packet_ptr -> nx_packet_address.nx_packet_interface_ptr;

        /* Determine if fragmentation is needed.  */
        if (packet_ptr -> nx_packet_length > packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_ip_mtu_size)
        {

#ifndef NX_DISABLE_FRAGMENTATION
            /* Fragmentation is needed, call the fragment routine if available. */
            if (ip_ptr -> nx_ip_fragment_processing)
            {

                /* Call the IP fragment processing routine.  */
                (ip_ptr -> nx_ip_fragment_processing)(&driver_request);
            }
            else
            {
#endif /* NX_DISABLE_FRAGMENTATION */

#ifndef NX_DISABLE_IP_INFO

                /* Increment the IP send packets dropped count.  */
                ip_ptr -> nx_ip_send_packets_dropped++;
#endif

                /* Just release the packet.  */
                _nx_packet_transmit_release(packet_ptr);
#ifndef NX_DISABLE_FRAGMENTATION
            }
#endif /* NX_DISABLE_FRAGMENTATION */
        }
        else
        {

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP packet sent count.  */
            ip_ptr -> nx_ip_total_packets_sent++;

            /* Increment the IP bytes sent count.  */
            ip_ptr -> nx_ip_total_bytes_sent +=  packet_ptr -> nx_packet_length - (ULONG)sizeof(NX_IPV4_HEADER);
#endif

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IO_DRIVER_PACKET_SEND, ip_ptr, packet_ptr, packet_ptr -> nx_packet_length, 0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

            /* Send the queued IP packet out on the network via the attached driver.  */
            (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_link_driver_entry)(&driver_request);
        }
    }
}
#endif /* !NX_DISABLE_IPV4  */

