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
/*    _nxd_icmp_source_ping                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Depending on the destination IP address, this function performs     */
/*    ICMP ping or ICMPv6 ping. using specified source address.           */
/*    For ICMPv6 ping, the sender address is set to the IPv6 indexed      */
/*    by address_index.  The parameter address_index is ignored for       */
/*    IPv4 ping operations.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    ip_address                            IP address to ping            */
/*    address_index                         Index to the IPv6 address     */
/*    data_ptr                              User Data pointer             */
/*    data_size                             Size of User Data             */
/*    response_ptr                          Pointer to Response Packet    */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion         */
/*    NX_NOT_SUPPORTED                      IPv6 or ICMP not enabled      */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_route_find                     Find suitable interface       */
/*    _nx_icmp_interface_ping               IPv4 ICMP ping routine.       */
/*    _nx_icmp_interface_ping6              IPv6 ICMP ping routine.       */
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
UINT  _nxd_icmp_source_ping(NX_IP *ip_ptr, NXD_ADDRESS *ip_address, UINT address_index,
                            CHAR *data_ptr, ULONG data_size,
                            NX_PACKET **response_ptr, ULONG wait_option)
{

UINT          status = NX_NOT_SUPPORTED;
#ifndef NX_DISABLE_IPV4
ULONG         next_hop_address;
NX_INTERFACE *interface_ptr;
#endif /* !NX_DISABLE_IPV4  */

#ifndef NX_DISABLE_IPV4
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {

        /* Set the outgoing interface. */
        interface_ptr = &ip_ptr -> nx_ip_interface[address_index];

        /* Find the next hop address. . */
        _nx_ip_route_find(ip_ptr, ip_address -> nxd_ip_address.v4, &interface_ptr, &next_hop_address);

        /*lint -e{644} suppress variable might not be initialized, since "next_hop_address" was initialized in _nx_ip_route_find. */
        status = _nx_icmp_interface_ping(ip_ptr, ip_address -> nxd_ip_address.v4, interface_ptr, next_hop_address,
                                         data_ptr, data_size, response_ptr, wait_option);
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        status = _nx_icmp_interface_ping6(ip_ptr, ip_address, data_ptr, data_size,
                                          &ip_ptr -> nx_ipv6_address[address_index], response_ptr, wait_option);
    }
#endif /* FEATURE_NX_IPV6 */

    return(status);
}

