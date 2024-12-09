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


#ifdef FEATURE_NX_IPV6

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_default_router_table_init                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes IPv6 routing table.  Note it is up to the */
/*    caller to obtain mutex protection before writing to the table.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
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
/*    nx_ipv6_enable                                                      */
/*                                                                        */
/*  NOTE                                                                  */
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
VOID _nxd_ipv6_default_router_table_init(NX_IP *ip_ptr)
{

ULONG i;

    /* Initialize each entry in the default router table to null. */
    for (i = 0; i < NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE; i++)
    {

        ip_ptr -> nx_ipv6_default_router_table[i].nx_ipv6_default_router_entry_flag = 0;
        ip_ptr -> nx_ipv6_default_router_table[i].nx_ipv6_default_router_entry_life_time = 0;
        ip_ptr -> nx_ipv6_default_router_table[i].nx_ipv6_default_router_entry_neighbor_cache_ptr = NX_NULL;
    }

    /* Set the initial size to zero. */
    ip_ptr -> nx_ipv6_default_router_table_size = 0;

    /* Initialize the index for recruiting less "reachable" routers
       when the current router cannot be reached. */
    ip_ptr -> nx_ipv6_default_router_table_round_robin_index = 0;

    /* Initialize the start of the prefix table (linked list) to NULL. */
    ip_ptr -> nx_ipv6_prefix_list_table[0].nx_ipv6_prefix_entry_prev = NX_NULL;
    ip_ptr -> nx_ipv6_prefix_list_table[0].nx_ipv6_prefix_entry_next = &ip_ptr -> nx_ipv6_prefix_list_table[1];

    /* Link up the entries in the prefix table.  */
    for (i = 1; i < NX_IPV6_PREFIX_LIST_TABLE_SIZE - 1; i++)
    {

        ip_ptr -> nx_ipv6_prefix_list_table[i].nx_ipv6_prefix_entry_prev = &ip_ptr -> nx_ipv6_prefix_list_table[i - 1];
        ip_ptr -> nx_ipv6_prefix_list_table[i].nx_ipv6_prefix_entry_next = &ip_ptr -> nx_ipv6_prefix_list_table[i + 1];
    }

    /* Null terminate the end of the prefix table (linked list). */
    ip_ptr -> nx_ipv6_prefix_list_table[i].nx_ipv6_prefix_entry_prev = &ip_ptr -> nx_ipv6_prefix_list_table[i - 1];
    ip_ptr -> nx_ipv6_prefix_list_table[i].nx_ipv6_prefix_entry_next = NX_NULL;

    /* Set the free list pointer to the 1st entry. */
    ip_ptr -> nx_ipv6_prefix_entry_free_list = &ip_ptr -> nx_ipv6_prefix_list_table[0];
    ip_ptr -> nx_ipv6_prefix_list_ptr = NX_NULL;

    /* All done, return. */
}


#endif /* FEATURE_NX_IPV6 */

