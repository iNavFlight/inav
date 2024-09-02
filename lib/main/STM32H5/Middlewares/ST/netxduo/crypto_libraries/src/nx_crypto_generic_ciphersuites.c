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
/** NetX Crypto Component                                                 */
/**                                                                       */
/**   Transport Layer Security (TLS)                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#ifndef NX_CRYPTO_STANDALONE_ENABLE
#include "nx_secure_tls.h"


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_crypto_generic_ciphersuites                      PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*     This table of function pointers provides a mapping from TLS        */
/*     ciphersuites to the necessary cryptographic methods for a given    */
/*     platform. It can be used as a model to develop a hardware-specific */
/*     cryptography table for TLS.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s), added    */
/*                                            curves in the crypto array, */
/*                                            added TLS ciphersuite entry,*/
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), added    */
/*                                            x25519 and x448 curves,     */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            added x448 curves,          */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022     Yanwu Cai                Modified comment(s),          */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

/* Define cryptographic methods for use with TLS. */

extern NX_CRYPTO_METHOD crypto_method_none;
extern NX_CRYPTO_METHOD crypto_method_null;
extern NX_CRYPTO_METHOD crypto_method_aes_cbc_128;
extern NX_CRYPTO_METHOD crypto_method_aes_cbc_256;
extern NX_CRYPTO_METHOD crypto_method_aes_ccm_8;
extern NX_CRYPTO_METHOD crypto_method_aes_ccm_16;
extern NX_CRYPTO_METHOD crypto_method_aes_128_gcm_16;
extern NX_CRYPTO_METHOD crypto_method_aes_256_gcm_16;
extern NX_CRYPTO_METHOD crypto_method_ecdsa;
extern NX_CRYPTO_METHOD crypto_method_ecdhe;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha1;
extern NX_CRYPTO_METHOD crypto_method_hmac_sha256;
extern NX_CRYPTO_METHOD crypto_method_hmac_md5;
extern NX_CRYPTO_METHOD crypto_method_rsa;
extern NX_CRYPTO_METHOD crypto_method_pkcs1;
extern NX_CRYPTO_METHOD crypto_method_auth_psk;
extern NX_CRYPTO_METHOD crypto_method_ec_secp256;
extern NX_CRYPTO_METHOD crypto_method_ec_secp384;
extern NX_CRYPTO_METHOD crypto_method_ec_secp521;
extern NX_CRYPTO_METHOD crypto_method_ec_x25519;
extern NX_CRYPTO_METHOD crypto_method_ec_x448;
extern NX_CRYPTO_METHOD crypto_method_md5;
extern NX_CRYPTO_METHOD crypto_method_sha1;
extern NX_CRYPTO_METHOD crypto_method_sha224;
extern NX_CRYPTO_METHOD crypto_method_sha256;
extern NX_CRYPTO_METHOD crypto_method_sha384;
extern NX_CRYPTO_METHOD crypto_method_sha512;
extern NX_CRYPTO_METHOD crypto_method_hkdf_sha1;
extern NX_CRYPTO_METHOD crypto_method_hkdf_sha256;
extern NX_CRYPTO_METHOD crypto_method_tls_prf_1;
extern NX_CRYPTO_METHOD crypto_method_tls_prf_sha256;
extern NX_CRYPTO_METHOD crypto_method_tls_prf_sha384;
extern NX_CRYPTO_METHOD crypto_method_hkdf;
extern NX_CRYPTO_METHOD crypto_method_hmac;


