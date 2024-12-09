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
#endif /* FEATURE_NX_IPV6 */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_default_router_number_of_entries_get      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets router entries from the default IPv6 routing     */
/*    table.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    if_index                              Interface Index               */
/*    num_entries                           Destination for number of     */
/*                                            IPv6 routers on a specified */
/*                                            network interface           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
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
UINT  _nxd_ipv6_default_router_number_of_entries_get(NX_IP *ip_ptr, UINT if_index, UINT *num_entries)
{
#ifdef FEATURE_NX_IPV6

UINT                          entries;
UINT                          i;
NX_IPV6_DEFAULT_ROUTER_ENTRY *rt_entry;


    /* Obtain protection on this IP instance for access into the router table. */

    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Initialize the return value to be 0. */
    entries = 0;

    for (i = 0; i < NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE; i++)
    {

        rt_entry = &ip_ptr -> nx_ipv6_default_router_table[i];
        /* Skip invalid entries. */
        if (rt_entry -> nx_ipv6_default_router_entry_flag == 0)
        {
            continue;
        }

        /* Match the interface. */
        if (rt_entry -> nx_ipv6_default_router_entry_interface_ptr -> nx_interface_index != if_index)
        {
            continue;
        }

        /* Find router entriy. */
        entries++;
    }

    /* Release the mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    *num_entries = entries;

    return(NX_SUCCESS);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(if_index);
    NX_PARAMETER_NOT_USED(num_entries);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

