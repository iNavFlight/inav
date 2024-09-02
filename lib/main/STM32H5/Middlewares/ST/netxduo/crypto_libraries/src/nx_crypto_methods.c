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
/**   Crypto Methods                                                      */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#include "nx_crypto_phash.h"
#include "nx_crypto_tls_prf_1.h"
#include "nx_crypto_tls_prf_sha256.h"
#include "nx_crypto_tls_prf_sha384.h"
#include "nx_crypto_tls_prf_sha512.h"
#include "nx_crypto_hkdf.h"
#include "nx_crypto_3des.h"
#include "nx_crypto.h"
#include "nx_crypto_md5.h"
#include "nx_crypto_sha1.h"
#include "nx_crypto_sha2.h"
#include "nx_crypto_sha5.h"
#include "nx_crypto.h"
#include "nx_crypto_hmac_sha1.h"
#include "nx_crypto_hmac_sha2.h"
#include "nx_crypto_hmac_sha5.h"
#include "nx_crypto_hmac_md5.h"
#include "nx_crypto_aes.h"
#include "nx_crypto_rsa.h"
#include "nx_crypto_null.h"
#include "nx_crypto_ecjpake.h"
#include "nx_crypto_ecdsa.h"
#include "nx_crypto_ecdh.h"
#include "nx_crypto_drbg.h"
#include "nx_crypto_pkcs1_v1.5.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    nx_crypto_methods                                   PORTABLE C      */
/*                                                           6.1.12       */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Timothy Stapko           Modified comments(s), added   */
/*                                            warning supression for      */
/*                                            obsolete DES/3DES,          */
/*                                            added x25519 curve,         */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            added x448 curve,           */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/

/* Define cryptographic methods. */

/* Declare the NONE method:  encrypt / hash method not config */
NX_CRYPTO_METHOD crypto_method_none =
{
    NX_CRYPTO_NONE,                           /* Name of the crypto algorithm          */
    0,                                        /* Key size in bits, not used            */
    0,                                        /* IV size in bits, not used             */
    0,                                        /* ICV size in bits, not used            */
    0,                                        /* Block size in bytes                   */
    0,                                        /* Metadata size in bytes                */
    NX_CRYPTO_NULL,                           /* Initialization routine, not used      */
    NX_CRYPTO_NULL,                           /* Cleanup routine, not used             */
    NX_CRYPTO_NULL                            /* NULL operation                        */
};


/* Declare the NULL encrypt */
NX_CRYPTO_METHOD crypto_method_null =
{
    NX_CRYPTO_ENCRYPTION_NULL,                /* Name of the crypto algorithm          */
    0,                                        /* Key size in bits, not used            */
    0,                                        /* IV size in bits, not used             */
    0,                                        /* ICV size in bits, not used            */
    4,                                        /* Block size in bytes                   */
    0,                                        /* Metadata size in bytes                */
    NX_CRYPTO_NULL,                           /* Initialization routine, not used      */
    NX_CRYPTO_NULL,                           /* Cleanup routine, not used             */
    NX_CRYPTO_NULL                            /* NULL operation                        */
};

/* Declare the AES-CBC 128 encrytion method. */
NX_CRYPTO_METHOD crypto_method_aes_cbc_128 =
{
    NX_CRYPTO_ENCRYPTION_AES_CBC,                /* AES crypto algorithm                   */
    NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,           /* Key size in bits                       */
    NX_CRYPTO_AES_IV_LEN_IN_BITS,                /* IV size in bits                        */
    0,                                           /* ICV size in bits, not used             */
    (NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS >> 3),     /* Block size in bytes                    */
    sizeof(NX_CRYPTO_AES),                       /* Metadata size in bytes                 */
    _nx_crypto_method_aes_init,                  /* AES-CBC initialization routine         */
    _nx_crypto_method_aes_cleanup,               /* AES-CBC cleanup routine                */
    _nx_crypto_method_aes_cbc_operation          /* AES-CBC operation                      */
};

/* Declare the AES-CBC 192 encrytion method. */
NX_CRYPTO_METHOD crypto_method_aes_cbc_192 =
{
    NX_CRYPTO_ENCRYPTION_AES_CBC,                /* AES crypto algorithm                   */
    NX_CRYPTO_AES_192_KEY_LEN_IN_BITS,           /* Key size in bits                       */
    NX_CRYPTO_AES_IV_LEN_IN_BITS,                /* IV size in bits                        */
    0,                                           /* ICV size in bits, not used             */
    (NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS >> 3),     /* Block size in bytes                    */
    sizeof(NX_CRYPTO_AES),                       /* Metadata size in bytes                 */
    _nx_crypto_method_aes_init,                  /* AES-CBC initialization routine         */
    _nx_crypto_method_aes_cleanup,               /* AES-CBC cleanup routine                */
    _nx_crypto_method_aes_operation              /* AES-CBC operation                      */
};

/* Declare the AES-CBC 256 encryption method */
NX_CRYPTO_METHOD crypto_method_aes_cbc_256 =
{
    NX_CRYPTO_ENCRYPTION_AES_CBC,                /* AES crypto algorithm                   */
    NX_CRYPTO_AES_256_KEY_LEN_IN_BITS,           /* Key size in bits                       */
    NX_CRYPTO_AES_IV_LEN_IN_BITS,                /* IV size in bits                        */
    0,                                           /* ICV size in bits, not used             */
    (NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS >> 3),     /* Block size in bytes                    */
    sizeof(NX_CRYPTO_AES),                       /* Metadata size in bytes                 */
    _nx_crypto_method_aes_init,                  /* AES-CBC initialization routine         */
    _nx_crypto_method_aes_cleanup,               /* AES-CBC cleanup routine                */
    _nx_crypto_method_aes_cbc_operation          /* AES-CBC operation                      */
};


