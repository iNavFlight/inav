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
/*    _nx_secure_x509_certificate_list_remove             PORTABLE C      */
/*                                                           6.1.6        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes an NX_SECURE_X509_CERT instance from          */
/*    a certificate linked list, keyed on the Common Name field.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    list_head                             Pointer to list head pointer  */
/*    name                                  Distinguished name for match  */
/*    cert_id                               Numeric ID for match          */
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
/*    _nx_secure_x509_store_certificate_remove                            */
/*                                          Remove certificate from store */
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
UINT _nx_secure_x509_certificate_list_remove(NX_SECURE_X509_CERT **list_head,
                                             NX_SECURE_X509_DISTINGUISHED_NAME *name, UINT cert_id)
{
NX_SECURE_X509_CERT **current_cert;
INT                   compare_result;

    /* Check to see if the head of the certificates list is NULL. */
    if (list_head == NX_CRYPTO_NULL)
    {

        /* Our certificate pointer was NULL, so return error. */
#ifdef NX_CRYPTO_STANDALONE_ENABLE
        return(NX_CRYPTO_PTR_ERROR);
#else
        return(NX_PTR_ERROR);
#endif /* NX_CRYPTO_STANDALONE_ENABLE */
    }
    else
    {
        /* Walk the list until we find a certificate with a matching CN. */
        current_cert = list_head;

        while ((*current_cert) != NX_CRYPTO_NULL)
        {
            /* If name is passed as NX_NULL, use the cert_id to match. */
            if (name == NX_CRYPTO_NULL)
            {
                /* Check the cert_id against the ID in the certificate. */
                compare_result = ((*current_cert) -> nx_secure_x509_cert_identifier == cert_id) ? 0 : 1;
            }
            else
            {
                /* Check the common name passed in against the parsed certificate CN. */
                compare_result = _nx_secure_x509_distinguished_name_compare(name, &(*current_cert) -> nx_secure_x509_distinguished_name, NX_SECURE_X509_NAME_COMMON_NAME);
            }

            if (compare_result == 0)
            {
                /* We found a match, remove it from the linked list. */
                *current_cert = (*current_cert) -> nx_secure_x509_next_certificate;

                /* We are OK to quit now, we removed the certificate. */
                return(NX_SECURE_X509_SUCCESS);
            }

            /* Advance our current certificate pointers. */
            current_cert = &(*current_cert) -> nx_secure_x509_next_certificate;
        }
    }

    return(NX_SECURE_X509_CERTIFICATE_NOT_FOUND);
}

