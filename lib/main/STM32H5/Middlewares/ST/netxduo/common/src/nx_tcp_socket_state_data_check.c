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
#include "nx_packet.h"
#include "nx_ip.h"
#include "nx_tcp.h"
#ifdef NX_ENABLE_HTTP_PROXY
#include "nx_http_proxy_client.h"
#endif /* NX_ENABLE_HTTP_PROXY */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_state_data_trim                      PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function trims extra bytes from the data packet.               */
/*    This is an internal utility function, only used by                  */
/*    _nx_tcp_socket_state_data_check.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to packet to process  */
/*    amount                                Number of bytes to remove     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release packet on overlap     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_socket_state_data_check       Process TCP packet for socket */
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
VOID _nx_tcp_socket_state_data_trim(NX_PACKET *packet_ptr, ULONG amount)
{
ULONG      bytes_to_keep;
NX_PACKET *work_ptr;

    if (amount >= packet_ptr -> nx_packet_length)
    {
        /* Invalid input. */
        return;
    }

    bytes_to_keep = packet_ptr -> nx_packet_length - amount;

    packet_ptr -> nx_packet_length = bytes_to_keep;

    work_ptr = packet_ptr;

#ifndef NX_DISABLE_PACKET_CHAIN
    /* Walk down the packet chain for the "bytes_to_keep" amount. */
    while (work_ptr)
    {

    NX_PACKET *tmp_ptr;

        /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
        if ((INT)(work_ptr -> nx_packet_append_ptr - work_ptr -> nx_packet_prepend_ptr) < (INT)bytes_to_keep)
        {

            /*lint -e{923} suppress cast of pointer to ULONG.  */
            bytes_to_keep -= (ULONG)((ALIGN_TYPE)work_ptr -> nx_packet_append_ptr - (ALIGN_TYPE)work_ptr -> nx_packet_prepend_ptr);

            work_ptr = work_ptr -> nx_packet_next;

            continue;
        }
#endif /* NX_DISABLE_PACKET_CHAIN */

        /* This is the last packet. */
        work_ptr -> nx_packet_append_ptr = work_ptr -> nx_packet_prepend_ptr + bytes_to_keep;

#ifndef NX_DISABLE_PACKET_CHAIN
        /* Free the rest of the packet chain. */
        tmp_ptr = work_ptr -> nx_packet_next;
        work_ptr -> nx_packet_next = NX_NULL;
        work_ptr = tmp_ptr;

        if (work_ptr)
        {

            /*lint -e{923} suppress cast of ULONG to pointer.  */
            work_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ALLOCATED;

            _nx_packet_release(work_ptr);

            /* All done. Break out of the while loop and return. */
            break;
        }
    }
#endif /* NX_DISABLE_PACKET_CHAIN */
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_state_data_trim_front                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function trims extra bytes from the data packet.               */
/*    This is an internal utility function, only used by                  */
/*    _nx_tcp_socket_state_data_check.                                    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    packet_ptr                            Pointer to packet to process  */
/*    amount                                Number of bytes to remove     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release packet on overlap     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_socket_state_data_check       Process TCP packet for socket */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            verified memmove use cases, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
VOID _nx_tcp_socket_state_data_trim_front(NX_PACKET *packet_ptr, ULONG amount)
{
NX_PACKET *work_ptr = packet_ptr;
ULONG      work_length;

    if (amount >= packet_ptr -> nx_packet_length || amount == 0)
    {
        /* Invalid input. */
        return;
    }

    /* Adjust the packet length.  */
    packet_ptr -> nx_packet_length -= amount;

    /* Move prepend_ptr of first packet to TCP data.  */
    packet_ptr -> nx_packet_prepend_ptr += sizeof(NX_TCP_HEADER);

#ifndef NX_DISABLE_PACKET_CHAIN
    /* Walk down the packet chain for the amount. */
    while (amount)
    {
#endif /* NX_DISABLE_PACKET_CHAIN */

        /* Compute the size of the data portion work_ptr.  */
        /*lint -e{923} suppress cast of pointer to ULONG.  */
        work_length = (ULONG)((ALIGN_TYPE)work_ptr -> nx_packet_append_ptr - (ALIGN_TYPE)work_ptr -> nx_packet_prepend_ptr);

#ifndef NX_DISABLE_PACKET_CHAIN
        if (amount > work_length)
        {

            /* All data in work_ptr need to be trimmed.  */
            if (work_ptr == packet_ptr)
            {

                /* This packet is the header of packet chain.  */
                /* Clear TCP data in this packet.  */
                work_ptr -> nx_packet_append_ptr = work_ptr -> nx_packet_prepend_ptr;
            }
            else
            {

                /* This packet is not the first packet.  */
                /* Remove work_ptr from packet chain.  */
                packet_ptr -> nx_packet_next = work_ptr -> nx_packet_next;

                /* Disconnect work_ptr from the rest of the packet chain. */
                work_ptr -> nx_packet_next = NX_NULL;

                /* Mark the packet as ALLOCATED. */
                /*lint -e{923} suppress cast of ULONG to pointer.  */
                work_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ALLOCATED;

                _nx_packet_release(work_ptr);
            }
            /* Reduce the amount being trimmed.  */
            amount -= work_length;

            /* Move to the next packet. */
            work_ptr = packet_ptr -> nx_packet_next;
        }
        else
        {

            /* This is the last packet to trim.  */

            if (work_ptr == packet_ptr)
            {
#endif /* NX_DISABLE_PACKET_CHAIN */

                /* For the first packet, move data towards the beginning
                   of the packet, right after TCP header.  */
                memmove(packet_ptr -> nx_packet_prepend_ptr, /* Use case of memmove is verified.  */
                        packet_ptr -> nx_packet_prepend_ptr + amount,
                        work_length - amount);
                packet_ptr -> nx_packet_append_ptr -= amount;
#ifndef NX_DISABLE_PACKET_CHAIN
            }
            else
            {

                /* Advance nx_packet_prepend_ptr to where the usable data starts. */
                work_ptr -> nx_packet_prepend_ptr += amount;
            }

            /* Cut down amount*/
            amount = 0;
        }
    }
#endif /* NX_DISABLE_PACKET_CHAIN */

    /* Restore prepend_ptr of first packet to TCP data.  */
    packet_ptr -> nx_packet_prepend_ptr -= sizeof(NX_TCP_HEADER);
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_state_data_check                     PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function looks for incoming data from packets received         */
/*    primarily in the ESTABLISHED state, but also in the later states    */
/*    of connection and the early disconnection states.                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to owning socket      */
/*    packet_ptr                            Pointer to packet to process  */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_TRUE                               If the packet had data        */
/*    NX_FALSE                              If the packet had no data     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release packet on overlap     */
/*    _nx_tcp_socket_thread_resume          Resume suspended thread       */
/*    _nx_tcp_packet_send_ack               Send immediate ACK            */
/*    (nx_tcp_receive_callback)             Packet receive notify function*/
/*    _nx_tcp_socket_state_data_trim        Trim off extra bytes          */
/*    _nx_tcp_socket_state_data_trim_front  Trim off front extra bytes    */
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
UINT  _nx_tcp_socket_state_data_check(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr)
{

NX_TCP_HEADER *tcp_header_ptr;
NX_TCP_HEADER *search_header_ptr;
NX_PACKET     *search_ptr;
NX_PACKET     *previous_ptr;
ULONG          expected_sequence;
ULONG          header_length;
ULONG          search_header_length;
ULONG          packet_begin_sequence;
ULONG          packet_end_sequence;
ULONG          packet_data_length;
ULONG          search_begin_sequence;
ULONG          search_end_sequence;
ULONG          original_rx_sequence;
ULONG          trim_data_length;
TX_THREAD     *thread_ptr;
ULONG          acked_packets = 0;
UINT           need_ack = NX_FALSE;
#ifdef NX_ENABLE_TCPIP_OFFLOAD
ULONG          tcpip_offload;
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
#ifdef NX_ENABLE_LOW_WATERMARK
UCHAR          drop_packet = NX_FALSE;
#endif /* NX_ENABLE_LOW_WATERMARK */
#if ((!defined(NX_DISABLE_TCP_INFO)) || defined(TX_ENABLE_EVENT_TRACE))
NX_IP         *ip_ptr;

    /* Setup the IP pointer.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;
#endif

#ifdef NX_ENABLE_TCPIP_OFFLOAD
    tcpip_offload = socket_ptr -> nx_tcp_socket_connect_interface -> nx_interface_capability_flag &
                    NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD;
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

    /* Pickup the pointer to the head of the TCP packet.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    tcp_header_ptr =  (NX_TCP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

    /* Determine the size of the TCP header.  */
    header_length =  (tcp_header_ptr -> nx_tcp_header_word_3 >> NX_TCP_HEADER_SHIFT) * (ULONG)sizeof(ULONG);

    /* Record the original rx_sequence. */
    original_rx_sequence = socket_ptr -> nx_tcp_socket_rx_sequence;

    /* Pickup the begin sequence of this packet. */
    packet_begin_sequence = tcp_header_ptr -> nx_tcp_sequence_number;

    /* Calculate the data length in the packet.  */
    packet_data_length = packet_ptr -> nx_packet_length - header_length;

    /* Pickup the end sequence of this packet. The end sequence is one byte to the last byte in this packet. */
    packet_end_sequence =  tcp_header_ptr -> nx_tcp_sequence_number + packet_data_length;

    /* Trim the data that out of the receive window, make sure all data are in receive window.  */
    if (packet_data_length
#ifdef NX_ENABLE_TCPIP_OFFLOAD
        && (!tcpip_offload)
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
       )
    {

        /* Step1. trim the data on the left side of the receive window.  */
        if (((INT)(socket_ptr -> nx_tcp_socket_rx_sequence - packet_begin_sequence)) > 0)
        {

            /* Calculate the data length that out of window.  */
            trim_data_length = socket_ptr -> nx_tcp_socket_rx_sequence - packet_begin_sequence;

            /* Trim the data that exceed the receive window.  */
            _nx_tcp_socket_state_data_trim_front(packet_ptr, trim_data_length);

            /* Fix the sequence of this packet. */
            tcp_header_ptr -> nx_tcp_sequence_number += trim_data_length;

            /* Update the data length and begin sequence.  */
            packet_data_length -= trim_data_length;
            packet_begin_sequence += trim_data_length;
        }

        /* Step2. trim the data on the right side of the receive window.  */
        if (((INT)((packet_end_sequence - socket_ptr -> nx_tcp_socket_rx_sequence) -
                   socket_ptr -> nx_tcp_socket_rx_window_current)) > 0)
        {

            /* Calculate the data length that out of window.  */
            trim_data_length = packet_end_sequence - (socket_ptr -> nx_tcp_socket_rx_sequence + socket_ptr -> nx_tcp_socket_rx_window_current);

            /* Trim the data that exceed the receive window.  */
            _nx_tcp_socket_state_data_trim(packet_ptr, trim_data_length);

            /* Update the data length and end sequence.  */
            packet_data_length -= trim_data_length;
            packet_end_sequence -= trim_data_length;
        }
    }

    /* Determine if the packet has the FIN bit set to signal a disconnect.  */
    if (tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_FIN_BIT)
    {

        /* Setup the FIN sequence number that we need to look at.  */
        socket_ptr -> nx_tcp_socket_fin_sequence =  tcp_header_ptr -> nx_tcp_sequence_number + packet_data_length;

        /* Indicate that the FIN sequence is now valid.  Once the receive chain is complete
           we will process (ACK) the FIN command which is part of a disconnect started by the
           other side of the connection.  */
        socket_ptr -> nx_tcp_socket_fin_received =  NX_TRUE;

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_FIN_RECEIVE, ip_ptr, socket_ptr, packet_ptr, tcp_header_ptr -> nx_tcp_sequence_number, NX_TRACE_INTERNAL_EVENTS, 0, 0);
    }

    /* Compute the amount of payload data in this packet. */
    if (packet_data_length == 0)
    {
        /* This packet does not contain TCP data payload.  */

        /* Check for invalid sequence number.  */
        if ((socket_ptr -> nx_tcp_socket_state == NX_TCP_ESTABLISHED) &&
            (socket_ptr -> nx_tcp_socket_receive_queue_count == 0) &&
            (socket_ptr -> nx_tcp_socket_rx_sequence != tcp_header_ptr -> nx_tcp_sequence_number) &&
            ((socket_ptr -> nx_tcp_socket_rx_sequence - 1) != tcp_header_ptr -> nx_tcp_sequence_number))
        {

            /* Send an immediate ACK.  */
            _nx_tcp_packet_send_ack(socket_ptr, socket_ptr -> nx_tcp_socket_tx_sequence);
        }

        /* This packet does not have data, so return false. */
        return(NX_FALSE);
    }


    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_DATA_RECEIVE, ip_ptr, socket_ptr, packet_ptr, tcp_header_ptr -> nx_tcp_sequence_number, NX_TRACE_INTERNAL_EVENTS, 0, 0);

    /* Ensure the next pointer in the packet is set to NULL, which will indicate to the
       receive logic that it is not yet part of a contiguous stream.  */
    packet_ptr -> nx_packet_queue_next =  (NX_PACKET *)NX_NULL;

    /* Otherwise, the packet is within the receive window so continue processing
       the incoming TCP data.  */

    /* Pickup the tail pointer of the receive queue.  */
    search_ptr = socket_ptr -> nx_tcp_socket_receive_queue_tail;

    /* Check to see if the tail pointer is part of a contiguous stream.  */
    if (search_ptr)
    {

        /* Setup a pointer to header of this packet in the sent list.  */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        search_header_ptr =  (NX_TCP_HEADER *)search_ptr -> nx_packet_prepend_ptr;

        /* Determine the size of the search TCP header.  */
        search_header_length =  (search_header_ptr -> nx_tcp_header_word_3 >> NX_TCP_HEADER_SHIFT) * (ULONG)sizeof(ULONG);

        /* Now see if the current sequence number accounts for the last packet.  */
        search_end_sequence = search_header_ptr -> nx_tcp_sequence_number  + search_ptr -> nx_packet_length - search_header_length;
    }
    else
    {

        /* Set the sequence number to the socket's receive sequence if there isn't a receive
           packet on the queue.  */
        search_end_sequence =  socket_ptr -> nx_tcp_socket_rx_sequence;
    }

    /* Does the number of queued packets exceed the maximum rx queue length, or the number of
     * available packets in the default packet pool reaches low watermark? */
#ifdef NX_ENABLE_LOW_WATERMARK
    if (((socket_ptr -> nx_tcp_socket_receive_queue_count >=
          socket_ptr -> nx_tcp_socket_receive_queue_maximum) ||
         (packet_ptr -> nx_packet_pool_owner -> nx_packet_pool_available <
          packet_ptr -> nx_packet_pool_owner -> nx_packet_pool_low_watermark))
#ifdef NX_ENABLE_TCPIP_OFFLOAD
         && (!tcpip_offload)
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
       )
    {

        /* Yes it is. The last packet in queue should be dropped. */
        drop_packet = NX_TRUE;
    }
#endif /* NX_ENABLE_LOW_WATERMARK */

    /* Determine if we have a simple case of TCP data coming in the correct order.  This means
       the socket's sequence number matches the incoming packet sequence number and the last packet's
       data on the socket's receive queue (if any) matches the current sequence number.  */
    if (((tcp_header_ptr -> nx_tcp_sequence_number == socket_ptr -> nx_tcp_socket_rx_sequence) &&
         (search_end_sequence == socket_ptr -> nx_tcp_socket_rx_sequence))
#ifdef NX_ENABLE_TCPIP_OFFLOAD
         || tcpip_offload
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
       )
    {

        /* Yes, this is the simple case of adding receive packets in sequence.  */
#ifdef NX_ENABLE_LOW_WATERMARK
        /* If this packet is to be dropped, do nothing. */
        if (drop_packet == NX_FALSE)
        {
#endif /* NX_ENABLE_LOW_WATERMARK */

            /* Mark the packet as ready. This is done to simplify the logic in socket receive.  */
            /*lint -e{923} suppress cast of ULONG to pointer.  */
            packet_ptr -> nx_packet_queue_next =  (NX_PACKET *)NX_PACKET_READY;

            /* Add debug information. */
            NX_PACKET_DEBUG(NX_PACKET_TCP_RECEIVE_QUEUE, __LINE__, packet_ptr);

            /* Place the packet on the receive queue.  Search pointer still points to the tail packet on
               the queue.  */
            if (search_ptr)
            {

                /* Nonempty receive queue, add packet to the end of the receive queue.  */
                search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next =  packet_ptr;

                /* Update the tail of the receive queue.  */
                socket_ptr -> nx_tcp_socket_receive_queue_tail =  packet_ptr;
            }
            else
            {

                /* Empty receive queue.  Set both the head and the tail pointers this packet.  */
                socket_ptr -> nx_tcp_socket_receive_queue_head =  packet_ptr;
                socket_ptr -> nx_tcp_socket_receive_queue_tail =  packet_ptr;

                /* Setup a new delayed ACK timeout.  */
#ifdef NX_ENABLE_TCPIP_OFFLOAD
                if (!tcpip_offload)
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
                {
                    socket_ptr -> nx_tcp_socket_delayed_ack_timeout =  _nx_tcp_ack_timer_rate;
                }
            }

            /* Increment the receive TCP packet count.  */
            socket_ptr -> nx_tcp_socket_receive_queue_count++;

            /* Set the next pointer to indicate the packet is part of a TCP queue.  */
            /*lint -e{923} suppress cast of ULONG to pointer.  */
            packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next =  (NX_PACKET *)NX_PACKET_ENQUEUED;

            /* Calculate the next sequence number.  */
            socket_ptr -> nx_tcp_socket_rx_sequence =  packet_end_sequence;

            /* All packets can be acked. */
            acked_packets = socket_ptr -> nx_tcp_socket_receive_queue_count;
#ifdef NX_ENABLE_LOW_WATERMARK
        }
        else
        {

            /* Release this packet. */
            _nx_packet_release(packet_ptr);

            /* Set window to zero. */
            socket_ptr -> nx_tcp_socket_rx_window_current = 0;
            socket_ptr -> nx_tcp_socket_rx_window_last_sent = 0;

            /* Send zero window. */
            _nx_tcp_packet_send_ack(socket_ptr, socket_ptr -> nx_tcp_socket_tx_sequence);

            /* All packets can be acked. */
            acked_packets = socket_ptr -> nx_tcp_socket_receive_queue_count;
        }
#endif /* NX_ENABLE_LOW_WATERMARK */

        /* End of the simple case: add new packet towards the end of the recv queue.
           All packets in the receive queue are in sequence. */
    }
    else if (socket_ptr -> nx_tcp_socket_receive_queue_head == NX_NULL)
    {

#ifdef NX_ENABLE_LOW_WATERMARK
        /* If this packet is to be dropped, do nothing. */
        if (drop_packet == NX_FALSE)
        {
#endif /* NX_ENABLE_LOW_WATERMARK */

            /* Packet data begins to the right of the expected sequence (out of sequence data). Force an ACK. */
            _nx_tcp_packet_send_ack(socket_ptr, socket_ptr -> nx_tcp_socket_tx_sequence);

            /* Add debug information. */
            NX_PACKET_DEBUG(NX_PACKET_TCP_RECEIVE_QUEUE, __LINE__, packet_ptr);

            /* There are no packets chained on the receive queue.  Simply add the
               new packet to the receive queue. */
            socket_ptr -> nx_tcp_socket_receive_queue_head = packet_ptr;
            socket_ptr -> nx_tcp_socket_receive_queue_tail = packet_ptr;

            /* Increase the receive queue count. */
            socket_ptr -> nx_tcp_socket_receive_queue_count = 1;

            /* Setup a new delayed ACK timeout.  */
            socket_ptr -> nx_tcp_socket_delayed_ack_timeout =  _nx_tcp_ack_timer_rate;

            /* Mark the packet as being part of a TCP queue.  */
            /*lint -e{923} suppress cast of ULONG to pointer.  */
            packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next =  (NX_PACKET *)NX_PACKET_ENQUEUED;
#ifdef NX_ENABLE_LOW_WATERMARK
        }
        else
        {

            /* Release this packet. */
            _nx_packet_release(packet_ptr);

            /* Set window to zero. */
            socket_ptr -> nx_tcp_socket_rx_window_current = 0;
            socket_ptr -> nx_tcp_socket_rx_window_last_sent = 0;

            /* Send zero window. */
            _nx_tcp_packet_send_ack(socket_ptr, socket_ptr -> nx_tcp_socket_tx_sequence);
        }
#endif /* NX_ENABLE_LOW_WATERMARK */
    }
    else
    {

        /* Out of order insertion with packets on the queue. */

        /* Either this packet is not in order or other out-of-order packets have already been
           received.  In this case searching the receive list must be done to find the proper
           place for the new packet.  */

        /* Go through the received packet chain, and locate the first packet that the
           packet_begin_sequence is to the right of the end of it. */

        /* Packet data begins to the right of the expected sequence (out of sequence data). Force an ACK. */
        if (((INT)(packet_begin_sequence - socket_ptr -> nx_tcp_socket_rx_sequence)) > 0)
        {
            _nx_tcp_packet_send_ack(socket_ptr, socket_ptr -> nx_tcp_socket_tx_sequence);
        }

        /* At this point, it is guaranteed that the receive queue contains packets. */
        search_ptr = socket_ptr -> nx_tcp_socket_receive_queue_head;

        previous_ptr = NX_NULL;

        while (search_ptr)
        {

            /*lint -e{923} suppress cast of ULONG to pointer.  */
            if (search_ptr == (NX_PACKET *)NX_PACKET_ENQUEUED)
            {
                /* We hit the end of the receive queue. */
                search_ptr = NX_NULL;

                /* Terminate the out-of-order search.  */
                break;
            }

            /* Setup a pointer to header of this packet in the receive list.  */
            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            search_header_ptr =  (NX_TCP_HEADER *)search_ptr -> nx_packet_prepend_ptr;

            search_begin_sequence = search_header_ptr -> nx_tcp_sequence_number;

            /* Calculate the header size for this packet.  */
            header_length =  (search_header_ptr -> nx_tcp_header_word_3 >> NX_TCP_HEADER_SHIFT) * (ULONG)sizeof(ULONG);

            search_end_sequence = search_begin_sequence + search_ptr -> nx_packet_length - header_length;

            /****************************************************************************************
             *  (1)              PPPPPPPP                                                           *
             *        SSSSSSSS                                                                      *
             *        In this configuration, the incoming packet is completely to the right of      *
             *        search_ptr.  Move to the next search packet.                                  *
             *                                                                                      *
             ****************************************************************************************/
            /* packet_ptr is to the right of search_ptr */
            if (((INT)(packet_begin_sequence - search_end_sequence)) >= 0)
            {
                /* Move on to the next packet. */
                previous_ptr = search_ptr;

                search_ptr = search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;

                /* Continue the search */
                continue;
            }

            /****************************************************************************************
             *  (2)   PPPPPP                                                                        *
             *              SSSSSSSSS                                                               *
             *        In this configuration, the incoming packet is completely to the left of       *
             *        search_ptr.  Incoming packet needs to be inserted in front of search ptr.     *
             *                                                                                      *
             ****************************************************************************************/
            if (((INT)(search_begin_sequence - packet_end_sequence)) >= 0)
            {
                /* packet_ptr is to the left of search_ptr. We are done. Break out of the while loop.*/
                break;
            }

            /* At this point search_ptr and packet_ptr overlapps. */
            /****************************************************************************************
             * There are four cases:(P - packet_ptr, S - search_ptr)                                *
             ****************************************************************************************/


            /****************************************************************************************
             *  (3)        PPPPPPPPPPPPPP                                                           *
             *        SSSSSSSSSSSSSSSSSSSSSSSS                                                      *
             *        In this configuration, the incoming packet is the same as the existing one,   *
             *        or is a subset of the existing one.  Remove the incoming packet.  No need     *
             *        to search for contigous data, therefore no need to wake up user thread.       *
             *        Howerver may need to send out ACK if new packet is to the right of the seq    *
             *        number.                                                                       *
             *                                                                                      *
             ****************************************************************************************/
            if ((((INT)(packet_begin_sequence - search_begin_sequence)) >= 0) &&
                (((INT)(search_end_sequence - packet_end_sequence)) >= 0))
            {

                /* Send an immediate ACK.  */
                _nx_tcp_packet_send_ack(socket_ptr, socket_ptr -> nx_tcp_socket_tx_sequence);

                /* Since packet is not queued, return NX_FALSE so the caller releases the packet. */
                return(NX_FALSE);
            }

            /****************************************************************************************
             *  (4)    PPPPPPPPPPPPPPPPPP                                                           *
             *          SSSSSSSSSSSSSSSS                                                            *
             *         In this configuration, New packet is a super-set of an existing packet.      *
             *         Release existing packet, and insert new packet, then check for the next      *
             *         packet on the chain.  The next search may yield case (5).  Need to check     *
             *         for contingous data, may need to send ACK.                                   *
             *                                                                                      *
            ****************************************************************************************/
            if ((((INT)(search_begin_sequence - packet_begin_sequence)) >= 0) &&
                (((INT)(packet_end_sequence - search_end_sequence) >= 0)))
            {
            NX_PACKET *tmp_ptr;
                /* Release the search_ptr, and move to the next packet on the chain. */
                tmp_ptr = search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;

                /* Mark the packet as no longer being part of the TCP queue. */
                /*lint -e{923} suppress cast of ULONG to pointer.  */
                search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ALLOCATED;

                /* Decrease the packet queue count */
                socket_ptr -> nx_tcp_socket_receive_queue_count--;

                /* Adjust the receive window. */

                /* Release the search packet. */
                _nx_packet_release(search_ptr);

#ifndef NX_DISABLE_TCP_INFO
                /* The new packet has been admitted to the receive queue. */

                /* Increment the TCP packet receive count and bytes received count.  */
                ip_ptr -> nx_ip_tcp_packets_received--;
                ip_ptr -> nx_ip_tcp_bytes_received -= (search_end_sequence - search_begin_sequence);

                /* Increment the TCP packet receive count and bytes received count for the socket.  */
                socket_ptr -> nx_tcp_socket_packets_received--;
                socket_ptr -> nx_tcp_socket_bytes_received -= (search_end_sequence - search_begin_sequence);

#endif /* NX_DISABLE_TCP_INFO */

                /* Move to the next packet.  (note: no need to update previous_ptr. */
                search_ptr = tmp_ptr;

                /* Continue the search. */
                continue;
            }


            /****************************************************************************************
             *  (5)    PPPPPPPPPPPPPPPPPP                                                           *
             *                   SSSSSSSSSSSS                                                       *
             *        In this configuration, remove data from the back of the new packet,  insert   *
             *        packet into the chain, and terminate the search.  Need to search for          *
             *        contigous data, may need to send out ACK.                                     *
             ****************************************************************************************/
            if (((INT)(search_begin_sequence - packet_begin_sequence)) >= 0)
            {

                _nx_tcp_socket_state_data_trim(packet_ptr, (packet_end_sequence - search_begin_sequence));

                /* Update packet_data_length. */
                packet_data_length -= (packet_end_sequence - search_begin_sequence);

                /* Now the packet should be chained before search_ptr. */

                break;
            }

            /****************************************************************************************
             *                                                                                      *
             *  (6)        PPPPPPPPPPPPPP                                                           *
             *        SSSSSSSS                                                                      *
             *        In this configuration, remove data from the beginning of the search_ptr,      *
             *        insert the packet after the search packet and continue the search.  This may  *
             *        lead to case (2) and (3).                                                     *
             *                                                                                      *
             *                                                                                      *
             ***************************************************************************************/
            _nx_tcp_socket_state_data_trim(search_ptr, (ULONG)(search_end_sequence - packet_begin_sequence));

#ifndef NX_DISABLE_TCP_INFO
            /* The new packet has been admitted to the receive queue. */

            /* Reduce the TCP bytes received count.  */
            ip_ptr -> nx_ip_tcp_bytes_received -= (search_end_sequence - packet_begin_sequence);

            /* Reduce the TCP bytes received count for the socket.  */
            socket_ptr -> nx_tcp_socket_bytes_received -= (search_end_sequence - packet_begin_sequence);

#endif /* NX_DISABLE_TCP_INFO */

            /* Move to the next packet and continue; */
            previous_ptr = search_ptr;
            search_ptr = search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;
        }   /* End of while (search_ptr) */

        /* At this point, the logic (within the while loop) finds a location where this packet should be inserted. */
        if (previous_ptr == NX_NULL)
        {

            /* The packet needs to be inserted at the beginning of the queue. */
            socket_ptr -> nx_tcp_socket_receive_queue_head = packet_ptr;
        }
        else
        {

            /* The packet needs to be inserted after previous_ptr. */
            previous_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next = packet_ptr;
        }

        if (search_ptr == NX_NULL)
        {

            /* This packet is on the last one on the queue. */
            socket_ptr -> nx_tcp_socket_receive_queue_tail = packet_ptr;

            /* Set the next pointer to indicate the packet is part of a TCP queue.  */
            /*lint -e{923} suppress cast of ULONG to pointer.  */
            packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ENQUEUED;
        }
        else
        {

            /* Chain search_ptr onto packet_ptr. */
            packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next = search_ptr;
        }

        /* Add debug information. */
        NX_PACKET_DEBUG(NX_PACKET_TCP_RECEIVE_QUEUE, __LINE__, packet_ptr);

        /* Increment the receive TCP packet count.  */
        socket_ptr -> nx_tcp_socket_receive_queue_count++;

        /* End of the out-of-order search.  At this point, the packet has been inserted. */

        /* Now we need to figure out how much, if any, we can ACK.  */
        search_ptr =    socket_ptr -> nx_tcp_socket_receive_queue_head;

        /* Get the sequence number expected by the TCP receive socket. */
        expected_sequence =  socket_ptr -> nx_tcp_socket_rx_sequence;

        /* Loop to see how much data is contiguous in the queued packets.  */
        do
        {

            /* Setup a pointer to header of this packet in the sent list.  */
            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            search_header_ptr =  (NX_TCP_HEADER *)search_ptr -> nx_packet_prepend_ptr;


            /* Calculate the header size for this packet.  */
            header_length =  (search_header_ptr -> nx_tcp_header_word_3 >> NX_TCP_HEADER_SHIFT) * (ULONG)sizeof(ULONG);

            search_begin_sequence = search_header_ptr -> nx_tcp_sequence_number;

            search_end_sequence = search_begin_sequence + search_ptr -> nx_packet_length - header_length;

            if ((INT)(expected_sequence - search_begin_sequence) >= 0)
            {

                if ((INT)(search_end_sequence - expected_sequence) > 0)
                {
                    /* Sequence number is within this packet.  Advance sequence number. */
                    expected_sequence = search_end_sequence;

                    socket_ptr -> nx_tcp_socket_rx_sequence = expected_sequence;

                    acked_packets++;

                    /* Mark this packet as ready for retrieval.  */
                    /*lint -e{923} suppress cast of ULONG to pointer.  */
                    search_ptr -> nx_packet_queue_next =  (NX_PACKET *)NX_PACKET_READY;
                }
            }
            else
            {

                /* Expected number is to the left of search_ptr.   Get out of the do-while loop!  */
                break;
            }

            /* Move the search pointer to the next queued receive packet.  */
            search_ptr =  search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;

            /* Determine if we are at the end of the queue.  */
            /*lint -e{923} suppress cast of ULONG to pointer.  */
            if (search_ptr == ((NX_PACKET *)NX_PACKET_ENQUEUED))
            {

                /* At the end, set the search pointer to NULL.  */
                search_ptr =  NX_NULL;
            }

#ifdef NX_ENABLE_LOW_WATERMARK
            /* Do not calculate sequence for packet that is to be dropped. */
            if (drop_packet && (acked_packets >= (ULONG)socket_ptr -> nx_tcp_socket_receive_queue_maximum))
            {

                /* Get out of the loop!  */
                break;
            }
#endif /* NX_ENABLE_LOW_WATERMARK */
        } while (search_ptr);

        /* If there are no packet crosses the rx_sequence, the TCP stream in the receive queue contains
           missing pieces. */

#ifdef NX_ENABLE_LOW_WATERMARK
        /* Does the number of queued packets exceed the maximum rx queue length, or the number of
         * available packets in the default packet pool reaches low watermark? */
        if (drop_packet)
        {

            /* Yes it is. Remove the last packet in queue. */
            socket_ptr -> nx_tcp_socket_receive_queue_tail -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ALLOCATED;
            if (socket_ptr -> nx_tcp_socket_receive_queue_count > 1)
            {

                /* Find the previous packet of tail. */
                search_ptr = socket_ptr -> nx_tcp_socket_receive_queue_head;
                while (search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next != socket_ptr -> nx_tcp_socket_receive_queue_tail)
                {
                    search_ptr = search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;
                }

                /* Release the tail. */
                _nx_packet_release(socket_ptr -> nx_tcp_socket_receive_queue_tail);

                /* Setup the tail packet. */
                socket_ptr -> nx_tcp_socket_receive_queue_tail = search_ptr;

                search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ENQUEUED;

                /* Calculate the ending sequence number for the last packet.  */
                search_header_ptr =  (NX_TCP_HEADER *)search_ptr -> nx_packet_prepend_ptr;
                header_length = (search_header_ptr -> nx_tcp_header_word_3 >> NX_TCP_HEADER_SHIFT) * sizeof(ULONG);

                /* Decrease window size. */
                socket_ptr -> nx_tcp_socket_rx_window_current = search_header_ptr -> nx_tcp_sequence_number +
                    (search_ptr -> nx_packet_length - header_length) -
                    socket_ptr -> nx_tcp_socket_rx_sequence;
                socket_ptr -> nx_tcp_socket_rx_window_last_sent = socket_ptr -> nx_tcp_socket_rx_window_current;
            }
            else
            {

                /* Release the tail. */
                _nx_packet_release(socket_ptr -> nx_tcp_socket_receive_queue_tail);

                /* Clear the head and tail packets. */
                socket_ptr -> nx_tcp_socket_receive_queue_head = NX_NULL;
                socket_ptr -> nx_tcp_socket_receive_queue_tail = NX_NULL;

                /* No packets can be received. */
                socket_ptr -> nx_tcp_socket_rx_window_current = 0;
                socket_ptr -> nx_tcp_socket_rx_window_last_sent = 0;
            }

            /* Decrease receive queue count. */
            socket_ptr -> nx_tcp_socket_receive_queue_count--;
        }
#endif /* NX_ENABLE_LOW_WATERMARK */
    }   /* End of out-of-order insertion. */


#ifndef NX_DISABLE_TCP_INFO
    /* The new packet has been admitted to the receive queue. */

    /* Increment the TCP packet receive count and bytes received count.  */
    ip_ptr -> nx_ip_tcp_packets_received++;
    ip_ptr -> nx_ip_tcp_bytes_received += packet_data_length;

    /* Increment the TCP packet receive count and bytes received count for the socket.  */
    socket_ptr -> nx_tcp_socket_packets_received++;
    socket_ptr -> nx_tcp_socket_bytes_received += packet_data_length;
#endif

    /* Check if the rx sequence number has been updated.  */
    if (original_rx_sequence != socket_ptr -> nx_tcp_socket_rx_sequence)
    {

        /* Decrease the receive window size since rx_sequence is updated.  */
        socket_ptr -> nx_tcp_socket_rx_window_current -= (socket_ptr -> nx_tcp_socket_rx_sequence - original_rx_sequence);

        /* Update the rx_window_last_sent for SWS avoidance algorithm.
           RFC1122, Section4.2.3.3, Page97-98.  */
        socket_ptr -> nx_tcp_socket_rx_window_last_sent -= (socket_ptr -> nx_tcp_socket_rx_sequence - original_rx_sequence);
    }

#ifdef NX_TCP_MAX_OUT_OF_ORDER_PACKETS
    /* Does the count of out of order packets exceed the defined value? */
    if ((socket_ptr -> nx_tcp_socket_receive_queue_count - acked_packets) >
        NX_TCP_MAX_OUT_OF_ORDER_PACKETS)
    {

        /* Yes it is. Remove the last packet in queue. */
        socket_ptr -> nx_tcp_socket_receive_queue_tail -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ALLOCATED;
        if (socket_ptr -> nx_tcp_socket_receive_queue_count > 1)
        {

            /* Find the previous packet of tail. */
            search_ptr = socket_ptr -> nx_tcp_socket_receive_queue_head;
            while (search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next != socket_ptr -> nx_tcp_socket_receive_queue_tail)
            {
                search_ptr = search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;
            }

            /* Release the tail. */
            _nx_packet_release(socket_ptr -> nx_tcp_socket_receive_queue_tail);

            /* Setup the tail packet. */
            socket_ptr -> nx_tcp_socket_receive_queue_tail = search_ptr;

            search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next = (NX_PACKET *)NX_PACKET_ENQUEUED;
        }
        else
        {

            /* Release the tail. */
            _nx_packet_release(socket_ptr -> nx_tcp_socket_receive_queue_tail);

            /* Clear the head and tail packets. */
            socket_ptr -> nx_tcp_socket_receive_queue_head = NX_NULL;
            socket_ptr -> nx_tcp_socket_receive_queue_tail = NX_NULL;
        }

        /* Decrease receive queue count. */
        socket_ptr -> nx_tcp_socket_receive_queue_count--;
    }
#endif /* NX_TCP_MAX_OUT_OF_ORDER_PACKETS */

    /* At this point, we can use the packet TCP header pointers since the received
       packet is already queued.  */

    /* Any packets for receiving? */
    while (acked_packets && socket_ptr -> nx_tcp_socket_receive_suspension_list
#ifdef NX_ENABLE_HTTP_PROXY
           && (socket_ptr -> nx_tcp_socket_http_proxy_state != NX_HTTP_PROXY_STATE_CONNECTING)
#endif /* NX_ENABLE_HTTP_PROXY */
          )
    {

        /* Setup a pointer to the first queued packet.  */
        packet_ptr =  socket_ptr -> nx_tcp_socket_receive_queue_head;
        /* Remove it from the queue.  */

        /* Simply update the head pointer of the queue.  */
        socket_ptr -> nx_tcp_socket_receive_queue_head =  packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;

        /* Mark the packet as no longer being part of the TCP queue.  */
        /*lint -e{923} suppress cast of ULONG to pointer.  */
        packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next =  (NX_PACKET *)NX_PACKET_ALLOCATED;

        /* Clear the queue next pointer.  */
        packet_ptr -> nx_packet_queue_next =  NX_NULL;

        /* Decrease the number of received packets.  */
        socket_ptr -> nx_tcp_socket_receive_queue_count--;

        /* Adjust the packet for delivery to the suspended thread.  */

        /* Setup a pointer to the TCP header of this packet.  */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        tcp_header_ptr =  (NX_TCP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

        /* Calculate the header size for this packet.  */
        header_length =  (tcp_header_ptr -> nx_tcp_header_word_3 >> NX_TCP_HEADER_SHIFT) * (ULONG)sizeof(ULONG);

        /* Adjust the packet prepend pointer and length to position past the TCP header.  */
        packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr + header_length;
        packet_ptr -> nx_packet_length =       packet_ptr -> nx_packet_length - header_length;

        /* Setup a pointer to the first thread suspended on the receive queue.  */
        thread_ptr =  socket_ptr -> nx_tcp_socket_receive_suspension_list;

        /* Place the packet pointer in the return pointer.  */
        *((NX_PACKET **)thread_ptr -> tx_thread_additional_suspend_info) =  packet_ptr;

        /* Increase the receive window size.  */
        socket_ptr -> nx_tcp_socket_rx_window_current += packet_ptr -> nx_packet_length;

        /* Remove the suspended thread from the list.  */

        /* Decrement the suspension count.  */
        socket_ptr -> nx_tcp_socket_receive_suspended_count--;

        /* Decrement the acked_packets count. */
        acked_packets--;

        /* Resume thread.  */
        _nx_tcp_socket_thread_resume(&(socket_ptr -> nx_tcp_socket_receive_suspension_list), NX_SUCCESS);
    }

    /* Is the queue empty?.  */
    if (socket_ptr -> nx_tcp_socket_receive_queue_count == 0)
    {

        /* Yes. Set both head and tail pointers to NULL.  */
        socket_ptr -> nx_tcp_socket_receive_queue_head =  NX_NULL;
        socket_ptr -> nx_tcp_socket_receive_queue_tail =  NX_NULL;
    }

    /* Determine if an ACK should be forced out for window update, SWS avoidance algorithm.
       RFC1122, Section4.2.3.3, Page97-98. */
    if ((socket_ptr -> nx_tcp_socket_rx_window_current - socket_ptr -> nx_tcp_socket_rx_window_last_sent) >= (socket_ptr -> nx_tcp_socket_rx_window_default / 2))
    {

        /* Need to send ACK for window update.  */
        need_ack = NX_TRUE;
    }

    /* If the incoming packet caused the sequence number to move forward,
       indicating the new piece of data is in order, in sequence, and valid for receiving. */
    if ((original_rx_sequence != socket_ptr -> nx_tcp_socket_rx_sequence)
#ifdef NX_ENABLE_TCPIP_OFFLOAD
        || tcpip_offload
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
       )
    {

#ifdef NX_ENABLE_HTTP_PROXY

        /* If HTTP Proxy is connecting, the data is the response from HTTP Proxy server, don't need to notify application.  */
        if (socket_ptr -> nx_tcp_socket_http_proxy_state != NX_HTTP_PROXY_STATE_CONNECTING)
#endif /* NX_ENABLE_HTTP_PROXY */
        {

            /* Determine if there is a socket receive notification function specified.  */
            if (socket_ptr -> nx_tcp_receive_callback)
            {

                /* Yes, notification is requested.  Call the application's receive notification
                   function for this socket.  */
                (socket_ptr -> nx_tcp_receive_callback)(socket_ptr);
            }
        }

#ifdef NX_TCP_ACK_EVERY_N_PACKETS
        /* Determine if we need to ACK up to the current sequence number.  */

        /* If we are still in an ESTABLISHED state, a FIN isn't present and we can
           allocate a packet for the ACK message, send an ACK message.  */
        if ((socket_ptr -> nx_tcp_socket_state == NX_TCP_ESTABLISHED) &&
            ((tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_FIN_BIT) == 0))
        {
            if (socket_ptr -> nx_tcp_socket_ack_n_packet_counter >= NX_TCP_ACK_EVERY_N_PACKETS)
            {
                socket_ptr -> nx_tcp_socket_ack_n_packet_counter = 1;

                /* Need to send an immediate ACK.  */
                need_ack = NX_TRUE;
            }
            else
            {
                socket_ptr -> nx_tcp_socket_ack_n_packet_counter++;
            }
        }
#endif
    }

    if (need_ack == NX_TRUE)
    {

        /* Need to send ACK.  */
        _nx_tcp_packet_send_ack(socket_ptr, socket_ptr -> nx_tcp_socket_tx_sequence);
    }

    /* Return true since the packet was queued.  */
    return(NX_TRUE);
}

