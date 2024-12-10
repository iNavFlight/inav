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

#if !defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION)
static UINT _nx_secure_tls_proc_clienthello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                UINT extension_length);
#endif


#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
static UINT _nx_secure_tls_proc_clienthello_keyshare_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                USHORT extension_length);
static UINT _nx_secure_tls_proc_clienthello_supported_versions_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                         UCHAR *packet_buffer,
                                                                         USHORT *supported_version,
                                                                         USHORT extension_length);
static VOID _nx_secure_tls_proc_clienthello_signature_algorithms_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                           const UCHAR *packet_buffer,
                                                                           USHORT extension_length);

#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
static UINT _nx_secure_tls_process_clienthello_psk_extension(NX_SECURE_TLS_SESSION *tls_session, const UCHAR *packet_buffer,
                                                             USHORT extension_length, const UCHAR *client_hello_buffer, UINT client_hello_length);
#endif
#endif


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_clienthello_extensions       PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes any extensions included in an incoming      */
/*    ClientHello message from a remote host.                             */
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
/*    _nx_secure_tls_proc_clienthello_sec_reneg_extension                 */
/*                                          Process ClientHello           */
/*                                            Renegotiation extension     */
/*    _nx_secure_tls_proc_clienthello_ec_groups_extension                 */
/*                                          Process ClientHello           */
/*                                            EC groups extension         */
/*    _nx_secure_tls_proc_clienthello_ec_point_formats_extension          */
/*                                          Process ClientHello           */
/*                                            EC point formats extension  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            ability to disable client   */
/*                                            initiated renegotiation,    */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_clienthello_extensions(NX_SECURE_TLS_SESSION *tls_session,
                                                   UCHAR *packet_buffer, UINT message_length,
                                                   NX_SECURE_TLS_HELLO_EXTENSION *extensions,
                                                   UINT *num_extensions, UCHAR *client_hello_buffer, UINT client_hello_length)
{
UINT   status = NX_SUCCESS;
UINT   offset;
UINT   max_extensions;
USHORT extension_id;
UINT   extension_length;
#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
USHORT supported_version = tls_session -> nx_secure_tls_protocol_version;
#endif

#if defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION) || defined(NX_SECURE_TLS_DISABLE_CLIENT_INITIATED_RENEGOTIATION)
    NX_PARAMETER_NOT_USED(tls_session);
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */

#if !(NX_SECURE_TLS_TLS_1_3_ENABLED) || !defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES)
    NX_PARAMETER_NOT_USED(client_hello_buffer);
    NX_PARAMETER_NOT_USED(client_hello_length);
#endif

    max_extensions = *num_extensions;
    offset = 0;
    *num_extensions = 0;

    /* Process extensions until we run out. */
    while (offset < message_length && *num_extensions < max_extensions)
    {

        /* Make sure there are at least 4 bytes available so we can read extension_id and
           extension_length. */
        if((offset + 4) > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* See what the extension is. */
        extension_id = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
        offset += 2;

        /* Get extension length. */
        extension_length = (UINT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
        offset += 2;

        /* Verify the message_length is at least "extension_length". */
        if((offset + extension_length) > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Parse the extension. */
        switch (extension_id)
        {
#if !defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION)
        case NX_SECURE_TLS_EXTENSION_SECURE_RENEGOTIATION:
            status = _nx_secure_tls_proc_clienthello_sec_reneg_extension(tls_session,
                                                                         &packet_buffer[offset],
                                                                         extension_length);

            if (status)
            {
                return(status);
            }
            break;
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */

#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
        case NX_SECURE_TLS_EXTENSION_KEY_SHARE:
            if(tls_session -> nx_secure_tls_1_3)
            {
                /* If TLS 1.3, process the key share extension. */
                status = _nx_secure_tls_proc_clienthello_keyshare_extension(tls_session,
                                                                            &packet_buffer[offset],
                                                                            (USHORT)extension_length);

                if (status)
                {
                    return(status);
                }
            }
            /* TLS 1.2 and earlier: ignore extension. */
            break;
        case NX_SECURE_TLS_EXTENSION_SUPPORTED_VERSIONS:
            if(tls_session -> nx_secure_tls_1_3)
            {

                /* Process the TLS 1.3 supported_versions extension. */
                status = _nx_secure_tls_proc_clienthello_supported_versions_extension(tls_session,
                                                                                      &packet_buffer[offset],
                                                                                      &supported_version,
                                                                                      (USHORT)extension_length);

                if (status)
                {
                    return(status);
                }
            }

            /* Ignore if not TLS 1.3. */
            break;
#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
        case NX_SECURE_TLS_EXTENSION_PRE_SHARED_KEY:
            if(tls_session -> nx_secure_tls_1_3)
            {

                /* Process the TLS 1.3 PSK extension. */
                status = _nx_secure_tls_process_clienthello_psk_extension(tls_session, &packet_buffer[offset],
                                                                          (USHORT)extension_length, client_hello_buffer, client_hello_length);

                if (status)
                {
                    return(status);
                }
            }
            break;
#endif
#endif

        case NX_SECURE_TLS_EXTENSION_SIGNATURE_ALGORITHMS:
        case NX_SECURE_TLS_EXTENSION_SERVER_NAME_INDICATION:
        case NX_SECURE_TLS_EXTENSION_MAX_FRAGMENT_LENGTH:
        case NX_SECURE_TLS_EXTENSION_CLIENT_CERTIFICATE_URL:
        case NX_SECURE_TLS_EXTENSION_TRUSTED_CA_INDICATION:
        case NX_SECURE_TLS_EXTENSION_CERTIFICATE_STATUS_REQUEST:
        case NX_SECURE_TLS_EXTENSION_EC_GROUPS:
        case NX_SECURE_TLS_EXTENSION_EC_POINT_FORMATS:
        case NX_SECURE_TLS_EXTENSION_ECJPAKE_KEY_KP_PAIR:
            /* These extensions require information to be passed to the application. Save off
               the extension data in our extensions array to pass along in the hello callback. */
            extensions[*num_extensions].nx_secure_tls_extension_id = extension_id;
            extensions[*num_extensions].nx_secure_tls_extension_data = &packet_buffer[offset];
            extensions[*num_extensions].nx_secure_tls_extension_data_length = (USHORT)extension_length;

            /* Added another extension to the array. */
            *num_extensions = *num_extensions + 1;

            break;
        case NX_SECURE_TLS_EXTENSION_TRUNCATED_HMAC:
        default:
            /* Unknown or unsupported extension, just ignore - TLS supports multiple extensions and the default
               behavior is to ignore any extensions that we don't know. */
            break;
        }

        /* Adjust our offset with the length of the extension we just parsed. */
        offset += extension_length;
    }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
    if((tls_session -> nx_secure_tls_1_3) && (supported_version != NX_SECURE_TLS_VERSION_TLS_1_3))
    {

        /* Negotiate a version of TLS prior to TLS 1.3. */
        if (tls_session -> nx_secure_tls_protocol_version_override == 0)
        {
            tls_session -> nx_secure_tls_1_3 = NX_FALSE;
#if !defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION)
            tls_session -> nx_secure_tls_renegotation_enabled = NX_TRUE;
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */
            tls_session -> nx_secure_tls_protocol_version = supported_version;
        }
        else
        {

            /* Protocol version is overridden to TLS 1.3. */
            return(NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION);
        }
    }
#endif

    return(status);
}



/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_clienthello_sec_reneg_extension PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the Secure Renegotiation Indication extension  */
/*    from an incoming ClientHello record.See RFC 5746 for more           */
/*    information.                                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    extension_length                      Length of extension data      */
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
/*    _nx_secure_tls_process_clienthello_extensions                       */
/*                                          Process ClientHello extensions*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            ability to disable client   */
/*                                            initiated renegotiation,    */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
#if !defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION)
static UINT _nx_secure_tls_proc_clienthello_sec_reneg_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                UINT extension_length)
{
ULONG  offset = 0;
UCHAR  renegotiated_connection_length;
INT    compare_value;

    /* Secure Renegotiation Indication Extensions structure:
     * Initial ClientHello:
     * |     2      |     2     |        1         |
     * |  Ext Type  |  Ext Len  |  Reneg Info Len  |
     * |   0xff01   |   0x0001  |       0x00       |
     *
     * Renegotiating ClientHello:
     * |     2      |     2     |        1         |         12         |
     * |  Ext Type  |  Ext Len  |  Reneg Info Len  | client_verify_data |
     * |   0xff01   |   0x000d  |       0x0c       |                    |
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

    if (extension_length < 1)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Get the "renegotiated_connection" field. */
    renegotiated_connection_length = packet_buffer[offset];
    offset++;

    if (offset + renegotiated_connection_length > extension_length)
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
            return(NX_SECURE_TLS_RENEGOTIATION_SESSION_INACTIVE);
        }

        if(!tls_session -> nx_secure_tls_secure_renegotiation)
        {
            return(NX_SECURE_TLS_RENEGOTIATION_FAILURE);
        }

        if (renegotiated_connection_length != NX_SECURE_TLS_FINISHED_HASH_SIZE)
        {
            /* Do not have the right amount of data for comparison. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        /* Compare the received verify data to our locally-stored version. */
        compare_value = NX_SECURE_MEMCMP(&packet_buffer[offset], tls_session -> nx_secure_tls_remote_verify_data, NX_SECURE_TLS_FINISHED_HASH_SIZE);

        if (compare_value)
        {
            /* Session verify data did not match what we expect - error. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        tls_session -> nx_secure_tls_secure_renegotiation_verified = NX_TRUE;
    }
    else
    {
        /* Verify that the extension contains only the initial handshake data - this is a new connection. */
        if ((extension_length != 1) || (tls_session -> nx_secure_tls_local_session_active))
        {
            /* Error - the provided extension length was not expected for an initial handshake. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }

        /* The remote host supports renegotiation. */
        tls_session -> nx_secure_tls_secure_renegotiation = NX_TRUE;
    }

    return(NX_SUCCESS);
}
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_clienthello_sec_sa_extension    PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes the supported groups extensions included in */
/*    an incoming ClientHello message from a remote host.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    exts                                  Parsed extensions             */
/*    num_extensions                        Number of extensions          */
/*    selected_curve                        Output selected ECDHE curve   */
/*    cert_curve                            Named curve of local cert     */
/*    cert_curve_supported                  Output whether curve used by  */
/*                                            local cert is supported     */
/*    ecdhe_signature_algorithm             Output signature algorithm    */
/*                                            for ECDHE key exchange      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_find_curve_method      Find named curve used by cert */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), added    */
/*                                            curve priority logic,       */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Timothy Stapko           Modified comment(s), fixed    */
/*                                            compilation issue with      */
/*                                            TLS 1.3 and disabling TLS   */
/*                                            server,                     */
/*                                            resulting in version 6.1.9  */
/*  10-31-2022     Yanwu Cai                Modified comment(s),          */
/*                                            updated parameters list,    */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
UINT _nx_secure_tls_proc_clienthello_sec_sa_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                      NX_SECURE_TLS_HELLO_EXTENSION *exts,
                                                      UINT num_extensions,
                                                      UINT *selected_curve, USHORT cert_curve,
                                                      UINT *cert_curve_supported,
                                                      USHORT *ecdhe_signature_algorithm,
                                                      NX_SECURE_X509_CERT *cert)
{
UINT   status = NX_SUCCESS;
UINT   i, j, k;
const UCHAR *groups;
USHORT group;
USHORT groups_len;
const NX_CRYPTO_METHOD *curve_method;
UINT curve_priority;
UINT new_curve_priority = 0;
#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
NX_SECURE_TLS_ECC *ecc_info;
UINT   signature_algorithm_exist = NX_FALSE;
#endif
const UCHAR *signature_algorithms;
USHORT signature_algorithms_len;
USHORT signature_algorithm;
USHORT signature_algorithm_id;
UCHAR expected_signature = 0;


    /* Select SHA256 as the signature hash. */
    *ecdhe_signature_algorithm = NX_SECURE_TLS_SIGNATURE_RSA_SHA256;


    *cert_curve_supported = NX_FALSE;
    *selected_curve = 0;

    if (cert_curve)
    {
        status = _nx_secure_tls_find_curve_method(&tls_session -> nx_secure_tls_ecc, cert_curve, &curve_method, NX_NULL);
        if (status == NX_SUCCESS && curve_method != NX_NULL)
        {
            *cert_curve_supported = NX_TRUE;
        }
    }

    if (cert != NX_NULL)
    {
        if (cert -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_EC)
        {
            expected_signature = NX_SECURE_TLS_SIGNATURE_ALGORITHM_ECDSA;
        }
        else if (cert -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_RSA)
        {
            expected_signature = NX_SECURE_TLS_SIGNATURE_ALGORITHM_RSA;
        }

#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
        if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
            tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1)
        {
            *ecdhe_signature_algorithm = (USHORT)((NX_SECURE_TLS_HASH_ALGORITHM_SHA1 << 8) | (expected_signature & 0xFF));
        }
#endif /* NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED */
    }

    for (i = 0; i < num_extensions; i++)
    {
        switch (exts[i].nx_secure_tls_extension_id)
        {
        case NX_SECURE_TLS_EXTENSION_EC_GROUPS:
            groups = exts[i].nx_secure_tls_extension_data;
            groups_len = exts[i].nx_secure_tls_extension_data_length;

            /* Set our start priority to the size of our supported curves list (lowest priority). */
            curve_priority = tls_session -> nx_secure_tls_ecc.nx_secure_tls_ecc_supported_groups_count;

            /* Loop through curves sent by client. */
            for (j = 0; j < groups_len; j += 2)
            {
                group = (USHORT)((groups[j] << 8) + groups[j + 1]);

                status = _nx_secure_tls_find_curve_method(&tls_session -> nx_secure_tls_ecc, group, &curve_method, &new_curve_priority);

                if ((status == NX_CRYTPO_MISSING_ECC_CURVE) || (new_curve_priority > curve_priority))
                {

                    /* Keep searching list. */
                    continue;
                }

                /* Found a higher-priority curve. */
                if (status == NX_SUCCESS)
                {
                    /* Found shared named curve. */
                    *selected_curve = group;
                    curve_priority = new_curve_priority;

#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
                    if (tls_session -> nx_secure_tls_1_3 &&
                        (tls_session -> nx_secure_tls_server_state == NX_SECURE_TLS_SERVER_STATE_SEND_HELLO_RETRY))
                    {

                        /* Get our session ECC data. */
                        ecc_info = &(tls_session -> nx_secure_tls_ecc);

                        /* Loop through all supported ECC curves in this session. */
                        for (k = 0; k < ecc_info -> nx_secure_tls_ecc_supported_groups_count; k++)
                        {
                            /* Find the matching group in the keys we generated and saved. */
                            if(group == tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data[k].nx_secure_tls_ecdhe_named_curve)
                            {

                                /* Store selected ECDHE key data index. */
                                tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data_selected = k;

                            }
                        }
                    }
#endif
                }
                else
                {

                    /* status is not NX_CRYTPO_MISSING_ECC_CURVE or NX_SUCCESS, return error. */
                    return(status);
                }
                /* Continue searching our supported curves list until we find the highest-priority curve. */
            }

            /* Reset status as we do not return NX_CRYTPO_MISSING_ECC_CURVE. */
            status = NX_SUCCESS;
            break;

        case NX_SECURE_TLS_EXTENSION_SIGNATURE_ALGORITHMS:
            /* This extension is not meaningful for TLS versions prior to 1.2. */
#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
            if (tls_session -> nx_secure_tls_1_3)
            {
                _nx_secure_tls_proc_clienthello_signature_algorithms_extension(tls_session,
                                                                               exts[i].nx_secure_tls_extension_data,
                                                                               exts[i].nx_secure_tls_extension_data_length);
                signature_algorithm_exist = NX_TRUE;
            }
            else
#endif
#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
            if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
            {

                signature_algorithms = exts[i].nx_secure_tls_extension_data;
                signature_algorithms_len = (USHORT)exts[i].nx_secure_tls_extension_data_length;

                if (signature_algorithms_len < 2 || (((signature_algorithms[0] << 8) + signature_algorithms[1]) != (signature_algorithms_len - 2)))
                {
                    return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
                }

                /* Loop the signature algorithms in the extension. */
                for (j = 2; j < signature_algorithms_len; j += 2)
                {
                    if (signature_algorithms[j + 1] != expected_signature)
                    {
                        continue;
                    }

                    signature_algorithm = (USHORT)((signature_algorithms[j] << 8) + signature_algorithms[j + 1]);

                    /* Map signature algorithm to internal ID. */
                    _nx_secure_tls_get_signature_algorithm_id((UINT)signature_algorithm, &signature_algorithm_id);

                    if (signature_algorithm_id == NX_SECURE_TLS_X509_TYPE_UNKNOWN)
                    {
                        continue;
                    }

                    /* Check if this signature algorithm is supported. */
                    for (k = 0; k < tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table_size; k++)
                    {
                        if (signature_algorithm_id == tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table[k].nx_secure_x509_crypto_identifier)
                        {
                            *ecdhe_signature_algorithm = signature_algorithm;
                            break;
                        }
                    }

                    /* Signature algorithm is selected. */
                    if (*ecdhe_signature_algorithm == signature_algorithm)
                    {
                        break;
                    }
                }
            }
#endif
            break;
        }
    }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)
    /* RFC8446 4.2.3: If a server is authenticating via a certificate and
       the client has not sent a "signature_algorithms" extension,
       then the server MUST abort the handshake with a "missing_extension" alert. */
    if ((cert != NX_NULL) && (tls_session -> nx_secure_tls_1_3))
    {
        if (signature_algorithm_exist == NX_FALSE)
        {
            return(NX_SECURE_TLS_MISSING_EXTENSION);
        }

        if (tls_session -> nx_secure_tls_signature_algorithm == 0)
        {
            return(NX_SECURE_TLS_UNKNOWN_CERT_SIG_ALGORITHM);
        }
    }
#endif

    return(status);
}
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_clienthello_keyshare_extension  PORTABLE C      */
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
/*  10-15-2021     Timothy Stapko           Modified comment(s), fixed    */
/*                                            compilation issue with      */
/*                                            TLS 1.3 and disabling TLS   */
/*                                            server,                     */
/*                                            resulting in version 6.1.9  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s), removed  */
/*                                            public key format checking, */
/*                                            resulting in version 6.1.11 */
/*  10-31-2022     Yanwu Cai                Modified comment(s),          */
/*                                            updated parameters list,    */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && !defined(NX_SECURE_TLS_SERVER_DISABLED)

extern NX_CRYPTO_METHOD crypto_method_ecdhe;

static UINT _nx_secure_tls_proc_clienthello_keyshare_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                UCHAR *packet_buffer,
                                                                USHORT extension_length)
{
UINT status;
UINT i;
USHORT  offset;
USHORT key_share_length;
USHORT key_group;
USHORT key_length;
NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA   *ecc_key_data;
UCHAR                                *pubkey = NX_NULL;
UCHAR                                *private_key;
UINT                                  private_key_length;
NX_CRYPTO_EXTENDED_OUTPUT             extended_output;
NX_CRYPTO_METHOD                     *ecdhe_method;
const NX_CRYPTO_METHOD               *curve_method;
VOID                                 *handler = NX_NULL;
NX_SECURE_TLS_ECC *ecc_info;

    /* Key Share Extension structure (for clienthello):
     *
     * ClientHello KeyShare:
     *                                   |                 <list length>                      |
     * |     2      |         2          |  <|       2      |    2    |     <Key len>     |>  |
     * |  Ext Type  |  Extension length  |  <|  Named Group | Key len | key_exchange data |>  |

     */

    if (extension_length == 0)
    {

        /* The client_shares may be empty if the client is requesting a HelloRetryRequest. */
        tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_SEND_HELLO_RETRY;
        return(NX_SUCCESS);
    }

    offset = 0;

    /* ecc_key_data will hold the key information from the chosen named curve. */
    ecc_key_data = NX_NULL;

    /* Get our session ECC data. */
    ecc_info = &(tls_session -> nx_secure_tls_ecc);

    if (extension_length < 2)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Get the key share length. */
    key_share_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
    offset = (USHORT)(offset + 2);

    if(offset + key_share_length > extension_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Loop through the list of keys looking for a supported group. */
    while(offset < extension_length && ecc_key_data == NX_NULL)
    {
        if (offset + 4 > extension_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Get the key group.  */
        key_group = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
        offset = (USHORT)(offset + 2);

        /* Get the key length. */
        key_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
        offset = (USHORT)(offset + 2);

        if (offset + key_length > extension_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Check the key group. */
        key_group = (USHORT)((ULONG)key_group | NX_CRYPTO_EC_MASK);

        /* Loop through all supported ECC curves in this session - see if we support the current group. */
        for (i = 0; i < ecc_info -> nx_secure_tls_ecc_supported_groups_count; i++)
        {
            /* Find the matching group in the keys we generated and saved. */
            if(key_group == tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data[i].nx_secure_tls_ecdhe_named_curve)
            {
                /* Got a matching key/curve combo, get a pointer to the selected key data. */
                ecc_key_data = &tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data[i];

                /* Extract the remote public key from our packet. */
                pubkey = &packet_buffer[offset];

                /* Store selected ECDHE key data index. */
                tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data_selected = i;

                break;
            }
        }

        /* Advance past the key data. */
        offset = (USHORT)(offset + key_length);
    }

    /* If we didn't find a matching curve, return error. */
    if(ecc_key_data == NX_NULL || offset > extension_length)
    {
        tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_SEND_HELLO_RETRY;
        return(NX_SUCCESS);
    }

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
    private_key = &ecc_key_data -> nx_secure_tls_ecdhe_private_key[0];
    private_key_length = ecc_key_data -> nx_secure_tls_ecdhe_private_key_length;

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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_clienthello_supported_versions_extension        */
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
/*    _nx_secure_tls_process_clienthello_extensions                       */
/*                                          Process ClientHello extensions*/
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
static UINT _nx_secure_tls_proc_clienthello_supported_versions_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                         UCHAR *packet_buffer,
                                                                         USHORT *supported_version,
                                                                         USHORT extension_length)
{
UINT i;
ULONG  offset;

    NX_PARAMETER_NOT_USED(tls_session);

    /* Supported Versions Extension structure (for clienthello):
     *
     * ClientHello:
     * |      1       | <list length> |
     * |  List Length |  TLS Versions |
     */

    offset = 0;

    /* Extract the extension length. */
    if ((extension_length) < 1 || (packet_buffer[0] != (extension_length - 1)))
    {

        /* Invalid Supported Versions Length. */
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    offset = 1;

    /* Loop through all supported versions in this session. */
    for (i = 0; i < (UINT)(extension_length - 1); i += 2)
    {

        /* Find the preferred protocol version. */
        *supported_version = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
        if (*supported_version == NX_SECURE_TLS_VERSION_TLS_1_3)
        {
            return(NX_SUCCESS);
        }
        else if (_nx_secure_tls_check_protocol_version(tls_session, *supported_version, NX_SECURE_TLS) == NX_SUCCESS)
        {
            return(NX_SUCCESS);
        }
        offset = (USHORT)(offset + 2);
    }

    return(NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION);
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_proc_clienthello_signature_algorithms_extension      */
/*                                                        PORTABLE C      */
/*                                                           6.1.9        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function parses the siganture algorithms extension introduced  */
/*    in TLS 1.3.                                                         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Outgoing TLS packet buffer    */
/*    extension_length                      Length of extension data      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_clienthello_extensions                       */
/*                                          Process ClientHello extensions*/
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Timothy Stapko           Modified comment(s), fixed    */
/*                                            compilation issue with      */
/*                                            TLS 1.3 and disabling TLS   */
/*                                            server,                     */
/*                                            resulting in version 6.1.9  */
/*                                                                        */
/**************************************************************************/
static VOID _nx_secure_tls_proc_clienthello_signature_algorithms_extension(NX_SECURE_TLS_SESSION *tls_session,
                                                                           const UCHAR *packet_buffer,
                                                                           USHORT extension_length)
{
UINT i;
USHORT offset;
UINT status = 0;
UINT expected_sign_alg = 0;
UINT sign_alg = 0;
NX_SECURE_X509_CERT *local_certificate = NX_NULL;


    /* Signature Algorithm Extension structure (for clienthello):
     *
     * ClientHello:
     * |      2       |      <list length>      |
     * |  List Length |  Signature Algorithems  |
     */

    tls_session -> nx_secure_tls_signature_algorithm = 0;
    offset = 0;

    /* Extract the extension length. */
    if ((extension_length < 2) || (((packet_buffer[0] << 8) + packet_buffer[1]) != (extension_length - 2)))
    {

        /* Invalid Supported Versions Length. */
        return;
    }

    offset = 2;

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
        if (local_certificate -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_RSA)
        {
            return;
        }
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
        else if (local_certificate -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_EC)
        {

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
                return;
            }
        }
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
    }

    /* Loop through all supported versions in this session. */
    for (i = 0; i < (UINT)(extension_length - 2); i += 2)
    {

        /* Find the expected signature algorithm. */
        sign_alg = (UINT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);

        if (sign_alg == expected_sign_alg)
        {
            break;
        }

        offset = (USHORT)(offset + 2);
    }

    /* Make sure we are using the right signature algorithm! */
    if (sign_alg != expected_sign_alg)
    {
        return;
    }

    tls_session -> nx_secure_tls_signature_algorithm = sign_alg;
}
#endif /* NX_SECURE_TLS_TLS_1_3_ENABLED */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_get_signature_algorithm_id           PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Get internal x509 ID of signature algorithm.                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    signature_algorithm                   Signature algorithm           */
/*    signature_algorithm_id                Internal ID                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
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
VOID _nx_secure_tls_get_signature_algorithm_id(UINT signature_algorithm, USHORT *signature_algorithm_id)
{

    *signature_algorithm_id = NX_SECURE_TLS_X509_TYPE_UNKNOWN;

    switch (signature_algorithm)
    {
    case NX_SECURE_TLS_SIGNATURE_RSA_MD5:
        *signature_algorithm_id = NX_SECURE_TLS_X509_TYPE_RSA_MD5;
        break;
    case NX_SECURE_TLS_SIGNATURE_RSA_SHA1:
        *signature_algorithm_id = NX_SECURE_TLS_X509_TYPE_RSA_SHA_1;
        break;
    case NX_SECURE_TLS_SIGNATURE_RSA_SHA256:
        *signature_algorithm_id = NX_SECURE_TLS_X509_TYPE_RSA_SHA_256;
        break;
    case NX_SECURE_TLS_SIGNATURE_RSA_SHA384:
        *signature_algorithm_id = NX_SECURE_TLS_X509_TYPE_RSA_SHA_384;
        break;
    case NX_SECURE_TLS_SIGNATURE_RSA_SHA512:
        *signature_algorithm_id = NX_SECURE_TLS_X509_TYPE_RSA_SHA_512;
        break;
    case NX_SECURE_TLS_SIGNATURE_ECDSA_SHA1:
        *signature_algorithm_id = NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_1;
        break;
    case NX_SECURE_TLS_SIGNATURE_ECDSA_SHA224:
        *signature_algorithm_id = NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_224;
        break;
    case NX_SECURE_TLS_SIGNATURE_ECDSA_SHA256:
        *signature_algorithm_id = NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_256;
        break;
    case NX_SECURE_TLS_SIGNATURE_ECDSA_SHA384:
        *signature_algorithm_id = NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_384;
        break;
    case NX_SECURE_TLS_SIGNATURE_ECDSA_SHA512:
        *signature_algorithm_id = NX_SECURE_TLS_X509_TYPE_ECDSA_SHA_512;
        break;
    default:
        return;
    }
}


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_clienthello_psk_extension    PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    Process an incoming TLS 1.3 PSK extension from a TLS Client.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Incoming packet data          */
/*    extension_length                      Length of PSK extension       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*                                                                        */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  08-02-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            hash clone and cleanup,     */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES)
static UINT _nx_secure_tls_process_clienthello_psk_extension(NX_SECURE_TLS_SESSION *tls_session, const UCHAR *packet_buffer,
                                                      USHORT extension_length, const UCHAR *client_hello_buffer, UINT client_hello_length)
{
UINT list_length;
UCHAR                                *psk_data = NX_NULL;
UINT                                  psk_length;
UCHAR   binder_len;
UINT psk_index;
UINT binder_index;
UINT psk_store_index = 0;
ULONG  offset;
UINT   id_len;
const UCHAR  *id;
const UCHAR  *binder;
INT   binder_total;
UINT   status;
UINT   age;
UINT psk_found = NX_FALSE;
UINT partial_hello_length;

NX_SECURE_TLS_PSK_STORE *psk_store;


    /* PSK Extension structure (From TLS 1.3 RFC 8446):
          struct {
              opaque identity<1..2^16-1>;
              uint32 obfuscated_ticket_age;
          } PskIdentity;

          opaque PskBinderEntry<32..255>;

          struct {
              PskIdentity identities<7..2^16-1>;
              PskBinderEntry binders<33..2^16-1>;
          } OfferedPsks;

          struct {
              select (Handshake.msg_type) {
                  case client_hello: OfferedPsks;
                  case server_hello: uint16 selected_identity;
              };
          } PreSharedKeyExtension;

       identity:  A label for a key.  For instance, a ticket (as defined in
          Appendix B.3.4) or a label for a pre-shared key established
          externally.

       obfuscated_ticket_age:  An obfuscated version of the age of the key.
          Section 4.2.11.1 describes how to form this value for identities
          established via the NewSessionTicket message.  For identities
          established externally, an obfuscated_ticket_age of 0 SHOULD be
          used, and servers MUST ignore the value.

       identities:  A list of the identities that the client is willing to
          negotiate with the server.  If sent alongside the "early_data"
          extension (see Section 4.2.10), the first identity is the one used
          for 0-RTT data.

       binders:  A series of HMAC values, one for each value in the
          identities list and in the same order, computed as described
          below.

       selected_identity:  The server's chosen identity expressed as a
          (0-based) index into the identities in the client's list.


      ClientHello PSK extension layout:
      |       2      |      <ID list len>        |      2      | <binders len>   |
      |  ID list len | <|<ID len> | ID | age |>  | binders len | <len> | binder  |
    */


    if (tls_session == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    offset = 0;

    if (extension_length < 2)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Get the length of the ID list. (Extension id and length already removed by caller). */
    list_length = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
    offset += 2;

    /* Make sure the length is reasonable. */
    if(list_length > extension_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Get our PSK store for easy access. */
    psk_store = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store;
    binder_total = 0;
    psk_index = 0;

    /* Loop through all IDs. */
    while(list_length > 0)
    {

        if (list_length < 2)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Extract ID length. */
        id_len = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
        offset += 2;

        if(offset + id_len > extension_length || list_length < 2 + id_len + 4)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Reference to ID data. */
        id = &packet_buffer[offset];
        offset += id_len;

        /* Extract the age field. */
        age = (UINT)((packet_buffer[offset]     << 24) + (packet_buffer[offset + 1] << 16) +
                     (packet_buffer[offset + 2] <<  8) +  packet_buffer[offset + 3]);
        offset += 4;

        /* Only support external PSKs (no session resumption). */
        if(age != 0)
        {
            return(NX_SECURE_TLS_BAD_CLIENTHELLO_PSK_EXTENSION);
        }

        /* Size of entry = 2 bytes id_len + <id_len> + 4 bytes age field. */
        list_length -= (2 + id_len + 4);

        /* Check the ID list against our PSK store. */
        status = _nx_secure_tls_psk_identity_find(tls_session, &psk_data, &psk_length, (UCHAR*)id, id_len, &psk_store_index);

        /* No match? Continue. */
        if(status == NX_SECURE_TLS_NO_MATCHING_PSK)
        {
            /* Advance our selected PSK index. */
            psk_index++;

            continue;
        }
        else if(status != NX_SUCCESS)
        {
            return(status);
        }

        /* At this point, we have a match - use this PSK if the age and binder check out.
         * Add the remaining list length to the offset to reach the binder list. */
        psk_found = NX_TRUE;
        offset += list_length;
        break;
    }

    /* If we didn't find a match, return the error. */
    if(!psk_found)
    {
        return(NX_SECURE_TLS_NO_MATCHING_PSK);
    }

    /* Now we can verify the age. */

    if (extension_length < offset + 2)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Extract the total binder list length. */
    binder_total = (USHORT)((packet_buffer[offset] << 8) + packet_buffer[offset + 1]);
    offset += 2;

    /* Make sure the length is reasonable. */
    if((UINT)binder_total + offset > extension_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* We need to subtract the binders from the ClientHello for our binder generation
       since the hash for the binder is generated from a partial ClientHello before
       the binders are generated. Include 2 bytes for the length field. */
    partial_hello_length = client_hello_length - ((UINT)binder_total + 2);

    binder = NX_NULL;
    binder_index = 0;

    /* Loop through the binders to get the binder for our selected PSK. */
    while(binder_total > 0)
    {
        /* Extract binder length. */
        binder_len = packet_buffer[offset];
        offset++;

        if (1 + binder_len > binder_total)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        /* Subtract the binder length and field from the total. */
        binder_total--;
        binder_total -= binder_len;

        /* Check our current binder index against the selected PSK index. */
        if(binder_index == psk_index)
        {
            /* Extract binder and exit loop. */
            binder = &packet_buffer[offset];
            break;
        }

        /* Advance the binder index. */
        binder_index++;
    }

    if(binder == NX_NULL)
    {
        /* Binder is missing! */
        return(NX_SECURE_TLS_BAD_CLIENTHELLO_PSK_EXTENSION);
    }

    /* Check binder against locally-generated version. */
    /* If nx_secure_tls_psk_data_size is zero, it means this is ClientHello1, need to initialize the handshake hash.
       If not, it means this is ClientHello2, need to save the metadata(ClientHello1 and HelloRetryRequest) to sratch buffer. */
    if (tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_data_size == 0)
    {

        /* Initialize the handshake hash to hash the ClientHello up to now. */
        _nx_secure_tls_handshake_hash_init(tls_session);
    }
    else
    {

        /* Save the handshake hash state. */
        NX_SECURE_HASH_METADATA_CLONE(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size); /* Use case of memcpy is verified. */
    }

    /* Hash the ClientHello, adding the TLS record header. */
    UCHAR header[] = { 0x01, 0x00, 0x00, 0x00 };
    header[2] = (UCHAR)((client_hello_length) >> 8);
    header[3] = (UCHAR)((client_hello_length) & 0xFF);
    _nx_secure_tls_handshake_hash_update(tls_session, header, sizeof(header));
    _nx_secure_tls_handshake_hash_update(tls_session, (UCHAR*)client_hello_buffer, partial_hello_length);

    /* Save the transcript hash for the ClientHello, which is used in generating the PSK binders. */
    status = _nx_secure_tls_1_3_transcript_hash_save(tls_session, NX_SECURE_TLS_TRANSCRIPT_IDX_CLIENTHELLO, NX_FALSE);
    if (status != NX_SUCCESS)
    {

        if (tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_data_size)
        {
            NX_SECURE_HASH_CLONE_CLEANUP(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                         tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size);
        }

        return(status);
    }

    /* Restore the original metadata(ClientHello1 and HelloRetryRequest) from scratch buffer. */
    if (tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_data_size)
    {

        NX_SECURE_HASH_CLONE_CLEANUP(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata,
                                     tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size);

        NX_SECURE_HASH_METADATA_CLONE(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                      tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size); /* Use case of memcpy is verified. */

        NX_SECURE_HASH_CLONE_CLEANUP(tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_scratch,
                                     tls_session -> nx_secure_tls_handshake_hash.nx_secure_tls_handshake_hash_sha256_metadata_size);
    }

    /* Generate the binder for our selected PSK. */
    _nx_secure_tls_psk_binder_generate(tls_session, &psk_store[psk_store_index]);

    /* Compare the generated binder to the received binder, using our generated length to avoid overflow
       from incoming length problems. */
    status = (UINT)NX_SECURE_MEMCMP(binder, psk_store[psk_store_index].nx_secure_tls_psk_binder, psk_store[psk_store_index].nx_secure_tls_psk_binder_size);

    /* Make sure the generated binder matches the one sent by the client. */
    if((status != 0) || (binder_len != psk_store[psk_store_index].nx_secure_tls_psk_binder_size))
    {
        return(NX_SECURE_TLS_PSK_BINDER_MISMATCH);
    }

    /* The PSK is too big to save in our internal buffer. */
    if(psk_length > NX_SECURE_TLS_MAX_PSK_SIZE)
    {
        return(NX_SECURE_TLS_NO_MORE_PSK_SPACE);
    }

    /* Make sure the Client PSK is initialized for later key generation. */
    NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_data, psk_data, psk_length); /* Use case of memcpy is verified. */
    tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_data_size = psk_length;

    return(NX_SUCCESS);
}
#endif



