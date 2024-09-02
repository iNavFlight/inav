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

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */


#ifdef FEATURE_NX_IPV6


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmpv6_DAD_failure                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called when the DAD process determines that a      */
/*    duplicate address is present on the local network.  The interface   */
/*    is set to an invalid state.                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP stack instance             */
/*    ipv6_address                          The local IPv6 interface      */
/*                                            that detects the failure.   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [ipv6_address_change_notify]         User callback function         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_icmpv6_process_na                                               */
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

#ifndef NX_DISABLE_IPV6_DAD
VOID _nx_icmpv6_DAD_failure(NX_IP *ip_ptr, NXD_IPV6_ADDRESS *ipv6_address)
{
#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
UINT              interface_index;
UINT              ipv6_addr_index;
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */
NXD_IPV6_ADDRESS *address_ptr;

    /* Set the interface to an invalid state. */
    ipv6_address -> nxd_ipv6_address_state = NX_IPV6_ADDR_STATE_UNKNOWN;
    ipv6_address -> nxd_ipv6_address_valid = NX_FALSE;

    /* Indicate the DAD process is disabled. */
    ipv6_address -> nxd_ipv6_address_DupAddrDetectTransmit = 0;
#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
    if (ip_ptr -> nx_ipv6_address_change_notify)
    {
        ipv6_addr_index = (ULONG)ipv6_address -> nxd_ipv6_address_index;
        interface_index = (ULONG)ipv6_address -> nxd_ipv6_address_attached -> nx_interface_index;
        ip_ptr -> nx_ipv6_address_change_notify(ip_ptr, NX_IPV6_ADDRESS_DAD_FAILURE, interface_index, ipv6_addr_index, &ipv6_address -> nxd_ipv6_address[0]);
    }
#else
    NX_PARAMETER_NOT_USED(ip_ptr);
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */

    /* Remove address from interface. */
    if (ipv6_address == ipv6_address -> nxd_ipv6_address_attached -> nxd_interface_ipv6_address_list_head)
    {
        ipv6_address -> nxd_ipv6_address_attached -> nxd_interface_ipv6_address_list_head = ipv6_address -> nxd_ipv6_address_next;
    }
    else
    {

        for (address_ptr = ipv6_address -> nxd_ipv6_address_attached -> nxd_interface_ipv6_address_list_head;
             address_ptr != NX_NULL;
             address_ptr = address_ptr -> nxd_ipv6_address_next)
        {
            if (address_ptr -> nxd_ipv6_address_next == ipv6_address)
            {
                address_ptr -> nxd_ipv6_address_next = ipv6_address -> nxd_ipv6_address_next;
            }
        }
    }
}

#endif  /* NX_DISABLE_IPV6_DAD */
#endif  /* FEATURE_NX_IPV6 */

