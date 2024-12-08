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
/*    _nx_secure_tls_send_certificate_request             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function populates an NX_PACKET with a CertificateRequest      */
/*    message which is used to indicate to the remote client that we wish */
/*    to do a Client certificate verification. The Client should respond  */
/*    with an appropriate certificate and CertificateVerify message.      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Packet to be filled           */
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
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
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
UINT _nx_secure_tls_send_certificate_request(NX_SECURE_TLS_SESSION *tls_session,
                                             NX_PACKET *send_packet)
{
UINT   length;
UCHAR *packet_buffer;
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
UINT   ext_len_pos = 0;
UINT   sighash_len = 0;
UINT   i;
UINT   sign_alg;
#endif

    /* Structure:
     * |       1            |    <Cert types count>    |             2              |  <Sig algs length>        |
     * |  Cert types count  | Cert types (1 byte each) | Sig Hash algorithms length | Algorithms (2 bytes each) |
     */

    if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) < 11u)
    {

        /* Packet buffer is too small. */
        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
    }

    packet_buffer = send_packet -> nx_packet_append_ptr;

    /* Use our length as an index into the buffer. */
    length = 0;

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

    if (tls_session -> nx_secure_tls_1_3)
    {

        /* Put context here.  */
        packet_buffer[length] = 0;
        length++;

        /* Skip the extensions length.  */
        ext_len_pos = length;
        length += 2;

        /* Add signature_algorithms extension.  */
        packet_buffer[length] = (UCHAR)(NX_SECURE_TLS_EXTENSION_SIGNATURE_ALGORITHMS >> 8);
        packet_buffer[length + 1] = (UCHAR)(NX_SECURE_TLS_EXTENSION_SIGNATURE_ALGORITHMS);
        length += 2;

        /* Skip signature_algorithm length.  */
        length += 2;
    }
    else
#endif
    {
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
        if (tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups_count != 0)
        {
            /* If ECC is initialized, we support RSA and ECDSA signatures. */
            packet_buffer[length] = 0x2;
            length += 1;

            packet_buffer[length] = NX_SECURE_TLS_CERT_TYPE_RSA_SIGN;
            length += 1;

            packet_buffer[length] = NX_SECURE_TLS_CERT_TYPE_ECDSA_SIGN;
            length += 1;
        }
        else
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
        {

            /* ECC is not initialized, we only support RSA signatures for now... */
            packet_buffer[length] = 0x1;
            length += 1;

            packet_buffer[length] = NX_SECURE_TLS_CERT_TYPE_RSA_SIGN;
            length += 1;
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
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
        if (tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups_count != 0)
        {

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
            if (tls_session -> nx_secure_tls_1_3)
            {
                /* Skip length field.  */
                length = length + 2;

                /* Add supported signature algorithms.  */
                for (i = 0; i < tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups_count; i++)
                {
                    switch (tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups[i])
                    {
                    case (NX_CRYPTO_EC_SECP256R1 & 0x00FF):
                        sign_alg = NX_SECURE_TLS_SIGNATURE_ECDSA_SHA256;
                        break;
                    case (NX_CRYPTO_EC_SECP384R1 & 0x00FF):
                        sign_alg = NX_SECURE_TLS_SIGNATURE_ECDSA_SHA384;
                        break;
                    case (NX_CRYPTO_EC_SECP521R1 & 0x00FF):
                        sign_alg = NX_SECURE_TLS_SIGNATURE_ECDSA_SHA512;
                        break;
                    default:
                        continue;
                    }

                    if (((ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr)) < (length + 2u))
                    {

                        /* Packet buffer is too small. */
                        return(NX_SECURE_TLS_PACKET_BUFFER_TOO_SMALL);
                    }

                    packet_buffer[length] = (UCHAR)(sign_alg >> 8);
                    packet_buffer[length + 1] = (UCHAR)(sign_alg);
                    length = length + 2;
                    sighash_len = sighash_len + 2;
                }
            }
            else
#endif
            {

                /* ECC is initialized. */
                /* Length of supported signature algorithms - each entry is 2 bytes so length
                   is number of entries * 2. */
                packet_buffer[length] = (UCHAR)0x0;
                packet_buffer[length + 1] = (UCHAR)0x4;
                length = length + 2;

                /* Extract the signature algorithms. */
                packet_buffer[length] = (UCHAR)(NX_SECURE_TLS_SIGNATURE_RSA_SHA256 >> 8);
                packet_buffer[length + 1] = (UCHAR)(NX_SECURE_TLS_SIGNATURE_RSA_SHA256);
                length = length + 2;

                packet_buffer[length] =     (UCHAR)(NX_SECURE_TLS_SIGNATURE_ECDSA_SHA256 >> 8);
                packet_buffer[length + 1] = (UCHAR)(NX_SECURE_TLS_SIGNATURE_ECDSA_SHA256);
                length = length + 2;
            }
        }
        else
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
        {
            /* Length of supported signature algorithms - each entry is 2 bytes so length
               is number of entries * 2. */
            /* Note: We only support 1 one signature algorithm for now... */
            packet_buffer[length]  = (UCHAR)0x0;
            packet_buffer[length + 1] = (UCHAR)0x2;
            length = length + 2;

            /* Extract the signature algorithms. */
            packet_buffer[length] =     (UCHAR)(NX_SECURE_TLS_SIGNATURE_RSA_SHA256 >> 8);
            packet_buffer[length + 1] = (UCHAR)(NX_SECURE_TLS_SIGNATURE_RSA_SHA256);
            length = length + 2;
        }

    }
#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if (tls_session -> nx_secure_tls_1_3)
    {

        /* Set extensions length and signature algorithm length.  */
        packet_buffer[ext_len_pos] = (UCHAR)((sighash_len + 6) >> 8);
        packet_buffer[ext_len_pos + 1] = (UCHAR)(sighash_len + 6);
        packet_buffer[ext_len_pos + 4] = (UCHAR)((sighash_len + 2) >> 8);
        packet_buffer[ext_len_pos + 5] = (UCHAR)(sighash_len + 2);
        packet_buffer[ext_len_pos + 6] = (UCHAR)(sighash_len >> 8);
        packet_buffer[ext_len_pos + 7] = (UCHAR)(sighash_len);
    }
    else
#endif
    {

        /* Distinguished names length - right now we do not support this. */
        packet_buffer[length] = (UCHAR)0x0;
        packet_buffer[length + 1] = (UCHAR)0x0;
        length = length + 2;
    }

    /* The remainder of the message is the Certificate Authorities list. It can be used
       to select a certificate in a particular authorization chain.  */

    /* Let the caller know how many bytes we wrote. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + (length);
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + (length);

    return(NX_SUCCESS);
}

