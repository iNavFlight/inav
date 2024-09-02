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
/*    _nx_secure_tls_process_changecipherspec             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ChangeCipherSpec message and    */
/*    sets the TLS state machine state accordingly.                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Pointer to message data       */
/*    message_length                        Length of message data (bytes)*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_session_keys_set       Set session keys              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_process_record        Process DTLS record           */
/*    _nx_secure_tls_process_record         Process TLS record            */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_changecipherspec(NX_SECURE_TLS_SESSION *tls_session,
                                             UCHAR *packet_buffer, UINT message_length)
{
UINT status = NX_SUCCESS;

    /* Verify that we received a proper ChangeCipherSpec message. */
    if (message_length != 1)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* The contents of a ChangeCipherSpec payload should always be a single byte with value 1. */
    if (packet_buffer[0] != 0x1)
    {
        return(NX_SECURE_TLS_BAD_CIPHERSPEC);
    }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    /* TLS 1.3 deprecates the ChangeCipherSpec message. Check that it's correct (above)
       but otherwise ignore it. */
    if(!tls_session->nx_secure_tls_1_3)
#endif
    {

#ifndef NX_SECURE_TLS_SERVER_DISABLED
        if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER &&
            tls_session -> nx_secure_tls_server_state != NX_SECURE_TLS_SERVER_STATE_KEY_EXCHANGE &&
            tls_session -> nx_secure_tls_server_state != NX_SECURE_TLS_SERVER_STATE_CERTIFICATE_VERIFY)
        {
            return(NX_SECURE_TLS_UNEXPECTED_MESSAGE);
        }
#endif
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
        if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT &&
            tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO_DONE)
        {
            return(NX_SECURE_TLS_UNEXPECTED_MESSAGE);
        }
#endif

        /* The remote session is now active - all incoming records from this point will be hashed and encrypted. */
        tls_session -> nx_secure_tls_remote_session_active = 1;

        /* Reset the sequence number now that we are starting a new session. */
        NX_SECURE_MEMSET(tls_session -> nx_secure_tls_remote_sequence_number, 0, sizeof(tls_session -> nx_secure_tls_remote_sequence_number));

        /* Set our remote session keys since we have received a CCS from the remote host. */
        status = _nx_secure_tls_session_keys_set(tls_session, NX_SECURE_TLS_KEY_SET_REMOTE);
    }
    return(status);
}

