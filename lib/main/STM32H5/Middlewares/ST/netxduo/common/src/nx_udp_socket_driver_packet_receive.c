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
#include "tx_thread.h"
#include "nx_udp.h"
#include "nx_packet.h"
#include "nx_ip.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif /* FEATURE_NX_IPV6 */


#ifdef NX_ENABLE_TCPIP_OFFLOAD
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_udp_socket_driver_packet_receive                PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives a UDP packet from the TCP/IP driver and add  */
/*    fake TCP/IP header. Then pass the packet to IP deferred receive     */
/*    routine.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to owning socket      */
/*    packet_ptr                            Pointer to packet to process  */
/*    local_ip                              Pointer to local IP address   */
/*    remote_ip                             Pointer to remote IP address  */
/*    remote_port                           Pointer to remote UDP port    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
/*    _nx_ip_packet_deferred_receive        Defer IP packet receive       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Driver                                                              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-02-2021     Yuxin Zhou               Initial Version 6.1.8         */
/*                                                                        */
/**************************************************************************/
VOID _nx_udp_socket_driver_packet_receive(NX_UDP_SOCKET *socket_ptr, NX_PACKET *packet_ptr,
                                          NXD_ADDRESS *local_ip, NXD_ADDRESS *remote_ip, UINT remote_port)
{

NX_IP          *ip_ptr;
NX_UDP_HEADER  *udp_header_ptr;

    /* Setup the IP pointer.  */
    ip_ptr =  socket_ptr -> nx_udp_socket_ip_ptr;

    if (packet_ptr == NX_NULL)
    {

        /* Socket error. Just ignore.  */
        return;
    }

    /* Fake UDP and IP header.  */
    /* Prepend the UDP header to the packet.  First, make room for the UDP header.  */
    packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_UDP_HEADER);

    /* Increase the packet length.  */
    packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length + (ULONG)sizeof(NX_UDP_HEADER);

    /* Setup the UDP header pointer.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    udp_header_ptr =  (NX_UDP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

    /* Build the first 32-bit word of the UDP header.  */
    udp_header_ptr -> nx_udp_header_word_0 =
        ((ULONG)remote_port << NX_SHIFT_BY_16) | (ULONG)socket_ptr -> nx_udp_socket_port;

    /* Build the second 32-bit word of the UDP header.  */
    udp_header_ptr -> nx_udp_header_word_1 =  (packet_ptr -> nx_packet_length << NX_SHIFT_BY_16);

    /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
    swap the endian of the UDP header.  */
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);

#ifndef NX_DISABLE_IPV4
    if (remote_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        _nx_ip_header_add(ip_ptr, packet_ptr, remote_ip -> nxd_ip_address.v4,
                          local_ip -> nxd_ip_address.v4,
                          socket_ptr -> nx_udp_socket_type_of_service,
                          socket_ptr -> nx_udp_socket_time_to_live,
                          NX_IP_UDP,
                          socket_ptr -> nx_udp_socket_fragment_enable);
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (remote_ip -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        if (_nx_ipv6_header_add(ip_ptr, &packet_ptr,
                                NX_PROTOCOL_UDP,
                                packet_ptr -> nx_packet_length,
                                ip_ptr -> nx_ipv6_hop_limit,
                                remote_ip -> nxd_ip_address.v6,
                                local_ip -> nxd_ip_address.v6,
                                NX_NULL))
        {
        
            /* Release the IP internal mutex.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));
            return;
        }
    }
#endif /* FEATURE_NX_IPV6 */

    _nx_ip_packet_deferred_receive(ip_ptr, packet_ptr);
}
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

