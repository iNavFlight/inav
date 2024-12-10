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
/*    _nx_secure_x509_certificate_chain_verify            PORTABLE C      */
/*                                                           6.1.12       */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function verifies a certificate chain (built using the service */
/*    nx_secure_certificate_chain_build) by checking each issuer back to  */
/*    a certificate in the trusted store of the given X509 store.         */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    store                                 Pointer to certificate store  */
/*    certificate                           Pointer to cert chain         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_certificate_verify    Verify a certificate          */
/*    _nx_secure_x509_store_certificate_find                              */
/*                                          Find a cert in a store        */
/*    _nx_secure_x509_distinguished_name_compare                          */
/*                                          Compare distinguished name    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_verify                            */
/*                                          Verify the server certificate */
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
/*  04-25-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            reorganized internal logic, */
/*                                            resulting in version 6.1.11 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            checked expiration for all  */
/*                                            the certs in the chain,     */
/*                                            resulting in version 6.1.12 */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_x509_certificate_chain_verify(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                              NX_SECURE_X509_CERT *certificate, ULONG current_time)
{
UINT                 status;
NX_SECURE_X509_CERT *current_certificate;
NX_SECURE_X509_CERT *issuer_certificate;
UINT                 issuer_location = NX_SECURE_X509_CERT_LOCATION_NONE;
INT                  compare_result;

    /* Process, following X509 basic certificate authentication (RFC 5280):
     *    1. Last certificate in chain is the end entity - start with it.
     *    2. Build chain from issuer to issuer - linked list of issuers. Find in stores: [ Remote, Trusted ]
     *    3. Walk list from end certificate back to a root CA in the trusted store, verifying each signature.
     *       Additionally, any policy enforcement should be done at each step.
     */

    /* Get working pointer to certificate chain entry. */
    current_certificate = certificate;

    while (current_certificate != NX_CRYPTO_NULL)
    {

        /* Check the certificate expiration against the current time. */
        if (current_time != 0)
        {
            status = _nx_secure_x509_expiration_check(current_certificate, current_time);

            if (status != NX_SECURE_X509_SUCCESS)
            {
                return(status);
            }
        }

        /* See if the certificate is self-signed or not. */
        compare_result = _nx_secure_x509_distinguished_name_compare(&current_certificate -> nx_secure_x509_distinguished_name,
                                                                    &current_certificate -> nx_secure_x509_issuer, NX_SECURE_X509_NAME_ALL_FIELDS);

        if (compare_result != 0)
        {
            /* Find the certificate issuer in the store. */
            status = _nx_secure_x509_store_certificate_find(store, &current_certificate -> nx_secure_x509_issuer, 0, &issuer_certificate, &issuer_location);

            if (status != NX_SECURE_X509_SUCCESS)
            {
                return(NX_SECURE_X509_ISSUER_CERTIFICATE_NOT_FOUND);
            }
        }
        else
        {
#ifndef NX_SECURE_ALLOW_SELF_SIGNED_CERTIFICATES
            /* The certificate is self-signed. If we don't allow that, return error. */
            return(NX_SECURE_X509_INVALID_SELF_SIGNED_CERT);
#else
            /* Certificate is self-signed and we are configured to accept them. */
            issuer_certificate = current_certificate;
#endif
        }

        /* Verify the current certificate against its issuer certificate. */
        status = _nx_secure_x509_certificate_verify(store, current_certificate, issuer_certificate);

        if (status != 0)
        {
            return(status);
        }
        else
        {
            /* The comparison passed, so we have a valid issuer. If the issuer is in the trusted
               store, our chain verification is complete. If the issuer is not in the trusted store,
               then continue the verification process. */
            if (issuer_location == NX_SECURE_X509_CERT_LOCATION_TRUSTED)
            {
                return(NX_SECURE_X509_SUCCESS);
            }

#ifdef NX_SECURE_ALLOW_SELF_SIGNED_CERTIFICATES
            /* Certificate is self-signed and we are configured to accept them. */
            if (issuer_certificate == current_certificate)
            {
                /* Check for self-signed certificate in trusted store. */
                status = _nx_secure_x509_store_certificate_find(store, &current_certificate -> nx_secure_x509_distinguished_name, 0, &issuer_certificate, &issuer_location);
                
                if(status == NX_SECURE_X509_SUCCESS && issuer_location == NX_SECURE_X509_CERT_LOCATION_TRUSTED)
                {
                    return(NX_SECURE_X509_SUCCESS);
                }
                /* Self-signed certificate is not trusted. */
                break;
            }
#endif
        }

        /* Advance our working pointer to the next entry in the list. */
        current_certificate = issuer_certificate;
    } /* End while. */

    /* Certificate is invalid. */
    return(NX_SECURE_X509_CHAIN_VERIFY_FAILURE);
}

