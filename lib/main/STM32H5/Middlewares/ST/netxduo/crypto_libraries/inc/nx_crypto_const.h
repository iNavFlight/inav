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
/**   Crypto                                                              */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_crypto_const.h                                  PORTABLE C       */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines the NetX Security Encryption component.           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), added    */
/*                                            new constants,              */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Bhupendra Naphade        Modified comment(s),          */
/*                                            renamed FIPS symbol to      */
/*                                            self-test,                  */
/*                                            resulting in version 6.1.7  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), added    */
/*                                            x25519 and x448 curves,     */
/*                                            resulting in version 6.1.11 */
/*  10-31-2022     Yanwu Cai                Modified comment(s), added    */
/*                                            EC curve type macro,        */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

#ifndef _NX_CRYPTO_CONST_H_
#define _NX_CRYPTO_CONST_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

/* Define the encryption algorithm, as outlined in RFC 4305 3.1.1 */
/* These values are used in nx_crypto_algorithm field. */
/* Values of 16 least significant bits are the same as defined in RFC 5996 3.3.2 */
#define NX_CRYPTO_ENCRYPTION_MASK                0x00000000
#define NX_CRYPTO_NONE                           0x00000000
#define NX_CRYPTO_ENCRYPTION_DES_IV64            0x00000001
#define NX_CRYPTO_ENCRYPTION_DES_CBC             0x00000002
#define NX_CRYPTO_ENCRYPTION_3DES_CBC            0x00000003
#define NX_CRYPTO_ENCRYPTION_RC5                 0x00000004
#define NX_CRYPTO_ENCRYPTION_IDEA                0x00000005
#define NX_CRYPTO_ENCRYPTION_CAST                0x00000006
#define NX_CRYPTO_ENCRYPTION_BLOWFISH            0x00000007
#define NX_CRYPTO_ENCRYPTION_3IDEA               0x00000008
#define NX_CRYPTO_ENCRYPTION_DES_IV32            0x00000009
#define NX_CRYPTO_ENCRYPTION_NULL                0x0000000B
#define NX_CRYPTO_ENCRYPTION_AES_CBC             0x0000000C
#define NX_CRYPTO_ENCRYPTION_AES_CTR             0x0000000D
#define NX_CRYPTO_ENCRYPTION_AES_CCM_8           0x0000000E
#define NX_CRYPTO_ENCRYPTION_AES_CCM_12          0x0000000F
#define NX_CRYPTO_ENCRYPTION_AES_CCM_16          0x00000010
#define NX_CRYPTO_ENCRYPTION_AES_CCM             0x00000011     /* Unassigned number in IANA, define it for all other length ICV.  */
#define NX_CRYPTO_ENCRYPTION_AES_GCM_8           0x00000012
#define NX_CRYPTO_ENCRYPTION_AES_GCM_12          0x00000013
#define NX_CRYPTO_ENCRYPTION_AES_GCM_16          0x00000014
#define NX_CRYPTO_ENCRYPTION_NULL_AUTH_AES_GMAC  0x00000015
#define NX_CRYPTO_ENCRYPTION_CAMELLIA_CBC        0x00000017
#define NX_CRYPTO_ENCRYPTION_CAMELLIA_CTR        0x00000018
#define NX_CRYPTO_ENCRYPTION_CAMELLIA_CCM_8      0x00000019
#define NX_CRYPTO_ENCRYPTION_CAMELLIA_CCM_12     0x0000001A
#define NX_CRYPTO_ENCRYPTION_CAMELLIA_CCM_16     0x0000001B
#define NX_CRYPTO_ENCRYPTION_CHACHA20_POLY1305   0x0000001C


