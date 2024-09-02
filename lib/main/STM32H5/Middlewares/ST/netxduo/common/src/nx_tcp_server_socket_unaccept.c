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
#include "nx_ipv6.h"

/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_server_socket_unaccept                      PORTABLE C      */
/*                                                           6.1          */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function removes the server socket from association with the   */
/*    port receiving an earlier passive connection.  It is left in a      */
/*    state identical to the state after it was created.                  */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    socket_ptr                            Pointer to new TCP socket     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    _nx_tcp_socket_receive_queue_flush    Release all receive packets   */
/*    tx_mutex_get                          Obtain a protection mutex     */
/*    tx_mutex_put                          Release a protection mutex    */
/*                                                                        */
/*  CALLED BY                                                             */
/*                                                                        */
/*    Application Code                                                    */
/*                                                                        */
/*  RELEASE HISTORY                                                       */
/*                                                                        */
/*    DATE              NAME                      DESCRIPTION             */
/*                                                                        */
/*  05-19-2020     Yuxin Zhou               Initial Version 6.0           */
/*  09-30-2020     Yuxin Zhou               Modified comment(s),          */
/*                                            resulting in version 6.1    */
/*                                                                        */
/**************************************************************************/
UINT  _nx_tcp_server_socket_unaccept(NX_TCP_SOCKET *socket_ptr)
{

struct NX_TCP_LISTEN_STRUCT *listen_ptr;
NX_IP                       *ip_ptr;
UINT                         index;
UINT                         port;


    /* Pickup the associated IP structure.  */
    ip_ptr =  socket_ptr -> nx_tcp_socket_ip_ptr;

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_SERVER_SOCKET_UNACCEPT, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, 0, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Obtain the IP mutex so we can access the IP and socket data structures.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if the socket is in a state of disconnect.  */
    if ((socket_ptr -> nx_tcp_socket_state >= NX_TCP_CLOSE_WAIT) ||
        ((socket_ptr -> nx_tcp_socket_state == NX_TCP_CLOSED) && (socket_ptr -> nx_tcp_socket_bound_next)))
    {

        /* If trace is enabled, insert this event into the trace buffer.  */
        NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_LISTEN_STATE, NX_TRACE_INTERNAL_EVENTS, 0, 0);

        /* Force to the listen state.  */
        socket_ptr -> nx_tcp_socket_state =  NX_TCP_LISTEN_STATE;

        /* Ensure the connect information is cleared.  */
        socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_version =    0;
#ifdef FEATURE_NX_IPV6
        /* Zero out the IP address entry. */
        SET_UNSPECIFIED_ADDRESS(socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v6);
#else /* FEATURE_NX_IPV6 */
        socket_ptr -> nx_tcp_socket_connect_ip.nxd_ip_address.v4 = 0;
#endif /* FEATURE_NX_IPV6 */

        socket_ptr -> nx_tcp_socket_connect_port =  0;
    }

    /* Determine if the socket is in the listen state now.  */
    if (socket_ptr -> nx_tcp_socket_state != NX_TCP_LISTEN_STATE)
    {

        /* Release the IP protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return an error code.  */
        return(NX_NOT_LISTEN_STATE);
    }

    /* Check for a thread suspended for disconnect processing to complete.  */
    if (socket_ptr -> nx_tcp_socket_disconnect_suspended_thread)
    {

        /* Call the disconnect thread suspension cleanup routine.  */
        _nx_tcp_disconnect_cleanup(socket_ptr -> nx_tcp_socket_disconnect_suspended_thread NX_CLEANUP_ARGUMENT);
    }

    /* Remove the TCP socket form the associated port.  */

    /* Pickup the port number in the TCP socket structure.  */
    port =  socket_ptr -> nx_tcp_socket_port;

    /* Calculate the hash index in the TCP port array of the associated IP instance.  */
    index =  (UINT)((port + (port >> 8)) & NX_TCP_PORT_TABLE_MASK);

    /* Determine if this is the only socket bound on this port list.  */
    if (socket_ptr -> nx_tcp_socket_bound_next == socket_ptr)
    {

        /* Yes, this is the only socket on the port list.  */

        /* Clear the list head pointer and the next pointer in the socket.  */
        ip_ptr -> nx_ip_tcp_port_table[index] =   NX_NULL;
        socket_ptr -> nx_tcp_socket_bound_next =  NX_NULL;
    }
    else if (socket_ptr -> nx_tcp_socket_bound_next)
    {

        /* Relink the neighbors of this TCP socket.  */

        /* Update the links of the adjacent sockets.  */
        (socket_ptr -> nx_tcp_socket_bound_next) -> nx_tcp_socket_bound_previous = socket_ptr -> nx_tcp_socket_bound_previous;
        (socket_ptr -> nx_tcp_socket_bound_previous) -> nx_tcp_socket_bound_next =  socket_ptr -> nx_tcp_socket_bound_next;

        /* Determine if the head of the port list points to the socket being removed.
           If so, we need to move the head pointer.  */
        if (ip_ptr -> nx_ip_tcp_port_table[index] == socket_ptr)
        {

            /* Yes, we need to move the port list head pointer.  */
            ip_ptr -> nx_ip_tcp_port_table[index] =  socket_ptr -> nx_tcp_socket_bound_next;
        }

        /* Clear the next pointer in the socket to indicate it is no longer bound.  */
        socket_ptr -> nx_tcp_socket_bound_next =  NX_NULL;
    }
    else
    {

        /* Not bound, so search through the active listen requests to see if this
           socket is an active listen socket.  */
        listen_ptr =  ip_ptr -> nx_ip_tcp_active_listen_requests;
        if (listen_ptr)
        {

            /* Search the active listen requests for this port.  */
            do
            {

                /* Determine if we are releasing a socket that is listening.  */
                if (listen_ptr -> nx_tcp_listen_socket_ptr == socket_ptr)
                {

                    /* Remove the socket from the listener.  A relisten will be required to receive another
                       connection.  */
                    listen_ptr -> nx_tcp_listen_socket_ptr =  NX_NULL;
                    break;
                }

                /* Move to the next listen request.  */
                listen_ptr =  listen_ptr -> nx_tcp_listen_next;
            } while (listen_ptr != ip_ptr -> nx_ip_tcp_active_listen_requests);
        }
    }

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_INTERNAL_TCP_STATE_CHANGE, ip_ptr, socket_ptr, socket_ptr -> nx_tcp_socket_state, NX_TCP_CLOSED, NX_TRACE_INTERNAL_EVENTS, 0, 0);

    /* Adjust the socket back to default states.  */
    socket_ptr -> nx_tcp_socket_state =        NX_TCP_CLOSED;
    socket_ptr -> nx_tcp_socket_client_type =  NX_TRUE;

    /* Socket is no longer active. Clear the timeout. */
    socket_ptr -> nx_tcp_socket_timeout =  0;

    /* The socket is off the bound list...  we need to check for queued receive packets and
       if found they need to be released.  */
    if (socket_ptr -> nx_tcp_socket_receive_queue_count)
    {

        /* Release queued receive packets.  */
        _nx_tcp_socket_receive_queue_flush(socket_ptr);
    }

    /* Release the IP protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return success.  */
    return(NX_SUCCESS);
}

