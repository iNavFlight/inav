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
#include "tx_thread.h"
#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */


#ifdef NX_ENABLE_TCPIP_OFFLOAD
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_driver_send                          PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a TCP packet through TCP/IP offload interface.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to socket             */
/*    packet_ptr                            Pointer to packet to send     */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_packet_send                    Packet send function          */
/*    _nx_ipv6_packet_send                  Packet send function          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_socket_send_internal                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-02-2021     Yuxin Zhou               Initial Version 6.1.8         */
/*                                                                        */
/**************************************************************************/
static UINT _nx_tcp_socket_driver_send(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr, ULONG wait_option)
{
UINT            status;
NX_IP          *ip_ptr;
NX_INTERFACE   *interface_ptr = socket_ptr -> nx_tcp_socket_connect_interface;
#ifdef NX_ENABLE_IP_PACKET_FILTER
UCHAR          *original_ptr = packet_ptr -> nx_packet_prepend_ptr;
ULONG           original_length = packet_ptr -> nx_packet_length;
NX_TCP_HEADER  *header_ptr;
#endif /* NX_ENABLE_IP_PACKET_FILTER */

    /* Setup the pointer to the associated IP instance.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;

#ifdef NX_ENABLE_IP_PACKET_FILTER
    /* Check if the IP packet filter is set. */
    if (ip_ptr -> nx_ip_packet_filter || ip_ptr -> nx_ip_packet_filter_extended)
    {

        /* Yes, add the TCP and IP Header to trigger filtering.  */          
        /* Prepend the TCP header to the packet.  First, make room for the TCP header.  */
        packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_TCP_HEADER);

        /* Add the length of the TCP header.  */
        packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length + (ULONG)sizeof(NX_TCP_HEADER);

        /* Pickup the pointer to the head of the TCP packet.  */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        header_ptr =  (NX_TCP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

        /* Build the output request in the TCP header.  */
        header_ptr -> nx_tcp_header_word_0 = (((ULONG)(socket_ptr -> nx_tcp_socket_port)) << NX_SHIFT_BY_16) |
                                                (ULONG)socket_ptr -> nx_tcp_socket_connect_port;
        header_ptr -> nx_tcp_acknowledgment_number = 0;
        header_ptr -> nx_tcp_sequence_number = 0;            
        header_ptr -> nx_tcp_header_word_3 = NX_TCP_HEADER_SIZE | NX_TCP_ACK_BIT | NX_TCP_PSH_BIT;
        header_ptr -> nx_tcp_header_word_4 = 0;
        
        /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
            swap the endian of the TCP header.  */
        NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_0);
        NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_3);

#ifndef NX_DISABLE_IPV4
        if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
        {
            _nx_ip_header_add(ip_ptr, packet_ptr,
                              socket_ptr -> nx_tcp_socket_connect_interface -> nx_interface_ip_address,
                              socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v4,
                              socket_ptr -> nx_tcp_socket_type_of_service,
                              socket_ptr -> nx_tcp_socket_time_to_live,
                              NX_IP_TCP,
                              socket_ptr -> nx_tcp_socket_fragment_enable);
        }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
        if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
        {
            if (_nx_ipv6_header_add(ip_ptr, &packet_ptr,
                                    NX_PROTOCOL_TCP,
                                    packet_ptr -> nx_packet_length,
                                    ip_ptr -> nx_ipv6_hop_limit,
                                    socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address,
                                    socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v6,
                                    NX_NULL))
            {

                /* Invalid interface address. Just return success.  */
                return(NX_SUCCESS);
            }
        }
#endif /* FEATURE_NX_IPV6 */

        if (ip_ptr -> nx_ip_packet_filter)
        {
            if (ip_ptr -> nx_ip_packet_filter((VOID *)(packet_ptr -> nx_packet_prepend_ptr),
                                              NX_IP_PACKET_OUT) != NX_SUCCESS)
            {

                /* Packet consumed by IP filter. Just return success.  */
                _nx_packet_transmit_release(packet_ptr);
                return(NX_SUCCESS);
            }
        }

        /* Check if the IP packet filter extended is set. */
        if (ip_ptr -> nx_ip_packet_filter_extended)
        {

            /* Yes, call the IP packet filter extended routine. */
            if (ip_ptr -> nx_ip_packet_filter_extended(ip_ptr, packet_ptr, NX_IP_PACKET_OUT) != NX_SUCCESS)
            {

                /* Packet consumed by IP filter. Just return success.  */
                _nx_packet_transmit_release(packet_ptr);
                return(NX_SUCCESS);
            }
        }

        /* Reset UDP and IP header.  */
        packet_ptr -> nx_packet_prepend_ptr = original_ptr;
        packet_ptr -> nx_packet_length = original_length;
    }
