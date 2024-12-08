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
/**   Internet Protocol (IP)                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_ip.h"
#include "nx_packet.h"

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /* NX_IPSEC_ENABLE */

#ifndef NX_DISABLE_IPV4
/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_ip_packet_send                                  PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function prepends an IP header and sends an IP packet to the   */
/*    appropriate link driver.                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*    destination_ip                        Destination IP address        */
/*    type_of_service                       Type of service for packet    */
/*    time_to_live                          Time to live value for packet */
/*    protocol                              Protocol being encapsulated   */
/*    fragment                              Don't fragment bit            */
/*    next_hop_address                      Next Hop address              */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_ip_checksum_compute               Compute IP checksum           */
/*    _nx_ip_header_add                     Add the IP header             */
/*    _nx_ip_route_find                     Find suitable outgoing        */
/*    _nx_ip_driver_packet_send             Send the IP packet            */
/*    _nx_packet_transmit_release           Release transmit packet       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    NetX Source Code                                                    */
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
/*                                            added new ip filter,        */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
VOID  _nx_ip_packet_send(NX_IP *ip_ptr, NX_PACKET *packet_ptr,
                         ULONG destination_ip, ULONG type_of_service, ULONG time_to_live,
                         ULONG protocol, ULONG fragment, ULONG next_hop_address)
{

#ifdef NX_IPSEC_ENABLE
UINT            status = 0;
ULONG           payload_size;
USHORT          value;
UCHAR           is_hw_processed = NX_FALSE;
NX_IPV4_HEADER *ip_header_ptr;
ULONG           checksum;
ULONG           val;
#endif /* NX_IPSEC_ENABLE */


    /* Add debug information. */
    NX_PACKET_DEBUG(__FILE__, __LINE__, packet_ptr);

#ifndef NX_DISABLE_IP_INFO

    /* Increment the total send requests counter.  */
    ip_ptr -> nx_ip_total_packet_send_requests++;
#endif

    /* Make sure the packet interface is set. */
    if (packet_ptr -> nx_packet_address.nx_packet_interface_ptr == NX_NULL)
    {

#ifndef NX_DISABLE_IP_INFO

        /* Increment the IP invalid packet error.  */
        ip_ptr -> nx_ip_invalid_transmit_packets++;
#endif /* !NX_DISABLE_IP_INFO */

        /* Prepend the IP header to the packet.  First, make room for the IP header.  */
        packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_IPV4_HEADER);

        /* Increase the packet length.  */
        packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length + (ULONG)sizeof(NX_IPV4_HEADER);

        /* Release the packet.  */
        _nx_packet_transmit_release(packet_ptr);

        /* Return... nothing more can be done!  */
        return;
    }

#ifdef NX_ENABLE_TCPIP_OFFLOAD
    if (packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag &
        NX_INTERFACE_CAPABILITY_TCPIP_OFFLOAD)
    {
#ifndef NX_DISABLE_IP_INFO

        /* Increment the IP invalid packet error.  */
        ip_ptr -> nx_ip_invalid_transmit_packets++;
#endif

        /* Ignore sending all packets for TCP/IP offload. Release the packet.  */
        _nx_packet_transmit_release(packet_ptr);

        /* Return... nothing more can be done!  */
        return;
    }
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

