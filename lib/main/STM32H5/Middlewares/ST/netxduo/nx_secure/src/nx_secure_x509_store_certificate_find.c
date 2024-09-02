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
/*    _nx_secure_x509_store_certificate_find              PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function finds a certificate in an X509 certificate store      */
/*    based on the Distinguished Name only. The actual position of the    */
/*    certificate is returned along with the certificate itself.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    store                                 Pointer to certificate store  */
/*    name                                  Distinguished name of cert    */
/*    cert_id                               Certificate ID                */
/*    certificate                           (Return) Pointer to cert      */
/*    location                              (Return) Location of cert     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*   _nx_secure_x509_certificate_list_find  Find certificate in list      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_certificate_chain_verify                            */
/*                                          Verify cert against stores    */
/*    _nx_secure_x509_crl_revocation_check  Check revocation in crl       */
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
UINT _nx_secure_x509_store_certificate_find(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                            NX_SECURE_X509_DISTINGUISHED_NAME *name,
                                            UINT cert_id,
                                            NX_SECURE_X509_CERT **certificate, UINT *location)
{
UINT status;

    /* Name and store must be non-NULL. */
    if (name == NX_CRYPTO_NULL || store == NX_CRYPTO_NULL || certificate == NX_CRYPTO_NULL || location == NX_CRYPTO_NULL)
    {
#ifdef NX_CRYPTO_STANDALONE_ENABLE
        return(NX_CRYPTO_PTR_ERROR);
#else
        return(NX_PTR_ERROR);
#endif /* NX_CRYPTO_STANDALONE_ENABLE */
    }

    /* Search each location in turn. */

    /* Start with trusted certificates - if we find one, we are probably done! */
    status = _nx_secure_x509_certificate_list_find(&store -> nx_secure_x509_trusted_certificates, name, cert_id, certificate);
    if (status == NX_SECURE_X509_SUCCESS)
    {
        *location = NX_SECURE_X509_CERT_LOCATION_TRUSTED;
        return(NX_SECURE_X509_SUCCESS);
    }

    /* Next, local certificates. */
    status = _nx_secure_x509_certificate_list_find(&store -> nx_secure_x509_local_certificates, name, cert_id, certificate);
    if (status == NX_SECURE_X509_SUCCESS)
    {
        *location = NX_SECURE_X509_CERT_LOCATION_LOCAL;
        return(NX_SECURE_X509_SUCCESS);
    }

    /* Finally, check remote certs. */
    status = _nx_secure_x509_certificate_list_find(&store -> nx_secure_x509_remote_certificates, name, cert_id, certificate);
    if (status == NX_SECURE_X509_SUCCESS)
    {
        *location = NX_SECURE_X509_CERT_LOCATION_REMOTE;
        return(NX_SECURE_X509_SUCCESS);
    }


    /* If we get here, the certificate was not found in any of the stores. */
    *location = NX_SECURE_X509_CERT_LOCATION_NONE;

    return(NX_SECURE_X509_CERTIFICATE_NOT_FOUND);
}

