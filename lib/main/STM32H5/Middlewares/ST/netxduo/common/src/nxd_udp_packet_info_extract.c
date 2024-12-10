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
#include "nx_ipv4.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_udp_packet_info_extract                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function extracts the source IP address, protocol, (the        */
/*    protocol is always UDP), port number and the incoming interface     */
/*    from the incoming packet.                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to UDP packet         */
/*    ip_address                            Pointer to sender IP address  */
/*    protocol                              Pointer to packet protocol.   */
/*                                            Always 17 (UDP)             */
/*    port                                  Pointer to sender source port */
/*    interface_index                         Pointer to interface index  */
/*                                            packet received on          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
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
UINT  _nxd_udp_packet_info_extract(NX_PACKET *packet_ptr, NXD_ADDRESS *ip_address,
                                   UINT *protocol, UINT *port, UINT *interface_index)
{

ULONG          *temp_ptr;
UINT            source_port;
NX_INTERFACE   *nx_interface;
#ifndef NX_DISABLE_IPV4
NX_IPV4_HEADER *ipv4_header;
#endif /* !NX_DISABLE_IPV4  */
#ifdef TX_ENABLE_EVENT_TRACE
ULONG           address = 0;
#endif  /* TX_ENABLE_EVENT_TRACE */
#ifdef FEATURE_NX_IPV6
NX_IPV6_HEADER *ipv6_header;
#endif /* FEATURE_NX_IPV6 */


    if (ip_address)
    {

#ifndef NX_DISABLE_IPV4
        if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
        {

            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            ipv4_header = (NX_IPV4_HEADER *)packet_ptr -> nx_packet_ip_header;

            ip_address -> nxd_ip_version = NX_IP_VERSION_V4;

            /* At this point, the IP address in the IPv4 header is in host byte order. */
            ip_address -> nxd_ip_address.v4 = ipv4_header -> nx_ip_header_source_ip;

#ifdef TX_ENABLE_EVENT_TRACE
            address = ip_address -> nxd_ip_address.v4;
#endif  /* TX_ENABLE_EVENT_TRACE */
        }
        else
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
        if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
        {

            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            ipv6_header = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

            ip_address -> nxd_ip_version = NX_IP_VERSION_V6;

            /* At this point, the IP address in the IPv6 header is in host byte order. */
            COPY_IPV6_ADDRESS(ipv6_header -> nx_ip_header_source_ip, ip_address -> nxd_ip_address.v6);

#ifdef TX_ENABLE_EVENT_TRACE
            address = ip_address -> nxd_ip_address.v6[3];
#endif /* TX_ENABLE_EVENT_TRACE*/
        }
        else
#endif /* FEATURE_NX_IPV6 */
        {

            /* Invalid IP version . */
            return(NX_INVALID_PACKET);
        }
    }

    /* Build an address to the current top of the packet.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    temp_ptr =  (ULONG *)packet_ptr -> nx_packet_prepend_ptr;

    /* Pickup the source port.  */
    source_port =  (UINT)(*(temp_ptr - 2) >> NX_SHIFT_BY_16);
    if (port != NX_NULL)
    {
        *port = source_port;
    }

    if (protocol != NX_NULL)
    {
        *protocol = 0x11;
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_UDP_SOURCE_EXTRACT, packet_ptr, address, source_port, 0, NX_TRACE_PACKET_EVENTS, 0, 0);

    if (interface_index == NX_NULL)
    {
        return(NX_SUCCESS);
    }

    /* Search for interface index number.  Initialize interface value as
       invalid (0xFFFFFFFF).  Once we find valid interface, we will update
       the returned value. */
    *interface_index = 0xFFFFFFFF;

    if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
    {
        nx_interface = packet_ptr -> nx_packet_address.nx_packet_interface_ptr;
    }
#ifdef FEATURE_NX_IPV6
    else if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
    {
        if (packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr == NX_NULL)
        {

            /* No interface attached.  Done here, and return success. */
            return(NX_SUCCESS);
        }
        else
        {
            nx_interface = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached;
        }
    }
#endif /* FEATURE_NX_IPV6 */
    else
    {
        return(NX_SUCCESS);
    }

    if (nx_interface == NX_NULL)
    {

        /* No interface attached.  Done here, and return success. */
        return(NX_SUCCESS);
    }

    *interface_index = (UINT)nx_interface -> nx_interface_index;

    return(NX_SUCCESS);
}

