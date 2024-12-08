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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_1_3_finished_hash_generate           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the Finished message hash using the hash    */
/*    data collected in the TLS session control block during the          */
/*    handshake. TLS 1.3 uses a different construction for the finished   */
/*    hash separate from the earlier versions.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    is_server                             Non-zero for server hash      */
/*    hash_size                             Return the size of the hash   */
/*    finished_hash                         Pointer to hash output buffer */
/*    available_size                        Available size of buffer      */
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
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT _nx_secure_tls_1_3_finished_hash_generate(NX_SECURE_TLS_SESSION *tls_session,
                                               UINT is_server, UINT *hash_size, UCHAR *finished_hash,
                                               ULONG available_size)
{
UINT                                  status;
const NX_CRYPTO_METHOD               *hmac_method = NX_NULL;
const NX_CRYPTO_METHOD               *hash_method = NX_NULL;
CHAR                                 *metadata = NX_NULL;
UINT                                  metadata_size;
VOID                                 *handler = NX_NULL;
UCHAR                                *finished_key = NX_NULL;
UINT                                  finished_key_len;
//UINT                                  hash_index;
UCHAR                                *transcript_hash;
NX_SECURE_TLS_KEY_SECRETS *secrets;

    /* We need to extract the Finished hash and verify that the handshake to this point
       has not been corrupted or tampered with. */

    /* From the RFC 8446 (TLS 1.3):
        finished_key = HKDF-Expand-Label(BaseKey, "finished", "", Hash.length)

        Structure of this message:

          struct {
              opaque verify_data[Hash.length];
          } Finished;

       The verify_data value is computed as follows:

          verify_data =
              HMAC(finished_key,
                   Transcript-Hash(Handshake Context,
                                   Certificate*, CertificateVerify*))
    */

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    if (available_size < tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash_size)
    {

        /* Packet buffer too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    /* Get our generated secrets. */
    secrets = &tls_session->nx_secure_tls_key_material.nx_secure_tls_key_secrets;


    /* Get our HMAC and hash methods. */
    hash_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash;
    hmac_method = tls_session -> nx_secure_tls_crypto_table->nx_secure_tls_hmac_method;
    *hash_size = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash_size;

    /* Get our Finished Hash metadata space. */
    metadata = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch;
    metadata_size = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size;    

    /* Now select the finished key (generated in _nx_secure_tls_1_3_generate_keys) based on the current request. */
    if(is_server)
    {
        finished_key = secrets->tls_server_finished_key;
        finished_key_len = secrets->tls_server_finished_key_len;
    }
    else
    {
        /* Client. */
        finished_key = secrets->tls_client_finished_key;
        finished_key_len = secrets->tls_client_finished_key_len;
    }

    /* Get the transcript hash context up to now - Server Finished received for Client, all messages before Finished sent for Server. */
    transcript_hash = tls_session->nx_secure_tls_key_material.nx_secure_tls_transcript_hashes[NX_SECURE_TLS_TRANSCRIPT_IDX_SERVER_FINISHED];

    /* Initialize HMAC. */
    if (hash_method -> nx_crypto_init)
    {
        status = hmac_method -> nx_crypto_init((NX_CRYPTO_METHOD*)hmac_method,
                                                finished_key,
                                                (finished_key_len << 3),
                                                handler,
                                                metadata,
                                                metadata_size);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* Initialize HMAC with our hash method. */
    status = hmac_method->nx_crypto_operation(NX_CRYPTO_HMAC_SET_HASH, NX_NULL, (NX_CRYPTO_METHOD*)hash_method, NX_NULL, 0, NX_NULL, 0,
                                              NX_NULL, NX_NULL, 0, metadata, metadata_size, NX_NULL, NX_NULL);

    if (status != NX_CRYPTO_SUCCESS)
    {
        return(status);
    }

    if (hmac_method -> nx_crypto_operation != NX_NULL)
    {
        status = hmac_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                                 handler,
                                                 (NX_CRYPTO_METHOD*)hmac_method,
                                                 finished_key,
                                                 (NX_CRYPTO_KEY_SIZE)(finished_key_len << 3),
                                                 NX_NULL,
                                                 0,
                                                 NX_NULL,
                                                 NX_NULL,
                                                 0,
                                                 metadata,
                                                 metadata_size,
                                                 NX_NULL,
                                                 NX_NULL);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* Update the HMAC with the transcript hash context. */
    if (hmac_method -> nx_crypto_operation != NX_NULL)
    {
        status = hmac_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                    handler,
                                                    (NX_CRYPTO_METHOD*)hmac_method,
                                                    NX_NULL, /* Key. */
                                                    0,
                                                    transcript_hash, /* Input. */
                                                    *hash_size,
                                                    NX_NULL,
                                                    NX_NULL, /* output */
                                                    0,
                                                    metadata,
                                                    metadata_size,
                                                    NX_NULL,
                                                    NX_NULL);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }

    /* Finally, calculate the Finished hash data. */
    if (hmac_method -> nx_crypto_operation != NX_NULL)
    {
        status = hmac_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                    handler,
                                                    (NX_CRYPTO_METHOD*)hmac_method,
                                                    NX_NULL,
                                                    0,
                                                    NX_NULL,
                                                    0,
                                                    NX_NULL,
                                                    finished_hash,
                                                    *hash_size,
                                                    metadata,
                                                    metadata_size,
                                                    NX_NULL,
                                                    NX_NULL);

        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }
    }
    else
    {
        return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
    }


#ifdef NX_SECURE_KEY_CLEAR
    NX_SECURE_MEMSET(finished_hash, 0, *hash_size);
#endif /* NX_SECURE_KEY_CLEAR  */

    return(NX_SUCCESS);
}
#endif
