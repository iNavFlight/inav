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
/**   Neighbor Discovery Cache                                            */
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
/*    _nx_nd_cache_add_entry                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function finds an entry in the ND cache that is       */
/*    mapped to the specified IPv6 address.  If the entry does not exist, */
/*    this function allocates an empty entry and add the IP address to it.*/
/*                                                                        */
/*  Note:                                                                 */
/*                                                                        */
/*    This routine acquires the nx_nd_cache_protection mutex.             */
/*       Application shall not hold this mutex before calling this        */
/*       function.                                                        */
/*                                                                        */
/*    If the table is full and NetX Duo is configured to purge older      */
/*    entries to make room for new entries, NetX Duo attempts to find the */
/*    the best candidate to remove (STALE or REACHABLE).  NetX Duo        */
/*    will not remove any cache entries in the INCOMPLETE, PROBE or DELAY */
/*    state since these are probably being processed e.g. neighborhood    */
/*    discovery by NetX Duo during this time.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                   Pointer to IP instance                     */
/*    dest_ip                  The IP address to match                    */
/*    iface_address            Pointer to the IPv6 address structure      */
/*    nd_cache_entry           User specified storage space for pointer to*/
/*                               the corresponding ND cache.              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS              ND cache entry found, contains valid value  */
/*    NX_NOT_SUCCESSFUL       ND cache entry not found or entry is invalid*/
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_process_redirect                                         */
/*    _nx_ipv6_packet_send                                                */
/*    _nx_nd_cache_add                                                    */
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

UINT _nx_nd_cache_add_entry(NX_IP *ip_ptr, ULONG *dest_ip,
                            NXD_IPV6_ADDRESS *iface_address,
                            ND_CACHE_ENTRY **nd_cache_entry)
{

UINT i;
UINT index;
UINT first_available;
#ifndef NX_DISABLE_IPV6_PURGE_UNUSED_CACHE_ENTRIES
UINT stale_timer_ticks;
UINT timer_ticks_left;
#endif

    NX_PARAMETER_NOT_USED(ip_ptr);

    /* Set the found slot past the end of the table. If a match or available
       slot found, this will have a lower value. */
    first_available = NX_IPV6_NEIGHBOR_CACHE_SIZE;

    /* Initialize the return value. */
    *nd_cache_entry = NX_NULL;

    /* Compute a simple hash based on the destination IP address. */
    index = (UINT)((dest_ip[0] + dest_ip[1] + dest_ip[2] + dest_ip[3]) %
                   (NX_IPV6_NEIGHBOR_CACHE_SIZE));

#ifndef NX_DISABLE_IPV6_PURGE_UNUSED_CACHE_ENTRIES

    /* Set the lowest possible timer ticks left to compare to. */
    stale_timer_ticks = 0;

    /* Start out at a very high number of remaining ticks to compare to. */
    timer_ticks_left = 0xFFFFFFFF;
#endif

    /* Loop through all the entries. */
    for (i = 0; i < NX_IPV6_NEIGHBOR_CACHE_SIZE; i++, index++)
    {

        /* Check for overflow */
        if (index == NX_IPV6_NEIGHBOR_CACHE_SIZE)
        {

            /* Start back at the first table entry. */
            index = 0;
        }

        /* Is the current entry available? */
        if (ip_ptr -> nx_ipv6_nd_cache[index].nx_nd_cache_nd_status == ND_CACHE_STATE_INVALID)
        {

            /* There is a chance the entry to add does not exist in the table. We create one using the
               invalid entry. */
            first_available = index;
            break;
        }

#ifndef NX_DISABLE_IPV6_PURGE_UNUSED_CACHE_ENTRIES
        /* Skip over routers and static entries. */
        if (ip_ptr -> nx_ipv6_nd_cache[index].nx_nd_cache_is_router != NX_NULL || ip_ptr -> nx_ipv6_nd_cache[index].nx_nd_cache_is_static)
        {
            continue;
        }

        /* Purging is enabled;
           Attempt to find a STALE entry and if there is more than one,
           choose the oldest one e.g. the highest timer ticks elapsed. */

        /* Check for stale entries. These are the best candidates for 'recycling.' */
        if (ip_ptr -> nx_ipv6_nd_cache[index].nx_nd_cache_nd_status == ND_CACHE_STATE_STALE)
        {

            /* Find the 'Stale' cache entry with the highest timer tick since
               timer tick is incremented in the Stale state.*/
            if (ip_ptr -> nx_ipv6_nd_cache[index].nx_nd_cache_timer_tick > stale_timer_ticks)
            {
                /* Set this entry as the oldest stale entry. */
                stale_timer_ticks = (UINT)ip_ptr -> nx_ipv6_nd_cache[index].nx_nd_cache_timer_tick;
                first_available = index;
            }
        }
        /* Next try finding a REACHABLE entry closest to its cache table expiration date. */
        else if (stale_timer_ticks == 0 &&
                 ip_ptr -> nx_ipv6_nd_cache[index].nx_nd_cache_nd_status == ND_CACHE_STATE_REACHABLE)
        {

            /* Is this entry older that our previous oldest entry? */
            if (ip_ptr -> nx_ipv6_nd_cache[index].nx_nd_cache_timer_tick < timer_ticks_left)
            {

                /* Set this entry as the oldest entry using timer ticks left. */
                timer_ticks_left = (UINT)ip_ptr -> nx_ipv6_nd_cache[index].nx_nd_cache_timer_tick;
                first_available = index;
            }
        }
#endif
    }

    /* Did not find a available entry. */
    if (first_available == NX_IPV6_NEIGHBOR_CACHE_SIZE)
    {

        /* Return unsuccessful status. */
        return(NX_NOT_SUCCESSFUL);
    }

    /* Yes; before we invalidate and delete the entry, we need to
       clean the nd cache. */
    _nx_nd_cache_delete_internal(ip_ptr, &ip_ptr -> nx_ipv6_nd_cache[first_available]);

    /* Record the IP address. */
    COPY_IPV6_ADDRESS(dest_ip, ip_ptr -> nx_ipv6_nd_cache[first_available].nx_nd_cache_dest_ip);

    /* A new entry starts with CREATED status. */
    ip_ptr -> nx_ipv6_nd_cache[first_available].nx_nd_cache_nd_status = ND_CACHE_STATE_CREATED;

    ip_ptr -> nx_ipv6_nd_cache[first_available].nx_nd_cache_outgoing_address = iface_address;

    ip_ptr -> nx_ipv6_nd_cache[first_available].nx_nd_cache_interface_ptr = iface_address -> nxd_ipv6_address_attached;

    /* Release the protection. */
    *nd_cache_entry = &ip_ptr -> nx_ipv6_nd_cache[first_available];

    return(NX_SUCCESS);
}

#endif /* FEATURE_NX_IPV6 */

