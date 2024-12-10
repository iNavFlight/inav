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
/*    _nx_arp_entry_allocate                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function allocates an ARP entry for a specific new IP          */
/*    destination.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    arp_list_ptr                          List head of where to place   */
/*                                            the newly allocated ARP     */
/*                                            entry                       */
/*    is_static                             Entry attribute static/dynamic*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_arp_dynamic_entry_delete          Delete the dynamic entry      */
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
UINT  _nx_arp_entry_allocate(NX_IP *ip_ptr, NX_ARP **arp_list_ptr, UINT is_static)
{

TX_INTERRUPT_SAVE_AREA
NX_ARP *arp_entry;
UINT    status;


    /* Determine if there is an ARP entry available in the dynamic list.  */
    if (ip_ptr -> nx_ip_arp_dynamic_list)
    {

        /* Yes there are one or more free entries.  */

        /* Pickup pointer to last used dynamic ARP entry.  */
        arp_entry =  (ip_ptr -> nx_ip_arp_dynamic_list) -> nx_arp_pool_previous;

        /* Remove from the dynamic list. */
        _nx_arp_dynamic_entry_delete(ip_ptr, arp_entry);

        /* Disable interrupts temporarily.  */
        TX_DISABLE

        /* Link the ARP entry at the head of the IP list.  */

        /* Determine if the ARP entry is being added to an empty list.  */
        if (*arp_list_ptr)
        {

            /* Add the ARP entry to the beginning of the nonempty ARP
               list.  */
            arp_entry -> nx_arp_active_list_head =  arp_list_ptr;
            arp_entry -> nx_arp_active_next =      *arp_list_ptr;
            arp_entry -> nx_arp_active_previous =  (*arp_list_ptr) -> nx_arp_active_previous;
            (arp_entry -> nx_arp_active_previous) -> nx_arp_active_next =  arp_entry;
            (*arp_list_ptr) -> nx_arp_active_previous =  arp_entry;
        }
        else
        {
            /* Empty list, just put the ARP entry at the beginning.  */
            arp_entry -> nx_arp_active_list_head =  arp_list_ptr;
            arp_entry -> nx_arp_active_next =       arp_entry;
            arp_entry -> nx_arp_active_previous =   arp_entry;

            /* Now setup the list head.  */
            *arp_list_ptr =  arp_entry;
        }

        /* Determine if this is a static entry. */
        if (is_static == NX_TRUE)
        {

            /* Remove this entry from the ARP dynamic list.  */

            /* Determine if this is the only ARP entry on the dynamic list.  */
            if (arp_entry == arp_entry -> nx_arp_pool_next)
            {

                /* Remove the sole entry from the dynamic list head.  */
                ip_ptr -> nx_ip_arp_dynamic_list =  NX_NULL;
            }
            else
            {

                /* Remove the entry from a list of more than one entry.  */

                /* Update the links of the adjacent ARP dynamic pool entries.  */
                (arp_entry -> nx_arp_pool_next) -> nx_arp_pool_previous = arp_entry -> nx_arp_pool_previous;
                (arp_entry -> nx_arp_pool_previous) -> nx_arp_pool_next = arp_entry -> nx_arp_pool_next;
            }

            /* Add the entry to the ARP static list.  */

            /* Determine if the ARP static list is empty.  */
            if (ip_ptr -> nx_ip_arp_static_list == NX_NULL)
            {

                /* Just place this single ARP entry on the list.  */
                arp_entry -> nx_arp_pool_next =     arp_entry;
                arp_entry -> nx_arp_pool_previous = arp_entry;
                ip_ptr -> nx_ip_arp_static_list =   arp_entry;
            }
            else
            {

                /* Add to the end of the ARP static list.  */
                arp_entry -> nx_arp_pool_next = ip_ptr -> nx_ip_arp_static_list;
                arp_entry -> nx_arp_pool_previous = (ip_ptr -> nx_ip_arp_static_list) -> nx_arp_pool_previous;
                ((ip_ptr -> nx_ip_arp_static_list) -> nx_arp_pool_previous) -> nx_arp_pool_next = arp_entry;
                (ip_ptr -> nx_ip_arp_static_list) -> nx_arp_pool_previous = arp_entry;
            }

#ifndef NX_DISABLE_ARP_INFO
            /* Increment the ARP static entry count.  */
            ip_ptr -> nx_ip_arp_static_entries++;
#endif
        }
        else  /* Allocate entry from dynamic list. */
        {

            /* Move this ARP entry to the front of the general ARP dynamic entry pool.  */
            if (arp_entry != ip_ptr -> nx_ip_arp_dynamic_list)
            {

                /* The current ARP entry is not at the front of the list, so it
                   must be moved.  */

                /* Link up the neighbors first.  */
                (arp_entry -> nx_arp_pool_next) -> nx_arp_pool_previous = arp_entry -> nx_arp_pool_previous;
                (arp_entry -> nx_arp_pool_previous) -> nx_arp_pool_next = arp_entry -> nx_arp_pool_next;

                /* Now link this ARP entry to the head of the list.  */
                arp_entry -> nx_arp_pool_next =  ip_ptr -> nx_ip_arp_dynamic_list;
                arp_entry -> nx_arp_pool_previous =    (arp_entry -> nx_arp_pool_next) -> nx_arp_pool_previous;
                (arp_entry -> nx_arp_pool_previous) -> nx_arp_pool_next =  arp_entry;
                (arp_entry -> nx_arp_pool_next) -> nx_arp_pool_previous =  arp_entry;

                /* Now set the list head to this ARP entry.  */
                ip_ptr -> nx_ip_arp_dynamic_list =  arp_entry;
            }

            /* Increment the number of active dynamic entries.  */
            ip_ptr -> nx_ip_arp_dynamic_active_count++;
        }

        /* Set the entry type.  */
        arp_entry -> nx_arp_route_static = is_static;

        /* Restore interrupts.  */
        TX_RESTORE

        /* Setup a successful status return.  */
        status =  NX_SUCCESS;
    }
    else
    {

        /* No more ARP entries are available, all the ARP entries must be
           allocated on the static list.  */
        status =  NX_NO_MORE_ENTRIES;
    }

    /* Return status to the caller.  */
    return(status);
}
#endif /* !NX_DISABLE_IPV4  */

