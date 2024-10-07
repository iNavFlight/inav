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

/* Bring in externs for caller checking code.  */

NX_SECURE_CALLER_CHECKING_EXTERNS

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nxe_secure_tls_session_create                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function checks for errors in the TLS session create call.     */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    session_ptr                           TLS session control block     */
/*    crypto_table                          crypto method table           */
/*    metadata_buffer                       Encryption metadata area      */
/*    metadata_size                         Encryption metadata size      */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_session_create         Actual TLS session create     */
/*                                            call                        */
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
UINT  _nxe_secure_tls_session_create(NX_SECURE_TLS_SESSION *session_ptr,
                                     const NX_SECURE_TLS_CRYPTO *cipher_table,
                                     VOID *metadata_area,
                                     ULONG metadata_size)
{
UINT status;
NX_SECURE_TLS_SESSION *created_tls_session;
ULONG created_count;

    if ((session_ptr == NX_NULL) || (cipher_table == NX_NULL) || (metadata_area == NX_NULL))
    {
        return(NX_PTR_ERROR);
    }

    /* Loop to check for the IP instance already created.  */
    created_tls_session = _nx_secure_tls_created_ptr;
    created_count = _nx_secure_tls_created_count;
    while (created_count--)
    {

        /* Is the new ip already created?  */
        if (session_ptr == created_tls_session)
        {

            /* Duplicate tls session created, return an error!  */
            return(NX_PTR_ERROR);
        }

        /* Move to next entry.  */
        created_tls_session = created_tls_session -> nx_secure_tls_created_next;
    }

    /* Check for appropriate caller.  */
    NX_THREADS_ONLY_CALLER_CHECKING

    status = _nx_secure_tls_session_create(session_ptr, cipher_table, metadata_area, metadata_size);

    /* Return completion status.  */
    return(status);
}

