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
/**   Transmission Control Protocol (TCP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_tcp.h"
#include "nx_packet.h"
#include "tx_thread.h"
#include "nx_ip.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif /* FEATURE_NX_IPV6 */

#ifdef NX_ENABLE_TCPIP_OFFLOAD
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_driver_packet_receive                PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function receives a TCP packet from the TCP/IP driver and add  */
/*    fake TCP/IP header. Then pass the packet to IP deferred receive     */
/*    routine.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to owning socket      */
/*    packet_ptr                            Pointer to packet to process  */
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
VOID _nx_tcp_socket_driver_packet_receive(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr)
{
NX_IP *ip_ptr;
NX_TCP_HEADER *header_ptr;

    /* Setup the IP pointer.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;
        
    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    if (packet_ptr == NX_NULL)
    {

        /* Connection closed.  */
        if (socket_ptr -> nx_tcp_socket_state == NX_TCP_ESTABLISHED)
        {
            socket_ptr -> nx_tcp_socket_state =  NX_TCP_CLOSE_WAIT;

            /* Loop to release all threads suspended while trying to receive on the socket.  */
            while (socket_ptr -> nx_tcp_socket_receive_suspension_list)
            {

                /* Release the head of the receive suspension list. */
                _nx_tcp_receive_cleanup(socket_ptr -> nx_tcp_socket_receive_suspension_list NX_CLEANUP_ARGUMENT);
            }

            /* If given, call the application's disconnect callback function
            for disconnect.  */
            if (socket_ptr -> nx_tcp_disconnect_callback)
            {

                /* Call the application's disconnect handling function.  It is
                responsible for calling the socket disconnect function.  */
                (socket_ptr -> nx_tcp_disconnect_callback)(socket_ptr);
            }
        }
        
        /* Release the IP internal mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));
        return;
    }

    /* Fake TCP and IP header.  */
    /* Prepend the TCP header to the packet.  First, make room for the TCP header.  */
    packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_TCP_HEADER);

    /* Add the length of the TCP header.  */
    packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length + (ULONG)sizeof(NX_TCP_HEADER);

    /* Pickup the pointer to the head of the TCP packet.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    header_ptr =  (NX_TCP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

    /* Build the output request in the TCP header.  */
    header_ptr -> nx_tcp_header_word_0 = (((ULONG)(socket_ptr -> nx_tcp_socket_connect_port)) << NX_SHIFT_BY_16) |
                                          (ULONG)socket_ptr -> nx_tcp_socket_port;
    NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_0);
    header_ptr -> nx_tcp_acknowledgment_number = 0;
    header_ptr -> nx_tcp_sequence_number = 0;
    header_ptr -> nx_tcp_header_word_3 = NX_TCP_HEADER_SIZE | NX_TCP_ACK_BIT | NX_TCP_PSH_BIT;
    NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_3);
    header_ptr -> nx_tcp_header_word_4 = 0;

#ifndef NX_DISABLE_IPV4
    if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
    {
        _nx_ip_header_add(ip_ptr, packet_ptr, socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v4,
                          packet_ptr -> nx_packet_ip_interface -> nx_interface_ip_address,
                          socket_ptr -> nx_tcp_socket_type_of_service,
                          socket_ptr -> nx_tcp_socket_time_to_live,
                          NX_IP_TCP,
                          socket_ptr -> nx_tcp_socket_fragment_enable);
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
    {
        if (_nx_ipv6_header_add(ip_ptr, &packet_ptr,
                                NX_PROTOCOL_TCP,
                                packet_ptr -> nx_packet_length,
                                ip_ptr -> nx_ipv6_hop_limit,
                                socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v6,
                                socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address,
                                NX_NULL))
        {
        
            /* Release the IP internal mutex.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));
            return;
        }
    }
#endif /* FEATURE_NX_IPV6 */
        
    /* Release the IP internal mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    _nx_ip_packet_deferred_receive(ip_ptr, packet_ptr);    
}
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

