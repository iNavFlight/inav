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

#include "nx_azure_iot_ciphersuites.h"

#if (!NX_SECURE_TLS_TLS_1_2_ENABLED)
#error "TLS 1.2 must be enabled."
#endif /* (!NX_SECURE_TLS_TLS_1_2_ENABLED) */

#ifdef NX_SECURE_DISABLE_X509
#error "X509 must be enabled."
#endif /* NX_SECURE_DISABLE_X509 */

/* Define supported crypto method.  */
extern NX_CRYPTO_METHOD crypto_method_hmac;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha256;
extern NX_CRYPTO_METHOD crypto_method_tls_prf_sha256;
extern NX_CRYPTO_METHOD crypto_method_sha256;
extern NX_CRYPTO_METHOD crypto_method_sha384;
extern NX_CRYPTO_METHOD crypto_method_aes_cbc_128;
extern NX_CRYPTO_METHOD crypto_method_rsa;
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
extern NX_CRYPTO_METHOD crypto_method_ecdhe;
extern NX_CRYPTO_METHOD crypto_method_ecdsa;
extern NX_CRYPTO_METHOD crypto_method_ec_secp384;
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */ 

const NX_CRYPTO_METHOD *_nx_azure_iot_tls_supported_crypto[] =
{
    &crypto_method_hmac,
    &crypto_method_hmac_sha256,
    &crypto_method_tls_prf_sha256,
    &crypto_method_sha256,
    &crypto_method_sha384,
    &crypto_method_aes_cbc_128,
    &crypto_method_rsa,
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    &crypto_method_ecdhe,
    &crypto_method_ecdsa,
    &crypto_method_ec_secp384,
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
};

const UINT _nx_azure_iot_tls_supported_crypto_size = sizeof(_nx_azure_iot_tls_supported_crypto) / sizeof(NX_CRYPTO_METHOD*);


/* Define supported TLS ciphersuites.  */
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
extern const NX_CRYPTO_CIPHERSUITE nx_crypto_tls_ecdhe_rsa_with_aes_128_cbc_sha256;
#else
extern const NX_CRYPTO_CIPHERSUITE nx_crypto_tls_rsa_with_aes_128_cbc_sha256;
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

/* Define supported X.509 ciphersuites.  */
extern const NX_CRYPTO_CIPHERSUITE nx_crypto_x509_rsa_sha_256;
extern const NX_CRYPTO_CIPHERSUITE nx_crypto_x509_rsa_sha_384;
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
extern const NX_CRYPTO_CIPHERSUITE nx_crypto_x509_ecdsa_sha_256;
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

const NX_CRYPTO_CIPHERSUITE *_nx_azure_iot_tls_ciphersuite_map[] =
{

    /* TLS ciphersuites. */
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    &nx_crypto_tls_ecdhe_rsa_with_aes_128_cbc_sha256,
#else
    &nx_crypto_tls_rsa_with_aes_128_cbc_sha256,
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

    /* X.509 ciphersuites. */
    &nx_crypto_x509_rsa_sha_256,
    &nx_crypto_x509_rsa_sha_384,
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    &nx_crypto_x509_ecdsa_sha_256,
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
};

const UINT _nx_azure_iot_tls_ciphersuite_map_size = sizeof(_nx_azure_iot_tls_ciphersuite_map) / sizeof(NX_CRYPTO_CIPHERSUITE*);