/* Define the authentication algorithm, as outlined in RFC 4305 3.2 */
/* See also: https://www.iana.org/assignments/ikev2-parameters/ikev2-parameters.xhtml */
/* These values are used in nx_crypto_algorithm field. */
/* Values of 16 least significant bits are the same as defined in RFC 5996 3.3.2 */
#define NX_CRYPTO_AUTHENTICATION_MASK            0x00010000
#define NX_CRYPTO_AUTHENTICATION_NONE            0x00010000
#define NX_CRYPTO_AUTHENTICATION_HMAC_MD5_96     0x00010001
#define NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_96    0x00010002
#define NX_CRYPTO_AUTHENTICATION_DES_MAC         0x00010003
#define NX_CRYPTO_AUTHENTICATION_KPDK_MD5        0x00010004
#define NX_CRYPTO_AUTHENTICATION_AES_XCBC_MAC_96 0x00010005
#define NX_CRYPTO_AUTHENTICATION_HMAC_MD5_128    0x00010006
#define NX_CRYPTO_AUTHENTICATION_HMAC_SHA1_160   0x00010007
#define NX_CRYPTO_AUTHENTICATION_AES_CMAC_96     0x00010008
#define NX_CRYPTO_AUTHENTICATION_AES_128_GMAC    0x00010009
#define NX_CRYPTO_AUTHENTICATION_AES_192_GMAC    0x0001000A
#define NX_CRYPTO_AUTHENTICATION_AES_256_GMAC    0x0001000B
#define NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_256   0x0001000C
#define NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_384   0x0001000D
#define NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512   0x0001000E
#define NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_224   0x0001000F /* Unassigned number in IANA. */
#define NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512_224 0x00010010 /* Unassigned number in IANA. */
#define NX_CRYPTO_AUTHENTICATION_HMAC_SHA2_512_256 0x00010011 /* Unassigned number in IANA. */

/* Define the Pseudorandom Function algorithm */
/* These values are used in nx_crypto_algorithm field. */
/* Values of 16 least significant bits are the same as defined in RFC 5996 3.3.2,
 * except for algorithms not found in that RFC such as the HKDF. */
#define NX_CRYPTO_PRF_MASK                       0x00020000
#define NX_CRYPTO_PRF_HMAC_MD5                   0x00020001
#define NX_CRYPTO_PRF_HMAC_SHA1                  0x00020002
#define NX_CRYPTO_PRF_HMAC_TIGER                 0x00020003
#define NX_CRYPTO_PRF_HMAC_AES128_XCBC           0x00020004
#define NX_CRYPTO_PRF_HMAC_SHA2_256              0x00020005
#define NX_CRYPTO_PRF_HMAC_SHA2_384              0x00020006
#define NX_CRYPTO_PRF_HMAC_SHA2_512              0x00020007
#define NX_CRYPTO_HKDF_METHOD                    0x00020008

/* Define the hash algorithm */
#define NX_CRYPTO_HASH_MASK                      0x00030000
#define NX_CRYPTO_HASH_NONE                      0x00030001
#define NX_CRYPTO_HASH_MD5                       0x00030002
#define NX_CRYPTO_HASH_SHA1                      0x00030003
#define NX_CRYPTO_HASH_SHA224                    0x00030004
#define NX_CRYPTO_HASH_SHA256                    0x00030005
#define NX_CRYPTO_HASH_SHA384                    0x00030006
#define NX_CRYPTO_HASH_SHA512                    0x00030007
#define NX_CRYPTO_HASH_SHA512_224                0x00030008
#define NX_CRYPTO_HASH_SHA512_256                0x00030009
#define NX_CRYPTO_HASH_HMAC                      0x0003000A /* Generic HMAC wrapper. */

/* Define the key exchange algorithm */
#define NX_CRYPTO_KEY_EXCHANGE_MASK              0x00040000
#define NX_CRYPTO_KEY_EXCHANGE_NONE              0x00040000
#define NX_CRYPTO_KEY_EXCHANGE_PSK               0x00040001
#define NX_CRYPTO_KEY_EXCHANGE_RSA               0x00040002
#define NX_CRYPTO_KEY_EXCHANGE_DH                0x00040003
#define NX_CRYPTO_KEY_EXCHANGE_DHE               0x00040004
#define NX_CRYPTO_KEY_EXCHANGE_ECDH              0x00040005
#define NX_CRYPTO_KEY_EXCHANGE_ECDHE             0x00040006
#define NX_CRYPTO_KEY_EXCHANGE_ECJPAKE           0x00040007

/*Define the digital signature algorithm */
#define NX_CRYPTO_DIGITAL_SIGNATURE_MASK         0x00050000
#define NX_CRYPTO_DIGITAL_SIGNATURE_ANONYMOUS    0x00050000
#define NX_CRYPTO_DIGITAL_SIGNATURE_RSA          0x00050001
#define NX_CRYPTO_DIGITAL_SIGNATURE_DSA          0x00050002
#define NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA        0x00050003

