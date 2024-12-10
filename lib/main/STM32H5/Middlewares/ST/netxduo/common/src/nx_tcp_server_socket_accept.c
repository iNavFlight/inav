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
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_server_socket_accept                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets up the server socket after an active connection  */
/*    request was received.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to new TCP socket     */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_packet_send_syn               Send SYN message              */
/*    _nx_tcp_socket_thread_suspend         Suspend thread for connection */
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release a protection mutex    */
/*    NX_RAND                               Random number for sequence    */
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
UINT  _nx_tcp_server_socket_accept(NX_TCP_SOCKET *socket_ptr, ULONG wait_option)
{

NX_IP *ip_ptr;


    /* Pickup the associated IP structure.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_SERVER_SOCKET_ACCEPT, ip_ptr, socket_ptr, wait_option, socket_ptr -> nx_tcp_socket_state, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Check if the socket has already made a connection, return successful outcome to accept(). */
    if (socket_ptr -> nx_tcp_socket_state == NX_TCP_ESTABLISHED)
    {
        return(NX_SUCCESS);
    }

    /* Determine if the socket is still in the listen state or has sent a SYN packet out already
       from a previous accept() call on this socket.  */
    if ((socket_ptr -> nx_tcp_socket_state != NX_TCP_LISTEN_STATE) && (socket_ptr -> nx_tcp_socket_state != NX_TCP_SYN_RECEIVED))
    {

        /* Socket has either been closed or in the process of closing*/
        return(NX_NOT_LISTEN_STATE);
    }


    /* Obtain the IP mutex so we can initiate accept processing for this socket.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    if (socket_ptr -> nx_tcp_socket_state == NX_TCP_LISTEN_STATE)
    {

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

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_SYN_RECEIVED, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Move the TCP state to Sequence Received, the next state of a passive open.  */
        socket_ptr -> nx_tcp_socket_state =  NX_TCP_SYN_RECEIVED;

        /* Clear the FIN received flag.  */
        socket_ptr -> nx_tcp_socket_fin_received =  NX_FALSE;
        socket_ptr -> nx_tcp_socket_fin_acked =  NX_FALSE;

        /* Determine if the listen command has completed.  This can be detected by checking
           to see if the socket is bound.  If it is bound and still in the listen state, then
           we know that this service is being called after a client connection request was
           received.  */
        if (socket_ptr -> nx_tcp_socket_bound_next)
        {

            /* Send a SYN message back to establish the connection, but increment the ACK first.  */
            socket_ptr -> nx_tcp_socket_rx_sequence++;

            /* Increment the sequence number.  */
            socket_ptr -> nx_tcp_socket_tx_sequence++;

            /* Setup a timeout so the connection attempt can be sent again.  */

            socket_ptr -> nx_tcp_socket_timeout =          socket_ptr -> nx_tcp_socket_timeout_rate;
            socket_ptr -> nx_tcp_socket_timeout_retries =  0;

            /* CLEANUP: Clean up any existing socket data before making a new connection. */
            socket_ptr -> nx_tcp_socket_tx_window_congestion = 0;
            socket_ptr -> nx_tcp_socket_tx_outstanding_bytes = 0;
            socket_ptr -> nx_tcp_socket_packets_sent = 0;
            socket_ptr -> nx_tcp_socket_bytes_sent = 0;
            socket_ptr -> nx_tcp_socket_packets_received = 0;
            socket_ptr -> nx_tcp_socket_bytes_received = 0;
            socket_ptr -> nx_tcp_socket_retransmit_packets = 0;
            socket_ptr -> nx_tcp_socket_checksum_errors = 0;
            socket_ptr -> nx_tcp_socket_transmit_sent_head  =  NX_NULL;
            socket_ptr -> nx_tcp_socket_transmit_sent_tail  =  NX_NULL;
            socket_ptr -> nx_tcp_socket_transmit_sent_count =  0;
            socket_ptr -> nx_tcp_socket_receive_queue_count =  0;
            socket_ptr -> nx_tcp_socket_receive_queue_head  =  NX_NULL;
            socket_ptr -> nx_tcp_socket_receive_queue_tail  =  NX_NULL;

            /* Send the SYN+ACK message.  */
            _nx_tcp_packet_send_syn(socket_ptr, (socket_ptr -> nx_tcp_socket_tx_sequence - 1));
        }
        else
        {
            socket_ptr -> nx_tcp_socket_timeout = 0;
        }
    }

    /* Determine if the wait option is specified.  If so, suspend the calling thread.
       Otherwise, return an in progress status.  */
    if ((wait_option) && (_tx_thread_current_ptr != &(ip_ptr -> nx_ip_thread)))
    {

        /* Suspend the thread on this socket's receive queue.  */
        _nx_tcp_socket_thread_suspend(&(socket_ptr -> nx_tcp_socket_connect_suspended_thread), _nx_tcp_connect_cleanup,
                                      socket_ptr, &(ip_ptr -> nx_ip_protection), wait_option);

        /* Check if the socket connection has failed.  */
        if (_tx_thread_current_ptr -> tx_thread_suspend_status)
        {

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_LISTEN_STATE, NX_TRACE_INTERNAL_EVENTS, 0, 0);

            /* Yes, socket connection has failed.  Return to the
               listen state so it can be tried again.  */
            socket_ptr -> nx_tcp_socket_state =  NX_TCP_LISTEN_STATE;

            /* Socket is not active. Clear the timeout. */
            socket_ptr -> nx_tcp_socket_timeout =  0;
        }

        /* If not, just return the status.  */
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

