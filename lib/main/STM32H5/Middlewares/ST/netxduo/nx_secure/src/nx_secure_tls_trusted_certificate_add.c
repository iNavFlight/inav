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
#include "nx_secure_x509.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_trusted_certificate_add              PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds an initialized NX_SECURE_TLS_CERTIFICATE to a    */
/*    TLS session for use as a trusted Root Certificate - the certificate */
/*    is used to verify incoming certificates from the remote host, by    */
/*    matching the incoming certificate's Issuer Common Name field with   */
/*    that of the certificates in the Trusted Store to find the trusted   */
/*    key used to verify that the incoming certificate is valid.          */
/*    The function may be called repeatedly to add multiple certificates  */
/*    to the internal linked-list.                                        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    certificate                           Pointer to certificate        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*    [nx_secure_trusted_certificate_add]   Add certificate to trusted    */
/*                                            store                       */
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
/*                                            resulting in version 6.1    */
/*  04-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            updated X.509 return value, */
/*                                            resulting in version 6.1.6  */
/*  10-31-2022     Yanwu Cai                Modified comment(s), added    */
/*                                            custom secret generation,   */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_trusted_certificate_add(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_CERT *certificate)
{
UINT status;


    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);


    /* Assign the TLS Session metadata areas to the certificate for later use. */
    certificate -> nx_secure_x509_public_cipher_metadata_area = tls_session -> nx_secure_public_cipher_metadata_area;
    certificate -> nx_secure_x509_public_cipher_metadata_size = tls_session -> nx_secure_public_cipher_metadata_size;

    certificate -> nx_secure_x509_hash_metadata_area = tls_session -> nx_secure_hash_mac_metadata_area;
    certificate -> nx_secure_x509_hash_metadata_size = tls_session -> nx_secure_hash_mac_metadata_size;

    /* Assign the cipher table from the parent TLS session. */
    certificate -> nx_secure_x509_cipher_table = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table;
    certificate -> nx_secure_x509_cipher_table_size = tls_session -> nx_secure_tls_crypto_table -> nx_secure_tls_x509_cipher_table_size;



    /* Add the certificate to the TLS session credentials X509 store. */
    status = tls_session -> nx_secure_trusted_certificate_add(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store, certificate);

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
}

