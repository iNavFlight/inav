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
#include "tx_thread.h"
#include "nx_ipv6.h"
#include "nx_ipv4.h"
#include "nx_ip.h"
#ifdef NX_ENABLE_HTTP_PROXY
#include "nx_http_proxy_client.h"
#endif /* NX_ENABLE_HTTP_PROXY */


#ifdef NX_ENABLE_TCPIP_OFFLOAD
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_tcp_client_socket_driver_connect               PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the connect request for TCP/IP offload        */
/*    interface.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP client socket  */
/*    server_ip                             IP address of server          */
/*    server_port                           Port number of server         */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection             */
/*    tx_mutex_put                          Release protection            */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nxd_tcp_client_socket_connect                                      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-02-2021     Yuxin Zhou               Initial Version 6.1.8         */
/*                                                                        */
/**************************************************************************/
static UINT _nxd_tcp_client_socket_driver_connect(NX_TCP_SOCKET *socket_ptr,
                                                  NXD_ADDRESS *server_ip,
                                                  UINT server_port,
                                                  ULONG wait_option)
{
UINT          status;
NX_INTERFACE *interface_ptr;
NX_IP        *ip_ptr;


    /* Setup the pointer to the associated IP instance.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;

    /* Clear the socket timeout.  */
    socket_ptr -> nx_tcp_socket_timeout =  0;

    /* Let TCP/IP offload interface make the connection.  */
    interface_ptr = socket_ptr -> nx_tcp_socket_connect_interface;
    status = interface_ptr -> nx_interface_tcpip_offload_handler(ip_ptr, interface_ptr, socket_ptr,
                                                                 NX_TCPIP_OFFLOAD_TCP_CLIENT_SOCKET_CONNECT,
                                                                 NX_NULL, NX_NULL, server_ip,
                                                                 socket_ptr -> nx_tcp_socket_port,
                                                                 &server_port, wait_option);
    if ((status == NX_SUCCESS) || (status == NX_IN_PROGRESS))
    {

        /* Set MSS.  */
#ifndef NX_DISABLE_IPV4
        if (server_ip -> nxd_ip_version == NX_IP_VERSION_V4)
        {
            socket_ptr -> nx_tcp_socket_mss =
                (ULONG)((interface_ptr -> nx_interface_ip_mtu_size - sizeof(NX_IPV4_HEADER)) - sizeof(NX_TCP_HEADER));
        }
#endif /* !NX_DISABLE_IPV4  */
#ifdef FEATURE_NX_IPV6
        if (server_ip -> nxd_ip_version == NX_IP_VERSION_V6)
        {
            socket_ptr -> nx_tcp_socket_mss =
                (ULONG)((interface_ptr -> nx_interface_ip_mtu_size - sizeof(NX_IPV6_HEADER)) - sizeof(NX_TCP_HEADER));
        }
#endif /* FEATURE_NX_IPV6 */
        socket_ptr -> nx_tcp_socket_connect_mss = socket_ptr -> nx_tcp_socket_mss;
        socket_ptr -> nx_tcp_socket_peer_mss = socket_ptr -> nx_tcp_socket_mss;

        if (status == NX_SUCCESS)
        {

            /* Connected to server.  */
            socket_ptr -> nx_tcp_socket_state =  NX_TCP_ESTABLISHED;
#ifndef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT

            /* Is a connection completion callback registered with the TCP socket?  */
            if (socket_ptr -> nx_tcp_establish_notify)
            {

                /* Call the application's establish callback function.    */
                (socket_ptr -> nx_tcp_establish_notify)(socket_ptr);
            }
#endif

            /* Release the IP protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));
            return(NX_SUCCESS);
        }
        else
        {

            /* Connected to server.  */
            socket_ptr -> nx_tcp_socket_state =  NX_TCP_SYN_SENT;
        }
    }
    else
    {

        /* Unable to connect to server.  */

#ifndef NX_DISABLE_TCP_INFO

        /* Reduce the active connections count.  */
        ip_ptr -> nx_ip_tcp_active_connections--;

        /* Reduce the TCP connections count.  */
        ip_ptr -> nx_ip_tcp_connections--;
#endif

        /* Restore the socket state. */
        socket_ptr -> nx_tcp_socket_state = NX_TCP_CLOSED;

        /* Reset server port and server IP address. */
        memset(&socket_ptr -> nx_tcp_socket_connect_ip, 0, sizeof(NXD_ADDRESS));
        socket_ptr -> nx_tcp_socket_connect_port = 0;

        /* Reset the next_hop_address information. */
        socket_ptr -> nx_tcp_socket_next_hop_address = 0;

        /* Release the IP protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));
        return(NX_TCPIP_OFFLOAD_ERROR);
    }

    /* Release the IP protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));
    return(NX_SUCCESS);
}
#endif /* NX_ENABLE_TCPIP_OFFLOAD */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_tcp_client_socket_connect                     PORTABLE C       */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the connect request for the supplied socket.  */
/*    If bound and not connected, this function will send the first SYN   */
/*    message to the specified server to initiate the connection process. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP client socket  */
/*    server_ip                             IP address of server          */
/*    server_port                           Port number of server         */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_socket_thread_suspend         Suspend thread for connection */
/*    _nx_tcp_packet_send_syn               Send SYN packet               */
/*    _nx_ip_route_find                     Find a suitable outgoing      */
/*                                            interface.                  */
/*    tx_mutex_get                          Obtain protection             */
/*    tx_mutex_put                          Release protection            */
/*    _nx_http_proxy_client_initialize      Initialize the HTTP Proxy     */
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
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported TCP/IP offload,   */
/*                                            resulting in version 6.1.8  */
/*  10-31-2022     Wenhui Xie               Modified comment(s), and      */
/*                                            supported HTTP Proxy,       */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT  _nxd_tcp_client_socket_connect(NX_TCP_SOCKET *socket_ptr,
                                     NXD_ADDRESS *server_ip,
                                     UINT server_port,
                                     ULONG wait_option)
{

UINT          ip_header_size = 0;
NX_IP        *ip_ptr;
NX_INTERFACE *outgoing_interface = NX_NULL;

#ifdef FEATURE_NX_IPV6
UINT          status;
#endif /* FEATURE_NX_IPV6 */

#ifdef TX_ENABLE_EVENT_TRACE
ULONG         ip_address_log = 0;
#endif /* TX_ENABLE_EVENT_TRACE */

    /* Setup IP pointer.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;

#ifdef NX_ENABLE_HTTP_PROXY
    if (ip_ptr -> nx_ip_http_proxy_enable)
    {

        /* Initialize the HTTP Proxy info and replace the peer IP and port with HTTP proxy server's IP and port.  */
        _nx_http_proxy_client_initialize(socket_ptr, &server_ip, &server_port);
    }
#endif /* NX_ENABLE_HTTP_PROXY */

    /* Make sure the server IP address is accesible. */
#ifndef NX_DISABLE_IPV4
    if (server_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        if (_nx_ip_route_find(ip_ptr, server_ip -> nxd_ip_address.v4, &outgoing_interface, &socket_ptr -> nx_tcp_socket_next_hop_address) != NX_SUCCESS)
        {
            /* Return an IP address error code.  */
            return(NX_IP_ADDRESS_ERROR);
        }
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    /* For IPv6 connections, find a suitable outgoing interface, based on the TCP peer address. */
    if (server_ip -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        status = _nxd_ipv6_interface_find(ip_ptr, server_ip -> nxd_ip_address.v6,
                                          &socket_ptr -> nx_tcp_socket_ipv6_addr,
                                          NX_NULL);

        if (status != NX_SUCCESS)
        {
            return(status);
        }

        outgoing_interface = socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address_attached;
    }
#endif /* FEATURE_NX_IPV6 */

#ifdef TX_ENABLE_EVENT_TRACE
#ifndef NX_DISABLE_IPV4
    if (server_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        ip_address_log = server_ip -> nxd_ip_address.v4;
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (server_ip -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        ip_address_log = server_ip -> nxd_ip_address.v6[3];
    }
#endif /* FEATURE_NX_IPV6 */

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_CLIENT_SOCKET_CONNECT, ip_ptr, socket_ptr, ip_address_log, server_port, NX_TRACE_TCP_EVENTS, 0, 0);
#endif /* TX_ENABLE_EVENT_TRACE */

    /* Obtain the IP mutex so we initiate the connect.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if the socket has already been bound to port or if a socket bind is
       already pending from another thread.  */
    if (!socket_ptr -> nx_tcp_socket_bound_next)
    {

        /* Release protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return a not bound error code.  */
        return(NX_NOT_BOUND);
    }

    /* Determine if the socket is in a pre-connection state.  */
    if ((socket_ptr -> nx_tcp_socket_state != NX_TCP_CLOSED) && (socket_ptr -> nx_tcp_socket_state != NX_TCP_TIMED_WAIT))
    {

        /* Release protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return a not closed error code.  */
        return(NX_NOT_CLOSED);
    }

#ifndef NX_DISABLE_TCP_INFO

    /* Increment the active connections count.  */
    ip_ptr -> nx_ip_tcp_active_connections++;

    /* Increment the TCP connections count.  */
    ip_ptr -> nx_ip_tcp_connections++;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_SYN_SENT, NX_TRACE_INTERNAL_EVENTS, 0, 0);

    /* Move the TCP state to Sequence Sent, the next state of an active open.  */
    socket_ptr -> nx_tcp_socket_state =  NX_TCP_SYN_SENT;

    /* Save the server port and server IP address.  */
    socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version       = server_ip -> nxd_ip_version;

#ifndef NX_DISABLE_IPV4
    if (server_ip -> nxd_ip_version == NX_IP_VERSION_V4)
    {

        /* copy the IPv4 address */
        socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v4 = server_ip -> nxd_ip_address.v4;

        ip_header_size = (UINT)sizeof(NX_IPV4_HEADER);
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (server_ip -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        COPY_IPV6_ADDRESS(server_ip -> nxd_ip_address.v6,
                          socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v6);

        ip_header_size = (UINT)sizeof(NX_IPV6_HEADER);
    }
#endif /* FEATURE_NX_IPV6 */

    socket_ptr -> nx_tcp_socket_connect_port = server_port;

    /* Outgoing interface must not be null. */
    NX_ASSERT(outgoing_interface != NX_NULL);

    /* Initialize the maximum segment size based on the interface MTU. */
    /*lint -e{644} suppress variable might not be initialized, since "outgoing_interface" was initialized by _nx_ip_route_find or _nxd_ipv6_interface_find. */
    if (outgoing_interface -> nx_interface_ip_mtu_size < (ip_header_size + NX_TCP_SYN_SIZE))
    {

        /* Interface MTU size is smaller than IP and TCP header size.  Invalid interface! */

#ifndef NX_DISABLE_TCP_INFO

        /* Reduce the active connections count.  */
        ip_ptr -> nx_ip_tcp_active_connections--;

        /* Reduce the TCP connections count.  */
        ip_ptr -> nx_ip_tcp_connections--;
#endif

        /* Restore the socket state. */
        socket_ptr -> nx_tcp_socket_state = NX_TCP_CLOSED;

        /* Reset server port and server IP address. */
        memset(&socket_ptr -> nx_tcp_socket_connect_ip, 0, sizeof(NXD_ADDRESS));
        socket_ptr -> nx_tcp_socket_connect_port = 0;

        /* Reset the next_hop_address information. */
        socket_ptr -> nx_tcp_socket_next_hop_address = 0;

        /* Release protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));


        /* Return an IP address error code.  */
        return(NX_INVALID_INTERFACE);
    }

    socket_ptr -> nx_tcp_socket_connect_interface = outgoing_interface;

    /* Setup the initial sequence number.  */
    if (socket_ptr -> nx_tcp_socket_tx_sequence == 0)
    {
        socket_ptr -> nx_tcp_socket_tx_sequence =  (((ULONG)NX_RAND()) << NX_SHIFT_BY_16) & 0xFFFFFFFF;
        socket_ptr -> nx_tcp_socket_tx_sequence |= (ULONG)NX_RAND();
    }
    else
    {
        socket_ptr -> nx_tcp_socket_tx_sequence =  socket_ptr -> nx_tcp_socket_tx_sequence + ((ULONG)(((ULONG)0x10000))) + ((ULONG)NX_RAND());
    }

    /* Ensure the rx window size logic is reset.  */
    socket_ptr -> nx_tcp_socket_rx_window_current =    socket_ptr -> nx_tcp_socket_rx_window_default;
    socket_ptr -> nx_tcp_socket_rx_window_last_sent =  socket_ptr -> nx_tcp_socket_rx_window_default;

    /* Clear the FIN received flag.  */
    socket_ptr -> nx_tcp_socket_fin_received =  NX_FALSE;
    socket_ptr -> nx_tcp_socket_fin_acked =  NX_FALSE;

    /* Increment the sequence number.  */
    socket_ptr -> nx_tcp_socket_tx_sequence++;

    /* Setup a timeout so the connection attempt can be sent again.  */
    socket_ptr -> nx_tcp_socket_timeout =          socket_ptr -> nx_tcp_socket_timeout_rate;
    socket_ptr -> nx_tcp_socket_timeout_retries =  0;

    /* CLEANUP: In case any existing packets on socket's receive queue.  */
    if (socket_ptr -> nx_tcp_socket_receive_queue_count)
    {

        /* Remove all packets on the socket's receive queue.  */
        _nx_tcp_socket_receive_queue_flush(socket_ptr);
    }

    /* CLEANUP: Clean up any existing socket data before making a new connection. */
    socket_ptr -> nx_tcp_socket_tx_window_congestion = 0;
    socket_ptr -> nx_tcp_socket_tx_outstanding_bytes = 0;
    socket_ptr -> nx_tcp_socket_packets_sent         = 0;
    socket_ptr -> nx_tcp_socket_bytes_sent           = 0;
    socket_ptr -> nx_tcp_socket_packets_received     = 0;
    socket_ptr -> nx_tcp_socket_bytes_received       = 0;
    socket_ptr -> nx_tcp_socket_retransmit_packets   = 0;
    socket_ptr -> nx_tcp_socket_checksum_errors      = 0;
    socket_ptr -> nx_tcp_socket_transmit_sent_head   = NX_NULL;
    socket_ptr -> nx_tcp_socket_transmit_sent_tail   = NX_NULL;
    socket_ptr -> nx_tcp_socket_transmit_sent_count  = 0;
    socket_ptr -> nx_tcp_socket_receive_queue_count  = 0;
    socket_ptr -> nx_tcp_socket_receive_queue_head   = NX_NULL;
    socket_ptr -> nx_tcp_socket_receive_queue_tail   = NX_NULL;

#ifdef NX_ENABLE_TCPIP_OFFLOAD
    if ((outgoing_interface -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD) &&
        (outgoing_interface -> nx_interface_tcpip_offload_handler))
    {
        
        /* This interface supports TCP/IP offload.  */
        return(_nxd_tcp_client_socket_driver_connect(socket_ptr, server_ip, server_port, wait_option));
    }
    else
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
    {

        /* Send the SYN message.  */
        _nx_tcp_packet_send_syn(socket_ptr, (socket_ptr -> nx_tcp_socket_tx_sequence - 1));
    }

#ifdef NX_ENABLE_HTTP_PROXY
    if (ip_ptr -> nx_ip_http_proxy_enable)
    {

        /* Set HTTP Proxy state as waiting for TCP connection.  */
        socket_ptr -> nx_tcp_socket_http_proxy_state = NX_HTTP_PROXY_STATE_WAITING;
    }
#endif /* NX_ENABLE_HTTP_PROXY */

    /* Optionally suspend the thread.  If timeout occurs, return a connection timeout status.  If
       immediate response is selected, return a connection in progress status.  Only on a real
       connection should success be returned.  */
    if ((wait_option) && (_tx_thread_current_ptr != &(ip_ptr -> nx_ip_thread)))
    {

        /* Suspend the thread on this socket's connection attempt.  */
        /* Note: the IP protection mutex is released inside _nx_tcp_socket_thread_suspend().  */

        _nx_tcp_socket_thread_suspend(&(socket_ptr -> nx_tcp_socket_connect_suspended_thread), _nx_tcp_connect_cleanup, socket_ptr, &(ip_ptr -> nx_ip_protection), wait_option);

        /* Just return the completion code.  */
        return(_tx_thread_current_ptr -> tx_thread_suspend_status);
    }
    else
    {

        /* No suspension is request, just release protection and return to the caller.  */

        /* Release the IP protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return in-progress completion status.  */
        return(NX_IN_PROGRESS);
    }
}

