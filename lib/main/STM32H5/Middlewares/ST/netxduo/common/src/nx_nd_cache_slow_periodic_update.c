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
/*    _nx_nd_cache_slow_periodic_update                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the ND entries in REACHABLE, STALE, and       */
/*    DELAY states.  It decrements the timer tick.  If the timer ticks    */
/*    reach zero, the entry is moved to the next state.                   */
/*                                                                        */
/*    Note that the only cache entries whose timer ticks are being        */
/*    decremented in this function are states whose timer tick was set in */
/*    effectively in seconds (as compared with the fast periodic update   */
/*    where cache entry'timer ticks' are updated in as timer ticks.  This */
/*    is intentional and correct behavior.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to the IP instance    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NONE                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_thread_entry                                                 */
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
VOID _nx_nd_cache_slow_periodic_update(NX_IP *ip_ptr)
{

INT i;

    /* Check all entries in the ND cache for timer expiration. */
    for (i = 0; i < NX_IPV6_NEIGHBOR_CACHE_SIZE; i++)
    {
        /* Skip the invalid or empty ones. */
        if (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_nd_status == ND_CACHE_STATE_INVALID)
        {
            continue;
        }

        /* No need to update the static entries! */
        if (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_is_static)
        {
            continue;
        }

        /* If this is a reachable entry... */
        if (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_nd_status == ND_CACHE_STATE_REACHABLE)
        {

            ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_timer_tick--;

            /* And we have timed out... */
            if (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_timer_tick == 0)
            {

                /* Time to move the state into the STALE state. */
                ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_nd_status = ND_CACHE_STATE_STALE;
            }
        }
        /* Entries in the delay state are set to be 'probed'. */
        else if (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_nd_status == ND_CACHE_STATE_DELAY)
        {

            ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_timer_tick--;

            /* Has the timeout expired? */
            if (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_timer_tick == 0)
            {

                /* Set to the probe state. We do not send out NS;
                   the nd_cache_fast_periodic_update will handle the
                   processing of this entry now. */
                ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_nd_status = ND_CACHE_STATE_PROBE;
                ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_num_solicit = NX_MAX_UNICAST_SOLICIT;
            }
        }
        else if (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_nd_status == ND_CACHE_STATE_STALE)
        {

            /* When the entry is in stale mode, we actually increment the timer_tick.
               The larger the timer_tick value, the longer then entry has been in
               stale mode.  This makes the entry a target for recycling (being replaced
               by a newer reachable entry). */
            ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_timer_tick++;
        }
    }
}


#endif /* FEATURE_NX_IPV6 */

