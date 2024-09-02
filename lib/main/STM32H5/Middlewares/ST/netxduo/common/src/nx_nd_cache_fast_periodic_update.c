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
#include "nx_icmpv6.h"

#ifdef FEATURE_NX_IPV6


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_nd_cache_fast_periodic_update                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the ND entries in PROBE or INCOMPLETE state.  */
/*    It decrements the timer tick.  If the ticker reaches zero           */
/*    a NS is sent.  If MAX NS has been sent, this entry times out        */
/*    and is marked as INVALID.                                           */
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
/*    _nx_icmpv6_send_ns                    Transmit a new NS message.    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_nd_cache_delete_internal                                        */
/*    _nx_ip_thread_entry                                                 */
/*    tx_mutex_get                                                        */
/*    tx_mutex_put                                                        */
/*                                                                        */
/*  Note:                                                                 */
/*                                                                        */
/*    This routine acquires the nx_nd_cache_protection mutex.             */
/*    Caller shall not hold this mutex before calling this function.      */
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
VOID _nx_nd_cache_fast_periodic_update(NX_IP *ip_ptr)
{

INT i;

    /* Loop through all entries, and invalidate the ones that are timed out. */
    for (i = 0; i < NX_IPV6_NEIGHBOR_CACHE_SIZE; i++)
    {

        /* Check the entry is valid. */
        if (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_nd_status == ND_CACHE_STATE_INVALID)
        {
            continue;
        }

        /* Is this entry being checked for neighbor discovery? */
        if ((ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_nd_status != ND_CACHE_STATE_PROBE) &&
            (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_nd_status != ND_CACHE_STATE_INCOMPLETE))
        {

            /* No, so skip over. */
            continue;
        }



        /* Has this entry timed out? */
        if (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_timer_tick == 0)
        {

            /* Yes, is the max number of solicitations used up? */
            if (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_num_solicit == 0)
            {

                /* Yes; We already sent #num_solicit packets. So the destination
                   is unreachable.  Clean up router, destination and cache table entries and crosslinks. */
                _nx_nd_cache_delete_internal(ip_ptr, &ip_ptr -> nx_ipv6_nd_cache[i]);
            }
            else
            {
            /*  Send another solicitation (NS) packet. */
            INT uniCastNS;

                uniCastNS = (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_nd_status == ND_CACHE_STATE_PROBE);

                /* Send out another mcast ns.*/
                _nx_icmpv6_send_ns(ip_ptr, ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_dest_ip,
                                   1, ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_outgoing_address, uniCastNS, &ip_ptr -> nx_ipv6_nd_cache[i]);

                /* Keep track of how many we have sent. */
                ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_num_solicit--;

                /* Reset the expiration timer for sending the next NS.  */
                ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_timer_tick = ip_ptr -> nx_ipv6_retrans_timer_ticks;
            }
        }
        else
        {

            /* Note that the only cache entries whose timer ticks are being decremented in this
               function are states whose timer tick was set in actual timer ticks (as compared
               with the slow periodic update where cache entry'timer ticks' are updated in
               seconds.  This is intentional and correct behavior. */
            ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_timer_tick--;
        }
    }
}

#endif /* FEATURE_NX_IPV6 */

