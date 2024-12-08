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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_raw_packet_source_send                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a raw IP packet through the specified IP        */
/*    source.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*    destination_ip                        Destination IP address        */
/*    address_index                         Index to the IPv6 address     */
/*    type_of_service                       Type of service for packet    */
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
UINT  _nx_ip_raw_packet_source_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                                    ULONG destination_ip, UINT address_index, ULONG type_of_service)
{

#ifndef NX_DISABLE_IPV4
ULONG next_hop_address;

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Store interface information into the packet structure. */
    packet_ptr -> nx_packet_address.nx_packet_interface_ptr = &(ip_ptr -> nx_ip_interface[address_index]);

    /* Get mutex protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Figure out a suitable outgoing interface. */
    _nx_ip_route_find(ip_ptr, destination_ip, &packet_ptr -> nx_packet_address.nx_packet_interface_ptr, &next_hop_address);

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_IP_RAW_PACKET_SEND, ip_ptr, packet_ptr, destination_ip, type_of_service, NX_TRACE_IP_EVENTS, 0, 0);

    /* Yes, raw packet sending and receiving is enabled send packet!  */
    /*lint -e{644} suppress variable might not be initialized, since "next_hop_address" was initialized in _nx_ip_route_find. */
    _nx_ip_packet_send(ip_ptr, packet_ptr, destination_ip, type_of_service, NX_IP_TIME_TO_LIVE, NX_IP_RAW, NX_FRAGMENT_OKAY, next_hop_address);

    /* Release mutex protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return a successful status!  */
    return(NX_SUCCESS);
#else /* NX_DISABLE_IPV4  */
    NX_PARAMETER_NOT_USED(ip_ptr);
    NX_PARAMETER_NOT_USED(packet_ptr);
    NX_PARAMETER_NOT_USED(destination_ip);
    NX_PARAMETER_NOT_USED(address_index);
    NX_PARAMETER_NOT_USED(type_of_service);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

