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
#include "nx_ip.h"
#include "nx_nd_cache.h"
#include "nx_icmpv6.h"
#endif /* FEATURE_NX_IPV6 */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_enable                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables the IPv6 for the specified IP instance.       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ipv6_multicast_join               Join the multicast group      */
/*    _nxd_ipv6_default_router_table_init   Initialize IPv6 routing table */
/*    _nx_ip_fast_periodic_timer_create     Create timer for IPv6 events  */
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
UINT  _nxd_ipv6_enable(NX_IP *ip_ptr)
{


#ifdef FEATURE_NX_IPV6

TX_INTERRUPT_SAVE_AREA
UINT  i;
#ifndef NX_DISABLE_ICMPV6_ROUTER_SOLICITATION
ULONG address[4];
#endif /* NX_DISABLE_ICMPV6_ROUTER_SOLICITATION */


    /* Make sure IPv6 is not already enabled. */
    if (ip_ptr -> nx_ipv6_packet_receive)
    {
        return(NX_ALREADY_ENABLED);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IPV6_ENABLE, ip_ptr, 0, 0, 0, NX_TRACE_IP_EVENTS, 0, 0);


    /* Obtain the IP mutex so we can manipulate the internal routing table. */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Disable Interrupt */
    TX_DISABLE

    /* Install IPv6 packet receive processing function pointer */
    ip_ptr -> nx_ipv6_packet_receive = _nx_ipv6_packet_receive;

    /* Enable Interrupt */
    TX_RESTORE

    /* Initialize IPv6 default router table */
    _nxd_ipv6_default_router_table_init(ip_ptr);

    /* Set the default reachable timer. */
    ip_ptr -> nx_ipv6_reachable_timer = NX_REACHABLE_TIME;

    /*
       Set the default retrans timer. The retrans timer value is
       expressed in millisecond.  To compute the timer ticks, first
       covert the retrans timer to second and then multiply
       the system_ticks_per_second value, or multiply system_ticks_per_second
       before doing division.
       If the retrans_timer is smaller than tick resolution, set it to 1.
     */
#if ((NX_RETRANS_TIMER * NX_IP_FAST_TIMER_RATE) > 1000)
    ip_ptr -> nx_ipv6_retrans_timer_ticks = (NX_RETRANS_TIMER * NX_IP_FAST_TIMER_RATE) / 1000;
#else
    ip_ptr -> nx_ipv6_retrans_timer_ticks = 1;
#endif

    /* Set the default hop limit. */
    ip_ptr -> nx_ipv6_hop_limit = 0xFF;

    /* Set index of each address. */
    for (i = 0; i < (NX_MAX_IPV6_ADDRESSES + NX_LOOPBACK_IPV6_ENABLED); i++)
    {
        ip_ptr -> nx_ipv6_address[i].nxd_ipv6_address_index = (UCHAR)i;
    }

    /* Check if router solicitation is not disabled. */
#ifndef NX_DISABLE_ICMPV6_ROUTER_SOLICITATION

    /* Create the all-node multicast group address, */
    address[0] = 0xFF020000;
    address[1] = 0;
    address[2] = 0;
    address[3] = 1;

    /* Initializes the router solicitation values . */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {
        if (ip_ptr -> nx_ip_interface[i].nx_interface_valid == NX_TRUE)
        {
            ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_max = NX_ICMPV6_MAX_RTR_SOLICITATIONS;
            ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_count = ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_max;
            ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_interval = NX_ICMPV6_RTR_SOLICITATION_INTERVAL;
            ip_ptr -> nx_ip_interface[i].nx_ipv6_rtr_solicitation_timer = NX_ICMPV6_RTR_SOLICITATION_DELAY;

            /* Join all-node multicast group. */
            _nx_ipv6_multicast_join(ip_ptr, address, &ip_ptr -> nx_ip_interface[i]);
        }
    }
#endif /* NX_DISABLE_ICMPV6_ROUTER_SOLICITATION */

    /* Start a faster periodic timer for IPv6 .*/
    _nx_ip_fast_periodic_timer_create(ip_ptr);

#ifndef NX_DISABLE_LOOPBACK_INTERFACE

    /* Setup loop back address. */
    /* Page 9, section 2.5.3, RFC 4291. */
    ip_ptr -> nx_ipv6_address[NX_LOOPBACK_IPV6_SOURCE_INDEX].nxd_ipv6_address[0] = 0x0;
    ip_ptr -> nx_ipv6_address[NX_LOOPBACK_IPV6_SOURCE_INDEX].nxd_ipv6_address[1] = 0x0;
    ip_ptr -> nx_ipv6_address[NX_LOOPBACK_IPV6_SOURCE_INDEX].nxd_ipv6_address[2] = 0x0;
    ip_ptr -> nx_ipv6_address[NX_LOOPBACK_IPV6_SOURCE_INDEX].nxd_ipv6_address[3] = 0x1;

    /* Config loop bcak address. */
    ip_ptr -> nx_ipv6_address[NX_LOOPBACK_IPV6_SOURCE_INDEX].nxd_ipv6_address_valid = NX_TRUE;
    ip_ptr -> nx_ipv6_address[NX_LOOPBACK_IPV6_SOURCE_INDEX].nxd_ipv6_address_type = NX_IP_VERSION_V6;
    ip_ptr -> nx_ipv6_address[NX_LOOPBACK_IPV6_SOURCE_INDEX].nxd_ipv6_address_prefix_length = 128;
    ip_ptr -> nx_ipv6_address[NX_LOOPBACK_IPV6_SOURCE_INDEX].nxd_ipv6_address_next = NX_NULL;
    ip_ptr -> nx_ipv6_address[NX_LOOPBACK_IPV6_SOURCE_INDEX].nxd_ipv6_address_attached = &ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE];
    ip_ptr -> nx_ipv6_address[NX_LOOPBACK_IPV6_SOURCE_INDEX].nxd_ipv6_address_ConfigurationMethod = NX_IPV6_ADDRESS_MANUAL_CONFIG;
    ip_ptr -> nx_ipv6_address[NX_LOOPBACK_IPV6_SOURCE_INDEX].nxd_ipv6_address_state = NX_IPV6_ADDR_STATE_VALID;

    ip_ptr -> nx_ip_interface[NX_LOOPBACK_INTERFACE].nxd_interface_ipv6_address_list_head = &ip_ptr -> nx_ipv6_address[NX_LOOPBACK_IPV6_SOURCE_INDEX];
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

