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


/**************************************************************************/
/*                                                                        */
/*  PORT SPECIFIC C INFORMATION                            RELEASE        */
/*                                                                        */
/*    nx_secure_user.h                                    PORTABLE C      */
/*                                                           6.2.0        */
/*                                                                        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This file contains user defines for configuring NetX Secure in      */
/*    specific ways. This file will have an effect only if the            */
/*    application and NetX Secure library are built with                  */
/*    NX_SECURE_INCLUDE_USER_DEFINE_FILE defined.                         */
/*    Note that all the defines in this file may also be made on the      */
/*    command line when building NetX library and application objects.    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1.8  */
/*  10-15-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            macro to disable client     */
/*                                            initiated renegotiation for */
/*                                            TLS server instances,       */
/*                                            resulting in version 6.1.9  */
/*  10-31-2022     Yanwu Cai                Modified comment(s), added    */
/*                                            macro to custom secret size,*/
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/

#ifndef SRC_NX_SECURE_USER_H
#define SRC_NX_SECURE_USER_H


/* Define various build options for the NetX Secure port.  The application should either make changes
   here by commenting or un-commenting the conditional compilation defined OR supply the defines
   though the compiler's equivalent of the -D option.  */


/* Override various options with default values already assigned in nx_secure_tls.h */


/* NX_SECURE_TLS_ENABLE_TLS_1_0 defines whether or not to enable TLS 1.0 protocol support.
   BY default TLS 1.0 is not supported. */
/*
   #define NX_SECURE_TLS_ENABLE_TLS_1_0
 */

/* NX_SECURE_TLS_ENABLE_TLS_1_1 defines whether or not to enable TLS 1.1 protocol support.
   BY default TLS 1.1 is not supported. */
/*
   #define NX_SECURE_TLS_ENABLE_TLS_1_1
*/

/* NX_SECURE_TLS_ENABLE_TLS_1_3 defines whether or not to disable TLS 1.3 protocol support.
   BY default TLS 1.3 is not enabled. */
/*
   #define NX_SECURE_TLS_ENABLE_TLS_1_3
*/

/* NX_SECURE_TLS_DISABLE_PROTOCOL_VERSION_DOWNGRADE defines whether or not to disables
   protocol version downgrade for TLS client. BY default protocol version downgrade is supported. */
/*
   #define NX_SECURE_TLS_DISABLE_PROTOCOL_VERSION_DOWNGRADE
 */

/* NX_SECURE_ENABLE_PSK_CIPHERSUITES enables Pre-Shared Key.  By default
   this feature is not enabled. */
/*
   #define NX_SECURE_ENABLE_PSK_CIPHERSUITES
 */

/* NX_SECURE_AEAD_CIPHER_CHECK allows to detect user-implemented AEAD algorithms other than AES-CCM or
   AES-GCM. It can be defined like #define NX_SECURE_AEAD_CIPHER_CHECK(a) ((a) == NEW_ALGORITHM_ID).
   It works only when NX_SECURE_ENABLE_AEAD_CIPHER is defined.
   By default this feature is not enabled. */
/*
   #define NX_SECURE_AEAD_CIPHER_CHECK(a) NX_FALSE
*/

/* NX_SECURE_ALLOW_SELF_SIGNED_CERTIFICATES enables self signed certificates. By default
   this feature is not enabled. */
/*
   #define NX_SECURE_ALLOW_SELF_SIGNED_CERTIFICATES
*/

/* NX_SECURE_DISABLE_X509 disables X509 feature. By default this feature is enabled. */
/*
   #define NX_SECURE_DISABLE_X509
*/

/* NX_SECURE_DTLS_COOKIE_LENGTH defines the length of DTLS cookie.
   The default value is 32. */
/*
   #define NX_SECURE_DTLS_COOKIE_LENGTH 32
*/

/* NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_RETRIES defineds the maximum retransmit retries
   for DTLS handshake packet. The default value is 10. */
/*
   #define NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_RETRIES 10
*/

/* NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_TIMEOUT defines the maximum DTLS retransmit rate.
   The default value is 60 seconds. */
/*
   #define NX_SECURE_DTLS_MAXIMUM_RETRANSMIT_TIMEOUT (60 * NX_IP_PERIODIC_RATE)
*/

/* NX_SECURE_DTLS_RETRANSMIT_RETRY_SHIFT defins how the retransmit timeout period changes between successive retries.
   If this value is 0, the initial retransmit timeout is the same as subsequent retransmit timeouts. If this
   value is 1, each successive retransmit is twice as long. The default value is 1.  */
