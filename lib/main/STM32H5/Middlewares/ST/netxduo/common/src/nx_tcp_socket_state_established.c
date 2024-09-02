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
#ifdef NX_ENABLE_HTTP_PROXY
#include "nx_http_proxy_client.h"
#endif /* NX_ENABLE_HTTP_PROXY */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_state_established                    PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes packets during the ESTABLISHED state,       */
/*    the state of the socket during typical TCP data sending and         */
/*    receiving.  The expected packet that changes state once in the      */
/*    established state is the FIN packet.  Reception of this will move   */
/*    the state for this socket to the CLOSE WAIT state.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to owning socket      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_packet_send_ack               Send ACK message              */
/*    (nx_tcp_disconnect_callback)          Application's callback        */
/*                                            function when disconnect    */
/*                                            FIN is received             */
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
VOID  _nx_tcp_socket_state_established(NX_TCP_SOCKET *socket_ptr)
{
#if !defined(NX_DISABLE_TCP_INFO) || defined(TX_ENABLE_EVENT_TRACE)
NX_IP *ip_ptr;


    /* Setup the IP pointer.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;
#endif
    /* Determine if a FIN has been previously detected in the _nx_tcp_socket_state_data_check
       routine and if the socket's sequence number matches the expected FIN sequence number.  */
    if ((socket_ptr -> nx_tcp_socket_fin_received) &&
        (socket_ptr -> nx_tcp_socket_fin_sequence == socket_ptr -> nx_tcp_socket_rx_sequence))
    {

#ifndef NX_DISABLE_TCP_INFO
        /* Increment the TCP disconnections count.  */
        ip_ptr -> nx_ip_tcp_disconnections++;
#endif

#ifdef NX_ENABLE_TCP_KEEPALIVE
        /* Is the keepalive feature enabled on this socket? */
        if (socket_ptr -> nx_tcp_socket_keepalive_enabled)
        {
            /* Clear the TCP Keepalive timer to disable it for this socket (only needed when
               the socket is connected.  */
            socket_ptr -> nx_tcp_socket_keepalive_timeout =  0;
            socket_ptr -> nx_tcp_socket_keepalive_retries =  0;
        }
#endif

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_CLOSE_WAIT, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* The FIN bit is set, we need to go into the finished state.  */
        socket_ptr -> nx_tcp_socket_state =  NX_TCP_CLOSE_WAIT;

        /* Increment the received sequence.  */
        socket_ptr -> nx_tcp_socket_rx_sequence++;

        /* Loop to release all threads suspended while trying to receive on the socket.  */
        while (socket_ptr -> nx_tcp_socket_receive_suspension_list)
        {

            /* Release the head of the receive suspension list. */
            _nx_tcp_receive_cleanup(socket_ptr -> nx_tcp_socket_receive_suspension_list NX_CLEANUP_ARGUMENT);
        }

        /* Send ACK message.  */
        _nx_tcp_packet_send_ack(socket_ptr, socket_ptr -> nx_tcp_socket_tx_sequence);

#ifdef NX_ENABLE_HTTP_PROXY
        if ((ip_ptr -> nx_ip_http_proxy_enable) &&
            (socket_ptr -> nx_tcp_socket_http_proxy_state == NX_HTTP_PROXY_STATE_CONNECTING))
        {

            /* If received FIN before HTTP Proxy connection established, disconnect the TCP connection
               and don't notify the application.  */
            _nx_tcp_socket_disconnect(socket_ptr, NX_NO_WAIT);
        }
        else
#endif /* NX_ENABLE_HTTP_PROXY */
        {

            /* If given, call the application's disconnect callback function
               for disconnect.  */
            if (socket_ptr -> nx_tcp_disconnect_callback)
            {

                /* Call the application's disconnect handling function.  It is
                   responsible for calling the socket disconnect function.  */
                (socket_ptr -> nx_tcp_disconnect_callback)(socket_ptr);
            }
        }
    }
}

