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
/*    _nx_secure_dtls_session_local_certificate_remove    PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes a local identity certificate from a DTLS      */
/*    session.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS session control block    */
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
/*    _nx_secure_tls_local_certificate_remove                             */
/*                                          Remove certificate from TLS   */
/*    _nx_secure_x509_store_certificate_remove                            */
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
/*  09-30-2020     Timothy Stapko           Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_dtls_session_local_certificate_remove(NX_SECURE_DTLS_SESSION *dtls_session,
                                                      UCHAR *common_name, UINT common_name_length, UINT cert_id)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;
NX_SECURE_TLS_SESSION *tls_session;

    /* Get the internal TLS session instance. */
    tls_session = &(dtls_session -> nx_secure_dtls_tls_session);

    if(cert_id != 0)
    {
        /* Remove certificate using certificate ID. */
        status = _nx_secure_x509_store_certificate_remove(&tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store,
                                                          NX_NULL, NX_SECURE_X509_CERT_LOCATION_LOCAL, cert_id);
    }
    else
    {
        /* Remove the local certificate with the given common name. */
        status = _nx_secure_tls_local_certificate_remove(tls_session, common_name, common_name_length);
    }

    return(status);
#else
    NX_PARAMETER_NOT_USED(dtls_session);
    NX_PARAMETER_NOT_USED(common_name);
    NX_PARAMETER_NOT_USED(common_name_length);
    NX_PARAMETER_NOT_USED(cert_id);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

