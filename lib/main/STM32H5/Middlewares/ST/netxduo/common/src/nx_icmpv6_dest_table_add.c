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
/*    _nx_icmpv6_dest_table_add                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches for a matching destination in the table entry*/
/*    and if no match is found, it creates one.  If a match is            */
/*    found, the next hop field is updated if the data is changed, and a  */
/*    pointer returned to that entry. If path MTU discovery is enabled,   */
/*    the path MTU and time out are set to supplied values.  If none are  */
/*    NetX Duo applies default values.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                               IP interface thread task       */
/*    destination_address                  Destination IP address.        */
/*    dest_entry_ptr                       Pointer to location of matching*/
/*                                           table entry, if any          */
/*    next_hop                             Pointer to next hop address    */
/*    path_mtu                             Optional path mtu update.      */
/*                                           Note: caller shall make sure */
/*                                           MTU does not exceed physical */
/*                                           link MTU size.               */
/*    mtu_timeout                          Optional entry timeout update  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                           Found matching entry           */
/*    NX_NOT_SUCCESSFUL                    Error searching table          */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                         Obtain destination table lock  */
/*    tx_mutex_put                         Release destination table lock */
/*    memset                               Clear memory block             */
/*    _nx_nd_cache_find_entry              Find next hop in the ND cache  */
/*    _nx_nd_cache_add_entry               Create an entry in ND cache    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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

UINT _nx_icmpv6_dest_table_add(NX_IP *ip_ptr, ULONG *destination_address,
                               NX_IPV6_DESTINATION_ENTRY **dest_entry_ptr, ULONG *next_hop,
                               ULONG path_mtu, ULONG mtu_timeout, NXD_IPV6_ADDRESS *ipv6_address)
{

UINT i, table_size;
UINT status;

    /* Pointers must not be NULL. */
    NX_ASSERT((destination_address != NX_NULL) && (dest_entry_ptr != NX_NULL) && (next_hop != NX_NULL));

    /* Check if destination table already exist. */
    status = _nx_icmpv6_dest_table_find(ip_ptr, destination_address, dest_entry_ptr, path_mtu, mtu_timeout);

    /* Check status.  */
    if (status == NX_SUCCESS)
    {

        /* Check if the next hop address is same.  */
        if (CHECK_IPV6_ADDRESSES_SAME(next_hop, (*dest_entry_ptr) -> nx_ipv6_destination_entry_next_hop))
        {

            /* Same next hop address. Return success.  */
            return(NX_SUCCESS);
        }
        else
        {

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY
            /* Get MTU from original destination table. */
            path_mtu = (*dest_entry_ptr) -> nx_ipv6_destination_entry_path_mtu;
            mtu_timeout = (*dest_entry_ptr) -> nx_ipv6_destination_entry_MTU_timer_tick;
#endif /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */

            /* Next hop is different. Delete this destination table and add new entry.  */
            (*dest_entry_ptr) -> nx_ipv6_destination_entry_valid = 0;

            /* Decrease the count of available destinations. */
            ip_ptr -> nx_ipv6_destination_table_size--;
        }
    }

    /* Set a local variable for convenience. */
    table_size = ip_ptr -> nx_ipv6_destination_table_size;

    /* There is no invalid destination in table. */
    if (table_size == NX_IPV6_DESTINATION_TABLE_SIZE)
    {
        return(NX_NOT_SUCCESSFUL);
    }

    /* Initialize the pointer to the table location where we will update/add information. */
    *dest_entry_ptr = NX_NULL;

    /* Go through the table to find an empty slot. */
    for (i = 0; i < NX_IPV6_DESTINATION_TABLE_SIZE; i++)
    {

        /* Is this slot empty? */
        if (!ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_valid)
        {
            /* Yes; we can use it for adding a new entry. */
            /* Have found an empty slot. */
            break;
        }
    }

    /* Destination is not empty so i must be less than table size. */
    NX_ASSERT(i < NX_IPV6_DESTINATION_TABLE_SIZE);

    /*
       If we are here, we did not find a match and are adding a new entry.
       The process is slightly different from updating a previously existing
       matching entry, so we handle it separately.
     */

    /* Clear out any previous data from this slot. */
    /*lint -e{669} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    memset(&ip_ptr -> nx_ipv6_destination_table[i], 0, sizeof(NX_IPV6_DESTINATION_ENTRY));

    /* Fill in the newly created table entry with the supplied and/or default information. */
    COPY_IPV6_ADDRESS(destination_address, ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_destination_address);

    /* Add next hop information to the entry. */
    COPY_IPV6_ADDRESS(next_hop, ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_next_hop);

    /* Attempt to find the matching entry in the cache table. NetX Duo will need to know
       how to get a packet to the next hop, not just the destination! */
    status = _nx_nd_cache_find_entry(ip_ptr, next_hop, &ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_nd_entry);

    /* Did not find the matching entry. Try to add one. */
    if (status)
    {
        status = _nx_nd_cache_add_entry(ip_ptr, next_hop, ipv6_address, &ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_nd_entry);

        /* Failed to add new entry. Return. */
        if (status)
        {
            return(NX_NOT_SUCCESSFUL);
        }
    }


    /* Validate this entry to ensure it will not be overwritten with new entries. */
    ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_valid = 1;

    /* Update the count of destinations currently in the table. */
    ip_ptr -> nx_ipv6_destination_table_size++;

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY

    /* Is a valid path mtu is given? */
    if (path_mtu > 0)
    {

        /* Update the destination path MTU with this supplied data. */
        ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_path_mtu = path_mtu;
    }
    else
    {

        /* No; set the path MTU to the IP task MTU (on link path MTU).*/
        ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_path_mtu = ipv6_address -> nxd_ipv6_address_attached -> nx_interface_ip_mtu_size;
    }

    /* Is a valid entry timeout is supplied? */
    if (mtu_timeout > 0)
    {

        /* Yes; Update the table entry timeout to the supplied data. */
        ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_MTU_timer_tick = mtu_timeout;
    }
    else
    {

        /* No; have we defaulted to our own IP task MTU?*/
        if (ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_path_mtu == ipv6_address -> nxd_ipv6_address_attached -> nx_interface_ip_mtu_size)
        {

            /* Yes; This is our optimal path MTU. So there is no need to age this table entry
               and attempt to increase the path MTU; ok set it to infinity. */
            ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_MTU_timer_tick = NX_WAIT_FOREVER;
        }
        else
        {
            /* No, this is less than our optimal path MTU. Wait the required interval
               before probing for a higher path MTU. */
            ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_MTU_timer_tick = NX_PATH_MTU_INCREASE_WAIT_INTERVAL_TICKS;
        }
    }
#else
    NX_PARAMETER_NOT_USED(path_mtu);
    NX_PARAMETER_NOT_USED(mtu_timeout);
#endif  /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */

    /* Set the table location pointer to the entry we just added/updated. */
    *dest_entry_ptr = &ip_ptr -> nx_ipv6_destination_table[i];

    return(NX_SUCCESS);
}

#endif  /* FEATURE_NX_IPV6 */

