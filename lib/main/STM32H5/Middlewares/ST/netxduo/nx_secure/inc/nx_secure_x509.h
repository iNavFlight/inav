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
/**    X.509 Digital Certificates                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/


/**************************************************************************/
/*                                                                        */
/*  COMPONENT DEFINITION                                   RELEASE        */
/*                                                                        */
/*    nx_secure_x509.h                                    PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file defines all service prototypes and data structure         */
/*    definitions for X.509 implementation.                               */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            fixed key usage bit order,  */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            removed dependency on TLS,  */
/*                                            resulting in version 6.1.6  */
/*  06-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            supported hardware EC       */
/*                                            private key,                */
/*                                            resulting in version 6.1.7  */
/*  01-31-2022     Timothy Stapko           Modified comment(s),          */
/*                                            ignored public key in EC    */
/*                                            private key,                */
/*                                            resulting in version 6.1.10 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            checked expiration for all  */
/*                                            the certs in the chain,     */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/

#ifndef SRC_NX_SECURE_X509_H_
#define SRC_NX_SECURE_X509_H_

/* Determine if a C++ compiler is being used.  If so, ensure that standard
   C is used to process the API information.  */
#ifdef __cplusplus

/* Yes, C++ compiler is present.  Use standard C.  */
extern   "C" {

#endif

#include "nx_crypto.h"

/* Enable ECC by default. */
#ifndef NX_SECURE_DISABLE_ECC_CIPHERSUITE
#ifndef NX_SECURE_ENABLE_ECC_CIPHERSUITE
#define NX_SECURE_ENABLE_ECC_CIPHERSUITE
#endif
#else
#undef NX_SECURE_ENABLE_ECC_CIPHERSUITE
#endif

#ifndef NX_SECURE_CALLER_CHECKING_EXTERNS
#ifdef NX_CRYPTO_STANDALONE_ENABLE
#define NX_SECURE_CALLER_CHECKING_EXTERNS
#else
#define NX_SECURE_CALLER_CHECKING_EXTERNS               NX_CALLER_CHECKING_EXTERNS
#endif
#endif

#ifndef NX_THREADS_ONLY_CALLER_CHECKING
#ifdef NX_CRYPTO_STANDALONE_ENABLE
#define NX_THREADS_ONLY_CALLER_CHECKING
#endif
#endif

/* Define memcpy, memset and memcmp functions used internal. */
#ifndef NX_SECURE_MEMCPY
#define NX_SECURE_MEMCPY                                memcpy
#endif /* NX_SECURE_MEMCPY */

#ifndef NX_SECURE_MEMCMP
#define NX_SECURE_MEMCMP                                memcmp
#endif /* NX_SECURE_MEMCMP */

#ifndef NX_SECURE_MEMSET
#define NX_SECURE_MEMSET                                memset
#endif /* NX_SECURE_MEMSET */

#ifndef NX_SECURE_MEMMOVE
#define NX_SECURE_MEMMOVE                               memmove
#endif /* NX_SECURE_MEMMOVE */

/* Define extensions used for user defined actions during X509 parse. */
#ifndef NX_SECURE_X509_PARSE_CERTIFICATE_EXTENSION
#define NX_SECURE_X509_PARSE_CERTIFICATE_EXTENSION
#endif /* NX_SECURE_X509_PARSE_CERTIFICATE_EXTENSION */

#ifndef NX_SECURE_X509_CERTIFICATE_VERIFY_EXTENSION
#define NX_SECURE_X509_CERTIFICATE_VERIFY_EXTENSION
#endif /* NX_SECURE_X509_CERTIFICATE_VERIFY_EXTENSION */

#ifndef NX_SECURE_X509_PARSE_CRL_EXTENSION
#define NX_SECURE_X509_PARSE_CRL_EXTENSION
#endif /* NX_SECURE_X509_PARSE_CRL_EXTENSION */

#ifndef NX_SECURE_X509_CRL_VERIFY_EXTENSION
#define NX_SECURE_X509_CRL_VERIFY_EXTENSION
#endif /* NX_SECURE_X509_CRL_VERIFY_EXTENSION */

#ifndef NX_SECURE_X509_CERTIFICATE_INITIALIZE_EXTENSION
#define NX_SECURE_X509_CERTIFICATE_INITIALIZE_EXTENSION
#endif /* NX_SECURE_X509_CERTIFICATE_INITIALIZE_EXTENSION */

/* Return values for X509 errors. */
#define NX_SECURE_X509_SUCCESS                                    0     /* Successful return status. */
#define NX_SECURE_X509_MULTIBYTE_TAG_UNSUPPORTED                  0x181 /* We encountered a multi-byte ASN.1 tag - not currently supported. */
#define NX_SECURE_X509_ASN1_LENGTH_TOO_LONG                       0x182 /* Encountered a length value longer than we can handle. */
#define NX_SECURE_X509_FOUND_NON_ZERO_PADDING                     0x183 /* Expected a padding value of 0 - got something different. */
#define NX_SECURE_X509_MISSING_PUBLIC_KEY                         0x184 /* X509 expected a public key but didn't find one. */
#define NX_SECURE_X509_INVALID_PUBLIC_KEY                         0x185 /* Found a public key, but it is invalid or has an incorrect format. */
#define NX_SECURE_X509_INVALID_CERTIFICATE_SEQUENCE               0x186 /* The top-level ASN.1 block is not a sequence - invalid X509 certificate. */
#define NX_SECURE_X509_MISSING_SIGNATURE_ALGORITHM                0x187 /* Expecting a signature algorithm identifier, did not find it. */
#define NX_SECURE_X509_INVALID_CERTIFICATE_DATA                   0x188 /* Certificate identity data is in an invalid format. */
#define NX_SECURE_X509_UNEXPECTED_ASN1_TAG                        0x189 /* We were expecting a specific ASN.1 tag for X509 format but we got something else. */
#define NX_SECURE_PKCS1_INVALID_PRIVATE_KEY                       0x18A /* A PKCS#1 private key file was passed in, but the formatting was incorrect. */
#define NX_SECURE_X509_CHAIN_TOO_SHORT                            0x18B /* An X509 certificate chain was too short to hold the entire chain during chain building. */
#define NX_SECURE_X509_CHAIN_VERIFY_FAILURE                       0x18C /* An X509 certificate chain was unable to be verified (catch-all error). */
#define NX_SECURE_X509_PKCS7_PARSING_FAILED                       0x18D /* Parsing an X.509 PKCS#7-encoded signature failed. */
#define NX_SECURE_X509_CERTIFICATE_NOT_FOUND                      0x18E /* In looking up a certificate, no matching entry was found. */
#define NX_SECURE_X509_INVALID_VERSION                            0x18F /* A certificate included a field that isn't compatible with the given version. */
#define NX_SECURE_X509_INVALID_TAG_CLASS                          0x190 /* A certificate included an ASN.1 tag with an invalid tag class value. */
#define NX_SECURE_X509_INVALID_EXTENSIONS                         0x191 /* A certificate included an extensions TLV but that did not contain a sequence. */
#define NX_SECURE_X509_INVALID_EXTENSION_SEQUENCE                 0x192 /* A certificate included an extension sequence that was invalid X.509. */
#define NX_SECURE_X509_CERTIFICATE_EXPIRED                        0x193 /* A certificate had a "not after" field that was less than the current time. */
#define NX_SECURE_X509_CERTIFICATE_NOT_YET_VALID                  0x194 /* A certificate had a "not before" field that was greater than the current time. */
#define NX_SECURE_X509_CERTIFICATE_DNS_MISMATCH                   0x195 /* A certificate Common Name or Subject Alt Name did not match a given DNS TLD. */
#define NX_SECURE_X509_INVALID_DATE_FORMAT                        0x196 /* A certificate contained a date field that is not in a recognized format. */
#define NX_SECURE_X509_CRL_ISSUER_MISMATCH                        0x197 /* A provided CRL and certificate were not issued by the same Certificate Authority. */
#define NX_SECURE_X509_CRL_SIGNATURE_CHECK_FAILED                 0x198 /* A CRL signature check failed against its issuer. */
#define NX_SECURE_X509_CRL_CERTIFICATE_REVOKED                    0x199 /* A certificate was found in a valid CRL and has therefore been revoked. */
#define NX_SECURE_X509_WRONG_SIGNATURE_METHOD                     0x19A /* In attempting to validate a signature the signature method did not match the expected method. */
#define NX_SECURE_X509_EXTENSION_NOT_FOUND                        0x19B /* In looking for an extension, no extension with a matching ID was found. */
#define NX_SECURE_X509_ALT_NAME_NOT_FOUND                         0x19C /* A name was searched for in a subjectAltName extension but was not found. */
#define NX_SECURE_X509_INVALID_PRIVATE_KEY_TYPE                   0x19D /* Private key type given was unknown or invalid. */
#define NX_SECURE_X509_NAME_STRING_TOO_LONG                       0x19E /* A name passed as a parameter was too long for an internal fixed-size buffer. */
#define NX_SECURE_X509_EXT_KEY_USAGE_NOT_FOUND                    0x19F /* In parsing an ExtendedKeyUsage extension, the specified usage was not found. */
#define NX_SECURE_X509_KEY_USAGE_ERROR                            0x1A0 /* For use with key usage extensions - return this to indicate an error at the application level with key usage. */

/* Return values from TLS. */
#define NX_SECURE_X509_UNSUPPORTED_PUBLIC_CIPHER                  0x1A1 /* A certificate provided by a server specified a public-key operation we do not support. */
#define NX_SECURE_X509_INVALID_CERTIFICATE                        0x1A2 /* An X509 certificate did not parse correctly. */
#define NX_SECURE_X509_UNKNOWN_CERT_SIG_ALGORITHM                 0x1A3 /* A certificate during verification had an unsupported signature algorithm. */
#define NX_SECURE_X509_CERTIFICATE_SIG_CHECK_FAILED               0x1A4 /* A certificate signature verification check failed - certificate data did not match signature. */
#define NX_SECURE_X509_INVALID_SELF_SIGNED_CERT                   0x1A5 /* The remote host sent a self-signed certificate and NX_SECURE_ALLOW_SELF_SIGNED_CERTIFICATES is not defined. */
#define NX_SECURE_X509_ISSUER_CERTIFICATE_NOT_FOUND               0x1A6 /* A remote certificate was received with an issuer not in the local trusted store. */
#define NX_SECURE_X509_NO_CERT_SPACE_ALLOCATED                    0x1A7 /* No certificate space was allocated for incoming remote certificates. */
#define NX_SECURE_X509_INSUFFICIENT_CERT_SPACE                    0x1A8 /* Not enough certificate buffer space allocated for a certificate. */
#define NX_SECURE_X509_CERT_ID_DUPLICATE                          0x1A9 /* Tried to add a certificate with a numeric ID that was already used - needs to be unique. */
#define NX_SECURE_X509_MISSING_CRYPTO_ROUTINE                     0x1AA /* In attempting to perform a cryptographic operation, an entry in the ciphersuite table (or one of its function pointers) was NULL. */

/* Defines for working with private key types. */
#define NX_SECURE_X509_KEY_TYPE_USER_DEFINED_MASK                 (0xFFFF0000)

/* Private key type defines for initializing private key data associated with an X.509 certificate. */
#define NX_SECURE_X509_KEY_TYPE_NONE                              0x00000000 /* Default value for no key. */
#define NX_SECURE_X509_KEY_TYPE_RSA_PKCS1_DER                     0x00000001 /* DER-encoded PKCS-1 RSA private key. */
#define NX_SECURE_X509_KEY_TYPE_EC_DER                            0x00000002 /* DER-encoded EC private key. */
#define NX_SECURE_X509_KEY_TYPE_HARDWARE                          0x00000003 /* Hardware private key */


/*  ASN.1 Format:
 *  <Type, Length, Value>
 *  - Type is simply a tag value (defined below)
 *  - Length is:
 *     <= 127 (7 bits), length is the number of bytes of Value
 *     > 127, high bit is set, and lower 7 bits becomes the number of following bytes of *length*
 *       so 841 bytes of Value is encoded as 0x82, 0x03, 0x49 (0x82 = 2 bytes of length, 0x0349 = 841).
 *  - Value depends on the Tag
 *  */

/*  ASN.1 Tags relevant to X509. */
#define NX_SECURE_ASN_TAG_BER                                     0
#define NX_SECURE_ASN_TAG_BOOLEAN                                 1
#define NX_SECURE_ASN_TAG_INTEGER                                 2
#define NX_SECURE_ASN_TAG_BIT_STRING                              3
#define NX_SECURE_ASN_TAG_OCTET_STRING                            4
#define NX_SECURE_ASN_TAG_NULL                                    5
#define NX_SECURE_ASN_TAG_OID                                     6
#define NX_SECURE_ASN_TAG_OBJ_DESCRIPTOR                          7
#define NX_SECURE_ASN_TAG_EXTERNAL_INSTANCE                       8
#define NX_SECURE_ASN_TAG_REAL                                    9
#define NX_SECURE_ASN_TAG_ENUMERATED                              10
#define NX_SECURE_ASN_TAG_EMBEDDED_PPV                            11
#define NX_SECURE_ASN_TAG_UTF8_STRING                             12
#define NX_SECURE_ASN_TAG_RELATIVE_OID                            13
#define NX_SECURE_ASN_TAG_UNDEFINED_14                            14
#define NX_SECURE_ASN_TAG_UNDEFINED_15                            15
#define NX_SECURE_ASN_TAG_SEQUENCE                                16
#define NX_SECURE_ASN_TAG_SET                                     17
#define NX_SECURE_ASN_TAG_NUMERIC_STRING                          18
#define NX_SECURE_ASN_TAG_PRINTABLE_STRING                        19
#define NX_SECURE_ASN_TAG_TELETEX_STRING                          20
#define NX_SECURE_ASN_TAG_T61_STRING                              20
#define NX_SECURE_ASN_TAG_VIDEOTEX_STRING                         21
#define NX_SECURE_ASN_TAG_IA5_STRING                              22
#define NX_SECURE_ASN_TAG_UTC_TIME                                23
#define NX_SECURE_ASN_TAG_GENERALIZED_TIME                        24
#define NX_SECURE_ASN_TAG_GRAPHIC_STRING                          25
#define NX_SECURE_ASN_TAG_VISIBLE_STRING                          26
#define NX_SECURE_ASN_TAG_GENERAL_STRING                          27
#define NX_SECURE_ASN_TAG_UNIVERSAL_STRING                        28
#define NX_SECURE_ASN_TAG_CHARACTER_STRING                        29
#define NX_SECURE_ASN_TAG_BMP_STRING                              30

#define NX_SECURE_ASN_TAG_CONSTRUCTED_MASK                        0x20 /*  If bit 6 is set, it is a constructed type. */

#define NX_SECURE_ASN_TAG_CLASS_MASK                              0xC0 /* Top 2 bits of tag are the "class". */
#define NX_SECURE_ASN_TAG_MASK                                    0x1F /* Bottom 6 bits are the tag itself. */
#define NX_SECURE_ASN_TAG_MULTIBYTE_MASK                          0x1F /* Some tags are multi-byte but never in x509. */

/* Tag classes. Bits refer to bit locations in the tag octet.
 * Note that "Application" and "Private" are not recommended for use and
 * should probably never be encountered in an X.509 certificate.
 * Class            |    Bit 7    |    Bit 8   |
 * ---------------------------------------------
 * Universal        |      0      |      0     |
 * Application      |      0      |      1     |
 * Context-specific |      1      |      0     |
 * Private          |      1      |      1     |
 */
#define NX_SECURE_ASN_TAG_CLASS_UNIVERSAL                         0x00 /* ASN.1 standard tag values. */
#define NX_SECURE_ASN_TAG_CLASS_APPLICATION                       0x01 /* (UNUSED) Application-specific tag values. */
#define NX_SECURE_ASN_TAG_CLASS_CONTEXT                           0x02 /* Context-specific tag values. */
#define NX_SECURE_ASN_TAG_CLASS_PRIVATE                           0x03 /* (UNUSED) Private tag values. */

/* X.509 version identifiers. */
#define NX_SECURE_X509_VERSION_1                                  (0x0)
#define NX_SECURE_X509_VERSION_2                                  (0x1)
#define NX_SECURE_X509_VERSION_3                                  (0x2)

/* X.509 context-specific tag values. */
#define NX_SECURE_X509_TAG_VERSION                                (0x00) /* In TBSCertificate ASN.1, version field tag. */
#define NX_SECURE_X509_TAG_ISSUER_UNIQUE_ID                       (0x01) /* In TBSCertificate ASN.1, Issuer unique id field tag. */
#define NX_SECURE_X509_TAG_SUBJECT_UNIQUE_ID                      (0x02) /* In TBSCertificate ASN.1, Subject unique id field tag. */
#define NX_SECURE_X509_TAG_EXTENSIONS                             (0x03) /* In TBSCertificate ASN.1, Extensions field tag. */

/* X.509 subjectAltName context-specific tag values. */
#define NX_SECURE_X509_SUB_ALT_NAME_TAG_OTHERNAME                 (0)
#define NX_SECURE_X509_SUB_ALT_NAME_TAG_RFC822NAME                (1)
#define NX_SECURE_X509_SUB_ALT_NAME_TAG_DNSNAME                   (2)
#define NX_SECURE_X509_SUB_ALT_NAME_TAG_X400ADDRESS               (3)
#define NX_SECURE_X509_SUB_ALT_NAME_TAG_DIRECTORYNAME             (4)
#define NX_SECURE_X509_SUB_ALT_NAME_TAG_EDIPARTYNAME              (5)
#define NX_SECURE_X509_SUB_ALT_NAME_TAG_UNIFORMRESOURCEIDENTIFIER (6)
#define NX_SECURE_X509_SUB_ALT_NAME_TAG_IPADDRESS                 (7)
#define NX_SECURE_X509_SUB_ALT_NAME_TAG_REGISTEREDID              (8)

/* X.509 CRL context-specific tags. */
#define NX_SECURE_X509_CRL_TAG_EXTENSIONS                         (0x00) /* In CRL ASN.1, extensions field tag. */

/* X.509 KeyUsage extension bit field values. */
#define NX_SECURE_X509_KEY_USAGE_DIGITAL_SIGNATURE                (0x8000)
#define NX_SECURE_X509_KEY_USAGE_NON_REPUDIATION                  (0x4000)
#define NX_SECURE_X509_KEY_USAGE_KEY_ENCIPHERMENT                 (0x2000)
#define NX_SECURE_X509_KEY_USAGE_DATA_ENCIPHERMENT                (0X1000)
#define NX_SECURE_X509_KEY_USAGE_KEY_AGREEMENT                    (0X0800)
#define NX_SECURE_X509_KEY_USAGE_KEY_CERT_SIGN                    (0X0400)
#define NX_SECURE_X509_KEY_USAGE_CRL_SIGN                         (0X0200)
#define NX_SECURE_X509_KEY_USAGE_ENCIPHER_ONLY                    (0X0100)
#define NX_SECURE_X509_KEY_USAGE_DECIPHER_ONLY                    (0X0080)


/* Internal NetX Secure identifiers for X.509 OID values. The OIDs are variable-length multi-byte
   values so it's useful to have them map to a simple enumeration. */
#define NX_SECURE_TLS_X509_TYPE_UNKNOWN                           0
#define NX_SECURE_TLS_X509_TYPE_RSA                               1
#define NX_SECURE_TLS_X509_TYPE_RSA_MD5                           2
#define NX_SECURE_TLS_X509_TYPE_RSA_SHA_1                         3
#define NX_SECURE_TLS_X509_TYPE_RSA_SHA_256                       4
#define NX_SECURE_TLS_X509_TYPE_RSA_SHA_384                       5
#define NX_SECURE_TLS_X509_TYPE_RSA_SHA_512                       6
#define NX_SECURE_TLS_X509_TYPE_DH                                7
#define NX_SECURE_TLS_X509_TYPE_DSS_SHA_1                         8
#define NX_SECURE_TLS_X509_TYPE_COMMON_NAME                       9
#define NX_SECURE_TLS_X509_TYPE_EMAIL                             10
#define NX_SECURE_TLS_X509_TYPE_COUNTRY                           11
#define NX_SECURE_TLS_X509_TYPE_STATE                             12
#define NX_SECURE_TLS_X509_TYPE_LOCALITY                          13
#define NX_SECURE_TLS_X509_TYPE_ORGANIZATION                      14
#define NX_SECURE_TLS_X509_TYPE_ORG_UNIT                          15
/*#define NX_SECURE_TLS_X509_TYPE_EXTENSIONS_PREFIX               16*/
#define NX_SECURE_TLS_X509_TYPE_DIRECTORY_ATTRIBUTES              17
#define NX_SECURE_TLS_X509_TYPE_SUBJECT_KEY_ID                    18
#define NX_SECURE_TLS_X509_TYPE_KEY_USAGE                         19
#define NX_SECURE_TLS_X509_TYPE_SUBJECT_ALT_NAME                  20
#define NX_SECURE_TLS_X509_TYPE_ISSUER_ALT_NAME                   21
#define NX_SECURE_TLS_X509_TYPE_BASIC_CONSTRAINTS                 22
#define NX_SECURE_TLS_X509_TYPE_NAME_CONSTRAINTS                  23
#define NX_SECURE_TLS_X509_TYPE_CRL_DISTRIBUTION                  24
#define NX_SECURE_TLS_X509_TYPE_CERTIFICATE_POLICIES              25
#define NX_SECURE_TLS_X509_TYPE_CERT_POLICY_MAPPINGS              26
#define NX_SECURE_TLS_X509_TYPE_AUTHORITY_KEY_ID                  27
#define NX_SECURE_TLS_X509_TYPE_POLICY_CONSTRAINTS                28
#define NX_SECURE_TLS_X509_TYPE_EXTENDED_KEY_USAGE                29
#define NX_SECURE_TLS_X509_TYPE_ANY_EXTENDED_KEY_USAGE            30
#define NX_SECURE_TLS_X509_TYPE_FRESHEST_CRL                      31
#define NX_SECURE_TLS_X509_TYPE_INHIBIT_ANYPOLICY                 32
#define NX_SECURE_TLS_X509_TYPE_SURNAME                           33
#define NX_SECURE_TLS_X509_TYPE_SERIAL_NUMBER                     34
#define NX_SECURE_TLS_X509_TYPE_TITLE                             35
#define NX_SECURE_TLS_X509_TYPE_NAME                              36
#define NX_SECURE_TLS_X509_TYPE_GIVEN_NAME                        37
#define NX_SECURE_TLS_X509_TYPE_INITIALS                          38
#define NX_SECURE_TLS_X509_TYPE_GENERATION                        39
#define NX_SECURE_TLS_X509_TYPE_DN_QUALIFIER                      40
#define NX_SECURE_TLS_X509_TYPE_PSEUDONYM                         41
#define NX_SECURE_TLS_X509_TYPE_PKIX_EXT_PREFIX                   42
#define NX_SECURE_TLS_X509_TYPE_PKIX_AIA                          43
#define NX_SECURE_TLS_X509_TYPE_PKIX_SIA                          44
#define NX_SECURE_TLS_X509_TYPE_NETSCAPE_COMMENT                  45
#define NX_SECURE_TLS_X509_TYPE_ANY_POLICY                        46
#define NX_SECURE_TLS_X509_TYPE_PKIX_QT                           47
#define NX_SECURE_TLS_X509_TYPE_PKIX_QT_CPS                       48
#define NX_SECURE_TLS_X509_TYPE_PKIX_QT_UNOTICE                   49
#define NX_SECURE_TLS_X509_TYPE_PKIX_KP                           50
#define NX_SECURE_TLS_X509_TYPE_PKIX_KP_SERVER_AUTH               51
#define NX_SECURE_TLS_X509_TYPE_PKIX_KP_CLIENT_AUTH               52
#define NX_SECURE_TLS_X509_TYPE_PKIX_KP_CODE_SIGNING              53
#define NX_SECURE_TLS_X509_TYPE_PKIX_KP_EMAIL_PROTECT             54
#define NX_SECURE_TLS_X509_TYPE_PKIX_KP_TIME_STAMPING             55
#define NX_SECURE_TLS_X509_TYPE_PKIX_KP_OCSP_SIGNING              56
#define NX_SECURE_TLS_X509_TYPE_EC                                57
#define NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_1                       58
#define NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_224                     59
#define NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_256                     60
#define NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_384                     61
#define NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_512                     62

#define NX_SECURE_TLS_X509_EC_SECT163K1                           0x00060001
#define NX_SECURE_TLS_X509_EC_SECT163R1                           0x00060002
#define NX_SECURE_TLS_X509_EC_SECT163R2                           0x00060003
#define NX_SECURE_TLS_X509_EC_SECT193R1                           0x00060004
#define NX_SECURE_TLS_X509_EC_SECT193R2                           0x00060005
#define NX_SECURE_TLS_X509_EC_SECT233K1                           0x00060006
#define NX_SECURE_TLS_X509_EC_SECT233R1                           0x00060007
#define NX_SECURE_TLS_X509_EC_SECT239K1                           0x00060008
#define NX_SECURE_TLS_X509_EC_SECT283K1                           0x00060009
#define NX_SECURE_TLS_X509_EC_SECT283R1                           0x0006000A
#define NX_SECURE_TLS_X509_EC_SECT409K1                           0x0006000B
#define NX_SECURE_TLS_X509_EC_SECT409R1                           0x0006000C
#define NX_SECURE_TLS_X509_EC_SECT571K1                           0x0006000D
#define NX_SECURE_TLS_X509_EC_SECT571R1                           0x0006000E
#define NX_SECURE_TLS_X509_EC_SECP160K1                           0x0006000F
#define NX_SECURE_TLS_X509_EC_SECP160R1                           0x00060010
#define NX_SECURE_TLS_X509_EC_SECP160R2                           0x00060011
#define NX_SECURE_TLS_X509_EC_SECP192K1                           0x00060012
#define NX_SECURE_TLS_X509_EC_SECP192R1                           0x00060013
#define NX_SECURE_TLS_X509_EC_SECP224K1                           0x00060014
#define NX_SECURE_TLS_X509_EC_SECP224R1                           0x00060015
#define NX_SECURE_TLS_X509_EC_SECP256K1                           0x00060016
#define NX_SECURE_TLS_X509_EC_SECP256R1                           0x00060017
#define NX_SECURE_TLS_X509_EC_SECP384R1                           0x00060018
#define NX_SECURE_TLS_X509_EC_SECP521R1                           0x00060019

/* Bitfield mappings for Distinguished name comparison. When using nx_secure_x509_distinguished_name_compare,
   these values are used for the "compare_fields" parameter - bitwise OR these values together to compare
   only the desired fields.
 */
#define NX_SECURE_X509_NAME_COUNTRY                               (0x00000001L)
#define NX_SECURE_X509_NAME_ORGANIZATION                          (0x00000002L)
#define NX_SECURE_X509_NAME_ORG_UNIT                              (0x00000004L)
#define NX_SECURE_X509_NAME_QUALIFIER                             (0x00000008L)
#define NX_SECURE_X509_NAME_STATE                                 (0x00000010L)
#define NX_SECURE_X509_NAME_COMMON_NAME                           (0x00000020L)
#define NX_SECURE_X509_NAME_SERIAL_NUMBER                         (0x00000040L)
#define NX_SECURE_X509_NAME_LOCALITY                              (0x00000080L)
#define NX_SECURE_X509_NAME_TITLE                                 (0x00000100L)
#define NX_SECURE_X509_NAME_SURNAME                               (0x00000200L)
#define NX_SECURE_X509_NAME_GIVEN_NAME                            (0x00000400L)
#define NX_SECURE_X509_NAME_INITIALS                              (0x00000800L)
#define NX_SECURE_X509_NAME_PSEUDONYM                             (0x00001000L)
#define NX_SECURE_X509_NAME_GENERATION_QUALIFIER                  (0x00002000L)
#define NX_SECURE_X509_NAME_ALL_FIELDS                            (0xFFFFFFFFL)

/* Structure to contain distinguished names. */
typedef struct NX_SECURE_X509_DISTINGUISHED_NAME_STRUCT
{
    /*
       X509 Distinguished Names consist of attributes.
       Must contain the following items:

     * country/region,
     * organization,
     * organizational unit,
     * distinguished name qualifier,
     * state or province name,
     * common name (e.g., "Susan Housley"), and
     * serial number.

       Optional attributes:
     * locality,
     * title,
     * surname,
     * given name,
     * initials,
     * pseudonym, and
     * generation qualifier (e.g., "Jr.", "3rd", or "IV").
     */


    /* The following fields are X509 distinguished name attributes that MUST
       be supported as per the X509 RFC. */
    const UCHAR *nx_secure_x509_country;
    USHORT       nx_secure_x509_country_length;

    const UCHAR *nx_secure_x509_organization;
    USHORT       nx_secure_x509_organization_length;

    const UCHAR *nx_secure_x509_org_unit;
    USHORT       nx_secure_x509_org_unit_length;

    const UCHAR *nx_secure_x509_distinguished_name_qualifier;
    USHORT       nx_secure_x509_distinguished_name_qualifier_length;

    const UCHAR *nx_secure_x509_state;
    USHORT       nx_secure_x509_state_length;

    const UCHAR *nx_secure_x509_common_name;
    USHORT       nx_secure_x509_common_name_length;

    const UCHAR *nx_secure_x509_serial_number;
    USHORT       nx_secure_x509_serial_number_length;

#ifdef NX_SECURE_X509_USE_EXTENDED_DISTINGUISHED_NAMES
    /* The following fields are OPTIONAL per the X509 RFC, disable
       them to save on memory usage. */
    const UCHAR *nx_secure_x509_locality;
    USHORT       nx_secure_x509_locality_length;

    const UCHAR *nx_secure_x509_title;
    USHORT       nx_secure_x509_title_length;

    const UCHAR *nx_secure_x509_surname;
    USHORT       nx_secure_x509_surname_length;

    const UCHAR *nx_secure_x509_given_name;
    USHORT       nx_secure_x509_given_name_length;

    const UCHAR *nx_secure_x509_initials;
    USHORT       nx_secure_x509_initials_length;

    const UCHAR *nx_secure_x509_pseudonym;
    USHORT       nx_secure_x509_pseudonym_length;

    const UCHAR *nx_secure_x509_generation_qualifier;
    USHORT       nx_secure_x509_generation_qualifier_length;
#endif
} NX_SECURE_X509_DISTINGUISHED_NAME;


/* RSA public key information. */
typedef struct NX_SECURE_RSA_PUBLIC_KEY_STRUCT
{
    /* Public Modulus for RSA or DH. */
    const UCHAR *nx_secure_rsa_public_modulus;

    /* Size of the key used by the algorithm. */
    USHORT nx_secure_rsa_public_modulus_length;

    /* Public Exponent for RSA. */
    const UCHAR *nx_secure_rsa_public_exponent;

    /* Size of the key used by the algorithm. */
    USHORT nx_secure_rsa_public_exponent_length;
} NX_SECURE_RSA_PUBLIC_KEY;

/* RSA private key information. */
typedef struct NX_SECURE_RSA_PRIVATE_KEY_STRUCT
{
    /* The public modulus is used in all RSA operations. */
    const UCHAR *nx_secure_rsa_public_modulus;
    USHORT       nx_secure_rsa_public_modulus_length;

    /* The public exponent is used for encrypting messages intended for the private key holder. */
    const UCHAR *nx_secure_rsa_public_exponent;
    USHORT       nx_secure_rsa_public_exponent_length;

    /* The private exponent is the true "private" part of an RSA key. */
    const UCHAR *nx_secure_rsa_private_exponent;
    USHORT       nx_secure_rsa_private_exponent_length;

    /* P and Q are the primes used to calculate the RSA key. Using these, we
       can utilize the Chinese Remainder Theorem and speed up RSA encryption. */
    const UCHAR *nx_secure_rsa_private_prime_q;
    USHORT       nx_secure_rsa_private_prime_q_length;
    const UCHAR *nx_secure_rsa_private_prime_p;
    USHORT       nx_secure_rsa_private_prime_p_length;
} NX_SECURE_RSA_PRIVATE_KEY;

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
/* EC public key information. */
typedef struct NX_SECURE_EC_PUBLIC_KEY_STRUCT
{
    /* Public key for EC. */
    const UCHAR *nx_secure_ec_public_key;

    /* Size of the key used by the algorithm. */
    USHORT nx_secure_ec_public_key_length;

    /* Named curve used. */
    UINT nx_secure_ec_named_curve;

} NX_SECURE_EC_PUBLIC_KEY;

/* EC private key information. */
typedef struct NX_SECURE_EC_PRIVATE_KEY_STRUCT
{
    /* Private key for EC. */
    const UCHAR *nx_secure_ec_private_key;

    /* Size of the EC private key. */
    USHORT nx_secure_ec_private_key_length;

    /* Named curve used. */
    UINT nx_secure_ec_named_curve;

} NX_SECURE_EC_PRIVATE_KEY;
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

/* Structure to hold X.509 cryptographic routine information. */
typedef struct NX_SECURE_X509_CRYPTO_STRUCT
{
    /* Internal NetX Secure identifier for certificate "ciphersuite" which consists of
       a hash and a public key operation. These can be mapped to OIDs in X.509. */
    USHORT nx_secure_x509_crypto_identifier;

    /* Public-Key Cryptographic method used by certificates. */
    const NX_CRYPTO_METHOD *nx_secure_x509_public_cipher_method;

    /* Hash method used by certificates. */
    const NX_CRYPTO_METHOD *nx_secure_x509_hash_method;
} NX_SECURE_X509_CRYPTO;

/* Structure to hold policy qualifiers for the certificatePolicies extension. */
typedef struct NX_SECURE_X509_CERTIFICATE_POLICY_STRUCT
{
    /* Pointer to OID for this specific policy, if needed. */
    const UCHAR *nx_secure_x509_policy_oid;

    /* Qualifier OID type - defines what the policy qualifier is (if present). */
    UINT nx_secure_x509_policy_qualifier_type;

    /* Qualifier data. */
    union
    {
        /* CPS URI policy qualifier. */
        UCHAR nx_secure_x509_policy_qualifier_cps[200];

        /* UserNotice qualifier. */
        struct
        {
            /* NoticeRef structure. */
            struct
            {
                UCHAR nx_secure_x509_policy_organization[200];
                UINT  nx_secure_x509_policy_notice_numbers[20];
            } nx_secure_x509_policy_notice_ref;

            /* Explicit text data. */
            UCHAR nx_secure_x509_policy_explicit_text[200];
        } nx_secure_x509_policy_unotice;
    } nx_secure_x509_policy_qualifier;

    /* Pointer for linked list. */
    struct NX_SECURE_X509_CERTIFICATE_POLICY_STRUCT *nx_secure_x509_policy_next;
} NX_SECURE_X509_CERTIFICATE_POLICY;

/* Structure to hold extension information for an X.509 certificate.
   Also used to contain information about an X.509 policy, either
   when parsing an X.509 certificate or when defining policies for
   X.509 path validation. */
typedef struct NX_SECURE_X509_EXTENSION_STRUCT
{
    /* Identifier (maps to OID) for this extension. */
    USHORT nx_secure_x509_extension_id;

    /* Critical flag - boolean value. */
    USHORT nx_secure_x509_extension_critical;

    /* Pointer to DER-encoded extension data. */
    const UCHAR *nx_secure_x509_extension_data;
    ULONG        nx_secure_x509_extension_data_length;
} NX_SECURE_X509_EXTENSION;


/* Domain name entries. */

/* Maximum size of a DNS name entry. */
#define NX_SECURE_X509_DNS_NAME_MAX (100)
typedef struct NX_SECURE_X509_DNS_NAME_STRUCT
{
    /* Pointer to string containing the DNS name. */
    UCHAR nx_secure_x509_dns_name[NX_SECURE_X509_DNS_NAME_MAX];

    /* Length of name. */
    USHORT nx_secure_x509_dns_name_length;

    /* Pointer for linked list. */
    struct NX_SECURE_X509_DNS_NAME_STRUCT *nx_secure_x509_dns_name_next;
} NX_SECURE_X509_DNS_NAME;

/* Structure to hold policy information for certificate chain verification. */
typedef struct NX_SECURE_X509_POLICIES_STRUCT
{
    /* X.509 policies and path validation (RFC 5280):
     * "Conforming implementations are not required to support the setting of
        all of these inputs.  For example, a conforming implementation may be
        designed to validate all certification paths using a value of FALSE
        for initial-any-policy-inhibit."
     */

    /* Linked list of policies, keyed on policy OID.*/
    NX_SECURE_X509_EXTENSION *nx_secure_x509_valid_policy_tree;

    /* Indicates if policy mapping is allowed for the chain. */
    USHORT nx_secure_x509_policy_mapping_inhibit;

    /* Indicates if the chain must be valid for at least one policy.
       If false, the chain does not need to be valid for any policies
       (it still must be trusted though).*/
    USHORT nx_secure_x509_explicit_policy;

    /* Indicates if the anyPolicy OID is processed or ignored. */
    USHORT nx_secure_x509_any_policy_inhibit;

    /* For name constraint checking, a collection of name subtrees that
       are permitted when checking DNS names (or other name types). */
    NX_SECURE_X509_DNS_NAME *nx_secure_x509_permitted_subtrees;

    /* For name constraint checking, a collection of name subtrees that
           are NOT permitted when checking DNS names (or other name types). */
    NX_SECURE_X509_DNS_NAME *nx_secure_x509_excluded_subtrees;

    /* Maximum path length for validation, set to n and decremented for each
     * non-self-signed certificate in the path. */
    USHORT nx_secure_x509_max_path_length;

    /* Other X.509 variables are contained within nx_secure_x509_certificate chain verify:
    (g)  working_public_key_algorithm
    (h)  working_public_key
    (i)  working_public_key_parameters
    (j)  working_issuer_name
    */
} NX_SECURE_X509_POLICIES;

/* This structure is used in parsing the X509 certificate. The ASN.1-encoded certificate
 * contains embedded byte strings which contain ASN.1 data within. The initial parsing
 * of the ASN.1 buffer will populate this structure, which will then be used to
 * extract further information. This is specific to X509 certificates!
 * NOTE: All specific data are returned as pointers into the original buffer to avoid copying! */
typedef struct NX_SECURE_X509_CERT_STRUCT
{
    /* Is this certificate used to identify the local device? */
    UINT nx_secure_x509_certificate_is_identity_cert;

    /* What version of certificate is this? */
    USHORT nx_secure_x509_version;

    /* Identifier used for TLS to distinguish between certificates outside of information
       contained within the certificate. */
    UINT nx_secure_x509_cert_identifier;

    /* Serial number. */
    const UCHAR *nx_secure_x509_serial_number;
    USHORT       nx_secure_x509_serial_number_length;

    /* Validity time format - either ASN.1 generalized time or ASN.1 UTC time.
       Uses the ASN.1 tag value. */
    USHORT nx_secure_x509_validity_format;

    /* Validity period. Stored as ASN.1 generalized time or UTC time. */
    const UCHAR *nx_secure_x509_not_before;
    USHORT       nx_secure_x509_not_before_length;
    const UCHAR *nx_secure_x509_not_after;
    USHORT       nx_secure_x509_not_after_length;

    /* Pointer to certificate data. */
    UCHAR       *nx_secure_x509_certificate_raw_data;
    UINT         nx_secure_x509_certificate_raw_buffer_size;
    UINT         nx_secure_x509_certificate_raw_data_length;

    const UCHAR *nx_secure_x509_certificate_data;
    UINT         nx_secure_x509_certificate_data_length;

    /* Signature Algorithm (For encrypting signature hash - RSA, DSS, etc). */
    UINT nx_secure_x509_signature_algorithm;

    /* Pointer to the signature data in the certificate used for verification. */
    const UCHAR *nx_secure_x509_signature_data;

    /* Length of the certificate digital signature data. */
    UINT nx_secure_x509_signature_data_length;

    /* Issuer distinguished name. Used to authenticate this certificate against the trusted store.
     * If this pointer is NULL, it is a root CA. For self-signed certs this will point to a
     * structure with the same information. */
    NX_SECURE_X509_DISTINGUISHED_NAME nx_secure_x509_issuer;

    /* This pointer points to the issuer chain for this certificate (follow the chain for verification). */
    struct NX_SECURE_X509_CERT_STRUCT *nx_secure_x509_issuer_chain;

    /* X509 subject - Distinguished Name of the certificate. */
    NX_SECURE_X509_DISTINGUISHED_NAME nx_secure_x509_distinguished_name;

    /* Pointer to lookup table for X.509 cryptographic routines. */
    NX_SECURE_X509_CRYPTO *nx_secure_x509_cipher_table;

    /* Number of entried in the cipher lookup table. */
    USHORT nx_secure_x509_cipher_table_size;

    /* Define the public cipher metadata area. */
    VOID *nx_secure_x509_public_cipher_metadata_area;

    /* Define the public cipher metadata size. */
    ULONG nx_secure_x509_public_cipher_metadata_size;

    /* Define the hash metadata area. */
    VOID *nx_secure_x509_hash_metadata_area;

    /* Define the hash metadata size. */
    ULONG nx_secure_x509_hash_metadata_size;

    /* This pointer points to a singly-linked list of certificates - used for the certificate stores. */
    struct NX_SECURE_X509_CERT_STRUCT *nx_secure_x509_next_certificate;

    /* Public Key Algorithm (RSA, DH, etc). Tagged union gives us the actual key data.*/
    UINT nx_secure_x509_public_algorithm;

    union
    {
        NX_SECURE_RSA_PUBLIC_KEY rsa_public_key;

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
        NX_SECURE_EC_PUBLIC_KEY ec_public_key;
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
    } nx_secure_x509_public_key;

    /* Private key associated with this certificate. A tagged union represents the key data in parsed form
       (if the type is known) or a pointer to a buffer of user-defined key data. Note that the private
       key is typically loaded separately from the certificate so even if the private key data contains
       the public key, we keep the private keys separate from the public keys (above).*/
    UINT nx_secure_x509_private_key_type;

    union
    {
        /* RSA key type. */
        NX_SECURE_RSA_PRIVATE_KEY rsa_private_key;

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
        /* EC key type. */
        NX_SECURE_EC_PRIVATE_KEY ec_private_key;
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

        /* User-defined key type. */
        struct
        {
            const UCHAR *key_data;
            ULONG        key_length;
        } user_key;
    } nx_secure_x509_private_key;

    /* Optional issuer identifier field. X509v2 or X509v3. */
    const UCHAR *nx_secure_x509_issuer_identifier;
    USHORT       nx_secure_x509_issuer_identifier_length;

    /* Optional subject identifier field. X509v2 or X509v3. */
    const UCHAR *nx_secure_x509_subject_identifier;
    USHORT       nx_secure_x509_subject_identifier_length;

    /* Pointer to start of extensions so we can optionally parse them later. */
    const UCHAR *nx_secure_x509_extensions_data;
    ULONG        nx_secure_x509_extensions_data_length;

    /* Indicates whether a certificate was allocated by the application or
       automatically by TLS. */
    UINT nx_secure_x509_user_allocated_cert;
} NX_SECURE_X509_CERT;

UINT _nx_secure_x509_certificate_parse(const UCHAR *buffer, UINT length, UINT *bytes_processed, NX_SECURE_X509_CERT *cert);
UINT _nx_secure_x509_asn1_tlv_block_parse(const UCHAR *buffer, ULONG *buffer_length, USHORT *tlv_type, USHORT *tlv_tag_class, ULONG *tlv_length, const UCHAR **tlv_data, ULONG *header_length);

UINT _nx_secure_x509_pkcs1_rsa_private_key_parse(const UCHAR *buffer, UINT length, UINT *bytes_processed, NX_SECURE_RSA_PRIVATE_KEY *rsa_key);
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
UINT _nx_secure_x509_ec_private_key_parse(const UCHAR *buffer, UINT length,
                                          UINT *bytes_processed,
                                          NX_SECURE_EC_PRIVATE_KEY *ec_key);
UINT _nx_secure_x509_find_curve_method(USHORT named_curve, const NX_CRYPTO_METHOD **curve_method);

#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

/* CRL parsing. */
#ifndef NX_SECURE_X509_DISABLE_CRL

typedef struct NX_SECURE_X509_CRL_STRUCT
{
    /* Pointer to raw CRL data, used for signature verification. */
    const UCHAR *nx_secure_x509_crl_verify_data;
    USHORT       nx_secure_x509_crl_verify_data_length;

    /* Issuer distinguished name. Used to authenticate this CRL against the trusted store. */
    NX_SECURE_X509_DISTINGUISHED_NAME nx_secure_x509_crl_issuer;

    /* What version of CRL is this? */
    USHORT nx_secure_x509_crl_version;

    /* Signature Algorithm (For encrypting signature hash - RSA, DSS, etc). */
    USHORT nx_secure_x509_crl_signature_algorithm;

    /* Pointer to the signature data in the CRL used for verification of the list. */
    const UCHAR *nx_secure_x509_crl_signature_data;

    /* Length of the CRL digital signature data. */
    UINT nx_secure_x509_crl_signature_data_length;

    /* Validity period. Stored as ASN.1 generalized time or UTC time. */
    USHORT       nx_secure_x509_crl_time_format;
    const UCHAR *nx_secure_x509_crl_this_update;
    USHORT       nx_secure_x509_crl_this_update_length;
    const UCHAR *nx_secure_x509_crl_next_update;
    USHORT       nx_secure_x509_crl_next_update_length;

    /* Pointer to start of revoked certificate list. This will be parsed whenever a certificate
       is being validated. */
    const UCHAR *nx_secure_x509_crl_revoked_certs;
    ULONG        nx_secure_x509_crl_revoked_certs_length;
} NX_SECURE_X509_CRL;

#endif

/* X509 API. */

#define NX_SECURE_X509_CERT_LOCATION_NONE       0 /* Certificate location is uninitialized. */
#define NX_SECURE_X509_CERT_LOCATION_LOCAL      1 /* Certificate is in the Local device certificate store. */
#define NX_SECURE_X509_CERT_LOCATION_REMOTE     2 /* Certificate is in the remote certificate store. */
#define NX_SECURE_X509_CERT_LOCATION_TRUSTED    3 /* Certificate is in the trusted store. */
#define NX_SECURE_X509_CERT_LOCATION_EXCEPTIONS 4 /* Certificate is added as an exception (trusted temporarily). */
#define NX_SECURE_X509_CERT_LOCATION_FREE       5 /* Certificate is uninitialized (except for next pointer) and usable by X509, TLS, etc. */

/* Certificate store structure - contains linked lists of all certificates for this device. */
typedef struct NX_SECURE_X509_CERTIFICATE_STORE_STRUCT
{
    /* Pointer to certificates identifying this host. */
    NX_SECURE_X509_CERT *nx_secure_x509_local_certificates;

    /* Pointer to certificates identifying the remote host. */
    NX_SECURE_X509_CERT *nx_secure_x509_remote_certificates;

    /* Pointer to certificates that are free to use for parsing (will be placed in another list
       after being initialized). */
    NX_SECURE_X509_CERT *nx_secure_x509_free_certificates;

    /* Pointer to local store of trusted certificates. */
    NX_SECURE_X509_CERT *nx_secure_x509_trusted_certificates;

    /* Pointer to store of exceptions - certificates that we want to trust but possibly only
       temporarily and not something we want in a the trusted store. Keep this store separate
       so we can clear it out more easily. */
    NX_SECURE_X509_CERT *nx_secure_x509_certificate_exceptions;
} NX_SECURE_X509_CERTIFICATE_STORE;

/* Get certificate for local device. */
UINT _nx_secure_x509_local_device_certificate_get(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                                  NX_SECURE_X509_DISTINGUISHED_NAME *name,
                                                  NX_SECURE_X509_CERT **certificate);

UINT _nx_secure_x509_local_certificate_find(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                            NX_SECURE_X509_CERT **certificate, UINT cert_id);

/* Get certificate sent by remote host. */
UINT _nx_secure_x509_remote_endpoint_certificate_get(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                                     NX_SECURE_X509_CERT **certificate);

/* Add a certificates to an X509 store. "location" indicates where the certificate will go: [ TRUSTED, LOCAL, REMOTE, EXCEPTION ] */
UINT _nx_secure_x509_store_certificate_add(NX_SECURE_X509_CERT *certificate,
                                           NX_SECURE_X509_CERTIFICATE_STORE *store, UINT location);

/* Remove a certificate from the supplied store given a distinguished name and optional location. */
UINT _nx_secure_x509_store_certificate_remove(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                              NX_SECURE_X509_DISTINGUISHED_NAME *name,
                                              UINT location, UINT cert_id);

/* Compares two distinguished names to see if they are equal. */
INT _nx_secure_x509_distinguished_name_compare(NX_SECURE_X509_DISTINGUISHED_NAME *name,
                                               NX_SECURE_X509_DISTINGUISHED_NAME *compare_name, ULONG compare_fields);

/* Parse an X.509 DER-encoded distinguished name. */
UINT _nx_secure_x509_distinguished_name_parse(const UCHAR *buffer, UINT length,
                                              UINT *bytes_processed,
                                              NX_SECURE_X509_DISTINGUISHED_NAME *name);

UINT _nx_secure_x509_store_certificate_find(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                            NX_SECURE_X509_DISTINGUISHED_NAME *name,
                                            UINT cert_id,
                                            NX_SECURE_X509_CERT **certificate, UINT *location);

/* Build an X509 certificate chain using the supplied certificate store and the given distinguished name. */
UINT _nx_secure_x509_certificate_chain_build(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                             NX_SECURE_X509_CERT *certificate);

/* Verify a certificate against its issuer. */
UINT _nx_secure_x509_certificate_verify(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                        NX_SECURE_X509_CERT *certificate,
                                        NX_SECURE_X509_CERT *issuer_certificate);

/* Verify a given certificate chain to see if the end-entity certificate can be traced through the chain to a trust anchor. */
UINT _nx_secure_x509_certificate_chain_verify(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                              NX_SECURE_X509_CERT *certificate, ULONG current_time);

/* Parse an OID string, returning an internally-used constant (defined above) for use in other parsing. */
VOID _nx_secure_x509_oid_parse(const UCHAR *oid, ULONG length, UINT *oid_value);

UINT _nx_secure_x509_pkcs7_decode(const UCHAR *signature_pointer, UINT signature_length,
                                  const UCHAR **signature_oid, UINT *signature_oid_length,
                                  const UCHAR **hash_data, UINT *hash_length);

/* Initialize an X509 certificate structure with a DER-encoded certificate blob. */
UINT _nx_secure_x509_certificate_initialize(NX_SECURE_X509_CERT *certificate,
                                            UCHAR *certificate_data, USHORT length,
                                            UCHAR *raw_data_buffer, USHORT buffer_size,
                                            const UCHAR *private_key,
                                            USHORT priv_len, UINT private_key_type);

UINT _nx_secure_x509_dns_name_initialize(NX_SECURE_X509_DNS_NAME *dns_name,
                                         const UCHAR *name_string, USHORT length);

/* Return a structure containing cryptographic methods for a particular certificate. */
UINT _nx_secure_x509_find_certificate_methods(NX_SECURE_X509_CERT *cert, USHORT signature_algorithm,
                                              NX_SECURE_X509_CRYPTO **crypto_methods);

/* Get a "free" certificate allocated earlier by the application. */
UINT _nx_secure_x509_free_certificate_get(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                          NX_SECURE_X509_CERT **certificate);

/* Utility methods for certificate linked-lists. */
UINT _nx_secure_x509_certificate_list_add(NX_SECURE_X509_CERT **list_head,
                                          NX_SECURE_X509_CERT *certificate, UINT duplicates_ok);
UINT _nx_secure_x509_certificate_list_find(NX_SECURE_X509_CERT **list_head,
                                           NX_SECURE_X509_DISTINGUISHED_NAME *name,
                                           UINT cert_id,
                                           NX_SECURE_X509_CERT **certificate);
UINT _nx_secure_x509_certificate_list_remove(NX_SECURE_X509_CERT **list_head,
                                             NX_SECURE_X509_DISTINGUISHED_NAME *name, UINT cert_id);

#ifndef NX_SECURE_X509_DISABLE_CRL
UINT _nx_secure_x509_certificate_revocation_list_parse(const UCHAR *buffer, UINT length,
                                                       UINT *bytes_processed, NX_SECURE_X509_CRL *crl);
#endif /* NX_SECURE_X509_DISABLE_CRL */
UINT _nx_secure_x509_common_name_dns_check(NX_SECURE_X509_CERT *certificate,
                                           const UCHAR *dns_tld, UINT dns_tld_length);
INT  _nx_secure_x509_wildcard_compare(const UCHAR *dns_name, UINT dns_name_len,
                                      const UCHAR *wildcard_name, UINT wildcard_len);
UINT _nx_secure_x509_crl_revocation_check(const UCHAR *crl_data, UINT length,
                                          NX_SECURE_X509_CERTIFICATE_STORE *store,
                                          NX_SECURE_X509_CERT *certificate);
#ifndef NX_SECURE_X509_DISABLE_CRL
UINT _nx_secure_x509_crl_verify(NX_SECURE_X509_CERT *certificate, NX_SECURE_X509_CRL *crl,
                                NX_SECURE_X509_CERTIFICATE_STORE *store,
                                NX_SECURE_X509_CERT *issuer_certificate);
#endif /* NX_SECURE_X509_DISABLE_CRL */
UINT _nx_secure_x509_expiration_check(NX_SECURE_X509_CERT *certificate, ULONG current_time);

UINT _nx_secure_x509_extended_key_usage_extension_parse(NX_SECURE_X509_CERT *certificate,
                                                        UINT key_usage);
UINT _nx_secure_x509_extension_find(NX_SECURE_X509_CERT *certificate,
                                    NX_SECURE_X509_EXTENSION *extension, USHORT extension_id);
UINT _nx_secure_x509_key_usage_extension_parse(NX_SECURE_X509_CERT *certificate, USHORT *bitfield);
UINT _nx_secure_x509_subject_alt_names_find(NX_SECURE_X509_EXTENSION *extension, const UCHAR *name,
                                            UINT name_length, USHORT name_type);

/* Error-checking APIs. */
UINT _nxe_secure_x509_certificate_initialize(NX_SECURE_X509_CERT *certificate,
                                             UCHAR *certificate_data, USHORT length,
                                             UCHAR *raw_data_buffer, USHORT buffer_size,
                                             const UCHAR *private_key, USHORT priv_len,
                                             UINT private_key_type);
UINT _nxe_secure_x509_common_name_dns_check(NX_SECURE_X509_CERT *certificate, const UCHAR *dns_tld,
                                            UINT dns_tld_length);
UINT _nxe_secure_x509_dns_name_initialize(NX_SECURE_X509_DNS_NAME *dns_name,
                                          const UCHAR *name_string, USHORT length);
UINT _nxe_secure_x509_crl_revocation_check(const UCHAR *crl_data, UINT crl_length,
                                           NX_SECURE_X509_CERTIFICATE_STORE *store,
                                           NX_SECURE_X509_CERT *certificate);
UINT _nxe_secure_x509_extended_key_usage_extension_parse(NX_SECURE_X509_CERT *certificate,
                                                         UINT key_usage);
UINT _nxe_secure_x509_extension_find(NX_SECURE_X509_CERT *certificate,
                                     NX_SECURE_X509_EXTENSION *extension, USHORT extension_id);
UINT _nxe_secure_x509_key_usage_extension_parse(NX_SECURE_X509_CERT *certificate, USHORT *bitfield);


/* MAP APIs. */
#ifdef NX_SECURE_DISABLE_ERROR_CHECKING
#define nx_secure_x509_certificate_initialize             _nx_secure_x509_certificate_initialize
#define nx_secure_x509_common_name_dns_check              _nx_secure_x509_common_name_dns_check
#define nx_secure_x509_dns_name_initialize                _nx_secure_x509_dns_name_initialize
#define nx_secure_x509_crl_revocation_check               _nx_secure_x509_crl_revocation_check
#define nx_secure_x509_extended_key_usage_extension_parse _nx_secure_x509_extended_key_usage_extension_parse
#define nx_secure_x509_extension_find                     _nx_secure_x509_extension_find
#define nx_secure_x509_key_usage_extension_parse          _nx_secure_x509_key_usage_extension_parse
#else
#define nx_secure_x509_certificate_initialize             _nxe_secure_x509_certificate_initialize
#define nx_secure_x509_common_name_dns_check              _nxe_secure_x509_common_name_dns_check
#define nx_secure_x509_dns_name_initialize                _nxe_secure_x509_dns_name_initialize
#define nx_secure_x509_crl_revocation_check               _nxe_secure_x509_crl_revocation_check
#define nx_secure_x509_extended_key_usage_extension_parse _nxe_secure_x509_extended_key_usage_extension_parse
#define nx_secure_x509_extension_find                     _nxe_secure_x509_extension_find
#define nx_secure_x509_key_usage_extension_parse          _nxe_secure_x509_key_usage_extension_parse
#endif

UINT nx_secure_x509_certificate_initialize(NX_SECURE_X509_CERT *certificate, UCHAR *certificate_data,
                                           USHORT length, UCHAR *raw_data_buffer, USHORT buffer_size,
                                           const UCHAR *private_key, USHORT priv_len,
                                           UINT private_key_type);
UINT nx_secure_x509_common_name_dns_check(NX_SECURE_X509_CERT *certificate, const UCHAR *dns_tld,
                                          UINT dns_tld_length);
UINT nx_secure_x509_dns_name_initialize(NX_SECURE_X509_DNS_NAME *dns_name,
                                        const UCHAR *name_string, USHORT length);
UINT nx_secure_x509_crl_revocation_check(const UCHAR *crl_data, UINT crl_length,
                                         NX_SECURE_X509_CERTIFICATE_STORE *store,
                                         NX_SECURE_X509_CERT *certificate);
UINT nx_secure_x509_extended_key_usage_extension_parse(NX_SECURE_X509_CERT *certificate,
                                                       UINT key_usage);
UINT nx_secure_x509_extension_find(NX_SECURE_X509_CERT *certificate,
                                   NX_SECURE_X509_EXTENSION *extension, USHORT extension_id);
UINT nx_secure_x509_key_usage_extension_parse(NX_SECURE_X509_CERT *certificate, USHORT *bitfield);

#ifdef __cplusplus
}
#endif

#endif /* SRC_NX_SECURE_X509_H_ */

