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
/*    _nx_secure_generate_premaster_secret                PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yanwu Cai, Microsoft Corporation                                    */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the Pre-Master Secret for TLS Client        */
/*    instances. It is sent to the remote host and used as the seed for   */
/*    session key generation.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ciphersuite                           Selected cipher suite         */
/*    protocol_version                      Selected TLS version          */
/*    tls_key_material                      TLS key material              */
/*    tls_credentials                       TLS credentials               */
/*    session_type                          Server or client session      */
/*    received_remote_credentials           Indicates credentials received*/
/*    public_cipher_metadata                Metadata for public cipher    */
/*    public_cipher_metadata_size           Size of public cipher metadata*/
/*    tls_ecc_curves                        ECC curves                    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_psk_find               Find PSK from store           */
/*    _nx_secure_tls_find_curve_method      Find named curve used         */
/*    [nx_crypto_init]                      Initialize crypto             */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_generate_premaster_secret                            */
/*                                          Generate pre-master secret    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Yanwu Cai                Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_generate_premaster_secret(const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite, USHORT protocol_version, NX_SECURE_TLS_KEY_MATERIAL *tls_key_material,
                                          NX_SECURE_TLS_CREDENTIALS *tls_credentials, UINT session_type, USHORT *received_remote_credentials,
                                          VOID *public_cipher_metadata, ULONG public_cipher_metadata_size, VOID *tls_ecc_curves)
{
UINT                      *buffer_ptr;
UINT                       i;
UINT                       status = NX_SECURE_TLS_SUCCESS;
#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
UCHAR                     *psk_data;
UINT                       psk_length;
UINT                       index;
#endif
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
NX_SECURE_X509_CERT       *server_certificate;
const NX_CRYPTO_METHOD    *curve_method_cert;
const NX_CRYPTO_METHOD    *ecdh_method;
NX_SECURE_EC_PUBLIC_KEY   *ec_pubkey;
VOID                      *handler = NX_NULL;
NX_CRYPTO_EXTENDED_OUTPUT  extended_output;
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

#ifndef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    NX_PARAMETER_NOT_USED(public_cipher_metadata);
    NX_PARAMETER_NOT_USED(public_cipher_metadata_size);
    NX_PARAMETER_NOT_USED(tls_ecc_curves);
#endif
#ifndef NX_SECURE_ENABLE_PSK_CIPHERSUITES
    NX_PARAMETER_NOT_USED(session_type);
    NX_PARAMETER_NOT_USED(ciphersuite);
    NX_PARAMETER_NOT_USED(tls_credentials);
    NX_PARAMETER_NOT_USED(public_cipher_metadata);
    NX_PARAMETER_NOT_USED(public_cipher_metadata_size);
    NX_PARAMETER_NOT_USED(tls_ecc_curves);
#ifndef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    NX_PARAMETER_NOT_USED(received_remote_credentials);
#endif
#endif

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    if (ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDHE)
    {

        return(NX_SECURE_TLS_SUCCESS);
    }
    else if (ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDH)
    {
        /* Get reference to remote server certificate so we can find out the named curve. */
        status = _nx_secure_x509_remote_endpoint_certificate_get(&tls_credentials -> nx_secure_tls_certificate_store,
                                                                 &server_certificate);
        if (status)
        {
            /* No certificate found, error! */
            return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
        }

        ec_pubkey = &server_certificate -> nx_secure_x509_public_key.ec_public_key;

        /* Find out which named curve the remote certificate is using. */
        status = _nx_secure_tls_find_curve_method((NX_SECURE_TLS_ECC *)tls_ecc_curves, (USHORT)(ec_pubkey -> nx_secure_ec_named_curve), &curve_method_cert, NX_NULL);

        if(status != NX_SUCCESS)
        {
            return(status);
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
                                                    (UCHAR *)curve_method_cert, sizeof(NX_CRYPTO_METHOD *), NX_NULL,
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

        tls_key_material -> nx_secure_tls_new_key_material_data[0] = (UCHAR)extended_output.nx_crypto_extended_output_actual_size;

        extended_output.nx_crypto_extended_output_data = tls_key_material -> nx_secure_tls_pre_master_secret;
        extended_output.nx_crypto_extended_output_length_in_byte = sizeof(tls_key_material -> nx_secure_tls_pre_master_secret);
        extended_output.nx_crypto_extended_output_actual_size = 0;
        status = ecdh_method -> nx_crypto_operation(NX_CRYPTO_DH_CALCULATE, handler,
                                                    (NX_CRYPTO_METHOD*)ecdh_method, NX_NULL, 0,
                                                    (UCHAR *)ec_pubkey -> nx_secure_ec_public_key,
                                                    ec_pubkey -> nx_secure_ec_public_key_length, NX_NULL,
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
            if(status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }
        }

        return(NX_SECURE_TLS_SUCCESS);
    }
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
    /* Check for PSK ciphersuites. */
    if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_PSK)
    {
        /* Now, using the identity as a key, find the PSK in our PSK store. */
        if (session_type == NX_SECURE_TLS_SESSION_TYPE_SERVER)
        {
            /* Server just uses its PSK. */
            psk_data = tls_credentials -> nx_secure_tls_psk_store[0].nx_secure_tls_psk_data;
            psk_length = tls_credentials -> nx_secure_tls_psk_store[0].nx_secure_tls_psk_data_size;
        }
        else
        {
            /*  Client has to search for the PSK based on the identity hint. */
            status = _nx_secure_tls_psk_find(tls_credentials, &psk_data, &psk_length, tls_credentials -> nx_secure_tls_remote_psk_id,
                                             tls_credentials -> nx_secure_tls_remote_psk_id_size, NX_NULL);

            if (status != NX_SUCCESS)
            {
                return(status);
            }
        }

        /* From RFC 4279:
           The premaster secret is formed as follows: if the PSK is N octets
           long, concatenate a uint16 with the value N, N zero octets, a second
           uint16 with the value N, and the PSK itself.
         |   2   |   <N>  |   2   |   <N>   |
         |   N   |    0   |   N   |   PSK   |
         */
        index = 0;

        if ((2 + psk_length + 2 + psk_length) > sizeof(tls_key_material -> nx_secure_tls_pre_master_secret))
        {

            /* No more PSK space. */
            return(NX_SECURE_TLS_NO_MORE_PSK_SPACE);
        }

        tls_key_material -> nx_secure_tls_pre_master_secret[0] = (UCHAR)(psk_length >> 8);
        tls_key_material -> nx_secure_tls_pre_master_secret[1] = (UCHAR)psk_length;
        index += 2;

        NX_SECURE_MEMSET(&tls_key_material -> nx_secure_tls_pre_master_secret[index], 0, psk_length);
        index += psk_length;

        tls_key_material -> nx_secure_tls_pre_master_secret[index] = (UCHAR)(psk_length >> 8);
        tls_key_material -> nx_secure_tls_pre_master_secret[index + 1] = (UCHAR)psk_length;
        index += 2;

        NX_SECURE_MEMCPY(&tls_key_material -> nx_secure_tls_pre_master_secret[index], psk_data, psk_length); /* Use case of memcpy is verified. */
        index += psk_length;

        /* Save the pre-master secret size for later use. */
        tls_key_material -> nx_secure_tls_pre_master_secret_size = 2 + psk_length + 2 + psk_length;

        /* We are using PSK for our credentials and now that we have generated keys we can consider the
           remote host's credentials to have been received. */
        *received_remote_credentials = NX_TRUE;

        return(NX_SECURE_TLS_SUCCESS);
    }
