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
/*    _nx_tcp_socket_driver_establish                     PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function is invoked when a TCP connection is established.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to owning socket      */
/*    interface_ptr                         Pointer to IP interface       */
/*    remote_port                           Pointer to remote UDP port    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    Status                                                              */
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
UINT _nx_tcp_socket_driver_establish(NX_TCP_SOCKET *socket_ptr, NX_INTERFACE *interface_ptr, UINT remote_port)
{
NX_IP *ip_ptr;
NXD_ADDRESS ip_address;
UINT status;
UINT index;
UINT port;
struct NX_TCP_LISTEN_STRUCT *listen_ptr;

    /* Setup the IP pointer.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;
        
    /* Obtain the IP internal mutex before processing the IP event.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Check socket state first.  */
    if ((socket_ptr -> nx_tcp_socket_state != NX_TCP_SYN_SENT) &&
        (socket_ptr -> nx_tcp_socket_state != NX_TCP_LISTEN_STATE)&&
        (socket_ptr -> nx_tcp_socket_state != NX_TCP_SYN_RECEIVED))
    {

        /* Just ignore socket not in connecting state.  */
        /* Release the IP internal mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));
        return(NX_NOT_SUCCESSFUL);
    }

    if (socket_ptr -> nx_tcp_socket_client_type == NX_FALSE)
    {
        if (socket_ptr -> nx_tcp_socket_bound_next)
        {

            /* TCP socket is connecting on other interface.  */
            /* Release the IP internal mutex.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));
            return(NX_NOT_SUCCESSFUL);
        }

        /* For TCP server socket, the connection is not established yet.  */
        port = socket_ptr -> nx_tcp_socket_port;
        status = interface_ptr -> nx_interface_tcpip_offload_handler(ip_ptr, interface_ptr, socket_ptr,
                                                                     NX_TCPIP_OFFLOAD_TCP_SERVER_SOCKET_ACCEPT,
                                                                     NX_NULL, NX_NULL, &ip_address, port,
                                                                     &remote_port, NX_NO_WAIT);

        if (status)
        {

            /* This should not happen as a connection is pendding. */
            /* Release the IP internal mutex.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));
            return(NX_TCPIP_OFFLOAD_ERROR);
        }

        /* Setup connection parameters.  */
        socket_ptr -> nx_tcp_socket_connect_interface = interface_ptr;

        /* Save the server port and server IP address.  */
        socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version = ip_address.nxd_ip_version;
#ifndef NX_DISABLE_IPV4
        if (ip_address.nxd_ip_version == NX_IP_VERSION_V4)
        {
            socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v4 = ip_address.nxd_ip_address.v4;
            socket_ptr -> nx_tcp_socket_mss =
                (ULONG)((interface_ptr -> nx_interface_ip_mtu_size - sizeof(NX_IPV4_HEADER)) - sizeof(NX_TCP_HEADER));
        }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
        if (ip_address.nxd_ip_version == NX_IP_VERSION_V6)
        {
            COPY_IPV6_ADDRESS(ip_address.nxd_ip_address.v6,
                              socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v6);
            socket_ptr -> nx_tcp_socket_mss =
                (ULONG)((interface_ptr -> nx_interface_ip_mtu_size - sizeof(NX_IPV6_HEADER)) - sizeof(NX_TCP_HEADER));
        }
#endif /* FEATURE_NX_IPV6 */

        socket_ptr -> nx_tcp_socket_connect_port = remote_port;

        /* Find listen callback function.  */
        listen_ptr =  ip_ptr -> nx_ip_tcp_active_listen_requests;
        if (listen_ptr)
        {
            do
            {
                if (listen_ptr -> nx_tcp_listen_port == port)
                {
                    if (listen_ptr -> nx_tcp_listen_callback)
                    {

                        /* Clear the server socket pointer in the listen request.  If the
                           application wishes to honor more server connections on this port,
                           the application must call relisten with a new server socket
                           pointer.  */
                        listen_ptr->nx_tcp_listen_socket_ptr = NX_NULL;
                        
                        /* Call the user's listen callback function.  */
                        (listen_ptr ->nx_tcp_listen_callback)(socket_ptr, listen_ptr -> nx_tcp_listen_port);
                        break;
                    }
                }

                /* Move to the next listen request.  */
                listen_ptr =  listen_ptr -> nx_tcp_listen_next;
            } while (listen_ptr != ip_ptr -> nx_ip_tcp_active_listen_requests);
        }

        /* Calculate the hash index in the TCP port array of the associated IP instance.  */
        index = (UINT)((port + (port >> 8)) & NX_TCP_PORT_TABLE_MASK);

        /* Determine if the list is NULL.  */
        if (ip_ptr -> nx_ip_tcp_port_table[index])
        {

            /* There are already sockets on this list... just add this one
                to the end.  */
            socket_ptr -> nx_tcp_socket_bound_next =
                ip_ptr -> nx_ip_tcp_port_table[index];
            socket_ptr -> nx_tcp_socket_bound_previous =
                (ip_ptr -> nx_ip_tcp_port_table[index]) -> nx_tcp_socket_bound_previous;
            ((ip_ptr -> nx_ip_tcp_port_table[index]) -> nx_tcp_socket_bound_previous) -> nx_tcp_socket_bound_next =
                socket_ptr;
            (ip_ptr -> nx_ip_tcp_port_table[index]) -> nx_tcp_socket_bound_previous = socket_ptr;
        }
        else
        {

            /* Nothing is on the TCP port list.  Add this TCP socket to an
                empty list.  */
            socket_ptr -> nx_tcp_socket_bound_next =      socket_ptr;
            socket_ptr -> nx_tcp_socket_bound_previous =  socket_ptr;
            ip_ptr -> nx_ip_tcp_port_table[index] =       socket_ptr;
        }
    }
    
    /* Update socket state.  */
    socket_ptr -> nx_tcp_socket_state =  NX_TCP_ESTABLISHED;

#ifndef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT

    /* Is a connection completion callback registered with the TCP socket?  */
    if (socket_ptr -> nx_tcp_establish_notify)
    {

        /* Call the application's establish callback function.    */
        (socket_ptr -> nx_tcp_establish_notify)(socket_ptr);
    }
#endif

    /* Determine if we need to wake a thread suspended on the connection.  */
    if (socket_ptr -> nx_tcp_socket_connect_suspended_thread)
    {

        /* Resume the suspended thread.  */
        _nx_tcp_socket_thread_resume(&(socket_ptr -> nx_tcp_socket_connect_suspended_thread), NX_SUCCESS);
    }
    
    /* Release the IP internal mutex.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    return(NX_SUCCESS);
}
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

