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

#ifndef NX_SECURE_TLS_CLIENT_DISABLED


#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
static UINT _nx_secure_tls_proc_serverhello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                USHORT *extension_length, UINT message_length);
#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
static UINT _nx_secure_tls_proc_serverhello_keyshare_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                USHORT *extension_length, UINT message_length);
#endif
static UINT _nx_secure_tls_proc_serverhello_supported_versions_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                         UCHAR *packet_buffer,
                                                                         USHORT *supported_version,
                                                                         USHORT *extension_length, UINT message_length);
#endif

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
static UINT _nx_secure_tls_proc_serverhello_ecc_point_formats(NX_SECURE_TLS_SESSION *tls_session,
                                                              UCHAR *packet_buffer,
                                                              USHORT *extension_length, UINT message_length);
static UINT _nx_secure_tls_proc_serverhello_ecjpake_key_kp_pair(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                USHORT *extension_length, UINT message_length);
#endif

#endif /* NX_SECURE_TLS_CLIENT_DISABLED */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_serverhello_extensions       PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes any extensions included in an incoming      */
/*    ServerHello message from a remote host.                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Pointer to message data       */
/*    message_length                        Length of message data (bytes)*/
/*    extensions                            Extensions for output         */
/*    num_extensions                        Number of extensions          */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_proc_serverhello_ecc_point_formats                   */
/*                                          Process ServerHello ECC       */
/*                                            point formats extension     */
/*    _nx_secure_tls_proc_serverhello_ecjpake_key_kp_pair                 */
/*                                          Process ServerHello ECJPAKE   */
/*                                            key kp pair extension       */
/*    _nx_secure_tls_proc_serverhello_sec_reneg_extension                 */
/*                                          Process ServerHello           */
/*                                            Renegotiation extension     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_server_handshake       Process ServerHello           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Timothy Stapko           Modified comment(s), fixed    */
/*                                            TLS 1.3 compilation issue,  */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_serverhello_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                   UCHAR *packet_buffer, UINT message_length,
                                                   NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                   UINT *num_extensions)
{
#ifndef NX_SECURE_TLS_CLIENT_DISABLED

UINT status;

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
USHORT                                ec_point_formats_match, zkp_verified;
#endif
UINT                                  offset;
USHORT                                extension_id;
USHORT                                extension_length;
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
USHORT                                supported_version = tls_session -> nx_secure_tls_protocol_version;
#endif

#ifdef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
#ifndef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    NX_PARAMETER_NOT_USED(tls_session);
#endif
#endif

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    ec_point_formats_match = NX_FALSE;
    zkp_verified = NX_FALSE;
#endif

    offset = 0;
    status = NX_SUCCESS;
    *num_extensions = 0;

    /* Process extensions until we run out. */
    while (offset < message_length)
    {

        /* Make sure there are at least 4 bytes available so we can read extension_id and
           extension_length. */
        if (offset + 4 > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* See what the extension is. */
        extension_id = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);

        /* Skip type id of extentions. */
        offset += 2;

        /* Parse the extension. */
        switch (extension_id)
        {
#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
        case NX_SECURE_TLS_EXTENSION_SECURE_RENEGOTIATION:
            status = _nx_secure_tls_proc_serverhello_sec_reneg_extension(tls_session, &packet_buffer[offset], &extension_length, message_length - offset);

            if (status)
            {
                return(status);
            }
            break;
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
        case NX_SECURE_TLS_EXTENSION_KEY_SHARE:

            if (tls_session -> nx_secure_tls_1_3)
            {
                /* Process the TLS 1.3 key share extension. */
                status = _nx_secure_tls_proc_serverhello_keyshare_extension(tls_session, &packet_buffer[offset], &extension_length, message_length - offset);

                if (status)
                {
                    return(status);
                }
            }
            else
            {
                extension_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
                if (extension_length + offset + 2 > message_length)
                {
                    return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
                }
            }
            offset += 2;

            /* Ignore if not TLS 1.3. */
            break;
#endif
        case NX_SECURE_TLS_EXTENSION_SUPPORTED_VERSIONS:
            if (tls_session -> nx_secure_tls_1_3)
            {

                /* Process the TLS 1.3 supported_versions extension. */
                status = _nx_secure_tls_proc_serverhello_supported_versions_extension(tls_session, &packet_buffer[offset], &supported_version, &extension_length, message_length - offset);

                if (status)
                {
                    return(status);
                }
            }
            else
            {
                extension_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
                if (extension_length + offset + 2 > message_length)
                {
                    return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
                }
            }
            offset += 2;

            /* Ignore if not TLS 1.3. */
            break;
        case NX_SECURE_TLS_EXTENSION_COOKIE:
            extension_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
            offset += 2;

            if (extension_length + offset > message_length)
            {
                return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
            }

            /* Store the pointer of cookie. */
            /* Note: Cookie data is stored in ServerHello packet buffer. This buffer should not be released
               or overwrote before Cookie is copied to ClientHello. */
            if (tls_session -> nx_secure_tls_1_3)
            {
                if (extension_length < 2)
                {
                    return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
                }

                tls_session -> nx_secure_tls_cookie_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
                tls_session -> nx_secure_tls_cookie = &packet_buffer[offset + 2];

                if (tls_session -> nx_secure_tls_cookie_length + 2 > extension_length)
                {
                    return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
                }
            }

            break;
#endif
#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
        /* ECJPAKE ciphersuite extensions. */
        case NX_SECURE_TLS_EXTENSION_EC_POINT_FORMATS:
            status = _nx_secure_tls_proc_serverhello_ecc_point_formats(tls_session, &packet_buffer[offset], &extension_length, message_length - offset);
            if (status == NX_SUCCESS)
            {
                ec_point_formats_match = NX_TRUE;
            }
            break;
        case NX_SECURE_TLS_EXTENSION_ECJPAKE_KEY_KP_PAIR:
            status = _nx_secure_tls_proc_serverhello_ecjpake_key_kp_pair(tls_session, &packet_buffer[offset], &extension_length, message_length - offset);
            if (status == NX_SUCCESS)
            {
                zkp_verified = NX_TRUE;
            }
            break;
#else
        case NX_SECURE_TLS_EXTENSION_EC_GROUPS:
        case NX_SECURE_TLS_EXTENSION_EC_POINT_FORMATS:
        case NX_SECURE_TLS_EXTENSION_ECJPAKE_KEY_KP_PAIR:
#endif /* NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE */
        case NX_SECURE_TLS_EXTENSION_SERVER_NAME_INDICATION:
        case NX_SECURE_TLS_EXTENSION_MAX_FRAGMENT_LENGTH:
        case NX_SECURE_TLS_EXTENSION_CLIENT_CERTIFICATE_URL:
        case NX_SECURE_TLS_EXTENSION_TRUSTED_CA_INDICATION:
        case NX_SECURE_TLS_EXTENSION_CERTIFICATE_STATUS_REQUEST:
            /* These extensions require information to be passed to the application. Save off
               the extension data in our extensions array to pass along in the hello callback. */
            extension_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
            offset += 2;

            if (extension_length + offset > message_length)
            {
                return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
            }

            if (*num_extensions < NX_SECURE_TLS_HELLO_EXTENSIONS_MAX)
            {
                extensions[*num_extensions].nx_secure_tls_extension_id = extension_id;
                extensions[*num_extensions].nx_secure_tls_extension_data = &packet_buffer[offset];
                extensions[*num_extensions].nx_secure_tls_extension_data_length = extension_length;

                /* Added another extension to the array. */
                *num_extensions = *num_extensions + 1;
            }

            break;
        case NX_SECURE_TLS_EXTENSION_SIGNATURE_ALGORITHMS:
        case NX_SECURE_TLS_EXTENSION_TRUNCATED_HMAC:
        default:
            /* Unknown extension, just ignore - TLS supports multiple extensions and the default
               behavior is to ignore any extensions that we don't know. Assume the next two
               octets are the length field so we can continue processing. */
            extension_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
            offset += 2;

            if (extension_length + offset > message_length)
            {
                return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
            }

            break;
        }

        /* Adjust our offset with the length of the extension we just parsed. */
        offset += extension_length;
    }

#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
    /* Make sure no ECJPAKE extensions are missing.  */
    if ((tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE) &&
        (ec_point_formats_match == 0 || zkp_verified == 0))
    {
        return(NX_SECURE_TLS_UNSUPPORTED_ECC_FORMAT);
    }
#endif /* NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE */

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if (tls_session -> nx_secure_tls_1_3)
    {
        if (supported_version != NX_SECURE_TLS_VERSION_TLS_1_3)
        {

            /* Server negotiates a version of TLS prior to TLS 1.3. */
            if (tls_session -> nx_secure_tls_protocol_version_override == 0)
            {
                tls_session -> nx_secure_tls_1_3 = NX_FALSE;
#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
                tls_session -> nx_secure_tls_renegotation_enabled = NX_TRUE;
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */
            }
            else
            {

                /* Protocol version is overridden to TLS 1.3. */
                return(NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION);
            }
        }
        else
        {
            if (tls_session -> nx_secure_tls_protocol_version != NX_SECURE_TLS_VERSION_TLS_1_2)
            {

                /* Server negotiates TLS 1.3 must set the legacy version to TLS 1.2. */
                return(NX_SECURE_TLS_UNKNOWN_TLS_VERSION);
            }
        }
    }
#endif

    return(status);
#else
    /* If Client TLS is disabled and we recieve a server key exchange, error! */
    NX_PARAMETER_NOT_USED(tls_session);
    NX_PARAMETER_NOT_USED(packet_buffer);
    NX_PARAMETER_NOT_USED(message_length);
    NX_PARAMETER_NOT_USED(extensions);
    NX_PARAMETER_NOT_USED(num_extensions);

    return(NX_SECURE_TLS_UNEXPECTED_MESSAGE);
#endif
}