/* Ciphersuite table without ECC. */
/* Lookup table used to map ciphersuites to cryptographic routines. */
/* For TLS Web servers, define NX_SECURE_ENABLE_AEAD_CIPHER to allow web browsers to connect using AES_128_GCM cipher suites. */
NX_SECURE_TLS_CIPHERSUITE_INFO _nx_crypto_ciphersuite_lookup_table[] =
{
    /* Ciphersuite,                           public cipher,            public_auth,              session cipher & cipher mode,   iv size, key size,  hash method,                    hash size, TLS PRF */
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    {TLS_RSA_WITH_AES_128_GCM_SHA256,         &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_128_gcm_16,  16,      16,        &crypto_method_null,            0,         &crypto_method_tls_prf_sha256},
#endif /* NX_SECURE_ENABLE_AEAD_CIPHER */
    {TLS_RSA_WITH_AES_256_CBC_SHA256,         &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_cbc_256,     16,      32,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
    {TLS_RSA_WITH_AES_128_CBC_SHA256,         &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
    {TLS_PSK_WITH_AES_128_CBC_SHA256,         &crypto_method_null,      &crypto_method_auth_psk,  &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    {TLS_PSK_WITH_AES_128_CCM_8,              &crypto_method_null,      &crypto_method_auth_psk,  &crypto_method_aes_ccm_8,       16,      16,        &crypto_method_null,            0,         &crypto_method_tls_prf_sha256},
#endif
#endif /* NX_SECURE_ENABLE_PSK_CIPHERSUITES */
};

const UINT _nx_crypto_ciphersuite_lookup_table_size = sizeof(_nx_crypto_ciphersuite_lookup_table) / sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO);

/* Lookup table for X.509 digital certificates - they need a public-key algorithm and a hash routine for verification. */
NX_SECURE_X509_CRYPTO _nx_crypto_x509_cipher_lookup_table[] =
{
    /* OID identifier,                        public cipher,            hash method */
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_256,    &crypto_method_rsa,       &crypto_method_sha256},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_384,    &crypto_method_rsa,       &crypto_method_sha384},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_512,    &crypto_method_rsa,       &crypto_method_sha512},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_1,      &crypto_method_rsa,       &crypto_method_sha1},
    {NX_SECURE_TLS_X509_TYPE_RSA_MD5,        &crypto_method_rsa,       &crypto_method_md5},
};

const UINT _nx_crypto_x509_cipher_lookup_table_size = sizeof(_nx_crypto_x509_cipher_lookup_table) / sizeof(NX_SECURE_X509_CRYPTO);

/* Define the object we can pass into TLS. */
NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers =
{
    /* Ciphersuite lookup table and size. */
    _nx_crypto_ciphersuite_lookup_table,
    sizeof(_nx_crypto_ciphersuite_lookup_table) / sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO),

#ifndef NX_SECURE_DISABLE_X509
    /* X.509 certificate cipher table and size. */
    _nx_crypto_x509_cipher_lookup_table,
    sizeof(_nx_crypto_x509_cipher_lookup_table) / sizeof(NX_SECURE_X509_CRYPTO),
#endif

    /* TLS version-specific methods. */
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    &crypto_method_md5,
    &crypto_method_sha1,
    &crypto_method_tls_prf_1,
#endif

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    &crypto_method_sha256,
    &crypto_method_tls_prf_sha256,
#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    &crypto_method_hkdf,
    &crypto_method_hmac,
    &crypto_method_ecdhe,
#endif
};


#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE

/* Lookup table for X.509 digital certificates - they need a public-key algorithm and a hash routine for verification. */
NX_SECURE_X509_CRYPTO _nx_crypto_x509_cipher_lookup_table_ecc[] =
{
    /* OID identifier,                        public cipher,            hash method */
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_256,  &crypto_method_ecdsa,     &crypto_method_sha256},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_384,  &crypto_method_ecdsa,     &crypto_method_sha384},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_512,  &crypto_method_ecdsa,     &crypto_method_sha512},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_256,    &crypto_method_rsa,       &crypto_method_sha256},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_384,    &crypto_method_rsa,       &crypto_method_sha384},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_512,    &crypto_method_rsa,       &crypto_method_sha512},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_224,  &crypto_method_ecdsa,     &crypto_method_sha224},
    {NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_1,    &crypto_method_ecdsa,     &crypto_method_sha1},
    {NX_SECURE_TLS_X509_TYPE_RSA_SHA_1,      &crypto_method_rsa,       &crypto_method_sha1},
    {NX_SECURE_TLS_X509_TYPE_RSA_MD5,        &crypto_method_rsa,       &crypto_method_md5},
};

const UINT _nx_crypto_x509_cipher_lookup_table_ecc_size = sizeof(_nx_crypto_x509_cipher_lookup_table_ecc) / sizeof(NX_SECURE_X509_CRYPTO);