/* Declare the AES-CCM-8 encrytion method. */
NX_CRYPTO_METHOD crypto_method_aes_ccm_8 =
{
    NX_CRYPTO_ENCRYPTION_AES_CCM_8,              /* AES crypto algorithm                   */
    NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,           /* Key size in bits                       */
    32,                                          /* IV size in bits                        */
    64,                                          /* ICV size in bits                       */
    (NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS >> 3),     /* Block size in bytes.                   */
    sizeof(NX_CRYPTO_AES),                       /* Metadata size in bytes                 */
    _nx_crypto_method_aes_init,                  /* AES-CCM8 initialization routine.       */
    _nx_crypto_method_aes_cleanup,               /* AES-CCM8 cleanup routine.              */
    _nx_crypto_method_aes_ccm_operation          /* AES-CCM8 operation                     */
};

/* Declare the AES-CCM-16 encrytion method. */
NX_CRYPTO_METHOD crypto_method_aes_ccm_16 =
{
    NX_CRYPTO_ENCRYPTION_AES_CCM_16,             /* AES crypto algorithm                   */
    NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,           /* Key size in bits                       */
    32,                                          /* IV size in bits                        */
    128,                                         /* ICV size in bits                       */
    (NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS >> 3),     /* Block size in bytes.                   */
    sizeof(NX_CRYPTO_AES),                       /* Metadata size in bytes                 */
    _nx_crypto_method_aes_init,                  /* AES-CCM16 initialization routine.      */
    _nx_crypto_method_aes_cleanup,               /* AES-CCM16 cleanup routine.             */
    _nx_crypto_method_aes_ccm_operation          /* AES-CCM16 operation                    */
};

/* Declare the AES-GCM encrytion method. */
NX_CRYPTO_METHOD crypto_method_aes_128_gcm_16 =
{
    NX_CRYPTO_ENCRYPTION_AES_GCM_16,             /* AES crypto algorithm                   */
    NX_CRYPTO_AES_128_KEY_LEN_IN_BITS,           /* Key size in bits                       */
    32,                                          /* IV size in bits                        */
    128,                                         /* ICV size in bits                       */
    (NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS >> 3),     /* Block size in bytes.                   */
    sizeof(NX_CRYPTO_AES),                       /* Metadata size in bytes                 */
    _nx_crypto_method_aes_init,                  /* AES-GCM initialization routine.        */
    _nx_crypto_method_aes_cleanup,               /* AES-GCM cleanup routine.               */
    _nx_crypto_method_aes_gcm_operation,         /* AES-GCM operation                      */
};

/* Declare the AES-GCM encrytion method. */
NX_CRYPTO_METHOD crypto_method_aes_256_gcm_16 =
{
    NX_CRYPTO_ENCRYPTION_AES_GCM_16,             /* AES crypto algorithm                   */
    NX_CRYPTO_AES_256_KEY_LEN_IN_BITS,           /* Key size in bits                       */
    32,                                          /* IV size in bits                        */
    128,                                         /* ICV size in bits                       */
    (NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS >> 3),     /* Block size in bytes.                   */
    sizeof(NX_CRYPTO_AES),                       /* Metadata size in bytes                 */
    _nx_crypto_method_aes_init,                  /* AES-GCM initialization routine.        */
    _nx_crypto_method_aes_cleanup,               /* AES-GCM cleanup routine.               */
    _nx_crypto_method_aes_gcm_operation,         /* AES-GCM operation                      */
};

/* Declare the AES-XCBC-MAC encrytion method. */
NX_CRYPTO_METHOD crypto_method_aes_xcbc_mac_96 =
{
    NX_CRYPTO_AUTHENTICATION_AES_XCBC_MAC_96,       /* AES_XCBC_MAC algorithm                */
    NX_CRYPTO_AES_XCBC_MAC_KEY_LEN_IN_BITS,         /* Key size in bits                      */
    0,                                              /* IV size in bits, not used             */
    NX_CRYPTO_AUTHENTICATION_ICV_TRUNC_BITS,        /* Transmitted ICV size in bits          */
    (NX_CRYPTO_AES_BLOCK_SIZE_IN_BITS >> 3),        /* Block size in bytes.                  */
    sizeof(NX_CRYPTO_AES),                          /* Metadata size in bytes                */
    _nx_crypto_method_aes_init,                     /* AES-XCBC_MAC initialization routine.  */
    _nx_crypto_method_aes_cleanup,                  /* AES-XCBC_MAC cleanup routine.         */
    _nx_crypto_method_aes_xcbc_operation            /* AES_XCBC_MAC operation                */
};

/* Declare the DRBG encrytion method. */
NX_CRYPTO_METHOD crypto_method_drbg =
{
    0,                                           /* DRBG crypto algorithm                  */
    0,                                           /* Key size in bits                       */
    0,                                           /* IV size in bits                        */
    0,                                           /* ICV size in bits, not used             */
    0,                                           /* Block size in bytes                    */
    sizeof(NX_CRYPTO_DRBG),                      /* Metadata size in bytes                 */
    _nx_crypto_method_drbg_init,                 /* DRBG initialization routine            */
    _nx_crypto_method_drbg_cleanup,              /* DRBG cleanup routine                   */
    _nx_crypto_method_drbg_operation,            /* DRBG operation                         */
};

