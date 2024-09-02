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
#include "nx_packet.h"
#include "nx_ip.h"

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
/*    _nx_tcp_packet_process                              PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming TCP packet, which includes      */
/*    matching the packet to an existing connection and dispatching to    */
/*    the socket specific processing routine.  If no connection is        */
/*    found, this routine checks for a new connection request and if      */
/*    found, processes it accordingly. If a reset packet is received, it  */
/*    checks the queue for a previous connection request which needs to be*/
/*    removed.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_packet_release                    Packet release function       */
/*    _nx_ip_checksum_compute               Calculate TCP packet checksum */
/*    _nx_tcp_mss_option_get                Get peer MSS option           */
/*    _nx_tcp_no_connection_reset           Reset on no connection        */
/*    _nx_tcp_packet_send_syn               Send SYN message              */
/*    _nx_tcp_socket_packet_process         Socket specific packet        */
/*                                            processing routine          */
/*    (nx_tcp_listen_callback)              Application listen callback   */
/*                                            function                    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_queue_process                 Process TCP packet queue      */
/*    _nx_tcp_packet_receive                Receive packet processing     */
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
VOID  _nx_tcp_packet_process(NX_IP *ip_ptr, NX_PACKET *packet_ptr)
{

UINT                         index;
UINT                         port;
ULONG                       *source_ip = NX_NULL;
ULONG                       *dest_ip = NX_NULL;
UINT                         source_port;
NX_TCP_SOCKET               *socket_ptr;
NX_TCP_HEADER               *tcp_header_ptr;
struct NX_TCP_LISTEN_STRUCT *listen_ptr;
VOID                         (*listen_callback)(NX_TCP_SOCKET *socket_ptr, UINT port);
ULONG                        option_words;
ULONG                        mss = 0;
ULONG                        checksum;
NX_INTERFACE                *interface_ptr = NX_NULL;
#if defined(NX_DISABLE_TCP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT                         compute_checksum = 1;
#endif /* defined(NX_DISABLE_TCP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
ULONG                        queued_count;
NX_PACKET                   *queued_ptr;
NX_PACKET                   *queued_prev_ptr;
ULONG                       *queued_source_ip;
UINT                         queued_source_port;
UINT                         is_a_RST_request;
UINT                         is_valid_option_flag = NX_TRUE;
UINT                         status;
#ifdef NX_ENABLE_TCP_WINDOW_SCALING
ULONG                        rwin_scale = 0xFF;
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */

#ifdef NX_DISABLE_TCP_RX_CHECKSUM
    compute_checksum = 0;
#endif /* NX_DISABLE_TCP_RX_CHECKSUM */

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Pickup the source IP address.  */
#ifndef NX_DISABLE_IPV4
    if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
    {

    NX_IPV4_HEADER *ip_header_ptr;

        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ip_header_ptr = (NX_IPV4_HEADER *)packet_ptr -> nx_packet_ip_header;

        source_ip = &ip_header_ptr -> nx_ip_header_source_ip;

        dest_ip = &ip_header_ptr -> nx_ip_header_destination_ip;

        mss = 536;

        interface_ptr = packet_ptr -> nx_packet_address.nx_packet_interface_ptr;
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
    {

    /* IPv6 */
    NX_IPV6_HEADER *ipv6_header;

        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ipv6_header = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

        source_ip = &ipv6_header -> nx_ip_header_source_ip[0];

        dest_ip = &ipv6_header -> nx_ip_header_destination_ip[0];

        mss = 1220;

        interface_ptr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached;
    }
#endif /* FEATURE_NX_IPV6 */

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
    if (interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_TCP_RX_CHECKSUM)
    {
        compute_checksum = 0;
    }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#ifdef NX_IPSEC_ENABLE
    if ((packet_ptr -> nx_packet_ipsec_sa_ptr != NX_NULL) && (((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_encryption_method != NX_CRYPTO_NONE))
    {
        compute_checksum = 1;
    }
#endif /* NX_IPSEC_ENABLE */

#if defined(NX_DISABLE_TCP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
    if (compute_checksum)
#endif /* defined(NX_DISABLE_TCP_RX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
    {
        checksum = _nx_ip_checksum_compute(packet_ptr, NX_PROTOCOL_TCP,
                                           (UINT)packet_ptr -> nx_packet_length,
                                           source_ip, dest_ip);

        checksum = NX_LOWER_16_MASK & ~checksum;

        /* Calculate the checksum.  */
        if (checksum != 0)
        {

#ifndef NX_DISABLE_TCP_INFO

            /* Increment the TCP invalid packet error count.  */
            ip_ptr -> nx_ip_tcp_invalid_packets++;

            /* Increment the TCP packet checksum error count.  */
            ip_ptr -> nx_ip_tcp_checksum_errors++;
#endif

            /* Checksum error, just release the packet.  */
            _nx_packet_release(packet_ptr);
            return;
        }
    }

    /* Pickup the pointer to the head of the TCP packet.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    tcp_header_ptr =  (NX_TCP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

    /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
       swap the endian of the TCP header.  */
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_sequence_number);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_acknowledgment_number);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_3);
    NX_CHANGE_ULONG_ENDIAN(tcp_header_ptr -> nx_tcp_header_word_4);

    /* Determine if there are any option words...  Note there are always 5 words in a TCP header.  */
    option_words =  (tcp_header_ptr -> nx_tcp_header_word_3 >> 28) - 5;

#ifndef NX_DISABLE_RX_SIZE_CHECKING
    /* Check for valid packet length.  */
    if (((INT)option_words < 0) || (packet_ptr -> nx_packet_length < (sizeof(NX_TCP_HEADER) + (option_words << 2))))
    {

#ifndef NX_DISABLE_TCP_INFO
        /* Increment the TCP invalid packet error.  */
        ip_ptr -> nx_ip_tcp_invalid_packets++;
#endif

        /* Invalid packet length, just release it.  */
        _nx_packet_release(packet_ptr);

        /* The function is complete, just return!  */
        return;
    }
#endif

    if (option_words)
    {

        /* Yes, there are one or more option words.  */

        /* Derive the Maximum Segment Size (MSS) in the option words.  */
        status = _nx_tcp_mss_option_get((packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_TCP_HEADER)), option_words * (ULONG)sizeof(ULONG), &mss);

        /* Check the status. if status is NX_FALSE, means Option Length is invalid.  */
        if (status == NX_FALSE)
        {

            /* The option is invalid.  */
            is_valid_option_flag = NX_FALSE;
        }
        else
        {

            /* Set the default MSS if the MSS value was not found.  */
            /*lint -e{644} suppress variable might not be initialized, since "mss" was initialized in _nx_tcp_mss_option_get. */
            if (mss == 0)
            {
#ifndef NX_DISABLE_IPV4
                if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
                {
                    mss = 536;
                }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
                if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
                {
                    mss = 1220;
                }
#endif /* FEATURE_NX_IPV6 */
            }
        }

#ifdef NX_ENABLE_TCP_WINDOW_SCALING
        status = _nx_tcp_window_scaling_option_get((packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_TCP_HEADER)), option_words * (ULONG)sizeof(ULONG), &rwin_scale);

        /* Check the status. if status is NX_FALSE, means Option Length is invalid.  */
        if (status == NX_FALSE)
        {
            is_valid_option_flag = NX_FALSE;
        }
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */
    }

    /* Pickup the destination TCP port.  */
    port =  (UINT)(tcp_header_ptr -> nx_tcp_header_word_0 & NX_LOWER_16_MASK);

    /* Pickup the source TCP port.  */
    source_port =  (UINT)(tcp_header_ptr -> nx_tcp_header_word_0 >> NX_SHIFT_BY_16);

    /* Calculate the hash index in the TCP port array of the associated IP instance.  */
    index =  (UINT)((port + (port >> 8)) & NX_TCP_PORT_TABLE_MASK);

    /* Search the bound sockets in this index for the particular port.  */
    socket_ptr =  ip_ptr -> nx_ip_tcp_port_table[index];

    /* Determine if there are any sockets bound on this port index.  */
    if (socket_ptr)
    {

    INT find_a_match;

        /*  Yes, loop to examine the list of bound ports on this index.  */
        do
        {

            find_a_match = 0;

            /* Determine if the port has been found.  */
            if ((socket_ptr -> nx_tcp_socket_port == port) &&
                (socket_ptr -> nx_tcp_socket_connect_port == source_port))
            {

                /* Make sure they are the same IP protocol */
                if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == packet_ptr -> nx_packet_ip_version)
                {

#ifndef NX_DISABLE_IPV4
                    if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
                    {

                        if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v4 == *source_ip)
                        {
                            find_a_match = 1;
                        }
                    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
                    if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
                    {
                        if (CHECK_IPV6_ADDRESSES_SAME(socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v6, source_ip))
                        {
                            find_a_match = 1;
                        }
                    }
#endif /* FEATURE_NX_IPV6 */
                }

                if (find_a_match)
                {

                    /* Yes, we have a match!  */

                    /* Determine if we need to update the tcp port head pointer.  This should
                       only be done if the found socket pointer is not the head pointer and
                       the mutex for this IP instance is available.  */

                    /* Move the port head pointer to this socket.  */
                    ip_ptr -> nx_ip_tcp_port_table[index] = socket_ptr;

                    /* If this packet contains SYN */
                    if (tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_SYN_BIT)
                    {

                        /* Record the MSS value if it is present and the   Otherwise use 536, as
                           outlined in RFC 1122 section 4.2.2.6. */
                        socket_ptr -> nx_tcp_socket_peer_mss = mss;

                        if ((mss > socket_ptr -> nx_tcp_socket_mss) && socket_ptr -> nx_tcp_socket_mss)
                        {
                            socket_ptr -> nx_tcp_socket_connect_mss  = socket_ptr -> nx_tcp_socket_mss;
                        }
                        else if ((socket_ptr -> nx_tcp_socket_state != NX_TCP_SYN_SENT) ||
                                 (socket_ptr -> nx_tcp_socket_connect_mss > mss))
                        {
                            socket_ptr -> nx_tcp_socket_connect_mss  = mss;
                        }

                        /* Compute the SMSS * SMSS value, so later TCP module doesn't need to redo the multiplication. */
                        socket_ptr -> nx_tcp_socket_connect_mss2 =
                            socket_ptr -> nx_tcp_socket_connect_mss * socket_ptr -> nx_tcp_socket_connect_mss;
#ifdef NX_ENABLE_TCP_WINDOW_SCALING
                        /*
                           Simply record the peer's window scale value. When we move to the
                           ESTABLISHED state, we will set the peer window scale to 0 if the
                           peer does not support this feature.
                         */
                        socket_ptr -> nx_tcp_snd_win_scale_value = rwin_scale;
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */
                    }

                    /* Process the packet within an existing TCP connection.  */
                    _nx_tcp_socket_packet_process(socket_ptr, packet_ptr);

                    /* Get out of the search loop and this function!  */
                    return;
                }
            }

            /* Move to the next entry in the bound index.  */
            socket_ptr =  socket_ptr -> nx_tcp_socket_bound_next;
        } while (socket_ptr != ip_ptr -> nx_ip_tcp_port_table[index]);
    }

    /* At this point, we know there is not an existing TCP connection.  */

    /* If this packet contains the valid option.  */
    if (is_valid_option_flag == NX_FALSE)
    {

        /* Send RST message.
           TCP MUST be prepared to handle an illegal option length (e.g., zero) without crashing;
           a suggested procedure is to reset the connection and log the reason, outlined in RFC 1122, Section 4.2.2.5, Page85. */
        _nx_tcp_no_connection_reset(ip_ptr, packet_ptr, tcp_header_ptr);

#ifndef NX_DISABLE_TCP_INFO
        /* Increment the TCP invalid packet error count.  */
        ip_ptr -> nx_ip_tcp_invalid_packets++;
#endif /* NX_DISABLE_TCP_INFO */

        /* Not a connection request, just release the packet.  */
        _nx_packet_release(packet_ptr);

        return;
    }