/*Define the elliptic curve algorithm */
/* Values of 16 least significant bits are the same as named curve defined in RFC 4492, section 5.1.1 */
#define NX_CRYPTO_EC_MASK                        0x00060000
#define NX_CRYPTO_EC_SECT163K1                   0x00060001
#define NX_CRYPTO_EC_SECT163R1                   0x00060002
#define NX_CRYPTO_EC_SECT163R2                   0x00060003
#define NX_CRYPTO_EC_SECT193R1                   0x00060004
#define NX_CRYPTO_EC_SECT193R2                   0x00060005
#define NX_CRYPTO_EC_SECT233K1                   0x00060006
#define NX_CRYPTO_EC_SECT233R1                   0x00060007
#define NX_CRYPTO_EC_SECT239K1                   0x00060008
#define NX_CRYPTO_EC_SECT283K1                   0x00060009
#define NX_CRYPTO_EC_SECT283R1                   0x0006000A
#define NX_CRYPTO_EC_SECT409K1                   0x0006000B
#define NX_CRYPTO_EC_SECT409R1                   0x0006000C
#define NX_CRYPTO_EC_SECT571K1                   0x0006000D
#define NX_CRYPTO_EC_SECT571R1                   0x0006000E
#define NX_CRYPTO_EC_SECP160K1                   0x0006000F
#define NX_CRYPTO_EC_SECP160R1                   0x00060010
#define NX_CRYPTO_EC_SECP160R2                   0x00060011
#define NX_CRYPTO_EC_SECP192K1                   0x00060012
#define NX_CRYPTO_EC_SECP192R1                   0x00060013
#define NX_CRYPTO_EC_SECP224K1                   0x00060014
#define NX_CRYPTO_EC_SECP224R1                   0x00060015
#define NX_CRYPTO_EC_SECP256K1                   0x00060016
#define NX_CRYPTO_EC_SECP256R1                   0x00060017
#define NX_CRYPTO_EC_SECP384R1                   0x00060018
#define NX_CRYPTO_EC_SECP521R1                   0x00060019
#define NX_CRYPTO_EC_BRAINPOOLP256r1             0x0006001A
#define NX_CRYPTO_EC_BRAINPOOLP384r1             0x0006001B
#define NX_CRYPTO_EC_BRAINPOOLP512r1             0x0006001C
#define NX_CRYPTO_EC_X25519                      0x0006001D
#define NX_CRYPTO_EC_X448                        0x0006001E
#define NX_CRYPTO_EC_FFDHE2048                   0x00060100
#define NX_CRYPTO_EC_FFDHE3072                   0x00060101
#define NX_CRYPTO_EC_FFDHE4096                   0x00060102
#define NX_CRYPTO_EC_FFDHE6144                   0x00060103
#define NX_CRYPTO_EC_FFDHE8192                   0x00060104
#define NX_CRYPTO_EC_PRIME                       0x0006FF01
#define NX_CRYPTO_EC_CHAR2                       0x0006FF02

/* Elliptic curve point format definitions. */
#define NX_CRYPTO_EC_POINT_UNCOMPRESSED              0
#define NX_CRYPTO_EC_POINT_ANSIX962_COMPRESSED_PRIME 1
#define NX_CRYPTO_EC_POINT_ANSIX962_COMPRESSED_CHAR2 2

/* Elliptic curve type definitions. */
#define NX_CRYPTO_EC_CURVE_TYPE_EXPLICIT_PRIME       1
#define NX_CRYPTO_EC_CURVE_TYPE_EXPLICIT_CHAR2       2
#define NX_CRYPTO_EC_CURVE_TYPE_NAMED_CURVE          3

/* Define crypto ICV bits size. */
#define NX_CRYPTO_AUTHENTICATION_ICV_TRUNC_BITS  96

#ifndef NX_CRYPTO_MAX_IV_SIZE_IN_BITS
#define NX_CRYPTO_MAX_IV_SIZE_IN_BITS            192
#endif /* NX_CRYPTO_MAX_IV_SIZE_IN_BYTES */

/* NX_CRYPTO_ROLE_xxx - used to identify the "role of a crypto algorithm
   in a ciphersuite/X.509 mapping. */
