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

static UCHAR _nx_secure_client_padded_pre_master[600];


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_process_client_key_exchange               PORTABLE C     */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yanwu Cai, Microsoft Corporation                                    */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ClientKeyExchange message,      */
/*    which contains the encrypted Pre-Master Secret. This function       */
/*    decrypts the Pre-Master Secret and saves it in the TLS session      */
/*    control block for use in generating session key material later.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ciphersuite                           Selected cipher suite         */
/*    protocol_version                      Selected TLS version          */
/*    packet_buffer                         Pointer to message data       */
/*    message_length                        Length of message data (bytes)*/
/*    received_remote_credentials           Indicates credentials received*/
/*    tls_key_material                      TLS key material              */
/*    tls_credentials                       TLS credentials               */
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
/*    _nx_secure_generate_premaster_secret  Generate the shared secret    */
/*                                            used to generate keys later */
/*    _nx_secure_x509_local_device_certificate_get                        */
/*                                          Get the local certificate     */
/*                                            for its keys                */
/*    _nx_secure_tls_find_curve_method      Find named curve used         */
/*    [nx_crypto_init]                      Initialize crypto             */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_client_key_exchange                          */
/*                                          Process ClientKeyExchange     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Yanwu Cai                Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_process_client_key_exchange(const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite, USHORT protocol_version,
                                            UCHAR *packet_buffer, UINT message_length, USHORT *received_remote_credentials,
                                            NX_SECURE_TLS_KEY_MATERIAL *tls_key_material, NX_SECURE_TLS_CREDENTIALS *tls_credentials,
                                            VOID *public_cipher_metadata, ULONG public_cipher_metadata_size,
                                            VOID *public_auth_metadata, ULONG public_auth_metadata_size, VOID *tls_ecc_curves)
{
USHORT                              length;
UINT                                status;
UCHAR                              *encrypted_pre_master_secret;
const NX_CRYPTO_METHOD             *public_cipher_method;
NX_SECURE_X509_CERT                *local_certificate;
UINT                                user_defined_key;
VOID                               *handler = NX_NULL;
UCHAR                               rand_byte;
UINT                                i;
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
NX_SECURE_EC_PRIVATE_KEY           *ec_privkey;
NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA *ecdhe_data;
NX_CRYPTO_EXTENDED_OUTPUT           extended_output;
const NX_CRYPTO_METHOD             *curve_method;
const NX_CRYPTO_METHOD             *ecdh_method;
UCHAR                              *private_key;
UINT                                private_key_length;
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

#ifndef NX_SECURE_ENABLE_PSK_CIPHERSUITES
    NX_PARAMETER_NOT_USED(received_remote_credentials);
#endif /* NX_SECURE_ENABLE_PSK_CIPHERSUITES */
#ifndef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    NX_PARAMETER_NOT_USED(tls_ecc_curves);
#endif
    NX_PARAMETER_NOT_USED(protocol_version);
    NX_PARAMETER_NOT_USED(public_auth_metadata);
    NX_PARAMETER_NOT_USED(public_auth_metadata_size);

    /* Process key material. The contents of the handshake record differ according to the
       ciphersuite chosen in the Client/Server Hello negotiation. */

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
       /* Check for ECJ-PAKE ciphersuites and generate the pre-master-secret. */
    if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
    {

        tls_key_material -> nx_secure_tls_pre_master_secret_size = 32;

        public_cipher_method = ciphersuite -> nx_secure_tls_public_auth;
        status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_CLIENT_KEY_EXCHANGE_PROCESS,
                                                             handler,
                                                             (NX_CRYPTO_METHOD *)public_cipher_method,
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

        if (public_cipher_method -> nx_crypto_cleanup)
        {
            status = public_cipher_method -> nx_crypto_cleanup(public_auth_metadata);

            if (status)
            {
                return(status);
            }
        }
    }
    else
