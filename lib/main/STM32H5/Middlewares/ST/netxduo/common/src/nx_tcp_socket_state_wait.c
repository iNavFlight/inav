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
/** NetX Component                                                        */
/**                                                                       */
/**   Transmission Control Protocol (TCP)                                 */
/**                                                                       */
/**************************************************************************/
/**************************************************************************/

#define NX_SOURCE_CODE


/* Include necessary system files.  */

#include "nx_api.h"
#include "nx_tcp.h"
#ifdef NX_ENABLE_HTTP_PROXY
#include "nx_http_proxy_client.h"
#endif /* NX_ENABLE_HTTP_PROXY */


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_socket_state_wait                           PORTABLE C      */
/*                                                           6.2.0        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function waits for the specified socket to reach the specified */
/*    TCP state.                                                          */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to socket             */
/*    desired_state                         Desired TCP state             */
/*    wait_option                           Suspension option             */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_thread_sleep                       Sleep to wait for state change*/
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application                                                         */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*  10-31-2022     Wenhui Xie               Modified comment(s), and      */
/*                                            supported HTTP Proxy,       */
/*                                            resulting in version 6.2.0  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_tcp_socket_state_wait(NX_TCP_SOCKET *socket_ptr, UINT desired_state, ULONG wait_option)
{

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_SOCKET_STATE_WAIT, socket_ptr -> nx_tcp_socket_ip_ptr, socket_ptr, desired_state, socket_ptr -> nx_tcp_socket_state, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Loop to wait for the desired socket state.   */
    for (;;)
    {

        /* Determine if the socket pointer is still valid.  */
        if (socket_ptr -> nx_tcp_socket_id != NX_TCP_ID)
        {

            /* Not still valid, return an error.  */
            return(NX_PTR_ERROR);
        }

        /* Determine if the desired state is present.  */
        if (socket_ptr -> nx_tcp_socket_state == desired_state)
        {

#ifdef NX_ENABLE_HTTP_PROXY
            if ((desired_state != NX_TCP_ESTABLISHED) ||
                (!socket_ptr -> nx_tcp_socket_ip_ptr -> nx_ip_http_proxy_enable) ||
                (!socket_ptr -> nx_tcp_socket_client_type) ||
                (socket_ptr -> nx_tcp_socket_http_proxy_state == NX_HTTP_PROXY_STATE_CONNECTED))
#endif /* NX_ENABLE_HTTP_PROXY */
            {
                /* The desired state is present, return success!  */
                return(NX_SUCCESS);
            }
        }

        /* Check to see if there is more time to wait.  */
        if (wait_option)
        {

            /* Yes, there is more time... sleep for a tick.  */
            tx_thread_sleep(1);

            /* Decrease the wait time.  */
            wait_option--;
        }
        else
        {

            /* Timeout, just return an error.  */
            return(NX_NOT_SUCCESSFUL);
        }
    }
}

