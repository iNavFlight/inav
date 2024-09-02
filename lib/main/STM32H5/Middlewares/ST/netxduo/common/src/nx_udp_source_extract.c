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



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_udp_socket_extract                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function extracts the source IP and UDP port number from the   */
/*    supplied packet.                                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to UDP packet pointer */
/*    ip_address                            Pointer to packet source IP   */
/*                                            address                     */
/*    port                                  Pointer to source UDP port    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
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
UINT  _nx_udp_source_extract(NX_PACKET *packet_ptr, ULONG *ip_address, UINT *port)
{

#ifndef NX_DISABLE_IPV4
ULONG          *temp_ptr;
NX_IPV4_HEADER *ipv4_header;

    /* Build an address to the current top of the packet.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    temp_ptr =  (ULONG *)packet_ptr -> nx_packet_prepend_ptr;

    /* Pickup the source port.  */
    *port =  (UINT)(*(temp_ptr - 2) >> NX_SHIFT_BY_16);

    /* Obtain the IPv4 header. */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    ipv4_header = (NX_IPV4_HEADER *)packet_ptr -> nx_packet_ip_header;

    /* Pickup the source IP address.  */
    *ip_address =  ipv4_header -> nx_ip_header_source_ip;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_UDP_SOURCE_EXTRACT, packet_ptr, *ip_address, *port, 0, NX_TRACE_UDP_EVENTS, 0, 0);

    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(packet_ptr);
    NX_PARAMETER_NOT_USED(ip_address);
    NX_PARAMETER_NOT_USED(port);

    return(NX_NOT_SUPPORTED);
#endif /* !NX_DISABLE_IPV4  */
}

