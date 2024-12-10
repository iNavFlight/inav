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
/**    Datagram Transport Layer Security (DTLS)                           */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SECURE_SOURCE_CODE

#include "nx_secure_dtls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_dtls_server_trusted_certificate_remove   PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes a trusted certificate from a DTLS server      */
/*    using either the Common Name or the numeric ID assigned when the    */
/*    certificate was added.                                              */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            DTLS server control block     */
/*    common_name                           Certificate common name       */
/*    common_name_length                    Length of common name string  */
/*    cert_id                               Numeric ID for certificate    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_store_certificate_remove                            */
/*                                          Remove certificate using      */
/*                                            certificate ID              */
/*    _nx_secure_tls_trusted_certificate_remove                           */
/*                                          Remove certificate from store */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), and      */
/*                                            fixed compiler warnings,    */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_server_trusted_certificate_remove(NX_SECURE_DTLS_SERVER *server_ptr,
                                                       UCHAR *common_name, UINT common_name_length, UINT cert_id)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;
UINT i;
NX_SECURE_DTLS_SESSION *current_session;
NX_SECURE_TLS_SESSION *tls_session;
NX_SECURE_X509_CERT *list_head = NX_NULL;
UINT num_sessions;


    /* Figure out number of sessions. */
    num_sessions = server_ptr -> nx_dtls_server_sessions_count;

    /* Remove certificate from the first session. */
    if (num_sessions > 0)
    {
        /* Get the first session. */
        current_session = &(server_ptr -> nx_dtls_server_sessions[0]);

        /* Get the internal TLS session instance. */
        tls_session = &(current_session -> nx_secure_dtls_tls_session);

        if(cert_id != 0)
        {
            /* Remove certificate using certificate ID. */
            status = _nx_secure_x509_store_certificate_remove(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                              NX_NULL, NX_SECURE_X509_CERT_LOCATION_TRUSTED, cert_id);
        }
        else
        {
            /* Remove the trusted certificate with the given common name. */
            status = _nx_secure_tls_trusted_certificate_remove(tls_session, common_name, common_name_length);
        }

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        /* Store the certificates list head for other sessions. */
        list_head = tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store.nx_secure_x509_trusted_certificates;
    }

    /* Remove certificate from the remaining sessions. */
    for(i = 1; i < num_sessions; ++i)
    {
        /* Get the current session. */
        current_session = &(server_ptr -> nx_dtls_server_sessions[i]);

        /* Get the internal TLS session instance. */
        tls_session = &(current_session -> nx_secure_dtls_tls_session);

        /* Set the trusted certificates list to the same as the first session. */
        tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store.nx_secure_x509_trusted_certificates = list_head;
    }


    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(common_name);
    NX_PARAMETER_NOT_USED(common_name_length);
    NX_PARAMETER_NOT_USED(cert_id);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