/* Declare the ECDSA crypto method */
NX_CRYPTO_METHOD crypto_method_ecdsa =
{
    NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA,           /* ECDSA crypto algorithm                 */
    0,                                           /* Key size in bits                       */
    0,                                           /* IV size in bits                        */
    0,                                           /* ICV size in bits, not used             */
    0,                                           /* Block size in bytes                    */
    sizeof(NX_CRYPTO_ECDSA),                     /* Metadata size in bytes                 */
    _nx_crypto_method_ecdsa_init,                /* ECDSA initialization routine           */
    _nx_crypto_method_ecdsa_cleanup,             /* ECDSA cleanup routine                  */
    _nx_crypto_method_ecdsa_operation            /* ECDSA operation                        */
};

/* Declare the ECDH crypto method */
NX_CRYPTO_METHOD crypto_method_ecdh =
{
    NX_CRYPTO_KEY_EXCHANGE_ECDH,                 /* ECDH crypto algorithm                  */
    0,                                           /* Key size in bits                       */
    0,                                           /* IV size in bits                        */
    0,                                           /* ICV size in bits, not used             */
    0,                                           /* Block size in bytes                    */
    sizeof(NX_CRYPTO_ECDH),                      /* Metadata size in bytes                 */
    _nx_crypto_method_ecdh_init,                 /* ECDH initialization routine            */
    _nx_crypto_method_ecdh_cleanup,              /* ECDH cleanup routine                   */
    _nx_crypto_method_ecdh_operation             /* ECDH operation                         */
};

/* Declare the ECDHE crypto method */
NX_CRYPTO_METHOD crypto_method_ecdhe =
{
    NX_CRYPTO_KEY_EXCHANGE_ECDHE,                /* ECDHE crypto algorithm                 */
    0,                                           /* Key size in bits                       */
    0,                                           /* IV size in bits                        */
    0,                                           /* ICV size in bits, not used             */
    0,                                           /* Block size in bytes                    */
    sizeof(NX_CRYPTO_ECDH),                      /* Metadata size in bytes                 */
    _nx_crypto_method_ecdh_init,                 /* ECDH initialization routine            */
    _nx_crypto_method_ecdh_cleanup,              /* ECDH cleanup routine                   */
    _nx_crypto_method_ecdh_operation             /* ECDH operation                         */
};

/* Declare the HMAC SHA1 authentication method */
NX_CRYPTO_METHOD crypto_method_hmac_sha1 =
{
    NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_160,      /* HMAC SHA1 algorithm                   */
    0,                                           /* Key size in bits                      */
    0,                                           /* IV size in bits, not used             */
    NX_CRYPTO_HMAC_SHA1_ICV_FULL_LEN_IN_BITS,    /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA1_BLOCK_SIZE_IN_BYTES,          /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA1_HMAC),                 /* Metadata size in bytes                */
    _nx_crypto_method_hmac_sha1_init,            /* HMAC SHA1 initialization routine      */
    _nx_crypto_method_hmac_sha1_cleanup,         /* HMAC SHA1 cleanup routine             */
    _nx_crypto_method_hmac_sha1_operation        /* HMAC SHA1 operation                   */
};

/* Declare the HMAC SHA224 authentication method */
NX_CRYPTO_METHOD crypto_method_hmac_sha224 =
{
    NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_224,       /* HMAC SHA224 algorithm                 */
    0,                                            /* Key size in bits                      */
    0,                                            /* IV size in bits, not used             */
    NX_CRYPTO_HMAC_SHA224_ICV_FULL_LEN_IN_BITS,   /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA2_BLOCK_SIZE_IN_BYTES,           /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA256_HMAC),                /* Metadata size in bytes                */
    _nx_crypto_method_hmac_sha256_init,           /* HMAC SHA224 initialization routine    */
    _nx_crypto_method_hmac_sha256_cleanup,        /* HMAC SHA224 cleanup routine           */
    _nx_crypto_method_hmac_sha256_operation       /* HMAC SHA224 operation                 */
};

/* Declare the HMAC SHA256 authentication method */
NX_CRYPTO_METHOD crypto_method_hmac_sha256 =
{
    NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256,       /* HMAC SHA256 algorithm                 */
    0,                                            /* Key size in bits                      */
    0,                                            /* IV size in bits, not used             */
    NX_CRYPTO_HMAC_SHA256_ICV_FULL_LEN_IN_BITS,   /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA2_BLOCK_SIZE_IN_BYTES,           /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA256_HMAC),                /* Metadata size in bytes                */
    _nx_crypto_method_hmac_sha256_init,           /* HMAC SHA256 initialization routine    */
    _nx_crypto_method_hmac_sha256_cleanup,        /* HMAC SHA256 cleanup routine           */
    _nx_crypto_method_hmac_sha256_operation       /* HMAC SHA256 operation                 */
};

