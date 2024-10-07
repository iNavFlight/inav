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
/**   Internet Control Message Protocol for IPv6 (ICMPv6)                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE

/* Include necessary system files.  */
#include "nx_api.h"
#include "nx_icmp.h"
#include "nx_icmpv6.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_icmp_enable                                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables the ICMPv4 and ICMPv6 services for the        */
/*    specified IP instance.                                              */
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
/*    memset                                Set the memory                */
/*    tx_mutex_get                          Obtain protection mutex       */
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
UINT _nxd_icmp_enable(NX_IP *ip_ptr)
{


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NXD_TRACE_ICMP_ENABLE, ip_ptr, 0, 0, 0, NX_TRACE_ICMP_EVENTS, 0, 0);

    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Setup the ICMP packet receiving routine, thereby enabling ICMP traffic.  */
    /* ICMPv4 and ICMPv6 share the same packet_receive routine.  */
    ip_ptr -> nx_ip_icmp_packet_receive =  _nx_icmp_packet_receive;

#ifndef NX_DISABLE_IPV4
    /* Setup the ICMP packet queue processing routine.  */
    ip_ptr -> nx_ip_icmp_queue_process =  _nx_icmp_queue_process;

    /* Start the ICMPv4 packet process routine */
    ip_ptr -> nx_ip_icmpv4_packet_process = _nx_icmpv4_packet_process;
#endif

#ifdef FEATURE_NX_IPV6
    /* Setup the ICMPv6 packet process routine */
    ip_ptr -> nx_ip_icmpv6_packet_process = _nx_icmpv6_packet_process;

    /* Setup the ND Cache periodic update routine */
    ip_ptr -> nx_nd_cache_fast_periodic_update = _nx_nd_cache_fast_periodic_update;
    ip_ptr -> nx_nd_cache_slow_periodic_update = _nx_nd_cache_slow_periodic_update;

    /* Initialize tables used in ICMPv6 protocols. */
    /* Clear the ND Cache table. */
    memset(&ip_ptr -> nx_ipv6_nd_cache[0], 0, sizeof(ND_CACHE_ENTRY) * NX_IPV6_NEIGHBOR_CACHE_SIZE);

    /* Clear the destination table. */
    memset(&ip_ptr -> nx_ipv6_destination_table[0], 0, sizeof(NX_IPV6_DESTINATION_ENTRY) * NX_IPV6_DESTINATION_TABLE_SIZE);

    /* Set the initial size to zero. */
    ip_ptr -> nx_ipv6_destination_table_size = 0;

#ifdef NX_ENABLE_IPV6_PATH_MTU_DISCOVERY
    /* Set up the MTU path discovery periodic update. */
    ip_ptr -> nx_destination_table_periodic_update = _nx_icmpv6_destination_table_periodic_update;
#endif /* NX_ENABLE_IPV6_PATH_MTU_DISCOVERY  */

#endif /* FEATURE_NX_IPV6 */

    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return a successful status!  */
    return(NX_SUCCESS);
}

