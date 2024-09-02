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
#ifdef NX_ENABLE_HTTP_PROXY
#include "nx_http_proxy_client.h"
#endif /* NX_ENABLE_HTTP_PROXY */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_state_syn_sent                       PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes packets during the SYN SENT state, which is */
/*    the state of the socket immediately after the initial SYN is sent   */
/*    in the establishment of a TCP connection.  We are expecting a SYN   */
/*    and an ACK from the other side of the connection in order to move   */
/*    into an established state.                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to owning socket      */
/*    tcp_header_ptr                        Pointer to packet header      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_packet_send_ack               Send ACK packet               */
/*    _nx_tcp_packet_send_syn               Send SYN packet               */
/*    _nx_tcp_packet_send_rst               Send RST packet               */
/*    _nx_tcp_socket_thread_resume          Resume suspended thread       */
/*    _nx_http_proxy_client_connect         Connect with HTTP Proxy       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_socket_packet_process         Process TCP packet for socket */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Wenhui Xie               Modified comment(s), and      */
/*                                            supported HTTP Proxy,       */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
VOID  _nx_tcp_socket_state_syn_sent(NX_TCP_SOCKET *socket_ptr, NX_TCP_HEADER *tcp_header_ptr, NX_PACKET *packet_ptr)
{


#ifndef TX_ENABLE_EVENT_TRACE
    NX_PARAMETER_NOT_USED(packet_ptr);
#endif /* TX_ENABLE_EVENT_TRACE */

    /* Check if a RST is present. */
    if (tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_RST_BIT)
    {

        /* Check if the ACK was acceptable. According to RFC 793, Section 3.9, Page 67.  */
        if ((tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_ACK_BIT) &&
            (tcp_header_ptr -> nx_tcp_acknowledgment_number == socket_ptr -> nx_tcp_socket_tx_sequence))
        {

#ifndef NX_DISABLE_TCP_INFO

            /* Increment the resets received count.  */
            (socket_ptr -> nx_tcp_socket_ip_ptr) -> nx_ip_tcp_resets_received++;
#endif

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_RESET_RECEIVE, socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, packet_ptr, tcp_header_ptr -> nx_tcp_sequence_number, NX_TRACE_INTERNAL_EVENTS, 0, 0);

            /* Reset connection.  */
            _nx_tcp_socket_connection_reset(socket_ptr);
        }

        /* Finished processing, simply return!  */
        return;
    }
    /* Determine if a valid SYN/ACK is present.  */
    else if ((tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_SYN_BIT) &&
             (tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_ACK_BIT) &&
             (tcp_header_ptr -> nx_tcp_acknowledgment_number == socket_ptr -> nx_tcp_socket_tx_sequence))
    {

        /* Yes, this is a proper SYN/ACK message.  We need to send an ACK
           back the other direction before we go into the ESTABLISHED
           state.  */

        /* Save the sequence number.  */
        socket_ptr -> nx_tcp_socket_rx_sequence =  tcp_header_ptr -> nx_tcp_sequence_number + 1;

        /* Save the window size.  */
        socket_ptr -> nx_tcp_socket_tx_window_advertised = tcp_header_ptr -> nx_tcp_header_word_3 & NX_LOWER_16_MASK;

#ifdef NX_ENABLE_TCP_WINDOW_SCALING

        /* The window size advertised in the SYN packet is NEVER scaled. Therefore there is no
           need to apply the scale shift.  However validate snd_win_scale  and rcv_win_scale. */
        if (socket_ptr -> nx_tcp_snd_win_scale_value == 0xFF)
        {
            /* Peer does not support window scale option. */
            socket_ptr -> nx_tcp_snd_win_scale_value = 0;
            socket_ptr -> nx_tcp_rcv_win_scale_value = 0;

            /* Since the peer does not offer window scaling feature, make sure
               our default window size for this connection does not exceed 65535 bytes. */
            if (socket_ptr -> nx_tcp_socket_rx_window_maximum > 65535)
            {
                socket_ptr -> nx_tcp_socket_rx_window_default = 65535;
                socket_ptr -> nx_tcp_socket_rx_window_current = 65535;
            }
        }

#endif /* NX_ENABLE_TCP_WINDOW_SCALING */

        /* Initialize the slow start threshold to be the advertised window size. */
        socket_ptr -> nx_tcp_socket_tx_slow_start_threshold = socket_ptr -> nx_tcp_socket_tx_window_advertised;

        /* Set the Initial transmit outstanding byte count. */
        socket_ptr -> nx_tcp_socket_tx_outstanding_bytes = 0;

        /* Set the initial congestion control window size. */
        /* Section 3.1, Page 5, RFC5681. */
        if (socket_ptr -> nx_tcp_socket_timeout_retries > 0)
        {

            /* Set the initial congestion control window size to be the mss. */
            socket_ptr -> nx_tcp_socket_tx_window_congestion = socket_ptr -> nx_tcp_socket_connect_mss;
        }
        else
        {
            socket_ptr -> nx_tcp_socket_tx_window_congestion = (socket_ptr -> nx_tcp_socket_connect_mss << 2);
            if (socket_ptr -> nx_tcp_socket_connect_mss > 1095)
            {
                socket_ptr -> nx_tcp_socket_tx_window_congestion -= socket_ptr -> nx_tcp_socket_connect_mss;
            }
            if (socket_ptr -> nx_tcp_socket_connect_mss > 2190)
            {
                socket_ptr -> nx_tcp_socket_tx_window_congestion -= socket_ptr -> nx_tcp_socket_connect_mss;
            }
        }

        /* Send the ACK.  */
        _nx_tcp_packet_send_ack(socket_ptr, socket_ptr -> nx_tcp_socket_tx_sequence);

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_ESTABLISHED, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Move to the ESTABLISHED state.  */
        socket_ptr -> nx_tcp_socket_state =  NX_TCP_ESTABLISHED;

        /* Clear the socket timeout.  */
        socket_ptr -> nx_tcp_socket_timeout =  0;

#ifdef NX_ENABLE_TCP_KEEPALIVE
        /* Is the keepalive feature enabled on this socket? */
        if (socket_ptr -> nx_tcp_socket_keepalive_enabled)
        {
            /* Setup the TCP Keepalive timer to initial values.  */
            socket_ptr -> nx_tcp_socket_keepalive_timeout =  NX_TCP_KEEPALIVE_INITIAL;
            socket_ptr -> nx_tcp_socket_keepalive_retries =  0;
        }
#endif

#ifdef NX_ENABLE_HTTP_PROXY

        /* Check if the HTTP Proxy is started and waiting for TCP socket connection.  */
        if ((socket_ptr -> nx_tcp_socket_ip_ptr -> nx_ip_http_proxy_enable) && 
            (socket_ptr -> nx_tcp_socket_http_proxy_state == NX_HTTP_PROXY_STATE_WAITING))
        {

            /* TCP connection established, start the HTTP Proxy connection.  */
            _nx_http_proxy_client_connect(socket_ptr);
        }
        else
#endif /* NX_ENABLE_HTTP_PROXY */
        {
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
        }
    }
    else if ((tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_SYN_BIT) &&
             (!(tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_ACK_BIT)))
    {

        /* Simultaneous Connection Synchronization,
           A SYN message was received.  We need to send both a SYN and ACK and move to the SYN RECEIVED state.  */

        /* Save the sequence number.  */
        socket_ptr -> nx_tcp_socket_rx_sequence =   tcp_header_ptr -> nx_tcp_sequence_number + 1;

        /* Save the window size.  */
        socket_ptr -> nx_tcp_socket_tx_window_advertised = tcp_header_ptr -> nx_tcp_header_word_3 & NX_LOWER_16_MASK;

#ifdef NX_ENABLE_TCP_WINDOW_SCALING
        socket_ptr -> nx_tcp_socket_tx_window_advertised <<= socket_ptr -> nx_tcp_rcv_win_scale_value;
#endif /* NX_ENABLE_TCP_WINDOW_SCALING  */

        /* Initialize the slow start threshold to be the advertised window size. */
        socket_ptr -> nx_tcp_socket_tx_slow_start_threshold = socket_ptr -> nx_tcp_socket_tx_window_advertised;

        /* Set the initial congestion control window size. */
        /* Section 3.1, Page 5, RFC5681. */
        socket_ptr -> nx_tcp_socket_tx_window_congestion = (socket_ptr -> nx_tcp_socket_connect_mss << 2);
        if (socket_ptr -> nx_tcp_socket_connect_mss > 1095)
        {
            socket_ptr -> nx_tcp_socket_tx_window_congestion -= socket_ptr -> nx_tcp_socket_connect_mss;
        }
        if (socket_ptr -> nx_tcp_socket_connect_mss > 2190)
        {
            socket_ptr -> nx_tcp_socket_tx_window_congestion -= socket_ptr -> nx_tcp_socket_connect_mss;
        }

        /* Set the Initial transmit outstanding byte count. */
        socket_ptr -> nx_tcp_socket_tx_outstanding_bytes = 0;

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_SYN_RECEIVED, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Move to the SYN RECEIVED state.  */
        socket_ptr -> nx_tcp_socket_state =  NX_TCP_SYN_RECEIVED;

        /* Clear the timeout.  */
        socket_ptr -> nx_tcp_socket_timeout =  0;

        /* Send the SYN packet.  */
        _nx_tcp_packet_send_syn(socket_ptr, (socket_ptr -> nx_tcp_socket_tx_sequence - 1));
    }
    /* Check for an invalid response to an attempted connection.  */
    else if ((tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_ACK_BIT) &&
             (tcp_header_ptr -> nx_tcp_acknowledgment_number != socket_ptr -> nx_tcp_socket_tx_sequence))
    {

        /* Invalid response was received, it is likely that the other side still
           thinks a previous connection is active.  Send a reset (RST) message to
           the other side to clear any previous connection.  */

        /* Send the RST packet.  */
        _nx_tcp_packet_send_rst(socket_ptr, tcp_header_ptr);
    }
}