#ifdef NX_IPSEC_ENABLE
    /* Check if this packet is continued after HW crypto engine. */
    if (packet_ptr -> nx_packet_ipsec_sa_ptr &&
        ((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_mode == NX_IPSEC_TRANSPORT_MODE &&
        (packet_ptr -> nx_packet_ipsec_state == NX_IPSEC_AH_PACKET ||
         packet_ptr -> nx_packet_ipsec_state == NX_IPSEC_ESP_PACKET))
    {
        is_hw_processed = NX_TRUE;
    }

    /* Process this packet in IPsec transport mode? */
    if (packet_ptr -> nx_packet_ipsec_sa_ptr &&
        packet_ptr -> nx_packet_ipsec_state != NX_IPSEC_ESP_PACKET &&
        packet_ptr -> nx_packet_ipsec_state != NX_IPSEC_AH_PACKET &&
        ((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_mode == NX_IPSEC_TRANSPORT_MODE)
    {

        /* Perform IPSec processing and insert IPsec headers before the IP header. */
        /* Notice that for TCP transmission, a new packet is used to store the encrypted data, while the
           original TCP packet is put back on to the transmitted queue. */
        payload_size = packet_ptr -> nx_packet_length;
        status = _nx_ipsec_ip_output_packet_process(ip_ptr, &packet_ptr, (protocol >> 16), payload_size, &payload_size);

        if ((status != NX_SUCCESS) &&
            (status != NX_IPSEC_HW_PENDING))
        {

            /* Install an area for the IP header.  This is required by the nx_packet_transmit_release. */
            packet_ptr -> nx_packet_prepend_ptr =  packet_ptr -> nx_packet_prepend_ptr - sizeof(NX_IPV4_HEADER);

            /* Increase the packet length for the IP header.  */
            packet_ptr -> nx_packet_length =  packet_ptr -> nx_packet_length + sizeof(NX_IPV4_HEADER);

            /* IPsec output packet process failed. */

            /* Release the packet.  */
            _nx_packet_transmit_release(packet_ptr);

            return;
        }

        /* Now set the IPSec protocol. */
        protocol = (ULONG)((((NX_IPSEC_SA *)packet_ptr -> nx_packet_ipsec_sa_ptr) -> nx_ipsec_sa_protocol) << 16);

        /* Set the DF bit.
           Transport mode SAs have been defined to not carry fragments (IPv4 or IPv6),RFC 4301 page 66&88..*/
        fragment = NX_DONT_FRAGMENT;
    }
#endif /* NX_IPSEC_ENABLE  */


    /* If the packet is processed by HW crypto engine, do not add IP header. */
#ifdef NX_IPSEC_ENABLE
    if (!is_hw_processed)
#endif /* NX_IPSEC_ENABLE  */
    {

        /* Add the IP Header to the packet.  */
        _nx_ip_header_add(ip_ptr, packet_ptr, packet_ptr -> nx_packet_ip_interface -> nx_interface_ip_address,
                          destination_ip, type_of_service, time_to_live, protocol, fragment);

#ifdef NX_ENABLE_IP_PACKET_FILTER
        /* Check if the IP packet filter is set. */
        if (ip_ptr -> nx_ip_packet_filter)
        {

            /* Yes, call the IP packet filter routine. */
            if (ip_ptr -> nx_ip_packet_filter((VOID *)(packet_ptr -> nx_packet_prepend_ptr),
                                              NX_IP_PACKET_OUT) != NX_SUCCESS)
            {

                /* Drop the packet. */
                _nx_packet_transmit_release(packet_ptr);
                return;
            }
        }

        /* Check if the IP packet filter extended is set. */
        if (ip_ptr -> nx_ip_packet_filter_extended)
        {

            /* Yes, call the IP packet filter extended routine. */
            if (ip_ptr -> nx_ip_packet_filter_extended(ip_ptr, packet_ptr, NX_IP_PACKET_OUT) != NX_SUCCESS)
            {

                /* Drop the packet. */
                _nx_packet_transmit_release(packet_ptr);
                return;
            }
        }
#endif /* NX_ENABLE_IP_PACKET_FILTER */
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_IP_SEND, ip_ptr, destination_ip, packet_ptr, packet_ptr -> nx_packet_length, NX_TRACE_INTERNAL_EVENTS, 0, 0);


#ifdef NX_IPSEC_ENABLE

    if (is_hw_processed)
    {

        /* Destination IP is unknow after HW crypto engine process.  */
        /* Get destination IP from IP header.  */
        /* Setup the IP header pointer.  */
        ip_header_ptr =  (NX_IPV4_HEADER *)packet_ptr -> nx_packet_prepend_ptr;

        /* Fix payload size.  */
        value = (USHORT)(packet_ptr -> nx_packet_length);
        NX_CHANGE_USHORT_ENDIAN(value);

        /* First clear payload_size field.  */
        ip_header_ptr -> nx_ip_header_word_0 &= 0xFFFF;

        /* Fill payload_size field.  */
        ip_header_ptr -> nx_ip_header_word_0 |= (ULONG)(value << NX_SHIFT_BY_16) & 0xFFFF0000;

#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        if (!(packet_ptr -> nx_packet_address.nx_packet_interface_ptr -> nx_interface_capability_flag & NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM))
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
        {

            /* Clear checksum in the IP header.  */
            ip_header_ptr -> nx_ip_header_word_2 =  ip_header_ptr -> nx_ip_header_word_2 & 0xFFFF;

            checksum = _nx_ip_checksum_compute(packet_ptr, NX_IP_VERSION_V4,
                                               /* Length is the size of IP header, including options */
                                               20,
                                               /* IPv4 header checksum does not use src/dest addresses */
                                               NULL, NULL);

            val = (ULONG)(~checksum);
            val = val & NX_LOWER_16_MASK;

            /* Convert to network byte order. */
            NX_CHANGE_ULONG_ENDIAN(val);

            /* Now store the checksum in the IP header.  */
            ip_header_ptr -> nx_ip_header_word_2 =  ip_header_ptr -> nx_ip_header_word_2 | val;
        }
#ifdef NX_ENABLE_INTERFACE_CAPABILITY
        else
        {
            packet_ptr -> nx_packet_interface_capability_flag |= NX_INTERFACE_CAPABILITY_IPV4_TX_CHECKSUM;
        }
#endif /* NX_ENABLE_INTERFACE_CAPABILITY */
    }

    /* Process this packet in IPsec tunnel mode? */
    if (packet_ptr -> nx_packet_ipsec_sa_ptr &&
        packet_ptr -> nx_packet_ipsec_state != NX_IPSEC_ESP_PACKET &&
        packet_ptr -> nx_packet_ipsec_state != NX_IPSEC_AH_PACKET &&
        ((NX_IPSEC_SA *)(packet_ptr -> nx_packet_ipsec_sa_ptr)) -> nx_ipsec_sa_mode == NX_IPSEC_TUNNEL_MODE)
    {


        /* Perform IPSec processing for tunneling. Insert IPsec headers and encapsulating the IP header. */
        payload_size = packet_ptr -> nx_packet_length;
        status = _nx_ipsec_ip_output_packet_process(ip_ptr, &packet_ptr, NX_PROTOCOL_IPV4, payload_size, &payload_size);

        if ((status != NX_SUCCESS) &&
            (status != NX_IPSEC_HW_PENDING))
        {
            /* IPsec output packet process failed. */

            /* Release the packet.  */
            _nx_packet_transmit_release(packet_ptr);
        }

        /* Tunnel consume the packet. */
        return;
    }

    /* Process IPSec on packet requiring AH processing. */
    if (packet_ptr -> nx_packet_ipsec_sa_ptr &&
        packet_ptr -> nx_packet_ipsec_state == NX_IPSEC_AH_PACKET)
    {

        status = ip_ptr -> nx_ip_ipsec_authentication_header_transmit(ip_ptr, &packet_ptr, protocol, 1);

        if ((status != NX_SUCCESS) &&
            (status != NX_IPSEC_HW_PENDING))
        {
            /* Release the packet.  */
            _nx_packet_transmit_release(packet_ptr);

            return;
        }
    }

    /* HW crypto driver is processing packet. */
    if (status == NX_IPSEC_HW_PENDING)
    {

#ifndef NX_DISABLE_IP_INFO

        /* Decrement the total send requests counter.  */
        ip_ptr -> nx_ip_total_packet_send_requests--;
#endif
        return;
    }

#endif

    /* If the next hop address is null, indicates the specified interface is unreached.  */
    if (next_hop_address == 0)
    {

        /* Check whether the forward feature is enabled.  */
        if (ip_ptr -> nx_ip_forward_packet_process)
        {

            /* Initialize the interface.  */
            packet_ptr -> nx_packet_address.nx_packet_interface_ptr = NX_NULL;

            /* Figure out the best interface to send the packet on. */
            _nx_ip_route_find(ip_ptr, destination_ip, &packet_ptr -> nx_packet_address.nx_packet_interface_ptr, &next_hop_address);
        }

        /* Make sure the packet interface and next hop address are set. */
        /*lint -e{644} suppress variable might not be initialized, since "next_hop_address" was initialized in _nx_ip_route_find. */
        if ((packet_ptr -> nx_packet_address.nx_packet_interface_ptr == NX_NULL) || (next_hop_address == 0))
        {

#ifndef NX_DISABLE_IP_INFO

            /* Increment the IP invalid packet error.  */
            ip_ptr -> nx_ip_invalid_transmit_packets++;
#endif /* !NX_DISABLE_IP_INFO */

            /* Release the packet.  */
            _nx_packet_transmit_release(packet_ptr);

            /* Return... nothing more can be done!  */
            return;
        }
    }

    /* Directly send the packet.  */
    _nx_ip_driver_packet_send(ip_ptr, packet_ptr, destination_ip, fragment, next_hop_address);
}

#endif /* NX_DISABLE_IPV4 */

