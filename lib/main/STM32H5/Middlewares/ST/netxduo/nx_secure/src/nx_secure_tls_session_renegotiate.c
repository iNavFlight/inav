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
/**    Transport Layer Security (TLS)                                     */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_session_renegotiate                  PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function re-negotiates a previously established connection     */
/*    with a remote host. This functionality allows a TLS host (client or */
/*    server) to generate new session keys in response to an application  */
/*    need, usually due to a connection being open for a long time or in  */
/*    response to a potential security issue.                             */
/*                                                                        */
/*    If the session is still active (no CloseNotify messages have been   */
/*    sent) then a re-negotiation is done using the Secure Renegotiation  */
/*    Indication Extension (RFC 5746), if enabled. If the session has     */
/*    been closed, a new session is established using the existing TCP    */
/*    socket assigned in the call to nx_secure_tls_session start. If a    */
/*    new session is being established, session resumption will be used   */
/*    if available.                                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_allocate_handshake_packet                            */
/*                                          Allocate TLS packet           */
/*    _nx_secure_tls_remote_certificate_free_all                          */
/*                                          Free all remote certificates  */
/*    _nx_secure_tls_send_clienthello       Send ClientHello              */
/*    _nx_secure_tls_send_handshake_record  Send TLS handshake record     */
/*    _nx_secure_tls_send_hellorequest      Send HelloRequest             */
/*    _nx_secure_tls_session_receive_records                              */
/*                                          Receive TLS records           */
/*    nx_secure_tls_packet_release          Release packet                */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            supported chained packet,   */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            fixed packet leak bug,      */
/*                                            resulting in version 6.1.8  */
/*  10-15-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            option to disable client    */
/*                                            initiated renegotiation,    */
/*                                            resulting in version 6.1.9  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), changed  */
/*                                            an error to assert,         */
/*                                            resulting in version 6.1.11 */
/*  10-31-2022     Yanwu Cai                Modified comment(s), and      */
/*                                            fixed renegotiation when    */
/*                                            receiving in non-block mode,*/
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
UINT _nx_secure_tls_session_renegotiate(NX_SECURE_TLS_SESSION *tls_session, UINT wait_option)
{
UINT       status = NX_NOT_SUCCESSFUL;
NX_PACKET *incoming_packet = NX_NULL;
NX_PACKET *send_packet;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Reset the record queue. */
    tls_session -> nx_secure_record_queue_header = NX_NULL;
    tls_session -> nx_secure_record_decrypted_packet = NX_NULL;

    /* If the session isn't active, trying to renegotiate is an error! */
    if (tls_session -> nx_secure_tls_remote_session_active != NX_TRUE || tls_session -> nx_secure_tls_local_session_active != NX_TRUE)
    {
        /* Release the protection. */
        tx_mutex_put(&_nx_secure_tls_protection);
        return(NX_SECURE_TLS_RENEGOTIATION_SESSION_INACTIVE);
    }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if (tls_session -> nx_secure_tls_1_3)
    {

        return(NX_SECURE_TLS_NO_RENEGOTIATION_ERROR);
    }
#endif

    /* Make sure the remote host supports renegotiation. */
    if(!tls_session -> nx_secure_tls_secure_renegotiation)
    {
        tx_mutex_put(&_nx_secure_tls_protection);
        return(NX_SECURE_TLS_RENEGOTIATION_FAILURE);
    }

    /* Re-establish the TLS connection based on the session type. */
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT)
    {

        /* Allocate a handshake packet so we can send the ClientHello. */
        status = _nx_secure_tls_allocate_handshake_packet(tls_session, tls_session -> nx_secure_tls_packet_pool, &send_packet, wait_option);

        if (status != NX_SUCCESS)
        {

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);
            return(status);
        }

        /* This is a renegotiation handshake so indicate that to the stack. */
        tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_RENEGOTIATING;
        tls_session -> nx_secure_tls_local_initiated_renegotiation = NX_TRUE;

        /* On a session resumption free all certificates for the new session.
         * SESSION RESUMPTION: if session resumption is enabled, don't free!!
         */
        status = _nx_secure_tls_remote_certificate_free_all(tls_session);

        if (status != NX_SUCCESS)
        {
            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);
            return(status);
        }

        /* Populate our packet with clienthello data. */
        status = _nx_secure_tls_send_clienthello(tls_session, send_packet);

        if (status == NX_SUCCESS)
        {

            /* Send the ClientHello to kick things off. */
            status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_CLIENT_HELLO, wait_option);
        }

        /* If anything after the allocate fails, we need to release our packet. */
        if (status != NX_SUCCESS)
        {

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);
            nx_secure_tls_packet_release(send_packet);
            return(status);
        }

        /* Now handle our incoming handshake messages. Continue processing until the handshake is complete
         * or an error/timeout occurs. */
        while (tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED)
        {
            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);

            /* Before handshake finished, incoming packet will not be set. */
            status = _nx_secure_tls_session_receive_records(tls_session, &incoming_packet, wait_option);

            /* Get the protection. */
            tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

            /* Make sure we didn't have an error during the receive. */
            if (status != NX_SUCCESS)
            {
                break;
            }
        }

        if (incoming_packet != NX_NULL)
        {
            nx_secure_tls_packet_release(incoming_packet);
        }

        if (tls_session -> nx_secure_tls_client_state == NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED)
        {
            tls_session -> nx_secure_tls_local_initiated_renegotiation = NX_FALSE;
            tls_session -> nx_secure_tls_renegotiation_handshake = NX_FALSE;
        }
    }
