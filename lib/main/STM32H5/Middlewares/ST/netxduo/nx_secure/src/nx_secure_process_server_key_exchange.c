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
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls.h"
#endif /* NX_SECURE_ENABLE_DTLS */

#ifndef NX_SECURE_TLS_CLIENT_DISABLED

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
static UCHAR hash[64]; /* We concatenate MD5 and SHA-1 hashes into this buffer, OR SHA-256, SHA-384, SHA512. */
static UCHAR decrypted_signature[512];
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_process_server_key_exchange               PORTABLE C     */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yanwu Cai, Microsoft Corporation                                    */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ServerKeyExchange message,      */
/*    which is sent by the remote TLS Server host when certain            */
/*    ciphersuites (e.g. those using Diffie-Hellman) are used.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ciphersuite                           Selected cipher suite         */
/*    tls_crypto_table                      TLS crypto methods            */
/*    protocol_version                      Selected TLS version          */
/*    packet_buffer                         Pointer to message data       */
/*    message_length                        Length of message data (bytes)*/
/*    tls_key_material                      TLS key material              */
/*    tls_credentials                       TLS credentials               */
/*    tls_handshake_hash                    Metadata for handshake hash   */
/*    public_cipher_metadata                Metadata for public cipher    */
/*    public_cipher_metadata_size           Size of public cipher metadata*/
/*    public_auth_metadata                  Metadata for public auth      */
/*    public_auth_metadata_size             Size of public auth metadata  */
/*    tls_ecc_curves                        ECC curves                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_find_curve_method      Find named curve used         */
/*    _nx_secure_x509_remote_endpoint_certificate_get                     */
/*                                          Get remote host certificate   */
/*    _nx_secure_x509_find_certificate_methods                            */
/*                                          Find certificate methods      */
/*    _nx_secure_x509_pkcs7_decode          Decode the PKCS#7 signature   */
/*    [nx_crypto_init]                      Crypto initialization         */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_server_key_exchange                          */
/*                                          Process ServerKeyExchange     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Yanwu Cai                Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_process_server_key_exchange(const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite, NX_SECURE_TLS_CRYPTO *tls_crypto_table,
                                            USHORT protocol_version, UCHAR *packet_buffer, UINT message_length,
                                            NX_SECURE_TLS_KEY_MATERIAL *tls_key_material, NX_SECURE_TLS_CREDENTIALS *tls_credentials,
                                            NX_SECURE_TLS_HANDSHAKE_HASH *tls_handshake_hash,
                                            VOID *public_cipher_metadata, ULONG public_cipher_metadata_size,
                                            VOID *public_auth_metadata, ULONG public_auth_metadata_size, VOID *tls_ecc_curves)
{

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
USHORT                                length;
#endif /* NX_SECURE_ENABLE_PSK_CIPHERSUITES */
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || \
   (defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE))
UINT                                  status;

#endif /* defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) */
#if defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
VOID                                 *handler = NX_NULL;
#endif
#if defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE)
const NX_CRYPTO_METHOD               *curve_method;
const NX_CRYPTO_METHOD               *ecdh_method;
NX_SECURE_X509_CERT                  *certificate;
NX_CRYPTO_EXTENDED_OUTPUT             extended_output;
UINT                                  key_length;
UCHAR                                *key;
const NX_CRYPTO_METHOD               *curve_method_cert;
const NX_CRYPTO_METHOD               *hash_method;
const NX_CRYPTO_METHOD               *auth_method;
USHORT                                signature_length;
NX_SECURE_X509_CRYPTO                *crypto_methods;
UINT                                  sig_oid_length;
const UCHAR                          *sig_oid;
UINT                                  decrypted_hash_length;
const UCHAR                          *decrypted_hash;
UINT                                  compare_result;
NX_SECURE_EC_PUBLIC_KEY              *ec_pubkey;
UCHAR                                *current_buffer;
UCHAR                                 hash_algorithm;
UCHAR                                 signature_algorithm;
USHORT                                signature_algorithm_id;
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
UINT                                  i;
#endif /* NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED */
#endif /* defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE) */

