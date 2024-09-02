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
/*    _nx_secure_tls_psk_add                              PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds a pre-shared key (PSK) to a TLS session for use  */
/*    with a PSK ciphersuite. The second parameter is the PSK identity    */
/*    used during the TLS handshake to select the proper key.             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    pre_shared_key                        Pointer to PSK data           */
/*    psk_length                            Length of PSK data            */
/*    psk_identity                          PSK identity data             */
/*    identity_length                       Length of identity data       */
/*    hint                                  PSK hint data                 */
/*    hint_length                           Length of hint data           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s), improved */
/*                                            buffer length verification, */
/*                                            verified memcpy use cases,  */
/*                                            resulting in version 6.1    */
/*  07-29-2022     Yuxin Zhou               Modified comment(s),          */
/*                                            fixed PSK size verification,*/
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
#if defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES) || defined(NX_SECURE_ENABLE_ECJPAKE_CIPHERSUITE)
UINT _nx_secure_tls_psk_add(NX_SECURE_TLS_SESSION *tls_session, UCHAR *pre_shared_key,
                            UINT psk_length, UCHAR *psk_identity, UINT identity_length, UCHAR *hint,
                            UINT hint_length)
{
UINT status;
UINT current_index;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    current_index = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_count;

    /* Make sure we have space to add the PSK and its identity data. */
    if ((current_index + 1) < NX_SECURE_TLS_MAX_PSK_KEYS &&
        psk_length <= NX_SECURE_TLS_MAX_PSK_SIZE &&
        identity_length <= NX_SECURE_TLS_MAX_PSK_ID_SIZE &&
        hint_length <= NX_SECURE_TLS_MAX_PSK_ID_SIZE)
    {
        /* Save off the PSK and its length. */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[current_index].nx_secure_tls_psk_data, pre_shared_key, psk_length); /* Use case of memcpy is verified. */
        tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[current_index].nx_secure_tls_psk_data_size = psk_length;

        /* Save off the identity and its length. */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[current_index].nx_secure_tls_psk_id, psk_identity, identity_length); /* Use case of memcpy is verified. */
        tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[current_index].nx_secure_tls_psk_id_size = identity_length;

        /* Save off the identity and its length. */
        NX_SECURE_MEMCPY(tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[current_index].nx_secure_tls_psk_id_hint, hint, hint_length); /* Use case of memcpy is verified. */
        tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[current_index].nx_secure_tls_psk_id_hint_size = hint_length;

        /* Increment the session counter. */
        tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_count = current_index + 1;

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