#endif /* NX_ENABLE_IP_PACKET_FILTER */

    /* Let TCP/IP offload interface send the packet.  */
    status = interface_ptr -> nx_interface_tcpip_offload_handler(ip_ptr, interface_ptr, socket_ptr,
                                                                 NX_TCPIP_OFFLOAD_TCP_SOCKET_SEND,
                                                                 packet_ptr, NX_NULL, NX_NULL, 0, NX_NULL,
                                                                 wait_option);

    if (status)
    {
        return(NX_TCPIP_OFFLOAD_ERROR);
    }
    else
    {
        return(NX_SUCCESS);
    }
}
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_send_internal                        PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a TCP packet through the specified socket.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to socket             */
/*    packet_ptr                            Pointer to packet to send     */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_packet_send                    Packet send function          */
/*    _nx_ipv6_packet_send                  Packet send function          */
/*    _nx_ip_checksum_compute               Calculate TCP checksum        */
/*    _nx_tcp_socket_thread_suspend         Suspend calling thread        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    _nx_tcp_socket_driver_send            TCP/IP offload send function  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_socket_send                                                 */
/*                                                                        */
/*  NOTE:                                                                 */
/*                                                                        */
/*    This is an internal function, only being called by                  */
/*    _nx_tcp_socket_send().  The caller guarantees that the payload      */
/*    size does not exceed MSS.                                           */
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
/*  10-15-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            fixed the bug of race       */
/*                                            condition,                  */
/*                                            resulting in version 6.1.9  */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved the throughput of  */
/*                                            TCP transmission,           */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT  _nx_tcp_socket_send_internal(NX_TCP_SOCKET *socket_ptr, NX_PACKET *packet_ptr, ULONG wait_option)
{

TX_INTERRUPT_SAVE_AREA

NX_IP          *ip_ptr;
NX_PACKET_POOL *pool_ptr;
NX_TCP_HEADER  *header_ptr;
ULONG           checksum = 0;
ULONG           sequence_number;
ULONG           tx_window_current;
ULONG           remaining_bytes;
ULONG          *source_ip = NX_NULL, *dest_ip = NX_NULL;
ULONG           send_mss;
NX_PACKET      *send_packet = packet_ptr;
NX_PACKET      *current_packet;
UCHAR          *current_ptr;
ULONG           data_offset = 0;
ULONG           source_data_size;
ULONG           copy_size;
UINT            data_left;
UINT            ret;
UCHAR           preempted = NX_FALSE;
UCHAR           adjust_packet;
UINT            old_threshold = 0;
ULONG           window_size;
#ifdef NX_ENABLE_TCPIP_OFFLOAD
UINT            status;
NX_INTERFACE   *interface_ptr;
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
#if defined(NX_DISABLE_TCP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT            compute_checksum = 1;
#endif /* defined(NX_DISABLE_TCP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */

#ifdef NX_DISABLE_TCP_TX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_TCP_TX_CHECKSUM */

    /* Check packet length. */
    if (packet_ptr -> nx_packet_length == 0)
    {

        /* Empty packet is not allowed. */
        return(NX_INVALID_PACKET);
    }

    /* Lockout interrupts.  */
    TX_DISABLE

    /* Determine if the socket is currently bound.  */
    if (!socket_ptr ->  nx_tcp_socket_bound_next)
    {

        /* Restore interrupts.  */
        TX_RESTORE

        /* Socket is not bound, return an error message.  */
        return(NX_NOT_BOUND);
    }

    /* Pickup the important information from the socket.  */

    /* Setup the pointer to the associated IP instance.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;

    /* Restore interrupts.  */
    TX_RESTORE

    /* Check if the connection is in progress. */
    if ((socket_ptr -> nx_tcp_socket_state == NX_TCP_SYN_SENT) || (socket_ptr -> nx_tcp_socket_state == NX_TCP_SYN_RECEIVED))
    {

        /* Yes it it. Wait for establish state. */
        _nx_tcp_socket_state_wait(socket_ptr, NX_TCP_ESTABLISHED, wait_option);
    }

    /* Obtain the IP mutex.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Check for the socket being in an established state.  */
    if ((socket_ptr -> nx_tcp_socket_state != NX_TCP_ESTABLISHED) && (socket_ptr -> nx_tcp_socket_state != NX_TCP_CLOSE_WAIT))
    {

        /* Release the protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Socket is not connected, return an error message.  */
        return(NX_NOT_CONNECTED);
    }

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

#ifndef NX_DISABLE_IPV4
    if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
    {

        /* Set the source address. */
        source_ip = &socket_ptr -> nx_tcp_socket_connect_interface -> nx_interface_ip_address;

        /* Set the destination address. */
        dest_ip = &socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v4;

        /* The outgoing interface should have been stored in the socket structure. */
        packet_ptr -> nx_packet_address.nx_packet_interface_ptr = socket_ptr -> nx_tcp_socket_connect_interface;

        /* Calculate the data offset required by fragmented TCP packet. */
        data_offset = NX_PHYSICAL_HEADER + sizeof(NX_IPV4_HEADER) + sizeof(NX_TCP_HEADER);
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
    {

        /* Determine whether or not the IPv6 address is valid. */
        if (socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address_state != NX_IPV6_ADDR_STATE_VALID)
        {

            /* Release the protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Add debug information. */
            NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

            return(NX_NO_INTERFACE_ADDRESS);
        }
        /* Set the source address. */
        source_ip = socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address;

        /* Set the destination address. */
        dest_ip = socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v6;

        /* The outgoing address should have been stored in the socket structure. */
        packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr = socket_ptr -> nx_tcp_socket_ipv6_addr;

        /* Calculate the data offset required by fragmented TCP packet. */
        data_offset = NX_PHYSICAL_HEADER + sizeof(NX_IPV6_HEADER) + sizeof(NX_TCP_HEADER);
    }
#endif /* FEATURE_NX_IPV6 */

#ifdef NX_ENABLE_TCPIP_OFFLOAD
    interface_ptr = socket_ptr -> nx_tcp_socket_connect_interface;
    if ((interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD) &&
        (interface_ptr -> nx_interface_tcpip_offload_handler))
    {

        /* This interface supports TCP/IP offload.  */
        status = _nx_tcp_socket_driver_send(socket_ptr, packet_ptr, wait_option);

        /* Release the IP protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        return(status);
    }
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

#ifdef NX_IPSEC_ENABLE
    /* Increase the data offset when IPsec is enabled. */
    data_offset += socket_ptr -> nx_tcp_socket_egress_sa_data_offset;
#endif /* NX_IPSEC_ENABLE */

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_SOCKET_SEND, socket_ptr, packet_ptr, packet_ptr -> nx_packet_length, socket_ptr -> nx_tcp_socket_tx_sequence, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Get the max mss this socket could send  */
    send_mss = socket_ptr -> nx_tcp_socket_connect_mss;

    /* Get original pool. */
    pool_ptr = packet_ptr -> nx_packet_pool_owner;

    /* Loop to send the packet. */
    for (;;)
    {

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

                /* Make sure the tx_window_current is less or equal to swnd. */
                if (tx_window_current > socket_ptr -> nx_tcp_socket_tx_window_advertised)
                {
                    tx_window_current = socket_ptr -> nx_tcp_socket_tx_window_advertised;
                }
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

        /* Pick up the min(tx_window, send_mss). */
        if (tx_window_current > send_mss)
        {
            tx_window_current = send_mss;
        }


        /* Store the data that is left. */
        data_left = packet_ptr -> nx_packet_length;

        /* Check whether data can be sent. */
        if ((tx_window_current != 0) && (socket_ptr -> nx_tcp_socket_transmit_sent_count < socket_ptr -> nx_tcp_socket_transmit_queue_maximum))
        {

            /* Whether to adjust the packet? */
            if (packet_ptr -> nx_packet_length > tx_window_current)
            {

                /* Packet need to be fragmented. */
                adjust_packet = NX_TRUE;
            }
            /*lint -e(923) suppress cast of pointer to ULONG.  */
            else if (((ALIGN_TYPE)packet_ptr -> nx_packet_prepend_ptr) & 3)
            {

                /* Starting address of TCP header need to be four bytes aligned. */
                adjust_packet = NX_TRUE;
            }
#ifndef NX_DISABLE_PACKET_CHAIN
            else if ((packet_ptr -> nx_packet_next != NX_NULL) &&
                     ((packet_ptr -> nx_packet_length + data_offset) < pool_ptr -> nx_packet_pool_payload_size) &&
                     (pool_ptr -> nx_packet_pool_available > 0))
            {

                /* All data can be sent in one packet but they are in chained packets. */
                adjust_packet = NX_TRUE;
            }
            else if (packet_ptr -> nx_packet_prepend_ptr == packet_ptr -> nx_packet_append_ptr)
            {

                /* Loop to find the first byte of data. */
                current_packet = packet_ptr -> nx_packet_next;

                while ((current_packet != NX_NULL) && (current_packet -> nx_packet_prepend_ptr == current_packet -> nx_packet_append_ptr))
                {

                    /* Move to next packet. */
                    current_packet = current_packet -> nx_packet_next;
                }

                /* packet length is not 0. Therefore the packet chain is expected to contain data. */
                NX_ASSERT(current_packet != NX_NULL);

                /*lint -e{923} suppress cast of pointer to ULONG.  */
                if (((ALIGN_TYPE)current_packet -> nx_packet_prepend_ptr) & 3)
                {

                    /* Starting address of TCP data need to be four bytes aligned. */
                    adjust_packet = NX_TRUE;
                }
                else
                {

                    /* Packet can be sent directly. */
                    adjust_packet = NX_FALSE;
                }
            }
#endif /* NX_DISABLE_PACKET_CHAIN */
            else
            {

                /* Packet can be sent directly. */
                adjust_packet = NX_FALSE;
            }

            /* Adjust the packet? */
            if (adjust_packet)
            {

                /* Yes. Obtain the size of the packet can be sent. */
                if (packet_ptr -> nx_packet_length > tx_window_current)
                {
                    remaining_bytes = tx_window_current;
                }
                else
                {
                    remaining_bytes = packet_ptr -> nx_packet_length;
                }

                /* Points to the source packet. */
                current_packet = packet_ptr;

                /* Mark the beginning of data. */
                current_ptr = packet_ptr -> nx_packet_prepend_ptr;

                /* Release the protection.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                /* Obtain a new segmentation. */
                ret = _nx_packet_allocate(pool_ptr, &send_packet,
                                          data_offset, wait_option);

                if (ret != NX_SUCCESS)
                {

                    /* Restore preemption? */
                    if (preempted == NX_TRUE)
                    {

                        /*lint -e{644} -e{530} suppress variable might not be initialized, since "old_threshold" was initialized when preempted was set to NX_TRUE. */
                        tx_thread_preemption_change(_tx_thread_current_ptr, old_threshold, &old_threshold);
                    }

                    /* Packet allocate failure. Return.*/
                    return(ret);
                }

                /* Regain exclusive access to IP instance. */
                tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

                /* Add debug information. */
                NX_PACKET_DEBUG(__FILE__, __LINE__, send_packet);

                /* Loop through the entire source packet. */
                while (remaining_bytes)
                {

                    /* Figure out whether or not the source packet still contains data. */
                    /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
                    source_data_size = (ULONG)(current_packet -> nx_packet_append_ptr - current_ptr);
                    while (source_data_size == 0)
                    {

#ifndef NX_DISABLE_PACKET_CHAIN
                        /* The current buffer is exhausted.  Move to the next buffer on the source packet chain. */
                        current_packet = current_packet -> nx_packet_next;

                        if (current_packet == NX_NULL)
                        {
#endif /* NX_DISABLE_PACKET_CHAIN */

                            /* Restore preemption? */
                            if (preempted == NX_TRUE)
                            {
                                tx_thread_preemption_change(_tx_thread_current_ptr, old_threshold, &old_threshold);
                            }

                            /* No more data in the source packet. However there are still bytes remaining even though
                               the packet is not done yet. This is an unrecoverable error. */
                            /*lint -e{644} suppress variable might not be initialized, since "send_packet" was initialized in _nx_packet_allocate. */
                            _nx_packet_release(send_packet);

                            /* Release the protection.  */
                            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                            /* Add debug information. */
                            NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

                            return(NX_INVALID_PACKET);
#ifndef NX_DISABLE_PACKET_CHAIN
                        }

                        /* Mark the beginning of data in the next packet. */
                        current_ptr = current_packet -> nx_packet_prepend_ptr;

                        /* Compute the amount of data present in this source buffer. */
                        /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
                        source_data_size = (ULONG)(current_packet -> nx_packet_append_ptr - current_ptr);
#endif /* NX_DISABLE_PACKET_CHAIN */
                    }


                    /* copy_size = min(send_packet, source) */
                    if (remaining_bytes > source_data_size)
                    {
                        copy_size = source_data_size;
                    }
                    else
                    {
                        copy_size = remaining_bytes;
                    }

                    /* Release the mutex before a blocking call. */
                    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                    /* Append data. */
                    ret = _nx_packet_data_append(send_packet, current_ptr, copy_size,
                                                 pool_ptr, wait_option);

                    /* Regain exclusive access to IP instance. */
                    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

                    /* Check for errors with data append. */
                    if (ret != NX_SUCCESS)
                    {

                        /* Append failed. */
                        if (send_packet -> nx_packet_length == 0)
                        {

                            /* The packet is empty, return. */
                            /* Restore preemption? */
                            if (preempted == NX_TRUE)
                            {

                                /*lint -e{644} -e{530} suppress variable might not be initialized, since "old_threshold" was initialized when preempted was set to NX_TRUE. */
                                tx_thread_preemption_change(_tx_thread_current_ptr, old_threshold, &old_threshold);
                            }

                            /* Release the protection.  */
                            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                            /* Release the packet. */
                            _nx_packet_release(send_packet);

                            /* Packet allocate failure. Return.*/
                            return(ret);
                        }

                        /* Partial data can be sent. Just break. */
                        break;
                    }

                    /* Reduce the remaining_bytes counter by the amount being copied over. */
                    remaining_bytes -= copy_size;

                    /* Advance the prepend ptr on the source buffer, by the amount being copied. */
                    current_ptr += copy_size;
                }

                send_packet -> nx_packet_address = packet_ptr -> nx_packet_address;
            }
            else
            {

                /* Send the packet directly. */
                send_packet = packet_ptr;
            }

            /* Now the send_packet can be sent. */
            /* Set IP version. */
            send_packet -> nx_packet_ip_version = (UCHAR)(socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version);

#ifdef NX_IPSEC_ENABLE
            send_packet -> nx_packet_ipsec_sa_ptr = socket_ptr -> nx_tcp_socket_egress_sa;
#endif /* NX_IPSEC_ENABLE */

            /* Prepend the TCP header to the packet.  First, make room for the TCP header.  */
            send_packet -> nx_packet_prepend_ptr =  send_packet -> nx_packet_prepend_ptr - sizeof(NX_TCP_HEADER);

            /* Add the length of the TCP header.  */
            send_packet -> nx_packet_length =  send_packet -> nx_packet_length + (ULONG)sizeof(NX_TCP_HEADER);

            /* Pickup the pointer to the head of the TCP packet.  */
            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            header_ptr =  (NX_TCP_HEADER *)send_packet -> nx_packet_prepend_ptr;

            /* Build the output request in the TCP header.  */
            header_ptr -> nx_tcp_header_word_0 =        (((ULONG)(socket_ptr -> nx_tcp_socket_port)) << NX_SHIFT_BY_16) | (ULONG)socket_ptr -> nx_tcp_socket_connect_port;
            header_ptr -> nx_tcp_acknowledgment_number = socket_ptr -> nx_tcp_socket_rx_sequence;

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
            header_ptr -> nx_tcp_header_word_4 =        0;

            /* Remember the last ACKed sequence and the last reported window size.  */
            socket_ptr -> nx_tcp_socket_rx_sequence_acked =    socket_ptr -> nx_tcp_socket_rx_sequence;
            socket_ptr -> nx_tcp_socket_rx_window_last_sent =  socket_ptr -> nx_tcp_socket_rx_window_current;

            /* Setup a new delayed ACK timeout.  */
            socket_ptr -> nx_tcp_socket_delayed_ack_timeout =  _nx_tcp_ack_timer_rate;

            /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
               swap the endian of the TCP header.  */
            NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_0);
            NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_acknowledgment_number);
            NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_3);
            NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_4);

            /* Release the protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Pickup the current transmit sequence number.  */
            header_ptr -> nx_tcp_sequence_number =  socket_ptr -> nx_tcp_socket_tx_sequence;
            sequence_number =  header_ptr -> nx_tcp_sequence_number;

            /* Swap the headers for endianness. */
            NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_sequence_number);

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
            if (socket_ptr -> nx_tcp_socket_connect_interface -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM)
            {
                compute_checksum = 0;
            }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#ifdef NX_IPSEC_ENABLE
            if ((send_packet -> nx_packet_ipsec_sa_ptr != NX_NULL) && (((NX_IPSEC_SA *)(send_packet -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_encryption_method != NX_CRYPTO_NONE))
            {
                compute_checksum = 1;
            }
#endif /* NX_IPSEC_ENABLE */

#if defined(NX_DISABLE_TCP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
            if (compute_checksum)
#endif /* defined(NX_DISABLE_TCP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
            {
                /* Calculate the TCP checksum without protection.  */
                checksum =  _nx_ip_checksum_compute(send_packet, NX_PROTOCOL_TCP,
                                                    (UINT)send_packet -> nx_packet_length,
                                                    source_ip, dest_ip);
                checksum = ~checksum & NX_LOWER_16_MASK;
            }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
            else
            {
                send_packet -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_TCP_TX_CHECKSUM;
            }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

            /* Place protection while we check the sequence number for the new TCP packet.  */
            tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

            /* Determine if the sequence number is the same.  */
            if (sequence_number != socket_ptr -> nx_tcp_socket_tx_sequence)
            {

                /* Another transmit on this socket took place and changed the sequence.  We need to
                   recalculate the checksum with a new sequence number.  Release protection and
                   just resume the loop.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                /* Release the packet when the sequence is changed. */
                if (send_packet != packet_ptr)
                {
                    _nx_packet_release(send_packet);
                }

                /* Regain exclusive access to IP instance. */
                tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);
                continue;
            }

            /* Check for the socket being in an established state.  It's possible the connection could have gone
               away during the TCP checksum calculation above.  */
            if ((socket_ptr -> nx_tcp_socket_state != NX_TCP_ESTABLISHED) && (socket_ptr -> nx_tcp_socket_state != NX_TCP_CLOSE_WAIT))
            {

                /* Restore preemption? */
                if (preempted == NX_TRUE)
                {
                    tx_thread_preemption_change(_tx_thread_current_ptr, old_threshold, &old_threshold);
                }

                /* Release protection.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                /* Release the packet when the sequence is changed. */
                if (send_packet != packet_ptr)
                {
                    _nx_packet_release(send_packet);
                }

                /* Add debug information. */
                NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

                /* Socket is not connected, return an error message.  */
                return(NX_NOT_CONNECTED);
            }

            /* Disable interrupts.  */
            TX_DISABLE

            /* Adjust the transmit sequence number to reflect the output data.  */
            socket_ptr -> nx_tcp_socket_tx_sequence = socket_ptr -> nx_tcp_socket_tx_sequence +
                (send_packet -> nx_packet_length - (ULONG)sizeof(NX_TCP_HEADER));

            /* Restore interrupts.  */
            TX_RESTORE

            /* Reset zero window probe flag. */
            socket_ptr -> nx_tcp_socket_zero_window_probe_has_data = NX_FALSE;

            /* Move the checksum into header.  */
            NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_4);
            header_ptr -> nx_tcp_header_word_4 =  (checksum << NX_SHIFT_BY_16);
            NX_CHANGE_ULONG_ENDIAN(header_ptr -> nx_tcp_header_word_4);

            /* Place the packet on the sent list.  */
            data_left -= (send_packet -> nx_packet_length - (ULONG)sizeof(NX_TCP_HEADER));
            if (socket_ptr -> nx_tcp_socket_transmit_sent_head)
            {

                /* Yes, other packets are on the list already.  Just add this one to the tail.  */
                (socket_ptr -> nx_tcp_socket_transmit_sent_tail) -> nx_packet_union_next.nx_packet_tcp_queue_next =  send_packet;
                socket_ptr -> nx_tcp_socket_transmit_sent_tail =  send_packet;
            }
            else
            {

                /* Empty list, just setup the head and tail to the current packet.  */
                socket_ptr -> nx_tcp_socket_transmit_sent_head =  send_packet;
                socket_ptr -> nx_tcp_socket_transmit_sent_tail =  send_packet;

                /* Setup a timeout for the packet at the head of the list.  */
                socket_ptr -> nx_tcp_socket_timeout =          socket_ptr -> nx_tcp_socket_timeout_rate;
                socket_ptr -> nx_tcp_socket_timeout_retries =  0;
                socket_ptr -> nx_tcp_socket_tx_outstanding_bytes = 0;
            }

            /* Set the next pointer to NX_PACKET_ENQUEUED to indicate the packet is part of a TCP queue.  */
            /*lint -e{923} suppress cast of ULONG to pointer.  */
            send_packet -> nx_packet_union_next.nx_packet_tcp_queue_next =  (NX_PACKET *)NX_PACKET_ENQUEUED;

            /* Increment the packet sent count.  */
            socket_ptr -> nx_tcp_socket_transmit_sent_count++;

            /* Increase the transmit outstanding byte count. */
            socket_ptr -> nx_tcp_socket_tx_outstanding_bytes +=
                (send_packet -> nx_packet_length - (ULONG)sizeof(NX_TCP_HEADER));
