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

/* Include necessary system files.  */

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_free              PORTABLE C      */
/*                                                           6.1.10       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function moves a remote certificate buffer back into the free  */
/*    store. It is used when the remote certificate is no longer needed,  */
/*    such as when a TLS session is ended.                                */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*    name                                  Certificate distinguished name*/
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_certificate_list_find                               */
/*                                          Find certificate by name      */
/*    _nx_secure_x509_store_certificate_remove                            */
/*                                          Remove certificate from store */
/*    _nx_secure_x509_store_certificate_add                               */
/*                                          Add certificate to store      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_free_all                          */
/*                                          Free all remote certificates  */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), fixed    */
/*                                            certificate allocation bug, */
/*                                            resulting in version 6.1    */
/*  04-02-2021     Timothy Stapko           Modified comment(s),          */
/*                                            updated X.509 return value, */
/*                                            resulting in version 6.1.6  */
/*  01-31-2022     Timothy Stapko           Modified comment(s), and      */
/*                                            improved code coverage      */
/*                                            results,                    */
/*                                            resulting in version 6.1.10 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_remote_certificate_free(NX_SECURE_TLS_SESSION *tls_session,
                                            NX_SECURE_X509_DISTINGUISHED_NAME *name)
{
UINT                              status;
NX_SECURE_X509_CERT              *list_head;
NX_SECURE_X509_CERTIFICATE_STORE *store;
NX_SECURE_X509_CERT              *certificate;

    /* Get the remote certificate store from our TLS session. */
    store = &tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store;

    /* Get the first certificate in the remote store. */
    list_head = store -> nx_secure_x509_remote_certificates;

    /* Find the certificate using it's name. */
    status = _nx_secure_x509_certificate_list_find(&list_head, name, 0, &certificate);

    /* Now status can only be NX_SECURE_X509_CERTIFICATE_NOT_FOUND or NX_SECURE_X509_SUCCESS as
       "&list_head" and "&certificate" are not NULL.
       Translate X.509 return values into TLS return values. */
    if (status == NX_SECURE_X509_CERTIFICATE_NOT_FOUND)
    {
        return(NX_SECURE_TLS_CERTIFICATE_NOT_FOUND);
    }

    /* Make sure status is NX_SECURE_X509_SUCCESS here. */
    NX_ASSERT(status == NX_SECURE_X509_SUCCESS);

    /* Remove the certificate from the remote store. */
    _nx_secure_x509_store_certificate_remove(store, name, NX_SECURE_X509_CERT_LOCATION_REMOTE, 0);

    /* Only user allocated certificate is added back to the free store. */
    if (certificate -> nx_secure_x509_user_allocated_cert)
    {

        /* Add the certificate back to the free store. */
        status = _nx_secure_x509_store_certificate_add(certificate, store, NX_SECURE_X509_CERT_LOCATION_FREE);

        if (status != NX_SUCCESS)
        {

            /* Translate some X.509 return values into TLS return values. */
            if (status == NX_SECURE_X509_CERT_ID_DUPLICATE)
            {
                return(NX_SECURE_TLS_CERT_ID_DUPLICATE);
            }

            return(status);
        }
    }

    /* Return completion status.  */
    return(status);
}