/*
   #define NX_SECURE_DTLS_RETRANSMIT_RETRY_SHIFT 1
*/

/* NX_SECURE_DTLS_RETRANSMIT_TIMEOUT defines the initial DTLS retransmit rate.
   The default value is 1 second. */
/*
   #define NX_SECURE_DTLS_RETRANSMIT_TIMEOUT NX_IP_PERIODIC_RATE
*/

/* NX_SECURE_ENABLE_AEAD_CIPHER enables AEAD ciphersuites.
   For AEAD ciphersuites other than AES-CCM or AES-GCM, additional defination of
   NX_SECURE_AEAD_CIPHER_CHECK must be defined. By default this feature is not enabled. */
/*
   #define NX_SECURE_ENABLE_AEAD_CIPHER
*/

/* NX_SECURE_ENABLE_CLIENT_CERTIFICATE_VERIFY enables client certificate verification.
   By default this feature is not enabled. */
/*
   #define NX_SECURE_ENABLE_CLIENT_CERTIFICATE_VERIFY
*/

/* NX_SECURE_ENABLE_DTLS enables DTLS feature. By default this feature is not enabled. */
/*
   #define NX_SECURE_ENABLE_DTLS
*/

/* NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE enables ECJPAKE ciphersuites for DTLS.
   By default this feature is not enabled. */
/*
   #define NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
*/

/* NX_SECURE_KEY_CLEAR enables key related materials cleanup when they are not used anymore.
   By default this feature is not enabled. */
/*
   #define NX_SECURE_KEY_CLEAR
*/

/* NX_SECURE_MEMCMP defines the memory compare function.
   By default it is mapped to C library function. */
/*
   #define NX_SECURE_MEMCMP memcmp
*/

/* NX_SECURE_MEMCPY defines the memory copy function.
   By default it is mapped to C library function. */
/*
   #define NX_SECURE_MEMCPY memcpy
*/

/* NX_SECURE_MEMMOVE defines the memory move function.
   By default it is mapped to C library function. */
/*
   #define NX_SECURE_MEMMOVE memmove
*/

/* NX_SECURE_MEMSET defines the memory set function.
   By default it is mapped to C library function. */
/*
   #define NX_SECURE_MEMSET memset
*/

/* NX_SECURE_POWER_ON_SELF_TEST_MODULE_INTEGRITY_CHECK enables module integrity
   self test. By default it is not enabled. */
/*
   #define NX_SECURE_POWER_ON_SELF_TEST_MODULE_INTEGRITY_CHECK
*/

/* NX_SECURE_RNG_CHECK_COUNT defines the random number check for duplication.
   By default it is 3. */
/*
   #define NX_SECURE_RNG_CHECK_COUNT 3
*/

/* NX_SECURE_TLS_CLIENT_DISABLED disables TLS client. By default TLS client is enabled. */
/*
   #define NX_SECURE_TLS_CLIENT_DISABLED
*/

/* NX_SECURE_TLS_MAX_PSK_ID_SIZE defines the maximum size of PSK ID.
   By default it is 20. */
/*
   #define NX_SECURE_TLS_MAX_PSK_ID_SIZE 20
*/

/* NX_SECURE_TLS_MAX_PSK_KEYS defines the maximum PSK keys.
   By default it is 5. */
/*
   #define NX_SECURE_TLS_MAX_PSK_KEYS 5
*/

/* NX_SECURE_TLS_MAX_PSK_SIZE defines the maximum size of PSK.
   By default it is 20. */
/*
   #define NX_SECURE_TLS_MAX_PSK_SIZE 20
*/

/* NX_SECURE_TLS_MINIMUM_CERTIFICATE_SIZE defines a minimum reasonable size for a TLS
   X509 certificate. This is used in checking for * errors in allocating certificate space.
   The size is determined by assuming a 512-bit RSA key, MD5 hash, and a rough estimate of
   other data. It is theoretically possible for a real certificate to be smaller,
   but in that case, bypass the error checking by re-defining this macro.
      Approximately: 64(RSA) + 16(MD5) + 176(ASN.1 + text data, common name, etc)
   The default value is 256. */
/*
   #define NX_SECURE_TLS_MINIMUM_CERTIFICATE_SIZE 256
*/

/* NX_SECURE_TLS_MINIMUM_MESSAGE_BUFFER_SIZE defines the minimum size for the TLS message buffer.
   It is determined by a number of factors, but primarily the expected size of the TLS handshake
   Certificate message (sent by the TLS server) that may contain multiple certificates of 1-2KB each.
   The upper limit is determined by the length field in the TLS header (16 bit), and is 64KB.
   The default value is 4000. */