/* Declare the HMAC SHA384 authentication method */
NX_CRYPTO_METHOD crypto_method_hmac_sha384 =
{
    NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_384,       /* HMAC SHA384 algorithm                 */
    0,                                            /* Key size in bits                      */
    0,                                            /* IV size in bits, not used             */
    NX_CRYPTO_HMAC_SHA384_ICV_FULL_LEN_IN_BITS,   /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES,         /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA512_HMAC),                /* Metadata size in bytes                */
    _nx_crypto_method_hmac_sha512_init,           /* HMAC SHA384 initialization routine    */
    _nx_crypto_method_hmac_sha512_cleanup,        /* HMAC SHA384 cleanup routine           */
    _nx_crypto_method_hmac_sha512_operation       /* HMAC SHA384 operation                 */
};

/* Declare the HMAC SHA512 authentication method */
NX_CRYPTO_METHOD crypto_method_hmac_sha512 =
{
    NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512,       /* HMAC SHA512 algorithm                 */
    0,                                            /* Key size in bits                      */
    0,                                            /* IV size in bits, not used             */
    NX_CRYPTO_HMAC_SHA512_ICV_FULL_LEN_IN_BITS,   /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES,         /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA512_HMAC),                /* Metadata size in bytes                */
    _nx_crypto_method_hmac_sha512_init,           /* HMAC SHA512 initialization routine    */
    _nx_crypto_method_hmac_sha512_cleanup,        /* HMAC SHA512 cleanup routine           */
    _nx_crypto_method_hmac_sha512_operation       /* HMAC SHA512 operation                 */
};

/* Declare the HMAC SHA512 authentication method */
NX_CRYPTO_METHOD crypto_method_hmac_sha512_224 =
{
    NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512_224,   /* HMAC SHA512224 algorithm              */
    0,                                            /* Key size in bits                      */
    0,                                            /* IV size in bits, not used             */
    NX_CRYPTO_HMAC_SHA512_224_ICV_FULL_LEN_IN_BITS,/* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES,         /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA512_HMAC),                /* Metadata size in bytes                */
    _nx_crypto_method_hmac_sha512_init,           /* HMAC SHA512224 initialization routine */
    _nx_crypto_method_hmac_sha512_cleanup,        /* HMAC SHA512224 cleanup routine        */
    _nx_crypto_method_hmac_sha512_operation       /* HMAC SHA512224 operation              */
};

/* Declare the HMAC SHA512 authentication method */
NX_CRYPTO_METHOD crypto_method_hmac_sha512_256 =
{
    NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512_256,   /* HMAC SHA512256 algorithm              */
    0,                                            /* Key size in bits                      */
    0,                                            /* IV size in bits, not used             */
    NX_CRYPTO_HMAC_SHA512_256_ICV_FULL_LEN_IN_BITS,/* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES,         /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA512_HMAC),                /* Metadata size in bytes                */
    _nx_crypto_method_hmac_sha512_init,           /* HMAC SHA512256 initialization routine */
    _nx_crypto_method_hmac_sha512_cleanup,        /* HMAC SHA512256 cleanup routine        */
    _nx_crypto_method_hmac_sha512_operation       /* HMAC SHA512256 operation              */
};

/* Declare the HMAC MD5 authentication method */
NX_CRYPTO_METHOD crypto_method_hmac_md5 =
{
    NX_CRYPTO_AUTHENTICATION_HMAC_MD5_128,            /* HMAC MD5 algorithm                    */
    0,                                                /* Key size in bits                      */
    0,                                                /* IV size in bits, not used             */
    NX_CRYPTO_HMAC_MD5_ICV_FULL_LEN_IN_BITS,          /* Transmitted ICV size in bits          */
    NX_CRYPTO_MD5_BLOCK_SIZE_IN_BYTES,                /* Block size in bytes                   */
    sizeof(NX_CRYPTO_MD5_HMAC),                       /* Metadata size in bytes                */
    _nx_crypto_method_hmac_md5_init,                  /* HMAC MD5 initialization routine       */
    _nx_crypto_method_hmac_md5_cleanup,               /* HMAC MD5 cleanup routine              */
    _nx_crypto_method_hmac_md5_operation              /* HMAC MD5 operation                    */
};

/* Declare the RSA public cipher method. */
NX_CRYPTO_METHOD crypto_method_rsa =
{
    NX_CRYPTO_KEY_EXCHANGE_RSA,               /* RSA crypto algorithm                   */
    0,                                        /* Key size in bits                       */
    0,                                        /* IV size in bits                        */
    0,                                        /* ICV size in bits, not used.            */
    0,                                        /* Block size in bytes.                   */
    sizeof(NX_CRYPTO_RSA),                    /* Metadata size in bytes                 */
    _nx_crypto_method_rsa_init,               /* RSA initialization routine.            */
    _nx_crypto_method_rsa_cleanup,            /* RSA cleanup routine                    */
    _nx_crypto_method_rsa_operation           /* RSA operation                          */
};

/* Declare a placeholder for PSK authentication. */
NX_CRYPTO_METHOD crypto_method_auth_psk =
{
    NX_CRYPTO_KEY_EXCHANGE_PSK,               /* PSK placeholder                        */
    0,                                        /* Key size in bits                       */
    0,                                        /* IV size in bits                        */
    0,                                        /* ICV size in bits, not used.            */
    0,                                        /* Block size in bytes.                   */
    0,                                        /* Metadata size in bytes                 */
    NX_CRYPTO_NULL,                           /* Initialization routine.                */
    NX_CRYPTO_NULL,                           /* Cleanup routine, not used.             */
    NX_CRYPTO_NULL                            /* Operation                              */
};

