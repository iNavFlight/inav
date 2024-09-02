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
/*    _nx_secure_tls_metadata_size_calculate              PORTABLE C      */
/*                                                           6.1.11       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function can be used to determine the size of the buffer       */
/*    needed by TLS for encryption metadata for a given ciphersuite       */
/*    table.                                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ciphersuite_table                     Ciphersuite table to analyze  */
/*    metadata_size                         Size needed to support the    */
/*                                            given ciphersuite table     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Timothy Stapko           Modified comment(s), and      */
/*                                            improved the code,          */
/*                                            resulting in version 6.1.11 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_metadata_size_calculate(const NX_SECURE_TLS_CRYPTO *crypto_table,
                                            ULONG *metadata_size)
{

UINT                            i;
UINT                            max_public_cipher_metadata_size  = 0;
UINT                            max_public_auth_metadata_size    = 0;
UINT                            max_session_cipher_metadata_size = 0;
UINT                            max_hash_mac_metadata_size       = 0;
UINT                            max_tls_prf_metadata_size        = 0;
UINT                            max_handshake_hash_metadata_size = 0;
UINT                            max_handshake_hash_scratch_size  = 0;
NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite_table;
USHORT                          ciphersuite_table_size;
ULONG                           max_total_metadata_size;
NX_SECURE_X509_CRYPTO          *cert_crypto;
USHORT                          cert_crypto_size;


#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
const NX_CRYPTO_METHOD *crypto_method_md5;
const NX_CRYPTO_METHOD *crypto_method_sha1;
#endif

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
const NX_CRYPTO_METHOD *crypto_method_sha256;
#endif

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    crypto_method_md5 = crypto_table -> nx_secure_tls_handshake_hash_md5_method;
    crypto_method_sha1 = crypto_table -> nx_secure_tls_handshake_hash_sha1_method;
#endif
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    crypto_method_sha256 = crypto_table -> nx_secure_tls_handshake_hash_sha256_method;
#endif


    /* Get working pointers to our tables. */
    ciphersuite_table = crypto_table -> nx_secure_tls_ciphersuite_lookup_table;
    ciphersuite_table_size = crypto_table -> nx_secure_tls_ciphersuite_lookup_table_size;
    cert_crypto = crypto_table -> nx_secure_tls_x509_cipher_table;
    cert_crypto_size = crypto_table -> nx_secure_tls_x509_cipher_table_size;


    /* Loop through the ciphersuite table and find the largest metadata for each type of cipher. */
    for (i = 0; i < ciphersuite_table_size; ++i)
    {
        if (max_public_cipher_metadata_size < ciphersuite_table[i].nx_secure_tls_public_cipher -> nx_crypto_metadata_area_size)
        {
            max_public_cipher_metadata_size = ciphersuite_table[i].nx_secure_tls_public_cipher -> nx_crypto_metadata_area_size;
        }

        if (max_public_auth_metadata_size < ciphersuite_table[i].nx_secure_tls_public_auth -> nx_crypto_metadata_area_size)
        {
            max_public_auth_metadata_size = ciphersuite_table[i].nx_secure_tls_public_auth -> nx_crypto_metadata_area_size;
        }

        if (max_session_cipher_metadata_size < ciphersuite_table[i].nx_secure_tls_session_cipher -> nx_crypto_metadata_area_size)
        {
            max_session_cipher_metadata_size = ciphersuite_table[i].nx_secure_tls_session_cipher -> nx_crypto_metadata_area_size;
        }

        if (max_tls_prf_metadata_size < ciphersuite_table[i].nx_secure_tls_prf -> nx_crypto_metadata_area_size)
        {
            max_tls_prf_metadata_size = ciphersuite_table[i].nx_secure_tls_prf -> nx_crypto_metadata_area_size;
        }

        if (max_hash_mac_metadata_size < ciphersuite_table[i].nx_secure_tls_hash -> nx_crypto_metadata_area_size)
        {
            max_hash_mac_metadata_size = ciphersuite_table[i].nx_secure_tls_hash -> nx_crypto_metadata_area_size;
        }
    }

    /* Loop through the certificate cipher table as well. */
    for (i = 0; i < cert_crypto_size; ++i)
    {

        if (max_public_auth_metadata_size < cert_crypto[i].nx_secure_x509_public_cipher_method -> nx_crypto_metadata_area_size)
        {
            max_public_auth_metadata_size = cert_crypto[i].nx_secure_x509_public_cipher_method -> nx_crypto_metadata_area_size;
        }

        if (max_hash_mac_metadata_size < cert_crypto[i].nx_secure_x509_hash_method -> nx_crypto_metadata_area_size)
        {
            max_hash_mac_metadata_size = cert_crypto[i].nx_secure_x509_hash_method -> nx_crypto_metadata_area_size;
        }

        if (max_handshake_hash_scratch_size < cert_crypto[i].nx_secure_x509_hash_method -> nx_crypto_metadata_area_size)
        {
            max_handshake_hash_scratch_size = cert_crypto[i].nx_secure_x509_hash_method -> nx_crypto_metadata_area_size;
        }
    }

    /* We also need metadata space for the TLS handshake hash, so add that into the total.
       We need some scratch space to copy the handshake hash metadata during final hash generation
       so figure out the largest metadata between SHA-1+MD5 (TLSv1.0, 1.1) and SHA256 (TLSv1.2). */
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    if (crypto_method_md5 != NX_NULL && crypto_method_sha1 != NX_NULL)
    {
        max_handshake_hash_metadata_size += (crypto_method_md5 -> nx_crypto_metadata_area_size +
                                             crypto_method_sha1 -> nx_crypto_metadata_area_size);
        if (max_handshake_hash_scratch_size < crypto_method_md5 -> nx_crypto_metadata_area_size + crypto_method_sha1 -> nx_crypto_metadata_area_size)
        {
            max_handshake_hash_scratch_size = crypto_method_md5 -> nx_crypto_metadata_area_size + crypto_method_sha1 -> nx_crypto_metadata_area_size;
        }

        if (max_tls_prf_metadata_size < crypto_table -> nx_secure_tls_prf_1_method -> nx_crypto_metadata_area_size)
        {
            max_tls_prf_metadata_size = crypto_table -> nx_secure_tls_prf_1_method -> nx_crypto_metadata_area_size;
        }
    }
