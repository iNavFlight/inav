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
#include "nx_ip.h"
#include "nx_tcp.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_create                               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function creates a TCP socket for the specified IP instance.   */
/*    Both client and server sockets are created by this service.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    socket_ptr                            Pointer to new TCP socket     */
/*    name                                  Name of new TCP socket        */
/*    type_of_service                       Type of service for this TCP  */
/*                                            socket                      */
/*    fragment                              Flag to enable IP fragmenting */
/*    time_to_live                          Time to live value for socket */
/*    window_size                           Size of socket's receive      */
/*                                            window                      */
/*    tcp_urgent_data_callback              Routine to call when urgent   */
/*                                            data is received            */
/*    tcp_disconnect_callback               Routine to call when a        */
/*                                            disconnect occurs           */
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
UINT  _nx_tcp_socket_create(NX_IP *ip_ptr, NX_TCP_SOCKET *socket_ptr, CHAR *name,
                            ULONG type_of_service, ULONG fragment, UINT time_to_live, ULONG window_size,
                            VOID (*tcp_urgent_data_callback)(NX_TCP_SOCKET *socket_ptr),
                            VOID (*tcp_disconnect_callback)(NX_TCP_SOCKET *socket_ptr))
{
TX_INTERRUPT_SAVE_AREA

NX_TCP_SOCKET *tail_ptr;


    /* Initialize the TCP control block to zero.  */
    memset((void *)socket_ptr, 0, sizeof(NX_TCP_SOCKET));

    /* Fill in the basic information in the new TCP socket structure.  */

    /* Remember the associated IP structure.  */
    socket_ptr -> nx_tcp_socket_ip_ptr =  ip_ptr;

    /* By default, indicate the socket is a client socket.  */
    socket_ptr -> nx_tcp_socket_client_type =  NX_TRUE;

    /* Save the TCP socket's name.  */
    socket_ptr -> nx_tcp_socket_name =  name;

    /* Setup the counter for duplicated ACK packet.  */
    socket_ptr -> nx_tcp_socket_duplicated_ack_received = 0;

    /* Setup this socket's maximum segment size (mss).  */
    socket_ptr -> nx_tcp_socket_mss = 0;

    /* Setup the default receiver's maximum segment size.  */
    socket_ptr -> nx_tcp_socket_connect_mss =  NX_TCP_MSS_SIZE;

    /* Save the type of service input parameter.  */
    socket_ptr -> nx_tcp_socket_type_of_service =  type_of_service;

    /* Save the fragment input parameter.  */
    socket_ptr -> nx_tcp_socket_fragment_enable =  fragment & NX_DONT_FRAGMENT;

    /* Save the time-to-live input parameter.  */
    socket_ptr -> nx_tcp_socket_time_to_live =  time_to_live;

    /* Clear the socket bind in progress flag.  */
    socket_ptr -> nx_tcp_socket_bind_in_progress =  NX_FALSE;

    /* Setup the delayed ACK timeout periodic rate.  */
    socket_ptr -> nx_tcp_socket_delayed_ack_timeout =  _nx_tcp_ack_timer_rate;

    /* Setup the default transmit timeout.  */
    socket_ptr -> nx_tcp_socket_timeout_rate =         _nx_tcp_transmit_timer_rate;
    socket_ptr -> nx_tcp_socket_timeout_max_retries =  NX_TCP_MAXIMUM_RETRIES;
    socket_ptr -> nx_tcp_socket_timeout_shift =        NX_TCP_RETRY_SHIFT;

    /* Setup the default maximum transmit queue depth.  */
    socket_ptr -> nx_tcp_socket_transmit_queue_maximum_default =  NX_TCP_MAXIMUM_TX_QUEUE;
    socket_ptr -> nx_tcp_socket_transmit_queue_maximum =          NX_TCP_MAXIMUM_TX_QUEUE;

#ifdef NX_ENABLE_LOW_WATERMARK
    /* Setup the default maximum receive queue depth.  */
    socket_ptr -> nx_tcp_socket_receive_queue_maximum = NX_TCP_MAXIMUM_RX_QUEUE;
#endif /* NX_ENABLE_LOW_WATERMARK */

#ifdef NX_ENABLE_TCP_WINDOW_SCALING

    /* Window scaling feature is enabled.  Record this user-specified window size. */
    socket_ptr -> nx_tcp_socket_rx_window_maximum = window_size;
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */

    /* Setup the sliding window information.  */
    socket_ptr -> nx_tcp_socket_rx_window_default =   window_size;
    socket_ptr -> nx_tcp_socket_rx_window_current =   window_size;
    socket_ptr -> nx_tcp_socket_tx_outstanding_bytes = 0;
    socket_ptr -> nx_tcp_socket_tx_window_advertised = 0;
    socket_ptr -> nx_tcp_socket_tx_window_congestion = 0;


    /* Initialize the ack_n_packet counter. */
    socket_ptr -> nx_tcp_socket_ack_n_packet_counter = 1;

    /* Save the application callback routines.  */
    socket_ptr -> nx_tcp_urgent_data_callback = tcp_urgent_data_callback;
    socket_ptr -> nx_tcp_disconnect_callback =  tcp_disconnect_callback;

    /* Clear the receive notify function pointer.  */
    socket_ptr -> nx_tcp_receive_callback =  NX_NULL;

#ifdef NX_ENABLE_TCP_KEEPALIVE
    /* If the Keep alive feature is enabled in NetX, enable it
       on all TCP sockets. */
    socket_ptr -> nx_tcp_socket_keepalive_enabled = NX_TRUE;

#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_CLOSED, NX_TRACE_INTERNAL_EVENTS, 0, 0);

    /* Setup the initial TCP socket state.  */
    socket_ptr -> nx_tcp_socket_state =  NX_TCP_CLOSED;

    /* If trace is enabled, register this object.  */
    NX_TRACE_OBJECT_REGISTER(NX_TRACE_OBJECT_TYPE_TCP_SOCKET, socket_ptr, name, type_of_service, window_size);

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_SOCKET_CREATE, ip_ptr, socket_ptr, type_of_service, window_size, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Get protection while we insert the TCP socket into the created list.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Disable interrupts while we link the new TCP socket to the IP structure.  */
    TX_DISABLE

    /* Load the TCP ID field in the TCP control block.  */
    socket_ptr -> nx_tcp_socket_id =  NX_TCP_ID;

    /* Place the new TCP control block on the list of created TCP sockets for this IP.  First,
       check for an empty list.  */
    if (ip_ptr -> nx_ip_tcp_created_sockets_ptr)
    {

        /* Pickup tail pointer.  */
        tail_ptr =  (ip_ptr -> nx_ip_tcp_created_sockets_ptr) -> nx_tcp_socket_created_previous;

        /* Place the new TCP socket control block in the list.  */
        (ip_ptr -> nx_ip_tcp_created_sockets_ptr) -> nx_tcp_socket_created_previous =  socket_ptr;
        tail_ptr ->  nx_tcp_socket_created_next =  socket_ptr;

        /* Setup this TCP socket's created links.  */
        socket_ptr -> nx_tcp_socket_created_previous =  tail_ptr;
        socket_ptr -> nx_tcp_socket_created_next =      ip_ptr -> nx_ip_tcp_created_sockets_ptr;
    }
    else
    {

        /* The created TCP socket list is empty.  Add TCP socket control block to empty list.  */
        ip_ptr -> nx_ip_tcp_created_sockets_ptr =       socket_ptr;
        socket_ptr -> nx_tcp_socket_created_previous =  socket_ptr;
        socket_ptr -> nx_tcp_socket_created_next =      socket_ptr;
    }

    /* Increment the created TCP socket counter.  */
    ip_ptr -> nx_ip_tcp_created_sockets_count++;

#ifdef FEATURE_NX_IPV6
    socket_ptr -> nx_tcp_socket_ipv6_addr   =             NX_NULL;
#endif /* FEATURE_NX_IPV6 */


#ifdef NX_IPSEC_ENABLE
    socket_ptr -> nx_tcp_socket_egress_sa = NX_NULL;
    socket_ptr -> nx_tcp_socket_egress_sa_data_offset = 0;
#endif /* NX_IPSEC_ENABLE */
    /* Restore previous interrupt posture.  */
    TX_RESTORE

    /* Release protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return successful completion.  */
    return(NX_SUCCESS);
}