#if !defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE) || (!NX_SECURE_TLS_TLS_1_0_ENABLED && !NX_SECURE_TLS_TLS_1_1_ENABLED)
    NX_PARAMETER_NOT_USED(tls_crypto_table);
    NX_PARAMETER_NOT_USED(protocol_version);
    NX_PARAMETER_NOT_USED(ciphersuite);
    NX_PARAMETER_NOT_USED(tls_key_material);
    NX_PARAMETER_NOT_USED(tls_credentials);
    NX_PARAMETER_NOT_USED(tls_handshake_hash);
    NX_PARAMETER_NOT_USED(public_cipher_metadata);
    NX_PARAMETER_NOT_USED(public_cipher_metadata_size);
    NX_PARAMETER_NOT_USED(public_auth_metadata);
    NX_PARAMETER_NOT_USED(public_auth_metadata_size);
    NX_PARAMETER_NOT_USED(tls_ecc_curves);
#endif

    NX_PARAMETER_NOT_USED(packet_buffer);
    NX_PARAMETER_NOT_USED(message_length);


#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
    /* Check for PSK ciphersuites. */
    if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_PSK)
    {

        /* Get identity hint length. */
        length = (USHORT)((packet_buffer[0] << 8) + (USHORT)packet_buffer[1]);
        packet_buffer += 2;

        if (length > message_length || length > NX_SECURE_TLS_MAX_PSK_ID_SIZE)
        {
            /* The payload is larger than the header indicated. */
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Extract the identity hint and save in the TLS Session. Then when pre-master is generated
            in client_handshake, we can do the right thing... */
        NX_SECURE_MEMCPY(tls_credentials -> nx_secure_tls_remote_psk_id, &packet_buffer[0], length); /* Use case of memcpy is verified. */
        tls_credentials -> nx_secure_tls_remote_psk_id_size = length;
        return(NX_SECURE_TLS_SUCCESS);


    }
#endif

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    /* Check for ECJ-PAKE ciphersuites. */
    if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
    {
        /* Make sure curve type is named_curve (3). */
        if (packet_buffer[0] != 3)
        {
            return(NX_SECURE_TLS_UNSUPPORTED_ECC_FORMAT);
        }

        /* Only secp256r1 (23) is supported.  */
        if (packet_buffer[1] != 0 || packet_buffer[2] != 23)
        {
            return(NX_SECURE_TLS_UNSUPPORTED_ECC_CURVE);
        }

        packet_buffer += 3;


        tls_key_material -> nx_secure_tls_pre_master_secret_size = 32;

        status = ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_SERVER_KEY_EXCHANGE_PROCESS,
                                                                                 &handler,
                                                                                 (NX_CRYPTO_METHOD *)ciphersuite -> nx_secure_tls_public_auth,
                                                                                 NX_NULL, 0,
                                                                                 packet_buffer,
                                                                                 message_length,
                                                                                 NX_NULL,
                                                                                 tls_key_material -> nx_secure_tls_pre_master_secret,
                                                                                 tls_key_material -> nx_secure_tls_pre_master_secret_size,
                                                                                 public_auth_metadata,
                                                                                 public_auth_metadata_size,
                                                                                 NX_NULL, NX_NULL);
        if (status)
        {
            return(status);
        }

        if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_cleanup)
        {
            status = ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_cleanup(public_auth_metadata);
        }

        return(status);
    }
#endif

