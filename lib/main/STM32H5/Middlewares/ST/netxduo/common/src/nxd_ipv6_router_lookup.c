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
/**   Internet Protocol version 6 Default Router Table (IPv6 router)      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/
#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ipv6.h"
#include "nx_nd_cache.h"
#include "nx_icmpv6.h"


#ifdef FEATURE_NX_IPV6
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_router_lookup                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds a valid router from the default router table,   */
/*    and this function also return the pointer to the router's cache     */
/*    entry in the NetX Duo cache table.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    if_ptr                                Pointer to interface          */
/*    router_address                        Pointer to default router     */
/*    nd_cache_entry                        Pointer to associated ND entry*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Non zero status               */
/*                                            router not found            */
/*                                          Zero status, router found     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_process_redirect                                         */
/*    _nx_ipv6_packet_send                                                */
/*                                                                        */
/*  NOTE                                                                  */
/*                                                                        */
/*    Internal Function.  Caller must have acquired IP protection mutex.  */
/*    This function should not be called from ISR.  Although it has no    */
/*    blocking calls it will slow down response time.                     */
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
UINT  _nxd_ipv6_router_lookup(NX_IP *ip_ptr, NX_INTERFACE *if_ptr, ULONG *router_address, void **nd_cache_entry)
{

UINT                          i;
UINT                          table_size;
UINT                          routers_checked;
NX_IPV6_DEFAULT_ROUTER_ENTRY *rt_entry;
ND_CACHE_ENTRY               *NDCacheEntry;

    NX_ASSERT(nd_cache_entry != NX_NULL)

    /* Initialize cache pointer to NULL (if no router found). */
    *nd_cache_entry = NULL;

    /* Set a local variable for convenience. */
    table_size = ip_ptr -> nx_ipv6_default_router_table_size;

    /* Check if there have been any routers added to the table. */
    if (table_size == 0)
    {

        /* Return a non zero (e.g. unsuccessful) error status. */
        return(NX_NOT_SUCCESSFUL);
    }

    /* Loop to check the router table.  */
    for (i = 0; table_size && (i < NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE); i++)
    {

        /* Local pointer for convenience. */
        rt_entry = &(ip_ptr -> nx_ipv6_default_router_table[i]);

        /* Does this slot contain a valid router? */
        if ((rt_entry -> nx_ipv6_default_router_entry_flag & NX_IPV6_ROUTE_TYPE_VALID) &&
            (rt_entry -> nx_ipv6_default_router_entry_interface_ptr == if_ptr))
        {

            /* Keep track of valid entries we have checked. */
            NDCacheEntry = rt_entry -> nx_ipv6_default_router_entry_neighbor_cache_ptr;

            /* Is this router reachable? */
            if (!NDCacheEntry ||
                (NDCacheEntry -> nx_nd_cache_nd_status < ND_CACHE_STATE_REACHABLE) ||
                (NDCacheEntry -> nx_nd_cache_nd_status > ND_CACHE_STATE_PROBE))
            {

                /* No, skip over. */
                table_size--;
                continue;
            }

            /* Yes, copy this router address into the return pointer. */
            COPY_IPV6_ADDRESS(ip_ptr -> nx_ipv6_default_router_table[i].nx_ipv6_default_router_entry_router_address, router_address);

            /* Copy the router's cache entry pointer to the supplied cache table pointer. */
            *nd_cache_entry = ip_ptr -> nx_ipv6_default_router_table[i].nx_ipv6_default_router_entry_neighbor_cache_ptr;

            /* We're done. Break out of the search. */
            return(NX_SUCCESS);
        }
    }

    /* If we are here, we did not find a suitable default router. Do a search
       of routers previously reachable. */

    /* Start at the round robin index so we don't always choose the first
       less-than-reachable router in the table. */
    i = ip_ptr -> nx_ipv6_default_router_table_round_robin_index;

    /* Find a router with previously known reachability. */
    for (routers_checked = 0; routers_checked < NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE; routers_checked++)
    {

        /* Local pointer for convenience. */
        rt_entry = &(ip_ptr -> nx_ipv6_default_router_table[i]);

        /* Does this slot contain a valid router? */
        if ((rt_entry -> nx_ipv6_default_router_entry_flag & NX_IPV6_ROUTE_TYPE_VALID) &&
            (rt_entry -> nx_ipv6_default_router_entry_interface_ptr == if_ptr))
        {

            /* Yes, copy this router to the return pointer. */
            COPY_IPV6_ADDRESS(rt_entry -> nx_ipv6_default_router_entry_router_address, router_address);

            /* Copy the router's cache entry pointer to the supplied cache table pointer. */
            *nd_cache_entry = rt_entry -> nx_ipv6_default_router_entry_neighbor_cache_ptr;

            /* Update the index so the same router is not chosen again if there
               any other less-than-reachable routers we can choose, RFC 2461 6.3.6. */
            ip_ptr -> nx_ipv6_default_router_table_round_robin_index++;

            /* Do we need wrap the index? */
            if (ip_ptr -> nx_ipv6_default_router_table_round_robin_index == NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE)
            {

                /* Yes, start back at the first slot. */
                ip_ptr -> nx_ipv6_default_router_table_round_robin_index = 0;
            }

            /* We're done. Return successful outcome status. */
            return(NX_SUCCESS);
        }

        /* Are we past the end of the table? */
        if (i == NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE - 1)
        {
            /* Yes, wrap to the first slot.*/
            i = 0;
        }
        else
        {
            i++;
        }
    }

    /* Router not found. */
    return(NX_NOT_SUCCESSFUL);
}


#endif /* FEATURE_NX_IPV6 */