#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
NX_SECURE_TLS_CIPHERSUITE_INFO _nx_crypto_ciphersuite_lookup_table_tls_1_3[] =
{
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    {TLS_AES_128_GCM_SHA256,                  &crypto_method_ecdhe,      &crypto_method_ecdsa,     &crypto_method_aes_128_gcm_16,  96,      16,        &crypto_method_sha256,         32,         &crypto_method_hkdf},
    /* SHA-384 ciphersuites not yet supported... {TLS_AES_256_GCM_SHA384,                  &crypto_method_ecdhe,      &crypto_method_rsa,     &crypto_method_aes_256_gcm_16,  16,      16,        &crypto_method_sha384,         48,         &crypto_method_hkdf},*/
    {TLS_AES_128_CCM_SHA256,                  &crypto_method_ecdhe,      &crypto_method_ecdsa,     &crypto_method_aes_ccm_16,       96,      16,        &crypto_method_sha256,         32,         &crypto_method_hkdf},
    {TLS_AES_128_CCM_8_SHA256,                &crypto_method_ecdhe,      &crypto_method_ecdsa,     &crypto_method_aes_ccm_8,       96,      16,        &crypto_method_sha256,         32,         &crypto_method_hkdf},
#endif
};

const UINT _nx_crypto_ciphersuite_lookup_table_tls_1_3_size = sizeof(_nx_crypto_ciphersuite_lookup_table_tls_1_3) / sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO);
#endif