#define NX_CRYPTO_ROLE_NONE                      0    /* Used to indicate the end of a list. */
#define NX_CRYPTO_ROLE_KEY_EXCHANGE              1    /* Cipher is used for key exchange (e.g. RSA, ECDHE) */
#define NX_CRYPTO_ROLE_SIGNATURE_CRYPTO          2    /* Cipher is used for encrypting a signature (e.g. RSA, DSA) */
#define NX_CRYPTO_ROLE_SIGNATURE_HASH            3    /* Cipher is used to generate a signature hash (e.g. SHA-1, SHA-256) */
#define NX_CRYPTO_ROLE_SYMMETRIC                 4    /* Cipher is used for symmetric encryption (e.g. AES, RC4) */
#define NX_CRYPTO_ROLE_MAC_HASH                  5    /* Cipher is used for hash MAC generation (e.g. HMAC-SHA-1, HMAC-SHA-256) */
#define NX_CRYPTO_ROLE_PRF                       6    /* Cipher is used for TLS PRF (key generation). */
#define NX_CRYPTO_ROLE_HMAC                      7    /* Generic HMAC wrapper to be used with a "raw" hash function. */
#define NX_CRYPTO_ROLE_RAW_HASH                  8    /* A "raw" hash function is the cryptographic primitive without a wrapper (e.g. SHA-256, no HMAC). */

/* Define values used for nx_crypto_type. */
#define NX_CRYPTO_ENCRYPT                        1    /* ESP Encrypt (egress) */
#define NX_CRYPTO_DECRYPT                        2    /* ESP Decrypt (ingress) */
#define NX_CRYPTO_AUTHENTICATE                   3    /* AH Authenticate (egress) */
#define NX_CRYPTO_VERIFY                         4    /* AH Verify (ingress) */
#define NX_CRYPTO_HASH_INITIALIZE                5    /* Hash initialize */
#define NX_CRYPTO_HASH_UPDATE                    6    /* Hash update */
#define NX_CRYPTO_HASH_CALCULATE                 7    /* Hash calculate */
#define NX_CRYPTO_PRF                            8    /* For the TLS PRF function. */
#define NX_CRYPTO_SET_PRIME_P                    9    /* Set Prime number P.  This is used in software RSA implementation. */
#define NX_CRYPTO_SET_PRIME_Q                    10   /* Set Prime number Q.  This is used in software RSA implementation. */
#define NX_CRYPTO_SET_ADDITIONAL_DATA            11   /* Set additional data pointer and length. */
#define NX_CRYPTO_HASH_METHOD_SET                12   /* Set hash method. */
#define NX_CRYPTO_SIGNATURE_GENERATE             13   /* Signature generation. */
#define NX_CRYPTO_SIGNATURE_VERIFY               14   /* Signature verification. */
#define NX_CRYPTO_PRF_SET_HASH                   NX_CRYPTO_HASH_METHOD_SET

/* ECJPAKE operations. */
#define NX_CRYPTO_ECJPAKE_HASH_METHOD_SET               NX_CRYPTO_HASH_METHOD_SET
#define NX_CRYPTO_ECJPAKE_CURVE_SET                     21
#define NX_CRYPTO_ECJPAKE_CLIENT_HELLO_GENERATE         22
#define NX_CRYPTO_ECJPAKE_SERVER_HELLO_GENERATE         23
#define NX_CRYPTO_ECJPAKE_CLIENT_HELLO_PROCESS          24
#define NX_CRYPTO_ECJPAKE_SERVER_HELLO_PROCESS          25
#define NX_CRYPTO_ECJPAKE_CLIENT_KEY_EXCHANGE_GENERATE  26
#define NX_CRYPTO_ECJPAKE_SERVER_KEY_EXCHANGE_GENERATE  27
#define NX_CRYPTO_ECJPAKE_CLIENT_KEY_EXCHANGE_PROCESS   28
#define NX_CRYPTO_ECJPAKE_SERVER_KEY_EXCHANGE_PROCESS   29

#define NX_CRYPTO_ENCRYPT_INITIALIZE             30   /* Encrypt initialize  */
#define NX_CRYPTO_DECRYPT_INITIALIZE             31   /* Decrypt initialize  */
#define NX_CRYPTO_ENCRYPT_UPDATE                 32   /* Encrypt update */
#define NX_CRYPTO_DECRYPT_UPDATE                 33   /* Decrypt update */
#define NX_CRYPTO_ENCRYPT_CALCULATE              34   /* Final encrypt calculation */
#define NX_CRYPTO_DECRYPT_CALCULATE              35   /* Final decrypt calculation */

/* EC operations. */
#define NX_CRYPTO_EC_CURVE_GET                   40
#define NX_CRYPTO_EC_CURVE_SET                   41
#define NX_CRYPTO_EC_KEY_PAIR_GENERATE           42

