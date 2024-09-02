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
/*    _nx_secure_tls_record_hash_calculate                PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function generates the Message Authentication Code (MAC) value */
/*    from previously added data. The MAC is placed at the end of all     */
/*    encrypted TLS messages.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*    record_hash                           Pointer to output hash buffer */
/*    hash_length                           Length of hash                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    [nx_crypto_operation]                 Crypto operation              */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_send_record            Send the TLS record           */
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
UINT _nx_secure_tls_record_hash_calculate(NX_SECURE_TLS_SESSION *tls_session, UCHAR *record_hash,
                                          UINT *hash_length)
{
UINT                                  hash_size;
UINT                                  status;
const NX_CRYPTO_METHOD               *authentication_method;

    /* We need to generate a Message Authentication Code (MAC) for each record during an "active" TLS session
       (following a ChangeCipherSpec message). The hash algorithm is determined by the ciphersuite, and HMAC
       is used with that hash algorithm to protect the TLS record contents from tampering.

       The MAC is generated as:

       HMAC_hash(MAC_write_secret, seq_num + type + version + length + data);

     */

    if (tls_session -> nx_secure_tls_session_ciphersuite == NX_NULL)
    {

        /* Likely internal error since at this point ciphersuite negotiation was theoretically completed. */
        return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
    }

    /* Get our authentication method from the ciphersuite. */
    authentication_method = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash;

    /* Assert that we have an authentication mechanism. */
    NX_ASSERT(authentication_method -> nx_crypto_operation != NX_NULL);

    /* Get the hash size and MAC secret for our current session. */
    hash_size = tls_session -> nx_secure_tls_session_ciphersuite -> nx_secure_tls_hash_size;

    /* Calculate the final hash for the data in question. */
    status = authentication_method -> nx_crypto_operation(NX_CRYPTO_HASH_CALCULATE,
                                                          tls_session -> nx_secure_hash_mac_handler,
                                                          (NX_CRYPTO_METHOD*)authentication_method,
                                                          NX_NULL,
                                                          0,
                                                          NX_NULL,
                                                          0,
                                                          NX_NULL,
                                                          record_hash,
                                                          NX_SECURE_TLS_MAX_HASH_SIZE,
                                                          tls_session -> nx_secure_hash_mac_metadata_area,
                                                          tls_session -> nx_secure_hash_mac_metadata_size,
                                                          NX_NULL,
                                                          NX_NULL);

    if (status)
    {
        return(status);
    }

    if (authentication_method -> nx_crypto_cleanup)
    {
        status = authentication_method -> nx_crypto_cleanup(tls_session -> nx_secure_hash_mac_metadata_area);
    }

    /* Return how many bytes our hash is since the caller doesn't necessarily know. */
    *hash_length = hash_size;

    return(status);
}

