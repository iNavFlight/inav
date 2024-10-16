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
/*    _nx_icmpv6_perform_DAD                              PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is called from IP thread periodic process routine.    */
/*    It starts the Duplicate Address Detection process if the interface  */
/*    address is in tentative state (not validated yet).  It does nothing */
/*    to an address if the state of the address is not tentative.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_icmpv6_DAD_clear_NDCache_entry   Remove entry from cache table  */
/*    _nx_icmpv6_send_ns                   Send a Neighbor Solicitation   */
/*                                                message                 */
/*    [ipv6_address_change_notify]         User callback fucntion         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_ip_thraed_entry                                                 */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            added internal ip address   */
/*                                            change notification,        */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/

#ifndef NX_DISABLE_IPV6_DAD

VOID _nx_icmpv6_perform_DAD(NX_IP *ip_ptr)
{

UINT              i;
NXD_IPV6_ADDRESS *nx_ipv6_address_next;

#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
UINT              ipv6_addr_index;
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */

    /* Go through all addresses bound to the IP instance. */
    for (i = 0; i < NX_MAX_PHYSICAL_INTERFACES; i++)
    {

        /* Check if this interface valid. */
        if (!ip_ptr -> nx_ip_interface[i].nxd_interface_ipv6_address_list_head)
        {
            continue;
        }

        /* Only interested in addresses in the tentative state. */
        for (nx_ipv6_address_next = ip_ptr -> nx_ip_interface[i].nxd_interface_ipv6_address_list_head;
             nx_ipv6_address_next;
             nx_ipv6_address_next = nx_ipv6_address_next -> nxd_ipv6_address_next)
        {

            /* Check the address state. */
            if (nx_ipv6_address_next -> nxd_ipv6_address_state == NX_IPV6_ADDR_STATE_TENTATIVE)
            {

                /* Check if the number of NS messages is used up. */
                if (nx_ipv6_address_next -> nxd_ipv6_address_DupAddrDetectTransmit)
                {

                    /* No. This interface is still under DAD.  Transmit a NS */
                    _nx_icmpv6_send_ns(ip_ptr,
                                       nx_ipv6_address_next -> nxd_ipv6_address,
                                       0, nx_ipv6_address_next, 0, NX_NULL);

                    nx_ipv6_address_next -> nxd_ipv6_address_DupAddrDetectTransmit--;
                }
                else
                {

                    /* So far we didn't get any conflict addresses back.
                       So promote the address to VALID */

                    nx_ipv6_address_next -> nxd_ipv6_address_state = NX_IPV6_ADDR_STATE_VALID;
                    _nx_icmpv6_DAD_clear_NDCache_entry(ip_ptr,
                                                       nx_ipv6_address_next -> nxd_ipv6_address);

#ifdef NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY
                    /* If the callback function is set, invoke the callback function . */
                    if (ip_ptr -> nx_ipv6_address_change_notify)
                    {
                        ipv6_addr_index = (ULONG)nx_ipv6_address_next -> nxd_ipv6_address_index;
                        ip_ptr -> nx_ipv6_address_change_notify(ip_ptr, NX_IPV6_ADDRESS_DAD_SUCCESSFUL,
                                                                i, ipv6_addr_index, &nx_ipv6_address_next -> nxd_ipv6_address[0]);
                    }

                    /* If the internal callback function is set, invoke the callback function . */
                    if (ip_ptr -> nx_ipv6_address_change_notify_internal)
                    {
                        ipv6_addr_index = (ULONG)nx_ipv6_address_next -> nxd_ipv6_address_index;
                        ip_ptr -> nx_ipv6_address_change_notify_internal(ip_ptr, NX_IPV6_ADDRESS_DAD_SUCCESSFUL,
                                                                         i, ipv6_addr_index, &nx_ipv6_address_next -> nxd_ipv6_address[0]);
                    }
#endif /* NX_ENABLE_IPV6_ADDRESS_CHANGE_NOTIFY */
                }
            }
        }
    }
}

#endif /* NX_DISABLE_IPV6_DAD */


#endif /* FEATURE_NX_IPV6 */

