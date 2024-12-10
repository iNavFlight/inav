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
/*    _nx_secure_tls_client_psk_set                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function sets the pre-shared key (PSK) for a TLS Client in a   */
/*    TLS session control block for use with a remote server that is      */
/*    using a PSK ciphersuite. The PSK is found using an "identity hint"  */
/*    that should match a field in the PSK structure in the TLS session.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    pre_shared_key                        The Preshared Key             */
/*    psk_length                            Length of preshared key       */
/*    psk_identity                          Identity string               */
/*    identity_length                       Length of the identity string */
/*    hint                                  Hint string                   */
/*    hint_length                           Length of the hint string     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_get                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
#ifdef NX_SECURE_ENABLE_PSK_CIPHERSUITES
UINT _nx_secure_tls_client_psk_set(NX_SECURE_TLS_SESSION *tls_session, UCHAR *pre_shared_key, UINT psk_length,
                                   UCHAR *psk_identity, UINT identity_length, UCHAR *hint, UINT hint_length)
{
UINT status;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Make sure the PSK will fit. */
    if (psk_length <= NX_SECURE_TLS_MAX_PSK_SIZE &&
        identity_length <= NX_SECURE_TLS_MAX_PSK_ID_SIZE &&
        hint_length <= NX_SECURE_TLS_MAX_PSK_ID_SIZE)
    {
        /* Save off the PSK and its length. */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_data, pre_shared_key, psk_length); /* Use case of memcpy is verified. */
        tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_data_size = psk_length;

        /* Save off the identity and its length. */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_id, psk_identity, identity_length); /* Use case of memcpy is verified. */
        tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_id_size = identity_length;

        /* Save off the hint and its length. */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_id_hint, hint, hint_length); /* Use case of memcpy is verified. */
        tls_session -> nx_secure_tls_credentials.nx_secure_tls_client_psk.nx_secure_tls_psk_id_hint_size = hint_length;

        status = NX_SUCCESS;
    }
    else
    {
        /* Can't add any more PSKs. */
        status = NX_SECURE_TLS_NO_MORE_PSK_SPACE;
    }

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
}
#endif

