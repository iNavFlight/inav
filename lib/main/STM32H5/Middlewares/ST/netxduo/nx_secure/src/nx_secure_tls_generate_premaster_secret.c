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
/*    _nx_secure_tls_generate_premaster_secret            PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the Pre-Master Secret for TLS Client        */
/*    instances. It is sent to the remote host and used as the seed for   */
/*    session key generation.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    id                                    TLS or DTLS                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_protocol_version_get   Get current TLS version to use*/
/*    [nx_secure_generate_premaster_secret] Generate pre-master secret    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_process_client_key_exchange                          */
/*                                          Process ClientKeyExchange     */
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
/*  04-25-2022     Yuxin Zhou               Modified comment(s), removed  */
/*                                            internal unreachable logic, */
/*                                            resulting in version 6.1.11 */
/*  10-31-2022     Yanwu Cai                Modified comment(s), added    */
/*                                            custom secret generation,   */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_generate_premaster_secret(NX_SECURE_TLS_SESSION *tls_session, UINT id)
{
UINT   status;
USHORT protocol_version;

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    _nx_secure_tls_protocol_version_get(tls_session, &protocol_version, id);

#ifdef NX_SECURE_ENABLE_ECC_CIPHERSUITE
    status = tls_session -> nx_secure_generate_premaster_secret(tls_session -> nx_secure_tls_session_ciphersuite, protocol_version,
                                                                &tls_session -> nx_secure_tls_key_material, &tls_session -> nx_secure_tls_credentials,
                                                                tls_session -> nx_secure_tls_socket_type,
                                                                &tls_session -> nx_secure_tls_received_remote_credentials, tls_session -> nx_secure_public_cipher_metadata_area,
                                                                tls_session -> nx_secure_public_cipher_metadata_size, &tls_session -> nx_secure_tls_ecc);
#else
    status = tls_session -> nx_secure_generate_premaster_secret(tls_session -> nx_secure_tls_session_ciphersuite, protocol_version,
                                                                &tls_session -> nx_secure_tls_key_material, &tls_session -> nx_secure_tls_credentials,
                                                                tls_session -> nx_secure_tls_socket_type,
                                                                &tls_session -> nx_secure_tls_received_remote_credentials, tls_session -> nx_secure_public_cipher_metadata_area,
                                                                tls_session -> nx_secure_public_cipher_metadata_size, NX_NULL);

#endif

    return(status);
}

