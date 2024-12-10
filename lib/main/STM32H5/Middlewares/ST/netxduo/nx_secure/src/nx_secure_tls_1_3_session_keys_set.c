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
/*    _nx_secure_tls_1_3_session_keys_set                 PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the session keys for a TLS 1.3 session when      */
/*    the keys are needed. The TLS 1.3 handshake is encrypted so we have  */
/*    to switch over the client and server keys separately.               */
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
/*    [nx_crypto_init]                      Initialize crypto             */
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
/*                                                                        */
/**************************************************************************/
#define NX_SECURE_SOURCE_CODE
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT _nx_secure_tls_1_3_session_keys_set(NX_SECURE_TLS_SESSION *tls_session, USHORT key_set)
{
UINT                       key_size;
UINT                       iv_size;
UINT                                  status;
UINT                                  is_client;
const NX_CRYPTO_METHOD                     *session_cipher_method = NX_NULL;

    /* The key material should have already been generated by nx_secure_tls_1_3_generate_keys once all
     * key generation data was available. This simply switches the appropriate key data over to the active
     * key set. */

    if (key_set == NX_SECURE_TLS_KEY_SET_LOCAL)
    {
        tls_session -> nx_secure_tls_local_session_active = 1;

        /* Reset the sequence number now that we are switching to new keys. */
        NX_SECURE_MEMSET(tls_session -> nx_secure_tls_local_sequence_number, 0, sizeof(tls_session -> nx_secure_tls_local_sequence_number));
    }
    else
    {
        tls_session -> nx_secure_tls_remote_session_active = 1;

        /* Reset the sequence number now that we are switching to new keys. */
        NX_SECURE_MEMSET(tls_session -> nx_secure_tls_remote_sequence_number, 0, sizeof(tls_session -> nx_secure_tls_remote_sequence_number));

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

    /* Get our session cipher method so we can get key sizes. */
    session_cipher_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher;
    key_size = session_cipher_method -> nx_crypto_key_size_in_bits >> 3;
    /* IV size for AES-128-GCM is 12 bytes. */
    iv_size = 12; //session_cipher_method -> nx_crypto_IV_size_in_bits >> 3;

    /* Copy over the keys. Check for non-zero size in the event we are using a NULL cipher (usually for debugging). */
    if (key_size > 0)
    {
        /* Copy new client session key if setting client keys. */
        if (is_client)
        {
            NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_key,
                             tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_next_write_key, key_size); /* Use case of memcpy is verified. */
        }
        else
        {
            NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_key,
                             tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_next_write_key, key_size); /* Use case of memcpy is verified. */
        }
    }

    /* Finally, the IVs. Some ciphers don't use IV's so the iv_size may be zero. */
    if (iv_size > 0)
    {
        /* Copy new client IV if setting client keys. */
        if (is_client)
        {
            NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_iv,
                             tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_next_iv, iv_size); /* Use case of memcpy is verified. */
        }
        else
        {
            NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_iv,
                             tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_next_iv, iv_size); /* Use case of memcpy is verified. */

        }
    }

    /* Initialize the crypto method used in the session cipher. */
    if (session_cipher_method -> nx_crypto_init != NULL)
    {
        /* Set client write key. */
        if (is_client)
        {
            if (tls_session -> nx_secure_tls_session_cipher_client_initialized && session_cipher_method -> nx_crypto_cleanup)
            {
                status = session_cipher_method -> nx_crypto_cleanup(tls_session -> nx_secure_session_cipher_metadata_area_client);
                if (status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }

                tls_session -> nx_secure_tls_session_cipher_client_initialized = 0;
            }

            status = session_cipher_method -> nx_crypto_init((NX_CRYPTO_METHOD*)session_cipher_method,
                                                             tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_write_key,
                                                             session_cipher_method -> nx_crypto_key_size_in_bits,
                                                             &tls_session -> nx_secure_session_cipher_handler_client,
                                                             tls_session -> nx_secure_session_cipher_metadata_area_client,
                                                             tls_session -> nx_secure_session_cipher_metadata_size);

            tls_session -> nx_secure_tls_session_cipher_client_initialized = 1;
        }
        else
        {
            if (tls_session -> nx_secure_tls_session_cipher_server_initialized && session_cipher_method -> nx_crypto_cleanup)
            {
                status = session_cipher_method -> nx_crypto_cleanup(tls_session -> nx_secure_session_cipher_metadata_area_server);
                if (status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }

                tls_session -> nx_secure_tls_session_cipher_server_initialized = 0;
            }

            /* Set server write key. */
            status = session_cipher_method -> nx_crypto_init((NX_CRYPTO_METHOD*)session_cipher_method,
                                                             tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_write_key,
                                                             session_cipher_method -> nx_crypto_key_size_in_bits,
                                                             &tls_session -> nx_secure_session_cipher_handler_server,
                                                             tls_session -> nx_secure_session_cipher_metadata_area_server,
                                                             tls_session -> nx_secure_session_cipher_metadata_size);

            tls_session -> nx_secure_tls_session_cipher_server_initialized = 1;
        }
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    return(NX_SECURE_TLS_SUCCESS);
}
#endif
