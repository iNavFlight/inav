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
extern NX_SECURE_TLS_ECC _nx_secure_tls_ecc_info;
#endif

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_1_3_crypto_init                      PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    TLS 1.3 introduces the concept of a partially-encrypted handshake,  */
/*    utilizing cryptographic primitives sent in the initial ClientHello  */
/*    message. In order to properly handle these primitives, certain      */
/*    initialization must be done prior to sending the ClientHello        */
/*    message. For example, if ECDHE is an option supported by the client,*/
/*    the ECC public key must be generated before the ClientHello is      */
/*    generated and sent. All pre-handshake initialization of that nature */
/*    for TLS 1.3 should be done here.                                    */
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
/*  10-31-2022     Yanwu Cai                Modified comment(s),          */
/*                                            updated parameters list,    */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
#if (NX_SECURE_TLS_TLS_1_3_ENABLED)

UINT _nx_secure_tls_1_3_crypto_init(NX_SECURE_TLS_SESSION *tls_session)
{
UINT                                 status = NX_NOT_SUCCESSFUL;
NX_SECURE_TLS_ECDHE_HANDSHAKE_DATA   *ecdhe_data;
UINT                                 length;
UINT                                 i;
NX_SECURE_TLS_ECC                    *ecc_info;


    if (tls_session == NX_NULL)
    {
        return(NX_PTR_ERROR);
    }

    /* Get ECC information from our TLS session. It should have been initialized
       by the application already. */
    ecc_info = &(tls_session -> nx_secure_tls_ecc);

    /* Loop through all supported ECC curves in this session. */
    for (i = 0; i < ecc_info -> nx_secure_tls_ecc_supported_groups_count; i++)
    {
        /* Get the method for this curve. */
        //curve_method = ((NX_CRYPTO_METHOD **)ecc_info -> nx_secure_tls_ecc_curves)[i];

        /* Get the ECDHE structure for our key output. */
        ecdhe_data = &tls_session -> nx_secure_tls_key_material.nx_secure_tls_ecc_key_data[i];

        /* Save off the curve ID so we can select the server's chosen key/curve later. */
        ecdhe_data -> nx_secure_tls_ecdhe_named_curve = ecc_info -> nx_secure_tls_ecc_supported_groups[i];

        /* Output the public key to our handshake data structure - we need the length of that buffer. */
        length = sizeof(ecdhe_data -> nx_secure_tls_ecdhe_public_key);

        /* Generate ECC keys and store in our TLS session. */
        status = _nx_secure_tls_ecc_generate_keys(tls_session -> nx_secure_tls_session_ciphersuite, tls_session -> nx_secure_tls_protocol_version,
                                                  tls_session -> nx_secure_tls_1_3, tls_session -> nx_secure_tls_crypto_table,
                                                  &tls_session -> nx_secure_tls_handshake_hash, ecc_info, &tls_session -> nx_secure_tls_key_material,
                                                  &tls_session -> nx_secure_tls_credentials, ecdhe_data -> nx_secure_tls_ecdhe_named_curve, NX_FALSE,
                                                  ecdhe_data -> nx_secure_tls_ecdhe_public_key, &length, ecdhe_data,
                                                  tls_session -> nx_secure_public_cipher_metadata_area,
                                                  tls_session -> nx_secure_public_cipher_metadata_size,
                                                  tls_session -> nx_secure_public_auth_metadata_area,
                                                  tls_session -> nx_secure_public_auth_metadata_size);

        /* Set the actual length of the generated key. */
        ecdhe_data -> nx_secure_tls_ecdhe_public_key_length = (USHORT)length;

        if (status != NX_SUCCESS)
        {
            return(status);
        }

    }


    return(status);
}

#endif
