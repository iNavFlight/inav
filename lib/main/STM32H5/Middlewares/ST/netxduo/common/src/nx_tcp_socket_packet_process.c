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
#ifdef NX_ENABLE_HTTP_PROXY
#include "nx_http_proxy_client.h"
#endif /* NX_ENABLE_HTTP_PROXY */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_packet_process                       PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming TCP packet relative to the      */
/*    socket it belongs to, including processing state changes, and       */
/*    sending and receiving data.                                         */
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
/*    _nx_packet_release                    Packet release function       */
/*    _nx_tcp_socket_connection_reset       Reset connection              */
/*    _nx_tcp_socket_state_ack_check        Process received ACKs         */
/*    _nx_tcp_socket_state_closing          Process CLOSING state         */
/*    _nx_tcp_socket_state_data_check       Process received data         */
/*    _nx_tcp_socket_state_established      Process ESTABLISHED state     */
/*    _nx_tcp_socket_state_fin_wait1        Process FIN WAIT 1 state      */
/*    _nx_tcp_socket_state_fin_wait2        Process FIN WAIT 2 state      */
/*    _nx_tcp_socket_state_last_ack         Process LAST ACK state        */
/*    _nx_tcp_socket_state_syn_received     Process SYN RECEIVED state    */
/*    _nx_tcp_socket_state_syn_sent         Process SYN SENT state        */
/*    _nx_tcp_socket_state_transmit_check   Check for transmit ability    */
/*    (nx_tcp_urgent_data_callback)         Application urgent callback   */
/*                                            function                    */
/*    _nx_http_proxy_client_connect_response_process                      */
/*                                          Process HTTP Proxy CONNECT    */
/*                                            response                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_packet_process                Process raw TCP packet        */
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
/*  01-31-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed unsigned integers     */
/*                                            comparison,                 */
/*                                            resulting in version 6.1.10 */
/*  10-31-2022     Wenhui Xie               Modified comment(s), and      */
/*                                            supported HTTP Proxy,       */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
VOID  _nx_tcp_socket_packet_process(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr)
{

UINT          packet_queued =  NX_FALSE;
NX_TCP_HEADER tcp_header_copy;
VOID          (*urgent_callback)(NX_TCP_SOCKET *socket_ptr);
ULONG         header_length;
ULONG         packet_data_length;
ULONG         packet_sequence;
ULONG         rx_sequence;
ULONG         rx_window;
UINT          outside_of_window;
ULONG         mss = 0;
#ifdef NX_ENABLE_TCPIP_OFFLOAD
ULONG         tcpip_offload; 

    tcpip_offload = socket_ptr -> nx_tcp_socket_connect_interface -> nx_interface_capability_flag &
                    NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD;
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Copy the TCP header, since the actual packet can be delivered to
       a waiting socket/thread during this routine and before we are done
       using the header.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    tcp_header_copy =  *((NX_TCP_HEADER *)packet_ptr -> nx_packet_prepend_ptr);

    /* Get the size of the TCP header.  */
    header_length =  (tcp_header_copy.nx_tcp_header_word_3 >> NX_TCP_HEADER_SHIFT) * (ULONG)sizeof(ULONG);

    /* Process the segment if socket state is equal or greater than NX_TCP_SYN_RECEIVED. According to RFC 793, Section 3.9, Page 69.  */
    if ((socket_ptr -> nx_tcp_socket_state >= NX_TCP_SYN_RECEIVED)
#ifdef NX_ENABLE_TCPIP_OFFLOAD
        && (!tcpip_offload)
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
       )
    {

        /* Step1: Check sequence number. According to RFC 793, Section 3.9, Page 69.  */

        /* Pickup the sequence of this packet. */
        packet_sequence = tcp_header_copy.nx_tcp_sequence_number;

        /* Calculate the data length in the packet.  */
        packet_data_length = packet_ptr -> nx_packet_length - header_length;

        /* Pickup the rx sequence.  */
        rx_sequence = socket_ptr -> nx_tcp_socket_rx_sequence;

#ifdef NX_ENABLE_LOW_WATERMARK
        if ((socket_ptr -> nx_tcp_socket_rx_window_current == 0) &&
            (socket_ptr -> nx_tcp_socket_receive_queue_head == NX_NULL) &&
            (packet_ptr -> nx_packet_pool_owner -> nx_packet_pool_available >=
             packet_ptr -> nx_packet_pool_owner -> nx_packet_pool_low_watermark))
        {

            /* Window was closed due to low watermark of packet pool. */
            /* Now reset the window size. */
            socket_ptr -> nx_tcp_socket_rx_window_current =  socket_ptr -> nx_tcp_socket_rx_window_default;
        }
#endif /* NX_ENABLE_LOW_WATERMARK */

        /* Pickup the rx window.  */
        rx_window = socket_ptr -> nx_tcp_socket_rx_window_current;

        /* There are four cases for the acceptability test for an incoming segment.
           Section 3.9 Page 69, RFC 793.  */
        outside_of_window = NX_TRUE;

        if (packet_data_length == 0)
        {
            if (rx_window == 0)
            {
                if (packet_sequence == rx_sequence)
                {
                    outside_of_window = NX_FALSE;
                }
                else if ((tcp_header_copy.nx_tcp_header_word_3 & NX_TCP_RST_BIT) ||
                         (tcp_header_copy.nx_tcp_header_word_3 & NX_TCP_URG_BIT) ||
                         ((tcp_header_copy.nx_tcp_header_word_3 & NX_TCP_CONTROL_MASK) == NX_TCP_ACK_BIT))
                {

                    /* If the RCV.WND is zero, no segments will be acceptable, but
                       special allowance should be made to accept valid ACKs, URGs and RSTs.
                       Section 3.9 Page 69, RFC 793. */
                    outside_of_window = NX_FALSE;
                }
            }
            else if (((INT)(packet_sequence - rx_sequence) >= 0) &&
                     ((INT)(rx_sequence + rx_window - packet_sequence) > 0))
            {
                outside_of_window = NX_FALSE;
            }
        }
        else
        {
            if ((rx_window > 0) &&
                ((((INT)(packet_sequence - rx_sequence) >= 0) &&
                  ((INT)(rx_sequence + rx_window - packet_sequence) > 0)) ||
                 (((INT)(packet_sequence + (packet_data_length - 1) - rx_sequence) >= 0) &&
                  ((INT)(rx_sequence + 1 + (rx_window - packet_sequence) - packet_data_length) > 0))))
            {
                outside_of_window = NX_FALSE;
            }
        }

        /* Detect whether or not the data is outside the window.  */
        if (outside_of_window)
        {

            /* If an incoming segment is not acceptable, an acknowledgment should be sent in reply
               (unless the RST bit is set, if so drop the segment and return).
               Section 3.9, Page 69, RFC 793.  */
            if (!(tcp_header_copy.nx_tcp_header_word_3 & NX_TCP_RST_BIT))
            {

                /* Send an immediate ACK.  */
                _nx_tcp_packet_send_ack(socket_ptr, socket_ptr -> nx_tcp_socket_tx_sequence);
            }

#ifndef NX_DISABLE_TCP_INFO

            /* Increment the TCP dropped packet count.  */
            socket_ptr -> nx_tcp_socket_ip_ptr -> nx_ip_tcp_receive_packets_dropped++;
#endif

            /* Release the packet.  */
            _nx_packet_release(packet_ptr);

            /* Finished processing, simply return!  */
            return;
        }

        /* Step2: Check the RST bit. According to RFC 793, Section 3.9, Page 70.  */
        if (tcp_header_copy.nx_tcp_header_word_3 & NX_TCP_RST_BIT)
        {

#ifndef NX_DISABLE_TCP_INFO

            /* Increment the resets received count.  */
            (socket_ptr -> nx_tcp_socket_ip_ptr) -> nx_ip_tcp_resets_received++;
#endif

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_RESET_RECEIVE, socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, packet_ptr, tcp_header_copy.nx_tcp_sequence_number, NX_TRACE_INTERNAL_EVENTS, 0, 0);

            /* Reset connection.  */
            _nx_tcp_socket_connection_reset(socket_ptr);

            /* Release the packet.  */
            _nx_packet_release(packet_ptr);

            /* Finished processing, simply return!  */
            return;
        }

        /* Step3: Check the SYN bit. According to RFC 793, Section 3.9, Page 71.  */
        if (tcp_header_copy.nx_tcp_header_word_3 & NX_TCP_SYN_BIT)
        {

            /* The SYN is in the window it is an error, send a reset.  */

            /* Adjust the SEQ for the SYN bit. */
            /* The reset logic uses the sequence number in tcp_header_ptr as its ACK number. */
            tcp_header_copy.nx_tcp_sequence_number++;

            /* Send RST message.  */
            _nx_tcp_packet_send_rst(socket_ptr, &tcp_header_copy);

            /* Reset the connection. */
            _nx_tcp_socket_connection_reset(socket_ptr);

            /* Release the packet.  */
            _nx_packet_release(packet_ptr);

            /* Finished processing, simply return!  */
            return;
        }

        /* Step4: Check the ACK field. According to RFC 793, Section 3.9, Page 72.  */
        if (socket_ptr -> nx_tcp_socket_state != NX_TCP_SYN_RECEIVED)
        {

            /* Check the ACK field.  */
            if (_nx_tcp_socket_state_ack_check(socket_ptr, &tcp_header_copy) == NX_FALSE)
            {

                /* Release the packet.  */
                _nx_packet_release(packet_ptr);

                /* Finished processing, simply return!  */
                return;
            }
        }
    }

    /* Illegal option length check. */
    if (header_length > sizeof(NX_TCP_HEADER))
    {

        /* There are one or more option words.  */
        /* The illegal option length is validated during MSS option get function. */
        if (!_nx_tcp_mss_option_get((packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_TCP_HEADER)),
                                    header_length - (ULONG)sizeof(NX_TCP_HEADER), &mss))
        {

            /* TCP MUST be prepared to handle an illegal option length (e.g., zero) without crashing;
               a suggested procedure is to reset the connection and log the reason, outlined in RFC 1122, Section 4.2.2.5, Page85. */

            /* Preprocess the sequence number if the incoming segment does not have an ACK field.
               Reset Generation, RFC793, Section3.4, Page37. */
            if (!(tcp_header_copy.nx_tcp_header_word_3 & NX_TCP_ACK_BIT))
            {

                /* Update sequence number to set the reset acknowledge number.  */
                tcp_header_copy.nx_tcp_sequence_number += (packet_ptr -> nx_packet_length - header_length);

                /* Check the SYN and FIN bits.  */
                if ((tcp_header_copy.nx_tcp_header_word_3 & NX_TCP_SYN_BIT) ||
                    (tcp_header_copy.nx_tcp_header_word_3 & NX_TCP_FIN_BIT))
                {

                    /* Update sequence number to set the reset acknowledge number.  */
                    tcp_header_copy.nx_tcp_sequence_number++;
                }
            }

            /* Send RST message.  */
            _nx_tcp_packet_send_rst(socket_ptr, &tcp_header_copy);

            /* Reset the connection. */
            _nx_tcp_socket_connection_reset(socket_ptr);

#ifndef NX_DISABLE_TCP_INFO
            /* Increment the TCP invalid packet error count.  */
            socket_ptr -> nx_tcp_socket_ip_ptr -> nx_ip_tcp_invalid_packets++;
#endif /* NX_DISABLE_TCP_INFO */

            /* Release the packet.  */
            _nx_packet_release(packet_ptr);

            return;
        }

    }

    /* Process relative to the state of the socket.  */
    switch (socket_ptr -> nx_tcp_socket_state)
    {

    case  NX_TCP_SYN_SENT:

        /* Call the SYN SENT state handling function to process any state
           changes caused by this new packet.  */
        _nx_tcp_socket_state_syn_sent(socket_ptr, &tcp_header_copy, packet_ptr);

        /* Check whether socket is established. */
        if (socket_ptr -> nx_tcp_socket_state == NX_TCP_ESTABLISHED)
        {

            /* Check for data in the current packet.  */
            packet_queued =  _nx_tcp_socket_state_data_check(socket_ptr, packet_ptr);
        }

        /* State processing is complete.  */
        break;

    case  NX_TCP_SYN_RECEIVED:

        /* Call the SYN RECEIVED state handling function to process any state
           changes caused by this new packet.  */
        _nx_tcp_socket_state_syn_received(socket_ptr, &tcp_header_copy);

        /* Check whether socket is established. */
        if (socket_ptr -> nx_tcp_socket_state == NX_TCP_ESTABLISHED)
        {

            /* Check for data in the current packet.  */
            packet_queued =  _nx_tcp_socket_state_data_check(socket_ptr, packet_ptr);
        }

        /* State processing is complete.  */
        break;

    case  NX_TCP_ESTABLISHED:

        /* Check for data in the current packet.  */
        packet_queued =  _nx_tcp_socket_state_data_check(socket_ptr, packet_ptr);

#ifdef NX_ENABLE_TCPIP_OFFLOAD
        if (!tcpip_offload)
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
        {

            /* Call the ESTABLISHED state handling function to process any state
            changes caused by this new packet.  */
            _nx_tcp_socket_state_established(socket_ptr);

            /* Determine if any transmit suspension can be lifted.  */
            _nx_tcp_socket_state_transmit_check(socket_ptr);
        }

        /* State processing is complete.  */
        break;

    case  NX_TCP_CLOSE_WAIT:

        /* Determine if any transmit suspension can be lifted.  */
        _nx_tcp_socket_state_transmit_check(socket_ptr);

        /* State processing is complete.  */
        break;

    case  NX_TCP_LAST_ACK:

        /* Call the LAST ACK state handling function to process any state
           changes caused by this new packet.  */
        _nx_tcp_socket_state_last_ack(socket_ptr, &tcp_header_copy);

        /* State processing is complete.  */
        break;

    case  NX_TCP_FIN_WAIT_1:

        /* Check for data in the current packet.  */
        packet_queued =  _nx_tcp_socket_state_data_check(socket_ptr, packet_ptr);

        /* Call the FIN WAIT 1 state handling function to process any state
           changes caused by this new packet.  */
        _nx_tcp_socket_state_fin_wait1(socket_ptr);

        /* State processing is complete.  */
        break;

    case  NX_TCP_FIN_WAIT_2:

        /* Check for data in the current packet.  */
        packet_queued =  _nx_tcp_socket_state_data_check(socket_ptr, packet_ptr);

        /* Call the FIN WAIT 2 state handling function to process any state
           changes caused by this new packet.  */
        _nx_tcp_socket_state_fin_wait2(socket_ptr);

        /* State processing is complete.  */
        break;

    case  NX_TCP_CLOSING:

        /* Call the CLOSING state handling function to process any state
           changes caused by this new packet.  */
        _nx_tcp_socket_state_closing(socket_ptr, &tcp_header_copy);

        /* State processing is complete.  */
        break;

    case  NX_TCP_TIMED_WAIT:

        /* State processing is complete.  */
        break;

    default:
        break;
    }

