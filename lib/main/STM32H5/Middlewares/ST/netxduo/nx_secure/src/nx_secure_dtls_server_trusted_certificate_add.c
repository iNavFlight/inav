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
/*    _nx_secure_dtls_server_trusted_certificate_add      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds a certificate to the trusted store for a DTLS    */
/*    server instance. This is required only if X.509 Client verification */
/*    and authentication is enabled using                                 */
/*    nx_secure_dtls_server_x509_client_verify_configure.                 */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            DTLS server control block     */
/*    certificate                           Pointer to trusted certificate*/
/*    cert_id                               Numeric id for certificate    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_trusted_certificate_add                              */
/*                                          Add cert to TLS session       */
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
UINT _nx_secure_dtls_server_trusted_certificate_add(NX_SECURE_DTLS_SERVER *server_ptr,
                                                    NX_SECURE_X509_CERT *certificate, UINT cert_id)
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

    /* Add certificate to the first session. */
    if (num_sessions > 0)
    {
        /* Get the first session. */
        current_session = &(server_ptr -> nx_dtls_server_sessions[0]);

        /* Get the internal TLS session instance. */
        tls_session = &(current_session -> nx_secure_dtls_tls_session);

        certificate -> nx_secure_x509_cert_identifier = cert_id;

        /* Add the trusted certificate. */
        status = _nx_secure_tls_trusted_certificate_add(tls_session, certificate);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        /* Store the certificates list head for other sessions. */
        list_head = tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store.nx_secure_x509_trusted_certificates;
    }

    /* Add certificate to the remaining sessions. */
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
    NX_PARAMETER_NOT_USED(certificate);
    NX_PARAMETER_NOT_USED(cert_id);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

