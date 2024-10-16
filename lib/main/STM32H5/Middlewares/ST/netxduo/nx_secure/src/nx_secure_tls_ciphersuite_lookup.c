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
/*    _nx_secure_tls_ciphersuite_lookup                   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns data about a selected ciphersuite for use     */
/*    in various TLS internal functions, such as the ciphers used and     */
/*    associated key sizes.                                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS session control block     */
/*    ciphersuite                           Ciphersuite value             */
/*    info                                  Pointer to ciphersuite info   */
/*                                            structure (output)          */
/*    priority                              Priority index of ciphersuite */
/*                                            in the ciphersuite table    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_dtls_process_clienthello   Process ClientHello           */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*    _nx_secure_tls_process_serverhello    Process ServerHello           */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), return   */
/*                                            priority of selected suite, */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_ciphersuite_lookup(NX_SECURE_TLS_SESSION *tls_session, UINT ciphersuite,
                                       NX_SECURE_TLS_CIPHERSUITE_INFO const **info, USHORT *priority)
{
USHORT                          index;
NX_SECURE_TLS_CIPHERSUITE_INFO *cipher_table;
USHORT                          cipher_table_size;

    cipher_table = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_ciphersuite_lookup_table;
    cipher_table_size = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_ciphersuite_lookup_table_size;

    /* The number of ciphersuites is very small so a linear search should be fine. */
    for (index = 0; index < cipher_table_size; ++index)
    {
        /* See if the ciphersuite is supported. */
        if (cipher_table[index].nx_secure_tls_ciphersuite == ciphersuite)
        {
            /* Return the ciphersuite information. */
            *info = &cipher_table[index];

            /* Return the priority index (lower number == higher priority). */
            *priority = index;
            return(NX_SUCCESS);
        }
    }

    /* No ciphersuite found, suite unknown. */
    return(NX_SECURE_TLS_UNKNOWN_CIPHERSUITE);
}

