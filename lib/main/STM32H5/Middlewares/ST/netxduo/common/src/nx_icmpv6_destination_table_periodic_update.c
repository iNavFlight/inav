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
/**   Internet Control Message Protocol (ICMP)                            */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */
#include "nx_api.h"
#include "nx_ipv6.h"
#include "nx_icmpv6.h"

#ifdef FEATURE_NX_IPV6


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*   _nx_icmpv6_destination_table_periodic_update         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called from the IP thread task, and updates the    */
/*    MTU fields in the destination table, including handling MTU timer   */
/*    expiration and initiating a new path MTU probe.                     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                           Search successfully completed  */
/*                                          regardless if match found or  */
/*                                          empty slot found for new entry*/
/*    NX_NOT_SUCCESSFUL                    Invalid input, search aborted  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ipv6_perform_min_path_MTU_discovery                             */
/*                                         Service to query the existing  */
/*                                           minimum path MTU for changes */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_thread_entry                  IP thread background task      */
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

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY

VOID _nx_icmpv6_destination_table_periodic_update(NX_IP *ip_ptr)
{

UINT i, table_size;

    /* Set a local variable for convenience. */
    table_size = ip_ptr -> nx_ipv6_destination_table_size;

    for (i = 0; table_size && (i < NX_IPV6_DESTINATION_TABLE_SIZE); i++)
    {

        /* Ignore entries whose path MTU is already at the host MTU. */
        if (ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_path_mtu &&
            ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_nd_entry &&
            (ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_path_mtu ==
             ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_nd_entry -> nx_nd_cache_interface_ptr -> nx_interface_ip_mtu_size))
        {
            continue;
        }

        /* Check for valid entries with a non infinite path MTU timeout. */
        if (ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_valid &&
            (ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_MTU_timer_tick != NX_WAIT_FOREVER))
        {

            /* Keep track of valid entries we have checked. */
            table_size--;

            /* Decrement the timer on table entry. */
            if (ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_MTU_timer_tick <= NX_IP_PERIODIC_RATE)
            {

                /* Reset the path MTU to the IP instance MTU. */
                ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_path_mtu =
                    ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_nd_entry -> nx_nd_cache_interface_ptr -> nx_interface_ip_mtu_size;

                /* This is our optimal path MTU. So there is no need to age this table entry
                   and attempt to increase the path MTU; ok set it to infinity. */
                ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_MTU_timer_tick = NX_WAIT_FOREVER;
            }
            else
            {

                ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_MTU_timer_tick -= (ULONG)NX_IP_PERIODIC_RATE;
            }
        }
    }

    return;
}
#endif  /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */

#endif  /* FEATURE_NX_IPV6 */

