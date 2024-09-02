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
/*    nx_nd_cache_add                                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This internal function adds IPv6 and MAC mapping to the ND cache    */
/*    table.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                   Pointer to the IP instance.                */
/*    dest_ip                  Pointer to the IP address.                 */
/*    if_ptr                   Interface through which the neighbor can be*/
/*                               reached.                                 */
/*    mac                      Pointer to the MAC address to map to.      */
/*    IsStatic                 1: the entry is manully configured,  thus  */
/*                                    does not time out.                  */
/*                             0: The entry is dynamically configured.    */
/*    status                   Initial status                             */
/*    iface_address            Interface associated with this entry.      */
/*    nd_cache_entry           User specified storage space of pointer to */
/*                                the corresponding ND cache.             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                   NX_SUCCESS: An ND cache entry has been     */
/*                                added.  nd_cache_entry contains valid   */
/*                                value.                                  */
/*                             NX_NOT_SUCCESSFUL:  The ND cache entry     */
/*                                cannot be added, or nd_cache_entry is   */
/*                                NULL.                                   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _nx_nd_cache_add_entry                Obtain an empty entry         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_process_na                 Process NA message            */
/*    _nx_icmpv6_process_ns                 Process NS message            */
/*    _nx_icmpv6_process_ra                 Process RA message            */
/*    nx_icmpv6_process_redirect.c          Process Redirect message      */
/*    nxd_nd_cache_entry_set.c              User level service            */
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
UINT _nx_nd_cache_add(NX_IP *ip_ptr, ULONG *dest_ip, NX_INTERFACE *if_ptr, CHAR *mac, INT IsStatic,
                      INT status, NXD_IPV6_ADDRESS *iface_address,
                      ND_CACHE_ENTRY **nd_cache_entry)
{

USHORT         *copy_from, *copy_to;
ND_CACHE_ENTRY *entry;


    /* Initialize the return value. */
    *nd_cache_entry = NX_NULL;

    /* First find if there has a exit entry. */
    if (_nx_nd_cache_find_entry(ip_ptr, dest_ip, &entry) != NX_SUCCESS)
    {

        /* Did not find a valid entry. Add one. */
        if (_nx_nd_cache_add_entry(ip_ptr, dest_ip, iface_address, &entry) != NX_SUCCESS)
        {

            /* Can not add, return. */
            return(NX_NOT_SUCCESSFUL);
        }
    }

    /* At this point we know the entry is in the ND cache.
       Finish up updating the rest of the information. */

    /*lint -e{644} suppress variable might not be initialized, since "entry" was initialized in _nx_nd_cache_find_entry or _nx_nd_cache_add_entry. */
    entry -> nx_nd_cache_is_static = IsStatic ? (UCHAR)1 : (UCHAR)0;

    entry -> nx_nd_cache_interface_ptr = if_ptr;

    /* If the entry is already in reachable state, and the link layer address
       is the same, we should not update the status field. */
    /*lint -e{929} -e{740} suppress cast of pointer to pointer, since it is necessary  */
    copy_from = (USHORT *)mac;

    /*lint -e{927} suppress cast of pointer to pointer, since it is necessary  */
    copy_to = (USHORT *)(entry -> nx_nd_cache_mac_addr);

    /* Set the return value. */
    *nd_cache_entry = entry;

    if (entry -> nx_nd_cache_nd_status == ND_CACHE_STATE_REACHABLE)
    {
        /* Check mac address.*/
        if ((copy_from[0] == copy_to[0]) &&
            (copy_from[1] == copy_to[1]) &&
            (copy_from[2] == copy_to[2]))
        {

            /* The MAC address is the same.  So we are done. */

            return(NX_SUCCESS);
        }
    }

    /* Is this a static entry? */
    if (IsStatic)
    {

        /* Just set the status, no need to update the cache entry timeout. */
        entry -> nx_nd_cache_nd_status = (UCHAR)status;
    }
    /* Is the status changed? */
    else if (entry -> nx_nd_cache_nd_status != (UCHAR)status)
    {

        /* Update status in the cache entry. */
        entry -> nx_nd_cache_nd_status = (UCHAR)status;

        if (entry -> nx_nd_cache_nd_status == ND_CACHE_STATE_REACHABLE) /* New entry */
        {

            /* Set the timer tick.  The tick value only applies to the REACHABLE state. */
            entry -> nx_nd_cache_timer_tick = ip_ptr -> nx_ipv6_reachable_timer;
        }
    }

    /* Copy the MAC address. */
    *copy_to = *copy_from;
    copy_to++; copy_from++;
    *copy_to = *copy_from;
    copy_to++; copy_from++;
    *copy_to = *copy_from;

    return(NX_SUCCESS);
}

#endif /* FEATURE_NX_IPV6 */