/* Ciphersuite table with ECC. */
/* Lookup table used to map ciphersuites to cryptographic routines. */
/* Ciphersuites are negotiated IN ORDER - top priority first. Ciphersuites lower in the list are considered less secure. */
/* For TLS Web servers, define NX_SECURE_ENABLE_AEAD_CIPHER to allow web browsers to connect using AES_128_GCM cipher suites. */
NX_SECURE_TLS_CIPHERSUITE_INFO _nx_crypto_ciphersuite_lookup_table_ecc[] =
{
    /* Ciphersuite,                           public cipher,            public_auth,              session cipher & cipher mode,   iv size, key size,  hash method,                    hash size, TLS PRF */
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    {TLS_AES_128_GCM_SHA256,                  &crypto_method_ecdhe,     &crypto_method_ecdsa,     &crypto_method_aes_128_gcm_16,  96,      16,        &crypto_method_sha256,         32,         &crypto_method_hkdf},
    {TLS_AES_128_CCM_SHA256,                  &crypto_method_ecdhe,     &crypto_method_ecdsa,     &crypto_method_aes_ccm_16,      96,      16,        &crypto_method_sha256,         32,         &crypto_method_hkdf},
    {TLS_AES_128_CCM_8_SHA256,                &crypto_method_ecdhe,     &crypto_method_ecdsa,     &crypto_method_aes_ccm_8,       96,      16,        &crypto_method_sha256,         32,         &crypto_method_hkdf},
#endif

#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    {TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256, &crypto_method_ecdhe,     &crypto_method_ecdsa,     &crypto_method_aes_128_gcm_16,  16,      16,        &crypto_method_null,            0,         &crypto_method_tls_prf_sha256},
    {TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,   &crypto_method_ecdhe,     &crypto_method_rsa,       &crypto_method_aes_128_gcm_16,  16,      16,        &crypto_method_null,            0,         &crypto_method_tls_prf_sha256},
#endif /* NX_SECURE_ENABLE_AEAD_CIPHER */

    {TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256, &crypto_method_ecdhe,     &crypto_method_ecdsa,     &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
    {TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,   &crypto_method_ecdhe,     &crypto_method_rsa,       &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},

#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    {TLS_RSA_WITH_AES_128_GCM_SHA256,         &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_128_gcm_16,  16,      16,        &crypto_method_null,            0,         &crypto_method_tls_prf_sha256},
#endif /* NX_SECURE_ENABLE_AEAD_CIPHER */

    {TLS_RSA_WITH_AES_256_CBC_SHA256,         &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_cbc_256,     16,      32,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
    {TLS_RSA_WITH_AES_128_CBC_SHA256,         &crypto_method_rsa,       &crypto_method_rsa,       &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
    {TLS_PSK_WITH_AES_128_CBC_SHA256,         &crypto_method_null,      &crypto_method_auth_psk,  &crypto_method_aes_cbc_128,     16,      16,        &crypto_method_hmac_sha256,     32,        &crypto_method_tls_prf_sha256},
#ifdef NX_SECURE_ENABLE_AEAD_CIPHER
    {TLS_PSK_WITH_AES_128_CCM_8,              &crypto_method_null,      &crypto_method_auth_psk,  &crypto_method_aes_ccm_8,       16,      16,        &crypto_method_null,            0,         &crypto_method_tls_prf_sha256},
#endif
#endif /* NX_SECURE_ENABLE_PSK_CIPHERSUITES */


};

const UINT _nx_crypto_ciphersuite_lookup_table_ecc_size = sizeof(_nx_crypto_ciphersuite_lookup_table_ecc) / sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO);


/* Define the object we can pass into TLS. */
const NX_SECURE_TLS_CRYPTO nx_crypto_tls_ciphers_ecc =
{
    /* Ciphersuite lookup table and size. */
    _nx_crypto_ciphersuite_lookup_table_ecc,
    sizeof(_nx_crypto_ciphersuite_lookup_table_ecc) / sizeof(NX_SECURE_TLS_CIPHERSUITE_INFO),

#ifndef NX_SECURE_DISABLE_X509
    /* X.509 certificate cipher table and size. */
    _nx_crypto_x509_cipher_lookup_table_ecc,
    sizeof(_nx_crypto_x509_cipher_lookup_table_ecc) / sizeof(NX_SECURE_X509_CRYPTO),
#endif

    /* TLS version-specific methods. */
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
    &crypto_method_md5,
    &crypto_method_sha1,
    &crypto_method_tls_prf_1,
#endif

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    &crypto_method_sha256,
    &crypto_method_tls_prf_sha256,
#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    &crypto_method_hkdf,
    &crypto_method_hmac,
    &crypto_method_ecdhe,
#endif


};

const USHORT nx_crypto_ecc_supported_groups[] =
{
    (USHORT)NX_CRYPTO_EC_SECP256R1,
#ifdef NX_CRYPTO_ENABLE_CURVE25519_448
    (USHORT)NX_CRYPTO_EC_X25519,
    (USHORT)NX_CRYPTO_EC_X448,
#endif /* NX_CRYPTO_ENABLE_CURVE25519_448 */
    (USHORT)NX_CRYPTO_EC_SECP384R1,
    (USHORT)NX_CRYPTO_EC_SECP521R1,
};

const NX_CRYPTO_METHOD *nx_crypto_ecc_curves[] =
{
    &crypto_method_ec_secp256,
#ifdef NX_CRYPTO_ENABLE_CURVE25519_448
    &crypto_method_ec_x25519,
    &crypto_method_ec_x448,
#endif /* NX_CRYPTO_ENABLE_CURVE25519_448 */
    &crypto_method_ec_secp384,
    &crypto_method_ec_secp521,
};

const UINT nx_crypto_ecc_supported_groups_size = sizeof(nx_crypto_ecc_supported_groups) / sizeof(USHORT);
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */



#if 0  /* This ciphersuite is provided for reference only. It can be used to construct legacy ciphersuites
          for use with TLS 1.0 or TLS 1.1 (SHA-1 based ciphersuites are not currently supported in TLS 1.2). */
const NX_CRYPTO_CIPHERSUITE nx_crypto_tls_rsa_with_aes_128_cbc_sha =
/* TLS ciphersuite entry. */
{   TLS_RSA_WITH_AES_128_CBC_SHA,       /* Ciphersuite ID. */
    NX_SECURE_APPLICATION_TLS,          /* Internal application label. */
    16,                                 /* Symmetric key size. */
    {   /* Cipher role array. */
        {NX_CRYPTO_KEY_EXCHANGE_RSA,             NX_CRYPTO_ROLE_KEY_EXCHANGE},
        {NX_CRYPTO_DIGITAL_SIGNATURE_RSA,        NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_ENCRYPTION_AES_CBC,           NX_CRYPTO_ROLE_SYMMETRIC},
        {NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_160, NX_CRYPTO_ROLE_MAC_HASH},
        {NX_CRYPTO_HASH_SHA1,                    NX_CRYPTO_ROLE_RAW_HASH},
        {NX_CRYPTO_HASH_HMAC,                    NX_CRYPTO_ROLE_HMAC},
        {NX_CRYPTO_PRF_HMAC_SHA2_256,            NX_CRYPTO_ROLE_PRF},
        {NX_CRYPTO_NONE,                         NX_CRYPTO_ROLE_NONE}
    },
    /* TLS/DTLS Versions supported. */
    (NX_SECURE_TLS_BITFIELD_VERSIONS_PRE_1_3 | NX_SECURE_DTLS_BITFIELD_VERSIONS_PRE_1_3)
};
#endif

const NX_CRYPTO_CIPHERSUITE nx_crypto_tls_rsa_with_aes_128_cbc_sha256 =
/* TLS ciphersuite entry. */
{   TLS_RSA_WITH_AES_128_CBC_SHA256,    /* Ciphersuite ID. */
    NX_SECURE_APPLICATION_TLS,          /* Internal application label. */
    16,                                 /* Symmetric key size. */
    {   /* Cipher role array. */
        {NX_CRYPTO_KEY_EXCHANGE_RSA,             NX_CRYPTO_ROLE_KEY_EXCHANGE},
        {NX_CRYPTO_KEY_EXCHANGE_RSA,             NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_ENCRYPTION_AES_CBC,           NX_CRYPTO_ROLE_SYMMETRIC},
        {NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256, NX_CRYPTO_ROLE_MAC_HASH},
        {NX_CRYPTO_HASH_SHA256,                  NX_CRYPTO_ROLE_RAW_HASH},
        {NX_CRYPTO_HASH_HMAC,                    NX_CRYPTO_ROLE_HMAC},
        {NX_CRYPTO_PRF_HMAC_SHA2_256,            NX_CRYPTO_ROLE_PRF},
        {NX_CRYPTO_NONE,                         NX_CRYPTO_ROLE_NONE}
    },
    /* TLS/DTLS Versions supported. */
    (NX_SECURE_TLS_BITFIELD_VERSIONS_PRE_1_3 | NX_SECURE_DTLS_BITFIELD_VERSIONS_PRE_1_3)
};

const NX_CRYPTO_CIPHERSUITE nx_crypto_tls_ecdhe_rsa_with_aes_128_cbc_sha256 =
/* TLS ciphersuite entry. */
{   TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256,   /* Ciphersuite ID. */
    NX_SECURE_APPLICATION_TLS,               /* Internal application label. */
    16,                                      /* Symmetric key size. */
    {   /* Cipher role array. */
        {NX_CRYPTO_KEY_EXCHANGE_ECDHE,           NX_CRYPTO_ROLE_KEY_EXCHANGE},
        {NX_CRYPTO_KEY_EXCHANGE_RSA,             NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_ENCRYPTION_AES_CBC,           NX_CRYPTO_ROLE_SYMMETRIC},
        {NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256, NX_CRYPTO_ROLE_MAC_HASH},
        {NX_CRYPTO_HASH_SHA256,                  NX_CRYPTO_ROLE_RAW_HASH},
        {NX_CRYPTO_HASH_HMAC,                    NX_CRYPTO_ROLE_HMAC},
        {NX_CRYPTO_PRF_HMAC_SHA2_256,            NX_CRYPTO_ROLE_PRF},
        {NX_CRYPTO_NONE,                         NX_CRYPTO_ROLE_NONE}
    },
    /* TLS/DTLS Versions supported. */
    (NX_SECURE_TLS_BITFIELD_VERSIONS_PRE_1_3 | NX_SECURE_DTLS_BITFIELD_VERSIONS_PRE_1_3)
};

const NX_CRYPTO_CIPHERSUITE nx_crypto_tls_ecdhe_rsa_with_aes_128_gcm_sha256 =
/* TLS ciphersuite entry. */
{   TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,   /* Ciphersuite ID. */
    NX_SECURE_APPLICATION_TLS,               /* Internal application label. */
    16,                                      /* Symmetric key size. */
    {   /* Cipher role array. */
        {NX_CRYPTO_KEY_EXCHANGE_ECDHE,           NX_CRYPTO_ROLE_KEY_EXCHANGE},
        {NX_CRYPTO_KEY_EXCHANGE_RSA,             NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_ENCRYPTION_AES_GCM_16,        NX_CRYPTO_ROLE_SYMMETRIC},
        {NX_CRYPTO_NONE,                         NX_CRYPTO_ROLE_MAC_HASH},
        {NX_CRYPTO_HASH_SHA256,                  NX_CRYPTO_ROLE_RAW_HASH},
        {NX_CRYPTO_HASH_HMAC,                    NX_CRYPTO_ROLE_HMAC},
        {NX_CRYPTO_PRF_HMAC_SHA2_256,            NX_CRYPTO_ROLE_PRF},
        {NX_CRYPTO_NONE,                         NX_CRYPTO_ROLE_NONE}
    },
    /* TLS/DTLS Versions supported. */
    (NX_SECURE_TLS_BITFIELD_VERSIONS_PRE_1_3 | NX_SECURE_DTLS_BITFIELD_VERSIONS_PRE_1_3)
};

const NX_CRYPTO_CIPHERSUITE nx_crypto_tls_ecdhe_ecdsa_with_aes_128_gcm_sha256 =
/* TLS ciphersuite entry. */
{   TLS_ECDHE_ECDSA_WITH_AES_128_GCM_SHA256, /* Ciphersuite ID. */
    NX_SECURE_APPLICATION_TLS,               /* Internal application label. */
    16,                                      /* Symmetric key size. */
    {   /* Cipher role array. */
        {NX_CRYPTO_KEY_EXCHANGE_ECDHE,           NX_CRYPTO_ROLE_KEY_EXCHANGE},
        {NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA,      NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_ENCRYPTION_AES_GCM_16,        NX_CRYPTO_ROLE_SYMMETRIC},
        {NX_CRYPTO_NONE,                         NX_CRYPTO_ROLE_MAC_HASH},
        {NX_CRYPTO_HASH_SHA256,                  NX_CRYPTO_ROLE_RAW_HASH},
        {NX_CRYPTO_HASH_HMAC,                    NX_CRYPTO_ROLE_HMAC},
        {NX_CRYPTO_PRF_HMAC_SHA2_256,            NX_CRYPTO_ROLE_PRF},
        {NX_CRYPTO_NONE,                         NX_CRYPTO_ROLE_NONE}
    },
    /* TLS/DTLS Versions supported. */
    (NX_SECURE_TLS_BITFIELD_VERSIONS_PRE_1_3 | NX_SECURE_DTLS_BITFIELD_VERSIONS_PRE_1_3)
};

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
const NX_CRYPTO_CIPHERSUITE nx_crypto_tls_aes_128_gcm_sha256 =
/* TLS ciphersuite entry. */
{   TLS_AES_128_GCM_SHA256,             /* Ciphersuite ID. */
    NX_SECURE_APPLICATION_TLS,          /* Internal application label. */
    16,                                 /* Symmetric key size. */
    {   /* Cipher role array. */
        {NX_CRYPTO_KEY_EXCHANGE_ECDHE,           NX_CRYPTO_ROLE_KEY_EXCHANGE},
        {NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA,      NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_ENCRYPTION_AES_GCM_16,        NX_CRYPTO_ROLE_SYMMETRIC},
        {NX_CRYPTO_HASH_SHA256,                  NX_CRYPTO_ROLE_MAC_HASH},
        {NX_CRYPTO_HASH_SHA256,                  NX_CRYPTO_ROLE_RAW_HASH},
        {NX_CRYPTO_HKDF_METHOD,                  NX_CRYPTO_ROLE_PRF},
        {NX_CRYPTO_NONE,                         NX_CRYPTO_ROLE_NONE}
    },
    /* TLS/DTLS Versions supported. */
    (NX_SECURE_TLS_BITFIELD_VERSION_1_3 | NX_SECURE_DTLS_BITFIELD_VERSION_1_3)
};
#endif

const NX_CRYPTO_CIPHERSUITE nx_crypto_x509_rsa_md5 =
/* X.509 ciphersuite entry. */
{
    NX_SECURE_TLS_X509_TYPE_RSA_MD5,
    NX_SECURE_APPLICATION_X509,
    0,                                 /* Symmetric key size. */
    {
        {NX_CRYPTO_KEY_EXCHANGE_RSA,         NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_HASH_MD5,                 NX_CRYPTO_ROLE_SIGNATURE_HASH},
        {NX_CRYPTO_NONE,                     NX_CRYPTO_ROLE_NONE}
    },
    /* Versions supported. */
    NX_SECURE_X509_BITFIELD_VERSION_3
};


const NX_CRYPTO_CIPHERSUITE nx_crypto_x509_rsa_sha_1 =
/* X.509 ciphersuite entry. */
{
    NX_SECURE_TLS_X509_TYPE_RSA_SHA_1,
    NX_SECURE_APPLICATION_X509,
    0,                                 /* Symmetric key size. */
    {
        {NX_CRYPTO_KEY_EXCHANGE_RSA,         NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_HASH_SHA1,                NX_CRYPTO_ROLE_SIGNATURE_HASH},
        {NX_CRYPTO_NONE,                     NX_CRYPTO_ROLE_NONE}
    },
    /* Versions supported. */
    NX_SECURE_X509_BITFIELD_VERSION_3
};

const NX_CRYPTO_CIPHERSUITE nx_crypto_x509_rsa_sha_256 =
/* X.509 ciphersuite entry. */
{
    NX_SECURE_TLS_X509_TYPE_RSA_SHA_256,
    NX_SECURE_APPLICATION_X509,
    0,                                 /* Symmetric key size. */
    {
        {NX_CRYPTO_KEY_EXCHANGE_RSA,         NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_HASH_SHA256,              NX_CRYPTO_ROLE_SIGNATURE_HASH},
        {NX_CRYPTO_NONE,                     NX_CRYPTO_ROLE_NONE}
    },
    /* Versions supported. */
    NX_SECURE_X509_BITFIELD_VERSION_3
};

const NX_CRYPTO_CIPHERSUITE nx_crypto_x509_rsa_sha_384 =
/* X.509 ciphersuite entry. */
{
    NX_SECURE_TLS_X509_TYPE_RSA_SHA_384,
    NX_SECURE_APPLICATION_X509,
    0,                                 /* Symmetric key size. */
    {
        {NX_CRYPTO_KEY_EXCHANGE_RSA,         NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_HASH_SHA384,              NX_CRYPTO_ROLE_SIGNATURE_HASH},
        {NX_CRYPTO_NONE,                     NX_CRYPTO_ROLE_NONE}
    },
    /* Versions supported. */
    NX_SECURE_X509_BITFIELD_VERSION_3
};

const NX_CRYPTO_CIPHERSUITE nx_crypto_x509_rsa_sha_512 =
/* X.509 ciphersuite entry. */
{
    NX_SECURE_TLS_X509_TYPE_RSA_SHA_512,
    NX_SECURE_APPLICATION_X509,
    0,                                 /* Symmetric key size. */
    {
        {NX_CRYPTO_KEY_EXCHANGE_RSA,         NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_HASH_SHA512,              NX_CRYPTO_ROLE_SIGNATURE_HASH},
        {NX_CRYPTO_NONE,                     NX_CRYPTO_ROLE_NONE}
    },
    /* Versions supported. */
    NX_SECURE_X509_BITFIELD_VERSION_3
};



const NX_CRYPTO_CIPHERSUITE nx_crypto_x509_ecdsa_sha_1 =
/* X.509 ciphersuite entry. */
{
    NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_1,
    NX_SECURE_APPLICATION_X509,
    0,                                 /* Symmetric key size. */
    {
        {NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA,  NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_HASH_SHA1,                NX_CRYPTO_ROLE_SIGNATURE_HASH},
        {NX_CRYPTO_NONE,                     NX_CRYPTO_ROLE_NONE}
    },
    /* Versions supported. */
    NX_SECURE_X509_BITFIELD_VERSION_3
};

const NX_CRYPTO_CIPHERSUITE nx_crypto_x509_ecdsa_sha_224 =
/* X.509 ciphersuite entry. */
{
    NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_224,
    NX_SECURE_APPLICATION_X509,
    0,                                 /* Symmetric key size. */
    {
        {NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA,  NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_HASH_SHA224,              NX_CRYPTO_ROLE_SIGNATURE_HASH},
        {NX_CRYPTO_NONE,                     NX_CRYPTO_ROLE_NONE}
    },
    /* Versions supported. */
    NX_SECURE_X509_BITFIELD_VERSION_3
};

const NX_CRYPTO_CIPHERSUITE nx_crypto_x509_ecdsa_sha_256 =
/* X.509 ciphersuite entry. */
{
    NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_256,
    NX_SECURE_APPLICATION_X509,
    0,                                 /* Symmetric key size. */
    {
        {NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA,  NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_HASH_SHA256,              NX_CRYPTO_ROLE_SIGNATURE_HASH},
        {NX_CRYPTO_NONE,                     NX_CRYPTO_ROLE_NONE}
    },
    /* Versions supported. */
    NX_SECURE_X509_BITFIELD_VERSION_3
};

const NX_CRYPTO_CIPHERSUITE nx_crypto_x509_ecdsa_sha_384 =
/* X.509 ciphersuite entry. */
{
    NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_384,
    NX_SECURE_APPLICATION_X509,
    0,                                 /* Symmetric key size. */
    {
        {NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA,  NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_HASH_SHA384,              NX_CRYPTO_ROLE_SIGNATURE_HASH},
        {NX_CRYPTO_NONE,                     NX_CRYPTO_ROLE_NONE}
    },
    /* Versions supported. */
    NX_SECURE_X509_BITFIELD_VERSION_3
};

const NX_CRYPTO_CIPHERSUITE nx_crypto_x509_ecdsa_sha_512 =
/* X.509 ciphersuite entry. */
{
    NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_512,
    NX_SECURE_APPLICATION_X509,
    0,                                 /* Symmetric key size. */
    {
        {NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA,  NX_CRYPTO_ROLE_SIGNATURE_CRYPTO},
        {NX_CRYPTO_HASH_SHA512,              NX_CRYPTO_ROLE_SIGNATURE_HASH},
        {NX_CRYPTO_NONE,                     NX_CRYPTO_ROLE_NONE}
    },
    /* Versions supported. */
    NX_SECURE_X509_BITFIELD_VERSION_3
};


const NX_CRYPTO_METHOD *supported_crypto[] =
{
    &crypto_method_none,
    &crypto_method_rsa,
    &crypto_method_pkcs1,
    &crypto_method_ecdhe,
    &crypto_method_ecdsa,
    &crypto_method_aes_ccm_8,
    &crypto_method_aes_cbc_128,
    &crypto_method_aes_cbc_256,
    &crypto_method_aes_128_gcm_16,
    &crypto_method_aes_256_gcm_16,
    &crypto_method_hmac,
    &crypto_method_hmac_md5,
    &crypto_method_hmac_sha1,
    &crypto_method_hmac_sha256,
    &crypto_method_md5,
    &crypto_method_sha1,
    &crypto_method_sha224,
    &crypto_method_sha256,
    &crypto_method_sha384,
    &crypto_method_sha512,
    &crypto_method_tls_prf_1,
    &crypto_method_tls_prf_sha256,
    &crypto_method_hkdf,
    &crypto_method_ec_secp256,
#ifdef NX_CRYPTO_ENABLE_CURVE25519_448
    &crypto_method_ec_x25519,
    &crypto_method_ec_x448,
#endif /* NX_CRYPTO_ENABLE_CURVE25519_448 */
    &crypto_method_ec_secp384,
    &crypto_method_ec_secp521,
};

const UINT supported_crypto_size = sizeof(supported_crypto) / sizeof(NX_CRYPTO_METHOD*);

const NX_CRYPTO_CIPHERSUITE *ciphersuite_map[] =
{
    /* TLS ciphersuites. */
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    &nx_crypto_tls_aes_128_gcm_sha256,
#endif
    &nx_crypto_tls_ecdhe_rsa_with_aes_128_gcm_sha256,
    &nx_crypto_tls_ecdhe_ecdsa_with_aes_128_gcm_sha256,
    &nx_crypto_tls_rsa_with_aes_128_cbc_sha256,

    /* X.509 ciphersuites. */
    &nx_crypto_x509_ecdsa_sha_256,
    &nx_crypto_x509_ecdsa_sha_384,
    &nx_crypto_x509_ecdsa_sha_512,
    &nx_crypto_x509_rsa_sha_256,
    &nx_crypto_x509_rsa_sha_384,
    &nx_crypto_x509_rsa_sha_512,
    &nx_crypto_x509_ecdsa_sha_224,
    &nx_crypto_x509_ecdsa_sha_1,
    &nx_crypto_x509_rsa_sha_1,
    &nx_crypto_x509_rsa_md5,
};

const UINT ciphersuite_map_size = sizeof(ciphersuite_map) / sizeof(NX_CRYPTO_CIPHERSUITE*);

#endif /* NX_CRYPTO_STANDALONE_ENABLE */




