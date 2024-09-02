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
/*    _nx_secure_remote_certificate_verify                PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yanwu Cai, Microsoft Corporation                                    */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies the authenticity of a certificate provided   */
/*    by the remote host by checking its digital signature against the    */
/*    trusted store, checking the certificate's validity period, and      */
/*    optionally checking the Common Name against the Top-Level Domain    */
/*    (TLD) name used to access the remote host.                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    store                                 Pointer to certificate store  */
/*    certificate                           Pointer to cert chain         */
/*    current_time                          Current timestamp             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Certificate validity status   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_certificate_chain_verify                            */
/*                                          Verify cert against stores    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_verify                            */
/*                                          Verify the server certificate */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  10-31-2022     Yanwu Cai                Initial Version 6.2.0         */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_remote_certificate_verify(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                          NX_SECURE_X509_CERT *certificate, ULONG current_time)
{
UINT status;

    /* Now verify our remote certificate chain. If the certificate can be linked to an issuer in the trusted store
       through an issuer chain, this function will return NX_SUCCESS. */
    status = _nx_secure_x509_certificate_chain_verify(store, certificate, current_time);

    if (status != NX_SUCCESS)
    {

        /* Translate some X.509 return values into TLS return values. NX_SECURE_X509_CERTIFICATE_NOT_FOUND is removed
           as _nx_secure_x509_certificate_chain_verify() will not return this value. */
        switch (status)
        {
        case NX_SECURE_X509_UNSUPPORTED_PUBLIC_CIPHER:
            return(NX_SECURE_TLS_UNSUPPORTED_PUBLIC_CIPHER);
        case NX_SECURE_X509_UNKNOWN_CERT_SIG_ALGORITHM:
            return(NX_SECURE_TLS_UNKNOWN_CERT_SIG_ALGORITHM);
        case NX_SECURE_X509_CERTIFICATE_SIG_CHECK_FAILED:
            return(NX_SECURE_TLS_CERTIFICATE_SIG_CHECK_FAILED);
#ifndef NX_SECURE_ALLOW_SELF_SIGNED_CERTIFICATES
        case NX_SECURE_X509_INVALID_SELF_SIGNED_CERT:
            return(NX_SECURE_TLS_INVALID_SELF_SIGNED_CERT);
#endif
        case NX_SECURE_X509_ISSUER_CERTIFICATE_NOT_FOUND:
            return(NX_SECURE_TLS_ISSUER_CERTIFICATE_NOT_FOUND);
        case NX_SECURE_X509_MISSING_CRYPTO_ROUTINE:
            return(NX_SECURE_TLS_MISSING_CRYPTO_ROUTINE);
        default:
            return(status);
        }
    }

    return(status);
}

