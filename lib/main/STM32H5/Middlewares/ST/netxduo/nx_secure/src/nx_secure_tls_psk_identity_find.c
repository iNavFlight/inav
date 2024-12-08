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
/*    _nx_secure_tls_psk_find                             PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds a pre-shared key (PSK) in a TLS session for use */
/*    with a TLS 1.3 PSK extension. The PSK is found using an "identity"  */
/*    that should match a field in the PSK structure in the TLS session.  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    psk_data                              Pointer to PSK data           */
/*    psk_length                            Length of PSK data            */
/*    psk_identity                          PSK identity data             */
/*    identity_length                       Length of identity data       */
/*    psk_store_index                       Index of found PSK in store   */
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
/*    _nx_secure_tls_process_clienthello_psk_extension                    */
/*                                          Process an incoming TLS 1.3   */
/*                                            PSK extension               */
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
#if (NX_SECURE_TLS_TLS_1_3_ENABLED) && defined(NX_SECURE_ENABLE_PSK_CIPHERSUITES)
UINT _nx_secure_tls_psk_identity_find(NX_SECURE_TLS_SESSION *tls_session, UCHAR **psk_data, UINT *psk_length,
                                      UCHAR *psk_identity, UINT identity_length, UINT *psk_store_index)
{
UINT psk_list_size;
UINT compare_val;
UINT i;

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    psk_list_size = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_count;

    /* Loop through all PSKs, looking for a matching identity string. */
    for (i = 0; i < psk_list_size; ++i)
    {
        /* Save off the PSK and its length. */
        compare_val = (UINT)NX_SECURE_MEMCMP(tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[i].nx_secure_tls_psk_id, psk_identity, identity_length);

        /* See if the identity matched, and the length is the same (without the length, we could have a
           matching prefix which could be a possible attack vector... */
        if (compare_val == 0 && identity_length == tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[i].nx_secure_tls_psk_id_size)
        {
            /* Found a matching identity, return the associated PSK. */
            *psk_data = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[i].nx_secure_tls_psk_data;
            *psk_length = tls_session -> nx_secure_tls_credentials.nx_secure_tls_psk_store[i].nx_secure_tls_psk_data_size;

            if(psk_store_index != NX_NULL)
            {
                *psk_store_index = i;
            }
            
            /* Release the protection. */
            tx_mutex_put(&_nx_secure_tls_protection);

            return(NX_SUCCESS);
        }
    }

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(NX_SECURE_TLS_NO_MATCHING_PSK);
}
#endif