#ifdef NX_ENABLE_HTTP_PROXY

    /* Check if HTTP Proxy is started and waiting for the response form the HTTP Proxy server.  */
    if ((socket_ptr -> nx_tcp_socket_state == NX_TCP_ESTABLISHED) &&
        (socket_ptr -> nx_tcp_socket_ip_ptr -> nx_ip_http_proxy_enable) &&
        (socket_ptr -> nx_tcp_socket_http_proxy_state == NX_HTTP_PROXY_STATE_CONNECTING))
    {

        /* Receive and process the response.  */
        _nx_http_proxy_client_connect_response_process(socket_ptr);
    }
#endif /* NX_ENABLE_HTTP_PROXY */

    /* Check for an URG (urgent) bit set.  */
    /*lint -e{644} suppress variable might not be initialized, since "tcp_header_copy" was initialized. */
    if (tcp_header_copy.nx_tcp_header_word_3 & NX_TCP_URG_BIT)
    {

        /* Yes, an Urgent bit is set.  */

        /* Pickup the urgent callback function specified when the socket was created.  */
        urgent_callback =  socket_ptr -> nx_tcp_urgent_data_callback;

        /* Determine if there is an urgent callback function specified.  */
        if (urgent_callback)
        {

            /* Yes, call the application's urgent callback function to alert the application
               of the presence of the urgent bit.  */
            (urgent_callback)(socket_ptr);
        }
    }

    /* Determine if we need to release the packet.  */
    if (!packet_queued)
    {

        /* Yes, the packet was not queued up above, so it needs to be released.  */
        _nx_packet_release(packet_ptr);
    }
}

