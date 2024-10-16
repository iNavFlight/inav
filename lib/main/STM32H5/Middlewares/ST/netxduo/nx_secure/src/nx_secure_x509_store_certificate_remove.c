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
/*    _nx_secure_x509_store_certificate_remove            PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes a certificate from an X509 certificate store  */
/*    in a caller-specified position (local device certificates, remote   */
/*    certs, or the trusted store).                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    store                                 Pointer to certificate store  */
/*    name                                  Name for cert matching        */
/*    location                              Location of certificate       */
/*    cert_id                               ID for cert match.            */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_certificate_list_remove                             */
/*                                          Remove certificate from list  */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_local_certificate_remove                             */
/*                                          Remove certificate from TLS   */
/*                                           local store                  */
/*    _nx_secure_tls_remote_certificate_free                              */
/*                                          Free remote certificate       */
/*    _nx_secure_tls_server_certificate_remove                            */
/*                                          Remove certificate from TLS   */
/*                                           server store                 */
/*    _nx_secure_tls_trusted_certificate_remove                           */
/*                                          Remove certificate from TLS   */
/*                                           trusted store                */
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
UINT _nx_secure_x509_store_certificate_remove(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                              NX_SECURE_X509_DISTINGUISHED_NAME *name,
                                              UINT location, UINT cert_id)
{
UINT                  status;
NX_SECURE_X509_CERT **store_ptr = NX_CRYPTO_NULL;

    /* Store must be non-NULL. */
    if (store == NX_CRYPTO_NULL)
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
    case NX_SECURE_X509_CERT_LOCATION_NONE:     /* Deliberate fall-through. */
    default:
#ifdef NX_CRYPTO_STANDALONE_ENABLE
        status = NX_CRYPTO_INVALID_PARAMETER;
#else
        status = NX_INVALID_PARAMETERS;
#endif /* NX_CRYPTO_STANDALONE_ENABLE */
        break;
    }

    /* Invalid certificate location or other issue. */
    if (status)
    {
        return(status);
    }

    /* Remove the certificate from the selected store. */
    status = _nx_secure_x509_certificate_list_remove(store_ptr, name, cert_id);

    return(status);
}

