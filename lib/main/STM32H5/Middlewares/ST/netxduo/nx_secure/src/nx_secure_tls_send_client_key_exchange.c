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
/*    _nx_secure_tls_send_client_key_exchange             PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function encrypts the Pre-Master Secret (generated earlier)    */
/*    and populates an NX_PACKET with the complete ClientKeyExchange      */
/*    message (to be sent by the caller). It also will send ephemeral     */
/*    keys for ciphersuites that require them.                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Outgoing TLS packet           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_secure_generate_client_key_exchange]                            */
/*                                          Generate ClientKeyExchange    */
/*    _nx_secure_tls_remote_certificate_free_all                          */
/*                                          Free all remote certificates  */
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
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  04-25-2022     Zhen Kong                Modified comment(s), improved */
/*                                            internal logic to check data*/
/*                                            size and then improved code */
/*                                            coverage, resulting in      */
/*                                            version 6.1.11              */
/*  10-31-2022     Yanwu Cai                Modified comment(s), added    */
/*                                            custom secret generation,   */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_client_key_exchange(NX_SECURE_TLS_SESSION *tls_session,
                                             NX_PACKET *send_packet)
{
#if !defined(NX_SECURE_TLS_CLIENT_DISABLED)
UINT  status;
ULONG data_size = 0;
ULONG buffer_length;

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    buffer_length = (ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr);

    status = tls_session -> nx_secure_generate_client_key_exchange(tls_session -> nx_secure_tls_session_ciphersuite,
                                                                   &tls_session -> nx_secure_tls_key_material, &tls_session -> nx_secure_tls_credentials,
                                                                   send_packet -> nx_packet_append_ptr,
                                                                   buffer_length,
                                                                   &data_size, tls_session -> nx_secure_public_cipher_metadata_area,
                                                                   tls_session -> nx_secure_public_cipher_metadata_size,
                                                                   tls_session -> nx_secure_public_auth_metadata_area,
                                                                   tls_session -> nx_secure_public_auth_metadata_size);
    if (status)
    {
        _nx_secure_tls_remote_certificate_free_all(tls_session);
        return(status);
    }

    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + data_size;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + data_size;

    return(NX_SECURE_TLS_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(tls_session);
    NX_PARAMETER_NOT_USED(send_packet);
    return(NX_SECURE_TLS_INVALID_STATE);
#endif
}

