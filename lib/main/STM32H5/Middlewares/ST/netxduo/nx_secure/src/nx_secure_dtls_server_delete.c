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
/*    _nx_secure_dtls_server_delete                       PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Timothy Stapko, Microsoft Corporation                               */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function frees up the resources used by a DTLS server instance */
/*    when that instance is no longer needed by the application.          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    server_ptr                            DTLS server control block     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_secure_dtls_session_delete        Delete DTLS session           */
/*    nx_udp_socket_unbind                  Unbind the UDP socket         */
/*    nx_udp_socket_delete                  Delete the UDP socket         */
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
UINT _nx_secure_dtls_server_delete(NX_SECURE_DTLS_SERVER *server_ptr)
{
#ifdef NX_SECURE_ENABLE_DTLS
UINT i;
UINT status = NX_SUCCESS;

    /* Loop through all DTLS sessions and delete. */
    for(i = 0; i < server_ptr->nx_dtls_server_sessions_count; ++i)
    {
        _nx_secure_dtls_session_delete(&(server_ptr -> nx_dtls_server_sessions[i]));
    }

    /* Delete the UDP socket used by the server. */
    status = nx_udp_socket_delete(&(server_ptr -> nx_dtls_server_udp_socket));

    if (status)
    {
        return(status);
    }

    /* Get the protection. */
    tx_mutex_get(&_nx_secure_tls_protection, TX_WAIT_FOREVER);

    /* Remove the DTLS server instance from the created list. */
    /* See if the DTLS server instance is the only one on the list. */
    if (server_ptr == server_ptr -> nx_dtls_server_created_next)
    {

        /* Only created DTLS server instance, just set the created list to NULL. */
        _nx_secure_dtls_server_created_ptr = NX_NULL;
    }
    else
    {

        /* Otherwise, not the only created DTLS server, link-up the neighbors. */
        if (server_ptr -> nx_dtls_server_created_next != NX_NULL)
        {
            (server_ptr -> nx_dtls_server_created_next) -> nx_dtls_server_created_previous =
                    server_ptr -> nx_dtls_server_created_previous;
        }

        (server_ptr -> nx_dtls_server_created_previous) -> nx_dtls_server_created_next =
            server_ptr -> nx_dtls_server_created_next;

        /* See if we have to update the created list head pointer. */
        if (_nx_secure_dtls_server_created_ptr == server_ptr)
        {

            /* Yes, move the head pointer to the next link. */
            _nx_secure_dtls_server_created_ptr = server_ptr -> nx_dtls_server_created_next;
        }
    }
    _nx_secure_dtls_server_created_count--;

    /* Release the protection. */
    tx_mutex_put(&_nx_secure_tls_protection);

    return(NX_SUCCESS);
#else
    NX_PARAMETER_NOT_USED(server_ptr);

    return(NX_NOT_SUPPORTED);
#endif /* NX_SECURE_ENABLE_DTLS */
}

