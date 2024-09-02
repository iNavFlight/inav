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
/*    _nx_secure_dtls_server_x509_client_verify_configure PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function enables and configures X.509 Client certificate       */
/*    verification and authentication for a particular DTLS server        */
/*    instance. The buffer parameter points to space allocated for        */
/*    incoming certificates from remote clients. It must be large enough  */
/*    to hold the entire maximum expected certificate chain size times    */
/*    number of DTLS sessions assigned to the server. The certificate     */
/*    chain size is the number of certificates in the chain times the     */
/*    size of each certificate (and control blocks: NX_SECURE_X509_CERT)  */
/*    Therefore, the buffer size can be determined with the following     */
/*    formula:                                                            */
/*                                                                        */
/*        buffer size = (Number of DTLS sessions) *                       */
/*                      (Max. # of expected certs in chain) *             */
/*             (Max. cert size in bytes + sizeof(NX_SECURE_X509_CERT))    */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            DTLS server control block     */
/*    certs_per_session                     # of certs allocated          */
/*    certs_buffer                          Buffer to use for remote certs*/
/*    buffer_size                           Size of buffer                */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_session_x509_client_verify_configure                 */
/*                                          Configure individual session  */
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
UINT _nx_secure_dtls_server_x509_client_verify_configure(NX_SECURE_DTLS_SERVER *server_ptr, UINT certs_per_session,
                                                         UCHAR *certs_buffer, ULONG buffer_size)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;
UINT i;
NX_SECURE_DTLS_SESSION *current_session;
NX_SECURE_TLS_SESSION *tls_session;
UINT per_session_buffer_size;
UINT num_sessions;
UCHAR *session_buffer_ptr;

    /* Figure out number of sessions. */
    num_sessions = server_ptr->nx_dtls_server_sessions_count;

    /* Check sessions buffer size. */
    if(num_sessions == 0)
    {
        return(NX_INVALID_PARAMETERS);
    }

    /* Calculate the per-session size allocated from the buffer. */
    per_session_buffer_size = buffer_size / num_sessions;

    /* Get a working pointer for our buffer allocation. */
    session_buffer_ptr = certs_buffer;

    /* Loop through all sessions. */
    for(i = 0; i < num_sessions; ++i)
    {
        /* Get the current session. */
        current_session = &(server_ptr->nx_dtls_server_sessions[i]);

        /* Get the internal TLS session instance. */
        tls_session = &(current_session -> nx_secure_dtls_tls_session);

        /* Enable client certificate verification and allocate buffer space for certificates. */
        status = _nx_secure_tls_session_x509_client_verify_configure(tls_session, certs_per_session, session_buffer_ptr, per_session_buffer_size);

        if(status != NX_SUCCESS)
        {
            return(status);
        }

        /* Advance the buffer pointer to the next session buffer carved from the passed-in buffer. */
        session_buffer_ptr += per_session_buffer_size;
    }

    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(certs_per_session);
    NX_PARAMETER_NOT_USED(certs_buffer);
    NX_PARAMETER_NOT_USED(buffer_size);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

