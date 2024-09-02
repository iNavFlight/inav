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
#include "nx_ip.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif /* FEATURE_NX_IPV6 */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_server_socket_relisten                      PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function registers another server socket for an existing       */
/*    listen request on the specified TCP port.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    port                                  TCP port number               */
/*    socket_ptr                            Server socket pointer         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_SUCCESS                            Successful completion status  */
/*    NX_CONNECTION_PENDING                 Connection already underway   */
/*                                           (this is a successful status)*/
/*    NX_INVALID_RELISTEN                   No active listen requests     */
/*    NX_NOT_CLOSED                         Socket state not closed       */
/*    NX_ALREADY_BOUND                      Socket already bound to port  */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release packet                */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
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
/*                                                                        */
/**************************************************************************/
UINT  _nx_tcp_server_socket_relisten(NX_IP *ip_ptr, UINT port, NX_TCP_SOCKET *socket_ptr)
{

UINT                         index;
struct NX_TCP_LISTEN_STRUCT *listen_ptr;
NX_PACKET                   *packet_ptr;
NX_TCP_HEADER               *tcp_header_ptr;
NXD_ADDRESS                  source_ip;
UINT                         source_port;
ULONG                        mss = 0;
ULONG                        option_words;
#ifndef NX_DISABLE_IPV4
NX_IPV4_HEADER              *ipv4_header_ptr;
#endif /* !NX_DISABLE_IPV4  */
#ifdef NX_ENABLE_TCP_WINDOW_SCALING
ULONG                        rwin_scale = 0;
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */
VOID                         (*listen_callback)(NX_TCP_SOCKET *socket_ptr, UINT port);


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_SERVER_SOCKET_RELISTEN, ip_ptr, port, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Obtain the IP protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if the server socket is in a proper state.  */
    if (socket_ptr -> nx_tcp_socket_state != NX_TCP_CLOSED)
    {

        /* Release the protection mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return the not closed error code.  */
        return(NX_NOT_CLOSED);
    }

    /* Determine if the server socket has already been bound to port or if a socket bind is
       already pending from another thread.  */
    if ((socket_ptr -> nx_tcp_socket_bound_next) ||
        (socket_ptr -> nx_tcp_socket_bind_in_progress))
    {

        /* Release the protection mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return an already bound error code.  */
        return(NX_ALREADY_BOUND);
    }

    /* Search through the active listen requests to see if there is already
       one active.  */
    listen_ptr =  ip_ptr -> nx_ip_tcp_active_listen_requests;
    if (listen_ptr)
    {

        /* Search the active listen requests for this port.  */
        do
        {

            /* Determine if there is another listen request for the same port.  */
            if ((listen_ptr -> nx_tcp_listen_port == port) &&
                (!listen_ptr -> nx_tcp_listen_socket_ptr))
            {

                /* Yes, a listen request was found for this port, with an empty
                   socket designation.  */

#ifdef NX_ENABLE_TCPIP_OFFLOAD
                /* Listen to TCP/IP offload interfaces.  */
                if (_nx_tcp_server_socket_driver_listen(ip_ptr, port, socket_ptr))
                {

                    /* Listen request failure, release the protection.  */
                    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                    /* Return an already bound error code.  */
                    return(NX_TCPIP_OFFLOAD_ERROR);
                }
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

                /* Determine if there are any connection requests queued up.  */
                if (!listen_ptr -> nx_tcp_listen_queue_current)
                {

                    /* Nothing is queued up, so simply store the new socket
                       in the listen structure.  */

                    /* Place this socket in the listen structure.  */
                    listen_ptr -> nx_tcp_listen_socket_ptr =  socket_ptr;

                    /* Indicate this socket is a server socket.  */
                    socket_ptr -> nx_tcp_socket_client_type =  NX_FALSE;

                    /* Clean connected interface. */
                    socket_ptr -> nx_tcp_socket_connect_interface = NX_NULL;

                    /* If trace is enabled, insert this event into the trace buffer.  */
                    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_LISTEN_STATE, NX_TRACE_INTERNAL_EVENTS, 0, 0);

                    /* Move to the listen state.  */
                    socket_ptr -> nx_tcp_socket_state =  NX_TCP_LISTEN_STATE;

                    /* Remember what port is associated for this socket.  */
                    socket_ptr -> nx_tcp_socket_port =  port;

                    /* Release the protection.  */
                    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                    /* Return success.  */
                    return(NX_SUCCESS);
                }
                else
                {

                    /* There is one or more connection requests queued up.  Remove
                       the first (oldest) connection request and setup the current
                       socket.  */

                    /* First, remove the first queued connection request.  */
                    packet_ptr =  listen_ptr -> nx_tcp_listen_queue_head;
                    listen_ptr -> nx_tcp_listen_queue_head =  packet_ptr -> nx_packet_queue_next;

                    /* Determine if the tail needs to be adjusted.  */
                    if (packet_ptr == listen_ptr -> nx_tcp_listen_queue_tail)
                    {
                        listen_ptr -> nx_tcp_listen_queue_tail =  NX_NULL;
                    }

                    /* Decrease the total number of connections queued.  */
                    listen_ptr -> nx_tcp_listen_queue_current--;

                    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                    tcp_header_ptr =  (NX_TCP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

                    /* If this packet contains SYN */
                    if (tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_SYN_BIT)
                    {
                        /* Determine if there are any option words...  Note there are always 5 words in a TCP header.  */
                        option_words = (tcp_header_ptr -> nx_tcp_header_word_3 >> 28) - 5;
                        if (option_words > 0)
                        {

                            /* Yes, there are one or more option words.  */

                            /* Derive the Maximum Segment Size (MSS) in the option words.  */
                            _nx_tcp_mss_option_get((packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_TCP_HEADER)), option_words * (ULONG)sizeof(ULONG), &mss);

#ifdef NX_ENABLE_TCP_WINDOW_SCALING
                            _nx_tcp_window_scaling_option_get((packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_TCP_HEADER)), option_words * (ULONG)sizeof(ULONG), &rwin_scale);
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */
                        }
                    }

                    /* Set the default MSS if the MSS value was not found.  */
                    /*lint -e{644} suppress variable might not be initialized, since "mss" was initialized in _nx_tcp_mss_option_get. */
                    if (mss == 0)
                    {
#ifndef NX_DISABLE_IPV4
                        if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
                        {
                            mss = 536;
                        }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
                        if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
                        {
                            mss = 1220;
                        }
#endif /* FEATURE_NX_IPV6 */
                    }

#ifndef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT
                    /* If extended notify is enabled, call the syn_received notify function.
                       This user-supplied function decides whether or not this SYN request
                       should be accepted. */
                    if (socket_ptr -> nx_tcp_socket_syn_received_notify)
                    {
                        if ((socket_ptr -> nx_tcp_socket_syn_received_notify)(socket_ptr, packet_ptr) != NX_TRUE)
                        {

                            /* Release the packet.  */
                            _nx_packet_release(packet_ptr);

                            /* Release the protection.  */
                            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                            /* Return success.  */
                            return(NX_SUCCESS);
                        }
                    }
#endif /* NX_DISABLE_EXTENDED_NOTIFY_SUPPORT */

#ifndef NX_DISABLE_IPV4
                    if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
                    {

                        /* Pickup the source IP address.  */
                        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                        ipv4_header_ptr = (NX_IPV4_HEADER *)packet_ptr -> nx_packet_ip_header;

                        source_ip.nxd_ip_version = NX_IP_VERSION_V4;
                        source_ip.nxd_ip_address.v4 = ipv4_header_ptr -> nx_ip_header_source_ip;
                        socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v4 = source_ip.nxd_ip_address.v4;

                        /* Set the interface.  */
                        socket_ptr -> nx_tcp_socket_connect_interface = packet_ptr -> nx_packet_address.nx_packet_interface_ptr;
                        socket_ptr -> nx_tcp_socket_next_hop_address = NX_NULL;

                        /* Set the next hop address.  */
                        _nx_ip_route_find(ip_ptr, source_ip.nxd_ip_address.v4,
                                          &socket_ptr -> nx_tcp_socket_connect_interface,
                                          &socket_ptr -> nx_tcp_socket_next_hop_address);
                    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
                    if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
                    {

                    /*lint -e{928} -e{929} -e{826} -e{740} suppress cast from pointer to pointer, since it is necessary  */
                    NX_IPV6_HEADER *ipv6_header =
                        (NX_IPV6_HEADER *)((CHAR *)tcp_header_ptr - sizeof(NX_IPV6_HEADER));

                        source_ip.nxd_ip_version = NX_IP_VERSION_V6;
                        socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version = NX_IP_VERSION_V6;
                        source_ip.nxd_ip_address.v6[0] = ipv6_header -> nx_ip_header_source_ip[0];
                        source_ip.nxd_ip_address.v6[1] = ipv6_header -> nx_ip_header_source_ip[1];
                        source_ip.nxd_ip_address.v6[2] = ipv6_header -> nx_ip_header_source_ip[2];
                        source_ip.nxd_ip_address.v6[3] = ipv6_header -> nx_ip_header_source_ip[3];

                        COPY_IPV6_ADDRESS(source_ip.nxd_ip_address.v6,
                                          socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v6);


                        /* Also record the outgoing interface information. */
                        socket_ptr -> nx_tcp_socket_ipv6_addr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr;
                        socket_ptr -> nx_tcp_socket_connect_interface = socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address_attached;
                    }
#endif /* FEATURE_NX_IPV6 */

                    /* Pickup the source TCP port.  */
                    source_port =  (UINT)(tcp_header_ptr -> nx_tcp_header_word_0 >> NX_SHIFT_BY_16);

                    /* Fill the socket in with the appropriate information.  */
                    /*lint -e{644} suppress variable might not be initialized, since "source_ip" was initialized above. */
                    socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version =  source_ip.nxd_ip_version;

                    socket_ptr -> nx_tcp_socket_connect_port =  source_port;
                    socket_ptr -> nx_tcp_socket_rx_sequence =   tcp_header_ptr -> nx_tcp_sequence_number;

                    /* Indicate this socket is a server socket.  */
                    socket_ptr -> nx_tcp_socket_client_type =  NX_FALSE;

                    socket_ptr -> nx_tcp_socket_peer_mss = mss;

#ifdef NX_ENABLE_TCP_WINDOW_SCALING
                    /*
                       Simply record the peer's window scale value. When we move to the
                       ESTABLISHED state, we will set the peer window scale to 0 if the
                       peer does not support this feature.
                     */
                    socket_ptr -> nx_tcp_snd_win_scale_value = rwin_scale;
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */

                    /* If trace is enabled, insert this event into the trace buffer.  */
                    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_LISTEN_STATE, NX_TRACE_INTERNAL_EVENTS, 0, 0);

                    /* Move to the listen state.  */
                    socket_ptr -> nx_tcp_socket_state =  NX_TCP_LISTEN_STATE;

                    /* Remember what port is associated for this socket.  */
                    socket_ptr -> nx_tcp_socket_port =  port;

                    /* Calculate the hash index in the TCP port array of the associated IP instance.  */
                    index =  (UINT)((port + (port >> 8)) & NX_TCP_PORT_TABLE_MASK);

                    /* Determine if the list is NULL.  */
                    if (ip_ptr -> nx_ip_tcp_port_table[index])
                    {

                        /* There are already sockets on this list... just add this one
                           to the end.  */
                        socket_ptr -> nx_tcp_socket_bound_next = ip_ptr -> nx_ip_tcp_port_table[index];
                        socket_ptr -> nx_tcp_socket_bound_previous = (ip_ptr -> nx_ip_tcp_port_table[index]) -> nx_tcp_socket_bound_previous;
                        ((ip_ptr -> nx_ip_tcp_port_table[index]) -> nx_tcp_socket_bound_previous) -> nx_tcp_socket_bound_next = socket_ptr;
                        (ip_ptr -> nx_ip_tcp_port_table[index]) -> nx_tcp_socket_bound_previous =   socket_ptr;
                    }
                    else
                    {

                        /* Nothing is on the TCP port list.  Add this TCP socket to an
                           empty list.  */
                        socket_ptr -> nx_tcp_socket_bound_next =      socket_ptr;
                        socket_ptr -> nx_tcp_socket_bound_previous =  socket_ptr;
                        ip_ptr -> nx_ip_tcp_port_table[index] =       socket_ptr;
                    }

                    /* Pickup the listen callback routine.  */
                    listen_callback =  listen_ptr -> nx_tcp_listen_callback;

                    /* Release protection.  */
                    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                    /* Determine if there is a listen callback function.  */
                    if (listen_callback)
                    {

                        /* Call the user's listen callback function.  */
                        (listen_callback)(socket_ptr, port);
                    }

                    /* Release the incoming packet.  */
                    _nx_packet_release(packet_ptr);

                    /* Return a connection pending status so the caller knows
                       that a new connection request is already underway.  This
                       is also a successful status.  */
                    return(NX_CONNECTION_PENDING);
                }
            }

            /* Move to the next listen request.  */
            listen_ptr =  listen_ptr -> nx_tcp_listen_next;
        } while (listen_ptr != ip_ptr -> nx_ip_tcp_active_listen_requests);
    }


    /* Successful listen request, release the protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return an invalid relisten - nothing.  */
    return(NX_INVALID_RELISTEN);
}

