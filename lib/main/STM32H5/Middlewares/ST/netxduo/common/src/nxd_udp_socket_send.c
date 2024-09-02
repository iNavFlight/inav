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
#include "nx_udp.h"
#include "nx_ip.h"
#include "nx_packet.h"
#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif /* FEATURE_NX_IPV6 */

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */


#ifdef NX_ENABLE_TCPIP_OFFLOAD
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_udp_socket_driver_send                          PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a UDP packet through TCP/IP offload interface.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to UDP socket         */
/*    packet_ptr                            Pointer to UDP packet         */
/*    ip_src_address                        Source IP address             */
/*    ip_dst_address                        Destination IP address        */
/*    port                                  16-bit UDP port number        */
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
/*    _nxd_udp_socket_send                                                */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  08-02-2021     Yuxin Zhou               Initial Version 6.1.8         */
/*  01-31-2022     Yuxin Zhou               Modified comment(s), corrected*/
/*                                            the logic for queued packet,*/
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
static UINT _nx_udp_socket_driver_send(NX_UDP_SOCKET *socket_ptr,
                                       NX_PACKET     *packet_ptr,
                                       NXD_ADDRESS   *ip_src_address,
                                       NXD_ADDRESS   *ip_dst_address,
                                       UINT           port)
{
UINT            status;
NX_IP          *ip_ptr;
NX_INTERFACE   *interface_ptr = NX_NULL;
UCHAR          *original_ptr = packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_UDP_HEADER);
ULONG           original_length = packet_ptr -> nx_packet_length - sizeof(NX_UDP_HEADER);
UINT            packet_reset = NX_FALSE;

    /* Set up the pointer to the associated IP instance.  */
    ip_ptr =  socket_ptr -> nx_udp_socket_ip_ptr;

    /* Get the outgoing interface. */
#ifndef NX_DISABLE_IPV4
    if (ip_dst_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        interface_ptr = packet_ptr -> nx_packet_address.nx_packet_interface_ptr;
    }
#endif /* NX_DISABLE_IPV4 */
#ifdef FEATURE_NX_IPV6
    if (ip_dst_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        interface_ptr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached;
    }
#endif /* FEATURE_NX_IPV6 */

