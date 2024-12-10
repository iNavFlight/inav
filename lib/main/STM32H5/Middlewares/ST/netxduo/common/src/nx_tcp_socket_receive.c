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
#include "nx_packet.h"
#include "nx_tcp.h"
#include "tx_thread.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_receive                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function attempts to receive one or more TCP packets from the  */
/*    specified socket.                                                   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to socket             */
/*    packet_ptr                            Pointer to packet pointer     */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_packet_send_ack               Send ACK message              */
/*    _nx_tcp_socket_thread_suspend         Suspend calling thread        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
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
UINT  _nx_tcp_socket_receive(NX_TCP_SOCKET *socket_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{

NX_IP                 *ip_ptr;
NX_TCP_HEADER         *header_ptr;
NX_PACKET             *head_packet_ptr;
ULONG                  header_length;

#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY *trace_event;
ULONG                  trace_timestamp;
#endif

    /* Setup the pointer to the associated IP instance.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;

    /* Set the return pointer to NULL initially.  */
    *packet_ptr =   NX_NULL;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_SOCKET_RECEIVE, socket_ptr, 0, 0, 0, NX_TRACE_TCP_EVENTS, &trace_event, &trace_timestamp);

    /* Get protection while we look at this socket.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if the socket is currently bound.  */
    if (!socket_ptr ->  nx_tcp_socket_bound_next)
    {

        /* Release protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Socket is not bound, return an error message.  */
        return(NX_NOT_BOUND);
    }

    /* Do not return without data if there is data on the queue. */
    if (!socket_ptr -> nx_tcp_socket_receive_queue_head)
    {
        /* There is no data on the queue. */

        /* Determine if the socket is still in an active state, but also allow
           a receive socket operation if there are still more queued receive
           packets for this socket.  */
        if ((socket_ptr -> nx_tcp_socket_state < NX_TCP_SYN_SENT)   ||
            (socket_ptr -> nx_tcp_socket_state == NX_TCP_CLOSE_WAIT) ||
            (socket_ptr -> nx_tcp_socket_state >= NX_TCP_CLOSING))
        {

            /* Release the IP protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return an error code.  */
            return(NX_NOT_CONNECTED);
        }
    }

    /* Pickup the important information from the socket.  */

    /* Attempt to build a pointer to the first packet in the socket's
       receive queue.  */
    if (socket_ptr -> nx_tcp_socket_receive_queue_head)
    {

        /* Yes, there is a packet on the receive queue.  Setup a pointer to it and
           its header.  */
        head_packet_ptr =  socket_ptr -> nx_tcp_socket_receive_queue_head;
    }
    else
    {

        /* Just set the pointers to NULL.  */
        head_packet_ptr =  NX_NULL;
    }

    /* Determine if there is a receive packet available.  */
    /*lint -e{923} suppress cast of ULONT to pointer.  */
    if ((head_packet_ptr) && (head_packet_ptr -> nx_packet_queue_next == ((NX_PACKET *)NX_PACKET_READY)))
    {


        /* Yes, the first packet in the queue is available and has been ACKed.  Remove it
           from the queue and return it to the caller.  */
        if (head_packet_ptr == socket_ptr -> nx_tcp_socket_receive_queue_tail)
        {

            /* Only item in the queue.  Set the head and tail pointers to NULL.  */
            socket_ptr -> nx_tcp_socket_receive_queue_head =  NX_NULL;
            socket_ptr -> nx_tcp_socket_receive_queue_tail =  NX_NULL;
        }
        else
        {

            /* Simply update the head pointer to the packet after the current. The tail pointer does not
               need update.  */
            socket_ptr -> nx_tcp_socket_receive_queue_head =  head_packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;
        }

        /* Decrease the number of received packets.  */
        socket_ptr -> nx_tcp_socket_receive_queue_count--;

        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        header_ptr =  (NX_TCP_HEADER *)head_packet_ptr -> nx_packet_prepend_ptr;

        /* Calculate the header size for this packet.  */
        header_length =  (header_ptr -> nx_tcp_header_word_3 >> NX_TCP_HEADER_SHIFT) * (ULONG)sizeof(ULONG);

        /* Adjust the packet prepend pointer and length to position past the TCP header.  */
        head_packet_ptr -> nx_packet_prepend_ptr =  head_packet_ptr -> nx_packet_prepend_ptr + header_length;
        head_packet_ptr -> nx_packet_length =       head_packet_ptr -> nx_packet_length - header_length;

        /* Indicate that this TCP packet is no longer enqueued by marking it again as allocated. This is what
           it was prior to being part of the TCP receive queue.  */
        /*lint -e{923} suppress cast of ULONT to pointer.  */
        head_packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next =  (NX_PACKET *)NX_PACKET_ALLOCATED;

        /* Clear the queue next pointer.  */
        head_packet_ptr -> nx_packet_queue_next =  NX_NULL;

        /* Place the packet pointer in the return pointer.  */
        *packet_ptr =  head_packet_ptr;

        /* Check the receive queue count.  */
        if (socket_ptr -> nx_tcp_socket_receive_queue_count == 0)
        {

            /* Make sure the current receive window is the default window!  */
            socket_ptr -> nx_tcp_socket_rx_window_current =  socket_ptr -> nx_tcp_socket_rx_window_default;
        }
        else
        {

            /* Increase the receive window size.  */
            socket_ptr -> nx_tcp_socket_rx_window_current += (*packet_ptr) -> nx_packet_length;
        }

        /* Determine if an ACK should be forced out for window update, SWS avoidance algorithm.
           RFC1122, Section4.2.3.3, Page97-98. */
        if (((socket_ptr -> nx_tcp_socket_rx_window_current - socket_ptr -> nx_tcp_socket_rx_window_last_sent) >= (socket_ptr -> nx_tcp_socket_rx_window_default / 2)) &&
            ((socket_ptr -> nx_tcp_socket_state == NX_TCP_ESTABLISHED) || (socket_ptr -> nx_tcp_socket_state == NX_TCP_FIN_WAIT_1) || (socket_ptr -> nx_tcp_socket_state == NX_TCP_FIN_WAIT_2)))
        {

            /* Send a Window Update.  */
            _nx_tcp_packet_send_ack(socket_ptr, socket_ptr -> nx_tcp_socket_tx_sequence);
        }

#ifdef TX_ENABLE_EVENT_TRACE
        /* Update the trace event with the status.  */
        NX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, NX_TRACE_TCP_SOCKET_RECEIVE, 0, *packet_ptr, (*packet_ptr) -> nx_packet_length, socket_ptr -> nx_tcp_socket_rx_sequence);
