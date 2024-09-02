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
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif /* FEATURE_NX_IPV6 */
#include "nx_packet.h"
#include "nx_tcp.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_state_transmit_check                 PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function determines if the new receive window value is large   */
/*    enough to satisfy a thread suspended trying to send data on the TCP */
/*    connection.  This is typically called from the ESTABLISHED state.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to TCP socket         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_socket_thread_resume          Resume suspended thread       */
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
VOID  _nx_tcp_socket_state_transmit_check(NX_TCP_SOCKET *socket_ptr)
{

ULONG tx_window_current;

    /* Now check to see if there is a thread suspended attempting to transmit.  */
    if (socket_ptr -> nx_tcp_socket_transmit_suspension_list)
    {

        /* Yes, a thread is suspended attempting to transmit when the transmit window
           is lower than its request size.  Determine if the current transmit window
           size can now accommodate the request.  */

        /* Pick up the min(cwnd, swnd) */
        if (socket_ptr -> nx_tcp_socket_tx_window_advertised > socket_ptr -> nx_tcp_socket_tx_window_congestion)
        {
            tx_window_current = socket_ptr -> nx_tcp_socket_tx_window_congestion;

            /* On the first and second duplicate ACKs received, the total FlightSize would
               remain less than or equal to cwnd plus 2*SMSS.
               Section 3.2, Page 9, RFC5681. */
            if ((socket_ptr -> nx_tcp_socket_duplicated_ack_received == 1) ||
                (socket_ptr -> nx_tcp_socket_duplicated_ack_received == 2))
            {
                tx_window_current += (socket_ptr -> nx_tcp_socket_connect_mss << 1);
            }

            /* Make sure the tx_window_current is less or equal to swnd. */
            if (tx_window_current > socket_ptr -> nx_tcp_socket_tx_window_advertised)
            {
                tx_window_current = socket_ptr -> nx_tcp_socket_tx_window_advertised;
            }
        }
        else
        {
            tx_window_current = socket_ptr -> nx_tcp_socket_tx_window_advertised;
        }

        /* Substract any data transmitted but unacked (outstanding bytes) */
        if (tx_window_current > socket_ptr -> nx_tcp_socket_tx_outstanding_bytes)
        {
            tx_window_current -= socket_ptr -> nx_tcp_socket_tx_outstanding_bytes;
        }
        else    /* Set tx_window_current to zero. */
        {
            tx_window_current = 0;
        }


        /* Determine if the current transmit window (received from the connected socket)
           is large enough to handle the transmit.  */
        if ((tx_window_current) &&
            (socket_ptr -> nx_tcp_socket_transmit_sent_count < socket_ptr -> nx_tcp_socket_transmit_queue_maximum))
        {

            /* Is NetX set up with a windows update callback? */
            if (socket_ptr -> nx_tcp_socket_window_update_notify)
            {

                /* Yes; Call this function when there is a change in transmit windows size. */
                (socket_ptr -> nx_tcp_socket_window_update_notify)(socket_ptr);
            }


            /* Decrement the suspension count.  */
            socket_ptr -> nx_tcp_socket_transmit_suspended_count--;

            /* Remove the suspended thread from the list.  */
            _nx_tcp_socket_thread_resume(&(socket_ptr -> nx_tcp_socket_transmit_suspension_list), NX_SUCCESS);
        }
    }
}

