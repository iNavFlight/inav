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

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_arp_static_entry_delete_internal                PORTABLE C      */
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
/*    ip_ptr                                IP instance owner of APR table*/
/*    arp_entry                             ARP entry to delete           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_arp_interface_entries_delete      Delete ARP entry associated   */
/*                                            with the specified interface*/
/*    _nx_arp_static_entry_delete           Delete static ARP entry       */
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
VOID _nx_arp_static_entry_delete_internal(NX_IP *ip_ptr, NX_ARP *arp_entry)
{

TX_INTERRUPT_SAVE_AREA


#ifndef NX_DISABLE_ARP_INFO
    /* Decrement the ARP static entry count.  */
    ip_ptr -> nx_ip_arp_static_entries--;
#endif

    /* Disable interrupts temporarily.  */
    TX_DISABLE

    /* Determine if this ARP entry is already active.  */
    if (arp_entry -> nx_arp_active_list_head)
    {

        /* Remove this dynamic ARP entry from the associated list.  */

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
    }

    /* Remove this entry from the static ARP list.  */

    /* Determine if this is the only ARP entry on the static list.  */
    if (arp_entry == arp_entry -> nx_arp_pool_next)
    {

        /* Remove the sole entry from the static list head.  */
        ip_ptr -> nx_ip_arp_static_list =  NX_NULL;
    }
    else
    {

        /* Remove the entry from a list of more than one entry.  */

        /* Update the links of the adjacent ARP dynamic pool entries.  */
        (arp_entry -> nx_arp_pool_next) -> nx_arp_pool_previous =
            arp_entry -> nx_arp_pool_previous;
        (arp_entry -> nx_arp_pool_previous) -> nx_arp_pool_next =
            arp_entry -> nx_arp_pool_next;

        /* Update the list head pointer.  */
        if (ip_ptr -> nx_ip_arp_static_list == arp_entry)
        {
            ip_ptr -> nx_ip_arp_static_list =  arp_entry -> nx_arp_pool_next;
        }
    }

    /* Clear the fields that indicate the ARP entry is a static entry and make sure
       it is viewed as inactive in preparation for returning it to the dynamic ARP
       pool.  */
    arp_entry -> nx_arp_route_static =      NX_FALSE;
    arp_entry -> nx_arp_active_list_head =  NX_NULL;

    /* Place the ARP entry at the end of the dynamic ARP pool, which is where new
       ARP requests are allocated from.  */

    /* Determine if the dynamic ARP pool is empty.  */
    if (ip_ptr -> nx_ip_arp_dynamic_list)
    {

        /* Dynamic list is not empty, add former static ARP entry to the end of the list.  */
        arp_entry -> nx_arp_pool_next = ip_ptr -> nx_ip_arp_dynamic_list;
        arp_entry -> nx_arp_pool_previous = (ip_ptr -> nx_ip_arp_dynamic_list) -> nx_arp_pool_previous;
        ((ip_ptr -> nx_ip_arp_dynamic_list) -> nx_arp_pool_previous) -> nx_arp_pool_next = arp_entry;
        (ip_ptr -> nx_ip_arp_dynamic_list) -> nx_arp_pool_previous = arp_entry;
    }
    else
    {

        /* Dynamic list was empty, just place it at the head of the dynamic list.  */
        ip_ptr -> nx_ip_arp_dynamic_list =  arp_entry;
        arp_entry -> nx_arp_pool_next =     arp_entry;
        arp_entry -> nx_arp_pool_previous = arp_entry;
    }

    /* Restore interrupts.  */
    TX_RESTORE
}
#endif /* !NX_DISABLE_IPV4  */