#if defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE)
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
    if ((ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDHE))
    {

        if (message_length < 4)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Make sure curve type is named_curve (3). */
        if (packet_buffer[0] != 3)
        {
            return(NX_SECURE_TLS_UNSUPPORTED_ECC_FORMAT);
        }

        /* Find out which named curve the server is using. */
        status = _nx_secure_tls_find_curve_method((NX_SECURE_TLS_ECC *)tls_ecc_curves, (USHORT)((packet_buffer[1] << 8) + packet_buffer[2]), &curve_method, NX_NULL);

        if (status != NX_SUCCESS)
        {

            /* The remote server is using an unsupported curve. */
            return(NX_SECURE_TLS_UNSUPPORTED_ECC_CURVE);
        }

        current_buffer = &packet_buffer[3];

        /* Get reference to remote server certificate so we can get the public key for signature verification. */
        status = _nx_secure_x509_remote_endpoint_certificate_get(&tls_credentials -> nx_secure_tls_certificate_store,
                                                                 &certificate);
        if (status)
        {
            /* No certificate found, error! */
            return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
        }

        key_length = current_buffer[0];
        key = &current_buffer[1];

        current_buffer += key_length + 1;

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
        if (protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1 ||
            protocol_version == NX_SECURE_DTLS_VERSION_1_0)
#else
        if (protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1)
#endif /* NX_SECURE_ENABLE_DTLS */
        {
            if ((UINT)key_length + 6 > message_length)
            {
                return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
            }

            hash_algorithm = NX_SECURE_TLS_HASH_ALGORITHM_SHA1;
            if (certificate -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_EC)
            {
                signature_algorithm = NX_SECURE_TLS_SIGNATURE_ALGORITHM_ECDSA;
            }
            else
            {
                signature_algorithm = NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA;
            }
        }
        else
#endif /* NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED */
        {
            if ((UINT)key_length + 8 > message_length)
            {
                return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
            }

            hash_algorithm = current_buffer[0];
            signature_algorithm = current_buffer[1];
            current_buffer += 2;
        }

        /* Find out the hash algorithm used for the signature. */
        /* Map signature algorithm to internal ID. */
        _nx_secure_tls_get_signature_algorithm_id(((UINT)(hash_algorithm << 8) + signature_algorithm),
                                                  &signature_algorithm_id);

        /* Get the cypto method. */
        status = _nx_secure_x509_find_certificate_methods(certificate,
                                                          signature_algorithm_id,
                                                          &crypto_methods);
        if (status)
        {
            return(NX_SECURE_TLS_UNSUPPORTED_SIGNATURE_ALGORITHM);
        }

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
        if (signature_algorithm == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
           (protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1 ||
            protocol_version == NX_SECURE_DTLS_VERSION_1_0))
#else
        if (signature_algorithm == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
           (protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1))
#endif /* NX_SECURE_ENABLE_DTLS */
        {

            /* TLS 1.0 and TLS 1.1 use MD5 + SHA1 hash for RSA signatures. */
            hash_method = tls_crypto_table -> nx_secure_tls_handshake_hash_md5_method;
        }
        else
#endif /* NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED */
        {
            hash_method = crypto_methods -> nx_secure_x509_hash_method;
        }


        /* Calculate the hash: SHA(ClientHello.random + ServerHello.random +
                                   ServerKeyExchange.params); */
        if (hash_method -> nx_crypto_init)
        {
            status = hash_method -> nx_crypto_init((NX_CRYPTO_METHOD*)hash_method,
                                          NX_NULL,
                                          0,
                                          &handler,
                                          tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch,
                                          tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch_size);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }
        }

        if (hash_method -> nx_crypto_operation != NX_NULL)
        {
            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                               handler,
                                               (NX_CRYPTO_METHOD*)hash_method,
                                               NX_NULL,
                                               0,
                                               NX_NULL,
                                               0,
                                               NX_NULL,
                                               NX_NULL,
                                               0,
                                               tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch,
                                               tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch_size,
                                               NX_NULL,
                                               NX_NULL);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }
        }
        else
        {
            return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
        }

        status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                           handler,
                                           (NX_CRYPTO_METHOD*)hash_method,
                                           NX_NULL,
                                           0,
                                           tls_key_material -> nx_secure_tls_client_random,
                                           32,
                                           NX_NULL,
                                           NX_NULL,
                                           0,
                                           tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch,
                                           tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch_size,
                                           NX_NULL,
                                           NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                           handler,
                                           (NX_CRYPTO_METHOD*)hash_method,
                                           NX_NULL,
                                           0,
                                           tls_key_material -> nx_secure_tls_server_random,
                                           32,
                                           NX_NULL,
                                           NX_NULL,
                                           0,
                                           tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch,
                                           tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch_size,
                                           NX_NULL,
                                           NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                           handler,
                                           (NX_CRYPTO_METHOD*)hash_method,
                                           NX_NULL,
                                           0,
                                           packet_buffer,
                                           (ULONG)(4 + key_length),
                                           NX_NULL,
                                           NX_NULL,
                                           0,
                                           tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch,
                                           tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch_size,
                                           NX_NULL,
                                           NX_NULL);

        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                           handler,
                                           (NX_CRYPTO_METHOD*)hash_method,
                                           NX_NULL,
                                           0,
                                           NX_NULL,
                                           0,
                                           NX_NULL,
                                           hash,
                                           hash_method -> nx_crypto_ICV_size_in_bits >> 3,
                                           tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch,
                                           tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch_size,
                                           NX_NULL,
                                           NX_NULL);
        if(status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        if (hash_method -> nx_crypto_cleanup)
        {
            status = hash_method -> nx_crypto_cleanup(tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }
        }
        handler = NX_NULL;

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
        if (signature_algorithm == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
           (protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1 ||
            protocol_version == NX_SECURE_DTLS_VERSION_1_0))
#else
        if (signature_algorithm == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
           (protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1))
#endif /* NX_SECURE_ENABLE_DTLS */
        {
            hash_method = tls_crypto_table -> nx_secure_tls_handshake_hash_sha1_method;;

            /* Calculate the hash: SHA(ClientHello.random + ServerHello.random +
                                       ServerKeyExchange.params); */
            if (hash_method -> nx_crypto_init)
            {
                status = hash_method -> nx_crypto_init((NX_CRYPTO_METHOD*)hash_method,
                                                NX_NULL,
                                                0,
                                                &handler,
                                                tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch,
                                                tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch_size);

                if(status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }

            if (hash_method -> nx_crypto_operation != NX_NULL)
            {
                status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_INITIALIZE,
                                                    handler,
                                                    (NX_CRYPTO_METHOD*)hash_method,
                                                    NX_NULL,
                                                    0,
                                                    NX_NULL,
                                                    0,
                                                    NX_NULL,
                                                    NX_NULL,
                                                    0,
                                                    tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch,
                                                    tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch_size,
                                                    NX_NULL,
                                                    NX_NULL);

                if(status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }
            else
            {
                return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
            }

            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                handler,
                                                (NX_CRYPTO_METHOD*)hash_method,
                                                NX_NULL,
                                                0,
                                                tls_key_material -> nx_secure_tls_client_random,
                                                32,
                                                NX_NULL,
                                                NX_NULL,
                                                0,
                                                tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch,
                                                tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch_size,
                                                NX_NULL,
                                                NX_NULL);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                handler,
                                                (NX_CRYPTO_METHOD*)hash_method,
                                                NX_NULL,
                                                0,
                                                tls_key_material -> nx_secure_tls_server_random,
                                                32,
                                                NX_NULL,
                                                NX_NULL,
                                                0,
                                                tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch,
                                                tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch_size,
                                                NX_NULL,
                                                NX_NULL);

            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_UPDATE,
                                                handler,
                                                (NX_CRYPTO_METHOD*)hash_method,
                                                NX_NULL,
                                                0,
                                                packet_buffer,
                                                (ULONG)(4 + key_length),
                                                NX_NULL,
                                                NX_NULL,
                                                0,
                                                tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch,
                                                tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch_size,
                                                NX_NULL,
                                                NX_NULL);

            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            status = hash_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                handler,
                                                (NX_CRYPTO_METHOD*)hash_method,
                                                NX_NULL,
                                                0,
                                                NX_NULL,
                                                0,
                                                NX_NULL,
                                                &hash[16],
                                                hash_method -> nx_crypto_ICV_size_in_bits >> 3,
                                                tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch,
                                                tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch_size,
                                                NX_NULL,
                                                NX_NULL);

            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            if (hash_method -> nx_crypto_cleanup)
            {
                status = hash_method -> nx_crypto_cleanup(tls_handshake_hash -> nx_secure_tls_handshake_hash_scratch);

                if(status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }
            handler = NX_NULL;
        }
