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
/**   User Datagram Protocol (UDP)                                        */
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
#include "tx_thread.h"
#include "nx_packet.h"
#include "nx_udp.h"
#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_udp_socket_receive                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for a received UDP packet on the specified     */
/*    socket and if no packets are on the receive queue, suspends for the */
/*    wait option duration.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to UDP socket         */
/*    packet_ptr                            Pointer to UDP packet pointer */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Release data packet           */
/*    _tx_thread_system_suspend             Suspend thread                */
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
UINT  _nx_udp_socket_receive(NX_UDP_SOCKET *socket_ptr, NX_PACKET **packet_ptr, ULONG wait_option)
{
TX_INTERRUPT_SAVE_AREA

ULONG                 *temp_ptr;
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
NX_INTERFACE          *interface_ptr = NX_NULL;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
#if defined(NX_DISABLE_UDP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT                   compute_checksum = 1;
#endif /* defined(NX_DISABLE_UDP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
TX_THREAD             *thread_ptr;
#ifdef TX_ENABLE_EVENT_TRACE
TX_TRACE_BUFFER_ENTRY *trace_event;
ULONG                  trace_timestamp;
#endif


#ifdef NX_DISABLE_UDP_RX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_UDP_RX_CHECKSUM */

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_UDP_SOCKET_RECEIVE, socket_ptr -> nx_udp_socket_ip_ptr, socket_ptr, 0, 0, NX_TRACE_UDP_EVENTS, &trace_event, &trace_timestamp);

    /* Set the return pointer to NULL initially.  */
    *packet_ptr =   NX_NULL;

    /* Loop to retrieve a packet from the interface.  */
    for (;;)
    {

        /* Lockout interrupts.  */
        TX_DISABLE

        /* Determine if the socket is currently bound.  */
        if (!socket_ptr ->  nx_udp_socket_bound_next)
        {

            /* Restore interrupts.  */
            TX_RESTORE

            /* Socket is not bound, return an error message.  */
            return(NX_NOT_BOUND);
        }

        /* Determine if there is a packet already queued up for this socket.  */
        if (socket_ptr -> nx_udp_socket_receive_head)
        {

            /* Yes, there is a packet waiting.  */

            /* Remove it and place it in the thread's destination.  */
            *packet_ptr =  socket_ptr -> nx_udp_socket_receive_head;
            socket_ptr -> nx_udp_socket_receive_head =  (*packet_ptr) -> nx_packet_queue_next;

            /* If this was the last packet, set the tail pointer to NULL.  */
            if (socket_ptr -> nx_udp_socket_receive_head == NX_NULL)
            {
                socket_ptr -> nx_udp_socket_receive_tail =  NX_NULL;
            }

            /* Decrease the queued packet count.  */
            socket_ptr -> nx_udp_socket_receive_count--;

            /* Restore interrupts.  */
            TX_RESTORE
        }
        else
        {

            /* Determine if the request specifies suspension.  */
            if (wait_option)
            {

                /* Prepare for suspension of this thread.  */

                /* Pickup thread pointer.  */
                thread_ptr =  _tx_thread_current_ptr;

                /* Setup cleanup routine pointer.  */
                thread_ptr -> tx_thread_suspend_cleanup =  _nx_udp_receive_cleanup;

                /* Setup cleanup information, i.e. this pool control
                   block.  */
                thread_ptr -> tx_thread_suspend_control_block =  (void *)socket_ptr;

                /* Save the return packet pointer address as well.  */
                thread_ptr -> tx_thread_additional_suspend_info =  (void *)packet_ptr;

                /* Setup suspension list.  */
                if (socket_ptr -> nx_udp_socket_receive_suspension_list)
                {

                    /* This list is not NULL, add current thread to the end. */
                    thread_ptr -> tx_thread_suspended_next =
                        socket_ptr -> nx_udp_socket_receive_suspension_list;
                    thread_ptr -> tx_thread_suspended_previous =
                        (socket_ptr -> nx_udp_socket_receive_suspension_list) -> tx_thread_suspended_previous;
                    ((socket_ptr -> nx_udp_socket_receive_suspension_list) -> tx_thread_suspended_previous) -> tx_thread_suspended_next =
                        thread_ptr;
                    (socket_ptr -> nx_udp_socket_receive_suspension_list) -> tx_thread_suspended_previous =   thread_ptr;
                }
                else
                {

                    /* No other threads are suspended.  Setup the head pointer and
                       just setup this threads pointers to itself.  */
                    socket_ptr -> nx_udp_socket_receive_suspension_list =   thread_ptr;
                    thread_ptr -> tx_thread_suspended_next              =   thread_ptr;
                    thread_ptr -> tx_thread_suspended_previous          =   thread_ptr;
                }

                /* Increment the suspended thread count.  */
                socket_ptr -> nx_udp_socket_receive_suspended_count++;

                /* Set the state to suspended.  */
                thread_ptr -> tx_thread_state =  TX_TCP_IP;

                /* Set the suspending flag.  */
                thread_ptr -> tx_thread_suspending =  TX_TRUE;

                /* Temporarily disable preemption.  */
                _tx_thread_preempt_disable++;

                /* Save the timeout value.  */
                thread_ptr -> tx_thread_timer.tx_timer_internal_remaining_ticks =  wait_option;

                /* Restore interrupts.  */
                TX_RESTORE

                /* Call actual thread suspension routine.  */
                _tx_thread_system_suspend(thread_ptr);

                /* Determine if a packet was received successfully.  */
                if (thread_ptr -> tx_thread_suspend_status != NX_SUCCESS)
                {

                    /* If not, just return the error code.  */
                    return(thread_ptr -> tx_thread_suspend_status);
                }

                /* Otherwise, just fall through to the checksum logic for the UDP
                   packet.  */
            }
            else
            {

                /* Restore interrupts.  */
                TX_RESTORE

                /* Set the return pointer to NULL in case it was set but released due to checksum error.  */
                *packet_ptr =   NX_NULL;

                /* Immediate return, return error completion.  */
                return(NX_NO_PACKET);
            }
        }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        /* Get the packet interface. */
#ifndef NX_DISABLE_IPV4
        if ((*packet_ptr) -> nx_packet_ip_version == NX_IP_VERSION_V4)
        {
            interface_ptr = (*packet_ptr) -> nx_packet_address.nx_packet_interface_ptr;
        }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
        if ((*packet_ptr) -> nx_packet_ip_version == NX_IP_VERSION_V6)
        {
            interface_ptr = (*packet_ptr) -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached;
        }
#endif /* FEATURE_NX_IPV6 */

        if (interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_UDP_RX_CHECKSUM)
        {
            compute_checksum = 0;
        }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#ifdef NX_IPSEC_ENABLE
        if (((*packet_ptr) -> nx_packet_ipsec_sa_ptr != NX_NULL) && (((NX_IPSEC_SA *)((*packet_ptr) -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_encryption_method != NX_CRYPTO_NONE))
        {
            compute_checksum = 1;
        }
#endif /* NX_IPSEC_ENABLE */

#if defined(NX_DISABLE_UDP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
        if (compute_checksum)
#endif /* defined(NX_DISABLE_UDP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
        {

            /* Determine if we need to compute the UDP checksum.  If it is disabled for this socket
               or if the UDP packet has a zero in the checksum field (indicating it was not computed
               by the sender, skip the checksum processing.  */
            /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
            temp_ptr =  (ULONG *)(*packet_ptr) -> nx_packet_prepend_ptr;
            if ((!socket_ptr -> nx_udp_socket_disable_checksum && (*(temp_ptr + 1) & NX_LOWER_16_MASK)) || /* per-socket checksum is not disabled, and the checksum field is not zero*/
                ((*packet_ptr) -> nx_packet_ip_version == NX_IP_VERSION_V6))                               /* It is IPv6 packet */
            {
            ULONG         *ip_src_addr = NX_NULL, *ip_dest_addr = NX_NULL;
            ULONG          checksum;
            NX_PACKET     *current_ptr = *packet_ptr;
#ifdef NX_LITTLE_ENDIAN
            NX_UDP_HEADER *udp_header_ptr;

                /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                udp_header_ptr = (NX_UDP_HEADER *)(current_ptr -> nx_packet_prepend_ptr);
#endif /* NX_LITTLE_ENDIAN */

#ifndef NX_DISABLE_IPV4
                if (current_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
                {
                NX_IPV4_HEADER *ipv4_header;

                    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                    ipv4_header = (NX_IPV4_HEADER *)(current_ptr -> nx_packet_ip_header);
                    ip_src_addr = &(ipv4_header -> nx_ip_header_source_ip);
                    ip_dest_addr = &(ipv4_header -> nx_ip_header_destination_ip);
                }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
                if (current_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6) /*  IPv6 */
                {
                NX_IPV6_HEADER *ipv6_header;

                    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                    ipv6_header = (NX_IPV6_HEADER *)(current_ptr -> nx_packet_ip_header);
                    ip_src_addr = (&ipv6_header -> nx_ip_header_source_ip[0]);
                    ip_dest_addr = (&ipv6_header -> nx_ip_header_destination_ip[0]);
                }

#endif /* FEATURE_NX_IPV6 */

#ifdef NX_LITTLE_ENDIAN
                /* Restore UDP header to network byte order */
                NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_0);
                NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);
#endif /* NX_LITTLE_ENDIAN */

                /* nx_ip_checksum_compute takes care of both even number length and odd number length */
                /* Compute the checksum of the first packet */
                checksum = _nx_ip_checksum_compute(current_ptr, NX_PROTOCOL_UDP,
                                                   (UINT)current_ptr -> nx_packet_length,
                                                   /* IPv6 src/dest address */
                                                   ip_src_addr,
                                                   ip_dest_addr);

#ifdef NX_LITTLE_ENDIAN
                /* Convert UDP header to host byte order */
                NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_0);
                NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);
#endif /* NX_LITTLE_ENDIAN */

                /* Perform the one's complement processing on the checksum.  */
                checksum =  NX_LOWER_16_MASK & ~checksum;

                /* Determine if it is valid.  */
                if (checksum == 0)
                {

                    /* The checksum is okay, so get out of the loop. */
                    break;
                }
                else
                {

#ifndef NX_DISABLE_UDP_INFO

                    /* Disable interrupts.  */
                    TX_DISABLE

                    /* Increment the UDP checksum error count.  */
                    (socket_ptr -> nx_udp_socket_ip_ptr) -> nx_ip_udp_checksum_errors++;

                    /* Increment the UDP invalid packets error count.  */
                    (socket_ptr -> nx_udp_socket_ip_ptr) -> nx_ip_udp_invalid_packets++;

                    /* Increment the UDP checksum error count for this socket.  */
                    socket_ptr -> nx_udp_socket_checksum_errors++;

                    /* Decrement the total UDP receive packets count.  */
                    (socket_ptr -> nx_udp_socket_ip_ptr) -> nx_ip_udp_packets_received--;

                    /* Decrement the total UDP receive bytes.  */
                    (socket_ptr -> nx_udp_socket_ip_ptr) -> nx_ip_udp_bytes_received -=  (*packet_ptr) -> nx_packet_length - (ULONG)sizeof(NX_UDP_HEADER);

                    /* Decrement the total UDP receive packets count.  */
                    socket_ptr -> nx_udp_socket_packets_received--;

                    /* Decrement the total UDP receive bytes.  */
                    socket_ptr -> nx_udp_socket_bytes_received -=  (*packet_ptr) -> nx_packet_length - (ULONG)sizeof(NX_UDP_HEADER);

                    /* Restore interrupts.  */
                    TX_RESTORE
#endif

                    /* Bad UDP checksum.  Release the packet. */
                    _nx_packet_release(*packet_ptr);
                }
            }
            else
            {

                /* Checksum logic is either disabled for this socket or the received
                   UDP packet checksum was not calculated - get out of the loop.  */
                break;
            }
        }
#if defined(NX_DISABLE_UDP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
        else
        {

            /* Simply break - checksum logic is conditionally disabled.  */
            break;
        }
#endif /* defined(NX_DISABLE_UDP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
    }

    /* At this point, we have a valid UDP packet for the caller.  */

    /* Remove the UDP header.  */

    /* Decrease the packet length.  */
    (*packet_ptr) -> nx_packet_length =  (*packet_ptr) -> nx_packet_length - (ULONG)sizeof(NX_UDP_HEADER);

    /* Position past the UDP header pointer.  */
    (*packet_ptr) -> nx_packet_prepend_ptr =   (*packet_ptr) -> nx_packet_prepend_ptr + sizeof(NX_UDP_HEADER);

    /* Update the trace event with the status.  */
    NX_TRACE_EVENT_UPDATE(trace_event, trace_timestamp, NX_TRACE_UDP_SOCKET_RECEIVE, 0, 0, *packet_ptr, (*packet_ptr) -> nx_packet_length);

    /* Return a successful status to the caller.  */
    return(NX_SUCCESS);
}

