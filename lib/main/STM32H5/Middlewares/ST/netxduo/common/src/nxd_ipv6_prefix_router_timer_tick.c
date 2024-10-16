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
/*    _nxd_ipv6_prefix_router_timer_tick                   PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    At every time tick, this function checks for time out expiration on */
/*    both the default router list and the prefix list tables bound to the*/
/*    supplied IP instance.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_invalidate_destination_entry    Invalidate the entry in the     */
/*                                           destination                  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_thread_entry                                                 */
/*                                                                        */
/*  NOTE                                                                  */
/*                                                                        */
/*    The nx_ip_protection mutex must be obtained before calling this     */
/*    function.                                                           */
/*                                                                        */
/*    This function cannot be called from ISR.                            */
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
VOID _nxd_ipv6_prefix_router_timer_tick(NX_IP *ip_ptr)
{

UINT                          i, table_size;
NX_IPV6_PREFIX_ENTRY         *tmp, *prefix_entry;
NX_IPV6_DEFAULT_ROUTER_ENTRY *rt_entry;


    /* Set a local variable for convenience. */
    table_size = ip_ptr -> nx_ipv6_default_router_table_size;

    /* Check each entry in the default router table. */
    for (i = 0; table_size && (i < NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE); i++)
    {

        /* Set a local variable for convenience. */
        rt_entry = &ip_ptr -> nx_ipv6_default_router_table[i];

        /* Skip invalid or empty slots. */
        if ((rt_entry -> nx_ipv6_default_router_entry_flag & NX_IPV6_ROUTE_TYPE_VALID) == 0)
        {
            continue;
        }

        /* Keep track of valid entries we've checked. */
        table_size--;

        /* Has the entry on the current table entry expired? */
        if (rt_entry -> nx_ipv6_default_router_entry_life_time == 0)
        {

            /* Yes, the router has timed out. */
            /* Does this router have an entry in the ND cache table? */
            if (rt_entry -> nx_ipv6_default_router_entry_neighbor_cache_ptr)
            {

                /* Yes, clear out that entry, we are invalidating this entry. */
                rt_entry -> nx_ipv6_default_router_entry_neighbor_cache_ptr -> nx_nd_cache_is_router = NX_NULL;
            }

            /* Clean any entries in the destination table for this router.  */
            _nx_invalidate_destination_entry(ip_ptr, rt_entry -> nx_ipv6_default_router_entry_router_address);

            /* Invalidate the entry in the router table. */
            rt_entry -> nx_ipv6_default_router_entry_flag = 0;

            /* Clear the interface pointer .*/
            rt_entry -> nx_ipv6_default_router_entry_interface_ptr = NX_NULL;

            /* Decrease the IP instance default router count. */
            ip_ptr -> nx_ipv6_default_router_table_size--;
        }
        else
        {
            /* Is this a static router (infinite timeout)? */
            if (rt_entry -> nx_ipv6_default_router_entry_life_time != 0xFFFF)
            {

                /* No, so decrement the lifetime by one tick.*/
                rt_entry -> nx_ipv6_default_router_entry_life_time--;
            }
        }
    }

    /* Set a pointer to the first prefix entry in the IP prefix list. */
    prefix_entry = ip_ptr -> nx_ipv6_prefix_list_ptr;

    /* Loop through the entire list. */
    while (prefix_entry)
    {

        /* Set a placemarker at the current prefix. */
        tmp = prefix_entry;

        /* Get a pointer to the next prefix. */
        prefix_entry = prefix_entry -> nx_ipv6_prefix_entry_next;

        /* Skip the static entries. */
        if (tmp -> nx_ipv6_prefix_entry_valid_lifetime != (UINT)0xFFFFFFFF)
        {

            /* Has the prefix entry timeout expired? */
            if (tmp -> nx_ipv6_prefix_entry_valid_lifetime == 0)
            {

                /* Yes, so delete it from the list. */
                _nx_ipv6_prefix_list_delete_entry(ip_ptr, tmp);
            }
            else
            {

                /* Just decrement the time remaining. */
                tmp -> nx_ipv6_prefix_entry_valid_lifetime--;
            }
        }
    }

    return;
}

#endif /* FEATURE_NX_IPV6 */

