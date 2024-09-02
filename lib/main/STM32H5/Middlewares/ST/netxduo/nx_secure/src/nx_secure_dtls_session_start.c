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
/** NetX Secure Component                                                 */
/**                                                                       */
/**    Datagram Transport Layer Security (DTLS)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_dtls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_session_start                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function starts a DTLS session given a UDP socket.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS control block            */
/*    udp_socket                            UDP socket pointer            */
/*    is_client                             Is client or server           */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_allocate_handshake_packet                           */
/*                                          Allocate DTLS handshake packet*/
/*    _nx_secure_dtls_retransmit            Retransmit UDP packet         */
/*    _nx_secure_dtls_retransmit_queue_flush                              */
/*                                          Flush retransmit queue        */
/*    _nx_secure_dtls_send_clienthello      Send ClientHello              */
/*    _nx_secure_dtls_send_handshake_record Send DTLS handshake record    */
/*    _nx_secure_dtls_session_receive       Receive DTLS data             */
/*    nx_secure_tls_packet_release          Release packet                */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_secure_dtls_client_session_start  Client session start          */
/*    _nx_secure_dtls_server_session_start  Server session start          */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            released packet securely,   */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_session_start(NX_SECURE_DTLS_SESSION *dtls_session, NX_UDP_SOCKET *udp_socket,
                                   UINT is_client, UINT wait_option)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT                    status = NX_NOT_SUCCESSFUL;
NX_PACKET              *incoming_packet;

NX_SECURE_TLS_SESSION  *tls_session;

UINT                    minimum_wait_option;

#ifndef NX_SECURE_TLS_CLIENT_DISABLED
NX_PACKET *send_packet;
#endif

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Get reference to internal TLS state. */
    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

    /* Assign the packet pool from which TLS will allocate internal message packets. */
    tls_session -> nx_secure_tls_packet_pool = udp_socket -> nx_udp_socket_ip_ptr -> nx_ip_default_packet_pool;

    /* Make sure we are starting with a fresh session. */
    tls_session -> nx_secure_tls_received_remote_credentials = NX_FALSE;

#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION

    /* Renegotiation is not enabled in DTLS session. */
    tls_session -> nx_secure_tls_renegotation_enabled = NX_FALSE;
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */

    /* Assign the TCP socket to the TLS session. */
    dtls_session -> nx_secure_dtls_udp_socket = udp_socket;

    /* Reset the record queue. */
    tls_session -> nx_secure_record_queue_header = NX_NULL;
    tls_session -> nx_secure_record_decrypted_packet = NX_NULL;

    /* See if this is a TCP server started with listen/accept, or a TCP client started with connect. */
    if (is_client)
    {
        /* The TCP socket is a client, so our TLS session is a TLS Client. */
        tls_session -> nx_secure_tls_socket_type = NX_SECURE_TLS_SESSION_TYPE_CLIENT;
    }
    else
    {
        /* This session is now being treated as a server - indicate that fact to the TLS stack. */
        tls_session -> nx_secure_tls_socket_type = NX_SECURE_TLS_SESSION_TYPE_SERVER;
    }

    /* Now process the handshake depending on the TLS session type. */
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT)
    {

        /* Allocate a handshake packet so we can send the ClientHello. */
        status = _nx_secure_dtls_allocate_handshake_packet(dtls_session, tls_session -> nx_secure_tls_packet_pool, &send_packet, wait_option);

        if (status != NX_SUCCESS)
        {

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);
            return(status);
        }

        /* Populate our packet with clienthello data. */
        status = _nx_secure_dtls_send_clienthello(dtls_session, send_packet);

        if (status == NX_SUCCESS)
        {

            /* Send the ClientHello to kick things off. */
            status = _nx_secure_dtls_send_handshake_record(dtls_session, send_packet, NX_SECURE_TLS_CLIENT_HELLO, wait_option, 0);

            /* Clear the protocol version to avoid checking the version in the HelloVerifyRequest which is always DTLS 1.0. */
            tls_session->nx_secure_tls_protocol_version = 0;
        }

        /* If anything after the allocate fails, we need to release our packet. */
        if (status != NX_SUCCESS)
        {

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);
            nx_secure_tls_packet_release(send_packet);
            return(status);
        }
    }
#endif

    /* Now handle our incoming handshake messages. Continue processing until the handshake is complete
     * or an error/timeout occurs. */
    for (;;)
    {
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
        if ((tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT) &&
            (tls_session -> nx_secure_tls_client_state == NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED))
        {
            break;
        }
#endif

#ifndef NX_SECURE_TLS_SERVER_DISABLED
        if ((tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER) &&
            (tls_session -> nx_secure_tls_server_state == NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED))
        {
            break;
        }
#endif

        if ((dtls_session -> nx_secure_dtls_handshake_timeout > wait_option) ||
            (dtls_session -> nx_secure_dtls_handshake_timeout == 0) ||
            (dtls_session -> nx_secure_dtls_transmit_sent_head == NX_NULL))
        {
            minimum_wait_option = wait_option;
        }
        else
        {
            minimum_wait_option = dtls_session -> nx_secure_dtls_handshake_timeout;
        }

        /* Release the protection. */
        tx_mutex_put(&_nx_secure_tls_protection);

        status = _nx_secure_dtls_session_receive(dtls_session, &incoming_packet,
                                                 minimum_wait_option);

        /* Get the protection. */
        tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

        /* Make sure we didn't have an error during the receive. */
        if (status != NX_SUCCESS)
        {

            /* Check error status. */
            if (status != NX_NO_PACKET && status != NX_CONTINUE)
            {
                break;
            }

            if (minimum_wait_option == wait_option)
            {
                break;
            }

            if (dtls_session -> nx_secure_dtls_timeout_retries >=
                NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_RETRIES)
            {
                break;
            }

            /* Retransmit timeout. */
            _nx_secure_dtls_retransmit(dtls_session);

            /* Decrease the wait option. */
            wait_option -= minimum_wait_option;
        }
        else
        {

            /* On any error, the handshake has failed so break out of our processing loop and return. */
            nx_secure_tls_packet_release(incoming_packet);
        }
    }

    /* This is the end of a flight, clear out the transmit queue. */
    _nx_secure_dtls_retransmit_queue_flush(dtls_session);

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
#else
    NX_PARAMETER_NOT_USED(dtls_session);
    NX_PARAMETER_NOT_USED(udp_socket);
    NX_PARAMETER_NOT_USED(is_client);
    NX_PARAMETER_NOT_USED(wait_option);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

