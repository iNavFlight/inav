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
/*    _nx_secure_dtls_session_delete                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes a DTLS session object, returning any resources*/
/*    to the system.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    dtls_session                          DTLS session control block    */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_session_reset         Clear DTLS control block      */
/*    _nx_secure_tls_session_delete         Delete TLS session            */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
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
UINT  _nx_secure_dtls_session_delete(NX_SECURE_DTLS_SESSION *dtls_session)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT status;


    /* Reset the DTLS state so this socket can be reused. */
    _nx_secure_dtls_session_reset(dtls_session);

    /* Delete TLS session.  */
    status = _nx_secure_tls_session_delete(&dtls_session -> nx_secure_dtls_tls_session);

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Remove the DTLS instance from the created list. */
    /* See if the DTLS instance is the only one on the list. */
    if (dtls_session == dtls_session -> nx_secure_dtls_created_next)
    {

        /* Only created DTLS instance, just set the created list to NULL. */
        _nx_secure_dtls_created_ptr = NX_NULL;
    }
    else
    {

        /* Otherwise, not the only created DTLS, link-up the neighbors. */
        if (dtls_session -> nx_secure_dtls_created_next != NX_NULL)
        {
            (dtls_session -> nx_secure_dtls_created_next) -> nx_secure_dtls_created_previous =
                    dtls_session -> nx_secure_dtls_created_previous;
        }

        (dtls_session -> nx_secure_dtls_created_previous) -> nx_secure_dtls_created_next =
            dtls_session -> nx_secure_dtls_created_next;

        /* See if we have to update the created list head pointer. */
        if (_nx_secure_dtls_created_ptr == dtls_session)
        {

            /* Yes, move the head pointer to the next link. */
            _nx_secure_dtls_created_ptr = dtls_session -> nx_secure_dtls_created_next;
        }
    }
    _nx_secure_dtls_created_count--;

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
#else
    NX_PARAMETER_NOT_USED(dtls_session);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

