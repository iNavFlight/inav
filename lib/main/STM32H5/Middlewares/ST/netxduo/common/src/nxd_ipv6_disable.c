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
/**   Internet Protocol (IP)                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ipv6.h"
#ifdef FEATURE_NX_IPV6
#include "nx_nd_cache.h"
#include "nx_icmpv6.h"
#endif /* FEATURE_NX_IPV6 */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_disable                                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function disables the IPv6 for the specified IP instance.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*    NX_ALREADY_ENABLED                    IPv6 already enabled on IP    */
/*    NX_NOT_SUPPORTED                      IPv6 not supported on this IP */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ipv6_multicast_leave              Leave the multicast group     */
/*    _nxd_ipv6_default_router_table_init   Initialize IPv6 routing table */
/*    _nx_nd_cache_delete_internal          Delete ND Cache Entry         */
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release protection mutex      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
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
UINT  _nxd_ipv6_disable(NX_IP *ip_ptr)
{


#ifdef FEATURE_NX_IPV6

TX_INTERRUPT_SAVE_AREA
ULONG address[4];
UINT  i;


    /* Make sure IPv6 is not already enabled. */
    /* Cast the function pointer into a ULONG. Since this is exactly what we wish to do, disable the lint warning with the following comment:  */
    /*lint -e{923} suppress cast of pointer to ULONG.  */
    if ((ALIGN_TYPE)ip_ptr -> nx_ipv6_packet_receive == NX_NULL)
    {
        return(NX_SUCCESS);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IPV6_DISABLE, ip_ptr, 0, 0, 0, NX_TRACE_IP_EVENTS, 0, 0);


    /* Obtain the IP mutex so we can manipulate the internal routing table. */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Disable Interrupt */
    TX_DISABLE

    /* Install IPv6 packet receive processing function pointer */
    ip_ptr -> nx_ipv6_packet_receive = NX_NULL;

    /* Enable Interrupt */
    TX_RESTORE

    /* Zero out the IPv6 default router table */
    _nxd_ipv6_default_router_table_init(ip_ptr);

    /* Clear IPv6 internal timers. */
    ip_ptr -> nx_ipv6_reachable_timer = 0;
    ip_ptr -> nx_ipv6_retrans_timer_ticks = 0;

    /* Check if router solicitation is not disabled. */
#ifndef NX_DISABLE_ICMPV6_ROUTER_SOLICITATION

    /* Create the all-node multicast group address, */
    address[0] = 0xFF020000;
    address[1] = 0;
    address[2] = 0;
    address[3] = 1;
#endif /* NX_DISABLE_ICMPV6_ROUTER_SOLICITATION */


    /* Reset the router solicitation values . */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {
        if (ip_ptr -> nx_ip_interface[i].nx_interface_valid == NX_TRUE)
        {
            ip_ptr -> nx_ip_interface[i].nxd_interface_ipv6_address_list_head = NX_NULL;

#ifndef NX_DISABLE_ICMPV6_ROUTER_SOLICITATION
            ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_max = 0;
            ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_count = 0;
            ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_interval = 0;
            ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_timer = 0;

            /* Leave all-node multicast group. */
            _nx_ipv6_multicast_leave(ip_ptr, address, &ip_ptr -> nx_ip_interface[i]);

#endif /* NX_DISABLE_ICMPV6_ROUTER_SOLICITATION */
        }
    }


    /* Remove the IPv6 addresses, and leave the corresponding multicast groups. */
    for (i = 0; i < NX_MAX_IPV6_ADDRESSES; i++)
    {
        if (ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address_valid)
        {

            SET_SOLICITED_NODE_MULTICAST_ADDRESS(address, ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address);

            _nx_ipv6_multicast_leave(ip_ptr, &address[0], ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address_attached);
        }
    }

    /* Zero out the IPv6 address structure table. */
    memset(&ip_ptr -> nx_ipv6_address[0], 0, sizeof(ip_ptr -> nx_ipv6_address));

    /* Clean up the ND Cache table.  */
    if (ip_ptr -> nx_ip_icmpv6_packet_process)
    {
        for (i = 0; i < NX_IPV6_NEIGHBOR_CACHE_SIZE; i++)
        {
            if ((ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_nd_status != ND_CACHE_STATE_INVALID) &&
                (ip_ptr -> nx_ipv6_nd_cache[i].nx_nd_cache_interface_ptr))
            {
                _nx_nd_cache_delete_internal(ip_ptr, &ip_ptr -> nx_ipv6_nd_cache[i]);
            }
        }
    }

    /* Disable ND cache periodic update routine. */
    ip_ptr -> nx_nd_cache_fast_periodic_update = NX_NULL;
    ip_ptr -> nx_nd_cache_slow_periodic_update = NX_NULL;

    /* Disable ICMPv6 services. */
    ip_ptr -> nx_ip_icmpv6_packet_process = NX_NULL;

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY
    ip_ptr -> nx_destination_table_periodic_update = NX_NULL;
#endif /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY */

    /* Clean up the loop back address. */
#ifndef NX_DISABLE_LOOPBACK_INTERFACE
    ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE].nxd_interface_ipv6_address_list_head = NX_NULL;
#endif /* NX_DISABLE_LOOPBACK_INTERFACE */

    /* Release the IP protection. */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return successful completion.  */
    return(NX_SUCCESS);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

