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
/*    _nx_arp_periodic_update                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes ARP periodic update requests by walking     */
/*    through the dynamic ARP list to see if another ARP request needs to */
/*    be sent.                                                            */
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
/*    _nx_arp_packet_send                   Send periodic ARP request out */
/*    _nx_packet_transmit_release           Release queued packet         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_thread_entry                   IP helper thread              */
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
VOID  _nx_arp_periodic_update(NX_IP *ip_ptr)
{

TX_INTERRUPT_SAVE_AREA

ULONG      i;
NX_ARP    *arp_entry;
NX_PACKET *packet_ptr;
NX_PACKET *next_packet_ptr;


    /* Pickup pointer to ARP dynamic list.  */
    arp_entry =  ip_ptr -> nx_ip_arp_dynamic_list;

    /* Loop through the active ARP entries to see if they need updating.  */
    for (i = 0; i < ip_ptr -> nx_ip_arp_dynamic_active_count; i++)
    {

        /* Check this ARP entry to see if it need updating.  */
        if (arp_entry -> nx_arp_entry_next_update)
        {

            /* Decrement the next update field.  */
            arp_entry -> nx_arp_entry_next_update--;

            /* Determine if an ARP expiration is present.  */
            if (!arp_entry -> nx_arp_entry_next_update)
            {

                /* Yes, an ARP expiration is present.   */

                /* Determine if the retry counter has been exceeded.  */
                if (arp_entry -> nx_arp_retries == NX_ARP_MAXIMUM_RETRIES)
                {

                    /* The number of retries has been exceeded. The entry is removed
                       from the active list and any queued packet is released.  */

                    /* Disable interrupts.  */
                    TX_DISABLE

                    /* This ARP entry has expired, remove it from the active ARP list.  Check to make
                       sure it is still active.  */
                    if (arp_entry -> nx_arp_active_list_head)
                    {

                        /* Determine if this is the only ARP entry on the list.  */
                        if (arp_entry == arp_entry -> nx_arp_active_next)
                        {

                            /* Remove the entry from the list.  */
                            *(arp_entry -> nx_arp_active_list_head) =  NX_NULL;
                        }
                        else
                        {

                            /* Remove the entry from a list of more than one entry.  */

                            /* Update the list head pointer.  */
                            if (*(arp_entry -> nx_arp_active_list_head) == arp_entry)
                            {
                                *(arp_entry -> nx_arp_active_list_head) =  arp_entry -> nx_arp_active_next;
                            }

                            /* Update the links of the adjacent ARP entries.  */
                            (arp_entry -> nx_arp_active_next) -> nx_arp_active_previous =
                                arp_entry -> nx_arp_active_previous;
                            (arp_entry -> nx_arp_active_previous) -> nx_arp_active_next =
                                arp_entry -> nx_arp_active_next;
                        }

                        /* Decrease the number of active ARP entries.  */
                        ip_ptr -> nx_ip_arp_dynamic_active_count--;

                        /* Clear the active head pointer.  */
                        arp_entry -> nx_arp_active_list_head =  NX_NULL;
                    }

                    /* Determine if this is the only ARP entry on the dynamic list.  */
                    if (arp_entry != arp_entry -> nx_arp_pool_next)
                    {

                        /* No. Place the ARP entry at the end of the dynamic ARP pool, which is where new
                           ARP requests are allocated from.  */

                        /* Remove the entry from a list of more than one entry.  */
                        /* Update the links of the adjacent ARP dynamic pool entries.  */
                        (arp_entry -> nx_arp_pool_next) -> nx_arp_pool_previous =
                            arp_entry -> nx_arp_pool_previous;
                        (arp_entry -> nx_arp_pool_previous) -> nx_arp_pool_next =
                            arp_entry -> nx_arp_pool_next;

                        /* Update the list head pointer.  */
                        if (ip_ptr -> nx_ip_arp_dynamic_list == arp_entry)
                        {
                            ip_ptr -> nx_ip_arp_dynamic_list =  arp_entry -> nx_arp_pool_next;
                        }


                        /* Add ARP entry to the end of the list.  */
                        arp_entry -> nx_arp_pool_next =
                            ip_ptr -> nx_ip_arp_dynamic_list;
                        arp_entry -> nx_arp_pool_previous =
                            (ip_ptr -> nx_ip_arp_dynamic_list) -> nx_arp_pool_previous;
                        ((ip_ptr -> nx_ip_arp_dynamic_list) -> nx_arp_pool_previous) -> nx_arp_pool_next =
                            arp_entry;
                        (ip_ptr -> nx_ip_arp_dynamic_list) -> nx_arp_pool_previous =   arp_entry;
                    }

                    /* Pickup the queued packets head pointer.  */
                    next_packet_ptr =  arp_entry -> nx_arp_packets_waiting;

                    /* Clear the queued packets head pointer.  */
                    arp_entry -> nx_arp_packets_waiting =  NX_NULL;

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

                        /* Release the packet that was queued for the expired ARP entry.  */
                        _nx_packet_transmit_release(packet_ptr);
                    }
                }
                else
                {

                    /* We haven't yet had a response to this ARP request so send it again!  */

                    /* Increment the ARP retry counter.  */
                    arp_entry -> nx_arp_retries++;

                    /* Setup the ARP update rate to the maximum value again.  */
                    arp_entry -> nx_arp_entry_next_update =  NX_ARP_UPDATE_RATE;

                    /* Send the ARP request out.  */
                    _nx_arp_packet_send(ip_ptr, arp_entry -> nx_arp_ip_address, arp_entry -> nx_arp_ip_interface);
                }
            }
        }

        /* Move to the next ARP entry.  */
        arp_entry =  arp_entry -> nx_arp_pool_next;
    }


    /* Reduce the defend timeout of interfaces.  */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {
        if (ip_ptr -> nx_ip_interface[i].nx_interface_valid == NX_FALSE)
        {
            continue;
        }

        if (ip_ptr -> nx_ip_interface[i].nx_interface_arp_defend_timeout == 0)
        {
            continue;
        }

        ip_ptr -> nx_ip_interface[i].nx_interface_arp_defend_timeout--;
    }
}
#endif /* !NX_DISABLE_IPV4  */

