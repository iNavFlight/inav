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

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
/* Defined in nx_secure_tls_send_serverhello.c */
extern const UCHAR _nx_secure_tls_hello_retry_request_random[32];
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_process_serverhello                  PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function processes an incoming ServerHello message, which is   */
/*    the response to a TLS ClientHello coming from this host. The        */
/*    ServerHello message contains the desired ciphersuite and data used  */
/*    in the key generation process later in the handshake.               */
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
/*    _nx_secure_tls_check_protocol_version Checking incoming TLS version */
/*    _nx_secure_tls_ciphersuite_lookup     Lookup current ciphersuite    */
/*    _nx_secure_tls_process_serverhello_extensions                       */
/*                                          Process ServerHello extensions*/
/*    [nx_secure_tls_session_client_callback                              */
/*                                          Client session callback       */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s), added    */
/*                                            priority ciphersuite logic, */
/*                                            verified memcpy use cases,  */
/*                                            fixed renegotiation bug,    */
/*                                            resulting in version 6.1    */
/*  10-15-2021     Timothy Stapko           Modified comment(s), fixed    */
/*                                            TLS 1.3 compilation issue,  */
/*                                            resulting in version 6.1.9  */
/*  10-31-2022     Yanwu Cai                Modified comment(s), fixed    */
/*                                            TLS 1.3 version negotiation,*/
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_serverhello(NX_SECURE_TLS_SESSION *tls_session, UCHAR *packet_buffer,
                                        UINT message_length)
{
#ifndef NX_SECURE_TLS_CLIENT_DISABLED
UINT                                  length;
UCHAR                                 compression_method;
USHORT                                version, total_extensions_length;
UINT                                  status;
USHORT                                ciphersuite;
USHORT                                ciphersuite_priority;
NX_SECURE_TLS_HELLO_EXTENSION         extension_data[NX_SECURE_TLS_HELLO_EXTENSIONS_MAX];
UINT                                  num_extensions;
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
USHORT                                tls_1_3 = tls_session -> nx_secure_tls_1_3;
USHORT                                no_extension = NX_FALSE;
NX_SECURE_TLS_SERVER_STATE            old_client_state = tls_session -> nx_secure_tls_client_state;

    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_IDLE;
#endif

    /* Parse the ServerHello message.
     * Structure:
     * |     2       |          4 + 28          |    1       |   <SID len>  |      2      |      1      |    2     | <Ext. Len> |
     * | TLS version |  Random (time + random)  | SID length |  Session ID  | Ciphersuite | Compression | Ext. Len | Extensions |
     */

    if (message_length < 38)
    {
        /* Message was not the minimum required size for a ServerHello. */
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Use our length as an index into the buffer. */
    length = 0;

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if (tls_session -> nx_secure_tls_1_3 && tls_session -> nx_secure_tls_local_session_active)
    {

        /* Client has negotiated TLS 1.3 and receives a ServerHello again.
         * Send an unexpected message alert. */
        return(NX_SECURE_TLS_UNEXPECTED_MESSAGE);
    }
#endif

    /* First two bytes of the server hello following the header are the TLS major and minor version numbers. */
    version = (USHORT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
    length += 2;
    /* Verify the version coming from the server. */
    status = _nx_secure_tls_check_protocol_version(tls_session, version, NX_SECURE_TLS);
    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Set the protocol version to whatever the Server negotiated - we have checked that
       we support this version in the call above, so it's fine to continue. */
    tls_session -> nx_secure_tls_protocol_version = version;

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if (tls_session -> nx_secure_tls_1_3 &&
        (NX_SECURE_MEMCMP(_nx_secure_tls_hello_retry_request_random, &packet_buffer[length], NX_SECURE_TLS_RANDOM_SIZE) == 0))
    {

        /* A HelloRetryRequest is received. */
        if (old_client_state == NX_SECURE_TLS_CLIENT_STATE_HELLO_RETRY)
        {

            /* A second HelloRetryRequest is received. */
            return(NX_SECURE_TLS_UNRECOGNIZED_MESSAGE_TYPE);
        }
        else
        {
            tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_HELLO_RETRY;
        }
    }
    else
#endif
    {
            
        /* Set the Server random data, used in key generation. First 4 bytes is GMT time. */
        NX_SECURE_MEMCPY(&tls_session -> nx_secure_tls_key_material.nx_secure_tls_server_random[0], &packet_buffer[length], NX_SECURE_TLS_RANDOM_SIZE); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */
    }
    length += NX_SECURE_TLS_RANDOM_SIZE;

    /* Session ID length is one byte. */
    tls_session -> nx_secure_tls_session_id_length = packet_buffer[length];
    length++;

    if ((length + tls_session -> nx_secure_tls_session_id_length) > message_length)
    {
        return(NX_SECURE_TLS_INCORRECT_MESSAGE_LENGTH);
    }

    /* Session ID follows. */
    if (tls_session -> nx_secure_tls_session_id_length > 0)
    {
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_session_id, &packet_buffer[length], tls_session -> nx_secure_tls_session_id_length); /* Use case of memcpy is verified.  lgtm[cpp/banned-api-usage-required-any] */
        length += tls_session -> nx_secure_tls_session_id_length;
    }

    /* Finally, the chosen ciphersuite - this is selected by the server from the list we provided in the ClientHello. */
    ciphersuite = (USHORT)((packet_buffer[length] << 8) + packet_buffer[length + 1]);
    length += 2;

    /* Find out the ciphersuite info of the chosen ciphersuite. */
    status = _nx_secure_tls_ciphersuite_lookup(tls_session, ciphersuite, &tls_session -> nx_secure_tls_session_ciphersuite, &ciphersuite_priority);
    if (status != NX_SUCCESS)
    {
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        if (tls_session -> nx_secure_tls_1_3)
        {
            return(NX_SECURE_TLS_1_3_UNKNOWN_CIPHERSUITE);
        }
#endif

        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Compression method - for now this should be NULL. */
    compression_method = packet_buffer[length];

    /* There are no supported compression methods, so non-zero is an error. */
    if (compression_method != 0x00)
    {
        return(NX_SECURE_TLS_BAD_COMPRESSION_METHOD);
    }
    length++;

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
            status = _nx_secure_tls_process_serverhello_extensions(tls_session, &packet_buffer[length], total_extensions_length, extension_data, &num_extensions);

            /* Check for error. */
            if (status != NX_SUCCESS)
            {
                return(status);
            }

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
            if (tls_session -> nx_secure_tls_1_3 != tls_1_3)
            {

                /* Server negotiates a version of TLS prior to TLS 1.3. */
                return(status);
            }
#endif

            /* If the server callback is set, invoke it now with the extensions that require application input. */
            if (tls_session -> nx_secure_tls_session_client_callback != NX_NULL)
            {
                status = tls_session -> nx_secure_tls_session_client_callback(tls_session, extension_data, num_extensions);

                /* Check for error. */
                if (status != NX_SUCCESS)
                {
                    return(status);
                }
            }
        }
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
        else
        {
            no_extension = NX_TRUE;
        }
#endif
    }
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    else
    {
        no_extension = NX_TRUE;
    }

    if ((tls_session -> nx_secure_tls_1_3) && (no_extension == NX_TRUE))
    {

        /* Server negotiates a version of TLS prior to TLS 1.3. */
        if (tls_session -> nx_secure_tls_protocol_version_override == 0)
        {
            tls_session -> nx_secure_tls_1_3 = NX_FALSE;
#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
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

#ifndef NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION
#ifdef NX_SECURE_TLS_REQUIRE_RENEGOTIATION_EXT
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if (!tls_session -> nx_secure_tls_1_3)
#endif /* NX_SECURE_TLS_TLS_1_3_ENABLED */
    {
        if ((tls_session -> nx_secure_tls_renegotation_enabled) && (!tls_session -> nx_secure_tls_secure_renegotiation))
        {

            /* No "renegotiation_info" extension present, some clients may want to terminate the handshake. */
            return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
        }
    }
#endif /* NX_SECURE_TLS_REQUIRE_RENEGOTIATION_EXT */

    if ((tls_session -> nx_secure_tls_local_session_active) && (!tls_session -> nx_secure_tls_secure_renegotiation_verified))
    {

        /* The client did not receive the "renegotiation_info" extension, the handshake must be aborted. */
        return(NX_SECURE_TLS_RENEGOTIATION_EXTENSION_ERROR);
    }

    tls_session -> nx_secure_tls_secure_renegotiation_verified = NX_FALSE;
#endif /* NX_SECURE_TLS_DISABLE_SECURE_RENEGOTIATION */

#ifdef NX_SECURE_TLS_CLIENT_DISABLED
    /* If TLS Client is disabled and we have processed a ServerHello, something is wrong... */
    tls_session -> nx_secure_tls_server_state = NX_SECURE_TLS_SERVER_STATE_ERROR;

    return(NX_SECURE_TLS_INVALID_STATE);
#else

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    if ((tls_session -> nx_secure_tls_1_3) && (old_client_state == NX_SECURE_TLS_CLIENT_STATE_IDLE))
    {

        /* We have selected a ciphersuite so now we can initialize the handshake hash. */
        status = _nx_secure_tls_handshake_hash_init(tls_session);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
    }

    if (tls_session -> nx_secure_tls_client_state != NX_SECURE_TLS_CLIENT_STATE_HELLO_RETRY)
#endif
    {
        
        /* Set our state to indicate we sucessfully parsed the ServerHello. */
        tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_SERVERHELLO;
    }

    return(NX_SUCCESS);
#endif
#else /* NX_SECURE_TLS_SERVER_DISABLED */
    /* If Server TLS is disabled and we recieve a serverhello, error! */    
    NX_PARAMETER_NOT_USED(tls_session);
    NX_PARAMETER_NOT_USED(packet_buffer);
    NX_PARAMETER_NOT_USED(message_length);
    
    return(NX_SECURE_TLS_UNEXPECTED_MESSAGE);
#endif    
}

