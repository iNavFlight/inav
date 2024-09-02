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

/* Include necessary system files. */

#include "nx_api.h"
#include "nx_arp.h"

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_arp_interface_entries_delete                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes ARP entries associated with the interface     */
/*    specified by index from the IP's arp table.                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    index                                 IP interface index            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    _nx_arp_dynamic_entry_delete          Delete dynamic ARP entry      */
/*    _nx_arp_static_entry_delete_internal  Internal static ARP entry     */
/*                                            delete function             */
/*    tx_mutex_put                          release protection mutex      */
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
UINT _nx_arp_interface_entries_delete(NX_IP *ip_ptr, UINT index)
{

NX_ARP       *arp_entry;
NX_ARP       *next_arp_entry;
NX_ARP       *last_arp_entry;
NX_INTERFACE *interface_ptr;

    interface_ptr = &(ip_ptr -> nx_ip_interface[index]);

    /* Obtain protection on this IP instance for access into the ARP dynamic list. */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Begain remove ARP entries from the dynamic list. */

    /* Setup pointers to the starting and ending ARP entries in the dynamic list. */
    arp_entry = ip_ptr -> nx_ip_arp_dynamic_list;

    /* Walk through the dynamic ARP list until there are no more active entries. */
    while ((arp_entry) && (ip_ptr -> nx_ip_arp_dynamic_active_count))
    {

        /* Yes, there is one or more dynamic entries. */
        /* Determine if this ARP entry is for the specified interface. */
        if (arp_entry -> nx_arp_ip_interface == interface_ptr)
        {

            _nx_arp_dynamic_entry_delete(ip_ptr, arp_entry);

            /* Cleanup the nx_arp_ip_interface filed. */
            arp_entry -> nx_arp_ip_interface = NX_NULL;
        }

        /* Determine if we are at the end of the dynamic list. */
        if (arp_entry -> nx_arp_pool_next != ip_ptr -> nx_ip_arp_dynamic_list)
        {
            /* No, simply move to the next dynamic entry. */
            arp_entry = arp_entry -> nx_arp_pool_next;
        }
        else
        {
            /* Yes, we are at the end of the dynamic list, break out of the loop. */
            break;
        }
    }

    /* End remove ARP entries from the dynamic list. */

    /* Begain remove ARP entries from the static list. */

    /* Setup pointers to the starting/ending ARP entry in the static list. */
    next_arp_entry = ip_ptr -> nx_ip_arp_static_list;
    if (next_arp_entry)
    {

        /* Pick up the last ARP entry. */
        last_arp_entry = next_arp_entry -> nx_arp_pool_previous;

        /* Walk through the static ARP list until there are no more active entries. */
        while (next_arp_entry)
        {

            /* Yes, there is one or more static ARP entries. */
            arp_entry = next_arp_entry;

            /* Move to the next pakcet in the queue. */
            next_arp_entry = next_arp_entry -> nx_arp_pool_next;

            if (arp_entry -> nx_arp_ip_interface == interface_ptr)
            {

                /* The static entry was found.  It needs to be unlinked from the active
                   list and the static list and re-linked to the end of the dynamic list.  */
                _nx_arp_static_entry_delete_internal(ip_ptr, arp_entry);
            }

            if (arp_entry == last_arp_entry)
            {
                break;
            }
        }
    }

    /* End remove ARP entries from the static list. */

    /* Release the mutex. */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return successful status to the caller. */
    return(NX_SUCCESS);
}
#endif /* !NX_DISABLE_IPV4  */