#ifdef NX_ENABLE_IP_PACKET_FILTER
    /* Check if the IP packet filter is set. */
    if (ip_ptr -> nx_ip_packet_filter || ip_ptr -> nx_ip_packet_filter_extended)
    {

        /* Add the IP Header to trigger filtering.  */
#ifndef NX_DISABLE_IPV4
        if (ip_dst_address -> nxd_ip_version == NX_IP_VERSION_V4)
        {
            _nx_ip_header_add(ip_ptr, packet_ptr,
                              ip_src_address -> nxd_ip_address.v4,
                              ip_dst_address -> nxd_ip_address.v4,
                              socket_ptr -> nx_udp_socket_type_of_service,
                              socket_ptr -> nx_udp_socket_time_to_live,
                              NX_IP_UDP, socket_ptr -> nx_udp_socket_fragment_enable);
        }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
        if (ip_dst_address -> nxd_ip_version == NX_IP_VERSION_V6)
        {
            if (_nx_ipv6_header_add(ip_ptr, &packet_ptr, NX_PROTOCOL_UDP,
                                    packet_ptr -> nx_packet_length,
                                    ip_ptr -> nx_ipv6_hop_limit,
                                    ip_src_address -> nxd_ip_address.v6,
                                    ip_dst_address -> nxd_ip_address.v6, NX_NULL))
            {

                /* Packet consumed by IPv6 layer. Just return success.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));
                return(NX_SUCCESS);
            }

            /* Reset IP header.  */
            packet_ptr -> nx_packet_prepend_ptr = packet_ptr -> nx_packet_prepend_ptr + sizeof(NX_IPV6_HEADER);
            packet_ptr -> nx_packet_length = packet_ptr -> nx_packet_length - sizeof(NX_IPV6_HEADER);
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
    }
#endif /* NX_ENABLE_IP_PACKET_FILTER */

    /* Reset UDP and IP header.  */
    packet_ptr -> nx_packet_prepend_ptr = original_ptr;
    packet_ptr -> nx_packet_length = original_length;

    /* Determine if the packet is a queued data packet. _nx_packet_transmit_release in Offload handler
       does not release the packet immediately and only adjusts the prepend pointer to User data,
       since the packet may need to be resent. To keep the same logic for retransmission in upper layer,
       the prepend pointer must be reset to UDP header.  */
    if ((packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next != ((NX_PACKET*)NX_PACKET_ALLOCATED)) &&
        (packet_ptr -> nx_packet_union_next.nx_packet_tcp_queue_next != ((NX_PACKET*)NX_PACKET_FREE)))
    {
        packet_reset = NX_TRUE;
    }

    /* Let TCP/IP offload interface send the packet.  */
    status = interface_ptr -> nx_interface_tcpip_offload_handler(ip_ptr, interface_ptr, socket_ptr,
                                                                 NX_TCPIP_OFFLOAD_UDP_SOCKET_SEND,
                                                                 packet_ptr, ip_src_address, ip_dst_address,
                                                                 socket_ptr -> nx_udp_socket_port,
                                                                 &port, NX_NO_WAIT);

    /* Release the IP protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    if (status)
    {
        return(NX_TCPIP_OFFLOAD_ERROR);
    }
    else
    {

        /* Reset prepend pointer to UDP header for queued packet.  */
        if (packet_reset == NX_TRUE)
        {
            packet_ptr -> nx_packet_prepend_ptr = original_ptr - sizeof(NX_UDP_HEADER);
            packet_ptr -> nx_packet_length = original_length + sizeof(NX_UDP_HEADER);
        }

        return(NX_SUCCESS);
    }
}
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxd_udp_socket_send                                PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a UDP packet through the specified socket       */
/*    with the input IP address and port.                                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to UDP socket         */
/*    packet_ptr                            Pointer to UDP packet         */
/*    ip_address                            IP address                    */
/*    port                                  16-bit UDP port number        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*    NX_IPSEC_REJECTED                     Failed IPSec check            */
/*    NX_NOT_BOUND                          Socket not bound to a port    */
/*    NX_NO_INTERFACE_ADDRESS               Socket interface not marked   */
/*                                             valid                      */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_packet_send                    Send UDP packet over IPv4     */
/*    _nx_ipv6_packet_send                  Send UDP packet over IPv6     */
/*    nx_ip_checksum_compute                Compute UDP header checksum   */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    _nxd_ipv6_interface_find              Find interface for input      */
/*                                             address in IP address table*/
/*    [_nx_packet_egress_sa_lookup]         IPsec process                 */
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
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported TCP/IP offload,   */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
UINT  _nxd_udp_socket_send(NX_UDP_SOCKET *socket_ptr,
                           NX_PACKET     *packet_ptr,
                           NXD_ADDRESS   *ip_address,
                           UINT           port)
{
TX_INTERRUPT_SAVE_AREA

NX_IP         *ip_ptr;
NX_UDP_HEADER *udp_header_ptr;
ULONG         *ip_src_addr = NX_NULL, *ip_dest_addr = NX_NULL;
#ifndef NX_DISABLE_IPV4
ULONG          next_hop_address = 0;
#endif /* !NX_DISABLE_IPV4  */
#if !defined(NX_DISABLE_IPV4) || (defined(FEATURE_NX_IPV6) && defined(NX_ENABLE_INTERFACE_CAPABILITY))
NX_INTERFACE  *interface_ptr = NX_NULL;
#endif /* !NX_DISABLE_IPV4 || (FEATURE_NX_IPV6 && NX_ENABLE_INTERFACE_CAPABILITY) */
#ifdef FEATURE_NX_IPV6
UINT           status;
#endif /* FEATURE_NX_IPV6 */

#ifdef NX_ENABLE_TCPIP_OFFLOAD
NXD_ADDRESS    ip_src_address;
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

#ifdef NX_IPSEC_ENABLE
VOID          *sa = NX_NULL;
ULONG          data_offset;
NXD_ADDRESS    src_addr;
UINT           ret;
#endif /* NX_IPSEC_ENABLE */

#ifdef TX_ENABLE_EVENT_TRACE
UINT           ip_address_log = 0;
#endif /* TX_ENABLE_EVENT_TRACE */

#if defined(NX_DISABLE_UDP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
UINT           compute_checksum = 1;
#endif /* defined(NX_DISABLE_UDP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */

#ifdef NX_DISABLE_UDP_TX_CHECKSUM
    /* Disable UDP TX checksum. */
    compute_checksum = 0;
#endif /* NX_DISABLE_UDP_TX_CHECKSUM */

    /* Lockout interrupts.  */
    TX_DISABLE

    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

    /* Determine if the socket is currently bound to a UDP port.  */
    if (!socket_ptr ->  nx_udp_socket_bound_next)
    {

        /* Restore interrupts.  */
        TX_RESTORE

        /* Socket is not bound, return an error message.  */
        return(NX_NOT_BOUND);
    }

    /* Pickup the important information from the socket.  */

    /* Set up the pointer to the associated IP instance.  */
    ip_ptr =  socket_ptr -> nx_udp_socket_ip_ptr;

#ifdef TX_ENABLE_EVENT_TRACE

#ifndef NX_DISABLE_IPV4
    /* For IPv4 packets, log the whole IP address. */
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {
        ip_address_log = ip_address -> nxd_ip_address.v4;
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6

    /* For IPv6 packets, log the least significant 32-bit of the IP address. */
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {
        ip_address_log = ip_address -> nxd_ip_address.v6[3];
    }
#endif /* FEATURE_NX_IPV6  */

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_UDP_SOCKET_SEND, socket_ptr, packet_ptr, packet_ptr -> nx_packet_length, ip_address_log, NX_TRACE_UDP_EVENTS, 0, 0);

#endif /* TX_ENABLE_EVENT_TRACE */

    /* Restore interrupts.  */
    TX_RESTORE

#ifndef NX_DISABLE_IPV4
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {

        /* Look for a suitable interface. */
        _nx_ip_route_find(ip_ptr, ip_address -> nxd_ip_address.v4, &packet_ptr -> nx_packet_address.nx_packet_interface_ptr,
                          &next_hop_address);

        /* Check the packet interface.  */
        if (!packet_ptr -> nx_packet_address.nx_packet_interface_ptr)
        {

            /* None found; return the error status. */
            return(NX_IP_ADDRESS_ERROR);
        }

        interface_ptr = packet_ptr -> nx_packet_address.nx_packet_interface_ptr;

        /* Fill in the IP src/dest address */
        ip_dest_addr = &ip_address -> nxd_ip_address.v4;
        ip_src_addr = &interface_ptr -> nx_interface_ip_address;

#ifdef NX_ENABLE_TCPIP_OFFLOAD
        ip_src_address.nxd_ip_version = NX_IP_VERSION_V4;
        ip_src_address.nxd_ip_address.v4 = interface_ptr -> nx_interface_ip_address;
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
    }
#endif /* NX_DISABLE_IPV4 */

#ifdef FEATURE_NX_IPV6
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        if (packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr == NX_NULL)
        {

            /* Determine if the IP instance has a matching address for the packet destination. */
            status = _nxd_ipv6_interface_find(ip_ptr, ip_address -> nxd_ip_address.v6,
                                              &packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr,
                                              NX_NULL);

            /* If not, return the error status. */
            if (status != NX_SUCCESS)
            {
                return(status);
            }
        }

        /* Fill in the IP src/dest address */
        ip_dest_addr = &ip_address -> nxd_ip_address.v6[0];
        ip_src_addr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address;

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        /* Get the packet interface information. */
        interface_ptr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached;
#endif /* NX_ENABLE_INTERFACE_CAPABILITY  */

#ifdef NX_ENABLE_TCPIP_OFFLOAD
        ip_src_address.nxd_ip_version = NX_IP_VERSION_V6;
        COPY_IPV6_ADDRESS(ip_src_addr, ip_src_address.nxd_ip_address.v6);
#endif /* NX_ENABLE_TCPIP_OFFLOAD */
    }
