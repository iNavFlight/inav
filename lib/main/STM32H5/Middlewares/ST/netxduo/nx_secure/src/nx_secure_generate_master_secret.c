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
/*    _nx_secure_generate_master_secret                   PORTABLE C      */
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
/*    pre_master_sec                        Pointer to pre-master secret  */
/*    pre_master_sec_size                   Size of pre-master secret     */
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
UINT _nx_secure_generate_master_secret(const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite, USHORT protocol_version,
                                       const NX_CRYPTO_METHOD *session_prf_method, NX_SECURE_TLS_KEY_MATERIAL *tls_key_material,
                                       UCHAR *pre_master_sec, UINT pre_master_sec_size, UCHAR *master_sec,
                                       VOID *prf_metadata, ULONG prf_metadata_size)
{
;
UINT  status;
VOID *handler = NX_NULL;

    NX_PARAMETER_NOT_USED(protocol_version);

    /* Generate the session keys using the parameters obtained in the handshake.
       By this point all the information needed to generate the TLS session key
       material should have been gathered and stored in the TLS socket structure. */

    /* Generate the Master Secret from the Pre-Master Secret.
       From the RFC (TLS 1.1, TLS 1.2):

        master_secret = PRF(pre_master_secret, "master secret",
                        ClientHello.random + ServerHello.random) [0..47];

        The master secret is always exactly 48 bytes in length.  The length
        of the premaster secret will vary depending on key exchange method.
     */

    /* The generation of key material is different between RSA and DH. */
    if (ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
        || ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDHE
        || ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDH
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
        || ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_PSK
#endif
#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
        || ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE
#endif
        )
    {
        /* Concatenate random values to feed into PRF. */
        NX_SECURE_MEMCPY(_nx_secure_tls_gen_keys_random, tls_key_material -> nx_secure_tls_client_random,
                         NX_SECURE_TLS_RANDOM_SIZE); /* Use case of memcpy is verified. */
        NX_SECURE_MEMCPY(&_nx_secure_tls_gen_keys_random[NX_SECURE_TLS_RANDOM_SIZE],
                         tls_key_material -> nx_secure_tls_server_random, NX_SECURE_TLS_RANDOM_SIZE); /* Use case of memcpy is verified. */

        /* Generate the master secret using the pre-master secret, the defined TLS label, and the concatenated
           random values. */

        /* If we don't have a PRF method, the version is wrong! */
        if (session_prf_method == NX_NULL)
        {
#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(_nx_secure_tls_gen_keys_random, 0, sizeof(_nx_secure_tls_gen_keys_random));
#endif /* NX_SECURE_KEY_CLEAR  */

            return(NX_SECURE_TLS_PROTOCOL_VERSION_CHANGED);
        }

        /* Use the PRF to generate the master secret. */
        if (session_prf_method -> nx_crypto_init != NX_NULL)
        {
            status = session_prf_method -> nx_crypto_init((NX_CRYPTO_METHOD*)session_prf_method,
                                                 pre_master_sec, (NX_CRYPTO_KEY_SIZE)pre_master_sec_size,
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
                                                      (UCHAR *)"master secret",
                                                      13,
                                                      _nx_secure_tls_gen_keys_random,
                                                      64,
                                                      NX_NULL,
                                                      master_sec,
                                                      48,
                                                      prf_metadata,
                                                      prf_metadata_size,
                                                      NX_NULL,
                                                      NX_NULL);

#ifdef NX_SECURE_KEY_CLEAR
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
                /* All secrets cleared above. */
                return(status);
            }
        }
    }
    else
    {
        /* The chosen cipher is not supported. Likely an internal error since negotiation has already finished. */
        return(NX_SECURE_TLS_UNSUPPORTED_CIPHER);
    }

    return(NX_SECURE_TLS_SUCCESS);
}