#ifndef NX_SECURE_TLS_CLIENT_DISABLED

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_serverhello_ecc_point_formats   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the ec_point_formats extension when ECC        */
/*    ciphersuites are being used.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    extension_length                      Length of extension data      */
/*    message_length                        Length of message data (bytes)*/
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
/*    _nx_secure_tls_process_serverhello_extensions                       */
/*                                          Process ServerHello extensions*/
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
#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
static UINT _nx_secure_tls_proc_serverhello_ecc_point_formats(NX_SECURE_TLS_SESSION *tls_session,
                                                              UCHAR *packet_buffer,
                                                              USHORT *extension_length, UINT message_length)
{
UINT                                  status;
UINT                                  i;
UCHAR                                 ec_point_formats_length;
UINT                                  offset;

    *extension_length = (USHORT)((*packet_buffer << 8) + packet_buffer[1] + 2);

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Skip the length field of this extension. */
    offset = 2;

    if (*extension_length > message_length || *extension_length < 3)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* ec_point_formats Extension.  */
    ec_point_formats_length = packet_buffer[offset];
    offset += 1;

    if (offset + ec_point_formats_length != *extension_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Ignore the extension if we are not using ECJPAKE. */
    if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
    {
        /* Make sure uncompressed (0) format is supported. */
        status = NX_SECURE_TLS_UNSUPPORTED_CIPHER;
        for (i = 0; i < ec_point_formats_length; ++i)
        {
            if (packet_buffer[offset + i] == 0x0)
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
    }

    return(NX_SUCCESS);
}
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_serverhello_ecjpake_key_kp_pair PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the ecjpake_key_kp_pair extension when ECC     */
/*    JPAKE ciphersuites are being used.                                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    extension_length                      Length of extension data      */
/*    message_length                        Length of message data (bytes)*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_serverhello_extensions                       */
/*                                          Process ServerHello extensions*/
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
#ifdef NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE
static UINT _nx_secure_tls_proc_serverhello_ecjpake_key_kp_pair(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                USHORT *extension_length, UINT message_length)
{
UINT                                  status;
NX_CRYPTO_METHOD                     *crypto_method;

    *extension_length = (USHORT)((*packet_buffer << 8) + packet_buffer[1] + 2);

    if (*extension_length > message_length || *extension_length <= 2)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Ignore the extension if we are not using ECJPAKE. */
    if (tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth -> nx_crypto_algorithm == NX_CRYPTO_KEY_EXCHANGE_ECJPAKE)
    {

        crypto_method = (NX_CRYPTO_METHOD*)tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_public_auth;

        /* ecjpake_key_kp_pair Extension.  */
        status = crypto_method -> nx_crypto_operation(NX_CRYPTO_ECJPAKE_SERVER_HELLO_PROCESS,
                                                      tls_session -> nx_secure_public_auth_handler,
                                                      crypto_method,
                                                      NX_NULL, 0,
                                                      &packet_buffer[2], /* Skip extension length. */
                                                      (ULONG)(*extension_length - 2),
                                                      NX_NULL, NX_NULL, 0,
                                                      tls_session -> nx_secure_public_auth_metadata_area,
                                                      tls_session -> nx_secure_public_auth_metadata_size,
                                                      NX_NULL, NX_NULL);
        if (status)
        {
            return(status);
        }
    }

    return(NX_SUCCESS);
}
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_serverhello_sec_reneg_extension PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the Secure Renegotiation Indication extension  */
/*    from an incoming ServerHello record.See RFC 5746 for more           */
/*    information.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    extension_length                      Length of extension data      */
/*    message_length                        Length of message data (bytes)*/
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
/*    _nx_secure_tls_process_serverhello_extensions                       */
/*                                          Process ServerHello extensions*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
static UINT _nx_secure_tls_proc_serverhello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                USHORT *extension_length, UINT message_length)
{
ULONG  offset;
UCHAR  renegotiated_connection_length;
USHORT parsed_length;
INT    compare_value;

    /* Secure Renegotiation Indication Extensions structure (for serverhello):
     * Initial ServerHello:
     * |     2      |     2     |        1         |
     * |  Ext Type  |  Ext Len  |  Reneg Info Len  |
     * |   0xff01   |   0x0001  |       0x00       |
     *
     * Renegotiating ServerHello:
     * |     2      |     2     |        1         |         12         |         12           |
     * |  Ext Type  |  Ext Len  |  Reneg Info Len  | client_verify_data |  server_verify_data  |
     * |   0xff01   |   0x0019  |       0x18       |                    |                      |
     */
    /*  From RFC 5746:
        struct {
             opaque renegotiated_connection<0..255>;
         } RenegotiationInfo;

          The contents of this extension are specified as follows.

      -  If this is the initial handshake for a connection, then the
         "renegotiated_connection" field is of zero length in both the
         ClientHello and the ServerHello.  Thus, the entire encoding of the
         extension is ff 01 00 01 00.  The first two octets represent the
         extension type, the third and fourth octets the length of the
         extension itself, and the final octet the zero length byte for the
         "renegotiated_connection" field.

      -  For ClientHellos that are renegotiating, this field contains the
         "client_verify_data" specified in Section 3.1.

      -  For ServerHellos that are renegotiating, this field contains the
         concatenation of client_verify_data and server_verify_data.  For
         current versions of TLS, this will be a 24-byte value (for SSLv3,
         it will be a 72-byte value).
     */

    /* Get the extension length. */
    parsed_length = (USHORT)((packet_buffer[0] << 8) + packet_buffer[1]);
    *extension_length = (USHORT)(2 + parsed_length);
    offset = 2;

    if (*extension_length > message_length || *extension_length < 3)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Get the "renegotiated_connection" field. */
    renegotiated_connection_length = packet_buffer[offset];
    offset++;

    if (offset + renegotiated_connection_length > *extension_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* See if the client is attempting to renegotiate an established connection. */
    if (renegotiated_connection_length)
    {
        /* The remote host is attempting a renegotiation - make sure our local session is active and renegotiation is OK. */
        if (!tls_session -> nx_secure_tls_local_session_active || !tls_session -> nx_secure_tls_remote_session_active)
        {
            /* Remote host is attempting a renegotiation but the server is not currently in a session! */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        if(!tls_session -> nx_secure_tls_secure_renegotiation)
        {
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }


        /* Check that the received verification data is the expected size. For ServerHello, the size is twice the
           finished verification hash size because it includes both client and server hashes. */
        if (renegotiated_connection_length != (2 * NX_SECURE_TLS_FINISHED_HASH_SIZE))
        {
            /* Do not have the right amount of data for comparison. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        /* Compare the received verify data to our locally-stored version - start with client (local) verify data. */
        compare_value = NX_SECURE_MEMCMP(&packet_buffer[offset], tls_session -> nx_secure_tls_local_verify_data, NX_SECURE_TLS_FINISHED_HASH_SIZE);

        if (compare_value)
        {
            /* Session verify data did not match what we expect - error. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        /* Now compare the remote verify data with what we just received. */
        offset += NX_SECURE_TLS_FINISHED_HASH_SIZE;
        compare_value = NX_SECURE_MEMCMP(&packet_buffer[offset], tls_session -> nx_secure_tls_remote_verify_data, NX_SECURE_TLS_FINISHED_HASH_SIZE);

        if (compare_value)
        {
            /* Session verify data did not match what we expect - error. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        /* If we get here, the verification data is good! */
        tls_session -> nx_secure_tls_secure_renegotiation_verified = NX_TRUE;
    }
    else
    {
        /* Verify that the extension contains only the initial handshake data - this is a new connection. */
        if ((parsed_length != 1) || (tls_session -> nx_secure_tls_local_session_active))
        {
            /* Error - the provided extension length was not expected for an initial handshake. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        /* The remote host supports secure renegotiation. */
        tls_session -> nx_secure_tls_secure_renegotiation = NX_TRUE;
    }

    return(NX_SUCCESS);
}
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_serverhello_keyshare_extension  PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the Key Share extension introduced in TLS 1.3. */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    extension_length                      Length of extension data      */
/*    message_length                        Length of message data (bytes)*/
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
/*    _nx_secure_tls_process_serverhello_extensions                       */
/*                                          Process ServerHello extensions*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), update   */
/*                                            ECC find curve method,      */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), removed  */
/*                                            public key format checking, */
/*                                            resulting in version 6.1.11 */
/*  10-31-2022     Yanwu Cai                Modified comment(s),          */
/*                                            updated parameters list,    */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

extern NX_CRYPTO_METHOD crypto_method_ecdhe;

static UINT _nx_secure_tls_proc_serverhello_keyshare_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                USHORT *extension_length, UINT message_length)
{
UINT status;
UINT i;
ULONG  offset;
USHORT key_group;
USHORT key_length;
NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA *ecc_key_data;
UCHAR                                *pubkey;
UCHAR                                *private_key;
UINT                                  private_key_length;
NX_CRYPTO_EXTENDED_OUTPUT             extended_output;
NX_CRYPTO_METHOD                     *ecdhe_method;
const NX_CRYPTO_METHOD               *curve_method;
VOID                                 *handler = NX_NULL;
NX_SECURE_TLS_ECC *ecc_info;

    /* Key Share Extension structure (for serverhello):
     *
     * ServerHello:
     * |      2     |     2     |     <key len>       |
     * |  Key Group |  Key Len  |  Key Exchange value |
     */

    offset = 0;

    /* Extract the extension length. */
    *extension_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
    offset = (USHORT)(offset + 2);

    if (offset + *extension_length > message_length || *extension_length < 2)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Get the key group. It must match one we sent in our ClientHello KeyShare extension. */
    key_group = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
    offset = (USHORT)(offset + 2);

    /* Get our session ECC data. */
    ecc_info = &(tls_session -> nx_secure_tls_ecc);

    /* Check the key group. */
    key_group = (USHORT)((ULONG)key_group | NX_CRYPTO_EC_MASK);


    /* Loop through all supported ECC curves in this session. */
    for (i = 0; i < ecc_info -> nx_secure_tls_ecc_supported_groups_count; i++)
    {
        /* Find the matchin group in the keys we generated and saved. */
        if (key_group != tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data[i].nx_secure_tls_ecdhe_named_curve)
        {
            continue;
        }

        /* Store selected ECDHE key data index. */
        tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data_selected = i;

        if (tls_session -> nx_secure_tls_client_state == NX_SECURE_TLS_CLIENT_STATE_HELLO_RETRY)
        {

            /* A HelloRetryRequest is received. Done here. */
            return(NX_SUCCESS);
        }

        /* Got a matching key/curve combo, get a pointer to the selected key data. */
        ecc_key_data = tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data;

        if (*extension_length < 4)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Get the key length. */
        key_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
        if (offset + key_length > *extension_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }
        offset = (USHORT)(offset + 2);

        /* Initialize the remote public key in our session. */
        pubkey = &packet_buffer[offset];

        /* Get the curve method to initialize the remote public key data. */
        _nx_secure_tls_find_curve_method(&tls_session -> nx_secure_tls_ecc, key_group, &curve_method, NX_NULL);

        if (curve_method == NX_NULL)
        {
            return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
        }

        /* Get the ECDHE method we are going to use. */
        ecdhe_method = &crypto_method_ecdhe;

        if (ecdhe_method -> nx_crypto_operation == NX_NULL )
        {
            return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
        }

        /* Initialize the ECDHE method. */
        if (ecdhe_method -> nx_crypto_init != NX_NULL)
        {
            status = ecdhe_method -> nx_crypto_init(ecdhe_method,
                                                    NX_NULL,
                                                    0,
                                                    &handler,
                                                    tls_session -> nx_secure_public_cipher_metadata_area,
                                                    tls_session -> nx_secure_public_cipher_metadata_size);

            if (status != NX_CRYPTO_SUCCESS)
            {
                return(status);
            }
        }

        /* Set the curve we want to use. */
        status = ecdhe_method -> nx_crypto_operation(NX_CRYPTO_EC_CURVE_SET, handler,
                                                     ecdhe_method, NX_NULL, 0,
                                                     (UCHAR *)curve_method, sizeof(NX_CRYPTO_METHOD *), NX_NULL,
                                                     NX_NULL, 0,
                                                     tls_session -> nx_secure_public_cipher_metadata_area,
                                                     tls_session -> nx_secure_public_cipher_metadata_size,
                                                     NX_NULL, NX_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        /* Import the private key to the ECDH context. */
        private_key = &ecc_key_data[i].nx_secure_tls_ecdhe_private_key[0];
        private_key_length = ecc_key_data[i].nx_secure_tls_ecdhe_private_key_length;

        status = ecdhe_method -> nx_crypto_operation(NX_CRYPTO_DH_KEY_PAIR_IMPORT, handler,
                                                    (NX_CRYPTO_METHOD*)ecdhe_method,
                                                    private_key, private_key_length << 3,
                                                    NX_NULL, 0, NX_NULL,
                                                    NX_NULL,
                                                    0,
                                                    tls_session -> nx_secure_public_cipher_metadata_area,
                                                    tls_session -> nx_secure_public_cipher_metadata_size,
                                                    NX_NULL, NX_NULL);


        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        //tls_session -> nx_secure_tls_key_material.nx_secure_tls_new_key_material_data[0] = (UCHAR)extended_output.nx_crypto_extended_output_actual_size;

        /* Calculate the final pre_master_secret - the "private key" here is the Pre-Master Secret. */
        extended_output.nx_crypto_extended_output_data = tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret;
        extended_output.nx_crypto_extended_output_length_in_byte = sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret);
        extended_output.nx_crypto_extended_output_actual_size = 0;
        status = ecdhe_method -> nx_crypto_operation(NX_CRYPTO_DH_CALCULATE, handler,
                                                     ecdhe_method, NX_NULL, 0,
                                                     pubkey, key_length, NX_NULL,
                                                     (UCHAR *)&extended_output,
                                                     sizeof(extended_output),
                                                     tls_session -> nx_secure_public_cipher_metadata_area,
                                                     tls_session -> nx_secure_public_cipher_metadata_size,
                                                     NX_NULL, NX_NULL);
        if (status != NX_CRYPTO_SUCCESS)
        {
            return(status);
        }

        tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size = extended_output.nx_crypto_extended_output_actual_size;

        if (ecdhe_method -> nx_crypto_cleanup)
        {
            status = ecdhe_method -> nx_crypto_cleanup(tls_session -> nx_secure_public_cipher_metadata_area);
        }

        return(status);
    }

    /* If we exhaust the list of ECC curves we sent, the server is doing something weird. */
    return(NX_SECURE_TLS_BAD_SERVERHELLO_KEYSHARE);

}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_serverhello_supported_versions_extension        */
/*                                                        PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the supported versions extension introduced in */
/*    TLS 1.3.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    supported_version                     Supported version             */
/*    extension_length                      Length of extension data      */
/*    message_length                        Length of message data (bytes)*/
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
/*    _nx_secure_tls_process_serverhello_extensions                       */
/*                                          Process ServerHello extensions*/
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
static UINT _nx_secure_tls_proc_serverhello_supported_versions_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                         UCHAR *packet_buffer,
                                                                         USHORT *supported_version,
                                                                         USHORT *extension_length, UINT message_length)
{
ULONG  offset;

    NX_PARAMETER_NOT_USED(tls_session);

    /* Supported Versions Extension structure (for serverhello):
     *
     * ServerHello:
     * |   2   |     2    |        ...         |
     * |  Type |  Length  |  Selected Version  |
     */

    /* RFC 8446, section 4.2.1, page 39.
     * A server negotiates TLS 1.3 MUST respond with 0x0304 in supported_versions extension.
     */

    offset = 0;

    /* Extract the extension length. */
    *extension_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
    offset = (USHORT)(offset + 2);

    if (offset + *extension_length > message_length || *extension_length != 2)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Find the selected version 0x0304(TLS 1.3). */
    *supported_version = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
    if (*supported_version == NX_SECURE_TLS_VERSION_TLS_1_3)
    {
        return(NX_SUCCESS);
    }

    return(NX_SECURE_TLS_UNKNOWN_TLS_VERSION);

}
#endif

#endif /* NX_SECURE_TLS_CLIENT_DISABLED */
