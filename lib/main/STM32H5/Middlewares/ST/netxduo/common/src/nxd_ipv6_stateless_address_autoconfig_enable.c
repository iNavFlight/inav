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
#include "nx_icmpv6.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_stateless_address_autoconfig_enable       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables the IPv6 stateless address auto configuration */
/*    feature,                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    interface_index                       The interface to operate on   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
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
UINT  _nxd_ipv6_stateless_address_autoconfig_enable(NX_IP *ip_ptr, UINT interface_index)
{


#if defined(FEATURE_NX_IPV6) && defined(NX_IPV6_STATELESS_AUTOCONFIG_CONTROL)
NX_INTERFACE *interface_ptr;


    /* Set interface_ptr.  */
    interface_ptr = &ip_ptr -> nx_ip_interface[interface_index];

    /* Make sure IPv6 is not already enabled. */
    if (interface_ptr -> nx_ipv6_stateless_address_autoconfig_status == NX_STATELESS_ADDRESS_AUTOCONFIG_ENABLED)
    {
        return(NX_ALREADY_ENABLED);
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NXD_TRACE_IPV6_STATELESS_ADDRESS_AUTOCONFIG_ENABLE, ip_ptr, interface_ptr, 0, 0, NX_TRACE_IP_EVENTS, 0, 0);

    /* Obtain the IP mutex so we can manipulate the internal routing table. */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Install IPv6 packet receive processing function pointer */
    interface_ptr -> nx_ipv6_stateless_address_autoconfig_status = NX_STATELESS_ADDRESS_AUTOCONFIG_ENABLED;

    /* Reset the RS count. */
    interface_ptr -> nx_ipv6_rtr_solicitation_count = ip_ptr -> nx_ip_interface[interface_index].nx_ipv6_rtr_solicitation_max;

    interface_ptr -> nx_ipv6_rtr_solicitation_timer = NX_ICMPV6_RTR_SOLICITATION_DELAY;

    /* Release the IP protection. */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return successful completion.  */
    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(interface_index);

    return(NX_NOT_SUPPORTED);
#endif /* FEATURE_NX_IPV6  && NX_IPV6_STATELESS_AUTOCONFIG_CONTROL */
}

