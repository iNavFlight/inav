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


/* Include necessary system files.  */

#include "nx_secure_crypto_table_self_test.h"


#ifdef NX_SECURE_POWER_ON_SELF_TEST_MODULE_INTEGRITY_CHECK

static const NX_SECURE_CRYPTO_SELF_TEST nx_crypto_self_test_map[] =
{
    {NX_CRYPTO_ENCRYPTION_AES_CBC, _nx_secure_crypto_method_self_test_aes},
    {NX_CRYPTO_ENCRYPTION_AES_CTR, _nx_secure_crypto_method_self_test_aes},
    {NX_CRYPTO_ENCRYPTION_AES_GCM_16, _nx_secure_crypto_method_self_test_aes},
    {NX_CRYPTO_ENCRYPTION_DES_CBC, _nx_secure_crypto_method_self_test_des},
    {NX_CRYPTO_ENCRYPTION_3DES_CBC, _nx_secure_crypto_method_self_test_3des},
    {NX_CRYPTO_HASH_SHA1, _nx_secure_crypto_method_self_test_sha},
    {NX_CRYPTO_HASH_SHA256, _nx_secure_crypto_method_self_test_sha},
    {NX_CRYPTO_HASH_SHA384, _nx_secure_crypto_method_self_test_sha},
    {NX_CRYPTO_HASH_SHA512, _nx_secure_crypto_method_self_test_sha},
    {NX_CRYPTO_HASH_MD5, _nx_secure_crypto_method_self_test_md5},
    {NX_CRYPTO_KEY_EXCHANGE_RSA, _nx_secure_crypto_method_self_test_rsa},
    {NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96, _nx_secure_crypto_method_self_test_hmac_sha},
    {NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_160, _nx_secure_crypto_method_self_test_hmac_sha},
    {NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256, _nx_secure_crypto_method_self_test_hmac_sha},
    {NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_384, _nx_secure_crypto_method_self_test_hmac_sha},
    {NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512, _nx_secure_crypto_method_self_test_hmac_sha},
    {NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96, _nx_secure_crypto_method_self_test_hmac_md5},
    {NX_CRYPTO_AUTHENTICATION_HMAC_MD5_128, _nx_secure_crypto_method_self_test_hmac_md5},
    {NX_CRYPTO_PRF_HMAC_SHA1, _nx_secure_crypto_method_self_test_prf},
    {NX_CRYPTO_PRF_HMAC_SHA2_256, _nx_secure_crypto_method_self_test_prf},
};

/* Define an array to avoid duplicate self testing for identical crypto method. */
static const NX_CRYPTO_METHOD *nx_crypto_self_test_methods[sizeof(nx_crypto_self_test_map) / sizeof(NX_SECURE_CRYPTO_SELF_TEST)];

UINT _nx_secure_crypto_method_self_test(const NX_CRYPTO_METHOD *crypto_method,
                                        VOID *metadata, UINT metadata_size);

