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
/*    _nx_secure_trusted_certificate_add                  PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yanwu Cai, Microsoft Corporation                                    */
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
/*    store                                 Pointer to certificate store  */
/*    certificate                           Pointer to certificate        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_store_certificate_add Add certificate to trusted    */
/*                                           store                        */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Yanwu Cai                Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_trusted_certificate_add(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                        NX_SECURE_X509_CERT *certificate)
{
UINT status;


    /* Add the certificate to the TLS session credentials X509 store. */
    status = _nx_secure_x509_store_certificate_add(certificate, store,
                                                   NX_SECURE_X509_CERT_LOCATION_TRUSTED);

    /* Translate some X.509 return values into TLS return values. */
    if (status == NX_SECURE_X509_CERT_ID_DUPLICATE)
    {
        return(NX_SECURE_TLS_CERT_ID_DUPLICATE);
    }



    return(status);
}

