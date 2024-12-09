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
/*    _nxd_ip_raw_packet_source_send                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a raw IP packet using specified interface IP    */
/*    address as source.                                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*    destination_ip                        Destination IP address        */
/*    address_index                         Index of IPv4 or IPv6 address */
/*                                            to use as the source address*/
/*    protocol                              Value for the protocol field  */
/*    ttl                                   Value for ttl or hop limit    */
/*    tos                                   Value for tos or traffic      */
/*                                            class and flow label        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    nx_ip_packet_send                     Core IP packet send service   */
/*    nx_ip_route_find                      Find a suitable outgoing      */
/*                                            interface.                  */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    _nxd_ipv6_raw_packet_send_internal    IPv6 raw packet send          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
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
UINT  _nxd_ip_raw_packet_source_send(NX_IP *ip_ptr,
                                     NX_PACKET *packet_ptr,
                                     NXD_ADDRESS *destination_ip,
                                     UINT address_index,
                                     ULONG protocol,
                                     UINT ttl,
                                     ULONG tos)
{

#ifndef NX_DISABLE_IPV4
ULONG next_hop_address = NX_NULL;
#endif /* NX_DISABLE_IPV4  */
UINT  status = NX_SUCCESS;

#ifdef NX_DISABLE_IPV4
    NX_PARAMETER_NOT_USED(ttl);
    NX_PARAMETER_NOT_USED(tos);
#endif /* NX_DISABLE_IPV4 */

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Get mutex protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

#ifndef NX_DISABLE_IPV4

    /* Store address information into the packet structure. */
    if (destination_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        packet_ptr -> nx_packet_address.nx_packet_interface_ptr = &(ip_ptr -> nx_ip_interface[address_index]);

        /* Figure out a suitable outgoing interface. */
        /* Since next_hop_address is validated in _nx_ip_packet_send, there is no need to check the value here. */
        _nx_ip_route_find(ip_ptr, destination_ip -> nxd_ip_address.v4, &packet_ptr -> nx_packet_address.nx_packet_interface_ptr, &next_hop_address);

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_RAW_PACKET_SEND, ip_ptr, packet_ptr, destination_ip, 0, NX_TRACE_IP_EVENTS, 0, 0);

        /* Yes, raw packet sending and receiving is enabled send packet!  */
        /*lint -e{644} suppress variable might not be initialized, since "next_hop_address" was initialized in _nx_ip_route_find. */
        _nx_ip_packet_send(ip_ptr, packet_ptr, destination_ip -> nxd_ip_address.v4, (tos & 0xFF) << 16, (ttl & 0xFF), protocol << 16, NX_FRAGMENT_OKAY, next_hop_address);
    }
#endif /* NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (destination_ip -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr = &(ip_ptr -> nx_ipv6_address[address_index]);

        status =  _nxd_ipv6_raw_packet_send_internal(ip_ptr, packet_ptr, destination_ip, protocol);
    }
#endif /* FEATURE_NX_IPV6 */

    /* Release mutex protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return a status!  */
    return(status);
}

