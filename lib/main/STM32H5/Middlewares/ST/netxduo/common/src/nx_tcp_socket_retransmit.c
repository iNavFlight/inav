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
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif /* FEATURE_NX_IPV6 */
#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_retransmit                           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function retransmit a TCP packet.                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                IP instance pointer           */
/*    socket_ptr                            Pointer to owning socket      */
/*    need_fast_retransmit                  Need fast retransmit or not   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_packet_send_probe             Send zero window probe        */
/*    _nx_ip_checksum_compute               Calculate TCP checksum        */
/*    _nx_ip_packet_send                    Resend the transmit packet    */
/*    _nx_ipv6_packet_send                  Resend the transmit packet    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_fast_periodic_processing      Process TCP packet for socket */
/*    _nx_tcp_socket_state_ack_check        Process ACK number            */
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
VOID  _nx_tcp_socket_retransmit(NX_IP *ip_ptr, NX_TCP_SOCKET *socket_ptr, UINT need_fast_retransmit)
{
NX_PACKET *packet_ptr;
ULONG      window;
ULONG      original_acknowledgment_number;
ULONG      original_header_word_3;
ULONG      original_header_word_4;
ULONG      available;
ULONG      window_size;

    /* If the receiver winodw is zero, we enter the zero window probe phase
       RFC 793 Sec 3.7, p42: keep send new data.

       In the zero window probe phase, we send the zero window probe, and increase
       exponentially the interval between successive probes.
       RFC 1122 Sec 4.2.2.17, p92.  */
    if (socket_ptr -> nx_tcp_socket_tx_window_advertised == 0)
    {

        /* Pickup the head of the transmit queue.  */
        packet_ptr =  socket_ptr -> nx_tcp_socket_transmit_sent_head;

        if (packet_ptr)
        {

        /* Get one byte from send queue. */
        /* Pick up the pointer to the head of the TCP packet.  */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        NX_TCP_HEADER *header_ptr =  (NX_TCP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

            NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_3);
            NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_sequence_number);

            /* Get sequence number and first byte. */
            socket_ptr -> nx_tcp_socket_zero_window_probe_data = *(packet_ptr -> nx_packet_prepend_ptr + ((header_ptr -> nx_tcp_header_word_3 >> 28) << 2));

            /* Now set zero window probe started. */
            socket_ptr -> nx_tcp_socket_zero_window_probe_has_data = NX_TRUE;
            socket_ptr -> nx_tcp_socket_zero_window_probe_sequence = header_ptr -> nx_tcp_sequence_number;
            socket_ptr -> nx_tcp_socket_zero_window_probe_failure = 0;

            NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_sequence_number);
            NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_3);
        }
        else if (socket_ptr -> nx_tcp_socket_zero_window_probe_has_data == NX_FALSE)
        {
            return;
        }

        /* In the zero window probe phase, we send the zero window probe, and increase
           exponentially the interval between successive probes.  */

        /* Increment the retry counter.  */
        socket_ptr -> nx_tcp_socket_timeout_retries++;
        socket_ptr -> nx_tcp_socket_zero_window_probe_failure++;

        /* Setup the next timeout.  */
        socket_ptr -> nx_tcp_socket_timeout = socket_ptr -> nx_tcp_socket_timeout_rate <<
            (socket_ptr -> nx_tcp_socket_timeout_retries * socket_ptr -> nx_tcp_socket_timeout_shift);

        /* Send the zero window probe.  */
        _nx_tcp_packet_send_probe(socket_ptr, socket_ptr -> nx_tcp_socket_zero_window_probe_sequence,
                                  socket_ptr -> nx_tcp_socket_zero_window_probe_data);

        return;
    }
    else if (socket_ptr -> nx_tcp_socket_zero_window_probe_has_data == NX_TRUE)
    {

        /* If advertised window isn't zero, reset zero window probe flag. */
        socket_ptr -> nx_tcp_socket_zero_window_probe_has_data = NX_FALSE;
    }

    /* Increment the retry counter only if the receiver window is open. */
    /* Increment the retry counter.  */
    socket_ptr -> nx_tcp_socket_timeout_retries++;

    if ((need_fast_retransmit == NX_TRUE) || (socket_ptr -> nx_tcp_socket_fast_recovery == NX_FALSE))
    {

        /* Timed out on an outgoing packet.  Enter slow start mode. */
        /* Compute the flight size / 2 value. */
        window = socket_ptr -> nx_tcp_socket_tx_outstanding_bytes >> 1;

        /* Make sure we have at least 2 * MSS */
        if (window < (socket_ptr -> nx_tcp_socket_connect_mss << 1))
        {
            window = socket_ptr -> nx_tcp_socket_connect_mss << 1;
        }

        /* Set the slow_start_threshold */
        socket_ptr -> nx_tcp_socket_tx_slow_start_threshold = window;

        /* Set the current window to be MSS size. */
        socket_ptr -> nx_tcp_socket_tx_window_congestion = socket_ptr -> nx_tcp_socket_connect_mss;

        /* Determine if this socket needs fast retransmit.  */
        if (need_fast_retransmit == NX_TRUE)
        {

            /* Update cwnd to ssthreshold plus 3 * MSS.  */
            socket_ptr -> nx_tcp_socket_tx_window_congestion += window + (socket_ptr -> nx_tcp_socket_connect_mss << 1);

            /* Now TCP is in fast recovery procedure. */
            socket_ptr -> nx_tcp_socket_fast_recovery = NX_TRUE;

            /* Update the transmit sequence that enters fast transmit. */
            socket_ptr -> nx_tcp_socket_tx_sequence_recover = socket_ptr -> nx_tcp_socket_tx_sequence - 1;
        }
    }

    /* Setup the next timeout.  */
    socket_ptr -> nx_tcp_socket_timeout = socket_ptr -> nx_tcp_socket_timeout_rate <<
        (socket_ptr -> nx_tcp_socket_timeout_retries * socket_ptr -> nx_tcp_socket_timeout_shift);

    /* Get available size of packet that can be sent. */
    available = socket_ptr -> nx_tcp_socket_tx_window_congestion;

    /* Pickup the head of the transmit queue.  */
    packet_ptr =  socket_ptr -> nx_tcp_socket_transmit_sent_head;

    /* Determine if the packet has been released by the
       application I/O driver.  */
    /*lint -e{923} suppress cast of ULONG to pointer.  */
    while (packet_ptr && (packet_ptr -> nx_packet_queue_next == (NX_PACKET *)NX_DRIVER_TX_DONE))
    {

    /* Update the ACK number in case it has changed since the data was originally transmitted. */
    ULONG          checksum;
    NX_TCP_HEADER *header_ptr;
    ULONG         *source_ip = NX_NULL, *dest_ip = NX_NULL;
    NX_PACKET     *next_ptr;
#if defined(NX_DISABLE_TCP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
    UINT           compute_checksum = 1;
#endif /* defined(NX_DISABLE_TCP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */

#ifdef NX_DISABLE_TCP_TX_CHECKSUM
        compute_checksum = 0;
#endif /* NX_DISABLE_TCP_TX_CHECKSUM */

        if (packet_ptr -> nx_packet_length > (available + sizeof(NX_TCP_HEADER)))
        {

            /* This packet can not be sent. */
            break;
        }

        /* Decrease the available size. */
        available -= (packet_ptr -> nx_packet_length - (ULONG)sizeof(NX_TCP_HEADER));

        /* Pickup next packet. */
        next_ptr = packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next;

#ifndef NX_DISABLE_IPV4
        /* Is this an IPv4 connection? */
        if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
        {

            packet_ptr -> nx_packet_ip_version = NX_IP_VERSION_V4;

            /* Get the source and destination addresses. */
            source_ip = &socket_ptr -> nx_tcp_socket_connect_interface -> nx_interface_ip_address;
            dest_ip = &socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v4;
        }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
        if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
        {

            /* Set the packet for IPv6 connectivity. */
            packet_ptr -> nx_packet_ip_version = NX_IP_VERSION_V6;

            /* Get the source and destination addresses. */
            source_ip = socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address;
            dest_ip = socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v6;
        }
#endif /* FEATURE_NX_IPV6 */

        /* Pick up the pointer to the head of the TCP packet.  */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        header_ptr =  (NX_TCP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

        /* Record the original data.  */
        original_acknowledgment_number = header_ptr -> nx_tcp_acknowledgment_number;
        original_header_word_3 = header_ptr -> nx_tcp_header_word_3;
        original_header_word_4 = header_ptr -> nx_tcp_header_word_4;

        /* Update the ACK number in the TCP header.  */
        header_ptr -> nx_tcp_acknowledgment_number = socket_ptr -> nx_tcp_socket_rx_sequence;

        /* Convert to network byte order for checksum */
        NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_acknowledgment_number);

        /* Set window size. */
#ifdef NX_ENABLE_TCP_WINDOW_SCALING
        window_size = socket_ptr -> nx_tcp_socket_rx_window_current >> socket_ptr -> nx_tcp_rcv_win_scale_value;

        /* Make sure the window_size is less than 0xFFFF. */
        if (window_size > 0xFFFF)
        {
            window_size = 0xFFFF;
        }
#else
        window_size = socket_ptr -> nx_tcp_socket_rx_window_current;
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */

        header_ptr -> nx_tcp_header_word_3 =        NX_TCP_HEADER_SIZE | NX_TCP_ACK_BIT | NX_TCP_PSH_BIT | window_size;

        /* Swap the content to network byte order. */
        NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_3);

        /* Convert back to host byte order to so we can zero out the checksum. */
        NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_4);

        /* Remember the last ACKed sequence and the last reported window size.  */
        socket_ptr -> nx_tcp_socket_rx_sequence_acked =    socket_ptr -> nx_tcp_socket_rx_sequence;
        socket_ptr -> nx_tcp_socket_rx_window_last_sent =  socket_ptr -> nx_tcp_socket_rx_window_current;

        /* Zero out existing checksum before computing new one. */
        header_ptr -> nx_tcp_header_word_4 = header_ptr -> nx_tcp_header_word_4 & 0x0000FFFF;

        /* Convert back to network byte order to so we can do the checksum. */
        NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_4);


