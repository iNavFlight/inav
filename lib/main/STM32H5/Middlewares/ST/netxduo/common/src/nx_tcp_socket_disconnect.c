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
#include "nx_ipv6.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_disconnect                           PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function handles the disconnect request for both active and    */
/*    passive calls.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP client socket  */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_connect_cleanup               Clear connect suspension      */
/*    _nx_tcp_disconnect_cleanup            Clear disconnect suspension   */
/*    _nx_tcp_packet_send_fin               Send FIN message              */
/*    _nx_tcp_packet_send_rst               Send RST on no timeout        */
/*    _nx_tcp_receive_cleanup               Clear receive suspension      */
/*    _nx_tcp_transmit_cleanup              Clear transmit suspension     */
/*    _nx_tcp_socket_thread_suspend         Suspend calling thread        */
/*    _nx_tcp_socket_transmit_queue_flush   Release all transmit packets  */
/*    _nx_tcp_socket_block_cleanup          Cleanup the socket block      */
/*    tx_mutex_get                          Get protection                */
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
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported TCP/IP offload,   */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_tcp_socket_disconnect(NX_TCP_SOCKET *socket_ptr, ULONG wait_option)
{

#ifndef NX_DISABLE_RESET_DISCONNECT
NX_TCP_HEADER tcp_header;
#endif
#ifdef NX_ENABLE_TCPIP_OFFLOAD
NX_INTERFACE *interface_ptr;
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
UINT          status;
NX_IP        *ip_ptr;

    /* Setup IP pointer.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_SOCKET_DISCONNECT, ip_ptr, socket_ptr, wait_option, socket_ptr -> nx_tcp_socket_state, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Default status to success.  */
    status =  NX_SUCCESS;

    /* Obtain the IP mutex so we can access socket and IP information.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

#ifndef NX_DISABLE_TCP_INFO
    /* Increment the TCP disconnections count.  */
    ip_ptr -> nx_ip_tcp_disconnections++;
#endif

    /* Determine if the socket is in a state not valid for a disconnect.  */
    if ((socket_ptr -> nx_tcp_socket_state != NX_TCP_ESTABLISHED) &&
        (socket_ptr -> nx_tcp_socket_state != NX_TCP_SYN_SENT) &&
        (socket_ptr -> nx_tcp_socket_state != NX_TCP_SYN_RECEIVED) &&
        (socket_ptr -> nx_tcp_socket_state != NX_TCP_CLOSE_WAIT))
    {

        /* Release protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return a not connected error code.  */
        return(NX_NOT_CONNECTED);
    }

#ifdef NX_ENABLE_TCP_KEEPALIVE
    /* Clear the TCP Keepalive timer to disable it for this socket (only needed when
       the socket is connected.  */
    socket_ptr -> nx_tcp_socket_keepalive_timeout =  0;
    socket_ptr -> nx_tcp_socket_keepalive_retries =  0;
#endif

#ifdef NX_ENABLE_TCPIP_OFFLOAD
    interface_ptr = socket_ptr -> nx_tcp_socket_connect_interface;
    if (interface_ptr &&
        (interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD) &&
        (interface_ptr -> nx_interface_tcpip_offload_handler))
    {

        /* Let TCP/IP offload interface close the connection.  */
        interface_ptr -> nx_interface_tcpip_offload_handler(ip_ptr, interface_ptr, socket_ptr,
                                                            NX_TCPIP_OFFLOAD_TCP_SOCKET_DISCONNECT,
                                                            NX_NULL, NX_NULL, NX_NULL, 0, NX_NULL,
                                                            wait_option);

        /* Reset socket state.  */
        socket_ptr -> nx_tcp_socket_state = NX_TCP_CLOSED;
    }
    else
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

    /* Determine if the connection wasn't fully completed.  */
    if ((socket_ptr -> nx_tcp_socket_state == NX_TCP_SYN_SENT) ||
        (socket_ptr -> nx_tcp_socket_state == NX_TCP_SYN_RECEIVED))
    {

        /* Connection wasn't fully completed, reset to the proper socket state.  */
        if (socket_ptr -> nx_tcp_socket_client_type)
        {

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_CLOSED, NX_TRACE_INTERNAL_EVENTS, 0, 0);

            if (socket_ptr -> nx_tcp_socket_state == NX_TCP_SYN_RECEIVED)
            {

                /* Setup FIN timeout.  */
                socket_ptr -> nx_tcp_socket_timeout = socket_ptr -> nx_tcp_socket_timeout_rate;
                socket_ptr -> nx_tcp_socket_timeout_retries =  0;

                /* Increment the sequence number.  */
                socket_ptr -> nx_tcp_socket_tx_sequence++;

                /* Send FIN packet.  */
                _nx_tcp_packet_send_fin(socket_ptr, (socket_ptr -> nx_tcp_socket_tx_sequence - 1));
            }

            /* Client socket, return to a CLOSED state.  */
            socket_ptr -> nx_tcp_socket_state =  NX_TCP_CLOSED;
        }
        else
        {

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_LISTEN_STATE, NX_TRACE_INTERNAL_EVENTS, 0, 0);

            if ((socket_ptr -> nx_tcp_socket_state == NX_TCP_SYN_RECEIVED) &&
                (socket_ptr -> nx_tcp_socket_connect_interface != NX_NULL))
            {

                /* Setup FIN timeout.  */
                socket_ptr -> nx_tcp_socket_timeout = socket_ptr -> nx_tcp_socket_timeout_rate;
                socket_ptr -> nx_tcp_socket_timeout_retries =  0;

                /* Increment the sequence number.  */
                socket_ptr -> nx_tcp_socket_tx_sequence++;

                /* Send FIN packet.  */
                _nx_tcp_packet_send_fin(socket_ptr, (socket_ptr -> nx_tcp_socket_tx_sequence - 1));
            }

            /* Server socket, return to LISTEN state.  */
            socket_ptr -> nx_tcp_socket_state =  NX_TCP_LISTEN_STATE;

            /* Move back the acknowledgment number just in case there is a retry.  */
            socket_ptr -> nx_tcp_socket_rx_sequence--;
        }

        /* Socket is no longer active. Clear the timeout. */
        socket_ptr -> nx_tcp_socket_timeout =  0;
    }

