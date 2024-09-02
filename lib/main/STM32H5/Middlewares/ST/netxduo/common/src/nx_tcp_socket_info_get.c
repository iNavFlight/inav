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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_info_get                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retrieves TCP information for the specified TCP       */
/*    socket.                                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to the TCP socket     */
/*    tcp_packets_sent                      Destination for number of     */
/*                                            packets sent                */
/*    tcp_bytes_sent                        Destination for number of     */
/*                                            bytes sent                  */
/*    tcp_packets_received                  Destination for number of     */
/*                                            packets received            */
/*    tcp_bytes_received                    Destination for number of     */
/*                                            bytes received              */
/*    tcp_retransmit_packets                Destination for number of     */
/*                                            retransmit packets          */
/*    tcp_packets_queued                    Destination for number of     */
/*                                            receive packets queued      */
/*    tcp_checksum_errors                   Destination for number of     */
/*                                            checksum errors             */
/*    tcp_socket_state                      Destination for the current   */
/*                                            socket state                */
/*    tcp_transmit_queue_depth              Destination for number of     */
/*                                            sockets still in transmit   */
/*                                            queue                       */
/*    tcp_transmit_window                   Destination for number of     */
/*                                            bytes in transmit window    */
/*    tcp_receive_window                    Destination for number of     */
/*                                            bytes in receive window     */
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
UINT  _nx_tcp_socket_info_get(NX_TCP_SOCKET *socket_ptr, ULONG *tcp_packets_sent, ULONG *tcp_bytes_sent,
                              ULONG *tcp_packets_received, ULONG *tcp_bytes_received,
                              ULONG *tcp_retransmit_packets, ULONG *tcp_packets_queued,
                              ULONG *tcp_checksum_errors, ULONG *tcp_socket_state,
                              ULONG *tcp_transmit_queue_depth, ULONG *tcp_transmit_window,
                              ULONG *tcp_receive_window)
{

NX_IP *ip_ptr;


    /* Setup IP pointer.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_SOCKET_INFO_GET, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_packets_sent, socket_ptr -> nx_tcp_socket_bytes_received, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Obtain the IP mutex so we can examine the bound port.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if packets sent is wanted.  */
    if (tcp_packets_sent)
    {

        /* Return the number of packets sent by this socket.  */
        *tcp_packets_sent =  socket_ptr -> nx_tcp_socket_packets_sent;
    }

    /* Determine if bytes sent is wanted.  */
    if (tcp_bytes_sent)
    {

        /* Return the number of bytes sent by this socket.  */
        *tcp_bytes_sent =  socket_ptr -> nx_tcp_socket_bytes_sent;
    }

    /* Determine if packets received is wanted.  */
    if (tcp_packets_received)
    {

        /* Return the number of packets received by this socket.  */
        *tcp_packets_received =  socket_ptr -> nx_tcp_socket_packets_received;
    }

    /* Determine if bytes received is wanted.  */
    if (tcp_bytes_received)
    {

        /* Return the number of bytes received by this socket.  */
        *tcp_bytes_received =  socket_ptr -> nx_tcp_socket_bytes_received;
    }

    /* Determine if retransmit packets is wanted.  */
    if (tcp_retransmit_packets)
    {

        /* Return the number of retransmit packets by this socket.  */
        *tcp_retransmit_packets =  socket_ptr -> nx_tcp_socket_retransmit_packets;
    }

    /* Determine if packets queued is wanted.  */
    if (tcp_packets_queued)
    {

        /* Return the number of packets queued by this socket.  */
        *tcp_packets_queued =  socket_ptr -> nx_tcp_socket_receive_queue_count;
    }

    /* Determine if checksum errors is wanted.  */
    if (tcp_checksum_errors)
    {

        /* Return the number of checksum errors by this socket.  */
        *tcp_checksum_errors =  socket_ptr -> nx_tcp_socket_checksum_errors;
    }

    /* Determine if socket state is wanted.  */
    if (tcp_socket_state)
    {

        /* Return the state this socket.  */
        *tcp_socket_state =  socket_ptr -> nx_tcp_socket_state;
    }

    /* Determine if transmit queue depth is wanted.  */
    if (tcp_transmit_queue_depth)
    {

        /* Return the transmit queue depth of this socket.  */
        *tcp_transmit_queue_depth =  socket_ptr -> nx_tcp_socket_transmit_sent_count;
    }

    /* Determine if transmit window size is wanted.  */
    if (tcp_transmit_window)
    {

        /* Return the transmit window size of this socket.  */
        if (socket_ptr -> nx_tcp_socket_tx_window_advertised > socket_ptr -> nx_tcp_socket_tx_window_congestion)
        {
            *tcp_transmit_window = socket_ptr -> nx_tcp_socket_tx_window_congestion;
        }
        else
        {
            *tcp_transmit_window = socket_ptr -> nx_tcp_socket_tx_window_advertised;
        }
        if (*tcp_transmit_window > socket_ptr -> nx_tcp_socket_tx_outstanding_bytes)
        {
            *tcp_transmit_window =  *tcp_transmit_window - socket_ptr -> nx_tcp_socket_tx_outstanding_bytes;
        }
        else
        {
            *tcp_transmit_window = 0;
        }
    }

    /* Determine if receive window size is wanted.  */
    if (tcp_receive_window)
    {

        /* Return the receive window size of this socket.  */
        *tcp_receive_window =  socket_ptr -> nx_tcp_socket_rx_window_current;
    }

    /* Release protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return successful completion status.  */
    return(NX_SUCCESS);
}

