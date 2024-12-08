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

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_secure_tls_session_delete                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function deletes a TLS session object, returning any resources */
/*    to the system.                                                      */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    tls_session                           TLS control block             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_tls_session_reset          Clear TLS control block       */
/*    tx_mutex_get                          Get protection mutex          */
/*    tx_mutex_put                          Put protection mutex          */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*    _nx_secure_dtls_session_delete        Delete the DTLS session       */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Timothy Stapko           Initial Version 6.0           */
/*  09-30-2020     Timothy Stapko           Modified comment(s), and      */
/*                                            fixed race condition for    */
/*                                            multithread transmission,   */
/*                                            fixed underflow issue,      */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_secure_tls_session_delete(NX_SECURE_TLS_SESSION *tls_session)
{
UINT status;

    /* Reset the TLS state so this socket can be reused. */
    status = _nx_secure_tls_session_reset(tls_session);

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Remove the TLS instance from the created list. */
    /* See if the TLS instance is the only one on the list. */
    if (tls_session == tls_session -> nx_secure_tls_created_next)
    {

        /* Only created TLS instance, just set the created list to NULL. */
        _nx_secure_tls_created_ptr = NX_NULL;
    }
    else
    {

        /* Otherwise, not the only created TLS, link-up the neighbors. */
        if(tls_session -> nx_secure_tls_created_next != NX_NULL)
        {
            (tls_session -> nx_secure_tls_created_next) -> nx_secure_tls_created_previous =
                    tls_session -> nx_secure_tls_created_previous;
        }

        (tls_session -> nx_secure_tls_created_previous) -> nx_secure_tls_created_next =
            tls_session -> nx_secure_tls_created_next;

        /* See if we have to update the created list head pointer. */
        if (_nx_secure_tls_created_ptr == tls_session)
        {

            /* Yes, move the head pointer to the next link. */
            _nx_secure_tls_created_ptr = tls_session -> nx_secure_tls_created_next;
        }
    }

    /* We shouldn't need this conditional but occasionally automated code may
       call delete after all sessions have already been deleted. */
    if(_nx_secure_tls_created_count > 0)
    {
        _nx_secure_tls_created_count--;
    }

    /* Make sure the session is completely reset - set ID to zero for error checking. */
    tls_session -> nx_secure_tls_id = 0;

    /* Delete the mutex used for TLS session while transmitting packets. */
    tx_mutex_delete(&(tls_session -> nx_secure_tls_session_transmit_mutex));

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(status);
}

