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
/*    _nx_secure_tls_session_reset                        PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function resets a TLS session object, clearing out all data    */
/*    for initialization or re-use.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_key_material_init      Clear TLS key material        */
/*    _nx_secure_tls_remote_certificate_free_all                          */
/*                                          Free all remote certificates  */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_secure_dtls_session_reset         Clear out the session         */
/*    _nx_secure_tls_session_create         Create the TLS session        */
/*    _nx_secure_tls_session_delete         Delete the TLS session        */
/*    _nx_secure_tls_session_end            End of a session              */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            cleanup for session cipher, */
/*                                            resulting in version 6.1.8  */
/*  10-15-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            option to disable client    */
/*                                            initiated renegotiation,    */
/*                                            resulting in version 6.1.9  */
/*  10-31-2022     Yanwu Cai                Modified comment(s), and      */
/*                                            fixed renegotiation when    */
/*                                            receiving in non-block mode,*/
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_session_reset(NX_SECURE_TLS_SESSION *session_ptr)
{
UINT status;
UINT temp_status;

    status = NX_SUCCESS;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Reset all state to bring the TLS socket back to an initial state, leaving
     * it as it was when created, but leaving certain items alone:
     * - packet pool
     * - local and trusted certificates
     * - callback functions
     * - crypto table and metadata
     *
     * Remote certificates must be freed (placed back into free store)
     */

    if (session_ptr -> nx_secure_tls_remote_session_active)
    {
        if (session_ptr -> nx_secure_tls_session_ciphersuite != NX_NULL)
        {
            if (session_ptr -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher -> nx_crypto_cleanup)
            {
                temp_status = session_ptr -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher -> nx_crypto_cleanup(session_ptr -> nx_secure_session_cipher_metadata_area_client);
                if(temp_status != NX_CRYPTO_SUCCESS)
                {
                    status = temp_status;
                }

                temp_status = session_ptr -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_session_cipher -> nx_crypto_cleanup(session_ptr -> nx_secure_session_cipher_metadata_area_server);
                if(temp_status != NX_CRYPTO_SUCCESS)
                {
                    status = temp_status;
                }

            }
        }
    }

    /* Reset socket type. */
    session_ptr -> nx_secure_tls_socket_type = NX_SECURE_TLS_SESSION_TYPE_NONE;

    /* Clear out the protocol version - assigned during the TLS handshake. */
    session_ptr -> nx_secure_tls_protocol_version = 0;


    /* Sessions are not active when we start the socket. */
    session_ptr -> nx_secure_tls_remote_session_active = 0;
    session_ptr -> nx_secure_tls_local_session_active = 0;
    session_ptr -> nx_secure_tls_session_cipher_client_initialized = 0;
    session_ptr -> nx_secure_tls_session_cipher_server_initialized = 0;

    /* Set the current ciphersuite to TLS_NULL_WITH_NULL_NULL which is the
    * specified ciphersuite for the handshake (pre-change cipher spec). */
    session_ptr -> nx_secure_tls_session_ciphersuite = NX_NULL;

    /* Initialize key material structure. */
    _nx_secure_tls_key_material_init(&session_ptr -> nx_secure_tls_key_material);

    /* Session ID length. Initialize to 0 - will be assigned during handshake. */
    session_ptr -> nx_secure_tls_session_id_length = 0;

    /* Clear out Session ID used for session re-negotiation. */
    NX_SECURE_MEMSET(session_ptr -> nx_secure_tls_session_id, 0, NX_SECURE_TLS_SESSION_ID_SIZE);

    /* Clear out sequence numbers for the current TLS session. */
    NX_SECURE_MEMSET(session_ptr -> nx_secure_tls_local_sequence_number, 0, sizeof(session_ptr -> nx_secure_tls_local_sequence_number));
    NX_SECURE_MEMSET(session_ptr -> nx_secure_tls_remote_sequence_number, 0, sizeof(session_ptr -> nx_secure_tls_remote_sequence_number));

    /* Clear out all remote certificates. */
    status = _nx_secure_tls_remote_certificate_free_all(session_ptr);

    /* Clear out the active certificate so if the session is reused it will return to the default (local cert). */
    session_ptr -> nx_secure_tls_credentials.nx_secure_tls_active_certificate = NX_NULL;


#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
    session_ptr -> nx_secure_tls_secure_renegotiation = NX_FALSE;

    NX_SECURE_MEMSET(session_ptr -> nx_secure_tls_remote_verify_data, 0, NX_SECURE_TLS_FINISHED_HASH_SIZE);
    NX_SECURE_MEMSET(session_ptr -> nx_secure_tls_local_verify_data, 0, NX_SECURE_TLS_FINISHED_HASH_SIZE);

    /* Flag to indicate when a session renegotiation is taking place. */
    session_ptr -> nx_secure_tls_renegotiation_handshake = NX_FALSE;
    session_ptr -> nx_secure_tls_secure_renegotiation_verified = NX_FALSE;
    session_ptr -> nx_secure_tls_server_renegotiation_requested = NX_FALSE;
    session_ptr -> nx_secure_tls_local_initiated_renegotiation = NX_FALSE;
#endif

    /* Flag to indicate when credentials have been received from the remote host. */
    session_ptr -> nx_secure_tls_received_remote_credentials = NX_FALSE;

#ifndef NX_SECURE_TLS_SERVER_DISABLED
    /* The state of the server handshake if this is a server socket. */
    session_ptr -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_IDLE;
#endif

#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    /* The state of the client handshake if this is a client socket. */
    session_ptr -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_IDLE;
#endif

    /* Indicate no messages to be hashed. */
    session_ptr -> nx_secure_tls_key_material.nx_secure_tls_handshake_cache_length = 0;

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    /* Reset TLS 1.3 state. */
    session_ptr -> nx_secure_tls_1_3 = session_ptr -> nx_secure_tls_1_3_supported;
#endif

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
}