#endif

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    /* When using ECJ-PAKE ciphersuite, pre-master secret is already generated. */
    if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
    {
        /* If using EC-JPAKE, credentials are passed differently - if we get here credentials should be OK. */
        *received_remote_credentials = NX_TRUE;

        return(NX_SECURE_TLS_SUCCESS);
    }
#endif

    /* Generate the Pre-Master Secret that is used to generate the key material
       used in the session. For TLS 1.1, the secret consists of two bytes
       representing the highest protocol version the client supports, followed
       by 46 random bytes. */
    buffer_ptr = (UINT *)tls_key_material -> nx_secure_tls_pre_master_secret;

    /* Generate 48 bytes of random data, fill in the version afterwards. */
    for (i = 0; i < 12; i++)
    {
        /* Fill with 12 ULONG randoms, then fix first two bytes to protocol version after. */
        *(buffer_ptr + i) = (UINT)NX_RAND();
    }

    /* First two bytes are newest version supported by client . */
    buffer_ptr[0] = ((ULONG)protocol_version << 16) | (buffer_ptr[0] & 0x0000FFFF);
    NX_CHANGE_ULONG_ENDIAN(buffer_ptr[0]);

    tls_key_material -> nx_secure_tls_pre_master_secret_size = 48;

    return(status);
}

