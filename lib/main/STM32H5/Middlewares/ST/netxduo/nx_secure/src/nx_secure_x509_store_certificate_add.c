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
/*    _nx_secure_x509_store_certificate_add               PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds a certificate to an X509 certificate store in a  */
/*    caller-specified position (local device certificates, remote certs, */
/*    or the trusted store).                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    certificate                           Pointer to certificate        */
/*    store                                 Pointer to certificate store  */
/*    location                              Location to put certificate   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_certificate_list_add  Add certificate to list       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_local_certificate_add  Add local certificate to      */
/*                                            TLS session                 */
/*    _nx_secure_tls_remote_certificate_allocate                          */
/*                                          Allocate remote certificate   */
/*    _nx_secure_tls_remote_certificate_free                              */
/*                                          Free remote certificate       */
/*    _nx_secure_tls_trusted_certificate_add                              */
/*                                          Add trusted certificate to    */
/*                                            TLS session                 */
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
UINT _nx_secure_x509_store_certificate_add(NX_SECURE_X509_CERT *certificate,
                                           NX_SECURE_X509_CERTIFICATE_STORE *store, UINT location)
{
UINT                  status;
NX_SECURE_X509_CERT **store_ptr = NX_CRYPTO_NULL;
UINT                  duplicates_ok = NX_CRYPTO_FALSE;

    /* Certificate and store must be non-NULL. */
    if (certificate == NX_CRYPTO_NULL || store == NX_CRYPTO_NULL)
    {
#ifdef NX_CRYPTO_STANDALONE_ENABLE
        return(NX_CRYPTO_PTR_ERROR);
#else
        return(NX_PTR_ERROR);
#endif /* NX_CRYPTO_STANDALONE_ENABLE */
    }

    status = NX_SECURE_X509_SUCCESS;

    /* Pick our store based on location. */
    switch (location)
    {
    case NX_SECURE_X509_CERT_LOCATION_LOCAL:
        store_ptr = &store -> nx_secure_x509_local_certificates;
        break;
    case NX_SECURE_X509_CERT_LOCATION_REMOTE:
        store_ptr = &store -> nx_secure_x509_remote_certificates;
        break;
    case NX_SECURE_X509_CERT_LOCATION_TRUSTED:
        store_ptr = &store -> nx_secure_x509_trusted_certificates;
        break;
    case NX_SECURE_X509_CERT_LOCATION_EXCEPTIONS:
        store_ptr = &store -> nx_secure_x509_certificate_exceptions;
        break;
    case NX_SECURE_X509_CERT_LOCATION_FREE:
        store_ptr = &store -> nx_secure_x509_free_certificates;
        duplicates_ok = NX_CRYPTO_TRUE;
        break;
    case NX_SECURE_X509_CERT_LOCATION_NONE:     /* Deliberate fall-through. */
    default:
#ifdef NX_CRYPTO_STANDALONE_ENABLE
        status = NX_CRYPTO_INVALID_PARAMETER;
#else
        status = NX_INVALID_PARAMETERS;
#endif /* NX_CRYPTO_STANDALONE_ENABLE */
        break;
    }

    /* If we are adding a certificate with a numeric identifier, it is OK to add duplicates. */
    if (certificate -> nx_secure_x509_cert_identifier != 0)
    {
        duplicates_ok = NX_CRYPTO_TRUE;
    }

    /* Invalid certificate location or other issue. */
    if (status)
    {
        return(status);
    }

    /* Add the certificate to the selected store. */
    status = _nx_secure_x509_certificate_list_add(store_ptr, certificate, duplicates_ok);

    return(status);
}

