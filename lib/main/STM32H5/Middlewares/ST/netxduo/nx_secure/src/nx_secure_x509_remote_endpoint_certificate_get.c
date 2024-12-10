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
/**    X.509 Digital Certificates                                         */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_x509.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_x509_remote_endpoint_certificate_get     PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches a given certificate store for an endpoint    */
/*    certificate. This is decided by searching the "remote" certificate  */
/*    list in the given store for a certificate that is not the issuer    */
/*    for another certificate in the store.                               */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    store                                 Pointer to certificate store  */
/*    certificate                           Pointer to cert pointer       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_distinguished_name_compare                          */
/*                                          Compare distinguished name    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_certificate_verify                           */
/*                                          Process CertificateVerify     */
/*    _nx_secure_tls_remote_certificate_verify                            */
/*                                          Verify the server certificate */
/*    _nx_secure_tls_send_client_key_exchange                             */
/*                                          Send ClientKeyExchange        */
/*    _nx_secure_tls_process_server_key_exchange                          */
/*                                          Process ServerKeyExchange     */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            removed dependency on TLS,  */
/*                                            resulting in version 6.1.6  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_remote_endpoint_certificate_get(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                                     NX_SECURE_X509_CERT **certificate)
{
NX_SECURE_X509_CERT *compare_cert;
NX_SECURE_X509_CERT *candidate;
NX_SECURE_X509_CERT *list_head;
INT                  compare_value;


    /* Get the first certificate in the remote store. */
    list_head = store -> nx_secure_x509_remote_certificates;
    candidate = list_head;

    if (candidate == NX_CRYPTO_NULL)
    {
        /* No remote certificates in this store! */
        return(NX_SECURE_X509_CERTIFICATE_NOT_FOUND);
    }

    /* At this point, we have multiple certificates in the remote store. We need to loop
       to find the one that isn't an issuer for the others. The list should almost always be
       short (< 5 entries) so optimization isn't critical. */
    while (candidate -> nx_secure_x509_next_certificate != NX_CRYPTO_NULL)
    {
        compare_cert = list_head;

        while (compare_cert != NX_CRYPTO_NULL)
        {
            /* Search the entire list for this certificate's distinguished name in the issuer fields. */
            compare_value = _nx_secure_x509_distinguished_name_compare(&candidate -> nx_secure_x509_distinguished_name,
                                                                       &compare_cert -> nx_secure_x509_issuer, NX_SECURE_X509_NAME_ALL_FIELDS);

            /* If we matched, break out of the loop. */
            if (compare_value == 0)
            {
                break;
            }

            /* Advance the comparison pointer. */
            compare_cert = compare_cert -> nx_secure_x509_next_certificate;
        }

        if (compare_cert != NX_CRYPTO_NULL)
        {
            /* Advance the pointer to the next entry in the list. */
            candidate = candidate -> nx_secure_x509_next_certificate;
        }
        else
        {
            /* We got through the compare loop without matching an issuer field, so break out of the loop
               and return the candidate. */
            break;
        }
    }

    /* Return the candidate. */
    *certificate = candidate;

    /* No matter what we found, it is a certificate so return success - if the certificate
       is invalid for any reason, that will be caught during certificate verification. */
    return(NX_SECURE_X509_SUCCESS);
}

