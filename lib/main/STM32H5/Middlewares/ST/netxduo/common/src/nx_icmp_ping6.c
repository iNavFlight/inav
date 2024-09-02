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
#include "nx_icmpv6.h"

#ifdef FEATURE_NX_IPV6



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_icmp_ping6                                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function builds an ICMPv6 ping request packet and calls the    */
/*    associated driver to send it out on the network.  The function will */
/*    then suspend for the specified time waiting for the ICMP ping       */
/*    response.                                                           */
/*                                                                        */
/*    Note that for multicast or link local destination addresses, NetX   */
/*    Duo will default to the primary interface.  To specify a non primary*/
/*    interface to send pings out on with these address destination       */
/*    address types, use the nx_icmp_interface_ping6 service.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    ip_address                            IPv6 address to ping          */
/*    data_ptr                              Pointer to user data to       */
/*                                           include in ping packet       */
/*    data_size                             Size of user data             */
/*    response_ptr                          Pointer to response packet    */
/*    wait_option                           Time out on packet allocate   */
/*                                             and sending packet         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_ipv6_interface_find             Finds outgoing interface       */
/*    _nx_icmp_interface_ping6             Handle details of sending pings*/
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
UINT  _nx_icmp_ping6(NX_IP *ip_ptr, NXD_ADDRESS *ip_address, CHAR *data_ptr, ULONG data_size,
                     NX_PACKET **response_ptr, ULONG wait_option)
{

UINT              status;
NXD_IPV6_ADDRESS *outgoing_address;

    /* Clear the destination pointer.  */
    *response_ptr =  NX_NULL;

    /* Find a suitable outgoing interface. */
    status = _nxd_ipv6_interface_find(ip_ptr, ip_address -> nxd_ip_address.v6,
                                      &outgoing_address, NX_NULL);

    /* Cannot find usable outgoing interface. */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Now send the actual ping packet. */
    /*lint -e{644} suppress variable might not be initialized, since "outgoing_address" was initialized in _nxd_ipv6_interface_find. */
    status = _nx_icmp_interface_ping6(ip_ptr, ip_address, data_ptr, data_size, outgoing_address, response_ptr, wait_option);

    return(status);
}

#endif /* FEATURE_NX_IPV6 */

