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
/*    _nx_arp_static_entry_delete                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes a previously setup static IP to hardware      */
/*    mapping and returns the associated ARP entry back to the dynamic    */
/*    ARP pool.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    ip_address                            IP Address binding to delete  */
/*    physical_msw                          Physical address MSW          */
/*    physical_lsw                          Physical address LSW          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    _nx_arp_static_entry_delete_internal  Internal ARP entry delete     */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_arp_static_entries_delete         Delete all static entries     */
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
UINT  _nx_arp_static_entry_delete(NX_IP *ip_ptr, ULONG ip_address,
                                  ULONG physical_msw, ULONG physical_lsw)
{

#ifndef NX_DISABLE_IPV4
NX_ARP *arp_entry;
UINT    status;


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_ARP_STATIC_ENTRY_DELETE, ip_ptr, ip_address, physical_msw, physical_lsw, NX_TRACE_ARP_EVENTS, 0, 0);

    /* Obtain protection on this IP instance for access into the ARP static
       list.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Search the static list for a matching IP and hardware mapping.  */
    arp_entry =  ip_ptr -> nx_ip_arp_static_list;
    while (arp_entry)
    {

        /* Determine if we have a match.  */
        if ((arp_entry -> nx_arp_ip_address == ip_address) &&
            (arp_entry -> nx_arp_physical_address_msw ==  physical_msw) &&
            (arp_entry -> nx_arp_physical_address_lsw ==  physical_lsw))
        {

            /* Yes, we have found the ARP entry we are looking for.  */
            break;
        }
        else
        {

            /* Determine if we are at the end of the list.  */
            if (arp_entry -> nx_arp_pool_next == ip_ptr -> nx_ip_arp_static_list)
            {

                /* Set the arp_entry to NULL to signify nothing was found and get
                   out of the search loop.  */
                arp_entry =  NX_NULL;
                break;
            }
            else
            {

                /* Just move to the next ARP entry on the static list.  */
                arp_entry =  arp_entry -> nx_arp_pool_next;
            }
        }
    }

    /* At this point the ARP entry pointer determines whether or not anything was found
       on the static list.  */
    if (arp_entry)
    {

        /* The static entry was found.  It needs to be unlinked from the active
           list and the static list and re-linked to the end of the dynamic list.  */
        _nx_arp_static_entry_delete_internal(ip_ptr, arp_entry);

        /* Setup the return status.  */
        status =  NX_SUCCESS;
    }
    else
    {

        /* Indicate the entry was not found.  */
        status =  NX_ENTRY_NOT_FOUND;
    }

    /* Release the protection on the ARP list.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return status to the caller.  */
    return(status);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(physical_msw);
    NX_PARAMETER_NOT_USED(physical_lsw);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

