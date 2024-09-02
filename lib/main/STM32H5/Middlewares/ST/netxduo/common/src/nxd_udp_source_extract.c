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
/**   User Datagram Protocol (UDP)                                        */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_udp.h"
#include "nx_ipv6.h"
#include "nx_ipv4.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_udp_socket_extract                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function extracts the source IP address and UDP port number    */
/*    from the packet received on a host UDP socket. It is up to the      */
/*    caller to verify the source port and ip address are valid (non null)*/
/*    data.                                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to UDP packet pointer */
/*    ip_address                            Pointer to source IP address  */
/*    port                                  Pointer to source UDP port    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Actual completion status      */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
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
UINT  _nxd_udp_source_extract(NX_PACKET *packet_ptr, NXD_ADDRESS *ip_address, UINT *port)
{

#ifdef TX_ENABLE_EVENT_TRACE
ULONG  ip_address_word3 = 0;
ULONG  ip_version;
#endif
ULONG *temp_ptr;


    /* Build an address to the current top of the packet.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    temp_ptr =  (ULONG *)packet_ptr -> nx_packet_prepend_ptr;

    /* Pickup the source port from the UDP header.  */
    *port =  (UINT)(*(temp_ptr - 2) >> NX_SHIFT_BY_16);

    /* Determine IPv4 or IPv6 connectivity. */
    ip_address -> nxd_ip_version = packet_ptr -> nx_packet_ip_version;

#ifndef NX_DISABLE_IPV4
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {

    NX_IPV4_HEADER *ipv4_header;

        /* Obtain the IPv4 header. */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ipv4_header = (NX_IPV4_HEADER *)packet_ptr -> nx_packet_ip_header;

        /* Pickup the source IP address.  */
        ip_address -> nxd_ip_address.v4 =  ipv4_header -> nx_ip_header_source_ip;

#ifdef TX_ENABLE_EVENT_TRACE
        ip_version = NX_IP_VERSION_V4;
        ip_address_word3 = ip_address -> nxd_ip_address.v4;
#endif /* TX_ENABLE_EVENT_TRACE */
    }
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {
    NX_IPV6_HEADER *ipv6_header;


        /* Obtain the IPv6 header. */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ipv6_header = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

        COPY_IPV6_ADDRESS(ipv6_header -> nx_ip_header_source_ip,
                          ip_address -> nxd_ip_address.v6);

#ifdef TX_ENABLE_EVENT_TRACE
        ip_version = NX_IP_VERSION_V6;
        ip_address_word3 = ip_address -> nxd_ip_address.v6[3];
#endif /* TX_ENABLE_EVENT_TRACE */
    }
#endif /* FEATURE_NX_IPV6 */

#ifdef TX_ENABLE_EVENT_TRACE

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NXD_TRACE_UDP_SOURCE_EXTRACT, packet_ptr, ip_version, ip_address_word3, *port, NX_TRACE_UDP_EVENTS, 0, 0);
#endif /* TX_ENABLE_EVENT_TRACE */

    return(NX_SUCCESS);
}

