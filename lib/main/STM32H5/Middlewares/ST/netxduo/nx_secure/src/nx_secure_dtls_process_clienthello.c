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
/**    Datagram Transport Layer Security (DTLS)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_dtls.h"

#ifdef NX_SECURE_ENABLE_DTLS
#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
extern NX_CRYPTO_METHOD crypto_method_ec_secp256;
#endif /* NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE */

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
static UINT _nx_secure_dtls_check_ciphersuite(const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite_info,
                                              NX_SECURE_X509_CERT *cert, UINT selected_curve,
                                              UINT cert_curve_supported);
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_process_clienthello                 PORTABLE C      */
/*                                                           6.1.3        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ClientHello message from a      */
/*    remote host, kicking off a DTLS handshake.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS control block            */
/*    packet_buffer                         Pointer to message data       */
/*    message_length                        Length of message data (bytes)*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_check_protocol_version Check protocol version        */
/*    _nx_secure_tls_ciphersuite_lookup     Get cipher information        */
/*    _nx_secure_tls_proc_clienthello_sec_sa_extension                    */
/*                                          Process ECC extensions        */
/*    _nx_secure_dtls_check_ciphersuite     Check if ECC suite is usable  */
/*    _nx_secure_x509_local_device_certificate_get                        */
/*                                          Get the local certificate     */
/*    [nx_crypto_init]                      Initialize crypto             */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), added    */
/*                                            priority ciphersuite logic, */
/*                                            verified memcpy use cases,  */
/*                                            fixed configuration problem */
/*                                            that would result in        */
/*                                            compiler error, fixed       */
/*                                            renegotiation bug,          */
/*                                            improved negotiation logic, */
/*                                            resulting in version 6.1    */
/*  12-31-2020     Timothy Stapko           Modified comment(s),          */
/*                                            improved buffer length      */
/*                                            verification,               */
/*                                            resulting in version 6.1.3  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_process_clienthello(NX_SECURE_DTLS_SESSION *dtls_session, UCHAR *packet_buffer,
                                         UINT message_length)
{
USHORT                                ciphersuite_list_length;
UCHAR                                 compression_methods_length;
USHORT                                cipher_entry;
UCHAR                                 session_id_length;
UINT                                  i;
UINT                                  status;
USHORT                                protocol_version;
UINT                                  total_extensions_length;
const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite_info = NX_NULL;
USHORT                                ciphersuite_priority;
USHORT                                new_ciphersuite_priority = 0;
NX_SECURE_TLS_SESSION                *tls_session;
NX_SECURE_TLS_HELLO_EXTENSION         extension_data[NX_SECURE_TLS_HELLO_EXTENSIONS_MAX];
UINT                                  num_extensions = NX_SECURE_TLS_HELLO_EXTENSIONS_MAX;
UCHAR                                *ciphersuite_list;
UCHAR                                *packet_buffer_start;
#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
INT                                   extension_total_length;
USHORT                                extension_length;
USHORT                                extension_type;
USHORT                                supported_ec_length;
UCHAR                                 ec_point_formats_length;
UCHAR                                 supported_ec_match, ec_point_formats_match, zkp_verified;
const NX_CRYPTO_METHOD               *crypto_method;
NX_SECURE_TLS_PSK_STORE              *psk_store;
#endif /* NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE */
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
NX_SECURE_X509_CERT                  *cert;
UINT                                  selected_curve;
UINT                                  cert_curve;
UINT                                  cert_curve_supported;
NX_SECURE_EC_PUBLIC_KEY              *ec_pubkey;
NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA   *ecdhe_data;
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */


    /* Structure of ClientHello:
     * |     2       |          4 + 28          |    1       |   <SID len>  |   2    | <CS Len>     |    1    | <Comp Len>  |    2    | <Ext. Len> |
     * | TLS version |  Random (time + random)  | SID length |  Session ID  | CS Len | Ciphersuites |Comp Len | Compression |Ext. Len | Extensions |
     */

    if (message_length < 38)
    {
        /* Message was not the minimum required size for a ClientHello. */
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Get a pointer to the start of our packet buffer so we can read ahead later. */
    packet_buffer_start = packet_buffer;

    /* Get a reference to TLS state. */
    tls_session = &dtls_session -> nx_secure_dtls_tls_session;

    /* If we are currently in a session, we have a renegotiation handshake. */
    if (tls_session -> nx_secure_tls_local_session_active)
    {

        /* Session renegotiation is disabled, send a "no_renegotiation" alert! */
        return(NX_SECURE_TLS_NO_RENEGOTIATION_ERROR);
    }

    /* Client is establishing a TLS session with our server. */
    /* Extract the protocol version - only part of the ClientHello message. */
    protocol_version = (USHORT)(((USHORT)packet_buffer[0] << 8) | packet_buffer[1]);
    packet_buffer += 2;


    /* Check protocol version provided by client. */
    status = _nx_secure_tls_check_protocol_version(tls_session, protocol_version, NX_SECURE_DTLS);

    if (status != NX_SECURE_TLS_SUCCESS)
    {
        /* If we have an active session, this is a renegotiation attempt, treat the protocol error as
           if we are starting a new session. */
        if (status == NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION || tls_session -> nx_secure_tls_local_session_active)
        {
            /* If the version isn't supported, it's not an issue - TLS is backward-compatible,
             * so pick the highest version we do support. If the version isn't recognized,
             * flag an error. */
            _nx_secure_tls_highest_supported_version_negotiate(tls_session, &protocol_version, NX_SECURE_DTLS);

            if (protocol_version == 0x0)
            {
                /* Error, no versions enabled. */
                return(NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION);
            }
        }
        else
        {
            /* Protocol version unknown (not TLS or SSL!), return status. */
            return(status);
        }
    }

    /* Assign our protocol version to our socket. This is used for all further communications
     * in this session. */
    tls_session -> nx_secure_tls_protocol_version = protocol_version;

    /* Save off the random value for key generation later. */
    NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random, packet_buffer, NX_SECURE_TLS_RANDOM_SIZE); /* Use case of memcpy is verified. */
    packet_buffer += NX_SECURE_TLS_RANDOM_SIZE;

    /* Extract the session ID if there is one. */
    session_id_length = packet_buffer[0];
    packet_buffer++;

    /* If there is a session ID, copy it into our TLS socket structure. */
    tls_session -> nx_secure_tls_session_id_length = session_id_length;
    if (session_id_length > 0)
    {
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_session_id, &packet_buffer[0], session_id_length); /* Use case of memcpy is verified. */
        packet_buffer += session_id_length;
    }

    /* DTLS - there might be a cookie we need to save. */

    /* Extract the cookie from the client. */
    dtls_session -> nx_secure_dtls_cookie_length = packet_buffer[0];
    packet_buffer += 1;

    if (dtls_session -> nx_secure_dtls_cookie_length > sizeof(dtls_session -> nx_secure_dtls_cookie))
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    if (dtls_session -> nx_secure_dtls_cookie_length > 0)
    {
        NX_SECURE_MEMCPY(dtls_session -> nx_secure_dtls_cookie, packet_buffer, dtls_session -> nx_secure_dtls_cookie_length); /* Use case of memcpy is verified. */
        packet_buffer += dtls_session -> nx_secure_dtls_cookie_length;
    }

    /* Negotiate the ciphersuite we want to use. */
    ciphersuite_list_length = (USHORT)((packet_buffer[0] << 8) + packet_buffer[1]);
    packet_buffer += 2;

    /* Make sure the list length makes sense. */
    if (ciphersuite_list_length > message_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    ciphersuite_list = packet_buffer;
    packet_buffer += ciphersuite_list_length;

    /* Compression methods length - one byte. For now we only support the NULL method. */
    compression_methods_length = packet_buffer[0];
    packet_buffer += 1;

    /* Message length overflow. */
    if (((UINT)(packet_buffer - packet_buffer_start + compression_methods_length)) > message_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Make sure NULL compression method is supported. */
    status = NX_SECURE_TLS_BAD_COMPRESSION_METHOD;
    for (i = 0; i < compression_methods_length; ++i)
    {
        if (packet_buffer[i] == 0x0)
        {
            status = NX_SUCCESS;
            break;
        }
    }

    /* Check for error. */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    packet_buffer += compression_methods_length;

    /* Padding data? */
    if (message_length >= (UINT)(packet_buffer - packet_buffer_start + 2))
    {

        /* TLS Extensions come next. Get the total length of all extensions first. */
        total_extensions_length = (USHORT)((packet_buffer[0] << 8) + packet_buffer[1]);

        /* Message length overflow. */
        if (((UINT)(packet_buffer - packet_buffer_start + 2) + total_extensions_length) > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        if (total_extensions_length > 0)
        {
            /* Process serverhello extensions. */
            status = _nx_secure_tls_process_clienthello_extensions(tls_session, &packet_buffer[2], total_extensions_length, extension_data, &num_extensions, packet_buffer, message_length);

            /* Check for error. */
            if (status != NX_SUCCESS)
            {
                return(status);
            }

            /* If the server callback is set, invoke it now with the extensions that require application input. */
            if (tls_session -> nx_secure_tls_session_server_callback != NX_NULL)
            {
                status = tls_session -> nx_secure_tls_session_server_callback(tls_session, extension_data, num_extensions);

                /* Check for error. */
                if (status != NX_SUCCESS)
                {
                    return(status);
                }
            }
        }
    }

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE

    /* Get the local certificate. */
    if (tls_session -> nx_secure_tls_credentials.nx_secure_tls_active_certificate != NX_NULL)
    {
        cert = tls_session -> nx_secure_tls_credentials.nx_secure_tls_active_certificate;
    }
    else
    {
        /* Get reference to local device certificate. NX_NULL is passed for name to get default entry. */
        status = _nx_secure_x509_local_device_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                              NX_NULL, &cert);
        if (status != NX_SUCCESS)
        {
            cert = NX_NULL;
        }
    }

    if (cert != NX_NULL && cert -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_EC)
    {
        ec_pubkey = &cert -> nx_secure_x509_public_key.ec_public_key;
        cert_curve = ec_pubkey -> nx_secure_ec_named_curve;
    }
    else
    {
        cert_curve = 0;
    }

    ecdhe_data = (NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA *)tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data;

    /* Parse the ECC extension of supported curve. */
    status = _nx_secure_tls_proc_clienthello_sec_sa_extension(tls_session,
                                                              extension_data,
                                                              num_extensions,
                                                              &selected_curve,
                                                              (USHORT)cert_curve, &cert_curve_supported,
                                                              &ecdhe_data -> nx_secure_tls_ecdhe_signature_algorithm,
                                                              cert);

    ecdhe_data -> nx_secure_tls_ecdhe_named_curve = selected_curve;

    /* Select signature algorithm by certificate type. */
    if (cert != NX_NULL && cert -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_EC)
    {
        ecdhe_data -> nx_secure_tls_ecdhe_signature_algorithm =
            (USHORT)((ecdhe_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF00) | NX_SECURE_TLS_SIGNATURE_ALGORITHM_ECDSA);

        if (cert_curve_supported == NX_FALSE)
        {
            /* The named curve in our server certificate is not supported by the client. */
            return(NX_SECURE_TLS_NO_SUPPORTED_CIPHERS);
        }
    }
    else if (cert != NX_NULL && cert -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_RSA)
    {
        ecdhe_data -> nx_secure_tls_ecdhe_signature_algorithm =
            (USHORT)((ecdhe_data -> nx_secure_tls_ecdhe_signature_algorithm & 0xFF00) | NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA);
    }
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

    /* Set our initial priority to the maximum value - the size of our ciphersuite crypto table. */
    ciphersuite_priority = (USHORT)(0xFFFFFFFF); 

    for (i = 0; i < ciphersuite_list_length; i += 2)
    {
        /* Loop through list of acceptable ciphersuites. */
        cipher_entry = (USHORT)((ciphersuite_list[i] << 8) + ciphersuite_list[i + 1]);

        status = _nx_secure_tls_ciphersuite_lookup(tls_session, cipher_entry, &ciphersuite_info, &new_ciphersuite_priority);

        if (status == NX_SUCCESS && (new_ciphersuite_priority < ciphersuite_priority))
        {
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
            if (NX_SUCCESS == _nx_secure_dtls_check_ciphersuite(ciphersuite_info, cert, selected_curve, cert_curve_supported))
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
            {
                tls_session -> nx_secure_tls_session_ciphersuite = ciphersuite_info;
                ciphersuite_priority = new_ciphersuite_priority;
            }
        }
    }

    /* See if we found an acceptable ciphersuite. */
    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* No supported ciphersuites found. */
        return(NX_SECURE_TLS_NO_SUPPORTED_CIPHERS);
    }

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE

    /* Check if there are extensions.  */
    if ((ciphersuite_info -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE) &&
        (dtls_session -> nx_secure_dtls_cookie_length != 0))
    {

        if (packet_buffer - packet_buffer_start > (INT)message_length - 2)
        {
            return(NX_SECURE_TLS_HANDSHAKE_FAILURE);
        }

        extension_total_length = (packet_buffer[0] << 8) + packet_buffer[1];
        packet_buffer += 2;

        /* Make sure the extension length make sense.  */
        if (packet_buffer - packet_buffer_start + extension_total_length > (INT)message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        supported_ec_match = 0;
        ec_point_formats_match = 0;
        zkp_verified = 0;

        crypto_method = ciphersuite_info -> nx_secure_tls_public_auth;
        if (crypto_method == NX_NULL)
        {
            return(NX_SECURE_TLS_HANDSHAKE_FAILURE);
        }

        if (crypto_method -> nx_crypto_init == NX_NULL)
        {
            return(NX_SECURE_TLS_HANDSHAKE_FAILURE);
        }

        if (tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_count == 0)
        {
            return(NX_SECURE_TLS_HANDSHAKE_FAILURE);
        }

        psk_store = &tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[0];
        status = crypto_method -> nx_crypto_init((NX_CRYPTO_METHOD*)crypto_method,
                                                 psk_store -> nx_secure_tls_psk_data,
                                                 (USHORT)(psk_store -> nx_secure_tls_psk_data_size << 3),
                                                 NX_NULL,
                                                 tls_session -> nx_secure_public_auth_metadata_area,
                                                 tls_session -> nx_secure_public_auth_metadata_size);

        if (status)
        {
            return(status);
        }

        status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_HASH_METHOD_SET,
                                                      NX_NULL,
                                                      (NX_CRYPTO_METHOD*)crypto_method,
                                                      NX_NULL,
                                                      (USHORT)(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size << 3),
                                                      (UCHAR *)(tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_handshake_hash_sha256_method),
                                                      sizeof(NX_CRYPTO_METHOD),
                                                      NX_NULL, NX_NULL, 0,
                                                      tls_session -> nx_secure_public_auth_metadata_area,
                                                      tls_session -> nx_secure_public_auth_metadata_size,
                                                      NX_NULL, NX_NULL);
        if (status)
        {
            return(status);
        }

        status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_CURVE_SET,
                                                      NX_NULL,
                                                      (NX_CRYPTO_METHOD*)crypto_method,
                                                      NX_NULL, 0,
                                                      (UCHAR *)&crypto_method_ec_secp256,
                                                      sizeof(NX_CRYPTO_METHOD),
                                                      NX_NULL, NX_NULL, 0,
                                                      tls_session -> nx_secure_public_auth_metadata_area,
                                                      tls_session -> nx_secure_public_auth_metadata_size,
                                                      NX_NULL, NX_NULL);
        if (status)
        {
            return(status);
        }

        /* Loop through all the extensions.  */
        while (extension_total_length > 0)
        {
            /* Get Extension Type.  */
            extension_type = (USHORT)((packet_buffer[0] << 8) + packet_buffer[1]);
            packet_buffer += 2;

            /* Get Extension Length.  */
            extension_length = (USHORT)((packet_buffer[0] << 8) + packet_buffer[1]);
            packet_buffer += 2;

            extension_total_length -= 4;

            /* Make sure Extension Length is within the total extension length.  */
            if (extension_length > extension_total_length)
            {
                return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
            }

            extension_total_length -= extension_length;

            switch (extension_type)
            {
            case NX_SECURE_TLS_EXTENSION_EC_GROUPS:
                /* Supported Elliptic Curves Extension.  */
                supported_ec_length = (USHORT)((packet_buffer[0] << 8) + packet_buffer[1]);
                packet_buffer += 2;

                /* Make sure secp256r1 (23) format is supported. */
                status = NX_SECURE_TLS_UNSUPPORTED_CIPHER;
                for (i = 0; i < (UINT)(supported_ec_length >> 1); ++i)
                {
                    if (packet_buffer[i * 2] == 0x0 && packet_buffer[i * 2 + 1] == 0x17)
                    {
                        status = NX_SUCCESS;
                        break;
                    }
                }

                /* Check for error. */
                if (status != NX_SUCCESS)
                {
                    return(status);
                }

                packet_buffer += supported_ec_length;
                supported_ec_match = 1;
                break;

            case NX_SECURE_TLS_EXTENSION_EC_POINT_FORMATS:
                /* ec_point_formats Extension.  */
                ec_point_formats_length = packet_buffer[0];
                packet_buffer += 1;

                /* Make sure uncompressed (0) format is supported. */
                status = NX_SECURE_TLS_UNSUPPORTED_CIPHER;
                for (i = 0; i < ec_point_formats_length; ++i)
                {
                    if (packet_buffer[i] == 0x0)
                    {
                        status = NX_SUCCESS;
                        break;
                    }
                }

                /* Check for error. */
                if (status != NX_SUCCESS)
                {
                    return(status);
                }

                packet_buffer += ec_point_formats_length;
                ec_point_formats_match = 1;
                break;

            case NX_SECURE_TLS_EXTENSION_ECJPAKE_KEY_KP_PAIR:
                /* ecjpake_key_kp_pair Extension.  */
                status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_CLIENT_HELLO_PROCESS,
                                                              NX_NULL,
                                                              (NX_CRYPTO_METHOD*)crypto_method,
                                                              NX_NULL, 0,
                                                              packet_buffer, extension_length,
                                                              NX_NULL, NX_NULL, 0,
                                                              tls_session -> nx_secure_public_auth_metadata_area,
                                                              tls_session -> nx_secure_public_auth_metadata_size,
                                                              NX_NULL, NX_NULL);
                if (status)
                {
                    return(status);
                }

                packet_buffer += extension_length;

                zkp_verified = 1;
                break;
            }
        }

        /* Make sure no extension is missing.  */
        if (supported_ec_match == 0 || ec_point_formats_match == 0 || zkp_verified == 0)
        {
            return(NX_SECURE_TLS_UNSUPPORTED_ECC_FORMAT);
        }
    }