#endif
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    max_handshake_hash_metadata_size += crypto_method_sha256 -> nx_crypto_metadata_area_size;

    /* See if the scratch size from above is bigger. */
    if (max_handshake_hash_scratch_size < crypto_method_sha256 -> nx_crypto_metadata_area_size)
    {
        max_handshake_hash_scratch_size = crypto_method_sha256 -> nx_crypto_metadata_area_size;
    }

    if (max_tls_prf_metadata_size < crypto_table -> nx_secure_tls_prf_sha256_method -> nx_crypto_metadata_area_size)
    {
        max_tls_prf_metadata_size = crypto_table -> nx_secure_tls_prf_sha256_method -> nx_crypto_metadata_area_size;
    }
#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    max_handshake_hash_scratch_size += crypto_table -> nx_secure_tls_hmac_method -> nx_crypto_metadata_area_size;
    if (max_tls_prf_metadata_size < crypto_table -> nx_secure_tls_hkdf_method -> nx_crypto_metadata_area_size)
    {
        max_tls_prf_metadata_size = crypto_table -> nx_secure_tls_hkdf_method -> nx_crypto_metadata_area_size;
    }
#endif

    /* The public cipher and authentication should never be used simultaneously, so it should be OK
         to share their metadata areas. */
    if (max_public_cipher_metadata_size < max_public_auth_metadata_size)
    {
        max_public_cipher_metadata_size = max_public_auth_metadata_size;
    }


    /* The Total metadata size needed is the sum of all the maximums calculated above.
       We need to keep track of two separate session cipher states, one for the server and one for the client,
       so account for that extra space. */
    max_total_metadata_size = max_public_cipher_metadata_size +
                              (2 * max_session_cipher_metadata_size) +
                              max_hash_mac_metadata_size +
                              max_tls_prf_metadata_size +
                              max_handshake_hash_metadata_size +
                              max_handshake_hash_scratch_size;

    *metadata_size = max_total_metadata_size;
    return(NX_SUCCESS);
}