#endif /* FEATURE_NX_IPV6 */

#ifdef NX_IPSEC_ENABLE
#ifndef NX_DISABLE_IPV4
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {

        /* Copy IP version and address for internal IPSec (SA) processing. */
        src_addr.nxd_ip_version = NX_IP_VERSION_V4;
        src_addr.nxd_ip_address.v4 = interface_ptr -> nx_interface_ip_address;
    }
#endif /* NX_DISABLE_IPV4 */

#ifdef FEATURE_NX_IPV6
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        /* Handle for IPv6 packets. */
        src_addr.nxd_ip_version = NX_IP_VERSION_V6;
        COPY_IPV6_ADDRESS(packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address, src_addr.nxd_ip_address.v6);
    }
#endif /* FEATURE_NX_IPV6 */

    /* Check for possible SA match. */
    if (ip_ptr -> nx_ip_packet_egress_sa_lookup != NX_NULL)               /* IPsec is enabled. */
    {
        /* If the SA has not been set. */
        ret = ip_ptr -> nx_ip_packet_egress_sa_lookup(ip_ptr,                                    /* IP ptr */
                                                      &src_addr,                                 /* src_addr */
                                                      ip_address,                                /* dest_addr */
                                                      NX_PROTOCOL_UDP,                           /* protocol */
                                                      socket_ptr -> nx_udp_socket_port,          /* src_port */
                                                      port,                                      /* dest_port */
                                                      &data_offset, &sa, 0);
        if (ret == NX_IPSEC_TRAFFIC_PROTECT)
        {

            /* Save the SA to the packet. */
            packet_ptr -> nx_packet_ipsec_sa_ptr = sa;
        }
        else if (ret == NX_IPSEC_TRAFFIC_DROP || ret == NX_IPSEC_TRAFFIC_PENDING_IKEV2)
        {
            return(NX_IPSEC_REJECTED);
        }
        else
        {
            /* Zero out sa information. */
            packet_ptr -> nx_packet_ipsec_sa_ptr = NX_NULL;
        }
    }