#endif

#ifdef NX_SECURE_TLS_SERVER_DISABLED
    /* If TLS Server is disabled and we have processed a ClientHello, something is wrong... */
    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_ERROR;
    return(NX_SECURE_TLS_INVALID_STATE);
#else
    if (dtls_session -> nx_secure_dtls_cookie_length == 0)
    {
        /* In DTLS, we actually send a HelloVerifyReqeust (expecting a second ClientHello in
         * response) before sending our ServerHello. */
        dtls_session -> nx_secure_dtls_tls_session.nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_SEND_HELLO_VERIFY;
    }
    else
    {
        dtls_session -> nx_secure_dtls_tls_session.nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_SEND_HELLO;
    }
    return(NX_SUCCESS);
#endif
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_check_ciphersuite                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks whether the specified ciphersuite is           */
/*    suitable for the server certificate, the curve in the certificate   */
/*    and the common shared curve.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ciphersuite_info                      The specified cipher suite    */
/*    cert                                  Local server certificate      */
/*    selected_curve                        Curve selected for ECC        */
/*    cert_curve_supported                  If cert curve is supported    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_process_clienthello   Process ClientHello           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
static UINT _nx_secure_dtls_check_ciphersuite(const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite_info,
                                              NX_SECURE_X509_CERT *cert, UINT selected_curve,
                                              UINT cert_curve_supported)
{
    if (ciphersuite_info -> nx_secure_tls_public_cipher == NX_NULL)
    {
        return(NX_SUCCESS);
    }

    if (ciphersuite_info -> nx_secure_tls_public_auth == NX_NULL)
    {
        return(NX_SUCCESS);
    }

    switch (ciphersuite_info -> nx_secure_tls_public_cipher -> nx_crypto_algorithm)
    {
    case NX_CRYPTO_KEY_EXCHANGE_ECDHE:
        if (selected_curve == NX_NULL || cert == NX_NULL)
        {
            /* No common named curve supported for ECDHE. */
            return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
        }

        if (ciphersuite_info -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA)
        {
            if (cert -> nx_secure_x509_public_algorithm != NX_SECURE_TLS_X509_TYPE_EC)
            {
                /* ECDSA auth requires EC certificate. */
                return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
            }
        }
        else
        {
            if (cert -> nx_secure_x509_public_algorithm != NX_SECURE_TLS_X509_TYPE_RSA)
            {
                /* RSA auth requires RSA certificate. */
                return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
            }
        }

        break;

    case NX_CRYPTO_KEY_EXCHANGE_ECDH:
        /* Check for ECDH_anon. */
        if (ciphersuite_info -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATURE_ANONYMOUS)
        {
            if (selected_curve == NX_NULL)
            {
                /* No common named curve supported supported. */
                return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
            }
        }
        else
        {
            /* ECDH key exchange requires an EC certificate. */
            if (cert == NX_NULL || cert -> nx_secure_x509_public_algorithm != NX_SECURE_TLS_X509_TYPE_EC)
            {
                return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
            }

            if (cert_curve_supported == NX_FALSE)
            {
                return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
            }

            /* Check the signatureAlgorithm of the certificate to determine the public auth algorithm. */
            if (ciphersuite_info -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_DIGITAL_SIGNATURE_ECDSA)
            {
                if (cert -> nx_secure_x509_signature_algorithm != NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_1 &&
                    cert -> nx_secure_x509_signature_algorithm != NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_224 &&
                    cert -> nx_secure_x509_signature_algorithm != NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_256 &&
                    cert -> nx_secure_x509_signature_algorithm != NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_384 &&
                    cert -> nx_secure_x509_signature_algorithm != NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_512)
                {
                    return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
                }
            }
            else
            {
                if (cert -> nx_secure_x509_signature_algorithm != NX_SECURE_TLS_X509_TYPE_RSA_MD5 &&
                    cert -> nx_secure_x509_signature_algorithm != NX_SECURE_TLS_X509_TYPE_RSA_SHA_1 &&
                    cert -> nx_secure_x509_signature_algorithm != NX_SECURE_TLS_X509_TYPE_RSA_SHA_256 &&
                    cert -> nx_secure_x509_signature_algorithm != NX_SECURE_TLS_X509_TYPE_RSA_SHA_384 &&
                    cert -> nx_secure_x509_signature_algorithm != NX_SECURE_TLS_X509_TYPE_RSA_SHA_512)
                {
                    return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
                }
            }
        }
        break;

    case NX_CRYPTO_KEY_EXCHANGE_RSA:
        if (cert == NX_NULL || cert -> nx_secure_x509_public_algorithm != NX_SECURE_TLS_X509_TYPE_RSA)
        {
            /* RSA key exchange requires RSA certificate. */
            return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
        }
        break;

    default:
        break;
    }

    return(NX_SUCCESS);
}
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

#endif /* NX_SECURE_ENABLE_DTLS */

