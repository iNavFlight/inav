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
/*    _nx_secure_tls_remote_certificate_verify            PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
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
/*    tls_session                           TLS session                   */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Certificate validity status   */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_x509_remote_endpoint_certificate_get                     */
/*                                          Get remote host certificate   */
/*    [nx_secure_remote_certificate_verify] Verify the remote certificate */
/*    [nx_secure_tls_session_certificate_callback]                        */
/*                                          Session certificate callback  */
/*    [nx_secure_tls_session_time_function] Session time callback         */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
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
/*                                            updated X.509 return value, */
/*                                            resulting in version 6.1.6  */
/*  01-31-2022     Timothy Stapko           Modified comment(s), and      */
/*                                            improved code coverage      */
/*                                            results,                    */
/*                                            resulting in version 6.1.10 */
/*  07-29-2022     Yuxin Zhou               Modified comment(s), and      */
/*                                            checked expiration for all  */
/*                                            the certs in the chain,     */
/*                                            resulting in version 6.1.12 */
/*  10-31-2022     Yanwu Cai                Modified comment(s), added    */
/*                                            custom secret generation,   */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT _nx_secure_tls_remote_certificate_verify(NX_SECURE_TLS_SESSION *tls_session)
{
UINT                              status;
NX_SECURE_X509_CERT              *remote_certificate;
NX_SECURE_X509_CERTIFICATE_STORE *store;
ULONG                             current_time;


    /* We need to find the remote certificate that represents the endpoint - the leaf in the PKI. */

    /* Process, following X509 basic certificate authentication (RFC 5280):
     *    1. Last certificate in chain is the end entity - start with it.
     *    2. Build chain from issuer to issuer - linked list of issuers. Find in stores: [ Remote, Trusted ]
     *    3. Walk list from end certificate back to a root CA in the trusted store, verifying each signature.
     *       Additionally, any policy enforcement should be done at each step.
     */
    store = &tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store;

    /* Extract the remote certificate processed earlier. */
    status = _nx_secure_x509_remote_endpoint_certificate_get(store, &remote_certificate);

    if (status)
    {
        /* No certificate found, error! */
        return(NX_SECURE_TLS_NO_CERT_SPACE_ALLOCATED);
    }

    /* Assign the TLS Session metadata areas to the certificate for later use. */
    remote_certificate -> nx_secure_x509_public_cipher_metadata_area = tls_session -> nx_secure_public_cipher_metadata_area;
    remote_certificate -> nx_secure_x509_public_cipher_metadata_size = tls_session -> nx_secure_public_cipher_metadata_size;

    remote_certificate -> nx_secure_x509_hash_metadata_area = tls_session -> nx_secure_hash_mac_metadata_area;
    remote_certificate -> nx_secure_x509_hash_metadata_size = tls_session -> nx_secure_hash_mac_metadata_size;

    /* See if we have a timestamp function to get the current time. */
    current_time = 0;
    if (tls_session -> nx_secure_tls_session_time_function != NX_NULL)
    {
        /* Get the current time from our callback. */
        current_time = tls_session -> nx_secure_tls_session_time_function();
    }

    /* Now verify our remote certificate chain. If the certificate can be linked to an issuer in the trusted store
       through an issuer chain, this function will return NX_SUCCESS. */
    status = tls_session -> nx_secure_remote_certificate_verify(store, remote_certificate, current_time);

    if (status != NX_SUCCESS)
    {
        return(status);
    }

    /* Now, see if the application has defined a callback to check additional certificate information. */
    if (tls_session -> nx_secure_tls_session_certificate_callback != NX_NULL)
    {
        /* Call the user-defined callback to allow the application to perform additional validation. */
        status = tls_session -> nx_secure_tls_session_certificate_callback(tls_session, remote_certificate);
    }

    /* If remote certificate verification was a success, we have received credentials
       from the remote host and may now pass Finished message processing once received.
       If this is a TLS Server, defer setting the remote credentials flag until after
       we have received and processed the CertificateVerify message. */
    if (tls_session -> nx_secure_tls_socket_type == NX_SECURE_TLS_SESSION_TYPE_CLIENT && status == NX_SUCCESS)
    {
        tls_session -> nx_secure_tls_received_remote_credentials = NX_TRUE;
    }

    return(status);
}