#endif /* NX_IPSEC_ENABLE */
        
    /* Prepend the UDP header to the packet.  First, make room for the UDP header.  */
    packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_UDP_HEADER);

    /* Set the correct IP version. */
    packet_ptr -> nx_packet_ip_version = (UCHAR)(ip_address -> nxd_ip_version);

#ifndef NX_DISABLE_UDP_INFO
    /* Increment the total UDP packets sent count.  */
    ip_ptr -> nx_ip_udp_packets_sent++;

    /* Increment the total UDP bytes sent.  */
    ip_ptr -> nx_ip_udp_bytes_sent +=  packet_ptr -> nx_packet_length;

    /* Increment the total UDP packets sent count for this socket.  */
    socket_ptr -> nx_udp_socket_packets_sent++;

    /* Increment the total UDP bytes sent for this socket.  */
    socket_ptr -> nx_udp_socket_bytes_sent +=  packet_ptr -> nx_packet_length;
#endif

    /* Increase the packet length.  */
    packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length + (ULONG)sizeof(NX_UDP_HEADER);

    /* Setup the UDP header pointer.  */
    /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
    udp_header_ptr =  (NX_UDP_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

    /* Build the first 32-bit word of the UDP header.  */
    udp_header_ptr -> nx_udp_header_word_0 =
        (((ULONG)socket_ptr -> nx_udp_socket_port) << NX_SHIFT_BY_16) | (ULONG)port;

    /* Build the second 32-bit word of the UDP header.  */
    udp_header_ptr -> nx_udp_header_word_1 =  (packet_ptr -> nx_packet_length << NX_SHIFT_BY_16);

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_UDP_SEND, ip_ptr, socket_ptr, packet_ptr, udp_header_ptr -> nx_udp_header_word_0, NX_TRACE_INTERNAL_EVENTS, 0, 0);

    /* Endian swapping logic.  If NX_LITTLE_ENDIAN is specified, these macros will
    swap the endian of the UDP header.  */
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_0);
    NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);

    /* Determine if we need to compute the UDP checksum.

    Note that with IPv6, UDP packet checksum is mandatory. However if the underly device
    driver is able to compute UDP checksum in hardware, let the driver handle the checksum
    computation.
    */

    if ((!socket_ptr -> nx_udp_socket_disable_checksum) ||
        (ip_address -> nxd_ip_version == NX_IP_VERSION_V6))
    {
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        if (interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM)
        {
            compute_checksum = 0;
        }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */

#ifdef NX_IPSEC_ENABLE
        /* In case this packet is going through the IPsec protected channel, the checksum would have to be computed
        in software even if the hardware checksum is available at driver layer.  The checksum value must be present
        in order when applying IPsec process. */

        if ((packet_ptr -> nx_packet_ipsec_sa_ptr != NX_NULL) && (((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_encryption_method != NX_CRYPTO_NONE))
        {
            compute_checksum = 1;
        }
#endif /* NX_IPSEC_ENABLE */

#if defined(NX_DISABLE_UDP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE)
        if (compute_checksum)
#endif /* defined(NX_DISABLE_UDP_TX_CHECKSUM) || defined(NX_ENABLE_INTERFACE_CAPABILITY) || defined(NX_IPSEC_ENABLE) */
        {

        ULONG checksum;

            /* Yes, we need to compute the UDP checksum.  */
            checksum = _nx_ip_checksum_compute(packet_ptr,
                                               NX_PROTOCOL_UDP,
                                               (UINT)packet_ptr -> nx_packet_length,
                                               ip_src_addr,
                                               ip_dest_addr);
            checksum = ~checksum & NX_LOWER_16_MASK;

            /* If the computed checksum is zero, it will be transmitted as all ones. */
            /* RFC 768, page 2. */
            if (checksum == 0)
            {
                checksum = 0xFFFF;
            }

            NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);

            udp_header_ptr -> nx_udp_header_word_1 = udp_header_ptr -> nx_udp_header_word_1 | checksum;

            NX_CHANGE_ULONG_ENDIAN(udp_header_ptr -> nx_udp_header_word_1);
        }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        else
        {
            /* Set CHECKSUM flag so the driver would invoke the HW checksum. */
            packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_UDP_TX_CHECKSUM;
        }
#endif
    }

    /* Get mutex protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

#ifdef NX_ENABLE_TCPIP_OFFLOAD
    if ((interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD) &&
        (interface_ptr -> nx_interface_tcpip_offload_handler))
    {
        return(_nx_udp_socket_driver_send(socket_ptr, packet_ptr, &ip_src_address, ip_address, port));
    }
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

#ifndef NX_DISABLE_IPV4
    /* Send the UDP packet to the IPv4 component.  */
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V4)
    {

        /*lint -e{644} suppress variable might not be initialized, since "next_hop_address" was initialized in _nx_ip_route_find. */
        _nx_ip_packet_send(ip_ptr, packet_ptr, ip_address -> nxd_ip_address.v4,
                           socket_ptr -> nx_udp_socket_type_of_service,
                           socket_ptr -> nx_udp_socket_time_to_live,
                           NX_IP_UDP, socket_ptr -> nx_udp_socket_fragment_enable,
                           next_hop_address);
    }
#endif /* NX_DISABLE_IPV4 */

#ifdef FEATURE_NX_IPV6
    if (ip_address -> nxd_ip_version == NX_IP_VERSION_V6)
    {

        /* Set the source IPv6 address */
        _nx_ipv6_packet_send(ip_ptr, packet_ptr, NX_PROTOCOL_UDP,
                             packet_ptr -> nx_packet_length, ip_ptr -> nx_ipv6_hop_limit,
                             ip_src_addr,
                             ip_dest_addr);
    }
#endif /* FEATURE_NX_IPV6 */

    /* Release mutex protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return a successful status.  */
    return(NX_SUCCESS);
}

