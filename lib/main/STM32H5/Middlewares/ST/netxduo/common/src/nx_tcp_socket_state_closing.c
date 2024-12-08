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
#include "nx_ipv6.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_state_closing                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes packets during the CLOSING state,           */
/*    which is the state at the end of an active, simultaneous            */
/*    disconnect.  If a valid ACK is present, set the socket state        */
/*    to TIMED WAIT.                                                      */
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
/*    _nx_tcp_packet_send_ack               Send ACK message              */
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
VOID  _nx_tcp_socket_state_closing(NX_TCP_SOCKET *socket_ptr, NX_TCP_HEADER *tcp_header_ptr)
{


    /* Determine if the incoming message is an ACK message.  */
    if (tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_ACK_BIT)
    {

        /* If it is proper, finish the disconnect. */
        if ((tcp_header_ptr -> nx_tcp_acknowledgment_number == socket_ptr -> nx_tcp_socket_tx_sequence) &&
            (tcp_header_ptr -> nx_tcp_sequence_number == socket_ptr -> nx_tcp_socket_rx_sequence))
        {

            /* Return to the proper socket state.  */

            /* Yes, return the socket to the TIMED WAIT state. */

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_TIMED_WAIT, NX_TRACE_INTERNAL_EVENTS, 0, 0);

            /* Set the socket state to TIMED WAIT now.  */
            socket_ptr -> nx_tcp_socket_state = NX_TCP_TIMED_WAIT;

            /* Set the timeout as 2MSL (Maximum Segment Lifetime). */
            socket_ptr -> nx_tcp_socket_timeout = _nx_tcp_2MSL_timer_rate;

            /* Determine if we need to wake a thread suspended on the connection.  */
            if (socket_ptr -> nx_tcp_socket_disconnect_suspended_thread)
            {

                /* Resume the thread suspended for the disconnect.  */
                _nx_tcp_socket_thread_resume(&(socket_ptr -> nx_tcp_socket_disconnect_suspended_thread), NX_SUCCESS);
            }

            /* If given, call the application's disconnect callback function
               for disconnect.  */
            if (socket_ptr -> nx_tcp_disconnect_callback)
            {

                /* Call the application's disconnect handling function.  It is
                   responsible for calling the socket disconnect function.  */
                (socket_ptr -> nx_tcp_disconnect_callback)(socket_ptr);
            }

#ifndef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT

            /* Is a timed wait callback registered for this socket?  */
            if (socket_ptr -> nx_tcp_timed_wait_callback)
            {

                /* Call the timed wait callback for this socket to let the host
                   know the socket can now be put in the timed wait state (if
                   the RE-USE ADDRESS socket option is not enabled). */
                (socket_ptr -> nx_tcp_timed_wait_callback)(socket_ptr);
            }

            /* Is a disconnect complete callback registered with the TCP socket? */
            if (socket_ptr -> nx_tcp_disconnect_complete_notify)
            {

                /* Call the application's disconnect_complete callback function.    */
                (socket_ptr -> nx_tcp_disconnect_complete_notify)(socket_ptr);
            }
#endif
        }

        /* Ignore the segment.  According to RFC 793, Section 3.9, Page 73.  */
    }
}

