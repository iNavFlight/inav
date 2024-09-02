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
#include "nx_packet.h"
#include "nx_icmpv6.h"

#ifdef FEATURE_NX_IPV6


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_nd_cache_delete_internal                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes an IPv6 and MAC mapping from the ND cache     */
/*    table.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP.                */
/*    dest_ip                               Pointer to the IP address.    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Address is deleted            */
/*    NX_ENTRY_NOT_FOUND                    Address not found in cache    */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_transmit_releas            Packet Release                */
/*    memset                                                              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    nxd_ipv6_disable                                                    */
/*    nx_nd_cache_fast_periodic_update                                    */
/*    nxd_nd_cache_entry_delete                                           */
/*                                                                        */
/*  Note:                                                                 */
/*                                                                        */
/*    This routine is an internal function.  Therefore it assumes the     */
/*       mutex is already locked.  Caller is responsible for accquiring   */
/*       and releasing the mutex before invoking this routine.            */
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
UINT _nx_nd_cache_delete_internal(NX_IP *ip_ptr, ND_CACHE_ENTRY *entry)
{

UINT       i = 0, table_size;
NX_PACKET *pkt, *next_pkt;

    /* Free up the queued packets. */
    pkt = entry -> nx_nd_cache_packet_waiting_head;

    /* Flush any packets enqueued waiting on neighbor reachability confirmation. */
    while (pkt)
    {

        next_pkt = pkt -> nx_packet_queue_next;
        _nx_packet_transmit_release(pkt);
        pkt = next_pkt;
    }
    entry -> nx_nd_cache_packet_waiting_queue_length = 0;

    /* Clear the pointers to the original start and end of the packet queue. */
    entry -> nx_nd_cache_packet_waiting_head = NX_NULL;
    entry -> nx_nd_cache_packet_waiting_tail = NX_NULL;

    /* Initialize the rest of the fields. */
    memset(entry -> nx_nd_cache_mac_addr, 0, 6);

    /* Clear the entry out.  */
    entry -> nx_nd_cache_nd_status = ND_CACHE_STATE_INVALID;
    entry -> nx_nd_cache_is_static = 0;

    /* Is there a corresponding link in the default router list? */
    if (entry -> nx_nd_cache_is_router)
    {

        /* Set its pointer to this entry in the cache table to NULL. */
        entry -> nx_nd_cache_is_router -> nx_ipv6_default_router_entry_neighbor_cache_ptr = NX_NULL;
    }

    /* And indicate that this cache entry is no longer a router. */
    entry -> nx_nd_cache_is_router = NX_NULL;

    /* Set a local variable for convenience. */
    table_size = ip_ptr -> nx_ipv6_destination_table_size;

    while (table_size && i < NX_IPV6_DESTINATION_TABLE_SIZE)
    {

        /* Skip invalid entries. */
        if (!ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_valid)
        {
            i++;
            continue;
        }

        /* Keep track of valid entries we have checked. */
        table_size--;

        /* Find the destination unit. */
        if (ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_nd_entry == entry)
        {

            /* Set the status. */
            ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_valid = 0;

            /* Set its pointer to this entry in the destination table to NULL. */
            ip_ptr -> nx_ipv6_destination_table[i].nx_ipv6_destination_entry_nd_entry = NX_NULL;

            /* Update the destination_table size. */
            ip_ptr -> nx_ipv6_destination_table_size--;
        }

        i++;
    }

    return(NX_SUCCESS);
}

#endif /* FEATURE_NX_IPV6 */

