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
/*    _nx_invalidate_destination_entry                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function invalidates all destination entries whose next hop    */
/*    address matches the supplied IP address regardless of the           */
/*    destination address.                                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP                 */
/*    next_hop_ip                           The next hop address to find  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nd_cache_delete                      Delete a neighbor cache.      */
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
VOID _nx_invalidate_destination_entry(NX_IP *ip_ptr, ULONG *next_hop_ip)
{

UINT i, table_size;

    /* Set a local variable for convenience. */
    table_size = ip_ptr -> nx_ipv6_destination_table_size;

    /* Check if there have been any destinations in the table. */
    if (table_size == 0)
    {
        return;
    }

    /* Loop through the whole table to match the IP address. */
    for (i = 0; table_size && (i < NX_IPV6_DESTINATION_TABLE_SIZE); i++)
    {

        /* Skip over empty slots. */
        if (!ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_valid)
        {
            continue;
        }

        /* Keep track of valid entries we have checked. */
        table_size--;

        /* Match the supplied next hop with the table entry next hop. */
        if (CHECK_IPV6_ADDRESSES_SAME(ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_next_hop, next_hop_ip))
        {

            /* A matching entry is found.  Mark the entry as invalid. */
            ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_valid = 0;

            /* Decrease the count of available destinations. */
            ip_ptr -> nx_ipv6_destination_table_size--;
        }
    }

    return;
}

#endif  /* FEATURE_NX_IPV6 */

