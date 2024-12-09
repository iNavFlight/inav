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
/*    _nxd_ipv6_default_router_entry_get                   PORTABLE C     */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function gets a router entry from the default IPv6 routing     */
/*    table.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    interface_index                       Index to the interface        */
/*    entry_index                           Entry Index                   */
/*    router_addr                           Router IPv6 Address           */
/*    router_lifetime                       Pointer to router life time   */
/*    prefix_length                         Pointer to prefix length      */
/*    configuration_method                  Pointer to the information    */
/*                                            on how the entry was        */
/*                                            configured                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application code                                                    */
/*    _nxd_ipv6_default_router_get                                        */
/*                                                                        */
/*  NOTE                                                                  */
/*                                                                        */
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
UINT  _nxd_ipv6_default_router_entry_get(NX_IP *ip_ptr, UINT interface_index, UINT entry_index,
                                         NXD_ADDRESS *router_addr, ULONG *router_lifetime,
                                         ULONG *prefix_length, ULONG *configuration_method)
{
#ifdef FEATURE_NX_IPV6

UINT                          status;
UINT                          i;
NX_IPV6_DEFAULT_ROUTER_ENTRY *rt_entry;


    /* Obtain protection on this IP instance for access into the router table. */

    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Initialize the return value to be NX_NOT_FOUND.  Once the router address is set,
       the return value is set to NX_SUCCESS. */
    status = NX_NOT_FOUND;

    for (i = 0; i < NX_IPV6_DEFAULT_ROUTER_TABLE_SIZE; i++)
    {

        rt_entry = &ip_ptr -> nx_ipv6_default_router_table[i];
        /* Skip invalid entries. */
        if (rt_entry -> nx_ipv6_default_router_entry_flag == 0)
        {
            continue;
        }

        /* Match the interface. */
        if (rt_entry -> nx_ipv6_default_router_entry_interface_ptr -> nx_interface_index != (UCHAR)interface_index)
        {
            continue;
        }

        /* Match the entry index */
        if (entry_index > 0)
        {
            entry_index--;
        }
        else
        {

            if (router_addr)
            {
                router_addr -> nxd_ip_version = NX_IP_VERSION_V6;

                COPY_IPV6_ADDRESS(&(rt_entry -> nx_ipv6_default_router_entry_router_address[0]),
                                  &router_addr -> nxd_ip_address.v6[0]);
            }


            if (router_lifetime)
            {
                *router_lifetime = rt_entry -> nx_ipv6_default_router_entry_life_time;
            }

            if (prefix_length)
            {
                if ((rt_entry -> nx_ipv6_default_router_entry_router_address[0] & (UINT)0xFFC00000) == (UINT)0xFE800000)
                {
                    *prefix_length = 10;
                }
                else
                {
                    *prefix_length = 64;
                }
            }

            if (configuration_method)
            {
                *configuration_method = (rt_entry -> nx_ipv6_default_router_entry_flag &
                                         (UCHAR)(~NX_IPV6_ROUTE_TYPE_VALID));
            }

            status = NX_SUCCESS;

            break;
        }
    }

    /* Release the mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    return(status);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(interface_index);
    NX_PARAMETER_NOT_USED(entry_index);
    NX_PARAMETER_NOT_USED(router_addr);
    NX_PARAMETER_NOT_USED(router_lifetime);
    NX_PARAMETER_NOT_USED(prefix_length);
    NX_PARAMETER_NOT_USED(configuration_method);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