#ifndef NX_DISABLE_RESET_DISCONNECT

    /* Determine if there is no timeout associated with the disconnect. If this is the case,
       we will send a RST and simply enter a closed state.  */
    else if (wait_option == NX_NO_WAIT)
    {

        /* No timeout was specified, simply send a RST and enter a closed or listen state.  */

        /* Clear this field so the RST packet handler knows this is a fake header. */
        tcp_header.nx_tcp_header_word_3 = NX_TCP_ACK_BIT;

        /* Send the RST packet. We just want to create a fake header, so assume this packet is incoming packet.  */
        tcp_header.nx_tcp_acknowledgment_number =  socket_ptr -> nx_tcp_socket_tx_sequence;
        tcp_header.nx_tcp_sequence_number = socket_ptr -> nx_tcp_socket_rx_sequence;
        _nx_tcp_packet_send_rst(socket_ptr, &tcp_header);

        /* Cleanup the transmission control block.  */
        _nx_tcp_socket_block_cleanup(socket_ptr);

        /* No suspension is requested, just set the return status to in progress.  */
        status =  NX_IN_PROGRESS;
    }
#endif

    /* Determine if this is an active disconnect, i.e. initiated by the application rather
       than in response to a disconnect from the connected socket.  */
    else if (socket_ptr -> nx_tcp_socket_state != NX_TCP_CLOSE_WAIT)
    {

        /* Yes, this disconnect was initiated by the application.  Initiate the disconnect
           process.  */

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_FIN_WAIT_1, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Move the TCP state to FIN WAIT 1 state, the first state of an active close.  */
        socket_ptr -> nx_tcp_socket_state =  NX_TCP_FIN_WAIT_1;

        /* Determine if the transmit queue is empty.  Only setup a FIN timeout here when
           there are no more transmit packets waiting to be ACKed.  If there are transmit
           packets still waiting, the FIN timeout will be setup when the transmit queue is completely
           acknowledged.  */
        if (socket_ptr -> nx_tcp_socket_transmit_sent_head == NX_NULL)
        {

            /* No transmit packets queue, setup FIN timeout.  */
            socket_ptr -> nx_tcp_socket_timeout =          socket_ptr -> nx_tcp_socket_timeout_rate;
            socket_ptr -> nx_tcp_socket_timeout_retries =  0;
        }

        /* Increment the sequence number.  */
        socket_ptr -> nx_tcp_socket_tx_sequence++;

        /* Send FIN packet.  */
        _nx_tcp_packet_send_fin(socket_ptr, (socket_ptr -> nx_tcp_socket_tx_sequence - 1));
    }
    else
    {

        /* This disconnect request is coming after the other side of the TCP connection
           initiated a disconnect.  */

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_LAST_ACK, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Move the TCP state to wait for the last ACK message for the complete disconnect.  */
        socket_ptr -> nx_tcp_socket_state =  NX_TCP_LAST_ACK;

        /* Send a FIN message back to the side of the connection that initiated the
           disconnect.  */

        /* Determine if the transmit queue is empty.  Only setup a FIN timeout here when
           there are no more transmit packets waiting to be ACKed.  If there are transmit
           packets still waiting, the FIN timeout will be setup when the transmit queue is completely
           acknowledged.  */
        if (socket_ptr -> nx_tcp_socket_transmit_sent_head == NX_NULL)
        {

            /* No transmit packets queue, setup FIN timeout.  */
            socket_ptr -> nx_tcp_socket_timeout =          socket_ptr -> nx_tcp_socket_timeout_rate;
            socket_ptr -> nx_tcp_socket_timeout_retries =  0;
        }

        /* Increment the sequence number.  */
        socket_ptr -> nx_tcp_socket_tx_sequence++;

        /* Send FIN packet.  */
        _nx_tcp_packet_send_fin(socket_ptr, (socket_ptr -> nx_tcp_socket_tx_sequence - 1));
    }

    /* Optionally suspend the thread.  If timeout occurs, return a disconnect timeout status.  If
       immediate response is selected, return a disconnect in progress status.  Only on a real
       disconnect should success be returned.  */
    if ((socket_ptr -> nx_tcp_socket_state != NX_TCP_CLOSED) && (socket_ptr -> nx_tcp_socket_state != NX_TCP_LISTEN_STATE) &&
#ifdef NX_DISABLE_RESET_DISCONNECT
        (wait_option) &&
#endif
        (_tx_thread_current_ptr != &(ip_ptr -> nx_ip_thread)))
    {

        /* Suspend the thread on socket disconnect.  */
        _nx_tcp_socket_thread_suspend(&(socket_ptr -> nx_tcp_socket_disconnect_suspended_thread), _nx_tcp_disconnect_cleanup, socket_ptr, &(ip_ptr -> nx_ip_protection), wait_option);

        /* Reobtain the IP mutex.  */
        tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

        /* Determine if the socket is in the timed wait state.  */
        if (socket_ptr -> nx_tcp_socket_state != NX_TCP_TIMED_WAIT)
        {

            /* Cleanup the transmission control block.  */
            _nx_tcp_socket_block_cleanup(socket_ptr);
        }

        /* Use the thread return the completion code.  */
        status =  _tx_thread_current_ptr -> tx_thread_suspend_status;
    }

    /* We now need to check for any remaining sent packets in the transmit queue.
       If found they need to be released.  */
    if (socket_ptr -> nx_tcp_socket_transmit_sent_head)
    {

        /* Release all transmit packets.  */
        _nx_tcp_socket_transmit_queue_flush(socket_ptr);
    }

    /* Clear any connection suspension on this socket.  */
    if (socket_ptr -> nx_tcp_socket_connect_suspended_thread)
    {

        /* Call the connect thread suspension cleanup routine.  */
        _nx_tcp_connect_cleanup(socket_ptr -> nx_tcp_socket_connect_suspended_thread NX_CLEANUP_ARGUMENT);
    }

    /* Clear all receive thread suspensions on this socket.  */
    while (socket_ptr -> nx_tcp_socket_receive_suspension_list)
    {

        /* Call the receive thread suspension cleanup routine.  */
        _nx_tcp_receive_cleanup(socket_ptr -> nx_tcp_socket_receive_suspension_list NX_CLEANUP_ARGUMENT);
    }

    /* Clear all transmit thread suspensions on this socket.  */
    while (socket_ptr -> nx_tcp_socket_transmit_suspension_list)
    {

        /* Call the receive thread suspension cleanup routine.  */
        _nx_tcp_transmit_cleanup(socket_ptr -> nx_tcp_socket_transmit_suspension_list NX_CLEANUP_ARGUMENT);
    }

    /* Release the IP protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return in-progress completion status.  */
    return(status);
}

