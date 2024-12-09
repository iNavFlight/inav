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
/*    _nx_secure_tls_1_3_transcript_hash_save             PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    TLS 1.3 requires that a hash of handshake messages (the             */
/*    "transcript hash") be generated at various points to be fed into    */
/*    secrets and key generation process. This Function is used to        */
/*    indicate when a hash needs to be generated, and what that hash is   */
/*    stored as (e.g. ClientHello transcript hash).                       */
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
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
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
/*                                            hash clone and cleanup,     */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

UINT _nx_secure_tls_1_3_transcript_hash_save(NX_SECURE_TLS_SESSION *tls_session, UINT hash_index, UINT need_copy)
{
UINT status = NX_NOT_SUCCESSFUL;
const NX_CRYPTO_METHOD *method_ptr;
UINT  hash_size;
CHAR *metadata;


    /* Don't oveflow the hash array. */
    if(hash_index > NX_SECURE_TLS_1_3_MAX_TRANSCRIPT_HASHES)
    {
        return(NX_INVALID_PARAMETERS);
    }

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

    /* If nx_secure_tls_handshake_hash_sha256_metadata can't be modified for it will be used later, copy it to scratch buffer. */
    if (need_copy)
    {

        /* Copy over the handshake hash state into a local structure to do the intermediate calculation. */
        NX_SECURE_HASH_METADATA_CLONE(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size); /* Use case of memcpy is verified. */

        metadata = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch;
    }
    else
    {
        metadata = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata;
    }

    /* Get hash output size. */
    hash_size = (method_ptr ->nx_crypto_ICV_size_in_bits >> 3);


    /* Generate a hash using our temporary copy of the hash metadata, place it into the TLS Session transcript hash array. */
    if (method_ptr  -> nx_crypto_operation != NX_NULL)
    {
        status = method_ptr  -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_handler,
                                           (NX_CRYPTO_METHOD*)method_ptr,
                                           NX_NULL,
                                           0,
                                           NX_NULL,
                                           0,
                                           NX_NULL,
                                           &tls_session->nx_secure_tls_key_material.nx_secure_tls_transcript_hashes[hash_index][0],
                                           hash_size,
                                           metadata,
                                           tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size,
                                           NX_NULL,
                                           NX_NULL);
    }

    if (need_copy)
    {
        NX_SECURE_HASH_CLONE_CLEANUP(metadata, tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size);
    }

    return(status);
}

#endif
