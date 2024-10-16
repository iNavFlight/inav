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
/*    _nx_secure_dtls_session_trusted_certificate_add     PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function adds a trusted certificate to a DTLS session instance.*/
/*    The certificate is used to verify incoming certificates from remote */
/*    hosts. For DTLS Clients, at least one certificate is required. For  */
/*    DTLS Servers, the certificate is only needed if the server is       */
/*    configured for X.509 Client verification.                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS session control block    */
/*    certificate                           Pointer to identity cert      */
/*    cert_id                               Numeric ID for cert           */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_trusted_certificate_add                              */
/*                                          Add certificate to TLS        */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_session_trusted_certificate_add(NX_SECURE_DTLS_SESSION *dtls_session,
                                                    NX_SECURE_X509_CERT *certificate, UINT cert_id)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;
NX_SECURE_TLS_SESSION *tls_session;

    /* Get the internal TLS session instance. */
    tls_session = &(dtls_session -> nx_secure_dtls_tls_session);

    /* Set the certificate ID. */
    certificate->nx_secure_x509_cert_identifier = cert_id;

    /* Add the certificate with the provided ID assigned above.  */
    status = _nx_secure_tls_trusted_certificate_add(tls_session, certificate);

    return(status);
#else
    NX_PARAMETER_NOT_USED(dtls_session);
    NX_PARAMETER_NOT_USED(certificate);
    NX_PARAMETER_NOT_USED(cert_id);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}