#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_crypto_table_self_test                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function runs through the TLS crypto table self test.          */
/*    The self test is in the form of Known Answer Test.                  */
/*    NetX Secure provides a number of vectors that can be used to test   */
/*    crypto methods supported by NetX Secure.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    crypto_table                          Pointer to the crypto method  */
/*                                            table to be tested          */
/*    metadata_buffer                       Crypto metadata area          */
/*    metadata_size                         crypto metadata size          */
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
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_crypto_table_self_test(const NX_SECURE_TLS_CRYPTO *crypto_table,
                                       VOID *metadata, UINT metadata_size)
{
#ifdef NX_SECURE_POWER_ON_SELF_TEST_MODULE_INTEGRITY_CHECK
UINT i;
UINT crypto_table_size;
NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite;
#ifndef NX_SECURE_DISABLE_X509
NX_SECURE_X509_CRYPTO *x509_crypto;
#endif
UINT status = NX_SECURE_TLS_SUCCESS;

    /* Validate the crypto method */
    if(crypto_table == NX_NULL)
        return(NX_PTR_ERROR);

    /* Initialize the crypto method list that is tested. */
    NX_CRYPTO_MEMSET(nx_crypto_self_test_methods, 0, sizeof(nx_crypto_self_test_methods));

    /* Go through our internal Known Answer Test table, and run through the matching
       test. */
    /* For now we shall be able to test the following algorithms, based on the value in
       nx_crypto_algorithm:
       NX_CRYPTO_KEY_EXCHANGE_RSA (1024/2048/4096 bit key)
       NX_CRYPTO_ENCRYPTION_DES_CBC
       NX_CRYPTO_ENCRYPTION_3DES_CBC
       NX_CRYPTO_ENCRYPTION_AES_CBC (check key_size field)
       NX_CRYPTO_ENCRYPTION_AES_CTR
       NX_CRYPTO_HASH_SHA1
       NX_CRYPTO_HASH_SHA256
       NX_CRYPTO_HASH_SHA384
       NX_CRYPTO_HASH_SHA512
       NX_CRYPTO_HASH_MD5
       NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96
       NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_160
       NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256
       NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_384
       NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512
       NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96
       NX_CRYPTO_AUTHENTICATION_HMAC_MD5_128
       NX_CRYPTO_PRF_HMAC_SHA1
       NX_CRYPTO_PRF_HMAC_SHA2_256
    */

    crypto_table_size = crypto_table -> nx_secure_tls_ciphersuite_lookup_table_size;

    /* Loop through cipher table and perform self tests. */
    for (i = 0; i < crypto_table_size; ++i)
    {
        ciphersuite = &crypto_table -> nx_secure_tls_ciphersuite_lookup_table[i];
        status = _nx_secure_crypto_method_self_test(ciphersuite -> nx_secure_tls_public_cipher, metadata, metadata_size);
        if (status)
        {
            return(status);
        }

        status = _nx_secure_crypto_method_self_test(ciphersuite -> nx_secure_tls_public_auth, metadata, metadata_size);
        if (status)
        {
            return(status);
        }

        status = _nx_secure_crypto_method_self_test(ciphersuite -> nx_secure_tls_session_cipher, metadata, metadata_size);
        if (status)
        {
            return(status);
        }

        status = _nx_secure_crypto_method_self_test(ciphersuite -> nx_secure_tls_hash, metadata, metadata_size);
        if (status)
        {
            return(status);
        }

        status = _nx_secure_crypto_method_self_test(ciphersuite -> nx_secure_tls_prf, metadata, metadata_size);
        if (status)
        {
            return(status);
        }

    }

#ifndef NX_SECURE_DISABLE_X509
    crypto_table_size = crypto_table -> nx_secure_tls_x509_cipher_table_size;

    /* Loop through X.509 crypto table and perform self tests. */
    for (i = 0; i < crypto_table_size; ++i)
    {
        x509_crypto = &crypto_table -> nx_secure_tls_x509_cipher_table[i];

        status = _nx_secure_crypto_method_self_test(x509_crypto -> nx_secure_x509_public_cipher_method, metadata, metadata_size);
        if (status)
        {
            return(status);
        }

        status = _nx_secure_crypto_method_self_test(x509_crypto -> nx_secure_x509_hash_method, metadata, metadata_size);
        if (status)
        {
            return(status);
        }

    }
#endif


    /* We will also need to maintain the testing information. If a crypto method has been
       tested, we don't want to test it again.  This happens when the same algorithm apeears
       multiple times in the lookup table.
    */


    return(NX_CRYPTO_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(crypto_table);
    NX_PARAMETER_NOT_USED(metadata);
    NX_PARAMETER_NOT_USED(metadata_size);
    return(NX_CRYPTO_SUCCESS);
#endif
}
#ifdef NX_SECURE_POWER_ON_SELF_TEST_MODULE_INTEGRITY_CHECK

UINT _nx_secure_crypto_method_self_test(const NX_CRYPTO_METHOD *crypto_method,
                                        VOID *metadata, UINT metadata_size)
{
UINT i;
UINT crypto_algorithm;
UINT status;

    /* Validate the crypto method */
    if(crypto_method == NX_NULL)
        return(NX_PTR_ERROR);

    crypto_algorithm = crypto_method -> nx_crypto_algorithm;

    /* No necessary to verify NULL or None algorithms. */
    if ((crypto_algorithm == NX_CRYPTO_ENCRYPTION_NULL) ||
        (crypto_algorithm == NX_CRYPTO_NONE) ||
        (crypto_algorithm == NX_CRYPTO_AUTHENTICATION_NONE) ||
        (crypto_algorithm == NX_CRYPTO_HASH_NONE) ||
        (crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_NONE))
        return(NX_CRYPTO_SUCCESS);

    /* Loop through the self test map and find the self test function for the crypto method. */
    for (i = 0; i < sizeof(nx_crypto_self_test_map) / sizeof(NX_SECURE_CRYPTO_SELF_TEST); ++i)
    {
        if (crypto_algorithm == nx_crypto_self_test_map[i].nx_crypto_algorithm)
        {
            if (crypto_method != nx_crypto_self_test_methods[i])
            {
                /* Cast away const since self-test functions take non-const methods (but shouldn't!). */
                status = nx_crypto_self_test_map[i].self_test_function((NX_CRYPTO_METHOD*)crypto_method, metadata, metadata_size);
                nx_crypto_self_test_methods[i] = crypto_method;
                return(status);
            }
            else
            {

                /* Duplicate crypto method. */
                return(NX_SECURE_TLS_SUCCESS);
            }
        }
    }

    /* Unknown crypto method. */
    return(NX_NOT_SUCCESSFUL);
}
#endif
