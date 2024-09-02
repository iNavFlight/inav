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
#include "nx_packet.h"

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_arp_dynamic_entry_delete                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes the ARP entry pointed to by the caller. Note  */
/*    the caller should already have searched the ARP list to verify a    */
/*    valid ARP entry to delete.  Also, it is assumed the caller already  */
/*    has obtained the IP protection mutex before invoking this service.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance of APR table      */
/*    arp_ptr                               ARP entry to delete           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_transmit_release           Release the transmitted packet*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Internal                                                            */
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
UINT  _nx_arp_dynamic_entry_delete(NX_IP *ip_ptr, NX_ARP *arp_ptr)
{

TX_INTERRUPT_SAVE_AREA
NX_PACKET *packet_ptr, *next_packet_ptr;


    /* Determine if this ARP entry is already active.  */
    if (arp_ptr -> nx_arp_active_list_head)
    {

        /* Remove this dynamic ARP entry from the associated list.  */

        /* Disable interrupts.  */
        TX_DISABLE

        /* Determine if this is the only ARP entry on the list.  */
        if (arp_ptr == arp_ptr -> nx_arp_active_next)
        {

            /* Remove the entry from the list.  */
            *(arp_ptr -> nx_arp_active_list_head) =  NX_NULL;
        }
        else
        {

            /* Remove the entry from a list of more than one entry.  */

            /* Update the list head pointer.  */
            if (*(arp_ptr -> nx_arp_active_list_head) == arp_ptr)
            {
                *(arp_ptr -> nx_arp_active_list_head) =  arp_ptr -> nx_arp_active_next;
            }

            /* Update the links of the adjacent ARP entries.  */
            (arp_ptr -> nx_arp_active_next) -> nx_arp_active_previous = arp_ptr -> nx_arp_active_previous;
            (arp_ptr -> nx_arp_active_previous) -> nx_arp_active_next =  arp_ptr -> nx_arp_active_next;
        }

        /* No longer active, clear the active list head.  */
        arp_ptr -> nx_arp_active_list_head =  NX_NULL;

        /* Decrease the number of active ARP entries.  */
        ip_ptr -> nx_ip_arp_dynamic_active_count--;

        /* Pickup the queued packets head pointer.  */
        next_packet_ptr =  arp_ptr -> nx_arp_packets_waiting;

        /* Clear the queued packets head pointer.  */
        arp_ptr -> nx_arp_packets_waiting =  NX_NULL;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Loop to remove all queued packets.  */
        while (next_packet_ptr)
        {

            /* Pickup the packet pointer at the head of the queue.  */
            packet_ptr =  next_packet_ptr;

            /* Move to the next packet in the queue.  */
            next_packet_ptr =  next_packet_ptr -> nx_packet_queue_next;

            /* Clear the next packet queue pointer.  */
            packet_ptr -> nx_packet_queue_next =  NX_NULL;

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP send packets dropped count.  */
            ip_ptr -> nx_ip_send_packets_dropped++;
#endif

            /* Release the packet that was queued from the previous ARP entry.  */
            _nx_packet_transmit_release(packet_ptr);
        }
    }

    return(NX_SUCCESS);
}
#endif /* !NX_DISABLE_IPV4  */