#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        if (socket_ptr -> nx_tcp_socket_connect_interface -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM)
        {
            compute_checksum = 0;
        }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#ifdef NX_IPSEC_ENABLE
        if ((packet_ptr -> nx_packet_ipsec_sa_ptr != NX_NULL) &&
            (((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_encryption_method != NX_CRYPTO_NONE))
        {
            compute_checksum = 1;
        }
#endif /* NX_IPSEC_ENABLE */

#if defined(NX_DISABLE_TCP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
        if (compute_checksum)
#endif /* defined(NX_DISABLE_TCP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
        {
            /* Calculate the TCP checksum without protection.  */
            checksum =  _nx_ip_checksum_compute(packet_ptr, NX_PROTOCOL_TCP,
                                                packet_ptr -> nx_packet_length,
                                                source_ip, dest_ip);
            checksum = ~checksum & NX_LOWER_16_MASK;

            /* Convert back to host byte order */
            NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_4);

            /* Move the checksum into header.  */
            header_ptr -> nx_tcp_header_word_4 =  header_ptr -> nx_tcp_header_word_4 | (checksum << NX_SHIFT_BY_16);

            /* Convert back to network byte order for transmit. */
            NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_4);
        }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        else
        {
            packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM;
        }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

        /* Determine if the retransmitted packet is identical to the original packet.
           RFC1122, Section3.2.1.5, Page32-33. RFC1122, Section4.2.2.15, Page90-91.  */
        if ((header_ptr -> nx_tcp_acknowledgment_number == original_acknowledgment_number) &&
            (header_ptr -> nx_tcp_header_word_3 == original_header_word_3) &&
            (header_ptr -> nx_tcp_header_word_4 == original_header_word_4))
        {

            /* Yes, identical packet, update the identification flag.  */
            packet_ptr -> nx_packet_identical_copy = NX_TRUE;
        }


