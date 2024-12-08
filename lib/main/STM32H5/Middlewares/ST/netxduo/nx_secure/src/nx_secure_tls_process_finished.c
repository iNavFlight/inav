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

static UCHAR generated_hash[NX_SECURE_TLS_MAX_HASH_SIZE];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_finished                     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming Finished message by generating  */
/*    a hash of the received handshake messages (up to this point) and    */
/*    comparing the result to the hash received from the remote host.     */
/*    If the generated hash and the received hash do not match, it        */
/*    indicates that the handshake messages were corrupted in some way.   */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Pointer to received data      */
/*    message_length                        Length of received data       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Result of hash comparison     */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_finished_hash_generate Generate Finished hash        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_finished(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                     UINT message_length)
{
UCHAR            *finished_label;
UINT              compare_result, status;
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT              hash_size = 0;
UINT              is_server;
#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    /* Make sure the length is what we expect. */
    if(tls_session -> nx_secure_tls_1_3)
    {
        /* TLS 1.3 Finished messages are different from earlier versions
           and require specific processing. */
        /* Check if the Finished is from server. */
        is_server = (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT);

        /* Verify Finished hash. */
        status = _nx_secure_tls_1_3_finished_hash_generate(tls_session, is_server, &hash_size, generated_hash, sizeof(generated_hash));

        if ((hash_size > message_length) || (hash_size > sizeof(generated_hash)))
        {

            /* Size error. */
            compare_result = 1;
        }
        else
        {
            
            /* Compare to see if the Finished hash matches the recevied hash. */
            compare_result = (UINT)NX_SECURE_MEMCMP(generated_hash, packet_buffer, hash_size);
        }
    }
    else
#endif
    {
        /* TLS 1.2 and earlier use the same Finished hash construction. */
        if (message_length != NX_SECURE_TLS_FINISHED_HASH_SIZE)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        
        /* If we received a Finished message but the session is not active, error! */
        if (!tls_session -> nx_secure_tls_remote_session_active)
        {
            return(NX_SECURE_TLS_UNEXPECTED_MESSAGE);
        }

        /* If we have not received credentials from the remote host, we
           cannot validate the handshake under any circumstances. */
        if (!tls_session -> nx_secure_tls_received_remote_credentials)
        {
            return(NX_SECURE_TLS_HANDSHAKE_FAILURE);
        }

        /* Select our label for generating the finished hash expansion - we are comparing against
           the remote hosts hash, so use the opposite label. */
        if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
        {
            finished_label = (UCHAR *)"client finished";
        }
        else
        {
            finished_label = (UCHAR *)"server finished";
        }

        /* Finally, generate the verification data required by TLS - 12 bytes using the PRF and the data
           we have collected. */
        status = _nx_secure_tls_finished_hash_generate(tls_session, finished_label, generated_hash);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
        /* If we are doing secure renegotiation as per RFC 5746, we need to save off the generated
           verify data now. For TLS 1.0-1.2 this is 12 bytes. If SSLv3 is ever used, it will be 36 bytes. */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_remote_verify_data, generated_hash, NX_SECURE_TLS_FINISHED_HASH_SIZE); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */

        /* The finished verify data is always 12 bytes (*except for SSLv3) - compare to see if the Finished hash matches the recevied hash. */
        compare_result = (UINT)NX_SECURE_MEMCMP(generated_hash, packet_buffer, NX_SECURE_TLS_FINISHED_HASH_SIZE);
    }

    if (compare_result != 0)
    {
        return(NX_SECURE_TLS_FINISHED_HASH_FAILURE);
    }

    /* If the comparison was OK, set the state appropriately. */
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT)
    {
        tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_HANDSHAKE_FINISHED;
    }
#endif

    /* Now that we are done with the handshake, free all remote certificates - we don't need them anymore. */
    status = _nx_secure_tls_remote_certificate_free_all(tls_session);

    if (status != NX_SUCCESS)
    {
        return(status);
    }

    return(NX_SECURE_TLS_SUCCESS);
}