#endif /* NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED */

        signature_length = (USHORT)((current_buffer[0] << 8) + current_buffer[1]);
        current_buffer += 2;

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
        if (protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1 ||
            protocol_version == NX_SECURE_DTLS_VERSION_1_0)
#else
        if (protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1)
#endif /* NX_SECURE_ENABLE_DTLS */
        {
            if ((UINT)signature_length + key_length + 6 > message_length)
            {
                return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
            }
        }
        else
#endif
        {
            if ((UINT)signature_length + key_length + 8 > message_length)
            {
                return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
            }
        }

        /* Verify the signature. */
        auth_method = ciphersuite -> nx_secure_tls_public_auth;

        if (signature_algorithm == NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA &&
            (auth_method -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATURE_RSA ||
            auth_method -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA))
        {
            /* Verify the RSA signature. */

            if (auth_method -> nx_crypto_init != NX_NULL)
            {
                /* Initialize the crypto method with public key. */
                status = auth_method -> nx_crypto_init((NX_CRYPTO_METHOD*)auth_method,
                                                       (UCHAR *)certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus,
                                                       (NX_CRYPTO_KEY_SIZE)(certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length << 3),
                                                       &handler,
                                                       public_auth_metadata,
                                                       public_auth_metadata_size);
                if(status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }

            if (auth_method -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA)
            {
                status = auth_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                            handler,
                                                            (NX_CRYPTO_METHOD*)auth_method,
                                                            (UCHAR *)certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_exponent,
                                                            (NX_CRYPTO_KEY_SIZE)(certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_exponent_length << 3),
                                                            current_buffer,
                                                            signature_length,
                                                            NX_NULL,
                                                            decrypted_signature,
                                                            sizeof(decrypted_signature),
                                                            public_auth_metadata,
                                                            public_auth_metadata_size,
                                                            NX_NULL, NX_NULL);
                if(status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }

            if (auth_method -> nx_crypto_cleanup)
            {
                status = auth_method -> nx_crypto_cleanup(public_auth_metadata);
                if(status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }
            handler = NX_NULL;

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
            if (protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
                protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1 ||
                protocol_version == NX_SECURE_DTLS_VERSION_1_0)
#else
            if (protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
                protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1)
#endif /* NX_SECURE_ENABLE_DTLS */
            {
                if (signature_length < 39)
                {
                    return(NX_SECURE_TLS_SIGNATURE_VERIFICATION_ERROR);
                }

                /* Block type is 0x00, 0x01 for signatures */
                if (decrypted_signature[0] != 0x0 && decrypted_signature[1] != 0x1)
                {
                    /* Unknown block type. */
                    return(NX_SECURE_TLS_PADDING_CHECK_FAILED);
                }

                /* Check padding. */
                for (i = 2; i < (UINT)(signature_length - 37); ++i)
                {
                    if (decrypted_signature[i] != (UCHAR)0xFF)
                    {
                        /* Bad padding value. */
                        return(NX_SECURE_TLS_PADDING_CHECK_FAILED);
                    }
                }

                /* Make sure we actually saw a NULL byte. */
                if (decrypted_signature[i] != 0x00)
                {
                    return(NX_SECURE_TLS_PADDING_CHECK_FAILED);
                }

                decrypted_hash = &decrypted_signature[i + 1];
                decrypted_hash_length = 36;
            }
            else
#endif /* NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED */
            {
                /* Decode the decrypted signature. */
                status = _nx_secure_x509_pkcs7_decode(decrypted_signature, signature_length, &sig_oid, &sig_oid_length,
                                                      &decrypted_hash, &decrypted_hash_length);
                if (status != NX_SUCCESS)
                {
                    return(NX_SECURE_TLS_SIGNATURE_VERIFICATION_ERROR);
                }

                if (decrypted_hash_length != (hash_method -> nx_crypto_ICV_size_in_bits >> 3))
                {
                    return(NX_SECURE_TLS_SIGNATURE_VERIFICATION_ERROR);
                }
            }

            /* Compare generated hash with decrypted hash. */
            compare_result = (UINT)NX_SECURE_MEMCMP(hash, decrypted_hash, decrypted_hash_length);

#ifdef NX_SECURE_KEY_CLEAR
            NX_SECURE_MEMSET(hash, 0, sizeof(hash));
            NX_SECURE_MEMSET(decrypted_signature, 0, sizeof(decrypted_signature));
#endif /* NX_SECURE_KEY_CLEAR  */

            if (compare_result != 0)
            {
                return(NX_SECURE_TLS_SIGNATURE_VERIFICATION_ERROR);
            }
        }
        else if (signature_algorithm == NX_SECURE_TLS_SIGNATURE_ALGORITHM_ECDSA &&
                 auth_method -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA)
        {
            /* Verify the ECDSA signature. */

            ec_pubkey = &certificate -> nx_secure_x509_public_key.ec_public_key;

            /* Find out which named curve the remote certificate is using. */
            status = _nx_secure_tls_find_curve_method((NX_SECURE_TLS_ECC *)tls_ecc_curves, (USHORT)(ec_pubkey -> nx_secure_ec_named_curve), &curve_method_cert, NX_NULL);

            if(status != NX_SUCCESS)
            {

                /* The remote certificate is using an unsupported curve. */
                return(NX_SECURE_TLS_UNSUPPORTED_ECC_CURVE);
            }

            if (auth_method -> nx_crypto_init != NX_NULL)
            {
                status = auth_method -> nx_crypto_init((NX_CRYPTO_METHOD*)auth_method,
                                                       (UCHAR *)ec_pubkey -> nx_secure_ec_public_key,
                                                       (NX_CRYPTO_KEY_SIZE)(ec_pubkey -> nx_secure_ec_public_key_length << 3),
                                                       &handler,
                                                       public_auth_metadata,
                                                       public_auth_metadata_size);
                if (status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }
            if (auth_method -> nx_crypto_operation == NX_NULL)
            {
                return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
            }

            status = auth_method -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_SET, handler,
                                                        (NX_CRYPTO_METHOD*)auth_method, NX_NULL, 0,
                                                        (UCHAR *)curve_method_cert, sizeof(NX_CRYPTO_METHOD *), NX_NULL,
                                                        NX_NULL, 0,
                                                        public_auth_metadata,
                                                        public_auth_metadata_size,
                                                        NX_NULL, NX_NULL);
            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            status = auth_method -> nx_crypto_operation(NX_CRYPTO_VERIFY, handler,
                                                        (NX_CRYPTO_METHOD*)auth_method,
                                                        (UCHAR *)ec_pubkey -> nx_secure_ec_public_key,
                                                        (NX_CRYPTO_KEY_SIZE)(ec_pubkey -> nx_secure_ec_public_key_length << 3),
                                                        hash,
                                                        hash_method -> nx_crypto_ICV_size_in_bits >> 3,
                                                        NX_NULL,
                                                        current_buffer,
                                                        signature_length,
                                                        public_auth_metadata,
                                                        public_auth_metadata_size,
                                                        NX_NULL, NX_NULL);

            if (status == NX_CRYPTO_AUTHENTICATION_FAILED)
            {
                return(NX_SECURE_TLS_SIGNATURE_VERIFICATION_ERROR);
            }

            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            if (auth_method -> nx_crypto_cleanup)
            {
                status = auth_method -> nx_crypto_cleanup(public_auth_metadata);
                if(status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }
        }
        else
        {
            /* The signature hash algorithm used by the server is not supported. */
            return(NX_SECURE_TLS_UNSUPPORTED_SIGNATURE_ALGORITHM);
        }

        ecdh_method = ciphersuite -> nx_secure_tls_public_cipher;
        if (ecdh_method -> nx_crypto_operation == NX_NULL)
        {
            return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
        }

        if (ecdh_method -> nx_crypto_init != NX_NULL)
        {
            status = ecdh_method -> nx_crypto_init((NX_CRYPTO_METHOD*)ecdh_method,
                                           NX_NULL,
                                           0,
                                           &handler,
                                           public_cipher_metadata,
                                           public_cipher_metadata_size);
            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }
        }

        status = ecdh_method -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_SET, handler,
                                                     (NX_CRYPTO_METHOD*)ecdh_method, NX_NULL, 0,
                                                     (UCHAR *)curve_method, sizeof(NX_CRYPTO_METHOD *), NX_NULL,
                                                     NX_NULL, 0,
                                                     public_cipher_metadata,
                                                     public_cipher_metadata_size,
                                                     NX_NULL, NX_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        /* Store public key in the nx_secure_tls_new_key_material_data. */
        extended_output.nx_crypto_extended_output_data = &tls_key_material -> nx_secure_tls_new_key_material_data[1];
        extended_output.nx_crypto_extended_output_length_in_byte = sizeof(tls_key_material -> nx_secure_tls_new_key_material_data) - 1;
        extended_output.nx_crypto_extended_output_actual_size = 0;
        status = ecdh_method -> nx_crypto_operation(NX_CRYPTO_DH_SETUP, handler,
                                                     (NX_CRYPTO_METHOD*)ecdh_method, NX_NULL, 0,
                                                     NX_NULL, 0, NX_NULL,
                                                     (UCHAR *)&extended_output,
                                                     sizeof(extended_output),
                                                     public_cipher_metadata,
                                                     public_cipher_metadata_size,
                                                     NX_NULL, NX_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        /* Store the length in the first octet. */
        tls_key_material -> nx_secure_tls_new_key_material_data[0] = (UCHAR)extended_output.nx_crypto_extended_output_actual_size;

        extended_output.nx_crypto_extended_output_data = tls_key_material -> nx_secure_tls_pre_master_secret;
        extended_output.nx_crypto_extended_output_length_in_byte = sizeof(tls_key_material -> nx_secure_tls_pre_master_secret);
        extended_output.nx_crypto_extended_output_actual_size = 0;
        status = ecdh_method -> nx_crypto_operation(NX_CRYPTO_DH_CALCULATE, handler,
                                                     (NX_CRYPTO_METHOD*)ecdh_method, NX_NULL, 0,
                                                     key, key_length, NX_NULL,
                                                     (UCHAR *)&extended_output,
                                                     sizeof(extended_output),
                                                     public_cipher_metadata,
                                                     public_cipher_metadata_size,
                                                     NX_NULL, NX_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        tls_key_material -> nx_secure_tls_pre_master_secret_size = extended_output.nx_crypto_extended_output_actual_size;

        if (ecdh_method -> nx_crypto_cleanup)
        {
            status = ecdh_method -> nx_crypto_cleanup(public_cipher_metadata);
        }

        return(status);
    }
#endif /* NX_SECURE_TLS_CLIENT_DISABLED */
#endif /* defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE) */

    /* No key exchange should be received by the current cipher suite. */
    return(NX_SECURE_TLS_UNEXPECTED_MESSAGE);
}

#endif