/* Declare a placeholder for ECJ-PAKE. */
NX_CRYPTO_METHOD crypto_method_auth_ecjpake =
{
    NX_CRYPTO_KEY_EXCHANGE_ECJPAKE,           /* ECJ-PAKE placeholder                   */
    0,                                        /* Key size in bits                       */
    0,                                        /* IV size in bits                        */
    0,                                        /* ICV size in bits, not used.            */
    0,                                        /* Block size in bytes                    */
    sizeof(NX_CRYPTO_ECJPAKE),                /* Metadata size in bytes                 */
    _nx_crypto_method_ecjpake_init,           /* ECJPAKE initialization routine         */
    _nx_crypto_method_ecjpake_cleanup,        /* ECJPAKE cleanup routine                */
    _nx_crypto_method_ecjpake_operation,      /* ECJPAKE operation                      */
};

/* Declare a placeholder for EC SECP192R1. */
NX_CRYPTO_METHOD crypto_method_ec_secp192 =
{
    NX_CRYPTO_EC_SECP192R1,                   /* EC placeholder                         */
    192,                                      /* Key size in bits                       */
    0,                                        /* IV size in bits                        */
    0,                                        /* ICV size in bits, not used.            */
    0,                                        /* Block size in bytes.                   */
    0,                                        /* Metadata size in bytes                 */
    NX_CRYPTO_NULL,                           /* Initialization routine.                */
    NX_CRYPTO_NULL,                           /* Cleanup routine, not used.             */
    _nx_crypto_method_ec_secp192r1_operation, /* Operation                              */
};

/* Declare a placeholder for EC SECP224R1. */
NX_CRYPTO_METHOD crypto_method_ec_secp224 =
{
    NX_CRYPTO_EC_SECP224R1,                   /* EC placeholder                         */
    224,                                      /* Key size in bits                       */
    0,                                        /* IV size in bits                        */
    0,                                        /* ICV size in bits, not used.            */
    0,                                        /* Block size in bytes.                   */
    0,                                        /* Metadata size in bytes                 */
    NX_CRYPTO_NULL,                           /* Initialization routine.                */
    NX_CRYPTO_NULL,                           /* Cleanup routine, not used.             */
    _nx_crypto_method_ec_secp224r1_operation, /* Operation                              */
};

/* Declare a placeholder for EC SECP256R1. */
NX_CRYPTO_METHOD crypto_method_ec_secp256 =
{
    NX_CRYPTO_EC_SECP256R1,                   /* EC placeholder                         */
    256,                                      /* Key size in bits                       */
    0,                                        /* IV size in bits                        */
    0,                                        /* ICV size in bits, not used.            */
    0,                                        /* Block size in bytes.                   */
    0,                                        /* Metadata size in bytes                 */
    NX_CRYPTO_NULL,                           /* Initialization routine.                */
    NX_CRYPTO_NULL,                           /* Cleanup routine, not used.             */
    _nx_crypto_method_ec_secp256r1_operation, /* Operation                              */
};

/* Declare a placeholder for EC SECP384R1. */
NX_CRYPTO_METHOD crypto_method_ec_secp384 =
{
    NX_CRYPTO_EC_SECP384R1,                   /* EC placeholder                         */
    384,                                      /* Key size in bits                       */
    0,                                        /* IV size in bits                        */
    0,                                        /* ICV size in bits, not used.            */
    0,                                        /* Block size in bytes.                   */
    0,                                        /* Metadata size in bytes                 */
    NX_CRYPTO_NULL,                           /* Initialization routine.                */
    NX_CRYPTO_NULL,                           /* Cleanup routine, not used.             */
    _nx_crypto_method_ec_secp384r1_operation, /* Operation                              */
};

/* Declare a placeholder for EC SECP521R1. */
NX_CRYPTO_METHOD crypto_method_ec_secp521 =
{
    NX_CRYPTO_EC_SECP521R1,                   /* EC placeholder                         */
    521,                                      /* Key size in bits                       */
    0,                                        /* IV size in bits                        */
    0,                                        /* ICV size in bits, not used.            */
    0,                                        /* Block size in bytes.                   */
    0,                                        /* Metadata size in bytes                 */
    NX_CRYPTO_NULL,                           /* Initialization routine.                */
    NX_CRYPTO_NULL,                           /* Cleanup routine, not used.             */
    _nx_crypto_method_ec_secp521r1_operation, /* Operation                              */
};

#ifdef NX_CRYPTO_ENABLE_CURVE25519_448

/* Declare a placeholder for EC x25519. */
NX_CRYPTO_METHOD crypto_method_ec_x25519 =
{
    NX_CRYPTO_EC_X25519,                      /* EC placeholder                         */
    255,                                      /* Key size in bits                       */
    0,                                        /* IV size in bits                        */
    0,                                        /* ICV size in bits, not used.            */
    0,                                        /* Block size in bytes.                   */
    0,                                        /* Metadata size in bytes                 */
    NX_CRYPTO_NULL,                           /* Initialization routine.                */
    NX_CRYPTO_NULL,                           /* Cleanup routine, not used.             */
    _nx_crypto_method_ec_x25519_operation,    /* Operation                              */
};

/* Declare a placeholder for EC x448. */
NX_CRYPTO_METHOD crypto_method_ec_x448 =
{
    NX_CRYPTO_EC_X448,                        /* EC placeholder                         */
    448,                                      /* Key size in bits                       */
    0,                                        /* IV size in bits                        */
    0,                                        /* ICV size in bits, not used.            */
    0,                                        /* Block size in bytes.                   */
    0,                                        /* Metadata size in bytes                 */
    NX_CRYPTO_NULL,                           /* Initialization routine.                */
    NX_CRYPTO_NULL,                           /* Cleanup routine, not used.             */
    _nx_crypto_method_ec_x448_operation,      /* Operation                              */
};

