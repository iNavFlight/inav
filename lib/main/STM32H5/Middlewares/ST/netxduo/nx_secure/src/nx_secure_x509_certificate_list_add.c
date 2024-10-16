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
/*    _nx_secure_x509_certificate_list_add                PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds a NX_SECURE_X509_CERT instance to a              */
/*    certificate linked list. These lists are used to store certificates */
/*    for the local device, trusted store, and for allocating space for   */
/*    receiving certificates.                                             */
/*                                                                        */
/*    This function will reject certificates with duplicate distinguished */
/*    names unless "duplicates_ok" is set to non-zero. Some stores (e.g.  */
/*    the free store for incoming remote certificates) need to be able to */
/*    store duplicate certificates.                                       */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    list_head                             Pointer to list head pointer  */
/*    certificate                           Pointer to certificate        */
/*    duplicates_ok                         If true, allow duplicates     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_distinguished_name_compare                          */
/*                                          Compare distinguished name    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    _nx_secure_x509_store_certificate_add Add certificate to free store */
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
UINT _nx_secure_x509_certificate_list_add(NX_SECURE_X509_CERT **list_head,
                                          NX_SECURE_X509_CERT *certificate, UINT duplicates_ok)
{
NX_SECURE_X509_CERT *current_cert;
NX_SECURE_X509_CERT *previous_cert;
INT                  compare_result;

    /* Check to see if the head of the certificates list is NULL. */
    if (*list_head == NULL)
    {
        /* Our certificate pointer was NULL, so just set it to this certificate. */
        *list_head = certificate;
        certificate -> nx_secure_x509_next_certificate = NULL;
    }
    else
    {
        /* There is already a certificate in the list. Walk the list
           until we find the end and add our new certificate. */
        current_cert = *list_head;
        previous_cert = current_cert;

        while (current_cert != NX_CRYPTO_NULL)
        {
            if (current_cert == certificate)
            {
                /* Oops, tried to add the same certificate twice (would lead
                   to circular list)! */
#ifdef NX_CRYPTO_STANDALONE_ENABLE
                return(NX_CRYPTO_INVALID_PARAMETER);
#else
                return(NX_INVALID_PARAMETERS);
#endif /* NX_CRYPTO_STANDALONE_ENABLE */
            }

            /* If the certificate has a non-zero identifier, make sure the identifier wasn't added yet! */
            if (certificate -> nx_secure_x509_cert_identifier != 0 &&
                current_cert -> nx_secure_x509_cert_identifier == certificate -> nx_secure_x509_cert_identifier)
            {
                /* Duplicate ID found - don't add to the list! */
                return(NX_SECURE_X509_CERT_ID_DUPLICATE);
            }

            /* We want to be able to add duplicate entries to some of the certificate stores (e.g. the
               free certificate list which contains uninitialized certs), so conditionally allow duplicates. */
            if (!duplicates_ok)
            {
                /* Make sure we don't try to add the same cert twice. */
                compare_result = _nx_secure_x509_distinguished_name_compare(&current_cert -> nx_secure_x509_distinguished_name,
                                                                            &certificate -> nx_secure_x509_distinguished_name, NX_SECURE_X509_NAME_COMMON_NAME);

                if (compare_result == 0)
                {
                    /* A certificate with the same distinguished name was already added to this list. */
#ifdef NX_CRYPTO_STANDALONE_ENABLE
                    return(NX_CRYPTO_INVALID_PARAMETER);
#else
                    return(NX_INVALID_PARAMETERS);
#endif /* NX_CRYPTO_STANDALONE_ENABLE */
                }
            }

            /* Advance to the next certificate. */
            previous_cert = current_cert;
            current_cert =  current_cert -> nx_secure_x509_next_certificate;
        }

        /* Append the new certificate to the end of the list. */
        previous_cert -> nx_secure_x509_next_certificate = certificate;
        certificate -> nx_secure_x509_next_certificate = NULL;
    }

    return(NX_SECURE_X509_SUCCESS);
}

