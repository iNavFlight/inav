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
/*    _nx_secure_tls_process_server_key_exchange           PORTABLE C     */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ServerKeyExchange message,      */
/*    which is sent by the remote TLS Server host when certain            */
/*    ciphersuites (e.g. those using Diffie-Hellman) are used.            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Pointer to message data       */
/*    message_length                        Length of message data (bytes)*/
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
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), update   */
/*                                            ECC find curve method,      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            removed unnecessary code,   */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022     Yanwu Cai                Modified comment(s), added    */
/*                                            custom secret generation,   */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_server_key_exchange(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, UINT message_length)
{

#if !defined(NX_SECURE_TLS_CLIENT_DISABLED)
UINT                                  status;
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || \
   (defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE))
const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite;
#endif /* defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) */

#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || \
   (defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE))
    /* Figure out which ciphersuite we are using. */
    ciphersuite = tls_session -> nx_secure_tls_session_ciphersuite;
    if (ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }
#endif /* defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) */


#if defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE)

    if (ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDHE)
    {
        if (tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_SERVER_CERTIFICATE)
        {
            return(NX_SECURE_TLS_UNEXPECTED_MESSAGE);
        }

        tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_SERVER_KEY_EXCHANGE;

    }

    status = tls_session -> nx_secure_process_server_key_exchange(tls_session -> nx_secure_tls_session_ciphersuite, tls_session -> nx_secure_tls_crypto_table,
                                                                  tls_session -> nx_secure_tls_protocol_version, packet_buffer, message_length,
                                                                  &tls_session -> nx_secure_tls_key_material, &tls_session -> nx_secure_tls_credentials,
                                                                  &tls_session -> nx_secure_tls_handshake_hash, tls_session -> nx_secure_public_cipher_metadata_area,
                                                                  tls_session -> nx_secure_public_cipher_metadata_size,
                                                                  tls_session -> nx_secure_public_auth_metadata_area, tls_session -> nx_secure_public_auth_metadata_size,
                                                                  &tls_session -> nx_secure_tls_ecc);

#else
    status = tls_session -> nx_secure_process_server_key_exchange(tls_session -> nx_secure_tls_session_ciphersuite, tls_session -> nx_secure_tls_crypto_table,
                                                                  tls_session -> nx_secure_tls_protocol_version, packet_buffer, message_length,
                                                                  &tls_session -> nx_secure_tls_key_material, &tls_session -> nx_secure_tls_credentials,
                                                                  &tls_session -> nx_secure_tls_handshake_hash, tls_session -> nx_secure_public_cipher_metadata_area,
                                                                  tls_session -> nx_secure_public_cipher_metadata_size,
                                                                  tls_session -> nx_secure_public_auth_metadata_area, tls_session -> nx_secure_public_auth_metadata_size,
                                                                  NX_NULL);

#endif /* defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE) */

    return(status);

#else /* !define(NX_SECURE_TLS_CLIENT_DISABLED) */

    /* If Client TLS is disabled and we recieve a server key exchange, error! */
    NX_PARAMETER_NOT_USED(tls_session);
    NX_PARAMETER_NOT_USED(packet_buffer);
    NX_PARAMETER_NOT_USED(message_length);

    return(NX_SECURE_TLS_UNEXPECTED_MESSAGE);
#endif
}