#endif /* TX_ENABLE_EVENT_TRACE */

        /* Release protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return a successful status.  */
        return(NX_SUCCESS);
    }
    else if ((wait_option) && (_tx_thread_current_ptr != &(ip_ptr -> nx_ip_thread)))
    {

        /* Suspend the thread on this socket's receive queue.  */

        /* Save the return packet pointer address as well.  */
        _tx_thread_current_ptr -> tx_thread_additional_suspend_info =  (void *)packet_ptr;

        /* Increment the suspended thread count.  */
        socket_ptr -> nx_tcp_socket_receive_suspended_count++;

        /* Suspend the thread on the receive queue.  */
        /* Note that the mutex is released inside _nx_tcp_socket_thread_suspend(). */
        _nx_tcp_socket_thread_suspend(&(socket_ptr -> nx_tcp_socket_receive_suspension_list), _nx_tcp_receive_cleanup, socket_ptr, &(ip_ptr -> nx_ip_protection), wait_option);
#ifdef TX_ENABLE_EVENT_TRACE
        if (*packet_ptr)
        {

            /* Update the trace event with the status.  */
            NX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, NX_TRACE_TCP_SOCKET_RECEIVE, 0, *packet_ptr, (*packet_ptr) -> nx_packet_length, socket_ptr -> nx_tcp_socket_rx_sequence);
        }
#endif /* TX_ENABLE_EVENT_TRACE */
        /* If not, just return the error code.  */
        return(_tx_thread_current_ptr -> tx_thread_suspend_status);
    }
    else
    {

        /* Release protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return an empty receive queue error message.  */
        return(NX_NO_PACKET);
    }
}

