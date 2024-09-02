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

#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_state_ack_check                      PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for ACK conditions in various states of the    */
/*    TCP socket.  ACK messages are examined against the queued transmit  */
/*    packets in order to see if one or more transmit packets may be      */
/*    removed from the socket's transmit queue.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to owning socket      */
/*    tcp_header_ptr                        Pointer to packet header      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    NX_TRUE                               The ACK number is acceptable  */
/*    NX_FALSE                              The ACK number is unacceptable*/
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_packet_send_ack               Send ACK message              */
/*    _nx_packet_release                    Packet release function       */
/*    _nx_tcp_socket_retransmit             Retransmit packet             */
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
/*  09-30-2020     Yuxin Zhou               Modified comment(s), and      */
/*                                            corrected the calculation of*/
/*                                            ending packet sequence,     */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1.7  */
/*  10-15-2021     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed the bug of race       */
/*                                            condition, removed useless  */
/*                                            code,                       */
/*                                            resulting in version 6.1.9  */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed unsigned integers     */
/*                                            comparison,                 */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_tcp_socket_state_ack_check(NX_TCP_SOCKET *socket_ptr, NX_TCP_HEADER *tcp_header_ptr)
{

TX_INTERRUPT_SAVE_AREA

NX_TCP_HEADER *search_header_ptr = NX_NULL;
NX_PACKET     *search_ptr;
NX_PACKET     *previous_ptr;
ULONG          header_length;
ULONG          search_sequence;
ULONG          temp;
ULONG          packet_release_count;
ULONG          ending_packet_sequence;
ULONG          starting_tx_sequence;
ULONG          ending_tx_sequence;
ULONG          ending_rx_sequence;
ULONG          acked_bytes;
ULONG          tcp_payload_length;
UINT           wrapped_flag = NX_FALSE;


    /* Determine if the header has an ACK bit set.  This is an
       acknowledgement of a previous transmission.  */
    if (tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_ACK_BIT)
    {

        /* Initialize tx sequence. */
        if (socket_ptr -> nx_tcp_socket_zero_window_probe_has_data)
        {
            ending_tx_sequence = socket_ptr -> nx_tcp_socket_tx_sequence + 1;
        }
        else
        {
            ending_tx_sequence = socket_ptr -> nx_tcp_socket_tx_sequence;
        }
        starting_tx_sequence = socket_ptr -> nx_tcp_socket_tx_sequence - socket_ptr -> nx_tcp_socket_tx_outstanding_bytes;

        /* Initialize ending rx sequence. */
        if (socket_ptr -> nx_tcp_socket_receive_queue_tail)
        {
            search_ptr = socket_ptr -> nx_tcp_socket_receive_queue_tail;

            /* Setup a pointer to header of this packet in the sent list.  */
#ifndef NX_DISABLE_IPV4
            if (search_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
            {

                /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                search_header_ptr =  (NX_TCP_HEADER *)(search_ptr -> nx_packet_ip_header +
                                                       sizeof(NX_IPV4_HEADER));
            }
            else
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
            if (search_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
            {

                /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                search_header_ptr =  (NX_TCP_HEADER *)(search_ptr -> nx_packet_ip_header +
                                                       sizeof(NX_IPV6_HEADER));
            }
            else
#endif /* FEATURE_NX_IPV6 */
            {
                return(NX_FALSE);
            }

            /* Determine the size of the TCP header.  */
            temp =  search_header_ptr -> nx_tcp_header_word_3;
            header_length =  (temp >> NX_TCP_HEADER_SHIFT) * (ULONG)sizeof(ULONG);

            /* Determine the sequence number in the TCP header.  */
            search_sequence =  search_header_ptr -> nx_tcp_sequence_number;

            /* Calculate the payload length of TCP. */
            tcp_payload_length = (search_ptr -> nx_packet_length -
                                  (header_length +
                                   (ULONG)((ALIGN_TYPE)search_header_ptr -
                                           (ALIGN_TYPE)search_ptr -> nx_packet_prepend_ptr)));

            /* Calculate the ending packet sequence.  */
            ending_rx_sequence =  search_sequence + tcp_payload_length;
        }
        else
        {
            ending_rx_sequence = socket_ptr -> nx_tcp_socket_rx_sequence;
        }

#ifdef NX_ENABLE_TCP_KEEPALIVE
        /* Determine if the socket is in the established state.  */
        if (socket_ptr -> nx_tcp_socket_state == NX_TCP_ESTABLISHED)
        {

            /* Is the keepalive feature enabled on this socket? */
            if (socket_ptr -> nx_tcp_socket_keepalive_enabled)
            {
                /* Yes, reset the TCP Keepalive timer to initial values.  */
                socket_ptr -> nx_tcp_socket_keepalive_timeout =  NX_TCP_KEEPALIVE_INITIAL;
                socket_ptr -> nx_tcp_socket_keepalive_retries =  0;

                /* Determine if we have received a Keepalive ACK request from the other side
                   of the connection.  */
                if (tcp_header_ptr -> nx_tcp_sequence_number == (socket_ptr -> nx_tcp_socket_rx_sequence - 1))
                {

                    /* Yes, a Keepalive ACK probe is present.  Respond with an ACK to let the other
                       side of the connection know that we are still alive.  */
                    _nx_tcp_packet_send_ack(socket_ptr, ending_tx_sequence);
                }
            }
        }
#endif

        /* First, determine if incoming ACK matches our transmit sequence.  */
        /*lint -e{923} suppress cast of pointer to ULONG.  */
        if (tcp_header_ptr -> nx_tcp_acknowledgment_number == ending_tx_sequence)
        {

            /* In this case, everything on the transmit list is acknowledged.  Simply set the packet
               release count to the number of packets in the transmit queue.  */
            packet_release_count =  socket_ptr -> nx_tcp_socket_transmit_sent_count;

            /* Set the previous pointer to the socket transmit tail pointer.  */
            previous_ptr =  socket_ptr -> nx_tcp_socket_transmit_sent_tail;

            /* Is this ACK to FIN? */
            if (socket_ptr -> nx_tcp_socket_state >= NX_TCP_FIN_WAIT_1)
            {

                /* Yes it is. */
                socket_ptr -> nx_tcp_socket_fin_acked = NX_TRUE;
            }
        }
        else
        {

            /* Calculate the start and end of the transmit sequence.  */

            /* Pickup the head of the transmit queue.  */
            search_ptr =    socket_ptr -> nx_tcp_socket_transmit_sent_head;

            /* Determine if there is a packet on the transmit queue... and determine if the packet has been
               transmitted.  */
            /*lint -e{923} suppress cast of ULONG to pointer.  */
            if ((search_ptr) && (search_ptr -> nx_packet_queue_next == ((NX_PACKET *)NX_DRIVER_TX_DONE)))
            {

                /* Determine if the incoming ACK matches the front of our transmit queue. */
                if (tcp_header_ptr -> nx_tcp_acknowledgment_number == starting_tx_sequence)
                {

                    /* Handle duplicated ACK packet.  */
                    socket_ptr -> nx_tcp_socket_duplicated_ack_received++;

                    if (socket_ptr -> nx_tcp_socket_duplicated_ack_received == 3)
                    {
                        if ((INT)((tcp_header_ptr -> nx_tcp_acknowledgment_number - 1) -
                                  socket_ptr -> nx_tcp_socket_tx_sequence_recover) > 0)
                        {

                            /* Cumulative acknowledge covers more than recover. */
                            /* Section 3.2, Page 5, RFC6582. */
                            /* Retransmit packet immediately. */
                            _nx_tcp_socket_retransmit(socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, NX_TRUE);
                        }
                        else if ((socket_ptr -> nx_tcp_socket_tx_window_congestion > socket_ptr -> nx_tcp_socket_connect_mss) &&
                                 ((INT)(tcp_header_ptr -> nx_tcp_acknowledgment_number - (socket_ptr -> nx_tcp_socket_previous_highest_ack +
                                                                                          (socket_ptr -> nx_tcp_socket_connect_mss << 2))) < 0))
                        {

                            /* Congestion window is greater than SMSS bytes and
                               the difference between highest_ack and prev_highest_ack is at most 4*SMSS bytes.*/
                            /* Section 4.1, Page 5, RFC6582. */
                            /* Retransmit packet immediately. */
                            _nx_tcp_socket_retransmit(socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, NX_TRUE);
                        }
                    }
                    else if ((socket_ptr -> nx_tcp_socket_duplicated_ack_received > 3) &&
                             (socket_ptr -> nx_tcp_socket_fast_recovery == NX_TRUE))
                    {

                        /* CWND += MSS  */
                        socket_ptr -> nx_tcp_socket_tx_window_congestion += socket_ptr -> nx_tcp_socket_connect_mss;
                    }
                }

                /* Determine if the transmit queue has wrapped.  */
                if (ending_tx_sequence > starting_tx_sequence)
                {

                    /* Clear the wrapped flag.  */
                    wrapped_flag =  NX_FALSE;
                }
                else
                {

                    /* Set the wrapped flag.  */
                    wrapped_flag =  NX_TRUE;
                }
            }

            /* Initialize the packet release count.  */
            packet_release_count =  0;

            /* See if we can find the sequence number in the sent queue for this
               socket.  */
            previous_ptr =  NX_NULL;
            while (search_ptr)
            {

                /* Determine if the packet has been transmitted.  */
                /*lint -e{923} suppress cast of ULONG to pointer.  */
                if (search_ptr -> nx_packet_queue_next != ((NX_PACKET *)NX_DRIVER_TX_DONE))
                {

                    /* Setup a pointer to header of this packet in the sent list.  */
                    search_header_ptr =  (NX_TCP_HEADER *)(search_ptr -> nx_packet_ip_header +
                                                           search_ptr -> nx_packet_ip_header_length);
                }
                else
                {

                    /* Setup a pointer to header of this packet in the sent list.  */
                    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                    search_header_ptr =  (NX_TCP_HEADER *)search_ptr -> nx_packet_prepend_ptr;
                }

                /* Determine the size of the TCP header.  */
                temp =  search_header_ptr -> nx_tcp_header_word_3;
                NX_CHANGE_ULONG_ENDIAN(temp);
                header_length =  (temp >> NX_TCP_HEADER_SHIFT) * (ULONG)sizeof(ULONG);

                /* Determine the sequence number in the TCP header.  */
                search_sequence =  search_header_ptr -> nx_tcp_sequence_number;
                NX_CHANGE_ULONG_ENDIAN(search_sequence);

                /* Calculate the payload length of TCP. */
                tcp_payload_length = (search_ptr -> nx_packet_length -
                                      (header_length +
                                       (ULONG)((ALIGN_TYPE)search_header_ptr -
                                               (ALIGN_TYPE)search_ptr -> nx_packet_prepend_ptr)));

                /* Calculate the ending packet sequence.  */
                ending_packet_sequence =  search_sequence + tcp_payload_length;

                /* Determine if the transmit window is wrapped.  */
                if (wrapped_flag == NX_FALSE)
                {

                    /* No, the transmit window is not wrapped. Perform a simple compare to determine if the ACK
                       covers the current search packet.  */

                    /* Is this ACK before the current search packet or after the transmit sequence?  */
                    if ((tcp_header_ptr -> nx_tcp_acknowledgment_number < ending_packet_sequence) ||
                        (tcp_header_ptr -> nx_tcp_acknowledgment_number > ending_tx_sequence))
                    {

                        /* The ACK is less than the current packet, just get out of the loop!  */
                        break;
                    }
                }
                else
                {

                    /* Yes, the transmit window has wrapped.  We need to now check for all the wrap conditions to
                       determine if ACK covers the current search packet.  */

                    /* Is the search packet's ending sequence number in the wrapped part of the window.  */
                    if (ending_packet_sequence < starting_tx_sequence)
                    {

                        /* The search packet ends in the wrapped portion of the window.  Determine if the ACK
                           sequence in the wrapped portion as well.  */
                        if (tcp_header_ptr -> nx_tcp_acknowledgment_number < starting_tx_sequence)
                        {

                            /* Yes, the ACK sequence is in the wrapped portion as well. Simply compare the ACK
                               sequence with the search packet sequence.  */
                            if (tcp_header_ptr -> nx_tcp_acknowledgment_number < ending_packet_sequence)
                            {

                                /* ACK does not cover the search packet. Break out of the loop.  */
                                break;
                            }
                        }
                        else
                        {

                            /* The ACK sequence is in the non-wrapped portion of the window and the ending sequence
                               of the search packet is in the wrapped portion - so the ACK doesn't cover the search
                               packet.  Break out of the loop!  */
                            break;
                        }
                    }
                    else
                    {

                        /* The search packet is in the non-wrapped portion of the window.  Determine if the ACK
                           sequence is in the non-wrapped portion as well.  */
                        if (tcp_header_ptr -> nx_tcp_acknowledgment_number >= starting_tx_sequence)
                        {

                            /* Yes, the ACK sequence is in the non-wrapped portion of the window. Simply compare the ACK
                               sequence with the search packet sequence.  */
                            if (tcp_header_ptr -> nx_tcp_acknowledgment_number < ending_packet_sequence)
                            {

                                /* ACK does not cover the search packet. Break out of the loop.  */
                                break;
                            }
                        }
                    }
                }

                /* At this point we know that the ACK received covers the search packet.  */

                /* Increase the packet release count.  */
                packet_release_count++;

                /* Move the search and previous pointers forward.  */
                previous_ptr =  search_ptr;
                search_ptr =  search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;

                /* Determine if we are at the end of the TCP queue.  */
                /*lint -e{923} suppress cast of ULONG to pointer.  */
                if (search_ptr == ((NX_PACKET *)NX_PACKET_ENQUEUED))
                {

                    /* Yes, set the search pointer to NULL.  */
                    search_ptr =  NX_NULL;
                }
            }
        }

        /* Determine if anything needs to be released.  */
        if (!packet_release_count)
        {

            /* No, check and see if the ACK is valid.  */
            /* If the ACK acks something not yet sent (SEG.ACK > SND.NXT) then send an ACK, drop the segment */
            /* Page 72, section 3.9, RFC 793.*/
            if (tcp_header_ptr -> nx_tcp_acknowledgment_number != ending_tx_sequence)
            {

                /* If the ACK is a duplicate, it can be ignored. */
                if ((INT)(tcp_header_ptr -> nx_tcp_acknowledgment_number - ending_tx_sequence) > 0)
                {

                    /* The ACK sequence is invalid. Respond with an ACK to let the other
                       side of the connection figure out if everything is still okay.  */
                    _nx_tcp_packet_send_ack(socket_ptr, ending_tx_sequence);
                    return(NX_FALSE);
                }
            }
            else if ((socket_ptr -> nx_tcp_socket_rx_window_current == 0) &&
                     (tcp_header_ptr -> nx_tcp_sequence_number == socket_ptr -> nx_tcp_socket_rx_sequence))
            {

                /* Response to zero window probe.  */
                _nx_tcp_packet_send_ack(socket_ptr, ending_tx_sequence);
            }
        }
        else
        {

            /* Congestion window adjustment during slow start and congestion avoidance is executed
               on every incoming ACK that acknowledges new data. RFC5681, Section3.1, Page4-8.  */

            /* Check whether the socket is in fast recovery procedure. */
            if (socket_ptr -> nx_tcp_socket_fast_recovery == NX_TRUE)
            {

                /* Yes. */
                if ((INT)(tcp_header_ptr -> nx_tcp_acknowledgment_number -
                          socket_ptr -> nx_tcp_socket_tx_sequence_recover) > 0)
                {

                    /* All packets sent before entering fast recovery are ACKed. */
                    /* Exit fast recovery procedure. */
                    socket_ptr -> nx_tcp_socket_fast_recovery = NX_FALSE;
                    socket_ptr -> nx_tcp_socket_tx_window_congestion = socket_ptr -> nx_tcp_socket_tx_slow_start_threshold;
                }
            }

            if ((INT)(socket_ptr -> nx_tcp_socket_tx_sequence_recover -
                      (tcp_header_ptr -> nx_tcp_acknowledgment_number - 2)) < 0)
            {

                /* Update the transmit sequence that entered fast transmit. */
                socket_ptr -> nx_tcp_socket_tx_sequence_recover = tcp_header_ptr -> nx_tcp_acknowledgment_number - 2;
            }

            /* Reset the duplicated ACK counter. */
            socket_ptr -> nx_tcp_socket_duplicated_ack_received = 0;

            /* Set previous cumulative acknowlesgement. */
            socket_ptr -> nx_tcp_socket_previous_highest_ack = starting_tx_sequence;

            /* Calculate ACKed length. */
            acked_bytes = tcp_header_ptr -> nx_tcp_acknowledgment_number - starting_tx_sequence;

            if (socket_ptr -> nx_tcp_socket_fast_recovery == NX_TRUE)
            {

                /* Process cwnd in fast recovery procedure. */
                socket_ptr -> nx_tcp_socket_tx_window_congestion -= acked_bytes;
                if (acked_bytes > socket_ptr -> nx_tcp_socket_connect_mss)
                {
                    socket_ptr -> nx_tcp_socket_tx_window_congestion += socket_ptr -> nx_tcp_socket_connect_mss;
                }
            }
            else
            {

                /* Adjust the transmit window.  In slow start phase, the transmit window is incremented for every ACK.
                   In Congestion Avoidance phase, the window is incremented for every RTT. Section 3.1, Page 4-7, RFC5681.  */
                if (socket_ptr -> nx_tcp_socket_tx_window_congestion >= socket_ptr -> nx_tcp_socket_tx_slow_start_threshold)
                {

                    /* In Congestion avoidance phase, for every ACK it receives, increase the window size using the
                       following approximation:
                       cwnd = cwnd + MSS * MSS / cwnd;  */
                    temp = socket_ptr -> nx_tcp_socket_connect_mss2 / socket_ptr -> nx_tcp_socket_tx_window_congestion;

                    /* If the above formula yields 0, the result SHOULD be rounded up to 1 byte.  */
                    if (temp == 0)
                    {
                        temp = 1;
                    }
                    socket_ptr -> nx_tcp_socket_tx_window_congestion = socket_ptr -> nx_tcp_socket_tx_window_congestion + temp;
                }
                else
                {

                    /* In Slow start phase:
                       cwnd += min (N, SMSS),
                       where N is the number of ACKed bytes. */
                    if (acked_bytes < socket_ptr -> nx_tcp_socket_connect_mss)
                    {

                        /* In Slow start phase. Increase the cwnd by acked bytes.*/
                        socket_ptr -> nx_tcp_socket_tx_window_congestion += acked_bytes;
                    }
                    else
                    {

                        /* In Slow start phase. Increase the cwnd by full MSS for every ack.*/
                        socket_ptr -> nx_tcp_socket_tx_window_congestion += socket_ptr -> nx_tcp_socket_connect_mss;
                    }
                }
            }
        }

        /* Update the window only when
         * 1. SND.UNA < SEG.ACK =< SND.NXT or
         * 2. SND.WL1 < SEG.SEQ or
         * 3. SND.WL1 = SEG.SEQ and SND.WL2 =< SEG.ACK
         * RFC793, Section 3.9, Page72. */
        if ((((INT)(tcp_header_ptr -> nx_tcp_acknowledgment_number - starting_tx_sequence) > 0) &&
             ((INT)(tcp_header_ptr -> nx_tcp_acknowledgment_number - ending_tx_sequence) <= 0)) ||
            ((INT)(tcp_header_ptr -> nx_tcp_sequence_number - ending_rx_sequence) > 0) ||
            (((INT)(tcp_header_ptr -> nx_tcp_sequence_number == ending_rx_sequence)) &&
             ((INT)(tcp_header_ptr -> nx_tcp_acknowledgment_number - starting_tx_sequence) >= 0)))
        {

            /* Update this socket's transmit window with the advertised window size in the ACK message.  */
            socket_ptr -> nx_tcp_socket_tx_window_advertised =  (tcp_header_ptr -> nx_tcp_header_word_3) & NX_LOWER_16_MASK;

#ifdef NX_ENABLE_TCP_WINDOW_SCALING
            socket_ptr -> nx_tcp_socket_tx_window_advertised <<= socket_ptr -> nx_tcp_snd_win_scale_value;
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */
        }

        /* Check advertised window. */
        if ((socket_ptr -> nx_tcp_socket_tx_window_advertised <= socket_ptr -> nx_tcp_socket_tx_outstanding_bytes) &&
            (tcp_header_ptr -> nx_tcp_acknowledgment_number >= socket_ptr -> nx_tcp_socket_zero_window_probe_sequence))
        {

            /* It is an ACK to Zero Window Probe. Reset the zero window probe failure. */
            socket_ptr -> nx_tcp_socket_zero_window_probe_failure = 0;
        }

        if (!packet_release_count)
        {
            /* Done, return to caller. */
            return(NX_TRUE);
        }

        /* Once we get this line, packet_release_count is non-zero,
           which indicates the transmit_sent_count value will be reduced. */

#ifdef NX_ENABLE_TCP_QUEUE_DEPTH_UPDATE_NOTIFY

        /* Is NetX Duo set up with a transmit socket queue depth update? */
        if (socket_ptr -> nx_tcp_socket_queue_depth_notify)
        {

            /* Yes it is. Verify that the packets on the transmit queue exceed the max queue depth, and that
               the packets to release bring that queue size below the max queue depth. */
            if ((socket_ptr -> nx_tcp_socket_transmit_sent_count >= socket_ptr -> nx_tcp_socket_transmit_queue_maximum) &&
                ((socket_ptr -> nx_tcp_socket_transmit_sent_count - packet_release_count) < socket_ptr -> nx_tcp_socket_transmit_queue_maximum))
            {

                /* Yes to both; Call the queue depth notify callback. */
                (socket_ptr -> nx_tcp_socket_queue_depth_notify)(socket_ptr);
            }
        }
#endif

        /* Save the front of the of the transmit queue.  */
        search_ptr =  socket_ptr -> nx_tcp_socket_transmit_sent_head;

        /* Okay so now the packet after the previous pointer needs to be the front of the
           queue.  */
        if (previous_ptr != socket_ptr -> nx_tcp_socket_transmit_sent_tail)
        {

            /* Just update the head pointer.  */
            socket_ptr -> nx_tcp_socket_transmit_sent_head  =  previous_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;

            /* And decrease the transmit queue count accordingly.  */
            socket_ptr -> nx_tcp_socket_transmit_sent_count =   socket_ptr -> nx_tcp_socket_transmit_sent_count - packet_release_count;

            /* Setup a new transmit timeout.  */
            socket_ptr -> nx_tcp_socket_timeout =          socket_ptr -> nx_tcp_socket_timeout_rate;
            socket_ptr -> nx_tcp_socket_timeout_retries =  0;
        }
        else
        {

            /* The transmit list is now cleared, just set the head and tail pointers to
               NULL.  */
            socket_ptr -> nx_tcp_socket_transmit_sent_head  =  NX_NULL;
            socket_ptr -> nx_tcp_socket_transmit_sent_tail  =  NX_NULL;

            /* Clear the transmit queue count.  */
            socket_ptr -> nx_tcp_socket_transmit_sent_count =  0;

            /* Determine if a disconnect FIN has been sent from this side of the connection.  */
            if ((socket_ptr -> nx_tcp_socket_state == NX_TCP_FIN_WAIT_1) ||
                (socket_ptr -> nx_tcp_socket_state == NX_TCP_CLOSING)    ||
                (socket_ptr -> nx_tcp_socket_state == NX_TCP_LAST_ACK))
            {

                /* Yes, setup timeout such that the FIN can be retried if it is lost.  */
                socket_ptr -> nx_tcp_socket_timeout =          socket_ptr -> nx_tcp_socket_timeout_rate;
                socket_ptr -> nx_tcp_socket_timeout_retries =  0;
            }
            else if (socket_ptr -> nx_tcp_socket_tx_window_advertised != 0)
            {

                /* Otherwise, a FIN has not been sent, simply clear the transmit timeout.  */
                socket_ptr -> nx_tcp_socket_timeout =  0;
            }
        }

        /* Now walk through the packets to release and set them
           free.  */
        while (packet_release_count--)
        {

            /* Use the previous pointer as the release pointer.  */
            previous_ptr =  search_ptr;

            /* Move to the next packet in the queue before we clip the
               next pointer.  */
            search_ptr =  search_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;

            /* Disable interrupts temporarily.  */
            TX_DISABLE

            /* Set the packet to allocated to indicate it is no longer part of the TCP queue.  */
            /*lint -e{923} suppress cast of ULONG to pointer.  */
            previous_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next =  ((NX_PACKET *)NX_PACKET_ALLOCATED);

            /* Has the packet been transmitted? This is only pertinent if a retransmit of
               the packet occurred prior to receiving the ACK. If so, the packet could be
               in an ARP queue or in a driver queue waiting for transmission so we can't
               release it directly at this point.  The driver or the ARP processing will
               release it when finished.  */
            /*lint -e{923} suppress cast of ULONG to pointer.  */
            if (previous_ptr -> nx_packet_queue_next ==  ((NX_PACKET *)NX_DRIVER_TX_DONE))
            {

                /* Restore interrupts.  */
                TX_RESTORE

                /* Yes, the driver has already released the packet.  */

                /* Open up the transmit window. */
                /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                search_header_ptr = (NX_TCP_HEADER *)previous_ptr -> nx_packet_prepend_ptr;

                temp = search_header_ptr -> nx_tcp_header_word_3;
                NX_CHANGE_ULONG_ENDIAN(temp);
                header_length = (temp >> NX_TCP_HEADER_SHIFT) * (ULONG)sizeof(ULONG);
                if (socket_ptr -> nx_tcp_socket_tx_outstanding_bytes > (previous_ptr -> nx_packet_length - header_length))
                {
                    socket_ptr -> nx_tcp_socket_tx_outstanding_bytes -= previous_ptr -> nx_packet_length - header_length;
                }
                else
                {
                    socket_ptr -> nx_tcp_socket_tx_outstanding_bytes = 0;
                }
                /* Release the packet.  */
                _nx_packet_release(previous_ptr);
            }
            else
            {

                /* No, the driver has not released the packet.  */
                /* Open up the transmit window. */
                search_header_ptr =  (NX_TCP_HEADER *)(previous_ptr -> nx_packet_ip_header +
                                                       previous_ptr -> nx_packet_ip_header_length);

                temp = search_header_ptr -> nx_tcp_header_word_3;
                NX_CHANGE_ULONG_ENDIAN(temp);
                header_length = (temp >> NX_TCP_HEADER_SHIFT) * (ULONG)sizeof(ULONG);
                tcp_payload_length = (previous_ptr -> nx_packet_length -
                                      (header_length +
                                       (ULONG)((ALIGN_TYPE)search_header_ptr -
                                               (ALIGN_TYPE)(previous_ptr -> nx_packet_prepend_ptr))));
                if (socket_ptr -> nx_tcp_socket_tx_outstanding_bytes > tcp_payload_length)
                {
                    socket_ptr -> nx_tcp_socket_tx_outstanding_bytes -= tcp_payload_length;
                }
                else
                {
                    socket_ptr -> nx_tcp_socket_tx_outstanding_bytes = 0;
                }

                /* Restore interrupts.  */
                TX_RESTORE
            }
        }

        if (socket_ptr -> nx_tcp_socket_fast_recovery == NX_TRUE)
        {

            /* Only partial data are ACKed. Retransmit packet immediately. */
            _nx_tcp_socket_retransmit(socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, NX_FALSE);
        }

        return(NX_TRUE);
    }
    else
    {

        /* The ACK bit is off drop the segment and return.  */
        /* RFC793, Section3.9, Page72.  */
        return(NX_FALSE);
    }
}

