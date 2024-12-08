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
#include "nx_ip.h"
#include "nx_ipv6.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_ipv6_address_get                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*   This function retrieves the IP address at the specified address list */
/*  index of the physical host interface specified by the interface index.*/
/*  Returns an IPv6 address and prefix length into the specified pointers.*/
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP control block pointer      */
/*    address_index                         Index to the IPv6 address     */
/*                                            table                       */
/*    ip_address                            Pointer to IP address         */
/*                                            structure to be filled      */
/*    prefix_length                         Pointer to prefix length      */
/*                                            to return                   */
/*    interface_index                       The index to the physical     */
/*                                            interface this address      */
/*                                            belongs to                  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  NOTE                                                                  */
/*    Application needs to fill in the ip_address.nxd_ip_version field.   */
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
UINT  _nxd_ipv6_address_get(NX_IP *ip_ptr, UINT address_index, NXD_ADDRESS *ip_address,
                            ULONG *prefix_length, UINT *interface_index)
{
#ifdef FEATURE_NX_IPV6

TX_INTERRUPT_SAVE_AREA

UINT              status;
NXD_IPV6_ADDRESS *interface_ipv6_address_next;
#ifdef TX_ENABLE_EVENT_TRACE
ULONG             ip_address_lsw;
#endif /* TX_ENABLE_EVENT_TRACE */


    status = NX_SUCCESS;

    /* Get mutex protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);
    /* Disable interrupts.  */
    TX_DISABLE

    /* Get the ip address.  */
    interface_ipv6_address_next = &ip_ptr -> nx_ipv6_address[address_index];

    /* Check if this is a valid IP address. */
    if (interface_ipv6_address_next -> nxd_ipv6_address_state != NX_IPV6_ADDR_STATE_VALID)
    {

        /* No, the address is not validated yet. */

        /* Zero out the return values. */
        *prefix_length = 0;
        SET_UNSPECIFIED_ADDRESS(ip_address -> nxd_ip_address.v6);

        /* Return the error status. */
        status = NX_NO_INTERFACE_ADDRESS;
    }
    else
    {

        /* Record the interface index.  */
        *interface_index = (UINT)ip_ptr -> nx_ipv6_address[address_index].nxd_ipv6_address_attached -> nx_interface_index;

        /* We have a valid address. Mark with the IPv6 stamp. */
        ip_address -> nxd_ip_version = NX_IP_VERSION_V6;

        /* Copy interface IP address from the address entry in the IP address table into the return address structure. */
        COPY_IPV6_ADDRESS(interface_ipv6_address_next -> nxd_ipv6_address,
                          ip_address -> nxd_ip_address.v6);

        /* Copy interface IP address prefix length from the address entry in the IP address table into the return prefix length. */
        *prefix_length = interface_ipv6_address_next -> nxd_ipv6_address_prefix_length;
    }

    /* Restore interrupts.  */
    TX_RESTORE

    /* Release mutex protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

#ifdef TX_ENABLE_EVENT_TRACE
    ip_address_lsw = ip_address -> nxd_ip_address.v6[3];

    /* If trace is enabled, insert this info into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NXD_TRACE_IPV6_INTERFACE_ADDRESS_GET, ip_ptr, ip_address_lsw, *prefix_length, address_index, NX_TRACE_IP_EVENTS, 0, 0);
#endif /* TX_ENABLE_EVENT_TRACE */

    /* Return completion status.  */
    return(status);

#else /* !FEATURE_NX_IPV6 */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(address_index);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(prefix_length);
    NX_PARAMETER_NOT_USED(interface_index);

    return(NX_NOT_SUPPORTED);

#endif /* FEATURE_NX_IPV6 */
}