/* DH and ECDH operations. */
#define NX_CRYPTO_DH_SETUP                       50
#define NX_CRYPTO_DH_CALCULATE                   51
#define NX_CRYPTO_DH_KEY_PAIR_IMPORT             52
#define NX_CRYPTO_DH_PRIVATE_KEY_EXPORT          53

/* DRBG operations. */
#define NX_CRYPTO_DRBG_OPTIONS_SET               60
#define NX_CRYPTO_DRBG_INSTANTIATE               61
#define NX_CRYPTO_DRBG_RESEED                    62
#define NX_CRYPTO_DRBG_GENERATE                  63

/* HKDF operations. */
#define NX_CRYPTO_HKDF_SET_HASH                  NX_CRYPTO_HASH_METHOD_SET   /* Set the generic hash routine to be used for HKDF. */
#define NX_CRYPTO_HKDF_EXTRACT                   70   /* Perform an HKDF-extract operation. */
#define NX_CRYPTO_HKDF_EXPAND                    71   /* Perform an HKDF-expand operation. */
#define NX_CRYPTO_HKDF_SET_PRK                   72   /* Set the Pseudo-Random Key for an HKDF-expand operation. */
#define NX_CRYPTO_HKDF_SET_HMAC                  73   /* Set the generic HMAC routine to be used for HKDF. */
#define NX_CRYPTO_HMAC_SET_HASH                  74   /* Set the generic hash routine to be used for HMAC operations. */

/* Define align MACRO to a byte boundry. */
#define NX_CRYPTO_ALIGN8(len)                    (((len) + 7) & ~7)

/* Find the offset of a structure. */
#define NX_CRYPTO_OFFSET(a, b)                   ((ULONG)(&(((a *)(0)) -> b)))


typedef UINT NX_CRYPTO_KEY_SIZE;

#define NX_CRYPTO_SUCCESS                        0x0           /* Function returned successfully. */
#define NX_CRYPTO_INVALID_LIBRARY                0x20001       /* Crypto library has not been initialized or failed 
                                                                  the Power On Self Test (POST). */
#define NX_CRYPTO_UNSUPPORTED_KEY_SIZE           0x20002       /* Unsupported key size.  */
#define NX_CRYPTO_AUTHENTICATION_FAILED          0x20003       /* Authentication failed.  */
#define NX_CRYPTO_INVALID_ALGORITHM              0x20004
#define NX_CRYPTO_INVALID_KEY                    0x20005
#define NX_CRYPTO_INVALID_BUFFER_SIZE            0x20006
#define NX_CRYPTO_PTR_ERROR                      0x20007
#define NX_CRYPTO_SIZE_ERROR                     0x20008
#define NX_CRYPTO_NOT_SUCCESSFUL                 0x20009
#define NX_CRYPTO_INVALID_PARAMETER              0x2000A
#define NX_CRYPTO_NO_INSTANCE                    0x2000B
#define NX_CRYPTO_METHOD_INITIALIZATION_FAILURE  0x2000C       /* A method was not properly initialized before use. */
#define NX_CRYPTO_METADATA_UNALIGNED             0x2000D       /* Crypto metadata must be 4-byte aligned. */
#define NX_CRYPTO_POINTER_ERROR                  0x2000E       /* An invalid (NULL?) pointer was passed into a crypto method. */
#define NX_CRYTPO_MISSING_ECC_CURVE              0x2000F       /* ECC curve lookup failed to return a matching curve. */
#define NX_CRYPTO_FORMAT_NOT_SUPPORTED           0x20010       /* Unsupported Format */

#define NX_CRYPTO_NULL                           0
#define NX_CRYPTO_FALSE                          0
#define NX_CRYPTO_TRUE                           1

/* The following symbols are mapped to the error code for backward compatibility. */
#define NX_CRYPTO_AES_UNSUPPORTED_KEY_SIZE       NX_CRYPTO_UNSUPPORTED_KEY_SIZE

#ifdef NX_CRYPTO_SELF_TEST
#define NX_CRYPTO_LIBRARY_STATE_UNINITIALIZED            0x00000001U
#define NX_CRYPTO_LIBRARY_STATE_POST_IN_PROGRESS         0x00000002U
#define NX_CRYPTO_LIBRARY_STATE_POST_FAILED              0x00000004U
#define NX_CRYPTO_LIBRARY_STATE_OPERATIONAL              0x80000000U

#endif /* NX_CRYPTO_SELF_TEST */


#ifdef __cplusplus
}
#endif

#endif /* _NX_CRYPTO_CONST_H_ */