#endif
    {

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
        /* Check for PSK ciphersuites and generate the pre-master-secret. */
        if (ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_PSK)
        {
            status = _nx_secure_generate_premaster_secret(ciphersuite, protocol_version, tls_key_material, tls_credentials,
                                                          NX_SECURE_TLS_SESSION_TYPE_SERVER, received_remote_credentials,
                                                          public_cipher_metadata, public_cipher_metadata_size, tls_ecc_curves);

            if (status != NX_SUCCESS)
            {
                return(status);
            }
        }
        else
#endif
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
        if (ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDH ||
            ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDHE)
        {
            length = packet_buffer[0];

            if ((UINT)length + 1 > message_length)
            {
                /* The public key length is larger than the header indicated. */
                return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
            }

            if (ciphersuite -> nx_secure_tls_public_cipher -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECDH)
            {
                /* Get the local certificate. */
                if (tls_credentials -> nx_secure_tls_active_certificate != NX_NULL)
                {
                    local_certificate = tls_credentials -> nx_secure_tls_active_certificate;
                }
                else
                {
                    /* Get reference to local device certificate. NX_NULL is passed for name to get default entry. */
                    status = _nx_secure_x509_local_device_certificate_get(&tls_credentials -> nx_secure_tls_certificate_store,
                                                                          NX_NULL, &local_certificate);
                    if (status != NX_SUCCESS)
                    {
                        local_certificate = NX_NULL;
                    }
                }

                if (local_certificate == NX_NULL)
                {
                    /* No certificate found, error! */
                    return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
                }

                ec_privkey = &local_certificate -> nx_secure_x509_private_key.ec_private_key;

                /* Find out which named curve the local certificate is using. */
                status = _nx_secure_tls_find_curve_method((NX_SECURE_TLS_ECC *)tls_ecc_curves, (USHORT)(ec_privkey -> nx_secure_ec_named_curve), &curve_method, NX_NULL);

                if (status != NX_SUCCESS)
                {
                    return(status);
                }

                private_key = (UCHAR *)ec_privkey -> nx_secure_ec_private_key;
                private_key_length = ec_privkey -> nx_secure_ec_private_key_length;
            }
            else
            {
                ecdhe_data = (NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA *)tls_key_material -> nx_secure_tls_new_key_material_data;

                /* Find out which named curve the we are using. */
                status = _nx_secure_tls_find_curve_method((NX_SECURE_TLS_ECC *)tls_ecc_curves, (USHORT)ecdhe_data -> nx_secure_tls_ecdhe_named_curve, &curve_method, NX_NULL);

                if (status != NX_SUCCESS)
                {
                    return(status);
                }

                private_key = ecdhe_data -> nx_secure_tls_ecdhe_private_key;
                private_key_length = ecdhe_data -> nx_secure_tls_ecdhe_private_key_length;
            }

            if (curve_method == NX_NULL)
            {
                /* No named curve is selected. */
                return(NX_SECURE_TLS_UNSUPPORTED_ECC_CURVE);
            }

            ecdh_method = ciphersuite -> nx_secure_tls_public_cipher;
            if (ecdh_method -> nx_crypto_operation == NX_NULL)
            {
                return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
            }

            if (ecdh_method -> nx_crypto_init != NX_NULL)
            {
                status = ecdh_method -> nx_crypto_init((NX_CRYPTO_METHOD *)ecdh_method,
                                                       NX_NULL,
                                                       0,
                                                       &handler,
                                                       public_cipher_metadata,
                                                       public_cipher_metadata_size);
                if (status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }

            status = ecdh_method -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_SET, handler,
                                                        (NX_CRYPTO_METHOD *)ecdh_method, NX_NULL, 0,
                                                        (UCHAR *)curve_method, sizeof(NX_CRYPTO_METHOD *), NX_NULL,
                                                        NX_NULL, 0,
                                                        public_cipher_metadata,
                                                        public_cipher_metadata_size,
                                                        NX_NULL, NX_NULL);
            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            /* Import the private key to the ECDH context. */
            status = ecdh_method -> nx_crypto_operation(NX_CRYPTO_DH_KEY_PAIR_IMPORT, handler,
                                                        (NX_CRYPTO_METHOD *)ecdh_method,
                                                        private_key, private_key_length << 3,
                                                        NX_NULL, 0, NX_NULL,
                                                        NX_NULL,
                                                        0,
                                                        public_cipher_metadata,
                                                        public_cipher_metadata_size,
                                                        NX_NULL, NX_NULL);
            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }

            extended_output.nx_crypto_extended_output_data = tls_key_material -> nx_secure_tls_pre_master_secret;
            extended_output.nx_crypto_extended_output_length_in_byte = sizeof(tls_key_material -> nx_secure_tls_pre_master_secret);
            extended_output.nx_crypto_extended_output_actual_size = 0;
            status = ecdh_method -> nx_crypto_operation(NX_CRYPTO_DH_CALCULATE, handler,
                                                        (NX_CRYPTO_METHOD *)ecdh_method, NX_NULL, 0,
                                                        &packet_buffer[1],
                                                        length, NX_NULL,
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
                if (status != NX_CRYPTO_SUCCESS)
                {
                    return(status);
                }
            }
        }
        else
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
        {       /* Certificate-based authentication. */

            if (message_length < 2)
            {
                return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
            }

            /* Get pre-master-secret length. */
            length = (USHORT)((packet_buffer[0] << 8) + (USHORT)packet_buffer[1]);
            packet_buffer += 2;

            if ((UINT)length + 2 > message_length)
            {
                /* The payload is larger than the header indicated. */
                return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
            }

            /* Pointer to the encrypted pre-master secret in our packet buffer. */
            encrypted_pre_master_secret = &packet_buffer[0];


            if (ciphersuite -> nx_secure_tls_ciphersuite == TLS_NULL_WITH_NULL_NULL)
            {
                /* Special case - NULL ciphersuite. No keys are generated. */
                if (length > sizeof(tls_key_material -> nx_secure_tls_pre_master_secret))
                {
                    length = sizeof(tls_key_material -> nx_secure_tls_pre_master_secret);
                }
                NX_SECURE_MEMCPY(tls_key_material -> nx_secure_tls_pre_master_secret, encrypted_pre_master_secret, length); /* Use case of memcpy is verified. */
                tls_key_material -> nx_secure_tls_pre_master_secret_size = length;
            }

            /* Get reference to local device certificate. NX_NULL is passed for name to get default entry. */
            status = _nx_secure_x509_local_device_certificate_get(&tls_credentials -> nx_secure_tls_certificate_store,
                                                                  NX_NULL, &local_certificate);

            if (status)
            {
                /* No certificate found, error! */
                return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
            }

            /* Get the public cipher method pointer for this session. */
            public_cipher_method = ciphersuite -> nx_secure_tls_public_cipher;

            /* Check for user-defined key types. */
            user_defined_key = NX_FALSE;
            if (((local_certificate -> nx_secure_x509_private_key_type & NX_SECURE_X509_KEY_TYPE_USER_DEFINED_MASK) != 0x0) ||
                (local_certificate -> nx_secure_x509_private_key_type == NX_SECURE_X509_KEY_TYPE_HARDWARE))
            {
                user_defined_key = NX_TRUE;
            }

            /* See if we are using RSA. Separate from other methods (e.g. ECC, DH) for proper handling of padding. */
            if (public_cipher_method -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_RSA &&
                local_certificate -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_RSA)
            {
                /* Check for user-defined keys. */
                if (user_defined_key)
                {
                    /* A user-defined key is passed directly into the crypto routine. */
                    status = public_cipher_method -> nx_crypto_operation(local_certificate -> nx_secure_x509_private_key_type,
                                                                         NX_NULL,
                                                                         (NX_CRYPTO_METHOD *)public_cipher_method,
                                                                         (UCHAR *)local_certificate -> nx_secure_x509_private_key.user_key.key_data,
                                                                         (NX_CRYPTO_KEY_SIZE)(local_certificate -> nx_secure_x509_private_key.user_key.key_length),
                                                                         encrypted_pre_master_secret,
                                                                         length,
                                                                         NX_NULL,
                                                                         _nx_secure_client_padded_pre_master,
                                                                         sizeof(_nx_secure_client_padded_pre_master),
                                                                         public_cipher_metadata,
                                                                         public_cipher_metadata_size,
                                                                         NX_NULL, NX_NULL);

                    if (status != NX_CRYPTO_SUCCESS)
                    {
                        return(status);
                    }
                }
                else
                {
                    /* Generic RSA operation, use pre-parsed RSA key data. */
                    if (public_cipher_method -> nx_crypto_init != NX_NULL)
                    {
                        /* Initialize the crypto method with public key. */
                        status = public_cipher_method -> nx_crypto_init((NX_CRYPTO_METHOD *)public_cipher_method,
                                                                        (UCHAR *)local_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus,
                                                                        (NX_CRYPTO_KEY_SIZE)(local_certificate -> nx_secure_x509_public_key.rsa_public_key.nx_secure_rsa_public_modulus_length << 3),
                                                                        &handler,
                                                                        public_cipher_metadata,
                                                                        public_cipher_metadata_size);

                        if (status != NX_CRYPTO_SUCCESS)
                        {
                            return(status);
                        }
                    }

                    if (public_cipher_method -> nx_crypto_operation != NX_NULL)
                    {


                        /* Check for P and Q in the private key. If they are present, we can use them to
                           speed up RSA using the Chinese Remainder Theorem version of the algorithm. */
                        if (local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_prime_p != NX_NULL &&
                            local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_prime_q != NX_NULL)
                        {


                            status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_SET_PRIME_P,
                                                                                 handler,
                                                                                 (NX_CRYPTO_METHOD *)public_cipher_method,
                                                                                 NX_NULL,
                                                                                 0,
                                                                                 (VOID *)local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_prime_p,
                                                                                 local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_prime_p_length,
                                                                                 NX_NULL,
                                                                                 NX_NULL,
                                                                                 0,
                                                                                 public_cipher_metadata,
                                                                                 public_cipher_metadata_size,
                                                                                 NX_NULL, NX_NULL);

                            if (status != NX_CRYPTO_SUCCESS)
                            {
                                return(status);
                            }

                            status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_SET_PRIME_Q,
                                                                                 handler,
                                                                                 (NX_CRYPTO_METHOD *)public_cipher_method,
                                                                                 NX_NULL,
                                                                                 0,
                                                                                 (VOID *)local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_prime_q,
                                                                                 local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_prime_q_length,
                                                                                 NX_NULL,
                                                                                 NX_NULL,
                                                                                 0,
                                                                                 public_cipher_metadata,
                                                                                 public_cipher_metadata_size,
                                                                                 NX_NULL, NX_NULL);

                            if (status != NX_CRYPTO_SUCCESS)
                            {
                                return(status);
                            }

                        }

                        /* Decrypt the pre-master-secret using the private key provided by the user
                            and place the result in the session key material space in our socket. */
                        status = public_cipher_method -> nx_crypto_operation(NX_CRYPTO_DECRYPT,
                                                                             handler,
                                                                             (NX_CRYPTO_METHOD *)public_cipher_method,
                                                                             (UCHAR *)local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_exponent,
                                                                             (NX_CRYPTO_KEY_SIZE)(local_certificate -> nx_secure_x509_private_key.rsa_private_key.nx_secure_rsa_private_exponent_length << 3),
                                                                             encrypted_pre_master_secret,
                                                                             length,
                                                                             NX_NULL,
                                                                             _nx_secure_client_padded_pre_master,
                                                                             sizeof(_nx_secure_client_padded_pre_master),
                                                                             public_cipher_metadata,
                                                                             public_cipher_metadata_size,
                                                                             NX_NULL, NX_NULL);

                        if (status != NX_CRYPTO_SUCCESS)
                        {
                            return(status);
                        }

                    }

                    if (public_cipher_method -> nx_crypto_cleanup)
                    {
                        status = public_cipher_method -> nx_crypto_cleanup(public_cipher_metadata);

                        if (status != NX_CRYPTO_SUCCESS)
                        {
#ifdef NX_SECURE_KEY_CLEAR
                            NX_SECURE_MEMSET(_nx_secure_client_padded_pre_master, 0, sizeof(_nx_secure_client_padded_pre_master));
#endif /* NX_SECURE_KEY_CLEAR  */

                            return(status);
                        }
                    }
                }

                if (length < NX_SECURE_TLS_RSA_PREMASTER_SIZE + 3)
                {
                    return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
                }

                /* Check padding - first 2 bytes should be 0x00, 0x02 for PKCS#1 padding. A 0x00 byte should immediately
                   precede the data. */
                if (_nx_secure_client_padded_pre_master[0] != 0x00 ||
                    _nx_secure_client_padded_pre_master[1] != 0x02 ||
                    _nx_secure_client_padded_pre_master[length - NX_SECURE_TLS_RSA_PREMASTER_SIZE - 1] != 0x00)
                {

                    /* Invalid padding.  To avoid Bleichenbacher's attack, use random numbers to
                       generate premaster secret and continue the operation (which will be properly
                       taken care of later in _nx_secure_tls_process_finished()).

                       This is described in RFC 5246, section 7.4.7.1, page 58-59. */

                       /* Generate premaster secret using random numbers. */
                    for (i = 0; i < NX_SECURE_TLS_RSA_PREMASTER_SIZE; ++i)
                    {

                        /* PKCS#1 padding must be random, but CANNOT be 0. */
                        do
                        {
                            rand_byte = (UCHAR)NX_RAND();
                        } while (rand_byte == 0);
                        tls_key_material -> nx_secure_tls_pre_master_secret[i] = rand_byte;
                    }
                }
                else
                {

                    /* Extract the 48 bytes of the actual pre-master secret from the data we just decrypted, stripping the padding, which
                       comes at the beginning of the decrypted block (the pre-master secret is the last 48 bytes. */
                    NX_SECURE_MEMCPY(tls_key_material -> nx_secure_tls_pre_master_secret,
                                     &_nx_secure_client_padded_pre_master[length - NX_SECURE_TLS_RSA_PREMASTER_SIZE], NX_SECURE_TLS_RSA_PREMASTER_SIZE); /* Use case of memcpy is verified. */
                }
                tls_key_material -> nx_secure_tls_pre_master_secret_size = NX_SECURE_TLS_RSA_PREMASTER_SIZE;
            }   /* End RSA-specific section. */
            else
            {
                /* Unknown or invalid public cipher. */
                return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
            }
        }
    }
#ifdef NX_SECURE_KEY_CLEAR
    NX_SECURE_MEMSET(_nx_secure_client_padded_pre_master, 0, sizeof(_nx_secure_client_padded_pre_master));
#endif /* NX_SECURE_KEY_CLEAR  */

    return(NX_SUCCESS);
}

