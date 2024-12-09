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
/*    _nx_arp_dynamic_entries_invalidate                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function invalidates all ARP dynamic entries currently in      */
/*    the ARP cache.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _nx_arp_dynamic_entry_delete          Delete dynamic ARP entry      */
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
UINT  _nx_arp_dynamic_entries_invalidate(NX_IP *ip_ptr)
{

#ifndef NX_DISABLE_IPV4
NX_ARP *arp_entry;


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_ARP_DYNAMIC_ENTRIES_INVALIDATE, ip_ptr, ip_ptr -> nx_ip_arp_dynamic_active_count, 0, 0, NX_TRACE_ARP_EVENTS, 0, 0);

    /* Obtain protection on this IP instance for access into the ARP dynamic
       list.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Setup pointers to the starting and ending ARP entries in the dynamic list.  */
    arp_entry = ip_ptr -> nx_ip_arp_dynamic_list;

    /* Walk through the dynamic ARP list until there are no more active entries.  */
    while ((arp_entry) && (ip_ptr -> nx_ip_arp_dynamic_active_count))
    {

        /* Remove from the dynamic list. */
        _nx_arp_dynamic_entry_delete(ip_ptr, arp_entry);

        /* Cleanup the nx_arp_ip_interface field. */
        arp_entry -> nx_arp_ip_interface = NX_NULL;

        /* Determine if we are at the end of the dynamic list.  */
        if (arp_entry -> nx_arp_pool_next != ip_ptr -> nx_ip_arp_dynamic_list)
        {

            /* No, simply move to the next dynamic entry.  */
            arp_entry =  arp_entry -> nx_arp_pool_next;
        }
        else
        {

            /* Yes, we are at the end of the dynamic list, break out of the loop.  */
            break;
        }
    }

    /* Release the mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return successful status to the caller.  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