#endif

#ifndef NX_SECURE_TLS_SERVER_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {
        /* Session is a TLS Server type. */

        /* The session is active, so send a HelloRequest to re-establish the connection. */
        /* Allocate a handshake packet so we can send the HelloRequest message. */
        status = _nx_secure_tls_allocate_handshake_packet(tls_session, tls_session -> nx_secure_tls_packet_pool, &send_packet, wait_option);

        if (status != NX_SUCCESS)
        {

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);
            return(status);
        }

        /* We are requesting a renegotiation from the server side - we need to know if we requested
           the renegotiation when the ClientHello comes in so we can reject client-initiated renegotiation
           if the user so chooses. */
        tls_session -> nx_secure_tls_server_renegotiation_requested = NX_TRUE;

        /* Populate our packet with HelloRequest data. */
        status = _nx_secure_tls_send_hellorequest(tls_session, send_packet);
        NX_ASSERT(status == NX_SUCCESS);

        tls_session -> nx_secure_tls_local_initiated_renegotiation = NX_TRUE;

        /* Send the HelloRequest to kick things off. */
        status = _nx_secure_tls_send_handshake_record(tls_session, send_packet, NX_SECURE_TLS_HELLO_REQUEST, wait_option);

        /* If anything after the allocate fails, we need to release our packet. */
        if (status != NX_SUCCESS)
        {

            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);
            nx_secure_tls_packet_release(send_packet);
            return(status);
        }

        /* The client socket connection has already been accepted at this point, process the handshake.  */

        /* Now handle our incoming handshake messages. Continue processing until the handshake is complete
         * or an error/timeout occurs. */
        while (tls_session -> nx_secure_tls_server_state != NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED)
        {
            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);

            /* Before handshake finished, incoming packet will not be set. */
            status = _nx_secure_tls_session_receive_records(tls_session, &incoming_packet, wait_option);

            /* Get the protection. */
            tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

            /* Make sure we didn't have an error during the receive. */
            if (status != NX_SUCCESS)
            {
                break;
            }
        }

        if (incoming_packet != NX_NULL)
        {
            nx_secure_tls_packet_release(incoming_packet);
        }

        if (tls_session -> nx_secure_tls_server_state == NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED)
        {
            tls_session -> nx_secure_tls_local_initiated_renegotiation = NX_FALSE;
            tls_session -> nx_secure_tls_renegotiation_handshake = NX_FALSE;
        }
    }
#endif

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
}
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */
