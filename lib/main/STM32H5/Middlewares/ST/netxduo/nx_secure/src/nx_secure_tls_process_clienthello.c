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

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
static UINT _nx_secure_tls_check_ciphersuite(const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite_info,
                                             NX_SECURE_X509_CERT *cert, UINT selected_curve,
                                             UINT cert_curve_supported);

#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_clienthello                  PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ClientHello message from a      */
/*    remote host, kicking off a TLS handshake.                           */
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
/*    _nx_secure_tls_ciphersuite_lookup     Lookup current ciphersuite    */
/*    _nx_secure_tls_check_protocol_version Check incoming TLS version    */
/*    _nx_secure_tls_newest_supported_version                             */
/*                                          Get newest TLS version        */
/*    _nx_secure_tls_process_clienthello_extensions                       */
/*                                          Process ClientHello extensions*/
/*    _nx_secure_tls_proc_clienthello_sec_sa_extension                    */
/*                                          Process ECC extensions        */
/*    _nx_secure_tls_check_ciphersuite      Check if ECC suite is usable  */
/*    _nx_secure_tls_remote_certificate_free_all                          */
/*                                          Free all remote certificates  */
/*    [_nx_secure_tls_session_renegotiate_callback_set]                   */
/*                                          Renegotiation callback        */
/*    [_nx_secure_tls_session_server_callback_set]                        */
/*                                          Server session callback       */
/*    _nx_secure_x509_local_device_certificate_get                        */
/*                                          Get the local certificate     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_server_handshake       TLS Server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), added    */
/*                                            priority ciphersuite logic, */
/*                                            verified memcpy use cases,  */
/*                                            fixed renegotiation bug,    */
/*                                            improved negotiation logic, */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Timothy Stapko           Modified comment(s), added    */
/*                                            ability to disable client   */
/*                                            initiated renegotiation,    */
/*                                            resulting in version 6.1.9  */
/*  04-25-2022     Yuxin Zhou               Modified comment(s),  fixed   */
/*                                            the bug of processing       */
/*                                            extensions,                 */
/*                                            resulting in version 6.1.11 */
/*  10-31-2022     Yanwu Cai                Modified comment(s), fixed    */
/*                                            TLS 1.3 version negotiation,*/
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_clienthello(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                        UINT message_length)
{
UINT                                  length;
USHORT                                ciphersuite_list_length;
UCHAR                                 compression_methods_length;
USHORT                                cipher_entry;
UCHAR                                 session_id_length;
UINT                                  i;
UINT                                  status;
USHORT                                protocol_version;
USHORT                                newest_version;
UINT                                  total_extensions_length;
const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite_info;
USHORT                                ciphersuite_priority;
USHORT                                new_ciphersuite_priority = 0;
NX_SECURE_TLS_HELLO_EXTENSION         extension_data[NX_SECURE_TLS_HELLO_EXTENSIONS_MAX];
UINT                                  num_extensions = NX_SECURE_TLS_HELLO_EXTENSIONS_MAX;
UCHAR                                *ciphersuite_list;
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
NX_SECURE_X509_CERT                  *cert;
UINT                                  selected_curve;
UINT                                  cert_curve;
UINT                                  cert_curve_supported;
NX_SECURE_EC_PUBLIC_KEY              *ec_pubkey;
NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA   *ecdhe_data;
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
USHORT                                tls_1_3 = tls_session -> nx_secure_tls_1_3;
USHORT                                no_extension = NX_FALSE;
#endif

    /* Structure of ClientHello:
     * |     2       |          4 + 28          |    1       |   <SID len>  |   2    | <CS Len>     |    1    | <Comp Len>  |    2    | <Ext. Len> |
     * | TLS version |  Random (time + random)  | SID length |  Session ID  | CS Len | Ciphersuites |Comp Len | Compression |Ext. Len | Extensions |
     */

    if (message_length < 38)
    {
        /* Message was not the minimum required size for a ClientHello. */
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Use our length as an index into the buffer. */
    length = 0;

    /* If we are currently in a session, we have a renegotiation handshake. */
    if (tls_session -> nx_secure_tls_local_session_active)
    {
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        if (tls_session->nx_secure_tls_1_3 == NX_TRUE)
        {

            /* RFC 8446, section 4.1.2, page 27.
             * Server has negotiated TLS 1.3 and receives a ClientHello again.
             * Send an unexpected message alert. */
            return(NX_SECURE_TLS_UNEXPECTED_CLIENTHELLO);
        }
#endif

#if !defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION) 
        /* If Client initiated renegotiation is enabled, handle the renegotiation 
           handshake if appropriate. Otherwise return "no renegotiation allowed" error. */
        if (tls_session -> nx_secure_tls_renegotation_enabled && tls_session -> nx_secure_tls_secure_renegotiation 
#if defined(NX_SECURE_TLS_DISABLE_CLIENT_INITIATED_RENEGOTIATION)
            /* If client initiated renegotiation is disabled for TLS servers, only allow 
               server-initiated renegotiations requested by the server itself. */
            && tls_session -> nx_secure_tls_server_renegotiation_requested
#endif
           )
        {
            tls_session -> nx_secure_tls_renegotiation_handshake = NX_TRUE;

            /* On a session resumption free all certificates for the new session.
             * SESSION RESUMPTION: if session resumption is enabled, don't free!!
             */
            status = _nx_secure_tls_remote_certificate_free_all(tls_session);

            if (status != NX_SUCCESS)
            {
                return(status);
            }

            /* Invoke user callback to notify application of renegotiation request. */
            if (tls_session -> nx_secure_tls_session_renegotiation_callback != NX_NULL)
            {
                status = tls_session -> nx_secure_tls_session_renegotiation_callback(tls_session);

                if (status != NX_SUCCESS)
                {
                    return(status);
                }
            }

            /* We don't want any more clienthellos to come in after this - only the first one
               in response to our original request. */
            tls_session -> nx_secure_tls_server_renegotiation_requested = NX_FALSE; 
    }
        else
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */
        {
            /* Session renegotiation is disabled, so this is an error! */
            return(NX_SECURE_TLS_NO_RENEGOTIATION_ERROR);
        }
    }

    /* Client is establishing a TLS session with our server. */
    /* Extract the protocol version - only part of the ClientHello message. */
    protocol_version = (USHORT)((packet_buffer[length] << 8) | packet_buffer[length + 1]);
    length += 2;

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if (tls_session->nx_secure_tls_1_3 == NX_FALSE)
#endif
    {

        /* Check protocol version provided by client. */
        status = _nx_secure_tls_check_protocol_version(tls_session, protocol_version, NX_SECURE_TLS);

        if (status != NX_SECURE_TLS_SUCCESS)
        {
            /* If we have an active session, this is a renegotiation attempt, treat the protocol error as
               if we are starting a new session. */
            if (status == NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION || tls_session -> nx_secure_tls_local_session_active)
            {
                /* If the version isn't supported, it's not an issue - TLS is backward-compatible,
                 * so negotiate the highest supported version. If the version isn't recognized,
                 * flag an error. */
                _nx_secure_tls_highest_supported_version_negotiate(tls_session, &protocol_version, NX_SECURE_TLS);

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
    }

    /* Assign our protocol version to our socket. This is used for all further communications
     * in this session. */
    tls_session -> nx_secure_tls_protocol_version = protocol_version;

    /* Save off the random value for key generation later. */
    NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_key_material.nx_secure_tls_client_random, &packet_buffer[length], NX_SECURE_TLS_RANDOM_SIZE); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */
    length += NX_SECURE_TLS_RANDOM_SIZE;

    /* Extract the session ID if there is one. */
    session_id_length = packet_buffer[length];
    length++;

    if ((length + session_id_length) > message_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* If there is a session ID, copy it into our TLS socket structure. */
    tls_session -> nx_secure_tls_session_id_length = session_id_length;
    if (session_id_length > 0)
    {
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_session_id, &packet_buffer[length], session_id_length); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */
        length += session_id_length;
    }

    /* Negotiate the ciphersuite we want to use. */
    ciphersuite_list_length = (USHORT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
    length += 2;

    /* Make sure the list length makes sense. */
    if (ciphersuite_list_length < 2 || (length + ciphersuite_list_length) > message_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    ciphersuite_list = &packet_buffer[length];

    length += ciphersuite_list_length;

    /* Compression methods length - one byte. For now we only support the NULL method. */
    compression_methods_length = packet_buffer[length];
    length++;

    /* Message length overflow. */
    if ((length + compression_methods_length) > message_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Make sure NULL compression method is supported. */
    status = NX_SECURE_TLS_BAD_COMPRESSION_METHOD;
    for (i = 0; i < compression_methods_length; ++i)
    {
        if (packet_buffer[length + i] == 0x0)
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

    length += compression_methods_length;

    /* Padding data? */
    if (message_length >= (length + 2))
    {

        /* TLS Extensions come next. Get the total length of all extensions first. */
        total_extensions_length = (USHORT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
        length += 2;

        /* Message length overflow. */
        if ((length + total_extensions_length) > message_length)
        {
            return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
        }

        if (total_extensions_length > 0)
        {
            /* Process serverhello extensions. */
            status = _nx_secure_tls_process_clienthello_extensions(tls_session, &packet_buffer[length], total_extensions_length, extension_data, &num_extensions, packet_buffer, message_length);

            /* Check for error. */
            if (status != NX_SUCCESS)
            {
                return(status);
            }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
            if (tls_session -> nx_secure_tls_1_3 != tls_1_3)
            {

                /* Negotiate a version of TLS prior to TLS 1.3. */
                return(status);
            }
#endif

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
        else
        {
            num_extensions = 0;
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
            no_extension = NX_TRUE;
#endif
        }
    }
    else
    {
        num_extensions = 0;
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        no_extension = NX_TRUE;
#endif
    }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if ((tls_session -> nx_secure_tls_1_3) && (no_extension == NX_TRUE))
    {

        /* Negotiate a version of TLS prior to TLS 1.3. */
        if (tls_session -> nx_secure_tls_protocol_version_override == 0)
        {
            tls_session -> nx_secure_tls_1_3 = NX_FALSE;
#if !defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION)
            tls_session -> nx_secure_tls_renegotation_enabled = NX_TRUE;
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */

            return(NX_SUCCESS);
        }
        else
        {

            /* Protocol version is overridden to TLS 1.3. */
            return(NX_SECURE_TLS_UNSUPPORTED_TLS_VERSION);
        }
    }
#endif

#if !defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION) 
    if ((tls_session -> nx_secure_tls_renegotiation_handshake) && (!tls_session -> nx_secure_tls_secure_renegotiation_verified))
    {

        /* The server did not receive the "renegotiation_info" extension, the handshake must be aborted. */
        return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
    }

    tls_session -> nx_secure_tls_secure_renegotiation_verified = NX_FALSE;
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */

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

    /* Check for error. */
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    ecdhe_data -> nx_secure_tls_ecdhe_named_curve = selected_curve;

    /* Check if certificate curve is supported. */
    if (cert != NX_NULL && cert -> nx_secure_x509_public_algorithm == NX_SECURE_TLS_X509_TYPE_EC)
    {
        if (cert_curve_supported == NX_FALSE)
        {
            /* The named curve in our server certificate is not supported by the client. */
            return(NX_SECURE_TLS_NO_SUPPORTED_CIPHERS);
        }
    }
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */

    /* Set our initial priority to the maximum value - the size of our ciphersuite crypto table. */
    ciphersuite_priority = (USHORT)(0xFFFFFFFF);

    for (i = 0; i < ciphersuite_list_length; i += 2)
    {
        /* Loop through list of acceptable ciphersuites. */
        cipher_entry = (USHORT)((ciphersuite_list[i] << 8) + ciphersuite_list[i + 1]);

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        if ((tls_session -> nx_secure_tls_1_3 == NX_TRUE) ^ (ciphersuite_list[i] == 0x13))
        {

            /* Ciphersuites for TLS 1.3 can not be used by TLS 1.2. */
            continue;
        }
#endif

        /* Look up the ciphersuite in our crypto table, then check the priority of the chosen ciphersuite. */
        status = _nx_secure_tls_ciphersuite_lookup(tls_session, cipher_entry, &ciphersuite_info, &new_ciphersuite_priority);

        /* Save the first ciphersuite we find - assume cipher table is in priority order. */
        if ((status == NX_SUCCESS) && (new_ciphersuite_priority < ciphersuite_priority))
        {
#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
            if (NX_SUCCESS == _nx_secure_tls_check_ciphersuite(ciphersuite_info, cert, selected_curve, cert_curve_supported))
#endif /* NX_SECURE_ENABLE_ECC_CIPHERSUITE */
            {
                /* Save the ciphersuite and its priority but continue processing the entire list to see if there
                   is a better ciphersuite available. */
                tls_session -> nx_secure_tls_session_ciphersuite = ciphersuite_info;
                ciphersuite_priority = new_ciphersuite_priority;
            }
        }

#if !defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION) 
        if (cipher_entry == TLS_EMPTY_RENEGOTIATION_INFO_SCSV)
        {
            if (tls_session -> nx_secure_tls_renegotiation_handshake)
            {

                /* When secure renegotiating, the server MUST verify that it
                   does not contain the TLS_EMPTY_RENEGOTIATION_INFO_SCSV SCSV. */
                return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
            }
            else
            {

                /* Secure Renegotiation signalling ciphersuite value was encountered.
                   This indicates that the Client supports secure renegotiation. */
                tls_session -> nx_secure_tls_secure_renegotiation = NX_TRUE;
            }
        }
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */


        /* Check for the fallback notification SCSV. */
        if (cipher_entry == TLS_FALLBACK_NOTIFY_SCSV)
        {

            /* A fallback is indicated by the Client, check the TLS version. */
            _nx_secure_tls_newest_supported_version(tls_session, &newest_version, NX_SECURE_TLS);

            if (protocol_version != newest_version)
            {
                return(NX_SECURE_TLS_INAPPROPRIATE_FALLBACK);
            }
        }
        /* Continue searching client ciphersuite list to get highest-priority selection. */
    }

#if !defined(NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION) && defined(NX_SECURE_TLS_REQUIRE_RENEGOTIATION_EXT)
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if (!tls_session -> nx_secure_tls_1_3)
#endif /* NX_SECURE_TLS_TLS_1_3_ENABLED */
    {
        if ((tls_session -> nx_secure_tls_renegotation_enabled) && (!tls_session -> nx_secure_tls_secure_renegotiation))
        {

            /* No "renegotiation_info" extension present, some servers may want to terminate the handshake. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }
    }
#endif /* !NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION && NX_SECURE_TLS_REQUIRE_RENEGOTIATION_EXT */

    /* See if we found an acceptable ciphersuite. */
    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* No supported ciphersuites found. */
        return(NX_SECURE_TLS_NO_SUPPORTED_CIPHERS);
    }

#ifdef NX_SECURE_TLS_SERVER_DISABLED
    /* If TLS Server is disabled and we have processed a ClientHello, something is wrong... */
    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_ERROR;
    return(NX_SECURE_TLS_INVALID_STATE);
#else
    return(NX_SUCCESS);
#endif
}

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_check_ciphersuite                    PORTABLE C      */
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
/*    _nx_secure_tls_server_handshake       TLS Server state machine      */
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
static UINT _nx_secure_tls_check_ciphersuite(const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite_info,
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
        if (selected_curve == 0 || cert == NX_NULL)
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
        if (selected_curve == 0)
        {
            /* No common named curve supported supported. */
            return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
        }

        /* Check for ECDH_anon. */
        if (ciphersuite_info -> nx_secure_tls_public_auth -> nx_crypto_algorithm != NX_CRYPTO_DIGITAL_SIGNATURE_ANONYMOUS)
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
