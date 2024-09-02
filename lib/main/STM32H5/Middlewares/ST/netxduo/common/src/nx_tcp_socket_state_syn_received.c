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
/*    _nx_tcp_socket_state_syn_received                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes packets during the SYN RECEIVED state,      */
/*    which is the state after the initial SYN message was responded to   */
/*    with an SYN/ACK message.  The expected value here is an ACK, which  */
/*    will move us into an ESTABLISHED state ready for sending and        */
/*    receiving of TCP data.                                              */
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
/*    _nx_tcp_socket_thread_resume          Resume suspended thread       */
/*    _nx_tcp_packet_send_rst               Send RST packet               */
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
/*                                                                        */
/**************************************************************************/
VOID  _nx_tcp_socket_state_syn_received(NX_TCP_SOCKET *socket_ptr, NX_TCP_HEADER *tcp_header_ptr)
{


    /* Determine if the incoming message is an ACK message.  If it is and
       if it is proper, move into the ESTABLISHED state.  */
    if (tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_ACK_BIT)
    {

        if (tcp_header_ptr -> nx_tcp_acknowledgment_number == socket_ptr -> nx_tcp_socket_tx_sequence)
        {

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_ESTABLISHED, NX_TRACE_INTERNAL_EVENTS, 0, 0);

            /* Save the window size.  */
            socket_ptr -> nx_tcp_socket_tx_window_advertised =
                tcp_header_ptr -> nx_tcp_header_word_3 & NX_LOWER_16_MASK;

#ifdef NX_ENABLE_TCP_WINDOW_SCALING

            /* The window size advertised in the SYN packet is NEVER scaled. Therefore there is no
               need to apply the scale shift.  However validate snd_win_scale  and rcv_win_scale. */
            if (socket_ptr -> nx_tcp_snd_win_scale_value == 0xFF)
            {

                /* Peer does not support window scale option. */
                socket_ptr -> nx_tcp_snd_win_scale_value = 0;
                socket_ptr -> nx_tcp_rcv_win_scale_value = 0;

                /* Since peer does not offer window scaling feature, make sure
                   our default window size for this connection does not exceed 65535 bytes. */
                if (socket_ptr -> nx_tcp_socket_rx_window_maximum > 65535)
                {
                    socket_ptr -> nx_tcp_socket_rx_window_default = 65535;
                    socket_ptr -> nx_tcp_socket_rx_window_current = 65535;
                }
            }

            /* Updated the window size.  */
            socket_ptr -> nx_tcp_socket_tx_window_advertised <<= socket_ptr -> nx_tcp_snd_win_scale_value;

#endif /* NX_ENABLE_TCP_WINDOW_SCALING  */

            /* Set the initial slow start threshold to be the advertised window size. */
            socket_ptr -> nx_tcp_socket_tx_slow_start_threshold = socket_ptr -> nx_tcp_socket_tx_window_advertised;

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

            /* Move into the ESTABLISHED state.  */
            socket_ptr -> nx_tcp_socket_state =  NX_TCP_ESTABLISHED;
#ifndef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT

            /* If registered with the TCP socket, call the application's connection completion callback function.  */
            if (socket_ptr -> nx_tcp_establish_notify)
            {

                /* Call the application's establish callback function.    */
                (socket_ptr -> nx_tcp_establish_notify)(socket_ptr);
            }
#endif


#ifdef NX_ENABLE_TCP_KEEPALIVE
            /* Is the keepalive feature enabled on this socket? */
            if (socket_ptr -> nx_tcp_socket_keepalive_enabled)
            {
                /* Setup the TCP Keepalive timer to initial values.  */
                socket_ptr -> nx_tcp_socket_keepalive_timeout =  NX_TCP_KEEPALIVE_INITIAL;
                socket_ptr -> nx_tcp_socket_keepalive_retries =  0;
            }
#endif
            /* Update the value of nx_tcp_socket_rx_sequence_acked */
            socket_ptr -> nx_tcp_socket_rx_sequence_acked =    socket_ptr -> nx_tcp_socket_rx_sequence;

            /* Determine if we need to wake a thread suspended on the connection.  */
            if (socket_ptr -> nx_tcp_socket_connect_suspended_thread)
            {

                /* Resume the suspended thread.  */
                _nx_tcp_socket_thread_resume(&(socket_ptr -> nx_tcp_socket_connect_suspended_thread), NX_SUCCESS);
            }
        }
        /* Check for an invalid ACK message that signals an error on the other side.  */
        else
        {

            /* Invalid response was received, it is likely that the other side still
               thinks a previous connection is active.  Send a reset (RST) message to
               the other side to clear any previous connection.  */

            /* Send the RST packet.  */
            _nx_tcp_packet_send_rst(socket_ptr, tcp_header_ptr);
        }
    }
}