#endif /* NX_CRYPTO_ENABLE_CURVE25519_448 */

/* Declare the public NULL cipher (not to be confused with the NULL methods above). This
 * is used as a placeholder in ciphersuites that do not use a cipher method for a
 * particular operation (e.g. some PSK ciphersuites don't use a public-key algorithm
 * like RSA).
 */
NX_CRYPTO_METHOD crypto_method_public_null =
{
    NX_CRYPTO_ENCRYPTION_NULL,                /* Name of the crypto algorithm          */
    0,                                        /* Key size in bits, not used            */
    0,                                        /* IV size in bits, not used             */
    0,                                        /* ICV size in bits, not used            */
    1,                                        /* Block size in bytes                   */
    0,                                        /* Metadata size in bytes                */
    _nx_crypto_method_null_init,              /* NULL initialization routine           */
    _nx_crypto_method_null_cleanup,           /* NULL cleanup routine                  */
    _nx_crypto_method_null_operation          /* NULL operation                        */
};



/* Declare the MD5 hash method */
NX_CRYPTO_METHOD crypto_method_md5 =
{
    NX_CRYPTO_HASH_MD5,                            /* MD5 algorithm                         */
    0,                                             /* Key size in bits                      */
    0,                                             /* IV size in bits, not used             */
    NX_CRYPTO_MD5_ICV_LEN_IN_BITS,                 /* Transmitted ICV size in bits          */
    NX_CRYPTO_MD5_BLOCK_SIZE_IN_BYTES,             /* Block size in bytes                   */
    sizeof(NX_CRYPTO_MD5),                         /* Metadata size in bytes                */
    _nx_crypto_method_md5_init,                    /* MD5 initialization routine            */
    _nx_crypto_method_md5_cleanup,                 /* MD5 cleanup routine                   */
    _nx_crypto_method_md5_operation                /* MD5 operation                         */
};

/* Declare the SHA1 hash method */
NX_CRYPTO_METHOD crypto_method_sha1 =
{
    NX_CRYPTO_HASH_SHA1,                           /* SHA1 algorithm                        */
    0,                                             /* Key size in bits                      */
    0,                                             /* IV size in bits, not used             */
    NX_CRYPTO_SHA1_ICV_LEN_IN_BITS,                /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA1_BLOCK_SIZE_IN_BYTES,            /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA1),                        /* Metadata size in bytes                */
    _nx_crypto_method_sha1_init,                   /* SHA1 initialization routine           */
    _nx_crypto_method_sha1_cleanup,                /* SHA1 cleanup routine                  */
    _nx_crypto_method_sha1_operation               /* SHA1 operation                        */
};

/* Declare the SHA224 hash method */
NX_CRYPTO_METHOD crypto_method_sha224 =
{
    NX_CRYPTO_HASH_SHA224,                         /* SHA224 algorithm                      */
    0,                                             /* Key size in bits                      */
    0,                                             /* IV size in bits, not used             */
    NX_CRYPTO_SHA224_ICV_LEN_IN_BITS,              /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA2_BLOCK_SIZE_IN_BYTES,            /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA256),                      /* Metadata size in bytes                */
    _nx_crypto_method_sha256_init,                 /* SHA224 initialization routine         */
    _nx_crypto_method_sha256_cleanup,              /* SHA224 cleanup routine                */
    _nx_crypto_method_sha256_operation             /* SHA224 operation                      */
};

/* Declare the SHA256 hash method */
NX_CRYPTO_METHOD crypto_method_sha256 =
{
    NX_CRYPTO_HASH_SHA256,                         /* SHA256 algorithm                      */
    0,                                             /* Key size in bits                      */
    0,                                             /* IV size in bits, not used             */
    NX_CRYPTO_SHA256_ICV_LEN_IN_BITS,              /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA2_BLOCK_SIZE_IN_BYTES,            /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA256),                      /* Metadata size in bytes                */
    _nx_crypto_method_sha256_init,                 /* SHA256 initialization routine         */
    _nx_crypto_method_sha256_cleanup,              /* SHA256 cleanup routine                */
    _nx_crypto_method_sha256_operation             /* SHA256 operation                      */
};

/* Declare the SHA384 hash method */
NX_CRYPTO_METHOD crypto_method_sha384 =
{
    NX_CRYPTO_HASH_SHA384,                         /* SHA384 algorithm                      */
    0,                                             /* Key size in bits                      */
    0,                                             /* IV size in bits, not used             */
    NX_CRYPTO_SHA384_ICV_LEN_IN_BITS,              /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES,          /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA512),                      /* Metadata size in bytes                */
    _nx_crypto_method_sha512_init,                 /* SHA384 initialization routine         */
    _nx_crypto_method_sha512_cleanup,              /* SHA384 cleanup routine                */
    _nx_crypto_method_sha512_operation             /* SHA384 operation                      */
};

