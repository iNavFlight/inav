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
/*    _nx_tcp_packet_send_syn                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sends a SYN from the specified socket.                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to socket             */
/*    tx_sequence                           Transmit sequence number      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_packet_send_control           Send TCP control packet       */
/*    _nx_packet_egress_sa_lookup           IPsec process                 */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_tcp_client_socket_connect         Client connect processing     */
/*    _nx_tcp_periodic_processing           Connection retry processing   */
/*    _nx_tcp_packet_process                Server connect response       */
/*                                            processing                  */
/*    _nx_tcp_server_socket_accept          Server socket accept          */
/*                                            processing                  */
/*    _nx_tcp_socket_state_syn_sent         Socket SYN sent processing    */
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
VOID  _nx_tcp_packet_send_syn(NX_TCP_SOCKET *socket_ptr, ULONG tx_sequence)
{

#ifdef NX_IPSEC_ENABLE
ULONG        data_offset = 0;
NXD_ADDRESS  src_addr;
UINT         ret;
NX_IPSEC_SA *cur_sa_ptr = NX_NULL;
#endif /* NX_IPSEC_ENABLE */
ULONG        option_word_1;
ULONG        option_word_2;
#ifdef NX_ENABLE_TCP_WINDOW_SCALING
UINT         include_window_scaling = NX_FALSE;
UINT         scale_factor;
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */
ULONG        mss = 0;

#ifdef NX_IPSEC_ENABLE
#ifndef NX_DISABLE_IPV4
    /* Look for egress SA first. */
    if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
    {
        src_addr.nxd_ip_version = NX_IP_VERSION_V4;
        src_addr.nxd_ip_address.v4 = socket_ptr -> nx_tcp_socket_connect_interface -> nx_interface_ip_address;
    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
    {
        /* IPv6 case. */
        src_addr.nxd_ip_version = NX_IP_VERSION_V6;
        COPY_IPV6_ADDRESS(socket_ptr -> nx_tcp_socket_ipv6_addr -> nxd_ipv6_address, src_addr.nxd_ip_address.v6);
    }
#endif /* FEATURE_NX_IPV6 */

    /* Check for possible SA match. */
    if (socket_ptr -> nx_tcp_socket_ip_ptr -> nx_ip_packet_egress_sa_lookup != NX_NULL)                   /* IPsec is enabled. */
    {

        /* If the SA has not been set. */
        ret = socket_ptr -> nx_tcp_socket_ip_ptr -> nx_ip_packet_egress_sa_lookup(socket_ptr -> nx_tcp_socket_ip_ptr,        /* IP ptr */
                                                                                  &src_addr,                                 /* src_addr */
                                                                                  &socket_ptr -> nx_tcp_socket_connect_ip,   /* dest_addr */
                                                                                  NX_PROTOCOL_TCP,                           /* protocol */
                                                                                  socket_ptr -> nx_tcp_socket_port,          /* src_port */
                                                                                  socket_ptr -> nx_tcp_socket_connect_port,  /* dest_port */
                                                                                  &data_offset, (VOID *)&cur_sa_ptr, 0);
        if (ret == NX_IPSEC_TRAFFIC_PROTECT)
        {

            /* Save the SA to the socket. */
            socket_ptr -> nx_tcp_socket_egress_sa = cur_sa_ptr;
            socket_ptr -> nx_tcp_socket_egress_sa_data_offset = data_offset;
        }
        else if (ret == NX_IPSEC_TRAFFIC_DROP || ret == NX_IPSEC_TRAFFIC_PENDING_IKEV2)
        {

            return;
        }
        else
        {

            /* Zero out SA information. */
            socket_ptr -> nx_tcp_socket_egress_sa = NX_NULL;
            socket_ptr -> nx_tcp_socket_egress_sa_data_offset = 0;
        }
    }
    else
    {
        socket_ptr -> nx_tcp_socket_egress_sa = NX_NULL;
    }
#endif /* NX_IPSEC_ENABLE */

#ifndef NX_DISABLE_IPV4
    /* Update the mss value based on IP version type. */
    if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V4)
    {
        mss = (ULONG)((socket_ptr -> nx_tcp_socket_connect_interface -> nx_interface_ip_mtu_size - sizeof(NX_IPV4_HEADER)) - sizeof(NX_TCP_HEADER));

#ifdef NX_IPSEC_ENABLE
        if (cur_sa_ptr != NX_NULL)
        {

            /* Update the mss value based on sa mode.  */
            if (cur_sa_ptr -> nx_ipsec_sa_protocol == NX_PROTOCOL_NEXT_HEADER_ENCAP_SECURITY)
            {

                /* Update the mss value. minus the ESP HEADER's pad length and IV size , */
                mss = mss - sizeof(NX_IPSEC_ESP_HEADER) -
                    (cur_sa_ptr -> nx_ipsec_sa_encryption_method -> nx_crypto_block_size_in_bytes) -
                    ((cur_sa_ptr -> nx_ipsec_sa_encryption_method -> nx_crypto_IV_size_in_bits) >> 3) -
                    ((cur_sa_ptr -> nx_ipsec_sa_integrity_method -> nx_crypto_ICV_size_in_bits) >> 3);
            }

            if (cur_sa_ptr -> nx_ipsec_sa_protocol == NX_PROTOCOL_NEXT_HEADER_AUTHENTICATION)
            {

                /* Update the mss value. minus the ESP HEADER's IV size and ICV size. */
                mss = mss - sizeof(NX_IPSEC_AUTHENTICATION_HEADER) -
                    (cur_sa_ptr -> nx_ipsec_sa_integrity_method -> nx_crypto_IV_size_in_bits >> 3) -
                    (cur_sa_ptr -> nx_ipsec_sa_integrity_method -> nx_crypto_ICV_size_in_bits >> 3);
            }

            /* If the sa is tunnel mode, the mss value should minus the IPV4 header size .  */
            if (cur_sa_ptr -> nx_ipsec_sa_mode == NX_IPSEC_TUNNEL_MODE)
            {
                mss -=  (sizeof(NX_IPV4_HEADER));
            }
        }
#endif /* NX_IPSEC_ENABLE */

    }
#endif /* !NX_DISABLE_IPV4  */

#ifdef FEATURE_NX_IPV6
    if (socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version == NX_IP_VERSION_V6)
    {
        mss = (ULONG)((socket_ptr -> nx_tcp_socket_connect_interface -> nx_interface_ip_mtu_size - sizeof(NX_IPV6_HEADER)) - sizeof(NX_TCP_HEADER));

#ifdef NX_IPSEC_ENABLE
        if (cur_sa_ptr != NX_NULL)
        {

            /* Update the mss value based on sa mode.  */
            if (cur_sa_ptr -> nx_ipsec_sa_protocol == NX_PROTOCOL_NEXT_HEADER_ENCAP_SECURITY)
            {

                /* Update the mss value. minus the ESP header's pad length and IV size , */
                mss = mss - sizeof(NX_IPSEC_ESP_HEADER) -
                    (cur_sa_ptr -> nx_ipsec_sa_encryption_method -> nx_crypto_block_size_in_bytes) -
                    ((cur_sa_ptr -> nx_ipsec_sa_encryption_method -> nx_crypto_IV_size_in_bits) >> 3) -
                    ((cur_sa_ptr -> nx_ipsec_sa_integrity_method -> nx_crypto_ICV_size_in_bits) >> 3);
            }

            if (cur_sa_ptr -> nx_ipsec_sa_protocol == NX_PROTOCOL_NEXT_HEADER_AUTHENTICATION)
            {

                /* Update the mss value. minus the ESP HEADER's IV size and ICV size. */
                mss = mss - sizeof(NX_IPSEC_AUTHENTICATION_HEADER) -
                    (cur_sa_ptr -> nx_ipsec_sa_integrity_method -> nx_crypto_IV_size_in_bits >> 3) -
                    (cur_sa_ptr -> nx_ipsec_sa_integrity_method -> nx_crypto_ICV_size_in_bits >> 3);
            }

            /* If the sa mode is tunnel mode,the mss value should minus the IPV6 header size .  */
            if (cur_sa_ptr -> nx_ipsec_sa_mode == NX_IPSEC_TUNNEL_MODE)
            {
                mss -=  (sizeof(NX_IPV6_HEADER));
            }
        }
#endif /* NX_IPSEC_ENABLE */
    }
#endif /* FEATURE_NX_IPV6 */

    mss &= 0x0000FFFFUL;

    if ((socket_ptr -> nx_tcp_socket_mss < mss) && socket_ptr -> nx_tcp_socket_mss)
    {

        /* Use the custom MSS. */
        mss = socket_ptr -> nx_tcp_socket_mss;
    }

    if (socket_ptr -> nx_tcp_socket_state == NX_TCP_SYN_RECEIVED)
    {

        /* Update the connect MSS for TCP server socket. */
        if (mss < socket_ptr -> nx_tcp_socket_peer_mss)
        {
            socket_ptr -> nx_tcp_socket_connect_mss  = mss;
        }
        else
        {
            socket_ptr -> nx_tcp_socket_connect_mss =  socket_ptr -> nx_tcp_socket_peer_mss;
        }

        /* Compute the SMSS * SMSS value, so later TCP module doesn't need to redo the multiplication. */
        socket_ptr -> nx_tcp_socket_connect_mss2 =
            socket_ptr -> nx_tcp_socket_connect_mss * socket_ptr -> nx_tcp_socket_connect_mss;
    }
    else
    {

        /* Set the MSS. */
        socket_ptr -> nx_tcp_socket_connect_mss = mss;
    }

    /* Build the MSS option.  */
    option_word_1 = NX_TCP_MSS_OPTION | mss;

    /* Set default option word2. */
    option_word_2 = NX_TCP_OPTION_END;

#ifdef NX_ENABLE_TCP_WINDOW_SCALING
    /* Include window scaling option if we initiates the SYN, or the peer supports Window Scaling. */
    if (socket_ptr -> nx_tcp_socket_state == NX_TCP_SYN_SENT)
    {
        include_window_scaling = NX_TRUE;
    }
    else if (socket_ptr -> nx_tcp_snd_win_scale_value != 0xFF)
    {
        include_window_scaling = NX_TRUE;
    }

    if (include_window_scaling)
    {

        /* Sets the window scaling option. */
        option_word_2 = NX_TCP_RWIN_OPTION;

        /* Compute the window scaling factor */
        for (scale_factor = 0; scale_factor < 15; scale_factor++)
        {

            if ((socket_ptr -> nx_tcp_socket_rx_window_current >> scale_factor) < 65536)
            {
                break;
            }
        }

        /*  Make sure window scale is limited to 14, per RFC 1323 pp.11. */
        if (scale_factor == 15)
        {
            scale_factor = 14;
            socket_ptr -> nx_tcp_socket_rx_window_default = (1 << 30) - 1;
            socket_ptr -> nx_tcp_socket_rx_window_current = (1 << 30) - 1;
        }

        option_word_2 |= scale_factor << 8;

        /* Update the socket with the scale factor. */
        socket_ptr -> nx_tcp_rcv_win_scale_value = scale_factor;
    }
#endif /* NX_ENABLE_TCP_WINDOW_SCALING */

    /* Send SYN or SYN+ACK packet according to socket state. */
    if (socket_ptr -> nx_tcp_socket_state == NX_TCP_SYN_SENT)
    {
        _nx_tcp_packet_send_control(socket_ptr, NX_TCP_SYN_BIT, tx_sequence,
                                    0, option_word_1, option_word_2, NX_NULL);
    }
    else
    {
        _nx_tcp_packet_send_control(socket_ptr, (NX_TCP_SYN_BIT | NX_TCP_ACK_BIT), tx_sequence,
                                    socket_ptr -> nx_tcp_socket_rx_sequence, option_word_1, option_word_2, NX_NULL);
    }

    /* Initialize recover sequence and previous cumulative acknowledgment. */
    socket_ptr -> nx_tcp_socket_tx_sequence_recover = tx_sequence;
    socket_ptr -> nx_tcp_socket_previous_highest_ack = tx_sequence;
}

