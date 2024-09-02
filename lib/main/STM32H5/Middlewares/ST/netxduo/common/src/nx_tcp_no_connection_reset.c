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

#ifdef NX_IPSEC_ENABLE
#include "nx_ipsec.h"
#endif /*NX_IPSEC_ENABLE*/

#ifdef FEATURE_NX_IPV6
#include "nx_ipv6.h"
#endif /* FEATURE_NX_IPV6 */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_no_connection_reset                         PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a reset when there is no connection present,    */
/*    which avoids the timeout processing on the other side of the        */
/*    connection.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP control block   */
/*    packet_ptr                            Pointer to packet to send     */
/*    tcp_header_ptr                        TCP header                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nxd_tcp_no_connect_reset             Invokes tcp no conn reset     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_packet_process                TCP packet processing         */
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
VOID  _nx_tcp_no_connection_reset(NX_IP *ip_ptr, NX_PACKET *packet_ptr, NX_TCP_HEADER *tcp_header_ptr)
{

NX_TCP_SOCKET fake_socket;
ULONG         header_length;
#ifdef NX_IPSEC_ENABLE
VOID         *sa;
NXD_ADDRESS   source_ip;
NXD_ADDRESS   destination_ip;
UINT          ret;
ULONG         data_offset = 0;
#endif /* NX_IPSEC_ENABLE */


    /* Clear the fake socket first.  */
    memset((void *)&fake_socket, 0, sizeof(NX_TCP_SOCKET));

    /* Build a fake socket so we can send a reset TCP requests that are not valid.  */
    fake_socket.nx_tcp_socket_ip_ptr = ip_ptr;

    /* Set the connection IP address.  */
#ifndef NX_DISABLE_IPV4
    if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V4)
    {
    NX_IPV4_HEADER *ip_header_ptr;

        /* Set the IP header.  */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ip_header_ptr =  (NX_IPV4_HEADER *)packet_ptr -> nx_packet_ip_header;

        /* Set the connection ip.  */
        fake_socket.nx_tcp_socket_connect_ip.nxd_ip_version = NX_IP_VERSION_V4;
        fake_socket.nx_tcp_socket_connect_ip.nxd_ip_address.v4 = ip_header_ptr -> nx_ip_header_source_ip;

        /* Assume the interface that receives the incoming packet is the best interface
           for sending responses. */
        fake_socket.nx_tcp_socket_connect_interface = packet_ptr -> nx_packet_address.nx_packet_interface_ptr;
        fake_socket.nx_tcp_socket_next_hop_address = NX_NULL;

        /* Find the next hop info. */
        _nx_ip_route_find(ip_ptr, fake_socket.nx_tcp_socket_connect_ip.nxd_ip_address.v4, &fake_socket.nx_tcp_socket_connect_interface,
                          &fake_socket.nx_tcp_socket_next_hop_address);

#ifdef NX_IPSEC_ENABLE
        /* Get the source ip address.  */
        source_ip.nxd_ip_version = NX_IP_VERSION_V4;
        source_ip.nxd_ip_address.v4 = fake_socket.nx_tcp_socket_connect_interface -> nx_interface_ip_address;

        /* Get the destination ip address.  */
        destination_ip.nxd_ip_version = NX_IP_VERSION_V4;
        destination_ip.nxd_ip_address.v4 = ip_header_ptr -> nx_ip_header_source_ip;
#endif /*NX_IPSEC_ENABLE*/
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (packet_ptr -> nx_packet_ip_version == NX_IP_VERSION_V6)
    {
    NX_IPV6_HEADER *ipv6_header_ptr;

        /* Set the IP header.  */
        /*lint -e{927} -e{826} suppress cast of pointer to pointer, since it is necessary  */
        ipv6_header_ptr = (NX_IPV6_HEADER *)packet_ptr -> nx_packet_ip_header;

        /* Set the connection ip.  */
        fake_socket.nx_tcp_socket_connect_ip.nxd_ip_version = NX_IP_VERSION_V6;
        COPY_IPV6_ADDRESS(&ipv6_header_ptr -> nx_ip_header_source_ip[0], fake_socket.nx_tcp_socket_connect_ip.nxd_ip_address.v6);

        /* Set the outgoing address.  */
        fake_socket.nx_tcp_socket_ipv6_addr = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr;

        /* Set the connect ip interface.  */
        fake_socket.nx_tcp_socket_connect_interface = packet_ptr -> nx_packet_address.nx_packet_ipv6_address_ptr -> nxd_ipv6_address_attached;

#ifdef NX_IPSEC_ENABLE
        /* Get the source ip address.  */
        source_ip.nxd_ip_version = NX_IP_VERSION_V6;
        COPY_IPV6_ADDRESS(fake_socket.nx_tcp_socket_ipv6_addr -> nxd_ipv6_address, source_ip.nxd_ip_address.v6);

        /* Get the destination ip address.  */
        destination_ip.nxd_ip_version = NX_IP_VERSION_V6;
        COPY_IPV6_ADDRESS(&ipv6_header_ptr -> nx_ip_header_source_ip[0], destination_ip.nxd_ip_address.v6);
#endif /*NX_IPSEC_ENABLE*/
    }
#endif /* FEATURE_NX_IPV6 */

    /* Set the source port and destination port.  */
    fake_socket.nx_tcp_socket_port  = (UINT)(tcp_header_ptr -> nx_tcp_header_word_0 & NX_LOWER_16_MASK);
    fake_socket.nx_tcp_socket_connect_port = (UINT)(tcp_header_ptr -> nx_tcp_header_word_0 >> NX_SHIFT_BY_16);

    /* Set the sequence number only if the incoming segment does not have the ACK flag, according to
       Section 3.4, "Reset Generation" on page 37, RFC793. */
    if (!(tcp_header_ptr -> nx_tcp_header_word_3 & NX_TCP_ACK_BIT))
    {
        /* Get the header length.  */
        header_length = (tcp_header_ptr -> nx_tcp_header_word_3 >> NX_TCP_HEADER_SHIFT) * (ULONG)sizeof(ULONG);

        /* Update sequence number to set the reset acknowledge number.  */
        tcp_header_ptr -> nx_tcp_sequence_number += (packet_ptr -> nx_packet_length - header_length);

        /* Check the SYN and FIN bits.  */
        if (tcp_header_ptr -> nx_tcp_header_word_3 & (NX_TCP_SYN_BIT | NX_TCP_FIN_BIT))
        {

            /* Update sequence number to set the reset acknowledge number.  */
            tcp_header_ptr -> nx_tcp_sequence_number++;
        }
    }

#ifdef NX_IPSEC_ENABLE
    /* Check for possible SA match. */
    if (ip_ptr -> nx_ip_packet_egress_sa_lookup != NX_NULL)                   /* IPsec is enabled. */
    {

        /* If the SA has not been set. */
        ret = ip_ptr -> nx_ip_packet_egress_sa_lookup(ip_ptr,                                   /* IP ptr */
                                                      &source_ip,                               /* src_addr */
                                                      &destination_ip,                          /* dest_addr */
                                                      NX_PROTOCOL_TCP,                          /* protocol */
                                                      fake_socket.nx_tcp_socket_port,           /* src_port */
                                                      fake_socket.nx_tcp_socket_connect_port,   /* dest_port */
                                                      &data_offset, &sa, 0);
        if (ret == NX_IPSEC_TRAFFIC_PROTECT)
        {

            /* Save the SA to the socket. */
            fake_socket.nx_tcp_socket_egress_sa = sa;
            fake_socket.nx_tcp_socket_egress_sa_data_offset = data_offset;
        }
        else if (ret == NX_IPSEC_TRAFFIC_DROP || ret == NX_IPSEC_TRAFFIC_PENDING_IKEV2)
        {

            return;
        }
        else
        {

            /* Zero out SA information. */
            fake_socket.nx_tcp_socket_egress_sa = NX_NULL;
            fake_socket.nx_tcp_socket_egress_sa_data_offset = 0;
        }
    }
#endif

    fake_socket.nx_tcp_socket_time_to_live = (UINT)NX_IP_TIME_TO_LIVE;

    /* Send a RST to indicate the connection was not available.  */
    _nx_tcp_packet_send_rst(&fake_socket, tcp_header_ptr);
}

