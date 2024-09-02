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


#if 0
// Utility function to print buffer
static void print_buffer(const char* buf_name, const UCHAR* buf, ULONG size)
{
UINT i;
    printf("Buffer %s of size: %ld. Data:\n", buf_name, size);
    if(buf)
    {
        for(i = 0; i < size; ++i)
        {
            printf("%02x ", (UINT)buf[i]);
            if((i+1) % 8 == 0)
            {
                printf("\n");
            }
        }
    }
    else
    {
        printf("NULL buffer passed as number\n");
    }
    printf("\n");
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_psk_binder_generate                  PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the PSK binder, a hash that is used to      */
/*    associate a PSK with a specific session/handshake. The PSK binder   */
/*    concept was introduced in TLS 1.3.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    psk_entry                             Pointer to PSK entry          */
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
/*    _nx_secure_tls_send_clienthello_extensions                          */
/*                                          Send ClientHello extensions   */
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
UINT _nx_secure_tls_psk_binder_generate(NX_SECURE_TLS_SESSION *tls_session, NX_SECURE_TLS_PSK_STORE *psk_entry)
{
UINT                                  status = NX_SUCCESS;
const NX_CRYPTO_METHOD               *hmac_method = NX_NULL;
const NX_CRYPTO_METHOD               *hash_method = NX_NULL;
CHAR                                 *metadata = NX_NULL;
UINT                                  metadata_size;
VOID                                 *handler = NX_NULL;
UCHAR                                *binder_key = NX_NULL;
UINT                                  binder_key_len;
UINT                                  hash_size;
UCHAR                                *transcript_hash;
UCHAR                                *psk_binder;

    /* From RFC 8446 (TLS 1.3, section 4.2.11.2):
       The PSK binder value forms a binding between a PSK and the current
       handshake, as well as a binding between the handshake in which the
       PSK was generated (if via a NewSessionTicket message) and the current
       handshake.  Each entry in the binders list is computed as an HMAC
       over a transcript hash (see Section 4.4.1) containing a partial
       ClientHello up to and including the PreSharedKeyExtension.identities
       field.  That is, it includes all of the ClientHello but not the
       binders list itself.  The length fields for the message (including
       the overall length, the length of the extensions block, and the
       length of the "pre_shared_key" extension) are all set as if binders
       of the correct lengths were present.

       The PskBinderEntry is computed in the same way as the Finished
       message (Section 4.4.4) but with the BaseKey being the binder_key
       derived via the key schedule from the corresponding PSK which is
       being offered (see Section 7.1).

       If the handshake includes a HelloRetryRequest, the initial
       ClientHello and HelloRetryRequest are included in the transcript
       along with the new ClientHello.  For instance, if the client sends
       ClientHello1, its binder will be computed over:

          Transcript-Hash(Truncate(ClientHello1))

       Where Truncate() removes the binders list from the ClientHello.

       If the server responds with a HelloRetryRequest and the client then
       sends ClientHello2, its binder will be computed over:

          Transcript-Hash(ClientHello1,
                          HelloRetryRequest,
                          Truncate(ClientHello2))

       The full ClientHello1/ClientHello2 is included in all other handshake
       hash computations.  Note that in the first flight,
       Truncate(ClientHello1) is hashed directly, but in the second flight,
       ClientHello1 is hashed and then reinjected as a "message_hash"
       message, as described in Section 4.4.1.

     For reference, the Finished message calculation is as follows :

          verify_data =
              HMAC(finished_key,
                   Transcript-Hash(Handshake Context,
                                   Certificate*, CertificateVerify*))
    */



    /* Get our HMAC and hash methods. */

    /* If the PSK binder ciphersuite is not NULL, then we have a previously-associated hash method or ciphersuite.
       This may be a session-resumption PSK or a user-defined hash routine/ciphersuite, but if it exists, use it. */
    if(psk_entry->nx_secure_tls_psk_binder_ciphersuite != NX_NULL)
    {
        hash_method = psk_entry->nx_secure_tls_psk_binder_ciphersuite->nx_secure_tls_hash;
        hash_size = psk_entry->nx_secure_tls_psk_binder_ciphersuite->nx_secure_tls_hash_size;
    }
    else
    {
        /* Default to SHA-256 if no ciphersuite is present. */
        hash_method = tls_session->nx_secure_tls_crypto_table->nx_secure_tls_handshake_hash_sha256_method;
        hash_size = (hash_method->nx_crypto_ICV_size_in_bits >> 3);
    }
    hmac_method = tls_session->nx_secure_tls_crypto_table->nx_secure_tls_hmac_method;


    /* Generate our binder key using our hash method and PSK. */
    status = _nx_secure_tls_1_3_generate_psk_secret(tls_session, psk_entry, hash_method);

    if(status != NX_SUCCESS)
    {
        return(status);
    }

    binder_key = psk_entry->nx_secure_tls_psk_finished_key;
    binder_key_len = psk_entry->nx_secure_tls_psk_finished_key_size;


//    printf("\n=====BINDER GENERATION=========\n");
//    print_buffer("Finished binder key", binder_key, binder_key_len);
//    printf("\n==============\n");

    /* Working pointer to PSK binder output. */
    psk_binder = &psk_entry->nx_secure_tls_psk_binder[0];

    /* Use the Finished Hash metadata scratch space to calculate our binder hash. */
    metadata = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch;
    metadata_size = tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch_size;    

    /* Get the transcript hash context up to now - Server Finished received for Client, all messages before Finished sent for Server. */
    transcript_hash = tls_session->nx_secure_tls_key_material.nx_secure_tls_transcript_hashes[NX_SECURE_TLS_TRANSCRIPT_IDX_CLIENTHELLO];

//    print_buffer("transcript hash value", transcript_hash, hash_size);
//    printf("\n==============\n");

    /* Initialize HMAC. */
    if (hash_method -> nx_crypto_init)
    {
        status = hmac_method -> nx_crypto_init((NX_CRYPTO_METHOD*)hmac_method,
                                                binder_key,
                                                (binder_key_len << 3),
                                                handler,
                                                metadata,
                                                metadata_size);

        if (status != NX_SUCCESS)
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
                                                 binder_key,
                                                 (NX_CRYPTO_KEY_SIZE)(binder_key_len << 3),
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
                                                    hash_size,
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

    /* Finally, calculate the PSK binder data. */
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
                                                    psk_binder,
                                                    hash_size,
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


//    print_buffer("psk binder", psk_binder, hash_size);
//    printf("\n==============\n");

    /* Finally, set the size of the PSK binder. */
    psk_entry->nx_secure_tls_psk_binder_size = hash_size;

    return(status);
}
#endif