/*
   #define NX_SECURE_TLS_MINIMUM_MESSAGE_BUFFER_SIZE 4000
*/

/* NX_SECURE_TLS_PREMASTER_SIZE defines the size of pre-master secret.
   The default value is 48. */
/*
   #define NX_SECURE_TLS_PREMASTER_SIZE 48
*/

/* NX_SECURE_TLS_MASTER_SIZE defines the size of master secret.
   The default value is 48. */
/*
   #define NX_SECURE_TLS_MASTER_SIZE 48
*/

/* NX_SECURE_TLS_KEY_MATERIAL_SIZE defines the size of key material.
   The default value is (2 * (NX_SECURE_TLS_MAX_HASH_SIZE + NX_SECURE_TLS_MAX_KEY_SIZE + NX_SECURE_TLS_MAX_IV_SIZE)). */
/*
   #define NX_SECURE_TLS_KEY_MATERIAL_SIZE (2 * (NX_SECURE_TLS_MAX_HASH_SIZE + NX_SECURE_TLS_MAX_KEY_SIZE + NX_SECURE_TLS_MAX_IV_SIZE))
*/

/* NX_SECURE_TLS_CLIENT_DISABLED disables TLS server. By default TLS server is enabled. */
/*
   #define NX_SECURE_TLS_SERVER_DISABLED
*/

/* NX_SECURE_TLS_SNI_EXTENSION_DISABLED disables Server Name Indication (SNI) extension.
   By default this feature is enabled */
/*
   #define NX_SECURE_TLS_SNI_EXTENSION_DISABLED
*/

/* NX_SECURE_TLS_USE_SCSV_CIPHPERSUITE enables SCSV ciphersuite in ClientHello message.
   By default this feature is not enabled. */
/*
   #define NX_SECURE_TLS_USE_SCSV_CIPHPERSUITE
*/

/* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION disables secure session renegotiation extension (RFC 5746).
   By default this feature is enabled. */
/*
   #define NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
*/

/* NX_SECURE_TLS_REQUIRE_RENEGOTIATION_EXT defines whether or not the connection should be terminated immediately
   upon failure to receive the secure renegotiation extension during the initial handshake.
   By default the connection is not terminated. */
/*
   #define NX_SECURE_TLS_REQUIRE_RENEGOTIATION_EXT
*/

/* NX_SECURE_TLS_DISABLE_CLIENT_INITIATED_RENEGOTIATION disables client-initiated renegotiation for TLS
   servers. In some instances, client-initiated renegotiation can become a possible denial-of-service 
   vulnerability. */
/*
  #define NX_SECURE_TLS_DISABLE_CLIENT_INITIATED_RENEGOTIATION
*/

/* NX_SECURE_CUSTOM_SECRET_GENERATION enables the user to pass pointers of customized secret generation functions to
   TLS in the user defined nx_secure_custom_secret_generation_init function. This will allow TLS to use customized
   secret generation functions. */
/*
  #define NX_SECURE_CUSTOM_SECRET_GENERATION
*/

/* NX_SECURE_X509_DISABLE_CRL disables X509 Certificate Revocation List check.
   By default this feature is enabled. */
/*
   #define NX_SECURE_X509_DISABLE_CRL
*/

/* NX_SECURE_X509_STRICT_NAME_COMPARE enables strict X509 comparisons for all fields.
   By default this feature is not enabled. */
/*
   #define NX_SECURE_X509_STRICT_NAME_COMPARE
*/

/* NX_SECURE_X509_USE_EXTENDED_DISTINGUISHED_NAMES enables extended distinguished names
   for strict X509 comparisons. By default this feature is not enabled. */
/*
   #define NX_SECURE_X509_USE_EXTENDED_DISTINGUISHED_NAMES
*/

/* If the handshake hash state cannot be copied using memory copy on metadata,
   NX_SECURE_HASH_METADATA_CLONE should be defined to a function that clones the hash state.
   UINT nx_crypto_hash_clone(VOID *dest_metadata, VOID *source_metadata, ULONG length);
   #define NX_SECURE_HASH_METADATA_CLONE nx_crypto_hash_clone
*/

/* If cleaning up is required for the handshake hash crypto after being cloned,
   NX_SECURE_HASH_CLONE_CLEANUP macro should be defined to a clean up function:
   UINT nx_crypto_clone_cleanup(VOID *metadata, ULONG length);
   #define NX_SECURE_HASH_CLONE_CLEANUP nx_crypto_clone_cleanup
*/

#endif /* SRC_NX_SECURE_USER_H */
