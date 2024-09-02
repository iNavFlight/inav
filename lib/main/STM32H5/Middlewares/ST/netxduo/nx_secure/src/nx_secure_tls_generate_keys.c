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
/**    Transport Layer Security (TLS) - Generate Session Keys             */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_tls.h"
#ifdef NX_SECURE_ENABLE_DTLS
#include "nx_secure_dtls.h"
#endif /* NX_SECURE_ENABLE_DTLS */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_generate_keys                        PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the session keys used by TLS to encrypt     */
/*    the data being transmitted. It uses data gathered during the TLS    */
/*    handshake to generate a block of "key material" that is split into  */
/*    the various keys needed for each session.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_secure_generate_master_secret]    Generate master secrets       */
/*    [nx_secure_generate_session_keys]     Generate session keys         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_client_handshake      DTLS client state machine     */
/*    _nx_secure_dtls_server_handshake      DTLS server state machine     */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
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
UINT _nx_secure_tls_generate_keys(NX_SECURE_TLS_SESSION *tls_session)
{
UCHAR                                *pre_master_sec;
UINT                                  pre_master_sec_size;
UCHAR                                *master_sec;
UINT                                  status;
const NX_CRYPTO_METHOD               *session_prf_method = NX_NULL;
const NX_SECURE_TLS_CIPHERSUITE_INFO *ciphersuite;

    /* Generate the session keys using the parameters obtained in the handshake.
       By this point all the information needed to generate the TLS session key
       material should have been gathered and stored in the TLS socket structure. */

    /* Generate the Master Secret from the Pre-Master Secret.
       From the RFC (TLS 1.1, TLS 1.2):

        master_secret = PRF(pre_master_secret, "master secret",
                        ClientHello.random + ServerHello.random) [0..47];

        The master secret is always exactly 48 bytes in length.  The length
        of the premaster secret will vary depending on key exchange method.
     */

    /* Figure out which ciphersuite we are using. */
    ciphersuite = tls_session -> nx_secure_tls_session_ciphersuite;
    if (ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    pre_master_sec = tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret;
    pre_master_sec_size = tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret_size;
    master_sec = tls_session -> nx_secure_tls_key_material.nx_secure_tls_master_secret;


#if (NX_SECURE_TLS_TLS_1_2_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_2)
#else
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_2)
#endif /* NX_SECURE_ENABLE_DTLS */
    {
        /* For TLS 1.2, the PRF is defined by the ciphersuite. However, if we are using an older ciphersuite,
            * default to the TLS 1.2 default PRF, which uses SHA-256-HMAC. */
        session_prf_method = ciphersuite -> nx_secure_tls_prf;
    }

#endif
#if (NX_SECURE_TLS_TLS_1_0_ENABLED || NX_SECURE_TLS_TLS_1_1_ENABLED)
#ifdef NX_SECURE_ENABLE_DTLS
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_DTLS_VERSION_1_0)
#else
    if (tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_0 ||
        tls_session -> nx_secure_tls_protocol_version == NX_SECURE_TLS_VERSION_TLS_1_1)
#endif /* NX_SECURE_ENABLE_DTLS */
    {
        /* TLS 1.0 and TLS 1.1 use the same PRF. */
        session_prf_method = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_prf_1_method;
    }
#endif

    status = tls_session -> nx_secure_generate_master_secret(ciphersuite, tls_session -> nx_secure_tls_protocol_version, session_prf_method,
                                                             &tls_session -> nx_secure_tls_key_material, pre_master_sec, pre_master_sec_size,
                                                             master_sec, tls_session -> nx_secure_tls_prf_metadata_area,
                                                             tls_session -> nx_secure_tls_prf_metadata_size);

    if (status != NX_SECURE_TLS_SUCCESS)
    {

        return(status);
    }

    /* Clear out the Pre-Master Secret (we don't need it anymore and keeping it in memory is dangerous). */
#ifdef NX_SECURE_KEY_CLEAR
    NX_SECURE_MEMSET(pre_master_sec, 0x0, sizeof(tls_session -> nx_secure_tls_key_material.nx_secure_tls_pre_master_secret));
#endif /* NX_SECURE_KEY_CLEAR  */


    status = tls_session -> nx_secure_generate_session_keys(ciphersuite, tls_session -> nx_secure_tls_protocol_version, session_prf_method,
                                                            &tls_session -> nx_secure_tls_key_material, master_sec, tls_session -> nx_secure_tls_prf_metadata_area,
                                                            tls_session -> nx_secure_tls_prf_metadata_size);
    return(status);
}

