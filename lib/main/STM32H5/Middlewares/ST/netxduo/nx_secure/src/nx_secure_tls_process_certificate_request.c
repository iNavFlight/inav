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
#include "nx_secure_x509.h"
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls.h"
#endif /* NX_SECURE_ENABLE_DTLS */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_certificate_request          PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming TLS Certificate request         */
/*    message, typically sent by a TLS Server to request a client         */
/*    certificate for authentication.                                     */
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
/*    _nx_secure_x509_local_device_certificate_get                        */
/*                                          Get the local certificate     */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_certificate_request(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, UINT message_length)
{
UINT  length;
UINT  cert_types_length;
UCHAR cert_type;
UINT  i;
NX_SECURE_X509_CERT *local_certificate = NX_NULL;
UCHAR expected_cert_type = 0;
UINT  status;

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
UINT sign_algs_length;
UINT sign_alg;
UINT expected_sign_alg = 0;
#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT extension_total_length;
UINT extension_length;
UINT extension_type;
#endif

    /* Structure:
     * |       1            |    <Cert types count>    |             2              |  <Sig algs length>        |
     * |  Cert types count  | Cert types (1 byte each) | Sig Hash algorithms length | Algorithms (2 bytes each) |
     */

    /* Use our length as an index into the buffer. */
    length = 0;

    /* Get the local certificate. */
    if (tls_session -> nx_secure_tls_credentials.nx_secure_tls_active_certificate != NX_NULL)
    {
        local_certificate = tls_session -> nx_secure_tls_credentials.nx_secure_tls_active_certificate;
    }
    else
    {
        /* Get reference to local device certificate. NX_NULL is passed for name to get default entry. */
        status = _nx_secure_x509_local_device_certificate_get(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                                NX_NULL, &local_certificate);
        if (status != NX_SUCCESS)
        {
            local_certificate = NX_NULL;
        }
    }

    if (local_certificate != NX_NULL)
    {
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        if (tls_session -> nx_secure_tls_1_3)
        {
            tls_session -> nx_secure_tls_signature_algorithm = 0;

            /* TLS1.3 uses RSASSA-PSS instead of RSASSA-PKCS. RSASSA-PSS is not supported now. */
            if (local_certificate -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_RSA)
            {
                return(NX_SECURE_TLS_UNSUPPORTED_CERT_SIGN_TYPE);
            }

            /* In TLS 1.3, the signing curve is constrained.  */
            switch (local_certificate -> nx_secure_x509_private_key.ec_private_key.nx_secure_ec_named_curve)
            {
            case NX_CRYPTO_EC_SECP256R1:
                expected_sign_alg = NX_SECURE_TLS_SIGNATURE_ECDSA_SHA256;
                break;
            case NX_CRYPTO_EC_SECP384R1:
                expected_sign_alg = NX_SECURE_TLS_SIGNATURE_ECDSA_SHA384;
                break;
            case NX_CRYPTO_EC_SECP521R1:
                expected_sign_alg = NX_SECURE_TLS_SIGNATURE_ECDSA_SHA512;
                break;
            default:
                return(NX_SECURE_TLS_UNSUPPORTED_CERT_SIGN_ALG);
            }
        }
        else
#endif
        {
            if (local_certificate -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_RSA)
            {
                expected_cert_type = NX_SECURE_TLS_CERT_TYPE_RSA_SIGN;
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
                expected_sign_alg = NX_SECURE_TLS_SIGNATURE_RSA_SHA256;
#endif
            }
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
            else if (local_certificate -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_EC)
            {
                expected_cert_type = NX_SECURE_TLS_CERT_TYPE_ECDSA_SIGN;
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
                expected_sign_alg = NX_SECURE_TLS_SIGNATURE_ECDSA_SHA256;
#endif
            }
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
        }
    }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    /*
      struct {
          opaque certificate_request_context<0..2^8-1>;
          Extension extensions<2..2^16-1>;
      } CertificateRequest;
    */
    if (tls_session -> nx_secure_tls_1_3)
    {

        if (message_length < 1 || (UINT)packet_buffer[0] + 3 > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Skip certificate request context.  */
        length = length + 1 + packet_buffer[length];

        extension_total_length = (UINT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
        length += 2;

        /* Make sure what we extracted makes sense. */
        if ((length + extension_total_length) > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Find SignatureAlgorithms extension.  */
        /* Note: Other extensions will be processed in the future.  */
        while (length < message_length)
        {
            extension_type = (UINT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
            length += 2;

            extension_length = (UINT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
            length += 2;

            if (extension_type == NX_SECURE_TLS_EXTENSION_SIGNATURE_ALGORITHMS)
            {
                break;
            }

            length += extension_length;
        }

        if (length >= message_length)
        {
            return(NX_SECURE_TLS_UNSUPPORTED_CERT_SIGN_ALG);
        }
    }
    else
#endif
    {

        /* Extract the count of certificate types from the incoming data. */
        cert_types_length = packet_buffer[length];
        length += 1;

        /* Make sure what we extracted makes sense. */
        if (cert_types_length + 1 > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }


        cert_type = NX_SECURE_TLS_CERT_TYPE_NONE;
        for (i = 0; i < cert_types_length; ++i)
        {
            if (packet_buffer[length] == expected_cert_type)
            {
                /* We found a type we support. */
                cert_type = packet_buffer[length];
            }
            length += 1;
        }

        /* Make sure our certificate type is one we support. */
        if (cert_type != expected_cert_type)
        {
            return(NX_SECURE_TLS_UNSUPPORTED_CERT_SIGN_TYPE);
        }
    }

#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
    /* TLS 1.2 CertificateRequest contains a list of signature algorithms that
       is not included in earlier TLS versions. */
#ifdef NX_SECURE_ENABLE_DTLS
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_2)
#else
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
#endif /* NX_SECURE_ENABLE_DTLS */
    {
        if (length + 2 > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Extract the count of algorithms from the incoming data. */
        sign_algs_length = (UINT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
        length = length + 2;

        /* Make sure what we extracted makes sense. */
        if ((length + sign_algs_length) > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Extract the signature algorithms. */
        sign_alg = NX_SECURE_TLS_HASH_ALGORITHM_NONE;
        for (i = 0; i < sign_algs_length; i += 2)
        {
            /* Look for a type we support. */
            if ((UINT)((packet_buffer[length] << 8) + packet_buffer[length + 1]) == expected_sign_alg)
            {
                sign_alg = (UINT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
                break;
            }
            length = length + 2;
        }

        /* Make sure we are using the right signature algorithm! */
        if (sign_alg != expected_sign_alg)
        {
            return(NX_SECURE_TLS_UNSUPPORTED_CERT_SIGN_ALG);
        }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        if (tls_session -> nx_secure_tls_1_3)
        {
            tls_session -> nx_secure_tls_signature_algorithm = sign_alg;
        }
#endif
    }
#endif

    /* The remainder of the message is the Certificate Authorities list. It can be used
       to select a certificate in a particular authorization chain. In general, if there
       is only one device certificate we will send that one and if the server doesn't like
       it the connection will be dropped. */

#ifdef NX_SECURE_TLS_CLIENT_DISABLED
    /* If TLS Client is disabled and we have processed a CertificateRequest, something is wrong... */
    tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_ERROR;

    return(NX_SECURE_TLS_INVALID_STATE);
#else
    /* Set our state to indicate we received a certificate request. */
    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_CERTIFICATE_REQUEST;

    return(NX_SUCCESS);
#endif
}

