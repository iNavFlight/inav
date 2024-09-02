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
/*    _nx_secure_x509_free_certificate_get                 PORTABLE C     */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function returns a pointer to a "free" certificate structure,  */
/*    which has been previously allocated by the application. This        */
/*    structure is removed from the free list and can be placed into      */
/*    one of the other X509 store locations.                              */
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
/*    None                                                                */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_remote_certificate                           */
/*                                          Process server certificate    */
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
UINT _nx_secure_x509_free_certificate_get(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                          NX_SECURE_X509_CERT **certificate)
{
NX_SECURE_X509_CERT *list_head;

    /* Get the first certificate in the remote store. */
    list_head = store -> nx_secure_x509_free_certificates;

    if (list_head == NX_CRYPTO_NULL)
    {
        /* No certificates in this store! */
        return(NX_SECURE_X509_NO_CERT_SPACE_ALLOCATED);
    }

    /* Use first entry in store. */
    *certificate = list_head;

    /* Remove the certificate from the store. */
    store -> nx_secure_x509_free_certificates = (*certificate) -> nx_secure_x509_next_certificate;
    (*certificate) -> nx_secure_x509_next_certificate = NX_CRYPTO_NULL;

    return(NX_SECURE_X509_SUCCESS);
}

