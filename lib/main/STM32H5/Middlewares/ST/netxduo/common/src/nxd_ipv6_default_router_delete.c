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
#ifdef FEATURE_NX_IPV6
#include "nx_nd_cache.h"
#include "nx_icmpv6.h"
#endif /* FEATURE_NX_IPV6 */



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_default_router_delete                      PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes an IPv6 routing table entry.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    router_addr                           Pointer to router  address to */
/*                                            delete                      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
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
UINT _nxd_ipv6_default_router_delete(NX_IP *ip_ptr, NXD_ADDRESS *router_address)
{
#ifdef FEATURE_NX_IPV6

INT                           i;
NX_IPV6_DEFAULT_ROUTER_ENTRY *rt_entry;


    NX_TRACE_IN_LINE_INSERT(NXD_TRACE_IPV6_DEFAULT_ROUTER_DELETE,
                            ip_ptr, router_address -> nxd_ip_address.v6[3], 0, 0, NX_TRACE_IP_EVENTS, 0, 0);

    /* Obtain protection on this IP instance for access into the default router table. */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* If our default route table is empty, just return */
    if (ip_ptr -> nx_ipv6_default_router_table_size == 0)
    {

        /* Release the mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        return(NX_SUCCESS);
    }

    /* Search the entire table for a matching entry. */
    for (i = 0; i < NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE; i++)
    {

        /*Set local pointer for convenience. */
        rt_entry = &ip_ptr -> nx_ipv6_default_router_table[i];

        /* Does this slot contain a router? */
        if (rt_entry -> nx_ipv6_default_router_entry_flag & NX_IPV6_ROUTE_TYPE_VALID)
        {

            /* Does it match the router address to search for? */
            if (CHECK_IPV6_ADDRESSES_SAME(router_address -> nxd_ip_address.v6,
                                          rt_entry -> nx_ipv6_default_router_entry_router_address))
            {

                /* Yes, does it have a pointer into the cache table? */
                if (rt_entry -> nx_ipv6_default_router_entry_neighbor_cache_ptr)
                {

                    /* Clear the router status. This will enable the entry
                       to time out eventually. */
                    rt_entry -> nx_ipv6_default_router_entry_neighbor_cache_ptr -> nx_nd_cache_is_router = NX_NULL;
                }

                /* Clean any entries in the destination table for this router.  */
                _nx_invalidate_destination_entry(ip_ptr, rt_entry -> nx_ipv6_default_router_entry_router_address);

                /* Mark the entry as empty. */
                rt_entry -> nx_ipv6_default_router_entry_flag = 0;

                /* Clear the interface pointer .*/
                rt_entry -> nx_ipv6_default_router_entry_interface_ptr = NX_NULL;

                /* Decrease the count of available routers. */
                ip_ptr -> nx_ipv6_default_router_table_size--;

                /* Release the mutex.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                return(NX_SUCCESS);
            }
        }
    }

    /* Release the mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    return(NX_NOT_FOUND);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(router_address);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

