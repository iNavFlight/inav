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
/*    _nx_secure_x509_local_certificate_find              PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches a given certificate store for a specific     */
/*    certificate. This is decided by searching the "local" certificate   */
/*    list in the given store for a certificate based on a specific       */
/*    unique ID in case multiple certificates share the same name.        */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    store                                 Pointer to certificate store  */
/*    certificate                           Pointer to cert pointer       */
/*    cert_id                               Unique certificate identifier */
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
/*    _nx_secure_tls_server_certificate_find                              */
/*                                          Find server certificate       */
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
UINT _nx_secure_x509_local_certificate_find(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                            NX_SECURE_X509_CERT **certificate, UINT cert_id)
{
NX_SECURE_X509_CERT *list_head;
NX_SECURE_X509_CERT *current_cert;

    /* Get the first certificate in the local store. */
    list_head = store -> nx_secure_x509_local_certificates;

    if (list_head == NX_CRYPTO_NULL)
    {
        /* No certificates in this store! */
        return(NX_SECURE_X509_CERTIFICATE_NOT_FOUND);
    }

    /* Walk the list until we find a certificate that has a matching ID. */
    current_cert = list_head;

    while (current_cert != NX_CRYPTO_NULL)
    {
        if (current_cert -> nx_secure_x509_cert_identifier == cert_id)
        {
            /* We found a match, return it. */
            if (certificate != NX_CRYPTO_NULL)
            {
                /* If certificate is NULL, just return that one was found, but nothing to return. */
                *certificate = current_cert;
            }

            /* We are OK to quit now, we found the certificate. */
            return(NX_SECURE_X509_SUCCESS);
        }

        /* Advance our current certificate pointer. */
        current_cert = current_cert -> nx_secure_x509_next_certificate;
    }

    /* No matching certificates in this store! */
    return(NX_SECURE_X509_CERTIFICATE_NOT_FOUND);
}

