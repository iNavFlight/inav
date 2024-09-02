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
/*    _nx_secure_tls_handshake_process                    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes a TLS handshake, whether at the beginning   */
/*    of a new TLS connection or during a session re-negotiation. The     */
/*    handshake state machine is implemented for each of TLS Client and   */
/*    Server in their own functions, this function is simply the entry    */
/*    point for handling the handshake messages.                          */
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
/*    _nx_secure_tls_session_receive_records                              */
/*                                          Receive TLS records           */
/*    nx_secure_tls_packet_release          Release packet                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_session_start          Start TLS session             */
/*    _nx_secure_tls_session_receive        Receive TCP data              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            released packet securely,   */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_handshake_process(NX_SECURE_TLS_SESSION *tls_session, UINT wait_option)
{
UINT       status = NX_NOT_SUCCESSFUL;
NX_PACKET *incoming_packet = NX_NULL;

    /* Process the handshake depending on the TLS session type. */
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT)
    {

        /* Handle our incoming handshake messages. Continue processing until the handshake is complete
         * or an error/timeout occurs. */
        while (tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED)
        {
            status = _nx_secure_tls_session_receive_records(tls_session, &incoming_packet, wait_option);

            /* Make sure we didn't have an error during the receive. */
            if (status != NX_SUCCESS)
            {
                break;
            }
        }

        if (tls_session -> nx_secure_tls_client_state == NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED)
        {

            /* Release the incoming packet if we do receive it. */
            nx_secure_tls_packet_release(incoming_packet);
        }
    }
#endif

#ifndef NX_SECURE_TLS_SERVER_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
    {
        /* Session is a TLS Server type. */
        /* The client socket connection has already been accepted at this point, process the handshake.  */

        /* Now handle our incoming handshake messages. Continue processing until the handshake is complete
         * or an error/timeout occurs. */
        while (tls_session -> nx_secure_tls_server_state != NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED)
        {
            status = _nx_secure_tls_session_receive_records(tls_session, &incoming_packet, wait_option);

            /* Make sure we didn't have an error during the receive. */
            if (status != NX_SUCCESS)
            {
                break;
            }
        }

        if (tls_session -> nx_secure_tls_server_state == NX_SECURE_TLS_SERVER_STATE_HANDSHAKE_FINISHED)
        {

            /* Release the incoming packet if we do receive it. */
            nx_secure_tls_packet_release(incoming_packet);
        }
    }
#endif

    if ((status == NX_NO_PACKET) && (wait_option == 0))
    {

        /* It is non blocking mode. */
        status = NX_CONTINUE;
    }

    return(status);
}

