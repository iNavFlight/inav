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
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls.h"
#endif /* NX_SECURE_ENABLE_DTLS */

/* This local static buffer needs to be large enough to hold both the server random and the client random. */
static UCHAR _nx_secure_tls_gen_keys_random[NX_SECURE_TLS_RANDOM_SIZE << 1];

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_generate_session_keys                    PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yanwu Cai, Microsoft Corporation                                    */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the session keys used by TLS to encrypt     */
/*    the data being transmitted. It uses data gathered during the TLS    */
/*    handshake to generate a block of "key material" that is split into  */
/*    the various keys needed for each session.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ciphersuite                           Selected cipher suite         */
/*    protocol_version                      Selected TLS version          */
/*    session_prf_method                    Pointer to PRF crypto method  */
/*    tls_key_material                      TLS key material              */
/*    master_sec                            Pointer to master secret      */
/*    prf_metadata                          Metadata for PRF crypto method*/
/*    prf_metadata_size                     Size of PRF metadata          */
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
/*    _nx_secure_tls_generate_keys          Generate session keys         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Yanwu Cai                Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_generate_session_keys(const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite, USHORT protocol_version,
                                      const NX_CRYPTO_METHOD *session_prf_method, NX_SECURE_TLS_KEY_MATERIAL *tls_key_material,
                                      UCHAR *master_sec, VOID *prf_metadata, ULONG prf_metadata_size)
{
UCHAR                  *key_block; /* Maximum ciphersuite key size - AES_256_CBC_SHA, 2x32 byte keys + 2x20 byte MAC secrets + 2x16 IVs. */
UINT                    key_block_size;
UINT                    key_size;
UINT                    hash_size;
UINT                    iv_size;
UINT                    status;
const NX_CRYPTO_METHOD *session_cipher_method = NX_NULL;
VOID                   *handler = NX_NULL;

    NX_PARAMETER_NOT_USED(protocol_version);

    key_block_size = 0;

    /* Working pointer into our new key material block since we are generating new keys. */
    key_block = tls_key_material -> nx_secure_tls_new_key_material_data;


    /* Get our session cipher method so we can get key sizes. */
    session_cipher_method = ciphersuite -> nx_secure_tls_session_cipher;


    /* Lookup ciphersuite data for key size. We need 2 keys for each session. */
    key_size = session_cipher_method -> nx_crypto_key_size_in_bits >> 3;

    /* Lookup ciphersuite data for hash size - used for the MAC secret. */
    hash_size = ciphersuite -> nx_secure_tls_hash_size;

    /* Lookup initialization vector size.  */
    iv_size = session_cipher_method -> nx_crypto_IV_size_in_bits >> 3;

    /* Now calculate block size. Need client and server for each key, hash, and iv. Note that
       key size and iv size may be zero depending on the ciphersuite, particularly when using
       the NULL ciphers. */
    key_block_size = 2 * (key_size + hash_size + iv_size);

    /* Now generate keys from the master secret we obtained above. */
    /* From the RFC (TLS 1.1):
       To generate the key material, compute

       key_block = PRF(SecurityParameters.master_secret,
                          "key expansion",
                          SecurityParameters.server_random +
             SecurityParameters.client_random);

       until enough output has been generated.  Then the key_block is
       partitioned as follows:

       client_write_MAC_secret[SecurityParameters.hash_size]
       server_write_MAC_secret[SecurityParameters.hash_size]
       client_write_key[SecurityParameters.key_material_length]
       server_write_key[SecurityParameters.key_material_length]
     */

    /* The order of the randoms is reversed from that used for the master secret
       when generating the key block.  */
    NX_SECURE_MEMCPY(_nx_secure_tls_gen_keys_random, tls_key_material -> nx_secure_tls_server_random,
                     NX_SECURE_TLS_RANDOM_SIZE); /* Use case of memcpy is verified. */
    NX_SECURE_MEMCPY(&_nx_secure_tls_gen_keys_random[NX_SECURE_TLS_RANDOM_SIZE],
                     tls_key_material -> nx_secure_tls_client_random, NX_SECURE_TLS_RANDOM_SIZE); /* Use case of memcpy is verified. */

    /* Key expansion uses the PRF to generate a block of key material from the master secret (generated
       above) and the client and server random values transmitted during the initial hello negotiation. */
    if (session_prf_method -> nx_crypto_init != NX_NULL)
    {
        status = session_prf_method -> nx_crypto_init((NX_CRYPTO_METHOD*)session_prf_method,
                                                      master_sec, 48,
                                                      &handler,
                                                      prf_metadata,
                                                      prf_metadata_size);

        if(status != NX_CRYPTO_SUCCESS)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(_nx_secure_tls_gen_keys_random, 0, sizeof(_nx_secure_tls_gen_keys_random));
#endif /* NX_SECURE_KEY_CLEAR  */

            return(status);
        }
    }

    if (session_prf_method -> nx_crypto_operation != NX_NULL)
    {
        status = session_prf_method -> nx_crypto_operation(NX_CRYPTO_PRF,
                                                           handler,
                                                           (NX_CRYPTO_METHOD*)session_prf_method,
                                                           (UCHAR *)"key expansion",
                                                           13,
                                                           _nx_secure_tls_gen_keys_random,
                                                           64,
                                                           NX_NULL,
                                                           key_block,
                                                           key_block_size,
                                                           prf_metadata,
                                                           prf_metadata_size,
                                                           NX_NULL,
                                                           NX_NULL);

#ifdef NX_SECURE_KEY_CLEAR
        /* We now have a key block, clear our temporary secrets buffer. */
        NX_SECURE_MEMSET(_nx_secure_tls_gen_keys_random, 0, sizeof(_nx_secure_tls_gen_keys_random));
#endif /* NX_SECURE_KEY_CLEAR  */

        if(status != NX_CRYPTO_SUCCESS)
        {
            /* Secrets cleared above. */
            return(status);
        }
    }

    if (session_prf_method -> nx_crypto_cleanup)
    {
        status = session_prf_method -> nx_crypto_cleanup(prf_metadata);

        if(status != NX_CRYPTO_SUCCESS)
        {
            /* Secrets cleared above. */
            return(status);
        }
    }

    return(NX_SECURE_TLS_SUCCESS);
}

