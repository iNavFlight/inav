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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_arp_entry_delete                                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches for the specified IP address in the ARP      */
/*    lists.  If found, the associated ARP entry is deleted.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    ip_address                            IP Address to search for      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _nx_arp_static_entry_delete           Remove static ARP entry       */
/*    _nx_arp_dynamic_entry_delete          Remove dynamic ARP entry      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT  _nx_arp_entry_delete(NX_IP *ip_ptr, ULONG ip_address)
{

#ifndef NX_DISABLE_IPV4
UINT    status;
NX_ARP *arp_ptr;
NX_ARP *arp_list_head;
NX_ARP *search_ptr;
UINT    index;


    /* Obtain protection on this IP instance for access into the ARP static  list.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Search the static list for a matching IP and hardware mapping.
       Compute the index that will tell us which linked list of ARP addresses to search,
       since we don't care if it is a static or dynamic entry.
       Search that linked list pointed in the IP ARP table.  */

    /* Calculate the hash index for the specified IP address.  */
    index =  (UINT)((ip_address + (ip_address >> 8)) & NX_ARP_TABLE_MASK);

    /* Pickup the head pointer of the ARP entries for this IP instance.  */
    arp_list_head =  ip_ptr -> nx_ip_arp_table[index];

    /* Search the ARP list for the same IP address.  */
    search_ptr =  arp_list_head;
    arp_ptr =     NX_NULL;

    while (search_ptr)
    {

        /* Determine if we have a match.  */
        if (search_ptr -> nx_arp_ip_address == ip_address)
        {

            /* Yes, the IP address matches, setup the ARP entry pointer.  */
            arp_ptr =  search_ptr;

            /* Get out of the loop.  */
            break;
        }

        /* Move to the next entry in the active list.  */
        search_ptr =  search_ptr -> nx_arp_active_next;

        /* Determine if the search pointer is back at the head of
           the list.  */
        if (search_ptr == arp_list_head)
        {

            /* End of the ARP list, end the search.  */
            break;
        }
    }

    /* Determine if we didn't find an ARP entry matching the input IP address.   */
    if (arp_ptr == NX_NULL)
    {

        /* Release the protection on the ARP list.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return status to the caller.  */
        return(NX_ENTRY_NOT_FOUND);
    }

    /* Determine if this is a static entry. */
    if (arp_ptr -> nx_arp_route_static == NX_TRUE)
    {

        /* Remove from the static list. */
        status = _nx_arp_static_entry_delete(ip_ptr, ip_address,
                                             arp_ptr -> nx_arp_physical_address_msw,
                                             arp_ptr -> nx_arp_physical_address_lsw);
    }
    else
    {

        /* Remove from the dynamic list. */
        status = _nx_arp_dynamic_entry_delete(ip_ptr, arp_ptr);
    }

    /* Release the protection on the ARP list.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return status to the caller.  */
    return(status);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(ip_address);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

