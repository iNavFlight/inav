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
/*    _nx_secure_tls_send_finished                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the Finished message to send to the remote  */
/*    host. The Finished message contains a hash of all handshake         */
/*    messages received up to this point which is used to verify that     */
/*    none of the messages have been corrupted.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Packet used to send message   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
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
UINT _nx_secure_tls_send_finished(NX_SECURE_TLS_SESSION *tls_session, NX_PACKET *send_packet)
{
UCHAR            *finished_label;
UINT             hash_size = 0;
UINT             status;
UINT             is_server;


    is_server = (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER);

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if(tls_session->nx_secure_tls_1_3)
    {

        /* Generate the TLS 1.3-specific finished data. */
        status = _nx_secure_tls_1_3_finished_hash_generate(tls_session, is_server, &hash_size,
                                                           send_packet -> nx_packet_append_ptr,
                                                           ((ULONG)(send_packet -> nx_packet_data_end) -
                                                            (ULONG)(send_packet -> nx_packet_append_ptr)));
    }
    else
#endif /* (NX_SECURE_TLS_TLS_1_3_ENABLED) */
    {
        /* Select our label for generating the finished hash expansion. */
        if (is_server)
        {
            finished_label = (UCHAR *)"server finished";
        }
        else
        {
            finished_label = (UCHAR *)"client finished";
        }

        if (NX_SECURE_TLS_FINISHED_HASH_SIZE > ((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)))
        {
            
            /* Packet buffer too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        /* Finally, generate the verification data required by TLS - 12 bytes using the PRF and the data
           we have collected. Place the result directly into the packet buffer. */
        status = _nx_secure_tls_finished_hash_generate(tls_session, finished_label, send_packet -> nx_packet_append_ptr);

#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
        /* If we are doing secure renegotiation as per RFC 5746, we need to save off the generated
           verify data now. For TLS 1.0-1.2 this is 12 bytes. If SSLv3 is ever used, it will be 36 bytes. */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_local_verify_data, send_packet -> nx_packet_append_ptr, NX_SECURE_TLS_FINISHED_HASH_SIZE); /* Use case of memcpy is verified. lgtm[cpp/banned-api-usage-required-any] */
#endif

        /* The finished verify data is always 12 bytes for TLS 1.2 and earlier. */
        hash_size = NX_SECURE_TLS_FINISHED_HASH_SIZE;
    }

    /* Adjust the packet into which we just wrote the finished hash. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + hash_size;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + hash_size;

    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Finished with the handshake - we can free certificates now. */
    status = _nx_secure_tls_remote_certificate_free_all(tls_session);

    return(status);
}

