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
/*    _nx_secure_dtls_server_ecc_initialize               PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function initializes supported curve lists for DTLS server     */
/*    instance.                                                           */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            DTLS server control block     */
/*    supported_groups                      List of supported groups      */
/*    supported_group_count                 Number of supported groups    */
/*    curves                                List of curve methods         */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_ecc_initialize         Initialize curve lists        */
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
UINT _nx_secure_dtls_server_ecc_initialize(NX_SECURE_DTLS_SERVER *server_ptr,
                                           const USHORT *supported_groups, USHORT supported_group_count,
                                           const NX_CRYPTO_METHOD **curves)
{
#if defined(NX_SECURE_ENABLE_DTLS) && defined(NX_SECURE_ENABLE_ECC_CIPHERSUITE)
UINT status;
UINT i;
NX_SECURE_DTLS_SESSION *current_session;
NX_SECURE_TLS_SESSION *tls_session;
UINT num_sessions;

    /* Figure out number of sessions. */
    num_sessions = server_ptr -> nx_dtls_server_sessions_count;

    /* Initialize curve list for all sessions. */
    for(i = 0; i < num_sessions; ++i)
    {
        /* Get the current session. */
        current_session = &(server_ptr -> nx_dtls_server_sessions[i]);

        /* Get the internal TLS session instance. */
        tls_session = &(current_session -> nx_secure_dtls_tls_session);

        /* Initialize supported curve list for TLS.  */
        status = _nx_secure_tls_ecc_initialize(tls_session, supported_groups, supported_group_count, curves);

        if(status != NX_SUCCESS)
        {
            return(status);
        }
    }

    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(server_ptr);
    NX_PARAMETER_NOT_USED(supported_groups);
    NX_PARAMETER_NOT_USED(supported_group_count);
    NX_PARAMETER_NOT_USED(curves);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS && NX_SECURE_ENABLE_ECC_CIPHERSUITE */
}