/* Declare the SHA512 hash method */
NX_CRYPTO_METHOD crypto_method_sha512 =
{
    NX_CRYPTO_HASH_SHA512,                         /* SHA512 algorithm                      */
    0,                                             /* Key size in bits                      */
    0,                                             /* IV size in bits, not used             */
    NX_CRYPTO_SHA512_ICV_LEN_IN_BITS,              /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES,          /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA512),                      /* Metadata size in bytes                */
    _nx_crypto_method_sha512_init,                 /* SHA512 initialization routine         */
    _nx_crypto_method_sha512_cleanup,              /* SHA512 cleanup routine                */
    _nx_crypto_method_sha512_operation             /* SHA512 operation                      */
};

/* Declare the SHA512/224 hash method */
NX_CRYPTO_METHOD crypto_method_sha512_224 =
{
    NX_CRYPTO_HASH_SHA512_224,                     /* SHA512224 algorithm                   */
    0,                                             /* Key size in bits                      */
    0,                                             /* IV size in bits, not used             */
    NX_CRYPTO_SHA512_224_ICV_LEN_IN_BITS,          /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES,          /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA512),                      /* Metadata size in bytes                */
    _nx_crypto_method_sha512_init,                 /* SHA512224 initialization routine      */
    _nx_crypto_method_sha512_cleanup,              /* SHA512224 cleanup routine             */
    _nx_crypto_method_sha512_operation             /* SHA512224 operation                   */
};

/* Declare the SHA512/256 hash method */
NX_CRYPTO_METHOD crypto_method_sha512_256 =
{
    NX_CRYPTO_HASH_SHA512_256,                     /* SHA512256 algorithm                   */
    0,                                             /* Key size in bits                      */
    0,                                             /* IV size in bits, not used             */
    NX_CRYPTO_SHA512_256_ICV_LEN_IN_BITS,          /* Transmitted ICV size in bits          */
    NX_CRYPTO_SHA512_BLOCK_SIZE_IN_BYTES,          /* Block size in bytes                   */
    sizeof(NX_CRYPTO_SHA512),                      /* Metadata size in bytes                */
    _nx_crypto_method_sha512_init,                 /* SHA512256 initialization routine      */
    _nx_crypto_method_sha512_cleanup,              /* SHA512256 cleanup routine             */
    _nx_crypto_method_sha512_operation             /* SHA512256 operation                   */
};

/* Declare the TLSv1.0/1.1 PRF hash method */
NX_CRYPTO_METHOD crypto_method_tls_prf_1 =
{
    NX_CRYPTO_PRF_HMAC_SHA1,                       /* TLS PRF algorithm                     */
    0,                                             /* Key size in bits, not used            */
    0,                                             /* IV size in bits, not used             */
    0,                                             /* Transmitted ICV size in bits, not used*/
    0,                                             /* Block size in bytes, not used         */
    sizeof(NX_CRYPTO_TLS_PRF_1),                   /* Metadata size in bytes                */
    _nx_crypto_method_prf_1_init,                  /* TLS PRF 1 initialization routine      */
    _nx_crypto_method_prf_1_cleanup,               /* TLS PRF 1 cleanup routine             */
    _nx_crypto_method_prf_1_operation              /* TLS PRF 1 operation                   */
};

/* For backward compatibility, this symbol is defined here */
#define crypto_method_tls_prf_sha_256 crypto_method_tls_prf_sha256
/* Declare the TLSv1.2 default PRF hash method */
NX_CRYPTO_METHOD crypto_method_tls_prf_sha256 =
{
    NX_CRYPTO_PRF_HMAC_SHA2_256,                   /* TLS PRF algorithm                     */
    0,                                             /* Key size in bits, not used            */
    0,                                             /* IV size in bits, not used             */
    0,                                             /* Transmitted ICV size in bits, not used*/
    0,                                             /* Block size in bytes, not used         */
    sizeof(NX_CRYPTO_TLS_PRF_SHA256),              /* Metadata size in bytes                */
    _nx_crypto_method_prf_sha_256_init,            /* TLS PRF SHA256 initialization routine */
    _nx_crypto_method_prf_sha_256_cleanup,         /* TLS PRF SHA256 cleanup routine        */
    _nx_crypto_method_prf_sha_256_operation        /* TLS PRF SHA256 operation              */
};

/* Declare the TLSv1.2 default PRF hash method */
NX_CRYPTO_METHOD crypto_method_tls_prf_sha384 =
{
    NX_CRYPTO_PRF_HMAC_SHA2_384,                   /* TLS PRF algorithm                     */
    0,                                             /* Key size in bits, not used            */
    0,                                             /* IV size in bits, not used             */
    0,                                             /* Transmitted ICV size in bits, not used*/
    0,                                             /* Block size in bytes, not used         */
    sizeof(NX_CRYPTO_TLS_PRF_SHA384),              /* Metadata size in bytes                */
    _nx_crypto_method_prf_sha384_init,             /* TLS PRF SHA384 initialization routine */
    _nx_crypto_method_prf_sha384_cleanup,          /* TLS PRF SHA384 cleanup routine        */
    _nx_crypto_method_prf_sha384_operation         /* TLS PRF SHA384 operation              */
};

/* Declare the TLSv1.2 default PRF hash method */
NX_CRYPTO_METHOD crypto_method_tls_prf_sha512 =
{
    NX_CRYPTO_PRF_HMAC_SHA2_512,                   /* TLS PRF algorithm                     */
    0,                                             /* Key size in bits, not used            */
    0,                                             /* IV size in bits, not used             */
    0,                                             /* Transmitted ICV size in bits, not used*/
    0,                                             /* Block size in bytes, not used         */
    sizeof(NX_CRYPTO_TLS_PRF_SHA512),              /* Metadata size in bytes                */
    _nx_crypto_method_prf_sha512_init,             /* TLS PRF SHA512 initialization routine */
    _nx_crypto_method_prf_sha512_cleanup,          /* TLS PRF SHA512 cleanup routine        */
    _nx_crypto_method_prf_sha512_operation         /* TLS PRF SHA512 operation              */
};