#ifndef NX_DISABLE_TCP_INFO
            /* Increment the TCP packet sent count and bytes sent count.  */
            ip_ptr -> nx_ip_tcp_packets_sent++;
            ip_ptr -> nx_ip_tcp_bytes_sent += send_packet -> nx_packet_length - (ULONG)sizeof(NX_TCP_HEADER);

            /* Increment the TCP packet sent count and bytes sent count for the socket.  */
            socket_ptr -> nx_tcp_socket_packets_sent++;
            socket_ptr -> nx_tcp_socket_bytes_sent += send_packet -> nx_packet_length - (ULONG)sizeof(NX_TCP_HEADER);
#endif /* NX_DISABLE_TCP_INFO */

            /* If trace is enabled, insert this event into the trace buffer.  */
            NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_DATA_SEND, ip_ptr, socket_ptr, send_packet, socket_ptr -> nx_tcp_socket_tx_sequence - (send_packet -> nx_packet_length - sizeof(NX_TCP_HEADER)), NX_TRACE_INTERNAL_EVENTS, 0, 0);

            /* Send the TCP packet to the IP component.  */
#ifndef NX_DISABLE_IPV4
            if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
            {


                _nx_ip_packet_send(ip_ptr, send_packet,
                                   socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v4,
                                   socket_ptr -> nx_tcp_socket_type_of_service,
                                   socket_ptr -> nx_tcp_socket_time_to_live,
                                   NX_IP_TCP,
                                   socket_ptr -> nx_tcp_socket_fragment_enable,
                                   socket_ptr -> nx_tcp_socket_next_hop_address);
            }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
            if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
            {

                /* Ready to send the packet! */
                _nx_ipv6_packet_send(ip_ptr,
                                     send_packet,
                                     NX_PROTOCOL_TCP,
                                     send_packet -> nx_packet_length,
                                     ip_ptr -> nx_ipv6_hop_limit,
                                     socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address,
                                     socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v6);
            }