#ifdef NX_ENABLE_TCP_MSS_CHECK
    /* Optionally check for a user specified minimum MSS. The user application may choose to
       define a minimum MSS value, and reject a TCP connection if peer MSS value does not
       meet the minimum. */
    if (mss < NX_TCP_MSS_MINIMUM)
    {

        /* Send RST message.  */
        _nx_tcp_no_connection_reset(ip_ptr, packet_ptr, tcp_header_ptr);

#ifndef NX_DISABLE_TCP_INFO
        /* Increment the TCP invalid packet error count.  */
        ip_ptr -> nx_ip_tcp_invalid_packets++;
#endif /* NX_DISABLE_TCP_INFO */

        /* Handle this as an invalid connection request. */
        _nx_packet_release(packet_ptr);

        return;
    }
#endif

    /* Handle new connection requests without ACK bit in NX_TCP_SYN_RECEIVED state.
       NX_TCP_SYN_RECEIVED state is equal of LISTEN state of RFC.
       RFC793, Section3.9, Page65. */
    if ((!(tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_ACK_BIT)) &&
        (ip_ptr -> nx_ip_tcp_active_listen_requests))
    {

#ifndef NX_DISABLE_IPV4
        if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
        {

            /* Check for LAND attack packet. This is an incoming packet with matching
               Source and Destination IP address, and matching source and destination port. */
            if ((*source_ip == *dest_ip) && (source_port == port))
            {

                /* Bogus packet. Drop it! */

#ifndef NX_DISABLE_TCP_INFO
                /* Increment the TCP invalid packet error count.  */
                ip_ptr -> nx_ip_tcp_invalid_packets++;
#endif /* NX_DISABLE_TCP_INFO */

                /* Release the packet we will not process any further.  */
                _nx_packet_release(packet_ptr);
                return;
            }

            /* It shall not make connections if the source IP address
               is broadcast or multicast.   */
            if (
                /* Check for Multicast address */
                ((*source_ip & NX_IP_CLASS_D_MASK) == NX_IP_CLASS_D_TYPE) ||
                /* Check for subnet-directed broadcast */
                (((*source_ip & interface_ptr -> nx_interface_ip_network_mask) == interface_ptr -> nx_interface_ip_network) &&
                 ((*source_ip & ~(interface_ptr -> nx_interface_ip_network_mask)) == ~(interface_ptr -> nx_interface_ip_network_mask))) ||
                /* Check for local subnet address */
                (*source_ip == interface_ptr -> nx_interface_ip_network)  ||
                /* Check for limited broadcast */
                (*source_ip == NX_IP_LIMITED_BROADCAST)
               )
            {

#ifndef NX_DISABLE_TCP_INFO
                /* Increment the TCP invalid packet error count.  */
                ip_ptr -> nx_ip_tcp_invalid_packets++;
#endif /* NX_DISABLE_TCP_INFO */

                /* Release the packet.  */
                _nx_packet_release(packet_ptr);

                /* Finished processing, simply return!  */
                return;
            }
        }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
        if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
        {

            /* Check for LAND attack packet. This is an incoming packet with matching
               Source and Destination IP address, and matching source and destination port. */
            if ((CHECK_IPV6_ADDRESSES_SAME(source_ip, dest_ip)) && (source_port == port))
            {

                /* Bogus packet. Drop it! */

#ifndef NX_DISABLE_TCP_INFO
                /* Increment the TCP invalid packet error count.  */
                ip_ptr -> nx_ip_tcp_invalid_packets++;
#endif /* NX_DISABLE_TCP_INFO */

                /* Release the packet we will not process any further.  */
                _nx_packet_release(packet_ptr);
                return;
            }

            /* It shall not make connections if the source IP address
               is broadcast or multicast.   */
            if (IPv6_Address_Type(source_ip) & IPV6_ADDRESS_MULTICAST)
            {

#ifndef NX_DISABLE_TCP_INFO
                /* Increment the TCP invalid packet error count.  */
                ip_ptr -> nx_ip_tcp_invalid_packets++;
#endif /* NX_DISABLE_TCP_INFO */

                /* Release the packet.  */
                _nx_packet_release(packet_ptr);

                /* Finished processing, simply return!  */
                return;
            }
        }
#endif /* FEATURE_NX_IPV6*/

        /* Search all ports in listen mode for a match. */
        listen_ptr =  ip_ptr -> nx_ip_tcp_active_listen_requests;
        do
        {

            /* Determine if this port is in a listen mode.  */
            if (listen_ptr -> nx_tcp_listen_port == port)
            {

                /* Determine if the packet is an initial connection request.
                   The incoming SYN packet is a connection request.
                   The incoming RST packet is related to a previous connection request.
                   Fourth other text or control. RFC793, Section3.9, Page66. */
                if ((!(tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_SYN_BIT)) &&
                    (!(tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_RST_BIT)))
                {

#ifndef NX_DISABLE_TCP_INFO
                    /* This is a duplicate connection request. Increment the TCP dropped packet count.  */
                    ip_ptr -> nx_ip_tcp_receive_packets_dropped++;
#endif /* NX_DISABLE_TCP_INFO */

                    /* Release the packet.  */
                    _nx_packet_release(packet_ptr);

                    return;
                }

#ifndef NX_DISABLE_TCP_INFO

                /* Check for a SYN bit set.  */
                if ((tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_SYN_BIT))
                {

                    /* Increment the passive TCP connections count.  */
                    ip_ptr -> nx_ip_tcp_passive_connections++;

                    /* Increment the TCP connections count.  */
                    ip_ptr -> nx_ip_tcp_connections++;
                }
#endif

                /* Okay, this port is in a listen mode.  We now need to see if
                   there is an available socket for the new connection request
                   present.  */
                if ((listen_ptr -> nx_tcp_listen_socket_ptr) &&
                    ((tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_RST_BIT) == NX_NULL))
                {

                    /* Yes there is indeed a socket present.  We now need to
                       fill in the appropriate info and call the server callback
                       routine.  */

                    /* Allocate the supplied server socket.  */
                    socket_ptr = listen_ptr -> nx_tcp_listen_socket_ptr;

#ifndef NX_DISABLE_EXTENDED_NOTIFY_SUPPORT
                    /* If extended notify is enabled, call the syn_received notify function.
                       This user-supplied function decides whether or not this SYN request
                       should be accepted. */
                    if (socket_ptr -> nx_tcp_socket_syn_received_notify)
                    {

                        /* Add debug information. */
                        NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

                        if ((socket_ptr -> nx_tcp_socket_syn_received_notify)(socket_ptr, packet_ptr) != NX_TRUE)
                        {

                            /* Release the packet.  */
                            _nx_packet_release(packet_ptr);

                            /* Finished processing, simply return!  */
                            return;
                        }
                    }
#endif /* NX_DISABLE_EXTENDED_NOTIFY_SUPPORT */

                    /* If trace is enabled, insert this event into the trace buffer.  */
                    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_SYN_RECEIVE, ip_ptr, socket_ptr, packet_ptr, tcp_header_ptr -> nx_tcp_sequence_number, NX_TRACE_INTERNAL_EVENTS, 0, 0);

                    /* Clear the server socket pointer in the listen request.  If the
                       application wishes to honor more server connections on this port,
                       the application must call relisten with a new server socket
                       pointer.  */
                    listen_ptr -> nx_tcp_listen_socket_ptr =  NX_NULL;

                    /* Fill the socket in with the appropriate information.  */


#ifndef NX_DISABLE_IPV4
                    if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
                    {

                        /* Assume the interface that receives the incoming packet is the best interface
                           for sending responses. */
                        socket_ptr -> nx_tcp_socket_connect_interface = interface_ptr;
                        socket_ptr -> nx_tcp_socket_next_hop_address = NX_NULL;

                        /* Set the next hop address.  */
                        _nx_ip_route_find(ip_ptr, *source_ip, &socket_ptr -> nx_tcp_socket_connect_interface,
                                          &socket_ptr -> nx_tcp_socket_next_hop_address);

                        socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version =  NX_IP_VERSION_V4;
                        socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v4 = *source_ip;
                    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
                    if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
                    {

                        socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version = NX_IP_VERSION_V6;
                        COPY_IPV6_ADDRESS(source_ip, socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v6);

                        /* Also record the outgoing interface information. */
                        socket_ptr -> nx_tcp_socket_ipv6_addr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr;
                        socket_ptr -> nx_tcp_socket_connect_interface = interface_ptr;
                    }
#endif /* FEATURE_NX_IPV6 */

                    socket_ptr -> nx_tcp_socket_connect_port = source_port;
                    socket_ptr -> nx_tcp_socket_rx_sequence =  tcp_header_ptr -> nx_tcp_sequence_number;


                    /* Yes, MSS was found, so store it!  */
                    socket_ptr -> nx_tcp_socket_peer_mss = mss;

#ifdef NX_ENABLE_TCP_WINDOW_SCALING
                    /*
                       Simply record the peer's window scale value. When we move to the
                       ESTABLISHED state, we will set the peer window scale to 0 if the
                       peer does not support this feature.
                     */
                    socket_ptr -> nx_tcp_snd_win_scale_value = rwin_scale;
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */

                    /* Set the initial slow start threshold to be the advertised window size. */
                    socket_ptr -> nx_tcp_socket_tx_slow_start_threshold = socket_ptr -> nx_tcp_socket_tx_window_advertised;

                    /* Slow start:  setup initial window (IW) to be MSS,  RFC 2581, 3.1 */
                    socket_ptr -> nx_tcp_socket_tx_window_congestion = mss;

                    /* Initialize the transmit outstanding byte count to zero. */
                    socket_ptr -> nx_tcp_socket_tx_outstanding_bytes = 0;

                    /* Calculate the hash index in the TCP port array of the associated IP instance.  */
                    index = (UINT)((port + (port >> 8)) & NX_TCP_PORT_TABLE_MASK);

                    /* Determine if the list is NULL.  */
                    if (ip_ptr -> nx_ip_tcp_port_table[index])
                    {

                        /* There are already sockets on this list... just add this one
                           to the end.  */
                        socket_ptr -> nx_tcp_socket_bound_next =
                            ip_ptr -> nx_ip_tcp_port_table[index];
                        socket_ptr -> nx_tcp_socket_bound_previous =
                            (ip_ptr -> nx_ip_tcp_port_table[index]) -> nx_tcp_socket_bound_previous;
                        ((ip_ptr -> nx_ip_tcp_port_table[index]) -> nx_tcp_socket_bound_previous) -> nx_tcp_socket_bound_next =
                            socket_ptr;
                        (ip_ptr -> nx_ip_tcp_port_table[index]) -> nx_tcp_socket_bound_previous = socket_ptr;
                    }
                    else
                    {

                        /* Nothing is on the TCP port list.  Add this TCP socket to an
                           empty list.  */
                        socket_ptr -> nx_tcp_socket_bound_next =      socket_ptr;
                        socket_ptr -> nx_tcp_socket_bound_previous =  socket_ptr;
                        ip_ptr -> nx_ip_tcp_port_table[index] =       socket_ptr;
                    }

                    /* Pickup the listen callback function.  */
                    listen_callback = listen_ptr -> nx_tcp_listen_callback;

                    /* Release the incoming packet.  */
                    _nx_packet_release(packet_ptr);

                    /* Determine if an accept call with suspension has already been made
                       for this socket.  If so, the SYN message needs to be sent from
                       here.  */
                    if (socket_ptr -> nx_tcp_socket_state == NX_TCP_SYN_RECEIVED)
                    {


                        /* If trace is enabled, insert this event into the trace buffer.  */
                        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, socket_ptr -> nx_tcp_socket_state, NX_TRACE_INTERNAL_EVENTS, 0, 0);


                        /* The application is suspended on an accept call for this socket.
                           Simply send the SYN now and keep the thread suspended until the
                           other side completes the connection.  */

                        /* Send the SYN message, but increment the ACK first.  */
                        socket_ptr -> nx_tcp_socket_rx_sequence++;

                        /* Increment the sequence number for the SYN message.  */
                        socket_ptr -> nx_tcp_socket_tx_sequence++;

                        /* Setup a timeout so the connection attempt can be sent again.  */
                        socket_ptr -> nx_tcp_socket_timeout =          socket_ptr -> nx_tcp_socket_timeout_rate;
                        socket_ptr -> nx_tcp_socket_timeout_retries =  0;

                        /* Send the SYN+ACK message.  */
                        _nx_tcp_packet_send_syn(socket_ptr, (socket_ptr -> nx_tcp_socket_tx_sequence - 1));
                    }

                    /* Determine if there is a listen callback function.  */
                    if (listen_callback)
                    {
                        /* Call the user's listen callback function.  */
                        (listen_callback)(socket_ptr, port);
                    }
                }
                else
                {

                    /* There is no server socket available for the new connection.  */

                    /* The application needs to call relisten with a new server request to process this queued
                       connection.  */

                    /* Check for a RST (reset) bit set.  */
                    if (!(tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_RST_BIT))
                    {

                        /* If trace is enabled, insert this event into the trace buffer.  */
                        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_SYN_RECEIVE, ip_ptr, NX_NULL, packet_ptr, tcp_header_ptr -> nx_tcp_sequence_number, NX_TRACE_INTERNAL_EVENTS, 0, 0);
                    }

                    /* Check for the same connection request already in the queue.  */
                    queued_count = listen_ptr -> nx_tcp_listen_queue_current;
                    queued_ptr = listen_ptr -> nx_tcp_listen_queue_head;
                    queued_prev_ptr = queued_ptr;

                    /* Initialize the check for queued request to false.*/
                    is_a_RST_request = NX_FALSE;

                    /* Loop through the queued list in order to search for duplicate request.  */
                    while (queued_count--)
                    {

                        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                        queued_source_port = (UINT)(*((ULONG *)queued_ptr -> nx_packet_prepend_ptr) >> NX_SHIFT_BY_16);

#ifndef NX_DISABLE_IPV4
                        /* Pickup the queued source port and source IP address for comparison.  */
                        if (queued_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
                        {

                            /*lint -e{929} -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                            queued_source_ip = (ULONG *)(((ULONG *)queued_ptr -> nx_packet_prepend_ptr) - 2);

                            /* Determine if this matches the current connection request.  */
                            if ((*queued_source_ip == *source_ip) && (queued_source_port == source_port))
                            {

                                /* Possible duplicate connection request to one that is already queued.  */

                                /* Check for a RST (reset) bit set.  */
                                if (tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_RST_BIT)
                                {

                                    /* RST packet matches a previously queued connection request. */
                                    is_a_RST_request = NX_TRUE;
                                }
                                else
                                {
#ifndef NX_DISABLE_TCP_INFO
                                    /* This is a duplicate connection request. Increment the TCP dropped packet count.  */
                                    ip_ptr -> nx_ip_tcp_receive_packets_dropped++;
#endif
                                    /* Simply release the packet and return.  */
                                    _nx_packet_release(packet_ptr);

                                    /* Return!  */
                                    return;
                                }
                            }
                        }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
                        if (queued_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
                        {

                            /*lint -e{929} -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
                            queued_source_ip = (ULONG *)(((ULONG *)queued_ptr -> nx_packet_prepend_ptr) - 8);

                            /* Determine if this matches the current connection request.  */
                            if ((CHECK_IPV6_ADDRESSES_SAME(queued_source_ip, source_ip)) && (queued_source_port == source_port))
                            {

                                /* Possible duplicate connection request to one that is already queued.  */

                                /* Check for a RST (reset) bit set.  */
                                if (tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_RST_BIT)
                                {

                                    /* RST packet matches a previously queued connection request. */
                                    is_a_RST_request = NX_TRUE;
                                }
                                else
                                {
#ifndef NX_DISABLE_TCP_INFO
                                    /* This is a duplicate connection request. Increment the TCP dropped packet count.  */
                                    ip_ptr -> nx_ip_tcp_receive_packets_dropped++;
#endif
                                    /* Simply release the packet and return.  */
                                    _nx_packet_release(packet_ptr);

                                    /* Return!  */
                                    return;
                                }
                            }
                        }
#endif /* FEATURE_NX_IPV6  */

                        /* Handle the case of the RST packet which cancels a previously received
                           connection request. */
                        if (is_a_RST_request)
                        {

                            /* A previous connection request needs to be removed from the listen queue. */
                            if (queued_ptr == listen_ptr -> nx_tcp_listen_queue_head)
                            {

                                /* Reset the front (oldest) of the queue to the next request. */
                                listen_ptr -> nx_tcp_listen_queue_head = queued_ptr -> nx_packet_queue_next;
                            }
                            else
                            {

                                /* Link around the request we are removing. */
                                /*lint -e{613} suppress possible use of null pointer, since 'queued_prev_ptr' must not be NULL.  */
                                queued_prev_ptr -> nx_packet_queue_next = queued_ptr -> nx_packet_queue_next;
                            }

                            /* Is the request being removed the tail (most recent connection?)   */
                            if (queued_ptr == listen_ptr -> nx_tcp_listen_queue_tail)
                            {

                                /* Yes, set the previous connection request as the tail. */
                                listen_ptr -> nx_tcp_listen_queue_tail = queued_prev_ptr;
                            }

                            /* Release the connection request packet.  */
                            _nx_packet_release(queued_ptr);

                            /* Update the listen queue. */
                            listen_ptr -> nx_tcp_listen_queue_current--;

#ifndef NX_DISABLE_TCP_INFO
                            /* Increment the TCP dropped packet count.  */
                            ip_ptr -> nx_ip_tcp_receive_packets_dropped++;
#endif

                            /* Simply release the packet and return.  */
                            _nx_packet_release(packet_ptr);

                            /* Return!  */
                            return;
                        }

                        /* Move to next item in the queue.  */
                        queued_prev_ptr = queued_ptr;
                        queued_ptr = queued_ptr -> nx_packet_queue_next;
                    }

                    /* Not a duplicate connection request, place this request on the listen queue.  */

                    /* Is this a RST packet? */
                    if (tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_RST_BIT)
                    {

                        /* Yes, so not a connection request. Do not place on the listen queue. */
#ifndef NX_DISABLE_TCP_INFO
                        /* Increment the TCP dropped packet count.  */
                        ip_ptr -> nx_ip_tcp_receive_packets_dropped++;
#endif

                        /* Release the packet.  */
                        _nx_packet_release(packet_ptr);

                        /* Return!  */
                        return;
                    }

                    /* Set the next pointer of the packet to NULL.  */
                    packet_ptr -> nx_packet_queue_next = NX_NULL;

                    /* Queue the new connection request.  */
                    if (listen_ptr -> nx_tcp_listen_queue_head)
                    {

                        /* There is a connection request already queued, just link packet to tail.  */
                        (listen_ptr -> nx_tcp_listen_queue_tail) -> nx_packet_queue_next = packet_ptr;
                    }
                    else
                    {

                        /* The queue is empty.  Setup head pointer to the new packet.  */
                        listen_ptr -> nx_tcp_listen_queue_head = packet_ptr;
                    }

                    /* Setup the tail pointer to the new packet and increment the queue count.  */
                    listen_ptr -> nx_tcp_listen_queue_tail =  packet_ptr;
                    listen_ptr -> nx_tcp_listen_queue_current++;

                    /* Add debug information. */
                    NX_PACKET_DEBUG(NX_PACKET_TCP_LISTEN_QUEUE, __LINE__, packet_ptr);

                    /* Determine if the queue depth has been exceeded.  */
                    if (listen_ptr -> nx_tcp_listen_queue_current > listen_ptr -> nx_tcp_listen_queue_maximum)
                    {

#ifndef NX_DISABLE_TCP_INFO

                        /* Increment the TCP connections dropped count.  */
                        ip_ptr -> nx_ip_tcp_connections_dropped++;
                        ip_ptr -> nx_ip_tcp_connections--;

                        /* Increment the TCP dropped packet count.  */
                        ip_ptr -> nx_ip_tcp_receive_packets_dropped++;
#endif

                        /* Save the head packet pointer, since this will be released below.  */
                        packet_ptr = listen_ptr -> nx_tcp_listen_queue_head;

                        /* Remove the oldest packet from the queue.  */
                        listen_ptr -> nx_tcp_listen_queue_head = (listen_ptr -> nx_tcp_listen_queue_head) -> nx_packet_queue_next;

                        /* Decrement the number of packets in the queue.  */
                        listen_ptr -> nx_tcp_listen_queue_current--;

                        /* We have exceeded the number of connections that can be
                           queued for this port.  */

                        /* Release the packet.  */
                        _nx_packet_release(packet_ptr);
                    }
                }

                /* Finished processing, just return.  */
                return;
            }

            /* Move to the next listen request.  */
            listen_ptr = listen_ptr -> nx_tcp_listen_next;
        } while (listen_ptr != ip_ptr -> nx_ip_tcp_active_listen_requests);
    }

#ifndef NX_DISABLE_TCP_INFO

    /* Determine if a connection request is present.  */
    if (tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_SYN_BIT)
    {

        /* Yes, increment the TCP connections dropped count.  */
        ip_ptr -> nx_ip_tcp_connections_dropped++;
    }

    /* Increment the TCP dropped packet count.  */
    ip_ptr -> nx_ip_tcp_receive_packets_dropped++;
#endif /* NX_DISABLE_TCP_INFO  */

    /* Determine if a RST is present. If so, don't send a RST in response.  */
    if (!(tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_RST_BIT))
    {

        /* Non RST is present, send reset when no connection is present.  */
        _nx_tcp_no_connection_reset(ip_ptr, packet_ptr, tcp_header_ptr);
    }

    /* Not a connection request, just release the packet.  */
    _nx_packet_release(packet_ptr);

    return;
}