/* Define generic HMAC cryptographic routine. */
NX_CRYPTO_METHOD crypto_method_hmac =
{
    NX_CRYPTO_HASH_HMAC,                            /* HMAC algorithm                        */
    0,                                              /* Key size in bits, not used            */
    0,                                              /* IV size in bits, not used             */
    0,                                              /* Transmitted ICV size in bits, not used*/
    0,                                              /* Block size in bytes, not used         */
    sizeof(NX_CRYPTO_HMAC),                         /* Metadata size in bytes                */
    _nx_crypto_method_hmac_init,                    /* HKDF initialization routine           */
    _nx_crypto_method_hmac_cleanup,                 /* HKDF cleanup routine                  */
    _nx_crypto_method_hmac_operation                /* HKDF operation                        */
};


/* Define generic HMAC-based Key Derivation Function method. */
NX_CRYPTO_METHOD crypto_method_hkdf =
{
    NX_CRYPTO_HKDF_METHOD,                          /* HKDF algorithm                        */
    0,                                              /* Key size in bits, not used            */
    0,                                              /* IV size in bits, not used             */
    0,                                              /* Transmitted ICV size in bits, not used*/
    0,                                              /* Block size in bytes, not used         */
    sizeof(NX_CRYPTO_HKDF) + sizeof(NX_CRYPTO_HMAC),/* Metadata size in bytes                */
    _nx_crypto_method_hkdf_init,                    /* HKDF initialization routine           */
    _nx_crypto_method_hkdf_cleanup,                 /* HKDF cleanup routine                  */
    _nx_crypto_method_hkdf_operation                /* HKDF operation                        */
};

/* Declare the 3DES-CBC 128 encrytion method. */
NX_CRYPTO_METHOD crypto_method_des =
{
    NX_CRYPTO_ENCRYPTION_DES_CBC,                /* DES crypto algorithm                   */ /* lgtm[cpp/weak-cryptographic-algorithm] */
    NX_CRYPTO_DES_KEY_LEN_IN_BITS,               /* Key size in bits                       */ /* lgtm[cpp/weak-cryptographic-algorithm] */
    NX_CRYPTO_DES_IV_LEN_IN_BITS,                /* IV size in bits                        */ /* lgtm[cpp/weak-cryptographic-algorithm] */
    0,                                           /* ICV size in bits, not used             */
    (NX_CRYPTO_DES_BLOCK_SIZE_IN_BITS >> 3),     /* Block size in bytes                    */ /* lgtm[cpp/weak-cryptographic-algorithm] */
    sizeof(NX_CRYPTO_DES),                       /* Metadata size in bytes                 */
    _nx_crypto_method_des_init,                  /* 3DES initialization routine            */
    _nx_crypto_method_des_cleanup,               /* 3DES cleanup routine                   */
    _nx_crypto_method_des_operation              /* 3DES operation                         */
};


/* Declare the 3DES-CBC 128 encrytion method. */
NX_CRYPTO_METHOD crypto_method_3des =
{
    NX_CRYPTO_ENCRYPTION_3DES_CBC,               /* 3DES crypto algorithm                  */ /* lgtm[cpp/weak-cryptographic-algorithm] */
    NX_CRYPTO_3DES_KEY_LEN_IN_BITS,              /* Key size in bits                       */ /* lgtm[cpp/weak-cryptographic-algorithm] */
    NX_CRYPTO_3DES_IV_LEN_IN_BITS,               /* IV size in bits                        */ /* lgtm[cpp/weak-cryptographic-algorithm] */
    0,                                           /* ICV size in bits, not used             */ 
    (NX_CRYPTO_3DES_BLOCK_SIZE_IN_BITS >> 3),    /* Block size in bytes                    */ /* lgtm[cpp/weak-cryptographic-algorithm] */
    sizeof(NX_CRYPTO_3DES),                      /* Metadata size in bytes                 */
    _nx_crypto_method_3des_init,                 /* 3DES initialization routine            */
    _nx_crypto_method_3des_cleanup,              /* 3DES cleanup routine                   */
    _nx_crypto_method_3des_operation             /* 3DES operation                         */
};

/* Declare the PKCS#1v1.5 encrytion method. */
NX_CRYPTO_METHOD crypto_method_pkcs1 =
{
    NX_CRYPTO_DIGITAL_SIGNATURE_RSA,             /* PKCS#1v1.5 crypto algorithm            */
    0,                                           /* Key size in bits, not used             */
    0,                                           /* IV size in bits, not used              */
    0,                                           /* ICV size in bits, not used             */
    0,                                           /* Block size in bytes, not used          */
    sizeof(NX_CRYPTO_PKCS1),                     /* Metadata size in bytes                 */
    _nx_crypto_method_pkcs1_v1_5_init,           /* PKCS#1v1.5 initialization routine      */
    _nx_crypto_method_pkcs1_v1_5_cleanup,        /* PKCS#1v1.5 cleanup routine             */
    _nx_crypto_method_pkcs1_v1_5_operation       /* PKCS#1v1.5 operation                   */
};
