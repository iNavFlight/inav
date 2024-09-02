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
/*    _nx_secure_x509_local_device_certificate_get        PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function searches a given certificate store for an device      */
/*    certificate. This is decided by searching the "local" certificate   */
/*    list in the given store for a certificate. If multiple certificates */
/*    are in the store, the optional name is used to decide.              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    store                                 Pointer to certificate store  */
/*    name                                  Optional name for selection   */
/*    certificate                           Pointer to cert pointer       */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_certificate_list_find Find certificate by name      */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_tls_process_certificate_request                          */
/*                                          Process certificate request   */
/*    _nx_secure_tls_process_client_key_exchange                          */
/*                                          Process ClientKeyExchange     */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*    _nx_secure_tls_send_certificate       Send TLS certificate          */
/*    _nx_secure_tls_send_certificate_verify                              */
/*                                          Send certificate verify       */
/*    _nx_secure_tls_send_server_key_exchange                             */
/*                                          Send ServerKeyExchange        */
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
UINT _nx_secure_x509_local_device_certificate_get(NX_SECURE_X509_CERTIFICATE_STORE *store,
                                                  NX_SECURE_X509_DISTINGUISHED_NAME *name,
                                                  NX_SECURE_X509_CERT **certificate)
{
NX_SECURE_X509_CERT *list_head;
UINT                 status;
NX_SECURE_X509_CERT *current_cert;

    /* Get the first certificate in the local store. */
    list_head = store -> nx_secure_x509_local_certificates;

    if (list_head == NX_CRYPTO_NULL)
    {
        /* No certificates in this store! */
        return(NX_SECURE_X509_CERTIFICATE_NOT_FOUND);
    }

    /* If the name is NX_CRYPTO_NULL, search for identity certificates. */
    if (name == NX_CRYPTO_NULL)
    {
        /* Walk the list until we find a certificate that is an identity certificate for this device
           (it has a private RSA key). */
        current_cert = list_head;

        while (current_cert != NX_CRYPTO_NULL)
        {
            if (current_cert -> nx_secure_x509_certificate_is_identity_cert == NX_CRYPTO_TRUE)
            {
                /* We found a match, return it. */
                if (certificate != NX_CRYPTO_NULL)
                {
                    /* If certificate is NULL, just return that we found one. */
                    *certificate = current_cert;
                }

                /* We are OK to quit now, we found the certificate. */
                return(NX_SECURE_X509_SUCCESS);
            }

            /* Advance our current certificate pointer. */
            current_cert = current_cert -> nx_secure_x509_next_certificate;
        }

        /* No valid certificates in this store! */
        return(NX_SECURE_X509_CERTIFICATE_NOT_FOUND);
    }

    /* At this point, we have a list and a name. Find the certificate with
       the given name. */
    status = _nx_secure_x509_certificate_list_find(&list_head, name, 0, certificate);

    return(status);
}

