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
/*    _nx_secure_generate_server_key_exchange             PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yanwu Cai, Microsoft Corporation                                    */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates a ServerKeyExchange message, which is used  */
/*    when the chosen ciphersuite requires additional information for key */
/*    generation, such as when using Diffie-Hellman ciphers.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ciphersuite                           Selected cipher suite         */
/*    protocol_version                      Selected TLS version          */
/*    tls_1_3                               Whether TLS 1.3 is chosen     */
/*    tls_crypto_table                      TLS crypto methods            */
/*    tls_handshake_hash                    Metadata for handshake hash   */
/*    tls_key_material                      TLS key material              */
/*    tls_credentials                       TLS credentials               */
/*    data_buffer                           Pointer to output buffer      */
/*    buffer_length                         Length of data buffer         */
/*    output_size                           Size of data written to buffer*/
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
/*    _nx_secure_tls_ecc_generate_keys      Generate keys for ECC exchange*/
/*    [nx_crypto_init]                      Initialize Crypto Method      */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_send_server_key_exchange                             */
/*                                          Send ServerKeyExchange        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Yanwu Cai                Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_generate_server_key_exchange(const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite, USHORT protocol_version, UCHAR tls_1_3,
                                             NX_SECURE_TLS_CRYPTO *tls_crypto_table, NX_SECURE_TLS_HANDSHAKE_HASH *tls_handshake_hash,
                                             NX_SECURE_TLS_KEY_MATERIAL *tls_key_material, NX_SECURE_TLS_CREDENTIALS *tls_credentials,
                                             UCHAR *data_buffer, ULONG buffer_length, ULONG *output_size,
                                             VOID *public_cipher_metadata, ULONG public_cipher_metadata_size,
                                             VOID *public_auth_metadata, ULONG public_auth_metadata_size, VOID *tls_ecc_curves)
{
UINT                                length;

#if defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE)
UINT                                status;
#endif /* defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) || defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE) */

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
USHORT                              identity_length;
UCHAR                              *identity;
#endif

#if defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
VOID                               *handler = NX_NULL;
USHORT                              named_curve;
NX_CRYPTO_METHOD                   *crypto_method;
NX_CRYPTO_EXTENDED_OUTPUT           extended_output;
#endif /* defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE) */

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA *ecdhe_data;
#endif


    /* Build up the server key exchange message. Structure:
     * |        2        |  <key data length>  |
     * | Key data length |  Key data (opaque)  |
     */

#ifndef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    NX_PARAMETER_NOT_USED(ciphersuite);
    NX_PARAMETER_NOT_USED(protocol_version);
    NX_PARAMETER_NOT_USED(tls_1_3);
    NX_PARAMETER_NOT_USED(tls_crypto_table);
    NX_PARAMETER_NOT_USED(tls_handshake_hash);
    NX_PARAMETER_NOT_USED(tls_key_material);
    NX_PARAMETER_NOT_USED(tls_credentials);
    NX_PARAMETER_NOT_USED(data_buffer);
    NX_PARAMETER_NOT_USED(buffer_length);
    NX_PARAMETER_NOT_USED(public_cipher_metadata);
    NX_PARAMETER_NOT_USED(public_cipher_metadata_size);
    NX_PARAMETER_NOT_USED(public_auth_metadata);
    NX_PARAMETER_NOT_USED(public_auth_metadata_size);
    NX_PARAMETER_NOT_USED(tls_ecc_curves);
#endif

    length = 0;

    /* In the future, any Diffie-Hellman-based ciphersuites will populate this message with key
       data. RSA ciphersuites do not use this message. */

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES

    /* Check for PSK ciphersuites. */
    if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_PSK)
    {
        /* If PSK is being used, the server sends an identity value to the client so
           the client can respond with the appropriate PSK. */

        /* Get identity hint and length. */
        identity = tls_credentials -> nx_secure_tls_psk_store[0].nx_secure_tls_psk_id_hint;
        identity_length = (USHORT)tls_credentials -> nx_secure_tls_psk_store[0].nx_secure_tls_psk_id_hint_size;

        if ((identity_length + 2u) > buffer_length)
        {

            /* Packet buffer too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        /* Use identity hint length for key data length. */
        data_buffer[length]     = (UCHAR)((identity_length & 0xFF00) >> 8);
        data_buffer[length + 1] = (UCHAR)(identity_length & 0x00FF);
        length = (USHORT)(length + 2);

        /* Extract the identity hint and put it into the packet buffer. */
        NX_SECURE_MEMCPY(&data_buffer[length], identity, identity_length); /* Use case of memcpy is verified. */

        /* Advance our total length. */
        length = (USHORT)(length + identity_length);
    }
#endif

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE

    /* Check for ECJ-PAKE ciphersuites. */
    if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
    {

        if (buffer_length < 3)
        {

            /* Packet buffer too small. */
            return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
        }

        /* ECCurveType: named_curve (3). */
        data_buffer[length] = NX_CRYPTO_EC_CURVE_TYPE_NAMED_CURVE;
        length += 1;

        /* NamedCurve: secp256r1 (23) */
        named_curve = (USHORT)NX_CRYPTO_EC_SECP256R1;
        data_buffer[length] = (UCHAR)((named_curve >> 8) & 0xFF);
        data_buffer[length + 1] = (UCHAR)(named_curve & 0xFF);
        length += 2;

        extended_output.nx_crypto_extended_output_data = &data_buffer[length];
        extended_output.nx_crypto_extended_output_length_in_byte = buffer_length - length;
        extended_output.nx_crypto_extended_output_actual_size = 0;

        crypto_method = (NX_CRYPTO_METHOD*)ciphersuite -> nx_secure_tls_public_auth;
        status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_SERVER_KEY_EXCHANGE_GENERATE,
                                                      handler,
                                                      crypto_method,
                                                      NX_NULL, 0,
                                                      NX_NULL, 0, NX_NULL,
                                                      (UCHAR *)&extended_output,
                                                      sizeof(extended_output),
                                                      public_auth_metadata,
                                                      public_auth_metadata_size,
                                                      NX_NULL, NX_NULL);
        if (status)
        {
            return(status);
        }
        length += extended_output.nx_crypto_extended_output_actual_size;
    }
#endif

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    /* Check for ECDHE ciphersuites. */
    if (ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDHE)
    {
        ecdhe_data = (NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA *)tls_key_material -> nx_secure_tls_new_key_material_data;

        length = buffer_length;

        status = _nx_secure_tls_ecc_generate_keys(ciphersuite, protocol_version, tls_1_3,
                                                  tls_crypto_table, tls_handshake_hash, (NX_SECURE_TLS_ECC *)tls_ecc_curves,
                                                  tls_key_material, tls_credentials, ecdhe_data -> nx_secure_tls_ecdhe_named_curve, NX_TRUE,
                                                  data_buffer, &length, ecdhe_data, public_cipher_metadata, public_cipher_metadata_size,
                                                  public_auth_metadata, public_auth_metadata_size);

        if (status != NX_SUCCESS)
        {
            return(status);
        }
    }
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */


    /* Finally, we let the caller know the length . */
    *output_size = length;

    return(NX_SECURE_TLS_SUCCESS);
}

