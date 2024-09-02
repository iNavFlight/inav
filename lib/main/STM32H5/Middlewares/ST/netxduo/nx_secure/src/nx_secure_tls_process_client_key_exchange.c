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
/*    _nx_secure_tls_process_client_key_exchange          PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
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
/*    tls_session                           TLS control block             */
/*    packet_buffer                         Pointer to message data       */
/*    message_length                        Length of message data (bytes)*/
/*    id                                    TLS or DTLS                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_secure_process_client_key_exchange]                             */
/*                                          Process ClientKeyExchange     */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_tls_server_handshake       TLS server state machine      */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), update   */
/*                                            ECC find curve method,      */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  06-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            supported hardware EC       */
/*                                            private key,                */
/*                                            resulting in version 6.1.7  */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022     Yanwu Cai                Modified comment(s), added    */
/*                                            custom secret generation,   */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_process_client_key_exchange(NX_SECURE_TLS_SESSION *tls_session,
                                                UCHAR *packet_buffer, UINT message_length, UINT id)
{
#ifndef NX_SECURE_TLS_SERVER_DISABLED
UINT status;

    NX_PARAMETER_NOT_USED(id);

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    /* Process key material. The contents of the handshake record differ according to the
       ciphersuite chosen in the Client/Server Hello negotiation. */
    status = tls_session -> nx_secure_process_client_key_exchange(tls_session -> nx_secure_tls_session_ciphersuite, tls_session -> nx_secure_tls_protocol_version,
                                                                  packet_buffer, message_length, &tls_session -> nx_secure_tls_received_remote_credentials, &tls_session -> nx_secure_tls_key_material,
                                                                  &tls_session -> nx_secure_tls_credentials, tls_session -> nx_secure_public_cipher_metadata_area,
                                                                  tls_session -> nx_secure_public_cipher_metadata_size,
                                                                  tls_session -> nx_secure_public_auth_metadata_area,
                                                                  tls_session -> nx_secure_public_auth_metadata_size,
                                                                  &tls_session -> nx_secure_tls_ecc);

#else

    /* Process key material. The contents of the handshake record differ according to the
       ciphersuite chosen in the Client/Server Hello negotiation. */
    status = tls_session -> nx_secure_process_client_key_exchange(tls_session -> nx_secure_tls_session_ciphersuite, tls_session -> nx_secure_tls_protocol_version,
                                                                  packet_buffer, message_length, &tls_session -> nx_secure_tls_received_remote_credentials, &tls_session -> nx_secure_tls_key_material,
                                                                  &tls_session -> nx_secure_tls_credentials, tls_session -> nx_secure_public_cipher_metadata_area,
                                                                  tls_session -> nx_secure_public_cipher_metadata_size,
                                                                  tls_session -> nx_secure_public_auth_metadata_area,
                                                                  tls_session -> nx_secure_public_auth_metadata_size,
                                                                  NX_NULL);
#endif

    return(status);

#else

    NX_PARAMETER_NOT_USED(packet_buffer);
    NX_PARAMETER_NOT_USED(message_length);
    NX_PARAMETER_NOT_USED(id);

    /* If TLS Server is disabled and we have processed a ClientKeyExchange, something is wrong... */
    tls_session -> nx_secure_tls_client_state = NX_SECURE_TLS_CLIENT_STATE_ERROR;
    return(NX_SECURE_TLS_INVALID_STATE);

#endif
}

