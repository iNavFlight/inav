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
/**    Transport Layer Security (TLS) - Generate Session Keys             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE


#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_session_keys_set                     PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the session keys for a TLS session following the */
/*    sending or receiving of a ChangeCipherSpec message. In              */
/*    renegotiation handshakes, two separate set of session keys will be  */
/*    in use simultaneously so we need this to be able to separate which  */
/*    keys are actually in use.                                           */
/*                                                                        */
/*    Once the keys are set, this function initializes the appropriate    */
/*    session cipher with the new key set.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    key_set                               Remote or local keys          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_secure_session_keys_set]          Set session keys              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*    _nx_secure_tls_process_changecipherspec                             */
/*                                          Process ChangeCipherSpec      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            cleanup for session cipher, */
/*                                            resulting in version 6.1.8  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            improved internal logic,    */
/*                                            resulting in version 6.1.11 */
/*  10-31-2022     Yanwu Cai                Modified comment(s), added    */
/*                                            custom secret generation,   */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
#define NX_SECURE_SOURCE_CODE
UINT _nx_secure_tls_session_keys_set(NX_SECURE_TLS_SESSION *tls_session, USHORT key_set)
{
UINT status;
UINT is_client;

    if (key_set == NX_SECURE_TLS_KEY_SET_LOCAL)
    {
        tls_session -> nx_secure_tls_local_session_active = 1;
    }
    else
    {
        tls_session -> nx_secure_tls_remote_session_active = 1;
    }

    /* See if we are setting server or client keys. */
    if ((key_set == NX_SECURE_TLS_KEY_SET_REMOTE && tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT) ||
        (key_set == NX_SECURE_TLS_KEY_SET_LOCAL  && tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_SERVER))
    {
        /* Setting remote keys for a client or local keys for a server: we are setting server keys. */
        is_client = NX_FALSE;
    }
    else
    {
        /* Local client/local keys or local server/remote keys. */
        is_client = NX_TRUE;
    }


    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Set client or server write key. */
    if (is_client)
    {
        status = tls_session -> nx_secure_session_keys_set(tls_session -> nx_secure_tls_session_ciphersuite, &tls_session -> nx_secure_tls_key_material,
                                                           sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_key_material_data),
                                                           is_client, &tls_session -> nx_secure_tls_session_cipher_client_initialized,
                                                           tls_session -> nx_secure_session_cipher_metadata_area_client, &tls_session -> nx_secure_session_cipher_handler_client,
                                                           tls_session -> nx_secure_session_cipher_metadata_size);
    }
    else
    {
        status = tls_session -> nx_secure_session_keys_set(tls_session -> nx_secure_tls_session_ciphersuite, &tls_session -> nx_secure_tls_key_material,
                                                           sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_key_material_data),
                                                           is_client, &tls_session -> nx_secure_tls_session_cipher_server_initialized,
                                                           tls_session -> nx_secure_session_cipher_metadata_area_server, &tls_session -> nx_secure_session_cipher_handler_server,
                                                           tls_session -> nx_secure_session_cipher_metadata_size);
    }

    return(status);
}

