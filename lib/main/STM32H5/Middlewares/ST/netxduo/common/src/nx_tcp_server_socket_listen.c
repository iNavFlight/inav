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


/**************************************************************************/
/*                                                                        */
/*  FUNCTION                                               RELEASE        */
/*                                                                        */
/*    _nx_tcp_server_socket_listen                        PORTABLE C      */
/*                                                           6.1.8        */
/*  AUTHOR                                                                */
/*                                                                        */
/*    Yuxin Zhou, Microsoft Corporation                                   */
/*                                                                        */
/*  DESCRIPTION                                                           */
/*                                                                        */
/*    This function registers a listen request and a server socket for    */
/*    the specified TCP port.                                             */
/*                                                                        */
/*  INPUT                                                                 */
/*                                                                        */
/*    ip_ptr                                Pointer to IP instance        */
/*    port                                  TCP port number               */
/*    socket_ptr                            Server socket pointer         */
/*    listen_queue_size                     Maximum number of connections */
/*                                            that can be queued          */
/*    tcp_listen_callback                   Callback routine when a       */
/*                                            connect request arrives     */
/*                                                                        */
/*  OUTPUT                                                                */
/*                                                                        */
/*    status                                Completion status             */
/*                                                                        */
/*  CALLS                                                                 */
/*                                                                        */
/*    tx_mutex_get                          Obtain protection mutex       */
/*    tx_mutex_put                          Release protection mutex      */
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
/*  08-02-2021     Yuxin Zhou               Modified comment(s), and      */
/*                                            supported TCP/IP offload,   */
/*                                            resulting in version 6.1.8  */
/*                                                                        */
/**************************************************************************/
UINT  _nx_tcp_server_socket_listen(NX_IP *ip_ptr, UINT port, NX_TCP_SOCKET *socket_ptr, UINT listen_queue_size,
                                   VOID (*tcp_listen_callback)(NX_TCP_SOCKET *socket_ptr, UINT port))
{

struct NX_TCP_LISTEN_STRUCT *listen_ptr;
struct NX_TCP_LISTEN_STRUCT *tail_ptr;
#ifdef NX_NAT_ENABLE
UINT                         bound;
#endif

    /* If trace is enabled, insert this event into the trace buffer.  */
    NX_TRACE_IN_LINE_INSERT(NX_TRACE_TCP_SERVER_SOCKET_LISTEN, ip_ptr, port, socket_ptr, listen_queue_size, NX_TRACE_TCP_EVENTS, 0, 0);

    /* Obtain the IP protection.  */
    tx_mutex_get(&(ip_ptr -> nx_ip_protection), TX_WAIT_FOREVER);

    /* Determine if the server socket is in a proper state.  */
    if (socket_ptr -> nx_tcp_socket_state != NX_TCP_CLOSED)
    {

        /* Release the protection mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return the not closed error code.  */
        return(NX_NOT_CLOSED);
    }

    /* Determine if the server socket has already been bound to port or if a socket bind is
       already pending from another thread.  */
    if ((socket_ptr -> nx_tcp_socket_bound_next) ||
        (socket_ptr -> nx_tcp_socket_bind_in_progress))
    {

        /* Release the protection mutex.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return an already bound error code.  */
        return(NX_ALREADY_BOUND);
    }

    /* Clean connected interface. */
    socket_ptr -> nx_tcp_socket_connect_interface = NX_NULL;

    /* Search through the active listen requests to see if there is already
       one active.  */
    listen_ptr =  ip_ptr -> nx_ip_tcp_active_listen_requests;
    if (listen_ptr)
    {

        /* Search the active listen requests for this port.  */
        do
        {

            /* Determine if there is another listen request for the same port.  */
            if (listen_ptr -> nx_tcp_listen_port == port)
            {

                /* This is a duplicate request, return an error.  */

                /* Release the protection.  */
                tx_mutex_put(&(ip_ptr -> nx_ip_protection));

                /* Return the error code.  */
                return(NX_DUPLICATE_LISTEN);
            }

            /* Move to the next listen request.  */
            listen_ptr =  listen_ptr -> nx_tcp_listen_next;
        } while (listen_ptr != ip_ptr -> nx_ip_tcp_active_listen_requests);
    }

#ifdef NX_NAT_ENABLE
    /* Check if this IP interface has a NAT service. */
    if (ip_ptr -> nx_ip_nat_port_verify)
    {

        /* Yes, so check the port by NAT handler. If NAT does not use this port, allow NetX to use it.  */
        bound = (ip_ptr -> nx_ip_nat_port_verify)(ip_ptr, NX_PROTOCOL_TCP, port);

        /* Check to see if the port has been used by NAT.  */
        if (bound == NX_TRUE)
        {

            /* Release the protection.  */
            tx_mutex_put(&(ip_ptr -> nx_ip_protection));

            /* Return the port unavailable error.  */
            return(NX_DUPLICATE_LISTEN);
        }
    }
#endif

    /* Okay, we have a new listen request.  */

    /* Determine if there is an available listen structure.  */
    if (!ip_ptr -> nx_ip_tcp_available_listen_requests)
    {

        /* No listen structures available, release the protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return a maximum listen error code.  */
        return(NX_MAX_LISTEN);
    }

    /* Remove the first available listen request from the available list.  */
    listen_ptr =  ip_ptr -> nx_ip_tcp_available_listen_requests;
    ip_ptr -> nx_ip_tcp_available_listen_requests =  listen_ptr -> nx_tcp_listen_next;

    /* Setup the listen structure with the port and callback info.  */
    listen_ptr -> nx_tcp_listen_port =        port;
    listen_ptr -> nx_tcp_listen_callback =    tcp_listen_callback;
    listen_ptr -> nx_tcp_listen_socket_ptr =  socket_ptr;

    /* Setup the listen queue information.  */
    listen_ptr -> nx_tcp_listen_queue_maximum =  listen_queue_size;
    listen_ptr -> nx_tcp_listen_queue_current =  0;
    listen_ptr -> nx_tcp_listen_queue_head =     NX_NULL;
    listen_ptr -> nx_tcp_listen_queue_tail =     NX_NULL;

    /* Indicate this socket is a server socket.  */
    socket_ptr -> nx_tcp_socket_client_type =  NX_FALSE;

    /* Move to the listen state.  */
    socket_ptr -> nx_tcp_socket_state =  NX_TCP_LISTEN_STATE;

    /* Socket is not active. Clear the timeout. */
    socket_ptr -> nx_tcp_socket_timeout =  0;

    /* Remember what port is associated for this socket.  */
    socket_ptr -> nx_tcp_socket_port =  port;

    /* Link the listen request on the active list.  */
    if (ip_ptr -> nx_ip_tcp_active_listen_requests)
    {

        /* Nonempty list.  Pickup tail pointer.  */
        tail_ptr =  (ip_ptr -> nx_ip_tcp_active_listen_requests) -> nx_tcp_listen_previous;

        /* Place the new listen request in the list.  */
        (ip_ptr -> nx_ip_tcp_active_listen_requests) -> nx_tcp_listen_previous =  listen_ptr;
        tail_ptr ->  nx_tcp_listen_next =  listen_ptr;

        /* Setup this listen request's links.  */
        listen_ptr -> nx_tcp_listen_previous =  tail_ptr;
        listen_ptr -> nx_tcp_listen_next =      ip_ptr -> nx_ip_tcp_active_listen_requests;
    }
    else
    {

        /* The active listen list is empty.  Add listen request to an empty list.  */
        ip_ptr -> nx_ip_tcp_active_listen_requests =  listen_ptr;
        listen_ptr -> nx_tcp_listen_previous =        listen_ptr;
        listen_ptr -> nx_tcp_listen_next =            listen_ptr;
    }

#ifdef NX_ENABLE_TCPIP_OFFLOAD
    /* Listen to TCP/IP offload interfaces.  */
    if (_nx_tcp_server_socket_driver_listen(ip_ptr, port, socket_ptr))
    {

        /* At least one of the interface fails to listen to port.  */
        _nx_tcp_server_socket_unlisten(ip_ptr, port);

        /* Listen request failure, release the protection.  */
        tx_mutex_put(&(ip_ptr -> nx_ip_protection));

        /* Return an already bound error code.  */
        return(NX_TCPIP_OFFLOAD_ERROR);
    }
#endif /* NX_ENABLE_TCPIP_OFFLOAD */

    /* Successful listen request, release the protection.  */
    tx_mutex_put(&(ip_ptr -> nx_ip_protection));

    /* Return success.  */
    return(NX_SUCCESS);
}

