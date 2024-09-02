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


/* Include necessary system files.  */

#include "nx_secure_tls.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_free_all          PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function moves all remote certificate buffers back into the    */
/*    free certificate store, allowing them to be used for a new TLS      */
/*    session.                                                            */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           Pointer to TLS Session        */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_remote_certificate_free                              */
/*                                          Free remote certificate       */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_secure_tls_client_handshake       TLS client state machine      */
/*    _nx_secure_tls_process_clienthello    Process ClientHello           */
/*    _nx_secure_tls_session_renegotiate    Renegotiate TLS session       */
/*    _nx_secure_tls_session_reset          Clear TLS control block       */
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
UINT _nx_secure_tls_remote_certificate_free_all(NX_SECURE_TLS_SESSION *tls_session)
{
UINT                              status = NX_SUCCESS;
NX_SECURE_X509_CERTIFICATE_STORE *store;
NX_SECURE_X509_CERT              *certificate;


    /* Reset the packet buffer if we allocated certificates from it. */
    tls_session->nx_secure_tls_packet_buffer_size = tls_session->nx_secure_tls_packet_buffer_original_size;

    /* Get the remote certificate store from our TLS session. */
    store = &tls_session -> nx_secure_tls_credentials.nx_secure_tls_certificate_store;

    certificate = store -> nx_secure_x509_remote_certificates;

    /* Loop through all remote certificates and remove each. */
    while (certificate != NX_NULL)
    {
        status = _nx_secure_tls_remote_certificate_free(tls_session, &certificate -> nx_secure_x509_distinguished_name);

        if (status != NX_SUCCESS)
        {
            return(status);
        }

        /* Get the new certificate list head after removal. */
        certificate = store -> nx_secure_x509_remote_certificates;
    }


    /* Return completion status.  */
    return(status);
}

