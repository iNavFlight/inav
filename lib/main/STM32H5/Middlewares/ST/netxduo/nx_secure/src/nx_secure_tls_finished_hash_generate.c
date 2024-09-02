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
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls.h"
#endif /* NX_SECURE_ENABLE_DTLS */

static UCHAR handshake_hash[16 + 20]; /* We concatenate MD5 and SHA-1 hashes into this buffer. */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_finished_hash_generate               PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the Finished message hash using the hash    */
/*    data collected in the TLS session control block during the          */
/*    handshake.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    finished_label                        Label used to generate hash   */
/*    finished_hash                         Pointer to hash output buffer */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_init]                      Initialize crypto             */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_send_finished          Send Finished message         */
/*    _nx_secure_tls_process_finished       Process Finished message      */
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
UINT _nx_secure_tls_finished_hash_generate(NX_SECURE_TLS_SESSION *tls_session,
                                           UCHAR *finished_label, UCHAR *finished_hash)
{
UINT                                  status;
UCHAR                                *master_sec;
const NX_CRYPTO_METHOD                     *method_ptr = NX_NULL;
VOID                                 *handler = NX_NULL;
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
VOID                                 *metadata;
UINT                                  metadata_size;
#endif /* (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED) */
UINT                                  hash_size = 0;

    /* We need to extract the Finished hash and verify that the handshake to this point
       has not been corrupted or tampered with. */

    /* From the RFC (TLS 1.1). TLS 1.2 uses the same strategy but uses only SHA-256.
       struct {
             opaque verify_data[12];
         } Finished;

         verify_data
             PRF(master_secret, finished_label, MD5(handshake_messages) +
             SHA-1(handshake_messages)) [0..11];

         finished_label
             For Finished messages sent by the client, the string "client
             finished".  For Finished messages sent by the server, the
             string "server finished".

         handshake_messages
             All of the data from all messages in this handshake (not
             including any HelloRequest messages) up to but not including
             this message.  This is only data visible at the handshake
             layer and does not include record layer headers.  This is the
             concatenation of all the Handshake structures, as defined in
             7.4, exchanged thus far.
     */

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }


    /* Get our master secret that was generated when we generated our key material. */
    master_sec = tls_session -> nx_secure_tls_key_material.nx_secure_tls_master_secret;

    /* Finally, generate the verification data required by TLS - 12 bytes using the PRF and the data
       we have collected. Place the result directly into the packet buffer. */
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_2)
#else
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
#endif /* NX_SECURE_ENABLE_DTLS */
    {
        /* Copy over the handshake hash state into scratch space to do the intermediate calculation. */
    NX_SECURE_HASH_METADATA_CLONE(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch, /* lgtm[cpp/banned-api-usage-required-any] */
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size); /* Use case of memcpy is verified. */

        /* Finalize the handshake message hash that we started at the beginning of the handshake. */
        method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha256_method;
        handler = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_handler;

        if (method_ptr -> nx_crypto_operation != NX_NULL)
        {
            status = method_ptr -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                              handler,
                                              (NX_CRYPTO_METHOD*)method_ptr,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              &handshake_hash[0],
                                              sizeof(handshake_hash),
                                              tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                              tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size,
                                              NX_NULL,
                                              NX_NULL);

        }
        else
        {
            status = NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE;
        }

        NX_SECURE_HASH_CLONE_CLEANUP(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                     tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                     

        /* For TLS 1.2, the PRF is defined by the ciphersuite. However, if we are using an older ciphersuite,
         * default to the TLS 1.2 default PRF, which uses SHA-256-HMAC. */
        method_ptr = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_prf;
        hash_size = 32; /* Size of SHA-256 output. */
    }

#endif

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_0)
#else
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1)
#endif /* NX_SECURE_ENABLE_DTLS */
    {
        /* Copy over the handshake hash metadata into scratch metadata area to do the intermediate calculation.  */
        NX_SECURE_HASH_METADATA_CLONE(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch +
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata_size,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata_size); /* Use case of memcpy is verified. */

        /* Finalize the handshake message hashes that we started at the beginning of the handshake. */
        method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_md5_method;
        handler = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_handler;
        metadata = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch +
                   tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata_size;
        metadata_size = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_md5_metadata_size;
        if (method_ptr -> nx_crypto_operation != NX_NULL)
        {
            status = method_ptr -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                              handler,
                                              (NX_CRYPTO_METHOD*)method_ptr,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              &handshake_hash[0],
                                              16,
                                              metadata,
                                              metadata_size,
                                              NX_NULL,
                                              NX_NULL);
        }
        else
        {
            status = NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE;
        }

        NX_SECURE_HASH_CLONE_CLEANUP(metadata, metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        NX_SECURE_HASH_METADATA_CLONE(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata_size); /* Use case of memcpy is verified. */

        method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha1_method;
        handler = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_handler;
        metadata = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch;
        metadata_size = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha1_metadata_size;

        if (method_ptr -> nx_crypto_operation != NX_NULL)
        {
            status = method_ptr -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                              handler,
                                              (NX_CRYPTO_METHOD*)method_ptr,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              0,
                                              NX_NULL,
                                              &handshake_hash[16],
                                              sizeof(handshake_hash) - 16,
                                              metadata,
                                              metadata_size,
                                              NX_NULL,
                                              NX_NULL);


        }
        else
        {
            status = NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE;
        }

        NX_SECURE_HASH_CLONE_CLEANUP(metadata, metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        /* TLS 1.0 and TLS 1.1 use the same PRF. */
        method_ptr = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_prf_1_method;
        hash_size = 36;
    }
#endif

    /* Make sure we found a supported version (essentially an assertion check). */
    if (method_ptr == NX_NULL)
    {
        return(NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION);
    }

    /* Do the final PRF encoding of the finished hash. */
    if (method_ptr -> nx_crypto_init != NX_NULL)
    {
        status = method_ptr -> nx_crypto_init((NX_CRYPTO_METHOD*)method_ptr,
                                     master_sec, 48,
                                     &handler,
                                     tls_session -> nx_secure_tls_prf_metadata_area,
                                     tls_session -> nx_secure_tls_prf_metadata_size);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                     
    }
    else
    {
        return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
    }

    if (method_ptr -> nx_crypto_operation != NX_NULL)
    {
        status = method_ptr -> nx_crypto_operation(NX_CRYPTO_PRF,
                                          handler,
                                          (NX_CRYPTO_METHOD*)method_ptr,
                                          finished_label,
                                          15,
                                          handshake_hash,
                                          hash_size,
                                          NX_NULL,
                                          &finished_hash[0],
                                          NX_SECURE_TLS_FINISHED_HASH_SIZE,
                                          tls_session -> nx_secure_tls_prf_metadata_area,
                                          tls_session -> nx_secure_tls_prf_metadata_size,
                                          NX_NULL,
                                          NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                     
    }
    else
    {
        return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
    }

    if (method_ptr -> nx_crypto_cleanup != NX_NULL)
    {
        status = method_ptr -> nx_crypto_cleanup(tls_session -> nx_secure_tls_prf_metadata_area);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }                                                     
    }
#ifdef NX_SECURE_KEY_CLEAR
    NX_SECURE_MEMSET(handshake_hash, 0, hash_size);
#endif /* NX_SECURE_KEY_CLEAR  */

    return(NX_SUCCESS);
}

