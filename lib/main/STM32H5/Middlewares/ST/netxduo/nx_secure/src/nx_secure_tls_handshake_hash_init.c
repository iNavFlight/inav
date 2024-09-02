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
/*    _nx_secure_tls_handshake_hash_init                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes the hash function states needed for the   */
/*    TLS Finished message handshake hash.                                */
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
/*    [nx_crypto_operation]                 Hash initialization functions */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_send_clienthello      Send ClientHello              */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_tls_send_clienthello       Send ClientHello              */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
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
UINT  _nx_secure_tls_handshake_hash_init(NX_SECURE_TLS_SESSION *tls_session)
{
const NX_CRYPTO_METHOD *method_ptr;
UINT              status;
VOID             *handler = NX_NULL;
VOID             *metadata;
UINT              metadata_size;


    /* We need to hash all of the handshake messages that we receive and send. When sending a ClientHello,
       we need to initialize the hashes (TLS 1.1 uses both MD5 and SHA-1, TLS 1.2 uses SHA-256). The final
       hash is generated in the "Finished" message.
       TLS 1.3 does things a little differently - the handshake hash is the same as that of the chosen
       ciphersuite, so */

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if(tls_session->nx_secure_tls_1_3)
    {
        /* Hash handshake record using ciphersuite hash routine. */
        if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
        {
            /* Set the hash method to the default of SHA-256 if no ciphersuite is available. */
            method_ptr = tls_session -> nx_secure_tls_crypto_table->nx_secure_tls_handshake_hash_sha256_method;

        }
        else
        {
            /* The handshake transcript hash in TLS 1.3 uses the hash routine associated with the chosen ciphersuite. */
            method_ptr = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash;
        }

        /* Generate the "transcript hash" for the point in the handshake where this message falls.
         * This is needed for TLS 1.3 key generation which uses the hash of all previous handshake
         * messages at multiple points during the handshake. Instead of saving the entirety of the
         * handshake messages, just generate a hash when each record is hashed. */


        /* The handshake transcript hash in TLS 1.3 uses the hash routine associated with the chosen ciphersuite. */
        metadata = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata;
        metadata_size = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size;

        if (method_ptr -> nx_crypto_init)
        {
            status = method_ptr -> nx_crypto_init((NX_CRYPTO_METHOD*)method_ptr,
                                         NX_NULL,
                                         0,
                                         &handler,
                                         metadata,
                                         metadata_size);
            tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_handler = handler;

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }
        }

        if (method_ptr -> nx_crypto_operation != NX_NULL)
        {
            status = method_ptr -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                              handler,
                                              (NX_CRYPTO_METHOD*)method_ptr,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              NX_NULL,
                                              0,
                                              metadata,
                                              metadata_size,
                                              NX_NULL,
                                              NX_NULL);
        }

        return(status);

    }
#endif



    /* Initialize both the handshake "finished" hashes - TLS 1.1 uses both SHA-1 and MD5, TLS 1.2 uses SHA-256 by default.
       At this point we don't yet know the version we will use, so initialize all of them. */
    /* Hash is determined by ciphersuite in TLS 1.2. Default is SHA-256. */
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    if (tls_session -> nx_secure_tls_supported_versions & (USHORT)(NX_SECURE_TLS_BITFIELD_VERSION_1_2))
    {
        method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha256_method;
        metadata = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata;
        metadata_size = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size;

        if (method_ptr -> nx_crypto_init)
        {
            status = method_ptr -> nx_crypto_init((NX_CRYPTO_METHOD*)method_ptr,
                                         NX_NULL,
                                         0,
                                         &handler,
                                         metadata,
                                         metadata_size);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }                                                     
            tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_handler = handler;
        }

        if (method_ptr -> nx_crypto_operation != NX_NULL)
        {
            status = method_ptr -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                              handler,
                                              (NX_CRYPTO_METHOD*)method_ptr,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              NX_NULL,
                                              0,
                                              metadata,
                                              metadata_size,
                                              NX_NULL,
                                              NX_NULL);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }                                                     
        }
    }
#endif

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    if (tls_session -> nx_secure_tls_supported_versions & (USHORT)(NX_SECURE_TLS_BITFIELD_VERSION_1_0 | NX_SECURE_TLS_BITFIELD_VERSION_1_1))
    {
        /* TLS 1.0 and 1.1 use both MD5 and SHA-1. */
        method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_md5_method;
        metadata = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata;
        metadata_size = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata_size;

        if (method_ptr -> nx_crypto_init)
        {
            status = method_ptr -> nx_crypto_init((NX_CRYPTO_METHOD*)method_ptr,
                                         NX_NULL,
                                         0,
                                         &handler,
                                         metadata,
                                         metadata_size);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }                                                     

            tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_handler = handler;
        }

        if (method_ptr -> nx_crypto_operation != NX_NULL)
        {
            status = method_ptr -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                              handler,
                                              (NX_CRYPTO_METHOD*)method_ptr,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              NX_NULL,
                                              0,
                                              metadata,
                                              metadata_size,
                                              NX_NULL,
                                              NX_NULL);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }                                                     
        }

        method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha1_method;
        metadata = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata;
        metadata_size = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata_size;

        if (method_ptr -> nx_crypto_init)
        {
            status = method_ptr -> nx_crypto_init((NX_CRYPTO_METHOD*)method_ptr,
                                         NX_NULL,
                                         0,
                                         &handler,
                                         metadata,
                                         metadata_size);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }                                                     
            tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_handler = handler;
        }

        if (method_ptr -> nx_crypto_operation != NX_NULL)
        {
            status = method_ptr -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                              handler,
                                              (NX_CRYPTO_METHOD*)method_ptr,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              NX_NULL,
                                              0,
                                              metadata,
                                              metadata_size,
                                              NX_NULL,
                                              NX_NULL);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }                                                     
        }
    }
#endif

    return(NX_SUCCESS);
}