#ifndef NX_DISABLE_TCP_INFO
        /* Increment the TCP retransmit count.  */
        ip_ptr -> nx_ip_tcp_retransmit_packets++;

        /* Increment the TCP retransmit count for the socket.  */
        socket_ptr -> nx_tcp_socket_retransmit_packets++;
#endif

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_RETRY, ip_ptr, socket_ptr, packet_ptr, socket_ptr -> nx_tcp_socket_timeout_retries, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Clear the queue next pointer.  */
        packet_ptr -> nx_packet_queue_next =  NX_NULL;

        /* Yes, the driver has finished with the packet at the head of the
           transmit sent list... so it can be sent again!  */

#ifndef NX_DISABLE_IPV4
        /* Is this an IPv4 connection? */
        if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
        {
            _nx_ip_packet_send(ip_ptr, packet_ptr,
                               socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v4,
                               socket_ptr -> nx_tcp_socket_type_of_service,
                               socket_ptr -> nx_tcp_socket_time_to_live, NX_IP_TCP,
                               socket_ptr -> nx_tcp_socket_fragment_enable,
                               socket_ptr -> nx_tcp_socket_next_hop_address);
        }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
        if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
        {

            /* Handle for an IPv6 connection. */
            /* Set the packet transmit interface before sending. */
            packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr = socket_ptr -> nx_tcp_socket_ipv6_addr;

            _nx_ipv6_packet_send(ip_ptr, packet_ptr, NX_PROTOCOL_TCP,
                                 packet_ptr -> nx_packet_length, ip_ptr -> nx_ipv6_hop_limit,
                                 socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address,
                                 socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v6);
        }
#endif /* FEATURE_NX_IPV6 */

        /* Move to next packet. */
        /* During fast recovery, only one packet is retransmitted at once. */
        /* After a timeout, the sending data can be at most one SMSS. */
        if ((next_ptr == (NX_PACKET *)NX_PACKET_ENQUEUED) ||
            (socket_ptr -> nx_tcp_socket_fast_recovery == NX_TRUE))
        {
            break;
        }
        else
        {
            packet_ptr = next_ptr;
        }
    }
}