#endif /* FEATURE_NX_IPV6 */

            if (data_left == 0)
            {

                /* Release the packet. */
                if (send_packet != packet_ptr)
                {
                    _nx_packet_release(packet_ptr);
                }

                /* Restore preemption? */
                if (preempted == NX_TRUE)
                {
                    tx_thread_preemption_change(_tx_thread_current_ptr, old_threshold, &old_threshold);
                }

                /* Release the protection.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                /* Add debug information. */
                NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

                /* Return successful status.  */
                return(NX_SUCCESS);
            }
            else
            {

                /* Adjust the orginal packet. */
                current_packet = packet_ptr;

                remaining_bytes = packet_ptr -> nx_packet_length - data_left;
#ifndef NX_DISABLE_PACKET_CHAIN
                /*lint -e{946} -e{947} suppress pointer subtraction, since it is necessary. */
                while (remaining_bytes >
                       (UINT)(current_packet -> nx_packet_append_ptr - current_packet -> nx_packet_prepend_ptr))
                {

                    /* Trim all data in the train. */
                    /*lint -e{923} suppress cast of pointer to ULONG.  */
                    packet_ptr -> nx_packet_length -= (ULONG)((ALIGN_TYPE)current_packet -> nx_packet_append_ptr - (ALIGN_TYPE)current_packet -> nx_packet_prepend_ptr);

                    /*lint -e{923} suppress cast of pointer to ULONG.  */
                    remaining_bytes -= (ULONG)((ALIGN_TYPE)current_packet -> nx_packet_append_ptr - (ALIGN_TYPE)current_packet -> nx_packet_prepend_ptr);

                    /*lint -e{923} suppress cast between ULONG and pointer.  */
                    current_packet -> nx_packet_append_ptr = (UCHAR *)(((ALIGN_TYPE)current_packet -> nx_packet_append_ptr) & (ALIGN_TYPE)(~3));
                    current_packet -> nx_packet_prepend_ptr = current_packet -> nx_packet_append_ptr;

                    /* Pointer to next packet. */
                    current_packet = current_packet -> nx_packet_next;
                }
#endif /* NX_DISABLE_PACKET_CHAIN */

                /* Trim partial data in the packet. */
                packet_ptr -> nx_packet_length -= remaining_bytes;
                current_packet -> nx_packet_prepend_ptr += remaining_bytes;

                /* Release the protection.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                /* Regain exclusive access to IP instance. */
                tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);
            }
        }
        else if ((wait_option) && (_tx_thread_current_ptr != &(ip_ptr -> nx_ip_thread)))
        {

            /* Suspend the thread on this socket's transmit queue.  */

            /* Save the return packet pointer address as well.  */
            _tx_thread_current_ptr -> tx_thread_additional_suspend_info =  (VOID *)packet_ptr;

            /* Add debug information. */
            NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

            /* Increment the suspended thread count.  */
            socket_ptr -> nx_tcp_socket_transmit_suspended_count++;

            if (socket_ptr -> nx_tcp_socket_zero_window_probe_has_data == NX_FALSE)
            {

                /* Set data for zero window probe. */
                socket_ptr -> nx_tcp_socket_zero_window_probe_has_data = NX_TRUE;
                socket_ptr -> nx_tcp_socket_zero_window_probe_data = *(packet_ptr -> nx_packet_prepend_ptr);
                socket_ptr -> nx_tcp_socket_zero_window_probe_sequence = socket_ptr -> nx_tcp_socket_tx_sequence;
                socket_ptr -> nx_tcp_socket_zero_window_probe_failure = 0;
            }

            /* Need preemption? */
            if (preempted == NX_FALSE)
            {
            UINT ip_thread_priority;

                /* Yes. It will be able to send packet out immediately TCP window is non zero. */
                tx_thread_info_get(&ip_ptr -> nx_ip_thread, NX_NULL, NX_NULL, NX_NULL, &ip_thread_priority, NX_NULL,
                                   NX_NULL, NX_NULL, NX_NULL);

                /*lint -e{644} suppress variable might not be initialized, since "ip_thread_priority" was initialized before TCP is enabled. */
                tx_thread_preemption_change(_tx_thread_current_ptr, ip_thread_priority, &old_threshold);
                preempted = NX_TRUE;
            }

            /* Suspend the thread on the transmit suspension list.  */
            _nx_tcp_socket_thread_suspend(&(socket_ptr -> nx_tcp_socket_transmit_suspension_list), _nx_tcp_transmit_cleanup, socket_ptr, &(ip_ptr -> nx_ip_protection), wait_option);

            /* Determine if the send request was successful.  */
            if (_tx_thread_current_ptr -> tx_thread_suspend_status)
            {

                /* Restore preemption. */
                tx_thread_preemption_change(_tx_thread_current_ptr, old_threshold, &old_threshold);

                /* Add debug information. */
                NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

                /* Just return the error code.  */
                return(_tx_thread_current_ptr -> tx_thread_suspend_status);
            }
        }
        else
        {

            /* Check advertised window. */
            if (socket_ptr -> nx_tcp_socket_zero_window_probe_has_data == NX_FALSE)
            {

                /* Set data for zero window probe. */
                socket_ptr -> nx_tcp_socket_zero_window_probe_has_data = NX_TRUE;
                socket_ptr -> nx_tcp_socket_zero_window_probe_data = *(packet_ptr -> nx_packet_prepend_ptr);
                socket_ptr -> nx_tcp_socket_zero_window_probe_sequence = socket_ptr -> nx_tcp_socket_tx_sequence;
                socket_ptr -> nx_tcp_socket_zero_window_probe_failure = 0;
            }

            /* Determine which transmit error is present.  */
            if (socket_ptr -> nx_tcp_socket_transmit_sent_count < socket_ptr -> nx_tcp_socket_transmit_queue_maximum)
            {

                /* Release protection.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                /* Add debug information. */
                NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

                /* Not a queue depth problem, return a window overflow error.  */
                return(NX_WINDOW_OVERFLOW);
            }
            else
            {

                /* Release protection.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                /* Add debug information. */
                NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

                /* Return a transmit queue exceeded error.  */
                return(NX_TX_QUEUE_DEPTH);
            }
        }
    }
}