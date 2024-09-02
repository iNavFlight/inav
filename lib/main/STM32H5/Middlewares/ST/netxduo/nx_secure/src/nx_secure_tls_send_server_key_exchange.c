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
/*    _nx_secure_tls_send_server_key_exchange             PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates a ServerKeyExchange message, which is used  */
/*    when the chosen ciphersuite requires additional information for key */
/*    generation, such as when using Diffie-Hellman ciphers.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    send_packet                           Packet used to send message   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_secure_generate_server_key_exchange]                            */
/*                                          Generate ServerKeyExchange    */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Yanwu Cai                Modified comment(s), added    */
/*                                            custom secret generation,   */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_send_server_key_exchange(NX_SECURE_TLS_SESSION *tls_session,
                                             NX_PACKET *send_packet)
{
#ifndef NX_SECURE_TLS_SERVER_DISABLED
ULONG                                 length;
const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite;
UINT                                  status;
VOID                                 *tls_ecc_curves = NX_NULL;
UCHAR                                 tls_1_3 = 0;



    /* Build up the server key exchange message. Structure:
     * |        2        |  <key data length>  |
     * | Key data length |  Key data (opaque)  |
     */

    /* Figure out which ciphersuite we are using. */
    ciphersuite = tls_session -> nx_secure_tls_session_ciphersuite;
    if (ciphersuite == NX_NULL)
    {
        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    length = 0;

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    tls_ecc_curves = &tls_session -> nx_secure_tls_ecc;
#endif

#if (NX_SECURE_TLS_TLS_1_3_ENABLED)
    tls_1_3 = tls_session -> nx_secure_tls_1_3;
#endif

    status = tls_session -> nx_secure_generate_server_key_exchange(ciphersuite, tls_session -> nx_secure_tls_protocol_version, tls_1_3,
                                                                   tls_session -> nx_secure_tls_crypto_table, &tls_session -> nx_secure_tls_handshake_hash,
                                                                   &tls_session -> nx_secure_tls_key_material, &tls_session -> nx_secure_tls_credentials,
                                                                   send_packet -> nx_packet_append_ptr,
                                                                   (ULONG)(send_packet -> nx_packet_data_end) - (ULONG)(send_packet -> nx_packet_append_ptr),
                                                                   &length, tls_session -> nx_secure_public_cipher_metadata_area,
                                                                   tls_session -> nx_secure_public_cipher_metadata_size,
                                                                   tls_session -> nx_secure_public_auth_metadata_area,
                                                                   tls_session -> nx_secure_public_auth_metadata_size,
                                                                   tls_ecc_curves);

    if (status)
    {

        return(status);
    }


    /* Finally, we have a complete length and can adjust our packet accordingly. */
    send_packet -> nx_packet_append_ptr = send_packet -> nx_packet_append_ptr + length;
    send_packet -> nx_packet_length = send_packet -> nx_packet_length + length;

    return(NX_SECURE_TLS_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(tls_session);
    NX_PARAMETER_NOT_USED(send_packet);
    return(NX_SECURE_TLS_INVALID_STATE);
#endif
}

